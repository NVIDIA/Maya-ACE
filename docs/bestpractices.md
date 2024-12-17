# Best Practices

## Proposed Workflow

Generally, it requires multiple tries to find an optimal setting. Users are expected to work with an iterative process with using Maya-ACE in their projects.
All animations should be downloaded to see the result through any changes; such as paramters. The typical workflow is,

1. Adjust **Parameters**
2. Update animation by clicking **Connect and Send Audio** button
3. **Play** the scene and check the animation with audio
4. **Repeat** from 1 to 3 until satisfied with the result
5. Bake animation of the blendshape weights
6. Save the scene

![](/docs/resource/standard_workflow.png)

## Parameter Tuning Guide

Please check
[ACE Parameter Tuning Guide](https://docs.nvidia.com/ace/latest/modules/a2f-docs/text/param_tuning.html)
to understand individual parameters.

## Save Animation through Bake Animation

Maya provides
[`Bake Simulation`](https://help.autodesk.com/view/MAYAUL/2024/ENU/?guid=GUID-A11424B4-8384-4832-B18D-01264E1A19D1)
to freeze and save animation that is downloaded from the service.

1. Select blendshape node and select all blendshape weights from **the channel box**
1. Click **Bake Simulation** through menu; (Animation Layout) menu -> Keys -> Bake Simulation
<br /><img src="resource/bake_animation_dialog.png" width="440" />

## Connecting Custom Blendshapes

ACE Audio2Face-3D generates facial animations using the predefined 52 blendshape names and order based on
[ARKit’s Face Anchor BlendShape Locations](https://developer.apple.com/documentation/arkit/arfaceanchor/blendshapelocation).
Managing these blendshape names and their order is crucial
for connecting Maya-ACE's AceAnimationPlayer nodes effectively to the blendshape nodes.

While ARKit's specification offers a complete set, ACE Audio2Face-3D also supports partial or custom sets of blendshapes through a specific connection process. Once the AceAnimationPlayer node is created through the menu or receives animations from the service, it automatically updates its output weights with the blendshape names received. When you use the `Connect existing Animation Player` menu, it attempts to find the best matching blendshape targets based on the names.

### Rules for Matching Blendshape Names

- Blendshape names should correspond to one of ARKit’s face locations.
- Names are matched in a case-insensitive manner.

### Steps for Name-Based Connection

1. Complete receiving the animation and update the `Output Weights` of the AceAnimationPlayer node with the appropriate blendshape names.
1. Select the AceAnimationPlayer node and the blendshape node you want to connect.
1. From the menu, click on `Connect existing Animation Player` -> `ACE`.

<img src="resource/partial_blendshape_2.png" width="480" />
