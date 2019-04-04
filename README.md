[![pipeline status](https://gitlab.com/VictorLamoine/libopencm3_freertos_bluepill_blink/badges/master/pipeline.svg)](https://gitlab.com/VictorLamoine/libopencm3_freertos_bluepill_blink/commits/master)

# Dependencies
```bash
sudo apt install -y cmake python
```

You need [texane/stlink](https://github.com/texane/stlink/) installed if you want to upload via a ST-Link V2.

## :warning: Compiler
- Do NOT use Kubuntu 18.04 ARM none eabi compiler: `arm-none-eabi-gcc (15:6.3.1+svn253039-1build1) 6.3.1 20170620` (it contains a bug with `memcpy`)
- Instead download and install the latest [GNU Embedded Toolchain for Arm](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads)

# Building
```bash
mkdir -p libopencm3_freertos_bluepill_blink/build
cd libopencm3_freertos_bluepill_blink
git clone --recurse-submodules https://gitlab.com/VictorLamoine/libopencm3_freertos_bluepill_blink.git src
cd build
cmake ../src
make -j4
```

To upload, make sure the Blue-Pill / ST-Link are connected and use one of the two commands:
```bash
make -j4 blink_c.bin_upload
make -j4 blink_cpp.bin_upload
```
