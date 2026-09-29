/*
 * INSTALL.EXE - installs cloudy on a DOS machine.
 *
 *   INSTALL        asks a few questions, then installs
 *   INSTALL /Y     installs with the default answers
 *   INSTALL /U     uninstalls (run the INSTALL.EXE in the installed directory)
 *
 * It copies the files next to INSTALL.EXE into the install directory, writes
 * the mTCP configuration and two batch files:
 *   NETSTART.BAT   loads the packet driver, runs DHCP, sets MTCPCFG
 *   CLOUDY.BAT     starts cloud.exe
 * and adds a block to AUTOEXEC.BAT which calls them. The block is between
 * marker lines, so installing again replaces it and uninstalling removes it.
 *
 * Plain C for the small memory model, runs on DOS 3.3+ and an 8086.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dos.h>
#include <direct.h>
#include <io.h>
#include <i86.h>

#define VERSION "0.1"

#define BEGIN_MARK "REM *** cloudy begin"
#define END_MARK   "REM *** cloudy end"

/* The files copied from the package */
static const char* payload[] =
{
    "CLOUD.EXE", "DHCP.EXE", "PING.EXE", "NE2000.COM",
    "README.TXT", "COPYING.TXT", "LICENSE.TXT", "INSTALL.EXE", NULL
};

/* The files the installer writes */
static const char* generated[] =
{
    "MTCP.CFG", "NETSTART.BAT", "CLOUDY.BAT", NULL
};

enum { DRIVER_NE2000 = 1, DRIVER_LOADED = 2, DRIVER_CUSTOM = 3 };

/* The longest line COMMAND.COM takes, after the %VARIABLES% are expanded */
#define MAX_BATCH_LINE 127

struct Settings
{
    char dir[68];
    int driver;
    unsigned pktint;
    unsigned io;
    unsigned irq;
    char custom[100];
    int dhcp;
    char ip[16];
    char mask[16];
    char gw[16];
    char dns[16];
    char peer[64];
    int autostart;
    int editAutoexec;
    int addPath;
    char autoexec[68];
};

static int unattended = 0;

/* 8K for copying files, and for AUTOEXEC.BAT */
static char buf[8192];
static char autoexecText[16384];

/* ------------------------------------------------------------------ */
/* Asking questions                                                    */
/* ------------------------------------------------------------------ */

static void trim(char* s)
{
    int len = strlen(s);
    while(len > 0 && isspace((unsigned char)s[len - 1]))
    {
        s[--len] = 0;
    }
    while(*s && isspace((unsigned char)*s))
    {
        memmove(s, s + 1, strlen(s));
    }
}

/* Asks for a line of text, an empty answer gives the default */
static void ask(const char* question, const char* def, char* out, int size)
{
    char line[128];

    if(unattended)
    {
        strncpy(out, def, size - 1);
        out[size - 1] = 0;
        return;
    }

    printf("%s [%s]: ", question, def);
    fflush(stdout);
    if(fgets(line, sizeof(line), stdin) == NULL)
    {
        line[0] = 0;
    }
    trim(line);

    strncpy(out, line[0] ? line : def, size - 1);
    out[size - 1] = 0;
}

static int askYesNo(const char* question, int def)
{
    char a[8];
    for(;;)
    {
        ask(question, def ? "Y" : "N", a, sizeof(a));
        if(toupper(a[0]) == 'Y') return 1;
        if(toupper(a[0]) == 'N') return 0;
        printf("  Please answer Y or N.\n");
    }
}

static int askChoice(const char* question, int max, int def)
{
    char a[8];
    char d[8];
    sprintf(d, "%d", def);
    for(;;)
    {
        int c;
        ask(question, d, a, sizeof(a));
        c = atoi(a);
        if(c >= 1 && c <= max) return c;
        printf("  Please answer 1 to %d.\n", max);
    }
}

/* Numbers as 0x... (hex) or decimal */
static unsigned parseNumber(const char* s)
{
    if(s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
    {
        return (unsigned)strtoul(s + 2, NULL, 16);
    }
    return (unsigned)strtoul(s, NULL, 10);
}

static unsigned askNumber(const char* question, unsigned def, unsigned min, unsigned max, int hex)
{
    char a[16];
    char d[16];
    sprintf(d, hex ? "0x%X" : "%u", def);
    for(;;)
    {
        unsigned n;
        ask(question, d, a, sizeof(a));
        n = parseNumber(a);
        if(n >= min && n <= max) return n;
        printf(hex ? "  Please enter a number between 0x%X and 0x%X.\n" : "  Please enter a number between %u and %u.\n", min, max);
    }
}

static int validIp(const char* s)
{
    int parts = 0;
    while(parts < 4)
    {
        int digits = 0;
        unsigned v = 0;
        while(isdigit((unsigned char)*s))
        {
            v = v * 10 + (*s - '0');
            s++;
            digits++;
        }
        if(digits == 0 || digits > 3 || v > 255) return 0;
        parts++;
        if(parts < 4)
        {
            if(*s != '.') return 0;
            s++;
        }
    }
    return *s == 0;
}

static void askIp(const char* question, const char* def, char* out)
{
    for(;;)
    {
        ask(question, def, out, 16);
        if(validIp(out)) return;
        printf("  Please enter an address like 192.168.1.10\n");
    }
}

/* ------------------------------------------------------------------ */
/* The system                                                          */
/* ------------------------------------------------------------------ */

/* The drive DOS booted from, C: if DOS cannot tell (before DOS 4) */
static char bootDrive(void)
{
    union REGS r;
    r.x.ax = 0x3305;
    intdos(&r, &r);
    if(r.h.al != 0xFF && r.h.dl >= 1 && r.h.dl <= 26)
    {
        return 'A' + r.h.dl - 1;
    }
    return 'C';
}

/* True on an 8088/8086: bits 12-15 of the flags cannot be cleared there */
static int is8086(void)
{
    unsigned short flags;
    _asm {
        pushf
        pop ax
        and ax, 0x0FFF
        push ax
        popf
        pushf
        pop ax
        mov flags, ax
    }
    return (flags & 0xF000) == 0xF000;
}

/* Would "SET PATH=%PATH%;dir" fit in a batch line with the current PATH? */
static int pathFits(const char* dir)
{
    const char* path = getenv("PATH");
    return strlen("SET PATH=;") + (path ? strlen(path) : 0) + strlen(dir) <= MAX_BATCH_LINE;
}

/* The software interrupt of a loaded packet driver, 0 if there is none */
static unsigned findPacketDriver(void)
{
    unsigned v;
    for(v = 0x60; v <= 0x80; v++)
    {
        char __far* handler = (char __far*)_dos_getvect(v);
        if(handler != NULL && _fmemcmp(handler + 3, "PKT DRVR", 8) == 0)
        {
            return v;
        }
    }
    return 0;
}

/* The directory INSTALL.EXE is in, with a \ at the end */
static void programDir(const char* argv0, char* out, int size)
{
    const char* slash = strrchr(argv0, '\\');
    if(slash == NULL)
    {
        getcwd(out, size - 1);
    }
    else
    {
        int len = slash - argv0;
        if(len > size - 2) len = size - 2;
        memcpy(out, argv0, len);
        out[len] = 0;
    }
    if(out[0] && out[strlen(out) - 1] != '\\')
    {
        strcat(out, "\\");
    }
}

static void pathJoin(char* out, const char* dir, const char* name)
{
    strcpy(out, dir);
    if(out[0] && out[strlen(out) - 1] != '\\')
    {
        strcat(out, "\\");
    }
    strcat(out, name);
}

static int fileExists(const char* path)
{
    return access(path, 0) == 0;
}

static long fileSize(const char* path)
{
    FILE* f = fopen(path, "rb");
    long size;
    if(f == NULL) return -1;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fclose(f);
    return size;
}

/* Creates the directory and its parents */
static int makeDirs(const char* path)
{
    char partial[80];
    const char* p = path;

    /* skip the drive */
    if(p[0] && p[1] == ':') p += 2;
    if(*p == '\\') p++;

    for(;;)
    {
        const char* next = strchr(p, '\\');
        int len = next ? next - path : strlen(path);
        memcpy(partial, path, len);
        partial[len] = 0;

        if(!fileExists(partial) && mkdir(partial) != 0)
        {
            return 0;
        }
        if(!next) return 1;
        p = next + 1;
    }
}

static int copyFile(const char* from, const char* to)
{
    FILE* in;
    FILE* out;
    size_t n;
    int ok = 1;

    in = fopen(from, "rb");
    if(in == NULL) return 0;
    out = fopen(to, "wb");
    if(out == NULL)
    {
        fclose(in);
        return 0;
    }

    while((n = fread(buf, 1, sizeof(buf), in)) > 0)
    {
        if(fwrite(buf, 1, n, out) != n)
        {
            ok = 0;
            break;
        }
    }
    if(ferror(in)) ok = 0;

    fclose(in);
    if(fclose(out) != 0) ok = 0;
    return ok;
}

static int writeTextFile(const char* path, const char* text)
{
    /* text mode: the \n become \r\n */
    FILE* f = fopen(path, "w");
    int ok;
    if(f == NULL) return 0;
    ok = fputs(text, f) >= 0;
    if(fclose(f) != 0) ok = 0;
    return ok;
}

/* ------------------------------------------------------------------ */
/* AUTOEXEC.BAT                                                        */
/* ------------------------------------------------------------------ */

static int startsWith(const char* s, const char* prefix)
{
    while(*prefix)
    {
        if(toupper((unsigned char)*s) != toupper((unsigned char)*prefix)) return 0;
        s++;
        prefix++;
    }
    return 1;
}

/*
 * Reads AUTOEXEC.BAT without our block into autoexecText (with \n line ends).
 * Returns -1 if it is too big to handle, 0 if it does not exist, 1 otherwise.
 */
static int readAutoexecWithoutBlock(const char* path)
{
    FILE* f;
    char line[256];
    int inBlock = 0;
    size_t used = 0;

    autoexecText[0] = 0;
    f = fopen(path, "r");
    if(f == NULL) return 0;

    while(fgets(line, sizeof(line), f) != NULL)
    {
        size_t len;
        const char* p = line;
        while(*p == ' ' || *p == '\t' || *p == '@') p++;

        if(startsWith(p, BEGIN_MARK))
        {
            inBlock = 1;
            continue;
        }
        if(startsWith(p, END_MARK))
        {
            inBlock = 0;
            continue;
        }
        if(inBlock) continue;

        /* a ^Z at the end of old files */
        if(line[0] == 0x1A) continue;

        len = strlen(line);
        if(used + len + 2 >= sizeof(autoexecText))
        {
            fclose(f);
            return -1;
        }
        strcpy(autoexecText + used, line);
        used += len;
        if(len > 0 && line[len - 1] != '\n')
        {
            autoexecText[used++] = '\n';
            autoexecText[used] = 0;
        }
    }
    fclose(f);
    return 1;
}

/* Writes AUTOEXEC.BAT: its old content without our block, then the block (if any) */
static int writeAutoexec(const char* path, const char* block)
{
    char backup[80];
    char* dot;
    int exists = readAutoexecWithoutBlock(path);
    FILE* f;

    if(exists < 0)
    {
        printf("  %s is too big for the installer, not changed.\n", path);
        return 0;
    }

    /* keep the original, the first time only */
    strcpy(backup, path);
    dot = strrchr(backup, '.');
    if(dot) strcpy(dot, ".CLD");
    if(exists && !fileExists(backup))
    {
        if(!copyFile(path, backup))
        {
            printf("  Cannot back up %s, not changed.\n", path);
            return 0;
        }
        printf("  The original is saved as %s\n", backup);
    }

    /* it might be read only */
    if(exists) _dos_setfileattr(path, _A_NORMAL);

    f = fopen(path, "w");
    if(f == NULL)
    {
        printf("  Cannot write %s\n", path);
        return 0;
    }
    fputs(autoexecText, f);
    if(block) fputs(block, f);
    if(fclose(f) != 0)
    {
        printf("  Cannot write %s, disk full?\n", path);
        return 0;
    }
    return 1;
}

/* ------------------------------------------------------------------ */
/* Installing                                                          */
/* ------------------------------------------------------------------ */

static void defaults(struct Settings* s)
{
    char drive = bootDrive();
    unsigned loaded = findPacketDriver();

    memset(s, 0, sizeof(*s));
    sprintf(s->dir, "%c:\\CLOUDY", drive == 'A' || drive == 'B' ? 'C' : drive);
    s->driver = loaded ? DRIVER_LOADED : DRIVER_NE2000;
    s->pktint = loaded ? loaded : 0x60;
    s->io = 0x300;
    /* 8 bit slots have no IRQ 10, AT class machines (and DOSBox-X) often use it */
    s->irq = is8086() ? 3 : 10;
    s->dhcp = 1;
    strcpy(s->ip, "192.168.1.50");
    strcpy(s->mask, "255.255.255.0");
    strcpy(s->gw, "192.168.1.1");
    strcpy(s->dns, "192.168.1.1");
    s->autostart = 0;
    s->editAutoexec = 1;
    sprintf(s->autoexec, "%c:\\AUTOEXEC.BAT", drive);
}

/* Takes the answers of an earlier installation in s->dir as the defaults */
static void loadPrevious(struct Settings* s)
{
    char path[80];
    char line[160];
    FILE* f;

    pathJoin(path, s->dir, "NETSTART.BAT");
    f = fopen(path, "r");
    if(f == NULL) return;

    s->driver = DRIVER_LOADED;
    s->dhcp = 0;
    while(fgets(line, sizeof(line), f))
    {
        char* p;
        trim(line);
        strupr(line);
        if(startsWith(line, "@") || startsWith(line, "REM") || startsWith(line, "SET ") ||
           startsWith(line, "IF ") || startsWith(line, "ECHO") || startsWith(line, ":") || !line[0])
        {
            continue;
        }
        if(strstr(line, "DHCP.EXE"))
        {
            s->dhcp = 1;
        }
        else if((p = strstr(line, "\\NE2000.COM ")) != NULL)
        {
            char a[16], b[16], c[16];
            if(sscanf(p + 12, "%15s %15s %15s", a, b, c) == 3)
            {
                s->driver = DRIVER_NE2000;
                s->pktint = parseNumber(a);
                s->irq = parseNumber(b);
                s->io = parseNumber(c);
            }
        }
        else
        {
            s->driver = DRIVER_CUSTOM;
            strncpy(s->custom, line, sizeof(s->custom) - 1);
        }
    }
    fclose(f);

    /* the fixed address */
    pathJoin(path, s->dir, "MTCP.CFG");
    f = fopen(path, "r");
    if(f != NULL)
    {
        while(fgets(line, sizeof(line), f))
        {
            char name[16], value[20];
            if(sscanf(line, "%15s %19s", name, value) != 2) continue;
            if(!stricmp(name, "PACKETINT")) s->pktint = parseNumber(value);
            if(s->dhcp) continue;
            if(!stricmp(name, "IPADDR") && validIp(value)) strcpy(s->ip, value);
            if(!stricmp(name, "NETMASK") && validIp(value)) strcpy(s->mask, value);
            if(!stricmp(name, "GATEWAY") && validIp(value)) strcpy(s->gw, value);
            if(!stricmp(name, "NAMESERVER") && validIp(value)) strcpy(s->dns, value);
        }
        fclose(f);
    }

    /* the peer: IF "%1"=="" C:\CLOUDY\CLOUD.EXE <peer> */
    pathJoin(path, s->dir, "CLOUDY.BAT");
    f = fopen(path, "r");
    if(f != NULL)
    {
        while(fgets(line, sizeof(line), f))
        {
            char* p = strstr(line, "CLOUD.EXE ");
            trim(line);
            if(startsWith(line, "IF \"%1\"==\"\"") && p != NULL)
            {
                sscanf(p + 10, "%63s", s->peer);
            }
        }
        fclose(f);
    }
    printf("The settings of the installation in %s are the defaults.\n", s->dir);
}

static void askSettings(struct Settings* s)
{
    unsigned loaded = findPacketDriver();
    char a[80];

    printf("\nWhere should cloudy go?\n");
    ask("Install directory", s->dir, a, sizeof(s->dir) - 3);
    strupr(a);
    if(a[1] != ':')
    {
        /* no drive given: the current one */
        unsigned drive;
        _dos_getdrive(&drive);
        sprintf(s->dir, "%c:%s%s", 'A' + drive - 1, a[0] == '\\' ? "" : "\\", a);
    }
    else
    {
        strcpy(s->dir, a);
    }
    if(strlen(s->dir) > 3 && s->dir[strlen(s->dir) - 1] == '\\')
    {
        s->dir[strlen(s->dir) - 1] = 0;
    }

    printf("\nmTCP talks to the network card through a packet driver.\n");
    if(loaded)
    {
        printf("A packet driver is loaded right now, at interrupt 0x%X.\n", loaded);
    }
    printf("  1) NE2000 compatible card, use the driver that comes with cloudy\n");
    printf("  2) My system already loads a packet driver when it starts\n");
    printf("  3) Another packet driver, I will give its command line\n");
    s->driver = askChoice("Packet driver", 3, s->driver);

    if(s->driver == DRIVER_NE2000)
    {
        printf("  Give the settings of the card (its jumpers, or its setup program).\n");
        printf("  DOSBox-X: 0x300 and IRQ 10. 86Box: the Novell NE2000 card (not the 8-bit\n");
        printf("  one) at 0x300 and IRQ 3, with COM2 disabled.\n");
        s->io = askNumber("  I/O base address of the card", s->io, 0x200, 0x3F0, 1);
        s->irq = askNumber("  IRQ of the card", s->irq, 2, 15, 0);
    }
    if(s->driver == DRIVER_CUSTOM)
    {
        for(;;)
        {
            ask("  Command line of the packet driver (for example C:\\NET\\3C509.COM 0x60)", "", s->custom, sizeof(s->custom));
            if(s->custom[0]) break;
        }
    }
    s->pktint = askNumber("  Software interrupt of the packet driver", s->pktint, 0x60, 0x80, 1);

    printf("\nHow does this machine get its IP address?\n");
    printf("  1) From a DHCP server (most home routers are one)\n");
    printf("  2) I will give a fixed address\n");
    s->dhcp = askChoice("IP address", 2, s->dhcp ? 1 : 2) == 1;
    if(!s->dhcp)
    {
        askIp("  IP address of this machine", s->ip, s->ip);
        askIp("  Netmask", s->mask, s->mask);
        askIp("  Gateway", s->gw, s->gw);
        askIp("  DNS server", s->dns, s->dns);
    }

    printf("\nThe address of the Linux machine running the cloudy peer.\n");
    printf("Leave it empty to type it in every time cloudy starts.\n");
    ask("Peer address", s->peer, s->peer, sizeof(s->peer));

    printf("\n");
    s->editAutoexec = askYesNo("Add the network start up to AUTOEXEC.BAT", s->editAutoexec);
    if(s->editAutoexec)
    {
        ask("  AUTOEXEC.BAT to change", s->autoexec, a, sizeof(s->autoexec));
        strupr(a);
        strcpy(s->autoexec, a);
        s->autostart = askYesNo("  Start cloudy right after booting", s->autostart);
        s->addPath = pathFits(s->dir);
        if(!s->addPath)
        {
            printf("  Your PATH is too long to add %s to it, start cloudy with %s\\CLOUDY\n", s->dir, s->dir);
        }
    }
}

static void printSettings(const struct Settings* s)
{
    printf("\nInstall directory: %s\n", s->dir);
    switch(s->driver)
    {
    case DRIVER_NE2000:
        printf("Packet driver:     NE2000, I/O 0x%X, IRQ %u, interrupt 0x%X\n", s->io, s->irq, s->pktint);
        break;
    case DRIVER_LOADED:
        printf("Packet driver:     already loaded, interrupt 0x%X\n", s->pktint);
        break;
    default:
        printf("Packet driver:     %s (interrupt 0x%X)\n", s->custom, s->pktint);
    }
    if(s->dhcp)
        printf("IP address:        from DHCP\n");
    else
        printf("IP address:        %s/%s, gateway %s, DNS %s\n", s->ip, s->mask, s->gw, s->dns);
    printf("Peer address:      %s\n", s->peer[0] ? s->peer : "(asked at start)");
    if(s->editAutoexec)
        printf("AUTOEXEC.BAT:      %s is updated%s%s\n", s->autoexec, s->addPath ? "" : " (not the PATH)",
               s->autostart ? ", cloudy starts at boot" : "");
    else
        printf("AUTOEXEC.BAT:      not changed, run NETSTART.BAT before cloudy\n");
}

static int enoughSpace(const struct Settings* s, const char* from)
{
    struct diskfree_t df;
    unsigned drive = toupper((unsigned char)s->dir[0]) - 'A' + 1;
    unsigned long need = 16384;
    unsigned long have;
    char path[80];
    int i;

    for(i = 0; payload[i]; i++)
    {
        long size;
        pathJoin(path, from, payload[i]);
        size = fileSize(path);
        if(size > 0) need += size;
    }

    if(_dos_getdiskfree(drive, &df) != 0)
    {
        return 1; /* let the copy find out */
    }
    have = (unsigned long)df.avail_clusters * df.sectors_per_cluster * df.bytes_per_sector;
    if(have < need)
    {
        printf("Not enough space on %c:, %luK is needed and %luK is free.\n", s->dir[0], need / 1024, have / 1024);
        return 0;
    }
    return 1;
}

static int writeGenerated(const struct Settings* s)
{
    /* static: the stack of a small model program is small */
    static char text[1024];
    char path[80];
    char line[160];

    /* MTCP.CFG, DHCP fills in the addresses */
    sprintf(text, "PACKETINT 0x%X\nHOSTNAME cloudy\nMTU 1500\n", s->pktint);
    if(!s->dhcp)
    {
        sprintf(line, "IPADDR %s\nNETMASK %s\nGATEWAY %s\nNAMESERVER %s\n", s->ip, s->mask, s->gw, s->dns);
        strcat(text, line);
    }
    pathJoin(path, s->dir, "MTCP.CFG");
    if(!writeTextFile(path, text)) return 0;

    /* NETSTART.BAT */
    sprintf(text, "@ECHO OFF\nREM Written by the cloudy installer: starts the network for cloudy\nSET MTCPCFG=%s\\MTCP.CFG\n", s->dir);
    if(s->driver == DRIVER_NE2000)
    {
        sprintf(line, "%s\\NE2000.COM 0x%X 0x%X 0x%X\n", s->dir, s->pktint, s->irq, s->io);
        strcat(text, line);
    }
    else if(s->driver == DRIVER_CUSTOM)
    {
        sprintf(line, "%s\n", s->custom);
        strcat(text, line);
    }
    if(s->dhcp)
    {
        /* the packet drivers have no usable errorlevel, DHCP has */
        sprintf(line, "%s\\DHCP.EXE\nIF NOT ERRORLEVEL 1 GOTO END\n", s->dir);
        strcat(text, line);
        strcat(text,
            "ECHO.\n"
            "ECHO cloudy: DHCP could not get an IP address, cloudy will not work.\n"
            "ECHO  - an Ethernet address like FF:FF:FF:FF:FF:FF above: wrong I/O address\n"
            "ECHO  - no packets seen on the wire: wrong IRQ, or the cable\n");
        sprintf(line, "ECHO Fix the packet driver line in %s\\NETSTART.BAT or install again.\n:END\n", s->dir);
        strcat(text, line);
    }
    pathJoin(path, s->dir, "NETSTART.BAT");
    if(!writeTextFile(path, text)) return 0;

    /* CLOUDY.BAT */
    sprintf(text, "@ECHO OFF\nREM Written by the cloudy installer: starts cloudy\nSET MTCPCFG=%s\\MTCP.CFG\n", s->dir);
    if(s->peer[0])
    {
        sprintf(line, "IF \"%%1\"==\"\" %s\\CLOUD.EXE %s\nIF NOT \"%%1\"==\"\" %s\\CLOUD.EXE %%1 %%2 %%3\n", s->dir, s->peer, s->dir);
    }
    else
    {
        sprintf(line, "%s\\CLOUD.EXE %%1 %%2 %%3\n", s->dir);
    }
    strcat(text, line);
    pathJoin(path, s->dir, "CLOUDY.BAT");
    return writeTextFile(path, text);
}

static int install(const struct Settings* s, const char* from)
{
    char src[80];
    char dst[80];
    static char block[400];
    char line[120];
    int i;

    if(!enoughSpace(s, from)) return 0;

    if(!makeDirs(s->dir))
    {
        printf("Cannot create %s\n", s->dir);
        return 0;
    }

    printf("\nCopying the files to %s\n", s->dir);
    for(i = 0; payload[i]; i++)
    {
        pathJoin(src, from, payload[i]);
        pathJoin(dst, s->dir, payload[i]);

        /* running the installer from the installed directory */
        if(stricmp(src, dst) == 0) continue;

        printf("  %s\n", payload[i]);
        if(!copyFile(src, dst))
        {
            printf("Cannot copy %s to %s\n", src, dst);
            return 0;
        }
    }

    printf("Writing the settings\n");
    if(!writeGenerated(s))
    {
        printf("Cannot write the settings into %s\n", s->dir);
        return 0;
    }

    if(s->editAutoexec)
    {
        printf("Updating %s\n", s->autoexec);
        sprintf(block, "%s - added by the cloudy installer, %s\\INSTALL /U removes it\n", BEGIN_MARK, s->dir);
        sprintf(line, "CALL %s\\NETSTART.BAT\n", s->dir);
        strcat(block, line);
        if(s->addPath)
        {
            sprintf(line, "SET PATH=%%PATH%%;%s\n", s->dir);
            strcat(block, line);
        }
        if(s->autostart)
        {
            sprintf(line, "CALL %s\\CLOUDY.BAT\n", s->dir);
            strcat(block, line);
        }
        strcat(block, END_MARK "\n");

        if(!writeAutoexec(s->autoexec, block)) return 0;
    }

    return 1;
}

/* ------------------------------------------------------------------ */
/* Uninstalling                                                        */
/* ------------------------------------------------------------------ */

static int uninstall(const char* dir)
{
    char path[80];
    char autoexec[20];
    char a[80];
    int i;

    /* the installed directory has the generated files */
    pathJoin(path, dir, "NETSTART.BAT");
    if(!fileExists(path))
    {
        printf("cloudy is not installed in %s, run the INSTALL.EXE of the installed copy.\n", dir);
        return 0;
    }

    sprintf(autoexec, "%c:\\AUTOEXEC.BAT", bootDrive());
    printf("\nUninstalling cloudy from %s\n", dir);
    if(!askYesNo("Are you sure", 1)) return 0;

    ask("AUTOEXEC.BAT to clean up", autoexec, a, sizeof(a));
    if(fileExists(a))
    {
        printf("Updating %s\n", a);
        writeAutoexec(a, NULL);
    }

    printf("Removing the files\n");
    for(i = 0; payload[i]; i++)
    {
        pathJoin(path, dir, payload[i]);
        remove(path);
    }
    for(i = 0; generated[i]; i++)
    {
        pathJoin(path, dir, generated[i]);
        remove(path);
    }

    /* the directory itself, if it became empty */
    strcpy(path, dir);
    if(path[strlen(path) - 1] == '\\') path[strlen(path) - 1] = 0;
    chdir("\\");
    if(rmdir(path) != 0)
    {
        printf("%s is not empty, it is kept.\n", path);
    }
    return 1;
}

/* ------------------------------------------------------------------ */

static void banner(void)
{
    printf("cloudy %s installer - file exchange between DOS and Linux\n", VERSION);
    printf("==========================================================\n");
}

int main(int argc, char* argv[])
{
    struct Settings s;
    char from[80];
    int doUninstall = 0;
    int i;

    for(i = 1; i < argc; i++)
    {
        if(!stricmp(argv[i], "/Y") || !stricmp(argv[i], "-Y")) unattended = 1;
        else if(!stricmp(argv[i], "/U") || !stricmp(argv[i], "-U")) doUninstall = 1;
        else
        {
            printf("Usage: INSTALL [/Y] [/U]\n");
            printf("  /Y  install with the default answers\n");
            printf("  /U  uninstall\n");
            return 1;
        }
    }

    banner();
    programDir(argv[0], from, sizeof(from));

    if(doUninstall)
    {
        from[strlen(from) - 1] = 0;
        if(!uninstall(from)) return 1;
        printf("\ncloudy is uninstalled. Reboot to unload the packet driver.\n");
        return 0;
    }

    pathJoin(s.dir, from, "CLOUD.EXE");
    if(!fileExists(s.dir))
    {
        printf("CLOUD.EXE is missing next to INSTALL.EXE, is the package complete?\n");
        return 1;
    }

    defaults(&s);
    loadPrevious(&s);
    for(;;)
    {
        askSettings(&s);
        printSettings(&s);
        if(unattended || askYesNo("\nInstall with these settings", 1)) break;
    }

    if(!install(&s, from))
    {
        printf("\nThe installation failed.\n");
        return 1;
    }

    printf("\ncloudy is installed.\n");
    if(s.editAutoexec)
    {
        printf("Reboot, then type %s to start it.\n", s.addPath ? "CLOUDY" : strcat(s.dir, "\\CLOUDY"));
    }
    else
    {
        printf("Run %s\\NETSTART.BAT once after booting, then %s\\CLOUDY.BAT\n", s.dir, s.dir);
    }
    printf("On the Linux machine run: python3 peer/cloudy_peer.py --bind 0.0.0.0\n");
    return 0;
}
