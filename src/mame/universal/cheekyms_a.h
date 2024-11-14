// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_UNIVERSAL_CHEEKYMS_A_H
#define MAME_UNIVERSAL_CHEEKYMS_A_H

#pragma once

#include "machine/netlist.h"


class cheekyms_audio_device : public device_t
{
public:
	cheekyms_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void mute_w(int state);         // 15
	void cheese_w(int state);       // 13
	void music_w(int state);        // 14
	void mouse_w(int state);        // 9
	void hammer_w(int state);       // 12
	void pest_w(int state);         // 8
	void mouse_dies_w(int state);   // 11
	void pest_dies_w(int state);    // 10
	void coin_extra_w(int state);   // 16

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	required_device<netlist_mame_logic_input_device>    m_mute;
	required_device<netlist_mame_logic_input_device>    m_cheese;
	required_device<netlist_mame_logic_input_device>    m_music;
	required_device<netlist_mame_logic_input_device>    m_mouse;
	required_device<netlist_mame_logic_input_device>    m_hammer;
	required_device<netlist_mame_logic_input_device>    m_pest;
	required_device<netlist_mame_logic_input_device>    m_mouse_dies;
	required_device<netlist_mame_logic_input_device>    m_pest_dies;
	required_device<netlist_mame_logic_input_device>    m_coin_extra;
};

DECLARE_DEVICE_TYPE(CHEEKY_MOUSE_AUDIO, cheekyms_audio_device)

#endif // MAME_UNIVERSAL_CHEEKYMS_A_H
