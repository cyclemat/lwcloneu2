# LWCloneU2 – Arbeitsnotizen (Claude)

Stand: 24.11.2026

## Was passiert ist
- Fork `cyclemat/lwcloneu2` stand auf Dez. 2013; per Fast-Forward auf Upstream `cithraidt/lwcloneu2` (Aug. 2023,
  37 Commits: Pro Micro, ADC, Overlapped-I/O in der DLL, LUFA-Submodul) gebracht.
- LUFA 130901 als gekürzte Kopie fest eingebettet (`firmware/lufa`), Submodul entfernt → ZIP-Download baut.
- Firmware auf **v2** umgebaut: Pin-Belegung zur Laufzeit im EEPROM, USB-Befehle `0xA0–0xA7` (siehe `firmware/proto.h`),
  WS2811/WS2812-Treiber (`ws2812.c`), Analog-Achsen mit Min/Mitte/Max + Totzone (`panel.c`), 4 Joysticks
  mit je 4 Achsen (int8) + 8 Tasten, Konfigurationsstruktur `config.h` (372 Byte Standard, boardabhängig).
- Alte `led.c`/`pinmap`-Makros entfernt; je Board jetzt `BOARD_PIN_TABLE` in `pinmap.h`.
- Windows-DLL: Fehler behoben (Hänger beim Prozessende in `DllMain`, Event-Leak, Listenpflege), CMake-Build.
- Konfigurations-Tool als WebHID-Seite: `tools/lwcconfig-web/index.html` (Edge/Chrome, ohne Installation).

## Entscheidungen
- Konfigurations-Tool läuft im Browser (WebHID) statt als .exe – Nutzer hat nachgefragt, ob seriell: **nein, USB-HID**,
  gleicher Weg wie die LED-Wiz-DLL; beim Mega 2560 reicht der 16u2 die Befehle über die interne UART an den 2560.
- Compiler für die DLL: LLVM-MinGW + GNU make per winget (Nutzerentscheidung 24.11.2026).
- Commit-Identität anonym (`cyclemat <6072436+cyclemat@users.noreply.github.com>`), `core.autocrlf=false`.
- Lokale Commits erlaubt („git vorbereiten“), **Push nur nach OK**.

## Status / Beta
- **Nichts an echter Hardware getestet** (keine Hardware hier). Alle Firmware-Ziele übersetzen und linken
  (avr-gcc 7.3 aus PlatformIO): Leonardo 36 % Flash / 54 % RAM, Mega2560 3 % / 20 %, Uno 25 % / 71 %,
  Breakout 32U2 31 % / 84 % (RAM knapp → kleinere Tabellen: 16 Eingänge, 0 Achsen, 8 Pixel).
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
- winget hing beim Quellen-Update → LLVM-MinGW als Zip lokal entpackt (nicht im Repo).

## Offen
- An Hardware testen: USB-Enumeration (3 HID-Schnittstellen), WS2812-Timing (Oszi), ADC-Achsen, Brückenmodus Mega/Uno.
- GitHub Actions nur `workflow_dispatch` – erster Lauf nach Push auf Ansage des Nutzers.
- DLL braucht UCRT (Windows 10+; auf Windows 7 UCRT-Update). Für eine MSVC-DLL: CI-Workflow (windows-latest).
- Webtool: Encoder-Eingänge werden nicht entprellt (wie Upstream), Joystick-Tasten 9–16 nicht vorgesehen.
- Mögliche Erweiterungen: PWM-Hardwarekanäle, mehr als 32 Eingänge auf dem Mega (CFG_NUM_INPUTS pro Board),
  lwcconfig.exe mit Konfigurationsbefehlen.
