# Getting Started

## Quickstart

### Set Up an API Key

1. [Set an environment variable](https://learn.microsoft.com/en-us/previous-versions/office/developer/sharepoint-2010/ee537574(v=office.14))
, `NVCF_API_KEY`,
with a valid [API key](https://build.nvidia.com/nvidia/audio2face-3d/api)
    - To get a key, click the green `Get API Key` text on the right of the page and proceed with instructions.
<br /><img src="resource/get_api_key.png" width="400" />

### Install the Plugin Package

1. Download the `mace` package, unzip and
[copy contents to a Maya module path](https://help.autodesk.com/view/MAYAUL/2024/ENU/?guid=Maya_SDK_Distributing_Maya_Plug_ins_DistributingUsingModules_InstallingModules_html)
<br /><img src="resource/copy_mace_module.png" width="440" />

### Download the Sample Scene and Audios

1. Download the
[sample maya scene](sample_data/maya_project/scenes/sample_ace_animation_player.mb)
and sample audio files:
[English(male)](sample_data/maya_project/sound/english_voice_male_p1_neutral.wav) or
[Chinese(female)](sample_data/maya_project/sound/chinese_voice_female_p01_neutral.wav)

### Configure Maya and Import an Audio

1. Launch Maya, load `maya_aceclient` plugin, and open the sample scene.
<br /><img src="resource/samplescene_static.png" width="440" />
<a name="maya-settings"></a>

1. Adjust [**Time Slider preferences**](https://help.autodesk.com/view/MAYAUL/2024/ENU/?guid=GUID-51A80586-9EEA-43A4-AA1F-BF1370C24A64)
: framerate=30, playback speed=30fps, looping=once
<br /><img src="resource/timeline_preferences.png" width="440" />

1. Disable `Cached Playback` on the Time Slider
<br /><img src="resource/disable_cached_playback.png" width="440" />
    - Maya-ACE v2.0+ is compatible with Cached Playback; however, it is recommended to disable this feature during initial testing.

1. [Import an audio into the Maya scene](https://help.autodesk.com/view/MAYAUL/2024/ENU/?guid=GUID-CF2B0358-6946-4C9D-9F8C-A783921CAECC) and set it to the Time Slider sound
<br /><img src="resource/import_audio_menu.png" width="440" />

1. Adjust **Time Slider range** to fit to the audio length

### Configure the ACE Animation Player and Get Animation

1. Select `AceAnimationPlayer1` node and open Attribute Editor
    - You may turn off **DAG Objects only** from the Display menu to see AceAnimationPlayer1 on the outliner window.
    <br /><img src="resource/disable_dag_only.png" width="400" />
    <br /><img src="resource/select_aceanimationplayer.png" width="400" />
    - To open attribute editor, click menu -> Windows -> General Editors -> Attribute Editor
    <br /><img src="resource/open_attribute_editor.png" width="400" />

1. Click the option menu next to Audiofile attribute, and select the audio imported before.
<br /><img src="resource/audiofile_menu.png" width="440" />

1. Click `Connect and Send Audio` button, and wait for the **Received ### frames** message.

1. The circle on the right of the button will change color to green if the communication was successful.
<br /><img src="resource/button_and_status_indicator.png" width="440" />

1. Click Maya's play button. Watch how face is moving

## Setting Up with Sample Assets

> ### Managing Blendshape Names and Order
>
> Proper management of blendshape names and their order is essential
> for connecting the AceAnimationPlayer node in Maya-ACE to blendshape nodes effectively.
> For custom asset connections, refer to the
> [Connecting Custom Blendshapes](/docs/bestpractices.md#connecting-custom-blendshapes) guide.
> To achieve optimal facial performance, it’s recommended to adhere to ARKit specifications
> for naming conventions and structure.

### Preparation

1. Follow steps to [get an API key and set an environment variable](#set-up-an-api-key).

1. Follow instructions to [install the plugin package](#install-the-plugin-package).

1. Download sample fbx files:
[Mark](sample_data/mark_bs_arkit_v2.fbx),
[Claire](sample_data/claire_bs_arkit_v2.fbx)

1. Download a sample audio file:
[English(male)](sample_data/maya_project/sound/english_voice_male_p1_neutral.wav) or
[Chinese(female)](sample_data/maya_project/sound/chinese_voice_female_p01_neutral.wav)

### Initial Maya Setup

1. Launch maya with a new scene. Load maya plugins; `maya_aceclient` and `fbxmaya`
<br /><img src="resource/plugin_manager.png" width="440" />

1. Create references of [sample fbx files](#download-sample-data)
<br /><img src="resource/reference_editor.png" width="440" />

1. Follow instructions
[to set up Maya settings and Time Slider with an audio](#configure-maya-and-import-an-audio).

### Create and Connect an AceAnimationPlayer to a Face

1. Select `c_headWatertight_mid` of mark_bs_arkit, and click **menu->ACE->Attach a new Animation Player**
<br /><img src="resource/menu_attach_player.png" width="440" />
    - (Optional) Tips to optimize the viewport for faces
      - Hide unused groups; such as **c_mouth_grp_mid**, **r_eye_grp_mid**, **l_eye_grp_mid**, **root**
      - Setup a viewport camera with focal length between **75mm** and **150mm** looking at the face(s)
      - Change viewport camera's **Near Clip Plane** to 10
      - Assign **standardSurface1** material to face(s)

### Configure the AceAnimationPlayer and Get Animation

1. Open Attribute Editor, Select `AceAnimationPlayer1` node
<br /><img src="resource/open_attribute_editor.png" width="440" />

1. Select one on the Network Preset or
[Set a valid Network Address, an Api Key, and a Function ID](https://build.nvidia.com/nvidia/audio2face-3d/api).
<br /><img src="resource/attribute_network_info.png" width="440" />

1. To change audio, Select an option from the AudioFile, and also update Time Slider to use the same audio.
<br /><img src="resource/select_audio.png" width="440" />
<br /><img src="resource/change_timeslider_audio.png" width="440" />

1. Click `Connect and Send Audio` button, and wait for the **Received ### frames** message.

1. The circle on the right of the button will change color to green if the communication was successful.
<br /><img src="resource/button_and_status_indicator.png" width="440" />

1. Click Maya's play button. Check animation on Mark's face.
<br /><img src="resource/mace_play_mark.gif" width="440" />

### Connect the AceAnimationPlayer to Another Face

1. Select both `AceAnimationPlayer1` and `c_headWatertight_mid` of Claire in order,
and click **menu->ACE->Connect an existing Animation Player**
<br /><img src="resource/select_to_connect.png" width="440" />
    - You may turn off **DAG Objects only** from the Display menu to see AceAnimationPlayer1 on the outliner window.
    <br /><img src="resource/disable_dag_only.png" width="400" />

1. Click Maya's play button again. Check animation on both faces.
<br /><img src="resource/mace_play_both.gif" width="440" />

### Connect Lower Teeth and Tongue

This process guides to setup the lower teeth and the tongue to move with mouth on the sample assets.

1. Unhide **c_mouth_grp_mid** group if it is hidden.

1. To connect lower teeth, select `AceAnimationPlayer1` and lower teeth in order, then click `ACE -> Connect an existing Animation Player`
    <br /><img src="resource/setup_connect_bottom_denture.png" width=440 />

1. Repeat the process for the tongue mesh. Check the result.
    <br /><img src="resource/setup_connect_tongue.png" width=440 />
    <br /><img src="resource/setup_connect_lower_face_done.png" width=440 />

## Interactive Editing with the Audio2Face-3D Authoring Service

> Audio2Face-3D Authoring Service is available for ACE Early Access Users.

Audio2Face-3D Authoring service provides an interactive editing of parameters. It provides immediate result of parameter changes and allows quick approach to the optimal parameter settings.

Using A2F authoring service requires a couple extra steps and has some limitations described below.

1. Chose one of the Authoring `Network Preset`s, or set `Client Type` to **Authoring** with a proper **Function ID**.
<br /><img src="resource/client_type_authoring.png" width="440" />
    1. Mark Model Authoring Function Id `be24fd18-4c26-4a38-84ad-c7f88da10835`
    1. Claire Model Authoring Function Id `f33c62b0-96d2-434a-9a4c-e89b7c064be5`

1. Click **Connect and Send Audio** button to start interactive session. Check status turns to green.

1. Change parameters and time. Face should be updated interactively.
<br /><img src="resource/authoring_param_change.gif" width="440" />
