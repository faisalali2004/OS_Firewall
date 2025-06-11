#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

FIREWALL_BIN="../build/firewall"
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

echo -e "${GREEN}==== Stopping Kali Firewall ====${NC}"

# Stop all running firewall processes
PIDS=$(pgrep -f "$FIREWALL_BIN")
if [ -z "$PIDS" ]; then
    echo -e "${YELLOW}[!] No running firewall process found.${NC}"
else
    for PID in $PIDS; do
        echo "[*] Stopping firewall process (PID: $PID)..."
        sudo kill "$PID"
        sleep 1
        if ps -p "$PID" > /dev/null; then
            echo -e "${RED}[!] Failed to stop firewall process with PID $PID.${NC}"
        else
            echo -e "${GREEN}[*] Firewall process $PID stopped successfully.${NC}"
        fi
    done
fi

# Remove all iptables rules using NFQUEUE
echo "[*] Scanning for iptables NFQUEUE rules to remove..."
NFQUEUE_RULES=$(sudo iptables-save | grep NFQUEUE || true)
if [ -z "$NFQUEUE_RULES" ]; then
    echo -e "${YELLOW}[!] No NFQUEUE rules found in iptables.${NC}"
else
    echo "[*] Removing all NFQUEUE rules from iptables..."
    sudo iptables-save | grep NFQUEUE | while read -r rule ; do
        chain=$(echo $rule | awk '{print $2}')
        rule_spec=$(echo $rule | sed 's/-A [A-Z0-9_]* //')
        echo "[*] Removing rule from $chain: $rule_spec"
        sudo iptables -D $chain $rule_spec 2>/dev/null || \
            echo -e "${YELLOW}[!] Could not remove rule: $rule_spec from $chain (may already be gone).${NC}"
    done
    echo -e "${GREEN}[*] All NFQUEUE rules removed from iptables.${NC}"
fi

echo -e "${GREEN}==== Firewall stopped and iptables cleaned. ====${NC}"