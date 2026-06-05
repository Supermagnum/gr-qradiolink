// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_DSSSENCODERBB_HPP
#define GNURADIO4_QRAD_DSSSENCODERBB_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/annotated.hpp>
#include <gnuradio-4.0/qradiolink/detail/DsssGdssTools.hpp>

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace gnuradio4::qradiolink {

GR_REGISTER_BLOCK(gnuradio4::qradiolink::DsssEncoderBb)

struct DsssEncoderBb : gr::Block<DsssEncoderBb, gr::Resampling<1UZ, 1UZ, false>> {
    using Description = gr::Doc<"DSSS chip expansion: one unpacked input bit becomes an entire PN chip train.">;

    gr::PortIn<std::uint8_t>                    in{};
    gr::PortOut<std::uint8_t>                 out{};
    gr::Annotated<std::string, "spread_bits_ascii_binary"> spread_bits_ascii_binary{std::string("1010")};

    GR_MAKE_REFLECTABLE(DsssEncoderBb, in, out, spread_bits_ascii_binary);

private:
    std::vector<int> _spread_chips{};

    void reload_spread_vector() noexcept
    {
        detail::parse_pn_ascii_binary(spread_bits_ascii_binary, _spread_chips);
        if (_spread_chips.empty()) {
            _spread_chips = {1};
        }
        const gr::Size_t out_len = static_cast<gr::Size_t>(_spread_chips.size());
        this->output_chunk_size = out_len == gr::Size_t{0U} ? gr::Size_t{1U} : out_len;
        this->input_chunk_size  = gr::Size_t{1U};
    }

public:
    void start() noexcept { reload_spread_vector(); }

    void settingsChanged(const gr::property_map&, const gr::property_map& neo) noexcept
    {
        if (neo.contains("spread_bits_ascii_binary")) {
            reload_spread_vector();
        }
    }

    [[nodiscard]] gr::work::Status processBulk(std::span<const std::uint8_t>& in_bit, std::span<std::uint8_t>& out_chips) noexcept
    {
        if (in_bit.size() < 1UZ || out_chips.size() < _spread_chips.size()) {
            return gr::work::Status::INSUFFICIENT_OUTPUT_ITEMS;
        }
        const int input_bit_nn = in_bit[0] != 0U ? 1 : 0;
        for (std::size_t chi = 0; chi < _spread_chips.size(); ++chi) {
            const int chip_value = _spread_chips[chi];
            out_chips[chi] =
                static_cast<std::uint8_t>(static_cast<int>(chip_value) ^ (static_cast<int>(1) - input_bit_nn));
        }
        return gr::work::Status::OK;
    }
};

} // namespace gnuradio4::qradiolink

#endif
