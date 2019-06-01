// license:BSD-3-Clause
// copyright-holders:hap, Sandro Ronco
/*

  The ChessMachine by Tasc

*/

#ifndef MAME_MACHINE_CHESSM_H
#define MAME_MACHINE_CHESSM_H

#pragma once

#include "cpu/arm/arm.h"


class chessmachine_device : public device_t
{
public:
	chessmachine_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	auto data_out() { return m_data_out.bind(); } // data_r

	// external read/write lines
	int data_r() { return m_latch[1]; }
	void data0_w(int state);
	void data1_w(int state);
	void reset_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset_after_children() override { m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE); }
	virtual void device_add_mconfig(machine_config &config) override;

	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	required_device<arm_cpu_device> m_maincpu;
	required_region_ptr<u8> m_bootstrap;

	devcb_write_line m_data_out;

	u8 m_latch[2];
	void sync0_callback(void *ptr, s32 param);
	void sync1_callback(void *ptr, s32 param);

	DECLARE_READ8_MEMBER(internal_r) { return m_latch[0]; }
	DECLARE_WRITE8_MEMBER(internal_w) { m_latch[1] = data & 1; m_data_out(m_latch[1]); }

	void main_map(address_map &map);
};


DECLARE_DEVICE_TYPE(CHESSMACHINE, chessmachine_device)

#endif // MAME_MACHINE_CHESSM_H
