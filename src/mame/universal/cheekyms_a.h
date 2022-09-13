// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_AUDIO_CHEEKYMS_H
#define MAME_AUDIO_CHEEKYMS_H

#pragma once

#include "machine/netlist.h"


class cheekyms_audio_device : public device_t
{
public:
	cheekyms_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE_LINE_MEMBER(mute_w);          // 15
	DECLARE_WRITE_LINE_MEMBER(cheese_w);        // 13
	DECLARE_WRITE_LINE_MEMBER(music_w);         // 14
	DECLARE_WRITE_LINE_MEMBER(mouse_w);         // 9
	DECLARE_WRITE_LINE_MEMBER(hammer_w);        // 12
	DECLARE_WRITE_LINE_MEMBER(pest_w);          // 8
	DECLARE_WRITE_LINE_MEMBER(mouse_dies_w);    // 11
	DECLARE_WRITE_LINE_MEMBER(pest_dies_w);     // 10
	DECLARE_WRITE_LINE_MEMBER(coin_extra_w);    // 16

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

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

#endif // MAME_AUDIO_CHEEKYMS_H
