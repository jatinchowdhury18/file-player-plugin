#pragma once

namespace jai
{
struct Context;
}
struct JaiContextWrapper
{
    JaiContextWrapper();
    ~JaiContextWrapper();

    jai::Context* get() { return internal; }
    operator jai::Context*() { return get(); }; // NOLINT

private:
    jai::Context* internal = nullptr;
};
