#pragma once

#include <cstdint>
#include <functional>

#include "pcm.h"

namespace StupidAR {

    // It manages device/driver specific stuff. It requests PCM data from you via a callback. The callback is passed to a constructor or a factory function.
    class IOutputAgent {
    public:
        // Accepts an array of buffers, one per channel. Returns true when successfully filled the buffers.
        using CallbackType = std::function<bool(char**)>;

        virtual ~IOutputAgent() {}

        // Sampling rate in Hz
        virtual int32_t sampling_rate() = 0;

        // Buffer size in frames
        virtual int32_t buffer_size() = 0;

        // Number of output channels
        virtual int32_t output_channels() = 0;

        // The expected format for the PCM data
        virtual PcmFormat pcm_format() = 0;

        // The agent will start requesting PCM data after this method returns.
        virtual void start() = 0;

        // The agent will stop requesting PCM data after this method returns.
        virtual void stop() = 0;
    };

}
