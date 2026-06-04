import json
import tinytuya
import socket

with open("config/tadiran_config.json", 'r') as f:
    data = json.load(f)

device_id = data["device_id"]
device_local_key = data["local_key"]
device_ip = data["ip"]
PORT = data["port"]

device = tinytuya.Device(device_id, device_ip, device_local_key)
device.set_version(3.3)

import socket

HOST = '0.0.0.0'  # Listen on all interfaces

COMMAND_MAP = {
    "on":  lambda d: d.set_value(1, True),
    "off": lambda d: d.set_value(1, False),
}

PARAM_COMMAND_MAP = {
    "set_temp": lambda d, v: d.set_value(2, int(v)),
    "set_mode": lambda d, v: d.set_value(4, v),
    "set_fan":  lambda d, v: d.set_value(5, v),
}

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind((HOST, PORT))
    server.listen()
    print(f"Listening on {HOST}:{PORT}")

    while True:
        conn, addr = server.accept()
        with conn:
            print(f"Connected by {addr}")
            while True:
                data = conn.recv(1024)
                if not data:
                    break
                cmd = data.decode().strip()
                print(f"Received: {cmd}")
                if ":" in cmd:
                    key, value = cmd.split(":", 1)
                    if key in PARAM_COMMAND_MAP:
                        try:
                            PARAM_COMMAND_MAP[key](device, value)
                            conn.sendall(b"OK\n")
                        except Exception as e:
                            conn.sendall(f"ERROR: {e}\n".encode())
                    else:
                        conn.sendall(b"ERROR: unknown command\n")
                elif cmd in COMMAND_MAP:
                    try:
                        COMMAND_MAP[cmd](device)
                        conn.sendall(b"OK\n")
                    except Exception as e:
                            conn.sendall(f"ERROR: {e}\n".encode())
                else:
                    conn.sendall(b"ERROR: unknown command\n")
