#ifndef TB2820_RECORD_H
#define TB2820_RECORD_H

/* Project headers */
#include "HardwareDB/HardwareDB.h"

enum TB2820ChannelType
{
    TB2820_CHANNEL_TYPE_DI = 0,
    TB2820_CHANNEL_TYPE_PWMI = 1,
    TB2820_CHANNEL_TYPE_SENTIN = 2,
    TB2820_CHANNEL_TYPE_HALL,
    TB2820_CHANNEL_TYPE_PULSE
};

enum TB2820_SentInMsgType
{
    TB2820_QUICK_MSG,
    TB2820_SERIAL_MSG,
    TB2820_SERIAL_RESERVED,
};

enum TB2820_HallMode
{
    TB2820_HALL,
    TB2820_THREE_PHASE_HALL
};

enum TB2820_PulseMode
{
    TB2820_PULSE,
    TB2820_TWO_PHASE_FOUR_STEP,
    TB2820_TWO_PHASE_EIGHT_STEP,
    TB2820_FOUR_PHASE_SINGLE_FOUR_STEP,
    TB2820_FOUR_PHASE_DOUBLE_FOUR_STEP,
    TB2820_FOUR_PHASE_EIGHT_STEP
};

enum TB2820_HallAndPulsePolarity
{
    TB2820_POLARITY_HIGH_ACTIVE,
    TB2820_POLARITY_LOW_ACTIVE
};

class TB2820Item : public HardwareDBItem
{
public:
    /* config */
    // ** once **
    float threshold;
    uint32_t timeout;
    uint32_t debounceTime;
    uint32_t hpPolarity;
    uint32_t hallMode;
    uint32_t pulseMode;
    uint32_t initStep;

    char diagnostic[MAX_RECORDNAME];
    float tickPeriod;
    float tickPeriodTolerance;
    uint32_t zeroNibbleHighTick;
    uint32_t lowTickNum;
    uint32_t synHighTick;
    uint32_t pauseEnable;
    uint32_t nibbleMessageNumber;
    uint32_t polarity;
    uint32_t crcImpType;
    uint32_t serialDataMode;
    uint32_t quickMsgTotalNum;
    uint32_t serialMsgTotalNum;

    // once
    uint32_t channelNum;
    int channelType;
    uint32_t msgType;
    uint32_t quickMsgNum;
    uint32_t quickNibbleNum;
    uint32_t serialMsgNum;
};

class TB2820Record : public HardwareRecord
{
public:
    int AnalyzeSpecialAttribute(std::string key, std::string value, HardwareDBItem *item);
    int AnalyzeRname(std::string rname, HardwareDBItem *item);

    std::string BoardName() const { return "HR-TB2820"; }
    short BoardId() const { return 2820; }

    std::shared_ptr<HardwareDBItem> NewItem()
    {
        return std::make_shared<TB2820Item>();
    }
};

#endif // TB2820_RECORD_H
