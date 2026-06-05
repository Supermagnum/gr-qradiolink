// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_MODDEMODSWITCH_HPP
#define GNURADIO4_QRAD_MODDEMODSWITCH_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/annotated.hpp>
#include <gnuradio-4.0/qradiolink/detail/GrIdentModeMap.hpp>
#include <gnuradio-4.0/qradiolink/detail/SimpleDigitalAnalog.hpp>

#include <complex>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace gnuradio4::qradiolink {

GR_REGISTER_BLOCK(gnuradio4::qradiolink::ModDemodSwitchRx)
GR_REGISTER_BLOCK(gnuradio4::qradiolink::ModDemodSwitchTx)

struct ModDemodSwitchRx : gr::Block<ModDemodSwitchRx> {
    using Description =
        gr::Doc<"IQ demod bank switched by gr-ident active_demod_route (from GrIdentModeControl or manual property).">;

    gr::PortIn<std::complex<float>>  in{};
    gr::PortOut<float>               audio_out{};
    gr::Annotated<std::uint8_t, "active_demod_route", gr::Doc<"detail::QrModemRoute; set by GrIdentModeControl.">>
        active_demod_route{static_cast<std::uint8_t>(detail::QrModemRoute::DemodNbfm)};
    gr::Annotated<float, "sample_rate_hz"> sample_rate_hz{48000.F};
    gr::Annotated<float, "carrier_hz"> carrier_hz{0.F};
    gr::Annotated<float, "nbfm_deviation_hz"> nbfm_deviation_hz{2500.F};
    gr::Annotated<float, "wbfm_peak_deviation_hz"> wbfm_peak_deviation_hz{75000.F};
    gr::Annotated<gr::Size_t, "fsk_samples_per_symbol"> fsk_samples_per_symbol{10U};
    gr::Annotated<float, "fsk_frequency_separation_hz"> fsk_frequency_separation_hz{2400.F};
    gr::Annotated<bool, "hold_when_unknown", gr::Doc<"Zero audio when route is Unknown.">>
        hold_when_unknown{true};

    GR_MAKE_REFLECTABLE(ModDemodSwitchRx,
        in,
        audio_out,
        active_demod_route,
        sample_rate_hz,
        carrier_hz,
        nbfm_deviation_hz,
        wbfm_peak_deviation_hz,
        fsk_samples_per_symbol,
        fsk_frequency_separation_hz,
        hold_when_unknown);

private:
    detail::AnalogModemState _modem{};

public:
    [[nodiscard]] gr::work::Status processBulk(std::span<const std::complex<float>>& in_span,
        std::span<float>& out_span) noexcept
    {
        if (out_span.size() < in_span.size()) {
            return gr::work::Status::INSUFFICIENT_OUTPUT_ITEMS;
        }
        const auto route = static_cast<detail::QrModemRoute>(active_demod_route.value);
        if (route == detail::QrModemRoute::Unknown && hold_when_unknown) {
            for (std::size_t i = 0; i < in_span.size(); ++i) {
                out_span[i] = 0.F;
            }
            return gr::work::Status::OK;
        }

        auto audio = out_span.subspan(0UZ, in_span.size());
        switch (route) {
        case detail::QrModemRoute::DemodAm:
            detail::analogDemodSamples(
                detail::AnalogKind::Am, in_span, audio, sample_rate_hz.value, carrier_hz.value, 1.F, 1.F, _modem);
            break;
        case detail::QrModemRoute::DemodSsbUsb:
            detail::analogDemodSamples(
                detail::AnalogKind::SsbUsb, in_span, audio, sample_rate_hz.value, carrier_hz.value, 1.F, 100.F, _modem);
            break;
        case detail::QrModemRoute::DemodSsbLsb:
            detail::analogDemodSamples(
                detail::AnalogKind::SsbUsb, in_span, audio, sample_rate_hz.value, carrier_hz.value, -1.F, 100.F, _modem);
            break;
        case detail::QrModemRoute::DemodWbfm:
            detail::analogDemodSamples(detail::AnalogKind::Wbfm,
                in_span,
                audio,
                sample_rate_hz.value,
                carrier_hz.value,
                1.F,
                wbfm_peak_deviation_hz.value,
                _modem);
            break;
        case detail::QrModemRoute::DemodNbfm:
        default:
            detail::analogDemodSamples(detail::AnalogKind::Nbfm,
                in_span,
                audio,
                sample_rate_hz.value,
                carrier_hz.value,
                1.F,
                nbfm_deviation_hz.value,
                _modem);
            break;
        case detail::QrModemRoute::Demod2Fsk:
        case detail::QrModemRoute::Demod4Fsk:
        case detail::QrModemRoute::Demod8Fsk:
        case detail::QrModemRoute::DemodBpsk:
        case detail::QrModemRoute::DemodQpsk:
        case detail::QrModemRoute::DemodGmsk:
        case detail::QrModemRoute::DemodDmr:
        case detail::QrModemRoute::DemodDpmr:
        case detail::QrModemRoute::DemodNxdn:
        case detail::QrModemRoute::DemodM17:
        case detail::QrModemRoute::DemodFreeDv:
        case detail::QrModemRoute::DemodMmdvmMulti:
        case detail::QrModemRoute::DemodDstar:
        case detail::QrModemRoute::DemodYsf:
        case detail::QrModemRoute::DemodP25: {
            const std::size_t sps =
                fsk_samples_per_symbol.value < gr::Size_t{1U} ? 1UZ : static_cast<std::size_t>(fsk_samples_per_symbol.value);
            const detail::DigitalKind dk =
                (route == detail::QrModemRoute::Demod2Fsk) ? detail::DigitalKind::Fsk2 : detail::DigitalKind::Fsk4;
            const std::size_t nsym = in_span.size() / sps;
            std::vector<std::uint8_t> bits(nsym);
            std::span<std::uint8_t>   bit_span(bits.data(), bits.size());
            detail::digitalDemodulateHardBits(dk, in_span, bit_span, sps, fsk_frequency_separation_hz.value);
            for (std::size_t i = 0; i < audio.size(); ++i) {
                const std::size_t sym = std::min(i / sps, bits.empty() ? 0UZ : bits.size() - 1UZ);
                audio[i]              = bits.empty() ? 0.F : (bits[sym] ? 0.5F : -0.5F);
            }
            break;
        }
        }
        return gr::work::Status::OK;
    }
};

struct ModDemodSwitchTx : gr::Block<ModDemodSwitchTx> {
    using Description =
        gr::Doc<"Byte/modem bank switched by gr-ident active_mod_route (companion to ModDemodSwitchRx on transmit).">;

    gr::PortIn<std::uint8_t>           in{};
    gr::PortOut<std::complex<float>>   out{};
    gr::Annotated<std::uint8_t, "active_mod_route"> active_mod_route{static_cast<std::uint8_t>(detail::QrModemRoute::ModNbfm)};
    gr::Annotated<float, "sample_rate_hz"> sample_rate_hz{48000.F};
    gr::Annotated<float, "carrier_offset_hz"> carrier_offset_hz{0.F};
    gr::Annotated<float, "nbfm_deviation_hz"> nbfm_deviation_hz{2500.F};
    gr::Annotated<gr::Size_t, "fsk_samples_per_symbol"> fsk_samples_per_symbol{10U};
    gr::Annotated<float, "fsk_frequency_separation_hz"> fsk_frequency_separation_hz{2400.F};

    GR_MAKE_REFLECTABLE(ModDemodSwitchTx,
        in,
        out,
        active_mod_route,
        sample_rate_hz,
        carrier_offset_hz,
        nbfm_deviation_hz,
        fsk_samples_per_symbol,
        fsk_frequency_separation_hz);

private:
    detail::AnalogModemState _modem{};

public:
    [[nodiscard]] gr::work::Status processBulk(std::span<const std::uint8_t>& in_span,
        std::span<std::complex<float>>& out_span) noexcept
    {
        const auto route = static_cast<detail::QrModemRoute>(active_mod_route.value);
        const std::size_t sps = fsk_samples_per_symbol.value < gr::Size_t{1U}
                                    ? 1UZ
                                    : static_cast<std::size_t>(fsk_samples_per_symbol.value);

        switch (route) {
        case detail::QrModemRoute::Mod2Fsk:
        case detail::QrModemRoute::Mod4Fsk:
        case detail::QrModemRoute::Mod8Fsk:
        case detail::QrModemRoute::ModBpsk:
        case detail::QrModemRoute::ModQpsk:
        case detail::QrModemRoute::ModGmsk:
        case detail::QrModemRoute::ModDmr:
        case detail::QrModemRoute::ModDpmr:
        case detail::QrModemRoute::ModNxdn:
        case detail::QrModemRoute::ModM17:
        case detail::QrModemRoute::ModFreeDv:
        case detail::QrModemRoute::ModMmdvmMulti: {
            const detail::DigitalKind dk =
                (route == detail::QrModemRoute::Mod2Fsk) ? detail::DigitalKind::Fsk2 : detail::DigitalKind::Fsk4;
            const std::size_t need = in_span.size() * sps;
            if (out_span.size() < need) {
                return gr::work::Status::INSUFFICIENT_OUTPUT_ITEMS;
            }
            detail::digitalModulateWaveform(dk,
                in_span,
                out_span.subspan(0UZ, need),
                sps,
                sample_rate_hz.value,
                carrier_offset_hz.value,
                fsk_frequency_separation_hz.value,
                _modem);
            return gr::work::Status::OK;
        }
        case detail::QrModemRoute::ModAm:
        case detail::QrModemRoute::ModSsb:
        case detail::QrModemRoute::ModNbfm:
        default: {
            const std::size_t need = in_span.size() * sps;
            if (out_span.size() < need) {
                return gr::work::Status::INSUFFICIENT_OUTPUT_ITEMS;
            }
            std::vector<std::uint8_t> expanded(need, 128U);
            for (std::size_t i = 0; i < in_span.size(); ++i) {
                const std::uint8_t v = static_cast<std::uint8_t>(in_span[i]);
                for (std::size_t k = 0; k < sps; ++k) {
                    expanded[i * sps + k] = v;
                }
            }
            const auto kind = (route == detail::QrModemRoute::ModAm) ? detail::AnalogKind::Am : detail::AnalogKind::Nbfm;
            detail::digitalModulateWaveform(detail::DigitalKind::Fsk2,
                expanded,
                out_span.subspan(0UZ, need),
                1UZ,
                sample_rate_hz.value,
                carrier_offset_hz.value,
                nbfm_deviation_hz.value,
                _modem);
            (void)kind;
            return gr::work::Status::OK;
        }
        }
    }
};

} // namespace gnuradio4::qradiolink

#endif
