#ifndef THIRD_PARTY_GTEST_INCLUDE_GTEST_GTEST_H_
#define THIRD_PARTY_GTEST_INCLUDE_GTEST_GTEST_H_

#include <cmath>
#include <exception>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace testing {

namespace internal { class UnitTestImpl; }

class Test
{
public:
    Test() {}
    virtual ~Test() {}

    virtual void SetUp() {}
    virtual void TearDown() {}

protected:
    virtual void TestBody() {}

    friend class internal::UnitTestImpl;
};

namespace internal {

struct TestInfo
{
    std::string testCaseName;
    std::string testName;
    Test *(*factory)();
};

class UnitTestImpl
{
public:
    static UnitTestImpl &GetInstance();

    void AddTest(const std::string &caseName, const std::string &testName, Test *(*factory)());

    int RunAllTests();

    void AddFailure(const char *file, int line, const std::string &message, bool fatal);

    void ResetCurrentTestState();

    bool HasFatalFailure() const;

    bool CurrentTestFailed() const;

private:
    UnitTestImpl();
    std::vector<TestInfo> tests_;
    int failedTests_;
    bool fatalFailure_;
    bool currentFailed_;
};

class TestRegistrar
{
public:
    TestRegistrar(const char *caseName, const char *testName, Test *(*factory)());
};

std::string FormatComparisonMessage(const char *expr1, const char *expr2, const std::string &val1, const std::string &val2);

void ExpectTrue(const char *file, int line, bool condition, const std::string &message);

bool AssertTrue(const char *file, int line, bool condition, const std::string &message);

template <typename T>
std::string ToString(const T &value)
{
    std::ostringstream stream;
    stream << value;
    return stream.str();
}

template <typename T1, typename T2>
void ExpectEqual(const char *file, int line, const T1 &val1, const T2 &val2, const char *expr1, const char *expr2)
{
    if (!(val1 == val2))
    {
        ExpectTrue(file, line, false, FormatComparisonMessage(expr1, expr2, ToString(val1), ToString(val2)));
    }
}

template <typename T1, typename T2>
bool AssertEqual(const char *file, int line, const T1 &val1, const T2 &val2, const char *expr1, const char *expr2)
{
    if (!(val1 == val2))
    {
        return AssertTrue(file, line, false, FormatComparisonMessage(expr1, expr2, ToString(val1), ToString(val2)));
    }
    return true;
}

inline void ExpectNear(const char *file, int line, double val1, double val2, double absError, const char *expr1, const char *expr2)
{
    if (std::fabs(val1 - val2) > absError)
    {
        std::ostringstream stream;
        stream << "The difference between " << expr1 << " and " << expr2 << " is "
               << std::fabs(val1 - val2) << ", which exceeds " << absError;
        ExpectTrue(file, line, false, stream.str());
    }
}

inline void ExpectFloatEqual(const char *file, int line, float val1, float val2, const char *expr1, const char *expr2)
{
    ExpectNear(file, line, val1, val2, 1e-5f, expr1, expr2);
}

} // namespace internal

void InitGoogleTest(int *argc, char **argv);
int RUN_ALL_TESTS();

namespace internal
{

inline Test *CreateTestInstance(Test *(*factory)())
{
    return factory ? factory() : static_cast<Test *>(0);
}

} // namespace internal

} // namespace testing

#define GTEST_CONCAT_TOKEN_IMPL(x, y) x##y
#define GTEST_CONCAT_TOKEN(x, y) GTEST_CONCAT_TOKEN_IMPL(x, y)

#define GTEST_TEST_CLASS_NAME_(test_case_name, test_name) test_case_name##_##test_name##_Test

#define TEST(test_case_name, test_name)                                                                            \
    class GTEST_TEST_CLASS_NAME_(test_case_name, test_name) : public ::testing::Test                               \
    {                                                                                                              \
    public:                                                                                                        \
        GTEST_TEST_CLASS_NAME_(test_case_name, test_name)() {}                                                     \
                                                                                                                   \
    protected:                                                                                                     \
        virtual void TestBody();                                                                                   \
                                                                                                                   \
    private:                                                                                                       \
        static ::testing::Test *Factory();                                                                         \
        static ::testing::internal::TestRegistrar registrar_;                                                      \
    };                                                                                                             \
    ::testing::Test *GTEST_TEST_CLASS_NAME_(test_case_name, test_name)::Factory()                                  \
    {                                                                                                              \
        return new GTEST_TEST_CLASS_NAME_(test_case_name, test_name);                                              \
    }                                                                                                              \
    ::testing::internal::TestRegistrar GTEST_TEST_CLASS_NAME_(test_case_name, test_name)::registrar_(              \
        #test_case_name, #test_name, &GTEST_TEST_CLASS_NAME_(test_case_name, test_name)::Factory);                 \
    void GTEST_TEST_CLASS_NAME_(test_case_name, test_name)::TestBody()

#define TEST_F(test_fixture, test_name)                                                                            \
    class GTEST_TEST_CLASS_NAME_(test_fixture, test_name) : public test_fixture                                    \
    {                                                                                                              \
    public:                                                                                                        \
        GTEST_TEST_CLASS_NAME_(test_fixture, test_name)() {}                                                       \
                                                                                                                   \
    protected:                                                                                                     \
        virtual void TestBody();                                                                                   \
                                                                                                                   \
    private:                                                                                                       \
        static ::testing::Test *Factory();                                                                         \
        static ::testing::internal::TestRegistrar registrar_;                                                      \
    };                                                                                                             \
    ::testing::Test *GTEST_TEST_CLASS_NAME_(test_fixture, test_name)::Factory()                                    \
    {                                                                                                              \
        return new GTEST_TEST_CLASS_NAME_(test_fixture, test_name);                                                \
    }                                                                                                              \
    ::testing::internal::TestRegistrar GTEST_TEST_CLASS_NAME_(test_fixture, test_name)::registrar_(                \
        #test_fixture, #test_name, &GTEST_TEST_CLASS_NAME_(test_fixture, test_name)::Factory);                     \
    void GTEST_TEST_CLASS_NAME_(test_fixture, test_name)::TestBody()

#define EXPECT_EQ(val1, val2) ::testing::internal::ExpectEqual(__FILE__, __LINE__, (val1), (val2), #val1, #val2)
#define EXPECT_FLOAT_EQ(val1, val2) ::testing::internal::ExpectFloatEqual(__FILE__, __LINE__, (val1), (val2), #val1, #val2)
#define EXPECT_NEAR(val1, val2, abs_error) ::testing::internal::ExpectNear(__FILE__, __LINE__, (val1), (val2), (abs_error), #val1, #val2)
#define EXPECT_TRUE(condition) ::testing::internal::ExpectTrue(__FILE__, __LINE__, static_cast<bool>(condition), #condition)
#define EXPECT_FALSE(condition) ::testing::internal::ExpectTrue(__FILE__, __LINE__, !(condition), std::string("! (") + #condition + ")")

#define ASSERT_EQ(val1, val2)                                                                                      \
    do                                                                                                             \
    {                                                                                                              \
        if (!::testing::internal::AssertEqual(__FILE__, __LINE__, (val1), (val2), #val1, #val2))                   \
        {                                                                                                          \
            return;                                                                                                \
        }                                                                                                          \
    } while (0)

#define ASSERT_TRUE(condition)                                                                                     \
    do                                                                                                             \
    {                                                                                                              \
        if (!::testing::internal::AssertTrue(__FILE__, __LINE__, static_cast<bool>(condition), #condition))        \
        {                                                                                                          \
            return;                                                                                                \
        }                                                                                                          \
    } while (0)

#define ASSERT_FALSE(condition)                                                                                    \
    do                                                                                                             \
    {                                                                                                              \
        if (!::testing::internal::AssertTrue(__FILE__, __LINE__, !(condition), std::string("! (") + #condition + ")")) \
        {                                                                                                          \
            return;                                                                                                \
        }                                                                                                          \
    } while (0)

#endif // THIRD_PARTY_GTEST_INCLUDE_GTEST_GTEST_H_
