// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_DETAIL_VOICEPDUENGINE_HPP
#define GNURADIO4_QRAD_DETAIL_VOICEPDUENGINE_HPP

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace gnuradio4::qradiolink::detail {

inline constexpr std::uint8_t kTagM17[]   = {'M', '1', '7', ':'};
inline constexpr std::uint8_t kTagM17crc[]       = {'M', '!', '!', '+'};
inline constexpr std::uint8_t kTagM17def[]       = {'M', '!', '!', '#'};

inline constexpr std::uint8_t kTagDm[]    = {'D', 'M', 'r', ':'};
inline constexpr std::uint8_t kTagDpmr[]       = {'d', 'p', '!', ':'};
inline constexpr std::uint8_t kTagNxdn[]       = {'N', 'x', '!', ':'};
inline constexpr std::uint8_t kTagDstar[]       = {'D', '*', '*', ':'};
inline constexpr std::uint8_t kTagYsf[]    = {'Y', 's', 'f', ':'};
inline constexpr std::uint8_t kTagP25[]       = {'P', '2', '5', ':'};
inline constexpr std::uint8_t kTagPo[]       = {'P', '|', '!', '#'};

inline void pduPrependTag(std::vector<std::uint8_t>& payload, const std::uint8_t (&magic)[4])
{
    std::vector<std::uint8_t> merged(payload.size() + static_cast<std::size_t>(4), 0U);
    merged[0] = magic[0];
    merged[1] = magic[1];
    merged[2] = magic[2];
    merged[3] = magic[3];
    std::copy(payload.begin(), payload.end(), merged.begin() + static_cast<std::ptrdiff_t>(4));
    payload.swap(merged);
}

inline bool pduStripTag(std::vector<std::uint8_t>& payload, const std::uint8_t (&magic)[4])
{
    if (payload.size() < static_cast<std::size_t>(4)) {
        return false;
    }
    if (payload[0] != magic[0] || payload[1] != magic[1] || payload[2] != magic[2] || payload[3] != magic[3]) {
        return false;
    }
    payload.erase(payload.begin(), payload.begin() + static_cast<std::ptrdiff_t>(4));
    return true;
}

inline void pduAppendXorChecksum(std::vector<std::uint8_t>& payload)
{
    std::uint8_t x = 0U;
    for (std::uint8_t octet : payload) {
        x ^= octet;
    }
    payload.push_back(x);
}

inline bool pduStripXorChecksum(std::vector<std::uint8_t>& payload)
{
    if (payload.empty()) {
        return false;
    }
    const std::uint8_t chk = payload.back();
    payload.pop_back();
    std::uint8_t x = 0U;
    for (std::uint8_t octet : payload) {
        x ^= octet;
    }
    return x == chk;
}

inline void pduApplyM17Encoder(std::vector<std::uint8_t>& payload)
{
    pduAppendXorChecksum(payload);
    pduPrependTag(payload, kTagM17crc);
}

inline bool pduApplyM17Decode(std::vector<std::uint8_t>& payload)
{
    if (!pduStripTag(payload, kTagM17crc)) {
        return false;
    }
    return pduStripXorChecksum(payload);
}

inline void pduM17WrapFrame(std::vector<std::uint8_t>& payload) { pduPrependTag(payload, kTagM17def); }
inline bool pduM17UnwrapFrame(std::vector<std::uint8_t>& payload) { return pduStripTag(payload, kTagM17def); }

inline constexpr std::uint8_t kTagFreedv[] = {'F', '|', '!', '#'};

inline void pduFdMod(std::vector<std::uint8_t>& payload)
{
#ifdef GR_QRAD_GR4_HAVE_CODEC2
    (void)payload;
#else
    pduPrependTag(payload, kTagFreedv);
#endif
}

inline bool pduFdDem(std::vector<std::uint8_t>& payload)
{
#ifdef GR_QRAD_GR4_HAVE_CODEC2
    (void)payload;
    return true;
#else
    return pduStripTag(payload, kTagFreedv);
#endif
}

inline bool pduFdCodecStub(std::vector<std::uint8_t>& payload)
{
#ifdef GR_QRAD_GR4_HAVE_CODEC2
    (void)payload;
    return true;
#else
    return true;
#endif
}

inline void pduApplyM17Mod(std::vector<std::uint8_t>& payload) { pduPrependTag(payload, kTagM17); }
inline bool pduApplyM17Demod(std::vector<std::uint8_t>& payload) { return pduStripTag(payload, kTagM17); }

inline void pduApplyDmEncode(std::vector<std::uint8_t>& payload) { pduPrependTag(payload, kTagDm); }
inline bool pduApplyDmDecode(std::vector<std::uint8_t>& payload) { return pduStripTag(payload, kTagDm); }

inline void pduApplyDpmEncode(std::vector<std::uint8_t>& payload) { pduPrependTag(payload, kTagDpmr); }
inline bool pduApplyDpmDecode(std::vector<std::uint8_t>& payload) { return pduStripTag(payload, kTagDpmr); }

inline void pduApplyNxEncode(std::vector<std::uint8_t>& payload) { pduPrependTag(payload, kTagNxdn); }
inline bool pduApplyNxDecode(std::vector<std::uint8_t>& payload) { return pduStripTag(payload, kTagNxdn); }

inline void pduApplyDstEncode(std::vector<std::uint8_t>& payload) { pduPrependTag(payload, kTagDstar); }
inline bool pduApplyDstDecode(std::vector<std::uint8_t>& payload) { return pduStripTag(payload, kTagDstar); }

inline void pduApplyYsfEncode(std::vector<std::uint8_t>& payload) { pduPrependTag(payload, kTagYsf); }
inline bool pduApplyYsfDecode(std::vector<std::uint8_t>& payload) { return pduStripTag(payload, kTagYsf); }

inline void pduApplyP25Encode(std::vector<std::uint8_t>& payload) { pduPrependTag(payload, kTagP25); }
inline bool pduApplyP25Decode(std::vector<std::uint8_t>& payload) { return pduStripTag(payload, kTagP25); }

inline void pduApplyPocaEncode(std::vector<std::uint8_t>& payload) { pduPrependTag(payload, kTagPo); }
inline bool pduApplyPocaDecode(std::vector<std::uint8_t>& payload) { return pduStripTag(payload, kTagPo); }

} // namespace gnuradio4::qradiolink::detail

#endif
