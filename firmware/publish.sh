#!/bin/sh
# Kopiert die gebauten Firmware-Dateien in den Ordner release/ und schreibt manifest.json – daraus bedient sich der Online-Flasher.
# AVR: build/*.hex (build.sh); Due: ../firmware_due/.pio/build/due/firmware.bin (pio run) → arduino_due.bin, falls vorhanden.
# Copies the built firmware files into release/ and writes manifest.json – the web flasher downloads from there.
# AVR: build/*.hex (build.sh); Due: ../firmware_due/.pio/build/due/firmware.bin (pio run) → arduino_due.bin, if present.
set -e
cd "$(dirname "$0")"
OUT=../release
DUE_BIN=../firmware_due/.pio/build/due/firmware.bin
mkdir -p "$OUT"
MAJ=$(sed -n 's/^#define FW_VERSION_MAJOR *\([0-9]*\).*/\1/p' proto.h)
MIN=$(sed -n 's/^#define FW_VERSION_MINOR *\([0-9]*\).*/\1/p' proto.h)
PROTO=$(sed -n 's/^#define PROTO_VERSION *\([0-9]*\).*/\1/p' proto.h)
DATE=$(date +%Y-%m-%d)
cp build/*.hex "$OUT"/
FILES="$(ls build/*.hex)"
if [ -f "$DUE_BIN" ]; then
	cp "$DUE_BIN" "$OUT/arduino_due.bin"
	FILES="$FILES $OUT/arduino_due.bin"
else
	echo "Hinweis: $DUE_BIN fehlt – Due-Firmware nicht gebaut (firmware_due: pio run) / missing, Due firmware not built"
fi
{
	echo "{"
	echo "  \"version\": \"$MAJ.$MIN\", \"proto\": $PROTO, \"date\": \"$DATE\","
	echo "  \"files\": {"
	first=1
	for f in $FILES; do
		n=$(basename "$f"); sz=$(wc -c < "$f" | tr -d ' ')
		[ $first = 1 ] || echo ","
		first=0
		printf '    "%s": %s' "$n" "$sz"
	done
	echo
	echo "  }"
	echo "}"
} > "$OUT/manifest.json"
echo "release aktualisiert / updated: $(ls "$OUT"/*.hex "$OUT"/*.bin 2>/dev/null | wc -l) Dateien / files, Version $MAJ.$MIN"
