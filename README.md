# Smart Home Hub

A multithreaded C++ smart home hub running on Raspberry Pi 5 with Hailo8 AI HAT. Controls home devices via a Telegram bot interface, with local AI inference for natural language command processing.

## Features

- **Multithreaded architecture** — per-device threads with independent message queues
- **Telegram bot interface** — send commands to your home devices via chat
- **Local AI inference** — Hailo8 HAT for on-device natural language processing (no cloud dependency)
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

- **Strategy Pattern** — swappable encoder implementations (`IEncoder`). Currently uses `SimpleEncoder` for rule-based parsing; designed to plug in Hailo8 AI encoder.
- **Factory Method Pattern** — `DeviceFactory` maps device type strings to constructors, loaded from JSON config at runtime.
- **Registry Pattern** — `DeviceRegistry` manages device instances and their dedicated message queues.
- **Producer-Consumer** — thread-safe `MessageQueue<T>` with mutex and condition variable. Each device gets its own queue and thread.

### Threading Model

```
Listener Thread: Telegram polling → Main Queue
Main Thread:     Main Queue → Parser → Server → Device Queues
Device Thread (×N): Device Queue → Device → FeedbackListener → Server → User
```

## Building

### Prerequisites

### Prerequisites
- CMake 3.20+
- GCC with C++17 support
- Install system dependencies:
```bash
  sudo apt install libssl-dev libboost-all-dev libcurl4-openssl-dev
```
- Internet connection (nlohmann/json and tgbot-cpp fetched automatically via CMake)

### Build

```bash
mkdir build
cd build
cmake ..
make
```

### Run

```bash
./build/smart_home_hub
```

### Commands

Send commands in the format `device_id:command`. Example:

```
roborock:clean
tapo_kitchen:on
```

Type `SHUTDOWN!!!` to exit cleanly.

## Device Configuration

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

## Authorized Users Configuration

Authorized user IDs are stored in `config/authorized_users.json` (not tracked by git). Copy the example file and edit it with your real IDs:

```bash
cp config/authorized_users.example.json config/authorized_users.json
```

Then open `config/authorized_users.json` and replace the placeholder IDs with your actual user IDs.

## Roadmap

- [x] Telegram bot integration
- [x] User authentication (authorized users whitelist)
- [ ] Hailo8 AI encoder for natural language commands
- [ ] Roborock vacuum integration
- [ ] Tapo smart plug integration
- [ ] Tadiran AC integration
- [ ] User authentication
- [ ] Persistent logging
