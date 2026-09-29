# cloudy
cloudy is a tool that will allow you to copy files from/to DOS systems, running 
in 86Box or some other emulator that can emulate network cards.

## Building

cloudy is cross compiled on Linux (x86-64) with Open Watcom into a 16-bit DOS
program, against the mTCP TCP/IP library. The steps below were tested with
Open Watcom 2.0 (the 2023-11-20 beta and the 2026-09-29 `Current-build`) and
mTCP 2023-03-31 and 2025-01-10.

### What you need

- Open Watcom 2 (C/C++), see below
- the mTCP sources, see below
- `git`, `curl`, `unzip`
- Python 3: the Linux peer and the protocol generator
- `zip` and `mtools`: only for building the DOS package
  (`sudo apt install zip mtools` on Debian and Ubuntu)
- [DOSBox-X](https://dosbox-x.com): to run it without a DOS machine

### 1. Open Watcom

The Linux release of Open Watcom 2 is a single file, which is both the
installer and a zip archive. Unzipping it is enough:

    curl -LO https://github.com/open-watcom/open-watcom-v2/releases/download/Current-build/open-watcom-2_0-c-linux-x64
    mkdir -p ~/watcom
    unzip open-watcom-2_0-c-linux-x64 -d ~/watcom
    chmod +x ~/watcom/binl64/* ~/watcom/binl/*

Then set up the environment, for example at the end of `~/.bashrc`:

    export WATCOM=~/watcom
    export PATH=$WATCOM/binl64:$WATCOM/binl:$PATH
    export INCLUDE=$WATCOM/h
    export EDPATH=$WATCOM/eddat

`wpp` without arguments should now print `Open Watcom C++ x86 16-bit
Optimizing Compiler`.

### 2. mTCP, and where this repository goes

cloudy is built as one of the mTCP applications: this repository has to be
checked out into the `APPS` directory of the mTCP sources, the makefile finds
mTCP two levels up (`../../tcpinc`, `../../tcplib`, `../../include`).

    curl -LO http://www.brutman.com/mTCP/download/mTCP-src_2025-01-10.zip
    unzip mTCP-src_2025-01-10.zip
    cd mTCP-src_2025-01-10/APPS
    git clone https://github.com/fritzone/cloudy.git CLOUD
    cd CLOUD
    ./tools/prepare_mtcp.sh

which gives:

    mTCP-src_2025-01-10/
    ├── tcpinc/  tcplib/  include/    mTCP, lowercased by prepare_mtcp.sh
    ├── APPS/
    │   ├── CLOUD/                    this repository
    │   ├── DHCP/  FTP/  PING/ ...    the mTCP applications
    └── ...

The name of the `CLOUD` directory does not matter, its depth does. (In older
mTCP source zips the same directories are under `TCP/API/`, then the
repository goes into `TCP/API/APPS/CLOUD`.) The mTCP zip has DOS style upper
case file names while its sources include lower case ones, which only works
on a case insensitive file system; `tools/prepare_mtcp.sh` lowercases what
cloudy uses. Running it again does no harm. brutman.com has only the latest
mTCP release, get it from http://www.brutman.com/mTCP/ if the file name
above does not work any more.

### 3. Build

In the `CLOUD` directory:

    wmake                     # cloud.exe, with debug info
    wmake link_debug=         # cloud.exe without debug info, as in the package
    (cd installer && wmake)   # installer/install.exe
    ./tools/mkdist.sh         # the DOS package into dist/ (builds the two above)

`wmake` always builds everything from scratch. After changing the messages in
`tools/idl_gen/main.idl`, regenerate the protocol code (see "The protocol"
below) before building. To try the result, see "Running in DOSBox-X".

## The Linux peer

`peer/cloudy_peer.py` (Python 3, no dependencies) serves a directory tree,
`$HOME` by default:

    python3 peer/cloudy_peer.py                   # for DOSBox on this machine
    python3 peer/cloudy_peer.py --bind 0.0.0.0    # for a real DOS machine on the LAN
    python3 peer/cloudy_peer.py --root /srv/dos -v

There is no authentication, anyone reaching port 8966 can read and write the
served tree: only use it on a network you trust.

## Running in DOSBox-X

`dosbox/` holds a DOSBox-X setup with an emulated NE2000 card on the slirp
backend, the packet driver and an mTCP config (`10.0.2.15`, host is
`10.0.2.2`). Start the peer, then:

    ./dosbox/run.sh                   # runs cloud.exe, connect to 10.0.2.2
    ./dosbox/run.sh -- -v 10.0.2.2    # connects right away, logs more
    ./dosbox/run.sh --shell           # just a DOS prompt with networking up

`cloud.exe [-v|-d] [ip]`: `-v` / `-d` log informational / debug messages
into `CLOUDER.LOG` (warnings and errors only by default).

Keys: `Tab` switches panels, `Enter` opens a directory, `Backspace` goes up,
`Ins` selects, `F5` copies the selected (or the current) files to the other
panel, `Esc` cancels a copy or quits. Directories are not copied yet.

## The DOS installation package

    ./tools/mkdist.sh

builds `dist/cloudy-<version>/CLOUDY/`, the same zipped and on a 1.44MB floppy
image (`dist/cloudy-<version>.img`). On the DOS machine run `INSTALL` from it:
it copies `CLOUD.EXE`, the mTCP `DHCP.EXE` and `PING.EXE` and the NE2000
packet driver into `C:\CLOUDY`, writes `MTCP.CFG`, `NETSTART.BAT` (packet
driver, DHCP, `MTCPCFG`) and `CLOUDY.BAT`, and adds a block calling them to
`AUTOEXEC.BAT` (the original is kept as `AUTOEXEC.CLD`). `INSTALL /Y` takes the
default answers, `C:\CLOUDY\INSTALL /U` uninstalls. The installer is in
`installer/`, its version (`VERSION` in `install.c`) names the package; the
bundled DOS binaries are in `ext/dos/`.

The package's `CLOUD.EXE` is built without debug info (`wmake link_debug=`).

## Running in 86Box

Tested with 86Box build 9001 (the Linux AppImage), machine `686nx` (Pentium Pro), MS-DOS 6.22:

1. With the machine **not running** (86Box writes `86box.cfg` back while the
   machine runs, hand edits get lost), in its settings, Network: card
   **Novell NE2000**, network type **SLiRP**, I/O **0x300**, IRQ **3**.
   In `86box.cfg`:

       [Network]
       net_01_card = novell_ne2k
       net_01_net_type = slirp

       [Novell NE2000 #1]
       base = 0300
       irq = 3

   The settings are in the section of the card chosen by `net_01_card`:
   `[Novell NE2000 #1]` for `novell_ne2k`, `[NE2000 Compatible #1]` for
   `ne2k`, `[NE2000 Compatible 8-bit #1]` for `ne2k8`.
2. Start the peer on the host: `python3 peer/cloudy_peer.py --bind 0.0.0.0`.
   Inside the machine the host is `10.0.2.2`.
3. Attach `dist/cloudy-<version>.img` as a floppy, boot, run `A:\INSTALL` and
   give the same values: packet driver `1` (NE2000), I/O `0x300`, IRQ `3`,
   interrupt `0x60`, IP address `1` (DHCP), peer `10.0.2.2`.
4. Reboot: `NETSTART.BAT` loads the driver and DHCP gives `10.0.2.15`. Start
   cloudy with `CLOUDY` (or `C:\CLOUDY\CLOUDY` if the installer said the PATH
   was too long).

What was tried on that machine:

| Card                    | I/O   | IRQ     | Result                                        |
|-------------------------|-------|---------|-----------------------------------------------|
| Novell NE2000           | 0x300 | 3       | works                                         |
| NE2000 Compatible       | 0x300 | 3 or 10 | works                                         |
| NE2000 Compatible       | 0x320 | 3 or 10 | DHCP: "no packets were seen on the wire"      |
| NE2000 Compatible 8-bit | 0x300 | 3       | DHCP: "no packets were seen on the wire"      |
| NE2000 Compatible 8-bit | 0x320 | 3       | DHCP: "no packets were seen on the wire"      |

The Novell NE2000's settings dialog does not offer IRQ 10. IRQ 3 is COM2's,
keep COM2 disabled (or use another free IRQ, and give it to the installer).

If cloudy stops at start with "the network settings ... are not usable",
run `C:\CLOUDY\NETSTART` and read what the packet driver and DHCP print.

## The protocol

The messages are described in `tools/idl_gen/main.idl`. After changing it run

    python3 tools/idl_gen/gen.py

which regenerates the C++ side in `msg_prot/` and the Python side in
`peer/cldproto.py`. On the wire every message is an XML document terminated
by a `\0` byte.

## Limits

Everything has to fit in 640K: the program image is about 340K, a remote
directory shows at most 600 entries.
