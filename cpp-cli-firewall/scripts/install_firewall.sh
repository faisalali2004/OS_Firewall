#!/bin/bash

# Script to install and manage the C++ CLI Firewall on Kali Linux

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

REQUIRED_PACKAGES=("g++" "cmake" "make")
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

cmake .. || { echo "[!] CMake configuration failed. Exiting."; exit 1; }
make || { echo "[!] Build failed. Exiting."; exit 1; }

echo "[+] Firewall build completed successfully."

# Function to start the firewall
start_firewall() {
    echo "[*] Starting firewall..."
    if [ -f ./cpp-cli-firewall ]; then
        ./cpp-cli-firewall start || { echo "[!] Failed to start firewall."; exit 1; }
    else
        echo "[!] Firewall binary not found. Build may have failed."
        exit 1
    fi
}

# Function to stop the firewall and clean up rules
stop_firewall() {
    echo "[*] Stopping firewall and cleaning up rules..."
    if [ -f ./cpp-cli-firewall ]; then
        ./cpp-cli-firewall stop || echo "[!] Firewall stop command failed."
    fi
    # Flush iptables rules (requires sudo)
    if check_command iptables; then
        sudo iptables -F
        echo "[*] iptables rules flushed."
    fi
}

# Ask user to start or stop firewall
while true; do
    echo "Choose an action:"
    echo "  1) Start Firewall"
    echo "  2) Stop Firewall and Flush Rules"
    echo "  3) Exit"
    read -rp "Enter choice [1-3]: " choice
    case "$choice" in
        1) start_firewall ;;
        2) stop_firewall ;;
        3) echo "Exiting installer."; exit 0 ;;
        *) echo "Invalid choice." ;;
    esac
done