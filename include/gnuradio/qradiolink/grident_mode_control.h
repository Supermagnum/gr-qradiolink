/*
 * Copyright 2024 QRadioLink Contributors
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_QRADIOLINK_GRIDENT_MODE_CONTROL_H
#define INCLUDED_QRADIOLINK_GRIDENT_MODE_CONTROL_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/block.h>

namespace gr {
namespace qradiolink {

/*!
 * \brief Apply gr-ident preamble JSON to mod/demod route selection (GNU Radio 3.x).
 * \ingroup qradiolink
 *
 * Message ports:
 *  - preamble_in: PMT string (JSON body) or dict with keys topic/json
 *  - route_out: PMT dict with mode_id, digital, demod_block, mod_block, ...
 */
class QRADIOLINK_API grident_mode_control : public gr::block
{
public:
    typedef std::shared_ptr<grident_mode_control> sptr;

    static sptr make();

    virtual std::uint16_t mode_id() const = 0;
    virtual bool        digital() const = 0;
    virtual bool        encrypted() const = 0;
    virtual bool        metadata_present() const = 0;
    virtual bool        route_valid() const = 0;
    virtual std::string demod_block() const = 0;
    virtual std::string mod_block() const = 0;

protected:
    grident_mode_control(const std::string& name,
                         gr::io_signature::sptr input_signature,
                         gr::io_signature::sptr output_signature)
        : block(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif
