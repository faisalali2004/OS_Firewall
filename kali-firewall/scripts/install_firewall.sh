#!/bin/bash

set -e

echo "==== Kali Firewall Installer ===="

# Function to check and install a package if missing
install_if_missing() {
    PKG="$1"
    if ! dpkg -s "$PKG" >/dev/null 2>&1; then
        echo "[*] Installing $PKG..."
        if ! sudo apt-get install -y "$PKG"; then
            echo "[!] Failed to install $PKG. Exiting."
            exit 1
        fi
    else
        echo "[*] $PKG already installed."
    fi
}

echo "[*] Updating package list..."
if ! sudo apt-get update; then
    echo "[!] Failed to update package list. Exiting."
    exit 1
fi

# Install dependencies
install_if_missing build-essential
install_if_missing cmake
install_if_missing qtbase5-dev
install_if_missing libsqlite3-dev
install_if_missing libnetfilter-queue-dev
install_if_missing nlohmann-json3-dev
install_if_missing pkg-config

# Build the project
echo "[*] Building the firewall..."
cd "$(dirname "$0")/.."
if [ ! -f CMakeLists.txt ]; then
    echo "[!] CMakeLists.txt not found. Please check your project structure."
    exit 1
fi

mkdir -p build
cd build
if ! cmake ..; then
    echo "[!] cmake configuration failed. Exiting."
    exit 1
fi
if ! make -j$(nproc); then
    echo "[!] Build failed. Exiting."
    exit 1
fi
cd ..

# Set up iptables rules
if [ -f ./scripts/setup_iptables.sh ]; then
    echo "[*] Setting up iptables rules for NFQUEUE..."
    if ! sudo ./scripts/setup_iptables.sh; then
        echo "[!] Failed to set up iptables rules. Exiting."
        exit 1
    fi
else
    echo "[!] setup_iptables.sh not found in scripts/. Skipping iptables setup."
fi

# Optionally call manage.sh if it exists
if [ -f ./scripts/manage.sh ]; then
    echo "[*] You can manage the firewall with: ./scripts/manage.sh start|stop|status"
fi

echo "==== Installation Complete ===="
echo "To run the firewall GUI, execute: sudo ./build/firewall"