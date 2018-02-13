// license:BSD-3-Clause
// copyright-holders:Howie Cohen, Yochizo
// thanks-to:Richard Bush
#ifndef MAME_INCLUDES_TAITO_X_H
#define MAME_INCLUDES_TAITO_X_H

#pragma once

#include "includes/seta.h"
#include "machine/taitocchip.h"

class taitox_state : public seta_state
{
public:
	taitox_state(const machine_config &mconfig, device_type type, const char *tag)
		: seta_state(mconfig, type, tag),
		m_cchip(*this, "cchip")
	{ }

	optional_device<taito_cchip_device> m_cchip;

	DECLARE_READ16_MEMBER(superman_dsw_input_r);
	DECLARE_READ16_MEMBER(daisenpu_input_r);
	DECLARE_WRITE16_MEMBER(daisenpu_input_w);
	DECLARE_WRITE16_MEMBER(kyustrkr_input_w);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_DRIVER_INIT(kyustrkr);
	DECLARE_MACHINE_START(taitox);
	DECLARE_MACHINE_START(superman);

	// superman c-chip
	uint16_t m_current_bank;
	uint8_t m_cc_port;
	DECLARE_READ16_MEMBER( cchip1_ctrl_r );
	DECLARE_READ16_MEMBER( cchip1_ram_r );
	DECLARE_WRITE16_MEMBER( cchip1_ctrl_w );
	DECLARE_WRITE16_MEMBER( cchip1_bank_w );
	DECLARE_WRITE16_MEMBER( cchip1_ram_w );
	void superman(machine_config &config);
	void ballbros(machine_config &config);
	void gigandes(machine_config &config);
	void daisenpu(machine_config &config);
	void ballbros_map(address_map &map);
	void daisenpu_map(address_map &map);
	void daisenpu_sound_map(address_map &map);
	void gigandes_map(address_map &map);
	void sound_map(address_map &map);
	void superman_map(address_map &map);
};

#endif // MAME_INCLUDES_TAITO_X_H
