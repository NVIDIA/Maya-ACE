# Requirements

## System Requirements

- [Autodesk Maya](https://www.autodesk.com/products/maya/overview) 2024 (recommended), or 2025
- Microsoft Windows 10 64-bit or 11
- Computer HW that is [good enough to run Maya](https://www.autodesk.com/support/technical/article/caas/sfdcarticles/sfdcarticles/System-requirements-for-Autodesk-Maya.html)

## Workflow-Specific Requirements

Please check below sections for more details including recommended versions and compatibility information.

| Requirements                         | Local Inference     | Connecting to Service  | Developers |
|--------------------------------------|:-------------------:|:----------------------:|:----------:|
| NVIDIA GPU                           | ✅                  | ❌                     | ✅         |
| NVIDIA Driver working with CUDA      | ✅                  | ❌                     | ✅         |
| Internet Connection                  | ❌                  | ✅                     | ✅         |
| API key or private service address   | ❌                  | ✅                     | ❌         |
| CUDA Toolkit                         | ❌                  | ❌                     | ✅         |
| TensorRT                             | ❌                  | ❌                     | ✅         |
| Audio2Face-3D SDK                    | ❌                  | ❌                     | ✅         |
| Maya Devkit                          | ❌                  | ❌                     | ✅         |
| Visual Studio                        | ❌                  | ❌                     | ✅         |
| Python                               | ❌                  | ❌                     | ✅         |
| Git and Git LFS                      | ❌                  | ❌                     | ✅         |

### Audio2Face-3D Local Inference Workflow

This workflow uses Audio2Face-3D-SDK running on a local GPU. Maya-ACE uses `fp32` precision by default, but it follows precision of the models if defined.

- NVIDIA GPU with `8GB VRAM` or higher, compatible with CUDA 12 and TensorRT 10.13
    > [TensorRT Hardware support matrix](https://docs.nvidia.com/deeplearning/tensorrt/latest/getting-started/support-matrix.html)

- NVIDIA `Driver 576` or higher, lower than 580, compatible with CUDA 12 and your GPU (we recommend 576 with CUDA 12.9).
    > [CUDA Driver compatibility](https://docs.nvidia.com/cuda/cuda-toolkit-release-notes/index.html#cuda-driver)

### Audio2Face-3D NIM Workflow

This workflow uses Audio2Face-3D NIM running on a remote server or cloud service.

- Internet connection
- [Audio2Face-3D API key](https://build.nvidia.com/nvidia/audio2face-3d/api) to use NVIDIA ACE service, or a privately hosted [Audio2Face-3D service address](https://catalog.ngc.nvidia.com/orgs/nvidia/teams/ace/helm-charts/a2f-service)

### Build Environment for Developers

Though we provide pre-built binaries with the most stable versions, you can build the plugin from the source code with your choice of versions while they are compatible.

Follow the [Build and Test](buildandtest.md#setting-up-development-environment) guide to setup the build environment.
Libraries should follow [Audio2Face-3D SDK requirements](https://github.com/NVIDIA/Audio2Face-3D-SDK).

- Visual Studio >=2019 (`2022` is recommended)
- Python `3.10` or 3.11
- Audio2Face-3D SDK `1.0`
- CUDA Toolkit >=12.8, <13.0 (`12.9` is recommended)
- TensorRT >=`10.13`, <11.0
- Maya Devkit (`2024` or 2025)
- Git and Git LFS
