// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, iq_132
/* for igs/pgmprot_orlegend.cpp type games */
#ifndef MAME_IGS_PGMPROT_ORLEGEND_H
#define MAME_IGS_PGMPROT_ORLEGEND_H

#pragma once

#include "igs003.h"

class pgm_asic3_state : public pgm_state
{
public:
	pgm_asic3_state(const machine_config &mconfig, device_type type, const char *tag)
		: pgm_state(mconfig, type, tag)
		, m_igs003(*this, "igs003")
	{
	}

	void init_orlegend() ATTR_COLD;
	void pgm_asic3(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<igs003_device> m_igs003;

	u8 m_igs003_out[3]{};

	u16 igs003_r();
	void igs003_w(offs_t offset, u16 data, u16 mem_mask = ~0);
};

INPUT_PORTS_EXTERN( orlegend );
INPUT_PORTS_EXTERN( orlegendt );
INPUT_PORTS_EXTERN( orlegendk );

#endif // MAME_IGS_PGMPROT_ORLEGEND_H
