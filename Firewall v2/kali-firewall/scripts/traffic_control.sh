#!/bin/bash

IFACE=${1:-eth0}
RATE=${2:-1024} # in kbit/s

if [ "$3" == "clear" ]; then
    echo "[*] Clearing traffic shaping on $IFACE..."
    tc qdisc del dev $IFACE root
else
    echo "[*] Applying traffic shaping on $IFACE at $RATE kbit/s..."
    tc qdisc replace dev $IFACE root tbf rate ${RATE}kbit burst 32kbit latency 400ms
fi