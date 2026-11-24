# Changelog

## 2.1.0 (Beta) – 2026-11-24

Alle Punkte **(Beta)**: übersetzt und geprüft, aber noch nicht an echter Hardware getestet.
All items **(Beta)**: compiled and reviewed, not yet tested on real hardware.

### Neu / New
- WS2811/WS2812-Strip ist jetzt ein **eigenes Modul** (`firmware/strip.c`) an einem zusätzlichen Pin mit zwei
  Betriebsarten: **Direkt** – der PC setzt die Pixel über die USB-Befehle `0xB0–0xB3` (`STRIP_INFO`, `STRIP_SET`,
  `STRIP_FILL`, `STRIP_SHOW`; die Antwort auf `STRIP_SHOW` kommt, sobald der Strip geschrieben ist); **Spiegel** –
  bis zu 32 Einträge „Ausgang → Pixel ab/Anzahl → Farbe“, die Helligkeit folgt dem LED-Wiz-Ausgang (DOF-tauglich).
  The LED strip is now a **separate module** on an extra pin with two modes: **direct** (host sets pixels via USB
  commands `0xB0–0xB3`) and **mirror** (up to 32 entries output → pixels → color following the LED-Wiz output).
- Konfigurations-Tool: eigener Reiter „LED-Strip“ mit Spiegel-Tabelle (Farbwähler) und Testknöpfen für den
  Direkt-Modus (Füllen, Pixel setzen, Lauflicht). / Tool: new "LED strip" tab with mirror table and direct-mode tests.

### Verbessert / Improved
- Die 32 LED-Wiz-Ausgänge sind wieder **reine GPIO-Ausgänge wie im Original** (`outputs.c` ohne Strip-Code);
  der Strip liest nur noch die Helligkeiten über `outputs_get_level()`.
  The 32 LED-Wiz outputs are plain GPIO outputs again like the original; the strip only reads their levels.
- Konfigurationslayout Version 2 (`config.h`): Ausgang = Pin + Flags, neuer Block `mirror[]`, `ws.mode`;
  Protokollversion 2, Firmware 2.1. Alte Konfigurationen im EEPROM werden erkannt und durch die Werksbelegung ersetzt.
  Configuration layout version 2; old EEPROM contents are detected and replaced by the board defaults.

## 2.0.0 (Beta) – 2026-11-24

Alle Punkte **(Beta)**: übersetzt und geprüft, aber noch nicht an echter Hardware getestet.
All items **(Beta)**: compiled and reviewed, not yet tested on real hardware.

### Neu / New
- Pin-Belegung zur Laufzeit: Ausgänge, Eingänge und Analog-Achsen werden per USB konfiguriert und im EEPROM
  gespeichert – ohne Neukompilieren (`firmware/config.h`, `firmware/proto.h`).
  Runtime pin assignment: outputs, inputs and analog axes are configured via USB and stored in EEPROM.
- WS2811/WS2812-Treiber (Bit-Bang, 8/16 MHz). In 2.0.0 noch als Ausgangstyp, seit 2.1.0 eigenes Modul (siehe oben).
  WS2811/WS2812 driver (bit-bang, 8/16 MHz). Output type in 2.0.0, separate module since 2.1.0 (see above).
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
- `ledwiz.dll`: Hänger bzw. Absturz bei `FreeLibrary` kurz nach `LoadLibrary` – der I/O-Thread wurde in `DllMain`
  erzeugt und konnte wegen des Loader-Locks noch nicht laufen. Er startet jetzt beim ersten API-Aufruf.
- `ledwiz.dll`/Tools: bauen jetzt auch mit Clang/MinGW (Aufrufkonventionen, Signaturen von `LWZ_RAWWRITE`/`LWZ_RAWREAD`
  passend zum Header, saubere `GetProcAddress`-Casts).
- `ledwiz.dll`: Event-Handle `heevent` wurde nie geschlossen; Geräteliste beim Entfernen falsch gepflegt;
  `LWZ_SET_NOTIFY_EX` räumte vorherige Registrierung nicht auf.
- Firmware: ADC-Wandlung startet erst, wenn Achsen konfiguriert sind; Pins werden beim Umkonfigurieren freigegeben.

## 1.x (Upstream, 2013–2023)
Siehe Git-Verlauf von `cithraidt/lwcloneu2`. / See the git history of `cithraidt/lwcloneu2`.
