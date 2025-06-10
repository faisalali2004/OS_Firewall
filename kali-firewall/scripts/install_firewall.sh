#!/bin/bash

set -e

echo "==== Kali Firewall Installer ===="

# List of required packages
REQUIRED_PKGS=(build-essential cmake qtbase5-dev qtbase5-dev-tools libsqlite3-dev libnetfilter-queue-dev nlohmann-json3-dev pkg-config)

# List of required scripts
REQUIRED_SCRIPTS=(setup_iptables.sh start_firewall.sh stop_firewall.sh)

# Function to check and install missing packages
check_and_install_packages() {
    echo "[*] Checking required packages..."
    MISSING_PKGS=()
    for pkg in "${REQUIRED_PKGS[@]}"; do
        if ! dpkg -s "$pkg" &>/dev/null; then
            MISSING_PKGS+=("$pkg")
        else
            echo "[*] $pkg already installed."
        fi
    done

    if [ ${#MISSING_PKGS[@]} -ne 0 ]; then
        echo "[!] Missing packages: ${MISSING_PKGS[*]}"
        echo "[*] Installing missing packages..."
        sudo apt-get update
        sudo apt-get install -y "${MISSING_PKGS[@]}"
    fi
}

# Function to check and chmod scripts
check_and_chmod_scripts() {
    echo "[*] Checking script permissions..."
    for script in "${REQUIRED_SCRIPTS[@]}"; do
        if [ ! -f "./$script" ]; then
            echo "[!] Required script $script not found in $(pwd). Exiting."
            exit 1
        fi
        if [ ! -x "./$script" ]; then
            echo "[*] Setting executable permission for $script"
            chmod +x "./$script"
        fi
    done
}

# Move to project root if in scripts/
if [ "$(basename "$PWD")" == "scripts" ]; then
    cd ..
    PROJECT_ROOT="$PWD"
    cd scripts
else
    PROJECT_ROOT="$PWD"
fi

# Step 1: Check and install packages
check_and_install_packages

# Step 2: Check and chmod scripts
check_and_chmod_scripts

# Step 3: Build the firewall
echo "[*] Building the firewall..."
cd ..
if [ ! -d build ]; then
    mkdir build
fi
cd build
cmake .. || { echo "[!] CMake configuration failed. Exiting."; exit 1; }
make || { echo "[!] Build failed. Exiting."; exit 1; }

# Step 4: Setup iptables rules
cd ../scripts
echo "[*] Setting up iptables rules for NFQUEUE..."
if sudo ./setup_iptables.sh; then
    echo "[*] iptables rules set up successfully."
else
    echo "[!] Failed to set up iptables rules. Exiting."
    exit 1
fi

echo "==== Installation Complete! ===="
echo "You can now use start_firewall.sh and stop_firewall.sh to control the firewall."