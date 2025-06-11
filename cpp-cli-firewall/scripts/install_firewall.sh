#!/bin/bash

# Advanced Install Script for C++ CLI Firewall

set -e

# Function to check if a command exists
check_command() {
    command -v "$1" >/dev/null 2>&1
}

# Function to install a package if not present
install_package() {
    if ! dpkg -s "$1" >/dev/null 2>&1; then
        echo "[*] Installing $1..."
        sudo apt-get install -y "$1" || { echo "[!] Failed to install $1. Exiting."; exit 1; }
    else
        echo "[*] $1 is already installed."
    fi
}

echo "[*] Updating package lists..."
sudo apt-get update || { echo "[!] apt-get update failed. Exiting."; exit 1; }

REQUIRED_PACKAGES=("g++" "cmake" "make" "libpcap-dev")
for pkg in "${REQUIRED_PACKAGES[@]}"; do
    install_package "$pkg"
done

echo "[*] All required packages are installed."

# Determine the script's directory and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_ROOT" || { echo "[!] Failed to navigate to project root."; exit 1; }

BUILD_DIR="build"
if [ ! -d "$BUILD_DIR" ]; then
    mkdir "$BUILD_DIR"
fi

cd "$BUILD_DIR" || { echo "[!] Failed to navigate to build directory"; exit 1; }

echo "[*] Running CMake..."
cmake .. || { echo "[!] CMake configuration failed. Exiting."; exit 1; }

echo "[*] Building the firewall..."
make || { echo "[!] Build failed. Exiting."; exit 1; }

echo "[+] Firewall build completed successfully."

# Prompt user for runtime options
read -rp "Enter network interface to capture on (default: any): " iface
iface=${iface:-any}

read -rp "Enable DPI? (on/off, default: off): " dpi
dpi=${dpi:-off}

read -rp "Enable Traffic Shaping? (on/off, default: off): " shape
shape=${shape:-off}

# Start the firewall
FIREWALL_BIN="./cpp-cli-firewall"
if [ ! -f "$FIREWALL_BIN" ]; then
    FIREWALL_BIN="./cli-firewall" # fallback for alternate binary name
fi

if [ -f "$FIREWALL_BIN" ]; then
    echo "[*] Starting firewall..."
    "$FIREWALL_BIN" start --iface "$iface" --dpi "$dpi" --shape "$shape"
else
    echo "[!] Firewall binary not found. Build may have failed."
    exit 1
fi

echo "[+] Firewall started. Use './cpp-cli-firewall interactive' for interactive mode."