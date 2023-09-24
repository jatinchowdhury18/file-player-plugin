#include "JaiContext.h"

#include <clap/clap.h>
#include <juce_core/juce_core.h>

#include "file_player_jai_lib.h"

JaiContextWrapper::JaiContextWrapper()
{
    internal = __jai_runtime_init (0, nullptr);

    const auto jai_info = get_jai_info (get());
    std::cout << std::string_view { (const char*) jai_info.data, (size_t) jai_info.count } << std::endl;
}

JaiContextWrapper::~JaiContextWrapper()
{
    __jai_runtime_fini (internal);
}
