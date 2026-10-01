// license:BSD-3-Clause
// copyright-holders:Mark Garlanger, Paul Galbraith
/***************************************************************************

  Heathkit H-17 Hard-sectored Floppy Disk Controller

  Heath built the same controller for both of its busses:

    H-88-1 - H89 right-hand slot (P506)
    H-8-17 - H8 Benton Harbor bus

  Both run the same 444-19 ROM, so both decode the same four ports and
  present the same registers. Only the way the card connects to the host
  differs, so everything except the bus attachment is here.

****************************************************************************/

#ifndef MAME_BUS_HEATHZENITH_H17_H17_FDC_BASE_H
#define MAME_BUS_HEATHZENITH_H17_H17_FDC_BASE_H

#pragma once

#include "imagedev/floppy.h"
#include "machine/s2350.h"
#include "machine/timer.h"


class heath_h17_fdc_base_device : public device_t
{
public:
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	void side_select_w(int state);

protected:
	static constexpr u8 MAX_FLOPPY_DRIVES = 3;

	heath_h17_fdc_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// Bit 7 of the control port write-enables the controller's 1k of RAM. On
	// the H-88-1 that RAM is on the H89 CPU board, so the bit leaves the card
	// as the P506 FMWE signal. On the H-8-17 the RAM is on the card itself.
	virtual void set_ram_write_enable(int state) = 0;

	void ctrl_w(u8 val);
	u8 floppy_status_r();

	static void floppy_formats(format_registration &fr);
	void set_floppy(floppy_image_device *floppy);
	void step_w(int state);
	void dir_w(int state);
	void set_motor(bool motor_on);
	void reset_rx_separator();
	void schedule_rx_cell();
	void rx_emit_cell();

	void sync_character_received(int state);

	TIMER_CALLBACK_MEMBER(rx_timer_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(tx_timer_cb);

	required_device<s2350_device> m_s2350;
	required_device_array<floppy_connector, MAX_FLOPPY_DRIVES> m_floppies;
	required_device<timer_device> m_tx_timer;
	emu_timer *m_rx_timer;

	attotime m_rx_cell_start;
	attotime m_rx_scan;
	bool     m_rx_have_clock;
	bool     m_rx_data;

	bool m_motor_on;
	bool m_write_gate;
	bool m_sync_char_received;
	u8   m_step_direction;
	u8   m_side;

	floppy_image_device *m_floppy;

	/// write bit control port
	static constexpr u8 CTRL_WRITE_GATE       = 0;
	static constexpr u8 CTRL_DRIVE_SELECT_0   = 1;
	static constexpr u8 CTRL_DRIVE_SELECT_1   = 2;
	static constexpr u8 CTRL_DRIVE_SELECT_2   = 3;
	static constexpr u8 CTRL_MOTOR_ON         = 4; // Controls all the drives
	static constexpr u8 CTRL_DIRECTION        = 5; // (0 = out)
	static constexpr u8 CTRL_STEP_COMMAND     = 6; // (Active high)
	static constexpr u8 CTRL_WRITE_ENABLE_RAM = 7; // 0 - write protected

	// USRT clock
	static constexpr XTAL USRT_BASE_CLOCK = XTAL(12'288'000) / 6 / 16;
	static constexpr u32  USRT_TX_CLOCK   = USRT_BASE_CLOCK.value();

	// A bit cell holds an FM clock half-cell followed by a data half-cell.
	static attotime fm_cell_time() { return attotime::from_hz(USRT_TX_CLOCK * 2); }
	static attotime fm_bit_time()  { return attotime::from_hz(USRT_TX_CLOCK); }
};

#endif // MAME_BUS_HEATHZENITH_H17_H17_FDC_BASE_H
