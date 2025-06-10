#!/bin/bash
# Run as root

echo "[*] Flushing all iptables rules..."
iptables -F

echo "[*] iptables rules reset."