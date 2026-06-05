// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_PNSEQUENCEGENERATOR_HPP
#define GNURADIO4_QRAD_PNSEQUENCEGENERATOR_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/annotated.hpp>
#include <gnuradio-4.0/qradiolink/detail/PnSequenceGenerator.hpp>

#include <complex>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace gnuradio4::qradiolink {

GR_REGISTER_BLOCK(gnuradio4::qradiolink::PnSequenceGenerator)

struct PnSequenceGenerator : gr::Block<PnSequenceGenerator> {
    using Description = gr::Doc<"Streams bipolar m-sequences as complex {Re,0} chips (preset 127/511/1023 length).">;

    gr::PortOut<std::complex<float>>                out{};
    gr::Annotated<gr::Size_t, "preset_length_bits_u"> preset_length_bits_u{127U};

    GR_MAKE_REFLECTABLE(PnSequenceGenerator, out, preset_length_bits_u);

private:
    std::vector<int> _bipolar_table{};
    std::size_t      _sequence_offset{0UZ};

    void load_pattern() noexcept
    {
        const unsigned len_sel = static_cast<unsigned>(preset_length_bits_u);
        if (len_sel == 511U) {
            _bipolar_table = detail::pnMsequence511();
        } else if (len_sel == 1023U) {
            _bipolar_table = detail::pnMsequence1023();
        } else {
            _bipolar_table = detail::pnMsequence127();
        }
        _sequence_offset = 0UZ;
    }

public:
    void start() noexcept { load_pattern(); }

    void settingsChanged(const gr::property_map&, const gr::property_map& neo) noexcept
    {
        if (neo.contains("preset_length_bits_u")) {
            load_pattern();
        }
    }

    [[nodiscard]] gr::work::Status processBulk(gr::OutputSpanLike auto& output_span_chunk) noexcept
    {
        if (_bipolar_table.empty()) {
            output_span_chunk.publish(0UZ);
            return gr::work::Status::OK;
        }
        const std::size_t publish_len = output_span_chunk.size();
        for (std::size_t n = 0; n < publish_len; ++n) {
            const int chip           = _bipolar_table[_sequence_offset % _bipolar_table.size()];
            output_span_chunk[static_cast<std::ptrdiff_t>(n)] = std::complex<float>{static_cast<float>(chip), 0.F};
            _sequence_offset++;
        }
        output_span_chunk.publish(publish_len);
        return gr::work::Status::OK;
    }
};

} // namespace gnuradio4::qradiolink

#endif
