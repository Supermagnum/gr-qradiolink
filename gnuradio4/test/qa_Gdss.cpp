// SPDX-License-Identifier: GPL-3.0-or-later
#include <boost/ut.hpp>

#include <gnuradio-4.0/Sequence.hpp>
#include <gnuradio-4.0/qradiolink/GdssDespreaderCc.hpp>
#include <gnuradio-4.0/qradiolink/GdssSpreaderCc.hpp>

#include <cmath>
#include <complex>
#include <cstddef>
#include <span>
#include <vector>

namespace {

const boost::ut::suite qr_gdss_pair_smoke = [] {
    using namespace boost::ut;
    using gnuradio4::qradiolink::GdssDespreaderCc;
    using gnuradio4::qradiolink::GdssSpreaderCc;

    "Gdss spread+despread round-trip tracks complex transmit symbol"_test = [] {
        auto seq_here = std::make_shared<gr::Sequence>();

        GdssSpreaderCc spread{};
        GdssDespreaderCc desp{};
        spread.init(seq_here);
        desp.init(seq_here);

        constexpr std::uint64_t pairing_seed_here = UINT64_C(0xABCDEF01ABCDEF02);
        spread.spread_sequence_bins               = static_cast<gr::Size_t>(112U);
        desp.spread_sequence_bins               = spread.spread_sequence_bins;
        spread.repeat_chip_factor_u               = static_cast<gr::Size_t>(12U);
        desp.repeat_chip_factor_u               = spread.repeat_chip_factor_u;

        constexpr float variance_estimate_qa = 1.55F;
        spread.spread_variance_estimate      = variance_estimate_qa;
        desp.spread_variance_estimate        = variance_estimate_qa;

        spread.rng_seed_u64_locked = pairing_seed_here;
        desp.rng_seed_u64_locked   = pairing_seed_here;

        spread.start();
        desp.start();

        const std::complex<float> tx_sym_here{3.125F, -1.9375F};

        std::vector<std::complex<float>> bursts(static_cast<std::size_t>(spread.output_chunk_size));
        std::span<const std::complex<float>> in_sym(&tx_sym_here, 1UZ);
        std::span<std::complex<float>>       mw(bursts.data(), bursts.size());

        expect(spread.processBulk(in_sym, mw) == gr::work::Status::OK);

        std::vector<std::complex<float>> collapsed(1UZ);
        std::span<const std::complex<float>> read_back(bursts.data(), bursts.size());
        std::span<std::complex<float>>       reconstructed(collapsed.data(), collapsed.size());

        expect(desp.processBulk(read_back, reconstructed) == gr::work::Status::OK);

        constexpr float abs_tol_here = 0.85F;
        expect(le(std::fabs(collapsed[0].real() - tx_sym_here.real()), abs_tol_here));
        expect(le(std::fabs(collapsed[0].imag() - tx_sym_here.imag()), abs_tol_here));
    };
};

} // namespace

int main() { return boost::ut::cfg<boost::ut::override>.run(); }
