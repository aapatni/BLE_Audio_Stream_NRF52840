#include "BLEAudioStreamingNRF52840.h"

void BLEAudioStreamingNRF52840::begin()
{
    Serial.begin(115200);
    while (!Serial)
        delay(10);

    // BLE setup
    setupBLE();

    // Microphone setup
    Mic.set_callback(audio_rec_callback);
    if (!Mic.begin())
    {
        Serial.println("Mic initialization failed");
        while (1)
            ;
    }
    Serial.println("Mic initialization done.");
}

void BLEAudioStreamingNRF52840::setupBLE()
{
    if (!BLE.begin())
    {
        Serial.println("Starting Bluetooth® Low Energy module failed!");
        while (1)
            ;
    }
    BLE.setLocalName(name);
    BLE.setAdvertisedService(audioService);
    audioService.addCharacteristic(audioCharacteristic);
    BLE.addService(audioService);
    BLE.advertise();
    Serial.println("Bluetooth® Low Energy Setup Complete!");
}

void BLEAudioStreamingNRF52840::loop()
{
    // BLE connection handling
    BLEDevice central = BLE.central();
    if (central)
    {
        if (!isConnected)
        {
            Serial.print("Connected to central: ");
            Serial.println(central.address());
            isConnected = true;
        }
    }
    else
    {
        isConnected = false;
    }

    // Data transmission when recording
    handleTransmission();
}

void BLEAudioStreamingNRF52840::handleTransmission()
{
    if (recording)
    {
        uint32_t available_samples = (recording_idx + micBufferSize - read_idx) % micBufferSize;
        if (available_samples >= txnChunkSize)
        {
            uint16_t chunk[txnChunkSize];
            for (int i = 0; i < txnChunkSize; i++)
            {
                chunk[i] = recording_buf[(read_idx + i) % micBufferSize];
            }
            audioCharacteristic.writeValue(chunk, sizeof(chunk));
            read_idx = (read_idx + txnChunkSize) % micBufferSize;
            delay(delay_ms);
        }
    }
}

static void BLEAudioStreamingNRF52840::audio_rec_callback(uint16_t *buf, uint32_t buf_len)
{
    if (!recording)
        return; // Early exit if not recording
    uint32_t num_elements = buf_len / SUBSAMPLE_RATE;
    bool data_overflow = false;
    // NO WRAPAROUND CASE
    if (recording_idx + num_elements < txnBufferSize)
    {
        if (read_idx > recording_idx && recording_idx + num_elements >= read_idx)
        {
            data_overflow = true;
        }
        else
        {
            data_overflow = false;
        }
    }
    // WRAPAROUND CASE
    else
    {
        if (recording_idx < read_idx)
        {
            data_overflow = true;
        }
        else
        {
            if ((recording_idx + num_elements) % txnBufferSize <= read_idx)
            {
                data_overflow = false;
            }
            else
            {
                data_overflow = true;
            }
        }
    }

    if (data_overflow)
    {
        read_idx = (recording_idx + num_elements) % txnBufferSize;
    }
    for (uint32_t i = 0; i < buf_len; i += downsampleFactor)
    {
        recording_buf[write_idx] = buf[i];
        write_idx = (write_idx + 1) % txnBufferSize;
    }
}


