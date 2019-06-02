// license:BSD-3-Clause
// copyright-holders:Howie Cohen, Yochizo
// thanks-to:Richard Bush
#ifndef MAME_INCLUDES_TAITO_X_H
#define MAME_INCLUDES_TAITO_X_H

#pragma once

#include "includes/seta.h"
#include "machine/taitocchip.h"
#include "machine/timer.h"

class taitox_state : public seta_state
{
public:
	taitox_state(const machine_config &mconfig, device_type type, const char *tag) :
		seta_state(mconfig, type, tag),
		m_cchip(*this, "cchip"),
		m_cchip_irq_clear(*this, "cchip_irq_clear"),
		m_z80bank(*this, "z80bank"),
		m_dswa_io(*this, "DSWA"),
		m_dswb_io(*this, "DSWB"),
		m_in_io(*this, "IN%u", 0U)
	{ }

	void superman(machine_config &config);
	void ballbros(machine_config &config);
	void kyustrkr(machine_config &config);
	void gigandes(machine_config &config);
	void daisenpu(machine_config &config);

private:
	DECLARE_READ16_MEMBER(superman_dsw_input_r);
	DECLARE_WRITE8_MEMBER(superman_counters_w);
	DECLARE_READ16_MEMBER(daisenpu_input_r);
	DECLARE_WRITE16_MEMBER(daisenpu_input_w);
	DECLARE_WRITE16_MEMBER(kyustrkr_input_w);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_MACHINE_START(taitox);

	INTERRUPT_GEN_MEMBER(interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(cchip_irq_clear_cb);

	void ballbros_map(address_map &map);
	void daisenpu_map(address_map &map);
	void daisenpu_sound_map(address_map &map);
	void gigandes_map(address_map &map);
	void kyustrkr_map(address_map &map);
	void sound_map(address_map &map);
	void superman_map(address_map &map);
	void taito_x_base_map(address_map &map);

	optional_device<taito_cchip_device> m_cchip;
	optional_device<timer_device> m_cchip_irq_clear;
	required_memory_bank m_z80bank;
	optional_ioport m_dswa_io;
	optional_ioport m_dswb_io;
	optional_ioport_array<3> m_in_io;
};

#endif // MAME_INCLUDES_TAITO_X_H
