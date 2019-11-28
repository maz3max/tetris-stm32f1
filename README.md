## WIP: Tetris Board firmware
Thanks to [Victor Lamoine](https://gitlab.com/VictorLamoine/libopencm3_freertos_bluepill_blink) for the amazing CMake skeleton!

## :warning: Compiler
- Do NOT use Kubuntu 18.04 ARM none eabi compiler: `arm-none-eabi-gcc (15:6.3.1+svn253039-1build1) 6.3.1 20170620` (it contains a bug with `memcpy`)
- Instead download and install the latest [GNU Embedded Toolchain for Arm](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads)

## building
- the `bin` folder of the GNU ARM Toolchain has to be in your PATH
- use CMake for building
