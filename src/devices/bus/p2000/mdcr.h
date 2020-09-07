// license:BSD-3-Clause
// copyright-holders:Erwin Jansen
/**********************************************************************

    Philips P2000  Mini Digital Cassette Recorder Emulation

**********************************************************************

                    +12V      1      8       !WCD
            OV (signal)       2      9       !REV
            OV (power)        3      A       !FWD
                    GND       4      B       RDC
                   !WDA       6      C       !RDA
                   !BET       7      D       !CIP
                                     E       !WEN

**********************************************************************/

#ifndef MAME_BUS_MDCR_CASS_H
#define MAME_BUS_MDCR_CASS_H

#pragma once

#include "imagedev/cassette.h"
#include "phase_decoder.h"
//**************************************************************************
//  CONSTANTS
//**************************************************************************
#define MDCR_PORT_TAG "mdcr"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mdcr_port_device
class device_mdcr_port_interface;

// Represents a MDCR210 cassette recorder you can find in the original P2000t
// It is basically a small tape drive that records bits using phase encoding.
// The tape drive is completely controlled by the computer, and requires no
// user action apart from insertion and removal.
//
// The tape deck retrieves the clock signal from the 0xAA synchronization bits
// it expects to see at start. The P2000t rom is responsible for writing the proper phase encoding
// (including) synchronization bits to tape.
//
// The tape deck can read both when moving forwards and when moving in reverse.
// When moving in reverse the RDC & RDA ports are swapped.
class mdcr_port_device
: public device_t
, public device_single_card_slot_interface<device_mdcr_port_interface>
{
public:
    template<typename T>
    mdcr_port_device(
        const machine_config& mconfig, const char* tag, device_t* owner, T&& opts, const char* dflt)
    : mdcr_port_device(mconfig, tag, owner, 0)
    {
        option_reset();
        opts(*this);
        set_default_option(dflt);
        set_fixed(false);
    }

    mdcr_port_device(const machine_config& mconfig,
                     const char*           tag,
                     device_t*             owner,
                     uint32_t              clock);
    virtual ~mdcr_port_device();

    // static configuration helpers
    auto read_handler() { return m_read_handler.bind(); }

    // computer interface

    // This is the read clock. The read clock is a flip flop that switches
    // whenever a new bit is available on rda. The original flips every 167us
    // This system flips at 154 / 176 usec, which is within the tolerance for
    // the rom and system diagnostics.
    //
    // Note that rdc & rda are flipped when the tape is moving in reverse.
    DECLARE_READ_LINE_MEMBER(rdc);

    // The current active data bit.
    DECLARE_READ_LINE_MEMBER(rda);

    // False indicates we have reached end/beginning of tape
    DECLARE_READ_LINE_MEMBER(bet);

    // False if a cassette is in place.
    DECLARE_READ_LINE_MEMBER(cip);

    // False when the cassette is write enabled.
    DECLARE_READ_LINE_MEMBER(wen);

    // True if we should activate the reverse motor.
    DECLARE_WRITE_LINE_MEMBER(rev);

    // True if we should activate the forward motor.
    // Note: A quick pulse (<20usec) will reset the phase decoder.
    DECLARE_WRITE_LINE_MEMBER(fwd);

    // The bit to write to tape. Make sure to set wda after wdc.
    DECLARE_WRITE_LINE_MEMBER(wda);

    // True if the current wda should be written to tape.
    DECLARE_WRITE_LINE_MEMBER(wdc);


protected:
    // device-level overrides
    virtual void device_start() override;

    devcb_write_line            m_read_handler;
    device_mdcr_port_interface* m_cart;

private:
    bool m_wda{ false };
    bool m_fwd{ false };
    bool m_rev{ false };
};


// ======================> device_mdcr_port_interface

class device_mdcr_port_interface : public device_interface
{
public:
    // construction/destruction
    virtual ~device_mdcr_port_interface();

    virtual void write_bit(bool bit) {}
    virtual void rewind() {}
    virtual void forward() {}
    virtual void stop() {}
    virtual bool tape_start_or_end() { return true; }
    virtual bool data_available() { return true; }
    virtual bool data() { return true; }

protected:
    device_mdcr_port_interface(const machine_config& mconfig, device_t& device);

    mdcr_port_device* m_slot;
};


// Models a MCR220
// Detailed documentation on the device can be found in this repository:
// https://github.com/p2000t/documentation/tree/master/hardware
class mdcr_device
: public device_t
, public device_mdcr_port_interface
{
public:
    // construction/destruction
    mdcr_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
    mdcr_device(const machine_config& mconfig,
                device_type           type,
                const char*           tag,
                device_t*             owner,
                uint32_t              clock);

    // device-level overrides
    virtual void device_start() override;
    virtual void device_timer(emu_timer& timer, device_timer_id id, int param, void* ptr) override;

    // optional information overrides
    virtual void device_add_mconfig(machine_config& config) override;

    virtual void write_bit(bool bit) override;
    virtual void rewind() override;
    virtual void forward() override;
    virtual void stop() override;
    virtual bool tape_start_or_end() override;
    virtual bool data_available() override;
    virtual bool data() override;

private:
    required_device<cassette_image_device> m_cassette;

    bool         m_recording{ false };
    bool         m_fwd{ false };
    double       m_fwd_pulse_time{ 0 };
    bool         m_rdc{ false };
    bool         m_rda{ false };
    PhaseDecoder phd;

    // timers
    emu_timer* m_read_timer;
    double     m_last_tape_time{ 0 };
};

// device type definition
DECLARE_DEVICE_TYPE(MDCR_PORT, mdcr_port_device)


void p2000_mdcr_devices(device_slot_interface& device);

#endif  // MAME_BUS_MDCR_CASS_H
