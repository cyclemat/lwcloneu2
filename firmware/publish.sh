#!/bin/sh
# Kopiert die gebauten Hex-Dateien in die Webseite (docs/firmware) und schreibt manifest.json für den Online-Flasher.
# Copies the built hex files into the web page (docs/firmware) and writes manifest.json for the web flasher.
set -e
cd "$(dirname "$0")"
OUT=../docs/firmware
mkdir -p "$OUT"
MAJ=$(sed -n 's/^#define FW_VERSION_MAJOR *\([0-9]*\).*/\1/p' proto.h)
MIN=$(sed -n 's/^#define FW_VERSION_MINOR *\([0-9]*\).*/\1/p' proto.h)
PROTO=$(sed -n 's/^#define PROTO_VERSION *\([0-9]*\).*/\1/p' proto.h)
DATE=$(date +%Y-%m-%d)
cp build/*.hex "$OUT"/
{
	echo "{"
	echo "  \"version\": \"$MAJ.$MIN\", \"proto\": $PROTO, \"date\": \"$DATE\","
	echo "  \"files\": {"
	first=1
	for f in build/*.hex; do
		n=$(basename "$f"); sz=$(wc -c < "$f" | tr -d ' ')
		[ $first = 1 ] || echo ","
		first=0
		printf '    "%s": %s' "$n" "$sz"
	done
	echo
	echo "  }"
	echo "}"
} > "$OUT/manifest.json"
echo "docs/firmware aktualisiert / updated: $(ls "$OUT"/*.hex | wc -l) hex, Version $MAJ.$MIN"
