#include "vrtzmq_source.h"
#include "imgui/imgui_stdlib.h"
#include "core/plugin.h"
#include "recorder/recorder.h"

void VrtzmqSource::VRTrecorderDrawPanelEvent(const satdump::RecorderDrawPanelEvent &evt)
{
    if (ImGui::CollapsingHeader("VRT Context"))
    {
        ImGui::Text("Object : %s", tracker_ext_context.object_name);
        ImGui::Text("Source : %s", tracker_ext_context.tracking_source);
        ImGui::Text("Frequency : %.2f", tracker_ext_context.frequency+tracker_ext_context.doppler);
        ImGui::Text("Doppler rate : %.4f", tracker_ext_context.doppler_rate);
        ImGui::Text("Azimuth : %.2f", tracker_ext_context.azimuth);
        ImGui::Text("Elevation : %.2f", tracker_ext_context.elevation);
        ImGui::Text("SDR gain : %i", vrt_context.gain);
        ImGui::Text("SDR Ref : %s", vrt_context.reflock ? "external" : "internal");
    }
}

void VrtzmqSource::set_settings(nlohmann::json settings)
{
    d_settings = settings;
    address = getValueOrDefault(d_settings["address"], address);
    instance = getValueOrDefault(d_settings["instance"], instance);
    port = getValueOrDefault(d_settings["port"], port);
    use_port = getValueOrDefault(d_settings["use_port"], use_port);
}

nlohmann::json VrtzmqSource::get_settings()
{
    d_settings["address"] = address;
    d_settings["instance"] = instance;
    d_settings["port"] = port;
    d_settings["use_port"] = use_port;
    return d_settings;
}

void VrtzmqSource::open()
{
    satdump::eventBus->register_handler<satdump::RecorderDrawPanelEvent>([this](const satdump::RecorderDrawPanelEvent &evt)
                                                                             { VRTrecorderDrawPanelEvent(evt); });
    is_open = true;
}

void VrtzmqSource::run_thread()
{
    bool start_rx = false;
    while (should_run)
    {
        if (is_started)
        {
            size_t len = zmq_recv(subscriber, buffer, ZMQ_BUFFER_SIZE, 0);
            if (not vrt_process(buffer, sizeof(buffer), &vrt_context, &vrt_packet)) {
                printf("Not a Vita49 packet?\n");
                continue;
            }
            if (vrt_packet.context) {
                // vrt_print_context(&vrt_context);
                if (not start_rx or vrt_context.context_changed) {
                    // logger->info("[VRT ZMQ] Set frequency to %.2f", (double)vrt_context.rf_freq);
                    satdump::eventBus->fire_event<satdump::RecorderSetDeviceSamplerateEvent>({vrt_context.sample_rate});
                    satdump::eventBus->fire_event<satdump::RecorderSetFrequencyEvent>({(double)vrt_context.rf_freq});
                }
                start_rx = true;
            }
            if (start_rx and vrt_packet.data) {
                for (uint32_t i = 0; i < vrt_packet.num_rx_samps; i++) {
                    int16_t re;
                    memcpy(&re, (char*)&buffer[vrt_packet.offset+i], 2);
                    int16_t img;
                    memcpy(&img, (char*)&buffer[vrt_packet.offset+i]+2, 2);
                    // convert to float32
                    float_data[2*i] = (float)re / 65535;
                    float_data[2*i+1] = (float)img / 65535;
                }
                // fill satdump buffer
                memcpy(output_stream->writeBuf, float_data, 2*vrt_packet.num_rx_samps*sizeof(float));
                output_stream->swap(vrt_packet.num_rx_samps);
            } else if  (vrt_packet.extended_context) {
                dt_process(buffer, sizeof(buffer), &vrt_packet, &dt_ext_context);
                tracker_process(buffer, sizeof(buffer), &vrt_packet, &tracker_ext_context);
            }
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            start_rx = false;
        }
    }
}

void VrtzmqSource::start()
{
   
    int hwm = 10000;
    init_context(&vrt_context);
    uint32_t channel = 0;
    vrt_packet.channel_filt = 1<<channel;
    uint16_t main_port;

    if (use_port)
        main_port = port;
    else
        main_port = DEFAULT_MAIN_PORT + MAX_CHANNELS*instance;

    // ZMQ
    context = zmq_ctx_new();
    subscriber = zmq_socket(context, ZMQ_SUB);
    int rc = zmq_setsockopt (subscriber, ZMQ_RCVHWM, &hwm, sizeof hwm);
    std::string connect_string = "tcp://" + address + ":" + std::to_string(main_port);
    rc = zmq_connect(subscriber, connect_string.c_str());
    assert(rc == 0);
    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "", 0);

    logger->info("Opening ZMQ to " + std::string("tcp://" + address + ":" + std::to_string(main_port)));

    DSPSampleSource::start();

    // set_frequency(d_frequency);

    is_started = true;
}

void VrtzmqSource::stop()
{
    if (!is_started)
        return;

    is_started = false;

    zmq_close(subscriber);
    zmq_ctx_destroy(context);

    output_stream->flush();
}

void VrtzmqSource::close()
{
    is_open = false;
}

void VrtzmqSource::set_frequency(uint64_t frequency)
{
    // DSPSampleSource::set_frequency(frequency);
    // logger->info("Set SDR frequency to %d", frequency);
}

void VrtzmqSource::drawControlUI()
{
    if (is_started)
        style::beginDisabled();

    current_samplerate.draw();

    ImGui::InputText("Address", &address);
    ImGui::InputInt("Instance", &instance);
    ImGui::InputInt("Port", &port);
    ImGui::Checkbox("Use Port", &use_port);

    if (is_started)
        style::endDisabled();
}

void VrtzmqSource::set_samplerate(uint64_t samplerate)
{
    current_samplerate.set(samplerate);
}

uint64_t VrtzmqSource::get_samplerate()
{
    return current_samplerate.get();
}

std::vector<dsp::SourceDescriptor> VrtzmqSource::getAvailableSources()
{
    std::vector<dsp::SourceDescriptor> results;

    results.push_back({"vrtzmq_source", "VRT ZMQ Source", "0", false});

    return results;
}
