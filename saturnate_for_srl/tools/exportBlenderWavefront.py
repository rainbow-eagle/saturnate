import bpy
import sys
from mathutils import Vector

# ======================  ARGUMENT PARSING  ======================
if "--" not in sys.argv:
    print("ERROR: Missing arguments. Use:")
    print("blender --background --python assets/shinobi/exportWavefront.py -- input.blend output.obj")
    sys.exit(1)

args = sys.argv[sys.argv.index("--") + 1:]
if len(args) < 2:
    print("ERROR: Need input .blend and output .obj paths")
    sys.exit(1)

blend_path = args[0]
obj_path = args[1]

print(f"Opening: {blend_path}")
print(f"Will export to: {obj_path}")

bpy.ops.wm.open_mainfile(filepath=blend_path)
if bpy.context.object and bpy.context.object.mode != 'OBJECT':
    bpy.ops.object.mode_set(mode='OBJECT')

# ======================  FIND ARMATURE ======================
armatures = [o for o in bpy.data.objects if o.type == 'ARMATURE']
arm = armatures[0] if armatures else None

if arm:
    print("Armature found → running full Saturn animation-ready pipeline")
    bpy.context.view_layer.objects.active = arm
    arm.select_set(True)
    
    # ======================  FORCE TRUE REST POSE ======================
    # Both the IK model and the baked-in model will export the exact same vertex positions.
    arm.data.pose_position = 'REST'
    bpy.context.view_layer.update()
    print("   → Armature forced to pure REST pose (ignores action + constraints)")
else:
    print("No armature found → exporting as static object (tree, box, prop...)")

# ======================  FULL PIPELINE ONLY IF ARMATURE EXISTS ======================
def reset_all_bones_absolute_final():
    bpy.ops.object.mode_set(mode='POSE')
    for pbone in arm.pose.bones:
        for constraint in pbone.constraints:
            constraint.mute = True
            
    bpy.ops.object.mode_set(mode='EDIT')
    edit_bones = arm.data.edit_bones

    heads_positions = {eb.name: eb.head.copy() for eb in edit_bones}
    lengths = {eb.name: eb.length for eb in edit_bones}
    
    for eb in edit_bones:
        eb.use_connect = False

    print("--- Redressement TOTAL vers Z+ ---")

    for eb in edit_bones:
        start_point = heads_positions[eb.name]
        eb.head = start_point
        eb.tail = start_point + Vector((0, 0, lengths[eb.name]))
        eb.roll = 0

    bpy.ops.object.mode_set(mode='OBJECT')
    bpy.context.view_layer.update()
    print("Succès : Tous les os pointent vers le haut.")

if arm:
    # 1. Bone reset
    reset_all_bones_absolute_final()

    # 2. Manual origin alignment
    print("--- Moving each mesh's local origin to its parent bone's head ---")
    bpy.ops.object.mode_set(mode='OBJECT')
    old_cursor_loc = bpy.context.scene.cursor.location.copy()

    for obj in bpy.data.objects:
        if obj.type == 'MESH' and obj.parent == arm and obj.parent_type == 'BONE':
            pose_bone = arm.pose.bones.get(obj.parent_bone)
            if pose_bone:
                bone_world_head = arm.matrix_world @ pose_bone.head
                bpy.context.scene.cursor.location = bone_world_head
                bpy.ops.object.select_all(action='DESELECT')
                obj.select_set(True)
                bpy.context.view_layer.objects.active = obj
                bpy.ops.object.origin_set(type='ORIGIN_CURSOR')
                print(f"  → [{obj.name}] origin aligned to bone [{obj.parent_bone}]")

    bpy.context.scene.cursor.location = old_cursor_loc
    print("Origin alignment finished.")

    # 3. Collect + rename
    mesh_parents = {o.parent_bone for o in bpy.data.objects if o.parent == arm and o.parent_type == 'BONE'}

    bone_order = []
    def process_bone_order(bone):
        bone_order.append(bone.name)
        children_sorted = sorted(bone.children, key=lambda b: b.name)
        for child in children_sorted:
            process_bone_order(child)

    roots = sorted([b for b in arm.data.bones if b.parent is None], key=lambda b: b.name)
    for r in roots:
        process_bone_order(r)

    ordered_meshes = []
    for bone_name in bone_order:
        if bone_name in mesh_parents:
            m = next((o for o in bpy.data.objects if o.parent == arm and o.parent_bone == bone_name), None)
            if m:
                ordered_meshes.append(m)

    for i, mesh in enumerate(ordered_meshes):
        if "original_name" not in mesh:
            mesh["original_name"] = mesh.name
        mesh.name = f"{i:03d}_{mesh.name}"

    print(f"Ordre synchronisé avec le MOT : {[m.name for m in ordered_meshes]}")

    # 4. Unparent + clear location
    print("--- Préparation des meshes (unparent + Alt+G) ---")
    meshes = [child for child in arm.children if child.type == 'MESH']
    if meshes:
        bpy.ops.object.select_all(action='DESELECT')
        for m in meshes:
            m.select_set(True)
        bpy.ops.object.parent_clear(type='CLEAR_KEEP_TRANSFORM')
        bpy.ops.object.location_clear()

    # ======================  FORCE EXPORT ORDER  ======================
    final_export_list = []
    
    for old_mesh in ordered_meshes:
        new_mesh = old_mesh.copy()
        new_mesh.data = old_mesh.data.copy()
        bpy.context.collection.objects.link(new_mesh)
        
        new_mesh.name = old_mesh["original_name"] if "original_name" in old_mesh else old_mesh.name
        final_export_list.append(new_mesh)
        
        bpy.data.objects.remove(old_mesh, do_unlink=True)

    ordered_meshes = final_export_list
    print(f"Meshes recreated in order: {[m.name for m in ordered_meshes]}")

else:
    ordered_meshes = [o for o in bpy.data.objects if o.type == 'MESH']
    print(f"Found {len(ordered_meshes)} static meshes → exporting directly.")

# ======================  EXPORT  ======================
for m in bpy.data.objects:
    if m.type == 'MESH':
        m.select_set(True)

if ordered_meshes:
    bpy.context.view_layer.objects.active = ordered_meshes[0]

bpy.ops.wm.obj_export(
    filepath=obj_path,
    export_selected_objects=True,
    global_scale=1.0,
    forward_axis='Z',
    up_axis='NEGATIVE_Y',
    export_normals=True,
    export_uv=True,
    export_materials=True,
    apply_modifiers=False
)
print(f"Exported {len(ordered_meshes)} meshes to {obj_path}")

# ======================  RESTORE NAMES ======================
if arm and ordered_meshes:
    for mesh in ordered_meshes:
        if "original_name" in mesh:
            mesh.name = mesh["original_name"]
    print("Original names restored.")

print("Exiting without saving the .blend file.")
bpy.ops.wm.quit_blender()
