#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

FIREWALL_BIN="../build/firewall"
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

echo -e "${GREEN}==== Stopping Kali Firewall ====${NC}"

# Stop the firewall process
PID=$(pgrep -f "$FIREWALL_BIN")
if [ -z "$PID" ]; then
    echo -e "${RED}[!] No running firewall process found.${NC}"
else
    echo "[*] Stopping firewall process (PID: $PID)..."
    sudo kill "$PID"
    sleep 1
    if pgrep -f "$FIREWALL_BIN" > /dev/null; then
        echo -e "${RED}[!] Failed to stop firewall process.${NC}"
        exit 1
    else
        echo -e "${GREEN}[*] Firewall process stopped successfully.${NC}"
    fi
fi

# Optionally, remove iptables rules if teardown_iptables.sh exists
if [ -f "./teardown_iptables.sh" ]; then
    echo "[*] Removing iptables rules..."
    if sudo ./teardown_iptables.sh; then
        echo -e "${GREEN}[*] iptables rules removed successfully.${NC}"
    else
        echo -e "${RED}[!] Failed to remove iptables rules.${NC}"
    fi
else
    echo "[*] No teardown_iptables.sh script found. Skipping iptables cleanup."
fi

echo -e "${GREEN}==== Firewall stopped. ====${NC}"