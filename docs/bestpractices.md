# Best Practices

## Parameter Tuning Guide

This guide helps you achieve the best facial animation results by following a systematic approach to parameter adjustment. For complete parameter descriptions, see the [User Interface](userinterface.md) documentation.

> **Additional Resource**: The [Audio2Face-3D NIM parameter tuning guide](https://docs.nvidia.com/ace/audio2face-3d-microservice/latest/text/deployment/container-config.html#parameter-tuning-guide) provides server-side configuration details.

### The Global-to-Local Approach

Start with broad adjustments that affect the entire face, then progressively refine specific regions and features. This systematic approach saves time and prevents redundant tweaking.

#### Step 1: Model and Identity Selection

Start with the foundation by choosing the appropriate model and identity:

- **Model**: Select based on your character's design and required features
- **Identity**: Choose the character identity that best matches your target character's facial structure (available in v3.x models)

#### Step 2: Global Face Parameters

These parameters affect the entire face and should be adjusted before fine-tuning specific regions:

- **Skin Strength**
  - Start with default values and adjust based on character's expression needs
  - Higher values create more dynamic skin movement
  - Very high values are useful for testing but may cause unnatural deformations

#### Step 3: Regional Face Controls

Fine-tune upper and lower face regions separately:

- **Strength Parameters** (Lower/Upper Face)
  - Adjust separately for mouth/jaw vs. eyes/brows
  - Start with balanced values, then fine-tune problem areas
- **Smoothing Parameters** (Lower/Upper Face)
  - Lower values: More responsive but may appear jittery
  - Higher values: Natural transitions but may lose detail
- **Face Mask Controls**
  - Adjust Mask Level to change where upper/lower regions meet
  - Increase Mask Softness for more natural blending between regions

#### Step 4: Feature-Specific Adjustments

After regional tuning, refine specific features:

- **Tongue Parameters** (A2F Player only)
  - Adjust strength for speech clarity
  - Use offsets to fix baseline position issues
- **Eyelid/Lip Offsets**
  - Fine-tune resting positions if they look unnatural
  - Small adjustments usually work best

#### Step 5: Emotion Parameters

Emotion parameters bring life and personality to your animations by blending AI-detected emotions with artistic direction:

- **Emotion Strength**
  - Master control - start here before adjusting other emotion settings

- **Auto Emotion** settings
  - Increase Contrast for more distinct emotions
  - Keep Smoothing above 0.3 to avoid jarring transitions
  - Reduce Max Emotions for clearer emotional states

- **Preferred Emotion**
  - Add subtle baseline emotions for character personality
  - Use Weight to blend manual and AI emotions:
    - Low weight = mostly AI-driven
    - High weight = mostly manual control

#### Step 6: Per-Blendshape Refinement

For the finest control, adjust individual blendshapes:

- **Multipliers** (keyframeable)
  - Scale down overactive expressions
  - Amplify subtle expressions that need emphasis
  - Set to 0 to completely disable problematic shapes
  
- **Offsets** (keyframeable)
  - Fix incorrect resting positions
  - Add permanent expression adjustments
  - Use sparingly - usually multipliers are sufficient

### Common Tuning Scenarios

#### Scenario 1: Character appears over-animated

1. First, reduce overall Skin Strength (e.g., from 1.0 to 0.7)
2. If specific areas still look exaggerated, reduce the relevant Face Strength:
   - Lower Face Strength for mouth/jaw issues
   - Upper Face Strength for eye/brow issues
3. Increase Smoothing parameters for more natural transitions

#### Scenario 2: Mouth movements don't match dialogue

1. Increase Lower Face Strength to amplify mouth movements
2. If the mouth's resting position looks wrong, adjust Lip Open Offset
3. For specific pronunciation issues, use blendshape multipliers on individual mouth shapes

#### Scenario 3: Character lacks emotional expression

1. Increase Emotion Strength (e.g., from 0.5 to 0.8) for more visible emotions
2. Add personality by enabling Preferred Emotion:
   - Set a subtle baseline emotion that fits the character
   - Adjust Weight to blend with detected emotions
3. Make emotions more distinct by increasing Auto Emotion Contrast
4. For clearer emotional states, reduce Max Emotions (e.g., from 6 to 2-3)

#### Scenario 4: Fixing specific expression problems

1. Play the animation and identify which blendshapes cause issues
2. For overactive expressions: Reduce their multipliers (e.g., 1.0 → 0.5)
3. For incorrect resting positions: Add small offsets to correct them

### Animation-Specific Fine-Tuning

Blendshape multipliers and offsets can be keyframed for precise control over specific moments:

1. **Find Problem Spots**: Play your animation and note when issues occur
2. **Add Targeted Keyframes**: Only keyframe where needed - avoid over-keyframing
3. **Common Uses for Keyframes**:
   - Emphasize important words or emotional moments
   - Fix pronunciation issues on specific phonemes
   - Create emotional arcs (gradually increase/decrease emotion)
   - Prevent mesh intersections during extreme expressions

### Tips for Effective Parameter Tuning

- **Make Small Changes**: Adjust one parameter at a time and preview the results
- **Save Your Work**: Export successful configurations as presets for future use
- **Consider Your Scene**: Different camera angles or lighting may require different settings
- **Use References**: Study real facial movements or quality animation examples
- **Stay Consistent**: Use similar settings across scenes to maintain character identity

> **Key Principle**: Most animation issues can be fixed with global or regional adjustments. Only use per-blendshape tuning when absolutely necessary.

## Saving Animation through Bake Animation

Maya provides the [Bake Simulation](https://help.autodesk.com/view/MAYAUL/2024/ENU/?guid=GUID-A11424B4-8384-4832-B18D-01264E1A19D1) feature to freeze and save animations generated by the ACE or A2F Player. This allows you to convert the procedural animation into standard keyframes.

1. Select the blendshape node and select all blendshape weights from **the channel box**
1. Click **Bake Simulation** from the menu: (Animation Layout) menu -> Keys -> Bake Simulation
<br /><img src="resource/bake-animation-dialog.png" alt="Bake Simulation dialog box" width="440" />

## Connecting Custom Blendshapes

Audio2Face-3D NIM generates facial animations using the predefined 52 blendshape names and their order based on
[ARKit's Face Anchor BlendShape Locations](https://developer.apple.com/documentation/arkit/arfaceanchor/blendshapelocation).
Managing these blendshape names and their order is crucial
for connecting both ACE and A2F Players effectively to the blendshape nodes.

While ARKit's specification offers a complete set, Audio2Face-3D NIM also supports partial or custom sets of blendshapes through a specific connection process. Once the ACE or A2F Player is created through the menu or receives animations from the service, it automatically updates its output weights with the blendshape names received. When you use the `Connect a Player to Blendshapes` menu, it attempts to find the best matching blendshape targets based on the names.

### Rules for Matching Blendshape Names

- Blendshape names should correspond to one of ARKit's face locations.
- Names are matched in a **case-insensitive** manner.

### Steps for Name-Based Connection

1. After receiving the animation, update the `Output Weights` of the ACE Player or A2F Player node with the appropriate blendshape names.
1. Select the ACE Player or A2F Player node and the blendshape node you want to connect.
1. From the menu, click **Audio2Face** > **Modify** > **Connect a Player to Blendshapes**.

<img src="resource/partial-blendshape-2.png" alt="Partial blendshape connection interface" width="480" />
