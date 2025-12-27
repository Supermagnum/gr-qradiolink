/*
 * Copyright 2025 jmfriedt.
 * Adapted for gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_QRADIOLINK_M17_CODER_H
#define INCLUDED_QRADIOLINK_M17_CODER_H

#include <gnuradio/block.h>
#include <gnuradio/qradiolink/api.h>

namespace gr
{
  namespace qradiolink
  {

/*!
 * \brief M17 Protocol Encoder
 * \ingroup qradiolink
 *
 * This block implements the M17 protocol layer encoder, handling LSF generation,
 * frame encoding, encryption (AES/Scrambler), and ECDSA signatures.
 * Works with mod_m17 for complete M17 transmission.
 */
    class QRADIOLINK_API m17_coder:virtual public gr::block
    {
    public:
      typedef std::shared_ptr < m17_coder > sptr;
      typedef enum
      {
	ENCR_NONE,
	ENCR_SCRAM,
	ENCR_AES,
	ENCR_RES		//reserved
      } encr_t;

      /*!
       * \brief Return a shared_ptr to a new instance of qradiolink::m17_coder.
       *
       * To avoid accidental use of raw pointers, qradiolink::m17_coder's
       * constructor is in a private implementation
       * class. qradiolink::m17_coder::make is the public interface for
       * creating new instances.
       */
      static sptr make (std::string src_id, std::string dst_id, int mode,
			int data, int encr_type, int encr_subtype, int aes_subtype, int can,
			std::string meta, std::string key,
			std::string priv_key, bool debug, bool signed_str, std::string seed, int eot_cnt);
      virtual void set_key (std::string meta) = 0;
      virtual void set_priv_key (std::string meta) = 0;
      virtual void set_seed (std::string dst_id) = 0;
      virtual void set_eot_cnt (int arg) = 0;
      virtual void set_meta (std::string meta) = 0;
      virtual void set_src_id (std::string src_id) = 0;
      virtual void set_dst_id (std::string dst_id) = 0;
      virtual void set_debug (bool debug) = 0;
      virtual void set_signed (bool signed_str) = 0;
      virtual void set_type (int mode, int data, encr_t encr_type,
			     int encr_subtype, int can) = 0;
      virtual void set_mode (int mode) = 0;
      virtual void set_data (int data) = 0;
      virtual void set_encr_type (int encr_type) = 0;
      virtual void set_encr_subtype (int encr_subtype) = 0;
      virtual void set_aes_subtype (int aes_subtype, int encr_type) = 0;
      virtual void set_can (int can) = 0;
    };

  }				// namespace qradiolink
}				// namespace gr

#endif				/* INCLUDED_QRADIOLINK_M17_CODER_H */

