// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_INTERLEAVERBB_HPP
#define GNURADIO4_QRAD_INTERLEAVERBB_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/annotated.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace gnuradio4::qradiolink {

GR_REGISTER_BLOCK(gnuradio4::qradiolink::InterleaverBb)

struct InterleaverBb : gr::Block<InterleaverBb, gr::Resampling<1UZ, 1UZ, false>> {
    using Description = gr::Doc<"Block byte interleaver with runtime rows/cols (mirrors unpacked bb behaviour).">;

    gr::PortIn<std::uint8_t>                    in{};
    gr::PortOut<std::uint8_t>                 out{};
    gr::Annotated<gr::Size_t, "row_count_ui"> row_count_ui{8U};
    gr::Annotated<gr::Size_t, "column_count_ui"> column_count_ui{8U};
    gr::Annotated<bool, "interleave_enable_bool"> interleave_enable_bool = true;

    GR_MAKE_REFLECTABLE(InterleaverBb, in, out, row_count_ui, column_count_ui, interleave_enable_bool);

private:
    std::vector<std::size_t> _permute_input_indexes{};

    void rebuild_lut() noexcept
    {
        const unsigned n_rows_gui = row_count_ui < static_cast<gr::Size_t>(1U)
                                        ? static_cast<unsigned>(1U)
                                        : static_cast<unsigned>(row_count_ui);
        const unsigned n_cols_gui = column_count_ui < static_cast<gr::Size_t>(1U)
                                        ? static_cast<unsigned>(1U)
                                        : static_cast<unsigned>(column_count_ui);
        const unsigned n_rows_eff = std::max(1U, n_rows_gui);
        const unsigned n_cols_eff = std::max(1U, n_cols_gui);

        const std::size_t block_bytes_sq = static_cast<std::size_t>(n_rows_eff) * static_cast<std::size_t>(n_cols_eff);
        this->input_chunk_size           = static_cast<gr::Size_t>(block_bytes_sq == 0UZ ? 1UZ : block_bytes_sq);
        this->output_chunk_size          = this->input_chunk_size;

        _permute_input_indexes.assign(block_bytes_sq, 0UZ);
        for (std::size_t out_idx_sq = 0; out_idx_sq < block_bytes_sq; ++out_idx_sq) {
            if (interleave_enable_bool) {
                const unsigned column_write = static_cast<unsigned>(out_idx_sq / static_cast<std::size_t>(n_rows_eff));
                const unsigned row_write =
                    static_cast<unsigned>(out_idx_sq % static_cast<std::size_t>(n_rows_eff));
                const unsigned in_idx_u =
                    row_write * n_cols_eff + column_write;
                _permute_input_indexes[out_idx_sq] = static_cast<std::size_t>(in_idx_u);
            } else {
                const unsigned row_write = static_cast<unsigned>(out_idx_sq / static_cast<std::size_t>(n_cols_eff));
                const unsigned col_write =
                    static_cast<unsigned>(out_idx_sq % static_cast<std::size_t>(n_cols_eff));
                const unsigned in_idx_reverse =
                    static_cast<unsigned>(col_write * n_rows_eff + row_write);
                _permute_input_indexes[out_idx_sq] = static_cast<std::size_t>(in_idx_reverse);
            }
        }
    }

public:

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& new_map) noexcept
    {
        if (new_map.contains("row_count_ui") || new_map.contains("column_count_ui") || new_map.contains("interleave_enable_bool")) {
            rebuild_lut();
        }
    }

    void start() noexcept { rebuild_lut(); }

    [[nodiscard]] gr::work::Status processBulk(std::span<const std::uint8_t>& ins_chunk, std::span<std::uint8_t>& out_chunk_inout) noexcept
    {
        if (_permute_input_indexes.size() != ins_chunk.size() || ins_chunk.size() != out_chunk_inout.size()) {
            return gr::work::Status::ERROR;
        }
        for (std::size_t out_ix = 0; out_ix < _permute_input_indexes.size(); ++out_ix) {
            out_chunk_inout[out_ix] = ins_chunk[_permute_input_indexes[out_ix]];
        }
        return gr::work::Status::OK;
    }
};

} // namespace gnuradio4::qradiolink

#endif
