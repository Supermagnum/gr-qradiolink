/*
 * Copyright 2024 QRadioLink Contributors
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_QRADIOLINK_GRIDENT_PREAMBLE_SUB_H
#define INCLUDED_QRADIOLINK_GRIDENT_PREAMBLE_SUB_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/qradiolink/grident_zmq_protocol.h>
#include <gnuradio/block.h>
#include <string>

namespace gr {
namespace qradiolink {

/*!
 * \brief SUBscribe to gr-ident PreambleResultZmqPub (default tcp://127.0.0.1:5560).
 * \ingroup qradiolink
 *
 * Requires libzmq at build time. Forwards multipart [topic, json] on message port
 * preamble_out (PMT dict with keys topic, json).
 */
class QRADIOLINK_API grident_preamble_sub : public gr::block
{
public:
    typedef std::shared_ptr<grident_preamble_sub> sptr;

    /*!
     * \param endpoint ZMQ endpoint (connect unless bind_socket true)
     * \param topic_filter ZMQ SUB prefix (default grident)
     * \param bind_socket If true, bind SUB; else connect
     */
    static sptr make(const std::string& endpoint = grident::k_preamble_pub_default,
                     const std::string& topic_filter = grident::k_preamble_topic,
                     bool               bind_socket = false);

protected:
    grident_preamble_sub(const std::string& name,
                         gr::io_signature::sptr input_signature,
                         gr::io_signature::sptr output_signature)
        : block(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif
