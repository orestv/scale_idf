#!/usr/bin/env bash
pio run && scp .pio/build/esp32dev/firmware.bin pi@ssh.trollcave.xyz:/home/pi/ch/data/firmware/