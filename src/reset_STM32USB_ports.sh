#!/bin/bash

# Define the Vendor ID and Product ID of your STM32 boards
VENDOR_ID="0483"
PRODUCT_ID="374b"

# Find STM32 boards connected to the system
STM32_BOARDS=$(lsusb | grep "ID $VENDOR_ID:$PRODUCT_ID" | awk '{print $2 ":" substr($4, 1, length($4)-1)}')

# Reset each found STM32 board
for board in $STM32_BOARDS; do
    BUS=$(echo $board | cut -f1 -d':')
    DEVICE=$(echo $board | cut -f2 -d':')
    DEVICE_ID="$BUS/$DEVICE"
    echo "Attempting to reset device: $DEVICE_ID"
    sudo usbreset $DEVICE_ID
done


# Define the target port
TARGET_PORT=6660

# Find the PID of the process listening on the target port
# We're filtering to openOCD processes here. If you want to target any process on this port, remove the grep 'openOCD'
PID=$(sudo lsof -i :$TARGET_PORT | grep 'openocd' | awk '{print $2}')

# Check if a PID was found
if [ -z "$PID" ]; then
    echo "No openOCD process found on port $TARGET_PORT"
else
    # Kill the process
    echo "Killing openOCD process with PID: $PID"
    sudo kill -9 $PID
fi
