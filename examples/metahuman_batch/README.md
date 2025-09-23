# MetaHuman Facial Animation with Maya ACE and Batch Processing Example

This example demonstrates how to integrate Maya ACE with MetaHuman facial rigs to generate facial animation from audio. The included batch script processes multiple audio files and parameter configurations efficiently.

This example provides:

- Integration scripts for using Maya ACE with MetaHuman facial rigs
- Batch processing scripts for multiple audio files and parameter sets
- Customizable configuration options for different animation requirements
- Step-by-step video tutorials for setup and usage

## Contents

- [Video Tutorials](#video-tutorials)
- [Key Files](#key-files)
- [Notes and Requirements](#notes-and-requirements)

## Video Tutorials

The following tutorials guide you through setup and usage:

1. **[Introduction: What is Maya ACE MetaHuman Integration?](video_tutorials/01_Intro_What_is_This.mp4)**  
   Overview of the tools and capabilities in this example

2. **[MetaHuman Integration Demo](video_tutorials/02_Demo_MetaHuman.mp4)**  
   Maya ACE working with MetaHuman facial rigs

3. **[Batch Processing Demo](video_tutorials/03_Demo_Batch.mp4)**  
   Processing multiple audio files automatically

4. **[Setup Guide](video_tutorials/04_How_to_Setup.mp4)**  
   Installation and configuration walkthrough

5. **[Code Overview for Customization](video_tutorials/05_Code_Overview_For_Customization.mp4)**  
   How to customize and extend the scripts

![Tutorial Thumbnails](video_tutorials/01_Intro_What_is_This.jpg)

## Key Files

- [scripts](./scripts): batch example scripts for Metahuman FBX
  - [README.md](./scripts/README.md): instructions for using the example scripts
  - [Mace_Add-On](./scripts/Mace_Add-On): add-on scripts for Maya ACE
    - [README.md](./scripts/Mace_Add-On/README.md): instructions on how to setup MetaHuman Mapping scripts in Mace.
  - [Samples](./scripts/Samples): sample audio files and parameter files
- [proto](./proto): gRPC python module to connect to the A2F-3D microservice for batch
  - [README.md](./proto/README.md): instructions for generating the gRPC python module
  - [protobuf_files](./proto/protobuf_files): proto files for the Audio2Face-3D microservice
  - [sample_wheel](./proto/sample_wheel): python wheel for the nvidia_ace grpc python package

## Notes and Requirements

- Python dependencies include `grpcio`, `numpy`, `scipy`, `pandas`, and the `fbx` Python SDK module.
- The `fbx` module requires the Autodesk FBX SDK with Python bindings installed and available on your `PYTHONPATH` or run in Maya python environment.
- For details and environment setup, see the video tutorials (especially the Setup Guide).

## Acknowledgments

- This example is based on the [ACE sample scripts](https://github.com/NVIDIA/ACE/tree/main/microservices/audio_2_face_microservice/1.2) provided by NVIDIA.
- To know more about the MetaHuman, please refer to the [MetaHuman for Maya](https://dev.epicgames.com/documentation/en-us/metahuman/metahuman-for-maya) documentation.
