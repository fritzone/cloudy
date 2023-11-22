#include "inputip.h"
#include "dos_cgui.h"
#include "dos_scrn.h"
#include "dos_neti.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <strstream.h>

InputIpState::InputIpState(State* next, State* prev) : State(next, prev),
    ip (new char*[4]), currentSegment(0)
{

    ipPositions[0] = 33; ipPositions[1] = 37; ipPositions[2] = 41; ipPositions[3] = 45;

    ip[0] = (char*)calloc(4, 1);
    strcpy(ip[0], "10");
    ipPosCtrs[0] = 2;

    ip[1] = (char*)calloc(4, 1);
    strcpy(ip[1], "0");
    ipPosCtrs[1] = 1;

    ip[2] = (char*)calloc(4, 1);
    strcpy(ip[2], "2");
    ipPosCtrs[2] = 1;

    ip[3] = (char*)calloc(4, 1);
    strcpy(ip[3], "2");
    ipPosCtrs[3] = 1;
}

InputIpState::~InputIpState()
{
    free(ip[0]);
    free(ip[1]);
    free(ip[2]);
    free(ip[3]);

    delete ip;
}

void InputIpState::paint(void *screen)
{
    connect_window(screen, ip);

    for(int i=0; i<4; i++)
    {
        int ci = atoi(ip[i]);
        if(ci <= 255)
        {
            writeString(ipPositions[i], 14, Black, White, ip[i], screen);
        }
        else
        {
            writeString(ipPositions[i], 14, Red, White, ip[i], screen);
        }
    }
}

void InputIpState::onEnter()
{
    // the network interface
    DosMTcpIpIface netIface;
    if(!netIface.setup())
    {
        fprintf(stderr, "Network setup failed");
        return;
    }

    void* clientSocket = netIface.provide_socket();
    if(!netIface.connect(clientSocket, getIp()))
    {
        netIface.shutdown();
        return;
    }


}

void InputIpState::onRightArrow()
{
}

void InputIpState::onRefreshContent()
{
    getCursor()->placeCursor(ipPositions[currentSegment] + strlen( ip[currentSegment] ), 14);
}

void InputIpState::onChar(char c)
{
    if(!isdigit(c) && c != '.')
    {
        return;
    }

    if(c == '.')
    {
        onTab();
        return;
    }

    if(ipPosCtrs[currentSegment] < 3)
    {
        ip[currentSegment][ipPosCtrs[currentSegment]] = c;

        ipPosCtrs[currentSegment] ++;

        if(strlen(ip[currentSegment]) == 3)
        {
            onTab();
        }
    }
}

void InputIpState::onTab()
{
    currentSegment ++;
    if(currentSegment == 4)
    {
        currentSegment = 0;
    }
}

void InputIpState::onBackspace()
{
    if(strlen(ip[currentSegment]) >= 1)
    {
        ip[currentSegment][ipPosCtrs[currentSegment] - 1] = 0;
        ipPosCtrs[currentSegment] --;
    }
    else
    {
        currentSegment --;
        if(currentSegment == -1)
        {
            currentSegment = 3;
        }
    }
}

const char* InputIpState::getIp()
{
    static char sip[128] = {0};
    sprintf(sip, "%s.%s.%s.%s", ip[0], ip[1], ip[2], ip[3]);
    return sip;
}
