# Concepts

This guide explains the core concepts of Maya ACE and helps you choose the right workflow for your needs.

## Table of Contents

- [Architecture Overview](#architecture-overview)
- [Workflow Comparison: Local Inference vs NIM](#workflow-comparison-local-inference-vs-nim)
  - [Local Inference Workflow](#local-inference-workflow)
  - [NIM Workflow](#nim-workflow)
- [Model Selection Guide](#model-selection-guide)
  - [Model Performance Benchmark](#model-performance-benchmark)
- [Model Outputs](#model-outputs)

## Architecture Overview

Audio2Face-3D uses state-of-the-art deep learning to transform audio input into highly detailed facial animations. Maya ACE integrates this technology through two complementary approaches; local inference workflow and network-based NIM workflow.

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="resource/mace-dataflow-2-dark.svg">
  <source media="(prefers-color-scheme: light)" srcset="resource/mace-dataflow-2-light.svg">
  <img alt="Maya ACE data flow diagram" src="resource/mace-dataflow-2-light.png">
</picture>

## Workflow Comparison: Local Inference vs NIM

Maya ACE provides powerful facial animation capabilities through Audio2Face-3D technology, offering two distinct workflows to suit different use cases and environments.

| Aspect | Audio2Face-3D Local Inference | Audio2Face-3D NIM |
| --- | --- | --- |
| **Controllability** | Real-time interactive control | Request-response based control |
| **Requirements** | NVIDIA GPU, Local A2F-3D model files | Network access to hosted services, no GPU required |
| **Applications** | - A2F Player on Blendshapes<br>- A2F Player on Mesh | - ACE Player on Blendshapes |
| **Available Models** | All models including custom: <br>- A2F-3D v3.0/v2.3<br>- A2E v3.0/v2.2<br>- Custom trained models | Service-hosted models: <br>- A2F-3D v2.3<br>- A2E v2.0 |
| **Outputs** | - Blendshape weights<br>- Face Geometry<br>- Tongue Geometry<br>- Jaw Transform<br>- Eye Rotation<br>- Emotion | - Blendshape weights<br>- Emotion |
| **Setup Complexity** | Moderate - requires model management | Easy - connect and use |
| **Initial Setup Time** | ~15 minutes (includes model download and TRT build) | ~5 minutes (service connection only) |
| **Best Use Cases** | - Creative authoring<br>- Research & development<br>- Offline production<br>- Custom model deployment | - Rapid prototyping<br>- Scalable production<br>- GPU-free environments<br>- Cloud-based pipelines |

### Local Inference Workflow

The local approach runs AI models directly on your machine for maximum control and performance.

#### Audio2Face-3D (A2F-3D) Models

[Audio2Face-3D models](https://huggingface.co/nvidia/Audio2Face-3D-v3.0) are the core AI models that power facial animation:

- **Pre-trained Models**: Ready-to-use models for facial animation (A2F-3D) and emotion detection (A2E)
- **Custom Models**: Train your own using the [Audio2Face-3D Training Framework](https://github.com/NVIDIA/Audio2Face-3D-training-framework)
- **Model Versions**: Support for both latest (v3.0) and legacy (v2.x) model versions

#### Audio2Face-3D SDK (Audio2X-SDK, A2X-SDK, A2F-SDK)

The [Audio2Face-3D SDK](https://github.com/NVIDIA/Audio2Face-3D-SDK) provides the runtime engine for local inference:

- **Model Integration**: Loads and runs Audio2Face-3D models
- **Real-time Processing**: Streams audio to facial animation with minimal latency
- **Parameter Control**: Interactive adjustment of animation parameters

#### TensorRT (TRT) Inference Engine

[NVIDIA TensorRT](https://developer.nvidia.com/tensorrt) accelerates model performance:

- **GPU Optimization**: Converts models to optimized engines for your specific GPU
- **One-time Setup**: Build TensorRT engines once, reuse for all sessions
- **Performance Boost**: Achieves real-time inference speeds

### NIM Workflow

The NIM approach delivers the same Audio2Face-3D models through cloud-based services for scalable deployment.

#### Audio2Face-3D Models (Service-Hosted)

The NIM workflow uses the same core Audio2Face-3D technology:

- **Pre-selected Models**: Service hosts optimized versions of A2F and A2E models
- **Version Compatibility**: Currently supports A2F v2.3 and A2E v2.0
- **Managed Updates**: Models are maintained and updated by the service provider

#### NVIDIA NIM Infrastructure

[NVIDIA NIM](https://www.nvidia.com/en-us/ai-data-science/products/nim-microservices/) provides the service infrastructure:

- **Cloud Deployment**: Run on any NVIDIA-accelerated infrastructure
- **API Access**: Simple REST/gRPC interfaces for animation requests
- **No Local GPU**: Process animations without local hardware requirements

#### Audio2Face-3D NIM Service

The [Audio2Face-3D NIM](https://docs.nvidia.com/ace/audio2face-3d-microservice/1.3/text/getting-started/overview.html) delivers optimized inference as a service:

- **Multi-stream Processing**: Handle multiple concurrent animation requests
- **Elastic Scaling**: Automatically scale to meet demand
- **Quick Start**: Available as a [Audio2Face-3D NIM on Cloud](https://build.nvidia.com/nvidia/audio2face-3d) for immediate use
- **Private Hosting**: Deploy as a [self-hosted service](https://docs.nvidia.com/ace/audio2face-3d-microservice/1.3/text/deployment/container-config.html) for full control

Using the hosted service through Maya ACE enables rapid prototyping without GPU requirements. The tools are free for development and testing.

#### Support

For ACE enterprise customers requiring support, please contact [NVIDIA Enterprise Support](https://www.nvidia.com/en-us/support/enterprise/).

## Model Selection Guide

The following table provides an overview of available models and their key characteristics. Model availability may vary by region and time. For the most up-to-date information, visit the [Audio2Face-3D model collection](https://huggingface.co/collections/nvidia/audio2face-3d-6865d22d6daec4ac85887b17).

| Model | Description | Best Used For |
| ----- | ----------- | ------------- |
| **Audio2Face-3D v3.0** | Latest diffusion-based model supporting multiple character identities | - Best overall quality<br>- High throughput applications<br>- General-purpose use (recommended) |
| **Audio2Face-3D v2.3.x-Claire** | Female voice model optimized for Chinese & English language | - Limited GPU memory<br>- Chinese and phonetically similar languages |
| **Audio2Face-3D v2.3.x-James** | Male voice model optimized for English language | - Limited GPU memory<br>- Strong emotional expression |
| **Audio2Face-3D v2.3-Mark** | Male voice model optimized for English language | - Limited GPU memory<br>- High-resolution facial geometry |
| **Audio2Emotion v2.2** | Dedicated emotion detection model | - Emotion analysis alongside Audio2Face-3D<br>- Enhanced emotional accuracy |
| **Audio2Emotion v3.0** | Advanced emotion detection model (experimental) | - Emotion analysis alongside Audio2Face-3D<br>- Improved emotional accuracy over v2.2 |

### Model Performance Benchmark

Reference System: NVIDIA RTX 5090 (32GB VRAM), Driver 576, Maya 2024

> **Note**: This performance data serves as a reference for comparing models. Actual performance varies based on system configuration. In Maya, visible FPS will be lower due to graph evaluation and viewport rendering.

| Model | Compute Time<sup>1</sup><br>(Geometry) | Node Evaluation<sup>2</sup><br>Time | Throughput<sup>3</sup> | VRAM Usage<sup>4</sup> |
|---------|:----------------:|:------------------:|:------------:|:------------:|
| Audio2Face-3D v3.0 | ~5 ms | ~7 ms | 500 frames/sec | ~1.3 GB |
| Audio2Face-3D v2.3.x-Claire | ~10 ms | ~13 ms | 100 frames/sec | ~0.2 GB |
| Audio2Emotion | ~10 ms | N/A<sup>5</sup> | N/A | ~1.2 GB |

#### ¹ Compute Time (Geometry)

The SDK processing time per frame to generate facial geometry output. Measured as time-to-first-result after parameter change. This includes:

- Model inference time
- Post-processing operations  
- Excludes blendshape weight solving (adds ~5-10 ms when connected)

#### ² Node Evaluation Time

Maya's graph evaluation time for a single A2F Animation Player node.

#### ³ Throughput

Maximum processing speed measured in frames per second. Indicates how quickly the SDK can process audio data when running continuously. Higher values mean faster processing.

#### ⁴ VRAM Usage

Approximate GPU memory consumption when the model is loaded. This includes model weights and runtime buffers. Actual usage may vary slightly based on batch size and GPU architecture.

#### ⁵ N/A Values

Audio2Emotion is computed within the A2F Animation Player node, hence no node evaluation time.

## Model Outputs

Audio2Face-3D models generate different types of data to drive facial animations. See the [Audio2Face-3D SDK](https://github.com/NVIDIA/Audio2Face-3D-SDK) documentation for technical details.

| Output Type | Description |
|-------------|-------------|
| **Blendshape Weights**<sup>1</sup> | Weight values (0.0-1.0) for pre-defined facial expressions |
| **Face Geometry** | Direct vertex positions for the face mesh |
| **Tongue Geometry** | Vertex positions for the tongue mesh (in zero jaw transform) |
| **Jaw Transform** | Transformation matrix for jaw or lower denture movement.<br>Pivot point of tongue geometry. |
| **Eye Rotation** | Eye movements including realistic micro-saccades |
| **Emotion** | Emotion detection |

### ¹ Blendshape Weights

Blendshape weights are the most versatile output format. The Audio2Face-3D SDK converts raw geometry into blendshape weights that work with standard character rigs. The current public Audio2Face-3D models include [ARKit-compatible shapes](https://developer.apple.com/documentation/arkit/arfaceanchor/blendshapelocation), while custom models may define their own blendshape targets.
