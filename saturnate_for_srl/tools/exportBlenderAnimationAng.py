import bpy
import math
import sys
import struct
from mathutils import Matrix, Euler
from collections import defaultdict

# ======================  ARGUMENT PARSING  ======================
if "--" not in sys.argv:
    print("ERROR: Missing arguments. Use:")
    print("blender --background --python exportAnimation.py -- [--reverse] input.blend output.MOT")
    sys.exit(1)

args = sys.argv[sys.argv.index("--") + 1:]
if len(args) < 2:
    print("ERROR: Need input .blend and output .MOT paths")
    sys.exit(1)

blend_path = args[-2]
output_path = args[-1]
is_reverse = "--reverse" in args

print(f"Opening: {blend_path}")
print(f"Will export binary animation to: {output_path}")

bpy.ops.wm.open_mainfile(filepath=blend_path)

# ======================  FIND ARMATURE ======================
armatures = [o for o in bpy.data.objects if o.type == 'ARMATURE']

if not armatures:
    print("No armature found → skipping .MOT creation (static object)")
    sys.exit(0)

arm = armatures[0]
bpy.context.view_layer.objects.active = arm
arm.select_set(True)

print(f"Using armature: {arm.name}")

# ======================  COLLECT ALL ANIMATIONS ======================
actions = []

# Force la récupération de TOUTES les actions du fichier
actions = [act for act in bpy.data.actions]
print(f"Total actions dans le fichier: {len(actions)}")

# Ensuite, dans ta boucle d'export :
for act in actions:
    arm.animation_data.action = act
    bpy.context.view_layer.update() # Force Blender à mettre à jour les poses

print(f"Found {len(actions)} animation(s): {[a.name for a in actions]}")

# ======================  HELPERS ======================
def to_fixed(val):
    return int(round(val * 65536))

def deg_to_angle(deg):
    return int(round(deg * (65536.0 / 360.0))) & 0xFFFF

# ======================  EXPORT ======================
def export_saturn_animation_binary(output_path):
    obj = bpy.context.active_object
    arm_scale = obj.scale
    armature_data = obj.data

    mesh_parents = {o.parent_bone for o in bpy.data.objects if o.parent == obj and o.parent_type == 'BONE'}
    valid_bones = [b for b in armature_data.bones if b.children or b.name in mesh_parents]

    effective_children = defaultdict(list)
    effective_parent = {}
    outputted_bones = []

    def process_bone(bone, accum_mat=Matrix.Identity(4), is_root_like=False, use_pose=False, compute_output=False, build_structure=False):
        if use_pose:
            pbone = obj.pose.bones[bone.name]
            rel_mat = pbone.matrix if bone.parent is None else pbone.parent.matrix.inverted() @ pbone.matrix
        else:
            rel_mat = bone.matrix_local if bone.parent is None else bone.parent.matrix_local.inverted() @ bone.matrix_local

        pos = rel_mat.to_translation()
        if is_root_like:
            tx = pos.x * arm_scale.x
            ty = -pos.z * arm_scale.z
            tz = -pos.y * arm_scale.y
            euler = Euler((0, 0, 0), 'ZYX')
        else:
            tx = pos.x * arm_scale.x
            ty = -pos.y * arm_scale.y
            tz = -pos.z * arm_scale.z
            pure_rot_mat = rel_mat.to_3x3().normalized()
            euler = pure_rot_mat.to_euler('ZYX')

        rotX = Matrix.Rotation(euler.x, 3, 'X')
        rotY = Matrix.Rotation(-euler.y, 3, 'Y')
        rotZ = Matrix.Rotation(-euler.z, 3, 'Z')
        rot_mat = rotX @ rotY @ rotZ

        M = rot_mat.to_4x4() @ Matrix.Translation((tx, ty, tz))
        combined_mat = accum_mat @ M

        has_mesh = bone.name in mesh_parents
        children_valides = sorted([c for c in bone.children if c in valid_bones], key=lambda b: b.name)

        if has_mesh and compute_output:
            pos_combined = combined_mat.to_translation()
            pure_rot_s = combined_mat.to_3x3().normalized()
            pos_s = pure_rot_s.inverted() @ pos_combined
            euler_s = pure_rot_s.to_euler('ZYX')

            deg_x = math.degrees(euler_s.x)
            deg_y = math.degrees(euler_s.y)
            deg_z = math.degrees(euler_s.z)

            current_frame_data.append([
                to_fixed(pos_s[0]),
                to_fixed(pos_s[1]),
                to_fixed(pos_s[2]),
                deg_to_angle(deg_x),
                deg_to_angle(deg_y),
                deg_to_angle(deg_z),
                0
            ])

        sub_bones = []
        for child in children_valides:
            next_accum = Matrix.Identity(4) if has_mesh else combined_mat
            child_sub = process_bone(child, next_accum, False, use_pose, compute_output, build_structure)
            if build_structure and child_sub:
                effective_children[bone].append(child_sub[0])
                effective_parent[child_sub[0]] = bone
            sub_bones += child_sub

        return [bone] + sub_bones if has_mesh else sub_bones

    # Build hierarchy & popcounts (once)
    roots = sorted([b for b in armature_data.bones if b.parent is None], key=lambda b: b.name)
    for r in roots:
        outputted_bones += process_bone(r, Matrix.Identity(4), True, use_pose=False, compute_output=False, build_structure=True)

    pop_counts = {}
    for bone in outputted_bones:
        pop_count = 0
        if not effective_children[bone]:
            pop_count = 1
            curr = bone
            while curr in effective_parent:
                parent = effective_parent[curr]
                siblings = effective_children[parent]
                if curr == siblings[-1]:
                    pop_count += 1
                    curr = parent
                else:
                    break
        pop_counts[bone] = pop_count

    meshCount = len(outputted_bones)

    # ======================  COLLECT ALL ANIMATION DATA ======================
    all_specs = []
    all_frame_data = []
    current_transform_index = 0

    anim_list = actions[:] if actions else [None]

    for action in anim_list:
        arm.animation_data.action = action 
        if action is None:
            frame_start = bpy.context.scene.frame_start
            frame_end   = bpy.context.scene.frame_end
        else:
            frame_start = int(action.frame_range[0])
            frame_end   = int(action.frame_range[1])

        num_frames = frame_end - frame_start + 1

        all_specs.append((1, num_frames, current_transform_index))   # (between, poseCount, transformID)

        this_anim_frames = []
        frame_range = range(frame_end, frame_start - 1, -1) if is_reverse else range(frame_start, frame_end + 1)

        for fnum in frame_range:
            bpy.context.scene.frame_set(fnum)
            current_frame_data = []

            for r in roots:
                process_bone(r, Matrix.Identity(4), True, use_pose=True, compute_output=True, build_structure=False)

            for i, t in enumerate(current_frame_data):
                t[6] = pop_counts[outputted_bones[i]]

            this_anim_frames.extend(current_frame_data)

        all_frame_data.append(this_anim_frames)
        current_transform_index += num_frames * meshCount

    print(f"Exporting {len(all_specs)} animation(s) × {meshCount} transform(s) each")

    # ======================  WRITE BINARY FILE ======================
    with open(output_path, 'wb') as f:
        # AnimationSetSpec
        f.write(struct.pack('>BBBB', 1, 0, len(all_specs), meshCount))

        # All AnimationSpec0
        for between, poseCount, transformID in all_specs:
            f.write(struct.pack('>bbH', between, poseCount, transformID))

        # All Transform data (flat)
        for anim_frames in all_frame_data:
            for t in anim_frames:
                f.write(struct.pack('>iiiHHHH', *t))

    print(f"✅ Multi-animation .MOT exported successfully!")
    print(f"    File           : {output_path}")
    print(f"    Animations     : {len(all_specs)}")
    print(f"    MeshCount      : {meshCount}")
    print(f"    Total Transforms : {current_transform_index}")

# ======================  RUN ======================
export_saturn_animation_binary(output_path)
