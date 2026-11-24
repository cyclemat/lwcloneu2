# LWCloneU2 – Arbeitsnotizen (Claude)

Stand: 24.11.2026

## Was passiert ist
- Fork `cyclemat/lwcloneu2` stand auf Dez. 2013; per Fast-Forward auf Upstream `cithraidt/lwcloneu2` (Aug. 2023,
  37 Commits: Pro Micro, ADC, Overlapped-I/O in der DLL, LUFA-Submodul) gebracht.
- LUFA 130901 als gekürzte Kopie fest eingebettet (`firmware/lufa`), Submodul entfernt → ZIP-Download baut.
- Firmware auf **v2** umgebaut: Pin-Belegung zur Laufzeit im EEPROM, USB-Befehle `0xA0–0xA7` (siehe `firmware/proto.h`),
  WS2811/WS2812-Treiber (`ws2812.c`), Analog-Achsen mit Min/Mitte/Max + Totzone (`panel.c`), 4 Joysticks
  mit je 4 Achsen (int8) + 8 Tasten, Konfigurationsstruktur `config.h` (564 Byte Standard, boardabhängig).
- **24.11.2026, Nutzerwunsch:** WS2811/12 soll „eine eigene Sache“ sein und den Grundcode nicht ändern → Umbau
  auf **v2.1**: `outputs.c` wieder reines GPIO (wie Original), neues Modul `strip.c` mit Direkt-Modus (USB-Befehle
  `0xB0–0xB3`) und Spiegel-Modus (`mirror[]`: Ausgang → Pixel ab/Anzahl → Farbe). Nutzer hat „Beides“ gewählt.
  Konfig-Layout Version 2, Protokoll 2; Tool-Reiter „LED-Strip“.
- Alte `led.c`/`pinmap`-Makros entfernt; je Board jetzt `BOARD_PIN_TABLE` in `pinmap.h`.
- Windows-DLL: Fehler behoben (Hänger beim Prozessende in `DllMain`, Event-Leak, Listenpflege), CMake-Build.
- Konfigurations-Tool als WebHID-Seite (Edge/Chrome, ohne Installation), seit 24.11.2026 unter `docs/config.html`.
- **24.11.2026, Nutzerwunsch „alten Inhalt in Ordner old, neuen direkt ins Git, mit Online-Flasher und Config-Tool“:**
  `old/` = unveränderter Upstream-Stand `40e39ac` (per `git archive`, ohne LUFA-Submodul, mit Hinweis-README);
  Webseite in `docs/` für GitHub Pages (Quelle: master, Ordner /docs – **Pages noch nicht aktiviert**, braucht
  `gh api repos/cyclemat/lwcloneu2/pages -X POST -f source[branch]=master -f source[path]=/docs` oder Repo-Settings):
  `index.html` (Start), `flash.html` (Online-Flasher), `config.html` (Tool), `style.css`, `firmware/*.hex` +
  `manifest.json` (aus `firmware/publish.sh`), `.nojekyll`. `tools/lwcconfig-web/` entfällt.
- Online-Flasher: Web Serial für AVR109/Caterina (inkl. 1200-Baud-Trick; Windows zeigt den Bootloader als neuen Port →
  zweiter Port-Dialog), STK500 v1 (optiboot, DTR-Reset), STK500 v2 (Mega, Wortadresse mit Bit 31); WebUSB für
  Atmel-DFU (Befehle wie dfu-programmer: Erase 04 00 FF, Program 01 00 start end + 32-Byte-Header + 16-Byte-Suffix,
  Read 03 00 …, Start-App 04 03 00 + leerer DNLOAD); Chip-Prüfung per Signatur bzw. DFU-PID (2FEE/2FEF/2FF0).
  Unter Windows braucht WebUSB den WinUSB-Treiber (Zadig) für das DFU-Gerät.
- Bootloader-Befehl 66 auf 32u4 war im Original wackelig (Caterina läuft per BOOTRST zuerst und löscht MCUSR, danach
  griff der WDRF-Test nicht). Jetzt Caterina-Schlüssel 0x7777 an 0x0800 vor dem Watchdog-Reset (`hwconfig_32u4.h`
  `BOOTLOADER_MAGIC_*`), kein Sprung mehr auf 32u4; DFU-Chips: `MCUSR = 0` vor dem Sprung, Bedingung `WDRF || MCUSR==0`.
- Neues Ziel `arduino_uno__m16u2` (Uno R3 hat 16U2; 8U2-Hex hätte falsche Bootloader-Adresse 0x1800 statt 0x3800).

## Entscheidungen
- Konfigurations-Tool läuft im Browser (WebHID) statt als .exe – Nutzer hat nachgefragt, ob seriell: **nein, USB-HID**,
  gleicher Weg wie die LED-Wiz-DLL; beim Mega 2560 reicht der 16u2 die Befehle über die interne UART an den 2560.
- Compiler für die DLL: LLVM-MinGW + GNU make per winget (Nutzerentscheidung 24.11.2026).
- Commit-Identität anonym (`cyclemat <6072436+cyclemat@users.noreply.github.com>`), `core.autocrlf=false`.
- Lokale Commits erlaubt („git vorbereiten“), **Push nur nach OK**.

## Status / Beta
- **Nichts an echter Hardware getestet** (keine Hardware hier). Alle Firmware-Ziele übersetzen und linken
  (avr-gcc 7.3 aus PlatformIO), Stand v2.1: Leonardo 38 % Flash / 62 % RAM, Mega2560 4 % / 23 %, Uno 27 % / 81 %,
  Breakout 32U2 33 % / 86 % (RAM knapp → kleinere Tabellen: 16 Eingänge, 0 Achsen, 8 Pixel, 8 Spiegel-Einträge).
- Strip-Übertragung sperrt Interrupts (~30 µs/Pixel). Auf Mega/Uno kann die Brücken-UART (250 kBit/s, 2-Byte-FIFO)
  währenddessen Reports verlieren → deshalb antwortet `STRIP_SHOW` erst nach dem Schreiben; Hosts sollen darauf warten.
- WS2812-Timing per Zyklen gerechnet (16 MHz: T0H 312 ns, T1H 812 ns; 8 MHz: 375/625 ns) – am Oszi prüfen.
- Dual-Chip-Pfad (Uno/Mega: Antworten über UART mit Byte 0 ≥ 0xA0) ungetestet.

## Gebaut und geprüft (24.11.2026)
- `ledwiz.dll` x86 + x64 mit LLVM-MinGW 20261117 (Clang 23) → `win32/bin/x86|x64/` (nicht im Git, Build-Ausgabe).
  7 Exporte undekoriert, Importe hid/setupapi/UCRT. testapp/lwcconfig (x86 + x64) je 3× ohne Gerät gestartet:
  Exit 0, „no ledwiz devices detected!“. Vorher: x64-lwcconfig hing in FreeLibrary, testapp stürzte beim Beenden
  ab → Ursache Thread-Erzeugung in DllMain; behoben durch Lazy-Start (Commit 94030a6). Ohne Gerät nicht weiter testbar.
  Kuriosum: gepufferte stdout-Ausgabe von testapp ging beim Beenden verloren → stdout jetzt ungepuffert.
- Firmware: `firmware/build.sh` erzeugt 7 Hex-Dateien in `firmware/build/`.
- Webtool: Headless-Edge-Test – Skript läuft ohne Fehler, Oberfläche wird gerendert; WebHID ohne Gerät nicht testbar.
- Flasher (24.11.2026): Headless-Edge über lokalen HTTP-Server – keine JS-Fehler, `parseHex` liefert exakt die
  avr-size-Programmgrößen (Leonardo 12574, Mega 9204), alle 6 Board-Varianten rendern, EN-Umschaltung, STK500v2-Rahmen
  mit Prüfsumme korrekt. Web Serial / WebUSB / DFU ohne Hardware nicht testbar.
- winget hing beim Quellen-Update → LLVM-MinGW als Zip lokal entpackt (nicht im Repo).

## Offen
- An Hardware testen: USB-Enumeration (3 HID-Schnittstellen), WS2812-Timing (Oszi), ADC-Achsen, Brückenmodus Mega/Uno,
  Online-Flasher (alle 4 Protokolle), Bootloader-Befehl 66 (Caterina-Schlüssel, DFU-Sprung).
- GitHub Actions nur `workflow_dispatch` – erster Lauf nach Push auf Ansage des Nutzers.
- GitHub Pages für `docs/` aktivieren (nach Push, auf Ansage) – dann ist https://cyclemat.github.io/lwcloneu2/ erreichbar.
- DLL braucht UCRT (Windows 10+; auf Windows 7 UCRT-Update). Für eine MSVC-DLL: CI-Workflow (windows-latest).
- Webtool: Encoder-Eingänge werden nicht entprellt (wie Upstream), Joystick-Tasten 9–16 nicht vorgesehen.
- Mögliche Erweiterungen: PWM-Hardwarekanäle, mehr als 32 Eingänge auf dem Mega (CFG_NUM_INPUTS pro Board),
  lwcconfig.exe mit Konfigurationsbefehlen, 64-Byte-Reports für schnellere Pixel-Übertragung (eigene HID-Schnittstelle),
  DOF-Plugin für den Direkt-Modus.
