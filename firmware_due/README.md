# LWCloneU2 für Arduino Due / LWCloneU2 for Arduino Due *(Beta)*

Eigene Firmware für den **Arduino Due** (SAM3X8E, ARM Cortex-M3, 84 MHz, **3,3 V**) am **nativen USB-Port**. Sie
verhält sich nach außen genau wie die AVR-Boards: LED-Wiz-HID-Gerät (VID 0xFAFA), Konfigurations-Befehle, Panel
(Tastatur, 4 Joysticks, Maus), WS2811/WS2812-Strip, COM-Port für DOF. Konfigurations-Tool und DOF brauchen nichts
Neues. **Ohne Hardware nicht getestet.**

Own firmware for the **Arduino Due** (SAM3X8E, ARM Cortex-M3, 84 MHz, **3.3 V**) on the **native USB port**. From the
outside it behaves exactly like the AVR boards: LED-Wiz HID device (VID 0xFAFA), configuration commands, panel
(keyboard, 4 joysticks, mouse), WS2811/WS2812 strip, COM port for DOF. **Untested on hardware.**

## Bauen / build

PlatformIO (CLI oder VS-Code-Erweiterung); die Plattform `atmelsam` und der Arduino-SAM-Core werden beim ersten Bauen
geladen. / PlatformIO; platform and core are downloaded on the first build.

```sh
cd firmware_due
pio run                  # → .pio/build/due/firmware.bin, Protokoll / log: build.log
pio run -t upload        # nativer USB-Port: 1200-Baud-Trick + bossac / native port
cd ../firmware && ./publish.sh   # kopiert die .bin als release/arduino_due.bin für den Online-Flasher / copies for the web flasher
```

Flashen ohne PlatformIO: [Online-Flasher](https://cyclemat.github.io/lwcloneu2/flash.html) (Board „Arduino Due“) oder
`bossac -p COMx -e -w -v -b arduino_due.bin -R` nach dem 1200-Baud-Trick bzw. ERASE + RESET.

## Aufbau / layout

Die Logik kommt **unverändert aus `../firmware`** (`shared.py` kompiliert `proto.c`, `config.c`, `outputs.c`,
`panel.c`, `strip.c`, `strip_serial.c` mit); hier liegt nur die Hardware-Seite.
The logic is compiled **unchanged from `../firmware`**; only the hardware side lives here.

| Datei / file | Inhalt / content |
|---|---|
| `src/hwconfig.h` | Board-Einstellungen: ID 6, 74 Pins, 255 Pixel, Werksbelegung D22–D53, Schnittstellen |
| `src/main.cpp` | Hauptschleife (Reports → `proto.c`, Antworten, Panel, Strip, DOF-Port), SETID/DFU, LED-Wiz-ID |
| `src/usb_lwz.cpp` | PluggableUSB-Modul: drei HID-Schnittstellen (Misc, Panel, LED-Wiz) und Gerätedeskriptor mit LED-Wiz-VID/PID zur Laufzeit; CDC vom Core |
| `src/pins_due.cpp` | Pin-Tabelle aus `g_APinDescription` des Cores, PIO-Zugriff (SODR/CODR/PDSR) |
| `src/hw_due.cpp` | Soft-PWM-Timer TC3 (200 µs), Freilauf-ADC (12 Bit → 10 Bit), EEPROM-Ersatz (1 KB am Ende von Flash-Bank 1, EEFC), Neustart / SAM-BA |
| `src/ws2812_due.cpp` | Bit-Bang 800 kHz mit Cortex-M3-Zyklenzähler (DWT), Interrupts kurz gesperrt |
| `src/compat/` | Ersatz-Header für `<util/atomic.h>`, `<avr/eeprom.h>`, `<avr/io.h>` |
| `boards/lwc_due.json` | PlatformIO-Board (Due, nativer Port, VID/PID LED-Wiz) |

## Hardware-Hinweise / hardware notes

- **3,3 V!** Ausgänge liefern höchstens 3,3 V, Eingänge vertragen keine 5 V. Für 5-V-Treiberplatinen und den
  Daten-Eingang von WS2812-Strips meist ein Pegelwandler (z. B. 74HCT125). / **3.3 V!** Use a level shifter for 5 V logic.
- Pin-Index = Arduino-Pin-Nummer: 0–53 digital, 54–65 = A0–A11 (ADC), 66/67 DAC0/1, 68/69 CANRX/TX, 70/71 SDA1/SCL1,
  72/73 RX-/TX-LED. **D0/D1 sind gesperrt** (UART zum ATmega16U2 des Programming-Ports).
- Konfiguration und LED-Wiz-ID liegen im letzten Kilobyte von Flash-Bank 1 (0x000FFC00). Der Online-Flasher schreibt nur
  die Firmware-Seiten, die Einstellungen bleiben. Die Taste **ERASE** löscht alles (auch die Einstellungen).
- Befehl 66 („Bootloader“) löscht das Boot-Flag GPNVM1 und startet neu → ROM-Bootloader SAM-BA („Bossa Program Port“).
- USB läuft mit High-Speed; Reihenfolge: IF0/1 CDC (DOF-Port), IF2 Misc, IF3 Panel, IF4 LED-Wiz. DOF öffnet den Port ohne
  DTR, deshalb werden Antworten direkt über den CDC-Endpunkt geschickt.
- Der Strip blockiert Interrupts ~30 µs je Pixel (255 Pixel ≈ 7,7 ms); die Soft-PWM setzt in dieser Zeit aus.

Lizenz / license: GPL v2 oder neuer (wie das Hauptprojekt). Arduino-SAM-Core: LGPL 2.1 (wird nur gelinkt).
