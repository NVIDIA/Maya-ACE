# Troubleshooting Tips

## Communication errors

These are common errors when connecting to ACE Audio2Face-3D through Maya-ACE.

| Error Messages | Possible Reasons (not all) |
|----------------|----------------------------|
| Unauthenticated | wrong api key, using http for an https endpoint |
| no authorization was passed | missing api key |
| SSL_ERROR_SSL | mismatched SSL version |
| Deadline Exceeded | bad network connection, slow machine or service, long audio |
| Cloud credits expired | out of credits |
| DNS resolution failed | network error, wrong address |
| Connection refused | service is unavailable |

## Cannot attach or connect AceAnimationPlayer

To attach AceAnimationPlayer, the selection should be a blendshape node or a mesh node with one blendshape connected.

To connect an existing AceAnimationPlayer to a new blendshape node, the selection should be one AceAnimationPlayer node and a blendshape node or a mesh with a blendshape node connected. There could be errors when,

- Selection is not in the correct order
- Selected multiple AceAnimationPlayer node or multiple blendshape node
- Selected a mesh connected with multiple blendshape nodes

Please make sure the selection is correct for each operation.
