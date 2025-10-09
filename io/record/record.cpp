#include "TB2820Record.h"

/* C header files */
#include <string>
#include <algorithm>
#include <iostream>
#include <array>

/* Project headers */
#include "Define.h"
#include "StringUtils.h"
#include "Exception.h"

enum HardwareDBKey_TB2820
{
    HARDWAREDBKEY_TB2820_CHANNELNUM,
    HARDWAREDBKEY_TB2820_TIMEOUT,
    HARDWAREDBKEY_TB2820_THRESHOLD,
    HARDWAREDBKEY_TB2820_TICK_PERIOD,
    HARDWAREDBKEY_TB2820_TICK_PERIOD_TOLERANCE,
    HARDWAREDBKEY_TB2820_ZERO_HIGH_TICK,
    HARDWAREDBKEY_TB2820_LOW_TICK_NUM,
    HARDWAREDBKEY_TB2820_SYNC_TICK_NUM,
    HARDWAREDBKEY_TB2820_PAUSE_ENABLE,
    HARDWAREDBKEY_TB2820_SENTINTIMEOUT,
    HARDWAREDBKEY_TB2820_NIBBLE_NUM,
    HARDWAREDBKEY_TB2820_POLARITY,
    HARDWAREDBKEY_TB2820_CRC_IMP_TYPE,
    HARDWAREDBKEY_TB2820_SERIAL_DATA_MODE,
    HARDWAREDBKEY_TB2820_QUICK_MSG_NUM,
    HARDWAREDBKEY_TB2820_SERIAL_MSG_NUM,
    HARDWAREDBKEY_TB2820_DEBOUNCE_TIME,
    HARDWAREDBKEY_TB2820_HALL_MODE,
    HARDWAREDBKEY_TB2820_PULSE_MODE,
    HARDWAREDBKEY_TB2820_SIGNAL_EFFECTIVENESS,
    HARDWAREDBKEY_TB2820_INIT_STEP
};

static HardwareDBKey_TB2820 String2HardwareDBKey_TB2820(const std::string &str);
static TB2820ChannelType String2TB2820ChannelType(const std::string &str);
static TB2820_SentInMsgType String2TB2820SentInMsgType(const std::string &str);
static TB2820_HallMode String2TB2820HallMode(const std::string &mode);
static TB2820_PulseMode String2TB2820PulseMode(const std::string &mode);
static TB2820_HallAndPulsePolarity String2TB2820HallAndPulsePolarity(const std::string &polarity);

int TB2820Record::AnalyzeSpecialAttribute(std::string key, std::string value, HardwareDBItem *item)
{
    try
    {
        auto tb2820Item = static_cast<TB2820Item *>(item);
        auto castedValue = static_cast<uint32_t>(std::stoi(value));

        switch (String2HardwareDBKey_TB2820(key))
        {
        case HARDWAREDBKEY_TB2820_CHANNELNUM:
            tb2820Item->channelNum = castedValue;
            break;
        case HARDWAREDBKEY_TB2820_THRESHOLD:
            tb2820Item->threshold = std::stof(value);
            break;
        case HARDWAREDBKEY_TB2820_TIMEOUT:
            tb2820Item->timeout = castedValue;
            break;
        case HARDWAREDBKEY_TB2820_TICK_PERIOD:
            tb2820Item->tickPeriod = std::stof(value);
            break;
        case HARDWAREDBKEY_TB2820_TICK_PERIOD_TOLERANCE:
            tb2820Item->tickPeriodTolerance = std::stof(value);
            break;
        case HARDWAREDBKEY_TB2820_ZERO_HIGH_TICK:
            tb2820Item->zeroNibbleHighTick = castedValue;
            break;
        case HARDWAREDBKEY_TB2820_LOW_TICK_NUM:
            tb2820Item->lowTickNum = castedValue;
            break;
        case HARDWAREDBKEY_TB2820_SYNC_TICK_NUM:
            tb2820Item->synHighTick = castedValue;
            break;
        case HARDWAREDBKEY_TB2820_PAUSE_ENABLE:
            tb2820Item->pauseEnable = castedValue;
            break;
        case HARDWAREDBKEY_TB2820_NIBBLE_NUM:
            tb2820Item->nibbleMessageNumber = castedValue;
            break;
        case HARDWAREDBKEY_TB2820_POLARITY:
            tb2820Item->polarity = castedValue;
            break;
        case HARDWAREDBKEY_TB2820_CRC_IMP_TYPE:
            tb2820Item->crcImpType = castedValue;
            break;
        case HARDWAREDBKEY_TB2820_SERIAL_DATA_MODE:
            tb2820Item->serialDataMode = castedValue;
            break;
        case HARDWAREDBKEY_TB2820_QUICK_MSG_NUM:
            tb2820Item->quickMsgTotalNum = castedValue;
            break;
        case HARDWAREDBKEY_TB2820_SERIAL_MSG_NUM:
            tb2820Item->serialMsgTotalNum = castedValue;
            break;
        case HARDWAREDBKEY_TB2820_DEBOUNCE_TIME:
            tb2820Item->debounceTime = castedValue;
            break;
        case HARDWAREDBKEY_TB2820_HALL_MODE:
            tb2820Item->hallMode = String2TB2820HallMode(value);
            printf("HARDWAREDBKEY_TB2820_HALL_MODE = %d\n", tb2820Item->hallMode);
            break;
        case HARDWAREDBKEY_TB2820_PULSE_MODE:
            tb2820Item->pulseMode = String2TB2820PulseMode(value);
            printf("HARDWAREDBKEY_TB2820_PULSE_MODE = %d\n", tb2820Item->pulseMode);
            break;
        case HARDWAREDBKEY_TB2820_SIGNAL_EFFECTIVENESS:
            tb2820Item->hpPolarity = String2TB2820HallAndPulsePolarity(value);
            break;
        case HARDWAREDBKEY_TB2820_INIT_STEP:
            tb2820Item->initStep = castedValue;
            break;
        default:
            break;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        throw;
    }
    return SUCCESS;
}

int HandleQuickSerialCase(TB2820Item *tbItem, const std::vector<std::string> &result, const std::string &key)
{
    if (result.size() < 5)
        throw HR_Exception("Invalid " + key + " format, expected 5 parts, got " + std::to_string(result.size()));

    const std::string &message = result[3];
    const std::string &variableName = result[4];
    auto numberParts = SplitString(message, ":", false);

    if (numberParts.size() < 2 || numberParts[1].empty())
        throw HR_Exception("Invalid message format: " + message);

    try
    {
        unsigned int msgNum = std::stoul(numberParts[1]);
        if (key == "Quick")
            tbItem->quickMsgNum = msgNum;
        else
            tbItem->serialMsgNum = msgNum;
    }
    catch (const std::exception &e)
    {
        throw HR_Exception("Invalid number format: " + numberParts[1]);
    }

    tbItem->msgType = String2TB2820SentInMsgType(key);
    strncpy(tbItem->varibleName, variableName.c_str(), sizeof(tbItem->varibleName) - 1);
    tbItem->varibleName[sizeof(tbItem->varibleName) - 1] = '\0';

    // nibble extraction
    const std::array<std::string, 3> EXEMPT_NAMES = {"crc", "impType", "data_id"};
    if (std::find(EXEMPT_NAMES.begin(), EXEMPT_NAMES.end(), variableName) == EXEMPT_NAMES.end())
    {
        auto nibbleParts = SplitString(variableName, "_", false);
        if (nibbleParts.size() >= 2 && !nibbleParts[1].empty())
        {
            try
            {
                tbItem->quickNibbleNum = std::stoul(nibbleParts[1]);
            }
            catch (const std::exception &e)
            {
                throw HR_Exception("Invalid nibble format: " + nibbleParts[1]);
            }
        }
    }

    return SUCCESS;
}

int TB2820Record::AnalyzeRname(std::string value, HardwareDBItem *item)
{
    std::vector<std::string> result = SplitString(value, ".", false);
    if (result.size() < 1)
        throw HR_Exception("Invalid result size: " + std::to_string(result.size()));

    TB2820ChannelType channelType = String2TB2820ChannelType(result[0]);
    auto tbItem = static_cast<TB2820Item *>(item);
    tbItem->channelType = channelType;

    if (channelType == TB2820_CHANNEL_TYPE_SENTIN)
        return AnalyzeRnameForSentIn(result, tbItem);

    throw HR_Exception("Invalid channelType format: " + result[0]);
}

int AnalyzeRnameForSentIn(const std::vector<std::string> &result, TB2820Item *tbItem)
{
    if (result.size() < 3)
        throw HR_Exception("Invalid sentin result size: " + std::to_string(result.size()));

    const std::string &field = result[2];

    if (field == "Quick" || field == "Serial")
        return HandleQuickSerialCase(tbItem, result, field);

    throw HR_Exception("Invalid sentin field: " + field);
}

HardwareDBKey_TB2820 String2HardwareDBKey_TB2820(const std::string &str)
{
    static const std::unordered_map<std::string, HardwareDBKey_TB2820> keyMap = {
        {"channelNum", HARDWAREDBKEY_TB2820_CHANNELNUM},
        {"threshold", HARDWAREDBKEY_TB2820_THRESHOLD},
        {"timeout", HARDWAREDBKEY_TB2820_TIMEOUT},
        {"tickPeriod", HARDWAREDBKEY_TB2820_TICK_PERIOD},
        {"tickPeriodTolerance", HARDWAREDBKEY_TB2820_TICK_PERIOD_TOLERANCE},
        {"zeroNibbleHighTick", HARDWAREDBKEY_TB2820_ZERO_HIGH_TICK},
        {"lowTick", HARDWAREDBKEY_TB2820_LOW_TICK_NUM},
        {"synHighTick", HARDWAREDBKEY_TB2820_SYNC_TICK_NUM},
        {"pauseEnable", HARDWAREDBKEY_TB2820_PAUSE_ENABLE},
        {"nibbleMessageNumber", HARDWAREDBKEY_TB2820_NIBBLE_NUM},
        {"polarity", HARDWAREDBKEY_TB2820_POLARITY},
        {"crcImpType", HARDWAREDBKEY_TB2820_CRC_IMP_TYPE},
        {"serialDataMode", HARDWAREDBKEY_TB2820_SERIAL_DATA_MODE},
        {"quickMsgNumber", HARDWAREDBKEY_TB2820_QUICK_MSG_NUM},
        {"serialMsgNumber", HARDWAREDBKEY_TB2820_SERIAL_MSG_NUM},
        {"debounceTime", HARDWAREDBKEY_TB2820_DEBOUNCE_TIME},
        {"hallMode", HARDWAREDBKEY_TB2820_HALL_MODE},
        {"pulseMode", HARDWAREDBKEY_TB2820_PULSE_MODE},
        {"signalEffectiveness", HARDWAREDBKEY_TB2820_SIGNAL_EFFECTIVENESS},
        {"initStep", HARDWAREDBKEY_TB2820_INIT_STEP}};

    auto it = keyMap.find(str);
    return it != keyMap.end() ? it->second : (HardwareDBKey_TB2820)-1;
}

TB2820_HallMode String2TB2820HallMode(const std::string &mode)
{
    static const std::unordered_map<std::string, TB2820_HallMode> keyMap = {
        {"Hall", TB2820_HALL},
        {"ThreePhaseHall", TB2820_THREE_PHASE_HALL}};
    auto it = keyMap.find(mode);
    return it != keyMap.end() ? it->second : (TB2820_HallMode)-1;
}

TB2820_PulseMode String2TB2820PulseMode(const std::string &mode)
{
    static const std::unordered_map<std::string, TB2820_PulseMode> keyMap = {
        {"SinglePhasePulse", TB2820_PULSE},
        {"TwoPhaseFourStep", TB2820_TWO_PHASE_FOUR_STEP},
        {"TwoPhaseEightStep", TB2820_TWO_PHASE_EIGHT_STEP},
        {"FourPhaseSingleFourStepMotor", TB2820_FOUR_PHASE_SINGLE_FOUR_STEP},
        {"FourPhaseDoubleFourStepMotor", TB2820_FOUR_PHASE_DOUBLE_FOUR_STEP},
        {"FourPhaseEightStepMotor", TB2820_FOUR_PHASE_EIGHT_STEP}};
    auto it = keyMap.find(mode);
    return it != keyMap.end() ? it->second : (TB2820_PulseMode)-1;
}

TB2820_HallAndPulsePolarity String2TB2820HallAndPulsePolarity(const std::string &polarity)
{
    static const std::unordered_map<std::string, TB2820_HallAndPulsePolarity> keyMap = {
        {"HighEffectiveness", TB2820_POLARITY_HIGH_ACTIVE},
        {"LowEffectiveness", TB2820_POLARITY_LOW_ACTIVE}};
    auto it = keyMap.find(polarity);
    return it != keyMap.end() ? it->second : (TB2820_HallAndPulsePolarity)-1;
}

TB2820ChannelType String2TB2820ChannelType(const std::string &str)
{
    static const std::unordered_map<std::string, TB2820ChannelType> keyMap = {
        {"di", TB2820_CHANNEL_TYPE_DI},
        {"pwmin", TB2820_CHANNEL_TYPE_PWMI},
        {"sentin", TB2820_CHANNEL_TYPE_SENTIN},
        {"hall", TB2820_CHANNEL_TYPE_HALL},
        {"pulse", TB2820_CHANNEL_TYPE_PULSE}};
    auto it = keyMap.find(str);
    return it != keyMap.end() ? it->second : (TB2820ChannelType)-1;
}

TB2820_SentInMsgType String2TB2820SentInMsgType(const std::string &str)
{
    static const std::unordered_map<std::string, TB2820_SentInMsgType> keyMap = {
        {"Quick", TB2820_QUICK_MSG},
        {"Serial", TB2820_SERIAL_MSG},
        {"SerialReserved", TB2820_SERIAL_RESERVED}};
    auto it = keyMap.find(str);
    return it != keyMap.end() ? it->second : (TB2820_SentInMsgType)-1;
}
