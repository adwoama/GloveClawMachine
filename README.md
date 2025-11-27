# Glove Claw Machine

A Unity project that uses a wearable sensor glove to control a virtual claw machine.

## Description

This project enables users to interact with a virtual claw machine in Unity using a wearable sensor glove. The glove's sensors detect hand movements and gestures, translating them into controls for operating the claw machine in the virtual environment.

## Requirements

- Unity (compatible version specified in `ProjectSettings/ProjectVersion.txt`)
- Wearable sensor glove hardware
- Claw machine 3D asset (see below)
- Blender (for optional mesh editing and material setup)

## Claw Machine Asset

The claw machine 3D model and textures are not included in this repository. You can download the asset for free from Sketchfab:

**[Claw Machine (Lowpoly, Rigged) by EFX](https://sketchfab.com/3d-models/claw-machine-lowpoly-rigged-a831da37d48f45d4b9845e337e053ba2)**

After downloading, place the models in `Assets/ClawMachine/Models/` and textures in `Assets/ClawMachine/Textures/`.

## Setup

1. Clone this repository
2. Download the claw machine asset from the Sketchfab link above
3. Import the asset into the appropriate folders:
   - Models → `Assets/ClawMachine/Models/`
   - Textures → `Assets/ClawMachine/Textures/`
4. Open the project in Unity
5. Configure your sensor glove hardware connection

## Blender Workflow: Separating and Preparing Glass Panels

To ensure the claw machine’s glass panels render correctly in Unity:

1. **Open the model in Blender**
   - Import the FBX/OBJ from Sketchfab into Blender.

2. **Separate the glass geometry**
   - Enter **Edit Mode** on the claw machine mesh.
   - Select the faces corresponding to the glass panels.
   - Press `P → Selection` to separate them into a new object.

3. **Create a glass material**
   - In **Material Properties**, add a new material (e.g., `GlassMaterial`).
   - Use a **Principled BSDF** shader.
   - Lower the **Alpha** value (0.2–0.4) for transparency.
   - Under **Settings**, set **Blend Mode = Alpha Blend** and **Shadow Mode = None**.

4. **Assign the material**
   - Apply `GlassMaterial` to the separated glass object.
   - Ensure the rest of the enclosure uses the opaque baked material.

5. **Export back to Unity**
   - Apply transforms (`Ctrl + A → Apply All Transforms`).
   - Export as FBX with settings:
     - **Forward = -Z Forward**
     - **Up = Y Up**
     - **Apply Modifiers = true**
     - **Add Leaf Bones = false**
     - **Selected Objects only**
   - Place the exported FBX in `Assets/ClawMachine/Models/`.

6. **Unity setup**
   - In Unity, assign the imported glass material to use **HDRP/Lit** (or URP/Lit if using URP).
   - Set **Surface Type = Transparent** and adjust **Smoothness** for gloss.
   - Add a **Reflection Probe** to the scene for realistic reflections.

## Dependencies

When cloning this repository, you will need to manually set up:

- **Claw Machine Asset**: Download from Sketchfab and place in `Assets/ClawMachine/Models/` and `Assets/ClawMachine/Textures/`.
- **Blender**: Required if you want to edit/separate the glass panels before reimporting into Unity.
- **Unity Render Pipeline**: This project uses HDRP (High Definition Render Pipeline).  
  - Install HDRP via Unity Package Manager.  
  - Assign an HDRP Pipeline Asset in `Project Settings → Graphics`.
- **Sensor Glove SDK/Drivers**: Install the drivers or SDK for your specific glove hardware to enable input mapping.

## License

Please respect the license terms of the claw machine asset from Sketchfab when using this project.