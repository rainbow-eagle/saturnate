# Saturnate

Blender animation export scripts under a Sega Saturn friendly format + Sega Saturn visualizer for [Saturn Ring Library](https://github.com/ReyeMe/SaturnRingLib) and [Jo Engine](https://github.com/johannes-fetz/joengine), using [ReyeMe's Model Converter](https://github.com/ReyeMe/ModelConverter-linux).

## Installation and usage

Works on both Linux and Windows.

Requires blender 5 and dotnet to be available in the path.

Requires Saturn Ring Library or Jo Engine (or a custom installation if you use SGL only).

1. Place the saturnate_for_jo_engine directory in Jo Engine's Projects/ directory and/or place the saturnate_for_srl directory in srl's Projects/ directory.
2. Place a valid blender file (see below) in the assets/ directory (or use the one that is provided as an example).
3. Run the "generate_model" script for your OS.
4. Run the compilation script for your OS.
5. Run the "run" script for your OS and emulator.

## Blender constraints

This tool aims to enable SGL's style animations to be imported from Blender. That is, models made out of several meshes organized in a hierarchical manner. This implies some restrictions on the model that can be used in Blender. However, SGL's capabilities can be used to their full extent.

* An object cannot be deformed. That is, modern animations where the model is made of a single mesh (object) that is deformed cannot be exported. Rather, the model is made of several objects that move relatively to each other.
* The animation must be driven by an armature.
* Each object must be parented to a bone. This ensures the object's transformations match those of its parent bone. SGL's matrix hierarchy follows the bone hierarchy of the armature.
* Extra bones that are not parent of a mesh can be present in the armature as root bone or leaves, that is, at the very beginning or at an end of the armature. They cannot be intermediary bones between two objects. This allows the use of control bones, like IK targets and pole targets.
* Bones don't have to be connected to one another.
* Each animation must be saved as an Action in the Action Editor and given a Manual Frame Range with a Start and an End.

## Principle

An animated Blender model is exported in 2 parts:
- the animation data is exported with exportBlenderAnimationAng.py to a custom .MOT format.
- the model itself is exported to Wavefront format (.obj) with exportBlenderWavefront.py (Blender's built-in Wavefront export is not enough); the Wavefront format is then converted to .NYA format thanks to [ReyeMe's Model Converter](https://github.com/ReyeMe/ModelConverter-linux). The resulting files can be easily used in a Sega Saturn game. 

The Saturnate visualizer expects files to bear matching names, e.g. MODEL.MOT and MODEL.NYA, TEST.MOT and TEST.NYA.

Please note that the .MOT format and the provided code do NOT constitute an optimal implementation of Blender animation on the Sega Saturn, as several optimization or features can be added. The sources are only provided as an example of implementation.

## Script usage

### Animation export script
```batch
blender --background --python $(COMMON_DIR)/exportBlenderAnimationAng.py -- [--reverse]
```
The --reverse option will trigger the export of the animation(s) in reverse.

### Wavefront export script
```batch
blender --background --python $(COMMON_DIR)/exportBlenderWavefront.py --
```

The Wavefront export script not only exports the model under the Wavefront format, it also correctly positions objects (meshes) and their centres of rotation for use within the Saturn's system of coordinates and rotations.

See ReyeMe's [explanations about how to use his ModelConverter](https://github.com/ReyeMe/ModelConverter-linux).

## .MOT file format

The .MOT (Motion) format is a binary animation format provided as an example (you can do better, and I definitely plan to move to a better format soon). Each file is composed of three main sections, packed linearly in memory:
* Header : Contains the file metadata, including the number of animations and the number of meshes per model.
* Animation Specifications : sub-headers describing all the animation (frame timing, pose count, and data offsets). One per animation.
* Transform Data (TTTransform in core/animation.h): The raw transformation data (Translation, Rotation, and Hierarchy control).
The philosophy is data oriented:

header, animationSpec1, animationSpec2, ..., animationSpecN, animationData1, animationData2, ..., animationDataN.

This goes contrary to the .NYA file format, which is more object oriented:

header, spec1, data1, spec2, data2, ..., specN, dataN.

### Header

Type | Name | Description
-|-|-
Uint8 | reserved1 | File format descriptor (can change depending on format expressiveness, e.g. would be different if I add support of scaling)
Uint8 | version | Format version number (enables support for better future versions that don't change expressiveness, e.g. would be different if I convey the same information in a more optimized format like, having the popCounts listed only once for the whole file).
Uint8 | animationCount | Total number of different animations in this file.
Uint8 | meshCount | Number of meshes that make up the model.

(see the struct TTMOTFileHeader in core/animation.h)

### Animation Specifications

Each animation defined in the file has a specification entry.

Type | Name | Description
-|-|-
Sint8 | totalFrameCountBetweenPoses | Number of frames a pose remains active before switching to the next one. (Actually the script always exports 1 in this field).
Sint8 | totalPoseCount | Total number of poses (key-frames) composing this animation.
Uint16 | transformOffset | Starting index in the global TTTransform array of the file for this specific animation.

(see the struct TTAnimationSpec in core/animation.h)

### Transform data

This is the core data unit. For every single pose in each animation, there are exactly meshCount transforms, that is, one for each mesh of the model. In other words, it is a 3 dimensional array  
Transform[animationCount][poseCount][meshCount]
where poseCount is variable but animationCount and meshCount are not.

Type | Name | Description
-|-|-
FIXED | xOffset, yOffset, zOffset | Directly used in slTranslate.
ANGLE | xAngle, yAngle, zAngle | Directly used in slRotX/Y/Z.
Uint16 | popCount | Number of times to call slPopMatrix() after drawing the mesh to navigate the hierarchy.

(see the struct TTTransform in core/animation.h)

## Ideas for improvement

* As is, the file format and the code that goes along with it are duplicating information many times and therefore wasting space in memory. Indeed, the popCount and x, y and z offsets will never change for most use cases, so they should be stored "one per model" rather than "one per pose".
* Rather than storing x, y and z offset at all, it's possible to add a constraint to a model: all the bones must be connected. That way, the starting point of a bone is no longer determined by a x, y and z offset from its parent bone, but rather by a rotation + boneLength from its parent bone. That means x, y and z are replaced in most cases by a single data : boneLength. Already as is, most bones are connected to a parent bone, meaning we already have x = 0, y = 0 and z = boneLength in most cases, wasting space on x and y.
* Support of scaling.