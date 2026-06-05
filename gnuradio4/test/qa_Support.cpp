// SPDX-License-Identifier: GPL-3.0-or-later
#include <boost/ut.hpp>

#include <gnuradio-4.0/Sequence.hpp>
#include <gnuradio-4.0/Tag.hpp>
#include <gnuradio-4.0/qradiolink/ClipperCc.hpp>
#include <gnuradio-4.0/qradiolink/DemodMmdvmMulti.hpp>
#include <gnuradio-4.0/qradiolink/Gr4FskDiscriminator.hpp>
#include <gnuradio-4.0/qradiolink/InterleaverBb.hpp>
#include <gnuradio-4.0/qradiolink/MmdvmSink.hpp>
#include <gnuradio-4.0/qradiolink/MmdvmSource.hpp>
#include <gnuradio-4.0/qradiolink/ModMmdvmMulti2.hpp>
#include <gnuradio-4.0/qradiolink/StretcherCc.hpp>
#include <gnuradio-4.0/qradiolink/ZeroIdleBursts.hpp>

#include <cmath>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace {

const boost::ut::suite qr_support_cluster_smoke = [] {
    using namespace boost::ut;

    using gnuradio4::qradiolink::ClipperCc;
    using gnuradio4::qradiolink::DemodMmdvmMulti;
    using gnuradio4::qradiolink::Gr4FskDiscriminator;
    using gnuradio4::qradiolink::InterleaverBb;
    using gnuradio4::qradiolink::MmdvmSink;
    using gnuradio4::qradiolink::MmdvmSource;
    using gnuradio4::qradiolink::ModMmdvmMulti2;
    using gnuradio4::qradiolink::StretcherCc;
    using gnuradio4::qradiolink::ZeroIdleBursts;

    "ClipperCc clamps IQ magnitude"_test = [] {
        ClipperCc clip_here{};
        clip_here.init(std::make_shared<gr::Sequence>());
        clip_here.threshold_level_amplitude                   = 0.5F;
        std::vector<std::complex<float>> samples_in{std::complex<float>{5.F, -3.F}};
        std::vector<std::complex<float>> samples_out(samples_in.size());

        std::span<const std::complex<float>> s_in(samples_in.data(), samples_in.size());
        std::span<std::complex<float>>       s_out(samples_out.data(), samples_out.size());
        expect(clip_here.processBulk(s_in, s_out) == gr::work::Status::OK);

        const float clipped_mag_here = std::hypot(samples_out[0UZ].real(), samples_out[0UZ].imag());
        expect(ge(0.5F - clipped_mag_here, -1.F * 1.F / 8192.F));
        expect(le(clipped_mag_here, 0.5F));
    };

    "Interleaver forward permutation is undone by reciprocal mode"_test = [] {
        auto seq_here = std::make_shared<gr::Sequence>();
        InterleaverBb coder{};
        InterleaverBb decoder{};
        coder.init(seq_here);
        decoder.init(seq_here);

        coder.row_count_ui          = static_cast<gr::Size_t>(8U);
        coder.column_count_ui       = static_cast<gr::Size_t>(8U);
        coder.interleave_enable_bool = true;
        coder.start();

        decoder.row_count_ui            = coder.row_count_ui;
        decoder.column_count_ui       = coder.column_count_ui;
        decoder.interleave_enable_bool = false;
        decoder.start();

        constexpr std::size_t nbytes = 64UZ;
        std::vector<std::uint8_t> frame(nbytes);
        for (std::size_t b = 0; b < nbytes; ++b) {
            frame[b] = static_cast<std::uint8_t>(b);
        }

        std::vector<std::uint8_t> shuffled_bytes(nbytes);
        std::vector<std::uint8_t> planar_back(nbytes);

        std::span<const std::uint8_t> raw_top(frame.data(), frame.size());
        std::span<std::uint8_t>       shuf_wr(shuffled_bytes.data(), shuffled_bytes.size());
        std::span<const std::uint8_t> read_mid(shuffled_bytes.data(), shuffled_bytes.size());
        std::span<std::uint8_t>       planar_wr(planar_back.data(), planar_back.size());

        expect(coder.processBulk(raw_top, shuf_wr) == gr::work::Status::OK);
        expect(decoder.processBulk(read_mid, planar_wr) == gr::work::Status::OK);

        for (std::size_t zx = 0; zx < nbytes; ++zx) {
            expect(eq(planar_back[zx], frame[zx]));
        }
    };

    "Stretcher stays finite across adjacent samples"_test = [] {
        StretcherCc stretch{};
        stretch.init(std::make_shared<gr::Sequence>());
        std::vector<std::complex<float>> in_pair{std::complex<float>{0.70710677F, 0.70710677F},
            std::complex<float>{0.70710677F, -0.70710677F}};
        std::vector<std::complex<float>> out_pair(in_pair.size());
        std::span<const std::complex<float>> sip(in_pair.data(), in_pair.size());
        std::span<std::complex<float>>       sop(out_pair.data(), out_pair.size());

        expect(stretch.processBulk(sip, sop) == gr::work::Status::OK);
        expect(std::isfinite(sop[0UZ].real()) && std::isfinite(sop[0UZ].imag()));
        (void)sop[out_pair.size() - 1UZ];
    };

    "ZeroIdleBursts clears samples after countdown request"_test = [] {
        ZeroIdleBursts zb{};
        zb.init(std::make_shared<gr::Sequence>());
        zb.requested_idle_zeros = static_cast<gr::Size_t>(3U);
        gr::property_map neo{{"requested_idle_zeros", static_cast<gr::Size_t>(3U)}};
        zb.settingsChanged({}, neo);

        std::vector<std::complex<float>> in_side(8UZ, std::complex<float>{1.F, -0.6F});
        std::vector<std::complex<float>> out_side(in_side.size());
        std::span<const std::complex<float>> isp(in_side.data(), in_side.size());
        std::span<std::complex<float>>       osp(out_side.data(), out_side.size());

        expect(zb.processBulk(isp, osp) == gr::work::Status::OK);
        expect(eq(std::hypot(osp[0UZ].real(), osp[0UZ].imag()), 0.F));
        expect(eq(std::hypot(osp[1UZ].real(), osp[1UZ].imag()), 0.F));
        expect(eq(std::hypot(osp[2UZ].real(), osp[2UZ].imag()), 0.F));
        expect(gt(std::hypot(osp[3UZ].real(), osp[3UZ].imag()), 0.1F));

        expect(zb.processBulk(isp, osp) == gr::work::Status::OK);
    };

    "Gr4 discriminator maps strongest branch0 to southwestern quadrant"_test = [] {
        Gr4FskDiscriminator disc{};
        disc.init(std::make_shared<gr::Sequence>());

        const std::size_t               lane_elems = 1UZ;
        std::vector<float>             b0(lane_elems, 0.9F);
        std::vector<float>             b1(lane_elems, 0.1F);
        std::vector<float>             b2(lane_elems, 0.05F);
        std::vector<float>             b3(lane_elems, 0.04F);
        std::vector<std::complex<float>> quadrant_out(lane_elems);

        std::span<const float>           s0(b0.data(), b0.size());
        std::span<const float>           s1(b1.data(), b1.size());
        std::span<const float>           s2(b2.data(), b2.size());
        std::span<const float>           s3(b3.data(), b3.size());
        std::span<std::complex<float>> quadrant_span(quadrant_out.data(), quadrant_out.size());

        expect(disc.processBulk(s0, s1, s2, s3, quadrant_span) == gr::work::Status::OK);

        constexpr float ref_component = +0.70710677F;
        expect(le(std::fabs(quadrant_out[0UZ].real() + ref_component), 0.02F));
        expect(le(std::fabs(quadrant_out[0UZ].imag() + ref_component), 0.02F));
    };

    "MMDVM stub IQ path executes tryReserve/processBulk"_test = [] {
        MmdvmSource src{};
        MmdvmSink   snk{};
        src.init(std::make_shared<gr::Sequence>());
        snk.init(std::make_shared<gr::Sequence>());

        auto output_chunk = src.out.tryReserve<gr::SpanReleasePolicy::ProcessAll>(7UZ);

        expect(src.processBulk(output_chunk) == gr::work::Status::OK);
        expect(ge(std::fabs(output_chunk[0UZ].real()), 5.1e-4F)); // nominal IQ stub ~0.001

        std::vector<std::complex<float>> xfer(output_chunk.size());
        for (std::size_t m = 0; m < output_chunk.size(); ++m) {
            xfer[m] = output_chunk[m];
        }

        ModMmdvmMulti2 relay{};
        relay.init(std::make_shared<gr::Sequence>());
        std::vector<std::complex<float>> thru(xfer.size());

        std::span<const std::complex<float>> p_i(xfer.data(), xfer.size());
        std::span<std::complex<float>>       p_o(thru.data(), thru.size());

        expect(relay.processBulk(p_i, p_o) == gr::work::Status::OK);

        std::span<const std::complex<float>> snk_ln(thru.data(), thru.size());

        expect(snk.processBulk(snk_ln) == gr::work::Status::OK);

        DemodMmdvmMulti quad{};
        quad.init(std::make_shared<gr::Sequence>());
        std::vector<float> qz0(thru.size(), 0.8F);
        std::vector<float> qz1(thru.size(), 0.12F);
        std::vector<float> qz2(thru.size(), 0.06F);
        std::vector<float> qz3(thru.size(), 0.04F);

        std::vector<std::complex<float>> q_out(thru.size());

        std::span<const float>           span0(qz0.data(), qz0.size());
        std::span<const float>           span1(qz1.data(), qz1.size());
        std::span<const float>           span2(qz2.data(), qz2.size());
        std::span<const float>           span3(qz3.data(), qz3.size());
        std::span<std::complex<float>>   q_ot(q_out.data(), q_out.size());

        expect(quad.processBulk(span0, span1, span2, span3, q_ot) == gr::work::Status::OK);
        expect(gt(std::hypot(q_out[0UZ].real(), q_out[0UZ].imag()), 0.01F));
    };
};
} // namespace

int main() { return boost::ut::cfg<boost::ut::override>.run(); }
