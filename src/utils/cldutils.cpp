#ifndef UTILS_CPP
#define UTILS_CPP

#include "cldutils.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

const char* __far rand_string(size_t size)
{
    static char str[128];
    memset(str, 0, 128);

    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJK123456789";
    if (size)
    {
        --size;
        for (size_t n = 0; n < size; n++)
        {
            int key = rand() % (int) (sizeof charset - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
    return str;
}


/*
 * Renders a human readable size from the bytes
 */
const char* __far renderHumanReadableSize(unsigned long size)
{
    static const char* __far suffix[] = {"b", "K", "M", "G", "T"};
    static const char* __far format_0 = "%.01lf%s";
    static char output[48];

    memset(output, 0, sizeof(output));

    int i = 0;
    double dBytes = size;
    if(size > 1024)
    {
        for(i=0; (size/1024) > 0 && i < 4; i++, size /= 1024)
        {
            dBytes = size / 1024.0;
        }
    }
    if(i == 0)
    {
        sprintf(output, "%lub", size);
    }
    else
    {
        sprintf(output, format_0, dBytes, suffix[i]);
    }
    return output;
}

/*
 * Renders a human readable size from kilobytes
 */
const char* __far renderHumanReadableSizeKB(unsigned long kb)
{
    static const char* __far suffix[] = {"K", "M", "G", "T"};
    static char output[48];

    int i = 0;
    double d = kb;
    while(d >= 1024.0 && i < 3)
    {
        d /= 1024.0;
        i++;
    }
    sprintf(output, "%.01lf%s", d, suffix[i]);
    return output;
}

/*
 * Turns a long file name into an uppercase DOS 8.3 one
 */
void makeDosName(const char* longName, char* out)
{
    static const char* invalid = " \"*+,/:;<=>?[\\]|";

    // the extension starts at the last dot, unless the name starts with it
    const char* lastDot = strrchr(longName, '.');
    if(lastDot == longName)
    {
        lastDot = NULL;
    }

    int o = 0;
    const char* p = longName;
    for(; *p && p != lastDot && o < 8; p++)
    {
        unsigned char c = (unsigned char)*p;
        if(c == '.')
        {
            continue;
        }
        out[o++] = (c < 32 || c > 126 || strchr(invalid, c)) ? '_' : toupper(c);
    }

    if(o == 0)
    {
        strcpy(out, "NONAME");
        o = 6;
    }

    if(lastDot && lastDot[1])
    {
        out[o++] = '.';
        int e = 0;
        for(p = lastDot + 1; *p && e < 3; p++, e++)
        {
            unsigned char c = (unsigned char)*p;
            out[o++] = (c < 32 || c > 126 || strchr(invalid, c)) ? '_' : toupper(c);
        }
    }

    out[o] = '\0';
}

#endif // UTILS_CPP
