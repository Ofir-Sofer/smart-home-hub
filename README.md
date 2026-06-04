# Smart Home Hub

A multithreaded C++ smart home hub running on Raspberry Pi 5 with Hailo8 AI HAT. Controls home devices via a Telegram bot interface, with local LLM-based natural language command processing running fully on-device.

## Features

- **Multithreaded architecture** — per-device threads with independent message queues
- **Telegram bot interface** — send commands to your home devices via chat
- **Swappable AI encoders** — pluggable encoder architecture (Strategy Pattern) supports rule-based (`SimpleEncoder`) and local LLM (`LlamaEncoder` via Phi-3-mini/llama.cpp). No cloud dependency.
- **Authorized users** — only whitelisted Telegram user IDs can send commands
- **Extensible device system** — add new devices via JSON config without recompiling
- **Python bridge for IoT devices** — lightweight Python bridges handle device-specific protocols (Tuya, etc.)
- **Graceful shutdown** — clean thread teardown on exit

## Architecture

The system is built around a producer-consumer pipeline:

```
User (Telegram) → Listener → Main Queue → Parser → Server → Device Queue → Device Thread
                                                                                    ↓
User (Telegram) ← Server ← FeedbackListener ←────────────────────────────── Device
```

For devices using a Python bridge:
```
Device Thread → TadiranDevice → [TCP socket] → tadiran_bridge.py → [Tuya local protocol] → AC unit
```

### Design Patterns Used

- **Strategy Pattern** — swappable encoder implementations (`IEncoder`). Currently supports `SimpleEncoder` (rule-based) and `LlamaEncoder` (local LLM via llama.cpp).
- **Factory Method Pattern** — `DeviceFactory` maps device type strings to constructors, loaded from JSON config at runtime.
- **Registry Pattern** — `DeviceRegistry` manages device instances and their dedicated message queues.
- **Producer-Consumer** — thread-safe `MessageQueue<T>` with mutex and condition variable. Each device gets its own queue and thread.

### Threading Model

```
Listener Thread:    Telegram polling → Main Queue
Main Thread:        Main Queue → Parser → Server → Device Queues
Device Thread (×N): Device Queue → Device → FeedbackListener → Server → User
```

## Building

### Prerequisites

#### System dependencies
```bash
sudo apt install -y git cmake build-essential libssl-dev libboost-all-dev libcurl4-openssl-dev
```

#### Python dependencies (required for device bridges)
```bash
pip3 install tinytuya --break-system-packages
```

#### llama.cpp (required for LlamaEncoder)

> **Note:** This step takes 10–30 minutes on Raspberry Pi 5. The model file is ~2.3GB. Requires at least 4GB RAM (8GB recommended).

```bash
git clone https://github.com/ggerganov/llama.cpp.git
cd llama.cpp
cmake -B build -DLLAMA_BUILD_TESTS=OFF -DLLAMA_BUILD_EXAMPLES=OFF
cmake --build build -j4
sudo cmake --install build
sudo cp build/bin/libggml*.so* /usr/local/lib/
sudo cp build/bin/libllama*.so* /usr/local/lib/
sudo cp ggml/include/ggml*.h /usr/local/include/
sudo cp ggml/include/gguf.h /usr/local/include/
sudo mkdir -p /usr/local/lib/cmake/llama
sudo cp build/llama-config.cmake /usr/local/lib/cmake/llama/
sudo cp build/llama-version.cmake /usr/local/lib/cmake/llama/
sudo mkdir -p /usr/local/lib/cmake/ggml
sudo cp build/ggml/ggml-config.cmake /usr/local/lib/cmake/ggml/
sudo cp build/ggml/ggml-version.cmake /usr/local/lib/cmake/ggml/
sudo ldconfig
```

#### Model file

Download the Phi-3-mini model and place it in the `models/` folder:

```bash
wget -P models/ https://huggingface.co/microsoft/Phi-3-mini-4k-instruct-gguf/resolve/main/Phi-3-mini-4k-instruct-q4.gguf
```

#### Environment variables

```bash
export TELEGRAM_TOKEN="your_telegram_bot_token"
```

To persist across sessions, add to `~/.bashrc`:

```bash
echo 'export TELEGRAM_TOKEN="your_token_here"' >> ~/.bashrc
source ~/.bashrc
```

### Build

```bash
cmake -S . -B build
cmake --build build
```

### Run

Start device bridges first, then the hub:

```bash
# Terminal 1 — start Tadiran bridge (if using Tadiran AC)
python3 scripts/tadiran_bridge.py

# Terminal 2 — start the hub
./build/smart_home_hub
```

Or use the provided start script:

```bash
./start.sh
```

### Run tests

```bash
./build/smart_home_hub_tests
```

### Commands

With `LlamaEncoder` — send natural language commands via Telegram:

```
turn on the ac
change ac mode to cold
set temperature to 22
turn off the ac
```

With `SimpleEncoder` — use `device_id:command` format:

```
mini_inverter:on
mini_inverter:set_temp:22
mini_inverter:set_mode:cold
```

Type `SHUTDOWN!!!` to exit cleanly.

## Configuration

### Encoder settings

The encoder and model path are configured in `config/settings.json`:

```json
{
    "model_path": "models/Phi-3-mini-4k-instruct-q4.gguf",
    "encoder": "llama",
    "available_encoders": ["simple", "llama"]
}
```

Set `"encoder": "simple"` to use rule-based parsing without a model. Set `"encoder": "llama"` for natural language processing — requires llama.cpp and a model file.

### Device configuration

Devices are configured in `config/devices.json`:

```json
{
    "devices": [
        {
            "device_id": "mini_inverter",
            "device_type": "tadiran"
        }
    ]
}
```

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
- [ ] Roborock vacuum integration (pending local API support)
- [ ] Tapo camera integration (person detection, voice commands, recording control)
- [ ] Hailo8 vision pipeline — person detection via Tapo camera feed
- [ ] Eco router integration
- [ ] Voice command support via Telegram voice messages
- [ ] Persistent logging