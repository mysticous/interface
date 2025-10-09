#ifndef TB2820_H
#define TB2820_H

#include "IoTask/IoTask.h"
#include "TESTBASE/v07/TB2820_Api.h"

#define TB2820_VERSION_MAJOR_NUMBER 0U
#define TB2820_VERSION_MINOR_NUMBER 0U
#define TB2820_VERSION_REVISION_NUMBER 0U
#define TB2820_QUICKMSG_NIBBLEANDCRC_MAX 6U

class TB2820MappingData : public MappingData
{
public:
    int channelType = 0;
    int channelNumber = 0;
    int mappingCount = 0;
};

enum TB2820VarKey
{
    SIGNAL_COUNT,
    RISING_EDGE_COUNT,
    FALLING_EDGE_COUNT,
    SIGNAL_STATUS,
    PULSE_A_POS_PHASE_STATUS,
    PULSE_B_POS_PHASE_STATUS,
    PULSE_C_POS_PHASE_STATUS,
    PULSE_D_POS_PHASE_STATUS,
    HALL_A_PHASE_STATUS,
    HALL_B_PHASE_STATUS,
    HALL_C_PHASE_STATUS,
    HALL_A_SIGNAL_COUNT,
    HALL_B_SIGNAL_COUNT,
    HALL_C_SIGNAL_COUNT,
    HALL_A_RISING_COUNT,
    HALL_B_RISING_COUNT,
    HALL_C_RISING_COUNT,
    HALL_A_FALLING_COUNT,
    HALL_B_FALLING_COUNT,
    HALL_C_FALLING_COUNT,
    MOTOR_SPEED,
    ANGLE_POSITION,
    PULSE_COUNT,
    STEP_COUNT,
    RESET
};

class TB2820 final : public IoTask
{
public:
    TB2820(short num, std::shared_ptr<HardwareRecord> record)
        : IoTask(num, record) {}

    MappingData *AllocateMappingData(void);
    void AddExtraMappingData(MappingData *data);
    void FreeMappingData(MappingData *data);
    short GetBoardId(void) const;
    bool IsHardwareDBItemConcerned(const HardwareDBItem *item) const;
    bool IsPointTypeSupported(PointType type);
    int OpenBoard(short instanceNumber);
    void ConfigureBoard(MappingData *data);
    void PostConfigureBoard(void);
    void PreExchangeData(void);
    void ExchangeData(MappingData *data);
    void PostExchangeData(void);
    void CleanupExtraData(void);

    void SetValueToRTDB(MappingData *data, CVTType type, unsigned int offset, double value);
    double GetValueFromRTDB(MappingData *data);

private:
    // Device handle and runtime states
    TB2820_Handler *handler = nullptr;

    uint32_t configureGroup = 0;
    uint32_t ch6Ch5Mode = 0;
    uint32_t ch1Ch6Mode = 0;
    uint32_t ch3Ch4Mode = 0;
    uint32_t ch8Ch9Mode = 0;

    alignas(8) char dmaBuffer[TB2820_DMABUFFER_MAXSIZE] = {0};
    uint32_t quickMsg = 0;
    uint32_t serialMsg = 0;
    uint32_t serialReserved = 0;

    uint32_t hallPolarity[2] = {0};
    uint32_t pulsePolarity[2] = {0};
    uint32_t stepperResetMask = 0;
    uint32_t hallResetMask = 0;
    uint32_t pulseResetMask = 0;
    uint32_t hall3ResetMask[2] = {0, 0};

    uint32_t hallSignalStatus[3] = {0};
    uint32_t pulseSignalStatus[2] = {0};
    uint32_t stepperSignalStatus = 0;

    uint32_t channelTypeMode[TB2820_CHANNEL_MAX] = {0};

    const int TB2820_HALL_START_CHANNEL = 0;
    const int TB2820_HALL_END_CHANNEL = 39;
    const int TB2820_PULSE_CHANNEL_PER_GROUP = 4;
    const int TB2820_PULSE_CHANNEL_MIN = 0;

private:
    // Configure helpers by channel type
    void ConfigureDI(TB2820MappingData *tb2820Data);
    void ConfigurePWMI(TB2820MappingData *tb2820Data);
    void ConfigureSentIn(TB2820MappingData *tb2820Data);
    void ConfigureHall(TB2820MappingData *tb2820Data);
    void ConfigurePulse(TB2820MappingData *tb2820Data);
    void ConfigureStepper(TB2820MappingData *tb2820Data);

    // Exchange helpers by channel type
    void ExchangeDIData(TB2820MappingData *tb2820Data);
    void ExchangePWMIData(TB2820MappingData *tb2820Data);
    void ExchangeSentInData(TB2820MappingData *tb2820Data);
    void ExchangeSentInQuickMsgData(TB2820MappingData *tb2820Data);
    void ExchangeSentInSerialMsgData(TB2820MappingData *tb2820Data);
    void ExchangeSentInDiagnosticData(TB2820MappingData *tb2820Data, unsigned int error);
    void ExchangeHallData(TB2820MappingData *tb2820Data);
    void ExchangeHall3Data(TB2820MappingData *tb2820Data);
    void ExchangePulseData(TB2820MappingData *tb2820Data);
    void ExchangeStepper2Data(TB2820MappingData *tb2820Data);
    void ExchangeStepper4Data(TB2820MappingData *tb2820Data);

    // Misc
    void ProcessThreePhaseHallMotorSpeed(TB2820MappingData *tb2820Data);

    TB2820VarKey String2VarKey(const char *name);
};

#endif // TB2820_H
