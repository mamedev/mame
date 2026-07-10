// license:BSD-3-Clause
// copyright-holders:Dave L. Rand
/***************************************************************************

    Toshiba T-200 / T-250 SASI host adapter

    Bridges the 8085 I/O ports 0x00-0x08 onto a SASI bus, mimicking the
    Intel 8155-based adapter board on real hardware.  See t250_sasi.h.

***************************************************************************/

#include "emu.h"
#include "t250_sasi.h"


// 500 ns minimum pulse width for SASI control lines.  Not load-bearing on
// either the BIOS' polling code or the nscsi target; a nominal value.
static constexpr attotime SASI_PULSE = attotime::from_nsec(500);


DEFINE_DEVICE_TYPE(T250_SASI_HOST, t250_sasi_host_device, "t250_sasi_host", "Toshiba T-200/T-250 SASI host adapter")

t250_sasi_host_device::t250_sasi_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, T250_SASI_HOST, tag, owner, clock)
	, nscsi_device_interface(mconfig, *this)
	, m_sasi_phase(SASI_IDLE)
	, m_sasi_cmd_first(0)
	, m_sasi_swallow_remaining(0)
	, m_sasi_synth_bsy(false)
	, m_sasi_synth_req(false)
	, m_prev_ctrl(0)
	, m_sasi_ack_timer(nullptr)
	, m_sasi_sel_timer(nullptr)
	, m_sasi_rst_timer(nullptr)
{
}

void t250_sasi_host_device::io_map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(t250_sasi_host_device::hconfg_r), FUNC(t250_sasi_host_device::hconfg_w));
	map(0x01, 0x01).w(FUNC(t250_sasi_host_device::hdata_w));
	map(0x02, 0x02).r(FUNC(t250_sasi_host_device::hdata1_r));
	map(0x03, 0x03).r(FUNC(t250_sasi_host_device::hstat_r));
	map(0x08, 0x08).rw(FUNC(t250_sasi_host_device::hsts_r), FUNC(t250_sasi_host_device::hrset_w));
}

void t250_sasi_host_device::device_start()
{
	save_item(NAME(m_sasi_phase));
	save_item(NAME(m_sasi_cmd_first));
	save_item(NAME(m_sasi_swallow_remaining));
	save_item(NAME(m_sasi_synth_bsy));
	save_item(NAME(m_sasi_synth_req));
	save_item(NAME(m_prev_ctrl));

	m_sasi_ack_timer = timer_alloc(FUNC(t250_sasi_host_device::sasi_ack_off), this);
	m_sasi_sel_timer = timer_alloc(FUNC(t250_sasi_host_device::sasi_sel_off), this);
	m_sasi_rst_timer = timer_alloc(FUNC(t250_sasi_host_device::sasi_rst_off), this);
}

void t250_sasi_host_device::device_reset()
{
	m_sasi_phase = SASI_IDLE;
	m_sasi_cmd_first = 0;
	m_sasi_swallow_remaining = 0;
	m_sasi_synth_bsy = false;
	m_sasi_synth_req = false;
	m_prev_ctrl = 0;
	m_scsi_bus->ctrl_w(m_scsi_refid, 0, nscsi_device_interface::S_ALL);
	m_scsi_bus->data_w(m_scsi_refid, 0);
}

void t250_sasi_host_device::scsi_ctrl_changed()
{
	const u32 ctrl = m_scsi_bus->ctrl_r();
	const u32 changed = ctrl ^ m_prev_ctrl;
	m_prev_ctrl = ctrl;
	if (changed & S_REQ) on_req_change((ctrl & S_REQ) ? 1 : 0);
	if (changed & S_BSY) on_bsy_change((ctrl & S_BSY) ? 1 : 0);
}

TIMER_CALLBACK_MEMBER(t250_sasi_host_device::sasi_ack_off)
{
	m_scsi_bus->ctrl_w(m_scsi_refid, 0, nscsi_device_interface::S_ACK);
	// Critical: release the data bus.  Real hardware uses an external buffer
	// (74LS244 etc.) gated by I/O so the host doesn't drive data while the
	// target is driving DATA_IN.  Without this release, the last CDB byte
	// we drove stays on the bus and OR's with any target-driven data byte —
	// causing every DATA_IN byte the BIOS reads to be corrupted (the asc_sasi
	// reference implementation handles this via its iio_w callback).
	m_scsi_bus->data_w(m_scsi_refid, 0);
}

TIMER_CALLBACK_MEMBER(t250_sasi_host_device::sasi_sel_off)
{
	m_scsi_bus->ctrl_w(m_scsi_refid, 0, nscsi_device_interface::S_SEL);
}

TIMER_CALLBACK_MEMBER(t250_sasi_host_device::sasi_rst_off)
{
	m_scsi_bus->ctrl_w(m_scsi_refid, 0, nscsi_device_interface::S_RST);
}

// REQ callback from the bus.  In a real SASI/SCSI initiator the host
// clears ACK when REQ drops.  We rely on the timer-driven ACK pulse so
// nothing extra is needed here, but the callback keeps the bus library
// notified that we observe ctrl changes.
void t250_sasi_host_device::on_req_change(int state)
{
	// No-op: ACK is pulsed via m_sasi_ack_timer right after each data
	// transfer (asc_sasi/bbc_sasi pattern).
	(void)state;
}

void t250_sasi_host_device::on_bsy_change(int state)
{
	// Target asserted BSY in response to our SEL — selection complete.
	// Drop SEL and data bus, then advance the bridge state machine into
	// command phase so the next hdata_w forwards the first CDB byte.
	// (BIOS polls hsts to detect BSY independently; this callback is what
	//  drives our state transition, not the BIOS-visible signalling.)
	if (state && m_sasi_phase == SASI_SELECTING)
	{
		m_scsi_bus->ctrl_w(m_scsi_refid, 0, nscsi_device_interface::S_SEL);
		m_scsi_bus->data_w(m_scsi_refid, 0);
		m_sasi_phase = SASI_CMD_FIRST;
	}
}

// Port 0x00 read.  In the BIOS "req" path (not-inch5) this returns the REQ
// bit at bit 1, active LOW (`ani 02h; rz` returns when bit 1 is zero).
// Bit 4 ("buffer full") is sampled separately via port 0x03 (hstat_r).
u8 t250_sasi_host_device::hconfg_r()
{
	// bit 1 = REQ (active LOW: 0 = REQ asserted), default 1 (REQ inactive)
	// bit 4 = buffer-full (active HIGH: 1 = data ready), default 0 (no data)
	// Start with d = 0xff masked so bit 4 is cleared; set it explicitly when
	// DTC actually has a byte queued in DATA_IN phase.
	u8 d = 0xff & ~0x10;
	const u32 ctrl = m_scsi_bus->ctrl_r();
	const bool req = (m_sasi_phase == SASI_SWALLOW_CMD ||
					   m_sasi_phase == SASI_SWALLOW_GARBAGE ||
					   m_sasi_phase == SASI_SWALLOW_STATUS)
		? m_sasi_synth_req
		: bool(ctrl & nscsi_device_interface::S_REQ);
	if (req)
		d &= ~0x02;  // bit 1 cleared = REQ asserted
	// Buffer-full whenever target is driving a byte toward us (S_INP asserted)
	// and REQ is up.  This covers BOTH DATA_IN (S_CTL=0) and STATUS phase
	// (S_CTL=1) — BIOS' hgetrs uses req1 to wait for the status byte, just
	// like hget does for data bytes.  Also synthesized for SWALLOW_STATUS
	// when we're feeding back the SS_GOOD reply to a swallowed mode-set.
	if ((m_sasi_phase == SASI_PASSTHROUGH &&
	     (ctrl & nscsi_device_interface::S_REQ) &&
	     (ctrl & nscsi_device_interface::S_INP)) ||
	    (m_sasi_phase == SASI_SWALLOW_STATUS && m_sasi_synth_req))
		d |= 0x10;
	return d;
}

// Port 0x00 write.  Real hardware: 8155 I/O configuration register.  BIOS
// writes 0x09 once (port A out, port B in, alt 4) before selecting.  We
// don't model the 8155 — the write is accepted and dropped.
void t250_sasi_host_device::hconfg_w(u8 data)
{
	(void)data;
}

// Port 0x03 read.  BIOS' "req1" path polls bit 4 ("buffer full", active
// HIGH) for the first byte of an inbound transfer.  Bit 1 mirrors REQ
// (active low) for the inch5 hgetrs path.
u8 t250_sasi_host_device::hstat_r()
{
	u8 d = 0;
	const u32 ctrl = m_scsi_bus->ctrl_r();
	const bool req_in = (m_sasi_phase == SASI_SWALLOW_STATUS)
		? m_sasi_synth_req
		: ((ctrl & nscsi_device_interface::S_REQ) &&
		   (ctrl & nscsi_device_interface::S_INP));
	if (req_in)
		d |= 0x10;        // buffer-full active high
	// Match hconfg_r: bit 1 = !REQ on the data-in side (BIOS req1 elsewhere
	// uses port 0x03 only for the buffer-full poll, but keep consistent).
	const bool req_any = (m_sasi_phase == SASI_SWALLOW_CMD ||
						   m_sasi_phase == SASI_SWALLOW_GARBAGE ||
						   m_sasi_phase == SASI_SWALLOW_STATUS)
		? m_sasi_synth_req
		: bool(ctrl & nscsi_device_interface::S_REQ);
	if (!req_any)
		d |= 0x02;
	return d;
}

// Port 0x08 read.  BIOS uses bit 7 as INVERTED BSY (1 = idle, 0 = busy),
// bit 2 as C/D phase, bit 1 as I/O, bit 0 as MSG.  See bios64.asm
// hconsel: `rlc; jnc hact` jumps to controller-reset when bit 7 reads 0
// (meaning a controller is currently busy/hung).
u8 t250_sasi_host_device::hsts_r()
{
	const u32 ctrl = m_scsi_bus->ctrl_r();
	// Lazy SELECTING -> CMD_FIRST transition.  Callback registration for BSY
	// changes can fail silently across the slot-card / driver-device boundary
	// (finder_dummy_tag warnings in -v), so we don't rely on sasi_bsy_w being
	// called.  BIOS' hsel1 loop polls hsts hundreds of times per microsecond
	// while waiting for BSY assertion, so doing the transition here is plenty
	// responsive.
	if (!machine().side_effects_disabled() && (m_sasi_phase == SASI_SELECTING) && (ctrl & nscsi_device_interface::S_BSY))
	{
		m_scsi_bus->ctrl_w(m_scsi_refid, 0, nscsi_device_interface::S_SEL);
		m_scsi_bus->data_w(m_scsi_refid, 0);
		m_sasi_phase = SASI_CMD_FIRST;
	}
	u8 d = 0xff;
	const bool bsy = m_sasi_synth_bsy || bool(ctrl & nscsi_device_interface::S_BSY);
	if (bsy)
		d &= ~0x80;       // inverted BSY at bit 7
	// For pass-through phases, mirror C/D, I/O, MSG; for swallow phases
	// drive synthetic phase information that matches what BIOS expects.
	if (m_sasi_phase == SASI_SWALLOW_CMD)
	{
		// command phase: C/D high, I/O low, MSG low
		d |= 0x04;        // bit 2 = C/D
		// req mirror at bit 1 (also inverted-ish — BIOS' inch5 `req` path
		// polls hsts bit 1 active low, just like hconfg)
		if (m_sasi_synth_req)
			d &= ~0x02;
	}
	else if (m_sasi_phase == SASI_SWALLOW_STATUS)
	{
		// status phase: C/D high, I/O high, MSG low
		d |= 0x04;
		d |= 0x01;        // BIOS hgetrs polls bit 0 implicitly via rlc/jnc
		if (m_sasi_synth_req)
			d &= ~0x02;
	}
	else
	{
		// Pass-through: mirror real bus.  All hsts phase bits are ACTIVE LOW
		// (same convention as BSY at bit 7).  d starts at 0xff so we need to
		// CLEAR each bit when the corresponding nscsi line is asserted.
		//   bit 2 = C/D  (BIOS hget checks `ani 04h; rz` = exit when bit 2=0,
		//                 i.e. when real C/D goes high = STATUS phase)
		//   bit 0 = I/O
		//   bit 1 = MSG
		if (ctrl & nscsi_device_interface::S_CTL) d &= ~0x04;
		if (ctrl & nscsi_device_interface::S_INP) d &= ~0x01;
		if (ctrl & nscsi_device_interface::S_MSG) d &= ~0x02;
	}
	return d;
}

// Port 0x08 write — controller reset.  BIOS pulses this three times in
// `hact` to recover a hung controller, and once at the start of `hcon` /
// `hwaitr`.  Pulse the SASI RST line and forcibly return to IDLE.
void t250_sasi_host_device::hrset_w(u8 data)
{
	(void)data;
	m_scsi_bus->ctrl_w(m_scsi_refid, nscsi_device_interface::S_RST, nscsi_device_interface::S_RST);
	// Force the target's state machine to observe the RST edge before the
	// pulse timer can deassert.  Same race as the SEL pulse fix: BIOS' hact
	// path pulses RST three times in rapid succession to recover a hung
	// controller, and without this sync any (or all) of them can be missed,
	// leaving the target in stale state and BIOS spinning in hsel1 forever.
	machine().scheduler().synchronize();
	m_sasi_rst_timer->adjust(SASI_PULSE);
	m_sasi_phase = SASI_IDLE;
	m_sasi_synth_bsy = false;
	m_sasi_synth_req = false;
	m_sasi_swallow_remaining = 0;
}

// Port 0x01 write — outbound data byte.
//
//   IDLE: BIOS' hconsel writes 0xFF to start a selection.  Drive data=0x01
//         (target id 0) plus SEL; wait for the target to assert BSY.
//   PASSTHROUGH (cmd or data-out): forward byte to bus and pulse ACK.
//   CMD_FIRST: cache the first command byte; decide whether to swallow
//              this command block locally (hcon mode-set) or forward to
//              the real target.
//   SWALLOW_CMD: consume payload bytes; on the last byte, hand off to
//                SWALLOW_STATUS so hgetrs sees an SS_GOOD reply.
void t250_sasi_host_device::hdata_w(u8 data)
{
	switch (m_sasi_phase)
	{
	case SASI_IDLE:
		// BIOS writes 0xFF to select.  Real hardware ANDs the data byte
		// into a SEL pulse on the bus; we hardwire target id 0.
		// SEL is pulsed (not held), matching the asc_sasi reference
		// implementation — SASI targets respond to the SEL edge, not a
		// continuous level.
		if (data == 0xff)
		{
			m_scsi_bus->data_w(m_scsi_refid, 0x01);                                          // target id 0
			m_scsi_bus->ctrl_w(m_scsi_refid, nscsi_device_interface::S_SEL, nscsi_device_interface::S_SEL);
			// Give the nscsi target's state machine a chance to observe the SEL
			// edge before our timer can deassert it.  Without this, rapid
			// back-to-back transactions can race: the target's slice doesn't
			// get scheduled within the 500 ns SEL pulse window, BSY never
			// asserts, and BIOS' hsel1 loop spins forever.  Confirmed by Dave
			// 2026-05-23: hang only manifests in normal speed; with -debug on,
			// the slower scheduler base loop hides the race.
			machine().scheduler().synchronize();
			m_sasi_sel_timer->adjust(SASI_PULSE);                             // auto-deassert SEL
			m_sasi_phase = SASI_SELECTING;
			// If a real target is at id 0, it will respond by asserting
			// BSY almost immediately; the next hsts read by BIOS will see
			// bit 7 drop and proceed to send the first command byte.
			// If no target is attached, BSY never asserts and BIOS times
			// out its 256-iteration spin in hsel1, then re-enters hconsel
			// — which loops forever.  This is fine because the default
			// configuration has no target attached and the BIOS does not
			// probe the HD path on a stock boot.
			if (m_scsi_bus->ctrl_r() & nscsi_device_interface::S_BSY)
			{
				// Target picked up — drop SEL, enter CMD_FIRST.
				m_scsi_bus->ctrl_w(m_scsi_refid, 0, nscsi_device_interface::S_SEL);
				m_scsi_bus->data_w(m_scsi_refid, 0);
				m_sasi_phase = SASI_CMD_FIRST;
			}
		}
		break;

	case SASI_SELECTING:
		// Shouldn't normally happen — BIOS only writes the 0xFF then polls.
		break;

	case SASI_CMD_FIRST:
		m_sasi_cmd_first = data;
		// Recognise the two hcon mode-set opcodes BIOS sends as the very
		// first byte of a config command block.  Anything else is a real
		// CDB byte that gets forwarded to the target.
		//   0xC1 — T-250 DTC-510 drive-type block (6 bytes total)
		//   0x0C — T-200 ACB-4000 set-parameters block (14 bytes total)
		// In SASI CDB terms 0xC1 is "LUN=6, opcode=01 (REZERO)" — illegal —
		// and 0x0C is INITIALIZE DRIVE CHARACTERISTICS, which generic
		// nscsi_harddisk does not implement.  Forwarding either would cause
		// the target to reject with CHECK CONDITION and BIOS to loop in
		// `hcon` forever (no retry limit on this path).  Instead, abort
		// the target's command phase with a RST pulse and synthesize our
		// own bus presence so BIOS can drain the payload and read an
		// SS_GOOD status.
		if (data == 0xc1 || data == 0x0c)
		{
			m_scsi_bus->ctrl_w(m_scsi_refid, nscsi_device_interface::S_RST, nscsi_device_interface::S_RST);
			m_sasi_rst_timer->adjust(SASI_PULSE);
			m_sasi_phase = SASI_SWALLOW_CMD;
			m_sasi_synth_bsy = true;
			m_sasi_synth_req = true;
			// 6 more bytes after this one for T-250 (total 6),
			// 13 more for T-200 (total 14).
			m_sasi_swallow_remaining = (data == 0xc1) ? 5 : 13;
		}
		else
		{
			// Real command — forward to the target's CMD phase.
			m_scsi_bus->data_w(m_scsi_refid, data);
			m_scsi_bus->ctrl_w(m_scsi_refid, nscsi_device_interface::S_ACK, nscsi_device_interface::S_ACK);
			m_sasi_ack_timer->adjust(SASI_PULSE);
			m_sasi_phase = SASI_PASSTHROUGH;
		}
		break;

	case SASI_PASSTHROUGH:
		m_scsi_bus->data_w(m_scsi_refid, data);
		m_scsi_bus->ctrl_w(m_scsi_refid, nscsi_device_interface::S_ACK, nscsi_device_interface::S_ACK);
		m_sasi_ack_timer->adjust(SASI_PULSE);
		break;

	case SASI_SWALLOW_CMD:
		if (m_sasi_swallow_remaining > 0)
			m_sasi_swallow_remaining--;
		if (m_sasi_swallow_remaining == 0)
			sasi_finish_swallow();
		break;

	case SASI_SWALLOW_STATUS:
		// BIOS shouldn't write here in this phase, but be defensive.
		break;
	}
}

void t250_sasi_host_device::sasi_finish_swallow()
{
	// All swallowed payload consumed.  BIOS will now do `in hdata1` once
	// (pickup garbage), then `call hgetrs` which polls REQ via port 0x03
	// bit 4 and reads the status byte from hdata1.  Move to SWALLOW_GARBAGE
	// first — that pickup-garbage read returns 0 and advances us to
	// SWALLOW_STATUS so the subsequent hgetrs sees the SS_GOOD reply.
	m_sasi_phase = SASI_SWALLOW_GARBAGE;
	m_sasi_synth_req = false;  // garbage read isn't gated by req1
}

// Port 0x02 read — inbound data byte.
u8 t250_sasi_host_device::hdata1_r()
{
	u8 data = 0xff;
	switch (m_sasi_phase)
	{
	case SASI_PASSTHROUGH:
		data = m_scsi_bus->data_r();
		if (!machine().side_effects_disabled())
		{
			if (m_scsi_bus->ctrl_r() & nscsi_device_interface::S_REQ)
			{
				m_scsi_bus->ctrl_w(m_scsi_refid, nscsi_device_interface::S_ACK, nscsi_device_interface::S_ACK);
				m_sasi_ack_timer->adjust(SASI_PULSE);
			}
			// If the bus just dropped BSY (target completed message-in), return
			// to idle so the next selection starts clean.
			if (!(m_scsi_bus->ctrl_r() & nscsi_device_interface::S_BSY))
				m_sasi_phase = SASI_IDLE;
		}
		break;

	case SASI_SWALLOW_CMD:
		// Shouldn't normally happen — BIOS doesn't read hdata1 while still
		// writing the command block.  Return 0 to stay quiet.
		data = 0x00;
		break;

	case SASI_SWALLOW_GARBAGE:
		// BIOS' `in hdata1; pickup garbage` after the last CDB byte.
		// Return 0 and advance to SWALLOW_STATUS, asserting synth_req so
		// hgetrs' req1 poll passes and the next read returns the status.
		data = 0x00;
		if (!machine().side_effects_disabled())
		{
			m_sasi_phase = SASI_SWALLOW_STATUS;
			m_sasi_synth_req = true;
		}
		break;

	case SASI_SWALLOW_STATUS:
		// First read = status byte = SS_GOOD (0x00).
		// After we deliver it, BIOS' hgetrs runs `in hdata1; in hsts; rlc;
		// jnc hgw` — looping while REQ is still asserted.  Drop synth_req
		// and synth_bsy now so the second read sees no REQ and BIOS exits
		// the wait loop, then return to IDLE.
		data = 0x00;
		if (!machine().side_effects_disabled())
		{
			m_sasi_synth_req = false;
			m_sasi_synth_bsy = false;
			m_sasi_phase = SASI_IDLE;
		}
		break;

	case SASI_IDLE:
	case SASI_SELECTING:
	case SASI_CMD_FIRST:
		// No data to deliver in these phases.
		break;
	}
	return data;
}
