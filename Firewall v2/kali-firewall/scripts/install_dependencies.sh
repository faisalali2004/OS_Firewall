#!/bin/bash

echo "[*] Installing dependencies..."
sudo apt update
sudo apt install -y build-essential qtbase5-dev libsqlite3-dev libnetfilter-queue-dev nlohmann-json3-dev pkg-config
echo "[*] All dependencies installed."