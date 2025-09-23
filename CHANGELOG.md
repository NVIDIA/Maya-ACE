# Maya ACE Change Log

## 2.0.0 (27 Aug 2025)

### Audio2Face-3D Local Inference Workflow

- Introduce the Audio2Face-3D Local Inference workflow via the Audio2Face-3D SDK with the latest generative AI models.
  - Add support for all available Audio2Face-3D and Audio2Emotion models, including Audio2Face-3D v3.0 (diffusion-based, multi-identity).
  - Add `A2FAnimationPlayer` node that provides interactive feedback and control over the generative models.
  - Add geometry and transform outputs alongside blendshape weights, enabling the full capabilities of the models.
- Add sample Maya scenes and assets for the new workflow.
- Add `TRT Model Manager` to view and manage TensorRT models.
- Add documentation for the new workflow and instructions to download TensorRT models.
- Enable `Import/Export A2F Parameters` between `AceAnimationPlayer` and `A2FAnimationPlayer`.

### Other New Features

- Add production example using MetaHuman and batch script.
- Support connecting an ACE/A2F Animation Player to multiple blendshape nodes.
- Add support for YAML configuration files.

### Improvements and Bug Fixes

- Move and anchor the `Request New Animation` button to the top of the UI.
- Refine attribute names and the Attribute Editor UI of the ACE Animation Player.
- Move the Config Import/Export dialog to the center of the window, not the screen.
- Remove MEL script usage in the `RequestSendAudio` command.
- Unlock attributes before applying preset attributes.
- Upgrade OpenSSL to 3.0.16.

### Deprecated

- Remove Audio2Face-3D Authoring service-related features and UI
  - Remove Client Type option from Network Settings
  - Remove interactive authoring mode in the ACE Animation Player
- Remove deprecated NVCF function IDs from presets
- Drop support for Maya 2023 and earlier

## 1.2.1 (23 Dec 2024)

### Improvements and Bug Fixes

- Update Authoring Service preset function IDs to new deployments

## 1.2.0 (16 Dec 2024)

### New Features

- Add Network Preset option UI; provide a list of available services
  - List Mark, Claire, and James model service function ids by default
  - Customizable by editing nvcf_ace_presets.json
- Add Service Status Indicator to the UI; a small circle with green, yellow, red, or grey
- Add Import A2F Parameters to ACE menu; import JSON config to an `AceAnimationPlayer` node
- Add Export A2F Service Parameters to ACE menu; export service configuration data from supported services
- Add support for the Audio2Face-3D Authoring service (Early Access users only)
  - Add Client Type option
  - Rename `Request Animation` button to `Connect and Send Audio`
  - Add interactive mode when connected to an authoring service
- Add support for Maya playback caching
- Add gRPC error information to the system messages
- Add emotion and blendshape output to the Output UI
- Clean up input string for using network attributes

### Improvements and Bug Fixes

- Change UI labels to support the Authoring Service
- Change build system to cmake from premake
- Remove Python usage in the `RequestSendAudio` command
- Fix an issue of out_of_breath emotion not affecting preferred emotion
- Fix float precision inconsistency on the UI

## 1.0.0 (28 Aug 2024)

### New Features

- Module package for Autodesk Maya 2024 and 2025 on Windows 64-bit
- Plugin with `AceAnimationPlayer` node
- Scripts and UI for the ACE Audio2Face workflow
- Sample assets and instructions
