#pragma once

#include <cstdint>

namespace StupidAR {

    enum class SampleFormat {
        Unknown,

        Float,
        Double,
        
        U8,
        S16,
        S24,
        S32,

        S16of32,
        S18of32,
        S20of32,
        S24of32,
    };

    // 24 bit, little endian, 2's complement integer
    struct pcm24_t {
        uint8_t data[3];

        static const int32_t max_i32 = INT32_MAX >> 8;
        static const int32_t min_i32 = INT32_MIN >> 8;

        int32_t int_value() const {
            int32_t v = *reinterpret_cast<const int8_t*>(&data[2]);
            v <<= 16;
            v |= uint16_t(data[1]) << 8;
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

    template <typename T>
    T reverse_bytes(typename std::enable_if_t<true, T> t) { // Intentionally prevent deduction of T, which behaves unexpectedly
        static_assert(CHAR_BIT == 8, "non-8-bit chars");

        T result;
        char* presult = reinterpret_cast<char*>(&result); // Defined behavior, see https://en.cppreference.com/w/cpp/language/reinterpret_cast
        char* psource = reinterpret_cast<char*>(&t);
        const size_t size = sizeof(T);
        
        for (size_t i = 0; i < size; ++i) {
            presult[i] = psource[size - i - 1];
        }

        return result;
    }

    template <SampleFormat format>
    struct PcmFormatTraits;

    template <>
    struct PcmFormatTraits<SampleFormat::Float> {
        using sample_t = float;
    };

    template <>
    struct PcmFormatTraits<SampleFormat::Double> {
        using sample_t = double;
    };

    template <>
    struct PcmFormatTraits<SampleFormat::U8> {
        using sample_t = uint8_t;
    };

    template <>
    struct PcmFormatTraits<SampleFormat::S16> {
        using sample_t = int16_t;
    };

    template <>
    struct PcmFormatTraits<SampleFormat::S24> {
        using sample_t = pcm24_t;
    };

    template <>
    struct PcmFormatTraits<SampleFormat::S32> {
        using sample_t = int32_t;
    };

    template <>
    struct PcmFormatTraits<SampleFormat::S16of32> {
        using sample_t = int32_t;
    };

    template <>
    struct PcmFormatTraits<SampleFormat::S18of32> {
        using sample_t = int32_t;
    };

    template <>
    struct PcmFormatTraits<SampleFormat::S20of32> {
        using sample_t = int32_t;
    };

    template <>
    struct PcmFormatTraits<SampleFormat::S24of32> {
        using sample_t = int32_t;
    };

    template <SampleFormat from, SampleFormat to>
    typename PcmFormatTraits<to>::sample_t convert_sample(typename PcmFormatTraits<from>::sample_t sample);

    // Integer <-> Floating point conversions:
    // (define other int-float conversions in terms of these 4)

    template <>
    float convert_sample<SampleFormat::S24, SampleFormat::Float>(pcm24_t sample) {
        return sample.int_value() / (pcm24_t::max_i32 + 1.0f);
    }

    template <>
    pcm24_t convert_sample<SampleFormat::Float, SampleFormat::S24>(float sample) {
        return pcm24_t::value_of(int32_t(sample * (pcm24_t::max_i32 + 1.0f)));
    }

    template <>
    double convert_sample<SampleFormat::S32, SampleFormat::Double>(int32_t sample) {
        return sample / (INT32_MAX + 1.0);
    }

    template <>
    int32_t convert_sample<SampleFormat::Double, SampleFormat::S32>(double sample) {
        return int32_t(sample * (INT32_MAX + 1.0));
    }

    // U8LE -> ... conversions:

    template <>
    int16_t convert_sample<SampleFormat::U8, SampleFormat::S16>(uint8_t sample) {
        return (int16_t(sample) - 128) << 8;
    }

    template <>
    int32_t convert_sample<SampleFormat::U8, SampleFormat::S16of32>(uint8_t sample) {
        return int32_t(convert_sample<SampleFormat::U8, SampleFormat::S16>(sample));
    }

    template <>
    int32_t convert_sample<SampleFormat::U8, SampleFormat::S18of32>(uint8_t sample) {
        return (int32_t(sample) - 128) << 10;
    }

    template <>
    int32_t convert_sample<SampleFormat::U8, SampleFormat::S20of32>(uint8_t sample) {
        return (int32_t(sample) - 128) << 12;
    }

    template <>
    int32_t convert_sample<SampleFormat::U8, SampleFormat::S24of32>(uint8_t sample) {
        return (int32_t(sample) - 128) << 16;
    }

    template <>
    pcm24_t convert_sample<SampleFormat::U8, SampleFormat::S24>(uint8_t sample) {
        return pcm24_t::value_of(
            convert_sample<SampleFormat::U8, SampleFormat::S24of32>(sample)
        );
    }

    template <>
    int32_t convert_sample<SampleFormat::U8, SampleFormat::S32>(uint8_t sample) {
        return (int32_t(sample) - 128) << 24;
    }

    template <>
    float convert_sample<SampleFormat::U8, SampleFormat::Float>(uint8_t sample) {
        return convert_sample<SampleFormat::S24, SampleFormat::Float>(
            convert_sample<SampleFormat::U8, SampleFormat::S24>(sample)
        );
    }

    template <>
    double convert_sample<SampleFormat::U8, SampleFormat::Double>(uint8_t sample) {
        return convert_sample<SampleFormat::S32, SampleFormat::Double>(
            convert_sample<SampleFormat::U8, SampleFormat::S32>(sample)
        );
    }

    // S16LE -> ... conversions:

    template <>
    uint8_t convert_sample<SampleFormat::S16, SampleFormat::U8>(int16_t sample) {
        return (sample >> 8) + 128;
    }

    template <>
    int16_t convert_sample<SampleFormat::S16, SampleFormat::S16>(int16_t sample) {
        return sample;
    }

    template <>
    int32_t convert_sample<SampleFormat::S16, SampleFormat::S18of32>(int16_t sample) {
        return int32_t(sample) << 2;
    }

    template <>
    int32_t convert_sample<SampleFormat::S16, SampleFormat::S20of32>(int16_t sample) {
        return int32_t(sample) << 4;
    }

    template <>
    int32_t convert_sample<SampleFormat::S16, SampleFormat::S24of32>(int16_t sample) {
        return int32_t(sample) << 8;
    }

    template <>
    pcm24_t convert_sample<SampleFormat::S16, SampleFormat::S24>(int16_t sample) {
        return pcm24_t::value_of(
            convert_sample<SampleFormat::S16, SampleFormat::S24of32>(sample)
        );
    }

    template <>
    int32_t convert_sample<SampleFormat::S16, SampleFormat::S32>(int16_t sample) {
        return int32_t(sample) << 16;
    }

    template <>
    float convert_sample<SampleFormat::S16, SampleFormat::Float>(int16_t sample) {
        return convert_sample<SampleFormat::S24, SampleFormat::Float>(
            convert_sample<SampleFormat::S16, SampleFormat::S24>(sample)
        );
    }

    template <>
    double convert_sample<SampleFormat::S16, SampleFormat::Double>(int16_t sample) {
        return convert_sample<SampleFormat::S32, SampleFormat::Double>(
            convert_sample<SampleFormat::S16, SampleFormat::S32>(sample)
        );
    }

    // S24LE -> ... conversions:

    template <>
    uint8_t convert_sample<SampleFormat::S24, SampleFormat::U8>(pcm24_t sample) {
        return (sample.int_value() >> 16) + 128;
    }

    template <>
    int16_t convert_sample<SampleFormat::S24, SampleFormat::S16>(pcm24_t sample) {
        return sample.int_value() >> 8;
    }

    template <>
    int32_t convert_sample<SampleFormat::S24, SampleFormat::S16of32>(pcm24_t sample) {
        return convert_sample<SampleFormat::S24, SampleFormat::S16>(sample);
    }

    template <>
    int32_t convert_sample<SampleFormat::S24, SampleFormat::S18of32>(pcm24_t sample) {
        return sample.int_value() >> 6;
    }

    template <>
    int32_t convert_sample<SampleFormat::S24, SampleFormat::S20of32>(pcm24_t sample) {
        return sample.int_value() >> 4;
    }

    template <>
    int32_t convert_sample<SampleFormat::S24, SampleFormat::S24of32>(pcm24_t sample) {
        return sample.int_value();
    }

    template <>
    int32_t convert_sample<SampleFormat::S24, SampleFormat::S32>(pcm24_t sample) {
        return sample.int_value() << 8;
    }

    template <>
    double convert_sample<SampleFormat::S24, SampleFormat::Double>(pcm24_t sample) {
        return convert_sample<SampleFormat::S32, SampleFormat::Double>(
            convert_sample<SampleFormat::S24, SampleFormat::S32>(sample)
        );
    }

    // S16of32LE -> ... conversions:

    template <>
    uint8_t convert_sample<SampleFormat::S16of32, SampleFormat::U8>(int32_t sample) {
        return ((sample << 16) >> 24) + 128;
    }

    template <>
    int16_t convert_sample<SampleFormat::S16of32, SampleFormat::S16>(int32_t sample) {
        return (sample << 16) >> 16;
    }

    template <>
    int32_t convert_sample<SampleFormat::S16of32, SampleFormat::S18of32>(int32_t sample) {
        return (sample << 16) >> 14;
    }

    template <>
    int32_t convert_sample<SampleFormat::S16of32, SampleFormat::S20of32>(int32_t sample) {
        return (sample << 16) >> 12;
    }

    template <>
    int32_t convert_sample<SampleFormat::S16of32, SampleFormat::S24of32>(int32_t sample) {
        return (sample << 16) >> 8;
    }

    template <>
    pcm24_t convert_sample<SampleFormat::S16of32, SampleFormat::S24>(int32_t sample) {
        return pcm24_t::value_of(
            convert_sample<SampleFormat::S16of32, SampleFormat::S24of32>(sample)
        );
    }

    template <>
    int32_t convert_sample<SampleFormat::S16of32, SampleFormat::S32>(int32_t sample) {
        return sample << 16;
    }

    template <>
    float convert_sample<SampleFormat::S16of32, SampleFormat::Float>(int32_t sample) {
        return convert_sample<SampleFormat::S24, SampleFormat::Float>(
            convert_sample<SampleFormat::S16of32, SampleFormat::S24>(sample)
        );
    }

    template <>
    double convert_sample<SampleFormat::S16of32, SampleFormat::Double>(int32_t sample) {
        return convert_sample<SampleFormat::S32, SampleFormat::Double>(
            convert_sample<SampleFormat::S16of32, SampleFormat::S32>(sample)
        );
    }

    // S18of32LE -> ... conversions:

    template <>
    uint8_t convert_sample<SampleFormat::S18of32, SampleFormat::U8>(int32_t sample) {
        return ((sample << 14) >> 24) + 128;
    }

    template <>
    int16_t convert_sample<SampleFormat::S18of32, SampleFormat::S16>(int32_t sample) {
        return (sample << 14) >> 16;
    }

    template <>
    int32_t convert_sample<SampleFormat::S18of32, SampleFormat::S16of32>(int32_t sample) {
        return convert_sample<SampleFormat::S18of32, SampleFormat::S16>(sample);
    }

    template <>
    int32_t convert_sample<SampleFormat::S18of32, SampleFormat::S20of32>(int32_t sample) {
        return (sample << 14) >> 12;
    }

    template <>
    int32_t convert_sample<SampleFormat::S18of32, SampleFormat::S24of32>(int32_t sample) {
        return (sample << 14) >> 8;
    }

    template <>
    pcm24_t convert_sample<SampleFormat::S18of32, SampleFormat::S24>(int32_t sample) {
        return pcm24_t::value_of(
            convert_sample<SampleFormat::S18of32, SampleFormat::S24of32>(sample)
        );
    }

    template <>
    int32_t convert_sample<SampleFormat::S18of32, SampleFormat::S32>(int32_t sample) {
        return sample << 14;
    }

    template <>
    float convert_sample<SampleFormat::S18of32, SampleFormat::Float>(int32_t sample) {
        return convert_sample<SampleFormat::S24, SampleFormat::Float>(
            convert_sample<SampleFormat::S18of32, SampleFormat::S24>(sample)
        );
    }

    template <>
    double convert_sample<SampleFormat::S18of32, SampleFormat::Double>(int32_t sample) {
        return convert_sample<SampleFormat::S32, SampleFormat::Double>(
            convert_sample<SampleFormat::S18of32, SampleFormat::S32>(sample)
        );
    }

    // S20of32LE -> ... conversions:

    template <>
    uint8_t convert_sample<SampleFormat::S20of32, SampleFormat::U8>(int32_t sample) {
        return ((sample << 12) >> 24) + 128;
    }

    template <>
    int16_t convert_sample<SampleFormat::S20of32, SampleFormat::S16>(int32_t sample) {
        return (sample << 12) >> 16;
    }

    template <>
    int32_t convert_sample<SampleFormat::S20of32, SampleFormat::S16of32>(int32_t sample) {
        return convert_sample<SampleFormat::S20of32, SampleFormat::S16>(sample);
    }

    template <>
    int32_t convert_sample<SampleFormat::S20of32, SampleFormat::S18of32>(int32_t sample) {
        return (sample << 12) >> 14;
    }

    template <>
    int32_t convert_sample<SampleFormat::S20of32, SampleFormat::S24of32>(int32_t sample) {
        return (sample << 12) >> 8;
    }

    template <>
    pcm24_t convert_sample<SampleFormat::S20of32, SampleFormat::S24>(int32_t sample) {
        return pcm24_t::value_of(
            convert_sample<SampleFormat::S20of32, SampleFormat::S24of32>(sample)
        );
    }

    template <>
    int32_t convert_sample<SampleFormat::S20of32, SampleFormat::S32>(int32_t sample) {
        return sample << 12;
    }

    template <>
    float convert_sample<SampleFormat::S20of32, SampleFormat::Float>(int32_t sample) {
        return convert_sample<SampleFormat::S24, SampleFormat::Float>(
            convert_sample<SampleFormat::S20of32, SampleFormat::S24>(sample)
        );
    }

    template <>
    double convert_sample<SampleFormat::S20of32, SampleFormat::Double>(int32_t sample) {
        return convert_sample<SampleFormat::S32, SampleFormat::Double>(
            convert_sample<SampleFormat::S20of32, SampleFormat::S32>(sample)
        );
    }

    // S24of32LE -> ... conversions:

    template <>
    uint8_t convert_sample<SampleFormat::S24of32, SampleFormat::U8>(int32_t sample) {
        return ((sample << 8) >> 24) + 128;
    }

    template <>
    int16_t convert_sample<SampleFormat::S24of32, SampleFormat::S16>(int32_t sample) {
        return (sample << 8) >> 16;
    }

    template <>
    int32_t convert_sample<SampleFormat::S24of32, SampleFormat::S16of32>(int32_t sample) {
        return convert_sample<SampleFormat::S24of32, SampleFormat::S16>(sample);
    }

    template <>
    int32_t convert_sample<SampleFormat::S24of32, SampleFormat::S18of32>(int32_t sample) {
        return (sample << 8) >> 14;
    }

    template <>
    int32_t convert_sample<SampleFormat::S24of32, SampleFormat::S20of32>(int32_t sample) {
        return (sample << 8) >> 12;
    }

    template <>
    pcm24_t convert_sample<SampleFormat::S24of32, SampleFormat::S24>(int32_t sample) {
        return pcm24_t::value_of((sample << 8) >> 8);
    }

    template <>
    int32_t convert_sample<SampleFormat::S24of32, SampleFormat::S32>(int32_t sample) {
        return sample << 8;
    }

    template <>
    float convert_sample<SampleFormat::S24of32, SampleFormat::Float>(int32_t sample) {
        return convert_sample<SampleFormat::S24, SampleFormat::Float>(
            convert_sample<SampleFormat::S24of32, SampleFormat::S24>(sample)
        );
    }

    template <>
    double convert_sample<SampleFormat::S24of32, SampleFormat::Double>(int32_t sample) {
        return convert_sample<SampleFormat::S32, SampleFormat::Double>(
            convert_sample<SampleFormat::S24of32, SampleFormat::S32>(sample)
        );
    }

    // S32LE -> ... conversions:

    template <>
    uint8_t convert_sample<SampleFormat::S32, SampleFormat::U8>(int32_t sample) {
        return (sample >> 24) + 128;
    }

    template <>
    int16_t convert_sample<SampleFormat::S32, SampleFormat::S16>(int32_t sample) {
        return sample >> 16;
    }

    template <>
    int32_t convert_sample<SampleFormat::S32, SampleFormat::S16of32>(int32_t sample) {
        return convert_sample<SampleFormat::S32, SampleFormat::S16>(sample);
    }

    template <>
    int32_t convert_sample<SampleFormat::S32, SampleFormat::S18of32>(int32_t sample) {
        return sample >> 14;
    }

    template <>
    int32_t convert_sample<SampleFormat::S32, SampleFormat::S20of32>(int32_t sample) {
        return sample >> 12;
    }

    template <>
    int32_t convert_sample<SampleFormat::S32, SampleFormat::S24of32>(int32_t sample) {
        return sample >> 8;
    }

    template <>
    pcm24_t convert_sample<SampleFormat::S32, SampleFormat::S24>(int32_t sample) {
        return pcm24_t::value_of(
            convert_sample<SampleFormat::S32, SampleFormat::S24of32>(sample)
        );
    }

    template <>
    int32_t convert_sample<SampleFormat::S32, SampleFormat::S32>(int32_t sample) {
        return sample;
    }

    template <>
    float convert_sample<SampleFormat::S32, SampleFormat::Float>(int32_t sample) {
        return convert_sample<SampleFormat::S24, SampleFormat::Float>(
            convert_sample<SampleFormat::S32, SampleFormat::S24>(sample)
        );
    }

    // Float -> ... conversions:

    template <>
    uint8_t convert_sample<SampleFormat::Float, SampleFormat::U8>(float sample) {
        return convert_sample<SampleFormat::S24, SampleFormat::U8>(
            convert_sample<SampleFormat::Float, SampleFormat::S24>(sample)
        );
    }

    template <>
    int16_t convert_sample<SampleFormat::Float, SampleFormat::S16>(float sample) {
        return convert_sample<SampleFormat::S24, SampleFormat::S16>(
            convert_sample<SampleFormat::Float, SampleFormat::S24>(sample)
        );
    }

    template <>
    int32_t convert_sample<SampleFormat::Float, SampleFormat::S16of32>(float sample) {
        return convert_sample<SampleFormat::S24, SampleFormat::S16of32>(
            convert_sample<SampleFormat::Float, SampleFormat::S24>(sample)
        );
    }

    template <>
    int32_t convert_sample<SampleFormat::Float, SampleFormat::S18of32>(float sample) {
        return convert_sample<SampleFormat::S24, SampleFormat::S18of32>(
            convert_sample<SampleFormat::Float, SampleFormat::S24>(sample)
        );
    }

    template <>
    int32_t convert_sample<SampleFormat::Float, SampleFormat::S20of32>(float sample) {
        return convert_sample<SampleFormat::S24, SampleFormat::S20of32>(
            convert_sample<SampleFormat::Float, SampleFormat::S24>(sample)
        );
    }

    template <>
    int32_t convert_sample<SampleFormat::Float, SampleFormat::S24of32>(float sample) {
        return convert_sample<SampleFormat::S24, SampleFormat::S24of32>(
            convert_sample<SampleFormat::Float, SampleFormat::S24>(sample)
        );
    }

    template <>
    int32_t convert_sample<SampleFormat::Float, SampleFormat::S32>(float sample) {
        return convert_sample<SampleFormat::S24, SampleFormat::S32>(
            convert_sample<SampleFormat::Float, SampleFormat::S24>(sample)
        );
    }

    template <>
    double convert_sample<SampleFormat::Float, SampleFormat::Double>(float sample) {
        return convert_sample<SampleFormat::S32, SampleFormat::Double>(
            convert_sample<SampleFormat::Float, SampleFormat::S32>(sample)
        );
    }

    // Double -> ... conversions:

    template <>
    uint8_t convert_sample<SampleFormat::Double, SampleFormat::U8>(double sample) {
        return convert_sample<SampleFormat::S32, SampleFormat::U8>(
            convert_sample<SampleFormat::Double, SampleFormat::S32>(sample)
        );
    }

    template <>
    int16_t convert_sample<SampleFormat::Double, SampleFormat::S16>(double sample) {
        return convert_sample<SampleFormat::S32, SampleFormat::S16>(
            convert_sample<SampleFormat::Double, SampleFormat::S32>(sample)
        );
    }

    template <>
    int32_t convert_sample<SampleFormat::Double, SampleFormat::S16of32>(double sample) {
        return convert_sample<SampleFormat::S32, SampleFormat::S16of32>(
            convert_sample<SampleFormat::Double, SampleFormat::S32>(sample)
        );
    }

    template <>
    int32_t convert_sample<SampleFormat::Double, SampleFormat::S18of32>(double sample) {
        return convert_sample<SampleFormat::S32, SampleFormat::S18of32>(
            convert_sample<SampleFormat::Double, SampleFormat::S32>(sample)
        );
    }

    template <>
    int32_t convert_sample<SampleFormat::Double, SampleFormat::S20of32>(double sample) {
        return convert_sample<SampleFormat::S32, SampleFormat::S20of32>(
            convert_sample<SampleFormat::Double, SampleFormat::S32>(sample)
        );
    }

    template <>
    int32_t convert_sample<SampleFormat::Double, SampleFormat::S24of32>(double sample) {
        return convert_sample<SampleFormat::S32, SampleFormat::S24of32>(
            convert_sample<SampleFormat::Double, SampleFormat::S32>(sample)
        );
    }

    template <>
    pcm24_t convert_sample<SampleFormat::Double, SampleFormat::S24>(double sample) {
        return convert_sample<SampleFormat::S32, SampleFormat::S24>(
            convert_sample<SampleFormat::Double, SampleFormat::S32>(sample)
        );
    }

    template <>
    float convert_sample<SampleFormat::Double, SampleFormat::Float>(double sample) {
        return convert_sample<SampleFormat::S32, SampleFormat::Float>(
            convert_sample<SampleFormat::Double, SampleFormat::S32>(sample)
        );
    }

}
