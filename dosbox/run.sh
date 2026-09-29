#!/bin/sh
#
# Starts cloud.exe inside DOSBox-X, with networking set up.
#
#   ./dosbox/run.sh                   build (if needed), then run cloud.exe
#   ./dosbox/run.sh -- -v 10.0.2.2    same, passing arguments to cloud.exe
#   ./dosbox/run.sh --shell           only give a DOS prompt, networking ready
#
# Otherwise the arguments are passed to dosbox-x unchanged.
#
# Start the Linux peer first, in another terminal:
#   python3 peer/cloudy_peer.py
# and connect to 10.0.2.2 from the IP screen (or pass it: ./dosbox/run.sh -- 10.0.2.2).

set -e

root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$root"

mode=run
args=""
case "$1" in
    --shell) mode=shell; shift ;;
    --) shift; args="$*"; set -- ;;
esac

if [ "$mode" = run ]; then
    [ -f cloud.exe ] || wmake
    exec dosbox-x -conf dosbox/cloud.conf -c "cloud.exe $args" -c "exit" "$@"
fi

exec dosbox-x -conf dosbox/cloud.conf "$@"
