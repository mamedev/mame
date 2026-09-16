// license:BSD-3-Clause
// copyright-holders:David Haywood, ElSemi
#ifndef MAME_IGS_PGMPROT_IGS025_IGS022_H
#define MAME_IGS_PGMPROT_IGS025_IGS022_H

#pragma once

class pgm_022_025_state : public pgm_state
{
public:
	pgm_022_025_state(const machine_config &mconfig, device_type type, const char *tag)
		: pgm_state(mconfig, type, tag)
		, m_igs025(*this, "igs025")
		, m_igs022(*this, "igs022")
	{
	}

	void init_killbld() ATTR_COLD;
	void init_drgw3() ATTR_COLD;

	void pgm_022_025(machine_config &config) ATTR_COLD;
	void pgm_022_025_dw3(machine_config &config) ATTR_COLD;
	void pgm_022_025_killbld(machine_config &config) ATTR_COLD;

private:
	required_device<igs025_device> m_igs025;
	required_device<igs022_device> m_igs022;

	DECLARE_MACHINE_RESET(killbld);
	DECLARE_MACHINE_RESET(dw3);

	void igs025_to_igs022_callback();
	u8 killbld_source_data_r(u32 region, u8 addr);
	u8 drgw3_source_data_r(u32 region, u8 addr);

	void pgm_dw3_decrypt() ATTR_COLD;
	void pgm_killbld_decrypt() ATTR_COLD;

	void killbld_mem(address_map &map) ATTR_COLD;
};

INPUT_PORTS_EXTERN( killbld );
INPUT_PORTS_EXTERN( dw3 );
INPUT_PORTS_EXTERN( dw3j );

#endif // MAME_IGS_PGMPROT_IGS025_IGS022_H
