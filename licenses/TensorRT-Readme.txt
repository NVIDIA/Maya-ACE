=== NVIDIA TensorRT ===

NVIDIA® TensorRT™ is a C++ library that facilitates high-performance inference
on NVIDIA GPUs. TensorRT takes a trained network, which consists of a network
definition and a set of trained parameters, and produces a highly optimized
runtime engine that performs inference for that network. TensorRT provides APIs
using C++ and Python that help to express deep learning models using the Network
Definition API or load a pre-defined model using the parsers that allow TensorRT
to optimize and run them on an NVIDIA GPU. TensorRT applies graph optimizations,
layer fusion, among other optimizations, while also finding the fastest
implementation of that model leveraging a diverse collection of highly optimized
kernels. TensorRT also supplies a runtime that you can use to execute this
network on NVIDIA’s GPUs.

For more information about TensorRT, visit https://developer.nvidia.com/tensorrt.

In previous TensorRT releases, PDF documentation was included inside the TensorRT
package. The PDF documentation has been removed from the package in favor of
online documentation, which is updated regularly. Online documentation can be
found at https://docs.nvidia.com/deeplearning/tensorrt.

For details on TensorRT's license agreement, visit https://docs.nvidia.com/deeplearning/tensorrt/sla/.

=== References ===

Quick Start Guide: https://docs.nvidia.com/deeplearning/tensorrt/quick-start-guide
Release Notes: https://docs.nvidia.com/deeplearning/tensorrt/release-notes
Support Matrix: https://docs.nvidia.com/deeplearning/tensorrt/support-matrix
Installation Guide: https://docs.nvidia.com/deeplearning/tensorrt/install-guide

API Reference: https://docs.nvidia.com/deeplearning/tensorrt/api
Developer Guide: https://docs.nvidia.com/deeplearning/tensorrt/developer-guide
Sample Support Guide: https://docs.nvidia.com/deeplearning/tensorrt/sample-support-guide
