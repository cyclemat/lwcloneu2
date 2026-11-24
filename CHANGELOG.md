# Changelog

## 2.0.0 (Beta) – 2026-11-24

Alle Punkte **(Beta)**: übersetzt und geprüft, aber noch nicht an echter Hardware getestet.
All items **(Beta)**: compiled and reviewed, not yet tested on real hardware.

### Neu / New
- Pin-Belegung zur Laufzeit: Ausgänge, Eingänge und Analog-Achsen werden per USB konfiguriert und im EEPROM
  gespeichert – ohne Neukompilieren (`firmware/config.h`, `firmware/proto.h`).
  Runtime pin assignment: outputs, inputs and analog axes are configured via USB and stored in EEPROM.
- WS2811/WS2812-Treiber: jeder der 32 LED-Wiz-Ausgänge kann einen Farbkanal (R/G/B/Weiß) eines Pixelbereichs steuern.
  WS2811/WS2812 driver: each of the 32 LED-Wiz outputs can drive a color channel of a pixel range.
- Analoge Joystick-Achsen (bis 8, auf 4 Joysticks mit X/Y/Z/Rz) mit Min/Mitte/Max-Kalibrierung, Totzone und Invertierung.
  Analog joystick axes with calibration, dead zone and inversion.
- Konfigurations-Tool im Browser (WebHID, Edge/Chrome): `tools/lwcconfig-web/index.html`, Deutsch/Englisch, hell/dunkel,
  Live-Anzeige von Tasten und Achsen, Ausgangs-Test, JSON-Export/Import.
- `LWZ_RAWREAD`-Antworten: 64-Byte-Input-Reports auf der LED-Wiz-Schnittstelle (8 Byte Nutzdaten).
- CMake-Build für `ledwiz.dll`, `testapp`, `lwcconfig` (MSVC, MinGW-w64, LLVM-MinGW; x86 und x64).
- LUFA 130901 fest im Repo (`firmware/lufa`), kein Submodul mehr.

### Verbessert / Improved
- Fork auf den Upstream-Stand von 2023 gebracht (Pro Micro, ADC, Overlapped-I/O in der DLL, Bootloader-Start).
- Firmware-Struktur: `pins.c` (Board-Tabellen), `outputs.c` (LED-Wiz-Logik + Soft-PWM), `panel.c` (Eingänge),
  `proto.c` (USB-Befehle), `config.c` (EEPROM) – Board-Dateien enthalten nur noch Hardware-Setup und Pin-Tabelle.
- Joystick-Reports einheitlich: 4 Achsen (int8) + 8 Tasten je Joystick, bis zu 4 Joysticks.
- Alle 32U4-Boards teilen `hwconfig_32u4.h` / `pinmap_32u4.h`.

### Behoben / Fixed
- `ledwiz.dll`: Hänger beim Prozessende – `DllMain` wartete auf den I/O-Thread, der beim Beenden schon beendet war.
- `ledwiz.dll`: Event-Handle `heevent` wurde nie geschlossen; Geräteliste beim Entfernen falsch gepflegt;
  `LWZ_SET_NOTIFY_EX` räumte vorherige Registrierung nicht auf.
- Firmware: ADC-Wandlung startet erst, wenn Achsen konfiguriert sind; Pins werden beim Umkonfigurieren freigegeben.

## 1.x (Upstream, 2013–2023)
Siehe Git-Verlauf von `cithraidt/lwcloneu2`. / See the git history of `cithraidt/lwcloneu2`.
