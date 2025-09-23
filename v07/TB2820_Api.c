#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "TB2820_Api.h"
#include "TB2820_Register.h"
#define TB2820_DEV_PATH_PREFIX "/dev/TBPCIE_2820_"
// #define TB2820_DEV_MMAP_SIZE TB_MEM_MB + 128 + 512
#define TB2820_DEV_MMAP_SIZE TB_MEM_MB
#define TB2820_VERSION "3.1.0"
#define TB2820_PCBA_SN_LENGTH 12

// Helper function to write to FPGA registers
static void WriteToFPGA(TB2820_Handler *handler, TB_U32 reg, TB_U32 value)
{
    TB_REG_WRITE(handler->bar0Address, reg, value);
}

static TB_U32 CalculateThresholdFPGAValue(TB_Float threshold)
{
    const TB_Float voltageRatio = 31.6f / (360.0f + 31.6f);
    const TB_Float referenceVoltage = 2.048f;
    const TB_U32 adcResolution = 4096;
    
    return (TB_U32)((threshold * voltageRatio * adcResolution) / referenceVoltage);
}

static TB_U32 CalculateTimeoutFPGAValue(TB_U32 time)
{
    const TB_U32 coefficient  = 100 * 1000;
    return (TB_U32)(coefficient * time);
}

static void CountChannelsPerMode(TB2820_Handler *handler)
{
    assert(handler != NULL);
    int i = 0;
    for(; i < TB2820_CHANNEL_MAX; ++i)
    {
        switch(handler->info[i].mode)
        {
            case TB2820_MODE_PWM:
            {
                handler->pwmChannelCount++;
                break;
            }
            case TB2820_MODE_SENTIN:
            {
                handler->sentChannelCount++;
            }
            case TB2820_MODE_HALL:
            {
                handler->hallChannelCount++;
                break;
            }
            case TB2820_MODE_HALL3:
            {
                handler->hall3ChannelCount++;
                break;
            }
            case TB2820_MODE_PULSE:
            {
                handler->pulseChannelCount++;
                break;
            }
            case TB2820_MODE_PHASE2_STEP4:
            {
                handler->stepper24ChannelCount++;
                break;
            }
            case TB2820_MODE_PHASE2_STEP8:
            {
                handler->stepper28ChannelCount++;
                break;
            }
            case TB2820_MODE_PHASE4_SINGLE_STEP4:
            {
                handler->stepper44ChannelCount++;
                break;
            }
            case TB2820_MODE_PHASE4_DOUBLE_STEP4:
            {
                handler->stepper444ChannelCount++;
                break;
            }
            case TB2820_MODE_PHASE4_STEP8:
            {
                handler->stepper48ChannelCount++;
                break;
            }
            default:
                break;

        }
    }
}

TB_Return TB2820_Open(TB2820_Handler **handler, TB_U32 devnum)
{
    if(!handler)
    {
        return TB_RETURN_NOT_OK;
    }
    char dmaName[TB2820_DEVICE_NAME_SIZE];
    *handler= malloc(sizeof(TB2820_Handler));
    TB2820_Handler *newHandler = *handler;

    // Open UIO device
    snprintf(newHandler->deviceName, sizeof(newHandler->deviceName), 
            "%s%u", TB2820_DEV_PATH_PREFIX, devnum);
    newHandler->fd = open(newHandler->deviceName, O_RDWR);
    if (newHandler->fd < 0) 
    {
        free(*handler);
        *handler = NULL;
        return TB_RETURN_NOT_OK;
    }

    // MMap UIO
    newHandler->bar0Address = mmap(NULL, TB2820_DEV_MMAP_SIZE, PROT_READ | PROT_WRITE, 
                            MAP_SHARED, newHandler->fd, 0);
    if (newHandler->bar0Address == MAP_FAILED) 
    {
        close(newHandler->fd);
        free(*handler);
        *handler = NULL;
        return TB_RETURN_NOT_OK;
    }
    TB_REG_WRITE(newHandler->bar0Address, TB2820_BOARDINFO_SOFT_RESET_REGISTER_OFFSET, TB2820_RESET);
    usleep(1000);
    TB_REG_WRITE(newHandler->bar0Address, TB2820_BOARDINFO_SOFT_RESET_REGISTER_OFFSET, TB2820_NO_RESET);

    // Open DMA device
    snprintf(dmaName, sizeof(dmaName), "%s%u_c2h_0", TB2820_DEV_PATH_PREFIX, devnum);
    newHandler->dmaFd = open(dmaName, O_RDWR);
    if (newHandler->dmaFd< 0) 
    {
        munmap(newHandler->bar0Address, TB2820_DEV_MMAP_SIZE);
        close(newHandler->fd);
        free(*handler);
        *handler = NULL;
        return TB_RETURN_NOT_OK;
    }

    return TB_RETURN_OK;
}

void TB2820_Close(TB2820_Handler **handler)
{
    if(!handler)
    {
        return;
    }
    TB2820_Handler *newHandler = *handler;
    int i = 0;
    for(i = 0x1000; i < 0x11EC; i = i + 4)
    {
        TB_REG_WRITE(newHandler->bar0Address, i, 0);
    }
    TB_U32 defaultValue[] = {
        300, 300, 300, 300, 
        30, 30, 30, 30,
        0x7777, 0x5555, 0x33333333, 0,
        0x6666, 300000, 300000, 300000,
        300000, 0, 0, 0,
        0, 0xf
    };
    for(i = 0; i < 22; ++i)
    {
        int offset = 0x1200 + i * 4;
        TB_REG_WRITE(newHandler->bar0Address, offset, defaultValue[i]);
    }
    TB_REG_WRITE(newHandler->bar0Address, TB2820_BOARDINFO_SOFT_RESET_REGISTER_OFFSET, TB2820_RESET);
    usleep(1000);
    TB_REG_WRITE(newHandler->bar0Address, TB2820_BOARDINFO_SOFT_RESET_REGISTER_OFFSET, TB2820_NO_RESET);
    if(!newHandler->bar0Address)
    {
        munmap(newHandler->bar0Address, TB2820_DEV_MMAP_SIZE);
    }
    if(newHandler->fd >= 0)
    {
        close(newHandler->fd);
    }
    if(newHandler->dmaFd >= 0)
    {
        close(newHandler->dmaFd);
    }
    free(newHandler);
    *handler = NULL;
}

TB_Return TB2820_SetCommonThreshold(TB2820_Handler *handler, TB_U32 channel, TB_Float threshold)
{
    if (!handler)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if (channel >= TB2820_CHANNEL_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    if (threshold < 0 || threshold > 25.0f)
    {
        return TB_RETURN_NOT_OK;
    }

    TB_U32 fpga_value = CalculateThresholdFPGAValue(threshold);
    WriteToFPGA(handler, TB2820_DI_CHANNEL_THRESHOLD_OFFSET + channel * 4, fpga_value);
    return TB_RETURN_OK;
}

TB_Return TB2820_SetCommonTimeoutTime(TB2820_Handler *handler, TB_U32 channel, TB_U32 time)
{
    if (!handler)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if (channel >= TB2820_CHANNEL_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }
    
    TB_U32 fpga_value = CalculateTimeoutFPGAValue(time);
    WriteToFPGA(handler, TB2820_DI_CHANNEL_TIMEOUT_OFFSET + channel * 4, fpga_value);
    return TB_RETURN_OK;
}

TB_Return TB2820_SetChannelMode(TB2820_Handler *handler, TB_U32 channel, TB2820_ChannelMode mode)
{
    if (!handler)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if (channel >= TB2820_CHANNEL_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    TB_Bool validMode = TB_FALSE;

    /* Determine channel range and validate mode */
    if(channel < 36)
    {
        validMode = (mode == TB2820_MODE_DI) 
                    || (mode == TB2820_MODE_PWM)
                    || (mode == TB2820_MODE_HALL) 
                    || (mode == TB2820_MODE_HALL3);
    }
    else if( channel >= 36 && channel < 60)
    {
        validMode = (mode == TB2820_MODE_DI) 
                    || (mode == TB2820_MODE_PWM)
                    || (mode == TB2820_MODE_PULSE) 
                    || (mode == TB2820_MODE_PHASE2_STEP4)
                    || (mode == TB2820_MODE_PHASE2_STEP8) 
                    || (mode == TB2820_MODE_PHASE4_SINGLE_STEP4)
                    || (mode == TB2820_MODE_PHASE4_DOUBLE_STEP4) 
                    || (mode == TB2820_MODE_PHASE4_STEP8)
                    || (mode == TB2820_MODE_SENTIN);
    }
    else
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    /* Validate mode for the channel */
    if (!validMode) 
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }

    /* Update channel mode information */
    handler->info[channel].mode = mode;

    return TB_RETURN_OK;
}

TB_Return TB2820_AdjustAndSetAllChannelsMode(TB2820_Handler *handler)
{
    if (!handler)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    int channel = 0;
    for(; channel < TB2820_CHANNEL_MAX; ++channel)
    {
        TB2820_ChannelMode mode = handler->info[channel].mode;
        printf("TB2820_AdjustAndSetAllChannelsMode channel = %d, mode = %d\n", channel, mode);
        if(mode == TB2820_MODE_HALL3)
        {
            TB_U32 group = channel / 3;
            handler->info[group * 3].mode = mode;
            handler->info[group * 3 + 1].mode = mode;
            handler->info[group * 3 + 2].mode = mode;
        }
        
        if((mode == TB2820_MODE_PHASE4_DOUBLE_STEP4) || (mode == TB2820_MODE_PHASE4_STEP8) 
                || (mode ==TB2820_MODE_PHASE4_SINGLE_STEP4) || (mode == TB2820_MODE_PHASE2_STEP8) 
                || (mode == TB2820_MODE_PHASE2_STEP4))
        {
            TB_U32 group = channel / 4;
            handler->info[group * 4].mode = mode;
            handler->info[group * 4 + 1].mode = mode;
            handler->info[group * 4 + 2].mode = mode;
            handler->info[group * 4 + 3].mode = mode;
        }
    }
    for(channel = 0; channel < TB2820_CHANNEL_MAX; channel += 8)
    {
        TB_U32 modeReg = handler->info[channel].mode 
                        + (handler->info[channel + 1].mode << 4)
                        + (handler->info[channel + 2].mode << 8)
                        + (handler->info[channel + 3].mode << 12)
                        + (handler->info[channel + 4].mode << 16)
                        + (handler->info[channel + 5].mode << 20)
                        + (handler->info[channel + 6].mode << 24)
                        + (handler->info[channel + 7].mode << 28);
        TB_REG_WRITE(handler->bar0Address, TB2820_MODE_OFFSET + (channel / 8) * 4, modeReg);
    }
    CountChannelsPerMode(handler);
    return TB_RETURN_OK;
}

TB_Return TB2820_SetHallPolarity(TB2820_Handler *handler, TB_U32 hallMask1_32, TB_U32 hallMask33_36)
{
    if (!handler)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    TB_REG_WRITE(handler->bar0Address, TB2820_HALL_POLARITY_OFFSET, hallMask1_32);
    TB_REG_WRITE(handler->bar0Address, TB2820_HALL_POLARITY_OFFSET + 4, hallMask33_36);
    return TB_RETURN_OK;
}

TB_Return TB2820_SetPulsePolarity(TB2820_Handler *handler, TB_U32 pulseMask)
{
    if (!handler)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    TB_REG_WRITE(handler->bar0Address, TB2820_PULSE_POLARITY_OFFSET, pulseMask);
    return TB_RETURN_OK;
}

TB_Return TB2820_SetStepper2InitSteps(TB2820_Handler *handler, TB_U32 group, TB_U32 steps)
{
    if (!handler)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if(group > TB2820_MOTOR_GROUP_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    TB_REG_WRITE(handler->bar0Address, TB2820_STEPPER2_INIT_STEPS_OFFSET + group * 4, steps);
    return TB_RETURN_OK;
}

TB_Return TB2820_SetStepper4InitSteps(TB2820_Handler *handler, TB_U32 group, TB_U32 steps)
{
    if (!handler)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if(group > TB2820_MOTOR_GROUP_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    TB_REG_WRITE(handler->bar0Address, TB2820_STEPPER4_INIT_STEPS_OFFSET + group * 4, steps);
    return TB_RETURN_OK;
}

TB_Return TB2820_SetDebounceTime(TB2820_Handler *handler, TB_U32 channel, TB_U32 time)
{
    if (!handler)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if(channel > TB2820_CHANNEL_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    if(time == 0)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }

    TB_REG_WRITE(handler->bar0Address, TB2820_DEBOUNCE_TIME_OFFSET + channel * 4, time / 10);
    return TB_RETURN_OK;
}

TB_Return TB2820_SetHallClearEnable(TB2820_Handler *handler, TB_U32 clearMask1_32, TB_U32 clearMask33_36)
{
    if (!handler)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    TB_REG_WRITE(handler->bar0Address, TB2820_HALL_CLEAR_ENABLE_OFFSET, clearMask1_32);
    TB_REG_WRITE(handler->bar0Address, TB2820_HALL_CLEAR_ENABLE_OFFSET + 4, clearMask33_36);
    return TB_RETURN_OK;
}

TB_Return TB2820_SetPulseClearEnable(TB2820_Handler *handler, TB_U32 clearMask)
{
    if (!handler)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    TB_REG_WRITE(handler->bar0Address, TB2820_PULSE_CLEAR_ENABLE_OFFSET, clearMask);
    return TB_RETURN_OK;
}

TB_Return TB2820_SetStepperClearEnable(TB2820_Handler *handler, TB_U32 groupMask)
{
    if (!handler)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    TB_REG_WRITE(handler->bar0Address, TB2820_MOTOR_STEP_CLEAR_ENABLE_OFFSET, groupMask);
    return TB_RETURN_OK;
}

TB_Return TB2820_SetHall3ClearEnable(TB2820_Handler *handler, TB_U32 groupMask)
{
    if (!handler)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    TB_REG_WRITE(handler->bar0Address, TB2820_HALL3_CLEAR_ENABLE_OFFSET, groupMask);
    return TB_RETURN_OK;
}

TB_Return TB2820_SetHall3StopTimeout(TB2820_Handler *handler, TB_U32 group, TB_U32 time)
{
    if (!handler)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if(group >= TB2820_HALL3_GROUP_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    TB_REG_WRITE(handler->bar0Address, TB2820_HALL3_STOP_TIMEOUT_OFFSET + group * 4, time / 10);
    return TB_RETURN_OK;
}

TB_Return TB2820_SetTickPeriodAndTolerance(TB2820_Handler *handler, TB_U32 channel, TB_Float periodUs, TB_Float tolerancePercent)
{
    if (!handler)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if (channel < 56 || channel >= TB2820_CHANNEL_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    if (periodUs < 3.0f || periodUs > 90.0f)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }

    if (tolerancePercent < 0.05f || tolerancePercent > 0.25f)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }

    TB_U32 periodVal = (TB_U32)(100 * periodUs);
    WriteToFPGA(handler, TB2820_CHANNEL_TICK_PERIOD_OFFSET + channel * 4, periodVal);
    TB_U32 toleranceVal = (TB_U32)(100 * periodUs * tolerancePercent);
    WriteToFPGA(handler, TB2820_CHANNEL_TICK_PERIOD_TOLERANCE_OFFSET + channel * 4, toleranceVal);
    return TB_RETURN_OK;
}

TB_Return TB2820_SetNibbleHighTick(TB2820_Handler *handler, TB_U32 channel, TB_U32 ticks)
{
    if (!handler)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if (channel < 56 || channel >= TB2820_CHANNEL_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    TB_U32 pos = channel - 56;
    TB_U32 val = TB_REG_READ(handler->bar0Address, TB2820_CHANNEL_ZERO_NIBBLE_HIGH_TICK_OFFSET);
    TB_U32 mask = ~(0xFu << (pos * 4));  /* Clear 4-bit field */
    TB_U32 tickBits = (ticks & 0x0Fu) << (pos * 4);
    TB_REG_WRITE(handler->bar0Address, TB2820_CHANNEL_ZERO_NIBBLE_HIGH_TICK_OFFSET, (val & mask) | tickBits);
    return TB_RETURN_OK;
}

TB_Return TB2820_SetLowTick(TB2820_Handler *handler, TB_U32 channel, TB_U32 ticks)
{
    if (!handler)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if (channel < 56 || channel >= TB2820_CHANNEL_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    TB_U32 pos = channel - 56;
    TB_U32 val = TB_REG_READ(handler->bar0Address, TB2820_CHANNEL_LOW_TICK_OFFSET);
    TB_U32 mask = ~(0xFu << (pos * 4));  /* Clear 4-bit field */
    TB_U32 tickBits = (ticks & 0x0Fu) << (pos * 4);
    TB_REG_WRITE(handler->bar0Address, TB2820_CHANNEL_LOW_TICK_OFFSET, (val & mask) | tickBits);
    return TB_RETURN_OK;
}

TB_Return TB2820_SetSyncHighTick(TB2820_Handler *handler, TB_U32 channel, TB_U32 ticks)
{
    if (!handler)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if (channel < 56 || channel >= TB2820_CHANNEL_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    TB_U32 pos = channel - 56;
    TB_U32 val = TB_REG_READ(handler->bar0Address, TB2820_CHANNEL_SYN_HIGH_TICK_OFFSET);
    TB_U32 mask = ~(0xFFu << (pos * 8));  /* Clear 8-bit field */
    TB_U32 tickBits = (ticks & 0xFFu) << (pos * 8);
    TB_REG_WRITE(handler->bar0Address, TB2820_CHANNEL_SYN_HIGH_TICK_OFFSET, (val & mask) | tickBits);
    return TB_RETURN_OK;
}

TB_Return TB2820_SetPauseEnable(TB2820_Handler *handler, TB_U32 channel, TB_U32 enable)
{
    if (!handler)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if (channel < 56 || channel >= TB2820_CHANNEL_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    if(enable != TB2820_ENABLE && enable != TB2820_DISABLE)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }

    TB_U32 pos = channel - 56;
    TB_U32 val = TB_REG_READ(handler->bar0Address, TB2820_CHANNEL_PAUSE_EN_OFFSET);
    TB_U32 mask = ~(0xFFu << pos);  /* Clear 1-bit field */
    TB_U32 enableBits = (enable & 0x1u) << pos;
    TB_REG_WRITE(handler->bar0Address, TB2820_CHANNEL_PAUSE_EN_OFFSET, (val & mask) | enableBits);
    return TB_RETURN_OK;
}

TB_Return TB2820_SetNibblePerMessage(TB2820_Handler *handler, TB_U32 channel, TB_U32 nibbleCount)
{
    if (!handler)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if (channel < 56 || channel >= TB2820_CHANNEL_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    TB_U32 pos = channel - 56;
    TB_U32 val = TB_REG_READ(handler->bar0Address, TB2820_CHANNEL_NIBBLES_PER_MESSAGE_OFFSET);
    TB_U32 mask = ~(0xFu << (pos * 4));  /* Clear 4-bit field */
    TB_U32 nibbleCountBits = (nibbleCount & 0x0Fu) << (pos * 4);
    TB_REG_WRITE(handler->bar0Address, TB2820_CHANNEL_NIBBLES_PER_MESSAGE_OFFSET, (val & mask) | nibbleCountBits);
    return TB_RETURN_OK;
}

TB_Return TB2820_SetSentInTimeout(TB2820_Handler *handler, TB_U32 channel, TB_Float timeoutMs)
{
    if (!handler)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if (channel < 56 || channel >= TB2820_CHANNEL_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    TB_U32 timeoutVal = 100000 * timeoutMs;
    WriteToFPGA(handler, TB2820_CHANNEL_TIME_OUT_OFFSET + (channel - 56) * 4, timeoutVal);
    return TB_RETURN_OK;
}

TB_Return TB2820_SetPolarity(TB2820_Handler *handler, TB_U32 channel, TB_U32 invert)
{
    if (!handler)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if (channel < 56 || channel >= TB2820_CHANNEL_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    if(invert != TB2820_SENTIN_POLARITY_REVERSAL && invert != TB2820_SENTIN_POLARITY_NONREVERSAL)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }

    TB_U32 pos = channel - 56;
    TB_U32 val = TB_REG_READ(handler->bar0Address, TB2820_CHANNEL_POLARITY_OFFSET);
    TB_U32 mask = ~(0x1u << pos);  /* Clear 1-bit field */
    TB_U32 invertBits = (invert & 0x1u) << pos;
    TB_REG_WRITE(handler->bar0Address, TB2820_CHANNEL_POLARITY_OFFSET, (val & mask) | invertBits);
    return TB_RETURN_OK;
}

TB_Return TB2820_SetCRCImplementationType(TB2820_Handler *handler, TB_U32 channel, TB_U32 crcType)
{
    if (!handler)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if (channel < 56 || channel >= TB2820_CHANNEL_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    if(crcType != TB2820_SENTIN_LEGACY && crcType != TB2820_SENTIN_RECOMMANDED)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }

    TB_U32 pos = channel - 56;
    TB_U32 val = TB_REG_READ(handler->bar0Address, TB2820_CHANNEL_CRC_IMP_TYPE_OFFSET);
    TB_U32 mask = ~(0x1u << pos);  /* Clear 1-bit field */
    TB_U32 crcTypeBits = (crcType & 0x1u) << pos;
    TB_REG_WRITE(handler->bar0Address, TB2820_CHANNEL_CRC_IMP_TYPE_OFFSET, (val & mask) | crcTypeBits);
    return TB_RETURN_OK;
}

TB_Return TB2820_SetSerDataMode(TB2820_Handler *handler, TB_U32 channel, TB_U32 mode)
{
    if (!handler)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if (channel < 56 || channel >= TB2820_CHANNEL_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    TB_U32 pos = channel - 56;
    TB_U32 val = TB_REG_READ(handler->bar0Address, TB2820_CHANNEL_SER_DATA_MODE_OFFSET);
    TB_U32 mask = ~(0x3u << pos);  /* Clear 1-bit field */
    TB_U32 modeBits = (mode & 0x3u) << pos;
    TB_REG_WRITE(handler->bar0Address, TB2820_CHANNEL_SER_DATA_MODE_OFFSET, (val & mask) | modeBits);
    return TB_RETURN_OK;
}

TB_Return TB2820_SetRxEnable(TB2820_Handler *handler, TB_U32 channel, TB_U32 enable)
{
    if (!handler)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if (channel < 56 || channel >= TB2820_CHANNEL_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    if(enable != TB2820_ENABLE && enable != TB2820_DISABLE)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }

    TB_U32 pos = channel - 56;
    TB_U32 val = TB_REG_READ(handler->bar0Address, TB2820_CHANNEL_RX_EN_OFFSET);
    TB_U32 mask = ~(0xFFu << pos);  /* Clear 1-bit field */
    TB_U32 enableBits = (enable & 0x1u) << pos;
    TB_REG_WRITE(handler->bar0Address, TB2820_CHANNEL_RX_EN_OFFSET, (val & mask) | enableBits);
    return TB_RETURN_OK;
}

TB_Return TB2820_ClearFault(TB2820_Handler *handler, TB_U32 channel)
{
    if (!handler)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if (channel < 56 || channel >= TB2820_CHANNEL_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    TB_U32 pos = channel - 56;
    TB_U32 val = TB_REG_READ(handler->bar0Address, TB2820_CHANNEL_CLEAR_FAULT_OFFSET);
    TB_U32 mask = ~(0xFFu << pos);  /* Clear 1-bit field */
    TB_U32 enableBits = (1 & 0x1u) << pos;
    TB_REG_WRITE(handler->bar0Address, TB2820_CHANNEL_CLEAR_FAULT_OFFSET, (val & mask) | enableBits);
    return TB_RETURN_OK;
}

size_t TB2820_GetDMABufferSize(TB2820_Handler *handler)
{
    assert(handler != NULL);
    size_t bufferSize = 8;
    int i = 0;
    for(; i < TB2820_CHANNEL_MAX; ++i)
    {
        TB2820_ChannelMode mode = handler->info[i].mode;
        switch (mode) 
        {
            case TB2820_MODE_PWM:
                bufferSize += 2;
                break;
            case TB2820_MODE_PHASE2_STEP4:
            case TB2820_MODE_PHASE2_STEP8:
            case TB2820_MODE_PHASE4_SINGLE_STEP4:
            case TB2820_MODE_PHASE4_DOUBLE_STEP4:
            case TB2820_MODE_PHASE4_STEP8:
            {
                if(i % 4 == 0)
                {
                    bufferSize += 2;
                }
                break;
            }
            case TB2820_MODE_HALL3:
            {
                if(i % 3 == 0)
                {
                    bufferSize += 8;
                }
                break;
            }
            case TB2820_MODE_PULSE:
            case TB2820_MODE_HALL:
                bufferSize += 3;
                break;
            case TB2820_MODE_SENTIN:
                bufferSize += 5;
                break;
                
            default:
                break;
        }
    }
    return bufferSize * 4;
}

TB_Return TB2820_ReadDMA(TB2820_Handler *handler, TB_U8* buffer, size_t size)
{
    assert(handler != NULL && buffer != NULL);
    lseek(handler->dmaFd, 0, SEEK_SET);
    int ret = read(handler->dmaFd, buffer, size);
    if(ret == -1)
    {
        return TB2820_ERR_READ_DMA_FAILED;
    }
    return TB_RETURN_OK;
}

TB_Return CheckBufferSize(TB2820_ChannelMode mode, size_t *currentOffset, size_t bufSize)
{
    assert(currentOffset != NULL);
    switch(mode)
    {
        case TB2820_MODE_PWM:
        {
            if (bufSize - *currentOffset < TB2820_PWM_CHANNEL_SIZE_BYTES)
            {
                return TB_RETURN_NOT_OK;
            }
            break;
        }
        case TB2820_MODE_SENTIN:
        {
            if (bufSize - *currentOffset < sizeof(SentIn))
            {
                return TB_RETURN_NOT_OK;
            }
            break;
        }
        case TB2820_MODE_HALL:
        {
            if (bufSize - *currentOffset < sizeof(SingleSignal))
            {
                return TB_RETURN_NOT_OK;
            }
            break;
        }
        case TB2820_MODE_HALL3:
        {
            if (bufSize - *currentOffset < sizeof(HallThreePhaseSignals))
            {
                return TB_RETURN_NOT_OK;
            }
            break;
        }
        case TB2820_MODE_PULSE:
        {
            if (bufSize - *currentOffset < sizeof(SingleSignal))
            {
                return TB_RETURN_NOT_OK;
            }
            break;
        }
        case TB2820_MODE_PHASE2_STEP4:
        case TB2820_MODE_PHASE2_STEP8:
        {
            if (bufSize - *currentOffset < sizeof(StepperMotor))
            {
                return TB_RETURN_NOT_OK;
            }
            break;
        }
        case TB2820_MODE_PHASE4_SINGLE_STEP4:
        case TB2820_MODE_PHASE4_DOUBLE_STEP4:
        case TB2820_MODE_PHASE4_STEP8:
        {
            if (bufSize - *currentOffset < sizeof(StepperMotor))
            {
                return TB_RETURN_NOT_OK;
            }
            break;
        }
        default:
            break;
    }
    return TB_RETURN_OK;
}

TB_Return TB2820_DemuxDMAData(TB2820_Handler *handler, TB_U8 *buffer, size_t *currentOffset, size_t bufSize, TB_U32 channel)
{
    assert(handler != NULL && buffer != NULL && currentOffset != NULL);

    TB2820_ChannelMode mode = handler->info[channel].mode;
    TB_Return ret = CheckBufferSize(mode, currentOffset, bufSize);
    if(ret == TB_RETURN_NOT_OK)
    {
        return TB_RETURN_NOT_OK;
    }
    switch(mode)
    {
        case TB2820_MODE_DI:
            break;
        case TB2820_MODE_PWM:
        {
            TB_U32 *pwmData = (TB_U32*)(buffer + *currentOffset);
            TB_U32 highCounter = pwmData[0];
            TB_U32 lowCounter = pwmData[1];
            TB_U32 totalCounter = highCounter + lowCounter;
            handler->info[channel].data.modeData.pwm.freq = (totalCounter != 0) ? 
                (TB_Float)TB2820_CLOCK_FREQ / (TB_Float)totalCounter : 0.0f;
            handler->info[channel].data.modeData.pwm.duty = (totalCounter != 0) ? 
                (TB_Float)highCounter / (TB_Float)totalCounter : 0.0f;
            *currentOffset += TB2820_PWM_CHANNEL_SIZE_BYTES;
            break;
        }
        case TB2820_MODE_SENTIN:
        {            
            SentIn *sentData = (SentIn*)(buffer + *currentOffset);
            handler->info[channel].data.modeData.sentIn = *sentData;
            *currentOffset += sizeof(SentIn);
            break;
        }
        case TB2820_MODE_HALL:
        {
            SingleSignal *hallData = (SingleSignal*)(buffer + *currentOffset);
            handler->info[channel].data.modeData.singleHall = *hallData;
            *currentOffset += sizeof(SingleSignal);
            break;
        }
        case TB2820_MODE_HALL3:
        {
            HallThreePhaseSignals *hall3Data = (HallThreePhaseSignals*)(buffer + *currentOffset);
            // Update three hall3 channel when detect the first channel in a hall3 group
            if(channel % 3 != 0)
            {
                break;
            }
            if(hall3Data->acutalRPM != 0)
            {
                hall3Data->acutalRPM = 1000000000 / hall3Data->acutalRPM;
            }
            handler->info[channel].data.modeData.threePhaseHall = *hall3Data;
            handler->info[channel + 1].data.modeData.threePhaseHall = *hall3Data;
            handler->info[channel + 2].data.modeData.threePhaseHall = *hall3Data;
            *currentOffset += sizeof(HallThreePhaseSignals);
            break;
        }
        case TB2820_MODE_PULSE:
        {
            SingleSignal *pulseData = (SingleSignal*)(buffer + *currentOffset);
            handler->info[channel].data.modeData.singlePulse = *pulseData;
            *currentOffset += sizeof(SingleSignal);
            break;
        }
        case TB2820_MODE_PHASE2_STEP4:
        case TB2820_MODE_PHASE2_STEP8:
        {
            StepperMotor *step2Data = (StepperMotor*)(buffer + *currentOffset);
            if(channel % 4 == 0)
            {
                handler->info[channel].data.modeData.motor2Phase = *step2Data;
                handler->info[channel + 1].data.modeData.motor2Phase = *step2Data;
                handler->info[channel + 2].data.modeData.motor2Phase = *step2Data;
                handler->info[channel + 3].data.modeData.motor2Phase = *step2Data;
                *currentOffset += sizeof(StepperMotor);
            }
            break;
        }
        case TB2820_MODE_PHASE4_SINGLE_STEP4:
        case TB2820_MODE_PHASE4_DOUBLE_STEP4:
        case TB2820_MODE_PHASE4_STEP8:
        {
            StepperMotor *step4Data = (StepperMotor*)(buffer + *currentOffset);
            if(channel % 4 == 0)
            {
                handler->info[channel].data.modeData.motor4Phase = *step4Data;
                handler->info[channel + 1].data.modeData.motor4Phase = *step4Data;
                handler->info[channel + 2].data.modeData.motor4Phase = *step4Data;
                handler->info[channel + 3].data.modeData.motor4Phase = *step4Data;
                *currentOffset += sizeof(StepperMotor);
            }
            break;
        }

        default:
            return TB_RETURN_NOT_OK;

    }
    return TB_RETURN_OK;
}

TB_Return TB2820_GetData(TB2820_Handler *handler)
{
    if (!handler)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    size_t bufSize = TB2820_GetDMABufferSize(handler);
    TB_U8 buffer[TB2820_DMABUFFER_MAXSIZE] = {0};

    TB_Return ret = TB2820_ReadDMA(handler, buffer, bufSize);
    if (ret != TB_RETURN_OK)
    {
        return ret;
    }

    TB_U32 *data = (TB_U32*)buffer;

    handler->diValues[0]                = data[0];  /**< DI CH1-32 */ 
    handler->diValues[1]                = data[1];  /**< DI CH33-60 */ 
    handler->singleHallStatus[0]        = data[2];  /**< CH1-32 Hall */ 
    handler->singleHallStatus[1]        = data[3];  /**< CH33-40 Hall */ 
    handler->threePhaseHallDirection    = data[4];  /**< 3-phase Hall direction */ 
    handler->singlePulseStatus          = data[5];  /**< Single-phase pulse */ 
    handler->twoPhaseStepperStatus      = data[6];  /**< 2-phase stepper */ 
    handler->fourPhaseStepperStatus     = data[7];  /**< 4-phase stepper */ 

    // Calculate base offsets for each protocol type
    size_t offsets[TB2820_MODE_MAX] = {0};
    offsets[TB2820_MODE_PWM] = TB2820_PWM_OFFSET;
    offsets[TB2820_MODE_SENTIN] = offsets[TB2820_MODE_PWM]
         + handler->pwmChannelCount * TB2820_PWM_CHANNEL_SIZE_BYTES;
    offsets[TB2820_MODE_HALL] = offsets[TB2820_MODE_SENTIN]
         + handler->sentChannelCount * sizeof(SentIn);
    offsets[TB2820_MODE_HALL3] = offsets[TB2820_MODE_HALL]
         + handler->hallChannelCount * TB2820_HALL_CHANNEL_SIZE_BYTES;
    // hall3GroupCount = handler->hall3ChannelCount / 3;
    offsets[TB2820_MODE_PULSE] = offsets[TB2820_MODE_HALL3]
         + (handler->hall3ChannelCount / 3) * TB2820_HALL3_GROUP_BYTES;
    offsets[TB2820_MODE_PHASE2_STEP4] = offsets[TB2820_MODE_PULSE]
         + handler->pulseChannelCount * TB2820_PULSE_CHANNEL_SIZE_BYTES;
    offsets[TB2820_MODE_PHASE2_STEP8] = offsets[TB2820_MODE_PHASE2_STEP4]
         + (handler->stepper24ChannelCount / 4) * TB2820_STEPPER2_GROUP_SIZE_BYTES;
    // stepper2GroupCount = handler->stepper2ChannelCount / 4
    offsets[TB2820_MODE_PHASE4_SINGLE_STEP4] = offsets[TB2820_MODE_PHASE2_STEP8]
         + (handler->stepper28ChannelCount / 4) * TB2820_STEPPER2_GROUP_SIZE_BYTES;
    offsets[TB2820_MODE_PHASE4_DOUBLE_STEP4] = offsets[TB2820_MODE_PHASE4_SINGLE_STEP4]
         + (handler->stepper44ChannelCount / 4) * TB2820_STEPPER4_GROUP_SIZE_BYTES;
    offsets[TB2820_MODE_PHASE4_STEP8] = offsets[TB2820_MODE_PHASE4_DOUBLE_STEP4]
         + (handler->stepper444ChannelCount / 4) * TB2820_STEPPER4_GROUP_SIZE_BYTES;
    int i = 0;
    for(; i < TB2820_CHANNEL_MAX; ++i)
    {
        size_t *currentOffset = &offsets[handler->info[i].mode];
        TB2820_DemuxDMAData(handler, buffer, currentOffset, bufSize, i);
    }
    return TB_RETURN_OK;
}

TB_Return TB2820_GetDIValue(TB2820_Handler *handler, TB_U32 channel, TB_U32 *data)
{
    if(!handler || !data)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if (channel < 32)
    {
        *data = (handler->diValues[0] >> channel)& 0x1;
    }
    else if(channel >= 32 && channel < TB2820_CHANNEL_MAX)
    {
        *data = (handler->diValues[1] >> (channel - 32))& 0x1;
    }
    else
    {
        return TB_ERR_INVALID_CHANNEL;
    }
    TB_DBG("TB2820_GetDIValue data = %u\n", *data);
    return TB_RETURN_OK;
}

TB_Return TB2820_GetPWMFreqAndDuty(TB2820_Handler *handler, TB_U32 channel, TB_Float* freq, TB_Float* duty)
{
    if(!handler)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if(!freq || !duty)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }

    if(channel >= TB2820_CHANNEL_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    if(handler->info[channel].mode != TB2820_MODE_PWM)
    {
        return TB2820_ERR_INVALID_CHANNEL_MODE;
    }

    *freq = handler->info[channel].data.modeData.pwm.freq;
    *duty = handler->info[channel].data.modeData.pwm.duty;
    TB_DBG("TB2820_GetPWMFreqAndDuty frequency = %f, duty = %f\n", *freq, *duty);
    return TB_RETURN_OK;
}

TB_Return TB2820_GetHallSignalCount(TB2820_Handler *handler, TB_U32 channel, TB_U32 *signalCount)
{
    if(!handler || !signalCount)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if(channel >= TB2820_HALL_CHANNEL_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    TB2820_ChannelMode mode = handler->info[channel].mode;
    if(mode != TB2820_MODE_HALL)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }
    *signalCount = handler->info[channel].data.modeData.singleHall.totalCount;
    TB_DBG("TB2820_GetHallSignalCount channel = %u, data = %f\n",channel, *signalCount);
    return TB_RETURN_OK;
}

TB_Return TB2820_GetHallRisingEdgeCount(TB2820_Handler *handler, TB_U32 channel, TB_U32 *risingEdgeCount)
{
    if(!handler || !risingEdgeCount)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if(channel >= TB2820_HALL_CHANNEL_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    if(handler->info[channel].mode != TB2820_MODE_HALL)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }

    *risingEdgeCount = handler->info[channel].data.modeData.singleHall.risingEdgeCount;
    TB_DBG("TB2820_GetHallRisingEdgeCount channel = %u, data = %f\n",channel, *risingEdgeCount);
    return TB_RETURN_OK;
}

TB_Return TB2820_GetHallFallingEdgeCount(TB2820_Handler *handler, TB_U32 channel, TB_U32 *fallingEdgeCount)
{
    if(!handler || !fallingEdgeCount)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if(channel >= TB2820_HALL_CHANNEL_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    if(handler->info[channel].mode != TB2820_MODE_HALL)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }

    *fallingEdgeCount = handler->info[channel].data.modeData.singleHall.fallingEdgeCount;
    TB_DBG("TB2820_GetHallFallingEdgeCount channel = %u, data = %f\n",channel, *fallingEdgeCount);
    return TB_RETURN_OK;
}

TB_Return TB2820_GetHall3MotorSpeed(TB2820_Handler *handler, TB_U32 group, TB_U32 *motorSpeed)
{
    if(!handler || !motorSpeed)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if(group >= 12)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    if(handler->info[group * 3].mode != TB2820_MODE_HALL3)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }

    *motorSpeed = handler->info[group * 3].data.modeData.threePhaseHall.acutalRPM;
    TB_DBG("TB2820_GetHall3MotorSpeed group = %u, data = %f\n", group, *motorSpeed);
    return TB_RETURN_OK;
}

TB_Return TB2820_GetHall3RealTimeAngle(TB2820_Handler *handler, TB_U32 group, TB_U32 *realTimeAngle)
{
    if(!handler || !realTimeAngle)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if(group >= TB2820_HALL3_GROUP_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    if(handler->info[group * 3].mode != TB2820_MODE_HALL3)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }

    *realTimeAngle = handler->info[group * 3].data.modeData.threePhaseHall.realTimeAngle;
    TB_DBG("TB2820_GetHall3RealTimeAngle group = %u, data = %f\n", group, *realTimeAngle);
    return TB_RETURN_OK;
}

TB_Return TB2820_GetHallCommonSignalStatus(TB2820_Handler *handler, TB_U32 channel, TB_U32 *signalStatus)
{
    if(!handler || !signalStatus)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if (channel >= TB2820_HALL_CHANNEL_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    if(handler->info[channel].mode != TB2820_MODE_HALL
        && handler->info[channel].mode != TB2820_MODE_HALL3)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }

    const TB_U32 index = channel >> 5;  /// Equivalent to channel /32
    const TB_U32 bit_pos = channel & 0x1F;  /// Equivalent to channel %32
    
    *signalStatus = handler->singleHallStatus[index] & (1U << bit_pos);
    TB_DBG("TB2820_GetHallSignalStatus channel = %u, direction = %f\n", channel, *signalStatus);

    return TB_RETURN_OK;
}

TB_Return TB2820_GetHall3Direction(TB2820_Handler *handler, TB_U32 group, TB_U32 *direction)
{
    if(!handler || !direction)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if (group >= TB2820_HALL3_GROUP_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    if(handler->info[group * 3].mode != TB2820_MODE_HALL3)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }

    *direction = (handler->threePhaseHallDirection >> group) & 1U;
    TB_DBG("TB2820_GetHall3Direction group = %u, direction = %f\n", group, *direction);
    
    return TB_RETURN_OK;
}

TB_Return TB2820_GetHall3SignalStatus(TB2820_Handler *handler, TB_U32 channel, TB_U32 *signalStatus)
{
    if(!handler || !signalStatus)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if (channel >= TB2820_HALL_CHANNEL_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    if(handler->info[channel].mode != TB2820_MODE_HALL3)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }

    const TB_U32 index = channel >> 5;  /// Equivalent to channel /32
    const TB_U32 bit_pos = channel & 0x1F;  /// Equivalent to channel %32
    
    *signalStatus = handler->singleHallStatus[index] & (1U << bit_pos);
    TB_DBG("TB2820_GetHall3SignalStatus channel = %u, direction = %f\n", channel, *signalStatus);

    return TB_RETURN_OK;
}

TB_Return TB2820_GetHall3RisingEdgeCount(TB2820_Handler *handler, TB_U32 channel, TB_U32 *risingEdgeCount)
{
    if(!handler || !risingEdgeCount)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if(channel >= TB2820_HALL_CHANNEL_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    if(handler->info[channel].mode != TB2820_MODE_HALL3)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }

    *risingEdgeCount = handler->info[channel].data.modeData.threePhaseHall.risingEdgeCount[channel % 3];
    TB_DBG("TB2820_GetHall3RisingEdgeCount channel = %u, data = %f\n",channel, *risingEdgeCount);
    return TB_RETURN_OK;
}

TB_Return TB2820_GetHall3FallingEdgeCount(TB2820_Handler *handler, TB_U32 channel, TB_U32 *fallingEdgeCount)
{
    if(!handler || !fallingEdgeCount)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if(channel >= TB2820_HALL_CHANNEL_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    if(handler->info[channel].mode != TB2820_MODE_HALL3)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }

    *fallingEdgeCount = handler->info[channel].data.modeData.threePhaseHall.fallingEdgeCount[channel % 3];
    TB_DBG("TB2820_GetHall3FallingEdgeCount channel = %u, data = %f\n",channel, *fallingEdgeCount);
    return TB_RETURN_OK;
}

TB_Return TB2820_GetPluseSignalCount(TB2820_Handler *handler, TB_U32 channel, TB_U32 *signalCount)
{
    if(!handler || !signalCount)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if(channel >= TB2820_PULSE_CHANNEL_MAX || channel < TB2820_PULSE_CHANNEL_MIN)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    if(handler->info[channel].mode != TB2820_MODE_PULSE)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }

    *signalCount = handler->info[channel].data.modeData.singlePulse.totalCount;
    TB_DBG("TB2820_GetPluseSignalCount channel = %u, data = %f\n", channel, *signalCount);
    
    return TB_RETURN_OK;
}

TB_Return TB2820_GetPulseRisingEdgeCount(TB2820_Handler *handler, TB_U32 channel, TB_U32 *risingEdgeCount)
{
    if(!handler || !risingEdgeCount)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if(channel >= TB2820_PULSE_CHANNEL_MAX || channel < TB2820_PULSE_CHANNEL_MIN)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    if(handler->info[channel].mode != TB2820_MODE_PULSE)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }

    *risingEdgeCount = handler->info[channel].data.modeData.singlePulse.risingEdgeCount;
    TB_DBG("TB2820_GetPulseRisingEdgeCount channel = %u, data = %f\n", channel, *risingEdgeCount);
    
    return TB_RETURN_OK;
}

TB_Return TB2820_GetPulseFallingEdgeCount(TB2820_Handler *handler, TB_U32 channel, TB_U32 *fallingEdgeCount)
{
    if(!handler || !fallingEdgeCount)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if(channel >= TB2820_PULSE_CHANNEL_MAX || channel < TB2820_PULSE_CHANNEL_MIN)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    if(handler->info[channel].mode != TB2820_MODE_PULSE)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }

    *fallingEdgeCount = handler->info[channel].data.modeData.singlePulse.fallingEdgeCount;
    TB_DBG("TB2820_GetPulseFallingEdgeCount channel = %u, data = %f\n", channel, *fallingEdgeCount);
    
    return TB_RETURN_OK;
}

TB_Return TB2820_GetPulseSignalStatus(TB2820_Handler *handler, TB_U32 channel, TB_U32 *signalStatus)
{
    if(!handler || !signalStatus)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if(channel >= TB2820_PULSE_CHANNEL_MAX || channel < TB2820_PULSE_CHANNEL_MIN)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    if(handler->info[channel].mode != TB2820_MODE_PULSE)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }
    
    *signalStatus = (handler->singlePulseStatus >> (channel - TB2820_PULSE_CHANNEL_MIN)) & 1U;
    TB_DBG("TB2820_GetPulseSignalStatus channel = %u, signalStatus = %u\n", channel, *signalStatus);
    
    return TB_RETURN_OK;
}

TB_Return TB2820_GetStepper2Steps(TB2820_Handler *handler, TB_U32 group, TB_U32 *step)
{
    if(!handler || !step)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if(group >= TB2820_MOTOR_GROUP_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }
    TB2820_ChannelMode mode = handler->info[group * 4 + TB2820_PULSE_CHANNEL_MIN].mode;
    if(mode != TB2820_MODE_PHASE2_STEP4 && mode != TB2820_MODE_PHASE2_STEP8)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }
    
    *step = handler->info[group * 4 + TB2820_PULSE_CHANNEL_MIN].data.modeData.motor2Phase.stepCount;
    TB_DBG("TB2820_GetStepper2Steps group = %u, step = %u\n", group, *step);
    
    return TB_RETURN_OK;
}

TB_Return TB2820_GetStepper2Phase(TB2820_Handler *handler, TB_U32 group, TB_U32 *phase)
{
    if(!handler || !phase)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if(group >= TB2820_MOTOR_GROUP_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }
    TB2820_ChannelMode mode = handler->info[group * 4 + TB2820_PULSE_CHANNEL_MIN].mode;
    if(mode != TB2820_MODE_PHASE2_STEP4 && mode != TB2820_MODE_PHASE2_STEP8)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }
    
    *phase = handler->info[group * 4 + TB2820_PULSE_CHANNEL_MIN].data.modeData.motor2Phase.pulseCount
    TB_DBG("TB2820_GetStepper2Phase group = %u, phase = %u\n", group, *phase);
    
    return TB_RETURN_OK;
}

TB_Return TB2820_GetStep2SignalStatus(TB2820_Handler *handler, TB_U32 group, TB_U32 *signalStatus)
{
    if(!handler || !signalStatus)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if(group >= TB2820_MOTOR_GROUP_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    TB2820_ChannelMode mode = handler->info[group * 4 + TB2820_PULSE_CHANNEL_MIN].mode;
    if(mode != TB2820_MODE_PHASE2_STEP4 && mode != TB2820_MODE_PHASE2_STEP8)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }
    
    *signalStatus = (handler->twoPhaseStepperStatus >> (group * 4)) & 0xf;
    TB_DBG("TB2820_GetStep2SignalStatus group = %u, signalStatus = %u\n", group, *signalStatus);
    
    return TB_RETURN_OK;
}

TB_Return TB2820_GetStepper4Steps(TB2820_Handler *handler, TB_U32 group, TB_U32 *step)
{
    if(!handler || !step)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if(group >= TB2820_MOTOR_GROUP_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }
    TB2820_ChannelMode mode = handler->info[group * 4 + TB2820_PULSE_CHANNEL_MIN].mode;
    if(mode != TB2820_MODE_PHASE4_SINGLE_STEP4 
        && mode != TB2820_MODE_PHASE4_DOUBLE_STEP4 
        && mode != TB2820_MODE_PHASE4_STEP8)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }
    
    *step = handler->info[group * 4 + TB2820_PULSE_CHANNEL_MIN].data.modeData.motor4Phase.stepCount;
    TB_DBG("TB2820_GetStepper4Steps group = %u, step = %u\n", group, *step);
    
    return TB_RETURN_OK;
}

TB_Return TB2820_GetStepper4Phase(TB2820_Handler *handler, TB_U32 group, TB_U32 *phase)
{
    if(!handler || !phase)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if(group >= TB2820_MOTOR_GROUP_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }
    TB2820_ChannelMode mode = handler->info[group * 4 + TB2820_PULSE_CHANNEL_MIN].mode;
    if(mode != TB2820_MODE_PHASE4_SINGLE_STEP4 
        && mode != TB2820_MODE_PHASE4_DOUBLE_STEP4 
        && mode != TB2820_MODE_PHASE4_STEP8)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }
    
    *phase = handler->info[group * 4 + TB2820_PULSE_CHANNEL_MIN].data.modeData.motor4Phase.pulseCount;
    TB_DBG("TB2820_GetStepper4Phase group = %u, step = %u\n", group, *phase);
    
    return TB_RETURN_OK;
}

TB_Return TB2820_GetStep4SignalStatus(TB2820_Handler *handler, TB_U32 group, TB_U32 *signalStatus)
{
    if(!handler || !signalStatus)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if(group >= TB2820_MOTOR_GROUP_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }
    TB2820_ChannelMode mode = handler->info[group * 4 + TB2820_PULSE_CHANNEL_MIN].mode;
    if(mode != TB2820_MODE_PHASE4_SINGLE_STEP4 
        && mode != TB2820_MODE_PHASE4_DOUBLE_STEP4 
        && mode != TB2820_MODE_PHASE4_STEP8)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }
    
    *signalStatus = (handler->fourPhaseStepperStatus >> (group * 4)) & 0xf;
    TB_DBG("TB2820_GetStep4SignalStatus group = %u, signalStatus = %u\n", group, *signalStatus);
    
    return TB_RETURN_OK;
}

TB_Return TB2820_GetActualTickPeriodTime(TB2820_Handler *handler, TB_U32 channel, TB_Float *data)
{
    if(!handler || !data)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if(channel < 36 || channel >= TB2820_CHANNEL_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    if(handler->info[channel].mode != TB2820_MODE_SENTIN)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }
    const TB_U32 synTickReg = TB_REG_READ(handler->bar0Address, TB2820_CHANNEL_SYN_HIGH_TICK_OFFSET);
    const TB_U32 shift = (channel - 36) * 8;
    const TB_U32 synTick = (synTickReg >> shift) & 0xFF;
    if(synTick == 0)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }
    *data = handler->info[channel].data.modeData.sentIn.actualTickPeriodTime / 100.0f / synTick;
    TB_DBG("TB2820_GetActualTickPeriodTime channel = %u, data = %f\n",channel, *data);

	return TB_RETURN_OK;
}

TB_Return TB2820_GetAllTicks(TB2820_Handler *handler, TB_U32 channel, TB_U32 *data)
{
    if(!handler || !data)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if(channel < 36 || channel >= TB2820_CHANNEL_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    if(handler->info[channel].mode != TB2820_MODE_SENTIN)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }

    // Safely read and extract synTick
    const TB_U32 synReg = TB_REG_READ(handler->bar0Address, TB2820_CHANNEL_SYN_HIGH_TICK_OFFSET);
    const TB_U32 shift = (channel - 36) * 8;
    const TB_U32 synTick = (synReg >> shift) & 0xFF;

    // Check for possible division by zero
    const TB_U32 actualTick = handler->info[channel].data.modeData.sentIn.actualTickPeriodTime;
    if (actualTick == 0 || synTick == 0)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }

    // Calculate using intermediate variable for clarity
    const TB_U32 intermediate = handler->info[channel].data.modeData.sentIn.allTicks / actualTick;
    *data = intermediate / synTick;
    TB_DBG("TB2820_GetAllTicks channel = %u, data = %u\n",channel, *data);

	return TB_RETURN_OK;
}

TB_Return TB2820_GetNibbleData(TB2820_Handler *handler, TB_U32 channel, TB_U32 *data)
{
    if(!handler || !data)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if(channel < 36 || channel >= TB2820_CHANNEL_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    if(handler->info[channel].mode != TB2820_MODE_SENTIN)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }

    *data = handler->info[channel].data.modeData.sentIn.nibbleData;
    TB_DBG("TB2820_GetNibbleData channel = %u, data = %u\n",channel, *data);

	return TB_RETURN_OK;
}

TB_Return TB2820_GetDiagnosticStatus(TB2820_Handler *handler, TB_U32 channel, TB_U32 *data)
{
    if(!handler || !data)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if(channel < 36 || channel >= TB2820_CHANNEL_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    if(handler->info[channel].mode != TB2820_MODE_SENTIN)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }

    *data = handler->info[channel].data.modeData.sentIn.diagnostic;
    TB_DBG("TB2820_GetDiagnosticStatus channel = %u, data = %u\n",channel, *data);

	return TB_RETURN_OK;
}

TB_Return TB2820_GetSerialData(TB2820_Handler *handler, TB_U32 channel, TB_U32 *data)
{
    if(!handler || !data)
    {
        return TB_ERR_INVALID_HANDLER;
    }

    if(channel < 36 || channel >= TB2820_CHANNEL_MAX)
    {
        return TB_ERR_INVALID_CHANNEL;
    }

    if(handler->info[channel].mode != TB2820_MODE_SENTIN)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }

    *data = handler->info[channel].data.modeData.sentIn.serialData;
    TB_DBG("TB2820_GetSerialData channel = %u, data = %u\n",channel, *data)

	return TB_RETURN_OK;
}

void TB2820_PrintVersion()
{
	fprintf(stderr, "libtb2820 version %s\n", TB2820_VERSION);
	fprintf(stderr, "libtb2820 support firmware version 07\n");
}

TB_Return TB2820_GetHardwareVersion(TB2820_Handler *handler, TB_Version *hVersion)
{
	if (handler == NULL || hVersion == NULL)
	{
		TB_ERR("TB820_GetHardwareVersion handler or hVersion is NULL\n");
		return TB_ERR_INVALID_HANDLER;
	}

	unsigned int temp = 0;
#ifndef TB_DEVICE_INTERFACE_VIRTUAL
	temp = TB_REG_READ(handler->bar0Address, TB2820_BOARDINFO_HARDWARE_VERSION_OFFSET) & 0xFFFF;
#endif
	hVersion->major = (temp >> 8) & 0xFF;
	hVersion->minor = temp & 0xFF;
	
	TB_DBG("TB2820_GetHardwareVersion version = %d.%d\n", hVersion->major, hVersion->minor);
	return TB_RETURN_OK;
}

TB_Return TB2820_GetFirmwareVersion(TB2820_Handler *handler, TB_Version *fVersion)
{
	if (handler == NULL || fVersion == NULL)
	{
		TB_ERR("TB2820_GetFirmwareVersion handler or fVersion is NULL\n");
		return TB_ERR_INVALID_HANDLER;
	}

	unsigned int temp = 0;
#ifndef TB_DEVICE_INTERFACE_VIRTUAL
	temp = TB_REG_READ(handler->bar0Address, TB2820_BOARDINFO_FIRMWARE_VERSION_OFFSET);
#endif
	fVersion->major = ((temp & 0x00FF0000) >> 16);
	fVersion->minor = ((temp & 0x0000FF00) >> 8);
	fVersion->revision = (temp & 0x000000FF);
	
	TB_DBG("TB2820_GetFirmwareVersion version = %d.%d.%d\n", fVersion->major, fVersion->minor, fVersion->revision);

	return TB_RETURN_OK;
}

TB_Return TB2820_GetFlashBootInfo(TB2820_Handler *handler, TB_U32 *flashBootFileVersion, TB_U32 *flashBootStatus)
{
	TB_DBG("TB2710_GetFlashBootInfo.\n");
	if (handler == NULL || flashBootFileVersion == NULL)
	{
		TB_ERR("TB2710_GetFlashBootInfo handler or data is NULL\n");
		return TB_ERR_INVALID_HANDLER;
	}

	unsigned int temp = 0;
#ifndef TB_DEVICE_INTERFACE_VIRTUAL
	temp = TB_REG_READ(handler->bar0Address, TB2820_BOARDINFO_FLASH_BOOT_REGISTER_OFFSET);
#endif
	*flashBootFileVersion = ((temp & 0xFFFF00) >> 8);

	unsigned int flashStatus = temp & 0x01;
	if (flashStatus == TB2820_UPDATING_BOOTLOADER || flashStatus == TB2820_UPDATING_APPLICATION) 
	{
		*flashBootStatus = flashStatus;
	}
	
	TB_DBG("TB2820_GetFlashBootInfo flashBootFileVersion = %u, flashBootStatus = %u\n",
			*flashBootFileVersion, *flashBootStatus);

	return TB_RETURN_OK;
}

TB_Return TB2820_GetDeviceID(TB2820_Handler *handler, unsigned int *deviceID)
{
	if (handler == NULL || deviceID == NULL)
	{
		TB_ERR("TB2820_GetDeviceID handler or data is NULL\n");
		return TB_ERR_INVALID_HANDLER;
	}
	unsigned int temp = 0;
#ifndef TB_DEVICE_INTERFACE_VIRTUAL
	temp = TB_REG_READ(handler->bar0Address, TB2820_BOARDINFO_DEVICE_ID_REGISTER_OFFSET) & 0xFFFF;
#endif
	*deviceID = temp;
	
	TB_DBG("TB2820_GetDeviceID data = %u\n", *deviceID);
	return TB_RETURN_OK;
}

TB_Return TB2820_SetSoftReset(TB2820_Handler *handler, unsigned int softReset)
{
	TB_DBG("TB2710_SetsoftReset data = %u\n", softReset);
	if (handler == NULL)
	{
		TB_ERR("TB2710_SetsoftReset handler is NULL\n");
		return TB_ERR_INVALID_HANDLER;
	}

	unsigned int temp = (softReset & 0x01);
	if (temp == TB2820_RESET || temp == TB2820_NO_RESET)
	{
#ifndef TB_DEVICE_INTERFACE_VIRTUAL
		TB_REG_WRITE(handler->bar0Address, TB2820_BOARDINFO_SOFT_RESET_REGISTER_OFFSET, temp);
#endif
	}

	return TB_RETURN_OK;
}

TB_Return TB2820_GetLEDStatus(TB2820_Handler *handler, unsigned int *LEDStatus)
{
	if (handler == NULL || LEDStatus == NULL)
	{
		TB_ERR("TB2820_GetLEDStatus handler or data is NULL\n");
		return TB_ERR_INVALID_HANDLER;
	}
	
	unsigned int temp = 0;
#ifndef TB_DEVICE_INTERFACE_VIRTUAL
	temp = TB_REG_READ(handler->bar0Address, TB2820_BOARDINFO_LED_CONTROL_REGISTER_OFFSET) & 0x01;
#endif
	*LEDStatus = temp;
	
	TB_DBG("TB2820_GetLEDStatus data = %u\n", *LEDStatus);
	return TB_RETURN_OK;
}

TB_Return TB2820_SetLEDControl(TB2820_Handler *handler, unsigned int LEDControl)
{
	TB_DBG("TB2820_SetLEDControl data = %u\n", LEDControl);
	if (handler == NULL)
	{
		TB_ERR("TB2820_SetLEDControl handler is NULL\n");
		return TB_ERR_INVALID_HANDLER;
	}
	
	unsigned int temp = (LEDControl & 0x01);
#ifndef TB_DEVICE_INTERFACE_VIRTUAL
	TB_REG_WRITE(handler->bar0Address, TB2820_BOARDINFO_LED_CONTROL_REGISTER_OFFSET, temp);
#endif
	
	return TB_RETURN_OK;
}

TB_Float ConvertRawToVoltage(TB_U32 raw)
{
    return raw * 3 /65536.0;
}

TB_Float ConvertRawToTemperature(TB_U32 raw)
{
    return raw *503.975 / 65536.0 - 273.15;
}

TB_Return TB2820_GetFPGACoreVoltage(TB2820_Handler *handler, TB_Float *coreVoltage)
{
    if(handler == NULL)
    {
        return TB_ERR_INVALID_HANDLER;
    }
    unsigned int regVal = 0;
#ifndef TB_DEVICE_INTERFACE_VIRTUAL
    regVal = TB_REG_READ(handler->bar0Address, TB2820_BOARDINFO_FPGA_STATUS_REGISTER_OFFSET);
#endif
    *coreVoltage = ConvertRawToVoltage(regVal >> 16);
    TB_DBG("TB2820_GetCoreVoltage corevaltage = %d\n", *coreVoltage);
    return TB_RETURN_OK;
}

TB_Return TB2820_GetFPGATemperature(TB2820_Handler *handler, TB_Float *temperature)
{
    if(handler == NULL)
    {
        return TB_ERR_INVALID_HANDLER;
    }
    unsigned int regVal = 0;
#ifndef TB_DEVICE_INTERFACE_VIRTUAL
    regVal = TB_REG_READ(handler->bar0Address, TB2820_BOARDINFO_FPGA_STATUS_REGISTER_OFFSET);
#endif
    *temperature = ConvertRawToTemperature(regVal & 0xffff);
    TB_DBG("TB2820_GetFPGATemputure temperature = %d\n", *temperature);
    return TB_RETURN_OK;
}

TB_Return TB2820_GetFPGABRAMVoltage(TB2820_Handler *handler, TB_Float *bramVoltage)
{
    if(handler == NULL)
    {
        return TB_ERR_INVALID_HANDLER;
    }
    unsigned int regVal = 0;
#ifndef TB_DEVICE_INTERFACE_VIRTUAL
    regVal = TB_REG_READ(handler->bar0Address, TB2820_BOARDINFO_FPGA_STATUS_REGISTER_OFFSET + 4);
#endif
    *bramVoltage = ConvertRawToVoltage(regVal >> 16);
    TB_DBG("TB2820_GetFPGABRAMVoltage temperature = %d\n", *temperature);
    return TB_RETURN_OK;
}

TB_Return TB2820_GetFPGAAuxVoltage(TB2820_Handler *handler, TB_Float *auxVoltage)
{
    if(handler == NULL)
    {
        return TB_ERR_INVALID_HANDLER;
    }
    unsigned int regVal = 0;
#ifndef TB_DEVICE_INTERFACE_VIRTUAL
    regVal = TB_REG_READ(handler->bar0Address, TB2820_BOARDINFO_FPGA_STATUS_REGISTER_OFFSET + 4);
#endif
    *auxVoltage = ConvertRawToVoltage(regVal & 0xffff);
    TB_DBG("TB2820_GetFPGAAuxVoltage auxVoltage = %d\n", *auxVoltage);
    return TB_RETURN_OK;
}

TB_Return TB2820_GetBoardPCBASerial(TB2820_Handler *handler, TB_U8 *serialBuf, TB_U32 size)
{
    if(handler == NULL)
    {
        return TB_ERR_INVALID_HANDLER;
    }
    if(serialBuf == NULL)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }
    if(size < TB2820_PCBA_SN_LENGTH)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }
    unsigned int part1 = 0, part2 = 0, part3 = 0;
#ifndef TB_DEVICE_INTERFACE_VIRTUAL
    part1 = TB_REG_READ(handler->bar0Address, TB2820_BOARDINFO_PCBA_SN_REGISTER_OFFSET);
    part2 = TB_REG_READ(handler->bar0Address, TB2820_BOARDINFO_PCBA_SN_REGISTER_OFFSET + 4);
    part3 = TB_REG_READ(handler->bar0Address, TB2820_BOARDINFO_PCBA_SN_REGISTER_OFFSET + 8);
#endif
    memcpy(serialBuf, &part1, sizeof(unsigned int));
    memcpy(serialBuf + sizeof(unsigned int), &part2, sizeof(unsigned int));
    memcpy(serialBuf + 2 * sizeof(unsigned int), &part3, sizeof(unsigned int));
    return TB_RETURN_OK;
}

TB_Return TB2820_GetCalibrationInfo(TB2820_Handler *handler, TB2820CalibrationStatus *status, TB_U32 *isCoefficientLoaded, TB_U32 *isCalibrationRequired)
{
    if(handler == NULL)
    {
        return TB_ERR_INVALID_HANDLER;
    }
    if(status == NULL)
    {
        return TB_ERR_INVALID_ARGUMENTS;
    }
    TB_U32 raw = 0;
#ifndef TB_DEVICE_INTERFACE_VIRTUAL
    raw = TB_REG_READ(handler->bar0Address, TB2820_BOARDINFO_CALIBRATION_REGISTER_OFFSET);
#endif
    *status = (raw >> 8) & 0xff;
    *isCoefficientLoaded = (raw >> 4) & 1u;
    *isCalibrationRequired = raw & 1u;
    TB_DBG("TB2820_GetCalibrationInfo CalibrationStatus = %d, isCoefficientLoaded = %d, isCalibrationRequired = %d\n", *status, *isCoefficientLoaded, *isCalibrationRequired);
    return TB_RETURN_OK;
}

