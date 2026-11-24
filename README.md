# LWCloneU2

**Deutsch** · [English below](#english)

LED-Wiz-kompatibler USB-Ausgangs-Controller (32 Ausgänge, Soft-PWM, WS2811/WS2812) **plus**
Eingabe-Encoder (Tastatur, bis zu 4 Joysticks mit analogen Achsen, Maus) für Arduino-Boards mit
Atmel-AVR-USB-Controller. Gedacht für MAME-/Pinball-Cabinets: ein Board für Beleuchtung und Bedienelemente.

Firmware **v2** – Fork von [cithraidt/lwcloneu2](https://github.com/cithraidt/lwcloneu2) (GPLv2).
Stand: **Beta** – übersetzt und geprüft, aber noch nicht an echter Hardware getestet.

## Was ist neu in v2

- **Pins frei belegbar, ohne Neukompilieren.** Welcher Pin Ausgang, Taster oder Analog-Achse ist, steht im EEPROM
  und wird per USB gesetzt – mit dem Konfigurations-Tool im Browser, auch im laufenden Betrieb („Anwenden“).
- **WS2811/WS2812-Strip:** ein Daten-Pin, jeder der 32 LED-Wiz-Ausgänge kann einen Farbkanal (Rot/Grün/Blau/Weiß)
  eines Pixelbereichs steuern. LED-Wiz-Software (DOF, LedBlinky, …) sieht weiterhin nur 32 Ausgänge.
- **Analoge Joystick-Achsen** (bis 8) mit Kalibrierung (Min/Mitte/Max), **Totzone** und Invertierung.
- **4 Joysticks** mit je 4 Achsen (X/Y/Z/Rz) und 8 Tasten, Tastatur, Medientasten, Maus (Encoder/Trackball).
- `ledwiz.dll`-Ersatz mit asynchronem USB-I/O (blockiert die Anwendung nicht), Fehler beim Prozessende behoben.
- Baut überall: LUFA liegt im Repo, CMake für Windows, GitHub Actions.

## Unterstützte Boards

| Board | MCU | Pins | Analog-Achsen | Hinweis |
|---|---|---|---|---|
| Arduino Leonardo | ATmega32U4 | D0–D17, A0–A5, TXLED | ja (12 ADC-Pins) | empfohlen |
| Pro Micro / Arduino Micro | ATmega32U4 | wie Leonardo, weniger herausgeführt | ja | |
| Arduino Mega 2560 | ATmega2560 + ATmega16U2 | D2–D53, A0–A15 | ja (16) | **beide** MCUs flashen |
| Arduino Uno R2/R3 | ATmega328P + ATmega8U2/16U2 | D2–D13, A0–A5 | ja (6) | beide MCUs flashen |
| Breakout ATmega32U2 | ATmega32U2 | 21 Pins | nein | 1 KB RAM: 16 Eingänge, 8 Pixel |

Beim Mega/Uno übernimmt der kleine USB-Chip (16U2/8U2) nur die USB-Seite und reicht alles über die interne UART an
den Haupt-MCU weiter; Konfiguration und Pins gehören zum Haupt-MCU.

## Schnellstart

1. **Firmware flashen** (fertige `.hex` aus den GitHub-Actions-Artefakten oder selbst bauen, siehe unten).
   - Leonardo/Micro: Reset drücken, dann innerhalb von ~8 s
     `avrdude -c avr109 -p atmega32u4 -P COMx -U flash:w:arduino_leonardo.hex:i`
   - Mega 2560: `avrdude -c wiring -p atmega2560 -P COMx -b 115200 -U flash:w:arduino_mega2560__m2560.hex:i`,
     danach den 16U2 per DFU (Atmel FLIP / `dfu-programmer`) mit `arduino_mega2560__m16u2.hex`.
   - Uno: `avrdude -c arduino -p m328p -P COMx -U flash:w:arduino_uno__m328.hex:i`, den 8U2/16U2 per DFU mit
     `arduino_uno__m8u2.hex`.
2. **Konfigurieren:** `tools/lwcconfig-web/index.html` in Microsoft Edge oder Google Chrome öffnen (Datei lokal,
   keine Installation), „Verbinden“, Gerät wählen. Pins zuweisen, „Anwenden“ zum Ausprobieren, „Speichern“ zum
   dauerhaften Ablegen im EEPROM. Beim ersten Start gilt die Werksbelegung des Boards (siehe `pinmap.h`).
3. **LED-Wiz-Software:** `win32/bin/x86/ledwiz.dll` (32 Bit) bzw. `x64/ledwiz.dll` neben die Anwendung legen oder
   deren mitgelieferte `ledwiz.dll` ersetzen. Die LED-Wiz-ID (1–16) wird im Tool unter „Gerät“ gesetzt.

## Konfigurations-Tool

- **Ausgänge:** Typ (GPIO/WS2812), Pin, invertiert; für WS2812 Pixel-Bereich und Farbkanal. „Test“ schaltet den
  Ausgang zum Ausprobieren ein.
- **Eingänge:** Pin → Funktion (Taste, Modifier, Medien, Maustaste, Joystick-Richtung/-Taste, Shift, Maus-Encoder),
  zweite Funktion bei gedrücktem Shift, aktiv high/low, Live-Anzeige.
- **Achsen:** Pin (nur ADC-fähige), Joystick, Achse, Min/Mitte/Max aus dem Live-Wert übernehmen, Totzone in %,
  Vorschau des Ausgabewerts (–127…127).
- **Gerät:** LED-Wiz-ID, JSON-Export/Import, Werkseinstellungen, Bootloader starten.

Das Tool nutzt WebHID; es spricht über dieselbe USB-HID-Verbindung wie die LED-Wiz-Software – kein COM-Port,
kein Treiber. Protokoll: [`firmware/proto.h`](firmware/proto.h), Datenlayout: [`firmware/config.h`](firmware/config.h).

## Selbst bauen

### Firmware
Benötigt `avr-gcc`, `avr-libc`, `make` (Linux: `apt install gcc-avr avr-libc make`; Windows: z. B. die Toolchain aus
PlatformIO `~/.platformio/packages/toolchain-atmelavr/bin` in den PATH, `make` per `winget install ezwinports.make`,
Aufruf aus der Git-Bash).

```sh
cd firmware
make                      # alle Boards
make -C arduino_leonardo  # ein Board; Ergebnis: arduino_leonardo/arduino_leonardo.hex
```

### Windows-DLL und Tools
```sh
cmake -S win32 -B build-x86 -A Win32 && cmake --build build-x86 --config Release   # MSVC
cmake -S win32 -B build-x64 -A x64   && cmake --build build-x64 --config Release
# oder mit LLVM-MinGW / MinGW-w64:
cmake -S win32 -B build-x86 -G Ninja -DCMAKE_CXX_COMPILER=i686-w64-mingw32-clang++ && cmake --build build-x86
```
Ergebnis in `win32/bin/x86` bzw. `win32/bin/x64`: `ledwiz.dll`, `testapp.exe` (schaltet alle Ausgänge ein),
`lwcconfig.exe` (LED-Wiz-ID per Kommandozeile, Bandbreitenmessung).

## Aufbau des Codes

```
firmware/
  main_usb.c        USB-MCU: LUFA, Reports entgegennehmen, Antworten/Panel-Reports senden (oder UART-Brücke)
  main_led.c        Haupt-MCU hinter einer Brücke (Mega 2560 / Uno)
  proto.c/.h        USB-Befehle: LED-Wiz (SBA/PBA) + Konfiguration 0xA0–0xA7
  config.c/.h       Konfigurationsstruktur, EEPROM, Prüfung
  pins.c/.h         Pin-Index → Port/Bit/ADC je Board (BOARD_PIN_TABLE in <board>/pinmap.h)
  outputs.c/.h      32 Ausgänge: LED-Wiz-Profile, Soft-PWM auf GPIO, Pixel-Rendering
  ws2812.c/.h       Bit-Bang-Treiber 800 kHz (8/16 MHz)
  panel.c/.h        Eingänge: Entprellung, Shift, Encoder, Analog-Achsen, HID-Reports
  descriptors.c/.h  USB-Deskriptoren (Misc, Panel, LED-Wiz)
  <board>/          hwconfig.h (Timer, UART, ADC), pinmap.h (Pin-Tabelle, Werksbelegung), devconfig.h, makefile
  lufa/             LUFA 130901 (gekürzt)
win32/
  driver/           ledwiz.dll (ledwiz.cpp, usbdev.cpp)
  testapp/, lwcconfig/
tools/lwcconfig-web/index.html   Konfigurations-Tool (WebHID)
```

Ein neues Board anlegen: Ordner mit `hwconfig.h` (Timer 0 = Soft-PWM, Timer 1 = Uhr, ADC, UART), `pinmap.h`
(`BOARD_ID`, `BOARD_PIN_TABLE`, `WS2812_MAX_PIXELS`, Werksbelegung), `devconfig.h`, `makefile`; Board-ID in
`pins.h` und Pin-Namen im Tool (`BOARDS`) ergänzen.

## Lizenz
GPL v2 oder neuer (siehe `LICENSE`). LUFA: eigene Lizenz (`firmware/lufa/LUFA/License.txt`).
Eingabe-Logik nach dem V-USB Mame Panel von Andreas Oberdorfer (GPLv2).

---

# English

LED-Wiz compatible USB output controller (32 outputs, soft PWM, WS2811/WS2812) **plus** input encoder
(keyboard, up to 4 joysticks with analog axes, mouse) for Arduino boards with Atmel AVR USB controllers.
Made for MAME/pinball cabinets: one board for lighting and controls.

Firmware **v2** – fork of [cithraidt/lwcloneu2](https://github.com/cithraidt/lwcloneu2) (GPLv2).
Status: **beta** – compiled and reviewed, not yet tested on real hardware.

## New in v2
- **Pins assignable at runtime, no recompiling.** Which pin is an output, a button or an analog axis is stored in
  EEPROM and set via USB with the browser based configuration tool – even while running ("Apply").
- **WS2811/WS2812 strip:** one data pin; each of the 32 LED-Wiz outputs can drive a color channel
  (red/green/blue/white) of a pixel range. LED-Wiz software (DOF, LedBlinky, …) still sees 32 outputs.
- **Analog joystick axes** (up to 8) with calibration (min/center/max), **dead zone** and inversion.
- **4 joysticks** with 4 axes (X/Y/Z/Rz) and 8 buttons each, keyboard, media keys, mouse (encoder/trackball).
- `ledwiz.dll` replacement with asynchronous USB I/O (does not block the application), process exit hang fixed.
- Builds everywhere: LUFA is in the repo, CMake for Windows, GitHub Actions.

## Supported boards
See the table above. On the Mega/Uno the small USB chip (16U2/8U2) only handles USB and forwards everything over
the internal UART to the main MCU, which owns configuration and pins – **both** MCUs have to be flashed.

## Quick start
1. **Flash the firmware** (`.hex` from the GitHub Actions artifacts or build it yourself).
   Leonardo/Micro: press reset, then within ~8 s `avrdude -c avr109 -p atmega32u4 -P COMx -U flash:w:arduino_leonardo.hex:i`.
   Mega 2560: `avrdude -c wiring -p atmega2560 -P COMx -b 115200 -U flash:w:arduino_mega2560__m2560.hex:i`, then
   the 16U2 via DFU (Atmel FLIP / `dfu-programmer`) with `arduino_mega2560__m16u2.hex`.
2. **Configure:** open `tools/lwcconfig-web/index.html` in Microsoft Edge or Google Chrome (local file, no install),
   "Connect", pick the device. Assign pins, "Apply" to try, "Save" to store permanently in EEPROM. On first start the
   board's factory mapping applies (see `pinmap.h`).
3. **LED-Wiz software:** put `win32/bin/x86/ledwiz.dll` (32 bit) or `x64/ledwiz.dll` next to the application or
   replace its bundled `ledwiz.dll`. The LED-Wiz id (1–16) is set in the tool under "Device".

## Configuration tool
Outputs (type, pin, inverted, WS2812 pixel range and channel, test button), inputs (pin → function, shift function,
active high/low, live indicator), axes (ADC pins only, joystick/axis, take min/center/max from the live value, dead
zone in %, output preview), device (LED-Wiz id, JSON export/import, factory defaults, bootloader).
The tool uses WebHID over the same USB HID connection the LED-Wiz software uses – no COM port, no driver.
Protocol: [`firmware/proto.h`](firmware/proto.h), data layout: [`firmware/config.h`](firmware/config.h).

## Building
Firmware: `avr-gcc`, `avr-libc`, `make` → `cd firmware && make` (all boards) or `make -C arduino_leonardo`.
Windows: `cmake -S win32 -B build-x86 -A Win32 && cmake --build build-x86 --config Release` (MSVC) or with
LLVM-MinGW/MinGW-w64 via `-G Ninja -DCMAKE_CXX_COMPILER=...`. Output in `win32/bin/<arch>/`.

## Code layout
See the tree above. To add a board: a folder with `hwconfig.h` (timer 0 = soft PWM, timer 1 = clock, ADC, UART),
`pinmap.h` (`BOARD_ID`, `BOARD_PIN_TABLE`, `WS2812_MAX_PIXELS`, factory defaults), `devconfig.h`, `makefile`; add
the board id to `pins.h` and the pin names to the tool (`BOARDS`).

## License
GPL v2 or later (see `LICENSE`). LUFA: own license (`firmware/lufa/LUFA/License.txt`).
Input logic after the V-USB Mame Panel by Andreas Oberdorfer (GPLv2).
