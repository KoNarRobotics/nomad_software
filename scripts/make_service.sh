#!/bin/bash

services=(
  nomad_base.service
  nomad_nav.service
)


dir="$(dirname "$0")"
cd "$dir/../services"
echo "Services to start: ${services[*]}"

for svc in "${services[@]}"; do
  sudo cp "$svc" /etc/systemd/system
done

sudo systemctl daemon-reload

for svc in "${services[@]}"; do
  sudo systemctl enable $svc
done
