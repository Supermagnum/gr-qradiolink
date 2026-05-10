#define BOOST_TEST_MODULE qa_gr4_smoke
#include <boost/test/unit_test.hpp>
#include <gnuradio-4.0/qradiolink/clipper.hpp>
#include <gnuradio-4.0/qradiolink/demod_2fsk.hpp>
#include <gnuradio-4.0/qradiolink/demod_4fsk.hpp>
#include <gnuradio-4.0/qradiolink/demod_8fsk.hpp>
#include <gnuradio-4.0/qradiolink/demod_am.hpp>
#include <gnuradio-4.0/qradiolink/demod_bpsk.hpp>
#include <gnuradio-4.0/qradiolink/demod_dmr.hpp>
#include <gnuradio-4.0/qradiolink/demod_dpmr.hpp>
#include <gnuradio-4.0/qradiolink/demod_gmsk.hpp>
#include <gnuradio-4.0/qradiolink/demod_nbfm.hpp>
#include <gnuradio-4.0/qradiolink/demod_nxdn.hpp>
#include <gnuradio-4.0/qradiolink/demod_qpsk.hpp>
#include <gnuradio-4.0/qradiolink/demod_soqpsk.hpp>
#include <gnuradio-4.0/qradiolink/demod_ssb.hpp>
#include <gnuradio-4.0/qradiolink/demod_wbfm.hpp>
#include <gnuradio-4.0/qradiolink/dsss_cdma_rx.hpp>
#include <gnuradio-4.0/qradiolink/dsss_cdma_tx.hpp>
#include <gnuradio-4.0/qradiolink/dsss_despreader.hpp>
#include <gnuradio-4.0/qradiolink/dsss_spreader.hpp>
#include <gnuradio-4.0/qradiolink/dstar_decoder.hpp>
#include <gnuradio-4.0/qradiolink/dstar_encoder.hpp>
#include <gnuradio-4.0/qradiolink/gdss_despreader.hpp>
#include <gnuradio-4.0/qradiolink/gdss_spreader.hpp>
#include <gnuradio-4.0/qradiolink/interleaver_bb.hpp>
#include <gnuradio-4.0/qradiolink/ldpc_decoder.hpp>
#include <gnuradio-4.0/qradiolink/ldpc_encoder.hpp>
#include <gnuradio-4.0/qradiolink/m17_coder.hpp>
#include <gnuradio-4.0/qradiolink/m17_decoder.hpp>
#include <gnuradio-4.0/qradiolink/m17_deframer.hpp>
#include <gnuradio-4.0/qradiolink/mmdvm_sink.hpp>
#include <gnuradio-4.0/qradiolink/mmdvm_source.hpp>
#include <gnuradio-4.0/qradiolink/mod_2fsk.hpp>
#include <gnuradio-4.0/qradiolink/mod_4fsk.hpp>
#include <gnuradio-4.0/qradiolink/mod_8fsk.hpp>
#include <gnuradio-4.0/qradiolink/mod_am.hpp>
#include <gnuradio-4.0/qradiolink/mod_bpsk.hpp>
#include <gnuradio-4.0/qradiolink/mod_cpm_4fsk.hpp>
#include <gnuradio-4.0/qradiolink/mod_dmr.hpp>
#include <gnuradio-4.0/qradiolink/mod_dpmr.hpp>
#include <gnuradio-4.0/qradiolink/mod_gmsk.hpp>
#include <gnuradio-4.0/qradiolink/mod_nbfm.hpp>
#include <gnuradio-4.0/qradiolink/mod_nxdn.hpp>
#include <gnuradio-4.0/qradiolink/mod_qpsk.hpp>
#include <gnuradio-4.0/qradiolink/mod_soqpsk.hpp>
#include <gnuradio-4.0/qradiolink/mod_ssb.hpp>
#include <gnuradio-4.0/qradiolink/mod_wbfm.hpp>
#include <gnuradio-4.0/qradiolink/p25_decoder.hpp>
#include <gnuradio-4.0/qradiolink/p25_encoder.hpp>
#include <gnuradio-4.0/qradiolink/pocsag_decoder.hpp>
#include <gnuradio-4.0/qradiolink/pocsag_encoder.hpp>
#include <gnuradio-4.0/qradiolink/rssi_tag.hpp>
#include <gnuradio-4.0/qradiolink/stretcher.hpp>
#include <gnuradio-4.0/qradiolink/ysf_decoder.hpp>
#include <gnuradio-4.0/qradiolink/ysf_encoder.hpp>
using namespace gr::qradiolink;
BOOST_AUTO_TEST_CASE(instantiate_templates)
{
    Mod2FSK<float> m; (void)m;
    Demod2FSK<float> d; (void)d;
    Mod4FSK<float> m4; (void)m4;
    POCSAGEncoder pe; (void)pe;
    InterleaverBB ib; (void)ib;
}
