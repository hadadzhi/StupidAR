#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "pcm.h"

using namespace StupidAR;

TEST_CASE("Sanity check", "[sanity]") {
    for (int32_t i = 0; i <= INT8_MAX; ++i) {
        REQUIRE((i >= 0 && i <= INT8_MAX));
    }
}

TEST_CASE("pcm24_t", "[custom_types]") {
    for (int32_t sample = pcm24_t::min_i32; sample <= pcm24_t::max_i32; ++sample) {
        REQUIRE(sample == pcm24_t::value_of(sample).int_value());
    }
}

TEST_CASE("S16 conversions", "[pcm_conversions]") {
    for (int32_t i = INT16_MIN; i <= INT16_MAX; ++i) {
        const int16_t sample = i;

        REQUIRE(
            sample == 
                convert_sample<SampleFormat::Float, SampleFormat::S16>(
                    convert_sample<SampleFormat::S16, SampleFormat::Float>(
                        sample
                    )
                )
        );

        REQUIRE(
            sample ==
                convert_sample<SampleFormat::Double, SampleFormat::S16>(
                    convert_sample<SampleFormat::S16, SampleFormat::Double>(
                        sample
                    )
                )
        );

        REQUIRE(
            convert_sample<SampleFormat::S16, SampleFormat::S32>(sample) == 
                convert_sample<SampleFormat::Float, SampleFormat::S32>(
                    convert_sample<SampleFormat::S16, SampleFormat::Float>(
                        sample
                    )
                )
        );

        REQUIRE(
            convert_sample<SampleFormat::S16, SampleFormat::S32>(sample) ==
                convert_sample<SampleFormat::Double, SampleFormat::S32>(
                    convert_sample<SampleFormat::S16, SampleFormat::Double>(
                        sample
                    )
                )
        );

        REQUIRE(
            convert_sample<SampleFormat::S16, SampleFormat::S24of32>(sample) ==
                convert_sample<SampleFormat::S16, SampleFormat::S24>(sample).int_value()
        );
    }
}

TEST_CASE("U8 conversions", "[pcm_conversions]") {
    for (int32_t i = 0; i <= UINT8_MAX; ++i) {
        const uint8_t sample = i;

        REQUIRE(
            sample ==
                convert_sample<SampleFormat::Float, SampleFormat::U8>(
                    convert_sample<SampleFormat::U8, SampleFormat::Float>(
                        sample
                    )
                )
        );

        REQUIRE(
            sample ==
                convert_sample<SampleFormat::Double, SampleFormat::U8>(
                    convert_sample<SampleFormat::U8, SampleFormat::Double>(
                        sample
                    )
                )
        );

        REQUIRE(
            convert_sample<SampleFormat::U8, SampleFormat::S32>(sample) ==
                convert_sample<SampleFormat::Float, SampleFormat::S32>(
                    convert_sample<SampleFormat::U8, SampleFormat::Float>(
                        sample
                    )
                )
        );
    }
}

TEST_CASE("S24 conversions", "[pcm_conversions]") {
    for (int32_t i = pcm24_t::min_i32; i <= pcm24_t::max_i32; ++i) {
        const pcm24_t sample = pcm24_t::value_of(i);
        const float sample_f = convert_sample<SampleFormat::S24, SampleFormat::Float>(sample);


        REQUIRE(
            // As in SaneAR
            (sample.int_value() << 8) / (INT32_MAX + 1.0f) == sample_f
        );

        REQUIRE(
            static_cast<int32_t>(sample_f * (INT32_MAX + 1.0f)) >> 8 ==
                convert_sample<SampleFormat::Float, SampleFormat::S24of32>(sample_f)
        );
    }
}

TEST_CASE("Reverse bytes", "[reverse_bytes]") {
    REQUIRE(INT8_MIN == reverse_bytes<int8_t>(INT8_MIN));
    REQUIRE(0x80 == reverse_bytes<int16_t>(INT16_MIN));
    REQUIRE(0x80 == reverse_bytes<int32_t>(INT32_MIN));
    REQUIRE(DBL_MIN == reverse_bytes<double>(reverse_bytes<double>(DBL_MIN)));
    REQUIRE(0x80 == reverse_bytes<pcm24_t>(pcm24_t::value_of(pcm24_t::min_i32)).int_value());
}


TEST_CASE("int32<->float", "[intfloat]") {
    const int32_t min = INT32_MIN >> 7;
    const int32_t max = INT32_MAX >> 7;
    for (int64_t i = min; i <= max; ++i) {
        const int32_t sample = i;
        const float sample_f = sample / (max + 1.0f);
        const int32_t sample_fi = sample_f * (max + 1.0f);
        REQUIRE(sample_fi == sample);
    }
}
