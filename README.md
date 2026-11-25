# Robot Natal: Christmas Robot with Wireless Gamepad and Sound

A remote-controlled Christmas (Santa) robot: a Bluetooth gamepad drives the robot through an ESP-NOW link, and gamepad buttons trigger sound effects played by a PC.

<!-- TODO: add demo GIF/photo -->

## Architecture

```
Gamepad ──BT──> ESP32 Master ──ESP-NOW──> ESP32 Slave ──UART2 (115200)──> Robot motor controller
                (Bluepad32)                    │
                                               └──USB serial (500000)──> PC: main.py ──> speakers
```

- **Master** (`Master/`): reads the gamepad with Bluepad32, maps the sticks to `x`, `y`, `z` speed values and sends them over ESP-NOW every 40 ms. Face buttons send one-shot sound commands.
- **Slave** (`Slave/`): receives ESP-NOW packets, forwards motion (`M x,y,z`) to the robot over UART2 (GPIO 16/17), and forwards all packets to the PC over USB serial.
- **PC** (`main.py`): reads the serial stream with low latency and plays the matching MP3 with Pygame.

## Features

- Low-latency ESP-NOW link (no Wi-Fi router needed)
- Exponential stick mapping with saturation near full deflection, max speed 140
- Separate message types for motion (`M`) and discrete commands (`D`)
- PC audio player drains the serial buffer each cycle to avoid lag from stale data

## Packet format

```c
typedef struct {
  char flag;   // 'M' = motion, 'D' = button command
  char cmd;    // 'a'..'d' when flag == 'D'
  int  x, y, z;
} struct_message;
```

## Button to sound mapping

| Button | Command | File |
| --- | --- | --- |
| Cross | `a` | `Sound/bioskop.mp3` |
| Triangle | `b` | `Sound/Jilat.mp3` |
| Circle | `c` | `Sound/Natal.mp3` |
| Square | `d` | `Sound/Welcome.mp3` |

## Setup

### Firmware

1. Install the Bluepad32 board package for the Master ESP32 and the standard ESP32 core for the Slave.
2. In `Master/Master.ino`, set `broadcastAddress` to the Slave ESP32's MAC address (or `FF:FF:FF:FF:FF:FF` to broadcast).
3. Upload `Master/Master.ino` and `Slave/Slave.ino` to their boards.

### PC audio

```bash
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
python main.py
```

Set `SERIAL_PORT` in `main.py` to the Slave's port (default `/dev/ttyUSB0`).
