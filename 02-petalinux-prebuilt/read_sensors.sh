#!/bin/sh
# Read ZUBoard 1CG PS Temperature and Barometric Pressure via Linux IIO sysfs

TEMP_RAW_PATH="/sys/bus/iio/devices/iio:device0/in_temp0_ps_temp_raw"
TEMP_SCALE_PATH="/sys/bus/iio/devices/iio:device0/in_temp0_ps_temp_scale"
PRESS_RAW_PATH="/sys/bus/iio/devices/iio:device1/in_pressure_raw"
PRESS_SCALE_PATH="/sys/bus/iio/devices/iio:device1/in_pressure_scale"

if [ -f "$TEMP_RAW_PATH" ] && [ -f "$TEMP_SCALE_PATH" ]; then
    RAW_T=$(cat "$TEMP_RAW_PATH")
    SCALE_T=$(cat "$TEMP_SCALE_PATH")
    awk -v r="$RAW_T" -v s="$SCALE_T" 'BEGIN {printf "PS Temperature: %.2f °C (raw=%s, scale=%s)\n", (r * s) - 273.15, r, s}'
else
    echo "Temperature IIO device not found at $TEMP_RAW_PATH"
fi

if [ -f "$PRESS_RAW_PATH" ] && [ -f "$PRESS_SCALE_PATH" ]; then
    RAW_P=$(cat "$PRESS_RAW_PATH")
    SCALE_P=$(cat "$PRESS_SCALE_PATH")
    awk -v r="$RAW_P" -v s="$SCALE_P" 'BEGIN {printf "Barometric Pressure: %.2f hPa (raw=%s, scale=%s)\n", r * s, r, s}'
else
    echo "Pressure IIO device not found at $PRESS_RAW_PATH"
fi
