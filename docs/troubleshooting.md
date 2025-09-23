# Troubleshooting Guides

## HTTP, HTTPS, and Port

When connecting to Audio2Face-3D NIM through Maya-ACE, please enter the correct address in the `Service Address` field including protocol (`http://` or `https://`) and port.

## Communication Errors

These are common errors when connecting to ACE Audio2Face-3D through Maya-ACE.

| Error Messages | Possible Reasons |
|----------------|----------------------------|
| Unauthenticated | wrong API key, using http for an https endpoint |
| no authorization was passed | missing API key |
| SSL_ERROR_SSL | mismatched SSL version |
| Deadline Exceeded | bad network connection, slow machine or service, long audio |
| Cloud credits expired | out of credits; please check [the documentation](https://build.nvidia.com/nvidia/audio2face-3d/api) |
| DNS resolution failed | network error, wrong address |
| Connection refused | service is unavailable |

## Cannot Attach or Connect Ace/A2FAnimationPlayer to Blendshape Nodes

To attach Ace/A2FAnimationPlayer, the selection should be blendshape nodes or mesh nodes that have blendshapes connected.

To connect an existing Ace/A2FAnimationPlayer to new blendshape nodes, the selection should be one Ace/A2FAnimationPlayer node and blendshape nodes or mesh nodes with blendshape nodes connected. There could be errors when,

- Selection is not in the correct order
- Selected multiple Ace/A2FAnimationPlayer nodes or no blendshape nodes

Please make sure the selection is correct for each operation.
