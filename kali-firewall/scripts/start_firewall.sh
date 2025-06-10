#!/bin/bash

set -e

FIREWALL_BIN="../build/firewall"
LOG_FILE="../firewall.log"

# Ensure we are in the scripts directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

# Check if firewall binary exists
if [ ! -f "$FIREWALL_BIN" ]; then
    echo "[!] Firewall binary not found at $FIREWALL_BIN. Please build first."
    exit 1
fi

# Check if already running
if pgrep -f "$FIREWALL_BIN" > /dev/null; then
    echo "[!] Firewall is already running."
    exit 1
fi

echo "[*] Starting firewall..."
sudo "$FIREWALL_BIN" | tee "$LOG_FILE"