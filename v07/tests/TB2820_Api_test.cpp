#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdarg>
#include <cstdio>
#include <string>
#include <vector>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

extern "C" {
#include "TB2820_Api.h"
#include "TB2820_Register.h"
}

extern "C" TB_Return TB2820_DemuxDMAData(TB2820_Handler *handler, TB_U8 *buffer, size_t *currentOffset, size_t bufSize, TB_U32 channel);
extern "C" TB_Float ConvertRawToVoltage(TB_U32 raw);
extern "C" TB_Float ConvertRawToTemperature(TB_U32 raw);
extern "C" ssize_t __real_read(int fd, void *buf, size_t count);
extern "C" int __real_open(const char *pathname, int flags, ...);
extern "C" int __real_close(int fd);
extern "C" void *__real_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
extern "C" int __real_munmap(void *addr, size_t length);
extern "C" off_t __real_lseek(int fd, off_t offset, int whence);

#include "cppstub/Stub.h"

static cppstub::Stub<ssize_t(int, void *, size_t)> g_readStub;
static cppstub::Stub<int(const char *, int)> g_openStub;
static cppstub::Stub<int(int)> g_closeStub;
static cppstub::Stub<void *(void *, size_t, int, int, int, off_t)> g_mmapStub;
static cppstub::Stub<int(void *, size_t)> g_munmapStub;
static cppstub::Stub<off_t(int, off_t, int)> g_lseekStub;

extern "C" ssize_t __wrap_read(int fd, void *buf, size_t count)
{
    if (g_readStub.HasStub())
    {
        return g_readStub.Invoke(fd, buf, count);
    }
    return __real_read(fd, buf, count);
}


extern "C" int __wrap_open(const char *pathname, int flags, ...)
{
    if (g_openStub.HasStub())
    {
        return g_openStub.Invoke(pathname, flags);
    }

    va_list args;
    va_start(args, flags);
    int result = 0;
    if (flags & O_CREAT)
    {
        mode_t mode = va_arg(args, mode_t);
        result = __real_open(pathname, flags, mode);
    }
    else
    {
        result = __real_open(pathname, flags);
    }
    va_end(args);
    return result;
}

extern "C" int __wrap_close(int fd)
{
    if (g_closeStub.HasStub())
    {
        return g_closeStub.Invoke(fd);
    }
    return __real_close(fd);
}

extern "C" void *__wrap_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    if (g_mmapStub.HasStub())
    {
        return g_mmapStub.Invoke(addr, length, prot, flags, fd, offset);
    }
    return __real_mmap(addr, length, prot, flags, fd, offset);
}

extern "C" int __wrap_munmap(void *addr, size_t length)
{
    if (g_munmapStub.HasStub())
    {
        return g_munmapStub.Invoke(addr, length);
    }
    return __real_munmap(addr, length);
}

extern "C" off_t __wrap_lseek(int fd, off_t offset, int whence)
{
    if (g_lseekStub.HasStub())
    {
        return g_lseekStub.Invoke(fd, offset, whence);
    }
    return __real_lseek(fd, offset, whence);
}


static ssize_t ReadStubFailure(int fd, void *buf, size_t count)
{
    (void)fd;
    (void)buf;
    (void)count;
    return -1;
}

static ssize_t ReadStubSuccess(int fd, void *buf, size_t count)
{
    (void)fd;
    unsigned char *data = static_cast<unsigned char *>(buf);
    for (size_t i = 0; i < count; ++i)
    {
        data[i] = 0xAB;
    }
    return static_cast<ssize_t>(count);
}

static int g_openResults[4] = {0, 0, 0, 0};
static size_t g_openResultCount = 0U;
static size_t g_openResultIndex = 0U;
static int g_lastClosedFd = -1;
static void *g_mmapReturnAddress = NULL;
static void *g_lastMunmapAddress = NULL;
static size_t g_lastMunmapLength = 0U;
static TB_U8 g_dmaStubBuffer[TB2820_DMABUFFER_MAXSIZE] = {0};
static size_t g_dmaStubLength = 0U;

static void ResetOpenResults()
{
    size_t i = 0U;
    for (; i < 4U; ++i)
    {
        g_openResults[i] = 0;
    }
    g_openResultCount = 0U;
    g_openResultIndex = 0U;
}

static void ConfigureOpenResults(const int *values, size_t count)
{
    ResetOpenResults();
    size_t limit = count;
    if (limit > 4U)
    {
        limit = 4U;
    }
    size_t i = 0U;
    for (; i < limit; ++i)
    {
        g_openResults[i] = values[i];
    }
    g_openResultCount = limit;
}

static int OpenStubSequence(const char *pathname, int flags)
{
    (void)pathname;
    (void)flags;
    if (g_openResultIndex < g_openResultCount)
    {
        int result = g_openResults[g_openResultIndex];
        ++g_openResultIndex;
        return result;
    }
    return -1;
}

static int CloseStubRecord(int fd)
{
    g_lastClosedFd = fd;
    return 0;
}

static void *MmapStubReturnFixed(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    (void)addr;
    (void)length;
    (void)prot;
    (void)flags;
    (void)fd;
    (void)offset;
    return g_mmapReturnAddress;
}

static void *MmapStubFailure(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    (void)addr;
    (void)length;
    (void)prot;
    (void)flags;
    (void)fd;
    (void)offset;
    return MAP_FAILED;
}

static int MunmapStubRecord(void *addr, size_t length)
{
    g_lastMunmapAddress = addr;
    g_lastMunmapLength = length;
    return 0;
}

static off_t LseekStubIgnore(int fd, off_t offset, int whence)
{
    (void)fd;
    (void)offset;
    (void)whence;
    return 0;
}

static ssize_t ReadStubCopyPreloaded(int fd, void *buf, size_t count)
{
    (void)fd;
    if (g_dmaStubLength < count)
    {
        return -1;
    }
    std::memcpy(buf, g_dmaStubBuffer, count);
    return static_cast<ssize_t>(count);
}

static void StoreWord(TB_U8 *buffer, size_t offset, TB_U32 value)
{
    std::memcpy(buffer + offset, &value, sizeof(TB_U32));
}

class StderrCapture
{
public:
    StderrCapture() : oldFd_(-1), readFd_(-1)
    {
        fflush(stderr);
        int pipeFds[2] = {-1, -1};
        if (pipe(pipeFds) == 0)
        {
            oldFd_ = dup(fileno(stderr));
            dup2(pipeFds[1], fileno(stderr));
            close(pipeFds[1]);
            readFd_ = pipeFds[0];
        }
    }

    ~StderrCapture()
    {
        if (oldFd_ >= 0)
        {
            fflush(stderr);
            dup2(oldFd_, fileno(stderr));
            close(oldFd_);
        }
        if (readFd_ >= 0)
        {
            close(readFd_);
        }
    }

    std::string GetOutput()
    {
        std::string result;
        if (readFd_ < 0)
        {
            return result;
        }
        fflush(stderr);
        char buffer[256];
        ssize_t count = 0;
        while ((count = ::read(readFd_, buffer, sizeof(buffer))) > 0)
        {
            result.append(buffer, static_cast<size_t>(count));
            if (count < static_cast<ssize_t>(sizeof(buffer)))
            {
                break;
            }
        }
        return result;
    }

private:
    int oldFd_;
    int readFd_;
};

class TB2820ApiTest : public ::testing::Test
{
protected:
    TB2820ApiTest() : registerSpace_(2048, 0)
    {
    }

    void SetUp() override
    {
        std::memset(&handler_, 0, sizeof(handler_));
        std::fill(registerSpace_.begin(), registerSpace_.end(), 0);
        handler_.bar0Address = registerSpace_.data();
    }

    void TearDown() override
    {
        g_readStub.Reset();
        g_openStub.Reset();
        g_closeStub.Reset();
        g_mmapStub.Reset();
        g_munmapStub.Reset();
        g_lseekStub.Reset();
        ResetOpenResults();
        g_lastClosedFd = -1;
        g_mmapReturnAddress = NULL;
        g_lastMunmapAddress = NULL;
        g_lastMunmapLength = 0U;
        g_dmaStubLength = 0U;
        std::memset(g_dmaStubBuffer, 0, sizeof(g_dmaStubBuffer));
    }

    TB_U32 ReadRegister(TB_U32 offset) const
    {
        return registerSpace_.at(offset / static_cast<TB_U32>(sizeof(TB_U32)));
    }

    void WriteRegister(TB_U32 offset, TB_U32 value)
    {
        registerSpace_.at(offset / static_cast<TB_U32>(sizeof(TB_U32))) = value;
    }

    TB2820_Handler handler_;
    std::vector<TB_U32> registerSpace_;
};

TEST_F(TB2820ApiTest, OpenRejectsNullHandlerPointer)
{
    TB_Return result = TB2820_Open(nullptr, 0U);
    EXPECT_EQ(TB_RETURN_NOT_OK, result);
}

TEST_F(TB2820ApiTest, OpenFailsWhenDeviceOpenFails)
{
    int openResults[1] = {-1};
    ConfigureOpenResults(openResults, 1U);
    g_openStub.SetStub(OpenStubSequence);
    g_closeStub.SetStub(CloseStubRecord);

    TB2820_Handler *handler = NULL;
    TB_Return result = TB2820_Open(&handler, 0U);

    EXPECT_EQ(TB_RETURN_NOT_OK, result);
    EXPECT_TRUE(handler == nullptr);
    EXPECT_EQ(1U, g_openStub.GetCallCount());
    EXPECT_EQ(0U, g_closeStub.GetCallCount());
}

TEST_F(TB2820ApiTest, OpenFailsWhenMmapFails)
{
    int openResults[1] = {3};
    ConfigureOpenResults(openResults, 1U);
    g_openStub.SetStub(OpenStubSequence);
    g_mmapStub.SetStub(MmapStubFailure);
    g_closeStub.SetStub(CloseStubRecord);

    TB2820_Handler *handler = NULL;
    TB_Return result = TB2820_Open(&handler, 1U);

    EXPECT_EQ(TB_RETURN_NOT_OK, result);
    EXPECT_TRUE(handler == nullptr);
    EXPECT_EQ(1U, g_openStub.GetCallCount());
    EXPECT_EQ(1U, g_mmapStub.GetCallCount());
    EXPECT_EQ(1U, g_closeStub.GetCallCount());
    EXPECT_EQ(3, g_lastClosedFd);
}

TEST_F(TB2820ApiTest, OpenFailsWhenDmaOpenFails)
{
    int openResults[2] = {3, -1};
    ConfigureOpenResults(openResults, 2U);
    g_openStub.SetStub(OpenStubSequence);
    g_mmapReturnAddress = registerSpace_.data();
    g_mmapStub.SetStub(MmapStubReturnFixed);
    g_closeStub.SetStub(CloseStubRecord);
    g_munmapStub.SetStub(MunmapStubRecord);

    TB2820_Handler *handler = NULL;
    TB_Return result = TB2820_Open(&handler, 2U);

    EXPECT_EQ(TB_RETURN_NOT_OK, result);
    EXPECT_TRUE(handler == nullptr);
    EXPECT_EQ(2U, g_openStub.GetCallCount());
    EXPECT_EQ(1U, g_mmapStub.GetCallCount());
    EXPECT_EQ(1U, g_munmapStub.GetCallCount());
    EXPECT_EQ(registerSpace_.data(), g_lastMunmapAddress);
    EXPECT_EQ(TB_MEM_MB, g_lastMunmapLength);
    EXPECT_EQ(1U, g_closeStub.GetCallCount());
    EXPECT_EQ(3, g_lastClosedFd);
}

TEST_F(TB2820ApiTest, OpenInitializesHandlerOnSuccess)
{
    int openResults[2] = {3, 4};
    ConfigureOpenResults(openResults, 2U);
    g_openStub.SetStub(OpenStubSequence);
    g_mmapReturnAddress = registerSpace_.data();
    g_mmapStub.SetStub(MmapStubReturnFixed);
    g_closeStub.SetStub(CloseStubRecord);

    TB2820_Handler *handler = NULL;
    TB_Return result = TB2820_Open(&handler, 3U);

    ASSERT_EQ(TB_RETURN_OK, result);
    ASSERT_TRUE(handler != nullptr);
    EXPECT_EQ(3, handler->fd);
    EXPECT_EQ(4, handler->dmaFd);
    EXPECT_EQ(registerSpace_.data(), handler->bar0Address);
    EXPECT_TRUE(std::string(handler->deviceName) == std::string("/dev/TBPCIE_2820_3"));
    EXPECT_EQ(TB2820_NO_RESET, ReadRegister(TB2820_BOARDINFO_SOFT_RESET_REGISTER_OFFSET));
    EXPECT_EQ(2U, g_openStub.GetCallCount());
    EXPECT_EQ(1U, g_mmapStub.GetCallCount());

    TB2820_Close(&handler);
    EXPECT_TRUE(handler == nullptr);
    EXPECT_EQ(2U, g_closeStub.GetCallCount());
    EXPECT_EQ(4, g_lastClosedFd);
}

TEST_F(TB2820ApiTest, SetCommonThresholdNullHandler)
{
    TB_Return result = TB2820_SetCommonThreshold(nullptr, 0U, 1.0f);
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, result);
}

TEST_F(TB2820ApiTest, SetCommonThresholdRejectsOutOfRangeChannel)
{
    TB_Return result = TB2820_SetCommonThreshold(&handler_, TB2820_CHANNEL_MAX, 1.0f);
    EXPECT_EQ(TB_ERR_INVALID_CHANNEL, result);
}

TEST_F(TB2820ApiTest, SetCommonThresholdRejectsInvalidThreshold)
{
    TB_Return negativeResult = TB2820_SetCommonThreshold(&handler_, 0U, -0.1f);
    EXPECT_EQ(TB_RETURN_NOT_OK, negativeResult);

    TB_Return highResult = TB2820_SetCommonThreshold(&handler_, 0U, 30.0f);
    EXPECT_EQ(TB_RETURN_NOT_OK, highResult);
}

TEST_F(TB2820ApiTest, SetCommonThresholdWritesConvertedValue)
{
    const TB_Float threshold = 5.5f;
    const TB_U32 channel = 3U;

    const TB_Float ratio = 31.6f / (360.0f + 31.6f);
    const TB_Float referenceVoltage = 2.048f;
    const TB_U32 adcResolution = 4096U;
    const TB_U32 expectedValue = static_cast<TB_U32>((threshold * ratio * adcResolution) / referenceVoltage);

    TB_Return result = TB2820_SetCommonThreshold(&handler_, channel, threshold);

    EXPECT_EQ(TB_RETURN_OK, result);
    EXPECT_EQ(expectedValue, ReadRegister(TB2820_DI_CHANNEL_THRESHOLD_OFFSET + channel * 4U));
}

TEST_F(TB2820ApiTest, SetCommonTimeoutTimeWritesScaledValue)
{
    const TB_U32 channel = 5U;
    const TB_U32 timeout = 12U;
    const TB_U32 expectedValue = 100000U * timeout;

    TB_Return result = TB2820_SetCommonTimeoutTime(&handler_, channel, timeout);

    EXPECT_EQ(TB_RETURN_OK, result);
    EXPECT_EQ(expectedValue, ReadRegister(TB2820_DI_CHANNEL_TIMEOUT_OFFSET + channel * 4U));
}

TEST_F(TB2820ApiTest, SetCommonTimeoutTimeRejectsInvalidInput)
{
    TB_Return nullHandler = TB2820_SetCommonTimeoutTime(nullptr, 0U, 1U);
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, nullHandler);

    TB_Return invalidChannel = TB2820_SetCommonTimeoutTime(&handler_, TB2820_CHANNEL_MAX, 1U);
    EXPECT_EQ(TB_ERR_INVALID_CHANNEL, invalidChannel);
}

TEST_F(TB2820ApiTest, SetChannelModeRejectsInvalidInputs)
{
    TB_Return nullHandler = TB2820_SetChannelMode(nullptr, 0U, TB2820_MODE_PWM);
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, nullHandler);

    TB_Return outOfRange = TB2820_SetChannelMode(&handler_, TB2820_CHANNEL_MAX, TB2820_MODE_PWM);
    EXPECT_EQ(TB_ERR_INVALID_CHANNEL, outOfRange);

    TB_Return invalidForLowRange = TB2820_SetChannelMode(&handler_, 2U, TB2820_MODE_PHASE2_STEP4);
    EXPECT_EQ(TB_ERR_INVALID_ARGUMENTS, invalidForLowRange);

    TB_Return invalidForHighRange = TB2820_SetChannelMode(&handler_, 40U, TB2820_MODE_HALL);
    EXPECT_EQ(TB_ERR_INVALID_ARGUMENTS, invalidForHighRange);
}

TEST_F(TB2820ApiTest, SetChannelModeUpdatesHandlerForValidMode)
{
    TB_Return result = TB2820_SetChannelMode(&handler_, 10U, TB2820_MODE_PWM);

    EXPECT_EQ(TB_RETURN_OK, result);
    EXPECT_EQ(TB2820_MODE_PWM, handler_.info[10U].mode);
}

TEST_F(TB2820ApiTest, AdjustAndSetAllChannelsModePropagatesGroupModesAndCounts)
{
    handler_.info[0U].mode = TB2820_MODE_HALL3;
    handler_.info[36U].mode = TB2820_MODE_PHASE4_STEP8;
    handler_.info[10U].mode = TB2820_MODE_PWM;

    TB_Return result = TB2820_AdjustAndSetAllChannelsMode(&handler_);

    EXPECT_EQ(TB_RETURN_OK, result);
    EXPECT_EQ(TB2820_MODE_HALL3, handler_.info[0U].mode);
    EXPECT_EQ(TB2820_MODE_HALL3, handler_.info[1U].mode);
    EXPECT_EQ(TB2820_MODE_HALL3, handler_.info[2U].mode);

    EXPECT_EQ(TB2820_MODE_PHASE4_STEP8, handler_.info[36U].mode);
    EXPECT_EQ(TB2820_MODE_PHASE4_STEP8, handler_.info[37U].mode);
    EXPECT_EQ(TB2820_MODE_PHASE4_STEP8, handler_.info[38U].mode);
    EXPECT_EQ(TB2820_MODE_PHASE4_STEP8, handler_.info[39U].mode);

    EXPECT_EQ(1U, handler_.pwmChannelCount);
    EXPECT_EQ(3U, handler_.hall3ChannelCount);
    EXPECT_EQ(4U, handler_.stepper48ChannelCount);

    EXPECT_EQ(0x00000555U, ReadRegister(TB2820_MODE_OFFSET));
    EXPECT_EQ(0xDDDD0000U, ReadRegister(TB2820_MODE_OFFSET + 4U * 4U));
}

TEST_F(TB2820ApiTest, SetHallPolarityWritesBothRegisters)
{
    TB_Return result = TB2820_SetHallPolarity(&handler_, 0x12345678U, 0x9ABCDEF0U);

    EXPECT_EQ(TB_RETURN_OK, result);
    EXPECT_EQ(0x12345678U, ReadRegister(TB2820_HALL_POLARITY_OFFSET));
    EXPECT_EQ(0x9ABCDEF0U, ReadRegister(TB2820_HALL_POLARITY_OFFSET + 4U));
}

TEST_F(TB2820ApiTest, SetPulsePolarityWritesRegister)
{
    TB_Return result = TB2820_SetPulsePolarity(&handler_, 0x13579BDFU);

    EXPECT_EQ(TB_RETURN_OK, result);
    EXPECT_EQ(0x13579BDFU, ReadRegister(TB2820_PULSE_POLARITY_OFFSET));
}

TEST_F(TB2820ApiTest, SetStepperInitStepsHandleErrors)
{
    TB_Return nullHandler = TB2820_SetStepper2InitSteps(nullptr, 0U, 0U);
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, nullHandler);

    TB_Return invalidGroup = TB2820_SetStepper2InitSteps(&handler_, TB2820_MOTOR_GROUP_MAX + 1U, 0U);
    EXPECT_EQ(TB_ERR_INVALID_CHANNEL, invalidGroup);

    TB_Return valid = TB2820_SetStepper2InitSteps(&handler_, 2U, 55U);
    EXPECT_EQ(TB_RETURN_OK, valid);
    EXPECT_EQ(55U, ReadRegister(TB2820_STEPPER2_INIT_STEPS_OFFSET + 2U * 4U));

    TB_Return invalidGroup4 = TB2820_SetStepper4InitSteps(&handler_, TB2820_MOTOR_GROUP_MAX + 1U, 0U);
    EXPECT_EQ(TB_ERR_INVALID_CHANNEL, invalidGroup4);

    TB_Return valid4 = TB2820_SetStepper4InitSteps(&handler_, 3U, 77U);
    EXPECT_EQ(TB_RETURN_OK, valid4);
    EXPECT_EQ(77U, ReadRegister(TB2820_STEPPER4_INIT_STEPS_OFFSET + 3U * 4U));
}

TEST_F(TB2820ApiTest, SetDebounceTimeValidatesInputs)
{
    TB_Return nullHandler = TB2820_SetDebounceTime(nullptr, 0U, 1U);
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, nullHandler);

    TB_Return invalidChannel = TB2820_SetDebounceTime(&handler_, TB2820_CHANNEL_MAX + 1U, 1U);
    EXPECT_EQ(TB_ERR_INVALID_CHANNEL, invalidChannel);

    TB_Return zeroTime = TB2820_SetDebounceTime(&handler_, 1U, 0U);
    EXPECT_EQ(TB_ERR_INVALID_ARGUMENTS, zeroTime);

    TB_Return valid = TB2820_SetDebounceTime(&handler_, 4U, 100U);
    EXPECT_EQ(TB_RETURN_OK, valid);
    EXPECT_EQ(10U, ReadRegister(TB2820_DEBOUNCE_TIME_OFFSET + 4U * 4U));
}

TEST_F(TB2820ApiTest, ClearEnableFunctionsWriteToRegisters)
{
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_SetHallClearEnable(nullptr, 0U, 0U));
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_SetPulseClearEnable(nullptr, 0U));
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_SetStepperClearEnable(nullptr, 0U));
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_SetHall3ClearEnable(nullptr, 0U));

    EXPECT_EQ(TB_RETURN_OK, TB2820_SetHallClearEnable(&handler_, 0xAAAA5555U, 0x12345678U));
    EXPECT_EQ(0xAAAA5555U, ReadRegister(TB2820_HALL_CLEAR_ENABLE_OFFSET));
    EXPECT_EQ(0x12345678U, ReadRegister(TB2820_HALL_CLEAR_ENABLE_OFFSET + 4U));

    EXPECT_EQ(TB_RETURN_OK, TB2820_SetPulseClearEnable(&handler_, 0xCAFEBABEU));
    EXPECT_EQ(0xCAFEBABEU, ReadRegister(TB2820_PULSE_CLEAR_ENABLE_OFFSET));

    EXPECT_EQ(TB_RETURN_OK, TB2820_SetStepperClearEnable(&handler_, 0x5AU));
    EXPECT_EQ(0x5AU, ReadRegister(TB2820_MOTOR_STEP_CLEAR_ENABLE_OFFSET));

    EXPECT_EQ(TB_RETURN_OK, TB2820_SetHall3ClearEnable(&handler_, 0x3CU));
    EXPECT_EQ(0x3CU, ReadRegister(TB2820_HALL3_CLEAR_ENABLE_OFFSET));
}

TEST_F(TB2820ApiTest, SetHall3StopTimeoutValidates)
{
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_SetHall3StopTimeout(nullptr, 0U, 0U));
    EXPECT_EQ(TB_ERR_INVALID_CHANNEL, TB2820_SetHall3StopTimeout(&handler_, TB2820_HALL3_GROUP_MAX, 0U));

    TB_Return result = TB2820_SetHall3StopTimeout(&handler_, 2U, 250U);
    EXPECT_EQ(TB_RETURN_OK, result);
    EXPECT_EQ(25U, ReadRegister(TB2820_HALL3_STOP_TIMEOUT_OFFSET + 2U * 4U));
}

TEST_F(TB2820ApiTest, SetTickPeriodAndToleranceValidates)
{
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_SetTickPeriodAndTolerance(nullptr, 56U, 3.0f, 0.1f));
    EXPECT_EQ(TB_ERR_INVALID_CHANNEL, TB2820_SetTickPeriodAndTolerance(&handler_, 30U, 3.0f, 0.1f));
    EXPECT_EQ(TB_ERR_INVALID_ARGUMENTS, TB2820_SetTickPeriodAndTolerance(&handler_, 56U, 2.0f, 0.1f));
    EXPECT_EQ(TB_ERR_INVALID_ARGUMENTS, TB2820_SetTickPeriodAndTolerance(&handler_, 56U, 3.0f, 0.01f));

    TB_Return result = TB2820_SetTickPeriodAndTolerance(&handler_, 57U, 10.0f, 0.1f);
    EXPECT_EQ(TB_RETURN_OK, result);
    EXPECT_EQ(1000U, ReadRegister(TB2820_CHANNEL_TICK_PERIOD_OFFSET + 57U * 4U));
    EXPECT_EQ(100U, ReadRegister(TB2820_CHANNEL_TICK_PERIOD_TOLERANCE_OFFSET + 57U * 4U));
}

TEST_F(TB2820ApiTest, SetNibbleAndLowTickWriteMaskedValues)
{
    WriteRegister(TB2820_CHANNEL_ZERO_NIBBLE_HIGH_TICK_OFFSET, 0xFFFFFFFFU);
    WriteRegister(TB2820_CHANNEL_LOW_TICK_OFFSET, 0xFFFFFFFFU);

    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_SetNibbleHighTick(nullptr, 56U, 0x5U));
    EXPECT_EQ(TB_ERR_INVALID_CHANNEL, TB2820_SetNibbleHighTick(&handler_, 30U, 0x5U));

    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_SetLowTick(nullptr, 56U, 0x5U));
    EXPECT_EQ(TB_ERR_INVALID_CHANNEL, TB2820_SetLowTick(&handler_, 30U, 0x5U));

    TB_Return nibble = TB2820_SetNibbleHighTick(&handler_, 58U, 0xAU);
    EXPECT_EQ(TB_RETURN_OK, nibble);
    EXPECT_EQ(0xFFFFFAFFU, ReadRegister(TB2820_CHANNEL_ZERO_NIBBLE_HIGH_TICK_OFFSET));

    TB_Return low = TB2820_SetLowTick(&handler_, 56U, 0xABU);
    EXPECT_EQ(TB_RETURN_OK, low);
    EXPECT_EQ(0xFFFFFFFBU, ReadRegister(TB2820_CHANNEL_LOW_TICK_OFFSET));
}

TEST_F(TB2820ApiTest, SetSyncHighTickPauseNibblePerMessage)
{
    WriteRegister(TB2820_CHANNEL_SYN_HIGH_TICK_OFFSET, 0xFFFFFFFFU);
    WriteRegister(TB2820_CHANNEL_PAUSE_EN_OFFSET, 0xFFFFFFFFU);
    WriteRegister(TB2820_CHANNEL_NIBBLES_PER_MESSAGE_OFFSET, 0xFFFFFFFFU);

    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_SetSyncHighTick(nullptr, 56U, 0x22U));
    EXPECT_EQ(TB_ERR_INVALID_CHANNEL, TB2820_SetSyncHighTick(&handler_, 40U, 0x22U));

    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_SetPauseEnable(nullptr, 56U, TB2820_ENABLE));
    EXPECT_EQ(TB_ERR_INVALID_ARGUMENTS, TB2820_SetPauseEnable(&handler_, 56U, 3U));

    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_SetNibblePerMessage(nullptr, 56U, 0x2U));

    TB_Return sync = TB2820_SetSyncHighTick(&handler_, 57U, 0x44U);
    EXPECT_EQ(TB_RETURN_OK, sync);
    EXPECT_EQ(0xFFFF44FFU, ReadRegister(TB2820_CHANNEL_SYN_HIGH_TICK_OFFSET));

    TB_Return pause = TB2820_SetPauseEnable(&handler_, 59U, TB2820_ENABLE);
    EXPECT_EQ(TB_RETURN_OK, pause);
    EXPECT_EQ(0xFFFFF80FU, ReadRegister(TB2820_CHANNEL_PAUSE_EN_OFFSET));

    TB_Return nibble = TB2820_SetNibblePerMessage(&handler_, 58U, 0x3U);
    EXPECT_EQ(TB_RETURN_OK, nibble);
    EXPECT_EQ(0xFFFFF3FFU, ReadRegister(TB2820_CHANNEL_NIBBLES_PER_MESSAGE_OFFSET));
}

TEST_F(TB2820ApiTest, SetSentInTimeoutAndPolarity)
{
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_SetSentInTimeout(nullptr, 56U, 1.0f));
    EXPECT_EQ(TB_ERR_INVALID_CHANNEL, TB2820_SetSentInTimeout(&handler_, 40U, 1.0f));

    TB_Return timeout = TB2820_SetSentInTimeout(&handler_, 56U, 1.5f);
    EXPECT_EQ(TB_RETURN_OK, timeout);
    EXPECT_EQ(150000U, ReadRegister(TB2820_CHANNEL_TIME_OUT_OFFSET));

    WriteRegister(TB2820_CHANNEL_POLARITY_OFFSET, 0xFFFFFFFFU);
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_SetPolarity(nullptr, 56U, TB2820_SENTIN_POLARITY_REVERSAL));
    EXPECT_EQ(TB_ERR_INVALID_CHANNEL, TB2820_SetPolarity(&handler_, 40U, TB2820_SENTIN_POLARITY_REVERSAL));
    EXPECT_EQ(TB_ERR_INVALID_ARGUMENTS, TB2820_SetPolarity(&handler_, 56U, 5U));

    TB_Return polarity = TB2820_SetPolarity(&handler_, 56U, TB2820_SENTIN_POLARITY_NONREVERSAL);
    EXPECT_EQ(TB_RETURN_OK, polarity);
    EXPECT_EQ(0xFFFFFFFFU, ReadRegister(TB2820_CHANNEL_POLARITY_OFFSET));
}

TEST_F(TB2820ApiTest, SetCRCImplementationTypeSerDataModeRxEnableClearFault)
{
    WriteRegister(TB2820_CHANNEL_CRC_IMP_TYPE_OFFSET, 0xFFFFFFFFU);
    WriteRegister(TB2820_CHANNEL_SER_DATA_MODE_OFFSET, 0xFFFFFFFFU);
    WriteRegister(TB2820_CHANNEL_RX_EN_OFFSET, 0xFFFFFFFFU);
    WriteRegister(TB2820_CHANNEL_CLEAR_FAULT_OFFSET, 0xFFFFFFFFU);

    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_SetCRCImplementationType(nullptr, 56U, TB2820_SENTIN_LEGACY));
    EXPECT_EQ(TB_ERR_INVALID_CHANNEL, TB2820_SetCRCImplementationType(&handler_, 55U, TB2820_SENTIN_LEGACY));
    EXPECT_EQ(TB_ERR_INVALID_ARGUMENTS, TB2820_SetCRCImplementationType(&handler_, 56U, 5U));

    TB_Return crc = TB2820_SetCRCImplementationType(&handler_, 56U, TB2820_SENTIN_RECOMMANDED);
    EXPECT_EQ(TB_RETURN_OK, crc);
    EXPECT_EQ(0xFFFFFFFFU, ReadRegister(TB2820_CHANNEL_CRC_IMP_TYPE_OFFSET));

    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_SetSerDataMode(nullptr, 56U, 1U));
    EXPECT_EQ(TB_ERR_INVALID_CHANNEL, TB2820_SetSerDataMode(&handler_, 40U, 1U));

    TB_Return ser = TB2820_SetSerDataMode(&handler_, 57U, 2U);
    EXPECT_EQ(TB_RETURN_OK, ser);
    EXPECT_EQ(0xFFFFFFFDU, ReadRegister(TB2820_CHANNEL_SER_DATA_MODE_OFFSET));

    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_SetRxEnable(nullptr, 56U, TB2820_ENABLE));
    EXPECT_EQ(TB_ERR_INVALID_CHANNEL, TB2820_SetRxEnable(&handler_, 40U, TB2820_ENABLE));
    EXPECT_EQ(TB_ERR_INVALID_ARGUMENTS, TB2820_SetRxEnable(&handler_, 56U, 3U));

    TB_Return rx = TB2820_SetRxEnable(&handler_, 58U, TB2820_DISABLE);
    EXPECT_EQ(TB_RETURN_OK, rx);
    EXPECT_EQ(0xFFFFFC03U, ReadRegister(TB2820_CHANNEL_RX_EN_OFFSET));

    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_ClearFault(nullptr, 56U));
    EXPECT_EQ(TB_ERR_INVALID_CHANNEL, TB2820_ClearFault(&handler_, 40U));

    TB_Return clear = TB2820_ClearFault(&handler_, 59U);
    EXPECT_EQ(TB_RETURN_OK, clear);
    EXPECT_EQ(0xFFFFF80FU, ReadRegister(TB2820_CHANNEL_CLEAR_FAULT_OFFSET));
}

TEST_F(TB2820ApiTest, GetDMABufferSizeAggregatesChannelModes)
{
    handler_.info[0U].mode = TB2820_MODE_PWM;
    handler_.info[1U].mode = TB2820_MODE_SENTIN;
    handler_.info[2U].mode = TB2820_MODE_HALL;
    handler_.info[3U].mode = TB2820_MODE_HALL3;
    handler_.info[4U].mode = TB2820_MODE_PHASE2_STEP4;

    size_t size = TB2820_GetDMABufferSize(&handler_);
    EXPECT_EQ(112U, size);
}

TEST_F(TB2820ApiTest, DemuxDMADataRejectsInsufficientBuffer)
{
    handler_.info[0U].mode = TB2820_MODE_PWM;
    size_t offset = 0U;
    TB_U8 buffer[4] = {0};

    TB_Return result = TB2820_DemuxDMAData(&handler_, buffer, &offset, sizeof(buffer), 0U);
    EXPECT_EQ(TB_RETURN_NOT_OK, result);
}

TEST_F(TB2820ApiTest, DemuxDMADataDecodesPWM)
{
    handler_.info[0U].mode = TB2820_MODE_PWM;
    TB_U8 buffer[TB2820_PWM_CHANNEL_SIZE_BYTES] = {0};
    TB_U32 *pwm = reinterpret_cast<TB_U32 *>(buffer);
    pwm[0] = 6U;
    pwm[1] = 14U;
    size_t offset = 0U;

    TB_Return result = TB2820_DemuxDMAData(&handler_, buffer, &offset, sizeof(buffer), 0U);

    EXPECT_EQ(TB_RETURN_OK, result);
    EXPECT_FLOAT_EQ(static_cast<TB_Float>(TB2820_CLOCK_FREQ) / 20.0f, handler_.info[0U].data.modeData.pwm.freq);
    EXPECT_FLOAT_EQ(6.0f / 20.0f, handler_.info[0U].data.modeData.pwm.duty);
    EXPECT_EQ(TB2820_PWM_CHANNEL_SIZE_BYTES, offset);
}

TEST_F(TB2820ApiTest, DemuxDMADataHandlesHall3Group)
{
    handler_.info[3U].mode = TB2820_MODE_HALL3;
    handler_.info[4U].mode = TB2820_MODE_HALL3;
    handler_.info[5U].mode = TB2820_MODE_HALL3;

    HallThreePhaseSignals data = {200U, 90U, {1U, 2U, 3U}, {4U, 5U, 6U}};
    TB_U8 buffer[sizeof(HallThreePhaseSignals)] = {0};
    std::memcpy(buffer, &data, sizeof(HallThreePhaseSignals));
    size_t offset = 0U;

    TB_Return result = TB2820_DemuxDMAData(&handler_, buffer, &offset, sizeof(buffer), 3U);

    EXPECT_EQ(TB_RETURN_OK, result);
    EXPECT_EQ(sizeof(HallThreePhaseSignals), offset);
    EXPECT_EQ(5000000U, handler_.info[3U].data.modeData.threePhaseHall.acutalRPM);
    EXPECT_EQ(handler_.info[3U].data.modeData.threePhaseHall.realTimeAngle, handler_.info[4U].data.modeData.threePhaseHall.realTimeAngle);
    EXPECT_EQ(handler_.info[3U].data.modeData.threePhaseHall.realTimeAngle, handler_.info[5U].data.modeData.threePhaseHall.realTimeAngle);
}
TEST_F(TB2820ApiTest, ReadDMAReportsFailureWhenReadReturnsError)
{
    handler_.dmaFd = 5;
    g_readStub.SetStub(ReadStubFailure);
    TB_U8 buffer[8] = {0};

    TB_Return result = TB2820_ReadDMA(&handler_, buffer, sizeof(buffer));

    EXPECT_EQ(TB2820_ERR_READ_DMA_FAILED, result);
    EXPECT_EQ(1U, g_readStub.GetCallCount());
}

TEST_F(TB2820ApiTest, ReadDMACopiesDataOnSuccess)
{
    handler_.dmaFd = 5;
    g_readStub.SetStub(ReadStubSuccess);
    TB_U8 buffer[4] = {0};

    TB_Return result = TB2820_ReadDMA(&handler_, buffer, sizeof(buffer));

    ASSERT_EQ(TB_RETURN_OK, result);
    EXPECT_EQ(1U, g_readStub.GetCallCount());
    for (size_t i = 0; i < sizeof(buffer); ++i)
    {
        EXPECT_EQ(0xAB, buffer[i]);
    }
}


TEST_F(TB2820ApiTest, GetDataRejectsNullHandler)
{
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_GetData(nullptr));
}

TEST_F(TB2820ApiTest, GetDataPropagatesReadFailures)
{
    handler_.dmaFd = 5;
    g_lseekStub.SetStub(LseekStubIgnore);
    g_readStub.SetStub(ReadStubFailure);

    TB_Return result = TB2820_GetData(&handler_);

    EXPECT_EQ(TB2820_ERR_READ_DMA_FAILED, result);
    EXPECT_EQ(1U, g_readStub.GetCallCount());
}

TEST_F(TB2820ApiTest, GetDataDemuxesAndPopulatesHandlerState)
{
    handler_.dmaFd = 0;
    g_lseekStub.SetStub(LseekStubIgnore);

    handler_.pwmChannelCount = 1U;
    handler_.sentChannelCount = 1U;
    handler_.hallChannelCount = 1U;
    handler_.hall3ChannelCount = 3U;
    handler_.pulseChannelCount = 1U;
    handler_.stepper24ChannelCount = 4U;
    handler_.stepper28ChannelCount = 0U;
    handler_.stepper44ChannelCount = 4U;
    handler_.stepper444ChannelCount = 0U;
    handler_.stepper48ChannelCount = 0U;

    handler_.info[0U].mode = TB2820_MODE_PWM;
    handler_.info[1U].mode = TB2820_MODE_SENTIN;
    handler_.info[2U].mode = TB2820_MODE_HALL;
    handler_.info[3U].mode = TB2820_MODE_HALL3;
    handler_.info[4U].mode = TB2820_MODE_HALL3;
    handler_.info[5U].mode = TB2820_MODE_HALL3;
    handler_.info[6U].mode = TB2820_MODE_PULSE;
    handler_.info[8U].mode = TB2820_MODE_PHASE2_STEP4;
    handler_.info[9U].mode = TB2820_MODE_PHASE2_STEP4;
    handler_.info[10U].mode = TB2820_MODE_PHASE2_STEP4;
    handler_.info[11U].mode = TB2820_MODE_PHASE2_STEP4;
    handler_.info[12U].mode = TB2820_MODE_PHASE4_SINGLE_STEP4;
    handler_.info[13U].mode = TB2820_MODE_PHASE4_SINGLE_STEP4;
    handler_.info[14U].mode = TB2820_MODE_PHASE4_SINGLE_STEP4;
    handler_.info[15U].mode = TB2820_MODE_PHASE4_SINGLE_STEP4;

    size_t bufferSize = TB2820_GetDMABufferSize(&handler_);
    g_dmaStubLength = bufferSize;
    std::memset(g_dmaStubBuffer, 0, bufferSize);

    StoreWord(g_dmaStubBuffer, 0U, 0xAAAAAAAAU);
    StoreWord(g_dmaStubBuffer, 4U, 0x55555555U);
    StoreWord(g_dmaStubBuffer, 8U, 0x11111111U);
    StoreWord(g_dmaStubBuffer, 12U, 0x22222222U);
    StoreWord(g_dmaStubBuffer, 16U, 0x33333333U);
    StoreWord(g_dmaStubBuffer, 20U, 0x44444444U);
    StoreWord(g_dmaStubBuffer, 24U, 0x55556666U);
    StoreWord(g_dmaStubBuffer, 28U, 0x77778888U);

    StoreWord(g_dmaStubBuffer, 0x20U, 2U);
    StoreWord(g_dmaStubBuffer, 0x24U, 3U);

    StoreWord(g_dmaStubBuffer, 0x28U, 1U);
    StoreWord(g_dmaStubBuffer, 0x2CU, 2U);
    StoreWord(g_dmaStubBuffer, 0x30U, 3U);
    StoreWord(g_dmaStubBuffer, 0x34U, 4U);
    StoreWord(g_dmaStubBuffer, 0x38U, 5U);

    StoreWord(g_dmaStubBuffer, 0x3CU, 10U);
    StoreWord(g_dmaStubBuffer, 0x40U, 11U);
    StoreWord(g_dmaStubBuffer, 0x44U, 12U);

    StoreWord(g_dmaStubBuffer, 0x48U, 50U);
    StoreWord(g_dmaStubBuffer, 0x4CU, 60U);
    StoreWord(g_dmaStubBuffer, 0x50U, 1U);
    StoreWord(g_dmaStubBuffer, 0x54U, 2U);
    StoreWord(g_dmaStubBuffer, 0x58U, 3U);
    StoreWord(g_dmaStubBuffer, 0x5CU, 4U);
    StoreWord(g_dmaStubBuffer, 0x60U, 5U);
    StoreWord(g_dmaStubBuffer, 0x64U, 6U);

    StoreWord(g_dmaStubBuffer, 0x68U, 20U);
    StoreWord(g_dmaStubBuffer, 0x6CU, 21U);
    StoreWord(g_dmaStubBuffer, 0x70U, 22U);

    StoreWord(g_dmaStubBuffer, 0x74U, 30U);
    StoreWord(g_dmaStubBuffer, 0x78U, 31U);

    StoreWord(g_dmaStubBuffer, 0x7CU, 40U);
    StoreWord(g_dmaStubBuffer, 0x80U, 41U);

    g_readStub.SetStub(ReadStubCopyPreloaded);

    TB_Return result = TB2820_GetData(&handler_);

    ASSERT_EQ(TB_RETURN_OK, result);
    EXPECT_EQ(0xAAAAAAAAU, handler_.diValues[0]);
    EXPECT_EQ(0x55555555U, handler_.diValues[1]);
    EXPECT_EQ(0x11111111U, handler_.singleHallStatus[0]);
    EXPECT_EQ(0x22222222U, handler_.singleHallStatus[1]);
    EXPECT_EQ(0x33333333U, handler_.threePhaseHallDirection);
    EXPECT_EQ(0x44444444U, handler_.singlePulseStatus);
    EXPECT_EQ(0x55556666U, handler_.twoPhaseStepperStatus);
    EXPECT_EQ(0x77778888U, handler_.fourPhaseStepperStatus);

    const TB_Float expectedFrequency = static_cast<TB_Float>(TB2820_CLOCK_FREQ) / 5.0f;
    EXPECT_NEAR(expectedFrequency, handler_.info[0U].data.modeData.pwm.freq, 1e-3f);
    EXPECT_NEAR(0.4f, handler_.info[0U].data.modeData.pwm.duty, 1e-6f);

    EXPECT_EQ(1U, handler_.info[1U].data.modeData.sentIn.actualTickPeriodTime);
    EXPECT_EQ(2U, handler_.info[1U].data.modeData.sentIn.allTicks);
    EXPECT_EQ(3U, handler_.info[1U].data.modeData.sentIn.diagnostic);
    EXPECT_EQ(4U, handler_.info[1U].data.modeData.sentIn.nibbleData);
    EXPECT_EQ(5U, handler_.info[1U].data.modeData.sentIn.serialData);

    EXPECT_EQ(10U, handler_.info[2U].data.modeData.singleHall.totalCount);
    EXPECT_EQ(11U, handler_.info[2U].data.modeData.singleHall.risingEdgeCount);
    EXPECT_EQ(12U, handler_.info[2U].data.modeData.singleHall.fallingEdgeCount);

    EXPECT_EQ(20000000U, handler_.info[3U].data.modeData.threePhaseHall.acutalRPM);
    EXPECT_EQ(60U, handler_.info[3U].data.modeData.threePhaseHall.realTimeAngle);
    EXPECT_EQ(1U, handler_.info[3U].data.modeData.threePhaseHall.risingEdgeCount[0]);
    EXPECT_EQ(2U, handler_.info[3U].data.modeData.threePhaseHall.risingEdgeCount[1]);
    EXPECT_EQ(3U, handler_.info[3U].data.modeData.threePhaseHall.risingEdgeCount[2]);
    EXPECT_EQ(4U, handler_.info[3U].data.modeData.threePhaseHall.fallingEdgeCount[0]);
    EXPECT_EQ(5U, handler_.info[3U].data.modeData.threePhaseHall.fallingEdgeCount[1]);
    EXPECT_EQ(6U, handler_.info[3U].data.modeData.threePhaseHall.fallingEdgeCount[2]);
    EXPECT_EQ(handler_.info[3U].data.modeData.threePhaseHall.acutalRPM,
              handler_.info[4U].data.modeData.threePhaseHall.acutalRPM);
    EXPECT_EQ(handler_.info[3U].data.modeData.threePhaseHall.acutalRPM,
              handler_.info[5U].data.modeData.threePhaseHall.acutalRPM);

    EXPECT_EQ(20U, handler_.info[6U].data.modeData.singlePulse.totalCount);
    EXPECT_EQ(21U, handler_.info[6U].data.modeData.singlePulse.risingEdgeCount);
    EXPECT_EQ(22U, handler_.info[6U].data.modeData.singlePulse.fallingEdgeCount);

    EXPECT_EQ(30U, handler_.info[8U].data.modeData.motor2Phase.stepCount);
    EXPECT_EQ(31U, handler_.info[8U].data.modeData.motor2Phase.pulseCount);
    EXPECT_EQ(30U, handler_.info[11U].data.modeData.motor2Phase.stepCount);

    EXPECT_EQ(40U, handler_.info[12U].data.modeData.motor4Phase.stepCount);
    EXPECT_EQ(41U, handler_.info[12U].data.modeData.motor4Phase.pulseCount);
    EXPECT_EQ(40U, handler_.info[15U].data.modeData.motor4Phase.stepCount);

    EXPECT_EQ(1U, g_readStub.GetCallCount());
}



TEST_F(TB2820ApiTest, GetDIValueHandlesRanges)
{
    handler_.diValues[0] = 0xAAAAAAAAU;
    handler_.diValues[1] = 0x55555555U;
    TB_U32 value = 0U;

    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_GetDIValue(nullptr, 0U, &value));
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_GetDIValue(&handler_, 0U, nullptr));

    EXPECT_EQ(TB_RETURN_OK, TB2820_GetDIValue(&handler_, 1U, &value));
    EXPECT_EQ(1U, value);

    EXPECT_EQ(TB_RETURN_OK, TB2820_GetDIValue(&handler_, 33U, &value));
    EXPECT_EQ(0U, value);

    EXPECT_EQ(TB_ERR_INVALID_CHANNEL, TB2820_GetDIValue(&handler_, 80U, &value));
}

TEST_F(TB2820ApiTest, GetPWMFreqAndDutyValidations)
{
    handler_.info[5U].mode = TB2820_MODE_PWM;
    handler_.info[5U].data.modeData.pwm.freq = 1000.0f;
    handler_.info[5U].data.modeData.pwm.duty = 0.25f;

    TB_Float freq = 0.0f;
    TB_Float duty = 0.0f;

    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_GetPWMFreqAndDuty(nullptr, 5U, &freq, &duty));
    EXPECT_EQ(TB_ERR_INVALID_ARGUMENTS, TB2820_GetPWMFreqAndDuty(&handler_, 5U, nullptr, &duty));
    EXPECT_EQ(TB_ERR_INVALID_ARGUMENTS, TB2820_GetPWMFreqAndDuty(&handler_, 5U, &freq, nullptr));
    EXPECT_EQ(TB_ERR_INVALID_CHANNEL, TB2820_GetPWMFreqAndDuty(&handler_, TB2820_CHANNEL_MAX, &freq, &duty));

    handler_.info[4U].mode = TB2820_MODE_DI;
    EXPECT_EQ(TB2820_ERR_INVALID_CHANNEL_MODE, TB2820_GetPWMFreqAndDuty(&handler_, 4U, &freq, &duty));

    EXPECT_EQ(TB_RETURN_OK, TB2820_GetPWMFreqAndDuty(&handler_, 5U, &freq, &duty));
    EXPECT_FLOAT_EQ(1000.0f, freq);
    EXPECT_FLOAT_EQ(0.25f, duty);
}

TEST_F(TB2820ApiTest, HallSignalAccessors)
{
    handler_.info[3U].mode = TB2820_MODE_HALL;
    handler_.info[3U].data.modeData.singleHall.totalCount = 100U;
    handler_.info[3U].data.modeData.singleHall.risingEdgeCount = 10U;
    handler_.info[3U].data.modeData.singleHall.fallingEdgeCount = 20U;

    TB_U32 value = 0U;
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_GetHallSignalCount(nullptr, 3U, &value));
    EXPECT_EQ(TB_ERR_INVALID_CHANNEL, TB2820_GetHallSignalCount(&handler_, TB2820_HALL_CHANNEL_MAX, &value));
    handler_.info[3U].mode = TB2820_MODE_DI;
    EXPECT_EQ(TB_ERR_INVALID_ARGUMENTS, TB2820_GetHallSignalCount(&handler_, 3U, &value));

    handler_.info[3U].mode = TB2820_MODE_HALL;
    EXPECT_EQ(TB_RETURN_OK, TB2820_GetHallSignalCount(&handler_, 3U, &value));
    EXPECT_EQ(100U, value);

    EXPECT_EQ(TB_RETURN_OK, TB2820_GetHallRisingEdgeCount(&handler_, 3U, &value));
    EXPECT_EQ(10U, value);

    EXPECT_EQ(TB_RETURN_OK, TB2820_GetHallFallingEdgeCount(&handler_, 3U, &value));
    EXPECT_EQ(20U, value);
}

TEST_F(TB2820ApiTest, Hall3Metrics)
{
    handler_.info[6U].mode = TB2820_MODE_HALL3;
    handler_.info[6U].data.modeData.threePhaseHall.acutalRPM = 1000U;
    handler_.info[6U].data.modeData.threePhaseHall.realTimeAngle = 30U;
    handler_.info[6U].data.modeData.threePhaseHall.risingEdgeCount[0] = 7U;
    handler_.info[6U].data.modeData.threePhaseHall.fallingEdgeCount[0] = 8U;
    handler_.threePhaseHallDirection = 0x20U;

    TB_U32 value = 0U;
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_GetHall3MotorSpeed(nullptr, 2U, &value));
    EXPECT_EQ(TB_ERR_INVALID_CHANNEL, TB2820_GetHall3MotorSpeed(&handler_, TB2820_HALL3_GROUP_MAX, &value));
    handler_.info[6U].mode = TB2820_MODE_DI;
    EXPECT_EQ(TB_ERR_INVALID_ARGUMENTS, TB2820_GetHall3MotorSpeed(&handler_, 2U, &value));

    handler_.info[6U].mode = TB2820_MODE_HALL3;
    EXPECT_EQ(TB_RETURN_OK, TB2820_GetHall3MotorSpeed(&handler_, 2U, &value));
    EXPECT_EQ(1000U, value);

    EXPECT_EQ(TB_RETURN_OK, TB2820_GetHall3RealTimeAngle(&handler_, 2U, &value));
    EXPECT_EQ(30U, value);

    EXPECT_EQ(TB_RETURN_OK, TB2820_GetHall3Direction(&handler_, 2U, &value));
    EXPECT_EQ(0U, value);

    EXPECT_EQ(TB_RETURN_OK, TB2820_GetHall3SignalStatus(&handler_, 6U, &value));
    EXPECT_EQ(0U, value);

    handler_.singleHallStatus[0] = (1U << 6U);
    EXPECT_EQ(TB_RETURN_OK, TB2820_GetHall3SignalStatus(&handler_, 6U, &value));
    EXPECT_EQ(1U << 6U, value);

    handler_.info[6U].data.modeData.threePhaseHall.risingEdgeCount[0] = 11U;
    handler_.info[6U].data.modeData.threePhaseHall.fallingEdgeCount[0] = 22U;
    EXPECT_EQ(TB_RETURN_OK, TB2820_GetHall3RisingEdgeCount(&handler_, 6U, &value));
    EXPECT_EQ(11U, value);
    EXPECT_EQ(TB_RETURN_OK, TB2820_GetHall3FallingEdgeCount(&handler_, 6U, &value));
    EXPECT_EQ(22U, value);
}

TEST_F(TB2820ApiTest, HallCommonSignalStatus)
{
    handler_.info[5U].mode = TB2820_MODE_HALL;
    handler_.singleHallStatus[0] = (1U << 5U);
    TB_U32 status = 0U;
    EXPECT_EQ(TB_RETURN_OK, TB2820_GetHallCommonSignalStatus(&handler_, 5U, &status));
    EXPECT_EQ(1U << 5U, status);
}

TEST_F(TB2820ApiTest, PulseMetrics)
{
    handler_.info[36U].mode = TB2820_MODE_PULSE;
    handler_.info[37U].mode = TB2820_MODE_PULSE;
    handler_.info[38U].mode = TB2820_MODE_PULSE;
    handler_.info[36U].data.modeData.singlePulse.totalCount = 200U;
    handler_.info[36U].data.modeData.singlePulse.risingEdgeCount = 30U;
    handler_.info[36U].data.modeData.singlePulse.fallingEdgeCount = 40U;
    handler_.singlePulseStatus = 0x4U;

    TB_U32 value = 0U;
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_GetPluseSignalCount(nullptr, 36U, &value));
    EXPECT_EQ(TB_ERR_INVALID_CHANNEL, TB2820_GetPluseSignalCount(&handler_, 20U, &value));
    handler_.info[36U].mode = TB2820_MODE_DI;
    EXPECT_EQ(TB_ERR_INVALID_ARGUMENTS, TB2820_GetPluseSignalCount(&handler_, 36U, &value));

    handler_.info[36U].mode = TB2820_MODE_PULSE;
    EXPECT_EQ(TB_RETURN_OK, TB2820_GetPluseSignalCount(&handler_, 36U, &value));
    EXPECT_EQ(200U, value);
    EXPECT_EQ(TB_RETURN_OK, TB2820_GetPulseRisingEdgeCount(&handler_, 36U, &value));
    EXPECT_EQ(30U, value);
    EXPECT_EQ(TB_RETURN_OK, TB2820_GetPulseFallingEdgeCount(&handler_, 36U, &value));
    EXPECT_EQ(40U, value);
    EXPECT_EQ(TB_RETURN_OK, TB2820_GetPulseSignalStatus(&handler_, 38U, &value));
    EXPECT_EQ((handler_.singlePulseStatus >> 2U) & 1U, value);
}

TEST_F(TB2820ApiTest, StepperMetrics)
{
    const TB_U32 base = TB2820_PULSE_CHANNEL_MIN + 4U;
    handler_.info[base].mode = TB2820_MODE_PHASE2_STEP4;
    handler_.info[base].data.modeData.motor2Phase.stepCount = 500U;
    handler_.info[base].data.modeData.motor2Phase.pulseCount = 20U;
    handler_.twoPhaseStepperStatus = 0x00F0U;

    TB_U32 value = 0U;
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_GetStepper2Steps(nullptr, 1U, &value));
    EXPECT_EQ(TB_ERR_INVALID_CHANNEL, TB2820_GetStepper2Steps(&handler_, TB2820_MOTOR_GROUP_MAX, &value));
    handler_.info[base].mode = TB2820_MODE_DI;
    EXPECT_EQ(TB_ERR_INVALID_ARGUMENTS, TB2820_GetStepper2Steps(&handler_, 1U, &value));

    handler_.info[base].mode = TB2820_MODE_PHASE2_STEP8;
    handler_.info[base].data.modeData.motor2Phase.stepCount = 500U;
    handler_.info[base].data.modeData.motor2Phase.pulseCount = 20U;
    EXPECT_EQ(TB_RETURN_OK, TB2820_GetStepper2Steps(&handler_, 1U, &value));
    EXPECT_EQ(500U, value);
    EXPECT_EQ(TB_RETURN_OK, TB2820_GetStepper2Phase(&handler_, 1U, &value));
    EXPECT_EQ(20U, value);
    EXPECT_EQ(TB_RETURN_OK, TB2820_GetStep2SignalStatus(&handler_, 1U, &value));
    EXPECT_EQ(0x0FU, value);

    const TB_U32 base4 = TB2820_PULSE_CHANNEL_MIN + 8U;
    handler_.info[base4].mode = TB2820_MODE_PHASE4_STEP8;
    handler_.info[base4].data.modeData.motor4Phase.stepCount = 700U;
    handler_.info[base4].data.modeData.motor4Phase.pulseCount = 35U;
    handler_.fourPhaseStepperStatus = 0x00000F00U;

    EXPECT_EQ(TB_RETURN_OK, TB2820_GetStepper4Steps(&handler_, 2U, &value));
    EXPECT_EQ(700U, value);
    EXPECT_EQ(TB_RETURN_OK, TB2820_GetStepper4Phase(&handler_, 2U, &value));
    EXPECT_EQ(35U, value);
    EXPECT_EQ(TB_RETURN_OK, TB2820_GetStep4SignalStatus(&handler_, 2U, &value));
    EXPECT_EQ(0xFU, value);
}

TEST_F(TB2820ApiTest, ActualTickPeriodAndAllTicks)
{
    handler_.info[56U].mode = TB2820_MODE_SENTIN;
    handler_.info[56U].data.modeData.sentIn.actualTickPeriodTime = 1000U;
    handler_.info[56U].data.modeData.sentIn.allTicks = 5000U;
    WriteRegister(TB2820_CHANNEL_SYN_HIGH_TICK_OFFSET, 0x00000005U);

    TB_Float tickPeriod = 0.0f;
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_GetActualTickPeriodTime(nullptr, 56U, &tickPeriod));
    EXPECT_EQ(TB_ERR_INVALID_CHANNEL, TB2820_GetActualTickPeriodTime(&handler_, 30U, &tickPeriod));
    handler_.info[56U].mode = TB2820_MODE_DI;
    EXPECT_EQ(TB_ERR_INVALID_ARGUMENTS, TB2820_GetActualTickPeriodTime(&handler_, 56U, &tickPeriod));
    handler_.info[56U].mode = TB2820_MODE_SENTIN;
    WriteRegister(TB2820_CHANNEL_SYN_HIGH_TICK_OFFSET, 0U);
    EXPECT_EQ(TB_ERR_INVALID_ARGUMENTS, TB2820_GetActualTickPeriodTime(&handler_, 56U, &tickPeriod));

    WriteRegister(TB2820_CHANNEL_SYN_HIGH_TICK_OFFSET, 0x00000005U);
    EXPECT_EQ(TB_RETURN_OK, TB2820_GetActualTickPeriodTime(&handler_, 56U, &tickPeriod));
    EXPECT_FLOAT_EQ(2.0f, tickPeriod);

    TB_U32 allTicks = 0U;
    WriteRegister(TB2820_CHANNEL_SYN_HIGH_TICK_OFFSET, 0x00000005U);
    EXPECT_EQ(TB_RETURN_OK, TB2820_GetAllTicks(&handler_, 56U, &allTicks));
    EXPECT_EQ(1U, allTicks);

    handler_.info[56U].data.modeData.sentIn.actualTickPeriodTime = 0U;
    EXPECT_EQ(TB_ERR_INVALID_ARGUMENTS, TB2820_GetAllTicks(&handler_, 56U, &allTicks));
}

TEST_F(TB2820ApiTest, SentInDataRetrieval)
{
    handler_.info[56U].mode = TB2820_MODE_SENTIN;
    handler_.info[56U].data.modeData.sentIn.actualTickPeriodTime = 123U;
    handler_.info[56U].data.modeData.sentIn.allTicks = 456U;
    handler_.info[56U].data.modeData.sentIn.diagnostic = 789U;
    handler_.info[56U].data.modeData.sentIn.nibbleData = 0xABU;
    handler_.info[56U].data.modeData.sentIn.serialData = 0xCDU;

    TB_Float tickPeriod = 0.0f;
    TB_U32 value = 0U;

    EXPECT_EQ(TB_RETURN_OK, TB2820_GetNibbleData(&handler_, 56U, &value));
    EXPECT_EQ(0xABU, value);

    EXPECT_EQ(TB_RETURN_OK, TB2820_GetDiagnosticStatus(&handler_, 56U, &value));
    EXPECT_EQ(789U, value);

    EXPECT_EQ(TB_RETURN_OK, TB2820_GetSerialData(&handler_, 56U, &value));
    EXPECT_EQ(0xCDU, value);
}

TEST_F(TB2820ApiTest, VoltageAndTemperatureConversions)
{
    EXPECT_NEAR(1.5f, ConvertRawToVoltage(32768U), 1e-6f);
    EXPECT_NEAR(-273.15f + 503.975f, ConvertRawToTemperature(65536U), 1e-3f);
}

TEST(TB2820ApiStandalone, PrintVersionWritesToStderr)
{
    StderrCapture capture;
    TB2820_PrintVersion();
    const std::string output = capture.GetOutput();
    EXPECT_TRUE(output.find("libtb2820 version") != std::string::npos);
    EXPECT_TRUE(output.find("libtb2820 support firmware version") != std::string::npos);
}

TEST_F(TB2820ApiTest, GetHardwareVersionValidatesAndReads)
{
    TB_Version version = {0U, 0U, 0U};
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_GetHardwareVersion(nullptr, &version));
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_GetHardwareVersion(&handler_, nullptr));

    WriteRegister(TB2820_BOARDINFO_HARDWARE_VERSION_OFFSET, 0x1234U);
    EXPECT_EQ(TB_RETURN_OK, TB2820_GetHardwareVersion(&handler_, &version));
    EXPECT_EQ(0x12U, version.major);
    EXPECT_EQ(0x34U, version.minor);
}

TEST_F(TB2820ApiTest, GetFirmwareVersionValidatesAndReads)
{
    TB_Version version = {0U, 0U, 0U};
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_GetFirmwareVersion(nullptr, &version));
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_GetFirmwareVersion(&handler_, nullptr));

    WriteRegister(TB2820_BOARDINFO_FIRMWARE_VERSION_OFFSET, 0x00112233U);
    EXPECT_EQ(TB_RETURN_OK, TB2820_GetFirmwareVersion(&handler_, &version));
    EXPECT_EQ(0x11U, version.major);
    EXPECT_EQ(0x22U, version.minor);
    EXPECT_EQ(0x33U, version.revision);
}

TEST_F(TB2820ApiTest, GetFlashBootInfoValidatesAndExtracts)
{
    TB_U32 version = 0U;
    TB_U32 status = 99U;

    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_GetFlashBootInfo(nullptr, &version, &status));
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_GetFlashBootInfo(&handler_, nullptr, &status));

    WriteRegister(TB2820_BOARDINFO_FLASH_BOOT_REGISTER_OFFSET, 0x00543200U);
    EXPECT_EQ(TB_RETURN_OK, TB2820_GetFlashBootInfo(&handler_, &version, &status));
    EXPECT_EQ(0x5432U, version);
    EXPECT_EQ(static_cast<TB_U32>(TB2820_UPDATING_BOOTLOADER), status);

    WriteRegister(TB2820_BOARDINFO_FLASH_BOOT_REGISTER_OFFSET, 0x00765401U);
    status = 0U;
    EXPECT_EQ(TB_RETURN_OK, TB2820_GetFlashBootInfo(&handler_, &version, &status));
    EXPECT_EQ(0x7654U, version);
    EXPECT_EQ(static_cast<TB_U32>(TB2820_UPDATING_APPLICATION), status);
}

TEST_F(TB2820ApiTest, GetDeviceIdValidatesAndReads)
{
    unsigned int deviceId = 0U;
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_GetDeviceID(nullptr, &deviceId));
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_GetDeviceID(&handler_, nullptr));

    WriteRegister(TB2820_BOARDINFO_DEVICE_ID_REGISTER_OFFSET, 0x00001234U);
    EXPECT_EQ(TB_RETURN_OK, TB2820_GetDeviceID(&handler_, &deviceId));
    EXPECT_EQ(0x1234U, deviceId);
}

TEST_F(TB2820ApiTest, SetSoftResetValidatesAndWrites)
{
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_SetSoftReset(nullptr, TB2820_RESET));

    WriteRegister(TB2820_BOARDINFO_SOFT_RESET_REGISTER_OFFSET, 0U);
    EXPECT_EQ(TB_RETURN_OK, TB2820_SetSoftReset(&handler_, TB2820_RESET));
    EXPECT_EQ(static_cast<TB_U32>(TB2820_RESET), ReadRegister(TB2820_BOARDINFO_SOFT_RESET_REGISTER_OFFSET));

    EXPECT_EQ(TB_RETURN_OK, TB2820_SetSoftReset(&handler_, 0xFFU));
    EXPECT_EQ(static_cast<TB_U32>(TB2820_RESET), ReadRegister(TB2820_BOARDINFO_SOFT_RESET_REGISTER_OFFSET));

    EXPECT_EQ(TB_RETURN_OK, TB2820_SetSoftReset(&handler_, TB2820_NO_RESET));
    EXPECT_EQ(static_cast<TB_U32>(TB2820_NO_RESET), ReadRegister(TB2820_BOARDINFO_SOFT_RESET_REGISTER_OFFSET));
}

TEST_F(TB2820ApiTest, GetLedStatusValidatesAndReads)
{
    unsigned int ledStatus = 0U;
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_GetLEDStatus(nullptr, &ledStatus));
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_GetLEDStatus(&handler_, nullptr));

    WriteRegister(TB2820_BOARDINFO_LED_CONTROL_REGISTER_OFFSET, 0xABU);
    EXPECT_EQ(TB_RETURN_OK, TB2820_GetLEDStatus(&handler_, &ledStatus));
    EXPECT_EQ(0x1U, ledStatus);

    WriteRegister(TB2820_BOARDINFO_LED_CONTROL_REGISTER_OFFSET, 0U);
    EXPECT_EQ(TB_RETURN_OK, TB2820_GetLEDStatus(&handler_, &ledStatus));
    EXPECT_EQ(0U, ledStatus);
}

TEST_F(TB2820ApiTest, SetLedControlValidatesAndMasks)
{
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_SetLEDControl(nullptr, 1U));

    WriteRegister(TB2820_BOARDINFO_LED_CONTROL_REGISTER_OFFSET, 0U);
    EXPECT_EQ(TB_RETURN_OK, TB2820_SetLEDControl(&handler_, 3U));
    EXPECT_EQ(1U, ReadRegister(TB2820_BOARDINFO_LED_CONTROL_REGISTER_OFFSET));

    EXPECT_EQ(TB_RETURN_OK, TB2820_SetLEDControl(&handler_, 0U));
    EXPECT_EQ(0U, ReadRegister(TB2820_BOARDINFO_LED_CONTROL_REGISTER_OFFSET));
}

TEST_F(TB2820ApiTest, FPGAVoltageAndTemperatureAccessors)
{
    WriteRegister(TB2820_BOARDINFO_FPGA_STATUS_REGISTER_OFFSET, (32768U << 16) | 20000U);
    WriteRegister(TB2820_BOARDINFO_FPGA_STATUS_REGISTER_OFFSET + 4U, (16384U << 16) | 10000U);

    TB_Float value = 0.0f;
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_GetFPGACoreVoltage(nullptr, &value));
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_GetFPGATemperature(nullptr, &value));
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_GetFPGABRAMVoltage(nullptr, &value));
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_GetFPGAAuxVoltage(nullptr, &value));

    EXPECT_EQ(TB_RETURN_OK, TB2820_GetFPGACoreVoltage(&handler_, &value));
    EXPECT_NEAR(ConvertRawToVoltage(32768U), value, 1e-6f);

    EXPECT_EQ(TB_RETURN_OK, TB2820_GetFPGATemperature(&handler_, &value));
    EXPECT_NEAR(ConvertRawToTemperature(20000U), value, 1e-3f);

    EXPECT_EQ(TB_RETURN_OK, TB2820_GetFPGABRAMVoltage(&handler_, &value));
    EXPECT_NEAR(ConvertRawToVoltage(16384U), value, 1e-6f);

    EXPECT_EQ(TB_RETURN_OK, TB2820_GetFPGAAuxVoltage(&handler_, &value));
    EXPECT_NEAR(ConvertRawToVoltage(10000U), value, 1e-6f);
}

TEST_F(TB2820ApiTest, GetBoardPCBASerialValidates)
{
    const size_t serialLength = 12U;
    TB_U8 buffer[serialLength] = {0};
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_GetBoardPCBASerial(nullptr, buffer, serialLength));
    EXPECT_EQ(TB_ERR_INVALID_ARGUMENTS, TB2820_GetBoardPCBASerial(&handler_, nullptr, serialLength));
    EXPECT_EQ(TB_ERR_INVALID_ARGUMENTS, TB2820_GetBoardPCBASerial(&handler_, buffer, 4U));

    WriteRegister(TB2820_BOARDINFO_PCBA_SN_REGISTER_OFFSET, 0x11223344U);
    WriteRegister(TB2820_BOARDINFO_PCBA_SN_REGISTER_OFFSET + 4U, 0x55667788U);
    WriteRegister(TB2820_BOARDINFO_PCBA_SN_REGISTER_OFFSET + 8U, 0x99AABBCCU);

    EXPECT_EQ(TB_RETURN_OK, TB2820_GetBoardPCBASerial(&handler_, buffer, serialLength));
    TB_U32 part = 0U;
    std::memcpy(&part, buffer, sizeof(TB_U32));
    EXPECT_EQ(0x11223344U, part);
    std::memcpy(&part, buffer + sizeof(TB_U32), sizeof(TB_U32));
    EXPECT_EQ(0x55667788U, part);
    std::memcpy(&part, buffer + 2U * sizeof(TB_U32), sizeof(TB_U32));
    EXPECT_EQ(0x99AABBCCU, part);
}

TEST_F(TB2820ApiTest, GetCalibrationInfoDecodesBits)
{
    EXPECT_EQ(TB_ERR_INVALID_HANDLER, TB2820_GetCalibrationInfo(nullptr, nullptr, nullptr, nullptr));

    TB2820CalibrationStatus status = TB2820_CAL_STATUS_DEFAULT;
    TB_U32 loaded = 0U;
    TB_U32 required = 0U;

    WriteRegister(TB2820_BOARDINFO_CALIBRATION_REGISTER_OFFSET, 0xAB1U);

    EXPECT_EQ(TB_RETURN_OK, TB2820_GetCalibrationInfo(&handler_, &status, &loaded, &required));
    EXPECT_EQ(static_cast<TB2820CalibrationStatus>(0x0AU), status);
    EXPECT_EQ(0x1U, loaded);
    EXPECT_EQ(0x1U, required);
}
