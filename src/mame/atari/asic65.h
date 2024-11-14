// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************
 *
 *  Implementation of ASIC65
 *
 *************************************/
#ifndef MAME_ATARI_ASIC65_H
#define MAME_ATARI_ASIC65_H

#pragma once

#include "cpu/tms32010/tms32010.h"

enum {
	ASIC65_STANDARD,
	ASIC65_STEELTAL,
	ASIC65_GUARDIANS,
	ASIC65_ROMBASED
};

class asic65_device : public device_t
{
public:
	asic65_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, u8 type)
		: asic65_device(mconfig, tag, owner, clock)
	{
		set_type(type);
	}

	asic65_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	void set_type(u8 type) { m_asic65_type = type; }

	void reset_line(int state);
	void data_w(offs_t offset, u16 data);
	u16 read();
	u16 io_r();

	void m68k_w(u16 data);
	u16 m68k_r();
	void stat_w(u16 data);
	u16 stat_r();

	void asic65_io_map(address_map &map) ATTR_COLD;
	void asic65_program_map(address_map &map) ATTR_COLD;
protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(synced_write);

private:
	emu_timer *m_synced_write_timer;
	u8   m_asic65_type;
	int  m_command;
	u16  m_param[32];
	u16  m_yorigin;
	u8   m_param_index;
	u8   m_result_index;
	u8   m_reset_state;
	u8   m_last_bank;

	/* ROM-based interface states */
	required_device<tms32010_device> m_ourcpu;
	u8   m_tfull;
	u8   m_68full;
	u8   m_cmd;
	u8   m_xflg;
	u16  m_68data;
	u16  m_tdata;

	int get_bio();
};

DECLARE_DEVICE_TYPE(ASIC65, asic65_device)

#endif // MAME_ATARI_ASIC65_H
