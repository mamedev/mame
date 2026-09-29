// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Codemasters CD Games Pack CD audio to joystick port adapter

**********************************************************************/

#ifndef MAME_BUS_VCS_CTRL_CDGAMES_H
#define MAME_BUS_VCS_CTRL_CDGAMES_H

#pragma once

#include "ctrl.h"
#include "imagedev/cdplayer.h"


class vcs_cd_adapter_device : public device_t, public device_vcs_control_port_interface
{
public:
	vcs_cd_adapter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_vcs_control_port_interface implementation
	virtual uint8_t vcs_joy_r() override { return m_joy; }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	static constexpr uint8_t LINE_UP = 0x01;
	static constexpr uint8_t LINE_FIRE = 0x20;
	static constexpr int EDGE_HIGH = 0x100;

	void sample_w(s16 left, s16 right);
	TIMER_CALLBACK_MEMBER(line_edge);

	required_device<cd_player_device> m_cd;
	emu_timer *m_edge_timer[2];
	cd_audio_comparator m_comparator[2];
	uint8_t m_joy;
};

DECLARE_DEVICE_TYPE(VCS_CD_ADAPTER, vcs_cd_adapter_device)

#endif // MAME_BUS_VCS_CTRL_CDGAMES_H
