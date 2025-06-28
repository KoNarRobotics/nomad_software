#!/bin/bash

if [ "$1" == "1" ]; then
  echo "Enabling light"
  cansend can0 00001112#0F
elif [ "$1" == "0" ]; then
  echo "Disabling light"
  cansend can0 00001112#00
else
  echo "Usage: $0 [1|0]"
  exit 1
fi