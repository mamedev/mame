// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Rainbow Arts CD audio to datassette port adapter

**********************************************************************/

#ifndef MAME_BUS_PET_CDAUDIO_H
#define MAME_BUS_PET_CDAUDIO_H

#pragma once

#include "cass.h"
#include "imagedev/cdplayer.h"


class cd_audio_adapter_device : public device_t, public device_pet_datassette_port_interface
{
public:
	cd_audio_adapter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_pet_datassette_port_interface implementation
	virtual int datassette_read() override { return m_read; }
	virtual int datassette_sense() override { return 0; }

private:
	void sample_w(s16 left, s16 right);
	TIMER_CALLBACK_MEMBER(read_edge);

	required_device<cd_player_device> m_cd;
	emu_timer *m_edge_timer;
	cd_audio_comparator m_comparator;
	u8 m_read;
};

DECLARE_DEVICE_TYPE(PET_CD_ADAPTER, cd_audio_adapter_device)

#endif // MAME_BUS_PET_CDAUDIO_H
