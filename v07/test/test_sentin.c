#include <stdio.h>
#include <unistd.h>
#include "../TB2820_Api.h"

int main()
{
    TB2820_Handler *handler = NULL;
    int openRet = TB2820_Open(&handler, 0);
    if(handler == NULL)
    {
        printf("handler is nullptr, open ret = %d\n", openRet);
    }
    int channel = 56;
    for(; channel < 60; ++channel)
    {
        TB2820_SetDebounceTime(handler, channel, 100);
        TB2820_SetDIThreshold(handler, channel, 2);
        TB2820_SetChannelMode(handler, channel, TB2820_MODE_SENTIN);
        TB2820_SetTickPeriodAndTolerance(handler, channel, 3, 0.1);
        TB2820_SetNibbleHighTick(handler, channel, 7);
        TB2820_SetLowTick(handler, channel, 5);
        TB2820_SetSyncHighTick(handler, channel, 51);
        TB2820_SetPauseEnable(handler, channel, 1);
        TB2820_SetNibblePerMessage(handler, channel, 1);
        TB2820_SetSentInTimeout(handler, channel, 3);
        TB2820_SetPolarity(handler, channel, TB2820_SENTIN_POLARITY_NONREVERSAL);
        TB2820_SetCRCImplementationType(handler, channel, TB2820_SENTIN_LEGACY);
    }

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

        int ch = 0;
        float actualTickPeriodTime = 0;
        unsigned int nibbleData = 0, AllTicks = 0, diagnosticStatus = 0, serialData = 0;

        TB2820_GetActualTickPeriodTime(handler, ch, &actualTickPeriodTime);
        TB2820_GetNibbleData(handler, ch, &nibbleData);
        TB2820_GetAllTicks(handler, ch, &AllTicks);
        TB2820_GetDiagnosticStatus(handler, ch, &diagnosticStatus);
        TB2820_GetSerialData(handler, ch, &serialData);
        printf("sentin ActualTickPeriodTime = %f, nibbleData = %u, AllTicks = %u, diagnosticStatus = %u, serialData = %u\n", actualTickPeriodTime, nibbleData, AllTicks, diagnosticStatus, serialData);

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