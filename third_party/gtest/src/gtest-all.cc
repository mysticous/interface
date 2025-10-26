#include "gtest/gtest.h"

#include <cstdlib>
#include <iomanip>
#include <memory>

namespace testing {
namespace internal {

UnitTestImpl::UnitTestImpl() : failedTests_(0), fatalFailure_(false), currentFailed_(false)
{
}

UnitTestImpl &UnitTestImpl::GetInstance()
{
    static UnitTestImpl instance;
    return instance;
}

void UnitTestImpl::AddTest(const std::string &caseName, const std::string &testName, Test *(*factory)())
{
    TestInfo info;
    info.testCaseName = caseName;
    info.testName = testName;
    info.factory = factory;
    tests_.push_back(info);
}

void UnitTestImpl::ResetCurrentTestState()
{
    fatalFailure_ = false;
    currentFailed_ = false;
}

bool UnitTestImpl::HasFatalFailure() const
{
    return fatalFailure_;
}

bool UnitTestImpl::CurrentTestFailed() const
{
    return currentFailed_;
}

void UnitTestImpl::AddFailure(const char *file, int line, const std::string &message, bool fatal)
{
    currentFailed_ = true;
    if (fatal)
    {
        fatalFailure_ = true;
    }
    std::cerr << file << ":" << line << ": Failure\n" << message << std::endl;
}

int UnitTestImpl::RunAllTests()
{
    std::cout << "[==========] Running " << tests_.size() << " tests." << std::endl;
    for (std::vector<TestInfo>::iterator it = tests_.begin(); it != tests_.end(); ++it)
    {
        TestInfo &info = *it;
        std::cout << "[ RUN      ] " << info.testCaseName << "." << info.testName << std::endl;
        std::unique_ptr<Test> test(CreateTestInstance(info.factory));
        ResetCurrentTestState();
        if (test.get())
        {
            test->SetUp();
            test->TestBody();
            test->TearDown();
        }
        if (HasFatalFailure() || CurrentTestFailed())
        {
            ++failedTests_;
            std::cout << "[  FAILED  ] " << info.testCaseName << "." << info.testName << std::endl;
        }
        else
        {
            std::cout << "[       OK ] " << info.testCaseName << "." << info.testName << std::endl;
        }
    }
    std::cout << "[==========] " << tests_.size() << " tests ran." << std::endl;
    if (failedTests_ == 0)
    {
        std::cout << "[  PASSED  ] All tests passed." << std::endl;
    }
    else
    {
        std::cout << "[  FAILED  ] " << failedTests_ << " test(s)." << std::endl;
    }
    return failedTests_;
}

TestRegistrar::TestRegistrar(const char *caseName, const char *testName, Test *(*factory)())
{
    UnitTestImpl::GetInstance().AddTest(caseName, testName, factory);
}

std::string FormatComparisonMessage(const char *expr1, const char *expr2, const std::string &val1, const std::string &val2)
{
    std::ostringstream stream;
    stream << "Expected equality of these values:\n  " << expr1 << "\n    Which is: " << val1
           << "\n  " << expr2 << "\n    Which is: " << val2;
    return stream.str();
}

void ExpectTrue(const char *file, int line, bool condition, const std::string &message)
{
    if (!condition)
    {
        UnitTestImpl::GetInstance().AddFailure(file, line, message, false);
    }
}

bool AssertTrue(const char *file, int line, bool condition, const std::string &message)
{
    if (!condition)
    {
        UnitTestImpl::GetInstance().AddFailure(file, line, message, true);
        return false;
    }
    return true;
}

} // namespace internal

void InitGoogleTest(int *argc, char **argv)
{
    (void)argc;
    (void)argv;
}

int RUN_ALL_TESTS()
{
    return internal::UnitTestImpl::GetInstance().RunAllTests();
}

} // namespace testing
