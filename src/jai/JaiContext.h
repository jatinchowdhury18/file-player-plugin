#pragma once

struct jai_Context;
struct JaiContextWrapper
{
    JaiContextWrapper();
    ~JaiContextWrapper();

    jai_Context* get() { return internal; }
    operator jai_Context*() { return get(); }; // NOLINT

private:
    jai_Context* internal = nullptr;
};
