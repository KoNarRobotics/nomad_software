#!/bin/bash

# Extract the IP address from the configuration file
config_file=$1

if [ ! -f "$config_file" ]; then
  echo "Configuration file not found!"
  exit 1
fi

echo "Using configuration file: $config_file"

IP_ADDRESS=$(grep -oP 'Address\s*=\s*\K[\d./]+' $config_file)
echo "Peer IP:$IP_ADDRESS"
echo "Starting WireGuard VPN connection..."


sudo ip link add dev wg0 type wireguard
sudo ip address add dev wg0 $IP_ADDRESS

sudo  wg setconf wg0 $config_file
sudo ip link set up dev wg0
sudo wg

# Check if the connection was successful
if [ $? -eq 0 ]; then
  echo "VPN connection established successfully."
else
  echo "Failed to establish VPN connection."
  exit 1
fi
