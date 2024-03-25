# BLE_Audio_Stream_NRF52840

Audio streaming library for bluetooth low energy devices. 

Specifically, this has ONLY been tested on the 

- Seeed Studio NRF52840 BLE Sense module

Contributors: Adam Patni and Axel Peytavin

**IMPORTANT NOTE:** This repository is currently in active development, and the code has only been tested locally on a subset of functionalities. It has not undergone extensive testing, especially in diverse environments or on different hardware configurations. Users should proceed with caution and are encouraged to report any issues or contribute to the development.


## Usage

To use the BLE_Audio_Stream_NRF52840 library in your project, follow these steps:

1. **Include the Library**: Ensure you have included the `BLEAudioStreamingNRF52840.h` header file in your project by downloading the github repo as a zip and adding it to your libraries in Arduino IDE. Run the functions in `BLEAudioStreamingNRF52840.c` to start up the BLE server for audio. 

2. **Start the Client**: Next run the `client.py` script to test the audio streaming functionality and transcription services. Ensure your BLE device is powered on and in range. Modify the `DEVICE_ID` in `client.py` to match your BLE device's address. This will enable you to read packets from your device on another device. This client can be replicated in a language or on the device of your choice.
