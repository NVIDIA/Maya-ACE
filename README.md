# Maya ACE &middot; [![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE.txt)

**A Reference Client Implementation for NVIDIA ACE Audio2Face-3D Service**

![preview](/docs/resource/samplescene_play.gif)

## What is Maya-ACE

Maya-ACE is a reference implementation designed as a client for the ACE Audio2Face-3D service, which leverages NVIDIA's cutting-edge Digital Human Technology to generate high-quality, audio-driven facial animation. With Maya-ACE, users can effortlessly access and utilize the Audio2Face-3D service through a simple, streamlined interface, or dive into the source code to develop their own custom clients.

This repository includes a Maya plugin, gRPC client libraries, test assets, and a sample scene—everything you need to explore, learn, and innovate with the ACE Audio2Face-3D service. Whether you're looking to integrate this technology into your own projects or simply experiment with its capabilities, Maya-ACE provides a robust foundation.

The project is released under the MIT license, granting you freedom to use and modify the code, with the responsibility to ensure its appropriate use. Please note that NVIDIA assumes no liability for any issues arising from the use of this software.

The ACE Audio2Face-3D service is accessible through [NVIDIA NIM](https://build.nvidia.com/nvidia/audio2face-3d)
, and more information for the service can be found from the [Documentation Hub](https://docs.nvidia.com/ace/latest/modules/a2f-docs/index.html).

## What can be done

![overview](/docs/resource/mace_overview.svg)

### Send Audio, Receive Animation

Maya-ACE allows users to send audio inputs and receive corresponding facial animations. These animations can be directly connected to a blendshape node, enabling you to animate any character in Maya seamlessly.

### Learning with User-Friendly Interface

Maya-ACE provides all the necessary functionalities through a straightforward UI. It serves as an excellent tool for learning and experiencing the ACE Audio2Face-3D service, helping users gain a deeper understanding of how it works.

### Seamless Transition to Tokkio and Unreal Engine

For those looking to expand their workflow, users can elevate their projects by transitioning to the [Kairos Unreal Engine integration](https://docs.nvidia.com/ace/latest/workflows/kairos/index.html) or [Tokkio: an interactive avatar virtual customer service assistant product SDK](https://docs.nvidia.com/ace/latest/workflows/tokkio/index.html).
This allows for the continued use of Audio2Face-3D within other platforms, sharing the same parameters from Maya-ACE for a consistent experience.

### Integration with Standard Maya Nodes

Maya-ACE is designed to work seamlessly with standard Maya nodes, including the blendshape node, making it adaptable to drive any character in your scene.

### Customizable and Extendable

The source code and scripts provided with Maya-ACE can be modified to create a custom pipeline within Maya or to develop a client for other platforms, giving users the flexibility to tailor the tool to their specific needs.

## About ACE

> NVIDIA ACE is a suite of real-time AI solutions for end-to-end development of interactive avatars and digital human applications at-scale.

Additional information about ACE is available from [NVIDIA Documentation Hub](https://docs.nvidia.com/ace/latest/index.html).

For ACE customers to get support, please contact through [NVIDIA Enterprise Support](https://www.nvidia.com/en-us/support/enterprise/)

## More Documentation

- [Requirements](/docs/requirements.md)
- [Getting Started](/docs/gettingstarted.md)
  - [Quickstart](/docs/gettingstarted.md#quickstart)
  - [Set up with Sample Assets](/docs/gettingstarted.md#set-up-with-sample-assets)
  - [Interactive Editing with the Audio2Face-3D Authoring Service](/docs/gettingstarted.md#interactive-editing-with-the-a2f-authoring-service)
  - [Reallusion Sample - High-quality 3D Character Example](/docs/reallusionsample.md#reallusion-sample-project-for-maya-ace)
- [User Interface](/docs/userinterface.md)
  - [Attribute Editor - AceAnimationPlayer](/docs/userinterface.md#attribute-editor---aceanimationplayer)
  - [Main Menu](/docs/userinterface.md#main-menu)
- [Best Practices](/docs/bestpractices.md)
  - [Proposed Workflow](/docs/bestpractices.md#proposed-workflow)
  - [Parameter Tuning Guide](/docs/bestpractices.md#parameter-tuning-guide)
  - [Save Animation through Bake Animation](/docs/bestpractices.md#save-animation-through-bake-animation)
  - [Connecting Custom Blendshapes](/docs/bestpractices.md#connecting-custom-blendshapes)
- [Troubleshooting](/docs/troubleshooting.md)
- [Build and Test Source Code](/docs/buildandtest.md)
  - [Maya ACE Plugin](/docs/buildandtest.md#maya-ace-client)
  - [ACE Client Library](/docs/buildandtest.md#ace-client-library)
  - [ACE gRPC Library](/source/ace_grpc_cpp/README.md)
- [Additional Knowledge](/docs/additionals.md)
  - [Audio File Requirements](/docs/additionals.md#audio-file-requirements)
