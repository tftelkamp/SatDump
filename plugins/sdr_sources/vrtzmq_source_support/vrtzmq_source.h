#pragma once

#include "common/dsp_source_sink/dsp_sample_source.h"
#include "common/widgets/notated_num.h"
#include "logger.h"
#include "imgui/imgui.h"
#include "core/style.h"
#include <thread>

#include "recorder/recorder.h"

#include <zmq.h>
#include "vrt-tools.h"
#include "dt-extended-context.h"
#include "tracker-extended-context.h"

class VrtzmqSource : public dsp::DSPSampleSource
{
protected:
    bool is_open = false, is_started = false;

    std::string address = "localhost";
    int port = 50100;
    int instance = 0;
    bool use_port = false;

    // VRT ZMQ
    void *context;
    void *subscriber;
    uint32_t buffer[ZMQ_BUFFER_SIZE];
    float float_data[VRT_SAMPLES_PER_PACKET*2];

    context_type vrt_context;
    packet_type vrt_packet;
    dt_ext_context_type dt_ext_context;
    tracker_ext_context_type tracker_ext_context;

    widgets::NotatedNum<uint64_t> current_samplerate = widgets::NotatedNum<uint64_t>("Samplerate##vrtzmq", 0, "sps");

    std::string error;

protected:
    bool should_run = true;
    std::thread work_thread;
    void run_thread();

public:
    VrtzmqSource(dsp::SourceDescriptor source) : DSPSampleSource(source)
    {
        should_run = true;
        work_thread = std::thread(&VrtzmqSource::run_thread, this);
    }

    ~VrtzmqSource()
    {
        stop();
        close();
        should_run = false;
        if (work_thread.joinable())
            work_thread.join();
    }

    void set_settings(nlohmann::json settings);
    nlohmann::json get_settings();

    void open();
    void start();
    void stop();
    void close();

    void set_frequency(uint64_t frequency);

    void drawControlUI();

    void set_samplerate(uint64_t samplerate);
    uint64_t get_samplerate();

    void VRTrecorderDrawPanelEvent(const satdump::RecorderDrawPanelEvent &evt);

    static std::string getID() { return "vrtzmq_source"; }
    static std::shared_ptr<dsp::DSPSampleSource> getInstance(dsp::SourceDescriptor source) { return std::make_shared<VrtzmqSource>(source); }
    static std::vector<dsp::SourceDescriptor> getAvailableSources();
};
