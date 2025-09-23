# Maya ACE Sample Project

This directory contains a Maya project to try the Maya ACE plug-in in a portable way, including sample scenes, audio files, and directory structure to get you started.

Please "Set Project" to this directory in Maya to load sample scenes without manually updating file paths each time.

## Table of Contents

- [Models](#models)
- [Scenes](#scenes)
- [Sound](#sound)
- [Face Assets](#face-assets)
  - [Maya Geom](#maya-geom)
  - [Maya Blendshape v3](#maya-blendshape-v3)
  - [Maya Blendshape v2](#maya-blendshape-v2)
  - [Asset Compatibility Guide](#asset-compatibility-guide-matching-face-assets-with-audio2face-models)

## Models

This directory is dedicated to storing the models for the sample project. Please [download the models](models/README.md) by following the instructions.

Example file tree structure after downloading the models:

```text
models/
├── audio2emotion-models/
│   └── audio2emotion-v2.2/
└── audio2face-models/
    ├── audio2face-3d-v3.0/
    └── ...
```

## Scenes

These are pre-configured scenes to try the Maya ACE plug-in. Please refer to the [explanation of sample assets](#face-assets) for more information about the geometry and blendshape sets.

| Filename          | Geometry | A2F-3D Model | Description |
|-------------------|----------|-------|-------------|
| [ace_v2_arkit.ma](scenes/ace_v2_arkit.ma) | Claire v2 & James v3 & Mark v2 | v2.3 (on Service v1.3) | ARKit blendshapes setup with NIM service |
| [claire_v2_fullface.ma](scenes/claire_v2_fullface.ma) | Claire v2 | v2.3.1 Claire | A2F-3D v2.3 full-face setup |
| [claire_v3_fullface.ma](scenes/claire_v3_fullface.ma) | Claire v3 | v3.0 identity=Claire | A2F-3D v3.0 full-face setup |
| [james_v3_fullface.ma](scenes/james_v3_fullface.ma) | James v3 | v3.0 identity=James | A2F-3D v3.0 full-face setup |
| [mark_v2_fullface.ma](scenes/mark_v2_fullface.ma) | Mark v2 | v2.3 Mark | A2F-3D v2.3 full-face setup|
| [mark_v3_fullface.ma](scenes/mark_v3_fullface.ma) | Mark v3 | v3.0 identity=Mark | A2F-3D v3.0 full-face setup |

## Sound

| Filename          | Description |
|-------------------|-------------|
| [english_voice_male_p1_neutral.wav](sound/english_voice_male_p1_neutral.wav) | English male neutral audio |
| [chinese_voice_female_p01_neutral.wav](sound/chinese_voice_female_p01_neutral.wav) | Chinese female neutral audio |

## Face Assets

### Maya Geom

Maya scenes with geometry meshes of face, eyes, and denture.

| Filename | Topology | Compatible A2F-3D Model | Description |
|----------|------------------|-------------|-------------|
| [claire_geom_v2_topo1.ma](maya_geom/claire_geom_v2_topo1.ma) | topo1 | v2.3.x-Claire | Claire topo1 face with denture and eyes |
| [claire_geom_v3.ma](maya_geom/claire_geom_v3.ma) | topo2 | v3.0 (identity=Claire) | Claire topo2 face with denture and eyes |
| [james_geom_v3.ma](maya_geom/james_geom_v3.ma) | topo2 | v3.0 (identity=James), v2.3.x-James | James topo2 face with denture and eyes |
| [mark_geom_v2_topo1.ma](maya_geom/mark_geom_v2_topo1.ma) | topo1 | v2.3-Mark | Mark topo1 face with denture and eyes |
| [mark_geom_v3.ma](maya_geom/mark_geom_v3.ma) | topo2 | v3.0 (identity=Mark) | Mark topo2 face with denture and eyes |

### Maya Blendshape v3

Maya scenes with blendshape rigs of ARKit and tongue targets.

| Filename | Topology | Matching A2F-3D Model | Description |
|----------|------------------|-------------|-------------|
| [claire_arkit_v3.ma](maya_blendshape_v3/claire_arkit_v3.ma) | topo2 | v3.0 (identity=Claire) | Claire topo2 face, eyes, and denture with ARKit blendshape v3 |
| [james_arkit_v3.ma](maya_blendshape_v3/james_arkit_v3.ma) | topo2 | v3.0 (identity=James) | James topo2 face, eyes, and denture with ARKit blendshape v3 |
| [mark_arkit_v3.ma](maya_blendshape_v3/mark_arkit_v3.ma) | topo2 | v3.0 (identity=Mark) | Mark topo2 face, eyes, and denture with ARKit blendshape v3 |

### Maya Blendshape v2

Maya scenes with blendshape rigs of ARKit and tongue targets.

| Filename | Topology | Matching A2F-3D Model | Description |
|----------|------------------|-------------|-------------|
| [claire_arkit_v2_topo1.ma](maya_blendshape_v2/claire_arkit_v2_topo1.ma) | topo1 | v2.3.x-Claire | Claire topo1 face and tongue with ARKit blendshape v2 |
| [james_arkit_v2.ma](maya_blendshape_v2/james_arkit_v2.ma) | topo2 | v2.3.x-James | James topo2 face and tongue with ARKit blendshape v2 |
| [mark_arkit_v2_topo1.ma](maya_blendshape_v2/mark_arkit_v2_topo1.ma) | topo1 | v2.3-Mark | Mark topo1 face and tongue with ARKit blendshape v2 |

### Asset Compatibility Guide: Matching Face Assets with Audio2Face Models

While you're encouraged to create your own blendshape rigs for custom characters, this guide helps you use the provided assets correctly.

To achieve optimal results with Audio2Face-3D models, you should match three critical properties between your face asset and the A2F-3D model:

1. [**Blendshape Version**](#blendshape-versions) - Affects animation quality
2. [**Geometry Topology**](#geometry-topology) - Must match exactly for geometry output
3. [**Model Identity**](#model-identity) - Affects eye and tongue positioning

#### Quick Reference: Choosing the Right Assets

| A2F-3D Model | Output Mode | Recommended Asset | Alternative Options |
|--------------|-------------|-------------------|---------------------|
| **v3.0 (James)** | Geometry | `james_geom_v3.ma` | Must match exactly |
| | Blendshape | `james_arkit_v3.ma` | Any ARKit asset (v2/v3)* |
| **v3.0 (Claire)** | Geometry | `claire_geom_v3.ma` | Must match exactly |
| | Blendshape | `claire_arkit_v3.ma` | Any ARKit asset (v2/v3)* |
| **v3.0 (Mark)** | Geometry | `mark_geom_v3.ma` | Must match exactly |
| | Blendshape | `mark_arkit_v3.ma` | Any ARKit asset (v2/v3)* |
| **v2.3.x-James** | Geometry | `james_geom_v3.ma` | Must match exactly |
| | Blendshape | `james_arkit_v2.ma` | Any ARKit asset (v2/v3)* |
| **v2.3.x-Claire** | Geometry | `claire_geom_v2_topo1.ma` | Must match exactly |
| | Blendshape | `claire_arkit_v2_topo1.ma` | Any ARKit asset (v2/v3)* |
| **v2.3-Mark** | Geometry | `mark_geom_v2_topo1.ma` | Must match exactly |
| | Blendshape | `mark_arkit_v2_topo1.ma` | Any ARKit asset (v2/v3)* |

#### Blendshape Versions

Blendshape versions correspond to the target poses and solve-data within Audio2Face models:

- **Version matching ensures optimal animation quality**
  - v3 blendshapes → A2F-3D v3.0 models
  - v2 blendshapes → A2F-3D v2.3.x models

- **Cross-version compatibility is possible** but may reduce quality
  - ARKit blendshape names are consistent across versions
  - Both v2.3 and v3.0 models output standard ARKit pose weights

#### Geometry Topology

Topology defines the vertex count and mesh structure:

- **For geometry output: Topology MUST match exactly**
  - A2F-3D v3.0 → Use topo2 assets only
  - A2F-3D v2.3.x → Check model docs (varies by model)
  
- **For blendshape output: Topology is flexible**
  - Focus on matching blendshape names instead

#### Model Identity

Each A2F model is trained with a specific character identity that affects:

- **Eye position and orientation**
- **Tongue placement and movement**
- **Jaw pivot and rotation**

While face meshes can be shared across identities within the same topology, auxiliary features require identity matching.

**Example scenarios:**

| Setup | Face Animation | Eyes/Tongue | Recommendation |
|-------|----------------|-------------|----------------|
| `claire_geom_v3.ma` + A2F-3D v3.0 (James) | ✓ Works | ✗ Wrong position | Use `james_geom_v3.ma` |
| `james_arkit_v3.ma` + A2F-3D v3.0 (Claire) | ✓ Works | ✓ Works | OK to use |
| `mark_geom_v2_topo1.ma` + A2F-3D v3.0 (Mark) | ✗ Fails | ✗ Wrong position | Use `mark_geom_v3.ma` |

#### Best Practices

1. **Start with matching sets**: Use assets designed for your specific A2F model
2. **Test before production**: Verify all facial features animate correctly
3. **Document your pipeline**: Keep track of which assets work with which models
4. **When in doubt**: Refer to the Quick Reference table above
5. **For geometry output**: Always match topology exactly - there's no flexibility here
6. **For blendshape output**: Prioritize matching versions for best quality, but cross-version use is acceptable
