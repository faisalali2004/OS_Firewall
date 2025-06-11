#!/bin/bash
# filepath: c:\Users\faisa\OneDrive\Documents\Semester 4\Operating Systems Lab\firewall_working\kali-firewall\scripts\install_firewall.sh

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}==== Kali Firewall Installer ====${NC}"

# Root check
if [[ $EUID -ne 0 ]]; then
    echo -e "${RED}[!] Please run this script as root (sudo $0)${NC}"
    exit 1
fi

# Detect package manager
if command -v apt-get &>/dev/null; then
    PKG="apt-get"
    PKG_INSTALL="apt-get install -y"
    PKG_CHECK="dpkg -s"
elif command -v dnf &>/dev/null; then
    PKG="dnf"
    PKG_INSTALL="dnf install -y"
    PKG_CHECK="rpm -q"
elif command -v pacman &>/dev/null; then
    PKG="pacman"
    PKG_INSTALL="pacman -Syu --noconfirm"
    PKG_CHECK="pacman -Qi"
else
    echo -e "${RED}[!] No supported package manager found (apt, dnf, pacman).${NC}"
    exit 1
fi

echo -e "${GREEN}[*] Using package manager: $PKG${NC}"

REQUIRED_PKGS=(build-essential cmake qtbase5-dev qtbase5-dev-tools libsqlite3-dev libnetfilter-queue-dev nlohmann-json3-dev pkg-config iptables)
REQUIRED_SCRIPTS=(setup_iptables.sh start_firewall.sh stop_firewall.sh)

# Check and install packages
echo "[*] Checking required packages..."
MISSING_PKGS=()
for pkg in "${REQUIRED_PKGS[@]}"; do
    if ! $PKG_CHECK "$pkg" &>/dev/null; then
        MISSING_PKGS+=("$pkg")
    else
        echo "[*] $pkg already installed."
    fi
done

if [ ${#MISSING_PKGS[@]} -ne 0 ]; then
    echo -e "${YELLOW}[!] Missing packages: ${MISSING_PKGS[*]}${NC}"
    echo "[*] Installing missing packages..."
    $PKG_INSTALL "${MISSING_PKGS[@]}"
fi

# Ensure we are in the scripts directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

# Move to project root
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
cd "$PROJECT_ROOT"

# Check and chmod scripts
echo "[*] Checking script permissions..."
for script in "${REQUIRED_SCRIPTS[@]}"; do
    if [ ! -f "./scripts/$script" ]; then
        echo -e "${RED}[!] Required script $script not found in ./scripts. Exiting.${NC}"
        exit 1
    fi
    if [ ! -x "./scripts/$script" ]; then
        echo "[*] Setting executable permission for $script"
        chmod +x "./scripts/$script"
    fi
done

# Build the firewall
echo "[*] Building the firewall..."
if [ ! -d build ]; then
    mkdir build
fi
cd build
if [ ! -f Makefile ] || [ "$(find .. -name '*.cpp' -newer Makefile | wc -l)" -gt 0 ]; then
    cmake .. || { echo -e "${RED}[!] CMake configuration failed. Exiting.${NC}"; exit 1; }
fi
make -j"$(nproc)" || { echo -e "${RED}[!] Build failed. Exiting.${NC}"; exit 1; }
cd "$PROJECT_ROOT"

# Verify binary
if [ ! -f build/firewall ]; then
    echo -e "${RED}[!] Firewall binary not found after build. Exiting.${NC}"
    exit 1
fi

# Ensure logs directory and SQLite DB file exist
if [ ! -d logs ]; then
    echo "[*] Creating logs directory..."
    mkdir logs
    chmod 755 logs
fi

if [ ! -f logs/firewall_log.db ]; then
    echo "[*] Creating empty SQLite log file logs/firewall_log.db..."
    sqlite3 logs/firewall_log.db "VACUUM;"
    chmod 644 logs/firewall_log.db
fi

# Check iptables is available
if ! command -v iptables &>/dev/null; then
    echo -e "${RED}[!] iptables not found. Please install it and rerun this script.${NC}"
    exit 1
fi

# Setup iptables rules
echo "[*] Setting up iptables rules for NFQUEUE..."
if ./scripts/setup_iptables.sh; then
    echo -e "${GREEN}[*] iptables rules set up successfully.${NC}"
else
    echo -e "${RED}[!] Failed to set up iptables rules. Exiting.${NC}"
    exit 1
fi

echo -e "${GREEN}==== Installation Complete! ====${NC}"
echo "You can now use ./scripts/start_firewall.sh and ./scripts/stop_firewall.sh to control the firewall."

# Prompt to run the firewall now
read -p "Do you want to start the firewall now? [y/N]: " yn
case $yn in
    [Yy]* )
        echo "[*] Starting firewall..."
        # Set XDG_RUNTIME_DIR if not set to avoid Qt warnings
        if [ -z "$XDG_RUNTIME_DIR" ]; then
            export XDG_RUNTIME_DIR="/tmp/runtime-$USER"
            mkdir -p "$XDG_RUNTIME_DIR"
            chmod 700 "$XDG_RUNTIME_DIR"
        fi
        ./scripts/start_firewall.sh &
        FW_PID=$!
        sleep 2
        read -p "Firewall is running (PID $FW_PID). Do you want to terminate it now? [y/N]: " killnow
        case $killnow in
            [Yy]* )
                ./scripts/stop_firewall.sh
                ;;
            * )
                echo "Firewall will keep running. Use ./scripts/stop_firewall.sh to stop it later."
                ;;
        esac
        ;;
    * )
        echo "You can start it later with ./scripts/start_firewall.sh"
        ;;
esac