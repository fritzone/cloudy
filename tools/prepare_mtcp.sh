#!/bin/sh
#
# Makes the mTCP sources buildable with Open Watcom on Linux.
#
# The mTCP source zip has DOS style upper case names (TCPINC/TCP.H), while its
# sources include lower and mixed case ones ("tcp.h", "Eth.h"), which only works
# on a case insensitive file system. This lowercases the three directories
# cloudy uses (TCPINC, TCPLIB, INCLUDE) with their files, and the includes in
# them. Running it again does nothing.
#
#   tools/prepare_mtcp.sh [mTCP directory]
#
# The mTCP directory is the one holding TCPINC, TCPLIB, INCLUDE and APPS, by
# default two levels above this project (this project lives in APPS/CLOUD).

set -e

here="$(cd "$(dirname "$0")/.." && pwd)"
mtcp="${1:-$here/../..}"
mtcp="$(cd "$mtcp" && pwd)"

found=0
for d in TCPINC TCPLIB INCLUDE tcpinc tcplib include; do
    [ -d "$mtcp/$d" ] && found=1
done
if [ $found = 0 ]; then
    echo "$mtcp does not look like the mTCP sources (no TCPINC, TCPLIB, INCLUDE)" >&2
    exit 1
fi

for d in TCPINC TCPLIB INCLUDE; do
    lower=$(echo "$d" | tr A-Z a-z)
    if [ -d "$mtcp/$d" ]; then
        mv "$mtcp/$d" "$mtcp/$lower"
    fi
    for f in "$mtcp/$lower"/*; do
        name=$(basename "$f")
        lname=$(echo "$name" | tr A-Z a-z)
        if [ "$name" != "$lname" ]; then
            mv "$f" "$mtcp/$lower/$lname"
        fi
    done
    # #include "Eth.h" -> #include "eth.h"
    sed -i -E 's/^(#include[ \t]+")([^"]*[A-Z][^"]*)(")/\1\L\2\E\3/' "$mtcp/$lower"/*
done

echo "mTCP in $mtcp is ready"
