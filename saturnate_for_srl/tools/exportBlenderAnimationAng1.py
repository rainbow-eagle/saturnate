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
    print("No armature found → skipping .MOT creation")
    sys.exit(0)

arm = armatures[0]
bpy.context.view_layer.objects.active = arm
arm.select_set(True)

print(f"Using armature: {arm.name}")

# ======================  COLLECT ALL ANIMATIONS ======================
actions = [act for act in bpy.data.actions]
print(f"Total actions: {len(actions)}")
print(f"Found animations: {[a.name for a in actions]}")

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
                to_fixed(pos_s[0]),     # 0
                to_fixed(pos_s[1]),     # 1
                to_fixed(pos_s[2]),     # 2
                deg_to_angle(deg_x),    # 3
                deg_to_angle(deg_y),    # 4
                deg_to_angle(deg_z),    # 5
                0                       # 6 popCount placeholder
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

    # === Build hierarchy & popcounts (once) ===
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
    print(f"MeshCount: {meshCount}")

    # ======================  COLLECT STATIC BONE DATA (rest pose) ======================
    bone_data = []
    current_frame_data = []
    for r in roots:
        process_bone(r, Matrix.Identity(4), True, use_pose=False, compute_output=True, build_structure=False)

    for i, t in enumerate(current_frame_data):
        bone = outputted_bones[i]
        bone_data.append([
            t[0], t[1], t[2],           # Rest xOffset, yOffset, zOffset
            pop_counts[bone],
            0,                          # reserved1
            0                           # reserved2
        ])

    # ======================  COLLECT ANIMATION DATA ======================
    all_specs = []
    all_root_positions = []   # TTRootBonePosition per pose
    all_rotations = []        # TTBoneRotation per mesh per pose
    total_poses = 0

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
        all_specs.append((1, num_frames, total_poses))   # transformOffset now points to root positions index

        frame_range = range(frame_end, frame_start - 1, -1) if is_reverse else range(frame_start, frame_end + 1)

        for fnum in frame_range:
            bpy.context.scene.frame_set(fnum)
            current_frame_data = []

            for r in roots:
                process_bone(r, Matrix.Identity(4), True, use_pose=True, compute_output=True, build_structure=False)

            # Root position (first mesh = hip/pelvis)
            root_pos = current_frame_data[0]
            all_root_positions.append([root_pos[0], root_pos[1], root_pos[2]])

            # Rotations for ALL meshes (including root)
            for t in current_frame_data:
                all_rotations.append([t[3], t[4], t[5], 0])  # xAngle, yAngle, zAngle, reserved

        total_poses += num_frames

    print(f"Exporting {len(all_specs)} animation(s) — Total poses: {total_poses} × {meshCount} rotations")

    # ======================  WRITE BINARY FILE ======================
    with open(output_path, 'wb') as f:
        # 1. TTMOTFileHeader
        f.write(struct.pack('>BBBB', 1, 0, len(all_specs), meshCount))

        # 2. TTAnimationSpec
        for between, poseCount, rootOffset in all_specs:
            f.write(struct.pack('>bbH', between, poseCount, rootOffset))

        # 3. TTBone (static per mesh)
        for b in bone_data:
            f.write(struct.pack('>iiiBBH', *b))

        # 4. TTRootBonePosition — one per pose (across all animations)
        for pos in all_root_positions:
            f.write(struct.pack('>iii', *pos))

        # 5. TTBoneRotation — per mesh per pose
        for rot in all_rotations:
            f.write(struct.pack('>HHH H', *rot))

    print(f"✅ Optimized .MOT exported successfully!")
    print(f"    File                    : {output_path}")
    print(f"    Animations              : {len(all_specs)}")
    print(f"    Meshes                  : {meshCount}")
    print(f"    Total Poses             : {total_poses}")
    print(f"    Root positions stored   : {len(all_root_positions)}")
    print(f"    Rotations stored        : {len(all_rotations)}")


# ======================  RUN ======================
export_saturn_animation_binary(output_path)
