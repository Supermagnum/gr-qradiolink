// SPDX-License-Identifier: GPL-3.0-or-later
#include <boost/ut.hpp>

#include <gnuradio-4.0/Sequence.hpp>
#include <gnuradio-4.0/qradiolink/PnSequenceGenerator.hpp>
#include <gnuradio-4.0/qradiolink/detail/PnSequenceGenerator.hpp>

#include <cmath>
#include <cstddef>
#include <span>
#include <vector>

namespace {

const boost::ut::suite qr_pn_stream_smoke_tests = [] {
    using namespace boost::ut;

    using gnuradio4::qradiolink::PnSequenceGenerator;

    "PnSequenceGenerator default m127 stream matches detail template"_test = [] {
        PnSequenceGenerator generator{};
        generator.init(std::make_shared<gr::Sequence>());
        generator.start();

        const std::vector<int> bipolar_ref = gnuradio4::qradiolink::detail::pnMsequence127();

        constexpr std::size_t take = 48UZ;
        auto                  wr   = generator.out.tryReserve<gr::SpanReleasePolicy::ProcessAll>(take);

        expect(generator.processBulk(wr) == gr::work::Status::OK);
        expect(eq(static_cast<std::size_t>(wr.size()), take));

        for (std::size_t r = 0; r < take && r < bipolar_ref.size(); ++r) {
            const float re_line = wr[static_cast<std::ptrdiff_t>(r)].real();
            expect(le(std::fabs(re_line - static_cast<float>(bipolar_ref[r])), 5.F / 8192.F));
            expect(eq(wr[static_cast<std::ptrdiff_t>(r)].imag(), 0.F));
        }
    };

    "preset_length_bits_u=511 swaps LFSR length"_test = [] {
        PnSequenceGenerator generator511{};
        generator511.init(std::make_shared<gr::Sequence>());
        generator511.preset_length_bits_u = static_cast<gr::Size_t>(511U);
        generator511.start();

        const std::vector<int> ref511_track = gnuradio4::qradiolink::detail::pnMsequence511();

        auto wr_here = generator511.out.tryReserve<gr::SpanReleasePolicy::ProcessAll>(5UZ);
        expect(generator511.processBulk(wr_here) == gr::work::Status::OK);
        expect(le(std::fabs(wr_here[0UZ].real() - static_cast<float>(ref511_track[0])), 5.F / 8192.F));
        expect(eq(wr_here[0UZ].imag(), 0.F));
    };
};

} // namespace

int main() { return boost::ut::cfg<boost::ut::override>.run(); }
