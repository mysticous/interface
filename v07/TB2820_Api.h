#ifndef _TB2820_Api_V07_H
#define _TB2820_Api_V07_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "TESTBASE.h"
#define TB2820_DEVICE_NAME_SIZE 100
#define TB2820_CHANNEL_MAX 60
#define TB2820_HALL_CHANNEL_MAX 36
#define TB2820_HALL3_GROUP_MAX 12
#define TB2820_PULSE_CHANNEL_MIN 36
#define TB2820_PULSE_CHANNEL_MAX 60
#define TB2820_MOTOR_GROUP_MAX 6

#define TB2820_MODE_MAX 13
#define TB2820_CLOCK_FREQ 100000000

typedef enum {
    TB2820_ERR_READ_DMA_FAILED = 9,
    TB2820_ERR_INVALID_CHANNEL_MODE = 10

}TB2820_ErrCode;

typedef enum {
    TB2820_MODE_DI                      = 0x0, /**< 4'b0000 */ 
    TB2820_MODE_PWM                     = 0x1,
    TB2820_MODE_SENTIN                  = 0x2,
    TB2820_MODE_HALL                    = 0x4,
    TB2820_MODE_HALL3                   = 0x5,
    TB2820_MODE_PULSE                   = 0x8,
    TB2820_MODE_PHASE2_STEP4            = 0x9,
    TB2820_MODE_PHASE2_STEP8            = 0xa,
    TB2820_MODE_PHASE4_SINGLE_STEP4     = 0xb,
    TB2820_MODE_PHASE4_DOUBLE_STEP4     = 0xc,
    TB2820_MODE_PHASE4_STEP8            = 0xd
} TB2820_ChannelMode;

typedef enum {
    TB2820_SENTIN_LEGACY = 0,
    TB2820_SENTIN_RECOMMANDED
}TB2820_Sentin_CRCtype;

typedef enum {
    TB2820_SENTIN_POLARITY_REVERSAL,
    TB2820_SENTIN_POLARITY_NONREVERSAL
}TB2820_Sentin_Polarity;

typedef enum {
    TB2820_DISABLE,
    TB2820_ENABLE
}TB2820_Enable;

typedef enum {
    TB2820_UPDATING_BOOTLOADER,
    TB2820_UPDATING_APPLICATION
}TB2820_FIRMWARE_UPDATE_STATUS;

typedef enum {
    TB2820_NO_RESET,
    TB2820_RESET
}TB2820_SOFT_RESET;

typedef enum {
    TB2820_CAL_STATUS_ERROR,
    TB2820_CAL_STATUS_DEFAULT,
    TB2820_CAL_STATUS_CALIBRATED,
    TB2820_CAL_STATUS_UNKNOW
}TB2820CalibrationStatus;

// typedef struct {
//     int serialReceiveEnable;
//     int receiveEnable;
//     int clearFault;
//     TB_U32 actualTickPecoidTime;
//     TB_U32 allTicks;
//     TB_U32 diagnostic;
//     TB_U32 nibbleData;
//     TB_U32 serialData;
//     int crcErrorFlag;
//     int systemErrorFlag;
//     int lowTickErrorFlag;
//     int nibbleValueOutOfRange;
//     int timeoutFlag;
//     int serialCRCErrorFlag;
//     int serialFormatErrorFlag;
//     int qMessage[16];
//     int sMessage[4];
//     int serialReserved;
// }SentIn;

typedef struct {
    TB_U32 actualTickPeriodTime;
    TB_U32 allTicks;
    TB_U32 diagnostic;
    TB_U32 nibbleData;
    TB_U32 serialData;
}SentIn;

typedef struct {
    TB_U32 totalCount;
    TB_U32 risingEdgeCount;
    TB_U32 fallingEdgeCount;
}SingleSignal;

typedef struct {
    TB_U32 acutalRPM;
    TB_U32 realTimeAngle;
    TB_U32 risingEdgeCount[3];
    TB_U32 fallingEdgeCount[3];
}HallThreePhaseSignals;

typedef struct {
    TB_U32 stepCount;
    TB_U32 pulseCount;
}StepperMotor;

typedef struct {
    union {
        struct { TB_U32 value; } di;
        struct { TB_Float duty; TB_Float freq; } pwm;
        SentIn sentIn;
        SingleSignal singleHall;
        HallThreePhaseSignals threePhaseHall;
        SingleSignal singlePulse;
        StepperMotor motor2Phase;
        StepperMotor motor4Phase;
    } modeData;
} TB2820_ChannelData;

#define TB2820_DMABUFFER_MAXSIZE TB2820_CHANNEL_MAX * sizeof(TB2820_ChannelData) + 32

typedef struct {
    TB2820_ChannelMode mode;
    TB2820_ChannelData data;
}ChannelInfo;

struct tbboardcfg
{
	int fd;
	char deviceName[TB2820_DEVICE_NAME_SIZE];
	TB_U32* bar0Address;
    int dmaFd;
    ChannelInfo info[TB2820_CHANNEL_MAX];

    /**
     * @brief Digital input values for two channel ranges
     * 
     * [0]: DI CH1~CH32 values (bit0-CH1 ... bit31-CH32)
     * [1]: DI CH33~CH60 values (bit0-CH33 ... bit27-CH60)
     */
    TB_U32 diValues[2];

    /**
     * @brief Single Hall sensor status for different channel ranges
     * 
     * [0]: CH1-CH32 status (bit0-CH1 ... bit31-CH32, 0=low/1=high)
     * [1]: CH33-CH40 status (bit0-CH33 ... bit7-CH40, 0=low/1=high)
     */
    TB_U32 singleHallStatus[2];

    /**
     * @brief Three-phase Hall sensor rotation direction status
     * 
     * Bit0-11: Group1-12 direction (0=forward/1=reverse)
     */
    TB_U32 threePhaseHallDirection;

    /**
     * @brief Single-phase pulse signal status
     * 
     * Bit0-15: CH1-CH16 status (0=low/1=high)
     */
    TB_U32 singlePulseStatus;

    /**
     * @brief Two-phase stepper motor signals
     * 
     * 4 bits per group:
     * - Group1: bit0-3 (A+,A-,B+,B-)
     * - Group2: bit4-7
     * - Group3: bit8-11
     * - Group4: bit12-15
     */
    TB_U32 twoPhaseStepperStatus;

    /**
     * @brief Four-phase stepper motor signals
     * 
     * 4 bits per group:
     * - Group1: bit0-3 (A,B,C,D)
     * - Group2: bit4-7
     * - Group3: bit8-11
     * - Group4: bit12-15
     */
    TB_U32 fourPhaseStepperStatus;

    TB_U32 pwmChannelCount;
    TB_U32 sentChannelCount;
    TB_U32 hallChannelCount;
    TB_U32 hall3ChannelCount;
    TB_U32 pulseChannelCount;
    TB_U32 stepper24ChannelCount;
    TB_U32 stepper28ChannelCount;
    TB_U32 stepper44ChannelCount;
    TB_U32 stepper444ChannelCount;
    TB_U32 stepper48ChannelCount;
};

typedef struct tbboardcfg TB2820_Handler;

TB_Return TB2820_Open(TB2820_Handler **handler, TB_U32 devnum);

void TB2820_Close(TB2820_Handler **handler);

/**********************once-config*************************/
TB_Return TB2820_SetCommonThreshold(TB2820_Handler *handler, TB_U32 channel, TB_Float threshold);//DI比较阈值设置

TB_Return TB2820_SetCommonTimeoutTime(TB2820_Handler *handler, TB_U32 channel, TB_U32 value);

TB_Return TB2820_SetChannelMode(TB2820_Handler *handler, TB_U32 channel, TB2820_ChannelMode mode);

TB_Return TB2820_AdjustAndSetAllChannelsMode(TB2820_Handler *handler);

/**
 * @brief Set polarity for Hall sensor channels
 * @param handler Device handler pointer
 * @param hallMask1_32 Bitmask for channels 1-32 (0: high active, 1: low active)
 * @param hallMask33_36 Bitmask for channels 33-36 (0: high active, 1: low active)
 * @return TB_Return Error code indicating success/failure
 * @note Bits beyond specified ranges will be ignored
 */
TB_Return TB2820_SetHallPolarity(TB2820_Handler *handler, TB_U32 hallMask1_32, TB_U32 hallMask33_36);

/**
 * @brief Set polarity for pulse measurement channels 1-16
 * @param handler Device handler pointer
 * @param pulseMask Bitmask for channels 1-16 (0: high active, 1: low active)
 * @return TB_Return Error code indicating success/failure
 * @note Bits beyond 16 will be ignored
 */
TB_Return TB2820_SetPulsePolarity(TB2820_Handler *handler, TB_U32 pulseMask);

/**
 * @brief Set initial steps for 2-phase stepper motor group
 * @param handler Device handler pointer
 * @param groupIndex Motor group index (0-based)
 * @param steps Initial step count (0-4294967295)
 * @return TB_Return Error code indicating success/failure
 */
TB_Return TB2820_SetStepper2InitSteps(TB2820_Handler *handler, TB_U32 group, TB_U32 steps);

/**
 * @brief Set initial steps for 4-phase stepper motor group
 * @param handler Device handler pointer
 * @param groupIndex Motor group index (0-based)
 * @param steps Initial step count (0-4294967295)
 * @return TB_Return Error code indicating success/failure
 */
TB_Return TB2820_SetStepper4InitSteps(TB2820_Handler *handler, TB_U32 group, TB_U32 steps);

/**
 * @brief Set debounce time for specified channel
 * @param handler Device handler pointer
 * @param channel Target channel index (1-36 for Hall, 1-16 for Pulse)
 * @param time Debounce time in units of 10ns (e.g., 100 = 1μs)
 * @return TB_Return Error code indicating success/failure
 */
TB_Return TB2820_SetDebounceTime(TB2820_Handler *handler, TB_U32 channel, TB_U32 time);

/**
 * @brief Enable/disable Hall counter clear function
 * @param handler Device handler pointer
 * @param clearMask1_32 Bitmask for channels 1-32 (1: enable clear)
 * @param clearMask33_36 Bitmask for channels 33-36 (1: enable clear)
 * @return TB_Return Error code indicating success/failure
 */
TB_Return TB2820_SetHallClearEnable(TB2820_Handler *handler, TB_U32 clearMask1_32, TB_U32 clearMask33_36);

/**
 * @brief Enable/disable pulse counter clear function
 * @param handler Device handler pointer
 * @param clearMask Bitmask for channels 1-16 (1: enable clear)
 * @return TB_Return Error code indicating success/failure
 */
TB_Return TB2820_SetPulseClearEnable(TB2820_Handler *handler, TB_U32 clearMask);

/**
 * @brief Enable/disable stepper motor counter clear
 * @param handler Device handler pointer
 * @param groupMask Bitmask for motor groups (1: enable clear)
 * @return TB_Return Error code indicating success/failure
 */
TB_Return TB2820_SetStepperClearEnable(TB2820_Handler *handler, TB_U32 groupMask);

TB_Return TB2820_SetHall3ClearEnable(TB2820_Handler *handler, TB_U32 groupMask);

TB_Return TB2820_SetHall3StopTimeout(TB2820_Handler *handler, TB_U32 group, TB_U32 time);

//SENTIN
// Tick period configuration (3us-90us) Tolerance configuration (5%-25%)
TB_Return TB2820_SetTickPeriodAndTolerance(TB2820_Handler *handler, TB_U32 channel, TB_Float periodUs, TB_Float tolerancePercent);

// Nibble high tick configuration (for data=0)
TB_Return TB2820_SetNibbleHighTick(TB2820_Handler *handler, TB_U32 channel, TB_U32 ticks);

// Low tick configuration
TB_Return TB2820_SetLowTick(TB2820_Handler *handler, TB_U32 channel, TB_U32 ticks);

// Sync pulse high tick configuration
TB_Return TB2820_SetSyncHighTick(TB2820_Handler *handler, TB_U32 channel, TB_U32 ticks);

// Pause enable configuration
TB_Return TB2820_SetPauseEnable(TB2820_Handler *handler, TB_U32 channel, TB_U32 enable);

// Nibbles per message configuration
TB_Return TB2820_SetNibblePerMessage(TB2820_Handler *handler, TB_U32 channel, TB_U32 nibbleCount);

// Timeout configuration (in ms)
TB_Return TB2820_SetSentInTimeout(TB2820_Handler *handler, TB_U32 channel, TB_Float timeoutMs);

// Polarity configuration
TB_Return TB2820_SetPolarity(TB2820_Handler *handler, TB_U32 channel, TB_U32 invert);

// CRC implementation type configuration
TB_Return TB2820_SetCRCImplementationType(TB2820_Handler *handler, TB_U32 channel, TB_U32 crcType);

// Serial data mode configuration (reserved)
TB_Return TB2820_SetSerDataMode(TB2820_Handler *handler, TB_U32 channel, TB_U32 mode);

TB_Return TB2820_SetRxEnable(TB2820_Handler *handler, TB_U32 channel, TB_U32 enable);

TB_Return TB2820_ClearFault(TB2820_Handler *handler, TB_U32 channel);

TB_Return TB2820_GetData(TB2820_Handler *handler);

size_t TB2820_GetDMABufferSize(TB2820_Handler *handler);

TB_Return TB2820_ReadDMA(TB2820_Handler *handler, TB_U8* buffer, size_t size);

TB_Return TB2820_GetDIValue(TB2820_Handler *handler, TB_U32 channel, TB_U32 *data);

TB_Return TB2820_GetPWMFreqAndDuty(TB2820_Handler *handler, TB_U32 channel, TB_Float* freq, TB_Float* duty);

//SENTIN
TB_Return TB2820_GetActualTickPeriodTime(TB2820_Handler *handler,TB_U32 channel, TB_Float *data);

TB_Return TB2820_GetNibbleData(TB2820_Handler *handler, TB_U32 channel, TB_U32 *data);

TB_Return TB2820_GetAllTicks(TB2820_Handler *handler, TB_U32 channel, TB_U32 *data);

TB_Return TB2820_GetDiagnosticStatus(TB2820_Handler *handler, TB_U32 channel, TB_U32 *data);

TB_Return TB2820_GetSerialData(TB2820_Handler *handler, TB_U32 channel, TB_U32 *data);

// hall
TB_Return TB2820_GetHallSignalCount(TB2820_Handler *handler, TB_U32 channel, TB_U32 *signalCount);

TB_Return TB2820_GetHallRisingEdgeCount(TB2820_Handler *handler, TB_U32 channel, TB_U32 *risingEdgeCount);

TB_Return TB2820_GetHallFallingEdgeCount(TB2820_Handler *handler, TB_U32 channel, TB_U32 *fallingEdgeCount);

TB_Return TB2820_GetHallCommonSignalStatus(TB2820_Handler *handler, TB_U32 channel, TB_U32 *signalStatus);

TB_Return TB2820_GetHall3MotorSpeed(TB2820_Handler *handler, TB_U32 group, TB_U32 *motorSpeed);

TB_Return TB2820_GetHall3RealTimeAngle(TB2820_Handler *handler, TB_U32 group, TB_U32 *realTimeAngle);

TB_Return TB2820_GetHall3Direction(TB2820_Handler *handler, TB_U32 group, TB_U32 *direction);

TB_Return TB2820_GetHall3SignalStatus(TB2820_Handler *handler, TB_U32 channel, TB_U32 *signalStatusMask);

TB_Return TB2820_GetHall3RisingEdgeCount(TB2820_Handler *handler, TB_U32 channel, TB_U32 *risingEdgeCount);

TB_Return TB2820_GetHall3FallingEdgeCount(TB2820_Handler *handler, TB_U32 channel, TB_U32 *fallingEdgeCount);

// pulse
TB_Return TB2820_GetPluseSignalCount(TB2820_Handler *handler, TB_U32 channel, TB_U32 *signalCount);

TB_Return TB2820_GetPulseRisingEdgeCount(TB2820_Handler *handler, TB_U32 channel, TB_U32 *risingEdgeCount);

TB_Return TB2820_GetPulseFallingEdgeCount(TB2820_Handler *handler, TB_U32 channel, TB_U32 *fallingEdgeCount);

TB_Return TB2820_GetPulseSignalStatus(TB2820_Handler *handler, TB_U32 channel, TB_U32 *signalStatus);

TB_Return TB2820_GetStepper2Steps(TB2820_Handler *handler, TB_U32 group, TB_U32 *step);

TB_Return TB2820_GetStepper2Phase(TB2820_Handler *handler, TB_U32 group, TB_U32 *phase);

TB_Return TB2820_GetStep2SignalStatus(TB2820_Handler *handler, TB_U32 group, TB_U32 *signalStatusMask);

TB_Return TB2820_GetStepper4Steps(TB2820_Handler *handler, TB_U32 group, TB_U32 *step);

TB_Return TB2820_GetStepper4Phase(TB2820_Handler *handler, TB_U32 group, TB_U32 *phase);

TB_Return TB2820_GetStep4SignalStatus(TB2820_Handler *handler, TB_U32 group, TB_U32 *signalStatusMask);

// Board Common Information Interface
TB_Return TB2820_GetFlashBootInfo(TB2820_Handler *handler, TB_U32 *flashBootFileVersion, TB_U32 *flashBootStatus);

TB_Return TB2820_GetDeviceID(TB2820_Handler *handler, unsigned int *deviceID);

TB_Return TB2820_SetSoftReset(TB2820_Handler *handler, unsigned int softReset);

TB_Return TB2820_GetLEDStatus(TB2820_Handler *handler, unsigned int *LEDStatus);

TB_Return TB2820_SetLEDControl(TB2820_Handler *handler, unsigned int LEDControl);

TB_Return TB2820_GetFPGACoreVoltage(TB2820_Handler *handler, TB_Float *coreVoltage);

TB_Return TB2820_GetFPGATemperature(TB2820_Handler *handler, TB_Float *temperature);

TB_Return TB2820_GetFPGABRAMVoltage(TB2820_Handler *handler, TB_Float *bramVoltage);

TB_Return TB2820_GetFPGAAuxVoltage(TB2820_Handler *handler, TB_Float *auxVoltage);

TB_Return TB2820_GetBoardPCBASerial(TB2820_Handler *handler, TB_U8 *serialBuf, TB_U32 size);

TB_Return TB2820_GetCalibrationInfo(TB2820_Handler *handler, TB2820CalibrationStatus *status, TB_U32 *isCoefficientLoaded, TB_U32 *isCalibrationRequired);

TB_Return TB2820_GetHardwareVersion(TB2820_Handler *handler, TB_Version *hVersion);

TB_Return TB2820_GetFirmwareVersion(TB2820_Handler *handler, TB_Version *fVersion);

void TB2820_PrintVersion();

#ifdef __cplusplus
}
#endif

#endif