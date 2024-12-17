# User Interface

## Attribute Editor - AceAnimationPlayer

### Network and Audio

Information to connect ACE Audio2Face-3D service, and audio file information to request animation.

<img src="resource/ui_network_and_audio.png" width="480" />

- NVCF Api Key: A valid api key acquired from the [web page](https://build.nvidia.com/nvidia/audio2face-3d/api). An environment variable can be used with `$` prefix.
- Network Preset: An option menu to use predefined network settings.
- Connect and Send Audio: A button to send audio and receives animation(Streaming) or start a live session(Authoring)
- Status Indicator: A color circle that visualizes communication status;
![green](/source/maya_aceclient/scripts/icon/green.svg)(success),
![red](/source/maya_aceclient/scripts/icon/red.svg)(error),
![yellow](/source/maya_aceclient/scripts/icon/yellow.svg)(out-dated)
- Network Address: A full url with protocol and port number. example: https://grpc.nvcf.nvidia.com:443
- NVCF Function Id: A valid function id that is specific for the service and an AI model.
Find a proper Function ID from the [web page](https://build.nvidia.com/nvidia/audio2face-3d/api)
  - Examples (as of November 10, 2024)
    - Streaming service - Mark model: 945ed566-a023-4677-9a49-61ede107fd5a
    - Streaming service - Claire model: 462f7853-60e8-474a-9728-7b598e58472c
- Audiofile: A path to an audio file to request animation from the service. It can be also selected from imported audios through the drop down option.

### Emotion Parameters

Parameters to control generated emotion and preferred(manual) emotion.
Audio2Face-3D generate animation with the emotion input which includes generated emotion and preferred emotion.
[Please watch this video to understand how it works.](https://www.nvidia.com/en-us/on-demand/session/omniverse2020-om1537/)

<img src="resource/ui_emotion_params.png" width="480" />

- Emotion Strength: the strength of the overall emotion; the total of auto emotion and preferred(manual) emotion.
  - emotion = emotion strength * (preferred weight * preferred emotion + (1.0 - preferred weight) * generated emotion)
- Preferred Emotion: Enable/disable and the ratio of the user driven emotion in the overall emotion (1.0 = 100% preferred emotion, 0.0 = 100% generated emotion).
- Auto Emotion: Parameters to control generated emotion.

### Face Parameters

Parameters to control overal face animation.
Please check [Audio2Face-3D Microservice documents](https://docs.nvidia.com/ace/latest/modules/a2f-docs/text/architecture/audio2face_ms.html)
for the updated information.

<img src="resource/ui_face_params.png" width="480" />

### Blendshape Multipliers

Override specific expressions by multiplying the Audio2Face-3D result.

<img src="resource/ui_bs_multipliers.png" width="480" />

### Blendshape Offsets

Override specific expressions by adding constant values to the Audio2Face-3D result;

- each output = (raw result * multipler) + offset

<img src="resource/ui_bs_offsets.png" width="480" />

## Main Menu

<img src="resource/ui_menu.png"/>

## Attach a new Animation Player

Create a new AceAnimationPlayer and connect to the selected blendshape node

## Connect an existing Animation Player

Connect the select AceAnimationPlayer to the secondly selected blendshape node

## Import A2F Parameters

Import a json file with parameters to an AceAnimationPlayer.

## Export A2F Parameters

Export a json file with parameters from an AceAnimationPlayer.

> **DISCLAIMER**: This export may not fully reflect recent updates from the server or ACE services.
The following limitations may result in potential discrepancies:
>
> - Parameters not set and controlled by the Maya plugin will be exported with default values.
> - Default values in the exported configuration files may differ if the A2F server is deployed with custom settings.
> - To ensure consistency between the client and server, carefully compare the server's configuration with the exported parameters.
> - For the most accurate and up-to-date information, please refer to the official website or the server configuration.

## Export A2F Service Configs

Export config files from an ACE A2F Service.

> **DISCLAIMER**: This export may not fully reflect recent updates from the server or ACE services.
The following limitations may result in potential discrepancies:
>
> - Exported files are provided by the A2F service, suitable for service deployment.
> - The format and content vary depending on the service and the service version.
> - Note that some services may not support this export.
