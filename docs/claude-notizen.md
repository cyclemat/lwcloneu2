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

## DOF und WS2811/WS2812 (recherchiert 24.11.2026, Quellen: TeensyStripController.ino, DOF TeensyStripController.cs,
## WemosD1StripController.cs, LedStrip.cs auf GitHub; die DOF-Doku selbst beschreibt das Protokoll nicht)
- DOF kennt für adressierbare Strips nur **serielle Controller** (USB-CDC/COM-Port): TeensyStripController (Teensy 3.2 +
  OctoWS2811, 8 Kanäle × max. 1100 LEDs), WemosD1MPStripController (gleiches Protokoll + Erweiterungen) und den
  eingestellten DirectStripController (FT245R-Bitbang). Kein HID-Strip-Controller, Baudrate egal (USB).
- **Protokoll (Byte-Befehle, Antwort 'A' = ACK / 'N' = NACK, Zahlen Big-Endian):**
  `L hi lo` LEDs je Kanal setzen · `M` → `hi lo` max. LEDs je Kanal + 'A' · `V` → `major minor` + 'A' ·
  `C` alle Pixel löschen · `O` ausgeben (show) · `R hi lo(first) hi lo(count)` + 3 Byte je LED ·
  `F hi lo hi lo r g b` Bereich füllen. Wemos zusätzlich: `Z strip nstrips-1 hi lo` (Länge je Strip), `T` Test,
  `Q first(2) nData(2) nLeds(2)` + RLE-Blöcke `cnt r g b`.
- **Handshake beim Verbinden:** DOF schickt ein 0x00-Byte, erwartet 'A' oder 'N' (20 ms); kommt nichts, schickt es
  3000 Null-Bytes (bricht ein halb empfangenes R-Kommando ab), wartet 50 ms, 20 Versuche. Dann `M`, Prüfung
  max ≥ konfiguriert, `L` mit max(NumberOfLedsStripN), `C`, `O`.
- **Je Frame:** für jeden Kanal i mit LEDs: `R` mit first = i × LEDs-je-Kanal, dann warten auf 'A'; zuletzt `O` +
  'A'. Ein Kanal (unser Fall) → first = 0 … n−1. Fehlende 'A' = Exception/Abbruch in DOF.
- **Farben:** DOF ordnet die Bytes selbst nach `ColorOrder` des LedStrip-Toys um (Default **RBG**!) und schickt sie
  roh; der Teensy sendet unverändert (WS2811_RGB). `FirstLedNumber` zählt ab 1 (→ Byte-Offset (n−1)×3).
  Für uns: in DOF `ColorOrder=RGB` empfehlen und die Reihenfolge nur im Tool (`ws.order`) einstellen – sonst doppelt.
- **Machbarkeit bei uns:** CDC braucht 3 Endpunkte (Notify IN, Data IN, Data OUT). 32u4 hat 6 (wir nutzen 3 HID-IN)
  → passt (Leonardo, Pro Micro). 16u2/8u2/32u2 haben nur 4 → CDC nur, wenn die 3 HID-Schnittstellen auf eine
  zusammengelegt würden, was die LED-Wiz-Kompatibilität (eigene Schnittstelle, 8-Byte-Reports ohne Report-ID) bricht
  → für Uno/Mega/Breakout nicht sinnvoll. LUFA-CDC-Klassentreiber liegt in `firmware/lufa` (CDCClassDevice.c).
  Windows 10/11 bindet CDC-ACM automatisch (usbser), kein INF nötig.
- **Eigene DOF-„DLL“ (Nutzerfrage 24.11.2026):** DOF (MIT-Lizenz, .NET Framework 4.8, C#) hat **keinen Plugin-Lader**
  für Output-Controller. `OutputControllerList.ReadXml` sucht den Typnamen aus Cabinet.xml zwar in *allen geladenen*
  Assemblies (`AppDomain.CurrentDomain.GetAssemblies()`), lädt aber selbst keine fremde DLL. Weg: eigene C#-Klasse
  `LWCloneU2StripController : OutputControllerCompleteBase` (→ `ISupportsSetValues`, damit das LedStrip-Toy sie nutzt)
  **in DOF einbauen** und als Pull Request an DirectOutput/DirectOutput bzw. mjrgh/DirectOutput (R3++); so kamen auch
  DudesCab, PinOne, PSPico (Pinscape Pico) hinein. Abstrakte Methoden: GetNumberOfConfiguredOutputs, VerifySettings,
  ConnectToController, DisconnectFromController, UpdateOutputs(byte[]). DOF spricht LED-Wiz **direkt per HID**
  (`HIDImports`, CreateFile/WriteFile) – unsere ledwiz.dll wird von DOF nicht benutzt (nur LedBlinky, PinballX u. a.).
  Firmware-Voraussetzung: 8-Byte-Reports (2 Pixel je SET_REPORT ≈ 1 ms) sind für DOF zu langsam → eigene 4. HID-
  Schnittstelle „Strip“ mit 64-Byte-Output-Report (≈ 20 Pixel/Report) + 1 IN-Endpunkt für die SHOW-Antwort; passt
  auch auf 16u2/8u2/32u2 (4 Endpunkte, 3 belegt). LED-Wiz-Report muss 8 Byte ohne Report-ID bleiben.
- **Entscheidung 24.11.2026:** Nutzer will **keine eigene DOF-Klasse** („DOF wird nicht mehr gepflegt“) → CDC-Weg
  umgesetzt: `strip_serial.c/.h` (Zustandsautomat, Teensy-Protokoll, 1 Kanal), CDC-ACM mit IAD in `descriptors.c/.h`
  (Gerät jetzt Klasse EF/02/01 auf 32u4, `config_flag_serial` in bcdDevice/Seriennummer gegen Windows-Deskriptor-Cache),
  LUFA-CDC-Klassentreiber eingebunden (build.sh + lufa.mk `LUFA_SRC_USBCLASS_DEVICE`), `ENABLE_STRIP_SERIAL` in
  devconfig.h von Leonardo/Pro Micro. Strip-API erweitert (`strip_direct_ready/count/set_count/set_pixel/fill/clear/show`),
  SHOW-Antwort als Bitmaske HID/SERIAL. 'L' setzt die Pixelzahl zur Laufzeit (nicht gespeichert). Leonardo: 44,7 % Flash
  (+2,1 KB), 63,4 % RAM. Endpunkte: EP4 Notify 8, EP5 IN 32, EP6 OUT 32.
- Ursprüngliche Umsetzungsidee CDC: Modul `strip_serial.c` (Build-Schalter, nur 32u4): CDC-Schnittstelle „LWCloneU2 Strip“, Teensy-
  Protokoll 1 Kanal (M = WS2812_MAX_PIXELS, L ≤ konfigurierte Pixel), schreibt in den Direkt-Modus-Puffer von
  `strip.c`; 'A' auf `O` erst nach dem Senden (wie STRIP_SHOW). In der Cabinet.xml: `<TeensyStripController>` mit
  `ComPortName` und `NumberOfLedsStrip1`. Blockierendes Warten des Teensy-Codes nicht übernehmen (Zustandsautomat).

## Arduino Due (24.11.2026, Nutzerwunsch „komplett neue Firmware … im gleichen Projekt“)
- Weg gewählt: **nativer USB-Port** mit Arduino-SAM-Core (PlatformIO `atmelsam@9.0.0`, Board-JSON `boards/lwc_due.json`
  wegen VID/PID/Produktname; `debug`-Block ist Pflicht, sonst „Missed J-Link Device ID“). Projekt `firmware_due/`.
- **Gemeinsame Module wiederverwendet** statt kopiert: `shared.py` (PRE-Skript! sonst „main program already constructed“)
  kompiliert proto/config/outputs/panel/strip/strip_serial aus `../firmware` mit. Dafür in den AVR-Quellen portabel
  gemacht: `pins.h` (`pin_io_t` je Plattform + `pins_read/pins_write`), `outputs_tick()` (ISR nur bei LED_TIMER_vect,
  sonst OUTPUTS_TICK_EXTERNAL), panel.c ADC über `ADC_select/ADC_read` auf Nicht-AVR, `<avr/io.h>` nur unter `__AVR__`.
  AVR-Builds danach identisch groß (Leonardo 14628 B).
- Ersatz-Header nur für `<util/atomic.h>` (PRIMASK + cleanup-Attribut), `<avr/eeprom.h>`, `<avr/io.h>`; **nicht**
  `avr/pgmspace.h`/`interrupt.h` – die liefert der SAM-Core selbst (WString.cpp braucht seine Fassung).
- USB: Der SAM-Core fragt vor jedem Deskriptor die PluggableUSB-Module → `usb_lwz.cpp` liefert den **Gerätedeskriptor
  selbst** (EF/02/01, VID FAFA, PID F0+ID, bcdDevice/Serial wie descriptors.c). ID wird in `initVariant()` (weak im
  Core, läuft vor `USBDevice.attach()`) aus dem Flash gelesen. Output-Reports kommen als SET_REPORT im USB-Interrupt →
  Ringpuffer → Hauptschleife. IN-Senden nur wenn TXINI (UDD_Send blockiert sonst). Due läuft **High-Speed** →
  bInterval in Mikroframes (4 = 1 ms). DOF öffnet ohne DTR → `lwz_cdc_send` direkt auf CDC_TX statt SerialUSB.write.
- EEPROM-Ersatz: 1 KB bei IFLASH1_ADDR+IFLASH1_SIZE−1024, eigener EEFC-Befehl aus `.ramfunc` (CLB + EWP je Seite),
  Firmware läuft aus Bank 0. Layout wie AVR (ID bei 16, Konfig ab 64). CMSIS des Cores kennt DWT nicht → Register
  direkt (0xE0001000/04, DEMCR 0xE000EDFC) im WS2812-Treiber.
- Ergebnis: 31,5 KB Flash, 6,3 KB RAM, keine eigenen Warnungen. Befehl 66 → `banzai()` (GPNVM1 löschen) → SAM-BA.
- Flasher: SAM-BA-Monitorprotokoll in JS (N#, V#, w/W/o/R, EEFC über W auf FCR + Polling FSR, CLB-Unlock je 64 Seiten,
  EWP je Seite mit 64 W-Befehlen in einem write(), Verify mit o+R wegen Zweierpotenz-Bug, SGPB 1, RSTC A500000D).
  Nicht an Hardware getestet; Risiken: W-Durchsatz des ROM-Monitors, Latch-Schreiben per Einzelwort (bossac nutzt
  WordCopy-Applet – laut Datenblatt reichen 32-Bit-Schreibzugriffe).
- Tool: BOARDS[6] (74 Namen), `bit: b & 0x7F`. publish.sh kopiert `arduino_due.bin`; Workflow-Job `due` (pio).
- Nutzer 24.11.2026: „als Beta beschreiben, auch in der README“ → BETA sichtbar in Flasher (Option, Hinweis,
  Warnkasten `warn_due_beta`, Anleitung g_d8), Startseite, README (Tabelle, Absätze, Flasher-Abschnitt, Schnellstart
  mit bossac). Commit accb4c8, gepusht am 24.11.2026 („speicher mal alles“).

## Offen
- An Hardware testen: USB-Enumeration (3 HID-Schnittstellen), WS2812-Timing (Oszi), ADC-Achsen, Brückenmodus Mega/Uno,
  Online-Flasher (alle 5 Protokolle inkl. SAM-BA), Bootloader-Befehl 66 (Caterina-Schlüssel, DFU-Sprung, Due → SAM-BA),
  Due komplett (Enumeration mit eigenem Gerätedeskriptor, Flash-Schreiben der Konfiguration, DWT-Timing, CDC ohne DTR).
- GitHub Actions nur `workflow_dispatch` – erster Lauf nach Push auf Ansage des Nutzers.
- **Pages-Panne 24.11.2026:** die ersten beiden Pages-Builds (13:46, 13:51 UTC) scheiterten mit
  „No url found for submodule path 'firmware/lufa' in .gitmodules“: `firmware/lufa` stand im Index noch als Gitlink
  (160000, Upstream-Commit 012c565), die `.gitmodules` war gelöscht, die LUFA-Dateien lokal da, aber nie committed.
  Fix: `git rm --cached firmware/lufa` + `git add firmware/lufa`; `.gitignore` `build/` → `/build/` + `firmware/build/`,
  weil Git unter Windows (core.ignorecase) sonst `LUFA/Build/` (Make-Fragmente für lufa.mk) verschluckt. Lokal liegt noch
  `.git/modules/firmware/` als Rest des alten Submoduls – harmlos. Vor dem Fix war das Repo von außen nicht baubar.
- **Ordner `release/` (Nutzerwunsch 24.11.2026):** alle gebauten Hex + manifest.json liegen im Wurzelverzeichnis
  (`firmware/publish.sh` → `../release`), nicht mehr in `docs/firmware`. Der Flasher probiert `RELEASE_BASES`
  (`../release/` lokal, dann `https://raw.githubusercontent.com/cyclemat/lwcloneu2/master/release/`), weil Pages nur
  `docs/` ausliefert; raw liefert `Access-Control-Allow-Origin: *`. Download-Links von raw per Blob (download-Attribut
  greift cross-origin nicht). Headless geprüft: lokal lädt Manifest 2.1; raw-Fallback erst nach Push prüfbar.
  Außerdem 24.11.: alte VS-2008/2012-Projektdateien nur noch in `old/` (CMake ersetzt sie).
- GitHub Pages für `docs/` ist seit 24.11.2026 aktiv (per `gh api … /pages`, Quelle master//docs; Nutzer-OK „ja“).
  https_enforced noch false – nach dem ersten Zertifikat per `gh api -X PUT repos/cyclemat/lwcloneu2/pages -F https_enforced=true` setzen.
- Flasher-Anleitung (24.11.2026, Nutzerwunsch „online flasher mit anleitung“): Karte „So funktioniert es“ in
  `docs/flash.html` mit `<details>`-Abschnitten, Texte als HTML in I18N (`data-i18n-html`), SVG des ICSP-Headers.
- DLL braucht UCRT (Windows 10+; auf Windows 7 UCRT-Update). Für eine MSVC-DLL: CI-Workflow (windows-latest).
- Webtool: Encoder-Eingänge werden nicht entprellt (wie Upstream), Joystick-Tasten 9–16 nicht vorgesehen.
- Mögliche Erweiterungen: PWM-Hardwarekanäle, mehr als 32 Eingänge auf dem Mega (CFG_NUM_INPUTS pro Board),
  lwcconfig.exe mit Konfigurationsbefehlen, 64-Byte-Reports für schnellere Pixel-Übertragung (eigene HID-Schnittstelle),
  DOF-Plugin für den Direkt-Modus.
