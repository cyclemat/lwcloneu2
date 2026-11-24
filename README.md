# LWCloneU2

**Deutsch** · [English below](#english)

LED-Wiz-kompatibler USB-Ausgangs-Controller (32 Ausgänge mit Soft-PWM) **plus** Eingabe-Encoder (Tastatur, bis zu
4 Joysticks mit analogen Achsen, Maus) **plus** WS2811/WS2812-LED-Strip – für Arduino-Boards mit Atmel-AVR-USB-Chip.
Gedacht für MAME-/Pinball-Cabinets: ein Board für Beleuchtung und Bedienelemente, eingestellt im Browser.

Firmware **v2.1** – Fork von [cithraidt/lwcloneu2](https://github.com/cithraidt/lwcloneu2) (GPLv2).
Stand: **Beta** – übersetzt und geprüft, aber noch nicht an echter Hardware getestet.

**Webseite: <https://cyclemat.github.io/lwcloneu2/>** – Firmware im Browser flashen und das Board einrichten
(Microsoft Edge oder Google Chrome, nichts zu installieren).

## Inhalt

1. [Was kann es](#was-kann-es)
2. [Unterstützte Boards](#unterstützte-boards)
3. [Schnellstart](#schnellstart)
4. [Der Online-Flasher](#der-online-flasher) – Firmware im Browser auf das Board bringen
5. [Das Konfigurations-Tool](#das-konfigurations-tool) – Verbinden, Ausgänge, Eingänge, Achsen, LED-Strip, Gerät, Fehlersuche
6. [ledwiz.dll](#ledwizdll)
7. [Selbst bauen](#selbst-bauen)
8. [Aufbau des Codes](#aufbau-des-codes)
9. [Lizenz](#lizenz)

## Was kann es

- **32 LED-Wiz-Ausgänge** mit 48 Helligkeitsstufen und den vier Pulsmustern des Originals. Jede LED-Wiz-Software
  (DOF, LedBlinky, PinballX, …) sieht ein normales LED-Wiz-Gerät.
- **Pins frei belegbar, ohne Neukompilieren.** Welcher Pin Ausgang, Taster, Analog-Achse oder Strip-Datenleitung ist,
  steht im EEPROM und wird im Browser eingestellt – auch im laufenden Betrieb („Anwenden“).
- **Eingänge:** Tasten, Modifier, Medientasten, Maustasten, Joystick-Richtungen und -Tasten für bis zu 4 Joysticks,
  eine Shift-Taste für Doppelbelegungen, Drehgeber/Trackball als Maus.
- **Analoge Achsen** (bis 8) mit Kalibrierung (Min/Mitte/Max), **Totzone** und Invertierung.
- **WS2811/WS2812-Strip** an einem zusätzlichen Pin, als eigenes Modul ohne Einfluss auf die 32 Ausgänge.
  **Spiegel-Modus:** Pixelbereiche leuchten in einer Farbe, die Helligkeit folgt einem LED-Wiz-Ausgang (läuft sofort
  mit DOF). **Direkt-Modus:** eigene Programme setzen die Pixel per USB; auf Leonardo/Pro Micro zusätzlich als
  virtueller COM-Port im Protokoll des DOF-TeensyStripControllers – DOF steuert den Strip dann direkt.
- `ledwiz.dll`-Ersatz (32/64 Bit), asynchron, blockiert die Anwendung nicht.

## Unterstützte Boards

| Board | MCU | Pins im Tool | Analog-Achsen | Werksbelegung der Ausgänge 1–n | Hinweis |
|---|---|---|---|---|---|
| Arduino Leonardo | ATmega32U4 | D0–D17, A0–A5, TXLED | ja (A0–A5, D4, D6, D8, D9, D10, D12) | D2–D13, A0–A5 (18) | empfohlen |
| Pro Micro / Arduino Micro | ATmega32U4 | wie Leonardo, weniger herausgeführt | ja | D2–D10, D14–D16, A0–A3 (16) | |
| Arduino Mega 2560 | ATmega2560 + ATmega16U2 | D2–D53, A0–A15 | ja (16) | D22–D53 (32) | **beide** MCUs flashen |
| Arduino Uno R2/R3 | ATmega328P + ATmega8U2/16U2 | D2–D13, A0–A5 | ja (6) | D2–D13, A0–A5 (18) | beide MCUs flashen |
| Breakout ATmega32U2 | ATmega32U2 | 21 Pins (PB0–7, PC2, PC4–7, PD0–7) | nein | wie alte Firmware | 1 KB RAM: 16 Eingänge, 8 Pixel |

Beim Mega/Uno übernimmt der kleine USB-Chip (16U2/8U2) nur die USB-Seite und reicht alles über die interne UART
(D0/D1, deshalb im Tool „reserviert“) an den Haupt-MCU weiter; Konfiguration und Pins gehören zum Haupt-MCU.
Ab Werk sind keine Eingänge und kein Strip eingerichtet (außer Breakout 32U2).

## Schnellstart

1. **Firmware flashen** – am einfachsten mit dem [Online-Flasher](https://cyclemat.github.io/lwcloneu2/flash.html)
   in Edge/Chrome (Board wählen, Port wählen, fertig; Details im nächsten Abschnitt). Von Hand mit den `.hex`-Dateien
   aus [`release/`](release/):
   - Leonardo/Micro: Reset drücken, dann innerhalb von ~8 s
     `avrdude -c avr109 -p atmega32u4 -P COMx -U flash:w:arduino_leonardo.hex:i`
   - Mega 2560: `avrdude -c wiring -p atmega2560 -P COMx -b 115200 -U flash:w:arduino_mega2560__m2560.hex:i`,
     danach den 16U2 per DFU (Atmel FLIP / `dfu-programmer`) mit `arduino_mega2560__m16u2.hex`.
   - Uno: `avrdude -c arduino -p m328p -P COMx -U flash:w:arduino_uno__m328.hex:i`, danach den USB-Chip per DFU:
     Uno R3 (16U2) mit `arduino_uno__m16u2.hex`, Uno R1/R2 (8U2) mit `arduino_uno__m8u2.hex`.
2. **Konfigurieren:** [Konfigurations-Tool](https://cyclemat.github.io/lwcloneu2/config.html) in Microsoft Edge
   oder Google Chrome öffnen, „Verbinden“, Pins zuweisen, „Speichern“. Ausführlich weiter unten.
3. **LED-Wiz-Software:** `ledwiz.dll` (x86 für 32-Bit-Programme, x64 für 64-Bit) neben die Anwendung legen oder
   deren mitgelieferte `ledwiz.dll` ersetzen. Die LED-Wiz-ID (1–16) wird im Tool unter „Gerät“ gesetzt.

## Der Online-Flasher

<https://cyclemat.github.io/lwcloneu2/flash.html> – flasht die Firmware direkt aus dem Browser, ohne avrdude,
Atmel FLIP oder Arduino-IDE. Die Seite benutzt **Web Serial** (für die Arduino-Bootloader) und **WebUSB** (für die
DFU-Bootloader der USB-Chips); sie liegt im Ordner [`docs/`](docs/) und kann auch lokal geöffnet werden.
**Stand Beta:** die Abläufe sind nach den Protokollen von avrdude, Caterina, optiboot, stk500boot und dfu-programmer
umgesetzt, aber noch nicht an echter Hardware getestet – Rückmeldungen bitte als Issue mit dem Protokolltext der Seite.

### Voraussetzungen

- **Browser:** Microsoft Edge oder Google Chrome ab Version 89 (Desktop). Firefox und Safari können kein Web Serial /
  WebUSB.
- **DFU unter Windows** (Uno/Mega-USB-Chip, Breakout 32U2): der Browser braucht den **WinUSB-Treiber** für das
  DFU-Gerät – einmalig mit [Zadig](https://zadig.akeo.ie/): „Options → List All Devices“, „ATmega16U2 DFU“ (o. ä.)
  wählen, Treiber „WinUSB“ installieren. Atmel FLIP funktioniert danach nicht mehr, bis man den Treiber zurücksetzt.
- **Linux:** udev-Regel für die Bootloader, z. B. in `/etc/udev/rules.d/60-lwcloneu2.rules`:
  `SUBSYSTEM=="usb", ATTRS{idVendor}=="03eb", MODE="0666"` (Atmel DFU) und
  `SUBSYSTEM=="hidraw", ATTRS{idVendor}=="fafa", MODE="0666"` (LWCloneU2 selbst); der Benutzer gehört zur Gruppe
  `dialout` (serielle Ports).
- **macOS:** nichts weiter nötig.

### So geht es

1. **Board wählen** – Leonardo, Pro Micro/Micro, Uno R3 (16U2), Uno R1/R2 (8U2), Mega 2560 oder Breakout 32U2. Beim
   Uno steht der USB-Chip auf dem kleinen Chip neben der USB-Buchse („MEGA16U2“ oder „MEGA8U2“). Die Seite zeigt die
   nötigen Schritte für genau dieses Board.
2. **Leonardo / Pro Micro** (ein Chip, Bootloader Caterina): „Port wählen und flashen“, im Dialog den Arduino-Port
   wählen. Läuft noch ein Arduino-Sketch, startet die Seite den Bootloader über den 1200-Baud-Trick; danach erscheint
   der Port-Dialog erneut – jetzt das „… bootloader“-Gerät wählen (der Bootloader läuft nur 8 s). Läuft bereits
   LWCloneU2, zuerst unten „Bootloader per USB-HID starten“ (oder Reset doppelt drücken) und dann innerhalb von 8 s
   den Port wählen.
3. **Uno / Mega 2560** (zwei Chips, Reihenfolge wichtig):
   1. **Hauptchip** (328P / 2560) über den USB-Seriell-Wandler des Boards: „Port wählen und flashen“. Die Seite
      setzt das Board per DTR zurück und spricht optiboot (STK500 v1) bzw. den Mega-Bootloader (STK500 v2) an. Der
      USB-Chip muss dafür noch seine Original-Firmware haben.
   2. **USB-Chip** (16U2 / 8U2) per DFU: am 6-Pin-ICSP-Header neben der USB-Buchse die beiden Pins **RESET und GND**
      (zur Platinenkante hin) kurz brücken → das Board meldet sich als „ATmega16U2 DFU“. „DFU-Gerät wählen und
      flashen“. Danach gibt es keinen seriellen Port mehr – das Board ist jetzt ein LWCloneU2.
4. **Breakout 32U2:** mit gedrückter HWB-Taste anschließen (DFU-Modus), dann „DFU-Gerät wählen und flashen“.
5. Nach dem Flashen startet das Board neu und meldet sich als LWCloneU2 (HID). Weiter mit dem Konfigurations-Tool.

Die Seite löscht, schreibt und **prüft** den Flash (Verify) und bricht bei falschem Chip (Signatur bzw. DFU-PID) ab.
Unter „Eigene Hex-Datei“ lässt sich eine selbst gebaute oder fremde Firmware flashen – zum Beispiel die
Original-USB-Seriell-Firmware des Uno/Mega aus der Arduino-IDE (`hardware/arduino/avr/firmwares/atmegaxxu2/`), um
das Board wieder in einen normalen Arduino zu verwandeln.

**Fehlersuche:** „kein Port gewählt“ – Dialog abgebrochen oder der Port fehlt (Treiber, Kabel nur zum Laden?).
„keine Antwort vom Bootloader“ – falsches Board gewählt, Bootloader-Fenster verpasst (Leonardo: 8 s), oder ein
anderes Programm (Arduino-IDE, serieller Monitor) hält den Port. „falscher Chip“ – Board-Auswahl prüfen (Uno R3 vs.
R1/R2). DFU „Gerät nicht gefunden / Zugriff verweigert“ – Windows: WinUSB-Treiber per Zadig; Linux: udev-Regel.

## Das Konfigurations-Tool

Das Tool läuft unter <https://cyclemat.github.io/lwcloneu2/config.html> (oder lokal: [`docs/config.html`](docs/config.html)).
Es läuft komplett im Browser und spricht per **WebHID** direkt mit dem Board – über dieselbe USB-HID-Verbindung,
die auch die LED-Wiz-Software benutzt. Kein COM-Port, kein Treiber, keine Installation, nichts geht ins Internet.

### Voraussetzungen

- **Browser:** Microsoft Edge oder Google Chrome ab Version 89 (auch Opera, Brave). Firefox und Safari können kein
  WebHID. Der Ordner `docs/` darf auch lokal geöffnet werden (Doppelklick auf `config.html`), ohne Webserver.
- **Firmware v2.1 oder neuer** auf dem Board. Mit älterer Firmware meldet das Tool „Firmware zu alt“.
- **Windows:** nichts weiter nötig. **Linux:** der Benutzer braucht Schreibrecht auf das `hidraw`-Gerät, z. B. per
  udev-Regel `SUBSYSTEM=="hidraw", ATTRS{idVendor}=="fafa", MODE="0666"`. **macOS:** funktioniert ohne Vorbereitung.
- Die Sprache (DE/EN) wechselt der Knopf oben rechts; hell/dunkel folgt dem System.

### Verbinden

1. Board per USB anschließen. Auf „**Verbinden**“ klicken.
2. Der Browser zeigt die gefundenen HID-Geräte. **Windows listet dasselbe Board mehrfach** (eine Zeile je
   USB-Schnittstelle: LED-Wiz, Eingaben, Sonstiges) – es ist egal, welche Zeile gewählt wird, das Tool probiert alle
   durch und nimmt die, die auf den Konfigurationsbefehl antwortet.
3. Oben erscheinen **Board**, **Firmware**-Version, **LED-Wiz-ID** und die Anzahl der **Pins**. Die Konfiguration
   wird automatisch vom Gerät geladen.
4. „Trennen“ gibt das Gerät wieder frei. Beim Abziehen trennt das Tool von selbst.

### Grundprinzip: Laden – Anwenden – Speichern

Die Knöpfe unten gelten immer für die **gesamte** Konfiguration:

| Knopf | Wirkung |
|---|---|
| **Vom Gerät laden** | verwirft alle Änderungen im Tool und holt den aktuellen Stand aus dem Board |
| **Anwenden (sofort)** | schreibt die Konfiguration ins RAM des Boards und schaltet sie **sofort** scharf – zum Ausprobieren; nach einem Neustart ist der alte Stand wieder da |
| **Speichern (EEPROM)** | wie Anwenden, aber dauerhaft. Das Board prüft die Konfiguration vorher |

Rot umrandete Felder sind **Konflikte** (ein Pin doppelt belegt, Pixelbereich länger als der Strip, Kalibrierung
nicht Min < Mitte < Max). Solange ein Konflikt da ist, lassen sich „Anwenden“ und „Speichern“ nicht ausführen; die
Statuszeile sagt es. Lehnt das Board eine Konfiguration trotzdem ab („ungültig“), ist meist ein Pin gemeint, den das
Board nicht kann (z. B. Achse auf einem Pin ohne ADC) – siehe Fehlersuche.

### Reiter „Ausgänge“ – die 32 LED-Wiz-Ausgänge

Die Tabelle hat 32 Zeilen, Zeile 1 ist LED-Wiz-Ausgang 1 (in DOF/LedBlinky „Port 1“ dieses Geräts).

- **Pin:** welcher Pin des Boards diesen Ausgang treibt. „– keine –“ lässt den Ausgang unbenutzt (die Software kann
  ihn trotzdem schalten, er landet nur auf keinem Pin – im Spiegel-Modus darf er weiterhin Pixel steuern).
  Als „reserviert“ angezeigte Pins sind von der Firmware belegt (Mega/Uno: D0/D1 für die UART-Brücke).
- **Invertiert:** der Pin ist bei „aus“ auf 5 V und bei „an“ auf 0 V. Für LEDs/Relais, die gegen VCC geschaltet
  werden, oder für Treiberplatinen mit invertierendem Eingang.
- **Test:** schaltet nur diesen Ausgang mit voller Helligkeit ein; **Alle an / Alle aus** oben rechts entsprechend.
  Das sind echte LED-Wiz-Befehle – eine gleichzeitig laufende LED-Wiz-Software überschreibt sie wieder.

Hinweis zur Hardware: Ein AVR-Pin liefert maximal ~20 mA (alle Pins zusammen deutlich weniger). Für Lampen, Motoren,
Relais und Kontaktoren gehört eine Treiberstufe dazwischen (MOSFET-Modul, ULN2803, Relaisplatine). Die PWM der
Ausgänge liegt bei etwa 100 Hz mit 49 Stufen – für LEDs und Lampen gut, für Motoren hörbar.

### Reiter „Eingänge“ – Taster, Joystick-Richtungen, Drehgeber

Jede Zeile ist ein Eingang (Standard 32, Breakout 32U2: 16).

- **Pin:** Taster zwischen Pin und **GND**. Der interne Pull-up ist aktiv, kein externer Widerstand nötig.
- **Funktion:** was beim Drücken passiert. Die Liste ist gruppiert:
  - *Taste* – Tastaturtasten (A–Z, 0–9, F1–F12, Pfeile, Enter, Esc, Leertaste, Ziffernblock …), dazu die
    Kombinationen „Alt+F4“ und „Shift+F7“.
  - *Modifier* – Strg, Shift, Alt, GUI (Windows-Taste) links/rechts. Mehrere Eingänge gleichzeitig ergeben
    Kombinationen (z. B. ein Eingang „LeftCtrl“, ein anderer „C“ → Strg+C).
  - *Medien* – Lauter, Leiser, Stumm.
  - *Maus* – Maustasten links/rechts/Mitte.
  - *Joystick 1–4* – Links/Rechts/Hoch/Runter und Taste 1–8 des jeweiligen Joysticks. Eine gedrückte digitale
    Richtung setzt die X- bzw. Y-Achse auf –127/+127 und überschreibt dabei eine Analog-Achse desselben Joysticks.
  - *Sonder* – **Shift** (siehe unten) und **Maus X/Y Takt/Richtung** für Drehgeber.
- **Mit Shift:** die zweite Funktion dieses Eingangs, solange ein als „Shift“ belegter Eingang gedrückt gehalten wird.
  Beispiel: Start-Taster = „1“ (Spieler 1), mit Shift = „Esc“ (Spiel beenden). Der Shift-Eingang selbst sendet
  nichts. Steht „Mit Shift“ auf „– keine –“, gilt immer die normale Funktion.
- **Aktiv high:** der Taster schaltet gegen **5 V** statt gegen GND; der Pull-up wird abgeschaltet, ein externer
  Pull-down-Widerstand (10 kΩ) ist nötig. Nur für fertige Module, die einen High-Pegel liefern.
- **Live:** der grüne Punkt zeigt den entprellten Zustand des Pins – ideal zum Prüfen der Verdrahtung, auch bevor die
  Funktion zugewiesen ist (Pin muss gesetzt und angewendet sein).

**Drehgeber / Trackball als Maus:** Für jede Achse zwei Eingänge belegen: einen mit „Maus X Takt“, den anderen mit
„Maus X Richtung“ (für Y entsprechend). Die beiden Encoder-Leitungen A/B kommen an diese Pins, Masse an GND.
Dreht es sich falsch herum, Takt und Richtung tauschen.

### Reiter „Achsen“ – analoge Joysticks, Potis, Hall-Sensoren

Bis zu 8 analoge Achsen; der Breakout 32U2 hat keinen ADC und zeigt diesen Reiter leer.

- **Pin:** nur ADC-fähige Pins werden angeboten (Leonardo: A0–A5, D4, D6, D8, D9, D10, D12). Der Sensor liefert
  0–5 V zwischen GND und dem Pin (Poti: Schleifer an den Pin, Enden an 5 V und GND).
- **Joystick** und **Achse** (X, Y, Z, Rz): auf welchem der 4 Joysticks und welcher Achse der Wert ausgegeben wird.
  Jeder Joystick erscheint in Windows als eigenes Gamepad mit 4 Achsen und 8 Tasten.
- **Live:** der Balken zeigt den Rohwert (0–1023) des ADC, der blaue Strich die eingestellte Mitte, der rote Bereich
  die Totzone. Rechts der Rohwert als Zahl.
- **Ausgabe:** der Wert, der tatsächlich zum PC geht (–127 … +127), bereits mit Kalibrierung, Totzone und Invertierung.

**Kalibrieren in drei Schritten** (jeweils den Knopf „übernehmen“ neben dem Feld drücken, der Live-Wert wird
eingetragen):

1. Joystick **loslassen** (Ruhelage) → „übernehmen“ bei **Mitte**.
2. Ganz in die eine Richtung drücken → „übernehmen“ bei **Min**.
3. Ganz in die andere Richtung → „übernehmen“ bei **Max**. Zeigt „Ausgabe“ dann das falsche Vorzeichen, **Invertiert**
   einschalten (statt Min und Max zu tauschen – Min muss kleiner als Max bleiben).

**Totzone** (0–100 %): Innerhalb dieses Bereichs um die Mitte ist die Ausgabe exakt 0 – gegen Zittern und Drift.
Außerhalb wird der Rest wieder auf den vollen Weg gestreckt, es geht also kein Ausschlag verloren. 5–10 % sind ein
guter Start; bei Hall-Sensoren reicht oft weniger.

Die Felder Min/Mitte/Max lassen sich auch von Hand eintippen. Der Wert gilt nach „Anwenden“ bzw. „Speichern“.

### Reiter „LED-Strip“ – WS2811 / WS2812

**Verdrahtung:** Datenleitung (DIN) des Strips an den gewählten **Daten-Pin**, GND des Strips an GND des Boards.
Ein Widerstand von 300–500 Ω in der Datenleitung und ein Kondensator (≥ 100 µF) an der Strip-Versorgung schützen
den ersten Pixel. Die 5 V für den Strip **nicht** über den Arduino ziehen – ab ca. 10 Pixeln ein eigenes Netzteil
(60 mA je Pixel bei Weiß). Die Boards laufen mit 5 V, der Datenpegel passt direkt (keine Pegelwandlung nötig).
Maximal 64 Pixel (Leonardo, Pro Micro, Uno), 128 (Mega 2560), 8 (Breakout 32U2).

**Einstellungen:**

- **Daten-Pin:** ein freier Pin; er darf nicht gleichzeitig Ausgang, Eingang oder Achse sein.
- **Pixel:** Anzahl der LEDs am Strip (0 = Strip aus).
- **Farbreihenfolge:** GRB für WS2812/WS2812B (Standard), RGB für die meisten WS2811-Module, BRG für einige.
  Stimmt die Farbe im Test nicht (Rot wird Grün), hier umstellen.
- **Betriebsart:**
  - **Spiegel (folgt den Ausgängen)** – für DOF, LedBlinky & Co. Die Tabelle „Spiegel-Einträge“ erscheint.
  - **Direkt (PC setzt Pixel)** – für eigene Programme. Die Testknöpfe erscheinen.

**Spiegel-Einträge** (bis zu 32, Breakout 32U2: 8). Jede Zeile sagt: *wenn LED-Wiz-Ausgang X leuchtet, dann leuchten
die Pixel ab … (Anzahl …) in dieser Farbe*, und zwar mit der Helligkeit des Ausgangs (inklusive Pulsmustern).

- **Ausgang:** welcher der 32 LED-Wiz-Ausgänge gespiegelt wird; „– keine –“ schaltet die Zeile ab. Der Ausgang darf
  zusätzlich einen echten Pin haben – oder keinen, dann steuert er nur Pixel.
- **Pixel ab / Anzahl:** der Bereich auf dem Strip (erster Pixel ist 0).
- **Farbe:** per Farbwähler. Mehrere Zeilen dürfen denselben Ausgang (z. B. links und rechts) oder dieselben Pixel
  nutzen – bei Überlappung gewinnt je Farbkanal der hellere Wert, Rot + Blau ergibt also Magenta.
- **1:1 vorbelegen:** Zeile n → Ausgang n → Pixel n, weiß. **Alle leeren** setzt alle Zeilen auf „– keine –“.

Beispiel Flipper-Unterbodenbeleuchtung: In DOF Ausgang 20 als „Undercab“ konfigurieren, hier Zeile 1 = Ausgang 20,
Pixel ab 0, Anzahl 30, Farbe Orange. Zum Prüfen im Reiter „Ausgänge“ bei Ausgang 20 auf „Test“ drücken.

**Test (Direkt-Modus):** Die Knöpfe wirken auf die im Board **aktive** Konfiguration – nach dem Umstellen von
Pin/Pixeln/Betriebsart also zuerst „Anwenden“. Die Statuszeile rechts sagt, ob das Board gerade einen Strip aktiv hat.
- **Alle füllen** mit der gewählten Farbe, **Alle aus**, **Pixel setzen** (Nummer links eintragen), **Lauflicht**
  (ein Pixel läuft durch; derselbe Knopf stoppt es).

**Eigene Programme (Direkt-Modus):** Die Befehle sind 8-Byte-Reports auf der LED-Wiz-Schnittstelle, also mit WebHID,
`hidapi` oder `LWZ_RAWWRITE`/`LWZ_RAWREAD` der `ledwiz.dll` nutzbar:

| Byte 0 | Befehl | Bytes 1–7 | Antwort |
|---|---|---|---|
| `0xB0` | STRIP_INFO | – | `[B0, Pixel aktiv, Pixel max, Betriebsart, Farbreihenfolge, 0, 0, 0]` |
| `0xB1` | STRIP_SET | `idx, r, g, b, r, g, b` – zwei Pixel ab `idx` in den Puffer | keine |
| `0xB2` | STRIP_FILL | `idx, n, r, g, b` – `n` Pixel ab `idx` füllen, `n = 0` bis zum Ende | keine |
| `0xB3` | STRIP_SHOW | – Puffer an den Strip senden | `[B3, 0]` **nachdem** der Strip geschrieben ist |

Ablauf: beliebig viele SET/FILL, dann SHOW, dann auf die Antwort warten, dann der nächste Frame. Das Warten ist wichtig:
Während der Strip geschrieben wird, sind Interrupts gesperrt (~30 µs je Pixel), und beim Mega/Uno kann die interne
UART in dieser Zeit Reports verlieren. Richtwert: 64 Pixel ≈ 32 Reports ≈ 30–40 Bilder/s.

### LED-Strip direkt aus DOF ansteuern (Leonardo / Pro Micro)

Das DirectOutput Framework kennt für adressierbare Strips nur serielle Controller (TeensyStripController,
Wemos). Deshalb meldet sich der LWCloneU2 auf dem **ATmega32U4** zusätzlich als **virtueller COM-Port** („USB
Serial Device“, Windows 10/11 ohne Treiber) und spricht dort das Protokoll des TeensyStripController mit einem Kanal.
DOF braucht keine Änderung – in der `Cabinet.xml`:

```xml
<OutputControllers>
  <TeensyStripController>
    <Name>LWCloneU2 Strip</Name>
    <ComPortName>COM7</ComPortName>          <!-- Port des LWCloneU2, siehe Geräte-Manager -->
    <NumberOfLedsStrip1>60</NumberOfLedsStrip1>
  </TeensyStripController>
</OutputControllers>
<Toys>
  <LedStrip>
    <Name>Undercab</Name>
    <OutputControllerName>LWCloneU2 Strip</OutputControllerName>
    <Width>60</Width><Height>1</Height>
    <FirstLedNumber>1</FirstLedNumber>
    <ColorOrder>RGB</ColorOrder>            <!-- Reihenfolge stellt das Tool ein, DOF nichts umsortieren lassen -->
  </LedStrip>
</Toys>
```

Voraussetzungen: im Tool Daten-Pin und Pixel gesetzt, Betriebsart **Direkt**, „Speichern“. `NumberOfLedsStrip1` darf
die Pixelzahl des Boards nicht überschreiten (DOF fragt die Grenze ab und setzt die Länge selbst). Nur Strip 1 wird
bedient; `ColorOrder` in DOF auf `RGB` lassen und die echte Reihenfolge im Tool wählen, sonst wird doppelt gedreht.
Die Antwort auf den Ausgabe-Befehl kommt erst, wenn der Strip geschrieben ist – DOF wartet darauf, so geht nichts
verloren. Uno, Mega und Breakout haben dafür keine freien USB-Endpunkte; dort bleibt der Spiegel-Modus, der mit DOF
über die LED-Wiz-Ausgänge ohnehin funktioniert.

### Reiter „Gerät“

- **LED-Wiz-ID (1–16):** unter welcher Nummer das Board in der LED-Wiz-Software erscheint. Mehrere Boards an einem PC
  brauchen verschiedene IDs. Nach „ID setzen“ startet das Board neu und muss erneut verbunden werden.
- **Sicherung:** **Als JSON exportieren** speichert die komplette Belegung in eine Datei (z. B. vor Firmware-Updates
  oder für ein zweites Board gleichen Typs). **JSON importieren** lädt sie ins Tool – danach „Speichern“ nicht
  vergessen. Die Datei passt nur zum gleichen Board-Typ.
- **Werkseinstellungen laden:** setzt das Tool und das RAM des Boards auf die Standardbelegung (siehe Tabelle oben);
  dauerhaft erst mit „Speichern“.
- **Bootloader starten:** trennt das Board vom USB und wartet auf eine neue Firmware (Leonardo/Micro: `avrdude`
  per COM-Port innerhalb von 8 s; Breakout 32U2: Atmel FLIP/`dfu-programmer`). Zurück zum Normalbetrieb: Reset
  drücken oder neu einstecken.

### Fehlersuche

| Symptom | Ursache / Abhilfe |
|---|---|
| „Dieser Browser unterstützt WebHID nicht“ | Edge oder Chrome verwenden; in Firefox/Safari geht es nicht |
| Im Gerätedialog erscheint nichts | Firmware nicht geflasht oder noch v1; Kabel nur Ladekabel?; beim Mega/Uno beide Chips geflasht? |
| „Kein LWCloneU2 mit Konfigurationsprotokoll gefunden“ | Firmware v1 oder Original-LED-Wiz; Firmware v2.1 flashen |
| „Firmware zu alt (Protokoll 1)“ | Firmware 2.0 → auf 2.1 aktualisieren (Konfigurations-Layout hat sich geändert) |
| „Keine Antwort vom Gerät“ | Board neu einstecken, Seite neu laden; eine andere Anwendung darf das Gerät offen haben, stört aber manchmal – LED-Wiz-Software kurz schließen |
| „Gerät hat die Konfiguration abgelehnt“ | Konflikt, den das Tool nicht kennt: Achse auf Pin ohne ADC, mehr Pixel als das Board kann, reservierter Pin. Mit „Vom Gerät laden“ zurück zum letzten gültigen Stand |
| Ausgang schaltet nicht | richtiger Pin? „Test“ im Tool; mit „Invertiert“ probieren; Treiberstufe und gemeinsame Masse prüfen |
| Taster reagiert nicht / Live-Punkt bleibt aus | Taster gegen GND verdrahtet? Konfiguration angewendet? Pin nicht doppelt belegt? |
| Joystick zittert in Ruhelage | Totzone erhöhen, Mitte neu „übernehmen“ |
| Strip bleibt dunkel | „Anwenden“ nach dem Einstellen; Daten-Pin, GND und Strip-Versorgung prüfen; DIN-Seite des Strips (Pfeile) |
| Strip leuchtet in falschen Farben | Farbreihenfolge umstellen (GRB ↔ RGB) |
| Ausgangs-PWM flackert, wenn der Strip läuft | normal bei langen Strips (Interrupts gesperrt während der Übertragung); Pixelzahl verringern |

## ledwiz.dll

`win32/bin/x86/ledwiz.dll` (für 32-Bit-Programme wie die meisten Pinball-Frontends/DOF-Builds) bzw.
`win32/bin/x64/ledwiz.dll`. Die DLL ist API-kompatibel zum Original (`LWZ_SBA`, `LWZ_PBA`, `LWZ_REGISTER`,
`LWZ_SET_NOTIFY`, `LWZ_SET_NOTIFY_EX`, `LWZ_RAWWRITE`, `LWZ_RAWREAD`), schreibt asynchron und blockiert die
Anwendung nicht. Sie benötigt Windows 10 oder neuer (Universal C Runtime). `testapp.exe` schaltet zum Prüfen alle
Ausgänge ein, `lwcconfig.exe` setzt die LED-Wiz-ID per Kommandozeile.

## Selbst bauen

### Firmware
Benötigt `avr-gcc`, `avr-libc` (Linux: `apt install gcc-avr avr-libc`; Windows: z. B. die Toolchain aus PlatformIO
`~/.platformio/packages/toolchain-atmelavr/bin` in den PATH, Aufruf aus der Git-Bash).

```sh
cd firmware
./build.sh                # alle Boards ohne make (nur avr-gcc); Ergebnis: firmware/build/*.hex, Protokoll build.log
./build.sh leonardo       # ein Board (leonardo, promicro, breakout_32u2, mega2560, uno)
./publish.sh              # Hex-Dateien + manifest.json nach release/ (daraus bedient sich der Online-Flasher)
make                      # alternativ mit den LUFA-Makefiles (Linux, make nötig); Ergebnis: <board>/<board>.hex
```

### Windows-DLL und Tools
```sh
cmake -S win32 -B build-x86 -A Win32 && cmake --build build-x86 --config Release   # MSVC
cmake -S win32 -B build-x64 -A x64   && cmake --build build-x64 --config Release
# oder mit LLVM-MinGW / MinGW-w64:
cmake -S win32 -B build-x86 -G Ninja -DCMAKE_CXX_COMPILER=i686-w64-mingw32-clang++ && cmake --build build-x86
```
Ergebnis in `win32/bin/x86` bzw. `win32/bin/x64`.

## Aufbau des Codes

```
firmware/
  main_usb.c        USB-MCU: LUFA, Reports entgegennehmen, Antworten/Panel-Reports senden (oder UART-Brücke)
  main_led.c        Haupt-MCU hinter einer Brücke (Mega 2560 / Uno)
  proto.c/.h        USB-Befehle: LED-Wiz (SBA/PBA) + Konfiguration 0xA0–0xA7 + Strip 0xB0–0xB3
  config.c/.h       Konfigurationsstruktur, EEPROM, Prüfung   ← Datenlayout, das Tool spiegelt es
  pins.c/.h         Pin-Index → Port/Bit/ADC je Board (BOARD_PIN_TABLE in <board>/pinmap.h)
  outputs.c/.h      32 Ausgänge: LED-Wiz-Profile, Soft-PWM auf GPIO (Grundcode, wie Original)
  strip.c/.h        WS2811/WS2812-Strip: Direkt-Modus (USB-Befehle) und Spiegel-Modus (folgt den Ausgängen)
  strip_serial.c/.h virtueller COM-Port (CDC) mit DOF-TeensyStripController-Protokoll (nur 32U4, ENABLE_STRIP_SERIAL)
  ws2812.c/.h       Bit-Bang-Treiber 800 kHz (8/16 MHz)
  panel.c/.h        Eingänge: Entprellung, Shift, Encoder, Analog-Achsen, HID-Reports
  descriptors.c/.h  USB-Deskriptoren (Misc, Panel, LED-Wiz)
  <board>/          hwconfig.h (Timer, UART, ADC), pinmap.h (Pin-Tabelle, Werksbelegung), devconfig.h, makefile
  lufa/             LUFA 130901 (gekürzt)
win32/
  driver/           ledwiz.dll (ledwiz.cpp, usbdev.cpp)
  testapp/, lwcconfig/
docs/               Webseite (GitHub Pages: https://cyclemat.github.io/lwcloneu2/)
  index.html        Startseite
  flash.html        Online-Flasher (Web Serial: AVR109/Caterina, STK500 v1/v2; WebUSB: Atmel DFU)
  config.html       Konfigurations-Tool (WebHID)
  style.css         gemeinsames Aussehen
release/            fertige .hex-Dateien + manifest.json (von firmware/publish.sh) – Quelle des Online-Flashers
old/                unveränderter Original-Stand von cithraidt/lwcloneu2 (Aug. 2023), nur zum Nachschlagen
```

Ein neues Board anlegen: Ordner mit `hwconfig.h` (Timer 0 = Soft-PWM, Timer 1 = Uhr, ADC, UART), `pinmap.h`
(`BOARD_ID`, `BOARD_PIN_TABLE`, `WS2812_MAX_PIXELS`, Werksbelegung), `devconfig.h`, `makefile`; Board-ID in
`pins.h` und Pin-Namen im Tool (`BOARDS`) ergänzen. Protokoll: [`firmware/proto.h`](firmware/proto.h),
Datenlayout: [`firmware/config.h`](firmware/config.h).

## Lizenz
GPL v2 oder neuer (siehe `LICENSE`). LUFA: eigene Lizenz (`firmware/lufa/LUFA/License.txt`).
Eingabe-Logik nach dem V-USB Mame Panel von Andreas Oberdorfer (GPLv2).

---

# English

LED-Wiz compatible USB output controller (32 outputs with soft PWM) **plus** input encoder (keyboard, up to
4 joysticks with analog axes, mouse) **plus** WS2811/WS2812 LED strip – for Arduino boards with an Atmel AVR USB chip.
Made for MAME/pinball cabinets: one board for lighting and controls, configured in the browser.

Firmware **v2.1** – fork of [cithraidt/lwcloneu2](https://github.com/cithraidt/lwcloneu2) (GPLv2).
Status: **beta** – compiled and reviewed, not yet tested on real hardware.

**Web page: <https://cyclemat.github.io/lwcloneu2/>** – flash the firmware and set up the board in the browser
(Microsoft Edge or Google Chrome, nothing to install).

## Features

- **32 LED-Wiz outputs** with 48 brightness levels and the original's four pulse patterns. Any LED-Wiz software
  (DOF, LedBlinky, PinballX, …) sees a regular LED-Wiz.
- **Pins assignable at runtime, no recompiling.** Which pin is an output, a button, an analog axis or the strip data
  line is stored in EEPROM and set in the browser – even while running ("Apply").
- **Inputs:** keys, modifiers, media keys, mouse buttons, joystick directions and buttons for up to 4 joysticks, a
  shift key for double assignments, rotary encoders/trackball as mouse.
- **Analog axes** (up to 8) with calibration (min/center/max), **dead zone** and inversion.
- **WS2811/WS2812 strip** on an extra pin as a separate module that leaves the 32 outputs untouched.
  **Mirror mode:** pixel ranges light up in a color, brightness follows an LED-Wiz output (works with DOF right away).
  **Direct mode:** your own programs set the pixels via USB; on Leonardo/Pro Micro also as a virtual COM port speaking
  the DOF TeensyStripController protocol – DOF then drives the strip directly.
- `ledwiz.dll` replacement (32/64 bit), asynchronous, does not block the application.

## Supported boards

| Board | MCU | Pins in the tool | Analog axes | Factory outputs 1–n | Note |
|---|---|---|---|---|---|
| Arduino Leonardo | ATmega32U4 | D0–D17, A0–A5, TXLED | yes (A0–A5, D4, D6, D8, D9, D10, D12) | D2–D13, A0–A5 (18) | recommended |
| Pro Micro / Arduino Micro | ATmega32U4 | like Leonardo, fewer pins exposed | yes | D2–D10, D14–D16, A0–A3 (16) | |
| Arduino Mega 2560 | ATmega2560 + ATmega16U2 | D2–D53, A0–A15 | yes (16) | D22–D53 (32) | flash **both** MCUs |
| Arduino Uno R2/R3 | ATmega328P + ATmega8U2/16U2 | D2–D13, A0–A5 | yes (6) | D2–D13, A0–A5 (18) | flash both MCUs |
| Breakout ATmega32U2 | ATmega32U2 | 21 pins (PB0–7, PC2, PC4–7, PD0–7) | no | like the old firmware | 1 KB RAM: 16 inputs, 8 pixels |

On the Mega/Uno the small USB chip (16U2/8U2) only handles USB and forwards everything over the internal UART
(D0/D1, hence "reserved" in the tool) to the main MCU, which owns configuration and pins. From the factory no inputs
and no strip are configured (except on the Breakout 32U2).

## Quick start

1. **Flash the firmware** – easiest with the [web flasher](https://cyclemat.github.io/lwcloneu2/flash.html) in
   Edge/Chrome (pick the board, pick the port, done; details in the next section). By hand with the `.hex` files
   from [`release/`](release/):
   - Leonardo/Micro: press reset, then within ~8 s
     `avrdude -c avr109 -p atmega32u4 -P COMx -U flash:w:arduino_leonardo.hex:i`
   - Mega 2560: `avrdude -c wiring -p atmega2560 -P COMx -b 115200 -U flash:w:arduino_mega2560__m2560.hex:i`, then
     the 16U2 via DFU (Atmel FLIP / `dfu-programmer`) with `arduino_mega2560__m16u2.hex`.
   - Uno: `avrdude -c arduino -p m328p -P COMx -U flash:w:arduino_uno__m328.hex:i`, then the USB chip via DFU:
     Uno R3 (16U2) with `arduino_uno__m16u2.hex`, Uno R1/R2 (8U2) with `arduino_uno__m8u2.hex`.
2. **Configure:** open the [configuration tool](https://cyclemat.github.io/lwcloneu2/config.html) in Microsoft Edge
   or Google Chrome, "Connect", assign pins, "Save". Details further below.
3. **LED-Wiz software:** put `ledwiz.dll` (x86 for 32-bit programs, x64 for 64-bit) next to the application or
   replace its bundled `ledwiz.dll`. The LED-Wiz id (1–16) is set in the tool under "Device".

## The web flasher

<https://cyclemat.github.io/lwcloneu2/flash.html> – flashes the firmware straight from the browser, without avrdude,
Atmel FLIP or the Arduino IDE. The page uses **Web Serial** (for the Arduino bootloaders) and **WebUSB** (for the DFU
bootloaders of the USB chips); it lives in [`docs/`](docs/) and can also be opened locally.
**Beta:** the procedures follow the protocols of avrdude, Caterina, optiboot, stk500boot and dfu-programmer but have
not been tested on real hardware yet – please report back via an issue including the page's log text.

### Requirements

- **Browser:** Microsoft Edge or Google Chrome 89 or newer (desktop). Firefox and Safari have no Web Serial / WebUSB.
- **DFU on Windows** (Uno/Mega USB chip, breakout 32U2): the browser needs the **WinUSB driver** for the DFU device –
  once, with [Zadig](https://zadig.akeo.ie/): "Options → List All Devices", pick "ATmega16U2 DFU" (or similar),
  install the "WinUSB" driver. Atmel FLIP will no longer work until you revert the driver.
- **Linux:** udev rules for the bootloaders, e.g. in `/etc/udev/rules.d/60-lwcloneu2.rules`:
  `SUBSYSTEM=="usb", ATTRS{idVendor}=="03eb", MODE="0666"` (Atmel DFU) and
  `SUBSYSTEM=="hidraw", ATTRS{idVendor}=="fafa", MODE="0666"` (LWCloneU2 itself); the user is in the `dialout` group
  (serial ports).
- **macOS:** nothing else needed.

### Step by step

1. **Pick the board** – Leonardo, Pro Micro/Micro, Uno R3 (16U2), Uno R1/R2 (8U2), Mega 2560 or breakout 32U2. On
   the Uno the USB chip is printed on the small chip next to the USB socket ("MEGA16U2" or "MEGA8U2"). The page shows
   the steps for exactly that board.
2. **Leonardo / Pro Micro** (one chip, Caterina bootloader): "Choose port and flash", pick the Arduino port. If an
   Arduino sketch is still running, the page starts the bootloader with the 1200 baud touch; the port dialog then
   appears again – now pick the "… bootloader" device (the bootloader only runs for 8 s). If LWCloneU2 is already
   running, first use "Start the bootloader via USB HID" below (or double-tap reset) and pick the port within 8 s.
3. **Uno / Mega 2560** (two chips, order matters):
   1. **Main chip** (328P / 2560) through the board's USB-serial converter: "Choose port and flash". The page resets
      the board via DTR and talks to optiboot (STK500 v1) or the Mega bootloader (STK500 v2). The USB chip must still
      have its original firmware for this.
   2. **USB chip** (16U2 / 8U2) via DFU: on the 6-pin ICSP header next to the USB socket, briefly bridge the two pins
      **RESET and GND** (towards the board edge) → the board shows up as "ATmega16U2 DFU". "Choose DFU device and
      flash". After that there is no serial port any more – the board is now an LWCloneU2.
4. **Breakout 32U2:** connect with the HWB button held (DFU mode), then "Choose DFU device and flash".
5. After flashing the board restarts and shows up as LWCloneU2 (HID). Continue with the configuration tool.

The page erases, writes and **verifies** the flash and aborts on a wrong chip (signature or DFU PID). "Own hex file"
flashes a self-built or third-party firmware – for example the original USB-serial firmware of the Uno/Mega from the
Arduino IDE (`hardware/arduino/avr/firmwares/atmegaxxu2/`) to turn the board back into a normal Arduino.

**Troubleshooting:** "no port chosen" – dialog cancelled or the port is missing (driver, charge-only cable?).
"no reply from bootloader" – wrong board selected, bootloader window missed (Leonardo: 8 s), or another program
(Arduino IDE, serial monitor) holds the port. "wrong chip" – check the board selection (Uno R3 vs. R1/R2). DFU
"device not found / access denied" – Windows: WinUSB driver via Zadig; Linux: udev rule.

## The configuration tool

The tool runs at <https://cyclemat.github.io/lwcloneu2/config.html> (or locally: [`docs/config.html`](docs/config.html)).
It runs entirely in the browser and talks to the board via **WebHID** – over the same USB HID connection the LED-Wiz
software uses. No COM port, no driver, no installation, nothing goes to the internet.

### Requirements

- **Browser:** Microsoft Edge or Google Chrome 89 or newer (Opera and Brave work too). Firefox and Safari have no
  WebHID. The `docs/` folder can also be opened locally (double-click `config.html`), no web server needed.
- **Firmware v2.1 or newer** on the board. With older firmware the tool reports "Firmware too old".
- **Windows:** nothing else needed. **Linux:** the user needs write access to the `hidraw` device, e.g. a udev rule
  `SUBSYSTEM=="hidraw", ATTRS{idVendor}=="fafa", MODE="0666"`. **macOS:** works out of the box.
- The button at the top right switches the language (DE/EN); light/dark follows the system.

### Connecting

1. Plug in the board. Click "**Connect**".
2. The browser lists the HID devices it found. **Windows lists the same board several times** (one row per USB
   interface: LED-Wiz, inputs, misc) – it does not matter which row you pick, the tool tries them all and keeps the
   one that answers the configuration command.
3. The header shows **board**, **firmware** version, **LED-Wiz id** and the number of **pins**. The configuration is
   loaded from the device automatically.
4. "Disconnect" releases the device. Unplugging disconnects automatically.

### Basics: load – apply – save

The buttons at the bottom always act on the **whole** configuration:

| Button | Effect |
|---|---|
| **Load from device** | discards all changes in the tool and fetches the current state from the board |
| **Apply (now)** | writes the configuration to the board's RAM and activates it **immediately** – for trying things out; after a restart the old state is back |
| **Save (EEPROM)** | like apply, but permanent. The board validates the configuration first |

Fields with a red outline are **conflicts** (a pin used twice, a pixel range longer than the strip, calibration not
min < center < max). While a conflict exists, "Apply" and "Save" refuse to run; the status line says so. If the board
still rejects a configuration ("invalid"), it is usually a pin the board cannot do (e.g. an axis on a pin without
ADC) – see troubleshooting.

### "Outputs" tab – the 32 LED-Wiz outputs

The table has 32 rows, row 1 is LED-Wiz output 1 ("port 1" of this device in DOF/LedBlinky).

- **Pin:** which board pin drives this output. "– none –" leaves the output unused (software can still switch it,
  it just reaches no pin – in mirror mode it may still drive pixels). Pins shown as "reserved" are used by the
  firmware (Mega/Uno: D0/D1 for the UART bridge).
- **Inverted:** the pin is at 5 V when "off" and 0 V when "on". For LEDs/relays switched to VCC or for driver boards
  with inverting inputs.
- **Test:** switches only this output on at full brightness; **All on / All off** at the top right accordingly. These
  are real LED-Wiz commands – LED-Wiz software running at the same time overrides them again.

Hardware note: an AVR pin sources at most ~20 mA (all pins together much less). Lamps, motors, relays and contactors
need a driver stage (MOSFET module, ULN2803, relay board). The output PWM runs at about 100 Hz with 49 steps – fine
for LEDs and lamps, audible on motors.

### "Inputs" tab – buttons, joystick directions, rotary encoders

Each row is one input (32 by default, Breakout 32U2: 16).

- **Pin:** button between the pin and **GND**. The internal pull-up is active, no external resistor needed.
- **Function:** what happens when pressed. The list is grouped:
  - *Key* – keyboard keys (A–Z, 0–9, F1–F12, arrows, Enter, Esc, Space, keypad …) plus the combinations "Alt+F4"
    and "Shift+F7".
  - *Modifier* – Ctrl, Shift, Alt, GUI (Windows key) left/right. Several inputs at once form combinations (e.g. one
    input "LeftCtrl", another "C" → Ctrl+C).
  - *Media* – volume up, volume down, mute.
  - *Mouse* – mouse buttons left/right/middle.
  - *Joystick 1–4* – left/right/up/down and button 1–8 of that joystick. A pressed digital direction sets the X or Y
    axis to –127/+127 and overrides an analog axis of the same joystick while held.
  - *Special* – **Shift** (see below) and **Mouse X/Y clock/direction** for rotary encoders.
- **With shift:** the second function of this input while an input assigned to "Shift" is held. Example: start
  button = "1" (player 1), with shift = "Esc" (quit game). The shift input itself sends nothing. With "– none –" the
  normal function always applies.
- **Active high:** the button switches to **5 V** instead of GND; the pull-up is disabled and an external pull-down
  resistor (10 kΩ) is required. Only for modules that provide a high level.
- **Live:** the green dot shows the debounced state of the pin – ideal for checking the wiring, even before a
  function is assigned (pin must be set and applied).

**Rotary encoder / trackball as mouse:** assign two inputs per axis: one "Mouse X clock", the other "Mouse X
direction" (same for Y). The encoder lines A/B go to these pins, ground to GND. If it turns the wrong way, swap clock
and direction.

### "Axes" tab – analog joysticks, pots, Hall sensors

Up to 8 analog axes; the Breakout 32U2 has no ADC and shows this tab empty.

- **Pin:** only ADC capable pins are offered (Leonardo: A0–A5, D4, D6, D8, D9, D10, D12). The sensor delivers 0–5 V
  between GND and the pin (pot: wiper to the pin, ends to 5 V and GND).
- **Joystick** and **axis** (X, Y, Z, Rz): on which of the 4 joysticks and which axis the value is reported. Each
  joystick appears in Windows as its own gamepad with 4 axes and 8 buttons.
- **Live:** the bar shows the raw ADC value (0–1023), the blue line the configured center, the red area the dead zone.
  The raw value is printed to the right.
- **Output:** the value actually sent to the PC (–127 … +127), with calibration, dead zone and inversion applied.

**Calibrating in three steps** (press "set" next to the field each time, the live value is copied):

1. **Release** the joystick (rest position) → "set" at **Center**.
2. Push it fully in one direction → "set" at **Min**.
3. Fully in the other direction → "set" at **Max**. If "Output" then has the wrong sign, enable **Inverted**
   (instead of swapping min and max – min has to stay smaller than max).

**Dead zone** (0–100 %): inside this range around the center the output is exactly 0 – against jitter and drift.
Outside, the remaining travel is stretched back to the full range, so no deflection is lost. 5–10 % is a good start;
Hall sensors often need less.

Min/center/max can also be typed in. Values take effect after "Apply" or "Save".

### "LED strip" tab – WS2811 / WS2812

**Wiring:** strip data line (DIN) to the chosen **data pin**, strip GND to board GND. A 300–500 Ω resistor in the data
line and a capacitor (≥ 100 µF) across the strip supply protect the first pixel. Do **not** power the strip from the
Arduino – from about 10 pixels on use a separate supply (60 mA per pixel at white). The boards run at 5 V, so the data
level matches directly (no level shifter). Maximum 64 pixels (Leonardo, Pro Micro, Uno), 128 (Mega 2560),
8 (Breakout 32U2).

**Settings:**

- **Data pin:** a free pin; it must not be an output, input or axis at the same time.
- **Pixels:** number of LEDs on the strip (0 = strip off).
- **Color order:** GRB for WS2812/WS2812B (default), RGB for most WS2811 modules, BRG for some. If the test shows the
  wrong color (red comes out green), change it here.
- **Mode:**
  - **Mirror (follows the outputs)** – for DOF, LedBlinky & co. The "Mirror entries" table appears.
  - **Direct (host sets pixels)** – for your own programs. The test buttons appear.

**Mirror entries** (up to 32, Breakout 32U2: 8). Each row says: *when LED-Wiz output X is lit, the pixels from …
(count …) light up in this color*, with the brightness of the output (pulse patterns included).

- **Output:** which of the 32 LED-Wiz outputs is mirrored; "– none –" disables the row. The output may also have a
  real pin – or none, then it only drives pixels.
- **First pixel / Count:** the range on the strip (the first pixel is 0).
- **Color:** via color picker. Several rows may use the same output (e.g. left and right) or the same pixels – on
  overlap the brighter value wins per channel, so red + blue gives magenta.
- **Preset 1:1:** row n → output n → pixel n, white. **Clear all** sets every row to "– none –".

Example undercab lighting: configure output 20 as "Undercab" in DOF, here row 1 = output 20, first pixel 0, count 30,
color orange. To check, press "Test" at output 20 on the "Outputs" tab.

**Test (direct mode):** the buttons act on the configuration **active** in the board – after changing pin/pixels/mode
press "Apply" first. The status text on the right tells whether the board currently has an active strip.
- **Fill all** with the chosen color, **All off**, **Set pixel** (enter the number on the left), **Chase** (one pixel
  runs along the strip; the same button stops it).

**Your own programs (direct mode):** the commands are 8 byte reports on the LED-Wiz interface, usable with WebHID,
`hidapi` or `LWZ_RAWWRITE`/`LWZ_RAWREAD` of `ledwiz.dll`:

| Byte 0 | Command | Bytes 1–7 | Reply |
|---|---|---|---|
| `0xB0` | STRIP_INFO | – | `[B0, pixels active, pixels max, mode, color order, 0, 0, 0]` |
| `0xB1` | STRIP_SET | `idx, r, g, b, r, g, b` – two pixels from `idx` into the buffer | none |
| `0xB2` | STRIP_FILL | `idx, n, r, g, b` – fill `n` pixels from `idx`, `n = 0` to the end | none |
| `0xB3` | STRIP_SHOW | – send the buffer to the strip | `[B3, 0]` **after** the strip has been written |

Sequence: any number of SET/FILL, then SHOW, then wait for the reply, then the next frame. The wait matters: while the
strip is written interrupts are blocked (~30 µs per pixel), and on the Mega/Uno the internal UART can lose reports
during that time. Rule of thumb: 64 pixels ≈ 32 reports ≈ 30–40 frames/s.

### Driving the LED strip directly from DOF (Leonardo / Pro Micro)

The DirectOutput Framework only knows serial controllers for addressable strips (TeensyStripController, Wemos).
That is why the LWCloneU2 on the **ATmega32U4** additionally shows up as a **virtual COM port** ("USB Serial Device",
no driver on Windows 10/11) and speaks the TeensyStripController protocol there with one channel. DOF needs no
change – in `Cabinet.xml`:

```xml
<OutputControllers>
  <TeensyStripController>
    <Name>LWCloneU2 Strip</Name>
    <ComPortName>COM7</ComPortName>          <!-- the LWCloneU2's port, see Device Manager -->
    <NumberOfLedsStrip1>60</NumberOfLedsStrip1>
  </TeensyStripController>
</OutputControllers>
<Toys>
  <LedStrip>
    <Name>Undercab</Name>
    <OutputControllerName>LWCloneU2 Strip</OutputControllerName>
    <Width>60</Width><Height>1</Height>
    <FirstLedNumber>1</FirstLedNumber>
    <ColorOrder>RGB</ColorOrder>            <!-- the tool sets the order, do not let DOF reorder -->
  </LedStrip>
</Toys>
```

Requirements: data pin and pixel count set in the tool, mode **Direct**, "Save". `NumberOfLedsStrip1` must not exceed
the board's pixel limit (DOF queries the limit and sets the length itself). Only strip 1 is served; leave `ColorOrder`
in DOF at `RGB` and choose the real order in the tool, otherwise colors get swapped twice. The reply to the output
command is sent only after the strip has been written – DOF waits for it, so nothing is lost. Uno, Mega and the
breakout have no free USB endpoints for this; there the mirror mode remains, which works with DOF through the LED-Wiz
outputs anyway.

### "Device" tab

- **LED-Wiz id (1–16):** the number under which the board appears in LED-Wiz software. Several boards on one PC need
  different ids. After "Set id" the board restarts and has to be connected again.
- **Backup:** **Export as JSON** saves the complete assignment to a file (e.g. before firmware updates or for a second
  board of the same type). **Import JSON** loads it into the tool – remember to "Save" afterwards. The file only fits
  the same board type.
- **Load factory defaults:** resets the tool and the board's RAM to the default assignment (see the table above);
  permanent only after "Save".
- **Start bootloader:** detaches the board from USB and waits for new firmware (Leonardo/Micro: `avrdude` via COM port
  within 8 s; Breakout 32U2: Atmel FLIP/`dfu-programmer`). Back to normal: press reset or replug.

### Troubleshooting

| Symptom | Cause / fix |
|---|---|
| "This browser does not support WebHID" | use Edge or Chrome; Firefox/Safari cannot do it |
| Nothing appears in the device dialog | firmware not flashed or still v1; charge-only cable?; Mega/Uno: both chips flashed? |
| "No LWCloneU2 with configuration protocol found" | firmware v1 or an original LED-Wiz; flash firmware v2.1 |
| "Firmware too old (protocol 1)" | firmware 2.0 → update to 2.1 (configuration layout changed) |
| "No reply from device" | replug the board, reload the page; another application may hold the device – close LED-Wiz software briefly |
| "Device rejected the configuration" | a conflict the tool does not know: axis on a pin without ADC, more pixels than the board allows, reserved pin. "Load from device" returns to the last valid state |
| Output does not switch | right pin? "Test" in the tool; try "Inverted"; check driver stage and common ground |
| Button does not react / live dot stays off | button wired to GND? configuration applied? pin not used twice? |
| Joystick jitters at rest | increase the dead zone, "set" the center again |
| Strip stays dark | "Apply" after configuring; check data pin, GND and strip supply; DIN side of the strip (arrows) |
| Strip shows wrong colors | change the color order (GRB ↔ RGB) |
| Output PWM flickers while the strip runs | normal with long strips (interrupts blocked during transfer); reduce the pixel count |

## ledwiz.dll

`win32/bin/x86/ledwiz.dll` (for 32-bit programs such as most pinball front ends/DOF builds) or
`win32/bin/x64/ledwiz.dll`. The DLL is API compatible with the original (`LWZ_SBA`, `LWZ_PBA`, `LWZ_REGISTER`,
`LWZ_SET_NOTIFY`, `LWZ_SET_NOTIFY_EX`, `LWZ_RAWWRITE`, `LWZ_RAWREAD`), writes asynchronously and does not block the
application. It needs Windows 10 or newer (Universal C Runtime). `testapp.exe` switches all outputs on for a check,
`lwcconfig.exe` sets the LED-Wiz id from the command line.

## Building

Firmware: `avr-gcc` + `avr-libc` → `cd firmware && ./build.sh` (all boards, no make needed, output in
`firmware/build/`) or `make` with the LUFA makefiles; `./publish.sh` copies the hex files and `manifest.json` to
`release/`, which the web flasher downloads from.
Windows: `cmake -S win32 -B build-x86 -A Win32 && cmake --build build-x86 --config Release` (MSVC) or with
LLVM-MinGW/MinGW-w64 via `-G Ninja -DCMAKE_CXX_COMPILER=...`. Output in `win32/bin/<arch>/`.

## Code layout

See the tree above (`docs/` = web page with flasher and configuration tool, `old/` = unchanged upstream snapshot
from August 2023 for reference). To add a board: a folder with `hwconfig.h` (timer 0 = soft PWM, timer 1 = clock, ADC, UART),
`pinmap.h` (`BOARD_ID`, `BOARD_PIN_TABLE`, `WS2812_MAX_PIXELS`, factory defaults), `devconfig.h`, `makefile`; add
the board id to `pins.h` and the pin names to the tool (`BOARDS`). Protocol: [`firmware/proto.h`](firmware/proto.h),
data layout: [`firmware/config.h`](firmware/config.h).

## License
GPL v2 or later (see `LICENSE`). LUFA: own license (`firmware/lufa/LUFA/License.txt`).
Input logic after the V-USB Mame Panel by Andreas Oberdorfer (GPLv2).
