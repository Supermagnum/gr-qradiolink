// SPDX-License-Identifier: GPL-3.0-or-later
#include <boost/ut.hpp>

#include <gnuradio-4.0/Sequence.hpp>
#include <gnuradio-4.0/qradiolink/DsssEncoderBb.hpp>

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>

namespace {

const boost::ut::suite qr_dsss_bb = [] {
    using namespace boost::ut;

    using gnuradio4::qradiolink::DsssEncoderBb;

    "DSSS encoder toggles chip train polarity with unpacked input bit"_test = [] {
        const auto chips_for_bit = [](std::uint8_t unpacked01) -> std::vector<std::uint8_t> {
            auto                 seq_here = std::make_shared<gr::Sequence>();
            DsssEncoderBb        enc{};
            enc.init(seq_here);
            enc.spread_bits_ascii_binary = std::string("1010");
            enc.start();

            std::vector<std::uint8_t> bit_lane{std::vector<std::uint8_t>{unpacked01}};
            const std::size_t         outp = static_cast<std::size_t>(enc.output_chunk_size);
            std::vector<std::uint8_t> train(outp);

            std::span<const std::uint8_t> ins(bit_lane.data(), bit_lane.size());
            std::span<std::uint8_t>       outs(train.data(), train.size());
            expect(enc.processBulk(ins, outs) == gr::work::Status::OK);
            return train;
        };

        const std::vector<std::uint8_t> chips_zero_v = chips_for_bit(0U);
        const std::vector<std::uint8_t> chips_one_z  = chips_for_bit(1U);

        std::ignore = chips_zero_v;
        unsigned mismatch_count_here = 0U;
        for (std::size_t i = 0; i < chips_zero_v.size() && i < chips_one_z.size(); ++i) {
            if (chips_zero_v[i] != chips_one_z[i]) {
                ++mismatch_count_here;
            }
        }

        expect(eq(chips_zero_v.size(), chips_one_z.size()));
        expect(ge(mismatch_count_here, 1U));
    };
};

} // namespace

int main() { return boost::ut::cfg<boost::ut::override>.run(); }
