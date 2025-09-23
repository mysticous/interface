#include <stdio.h>
#include <unistd.h>
#include "../TB2820_Api.h"

#define HALL 1
// #define HALL3 1

int main()
{
    TB2820_Handler *handler = NULL;
    int openRet = TB2820_Open(&handler, 0);
    if(handler == NULL)
    {
        printf("handler is nullptr, open ret = %d\n", openRet);
    }
    int channel = 0;
    for(; channel < 36; ++channel)
    {
        TB2820_SetDebounceTime(handler, channel, 100);
        TB2820_SetDIThreshold(handler, channel, 2);
#ifdef HALL
        TB2820_SetChannelMode(handler, channel, TB2820_MODE_HALL);
#elif HALL3
        TB2820_SetChannelMode(handler, channel, TB2820_MODE_HALL3);
#endif
    }
#ifdef HALL
    TB2820_SetHallPolarity(handler, 0x0, 0x0);
    TB2820_SetHallClearEnable(handler, 0x0, 0x0);
#elif HALL3
    TB2820_SetHall3ClearEnable(handler, 0x0);
    int group = 0;
    TB2820_SetHall3StopTimeout(handler, group, 123);
#endif
    TB2820_AdjustAndSetAllChannelsMode(handler);
    sleep(1);
    // while(1)
    {
        usleep(100000);
        int getRet = TB2820_GetData(handler);
        if(getRet != TB_RETURN_OK)
        {
            printf("Error occurred while get dma data, ret = %d", getRet);
        }
#ifdef HALL
        int ch = 0;
        unsigned int totalCount = 0, risingCount = 0, fallingCount = 0, status = 0;
        TB2820_GetHallSignalCount(handler, ch, &totalCount);
        TB2820_GetHallRisingEdgeCount(handler, ch, &risingCount);
        TB2820_GetHallFallingEdgeCount(handler, ch, &fallingCount);
        TB2820_GetHallSignalStatus(handler, channel, &status);
        printf("Hall totalCount = %u, risingCount = %u, fallingCount = %u, status = %u\n", totalCount, risingCount, fallingCount, status);
#elif HALL3
        unsigned int hall3Group = 0;
        unsigned int motorSpeed = 0, realTimeAngle = 0, direction = 0;
        TB2820_GetHall3MotorSpeed(handler, ch, &motorSpeed);
        TB2820_GetHall3RealTimeAngle(handler, ch, &realTimeAngle);
        TB2820_GetHall3Direction(handler, hall3Group, &direction);
        printf("hall3 motorSpeed = %u, realTimeAngle = %u, direction = %u\n", motorSpeed, realTimeAngle, direction);
#endif
    }
    char q;
    scanf("%c", &q);
    while(q != 'q')
    {
        usleep(1000000);
        scanf("%c", &q);
    }
    TB2820_Close(&handler);

    return 0;
}