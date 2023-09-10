// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/***************************************************************************

  Nintendo 8080 sound emulation

***************************************************************************/
#ifndef MAME_NINTENDO_N8080_A_H
#define MAME_NINTENDO_N8080_A_H

#pragma once

#include "cpu/mcs48/mcs48.h"


class n8080_sound_device_base : public device_t
{
public:
	void sound1_w(u8 data);
	void sound2_w(u8 data);

protected:
	n8080_sound_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *parent, u32 clock);

	required_device<i8035_device> m_cpu;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	u16 current_pins() const { return m_curr_pins; }

private:
	virtual void pins_changed(u16 curr, u16 prev) = 0;

	void prg_map(address_map &map) ATTR_COLD;

	u16 m_curr_pins;
};



DECLARE_DEVICE_TYPE(SPACEFEV_SOUND, n8080_sound_device_base)
DECLARE_DEVICE_TYPE(SHERIFF_SOUND,  n8080_sound_device_base)
DECLARE_DEVICE_TYPE(HELIFIRE_SOUND, n8080_sound_device_base)

#endif // MAME_NINTENDO_N8080_A_H
