#!/bin/sh
#
# Builds the DOS installation package of cloudy:
#
#   dist/cloudy-<version>/CLOUDY/   the files, run INSTALL.EXE from there
#   dist/cloudy-<version>.zip       the same, zipped
#   dist/cloudy-<version>.img       the same on a 1.44MB floppy image
#
# Needs Open Watcom (wmake, wcl), zip and mtools. The version comes from
# installer/install.c. Note that it leaves a release (no debug info)
# cloud.exe in the project directory, a plain wmake builds the debug one.

set -e

root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$root"

version=$(sed -n 's/^#define VERSION "\(.*\)"/\1/p' installer/install.c)
[ -n "$version" ] || { echo "No VERSION in installer/install.c" >&2; exit 1; }

out="dist/cloudy-$version"
pkg="$out/CLOUDY"

echo "Building cloud.exe (release)"
if ! wmake -h link_debug= > dist.log 2>&1; then
    cat dist.log; exit 1
fi
rm -f dist.log

echo "Building install.exe"
(cd installer && wmake -h > /dev/null)

echo "Collecting the files into $pkg"
rm -rf "$out" "$out.zip" "$out.img"
mkdir -p "$pkg"

cp cloud.exe "$pkg/CLOUD.EXE"
cp installer/install.exe "$pkg/INSTALL.EXE"
cp ext/dos/DHCP.EXE ext/dos/PING.EXE ext/dos/NE2000.COM "$pkg/"

# the text files with DOS line ends
crlf() {
    sed -e 's/\r$//' -e 's/$/\r/' "$1" > "$2"
}
sed "s/@VERSION@/$version/g" installer/README.TXT > "$out/readme.tmp"
crlf "$out/readme.tmp" "$pkg/README.TXT"
rm "$out/readme.tmp"
crlf LICENSE "$pkg/LICENSE.TXT"
crlf ext/dos/COPYING.TXT "$pkg/COPYING.TXT"

echo "Zipping"
(cd "$out" && zip -q -r "../cloudy-$version.zip" CLOUDY)

echo "Making the floppy image"
MTOOLS_SKIP_CHECK=1 mformat -i "$out.img" -C -f 1440 -v CLOUDY ::
MTOOLS_SKIP_CHECK=1 mcopy -i "$out.img" "$pkg"/* ::/

ls -l "$pkg"
echo
echo "Done: $out.zip and $out.img"
