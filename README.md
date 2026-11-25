# Avnet ZUBoard 1CG: Comprehensive Getting Started Tutorial

Welcome to the **Avnet ZUBoard 1CG** (AMD Zynq™ UltraScale+™ MPSoC `xczu1cg-sbva484-1-e`) hands-on learning repository. This guide is designed as a structured, step-by-step tutorial taking you from initial hardware bring-up and out-of-the-box demonstrations to custom Programmable Logic (PL) hardware design in Vivado, bare-metal C++ firmware development in Vitis, and building a custom PetaLinux image for the dual-camera module.

---

## Table of Contents

1. [Repository Structure](#repository-structure)
2. [Hardware Requirements & Boot Mode Reference](#hardware-requirements--boot-mode-reference)
3. [Development Environment Setup](#development-environment-setup)
4. [Module 1: Out-of-the-Box FreeRTOS Demonstration](#module-1-out-of-the-box-freertos-demonstration)
5. [Module 2: Pre-Built PetaLinux Image & On-Board Sensors](#module-2-pre-built-petalinux-image--on-board-sensors)
6. [Module 3: Bare-Metal Vivado PL Logic Gate & Vitis C++ Application](#module-3-bare-metal-vivado-pl-logic-gate--vitis-c-application)
   - [Part A: Bare-Metal Vivado Validation (PS Only)](#part-a-bare-metal-vivado-validation-ps-only)
   - [Part B: Adding FPGA Programmable Logic (Push-Button SW3 & RGB LED)](#part-b-adding-fpga-programmable-logic-push-button-sw3--rgb-led)
   - [Part C: Building the Bare-Metal Application in Vitis](#part-c-building-the-bare-metal-application-in-vitis)
7. [Module 4: Hands-On with the Dual-Camera Module (PetaLinux 2024.2)](#module-4-hands-on-with-the-dual-camera-module-petalinux-20242)
   - [Part A: Generating the Vivado Hardware Design (XSA)](#part-a-generating-the-vivado-hardware-design-xsa)
   - [Part B: Building the PetaLinux Image Inside Docker](#part-b-building-the-petalinux-image-inside-docker)
   - [Part C: Booting & Running the Dual-Camera Passthrough Demo](#part-c-booting--running-the-dual-camera-passthrough-demo)
8. [Troubleshooting & Helpful Shortcuts](#troubleshooting--helpful-shortcuts)

---

## Repository Structure

```text
zuboard-1cg-getting-started/
├── README.md                                      # Main comprehensive tutorial (this file)
├── .gitignore                                     # Standard GitLab/Vivado/Vitis ignore rules
├── 01-freertos-out-of-box/
│   └── README.md                                  # Quick reference for the shipped QSPI FreeRTOS demo
├── 02-petalinux-prebuilt/
│   └── read_sensors.sh                            # Helper script to read PS temperature and barometric pressure
├── 03-bare-metal-pl-gpio/
│   ├── constraints/
│   │   └── zuboard_1cg_pl_gpio.xdc                # Pin constraints for Push-Button SW3 and Blue RGB LED
│   └── src/
│       ├── main.cpp                               # Bare-metal C++ AXI GPIO polling application
│       └── UserConfig.cmake                       # Vitis CMake configuration snippet for C++ compilation
└── 04-dual-camera-petalinux/
    └── scripts/
        └── setup_docker_petalinux.sh              # Automation script for Ubuntu 22.04 Docker PetaLinux build
```

---

## Hardware Requirements & Boot Mode Reference

### Required Hardware
1. **Avnet ZUBoard 1CG** development board (`xczu1cg-sbva484-1-e`)
2. **External USB-C Power Supply**: 15V / 45W USB-C power adapter and cable
3. **Micro-USB Cable**: Used for UART serial console and JTAG debugging
4. **Ethernet Cable**: RJ45 patch cable for network connectivity and web/SSH access
5. **MicroSD Card**: 32 GB capacity (Class 10 / UHS-I recommended)
6. *(Optional for Module 4)* **Avnet Dual-Camera Mezzanine Module** (AR0144 sensors)

### Boot Mode Switch (`SW2`) Reference
The 4-position DIP switch **SW2** controls the Zynq UltraScale+ MPSoC boot source. Ensure power is disconnected before changing switch positions:

| Boot Mode | SW2 Setting (`[1:4]`) | Primary Use Case |
| :--- | :---: | :--- |
| **QSPI Flash** | `1011` | Running the factory-shipped FreeRTOS out-of-the-box demo |
| **SD Card** | `0101` | Booting pre-built or custom PetaLinux images from the MicroSD card |
| **JTAG** | `1111` | Bare-metal hardware validation, Vivado bitstream programming, and Vitis debugging |

> **Note:** On the ZUBoard 1CG, **SW7** is the board power-on push-button switch, and **SW6** is the PS system reset push-button.

---

## Development Environment Setup

The tutorials in this repository were validated using the following host environment:

* **Host Operating System:** Ubuntu 24.04.4 LTS
* **AMD Unified Installer:** `FPGAs_AdaptiveSoCs_Unified_SDI_2026.1_0616_1700_Lin64` (includes Vivado™ and Vitis™ 2026.1)
* **PetaLinux Toolchain (Module 4):** PetaLinux 2024.2 & Vivado 2024.2 (executed cleanly inside an Ubuntu 22.04 Docker container)

### Verifying Your Vitis & Cross-Compiler Installation
After sourcing your AMD tools installation script (e.g. `source /tools/Xilinx/Vitis/2026.1/settings64.sh`), verify that Vitis and the bare-metal ARM toolchains are accessible in your `PATH`:

```bash
which vitis
aarch64-none-elf-gcc --version
armr5-none-eabi-gcc --version
```

You should also install `picocom` on your host machine to communicate with the board over UART:

```bash
sudo apt-get update && sudo apt-get install -y picocom
```

---

## Module 1: Out-of-the-Box FreeRTOS Demonstration

The ZUBoard 1CG ships with a pre-flashed FreeRTOS web-server demonstration stored in its on-board QSPI flash memory.

### Step-by-Step Instructions
1. **Configure Boot Mode:** Set **SW2** to **`1011`** (QSPI boot mode).
2. **Connect Cables:**
   - Connect the Micro-USB cable between your host PC and the ZUBoard USB-UART/JTAG port.
   - Connect the Ethernet cable directly to your host PC or local network router.
   - Connect the 15V 45W USB-C power supply.
3. **Power On:** Press **SW7** to power up the board.
4. **Open the Serial Console:** Launch `picocom` at `115200` baud on `/dev/ttyUSB1` (note: your device index may vary depending on connected USB serial adapters):
   ```bash
   sudo picocom -b 115200 /dev/ttyUSB1
   ```
5. **Obtain the Board IP Address:**
   - Observe the IP address printed in the serial terminal.
   - *Tip:* If the terminal displays `terminal ready` with no boot output, press **SW6** to reset the processor and trigger the boot log.
   - Configure your host PC's Ethernet interface to reside on the same subnet as the ZUBoard.
6. **Interact via Web Browser:**
   - Open a web browser on your PC and navigate to the IP address shown in the terminal (e.g. `http://192.168.1.10`).
   - Test toggling the on-board RGB LEDs and inspect real-time readings from the on-board temperature and pressure sensors.
7. **Exit `picocom`:** Press `Ctrl+A` followed by `Ctrl+X`.

---

## Module 2: Pre-Built PetaLinux Image & On-Board Sensors

This module demonstrates booting Linux from a MicroSD card and querying hardware sensors via the Linux Industrial I/O (`IIO`) subsystem.

### Step-by-Step Instructions
1. **Download the Pre-Built Image:**
   - Download the official ZUBoard PetaLinux BSP image from: [https://bit.ly/zuboard_v3_0_1](https://bit.ly/zuboard_v3_0_1)
2. **Flash the Image to the MicroSD Card:**
   - Extract the `.wic` or `.img` file and flash it onto your 32 GB MicroSD card using `balenaEtcher`, `Raspberry Pi Imager`, or `dd`:
     ```bash
     # Replace /dev/sdX with your actual SD card block device (verify carefully with lsblk!)
     sudo dd if=<extracted_image_name>.wic of=/dev/sdX bs=4M status=progress conv=fsync
     ```
3. **Configure Boot Mode & Power Up:**
   - Insert the flashed MicroSD card into the ZUBoard slot.
   - Set **SW2** to **`0101`** (SD Card boot mode).
   - Connect power and press **SW7** to boot the board.
4. **Connect via UART Console:**
   ```bash
   sudo picocom -b 115200 /dev/ttyUSB1
   ```
   Log in once the Linux prompt appears (default username: `root`, no password required).
5. **Read On-Board Temperature and Barometric Pressure Sensors:**
   The Zynq UltraScale+ SYSMON and on-board I2C pressure sensor expose raw values and scaling factors under `/sys/bus/iio/devices/`. Run the following commands (or execute [`02-petalinux-prebuilt/read_sensors.sh`](02-petalinux-prebuilt/read_sensors.sh)):

   * **Processor System (PS) Temperature (in °C):**
     ```bash
     RAW_TEMP=$(cat /sys/bus/iio/devices/iio:device0/in_temp0_ps_temp_raw)
     SCALE_TEMP=$(cat /sys/bus/iio/devices/iio:device0/in_temp0_ps_temp_scale)
     # Formula: temp (°C) = (raw * scale) - 273.15
     awk "BEGIN {printf "PS Temperature: %.2f C\n", ($RAW_TEMP * $SCALE_TEMP) - 273.15}"
     ```
   * **Barometric Pressure (in hPa):**
     ```bash
     RAW_PRESS=$(cat /sys/bus/iio/devices/iio:device1/in_pressure_raw)
     SCALE_PRESS=$(cat /sys/bus/iio/devices/iio:device1/in_pressure_scale)
     # Formula: pressure (hPa) = raw * scale
     awk "BEGIN {printf "Pressure: %.2f hPa\n", $RAW_PRESS * $SCALE_PRESS}"
     ```

---

## Module 3: Bare-Metal Vivado PL Logic Gate & Vitis C++ Application

In this module, you will create a hardware design from scratch in Vivado, route physical I/O pins through custom FPGA logic gates (`AND` and `NOT`), and read the logic output from the ARM Cortex-A53 processor using a bare-metal C++ application in Vitis.

### Part A: Bare-Metal Vivado Validation (PS Only)

Before adding Programmable Logic (PL) peripherals, verify basic Processing System (PS) bitstream generation:

1. **Set Boot Mode:** Set **SW2** to **`1111`** (JTAG mode) and power on the board (**SW7**).
2. **Create a New Vivado Project:**
   - Launch Vivado and click **Create Project**.
   - Enter a project name (e.g. `zuboard_baremetal`) and click **Next**.
   - Select **RTL Project** and tick **Do not specify sources at this time**, then click **Next**.
   - Switch to the **Parts** tab, search for **`xczu1cg-sbva484-1-e`**, select it, and click **Finish**.
3. **Create a Block Design:**
   - In **Flow Navigator** → **IP INTEGRATOR**, click **Create Block Design** and accept the default name (`design_1`).
4. **Add and Configure the Zynq UltraScale+ MPSoC IP:**
   - Click the **`+`** (Add IP) button, search for **`Zynq UltraScale+ MPSoC`**, and add it to the canvas.
   - Double-click the `zynq_ultra_ps_e_0` block to customise it:
     1. **I/O Configuration:** Expand **Low Speed** → **I/O Peripherals** and enable **UART 0** (ensure it matches the board's USB-UART pins, typically MIO).
     2. **DDR Configuration:** Configure according to the ZUBoard 1CG LPDDR4 memory specifications (or apply the Avnet ZUBoard 1CG board preset if installed).
     3. **PS-PL Configuration:** Expand **PS-PL Interfaces** → **Master Interface** and **disable `AXI HPM0 LPD`** (for this initial PS-only test).
     4. **Clock Configuration:** Expand **Output Clocks** → **Low Power Domain Clocks** → **PL Fabric Clocks** and **disable `pl_clk0`**.
   - Click **OK**.
5. **Validate and Generate Bitstream:**
   - Press **`F6`** (or click **Validate Design**) to confirm there are no critical warnings or errors.
   - In the **Sources** tab, right-click `design_1.bd` and select **Create HDL Wrapper** (let Vivado manage the wrapper automatically).
   - Click **Generate Bitstream** in the Flow Navigator.

*(Note: Optional legacy `xsdb` low-level memory read/write test commands (`psu_init`, `mwr`, `mrd`) can also be run via the Xilinx System Debugger console if desired, though Vitis handles PS initialisation automatically in Part C.)*

---

### Part B: Adding FPGA Programmable Logic (Push-Button SW3 & RGB LED)

Now extend `design_1` so that pressing push-button **SW3** passes through a PL logic chain (`AND` gate + `NOT` gate), illuminates the on-board Blue RGB LED, and reports its state to the ARM processor via an **AXI GPIO** peripheral:

1. **Re-enable PS-PL Interface & Fabric Clock:**
   - Double-click `zynq_ultra_ps_e_0` in your block design:
     - **PS-PL Configuration** → **PS-PL Interfaces** → **Master Interface**: Tick **`AXI HPM0 LPD`**.
     - **Clock Configuration** → **Output Clocks** → **Low Power Domain Clocks** → **PL Fabric Clocks**: Enable **`pl_clk0`** and set the **Requested Frequency** to **`100` MHz**.
   - Click **OK**.
2. **Add the AXI GPIO IP:**
   - Click **`+`** (Add IP), search for **`AXI GPIO`**, and add `axi_gpio_0`.
   - Double-click `axi_gpio_0`:
     - Untick **Enable Dual Channel**.
     - Set **GPIO Width** to **`1`**.
     - Click **OK**.
3. **Run Connection Automation:**
   - Click **Run Connection Automation** at the top of the diagram banner, tick **All Automation**, and click **OK**.
   - Select and **delete** the auto-generated external interface port `gpio_rtl` (we will connect `gpio_io_i` internally to our logic gate instead).
4. **Build the PL Logic Gate Chain (`AND` → `NOT`):**
   - Click **`+`**, search for **`Utility Vector Logic`**, and add it (`util_vector_logic_0`). Configure it as an **`and`** gate with **C_SIZE** set to **`1`**.
   - Add a **`Constant`** IP (`xlconstant_0`), set its value to **`1`** (`Const Val = 1`, `Const Width = 1`), and connect its `dout` pin to **`Op1`** of the `AND` gate.
   - Right-click the **`Op2`** pin of the `AND` gate and select **Make External** (this creates external port `Op2_0[0:0]`, which will map to push-button **SW3**).
   - Add a second **`Utility Vector Logic`** IP (`util_vector_logic_1`), configure it as a **`not`** gate with **C_SIZE** set to **`1`**, and connect the `AND` gate's **`Res`** output to the `NOT` gate's **`Op1`** input.
5. **Connect the Logic Output to AXI GPIO and External LED Port:**
   - Expand the `GPIO` interface pin (`+` icon) on `axi_gpio_0` to reveal **`gpio_io_i[0:0]`**. Connect the `NOT` gate's **`Res`** output to **`gpio_io_i`** so the ARM Cortex-A53 can read the logic state.
   - Right-click an empty area of the Block Design canvas and select **Create Port...**:
     - **Port name:** `Res_0`
     - **Direction:** `Output`
     - **Type:** `Data` (1-bit)
     - Click **OK**, and connect the `NOT` gate's **`Res`** pin to **`Res_0`**.
6. **Add Physical Pin Constraints (`.xdc`):**
   - PL GPIO signals cannot route through PS MIO pins; they must be assigned to HD/HP PL bank pins via a constraints file.
   - In **Sources**, right-click **Constraints** → **Add Sources** → **Add or create constraints** → **Create File** (e.g. `zuboard_1cg_pl_gpio.xdc`).
   - Add the contents of [`03-bare-metal-pl-gpio/constraints/zuboard_1cg_pl_gpio.xdc`](03-bare-metal-pl-gpio/constraints/zuboard_1cg_pl_gpio.xdc):
     ```tcl
     #=====================================================================
     # ZUBoard 1CG - Fixed Pin Constraints
     #=====================================================================

     # Op2_0[0] connected to Push-Button SW3
     set_property PACKAGE_PIN A8 [get_ports {Op2_0[0]}]
     set_property IOSTANDARD LVCMOS18 [get_ports {Op2_0[0]}]

     # Res_0[0] connected to RGB LED (Blue)
     set_property PACKAGE_PIN B5 [get_ports {Res_0[0]}]
     set_property IOSTANDARD LVCMOS18 [get_ports {Res_0[0]}]
     ```
7. **Validate & Regenerate Bitstream:**
   - Press **`F6`** to validate the updated block design, save (`Ctrl+S`), and click **Generate Bitstream**.

---

### Part C: Building the Bare-Metal Application in Vitis

1. **Export Hardware (`.xsa`):**
   - In Vivado, navigate to **File** → **Export** → **Export Hardware...**.
   - Select **Include bitstream**, choose an export path, and click **Finish**.
2. **Create a Vitis Platform Component:**
   - Open **Vitis** and choose a workspace directory.
   - Go to **File** → **New Component** → **Platform**.
   - Select **Hardware Design** and browse to the `.xsa` file you just exported.
   - Set **Operating System** to **`standalone`** and **Processor** to **`psu_cortexa53_0`**, then click **Finish** and build the platform.
   - *(Tip: If you modify the Vivado hardware later, open `vitis-comp.json` in your platform component and click **Update Hardware Specification** to reload the updated `.xsa`.)*
3. **Create a Vitis Application Component:**
   - Go to **File** → **New Component** → **Application**.
   - Select the platform you just created, choose the **`standalone_psu_cortexa53_0`** domain, and pick the **Empty Application** template.
4. **Add C++ Source Code & Configure CMake:**
   - Create `main.cpp` inside the application's `src/` folder using the code in [`03-bare-metal-pl-gpio/src/main.cpp`](03-bare-metal-pl-gpio/src/main.cpp).
   - Because Vitis Unified IDE uses CMake and defaults to C source files, update your `UserConfig.cmake` (see [`03-bare-metal-pl-gpio/src/UserConfig.cmake`](03-bare-metal-pl-gpio/src/UserConfig.cmake)) so CMake compiles `main.cpp`:
     ```cmake
     set(USER_COMPILE_SOURCES "main.cpp")
     ```
   - Ensure `CMakeLists.txt` includes the appropriate source and header search directories if you add custom headers.
5. **Build and Run on Hardware:**
   - Select your application component in the **FLOW** panel and click **Build**.
   - Open your serial terminal on the host PC:
     ```bash
     sudo picocom -b 115200 /dev/ttyUSB1
     ```
   - In the Vitis **FLOW** panel, click **Run** (this automatically programs the FPGA bitstream, initialises the PS, and launches `main.cpp` over JTAG).
   - Press and release **SW3** on the ZUBoard 1CG: the Blue RGB LED will toggle, and the UART terminal will print the live state every 500 ms.

---

## Module 4: Hands-On with the Dual-Camera Module (PetaLinux 2024.2)

This advanced module builds a complete custom hardware platform and PetaLinux 2024.2 image supporting the Avnet Dual-Camera Mezzanine (dual AR0144 image sensors).

### Part A: Generating the Vivado Hardware Design (XSA)

1. **Prerequisites:** Install **Vivado 2024.2** and **PetaLinux 2024.2** on your host machine (for example, under `/home/<user>/apps/`).
2. **Clone Avnet Repositories:**
   Create a clean working directory (e.g. `~/dual_camera`) and clone the three official Avnet repositories, checking out the `2024.2` branch for `hdl` and `petalinux`:
   ```bash
   mkdir -p ~/dual_camera && cd ~/dual_camera

   git clone https://github.com/Avnet/bdf.git bdf
   git clone -b 2024.2 https://github.com/Avnet/hdl.git hdl
   git clone -b 2024.2 https://github.com/Avnet/petalinux.git petalinux
   ```
   > **Important:** Ensure the three folders inside your project directory are named **exact match**: `bdf`, `hdl`, and `petalinux`.
3. **Run the Avnet Vivado Build Script:**
   - Source the Vivado 2024.2 environment and launch Vivado:
     ```bash
     source /home/<user>/apps/Vivado/2024.2/settings64.sh
     vivado &
     ```
   - In the Vivado Tcl Console (or via `vivado -mode batch -source ...`), navigate to the `hdl/scripts` directory and execute **`make_zub1cg_sbc_dualcam.tcl`**:
     ```tcl
     cd ~/dual_camera/hdl/scripts
     source make_zub1cg_sbc_dualcam.tcl
     ```
   - *Memory Note:* Synthesis and implementation of the dual-camera MIPI CSI-2 pipeline can require significant RAM. Add a Linux swap file (e.g. 16 GB) before running the script if your system runs low on memory.
   - This Tcl script automatically builds the block design, runs synthesis and implementation, generates the bitstream, and exports the `.xsa` hardware handoff file.

---

### Part B: Building the PetaLinux Image Inside Docker

Because PetaLinux 2024.2 officially supports Ubuntu 22.04 LTS, running the build inside an Ubuntu 22.04 Docker container prevents host library conflicts on Ubuntu 24.04.

1. **Launch an Ubuntu 22.04 Container:**
   Install Docker on your host, pull `ubuntu:22.04`, and mount both your `dual_camera` workspace and your Xilinx tools installation directory:
   ```bash
   docker pull ubuntu:22.04

   docker run -it --name petalinux_env      -v /home/$USER/dual_camera:/workspace      -v /home/$USER/apps:/home/$USER/apps      ubuntu:22.04
   ```
2. **Install PetaLinux Dependencies & Configure Non-Root User:**
   Inside the container (or by running [`04-dual-camera-petalinux/scripts/setup_docker_petalinux.sh`](04-dual-camera-petalinux/scripts/setup_docker_petalinux.sh)), install required packages, set `en_US.UTF-8` locales, and create a `builder` user (PetaLinux refuses to run as `root`):
   ```bash
   apt-get update && apt-get install -y      sudo locales build-essential gawk gcc git make net-tools ncurses-dev      libncurses5-dev zlib1g-dev libssl-dev flex bison libselinux1 gnupg wget      diffstat chrpath socat xterm autoconf libtool texinfo unzip tar cpio      python3 python3-pip python3-pexpect xz-utils debianutils iputils-ping      python3-git python3-jinja2 libegl1-mesa libsdl1.2-dev x11-utils      libtinfo5 dnsutils rsync bc gcc-multilib lsb-release nano

   locale-gen en_US.UTF-8
   update-locale LANG=en_US.UTF-8

   useradd -ms /bin/bash builder
   echo "builder ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers
   su - builder
   ```
3. **Source Toolchains & Build the PetaLinux Project:**
   As the `builder` user inside the container:
   ```bash
   source /home/<user>/apps/Vivado/2024.2/settings64.sh
   source /home/<user>/apps/petalinux/settings.sh

   sudo mkdir -p /build
   sudo chown -R builder:builder /build

   cd /workspace/petalinux/scripts
   ./make_zub1cg_sbc_dualcam.sh
   ```
4. **Resolving Yocto SState / SPDX Build Errors (If Encountered):**
   If the build fails with an SState or SPDX generation error, edit `local.conf` and disable SPDX:
   ```bash
   nano /workspace/petalinux/projects/zub1cg_sbc_dualcam_2024_2/build/conf/local.conf
   ```
   Add or update the following line:
   ```text
   SPDX_ENABLED = "0"
   ```
   Save the file and re-run `./make_zub1cg_sbc_dualcam.sh`.
5. **Flash the Generated Image to MicroSD:**
   Once compilation succeeds, the bootable WIC image is located at:
   ```text
   ~/dual_camera/petalinux/projects/zub1cg_sbc_dualcam_2024_2/images/linux/rootfs.wic
   ```
   Flash `rootfs.wic` onto your 32 GB MicroSD card.

---

### Part C: Booting & Running the Dual-Camera Passthrough Demo

1. **Boot the Dual-Camera Image:**
   - Attach the Avnet Dual-Camera module to the ZUBoard 1CG SYZYGY/expansion connector while powered off.
   - Insert the flashed MicroSD card and set **SW2** to **`0101`** (SD Card boot mode).
   - Press **SW7** to power on the board and connect via `picocom` (`/dev/ttyUSB1`, `115200` baud).
   - Log in as **`root`** (no password is set by default).
2. **Verify Video & Media Device Nodes:**
   Confirm that the Linux kernel registered the MIPI CSI-2 media pipelines and V4L2 video capture nodes:
   ```bash
   ls /dev/media*
   ls /dev/video*
   ```
   You should see `/dev/media0`, `/dev/video0`, etc. listed.
3. **Configure Networking & Launch X11 Passthrough:**
   - On the ZUBoard serial console, assign a static IP address on the same subnet as your host PC:
     ```bash
     ifconfig end0 192.168.2.100 netmask 255.255.255.0
     ```
   - Open a **new terminal** on your host PC (with an active X11 display server) and connect via SSH with X11 forwarding enabled (`-X`):
     ```bash
     ssh -X root@192.168.2.100
     ```
   - Run the Avnet dual-camera Python passthrough example to stream live video from both AR0144 sensors back to your PC display:
     ```bash
     python3 /home/root/avnet_dualcam_python_examples/avnet_dualcam_passthrough.py --sensor ar0144 --mode dual
     ```

---

## Troubleshooting & Helpful Shortcuts

* **No Output in `picocom`:** Verify whether the USB-UART enumerates as `/dev/ttyUSB0`, `/dev/ttyUSB1`, or `/dev/ttyUSB2` (`ls /dev/ttyUSB*`), and press **SW6** (PS Reset) after opening `picocom`.
* **`picocom` Keyboard Shortcuts:**
  * `Ctrl+A` then `Ctrl+X`: Exit `picocom` cleanly.
  * `Ctrl+A` then `Ctrl+U`: Increase baud rate.
* **Vivado Out-of-Memory During Dual-Camera Synthesis:** Create a temporary swap file on Ubuntu before launching Vivado:
  ```bash
  sudo fallocate -l 16G /swapfile
  sudo chmod 600 /swapfile
  sudo mkswap /swapfile
  sudo swapon /swapfile
  ```
