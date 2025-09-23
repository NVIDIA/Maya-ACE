# Maya ACE Add-On for MetaHuman Integration

This add-on extends the Maya ACE plugin to provide MetaHuman integration capabilities, allowing you to connect A2F animation players to MetaHuman facial rigs and build batch processing lists for audio-to-animation workflows.

## Prerequisites

- **Maya ACE Plugin Version 2.0** must be installed in Maya
- Maya (compatible with ACE plugin v2.0)

## Installation Instructions

### Step 1: Add Setup Content
Add the content from `add-to-setup.py` to the Maya ACE script named `setup.py` located in:
- Maya modules path
- `mace\scripts\ace_tools` subfolder

### Step 2: Copy Required Files
Copy the following files to the same subfolder (`mace\scripts\ace_tools`):
- `Batch_List_MayaDialog.py`
- `ACE2Rig_Functions.py`
- `MH_Mapping.json`

### Step 3: Update userSetup.mel
Overwrite the `userSetup.mel` file located in the `mace\scripts` subfolder with the one provided in this add-on package.

## Available Tools

Once installation is complete, two new options will appear in the Maya menus under the **ACE** menu:

1. **Connect a new Animation Player to MetaHuman**
2. **Build a Batch List**

## Usage Instructions

### Connect a new Animation Player to MetaHuman

This tool allows you to attach an A2F player to MetaHuman facial rig controls:

1. Select the MetaHuman facial rig control UI in Maya
2. Use the **Connect a new Animation Player to MetaHuman** tool from the ACE menu
3. This will attach an A2F player to the MetaHuman controls for animation playback

### Build a Batch List

This tool creates batch processing lists for audio-to-animation workflows:

1. Click on the **Build a Batch List** menu option
2. The batch list dialog will open
3. Configure the following settings:
   - **Audio Files Folder**: Choose the folder containing all audio files
   - **A2F Parameters Folder**: Choose the folder containing all A2F parameter files
   - **Anims Output Path**: Define the destination folder for generated animations
   - **FBX Format**: Choose between:
     - **FBX for Maya Yup** : for Metahuman imported in Maya using Yup (MH version 2.0.0, UE 5.3-5.6)
     - **FBX for Maya Zup** : for Metahuman imported in Maya using Zup (MH version 2.0.0, UE 5.3-5.6)
     - **FBX for Unreal**: for Unreal Engine's axis system
     - *Note: Both FBX formats are generated for the MetaHuman control rig, but with different axis systems*

4. **Audio List**: Create a list of audio files to process, each with an associated parameter file
5. **Create Batch List**: Create the batch list file. For convenience, the batch list path is automatically copied to the clipboard when generated so that it can be used in the command line batch script.

## Integration with Python Batch Script

The generated batch list can be used with the Python batch script (`batch_a2f.py`) to process audio files and generate animations:

1. Use the batch list path (copied to clipboard) with the Python script
2. The script will process all audio files according to the batch list configuration
3. Generated animations will be saved in the specified output folder

## File Structure

```
Mace_Add-On/
├── add-to-setup.py          # content to add to setup.py script for Maya ACE
├── Batch_List_MayaDialog.py # Batch list creation dialog
├── ACE2MH_Functions.py      # MetaHuman integration functions
├── userSetup.mel           # Maya startup script
├── MH_Mapping.jason        # default mapping of ARKit blend shapes to Metahuman Facial Controls (MH version 2.0.0, UE 5.3-5.6)
└── README.md               # This documentation
```

## Importing FBX on Metahuman controls in Maya
- If you import FBX files from batch into Maya on the facial controls, you need to import in a scene **without** the ACE/A2F player connected to the controls. When Metahuman controls inputs are connected to ACE/A2F player's outputs, the animation won't be loaded on the controls.

## Troubleshooting

- Ensure Maya ACE Plugin v1.2 is properly installed before adding this functionality
- Verify all files are copied to the correct Maya script directories
- Check that the `userSetup.mel` file has been properly overwritten
- Restart Maya after installation to ensure all changes take effect
- MetaHuman has changed axis orientation in recent versions. Check with Maya Yup and Maya Zup axis system to see what works on a given MetaHuman
- Metahuman also changed facial controls in recent versions. The mapping provided has been tested in UE 5.3-5.6 versions. Check the mapping and controls for other versions or customized Metahumans. 
- if a Python module is not loaded in Maya and cause issues with the script, open a Powershell and type the following:
	
```
<Maya install Drive>:\Program Files\Autodesk\<MayaV ersion>\bin\mayapy.exe -m pip install <Python module> --target "C:/Users/<user name>/Documents/maya/<Maya Version>/scripts"
```

You can also check for valid install path by running the following script in the Python script editor and check the paths listed. 

```
import sys
for path in sys.path:
  print(path)
```

## Support

For issues or questions regarding this add-on, please refer to the main project documentation or contact the development team.
