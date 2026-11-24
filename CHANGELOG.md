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
- **Online-Flasher** (`docs/flash.html`, GitHub Pages): Firmware direkt aus Edge/Chrome flashen – Web Serial für
  Caterina/AVR109 (Leonardo, Pro Micro, inkl. 1200-Baud-Trick), optiboot/STK500 v1 (Uno 328P) und STK500 v2 (Mega 2560);
  WebUSB für Atmel-DFU (8U2/16U2/32U2). Mit Verify, Chip-Prüfung (Signatur/DFU-PID), eigener Hex-Datei und Knopf
  „Bootloader per USB-HID starten“. Startseite `docs/index.html`, Hex-Dateien + `manifest.json` in `docs/firmware/`
  (`firmware/publish.sh`). **Testfunktion – ohne Hardware nur im Headless-Browser geprüft.**
  **Web flasher** (`docs/flash.html`): flash from Edge/Chrome – Web Serial for Caterina/AVR109, optiboot/STK500 v1,
  STK500 v2; WebUSB for Atmel DFU. Verify, chip check, own hex file, "start bootloader via HID". Untested on hardware.
- Flasher mit eingebauter **Anleitung**: Überblick in fünf Schritten, aufklappbare Abschnitte (Was passiert beim
  Flashen, 8-Sekunden-Bootloader, Uno/Mega mit Zeichnung des ICSP-Headers für die RESET–GND-Brücke, WinUSB per Zadig,
  Linux/macOS, zurück zum Arduino, Fehlersuche-Tabelle) und Weiterleitung zum Konfigurations-Tool nach Erfolg.
  Flasher with built-in **guide**: five-step overview, collapsible sections (what happens, 8-second bootloader,
  Uno/Mega with ICSP header drawing, WinUSB via Zadig, Linux/macOS, back to Arduino, troubleshooting table), link to the
  configuration tool after success.
- Webseite unter <https://cyclemat.github.io/lwcloneu2/> (GitHub Pages aus `docs/`). / Web page on GitHub Pages.
- Ordner **`release/`** im Wurzelverzeichnis: alle gebauten Hex-Dateien + `manifest.json` (von `firmware/publish.sh`),
  daraus bedient sich der Online-Flasher – lokal relativ, online über `raw.githubusercontent.com`, weil Pages nur
  `docs/` ausliefert. / New **`release/`** folder with all built hex files + `manifest.json`; the web flasher loads
  from there (relative locally, via `raw.githubusercontent.com` online).
- **DOF steuert den Strip direkt** (Leonardo/Pro Micro): das Board meldet sich zusätzlich als virtueller COM-Port
  (CDC-ACM, kein Treiber ab Windows 10) und spricht dort das Protokoll des DOF-**TeensyStripControllers** mit einem
  Kanal (`firmware/strip_serial.c`, Befehle L/M/V/C/O/R/F, 'A'/'N'). In der Cabinet.xml genügt ein
  `<TeensyStripController>` mit COM-Port und `NumberOfLedsStrip1`; die Antwort auf „O“ kommt erst nach dem Schreiben
  des Strips. Build-Schalter `ENABLE_STRIP_SERIAL` (nur 32U4: 16U2/8U2/32U2 haben zu wenige Endpunkte).
  **Testfunktion – ohne Hardware nicht geprüft.**
  **DOF drives the strip directly** (Leonardo/Pro Micro): additional virtual COM port speaking the DOF
  TeensyStripController protocol (one channel, `strip_serial.c`); `<TeensyStripController>` in Cabinet.xml is enough.
  Build switch `ENABLE_STRIP_SERIAL` (32U4 only). Untested on hardware.
- Neues Firmware-Ziel `arduino_uno__m16u2` für den Uno R3 (16U2 statt 8U2: richtige Bootloader-Adresse).
  New firmware target `arduino_uno__m16u2` for the Uno R3 (16U2 instead of 8U2: correct bootloader address).
- **Arduino Due** (SAM3X8E, ARM Cortex-M3, nativer USB-Port): komplett neue Firmware in `firmware_due/` (PlatformIO,
  Arduino-SAM-Core), die Protokoll, Konfiguration, Ausgänge, Panel, Strip und DOF-COM-Port **unverändert aus
  `firmware/` mitkompiliert**. Eigenes PluggableUSB-Modul mit drei HID-Schnittstellen und Gerätedeskriptor
  (VID 0xFAFA, PID 0x00F0 + LED-Wiz-ID zur Laufzeit), Soft-PWM über TC3, Freilauf-ADC (12 Kanäle), EEPROM-Ersatz im
  letzten Kilobyte von Flash-Bank 1, WS2811/WS2812-Treiber mit Zyklenzähler (bis 255 Pixel), 74 Pins (D0/D1 gesperrt),
  Werksbelegung D22–D53. Befehl 66 startet den ROM-Bootloader SAM-BA. **Alle Pins 3,3 V.** Board-ID 6 im Tool.
  **Testfunktion – ohne Hardware nicht geprüft.**
  **Arduino Due** (SAM3X8E, native USB port): brand-new firmware in `firmware_due/` (PlatformIO, Arduino SAM core)
  compiling the shared modules from `firmware/` unchanged; own PluggableUSB module (three HID interfaces, runtime
  VID/PID), soft PWM via TC3, free-running ADC, EEPROM stand-in in flash bank 1, WS2812 driver with cycle counter
  (up to 255 pixels), 74 pins, factory outputs D22–D53, command 66 enters SAM-BA. All pins 3.3 V. Untested on hardware.
- Online-Flasher: **SAM-BA-Protokoll für den Due** über Web Serial (wie bossac: Chip-ID, Seiten in den Latch-Puffer,
  EEFC „Erase page and write page“, Zurücklesen, Boot-Flag GPNVM1, Reset), 1200-Baud-Trick in den ROM-Bootloader,
  rohe `.bin`-Dateien, Anleitungsabschnitt „Arduino Due“, 3,3-V-Warnung; Board, Hinweis, Warnkasten und Anleitung
  sind sichtbar als **BETA** gekennzeichnet (auch Startseite und README). `release/arduino_due.bin` über `publish.sh`.
  **Testfunktion – ohne Hardware nur im Headless-Browser geprüft.**
  Web flasher: **SAM-BA protocol for the Due** via Web Serial (like bossac), 1200 baud touch into the ROM bootloader,
  raw `.bin` files, guide section, 3.3 V warning. Untested on hardware.

### Verbessert / Improved
- Repo-Aufbau: Webseite (Flasher, Konfigurations-Tool, Hex-Dateien) in `docs/`, gemeinsames `style.css`; der
  unveränderte Original-Stand von 2023 liegt zum Nachschlagen in `old/`. `tools/lwcconfig-web/` entfällt.
  Die alten Visual-Studio-Projektdateien (`win32/*/build/vs_2008`, `vs_2012`) gibt es nur noch in `old/`; gebaut wird
  per CMake (`win32/CMakeLists.txt`).
  Repo layout: web page in `docs/`, unchanged upstream snapshot in `old/`; `tools/lwcconfig-web/` is gone. The old
  Visual Studio project files now live only in `old/`; the DLL and tools are built with CMake.
- Gemeinsame Module portabel gemacht: `pins.h` kapselt den Pin-Zugriff (`pins_read`/`pins_write`, AVR 8-Bit-Port oder
  SAM PIO), `outputs_tick()` statt fest verdrahtetem Timer-Interrupt, ADC-Hooks (`ADC_select`/`ADC_read`) für Nicht-AVR;
  AVR-Verhalten und -Größen unverändert.
  Shared modules made portable (`pins_read`/`pins_write`, `outputs_tick()`, ADC hooks); AVR behaviour and sizes unchanged.
- Konfigurations-Tool: Board „Arduino Due (3,3 V)“ mit 74 Pin-Namen; Pin-Bits bis 31 (ARM-Ports).
  Configuration tool: board "Arduino Due (3.3 V)" with 74 pin names; pin bits up to 31 (ARM ports).
- Die 32 LED-Wiz-Ausgänge sind wieder **reine GPIO-Ausgänge wie im Original** (`outputs.c` ohne Strip-Code);
  der Strip liest nur noch die Helligkeiten über `outputs_get_level()`.
  The 32 LED-Wiz outputs are plain GPIO outputs again like the original; the strip only reads their levels.
- Konfigurationslayout Version 2 (`config.h`): Ausgang = Pin + Flags, neuer Block `mirror[]`, `ws.mode`;
  Protokollversion 2, Firmware 2.1. Alte Konfigurationen im EEPROM werden erkannt und durch die Werksbelegung ersetzt.
  Configuration layout version 2; old EEPROM contents are detected and replaced by the board defaults.

### Behoben / Fixed
- `firmware/lufa` lag auf GitHub nur als toter Submodul-Verweis (die `.gitmodules` war schon entfernt, die Dateien
  aber nie eingecheckt). Dadurch schlug jeder Checkout mit Submodulen fehl – auch der GitHub-Pages-Aufbau – und das
  Repo war von außen nicht baubar. Die gekürzte LUFA-Kopie (Release 130901, MIT-artige Lizenz) ist jetzt wirklich im
  Repo; die `.gitignore`-Regel `build/` ignorierte zudem `LUFA/Build/` (Groß-/Kleinschreibung) und ist eingegrenzt.
  `firmware/lufa` was only a dangling submodule pointer on GitHub (`.gitmodules` removed, files never committed), so
  every recursive checkout failed – including the GitHub Pages build. The trimmed LUFA copy is now really in the
  repo; the `.gitignore` rule `build/` also swallowed `LUFA/Build/` and has been narrowed.
- Befehl „Bootloader starten“ (66) auf Leonardo/Pro Micro: Caterina läuft nach dem Reset zuerst und löscht die
  Reset-Flags, der bisherige Sprung in den Bootloader griff deshalb nicht. Jetzt wird der Caterina-Schlüssel
  (0x7777 an 0x0800) gesetzt wie beim 1200-Baud-Trick der Arduino-IDE; der Bootloader meldet sich dann 8 s als
  serieller Port. Für die DFU-Chips werden vor dem Sprung die Reset-Flags gelöscht (Atmel-DFU erkennt so den Sprung).
  "Start bootloader" (66) on Leonardo/Pro Micro did not work reliably because Caterina clears the reset flags; the
  Caterina key (0x7777 at 0x0800) is now set like the Arduino IDE's 1200 baud touch. DFU chips: MCUSR cleared first.

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
