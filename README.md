# Maya ACE &middot; [![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE.txt)

**High-Quality Audio-Driven Facial Animation with NVIDIA Audio2Face-3D SDK and ACE Service**

![preview](docs/resource/mace-play-james-a2f.gif)

## What is Maya-ACE

Maya-ACE is a powerful Maya plugin that brings NVIDIA's [Audio2Face-3D technology](docs/concepts.md#audio2face-3d-and-maya-ace)
directly to your creative workflow. It offers two complementary solutions: local GPU processing via the [Audio2Face-3D SDK](docs/concepts.md#local-inference-audio2face-3d-sdk-and-training-framework) for maximum performance and control, and cloud-based processing through the [Audio2Face-3D NIM](docs/concepts.md#audio2face-3d-nim) for scalability.

[Both workflows](docs/concepts.md#workflow-comparison-local-inference-vs-service-based) are fully integrated and optimized within Maya-ACE. Use the local SDK for instant feedback, complete privacy, and interactive parameter tuning on your workstation. Switch to the cloud service when you need distributed processing or enterprise-scale deployment. This flexibility ensures optimal workflow for any project size.

Regardless of your chosen method, Maya-ACE delivers consistent, high-quality facial animations with the same parameters and professional results.

This repository provides everything needed for professional facial animation: a feature-rich Maya plugin, Audio2Face-3D SDK integration, service connectivity via gRPC, comprehensive test assets, and production-ready sample scenes.

The project is released under the MIT license, granting you freedom to use and modify the code, with the responsibility to ensure its appropriate use. Please note that NVIDIA assumes no liability for any issues arising from the use of this software.

## What You Can Do

<picture>
  <source media="(prefers-color-scheme: light)" srcset="docs/resource/mace-overview-2-light.svg">
  <source media="(prefers-color-scheme: dark)" srcset="docs/resource/mace-overview-2-dark.svg">
  <img alt="overview" src="docs/resource/mace-overview-2-light.png">
</picture>

### Audio2Face-3D Application on Local GPU

#### Studio-Quality Facial Animation on Your Local GPU

Generate professional facial animations using NVIDIA's advanced AI models directly on your workstation. No internet connection or external services required.

#### Real-Time Generation with Interactive Control

Create animations instantly with TensorRT-optimized performance. Adjust parameters in interactive mode for immediate visual feedback and precise artistic control.

#### Universal Audio and Character Support

Compatible with any audio input and character design. From photorealistic humans to stylized characters, the flexible architecture adapts to your creative vision.

#### Complete Facial Performance Including Tongue

Capture every detail with high-resolution geometry output and full tongue articulation. Get comprehensive facial performance data for professional productions.

#### Fully Customizable Pipeline

Integrate custom AI models and proprietary blendshape sets. The open architecture empowers technical artists to tailor the pipeline to specific production requirements.

#### Artist-Friendly Interface with Developer Access

Boost productivity with an intuitive Maya-native UI designed for artists. Full source code access enables developers to customize and integrate into existing pipelines.

### Audio2Face-3D NIM Integration

Maya-ACE seamlessly integrates with the [Audio2Face-3D NIM](docs/concepts.md#audio2face-3d-nim) for cloud-based processing and enterprise scalability.
Access the service through [NVIDIA's cloud infrastructure or deploy your own private instance](docs/concepts.md#audio2face-3d-nim) while maintaining the same interface and parameters as local processing.
This unified experience makes Maya-ACE ideal for both individual artists and large studios.

### Seamless Transition to Other Platforms and ACE Services

Maya-ACE is part of [NVIDIA ACE](https://docs.nvidia.com/ace/overview/latest/index.html), a comprehensive suite of real-time AI solutions for interactive avatars and digital human applications.
Extend your pipeline with [Unreal Engine integration](https://docs.nvidia.com/ace/gaming-avatar/latest/index.html) for real-time game characters,
or build interactive customer service avatars using [Tokkio](https://docs.nvidia.com/ace/tokkio/latest/overview/overview.html).
Animation parameters, pose mappings, and Audio2Face settings transfer seamlessly between platforms for consistent results.

### Integration with Standard Maya Nodes

Native integration with Maya's animation system ensures compatibility with blendshape nodes and standard Maya features. Works with any character rig in your scene.

### Customizable and Extendable

Full source code access empowers studios to build custom pipelines or develop clients for other platforms, ensuring complete flexibility for any production environment.

## More Documentation

- [Requirements](docs/requirements.md)
  - [Audio2Face-3D Local Inference Workflow](docs/requirements.md#audio2face-3d-local-inference-workflow)
  - [Audio2Face-3D NIM Service Workflow](docs/requirements.md#audio2face-3d-nim-service-workflow)
  - [Build Environment for Developers](docs/requirements.md#build-environment-for-developers)
- Getting Started
  - [Prerequisites](docs/prerequisites.md)
    - [Install the Plugin](docs/prerequisites.md#install-the-plugin)
    - [Download the Sample Project](docs/prerequisites.md#download-the-sample-project)
    - [Download AI Models](docs/prerequisites.md#download-ai-models)
    - [Build TRT Engines](docs/prerequisites.md#build-trt-engines)
    - [Prepare Access to the Service](docs/prerequisites.md#prepare-access-to-the-service)
    - [Environment Variables](docs/prerequisites.md#environment-variables)
    - [Audio File Requirements](docs/prerequisites.md#audio-file-requirements)
  - [Concepts](docs/concepts.md)
    - [Architecture Overview](docs/concepts.md#architecture-overview)
    - [Workflow Comparison: Local Inference vs NIM](docs/concepts.md#workflow-comparison-local-inference-vs-nim)
    - [Model Selection Guide](docs/concepts.md#model-selection-guide)
    - [Model Outputs](docs/concepts.md#model-outputs)
  - [Quickstarts](docs/quickstart.md)
    - [Sample Scene with Audio2Face-3D Local Inference](docs/quickstart.md#sample-scene-with-audio2face-3d-local-inference)
    - [Sample Scene with Audio2Face-3D Service](docs/quickstart.md#sample-scene-with-audio2face-3d-service)
- User Guides
  - [Setup Instructions](docs/setupinstructions.md)
    - [Setting Up A2F Animation Player on Blendshapes](docs/setupinstructions.md#setting-up-a2f-animation-player-on-blendshapes)
    - [Setting Up A2F Animation Player for the Geometry Output](docs/setupinstructions.md#setting-up-a2f-animation-player-for-the-inference-geometry-output)
    - [Setting Up ACE Animation Player on Blendshapes](docs/setupinstructions.md#setting-up-ace-animation-player-on-blendshapes)
  - [Best Practices](docs/bestpractices.md)
    - [Parameter Tuning Guide](docs/bestpractices.md#parameter-tuning-guide)
    - [Saving Animation through Bake Animation](docs/bestpractices.md#saving-animation-through-bake-animation)
    - [Connecting Custom Blendshapes](docs/bestpractices.md#connecting-custom-blendshapes)
  - [Troubleshooting Guides](docs/troubleshooting.md)
- Samples Files and Production Examples
  - [Sample Maya Project](sample_project/README.md)
    - [Models](sample_project/README.md#models)
    - [Scenes](sample_project/README.md#scenes)
    - [Sound](sample_project/README.md#sound)
    - [Face Assets](sample_project/README.md#face-assets)
  - [Sample Data](sample_data/README.md)
  - [Reallusion High-quality 3D Character Example](docs/reallusionsample.md)
  - [MetaHuman Mapping and Batch Processing](examples/metahuman_batch/README.md)
- Developer Guides
  - [Setting up Development Environment](docs/buildandtest.md#setting-up-development-environment)
    - [CUDA Toolkit](docs/buildandtest.md#cuda-toolkit)
    - [TensorRT](docs/buildandtest.md#tensorrt)
    - [Audio2Face-3D SDK](docs/buildandtest.md#audio2face-3d-sdk)
    - [Maya Devkit](docs/buildandtest.md#maya-devkit)
  - [Building Maya ACE Plugin](docs/buildandtest.md#building-maya-ace-plugin)
    - [Build Commands](docs/buildandtest.md#build-commands)
    - [Debug Build](docs/buildandtest.md#debug-build)
    - [Output Files and Packaging](docs/buildandtest.md#output-files-and-packaging)
    - [Testing](docs/buildandtest.md#testing)
  - [ACE gRPC C++ Library](source/ace_grpc_cpp/README.md)
  - [Troubleshooting during Build](docs/buildandtest.md#troubleshooting)
- [User Interface](docs/userinterface.md)
  - [Compute Status - Timeslider Color](docs/userinterface.md#compute-status---timeslider-color)
  - [Attribute Editor](docs/userinterface.md#attribute-editor)
  - [Main Menu](docs/userinterface.md#main-menu)
  - [TRT Manager Window](docs/userinterface.md#trt-manager-window)
