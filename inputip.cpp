#include "inputip.h"
#include "dos_cgui.h"
#include "dos_scrn.h"
#include "log.h"
#include "messager.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <strstream.h>

GuiState_InputIp::GuiState_InputIp() : GuiState(), ip(new char*[4]), currentSegment(0), errorMode (0)
{
    // bound to 80x25 screen
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

GuiState_InputIp::~GuiState_InputIp()
{
    free(ip[0]);
    free(ip[1]);
    free(ip[2]);
    free(ip[3]);

    delete ip;
}

void GuiState_InputIp::paint(void *screen)
{
    const char* errStr = GuiStatemachine::instance().getError();
    log_debug() << "*********************' Error:" << errStr << ":" << strlen((char*)errStr);

    connect_window(screen, ip, errorMode, errStr);

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

void GuiState_InputIp::onEnter()
{
    log_info() << "Trying to advance ...........................................................";
    messager(MSG_IP_ENTERED, (void*)getIp());
    log_info() << "Advanced with error" << errorMode;

}

void GuiState_InputIp::onRightArrow()
{
}

void GuiState_InputIp::onRefreshContent()
{
    getCursor()->placeCursor(ipPositions[currentSegment] + strlen( ip[currentSegment] ), 14);
}

void GuiState_InputIp::onChar(char c)
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

void GuiState_InputIp::onTab()
{
    currentSegment ++;
    if(currentSegment == 4)
    {
        currentSegment = 0;
    }
}

void GuiState_InputIp::onBackspace()
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

const char* GuiState_InputIp::getIp()
{
    static char sip[128] = {0};
    sprintf(sip, "%s.%s.%s.%s", ip[0], ip[1], ip[2], ip[3]);
    return sip;
}
