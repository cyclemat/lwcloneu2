#!/bin/sh
# LWCloneU2 – alle Firmware-Ziele ohne make bauen (nur avr-gcc nötig).
# LWCloneU2 – build all firmware targets without make (only avr-gcc needed).
#
#   ./build.sh            alle Boards / all boards
#   ./build.sh leonardo   nur ein Board / one board (leonardo, promicro, breakout_32u2, mega2560, uno)
#
# Sucht avr-gcc im PATH, sonst in der PlatformIO-Toolchain (~/.platformio/packages/toolchain-atmelavr).
# Ergebnis: build/<ziel>.hex  (+ .elf, .map)  und build.log; danach ./publish.sh für docs/firmware
# Looks for avr-gcc in PATH, else in the PlatformIO toolchain. Output: build/<target>.hex and build.log

set -e
cd "$(dirname "$0")"

if command -v avr-gcc >/dev/null 2>&1; then
	BIN=""
elif [ -x "$HOME/.platformio/packages/toolchain-atmelavr/bin/avr-gcc" ] || [ -x "$HOME/.platformio/packages/toolchain-atmelavr/bin/avr-gcc.exe" ]; then
	BIN="$HOME/.platformio/packages/toolchain-atmelavr/bin/"
else
	echo "avr-gcc nicht gefunden / not found (PATH oder PlatformIO-Toolchain)" >&2
	exit 1
fi

GCC="${BIN}avr-gcc"
OBJCOPY="${BIN}avr-objcopy"
SIZE="${BIN}avr-size"

LUFA="lufa/LUFA/Drivers/USB"
LUFA_SRC="$LUFA/Core/AVR8/USBController_AVR8.c $LUFA/Core/AVR8/USBInterrupt_AVR8.c $LUFA/Core/AVR8/Device_AVR8.c
          $LUFA/Core/AVR8/Endpoint_AVR8.c $LUFA/Core/AVR8/EndpointStream_AVR8.c $LUFA/Core/AVR8/Host_AVR8.c
          $LUFA/Core/AVR8/Pipe_AVR8.c $LUFA/Core/AVR8/PipeStream_AVR8.c $LUFA/Core/ConfigDescriptors.c
          $LUFA/Core/Events.c $LUFA/Core/USBTask.c $LUFA/Core/DeviceStandardReq.c $LUFA/Core/HostStandardReq.c
          $LUFA/Class/Common/HIDParser.c $LUFA/Class/Device/CDCClassDevice.c"

COMMON="comm.c queue.c clock.c"
APP="config.c pins.c outputs.c ws2812.c strip.c strip_serial.c panel.c proto.c"

CFLAGS="-Os -std=gnu99 -Wall -Wstrict-prototypes -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
        -fno-strict-aliasing -ffunction-sections -fdata-sections -mrelax -DARCH=ARCH_AVR8 -DBOARD=BOARD_USER
        -DUSE_LUFA_CONFIG_HEADER -I. -I./lufa"

mkdir -p build
: > build.log

# build <name> <mcu> <f_cpu> <include-dir> <sources...>
build() {
	name=$1; mcu=$2; fcpu=$3; inc=$4; shift 4
	echo "== $name ($mcu, $fcpu Hz)" | tee -a build.log
	"$GCC" -mmcu=$mcu -DF_CPU=${fcpu}UL -DF_USB=${fcpu}UL -I"$inc" $CFLAGS \
		-Wl,--gc-sections -Wl,--relax -Wl,-Map=build/$name.map \
		-o build/$name.elf "$@" 2>&1 | grep -v -E "discards 'volatile'|return &f->buf|^\s*\^|In function 'chunk_" | tee -a build.log || true
	[ -f build/$name.elf ] || { echo "FEHLER / ERROR: $name" | tee -a build.log; exit 1; }
	"$OBJCOPY" -O ihex -R .eeprom build/$name.elf build/$name.hex
	"$SIZE" -C --mcu=$mcu build/$name.elf 2>/dev/null | grep -E "Program|Data:" | tee -a build.log || "$SIZE" build/$name.elf | tee -a build.log
}

want() { [ $# -eq 0 ] && return 0; for w in "$@"; do [ "$w" = "$1" ] && return 0; done; return 1; }
sel="$*"
hit() { [ -z "$sel" ] && return 0; case " $sel " in *" $1 "*) return 0;; esac; return 1; }

hit leonardo      && build arduino_leonardo        atmega32u4  16000000 ./arduino_leonardo        main_usb.c descriptors.c $COMMON $APP $LUFA_SRC
hit promicro      && build arduino_promicro        atmega32u4  16000000 ./arduino_promicro        main_usb.c descriptors.c $COMMON $APP $LUFA_SRC
hit breakout_32u2 && build breakout_32u2           atmega32u2   8000000 ./breakout_32u2           main_usb.c descriptors.c $COMMON $APP $LUFA_SRC
hit mega2560      && build arduino_mega2560__m16u2 atmega16u2  16000000 ./arduino_mega2560/m16u2  main_usb.c descriptors.c $COMMON $LUFA_SRC
hit mega2560      && build arduino_mega2560__m2560 atmega2560  16000000 ./arduino_mega2560/m2560  main_led.c $COMMON $APP
hit uno           && build arduino_uno__m8u2       atmega8u2   16000000 ./arduino_uno/m8u2        main_usb.c descriptors.c $COMMON $LUFA_SRC
hit uno           && build arduino_uno__m16u2      atmega16u2  16000000 ./arduino_uno/m8u2        main_usb.c descriptors.c $COMMON $LUFA_SRC
hit uno           && build arduino_uno__m328       atmega328p  16000000 ./arduino_uno/m328        main_led.c $COMMON $APP

echo "fertig / done: $(ls build/*.hex | wc -l) hex-Dateien in firmware/build/" | tee -a build.log
