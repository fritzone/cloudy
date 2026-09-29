#include "base64.h"

static const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

size_t base64_encode(const unsigned char* in, size_t len, char* out)
{
    size_t o = 0;
    size_t i = 0;

    for(; i + 2 < len; i += 3)
    {
        out[o++] = alphabet[in[i] >> 2];
        out[o++] = alphabet[((in[i] & 0x03) << 4) | (in[i + 1] >> 4)];
        out[o++] = alphabet[((in[i + 1] & 0x0f) << 2) | (in[i + 2] >> 6)];
        out[o++] = alphabet[in[i + 2] & 0x3f];
    }

    if(i < len)
    {
        out[o++] = alphabet[in[i] >> 2];
        if(i + 1 < len)
        {
            out[o++] = alphabet[((in[i] & 0x03) << 4) | (in[i + 1] >> 4)];
            out[o++] = alphabet[(in[i + 1] & 0x0f) << 2];
        }
        else
        {
            out[o++] = alphabet[(in[i] & 0x03) << 4];
            out[o++] = '=';
        }
        out[o++] = '=';
    }

    out[o] = '\0';
    return o;
}

static int decodeChar(char c)
{
    if(c >= 'A' && c <= 'Z') return c - 'A';
    if(c >= 'a' && c <= 'z') return c - 'a' + 26;
    if(c >= '0' && c <= '9') return c - '0' + 52;
    if(c == '+') return 62;
    if(c == '/') return 63;
    return -1;
}

long base64_decode(const char* in, unsigned char* out, size_t outSize)
{
    size_t o = 0;
    unsigned long acc = 0;
    int bits = 0;

    for(; *in && *in != '='; in++)
    {
        int v = decodeChar(*in);
        if(v < 0)
        {
            continue;
        }

        acc = (acc << 6) | v;
        bits += 6;
        if(bits >= 8)
        {
            bits -= 8;
            if(o >= outSize)
            {
                return -1;
            }
            out[o++] = (unsigned char)((acc >> bits) & 0xff);
        }
    }

    return (long)o;
}
