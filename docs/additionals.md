# Additional Knowledge

## Audio File Requirements

Maya-ACE leverages the [AudioFile](https://github.com/adamstark/AudioFile) library to read audio data, providing support for various WAV and AIFF files. However, when importing audio into the Maya timeline, users must adhere to the file format constraints set by Maya. Refer to [Maya's supported audio file formats'](https://help.autodesk.com/view/MAYAUL/2024/ENU/?guid=GUID-CF2B0358-6946-4C9D-9F8C-A783921CAECC) for more details on compatibility.

### Important Note

As of Maya 2025, only PCM audio formats are supported. Please ensure your audio files are in the correct format for successful import.

### Recommended Audio Specifications

For optimal compatibility and performance, it is recommended to use the following audio file specifications with Maya-ACE:

- File Format: WAV
- Data Format: 16-bit PCM
- Sample Rate: 16 kHz, 32 kHz, or 48 kHz
- Channels: Mono (1 channel)

By following these specifications, you can ensure the smooth integration of audio files within Maya and avoid common import issues.
