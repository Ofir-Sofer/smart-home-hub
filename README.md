# Smart Home Hub

A multithreaded C++ smart home hub running on Raspberry Pi 5 with Hailo8 AI HAT. Controls home devices via a Telegram bot interface, with local LLM-based natural language command processing running fully on-device.

## Features

- **Multithreaded architecture** — per-device threads with independent message queues
- **Telegram bot interface** — send commands to your home devices via chat
- **Swappable AI encoders** — pluggable encoder architecture (Strategy Pattern) supports rule-based (`SimpleEncoder`) and local LLM (`LlamaEncoder` via Phi-3-mini/llama.cpp). No cloud dependency.
- **Authorized users** — only whitelisted Telegram user IDs can send commands
- **Extensible device system** — add new devices via JSON config without recompiling
- **Graceful shutdown** — clean thread teardown on exit

## Architecture

The system is built around a producer-consumer pipeline:

```
User (Telegram) → Listener → Main Queue → Parser → Server → Device Queue → Device Thread
                                                                                    ↓
User (Telegram) ← Server ← FeedbackListener ←────────────────────────────── Device
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

Download the Phi-3-mini model and place it in the project root:

```bash
wget https://huggingface.co/microsoft/Phi-3-mini-4k-instruct-gguf/resolve/main/Phi-3-mini-4k-instruct-q4.gguf
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

```bash
./build/smart_home_hub
```

### Run tests

```bash
./build/smart_home_hub_tests
```

### Commands

With `LlamaEncoder` — send natural language commands via Telegram:

```
clean the living room
turn on the kitchen light
stop the vacuum
```

With `SimpleEncoder` — use `device_id:command` format:

```
roborock:clean
tapo_kitchen:on
```

Type `SHUTDOWN!!!` to exit cleanly.

## Configuration

### Device configuration

Devices are configured in `config/devices.json`:

```json
{
    "devices": [
        {
            "device_id": "roborock_living_room",
            "device_type": "roborock"
        }
    ]
}
```

### Encoder settings

The encoder and model path are configured in `config/settings.json`:

```json
{
    "model_path": "/path/to/Phi-3-mini-4k-instruct-q4.gguf",
    "encoder": "llama",
    "available_encoders": ["simple", "llama"]
}
```

Set `"encoder": "simple"` to use rule-based parsing without a model (commands must be in `device_id:command` format). Set `"encoder": "llama"` for natural language processing — requires llama.cpp and a model file installed.

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
- [ ] Roborock vacuum integration
- [ ] Tapo camera integration (person detection, voice commands, recording control)
- [ ] Hailo8 vision pipeline — person detection via Tapo camera feed
- [ ] Eco router integration
- [ ] Tadiran AC integration
- [ ] Voice command support via Telegram voice messages
- [ ] Persistent logging