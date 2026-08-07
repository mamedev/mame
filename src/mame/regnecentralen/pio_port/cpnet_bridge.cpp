// license: BSD-3-Clause
// copyright-holders: ravn
/***************************************************************************

    RC702 CP/NET host bridge — Z80 PIO port slot card

    Bytes flow between the Z80 PIO chip and an external client (the
    Pi/Pico bridge in production, or a Python test harness during
    bring-up) via a bitbanger image device.  Bitbanger handles the
    socket plumbing (OSD posix_osd_socket: non-blocking, polled
    inline by the emu thread).

    Direction tracking is purely protocol-driven: the chip's own mode
    routing decides which callback (read or write) fires.  No
    port-0x13 sniff, no chip-mode observation.

    See cpnet_bridge.h for the file-history note on why this device
    moved from a private listener thread + raw POSIX sockets to the
    bitbanger pattern (TL;DR: ~50x per-frame latency win in MAME).

***************************************************************************/

#include "emu.h"
#include "cpnet_bridge.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// Per-device logerror gate.  Default ON because bridge events fire at
// protocol rate (~hundreds/sec, not 14k/sec like the chip), so the
// volume is low.  Toggle to 0 to silence locally.
#define LOG_BRIDGE 1

#define BRIDGE_TS_FMT "[%8llu us] "
#define BRIDGE_TS_VAL ((unsigned long long)machine().time().as_ticks(1'000'000))


//**************************************************************************
//  DEVICE DEFINITION
//**************************************************************************

DEFINE_DEVICE_TYPE(RC702_PIO_CPNET_BRIDGE, rc702_pio_cpnet_bridge_device, "rc702_pio_cpnet_bridge", "RC702 CP/NET Host Bridge")


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

rc702_pio_cpnet_bridge_device::rc702_pio_cpnet_bridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, RC702_PIO_CPNET_BRIDGE, tag, owner, clock),
	device_rc702_pio_port_interface(mconfig, *this),
	m_stream(*this, "stream"),
	m_poll_timer(nullptr),
	m_input_count(0),
	m_input_index(0),
	m_brdy_high(true),  // PIO defaults BRDY high after reset
	m_last_byte(0)
{
}

void rc702_pio_cpnet_bridge_device::device_add_mconfig(machine_config &config)
{
	BITBANGER(config, m_stream, 0);
}

void rc702_pio_cpnet_bridge_device::device_start()
{
	// Poll the bitbanger for inbound bytes + drive STB to the chip
	// when m_input_buffer has data and BRDY is high.  1 ms cadence
	// is well below CP/NET frame round-trip latency yet well above
	// per-byte chip turnaround (~40 µs at 4 MHz Z80).
	m_poll_timer = timer_alloc(FUNC(rc702_pio_cpnet_bridge_device::poll_tick), this);
	m_poll_timer->adjust(attotime::from_msec(1), 0, attotime::from_msec(1));

	save_item(NAME(m_input_buffer));
	save_item(NAME(m_input_count));
	save_item(NAME(m_input_index));
	save_item(NAME(m_brdy_high));
	save_item(NAME(m_last_byte));
}


//**************************************************************************
//  CHIP-SIDE CALLBACKS (run on emulation thread)
//
//  STB timing — Mode 1 input flow:
//
//    1. Bridge places data on PB lines (= return value from read()).
//    2. Bridge pulses STB low-then-high.
//    3. PIO latches data on STB rising edge, asserts INT, drops BRDY.
//    4. Z80 ISR runs, IN A,(0x11) returns the latched byte, INT clears.
//    5. PIO raises BRDY -> rdy_w(1) here -> we strobe the next byte.
//
//**************************************************************************

uint8_t rc702_pio_cpnet_bridge_device::read()
{
	if (m_input_index < m_input_count)
	{
		uint8_t const b = m_input_buffer[m_input_index++];
		m_last_byte = b;
		if (LOG_BRIDGE) logerror(BRIDGE_TS_FMT "read() -> %02x  (FIFO[%u/%u])\n",
		                         BRIDGE_TS_VAL, b, m_input_index, m_input_count);
		return b;
	}

	// Empty: try a lazy refill so single-byte reads after long quiet
	// periods don't have to wait for the next poll_tick.  Bitbanger's
	// input() is non-blocking via the OSD layer.
	m_input_count = m_stream->input(m_input_buffer, sizeof(m_input_buffer));
	m_input_index = 0;
	if (m_input_index < m_input_count)
	{
		uint8_t const b = m_input_buffer[m_input_index++];
		m_last_byte = b;
		if (LOG_BRIDGE) logerror(BRIDGE_TS_FMT "read() -> %02x  (FIFO[%u/%u], lazy refill)\n",
		                         BRIDGE_TS_VAL, b, m_input_index, m_input_count);
		return b;
	}

	// Truly empty FIFO: return the last real byte, NOT a 0xff sentinel.
	// This keeps the PB lines 8-bit clean — every value 0x00..0xff is
	// treated as genuine data, no value is reserved to mean "empty".
	// The IRQ-driven snios path only forwards STB-triggered reads (a
	// read() that follows a real strobe pulse) to the Z80 ISR, so a
	// stale value returned here — from a spurious data_read resample
	// while STB is low, or a mode-flip callback — is never latched as
	// data.  The old 0xff sentinel conflated a real 0xff data byte
	// with "buffer empty" in the earlier polled cpnos-rom path (see
	// rc700-gensmedet/tasks/session34-direct-pio-stall-rootcause.md);
	// removing it makes the transport clean for any payload byte.
	if (LOG_BRIDGE) logerror(BRIDGE_TS_FMT "read() -> %02x  (FIFO empty, stale)\n",
	                         BRIDGE_TS_VAL, m_last_byte);
	return m_last_byte;
}

void rc702_pio_cpnet_bridge_device::write(uint8_t data)
{
	if (LOG_BRIDGE) logerror(BRIDGE_TS_FMT "write(%02x) -> TCP\n", BRIDGE_TS_VAL, data);

	m_stream->output(data);

	// Pulse STB so the chip releases BRDY (Mode 0 output ack
	// semantic).  Output direction is symmetric — chip wrote, we
	// ack to release.
	m_slot->strobe_w(0);
	m_slot->strobe_w(1);
}

void rc702_pio_cpnet_bridge_device::rdy_w(int state)
{
	bool const was_high = m_brdy_high;
	m_brdy_high = (state != 0);

	if (LOG_BRIDGE) logerror(BRIDGE_TS_FMT "rdy_w(%d) was=%d\n",
	                         BRIDGE_TS_VAL, state, was_high ? 1 : 0);

	// Rising edge: strobe only when there's a real byte to deliver.
	// On real Pi/Pico hardware this corresponds to "only pulse STB
	// when the TCP queue has something."  Empty buffer + no strobe
	// = no chip interrupt = Z80 ISR doesn't fire = snios's recv
	// queue stays empty and the recv loop times out cleanly.
	//
	// Earlier "always strobe" variant generated empty-strobes that
	// latched 0xff into the chip's m_input as a "no byte" sentinel,
	// which conflated a real 0xff data byte from mpm-net2 with
	// "buffer empty" in the polled snios path — see
	// rc700-gensmedet/tasks/session34-direct-pio-stall-rootcause.md.
	// The IRQ-driven snios path on the rc700-gensmedet
	// pio-mpm-irq-fix branch removes the polling and so this gate
	// is the right one again.
	if (m_brdy_high && !was_high && m_input_index < m_input_count)
	{
		m_slot->strobe_w(0);
		m_slot->strobe_w(1);
	}
}

TIMER_CALLBACK_MEMBER(rc702_pio_cpnet_bridge_device::poll_tick)
{
	// Refill the input buffer if drained.  Bitbanger's input() is
	// non-blocking: returns whatever is currently available (0 if
	// the host hasn't sent anything since the last poll).
	if (m_input_index >= m_input_count)
	{
		m_input_count = m_stream->input(m_input_buffer, sizeof(m_input_buffer));
		m_input_index = 0;
		if (LOG_BRIDGE && m_input_count > 0)
			logerror(BRIDGE_TS_FMT "poll_tick refill: %u bytes from TCP\n",
			         BRIDGE_TS_VAL, m_input_count);
	}

	// Gate strobing on chip BRDY high AND buffer non-empty: BRDY
	// low means the chip already has an unread byte latched in
	// m_input; if we strobed again, m_input would be overwritten
	// before the Z80 ISR reads it (silent byte loss on the IRQ
	// path).  The chip raises BRDY via data_read after each Z80 IN,
	// so this gate naturally rate-limits us to one strobe per
	// consumed byte.  m_brdy_high starts true (matching the
	// bridge's optimistic assumption); the first SEND-flip-RECV
	// cycle exercises the chip's set_rdy callbacks and brings the
	// two views into sync.  See ravn/mame#8 (MAME doesn't auto-raise
	// BRDY on Mode-1 entry per Zilog datasheet) for the boot-time
	// bootstrap analysis.
	if (m_input_index < m_input_count && m_brdy_high)
	{
		m_slot->strobe_w(0);
		m_slot->strobe_w(1);
	}
}
