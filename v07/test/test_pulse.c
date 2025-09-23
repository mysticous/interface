#include <stdio.h>
#include <unistd.h>
#include "../TB2820_Api.h"

// #define PULSE 1
// #define PHASE2_STEPPER 1
#define PHASE4_STEPPER 1

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
        TB2820_SetDIThreshold(handler, channel, 2);
        TB2820_SetDebounceTime(handler, channel, 100);
#ifdef PULSE
        TB2820_SetChannelMode(handler, channel, TB2820_MODE_PULSE);
#elif PHASE2_STEPPER
        TB2820_SetChannelMode(handler, channel, TB2820_MODE_PHASE2_STEP4);
        //TB2820_SetChannelMode(handler, channel, TB2820_MODE_PHASE2_STEP8);
#elif PHASE4_STEPPER
        TB2820_SetChannelMode(handler, channel, TB2820_MODE_PHASE4_SINGLE_STEP4);
        //TB2820_SetChannelMode(handler, channel, TB2820_MODE_PHASE4_DOUBLE_STEP4);
        //TB2820_SetChannelMode(handler, channel, TB2820_MODE_PHASE4_STEP8);
#endif
    }

    TB2820_SetPulsePolarity(handler, 0x0);
#ifdef PULSE
    TB2820_SetPulseClearEnable(handler, 0x0);
#elif PULSE2_STEPPER
    int group = 0;
    TB2820_SetStepper2InitSteps(handler, group, 1);
    TB2820_SetStepperClearEnable(handler, 0x0);
#elif PULSE4_STEPPER
    TB2820_SetStepper4InitSteps(handler, group, 1);
    TB2820_SetStepperClearEnable(handler, 0x0);
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
#ifdef PULSE
        int ch = 0;
        unsigned int totalCount = 0, risingCount = 0, fallingCount = 0, status = 0;
        TB2820_GetPluseSignalCount(handler, ch, &totalCount);
        TB2820_GetPulseRisingEdgeCount(handler, ch, &risingCount);
        TB2820_GetPulseFallingEdgeCount(handler, ch, &fallingCount);
        TB2820_GetPulseSignalStatus(handler, ch, &status);
        printf("Pulse totalCount = %u, risingCount = %u, fallingCount = %u, status = %u\n", totalCount, risingCount, fallingCount, status);
#elif PHASE2_STEPPER
        unsigned int phase2Group = 0;
        unsigned int step = 0, phase = 0, status = 0;
        TB2820_GetStepper2Steps(handler, phase2Group, &step);
        TB2820_GetStepper2Phase(handler, phase2Group, &phase);
        TB2820_GetStep2SignalStatus(handler, phase2Group, &status);
        printf("PHASE2_STEPPER step = %u, phase = %u, status = %u\n", step, phase, status);
#elif PHASE4_STEPPER
        unsigned int phase4Group = 0;
        unsigned int step = 0, phase = 0, status = 0;
        TB2820_GetStepper4Steps(handler, phase4Group, &step);
        TB2820_GetStepper4Phase(handler, phase4Group, &phase);
        TB2820_GetStep4SignalStatus(handler, phase4Group, &status);
        printf("PHASE4_STEPPER step = %u, phase = %u, status = %u\n", step, phase, status);
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