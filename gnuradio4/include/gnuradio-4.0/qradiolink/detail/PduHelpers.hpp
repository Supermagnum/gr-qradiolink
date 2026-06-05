// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_DETAIL_PDUHELPERS_HPP
#define GNURADIO4_QRAD_DETAIL_PDUHELPERS_HPP

#include <gnuradio-4.0/Tensor.hpp>
#include <gnuradio-4.0/Tag.hpp>
#include <gnuradio-4.0/Value.hpp>

#include <cstddef>
#include <string_view>

namespace gnuradio4::qradiolink::detail {

[[nodiscard]] inline const gr::Tensor<std::uint8_t>* tensorBytesFromMap(const gr::property_map& map, std::string_view keyView)
{
    const auto it = map.find(gr::convert_string_domain(keyView));
    if (it == map.end()) {
        return nullptr;
    }
    return it->second.get_if<gr::Tensor<std::uint8_t>>();
}

} // namespace gnuradio4::qradiolink::detail

#endif
