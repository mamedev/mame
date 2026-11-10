// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Codemasters CD Games Pack CD audio to joystick port adapter

**********************************************************************/

#ifndef MAME_BUS_VCS_CTRL_CDGAMES_H
#define MAME_BUS_VCS_CTRL_CDGAMES_H

#pragma once

#include "ctrl.h"
#include "sound/cdda.h"
#include "sound/zcross.h"


class vcs_cd_adapter_device :
	public device_t,
	public device_sound_interface,
	public device_vcs_control_port_interface,
	public device_cdda_player_interface
{
public:
	vcs_cd_adapter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_vcs_control_port_interface implementation
	virtual uint8_t vcs_joy_r() override { return m_joy; }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_sound_interface implementation
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	static constexpr uint8_t LINE_UP = 0x01;
	static constexpr uint8_t LINE_FIRE = 0x20;
	static constexpr int EDGE_HIGH = 0x100;

	TIMER_CALLBACK_MEMBER(line_edge);

	emu_timer *m_edge_timer[2];
	zero_crossing_comparator m_comparator[2];
	uint8_t m_joy;
};

DECLARE_DEVICE_TYPE(VCS_CD_ADAPTER, vcs_cd_adapter_device)

#endif // MAME_BUS_VCS_CTRL_CDGAMES_H
