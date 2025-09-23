# Audio2Face/Audio2Emotion Models Directory

This directory is dedicated to locate the models for Audio2Face and Audio2Emotion.

The default location is `models` directory under where the mace plugin module is installed.
For Example, `{user home}\Documents\maya\modules\mace\models\`.

## Requirements to Download Models

The models can be downloaded using the Hugging Face CLI or Git and Git LFS.
You can find the detailed instructions in [Hugging Face web page](https://huggingface.co/docs/hub/en/models-downloading).

## Instructions

Find models from [the Audio2Face-3D collection](https://huggingface.co/collections/nvidia/audio2face-3d-6865d22d6daec4ac85887b17) on the Hugging Face website and download them to this directory under the following sub-directories:

* Audio2Face models: `./audio2face-models/`
* Audio2Emotion models: `./audio2emotion-models/`

### Examples

#### Audio2Face-3D-v3.0

* Model Page: [audio2face-3d-v3.0](https://huggingface.co/nvidia/Audio2Face-3D-v3.0)

* Download through the Hugging Face CLI:

```Powershell
huggingface-cli download nvidia/Audio2Face-3D-v3.0 --local-dir ./audio2face-models/audio2face-3d-v3.0
```

* Download through the Git and Git LFS:

```Powershell
git lfs install
git clone https://huggingface.co/nvidia/Audio2Face-3D-v3.0 ./audio2face-models/audio2face-3d-v3.0
```

#### Audio2Emotion-v2.2

* Model Page: [audio2emotion-v2.2](https://huggingface.co/nvidia/Audio2Emotion-v2.2)

* Download through the Hugging Face CLI:

```Powershell
huggingface-cli download nvidia/Audio2Emotion-v2.2 --local-dir ./audio2emotion-models/audio2emotion-v2.2
```

* Download through the Git and Git LFS:

```Powershell
git lfs install
git clone https://huggingface.co/nvidia/Audio2Emotion-v2.2 ./audio2emotion-models/audio2emotion-v2.2
```
