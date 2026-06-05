// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_INCLUDE_QRAD_WRAP_HPP
#define GNURADIO4_INCLUDE_QRAD_WRAP_HPP

/**
 * Convenience umbrella for qradiolink-style GNU Radio 4 header blocks.
 * Block implementations live under `qradiolink/`; helpers under `qradiolink/detail/`
 * remain included indirectly by individual blocks only.
 */

#include <gnuradio-4.0/qradiolink/ClipperCc.hpp>
#include <gnuradio-4.0/qradiolink/Demod2Fsk.hpp>
#include <gnuradio-4.0/qradiolink/Demod4Fsk.hpp>
#include <gnuradio-4.0/qradiolink/Demod8Fsk.hpp>
#include <gnuradio-4.0/qradiolink/DemodAm.hpp>
#include <gnuradio-4.0/qradiolink/DemodBpsk.hpp>
#include <gnuradio-4.0/qradiolink/DemodDmr.hpp>
#include <gnuradio-4.0/qradiolink/DemodDpmr.hpp>
#include <gnuradio-4.0/qradiolink/DemodFreeDv.hpp>
#include <gnuradio-4.0/qradiolink/DemodGmsk.hpp>
#include <gnuradio-4.0/qradiolink/DemodM17.hpp>
#include <gnuradio-4.0/qradiolink/DemodMmdvmMulti.hpp>
#include <gnuradio-4.0/qradiolink/DemodMmdvmMulti2.hpp>
#include <gnuradio-4.0/qradiolink/DemodNbfm.hpp>
#include <gnuradio-4.0/qradiolink/DemodNxdn.hpp>
#include <gnuradio-4.0/qradiolink/DemodQpsk.hpp>
#include <gnuradio-4.0/qradiolink/DemodSoqpsk.hpp>
#include <gnuradio-4.0/qradiolink/DemodSsb.hpp>
#include <gnuradio-4.0/qradiolink/DemodWbfm.hpp>
#include <gnuradio-4.0/qradiolink/DstarDecoder.hpp>
#include <gnuradio-4.0/qradiolink/DstarEncoder.hpp>
#include <gnuradio-4.0/qradiolink/DsssCdmaReceiverCc.hpp>
#include <gnuradio-4.0/qradiolink/DsssCdmaTransmitterCc.hpp>
#include <gnuradio-4.0/qradiolink/DsssDecoderCc.hpp>
#include <gnuradio-4.0/qradiolink/DsssDespreaderCc.hpp>
#include <gnuradio-4.0/qradiolink/DsssEncoderBb.hpp>
#include <gnuradio-4.0/qradiolink/DsssSpreaderCc.hpp>
#include <gnuradio-4.0/qradiolink/GdssDespreaderCc.hpp>
#include <gnuradio-4.0/qradiolink/GdssSpreaderCc.hpp>
#include <gnuradio-4.0/qradiolink/GrIdentModeControl.hpp>
#if defined(GR_QRAD_GR4_HAVE_ZMQ)
#include <gnuradio-4.0/qradiolink/GrIdentPreambleSub.hpp>
#endif
#include <gnuradio-4.0/qradiolink/Gr4FskDiscriminator.hpp>
#include <gnuradio-4.0/qradiolink/ModDemodSwitch.hpp>
#include <gnuradio-4.0/qradiolink/InterleaverBb.hpp>
#include <gnuradio-4.0/qradiolink/M17Coder.hpp>
#include <gnuradio-4.0/qradiolink/M17Decoder.hpp>
#include <gnuradio-4.0/qradiolink/M17Deframer.hpp>
#include <gnuradio-4.0/qradiolink/MmdvmSink.hpp>
#include <gnuradio-4.0/qradiolink/MmdvmSource.hpp>
#include <gnuradio-4.0/qradiolink/Mod2Fsk.hpp>
#include <gnuradio-4.0/qradiolink/Mod4Fsk.hpp>
#include <gnuradio-4.0/qradiolink/Mod8Fsk.hpp>
#include <gnuradio-4.0/qradiolink/ModAm.hpp>
#include <gnuradio-4.0/qradiolink/ModBpsk.hpp>
#include <gnuradio-4.0/qradiolink/ModCpm4Fsk.hpp>
#include <gnuradio-4.0/qradiolink/ModDmr.hpp>
#include <gnuradio-4.0/qradiolink/ModDpmr.hpp>
#include <gnuradio-4.0/qradiolink/ModFreeDv.hpp>
#include <gnuradio-4.0/qradiolink/ModGmsk.hpp>
#include <gnuradio-4.0/qradiolink/ModM17.hpp>
#include <gnuradio-4.0/qradiolink/ModMmdvmMulti2.hpp>
#include <gnuradio-4.0/qradiolink/ModNbfm.hpp>
#include <gnuradio-4.0/qradiolink/ModNxdn.hpp>
#include <gnuradio-4.0/qradiolink/ModQpsk.hpp>
#include <gnuradio-4.0/qradiolink/ModSoqpsk.hpp>
#include <gnuradio-4.0/qradiolink/ModSsb.hpp>
#include <gnuradio-4.0/qradiolink/ModWbfm.hpp>
#include <gnuradio-4.0/qradiolink/P25Decoder.hpp>
#include <gnuradio-4.0/qradiolink/P25Encoder.hpp>
#include <gnuradio-4.0/qradiolink/PnSequenceGenerator.hpp>
#include <gnuradio-4.0/qradiolink/PocsagDecoder.hpp>
#include <gnuradio-4.0/qradiolink/PocsagEncoder.hpp>
#include <gnuradio-4.0/qradiolink/RssiTagBlock.hpp>
#include <gnuradio-4.0/qradiolink/StretcherCc.hpp>
#include <gnuradio-4.0/qradiolink/YsfDecoder.hpp>
#include <gnuradio-4.0/qradiolink/YsfEncoder.hpp>
#include <gnuradio-4.0/qradiolink/ZeroIdleBursts.hpp>

#endif
