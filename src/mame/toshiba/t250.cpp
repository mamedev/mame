// license:BSD-3-Clause
// copyright-holders:Dave L. Rand
/***************************************************************************

    Toshiba T-200 / T-250

    CP/M business machines, 1981.  Same main board, different floppy
    subsystem: T-200 uses 5.25" DSDD, T-250 uses 8" DSDD (both dual drive).

    Main board (per KiCad project + functional spec 1981-09):
      - Toshiba TMP8085A @ 2.67 MHz (375 ns clock period)
      - 4 KB EPROM (2732) + up to 64 KB DRAM (8x MB8264)
      - 2 KB video RAM (HM6116) and 2 KB downloadable character generator
        (HM6116), both bank-switched into 0xF800-0xFFFF
      - Intel 8279 keyboard/display controller
      - Intel 8257 DMA controller
      - Intel 8253 PIT (used as baud rate generator)
      - Hitachi HD46505SP (HD6845-compatible) CRTC
      - NEC uPD765A floppy disk controller

    Optional serial daughter board:
      - Intel 8251 USART, driven by 8253 outputs

    Memory map (64 KB version):
      0x0000-0x0FFF  bank-switched: ROM (cold) <-> DRAM
      0x1000-0xF7FF  DRAM
      0xF800-0xFFFF  bank-switched: video RAM <-> VPG (character RAM)

    Interrupts:
      TRAP     console (origin TBD)
      RST 7.5  20 Hz system tick, derived from 6845 HSYNC via divider
      RST 6.5  shared: keyboard (8279), printer, communications --
               source identified via the 0xD0 read register (rdint)
      RST 5.5  floppy disk controller (uPD765 INT)

    I/O map (per equate.lib in CP/M BIOS source):
      0x80-0x88  8257 DMA controller
      0x90-0x91  8279 keyboard
      0xA0-0xA3  8253 PIT
      0xA4-0xA7  8251 USART + CCM control
      0xC0       memory bank latch (write-only, pulse-stepped)
      0xC1-0xC3  printer data / command / IRQ ack
      0xD0       RST 6.5 source register
      0xD1       system DIP switches
      0xD3       printer + comm status
      0xE0-0xE1  6845 CRTC
      0xF0-0xF1  uPD765 FDC

***************************************************************************/

#include "emu.h"

#include "bus/centronics/ctronics.h"
#include "bus/nscsi/devices.h"
#include "bus/rs232/rs232.h"
#include "cpu/i8085/i8085.h"
#include "imagedev/floppy.h"
#include "machine/i8251.h"
#include "machine/i8257.h"
#include "machine/i8279.h"
#include "machine/input_merger.h"
#include "machine/nscsi_bus.h"
#include "machine/nscsi_cb.h"
#include "machine/output_latch.h"
#include "machine/pit8253.h"
#include "machine/upd765.h"
#include "sound/beep.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "formats/hxchfe_dsk.h"
#include "formats/imd_dsk.h"


namespace {

// 500 ns minimum pulse width for SASI control lines.
static constexpr attotime SASI_PULSE = attotime::from_nsec(500);

// ============================================================================
//  t250_sasi_host_device — SASI host adapter (Toshiba bridge board).
//
//  Sits on the SASI bus as an external device at refid 7.  Bridges the
//  8085 CPU's I/O ports 0x00-0x08 onto the SASI control / data lines
//  using a small state machine that mimics the Intel 8155-based
//  adapter board on real hardware.  Patrick's review (PR #15372) asked
//  for this code to live in its own device class subclassing
//  nscsi_device_interface so that m_scsi_refid is accessible directly,
//  and so that the controller can be attached to the SASI bus via the
//  modern set_external_device() idiom.
//
//  TODO: Replace the bespoke ACK/RST/SEL emu_timer pulses with an
//  i8155_device sub-instance whose internal timer drives those edges
//  on real hardware.  For now the I/O port semantics are preserved
//  bit-for-bit so the existing CP/M BIOS m32 build (drives C/D/E/F)
//  continues to work without further changes.
// ============================================================================

class t250_sasi_host_device : public device_t, public nscsi_device_interface
{
public:
	t250_sasi_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// 8085 I/O port handlers (mapped by the host driver onto 0x00-0x08).
	u8   hconfg_r();      // port 0x00 read  (REQ bit 1, active low)
	void hconfg_w(u8 d);  // port 0x00 write (8155 config: accept + ignore)
	void hdata_w(u8 d);   // port 0x01 write (data out + implicit ACK)
	u8   hdata1_r();      // port 0x02 read  (data in + implicit ACK)
	u8   hstat_r();       // port 0x03 read  (buffer-full bit 4, REQ bit 1)
	u8   hsts_r();        // port 0x08 read  (BSY inverted, C/D, I/O, MSG)
	void hrset_w(u8 d);   // port 0x08 write (controller reset)

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// nscsi_device_interface — invoked whenever any control line on the
	// bus changes.  We track REQ and BSY edges and dispatch to our
	// internal handlers, matching the per-signal callbacks the old
	// nscsi_callback_device wiring used.
	virtual void scsi_ctrl_changed() override;

private:
	void sasi_finish_swallow();
	TIMER_CALLBACK_MEMBER(sasi_ack_off);
	TIMER_CALLBACK_MEMBER(sasi_sel_off);
	TIMER_CALLBACK_MEMBER(sasi_rst_off);

	void on_req_change(int state);
	void on_bsy_change(int state);

	static constexpr u8 SASI_IDLE            = 0;
	static constexpr u8 SASI_SELECTING       = 1;
	static constexpr u8 SASI_CMD_FIRST       = 2;
	static constexpr u8 SASI_PASSTHROUGH     = 3;
	static constexpr u8 SASI_SWALLOW_CMD     = 4;
	static constexpr u8 SASI_SWALLOW_GARBAGE = 5;
	static constexpr u8 SASI_SWALLOW_STATUS  = 6;

	u8 m_sasi_phase = SASI_IDLE;
	u8 m_sasi_cmd_first = 0;
	u8 m_sasi_swallow_remaining = 0;
	bool m_sasi_synth_bsy = false;
	bool m_sasi_synth_req = false;
	u32 m_prev_ctrl = 0;            // last seen bus ctrl; edge-detect input
	emu_timer *m_sasi_ack_timer = nullptr;
	emu_timer *m_sasi_sel_timer = nullptr;
	emu_timer *m_sasi_rst_timer = nullptr;
};

DEFINE_DEVICE_TYPE_PRIVATE(T250_SASI_HOST, t250_sasi_host_device, t250_sasi_host_device, "t250_sasi_host", "Toshiba T-200/T-250 SASI host adapter")

t250_sasi_host_device::t250_sasi_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, T250_SASI_HOST, tag, owner, clock)
	, nscsi_device_interface(mconfig, *this)
{
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
	if (m_sasi_phase == SASI_SELECTING && (ctrl & nscsi_device_interface::S_BSY))
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
		if (m_scsi_bus->ctrl_r() & nscsi_device_interface::S_REQ)
		{
			m_scsi_bus->ctrl_w(m_scsi_refid, nscsi_device_interface::S_ACK, nscsi_device_interface::S_ACK);
			m_sasi_ack_timer->adjust(SASI_PULSE);
		}
		// If the bus just dropped BSY (target completed message-in), return
		// to idle so the next selection starts clean.
		if (!(m_scsi_bus->ctrl_r() & nscsi_device_interface::S_BSY))
			m_sasi_phase = SASI_IDLE;
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
		m_sasi_phase = SASI_SWALLOW_STATUS;
		m_sasi_synth_req = true;
		break;

	case SASI_SWALLOW_STATUS:
		// First read = status byte = SS_GOOD (0x00).
		// After we deliver it, BIOS' hgetrs runs `in hdata1; in hsts; rlc;
		// jnc hgw` — looping while REQ is still asserted.  Drop synth_req
		// and synth_bsy now so the second read sees no REQ and BIOS exits
		// the wait loop, then return to IDLE.
		data = 0x00;
		m_sasi_synth_req = false;
		m_sasi_synth_bsy = false;
		m_sasi_phase = SASI_IDLE;
		break;

	case SASI_IDLE:
	case SASI_SELECTING:
	case SASI_CMD_FIRST:
		// No data to deliver in these phases.
		break;
	}
	return data;
}


static DEVICE_INPUT_DEFAULTS_START(terminal)
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD",   0xff, RS232_BAUD_19200)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD",   0xff, RS232_BAUD_19200)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_8)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY",   0xff, RS232_PARITY_NONE)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_1)
DEVICE_INPUT_DEFAULTS_END

// ============================================================================
//  t250_ccm_device — Comms Control Module (CCM) board.
//
//  Physically a separate PCB carrying the Intel 8251 USART, the Intel 8253
//  PIT (counter 2 generates baud clock for both TxC and RxC), and the
//  RS-232 port.  Standard on every T-200/T-250 (Dave confirmed not user-
//  optional), but lives on its own card edge so a device-class
//  encapsulation is the right shape.  Patrick's review (PR #15372,
//  line 1335) asked for this split.
// ============================================================================

class t250_ccm_device : public device_t
{
public:
	t250_ccm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	u8   pit_r(offs_t offset)            { return m_pit->read(offset); }
	void pit_w(offs_t offset, u8 data)   { m_pit->write(offset, data); }
	u8   usart_r(offs_t offset)          { return m_usart->read(offset); }
	void usart_w(offs_t offset, u8 data);   // 0xA4/A5 — first-mode-byte quirk
	void mode_w(u8 data);                   // 0xA6 — CCM mode register
	u8   cicts_r();                         // 0xA7 — CI/CTS status

	auto rxrdy_handler() { return m_rxrdy_cb.bind(); }
	auto txrdy_handler() { return m_txrdy_cb.bind(); }
	bool rxrdy_state() const { return m_rxrdy_state; }
	bool txrdy_state() const { return m_txrdy_state; }
	bool cts_r() const       { return m_cts; }
	bool ci_r()  const       { return m_ri;  }

	// USART input lines forwarded to the CCM's 8251.  These are
	// invoked by the host's RS232 port callbacks; the inverse paths
	// (TX/DTR/RTS out of the 8251) are exposed by hooking the host's
	// m_rs232 directly into our usart().
	void rxd_w(int state) { m_usart->write_rxd(state); }
	void dsr_w(int state) { m_usart->write_dsr(state); }
	void cts_w(int state) { rs232_cts_w(state); }
	void ri_w(int state)  { rs232_ri_w(state);  }
	i8251_device &usart() const { return *m_usart; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void usart_rxrdy_w(int state);
	void usart_txrdy_w(int state);
	void rs232_cts_w(int state);
	void rs232_ri_w(int state);

	required_device<pit8253_device>    m_pit;
	required_device<i8251_device>      m_usart;

	devcb_write_line m_rxrdy_cb;
	devcb_write_line m_txrdy_cb;

	bool m_rxrdy_state = false;
	bool m_txrdy_state = false;
	bool m_usart_inited = false;
	bool m_cts = true;
	bool m_ri  = false;
};

DEFINE_DEVICE_TYPE_PRIVATE(T250_CCM, t250_ccm_device, t250_ccm_device, "t250_ccm", "Toshiba T-200/T-250 CCM (Comms Control Module)")

t250_ccm_device::t250_ccm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, T250_CCM, tag, owner, clock)
	, m_pit(*this, "pit")
	, m_usart(*this, "usart")
	, m_rxrdy_cb(*this)
	, m_txrdy_cb(*this)
{
}

void t250_ccm_device::device_start()
{
	save_item(NAME(m_rxrdy_state));
	save_item(NAME(m_txrdy_state));
	save_item(NAME(m_usart_inited));
	save_item(NAME(m_cts));
	save_item(NAME(m_ri));
}

void t250_ccm_device::device_reset()
{
	m_usart->write_cts(0);  // 8251 /CTS tied low on real HW; BIOS handles flow via port 0xA7
	m_rxrdy_state = false;
	m_txrdy_state = false;
	m_usart_inited = false;
	m_cts = true;
	m_ri  = false;
}

void t250_ccm_device::device_add_mconfig(machine_config &config)
{
	// CCM-board baud clock is 1.9968 MHz (15.9744 MHz crystal / 8), not the
	// standard 1.8432 MHz async baud crystal.  Per Dave from the set utility's
	// divisor table: 19200 -> 6 gives 1996800/6/16 = 20800 (~8% high; lower
	// rates are exact).
	PIT8253(config, m_pit, 0);
	m_pit->set_clk<0>(1'996'800);
	m_pit->set_clk<1>(1'996'800);
	m_pit->set_clk<2>(1'996'800);
	m_pit->out_handler<2>().set(m_usart, FUNC(i8251_device::write_txc));
	m_pit->out_handler<2>().append(m_usart, FUNC(i8251_device::write_rxc));

	I8251(config, m_usart, 1'843'200);
	// 8251 outputs route to the host's system-level RS232 port via
	// parent-tag ("^rs232").  The host owns the RS232 connector so
	// users can address it as -rs232 X at the command line.
	m_usart->txd_handler().set("^rs232", FUNC(rs232_port_device::write_txd));
	m_usart->dtr_handler().set("^rs232", FUNC(rs232_port_device::write_dtr));
	m_usart->rts_handler().set("^rs232", FUNC(rs232_port_device::write_rts));
	m_usart->rxrdy_handler().set(FUNC(t250_ccm_device::usart_rxrdy_w));
	m_usart->txrdy_handler().set(FUNC(t250_ccm_device::usart_txrdy_w));
	// txd/dtr/rts/rxd/dsr/cts/ri are wired by the host (t250_state)
	// because the RS232 connector lives at the system level so that
	// the user can address it as -rs232 X at the command line.
}


void t250_ccm_device::usart_rxrdy_w(int state)
{
	m_rxrdy_state = state;
	m_rxrdy_cb(state);
}

void t250_ccm_device::usart_txrdy_w(int state)
{
	m_txrdy_state = state;
	m_txrdy_cb(state);
}

// CCM CI/CTS status register (0xA7).  BIOS reads bit 5 to test /CTS
// (active low: 0 = ready to send).
u8 t250_ccm_device::cicts_r()
{
	u8 d = 0xff;
	if (m_cts) d &= ~0x20;
	return d;
}

// CCM mode register (0xA6).  BIOS init writes 0x18 then 0x00 around
// CCM mode register write ("asyn - rxdinh - txdwa" pulse sequence per
// BIOS handler.lib).  Exact function not modeled — BIOS just pulses
// 0x18 then 0x00 during init; no observable side-effect on emulated
// traffic so writes are dropped.
void t250_ccm_device::mode_w(u8 data)
{
}

// MAME RS232 convention: cts_handler is called with state=0 for "CTS asserted"
// (clear to send) and state=1 for deasserted.  We track the inverted value so
// m_cts == true means "asserted" (matches cicts_r / ios_r consumers).
// Same for RI.
void t250_ccm_device::rs232_cts_w(int state) { m_cts = !state; }

// 8251 write port shim.  BIOS initiu writes a stale A=0x18 to the 8251 data
// register (port 0xA4) *before* having set up the mode byte.  On real
// hardware the 8251 ignores CPU writes until properly initialized, but
// MAME's i8251 accepts the byte and locks TX_READY=0 forever because TxEN
// is also 0 at that moment and start_tx() never fires.  We drop data-port
// writes until the BIOS sets the mode byte (any write to 0xA5 after a
// reset / internal reset).  Control-port writes pass through normally so
// the 8251 can transition through its mode/cmd init sequence.
void t250_ccm_device::usart_w(offs_t offset, u8 data)
{
	if (offset == 0)  // 0xA4 = data port
	{
		if (!m_usart_inited)
			return;
	}
	else  // 0xA5 = control port
	{
		m_usart_inited = true;
	}
	m_usart->write(offset, data);
}
void t250_ccm_device::rs232_ri_w(int state)  { m_ri  = !state; }


class t250_state : public driver_device
{
public:
	t250_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dmac(*this, "dmac")
		, m_kbc(*this, "kbc")
		, m_crtc(*this, "crtc")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:%u", 0U)
		, m_centronics(*this, "centronics")
		, m_cent_data(*this, "cent_data")
		, m_sasi(*this, "sasi")
		, m_sasi_host(*this, "sasi_host")
		, m_beep(*this, "beep")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_rst65_merger(*this, "rst65_merger")
		, m_ccm(*this, "ccm")
		, m_rs232(*this, "rs232")
		, m_rom(*this, "maincpu")
		, m_dsw(*this, "DSW")
		, m_cfg(*this, "CFG")
		, m_jmp(*this, "JMP")
		, m_kbd(*this, "KBD%u", 0U)
		, m_kpd(*this, "KPD%u", 0U)
		, m_modifiers(*this, "MOD")
		, m_ram(*this, "ram", 0x10000, ENDIANNESS_LITTLE)
		, m_vram(*this, "vram", 0x800, ENDIANNESS_LITTLE)
		, m_vpg(*this, "vpg", 0x800, ENDIANNESS_LITTLE)
		, m_rom_view(*this, "rom_view")
		, m_vpg_view(*this, "vpg_view")
	{ }

	void t250(machine_config &config);
	void t200(machine_config &config);

private:
	void common(machine_config &config);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;


	void bank_w(u8 data);          // 0xC0
	void prnd_w(u8 data);          // 0xC1 (printer data + bank-switch select)
	void prn_cmd_w(u8 data);       // 0xC2 (printer command - bit 1 = strobe)
	void prn_ack_w(u8 data);       // 0xC3 (printer IRQ ack)
	u8 rdint_r();                  // 0xD0 (RST 6.5 source register)
	void kbd_irq_w(int state);     // 8279 IRQ (tracked for rdint)
	u8 dsw_r();                    // 0xD1
	u8 cfgsw_r();                  // 0xD2 (multiplexed config switch / jumper block)
	u8 ios_r();                    // 0xD3 (printer + comm status)
	u8 prnd_loopback_r();          // 0xD4 (undocumented printer-data loopback)
	void fdc_reset_w(u8 data);     // 0xF0 (write): external FDC reset latch
	void cent_busy_w(int state);
	void cent_select_w(int state);
	void cent_perror_w(int state);
	void cent_fault_w(int state);
	void kbd_scan_w(u8 data);      // 8279 SL output
	u8 kbd_rl_r();                 // 8279 RL input
	int kbd_shift_r();             // 8279 SHIFT input (bit 7 of FIFO, keypad - unused)
	int kbd_ctrl_r();              // 8279 CTRL/STROBE input (bit 6 of FIFO = shift key)

	void hrq_w(int state);
	void tc_w(int state);
	void fdc_irq_w(int state);
	void fdc_drq_w(int state);
	void dmac_mode_w(u8 data);
	u8 dma_loopback_r();
	void dma_loopback_w(u8 data);
	INTERRUPT_GEN_MEMBER(rst75_tick);
	u8 dma_memr(offs_t offset);
	void dma_memw(offs_t offset, u8 data);

	// SASI hard-disk host adapter (Intel 8155 bridge on real hardware).

	MC6845_UPDATE_ROW(crtc_update_row);

	required_device<i8085a_cpu_device> m_maincpu;
	required_device<i8257_device> m_dmac;
	required_device<i8279_device> m_kbc;
	required_device<hd6845s_device> m_crtc;
	required_device<upd765a_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data;
	required_device<nscsi_bus_device> m_sasi;
	required_device<t250_sasi_host_device> m_sasi_host;
	required_device<beep_device> m_beep;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<input_merger_device> m_rst65_merger;
	required_device<t250_ccm_device> m_ccm;
	required_device<rs232_port_device> m_rs232;
	required_region_ptr<u8> m_rom;
	required_ioport m_dsw;
	required_ioport m_cfg;
	required_ioport m_jmp;
	required_ioport_array<8> m_kbd;
	required_ioport_array<3> m_kpd;   // keypad rows 0..2 (3 RL positions x 8 scan)
	required_ioport m_modifiers;
	memory_share_creator<u8> m_ram;
	memory_share_creator<u8> m_vram;
	memory_share_creator<u8> m_vpg;
	memory_view m_rom_view;
	memory_view m_vpg_view;

	// Bank state.  Real hardware uses a counter pulsed by writes to 0xC0;
	// for the skeleton we hold ROM-at-low and video-RAM-at-high so the boot
	// ROM can at least begin executing.
	bool m_disp_enable = false;
	u8 m_prn_data = 0;
	u8 m_dma_temp = 0;        // mem-to-mem loopback (ch0 IOW -> ch1 IOR)
	bool m_dma_m2m_active = false;
	u8 m_kbd_scan = 0;        // current 8279 scan position (0-7)
	bool m_kbd_irq = false;   // latest 8279 IRQ state
	bool m_has_motor_ctrl = false;  // true for T-200 (5.25"): bit 6 of 0xF0 gates motor

	// Centronics printer signals (active-high in MAME's centronics: a 1 from
	// the device means the line is asserted in its sense).  m_prn_irq is
	// our latch, set on busy 1->0 transition, cleared by write to 0xC3.
	bool m_prn_busy   = false;
	bool m_prn_select = true;
	bool m_prn_perror = false;
	bool m_prn_fault  = false;
	bool m_prn_irq    = false;


	// SASI host-adapter state.  The bus + target slot are always
	// instantiated (so port reads return sensible "idle" values), but with
	// no hard disk mounted the bus stays quiescent and the BIOS hard-disk
	// path is never exercised.
	//
	// The BIOS' "hcon" routine sends a controller-specific drive-parameters
	// command block as the very first SASI command (opcode 0xC1 for the
	// DTC-510-style T-250 setup, opcode 0x0C for the Adaptec ACB-4000-style
	// T-200 setup).  Generic nscsi_harddisk does not implement these, so we
	// intercept them locally: pulse the bus to abort the target's command
	// phase, swallow the remaining payload bytes, then synthesize an
	// SS_GOOD status to the BIOS.  All subsequent commands (READ_6 0x08,
	// WRITE_6 0x0A, REQUEST_SENSE 0x03, TEST_UNIT_READY 0x00) pass through
	// to the bus.
	// Plain u8 phase tag (enum class doesn't satisfy MAME's save_item is_atom
};


void t250_state::machine_start()
{
	save_item(NAME(m_prn_data));
	save_item(NAME(m_dma_temp));
	save_item(NAME(m_dma_m2m_active));
	save_item(NAME(m_prn_busy));
	save_item(NAME(m_prn_select));
	save_item(NAME(m_prn_perror));
	save_item(NAME(m_prn_fault));
	save_item(NAME(m_prn_irq));
	save_item(NAME(m_disp_enable));

}

void t250_state::machine_reset()
{
	m_rom_view.select(0);   // boot ROM overlays 0x0000-0x0FFF
	m_vpg_view.select(0);   // video RAM at 0xF800-0xFFFF

	// uPD765A data rate matches the floppy subsystem clocked in the per-variant
	// machine config: 500 kbps for 8" DD (T-250), 250 kbps for 5.25" DD (T-200).
	// MAME's upd765 default of 250 kbps would halve the PLL clock at 8" rates
	// and fail to read any cells off IMD-built tracks.
	m_fdc->set_rate(m_fdc->clock() >= 8'000'000 ? 500000 : 250000);

	// 8251 CTS is hardwired to ground on the CCM board (always asserted)
	// so transmission isn't gated by external CTS.

}



void t250_state::bank_w(u8 data)
{
	// Bit-mapped write latch (functional spec p.27):
	//   0x20 DENB        display enable (1 = on)
	//   0x10 BUZZ        2 kHz keyboard buzzer (1 = on)
	//   0x08 RAM select  1 = DRAM at 0x0000-0x0FFF, 0 = boot ROM
	//   0x04 VPG select  1 = VPG (char gen RAM) at 0xF800, 0 = video RAM
	if (data & 0x08)
		m_rom_view.disable();   // DRAM exposed at 0x0000-0x0FFF
	else
		m_rom_view.select(0);   // boot ROM overlays 0x0000-0x0FFF
	m_vpg_view.select((data & 0x04) ? 1 : 0);
	// BUZZ: bit 4 gates a fixed ~2 kHz oscillator into the keyboard buzzer.
	// BIOS uses it for keyclick (pulsed ~500 ms via RST 7.5 buzcnt counter).
	m_beep->set_state(BIT(data, 4));
	// DENB: bit 5 enables the video output.  When cleared the CRTC still
	// runs but the pixel stream is gated off (screen reads as black).
	m_disp_enable = BIT(data, 5);
}

// RST 6.5 interrupt source register (read at 0xD0 by the BIOS rst65 handler).
// The handler checks bits 7..4 in turn for keyboard, comm-out, comm-in,
// and printer-out respectively (active LOW: bit clear = source is active).
// If all bits are high, the handler returns immediately.
u8 t250_state::rdint_r()
{
	u8 d = 0xff;
	if (m_kbd_irq)    d &= ~0x80;  // bit 7 = keyboard
	if (m_ccm->txrdy_state()) d &= ~0x40; // bit 6 = comm out (TxRDY)
	if (m_ccm->rxrdy_state()) d &= ~0x20; // bit 5 = comm in  (RxRDY)
	if (m_prn_irq)    d &= ~0x10;  // bit 4 = printer
	return d;
}

void t250_state::kbd_irq_w(int state)
{
	m_kbd_irq = state;
	m_rst65_merger->in_w<0>(state);
}

u8 t250_state::dsw_r()
{
	// Bit 5 is the live Ctrl-key state (active low).  Other bits are DIPs.
	u8 d = m_dsw->read();
	if (BIT(m_modifiers->read(), 1))
		d &= ~0x20;
	else
		d |= 0x20;
	return d;
}

void t250_state::kbd_scan_w(u8 data)
{
	m_kbd_scan = data & 7;
}

u8 t250_state::kbd_rl_r()
{
	// Main 8x8 matrix at the current scan position, merged (AND) with the
	// keypad's three rows at the same scan position.  Active-low: any key
	// pressed pulls its bit low, and BIOS sees it.
	u8 data = m_kbd[m_kbd_scan]->read();
	for (int i = 0; i < 3; i++)
		if (!BIT(m_kpd[i]->read(), m_kbd_scan))
			data &= ~(1 << i);
	return data;
}

// Bit 7 of the 8279 FIFO (= "rl8 / keypad flag" per BIOS kbint).  Real T-250
// hardware asserts this when a keypad key is sensed; we set it whenever any
// keypad key in the current scan position is pressed, so BIOS dispatches
// through the rl8 path into the keypad section of keytrans.
int t250_state::kbd_shift_r()
{
	for (int i = 0; i < 3; i++)
		if (!BIT(m_kpd[i]->read(), m_kbd_scan))
			return 1;
	return 0;
}

// Bit 6 of the 8279 FIFO is read by BIOS as the shift-key state.
int t250_state::kbd_ctrl_r()
{
	return BIT(m_modifiers->read(), 0);
}

void t250_state::prnd_w(u8 data)
{
	m_prn_data = data;
	m_cent_data->write(data);
}

// 0xC2 printer command.  Bit 1 is the data strobe to the printer (BIOS
// asserts then clears it after each data byte at 0xC1).  Other bits TBD.
void t250_state::prn_cmd_w(u8 data)
{
	m_centronics->write_strobe(BIT(data, 1));
}

// 0xC3 printer interrupt acknowledge.  Any write clears the printer-out
// IRQ latch (which would otherwise hold RST 6.5 asserted).
void t250_state::prn_ack_w(u8 data)
{
	(void)data;
	if (m_prn_irq)
	{
		m_prn_irq = false;
		m_rst65_merger->in_w<3>(0);
	}
}

// Port 0xD2 reads one of two switch banks on the main board, selected by
// the last value written to 0xC1 (manual p.33):
//   write 0x01 to 0xC1, read 0xD2 -> configuration DIP switch
//   write 0x02 to 0xC1, read 0xD2 -> jumper block (purpose TBD)
// The boot ROM and CP/M BIOS don't read 0xD2; only application programs do.
u8 t250_state::cfgsw_r()
{
	switch (m_prn_data)
	{
	case 0x01: return m_cfg->read();
	case 0x02: return m_jmp->read();
	default:   return 0xff;
	}
}

// Port 0xD3 combines printer (Centronics-style) status with CCM serial-board
// modem-control inputs:
//   bit 7  /CI   (active low)  - CCM Calling Indicator
//   bit 6  /CTS  (active low)  - CCM Clear To Send
//   bit 5  reserved
//   bit 4  printer SELECT     (1 = online)
//   bit 3  printer BUSY       (1 = busy)
//   bit 2  printer LD         (clock stop -- NOT USED)
//   bit 1  printer ERROR      (0 = no error)
//   bit 0  printer READY      (1 = ready / not paper-error)
u8 t250_state::ios_r()
{
	u8 d = 0xc0;                          // /CI and /CTS idle (both high)
	if (m_prn_select)         d |= 0x10;  // online
	if (m_prn_busy)           d |= 0x08;  // busy
	if (m_prn_fault)          d |= 0x02;  // error
	if (!m_prn_perror)        d |= 0x01;  // ready when paper-not-empty
	if (m_ccm->cts_r())       d &= ~0x40; // /CTS asserted -> bit 6 low
	if (m_ccm->ci_r())        d &= ~0x80; // /CI asserted -> bit 7 low
	return d;
}

void t250_state::cent_busy_w(int state)
{
	// Rising edge of BUSY -> deassert acknowledge.  Falling edge (printer
	// just became ready) latches the printer-out IRQ source for RST 6.5.
	if (m_prn_busy && !state)
	{
		m_prn_irq = true;
		m_rst65_merger->in_w<3>(1);
	}
	m_prn_busy = state;
}

void t250_state::cent_select_w(int state) { m_prn_select = state; }
void t250_state::cent_perror_w(int state) { m_prn_perror = state; }
// /FAULT is active LOW: state=1 means line HIGH = NO fault.  Store the
// active-true meaning here so ios_r can simply set bit 1 when fault asserted.
void t250_state::cent_fault_w(int state)  { m_prn_fault  = !state; }

// Boot ROM uses a write-0xC1 / read-0xD4 loopback to self-test the printer
// data path before declaring the CPU healthy.  Return the most recent
// printer-data byte to satisfy the test.
u8 t250_state::prnd_loopback_r()
{
	return m_prn_data;
}

// 0xF0 is the uPD765 main status register on reads, but on writes it
// drives an external latch on both T-200 and T-250:
//   bit 7 = FDC /RESET (both models): boot ROM writes 0x80 then 0x00.
//   bit 6 = 5.25" drive motor enable (T-200 only): boot ROM and BIOS
//           write 0x40 to spin up both 5.25" drives, then 0x00 to spin
//           down (BIOS RST 7.5 mocnt timer).
// On T-250 the 8" drives have continuously-spinning motors so bit 6 is
// unused; sending mon_w to the connector is harmless.
void t250_state::fdc_reset_w(u8 data)
{
	m_fdc->reset_w(BIT(data, 7));
	// On T-200 only, bit 6 also gates the 5.25" drive motor.  On T-250 the
	// 8" drives are motor_always_on and we must NOT drive mon_w (it would
	// override the always-on assertion and kill ready).
	if (m_has_motor_ctrl)
	{
		int motor = BIT(data, 6) ? 0 : 1;  // mon_w: 0=on, 1=off
		for (int i = 0; i < 2; i++)
			if (auto *fd = m_floppy[i]->get_device())
				fd->mon_w(motor);
	}
}


void t250_state::hrq_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state);
	m_dmac->hlda_w(state);
}

void t250_state::tc_w(int state)
{
	m_fdc->tc_w(state);
	// Do NOT release the m2m DREQs here.  Both ch0 and ch1 need to finish
	// individually (TC_STOP disables each as it hits its own TC).  Releasing
	// on the first TC pulse cuts the other channel off mid-transfer,
	// leaving ch0's status bit clear when ch1 finished first.
}

// The 8257 mode register at 0x88 is written with both ch0 and ch1 enable
// bits to start a memory-to-memory DMA (BIOS clsc/scroll).  Real T-250
// hardware drives DREQ0/1 internally for this; in MAME we have to assert
// them explicitly to make the 8257 run.
void t250_state::dmac_mode_w(u8 data)
{
	m_dmac->write(8, data);
	bool m2m_enable = ((data & 0x03) == 0x03);
	bool m2m_disable = ((data & 0x03) == 0x00);
	if (m2m_enable && !m_dma_m2m_active)
	{
		m_dma_m2m_active = true;
		m_dmac->dreq0_w(1);
		m_dmac->dreq1_w(1);
	}
	else if (m2m_disable && m_dma_m2m_active)
	{
		m_dma_m2m_active = false;
		m_dmac->dreq0_w(0);
		m_dmac->dreq1_w(0);
	}
}

// Loopback that lets the 8257 alternate ch0 (memory -> "device") and
// ch1 ("device" -> memory) to perform a memcpy.  m_dma_temp carries each
// byte between the two channels.
void t250_state::dma_loopback_w(u8 data) { m_dma_temp = data; }
u8   t250_state::dma_loopback_r()        { return m_dma_temp; }

void t250_state::fdc_irq_w(int state)
{
	m_maincpu->set_input_line(I8085_RST55_LINE, state);
}

void t250_state::fdc_drq_w(int state)
{
	m_dmac->dreq2_w(state);
}

// 20 Hz system tick.  On real hardware this is derived from the 6845
// horizontal sync via a divider chain; modeled here as a periodic interrupt
// until the divider is traced.
INTERRUPT_GEN_MEMBER(t250_state::rst75_tick)
{
	m_maincpu->pulse_input_line(I8085_RST75_LINE, attotime::zero);
}

u8 t250_state::dma_memr(offs_t offset)
{
	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

void t250_state::dma_memw(offs_t offset, u8 data)
{
	m_maincpu->space(AS_PROGRAM).write_byte(offset, data);
}


//**************************************************************************
//  SASI hard-disk host adapter
//
//  Real hardware bridges a SASI bus to the 8085 via an Intel 8155.  We do
//  not model the 8155 itself — we mimic the five port semantics the BIOS
//  actually uses.  The bus is always instantiated so reads return sensible
//  idle values; the default-empty target slot keeps the bus quiescent
//  unless the user attaches a hard disk image via `-sasi:0 harddisk
//  -hard1 <path>.chd` (see machine_config below).
//
//  All BIOS bit polarities verified against bios64.asm hsel/hconsel/hact/
//  hcon/hgetrs/hget/hput/req/req1 (lines 1298-1633).  KEY: hsts bit 7 is
//  INVERTED relative to SASI BSY (1 = idle, 0 = BSY asserted).
//**************************************************************************

// Pulse widths are not load-bearing on either the BIOS' polling code or
// the nscsi target; pick something nominal.



MC6845_UPDATE_ROW(t250_state::crtc_update_row)
{
	// Character cell is 10 px wide x 10 scanlines tall (ref manual p.39): 8
	// glyph pixels from the 74166 shift register bracketed by 1 blank pixel
	// each side for inter-character spacing.  Both video RAM (80x24 char
	// codes) and VPG (glyph bitmaps) are SRAMs continuously read by the 6845;
	// the CPU shares access at lower priority via the F800 bank-switch
	// (0xC0 bit 2).  Boot ROM loads a default font into VPG during init.
	// VPG layout: 128 chars * 16-byte slots = 2 KB exactly, only the first
	// 8-10 bytes per slot carry glyph data.  Bit 7 of vram drives the
	// hardware inverse-video XOR; address uses bits 0..6 of chr.  Inverse
	// covers the full 10x10 cell (side gap pixels and rasters 8-9 included).
	rgb_t const *const pen = m_palette->palette()->entry_list_raw();
	u32 *p = &bitmap.pix(y);

	// DENB (0xC0 bit 5) gates the pixel stream: when cleared the display is
	// blanked even though the CRTC continues to run.
	if (!m_disp_enable)
	{
		std::fill_n(p, x_count * 10, pen[0]);
		return;
	}

	for (int i = 0; i < x_count; i++)
	{
		u8 chr = m_vram[(ma + i) & 0x7ff];
		// All 10 cell rasters are populated by the boot-ROM-loaded font;
		// descenders (g, y, p, q, ...) may extend into rasters 8 and 9.
		u8 gfx = m_vpg[((chr & 0x7f) << 4) | ra];
		bool inv = (chr & 0x80) || (i == cursor_x);
		if (inv)
			gfx ^= 0xff;
		int gap = inv ? 1 : 0;
		*p++ = pen[gap];
		for (int b = 0; b < 8; b++)
			*p++ = pen[BIT(gfx, 7 - b)];
		*p++ = pen[gap];
	}
}


void t250_state::mem_map(address_map &map)
{
	// 64 KB DRAM is the base layer.  Two memory_view overlays handle the
	// hardware's bank-switched windows:
	//
	//   0x0000-0x0FFF : cold-boot ROM (m_rom_view state 0) vs DRAM
	//                   (m_rom_view disabled).  Even when ROM is being
	//                   read, writes pass through to the underlying DRAM
	//                   chip — that mirrors real hardware where the DRAM
	//                   is permanently wired and ignores the ROM-enable
	//                   line for write strobes.
	//   0xF800-0xFFFF : video RAM (m_vpg_view state 0) vs character-
	//                   generator RAM / VPG (state 1).  The underlying
	//                   DRAM at this range is never visible — one of the
	//                   two SRAMs is always selected.
	map(0x0000, 0xffff).ram().share("ram");

	map(0x0000, 0x0fff).view(m_rom_view);
	m_rom_view[0](0x0000, 0x0fff).rom().region("maincpu", 0)
		.lw8(NAME([this](offs_t offset, u8 data) { m_ram[offset] = data; }));

	map(0xf800, 0xffff).view(m_vpg_view);
	m_vpg_view[0](0xf800, 0xffff).ram().share("vram");
	m_vpg_view[1](0xf800, 0xffff).ram().share("vpg");
}

void t250_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	// SASI hard-disk host adapter — own device (see t250_sasi_host_device).
	// Ports 0x00-0x03 and 0x08 are unused unless a hard disk is attached
	// via -sasi:0 harddisk.
	map(0x00, 0x00).rw(m_sasi_host, FUNC(t250_sasi_host_device::hconfg_r), FUNC(t250_sasi_host_device::hconfg_w));
	map(0x01, 0x01).w(m_sasi_host, FUNC(t250_sasi_host_device::hdata_w));
	map(0x02, 0x02).r(m_sasi_host, FUNC(t250_sasi_host_device::hdata1_r));
	map(0x03, 0x03).r(m_sasi_host, FUNC(t250_sasi_host_device::hstat_r));
	map(0x08, 0x08).rw(m_sasi_host, FUNC(t250_sasi_host_device::hsts_r), FUNC(t250_sasi_host_device::hrset_w));
	map(0x80, 0x88).rw(m_dmac, FUNC(i8257_device::read), FUNC(i8257_device::write));
	map(0x88, 0x88).w(FUNC(t250_state::dmac_mode_w));   // intercept to trigger mem-to-mem
	map(0x90, 0x90).rw(m_kbc, FUNC(i8279_device::data_r), FUNC(i8279_device::data_w));
	map(0x91, 0x91).rw(m_kbc, FUNC(i8279_device::status_r), FUNC(i8279_device::cmd_w));
	// CCM (comms control module) board — own device, see t250_ccm_device.
	map(0xa0, 0xa3).rw(m_ccm, FUNC(t250_ccm_device::pit_r), FUNC(t250_ccm_device::pit_w));
	map(0xa4, 0xa5).rw(m_ccm, FUNC(t250_ccm_device::usart_r), FUNC(t250_ccm_device::usart_w));
	map(0xa6, 0xa6).w (m_ccm, FUNC(t250_ccm_device::mode_w));
	map(0xa7, 0xa7).r (m_ccm, FUNC(t250_ccm_device::cicts_r));
	map(0xc0, 0xc0).w(FUNC(t250_state::bank_w));
	map(0xc1, 0xc1).w(FUNC(t250_state::prnd_w));
	map(0xc2, 0xc2).w(FUNC(t250_state::prn_cmd_w));
	map(0xc3, 0xc3).w(FUNC(t250_state::prn_ack_w));
	map(0xd0, 0xd0).r(FUNC(t250_state::rdint_r));
	map(0xd1, 0xd1).r(FUNC(t250_state::dsw_r));
	map(0xd2, 0xd2).r(FUNC(t250_state::cfgsw_r));
	map(0xd3, 0xd3).r(FUNC(t250_state::ios_r));
	map(0xd4, 0xd4).r(FUNC(t250_state::prnd_loopback_r));
	map(0xe0, 0xe0).rw(m_crtc, FUNC(hd6845s_device::status_r), FUNC(hd6845s_device::address_w));
	map(0xe1, 0xe1).rw(m_crtc, FUNC(hd6845s_device::register_r), FUNC(hd6845s_device::register_w));
	map(0xf0, 0xf1).m(m_fdc, FUNC(upd765a_device::map));
	map(0xf0, 0xf0).w(FUNC(t250_state::fdc_reset_w));
}


// Keyboard.
//
// Physically the keyboard is on a separate PCB from the main board,
// connected via a high-pin-count round cable.  It is removable for
// service but is not a user-pluggable peripheral: there is no external
// keyboard port, no documented alternate keyboard for the T-200/T-250,
// and no period software that probes for a different layout.  For those
// reasons it is implemented inline here rather than as an external
// slot device.  If a second documented keyboard variant ever surfaces,
// the matrix + PORT_CHAR/keytrans table below can be lifted into a
// device class without touching the rest of the driver.
//
// The 8279 scans positions 0-7 (SL output); for each position it samples
// RL0-RL7.  The BIOS kbint handler builds a 6-bit key code from
// (return * 8) + scan and indexes into keytrans[].
//
// Active-low: a pressed key pulls the return line low.
//
// Layout per scan position (RL0..RL7):
//   scan 0: _    E   Z   9   J   /   F1   F9
//   scan 1: 1    R   X   0   K   -   F2   F10
//   scan 2: 2    T   C   Y   L   =   F3   Tab
//   scan 3: 3    A   V   U   ;   @   F4   Esc
//   scan 4: 4    S   B   I   N   ^   F5   Del
//   scan 5: 5    D   6   O   M   :   F6   Space
//   scan 6: Q    F   7   P   ,   ^C  F7   --
//   scan 7: W    G   8   H   .   CR  F8   --
//
// Function keys (RL6/RL7 of scans 0-1, RL6 of scans 2-7) are not mapped to
// host keys in this minimal cut.
static INPUT_PORTS_START(t250)
	PORT_START("KBD0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') PORT_CHAR(5)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR(26)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 )") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J') PORT_CHAR(10)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F9") PORT_CODE(KEYCODE_F9)

	PORT_START("KBD1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1 !") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R') PORT_CHAR(18)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR(24)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0 )") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') PORT_CHAR(11)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("- _") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F10") PORT_CODE(KEYCODE_F10)

	PORT_START("KBD2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 \"") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T') PORT_CHAR(20)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C') PORT_CHAR(3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y') PORT_CHAR(25)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR(12)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("= \\") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('\\')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)

	PORT_START("KBD3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') PORT_CHAR(1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') PORT_CHAR(22)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') PORT_CHAR(21)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("; +") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@ [") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@') PORT_CHAR('[')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)

	PORT_START("KBD4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 $") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') PORT_CHAR(19)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B') PORT_CHAR(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I') PORT_CHAR(9)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N') PORT_CHAR(14)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^ ]") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('^') PORT_CHAR(']')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del") PORT_CODE(KEYCODE_DEL) PORT_CHAR(127)

	PORT_START("KBD5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 %") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') PORT_CHAR(4)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 &") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR(15)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M') PORT_CHAR(13)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(": *") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F6") PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("KBD6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') PORT_CHAR(17)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F') PORT_CHAR(6)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 '") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR(16)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl-C") PORT_CODE(KEYCODE_END)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F7") PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KBD7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') PORT_CHAR(23)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G') PORT_CHAR(7)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 (") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H') PORT_CHAR(8)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F8") PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	// Keypad ("rl8" path).  3 rows of 8 keys.  When any of these is pressed,
	// kbd_shift_r returns 1, telling the BIOS to dispatch through keytrans
	// offset 256+ (the keypad section).  RL bits 0..2 carry the row data;
	// RL 3..7 are unused on the keypad side.
	PORT_START("KPD0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad <01h>")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad <FDh>")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad ^P")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad ^V")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad BS") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad <1Ch>")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad <1Eh>")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad <1Dh>")

	PORT_START("KPD1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP NAK")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP -") PORT_CODE(KEYCODE_MINUS_PAD)

	PORT_START("KPD2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP Enter") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP 0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP .") PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("MOD")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)

	PORT_START("DSW")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	// bit 5 is the live Ctrl key — read via dsw_r, not a DIP
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")

	// Configuration switch block (read at 0xD2 after writing 0x01 to 0xC1).
	// Default 0x1F matches the as-shipped T-250 setting (manual p.33).
	// The CFG DIP block is exposed at port 0xD2 (after writing 0x01 to 0xC1)
	// as a configuration source readable by software.  Stock V2.21 BIOS itself
	// only consults bits 0 (cold-boot source) and 1 (keyboard type); the
	// remaining bits (printer width, printer present, CCM presence) are
	// reported to application code which decides how to honour them.  None
	// of the printer DIPs reconfigure the Centronics slot device — they're
	// purely data for app-level use.  Alternate ROM implementations may
	// consult any subset; preserved here so any image can read them.
	PORT_START("CFG")
	PORT_DIPNAME(0x01, 0x01, "Cold boot source")              PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(   0x00, "Track 0, Head 0, Sector 1")
	PORT_DIPSETTING(   0x01, "Track 1, Head 0, Sector 1")
	PORT_DIPNAME(0x02, 0x02, "Keyboard type")                 PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(   0x00, "Standard")
	PORT_DIPSETTING(   0x02, "Touch-in")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPNAME(0x08, 0x08, "CCM (serial) board")            PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(   0x00, "Not present")
	PORT_DIPSETTING(   0x08, "Present")
	PORT_DIPNAME(0x10, 0x10, "Printer width")                 PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(   0x00, "80 columns")
	PORT_DIPSETTING(   0x10, "136 columns")
	PORT_DIPNAME(0x20, 0x00, "Printer")                       PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(   0x00, "Present")
	PORT_DIPSETTING(   0x20, "Not present")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x00, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x00, "SW2:8")

	// Jumper block (read at 0xD2 after writing 0x02 to 0xC1).  Wired pin
	// 1<->16, 2<->15, ... 8<->9 — purpose TBD.  Default 0xFF until usage
	// is documented.
	PORT_START("JMP")
	PORT_BIT(0xff, 0xff, IPT_UNKNOWN)
INPUT_PORTS_END


static void t250_floppies(device_slot_interface &device)
{
	device.option_add("8dsdd", FLOPPY_8_DSDD);
}

static void t200_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}

static void t250_floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_IMD_FORMAT);
	fr.add(FLOPPY_HFE_FORMAT);
}



void t250_state::common(machine_config &config)
{
	// 16 MHz master oscillator on board.  8085A clock input = 16/3 = 5.333 MHz
	// (internally divided by 2, yielding 2.667 MHz / 375 ns instruction phase).
	// 8257 DMA and 8279 KBC are clocked by the 8085's CLKOUT = 16/6 = 2.667 MHz.
	I8085A(config, m_maincpu, 16_MHz_XTAL / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &t250_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &t250_state::io_map);
	m_maincpu->set_periodic_int(FUNC(t250_state::rst75_tick), attotime::from_hz(20));

	// RST 6.5 sources OR'd into the 8085: keyboard (8279), comm in/out
	// (8251 RxRDY/TxRDY), and printer (centronics ACK).  rdint at 0xD0
	// reports the individual source states; the merger drives the CPU line.
	INPUT_MERGER_ANY_HIGH(config, m_rst65_merger).output_handler()
		.set_inputline(m_maincpu, I8085_RST65_LINE);

	I8257(config, m_dmac, 16_MHz_XTAL / 6);
	m_dmac->out_hrq_cb().set(FUNC(t250_state::hrq_w));
	m_dmac->out_tc_cb().set(FUNC(t250_state::tc_w));
	m_dmac->in_memr_cb().set(FUNC(t250_state::dma_memr));
	m_dmac->out_memw_cb().set(FUNC(t250_state::dma_memw));
	// Channels 0 and 1 are used by the CP/M BIOS for memory-to-memory
	// transfers (screen clear, scroll).  Their I/O sides loop back through
	// m_dma_temp so the 8257 effectively does a memcpy when both channels
	// are enabled together.
	m_dmac->out_iow_cb<0>().set(FUNC(t250_state::dma_loopback_w));
	m_dmac->in_ior_cb<1>().set(FUNC(t250_state::dma_loopback_r));
	// Channel 2 is wired to the uPD765 FDC for disk reads/writes.
	m_dmac->in_ior_cb<2>().set(m_fdc, FUNC(upd765a_device::dma_r));
	m_dmac->out_iow_cb<2>().set(m_fdc, FUNC(upd765a_device::dma_w));

	I8279(config, m_kbc, 16_MHz_XTAL / 6);
	m_kbc->out_irq_callback().set(FUNC(t250_state::kbd_irq_w));
	m_kbc->out_sl_callback().set(FUNC(t250_state::kbd_scan_w));
	m_kbc->in_rl_callback().set(FUNC(t250_state::kbd_rl_r));
	// MAME's i8279 places in_ctrl_cb result in bit 7 of the FIFO byte
	// and in_shift_cb result in bit 6.  The T-250 BIOS reads bit 7 as the
	// rl8/keypad flag and bit 6 as the shift key — so the wiring is the
	// opposite of the callback names.
	m_kbc->in_shift_callback().set(FUNC(t250_state::kbd_ctrl_r));  // shift state -> FIFO bit 6 (BIOS shift)
	m_kbc->in_ctrl_callback().set(FUNC(t250_state::kbd_shift_r));  // keypad flag -> FIFO bit 7 (BIOS rl8)

	// CCM serial board — own device.  See t250_ccm_device class above.
	T250_CCM(config, m_ccm);
	m_ccm->rxrdy_handler().set(m_rst65_merger, FUNC(input_merger_device::in_w<1>));
	m_ccm->txrdy_handler().set(m_rst65_merger, FUNC(input_merger_device::in_w<2>));

	// RS232 connector lives at the system level so users can address
	// it as -rs232 X at the command line.  Wire CCM <-> rs232 both ways.
	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->set_option_device_input_defaults("terminal",   DEVICE_INPUT_DEFAULTS_NAME(terminal));
	m_rs232->set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(terminal));
	m_rs232->rxd_handler().set(m_ccm, FUNC(t250_ccm_device::rxd_w));
	m_rs232->dsr_handler().set(m_ccm, FUNC(t250_ccm_device::dsr_w));
	m_rs232->cts_handler().set(m_ccm, FUNC(t250_ccm_device::cts_w));
	m_rs232->ri_handler().set(m_ccm, FUNC(t250_ccm_device::ri_w));

	// Default any attached terminal to match the BIOS' default mode
	// (8 data bits, 1 stop, no parity) at 19200 baud.  Users can override
	// per-session via -rs232:rxbaud / -rs232:txbaud.
	// Apply default 19200 8N1 (matching the BIOS' default mode byte 0x6E) to
	// the rs232 slot devices that have configurable bit-rate ports.  Loopback
	// has no baud setting (it echoes bits directly).  The "Serial Printer"
	// device only has RX (host→printer), so it gets a printer-specific defaults
	// block with no TX baud.

	// SASI bus for the optional hard-disk subsystem.  Slot id 0 is the
	// target (left empty by default — attach with `-sasi:0 harddisk
	// -hard1 <path>.chd`).  Slot id 7 is the host adapter (initiator);
	// NSCSI_CB exposes the raw bus lines for the t250_state h*_r/h*_w
	// methods that implement the 8155-bridge wire protocol.
	// NOTE: ALL 8 sasi:N connectors must be declared (even empty ones),
	// otherwise refid != SCSI id and ctrl_w(7,...)/data_w(7,...) writes
	// land in unused bus slots that the target can't see.  Confirmed by
	// reading nscsi_bus_device::device_resolve_objects() — m_devcnt only
	// increments for non-empty slots, so dense iteration matters.
	NSCSI_BUS(config, m_sasi);
	NSCSI_CONNECTOR(config, "sasi:0", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "sasi:1", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "sasi:2", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "sasi:3", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "sasi:4", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "sasi:5", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "sasi:6", default_scsi_devices, nullptr);
	// Instantiate the host adapter as an external device and attach it
	// to slot 7 on the bus.  set_external_device() is the modern
	// idiom: it bypasses the NSCSI_CONNECTOR/option_add_internal dance
	// that's deprecated, and the host device is allowed to subclass
	// nscsi_device_interface for direct m_scsi_refid access.
	T250_SASI_HOST(config, m_sasi_host);
	m_sasi->set_external_device(7, m_sasi_host);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(t250_state::cent_busy_w));
	m_centronics->select_handler().set(FUNC(t250_state::cent_select_w));
	m_centronics->perror_handler().set(FUNC(t250_state::cent_perror_w));
	m_centronics->fault_handler().set(FUNC(t250_state::cent_fault_w));
	OUTPUT_LATCH(config, m_cent_data);
	m_centronics->set_output_latch(*m_cent_data);

	// uPD765 FDC clock is set per variant in t250()/t200() (8 MHz for 8" DD,
	// 4 MHz for 5.25" DD at 300 RPM).  Machine-reset picks the matching data
	// rate from the configured clock.
	UPD765A(config, m_fdc, 0, true, true);
	m_fdc->intrq_wr_callback().set(FUNC(t250_state::fdc_irq_w));
	m_fdc->drq_wr_callback().set(FUNC(t250_state::fdc_drq_w));

	// Screen / CRTC.  16 MHz pixel clock from board oscillator.  Each
	// character cell is 10 px wide (per ref manual p.39): 8 glyph pixels
	// from the 74166 shift register flanked by a blank pixel on each side
	// for inter-character spacing.  Boot ROM programs the 6845 for:
	//   102 char x 26 row total, 80 x 24 displayed, 10 scanlines/cell
	//   -> 1020 x 262 raw, 800 x 240 visible, ~60 Hz vertical refresh.
	screen_device &screen(SCREEN(config, m_screen, SCREEN_TYPE_RASTER));
	screen.set_raw(16_MHz_XTAL, 1020, 0, 800, 262, 0, 240);
	screen.set_screen_update(m_crtc, FUNC(hd6845s_device::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	HD6845S(config, m_crtc, 16_MHz_XTAL / 10);
	m_crtc->set_screen(m_screen);
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(10);
	m_crtc->set_update_row_callback(FUNC(t250_state::crtc_update_row));

	// Keyboard buzzer: fixed ~2 kHz oscillator gated by BUZZ bit (0xC0 bit 4).
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 2000).add_route(ALL_OUTPUTS, "mono", 0.25);
}

void t250_state::t250(machine_config &config)
{
	common(config);
	// 8" DSDD: 500 kbps MFM at 360 RPM -> µPD765 clocked at 8 MHz.
	m_fdc->set_clock(8'000'000);
	FLOPPY_CONNECTOR(config, m_floppy[0], t250_floppies, "8dsdd", t250_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[1], t250_floppies, "8dsdd", t250_floppy_formats);
}

void t250_state::t200(machine_config &config)
{
	common(config);
	// 5.25" DSDD: 250 kbps MFM at 300 RPM -> µPD765 clocked at 4 MHz.
	m_fdc->set_clock(4'000'000);
	FLOPPY_CONNECTOR(config, m_floppy[0], t200_floppies, "525dd", t250_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[1], t200_floppies, "525dd", t250_floppy_formats);
	m_has_motor_ctrl = true;
}


ROM_START(t250)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("t250boot.bin", 0x0000, 0x1000, CRC(bf752b07) SHA1(57fc571dab686bd0eb01c62061153b58b8a90161))
ROM_END

ROM_START(t200)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("t200boot.bin", 0x0000, 0x1000, CRC(a73f4647) SHA1(f8f2eab78f993b0f086dfa995f3b9681d99fc08a))
ROM_END

} // anonymous namespace


//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY    FULLNAME  FLAGS
COMP( 1981, t250, 0,      0,      t250,    t250,  t250_state,  empty_init, "Toshiba", "T-250",  MACHINE_SUPPORTS_SAVE )
COMP( 1981, t200, 0,      0,      t200,    t250,  t250_state,  empty_init, "Toshiba", "T-200",  MACHINE_SUPPORTS_SAVE )
