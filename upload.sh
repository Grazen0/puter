#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    echo "Usage: $0 [DEVICE]"
    exit 1
fi

device="$1"

udisksctl mount -b "$device"

mount_dir=$(udisksctl info --block-device "$device" \
  | awk -F: '/MountPoints/ {print $2}' \
  | xargs -n1 echo)

cp ./os/build/kernel.bin "$mount_dir"

udisksctl unmount -b "$device"
