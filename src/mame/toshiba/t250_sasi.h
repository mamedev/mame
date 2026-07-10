// license:BSD-3-Clause
// copyright-holders:Dave L. Rand
#ifndef MAME_TOSHIBA_T250_SASI_H
#define MAME_TOSHIBA_T250_SASI_H

#pragma once

#include "machine/nscsi_bus.h"


// ============================================================================
//  t250_sasi_host_device — SASI host adapter (Toshiba bridge board).
//
//  Sits on the SASI bus as an external device at refid 7.  Bridges the 8085
//  CPU's I/O ports 0x00-0x08 onto the SASI control / data lines using a small
//  state machine that mimics the Intel 8155-based adapter board on real
//  hardware.
//
//  All BIOS bit polarities verified against bios64.asm hsel/hconsel/hact/hcon/
//  hgetrs/hget/hput/req/req1.  KEY: hsts bit 7 is INVERTED relative to SASI BSY
//  (1 = idle, 0 = BSY asserted).
//
//  The bus + target slot are always instantiated so port reads return sensible
//  idle values; with no hard disk mounted the bus stays quiescent and the BIOS
//  hard-disk path is never exercised.  The BIOS' "hcon" routine sends a
//  controller-specific drive-parameters command block as the very first SASI
//  command (opcode 0xC1 for the DTC-510-style setup).  Generic nscsi_harddisk
//  does not implement these, so we intercept them locally: pulse the bus to
//  abort the target's command phase, swallow the remaining payload bytes, then
//  synthesize an SS_GOOD status to the BIOS.  All subsequent commands (READ_6
//  0x08, WRITE_6 0x0A, REQUEST_SENSE 0x03, TEST_UNIT_READY 0x00) pass through.
//
//  TODO: replace the bespoke ACK/RST/SEL emu_timer pulses with an i8155_device
//  sub-instance whose internal timer drives those edges on real hardware.  For
//  now the I/O port semantics are preserved bit-for-bit so the existing CP/M
//  BIOS build continues to work without further changes.
// ============================================================================

class t250_sasi_host_device : public device_t, public nscsi_device_interface
{
public:
	t250_sasi_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// 8085 I/O ports 0x00-0x08, installed by the host driver via io_map().
	void io_map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// nscsi_device_interface — invoked whenever any control line on the bus
	// changes.  We track REQ and BSY edges and dispatch to our internal
	// handlers, matching the per-signal callbacks the old nscsi_callback_device
	// wiring used.
	virtual void scsi_ctrl_changed() override;

private:
	u8   hconfg_r();      // port 0x00 read  (REQ bit 1, active low)
	void hconfg_w(u8 d);  // port 0x00 write (8155 config: accept + ignore)
	void hdata_w(u8 d);   // port 0x01 write (data out + implicit ACK)
	u8   hdata1_r();      // port 0x02 read  (data in + implicit ACK)
	u8   hstat_r();       // port 0x03 read  (buffer-full bit 4, REQ bit 1)
	u8   hsts_r();        // port 0x08 read  (BSY inverted, C/D, I/O, MSG)
	void hrset_w(u8 d);   // port 0x08 write (controller reset)

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

	u8 m_sasi_phase;
	u8 m_sasi_cmd_first;
	u8 m_sasi_swallow_remaining;
	bool m_sasi_synth_bsy;
	bool m_sasi_synth_req;
	u32 m_prev_ctrl;               // last seen bus ctrl; edge-detect input
	emu_timer *m_sasi_ack_timer;
	emu_timer *m_sasi_sel_timer;
	emu_timer *m_sasi_rst_timer;
};

DECLARE_DEVICE_TYPE(T250_SASI_HOST, t250_sasi_host_device)

#endif // MAME_TOSHIBA_T250_SASI_H
