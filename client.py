# Audio transcription test client configuration:
# - Update Device ID as required.
# - Set Sample rate based on server's micSampleRate/downsampleFactor.
# - The recording is stored in recordings/timestamp.wav
# - Audio transcription via OpenAI Whisper is stored in recordings/timestamp.txt

import asyncio
import bleak
from bleak import BleakClient
import wave
from datetime import datetime
import numpy as np
import time
import os
from scipy.signal import stft, istft

TRANSCRIPTION = True
if TRANSCRIPTION:
    import whisper

    whisper_model = whisper.load_model("medium")

DEVICE_ID = "D50E2FE2-CDC7-DA96-504B-81F38465521F"
SERVICE_UUID = "19B10000-E8F2-537E-4F6C-D104768A1214"
CHARACTERISTIC_UUID = "19B10001-E8F2-537E-4F6C-D104768A1214"

SAMPLE_RATE = 4000  # Sample rate for the audio
SAMPLE_WIDTH = 2  # 16-bit audio
CHANNELS = 1  # Mono audio
CAPTURE_TIME = 10  # Time to capture audio in seconds


async def main():
    print("Discovering AudioRecorder...")
    devices = await bleak.discover(timeout=2.0)
    audio_recorder = None
    for device in devices:
        if device.name:
            print(device.name, device.address)
        if device.address == DEVICE_ID:
            audio_recorder = device
            break

    if not audio_recorder:
        print("AudioRecorder not found")
        return

    def handle_ble_disconnect(client):
        print("Disconnected from AudioRecorder")

    def filter_audio_data(audio_data):
        audio_data = np.frombuffer(audio_data, dtype=np.uint16)
        audio_data -= 32768
        scaling_factor = (
            2 * 32768 / (max(0, np.max(audio_data)) - min(0, np.min(audio_data)))
        )
        return (audio_data * scaling_factor).astype(np.int16)

    def export_audio_data(filtered_audio_data):
        recordings_dir = "recordings"
        if not os.path.exists(recordings_dir):
            os.makedirs(recordings_dir)
        filename = os.path.join(recordings_dir, datetime.now().strftime("%H-%M-%S-%f"))
        print(filename)
        with wave.open(filename + ".wav", "wb") as wav_file:
            wav_file.setnchannels(CHANNELS)
            wav_file.setsampwidth(SAMPLE_WIDTH)
            wav_file.setframerate(SAMPLE_RATE)
            wav_file.writeframes(
                filtered_audio_data.tobytes()
            )  # Ensure data is in bytes format

        if TRANSCRIPTION:
            try:
                result = whisper_model.transcribe(filename + ".wav")
                print(result)
                with open(filename + ".txt", "w") as txt_file:
                    txt_file.write(result["text"])
            except Exception as e:
                print(f"An error occurred: {str(e)}")

    async def process_audio(audio_data):
        if len(audio_data) == 0:
            print("Warning: Received empty audio data array.")
            return

        filtered_audio_data = filter_audio_data(audio_data)
        export_audio_data(filtered_audio_data)
        pass

    async with BleakClient(
        audio_recorder.address,
        services=[SERVICE_UUID],
        disconnect_callback=handle_ble_disconnect,
    ) as client:
        print("Connected to AudioRecorder")
        services = await client.get_services()
        audio_service = services.get_service(SERVICE_UUID)
        audio_characteristic = audio_service.get_characteristic(CHARACTERISTIC_UUID)

        audio_data = bytearray()

        def handle_audio_data(sender, data):
            print(f"Received {len(data)} bytes at {time.time()}")
            audio_data.extend(data)

        async def record_audio():
            await client.start_notify(audio_characteristic.uuid, handle_audio_data)
            print("Recording audio...")
            await asyncio.sleep(CAPTURE_TIME)
            print("Recording stopped")
            await client.stop_notify(audio_characteristic.uuid)

        async def record_and_process():
            while True:
                await record_audio()
                asyncio.ensure_future(process_audio(audio_data.copy()))
                audio_data.clear()

        await record_and_process()


asyncio.run(main())
