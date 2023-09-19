#include <clap/entry.h>
#include <clap/factory/plugin-factory.h>
#include <clap/plugin.h>

#include <clapwrapper/vst3.h>
#include <clapwrapper/auv2.h>

#include "file_player_plugin.h"

namespace file_player::plugin_entry
{
uint32_t clap_get_plugin_count (const clap_plugin_factory* f) { return 1; }
const clap_plugin_descriptor* clap_get_plugin_descriptor (const clap_plugin_factory*, uint32_t w)
{
    if (w == 0)
        return &plugin::File_Player_Plugin::descriptor;

    return nullptr;
}

static const clap_plugin* clap_create_plugin (const clap_plugin_factory*, const clap_host* host, const char* plugin_id)
{
    if (strcmp (plugin_id, plugin::File_Player_Plugin::descriptor.id) == 0)
    {
        auto p = new plugin::File_Player_Plugin { host };
        return p->clapPlugin();
    }

    return nullptr;
}

static bool clap_get_auv2_info (const clap_plugin_factory_as_auv2*,
                                uint32_t index,
                                clap_plugin_info_as_auv2_t* info)
{
    auto desc = clap_get_plugin_descriptor (nullptr, index); // we don't use the factory above
    auto plugin_id = desc->id;

    info->au_type[0] = 0; // use the features to determine the type
    if (strcmp (plugin_id, plugin::File_Player_Plugin::descriptor.id) == 0)
    {
        strncpy (info->au_subt, "Fpp2", 5);
        return true;
    }
    return false;
}

const struct clap_plugin_factory fpp_clap_factory = {
    file_player::plugin_entry::clap_get_plugin_count,
    file_player::plugin_entry::clap_get_plugin_descriptor,
    file_player::plugin_entry::clap_create_plugin,
};

constexpr struct clap_plugin_factory_as_auv2 fpp_auv2_factory = {
    "chow", // manu
    "Chowdhury DSP", // manu name
    clap_get_auv2_info
};

static const void* get_factory (const char* factory_id)
{
    if (! strcmp (factory_id, CLAP_PLUGIN_FACTORY_ID))
        return &fpp_clap_factory;

    if (! strcmp (factory_id, CLAP_PLUGIN_FACTORY_INFO_AUV2))
        return &fpp_auv2_factory;

    return nullptr;
}

// clap_init and clap_deinit are required to be fast, but we have nothing we need to do here
bool clap_init (const char*) { return true; }
void clap_deinit() {}

} // namespace file_player::plugin_entry

extern "C"
{
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes" // other peoples errors are outside my scope
#endif

    // clang-format off
    const CLAP_EXPORT struct clap_plugin_entry clap_entry = {
        CLAP_VERSION,
        file_player::plugin_entry::clap_init,
        file_player::plugin_entry::clap_deinit,
        file_player::plugin_entry::get_factory
    };
    // clang-format on
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
}
