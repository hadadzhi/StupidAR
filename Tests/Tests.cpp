#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "pcm.h"

using namespace StupidAR;

TEST_CASE("Sanity check", "[sanity]") {
    for (int32_t i = 0; i <= INT8_MAX; ++i) {
        REQUIRE((i >= 0 && i <= INT8_MAX));
    }
}

TEST_CASE("pcm24_t", "[pcm_conversions]") {
    for (int32_t sample = pcm24_t::min_i32; sample <= pcm24_t::max_i32; ++sample) {
        REQUIRE(sample == pcm24_t::value_of(sample).int_value());
    }
}

// These should cover most of the conversions internally
TEST_CASE("S16LE conversions", "[pcm_conversions]") {
    for (int32_t i = INT16_MIN; i <= INT16_MAX; ++i) {
        const int16_t sample = i;

        REQUIRE(
            sample == 
                convert_pcm<PcmFormat::Float, PcmFormat::S16LE>(
                    convert_pcm<PcmFormat::S16LE, PcmFormat::Float>(
                        sample
                    )
                )
        );

        REQUIRE(
            sample ==
                convert_pcm<PcmFormat::Double, PcmFormat::S16LE>(
                    convert_pcm<PcmFormat::S16LE, PcmFormat::Double>(
                        sample
                    )
                )
        );

        REQUIRE(
            convert_pcm<PcmFormat::S16LE, PcmFormat::S32LE>(sample) == 
                convert_pcm<PcmFormat::Float, PcmFormat::S32LE>(
                    convert_pcm<PcmFormat::S16LE, PcmFormat::Float>(
                        sample
                    )
                )
        );

        REQUIRE(
            convert_pcm<PcmFormat::S16LE, PcmFormat::S32LE>(sample) ==
                convert_pcm<PcmFormat::Double, PcmFormat::S32LE>(
                    convert_pcm<PcmFormat::S16LE, PcmFormat::Double>(
                        sample
                    )
                )
        );

        REQUIRE(
            convert_pcm<PcmFormat::S16LE, PcmFormat::S24of32LE>(sample) ==
                convert_pcm<PcmFormat::S16LE, PcmFormat::S24LE>(sample).int_value()
        );
    }
}
