#include "core/plugin.h"
#include "logger.h"
#include "vrtzmq_source.h"

class VrtzmqSourceSupport : public satdump::Plugin
{
public:
    std::string getID()
    {
        return "vrtzmq_source_support";
    }

    void init()
    {
        satdump::eventBus->register_handler<dsp::RegisterDSPSampleSourcesEvent>(registerSources);
    }

    static void registerSources(const dsp::RegisterDSPSampleSourcesEvent &evt)
    {
        evt.dsp_sources_registry.insert({VrtzmqSource::getID(), {VrtzmqSource::getInstance, VrtzmqSource::getAvailableSources}});
    }
};

PLUGIN_LOADER(VrtzmqSourceSupport)