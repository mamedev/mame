// license:BSD-3-Clause
// copyright-holders:David Haywood, ElSemi
#ifndef MAME_IGS_PGMPROT_IGS025_IGS012_H
#define MAME_IGS_PGMPROT_IGS025_IGS012_H

#pragma once

#include "igs012.h"

class pgm_012_025_state : public pgm_state
{
public:
	pgm_012_025_state(const machine_config &mconfig, device_type type, const char *tag)
		: pgm_state(mconfig, type, tag)
		, m_igs025(*this, "igs025")
		, m_igs012(*this, "igs012")
		, m_rom_view(*this, "rom_view")
	{
	}

	void init_drgw2() ATTR_COLD;
	void init_dw2v100x() ATTR_COLD;
	void init_drgw2c() ATTR_COLD;
	void init_drgw2c101() ATTR_COLD;
	void init_drgw2j() ATTR_COLD;
	void init_drgw2hk() ATTR_COLD;

	void pgm_012_025_drgw2(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<igs025_device> m_igs025;
	required_device<igs012_device> m_igs012;

	memory_view m_rom_view;

	u16 *m_rom = nullptr;

	u8 source_data_r(u32 region, u8 addr);

	void drgw2_common_init() ATTR_COLD;

	void pgm_drgw2_decrypt() ATTR_COLD;

	void drgw2_mem(address_map &map) ATTR_COLD;

	void igs012_regs_w(offs_t offset, u16 data, u16 mem_mask = ~0);
};

#endif // MAME_IGS_PGMPROT_IGS025_IGS012_H
