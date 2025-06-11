#!/bin/bash

echo "==== Kali Firewall Uninstaller ===="

# Remove iptables rules
echo "[*] Removing iptables NFQUEUE rules..."
sudo ./scripts/reset_iptables.sh

# Remove build artifacts
echo "[*] Removing build directory and binaries..."
rm -rf build
rm -f firewall

# Optionally remove logs and config (uncomment if desired)
# echo "[*] Removing logs and config..."
# rm -rf logs/*
# rm -rf config/*

echo "==== Uninstallation Complete ===="