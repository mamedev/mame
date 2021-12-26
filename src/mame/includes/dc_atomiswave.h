// license:LGPL-2.1+
// copyright-holders: Samuele Zannoli, R. Belmont, ElSemi, David Haywood, Angelo Salese, Olivier Galibert, MetalliC

#ifndef MAME_INCLUDES_DC_ATOMISWAVE_H
#define MAME_INCLUDES_DC_ATOMISWAVE_H

#pragma once

#include "cpu/sh/sh4.h"
#include "cpu/arm7/arm7core.h"
#include "cpu/arm7/arm7.h"
#include "machine/awboard.h"
#include "machine/nvram.h"
#include "machine/aicartc.h"
#include "machine/m3comm.h"
#include "machine/gunsense.h"
#include "machine/segashiobd.h"
#include "sound/aica.h"
#include "dc.h"
#include "naomi.h"

class atomiswave_state : public dc_state
{
public:
	atomiswave_state(const machine_config &mconfig, device_type type, const char *tag)
		: dc_state(mconfig, type, tag)
		, m_awflash(*this, "awflash")
		, m_exid(*this, "EXID")
	   { }

	void aw_base(machine_config &config);
	void aw1c(machine_config &config);
	void aw2c(machine_config &config);
	void aw4c(machine_config &config);

	void init_atomiswave();

protected:
	virtual void aw_map(address_map &map);
	void aw_port(address_map &map);

private:
	required_device<macronix_29l001mc_device> m_awflash;
	optional_ioport m_exid;

	uint64_t aw_flash_r(offs_t offset);
	void aw_flash_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint32_t aw_modem_r(offs_t offset, uint32_t mem_mask = ~0);
	void aw_modem_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint8_t aw_ctrl_type;
//	inline int decode_reg32_64(uint32_t offset, uint64_t mem_mask, uint64_t *shift);
};

class atomiswave_xtrmhnt2_state : public atomiswave_state
{
public:
	atomiswave_xtrmhnt2_state(const machine_config &mconfig, device_type type, const char *tag)
		: atomiswave_state(mconfig, type, tag)
	   { }

protected:
	virtual void aw_map(address_map &map) override;
private:
	uint64_t allnet_hack_r(offs_t offset, uint64_t mem_mask = ~0);
};

#endif // MAME_INCLUDES_DC_ATOMISWAVE_H
