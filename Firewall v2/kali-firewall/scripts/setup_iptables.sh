#!/bin/bash
# Run as root

echo "[*] Flushing old iptables rules..."
iptables -F

echo "[*] Redirecting all INPUT and OUTPUT packets to NFQUEUE 0..."
iptables -A INPUT -j NFQUEUE --queue-num 0
iptables -A OUTPUT -j NFQUEUE --queue-num 0

echo "[*] iptables rules set for NFQUEUE 0."