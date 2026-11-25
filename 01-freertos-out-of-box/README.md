# Module 1: Out-of-the-Box FreeRTOS Demonstration

Quick checklist for running the factory QSPI image on the Avnet ZUBoard 1CG:

1. Set DIP switch **SW2** to **`1011`** (QSPI Boot Mode).
2. Connect the 15V 45W USB-C power supply, Micro-USB cable, and Ethernet cable.
3. Press **SW7** to power on the board.
4. Connect to the UART serial terminal:
   ```bash
   sudo picocom -b 115200 /dev/ttyUSB1
   ```
5. If no output appears after `terminal ready`, press **SW6** (PS Reset).
6. Note the printed IP address, configure your host Ethernet adapter to the same subnet, and open the IP address in a web browser to control the on-board LEDs and monitor sensor telemetry.
