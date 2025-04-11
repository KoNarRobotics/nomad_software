#!/bin/bash


while ! ip link show can0 > /dev/null 2>&1; do
  sleep 1
done

sudo ip link set can0 txqueuelen 1000
sudo ip link set dev can0 up type can tq 10 prop-seg 36 phase-seg1 37 phase-seg2 25 sjw 21
