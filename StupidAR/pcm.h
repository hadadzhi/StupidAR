#pragma once

#include <cstdint>

namespace StupidAR {

    enum class PcmFormat {
        Unknown, 
        Float, 
        Double, 
        U8LE,
        S16LE,
        S24LE,
        S24of32LE,
        S32LE,
    };

    // 24 bit, little endian, 2's complement integer
    struct pcm24_t {
        uint8_t data[3];

        static const int32_t max_i32 = INT32_MAX >> 8;
        static const int32_t min_i32 = INT32_MIN >> 8;

        int32_t int_value() const {
            int32_t v = *reinterpret_cast<const int8_t*>(&data[2]);
            v <<= 16;
            v |= static_cast<uint16_t>(data[1]) << 8;
            v |= data[0];
            return v;
        }

        static pcm24_t value_of(int32_t s24) {
            s24 <<= 8;
            uint32_t uint = *reinterpret_cast<uint32_t*>(&s24);
            pcm24_t v = {};
            v.data[2] = uint >> 24;
            v.data[1] = uint >> 16;
            v.data[0] = uint >> 8;
            return v;
        }
    };

    static_assert(sizeof(pcm24_t) == 3, "pcm24_t packing failed");
    static_assert(sizeof(pcm24_t[10]) == 30, "pcm24_t packing failed");
    static_assert(alignof(pcm24_t) == 1, "pcm24_t packing failed");

    template <PcmFormat format>
    struct PcmFormatTraits;

    template <>
    struct PcmFormatTraits<PcmFormat::Float> {
        using sample_t = float;
    };

    template <>
    struct PcmFormatTraits<PcmFormat::Double> {
        using sample_t = double;
    };

    template <>
    struct PcmFormatTraits<PcmFormat::U8LE> {
        using sample_t = uint8_t;
    };

    template <>
    struct PcmFormatTraits<PcmFormat::S16LE> {
        using sample_t = int16_t;
    };

    template <>
    struct PcmFormatTraits<PcmFormat::S24LE> {
        using sample_t = pcm24_t;
    };

    template <>
    struct PcmFormatTraits<PcmFormat::S24of32LE> {
        using sample_t = int32_t;
    };

    template <>
    struct PcmFormatTraits<PcmFormat::S32LE> {
        using sample_t = int32_t;
    };

    template <PcmFormat from, PcmFormat to>
    typename PcmFormatTraits<to>::sample_t convert_pcm(typename PcmFormatTraits<from>::sample_t sample);

    // Integer <-> Floating point conversions:
    // (define other int-float conversions in terms of these 4)

    template <>
    float convert_pcm<PcmFormat::S24LE, PcmFormat::Float>(pcm24_t sample) {
        return sample.int_value() / (pcm24_t::max_i32 + 1.0f);
    }

    template <>
    pcm24_t convert_pcm<PcmFormat::Float, PcmFormat::S24LE>(float sample) {
        return pcm24_t::value_of(static_cast<int32_t>(sample * (pcm24_t::max_i32 + 1.0f)));
    }

    template <>
    double convert_pcm<PcmFormat::S32LE, PcmFormat::Double>(int32_t sample) {
        return sample / (INT32_MAX + 1.0);
    }

    template <>
    int32_t convert_pcm<PcmFormat::Double, PcmFormat::S32LE>(double sample) {
        return static_cast<int32_t>(sample * (INT32_MAX + 1.0));
    }

    // U8LE -> ... conversions:

    template <>
    uint8_t convert_pcm<PcmFormat::U8LE, PcmFormat::U8LE>(uint8_t sample) {
        return sample;
    }

    template <>
    int16_t convert_pcm<PcmFormat::U8LE, PcmFormat::S16LE>(uint8_t sample) {
        return (static_cast<int16_t>(sample) - 128) << 8;
    }

    template <>
    int32_t convert_pcm<PcmFormat::U8LE, PcmFormat::S24of32LE>(uint8_t sample) {
        return (static_cast<int32_t>(sample) - 128) << 16;
    }

    template <>
    pcm24_t convert_pcm<PcmFormat::U8LE, PcmFormat::S24LE>(uint8_t sample) {
        return pcm24_t::value_of(convert_pcm<PcmFormat::U8LE, PcmFormat::S24of32LE>(sample));
    }

    template <>
    int32_t convert_pcm<PcmFormat::U8LE, PcmFormat::S32LE>(uint8_t sample) {
        return (static_cast<int32_t>(sample) - 128) << 24;
    }

    template <>
    float convert_pcm<PcmFormat::U8LE, PcmFormat::Float>(uint8_t sample) {
        return convert_pcm<PcmFormat::S24LE, PcmFormat::Float>(
            convert_pcm<PcmFormat::U8LE, PcmFormat::S24LE>(sample)
        );
    }

    template <>
    double convert_pcm<PcmFormat::U8LE, PcmFormat::Double>(uint8_t sample) {
        return convert_pcm<PcmFormat::S32LE, PcmFormat::Double>(
            convert_pcm<PcmFormat::U8LE, PcmFormat::S32LE>(sample)
        );
    }

    // S16LE -> ... conversions:

    template <>
    uint8_t convert_pcm<PcmFormat::S16LE, PcmFormat::U8LE>(int16_t sample) {
        return (sample >> 8) + 128;
    }

    template <>
    int16_t convert_pcm<PcmFormat::S16LE, PcmFormat::S16LE>(int16_t sample) {
        return sample;
    }

    template <>
    int32_t convert_pcm<PcmFormat::S16LE, PcmFormat::S24of32LE>(int16_t sample) {
        return static_cast<int32_t>(sample) << 8;
    }

    template <>
    pcm24_t convert_pcm<PcmFormat::S16LE, PcmFormat::S24LE>(int16_t sample) {
        return pcm24_t::value_of(convert_pcm<PcmFormat::S16LE, PcmFormat::S24of32LE>(sample));
    }

    template <>
    int32_t convert_pcm<PcmFormat::S16LE, PcmFormat::S32LE>(int16_t sample) {
        return static_cast<int32_t>(sample) << 16;
    }

    template <>
    float convert_pcm<PcmFormat::S16LE, PcmFormat::Float>(int16_t sample) {
        return convert_pcm<PcmFormat::S24LE, PcmFormat::Float>(
            convert_pcm<PcmFormat::S16LE, PcmFormat::S24LE>(sample)
        );
    }

    template <>
    double convert_pcm<PcmFormat::S16LE, PcmFormat::Double>(int16_t sample) {
        return convert_pcm<PcmFormat::S32LE, PcmFormat::Double>(
            convert_pcm<PcmFormat::S16LE, PcmFormat::S32LE>(sample)
        );
    }

    // S24LE -> ... conversions:

    template <>
    uint8_t convert_pcm<PcmFormat::S24LE, PcmFormat::U8LE>(pcm24_t sample) {
        return (sample.int_value() >> 16) + 128;
    }

    template <>
    int16_t convert_pcm<PcmFormat::S24LE, PcmFormat::S16LE>(pcm24_t sample) {
        return sample.int_value() >> 8;
    }

    template <>
    int32_t convert_pcm<PcmFormat::S24LE, PcmFormat::S24of32LE>(pcm24_t sample) {
        return sample.int_value();
    }

    template <>
    pcm24_t convert_pcm<PcmFormat::S24LE, PcmFormat::S24LE>(pcm24_t sample) {
        return sample;
    }

    template <>
    int32_t convert_pcm<PcmFormat::S24LE, PcmFormat::S32LE>(pcm24_t sample) {
        return sample.int_value() << 8;
    }

    template <>
    double convert_pcm<PcmFormat::S24LE, PcmFormat::Double>(pcm24_t sample) {
        return convert_pcm<PcmFormat::S32LE, PcmFormat::Double>(
            convert_pcm<PcmFormat::S24LE, PcmFormat::S32LE>(sample)
        );
    }

    // S24of32LE -> ... conversions:

    template <>
    uint8_t convert_pcm<PcmFormat::S24of32LE, PcmFormat::U8LE>(int32_t sample) {
        return (sample >> 16) + 128;
    }

    template <>
    int16_t convert_pcm<PcmFormat::S24of32LE, PcmFormat::S16LE>(int32_t sample) {
        return sample >> 8;
    }

    template <>
    int32_t convert_pcm<PcmFormat::S24of32LE, PcmFormat::S24of32LE>(int32_t sample) {
        return sample;
    }

    template <>
    pcm24_t convert_pcm<PcmFormat::S24of32LE, PcmFormat::S24LE>(int32_t sample) {
        return pcm24_t::value_of(sample);
    }

    template <>
    int32_t convert_pcm<PcmFormat::S24of32LE, PcmFormat::S32LE>(int32_t sample) {
        return sample << 8;
    }

    template <>
    float convert_pcm<PcmFormat::S24of32LE, PcmFormat::Float>(int32_t sample) {
        return convert_pcm<PcmFormat::S24LE, PcmFormat::Float>(
            convert_pcm<PcmFormat::S24of32LE, PcmFormat::S24LE>(sample)
        );
    }

    template <>
    double convert_pcm<PcmFormat::S24of32LE, PcmFormat::Double>(int32_t sample) {
        return convert_pcm<PcmFormat::S32LE, PcmFormat::Double>(
            convert_pcm<PcmFormat::S24of32LE, PcmFormat::S32LE>(sample)
        );
    }

    // S32LE -> ... conversions:

    template <>
    uint8_t convert_pcm<PcmFormat::S32LE, PcmFormat::U8LE>(int32_t sample) {
        return (sample >> 24) + 128;
    }

    template <>
    int16_t convert_pcm<PcmFormat::S32LE, PcmFormat::S16LE>(int32_t sample) {
        return sample >> 16;
    }

    template <>
    int32_t convert_pcm<PcmFormat::S32LE, PcmFormat::S24of32LE>(int32_t sample) {
        return sample >> 8;
    }

    template <>
    pcm24_t convert_pcm<PcmFormat::S32LE, PcmFormat::S24LE>(int32_t sample) {
        return pcm24_t::value_of(sample >> 8);
    }

    template <>
    int32_t convert_pcm<PcmFormat::S32LE, PcmFormat::S32LE>(int32_t sample) {
        return sample;
    }

    template <>
    float convert_pcm<PcmFormat::S32LE, PcmFormat::Float>(int32_t sample) {
        return convert_pcm<PcmFormat::S24LE, PcmFormat::Float>(
            convert_pcm<PcmFormat::S32LE, PcmFormat::S24LE>(sample)
        );
    }

    // Float -> ... conversions:

    template <>
    uint8_t convert_pcm<PcmFormat::Float, PcmFormat::U8LE>(float sample) {
        return convert_pcm<PcmFormat::S24LE, PcmFormat::U8LE>(
            convert_pcm<PcmFormat::Float, PcmFormat::S24LE>(sample)
        );
    }

    template <>
    int16_t convert_pcm<PcmFormat::Float, PcmFormat::S16LE>(float sample) {
        return convert_pcm<PcmFormat::S24LE, PcmFormat::S16LE>(
            convert_pcm<PcmFormat::Float, PcmFormat::S24LE>(sample)
        );
    }

    template <>
    int32_t convert_pcm<PcmFormat::Float, PcmFormat::S24of32LE>(float sample) {
        return convert_pcm<PcmFormat::S24LE, PcmFormat::S24of32LE>(
            convert_pcm<PcmFormat::Float, PcmFormat::S24LE>(sample)
        );
    }

    template <>
    int32_t convert_pcm<PcmFormat::Float, PcmFormat::S32LE>(float sample) {
        return convert_pcm<PcmFormat::S24LE, PcmFormat::S32LE>(
            convert_pcm<PcmFormat::Float, PcmFormat::S24LE>(sample)
        );
    }

    template <>
    float convert_pcm<PcmFormat::Float, PcmFormat::Float>(float sample) {
        return sample;
    }

    template <>
    double convert_pcm<PcmFormat::Float, PcmFormat::Double>(float sample) {
        return convert_pcm<PcmFormat::S32LE, PcmFormat::Double>(
            convert_pcm<PcmFormat::Float, PcmFormat::S32LE>(sample)
        );
    }

    // Double -> ... conversions:

    template <>
    uint8_t convert_pcm<PcmFormat::Double, PcmFormat::U8LE>(double sample) {
        return convert_pcm<PcmFormat::S32LE, PcmFormat::U8LE>(
            convert_pcm<PcmFormat::Double, PcmFormat::S32LE>(sample)
        );
    }

    template <>
    int16_t convert_pcm<PcmFormat::Double, PcmFormat::S16LE>(double sample) {
        return convert_pcm<PcmFormat::S32LE, PcmFormat::S16LE>(
            convert_pcm<PcmFormat::Double, PcmFormat::S32LE>(sample)
        );
    }

    template <>
    int32_t convert_pcm<PcmFormat::Double, PcmFormat::S24of32LE>(double sample) {
        return convert_pcm<PcmFormat::S32LE, PcmFormat::S24of32LE>(
            convert_pcm<PcmFormat::Double, PcmFormat::S32LE>(sample)
        );
    }

    template <>
    float convert_pcm<PcmFormat::Double, PcmFormat::Float>(double sample) {
        return convert_pcm<PcmFormat::S32LE, PcmFormat::Float>(
            convert_pcm<PcmFormat::Double, PcmFormat::S32LE>(sample)
        );
    }

    template <>
    double convert_pcm<PcmFormat::Double, PcmFormat::Double>(double sample) {
        return sample;
    }

}
