// license: BSD-3-Clause
// copyright-holders: ravn
/***************************************************************************

    RC702 CP/NET host bridge — Z80 PIO port slot card

    Implements the host side of the CP/NET fast-link transport (see
    rc700-gensmedet/docs/cpnet_fast_link.md, "Option P").  Plug into
    PIO-B (J3 in real hardware) via the slot system, with the
    underlying byte stream wired to a bitbanger image device:

        mame rc702 -piob cpnet_bridge -bitb<N> socket.127.0.0.1:4003

    Direction tracking is purely protocol-driven on the host side
    (CP/NET SCB length fields).  On the chip side, the Z80-PIO's own
    mode routing decides whether `read()` (chip pulls a byte from the
    bridge in Mode 1) or `write()` (chip pushes a byte to the bridge
    in Mode 0) gets called.  The bridge buffers host->Z80 bytes in a
    fixed-size scratch array (refilled from the bitbanger on the emu
    thread's poll timer) and forwards Z80->host bytes directly.

    Modeled after src/devices/bus/rs232/null_modem.cpp — same
    bitbanger-as-stream pattern, same emu-thread polled I/O.  Earlier
    revisions of this file used a private listener thread + raw POSIX
    sockets with a 50 ms select timeout, which dragged CP/NOS netboot
    over PIO from ~50 ms (real HW) to ~3.7 s emulated.  Switching to
    bitbanger inherits MAME's standard non-blocking-poll OSD socket
    layer (posix_osd_socket with zero-timeout select); per-frame
    latency drops by ~50x.

***************************************************************************/

#ifndef MAME_REGNECENTRALEN_PIO_PORT_CPNET_BRIDGE_H
#define MAME_REGNECENTRALEN_PIO_PORT_CPNET_BRIDGE_H

#pragma once

#include "pio_port.h"

#include "imagedev/bitbngr.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class rc702_pio_cpnet_bridge_device :
	public device_t,
	public device_rc702_pio_port_interface
{
public:
	rc702_pio_cpnet_bridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// device_rc702_pio_port_interface
	virtual uint8_t read() override;
	virtual void write(uint8_t data) override;
	virtual void rdy_w(int state) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	TIMER_CALLBACK_MEMBER(poll_tick);

	required_device<bitbanger_device> m_stream;
	emu_timer *m_poll_timer;

	// Input scratch (host -> Z80).  Refilled on poll_tick from the
	// bitbanger's non-blocking read; drained one byte per chip
	// `read()` callback.  Modeled on null_modem_device's m_input_*
	// pair.  Size chosen so a worst-case 256-byte CP/NET frame fits
	// in two refills.
	static constexpr unsigned INPUT_BUF_SIZE = 256;
	uint8_t  m_input_buffer[INPUT_BUF_SIZE];
	uint32_t m_input_count;     // bytes valid in m_input_buffer
	uint32_t m_input_index;     // next byte to hand to chip

	// Chip-side BRDY state.  Mode 1 input: BRDY high == chip ready to
	// accept a new latch; we only pulse STB when (a) buffer has bytes
	// to feed and (b) m_brdy_high.  Updated from rdy_w() on the emu
	// thread.
	bool m_brdy_high;

	// Last real byte handed to the chip via read().  When read() is
	// called with an empty FIFO (a spurious sample, e.g. a data_read
	// resample while STB is low, or a mode-flip callback), we return
	// THIS instead of a 0xff sentinel — keeping the PB lines 8-bit
	// clean (all 256 values are valid data; no value is reserved to
	// mean "empty").  Only STB-triggered reads that follow a real
	// strobe are forwarded to the Z80 ISR, so returning the stale
	// last byte here is harmless.  See the read() comment.
	uint8_t m_last_byte;
};

DECLARE_DEVICE_TYPE(RC702_PIO_CPNET_BRIDGE, rc702_pio_cpnet_bridge_device)

#endif // MAME_REGNECENTRALEN_PIO_PORT_CPNET_BRIDGE_H
