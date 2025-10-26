#ifndef CPPSTUB_STUB_H
#define CPPSTUB_STUB_H

#include <cstddef>

namespace cppstub
{
class StubBase
{
public:
    StubBase() : callCount_(0U) {}
    virtual ~StubBase() {}

    std::size_t GetCallCount() const
    {
        return callCount_;
    }

protected:
    void IncrementCallCount()
    {
        ++callCount_;
    }

    void ResetCallCount()
    {
        callCount_ = 0U;
    }

private:
    std::size_t callCount_;
};

template <typename Signature>
class Stub;

template <typename Ret, typename... Args>
class Stub<Ret(Args...)> : public StubBase
{
public:
    typedef Ret (*FunctionType)(Args...);

    Stub() : function_(0) {}

    void SetStub(FunctionType function)
    {
        function_ = function;
        ResetCallCount();
    }

    void Reset()
    {
        function_ = 0;
        ResetCallCount();
    }

    bool HasStub() const
    {
        return function_ != 0;
    }

    FunctionType GetStub() const
    {
        return function_;
    }

    Ret Invoke(Args... args)
    {
        IncrementCallCount();
        return function_(args...);
    }

private:
    FunctionType function_;
};

} // namespace cppstub

#endif // CPPSTUB_STUB_H
