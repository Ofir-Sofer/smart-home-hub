# Smart Home Hub

A multithreaded C++ smart home hub running on Raspberry Pi 5 with Hailo8 AI HAT. Controls home devices via a Telegram bot interface, with local LLM-based natural language command processing and Hailo8-accelerated speech recognition — fully on-device, no cloud dependency.

## Features

- **Multithreaded architecture** — per-device threads with independent message queues
- **Telegram bot interface** — send text or voice messages to control your home devices
- **Voice commands** — Hailo8 AI HAT runs OpenAI Whisper for on-device speech-to-text
- **Swappable AI encoders** — pluggable encoder architecture (Strategy Pattern) supports rule-based (`SimpleEncoder`) and local LLM (`LlamaEncoder` via Phi-3-mini/llama.cpp). No cloud dependency.
- **Authorized users** — only whitelisted Telegram user IDs can send commands
- **Extensible device system** — add new devices via JSON config without recompiling
- **Python bridges for IoT devices** — lightweight Python bridges handle device-specific protocols (Tuya, etc.)
- **Home Assistant integration** — devices exposed via Home Assistant (e.g. Roborock vacuum) are controlled over its REST API, no separate bridge process needed
- **User feedback** — every command gets a Telegram response: confirmation on receipt, and device result (success/failure with command details) on completion
- **Graceful shutdown** — clean thread and process teardown on exit

## Architecture

The system is built around a producer-consumer pipeline:

```
User (Telegram) → Listener → Main Queue → Parser → Server → Device Queue → Device Thread
                                                                                    ↓
User (Telegram) ←──────── FeedbackListener ←─────────────────────────────── Device
```

Voice command flow:
```
Voice message → Listener downloads .ogg → pushes "voice_msg:/tmp/path"
→ Parser converts .ogg → .wav (ffmpeg)
→ hailo_speech_to_text.py runs Whisper on Hailo8
→ transcribed text → LlamaEncoder → device command
```

For devices using a Python bridge:
```
Device Thread → TadiranDevice → [TCP socket] → tadiran_bridge.py → [Tuya local protocol] → AC unit
```

For devices integrated via Home Assistant:
```
Device Thread → RoborockDevice → [HTTP REST, libcurl] → Home Assistant → [native Roborock integration] → vacuum
```

### Design Patterns Used

- **Strategy Pattern** — swappable encoder implementations (`IEncoder`). Currently supports `SimpleEncoder` (rule-based) and `LlamaEncoder` (local LLM via llama.cpp), which inherits from `SmartEncoder` — an intermediate base that caches the device/command list once at construction, shared by any future LLM-backed encoder.
- **Factory Method Pattern** — `DeviceFactory` maps device type strings to constructors, loaded from JSON config at runtime. Construction failures are isolated per-device (logged and skipped) rather than crashing the whole hub.
- **Registry Pattern** — `DeviceRegistry` manages device instances and their dedicated message queues.
- **Producer-Consumer** — thread-safe `MessageQueue<T>` with mutex and condition variable. Each device gets its own queue and thread.

### Threading Model

```
Listener Thread:    Telegram polling → Main Queue
Main Thread:        Main Queue → Parser → Server → Device Queues
Device Thread (×N): Device Queue → Device → FeedbackListener → User
```

## Building

### Quick setup

Run the setup script from the project root to install all dependencies, build and install llama.cpp system-wide, and download the configured model:

```bash
./setup.sh
```

It's idempotent (safe to re-run) and halts on the first error with an explanatory message. It does not run the project build itself — see [Build](#build) below for that. The sections below document what it automates, for reference or for setting things up manually.

### Prerequisites

#### System dependencies
```bash
sudo apt install -y git cmake build-essential libssl-dev libboost-all-dev libcurl4-openssl-dev ffmpeg libportaudio2 ninja-build
```

#### Python dependencies (required for device bridges and voice commands)
```bash
pip3 install tinytuya --break-system-packages
```

#### hailo-apps (required for voice commands via Hailo8 Whisper)
```bash
# Clone inside the smart-home-hub project root directory
git clone https://github.com/hailo-ai/hailo-apps.git hailo-apps
cd hailo-apps
sudo pip3 install -e ".[speech-rec]" --break-system-packages
sudo mkdir -p /usr/local/hailo && sudo chmod 777 /usr/local/hailo
cd ..
```

#### llama.cpp (required for LlamaEncoder)

> **Note:** This step takes 10–30 minutes on Raspberry Pi 5. The model file is ~2.3GB. Requires at least 4GB RAM (8GB recommended).

```bash
git clone https://github.com/ggerganov/llama.cpp.git
cd llama.cpp
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DLLAMA_BUILD_TESTS=OFF -DLLAMA_BUILD_EXAMPLES=OFF
cmake --build build -j$(nproc)
sudo cmake --install build
sudo ldconfig
cd ..
```

`cmake --install` places all required libraries, headers, and CMake config files (`llama-config.cmake`, `ggml-config.cmake`, etc.) under `/usr/local` on its own — no manual copying needed.

#### Model file

The model to download is determined by `active_model` in `config/settings.json` (see [Encoder settings](#encoder-settings) below).

> **Note:** `models/` is not tracked by git. It is created by `setup.sh` — either as a local directory (default) or as a symlink to an external drive (`--external-drive <path>`). If you are running setup manually, create it yourself: `mkdir models/`.

For the default configuration:

```bash
wget -P models/ https://huggingface.co/microsoft/Phi-3-mini-4k-instruct-gguf/resolve/main/Phi-3-mini-4k-instruct-q4.gguf
```

#### Home Assistant (required for Roborock vacuum integration)

Devices integrated via Home Assistant's REST API (currently the Roborock vacuum) need a running Home Assistant instance. A Docker Compose file is provided:

```bash
cd docker
docker compose up -d
```

This runs Home Assistant Container with its config persisted to `/mnt/storage/homeassistant` (adjust the volume path in `docker/compose.yml` if your storage layout differs). Once it's up, complete the one-time setup at `http://<pi-ip>:8123`, add your Roborock vacuum through HA's native Roborock integration, and generate a long-lived access token under your HA profile — you'll need it for `HA_TOKEN` below.

#### Environment variables

```bash
export TELEGRAM_TOKEN="your_telegram_bot_token"
export HA_TOKEN="your_home_assistant_long_lived_token"
```

To persist across sessions, add to `~/.bashrc`:

```bash
echo 'export TELEGRAM_TOKEN="your_token_here"' >> ~/.bashrc
echo 'export HA_TOKEN="your_ha_token_here"' >> ~/.bashrc
source ~/.bashrc
```

`HA_TOKEN` is only required if a Home Assistant-integrated device (e.g. Roborock) is enabled in `config/devices.json`.

### Build

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

### Run

```bash
./build/smart_home_hub
```

The Tadiran bridge starts and stops automatically with the hub.

### Run tests

```bash
./build/smart_home_hub_tests
```

### Commands

With `LlamaEncoder` — send natural language text or voice messages via Telegram:

```
turn on the ac
change ac mode to cold
set temperature to 22
turn off the ac
start the vacuum
send the vacuum back to the dock
run the living room cleaning routine
```

With `SimpleEncoder` — use `device_id:command` format:

```
mini_inverter:on
mini_inverter:set_temp:22
mini_inverter:set_mode:cold
roborock:start
roborock:return_to_base
roborock:set_fan_speed:quiet
roborock:run_routine:living_room
```

Type `SHUTDOWN!!!` to exit cleanly.

## Configuration

### Encoder settings

The encoder and model selection are configured in `config/settings.json`:

```json
{
    "active_model": "phi-3-mini-4k-instruct-q4",
    "encoder": "llama",
    "available_encoders": ["simple", "llama"],
    "available_models": {
        "phi-3-mini-4k-instruct-q4": {
            "path": "models/Phi-3-mini-4k-instruct-q4.gguf",
            "url": "https://huggingface.co/microsoft/Phi-3-mini-4k-instruct-gguf/resolve/main/Phi-3-mini-4k-instruct-q4.gguf",
            "expected_size_mb": 2390,
            "min_ram_mb": 4096
        }
    }
}
```

Set `"encoder": "simple"` to use rule-based parsing without a model. Set `"encoder": "llama"` for natural language processing — requires llama.cpp and a model file.

`active_model` selects an entry from `available_models` by key. Each entry holds the local file path, the download URL, the expected file size, and the minimum RAM needed to run that model — this is what `setup.sh` reads to download and validate the model automatically. Adding a new model means adding a new entry here and pointing `active_model` at it.

### Device configuration

Devices are configured in `config/devices.json`:

```json
{
    "devices": [
        {
            "device_id": "mini_inverter",
            "device_type": "tadiran"
        },
        {
            "device_id": "roborock",
            "device_type": "roborock"
        }
    ]
}
```

If a device fails to construct (e.g. an unreachable bridge or invalid config), the hub logs the error and continues starting up without that device — it simply won't appear in the device list or respond to commands. Other configured devices are unaffected.

### Tadiran AC bridge

Copy the example config and fill in your device credentials:

```bash
cp config/tadiran_config.example.json config/tadiran_config.json
```

Edit `config/tadiran_config.json` with your device details (obtain via [tinytuya wizard](https://github.com/jasonacox/tinytuya)):

```json
{
    "device_id": "your_device_id",
    "local_key": "your_local_key",
    "ip": "192.168.x.x",
    "bridge_ip": "127.0.0.1",
    "port": 9999
}
```

Supported Tadiran commands: `on`, `off`, `set_temp:X` (16-32°C), `set_mode:X` (auto/cold/hot/wet/wind), `set_fan:X` (low/middle/high/auto/sleep/etc.)

### Roborock vacuum

The Roborock vacuum is controlled through Home Assistant's REST API rather than a direct device connection — see [Home Assistant setup](#home-assistant-required-for-roborock-vacuum-integration) above to get HA running and connected to the vacuum first.

Copy the example config and fill in your entity details:

```bash
cp config/roborock_config.example.json config/roborock_config.json
```

Edit `config/roborock_config.json`:

```json
{
    "base_url": "http://192.168.x.x:8123",
    "vacuum_entity_id": "vacuum.your_vacuum_entity",
    "button_entity_prefix": "button.your_vacuum_",
    "routine_names": ["living_room", "bedroom"],
    "speed_values": ["quiet", "balanced", "turbo", "max"]
}
```

- `vacuum_entity_id` is the vacuum's entity ID in HA (`vacuum.*` domain) — used for `start`, `pause`, `stop`, `return_to_base`, `locate`, `clean_spot`, and `set_fan_speed`.
- `button_entity_prefix` is the shared prefix of your Roborock routine entities in HA (`button.*` domain) — routines are exposed as buttons, not vacuum commands. `run_routine:<name>` appends `<name>` to this prefix to find the right entity.
- `routine_names` and `speed_values` are the allowed values for `run_routine` and `set_fan_speed` — commands with any other value are rejected before an HTTP request is made. These same lists are surfaced to `LlamaEncoder` so the model only ever sees valid options.

Supported Roborock commands: `start`, `pause`, `stop`, `return_to_base`, `locate`, `clean_spot`, `set_fan_speed:X` (one of `speed_values`), `run_routine:X` (one of `routine_names`). Multi-parameter commands (e.g. cleaning specific rooms by name in one call) aren't supported yet — tracked on the [Roadmap](#roadmap).

Requires the `HA_TOKEN` environment variable (see [Environment variables](#environment-variables)).

### Authorized users

Only whitelisted Telegram user IDs can send commands. Copy the example file and edit it with your real IDs:

```bash
cp config/authorized_users.example.json config/authorized_users.json
```

To find your Telegram user ID, send any message to your bot and check the terminal output.

## Roadmap

- [x] Telegram bot integration
- [x] User authentication (authorized users whitelist)
- [x] Local LLM encoder (Phi-3-mini via llama.cpp)
- [x] Tadiran AC integration (local Tuya protocol via Python bridge)
- [x] Voice command support (Hailo8 Whisper speech-to-text)
- [x] Automated setup script (`setup.sh`)
- [x] External drive integration (offload model/build artifacts from SD card to free disk space)
- [x] Roborock vacuum integration (via Home Assistant REST API; simple commands only — multi-parameter commands like room-specific cleaning pending)
- [ ] Hebrew language support for voice and text commands
- [ ] Tapo camera integration (person detection, voice commands, recording control)
- [ ] Hailo8 vision pipeline — person detection via Tapo camera feed
- [ ] Eco router integration
- [ ] Persistent logging
- [ ] Hailo10 integration
