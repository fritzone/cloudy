cloudy 0.1 - file exchange between DOS and Linux
=====================================================

cloudy is a two panel file manager: the left panel is this DOS machine,
the right one is a directory on a Linux machine. Files are copied between
them over the network.


What you need
-------------

On the DOS machine:
  - an 8086 or better, DOS 3.3 or newer
  - about 560K of free conventional memory
  - a network card with a packet driver. The driver for NE2000 compatible
    cards comes with cloudy, for other cards get the packet driver of the
    card (the Crynwr collection has many)

On the Linux machine:
  - Python 3
  - the cloudy peer, peer/cloudy_peer.py in the cloudy sources


Installing
----------

Run INSTALL from this disk or directory:

    A:\>INSTALL

It asks where to install cloudy (C:\CLOUDY by default), which packet driver
to use and how this machine gets its IP address, then copies the files and
adds a few lines to AUTOEXEC.BAT. The original AUTOEXEC.BAT is kept as
AUTOEXEC.CLD. INSTALL /Y installs with the default answers.

The added lines are between "REM *** cloudy begin" and "REM *** cloudy end",
at the end of AUTOEXEC.BAT. If your AUTOEXEC.BAT starts a menu or Windows
at its end, move the block above that line.

In the install directory:
  CLOUD.EXE     cloudy itself
  CLOUDY.BAT    starts cloudy (the installer adds its directory to the PATH)
  NETSTART.BAT  loads the packet driver, gets an IP address (DHCP), sets
                MTCPCFG. AUTOEXEC.BAT calls it
  MTCP.CFG      the network settings of mTCP
  DHCP.EXE      mTCP DHCP client
  PING.EXE      mTCP ping, to check the network: PING 192.168.1.10
  NE2000.COM    Crynwr packet driver for NE2000 compatible cards


Using it
--------

On the Linux machine start the peer, it serves your home directory:

    python3 peer/cloudy_peer.py --bind 0.0.0.0

(--root DIR serves another directory.) There is no password: anyone who
reaches the machine on port 8966 can read and write the served files, so
only run it on your home network.

On the DOS machine:

    CLOUDY                 connects to the peer given at the installation
    CLOUDY 192.168.1.10    connects to this peer

Keys:
  Tab         switch between the panels
  Enter       open a directory
  Backspace   go to the parent directory
  Ins         select a file or directory
  F5          copy the selected files and directories (or the current one) to
              the other panel, directories with everything in them
  Esc         cancel a copy, or quit

Long Linux file and directory names become DOS 8.3 names when copied to DOS,
two names which would become the same get a ~1, ~2, ... at the end. Existing
files are overwritten, existing directories are copied into.

CLOUD /V or CLOUD /D writes more details into CLOUDER.LOG.


If it does not work
-------------------

cloudy checks the network when it starts and tells what is missing. Most
problems are in the packet driver settings. Run NETSTART and read what the
packet driver and DHCP print:

  - "My Ethernet address is FF:FF:FF:FF:FF:FF" (or FE:FE:...): the I/O
    address of the card is wrong
  - "no packets were seen on the wire": the IRQ is wrong, or taken by
    another device, or the cable is not connected
  - "there is already a packet driver": one was loaded before (maybe by
    NETSTART itself), reboot first

Fix the packet driver line in C:\CLOUDY\NETSTART.BAT, or run INSTALL again
(it offers the previous answers).

  - "Connection failed": is the peer running, with --bind 0.0.0.0? Does
    PING reach the Linux machine? Is port 8966 open in its firewall?
  - "Bad command or file name" for CLOUDY: the PATH was too long to add the
    cloudy directory to it, use C:\CLOUDY\CLOUDY

Emulators:
  - DOSBox-X: ne2000=true, backend=slirp; the card is at I/O 0x300, IRQ 10
  - 86Box: the "Novell NE2000" card at I/O 0x300, IRQ 3 (with COM2
    disabled), network type SLiRP. The "NE2000 Compatible" card works too,
    the "NE2000 Compatible 8-bit" one does not. Change the settings while
    the machine is not running, 86Box rewrites its configuration file


Uninstalling
------------

    C:\CLOUDY\INSTALL /U

removes the files and the lines from AUTOEXEC.BAT.


Licenses
--------

cloudy is under the MIT license, see LICENSE.TXT.
mTCP (DHCP.EXE, PING.EXE, and the TCP/IP code inside CLOUD.EXE) is
Copyright Michael B. Brutman, under the GNU GPL version 3, see COPYING.TXT.
Its source: http://www.brutman.com/mTCP/
NE2000.COM is from the Crynwr packet driver collection, under the GNU GPL.
