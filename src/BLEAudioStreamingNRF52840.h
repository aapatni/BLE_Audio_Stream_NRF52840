// Copyright 2008-2021 Seeed Studio(STU)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Created by Adam Patni 3/24/2024
// Adapted from Arduino BLE Library

#ifndef BLE_AUDIO_STREAMING_NRF52840_H
#define BLE_AUDIO_STREAMING_NRF52840_H

#include <ArduinoBLE.h>
#include <mic.h>

class BLEAudioStreamingNRF52840
{
public:
    BLEAudioStreamingNRF52840::BLEAudioStreamingNRF52840(
        const char* name = "BLEAudioRecorder",
        uint32_t micSampleRate = 16000,
        uint32_t micBufferSize = 16000,
        uint32_t txnBufferSize = 32000,
        uint32_t txnChunkSize = 200,
        uint32_t downsampleFactor = 4,
        uint32_t delay_ms = 75,
        bool debug = false)
        : name(name),
          debug(debug),
          downsampleFactor(downsampleFactor),
          txnChunkSize(txnChunkSize),
          txnBufferSize(txnBufferSize),
          micBufferSize(micBufferSize),
          micSampleRate(micSampleRate),
          delay_ms(delay_ms),
    {
    }
    void begin();
    void loop();


private:
    const char* name;
    uint32_t downsampleFactor;
    uint32_t txnChunkSize;
    uint32_t txnBufferSize;
    uint32_t micBufferSize;
    uint32_t micSampleRate;

    bool debug;

    uint16_t *recording_buf;
    volatile uint32_t recording_idx = 0;
    volatile uint32_t read_idx = 0;
    volatile bool recording = false;
    bool isConnected = false;

    mic_config_t mic_config;
    NRF52840_ADC_Class Mic;
    BLEService audioService;
    BLECharacteristic audioCharacteristic;

    static void audio_rec_callback(uint16_t *buf, uint32_t buf_len);

    void setupBLE();
    void handleTransmission();

    void start_recording()
    {
        recording = true;
        recording_idx = 0;
        read_idx = 0;
    }
};

#endif // BLE_AUDIO_STREAMING_NRF52840_H