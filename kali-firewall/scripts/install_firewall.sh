#!/bin/bash

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

echo -e "${GREEN}==== Kali Firewall Installer ====${NC}"

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
        echo -e "${RED}[!] Missing packages: ${MISSING_PKGS[*]}${NC}"
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
            echo -e "${RED}[!] Required script $script not found in $(pwd). Exiting.${NC}"
            exit 1
        fi
        if [ ! -x "./$script" ]; then
            echo "[*] Setting executable permission for $script"
            chmod +x "./$script"
        fi
    done
}

# Ensure we are in the scripts directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

# Move to project root
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
cd "$PROJECT_ROOT"

# Step 1: Check and install packages
check_and_install_packages

# Step 2: Check and chmod scripts
cd "$SCRIPT_DIR"
check_and_chmod_scripts

# Step 3: Build the firewall
echo "[*] Building the firewall..."
cd "$PROJECT_ROOT"
if [ ! -d build ]; then
    mkdir build
fi
cd build
cmake .. || { echo -e "${RED}[!] CMake configuration failed. Exiting.${NC}"; exit 1; }
make || { echo -e "${RED}[!] Build failed. Exiting.${NC}"; exit 1; }

# Step 4: Verify binary
if [ ! -f firewall ]; then
    echo -e "${RED}[!] Firewall binary not found after build. Exiting.${NC}"
    exit 1
fi

# Step 5: Setup iptables rules
cd "$SCRIPT_DIR"
echo "[*] Setting up iptables rules for NFQUEUE..."
if sudo ./setup_iptables.sh; then
    echo -e "${GREEN}[*] iptables rules set up successfully.${NC}"
else
    echo -e "${RED}[!] Failed to set up iptables rules. Exiting.${NC}"
    exit 1
fi

echo -e "${GREEN}==== Installation Complete! ====${NC}"
echo "You can now use start_firewall.sh and stop_firewall.sh to control the firewall."

# Optional: Prompt to run the firewall now
read -p "Do you want to start the firewall now? [y/N]: " yn
case $yn in
    [Yy]* ) ./start_firewall.sh ;;
    * ) echo "You can start it later with ./start_firewall.sh";;
esac