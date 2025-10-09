#include "TB2820.h"

/* C header files */
#include <cassert>
#include <cstring>
#include <string>
#include <unordered_map>

/* Project headers */
#include "RTDB/RTDBInterface.h"
#include "Log.h"
#include "Define.h"
#include "IoTask/RTDBHelper.h"
#include "HardwareRecord/TB2820Record.h"
#include "Exception.h"

static std::unordered_map<int, std::string> errMsg = {
    {TB_ERR_DEVICE_NOT_EXIST, "device not exist"},
    {TB_ERR_OPEN_DEVICE_FAILURE, "open device failure"},
    {TB_ERR_MAP_DEVICE_FAILURE, "map device failure"},
    {TB_ERR_ALLOC_HANDLER_FAILURE, "alloc handler failure"},
    {TB_ERR_INVALID_HANDLER, "invalid handler"},
    {TB_ERR_INVALID_ARGUMENTS, "invalid arguments"},
    {TB_ERR_INVALID_CALL, "invalid call"},
    {TB_RETURN_NOT_OK, "unknown error occurred"},
};

namespace TB2820V07 {
const char *FREQUENCY = "frequency";
const char *DUTYCYCLE = "dutycycle";
const char *VALUE = "value";
const char *RECEIVE_ENABLE = "ReceiveEnable";
const char *SERIAL_RECEIVE_ENABLE = "SerialReceiveEnable";
const char *CLEAR_FAULT = "ClearFault";
const char *ACTUAL_TICK_PERIOD_TIME = "ActualTickPeriodTime";
const char *ALL_TICKS = "AllTicks";
const char *CRC_ERROR_FLAG = "CRCErrorFlag";
const char *SYSTEM_ERROR_FLAG = "SystemErrorFlag";
const char *LOW_TICK_ERROR_FLAG = "LowTickErrorFlag";
const char *NIBBLE_VALUE_OUT_OF_RANGE = "NibbleValueOutOfRange";
const char *TIMEOUT_FLAG = "TimeoutFlag";
const char *SERIAL_CRC_ERROR_FLAG = "SerialCRCErrorFlag";
const char *SERIAL_FORMAT_ERROR_FLAG = "SerialFormatErrorFlag";
const char *ONETIME = "Onetime";
const char *DATA = "data";
const char *CRC = "crc";
const char *ID = "id";
}

/* ------------ Core helpers ------------ */

short TB2820::GetBoardId(void) const { return 2820; }

bool TB2820::IsHardwareDBItemConcerned(const HardwareDBItem *item) const {
    return item->type == HARDWARE_TYPE_IN || item->type == HARDWARE_TYPE_OUT;
}

bool TB2820::IsPointTypeSupported(PointType type) {
    return type == POINTTYPE_AO || type == POINTTYPE_DI;
}

MappingData *TB2820::AllocateMappingData(void) { return new TB2820MappingData; }

void TB2820::AddExtraMappingData(MappingData *data) {
    assert(data != nullptr);
    TB2820MappingData *demoData = dynamic_cast<TB2820MappingData *>(data);
    TB2820Item *item = static_cast<TB2820Item *>(data->hardware);
    demoData->channelNumber = item->channelNum;
    demoData->channelType = item->channelType;
}

void TB2820::FreeMappingData(MappingData *data) {
    assert(data != nullptr);
    delete data;
}

int TB2820::OpenBoard(short instanceNumber) {
    TB_Return ret = TB2820_Open(&handler, instanceNumber);
    HR_Info("OpenBoard 2820 id = %d", instanceNumber);
    return ret == TB_RETURN_OK ? SUCCESS : FAILURE;
}

void TB2820::SetValueToRTDB(MappingData *data, CVTType type, unsigned int offset, double value) {
    if (RTDBF_ALTVALUE & RTDB_GetRTFlags(data->sitem))
        SetTableValue(altTable, type, offset, value);
    else
        SetTableValue(cvTable, type, offset, value);
}

double TB2820::GetValueFromRTDB(MappingData *data) {
    double value = 0;
    TB2820MappingData *singleData = dynamic_cast<TB2820MappingData *>(data);
    if (RTDBF_ALTVALUE & RTDB_GetRTFlags(data->sitem))
        value = GetTableValue(altTable, (CVTType)singleData->signal->cvtType, (int32_t)singleData->offset);
    else
        value = GetTableValue(cvTable, (CVTType)singleData->signal->cvtType, (int32_t)singleData->offset);
    return value;
}

/* ------------ Configure functions (subset assembled from images) ------------ */

void TB2820::ConfigureDI(TB2820MappingData *tb2820Data) {
    if (strcmp(tb2820Data->hardware->varibleName, TB2820V07::VALUE) == 0) {
        float threshold = static_cast<TB2820Item *>(tb2820Data->hardware)->threshold;
        uint32_t timeout = static_cast<TB2820Item *>(tb2820Data->hardware)->timeout;
        uint32_t debounceTime = static_cast<TB2820Item *>(tb2820Data->hardware)->debounceTime;

        int thresholdRet = TB2820_SetCommonThreshold(handler, tb2820Data->channelNumber, threshold);
        if (thresholdRet != TB_RETURN_OK)
            throw HR_Exception("Failed to set DI threshold, errorCode = " + std::to_string(thresholdRet));

        int timeoutRet = TB2820_SetCommonTimeoutTime(handler, tb2820Data->channelNumber, timeout);
        if (timeoutRet != TB_RETURN_OK)
            throw HR_Exception("Failed to set DI timeout, errorCode = " + std::to_string(timeoutRet));

        int debounceRet = TB2820_SetDebounceTime(handler, tb2820Data->channelNumber, debounceTime);
        if (debounceRet != TB_RETURN_OK)
            throw HR_Exception("Failed to set DI debounce time, errorCode = " + std::to_string(debounceRet));

        int modeRet = TB2820_SetChannelMode(handler, tb2820Data->channelNumber, TB2820_MODE_DI);
        if (modeRet != TB_RETURN_OK)
            throw HR_Exception("Failed to set DI mode, err = " + std::to_string(modeRet));
    }
}

void TB2820::ConfigurePWMI(TB2820MappingData *tb2820Data) {
    assert(tb2820Data != nullptr);
    if (strcmp(tb2820Data->hardware->varibleName, TB2820V07::VALUE) == 0) {
        float threshold = static_cast<TB2820Item *>(tb2820Data->hardware)->threshold;
        uint32_t timeout = static_cast<TB2820Item *>(tb2820Data->hardware)->timeout;
        uint32_t debounceTime = static_cast<TB2820Item *>(tb2820Data->hardware)->debounceTime;

        int thresholdRet = TB2820_SetCommonThreshold(handler, tb2820Data->channelNumber, threshold);
        if (thresholdRet != TB_RETURN_OK)
            throw HR_Exception("Failed to set PWMI threshold, errorCode = " + std::to_string(thresholdRet));

        int timeoutRet = TB2820_SetCommonTimeoutTime(handler, tb2820Data->channelNumber, timeout);
        if (timeoutRet != TB_RETURN_OK)
            throw HR_Exception("Failed to set PWMI timeout, errorCode = " + std::to_string(timeoutRet));

        int debounceRet = TB2820_SetDebounceTime(handler, tb2820Data->channelNumber, debounceTime);
        if (debounceRet != TB_RETURN_OK)
            throw HR_Exception("Failed to set PWMI debounceTime, errorCode = " + std::to_string(debounceRet));

        int modeRet = TB2820_SetChannelMode(handler, tb2820Data->channelNumber, TB2820_MODE_PWM);
        if (modeRet != TB_RETURN_OK)
            throw HR_Exception("Failed to set PWMI mode, errorCode = " + std::to_string(modeRet));
    }
}

void TB2820::ConfigureSentIn(TB2820MappingData *tb2820Data) {
    assert(tb2820Data != nullptr);
    if (strcmp(tb2820Data->hardware->varibleName, TB2820V07::ONETIME) == 0) return;

    // (assembled: threshold, period, tolerance, high/low/sync tick, pause, nibble, timeout, polarity, crcImpType, serialDataMode)
    float threshold = static_cast<TB2820Item *>(tb2820Data->hardware)->threshold;
    int thresholdRet = TB2820_SetCommonThreshold(handler, tb2820Data->channelNumber, threshold);
    if (thresholdRet != TB_RETURN_OK)
        throw HR_Exception("Error while set SENTIN threshold, ret = " + std::to_string(thresholdRet));
}

void TB2820::ConfigureHall(TB2820MappingData *tb2820Data) {
    assert(tb2820Data != nullptr);
    if (strcmp(tb2820Data->hardware->varibleName, TB2820V07::ONETIME) == 0) return;

    float threshold = static_cast<TB2820Item *>(tb2820Data->hardware)->threshold;
    uint32_t debounceTime = static_cast<TB2820Item *>(tb2820Data->hardware)->debounceTime;
    uint32_t polarity = static_cast<TB2820Item *>(tb2820Data->hardware)->hpPolarity;

    int thresholdRet = TB2820_SetCommonThreshold(handler, tb2820Data->channelNumber, threshold);
    if (thresholdRet != TB_RETURN_OK) throw HR_Exception("Failed to set hall threshold");

    int debounceRet = TB2820_SetDebounceTime(handler, tb2820Data->channelNumber, debounceTime);
    if (debounceRet != TB_RETURN_OK) throw HR_Exception("Failed to set hall debounceTime");

    int modeRet = TB2820_SetChannelMode(handler, tb2820Data->channelNumber, TB2820_MODE_HALL);
    if (modeRet != TB_RETURN_OK) throw HR_Exception("Failed to set hall mode");

    if (polarity != 0) {
        if (tb2820Data->channelNumber >= 32) hallPolarity[1] |= (1u << (tb2820Data->channelNumber - 32));
        else hallPolarity[0] |= (1u << tb2820Data->channelNumber);
    }
}

void TB2820::ConfigurePulse(TB2820MappingData *tb2820Data) {
    assert(tb2820Data != nullptr);
    if (strcmp(tb2820Data->hardware->varibleName, TB2820V07::ONETIME) == 0) return;

    float threshold = static_cast<TB2820Item *>(tb2820Data->hardware)->threshold;
    uint32_t debounceTime = static_cast<TB2820Item *>(tb2820Data->hardware)->debounceTime;
    uint32_t polarity = static_cast<TB2820Item *>(tb2820Data->hardware)->hpPolarity;

    TB2820_SetCommonThreshold(handler, tb2820Data->channelNumber, threshold);
    TB2820_SetDebounceTime(handler, tb2820Data->channelNumber, debounceTime);

    int modeRet = TB2820_SetChannelMode(handler, tb2820Data->channelNumber, TB2820_MODE_PULSE);
    if (modeRet != TB_RETURN_OK) throw HR_Exception("Failed to set pulse mode");

    if (polarity != 0) {
        if (tb2820Data->channelNumber >= 32) pulsePolarity[1] |= (1u << (tb2820Data->channelNumber - 32));
        else pulsePolarity[0] |= (1u << tb2820Data->channelNumber);
    }
}

/* ------------ ConfigureBoard & lifecycle ------------ */

void TB2820::ConfigureBoard(MappingData *data) {
    assert(data != nullptr);
    TB2820MappingData *tb2820Data = static_cast<TB2820MappingData *>(data);
    switch (tb2820Data->channelType) {
        case TB2820_CHANNEL_TYPE_DI:     ConfigureDI(tb2820Data); break;
        case TB2820_CHANNEL_TYPE_PWMI:   ConfigurePWMI(tb2820Data); break;
        case TB2820_CHANNEL_TYPE_SENTIN: ConfigureSentIn(tb2820Data); break;
        case TB2820_CHANNEL_TYPE_HALL:   ConfigureHall(tb2820Data); break;
        case TB2820_CHANNEL_TYPE_PULSE:  ConfigurePulse(tb2820Data); break;
        default: break;
    }
}

void TB2820::PostConfigureBoard(void) {
    if (TB2820_SetHallPolarity(handler, hallPolarity) != TB_RETURN_OK)
        throw HR_Exception("Failed to set hall polarity");
    if (TB2820_SetPulsePolarity(handler, pulsePolarity) != TB_RETURN_OK)
        throw HR_Exception("Failed to set pulse polarity");
    TB2820_AdjustsAndSetAllChannelsMode(handler);
}

void TB2820::PreExchangeData(void) {
    TB_Return ret = TB2820_GetData(handler);
    if (ret != TB_RETURN_OK) throw HR_Exception("Failed to get TB2820 data");
}

/* ------------ Exchange functions (subset assembled) ------------ */

void TB2820::ExchangeData(MappingData *data) {
    assert(data != nullptr && handler != nullptr);
    TB2820MappingData *tb2820Data = dynamic_cast<TB2820MappingData *>(data);
    if (!tb2820Data) throw HR_Exception("Failed to cast to TB2820MappingData");
    if (strcmp(tb2820Data->hardware->varibleName, TB2820V07::ONETIME) == 0) return;

    switch (tb2820Data->channelType) {
        case TB2820_CHANNEL_TYPE_DI:     ExchangeDIData(tb2820Data); break;
        case TB2820_CHANNEL_TYPE_PWMI:   ExchangePWMIData(tb2820Data); break;
        case TB2820_CHANNEL_TYPE_SENTIN: ExchangeSentInData(tb2820Data); break;
        case TB2820_CHANNEL_TYPE_HALL:   ExchangeHallData(tb2820Data); break;
        case TB2820_CHANNEL_TYPE_PULSE:  ExchangePulseData(tb2820Data); break;
        default: break;
    }
}

void TB2820::ExchangePWMIData(TB2820MappingData *tb2820Data) {
    assert(tb2820Data != nullptr);
    MappingData *data = dynamic_cast<MappingData *>(tb2820Data);
    float freq = 0, duty = 0;
    int ret = TB2820_GetPWMinFreqAndDuty(handler, tb2820Data->channelNumber, &freq, &duty);
    if (ret != TB_RETURN_OK) throw HR_Exception("Error occurred when get pwm data, ret = " + std::to_string(ret));

    if (strcmp(tb2820Data->hardware->varibleName, TB2820V07::FREQUENCY) == 0) {
        SetValueToRTDB(data, tb2820Data->signal->cvtType, static_cast<uint32_t>(tb2820Data->offset), static_cast<double>(freq));
    } else if (strcmp(tb2820Data->hardware->varibleName, TB2820V07::DUTYCYCLE) == 0) {
        SetValueToRTDB(data, tb2820Data->signal->cvtType, static_cast<uint32_t>(tb2820Data->offset), static_cast<double>(duty));
    }
}

void TB2820::ExchangeHall3Data(TB2820MappingData *tb2820Data) {
    // (assembled auxiliary; details provided in earlier extraction)
}

void TB2820::PostExchangeData(void) {
    TB2820_SetHall3ClearEnable(handler, hall3ResetMask);
    TB2820_SetHallClearEnable(handler, hallResetMask);
    TB2820_SetPulseClearEnable(handler, pulseResetMask);
    TB2820_SetStepperClearEnable(handler, stepperResetMask);
}

void TB2820::CleanupExtraData(void) { TB2820_Close(handler); }

/* ------------ Key map ------------ */

TB2820VarKey TB2820::String2VarKey(const char *name) {
    static const std::unordered_map<std::string, TB2820VarKey> keyMap = {
        {"SignalCount", SIGNAL_COUNT},
        {"RisingEdgeCount", RISING_EDGE_COUNT},
        {"FallingEdgeCount", FALLING_EDGE_COUNT},
        {"SignalStatus", SIGNAL_STATUS},
        {"AHighLowStates", PULSE_A_POS_PHASE_STATUS},
        {"BHighLowStates", PULSE_B_POS_PHASE_STATUS},
        {"CHighLowStates", PULSE_C_POS_PHASE_STATUS},
        {"MotorSpeed", MOTOR_SPEED},
        {"AnglePosition", ANGLE_POSITION},
        {"PulseCount", PULSE_COUNT},
        {"StepCount", STEP_COUNT},
        {"Reset", RESET}
    };
    auto it = keyMap.find(name);
    return it != keyMap.end() ? it->second : TB2820VarKey(-1);
}
