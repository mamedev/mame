// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Manuel Abadia
/*************************************************************************

    Chequered Flag

*************************************************************************/
#ifndef MAME_INCLUDES_CHQFLAG_H
#define MAME_INCLUDES_CHQFLAG_H

#pragma once

#include "machine/bankdev.h"
#include "sound/k007232.h"
#include "video/k051960.h"
#include "video/k051316.h"
#include "video/k051733.h"
#include "video/konami_helper.h"
#include "emupal.h"

class chqflag_state : public driver_device
{
public:
	chqflag_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_bank1000(*this, "bank1000")
		, m_k007232(*this, "k007232_%u", 1)
		, m_k051960(*this, "k051960")
		, m_k051316(*this, "k051316_%u", 1)
		, m_palette(*this, "palette")
		, m_rombank(*this, "rombank")
	{
	}

	template<int Chip> DECLARE_READ8_MEMBER(k051316_ramrom_r);
	DECLARE_WRITE8_MEMBER(chqflag_bankswitch_w);
	DECLARE_WRITE8_MEMBER(chqflag_vreg_w);
	DECLARE_WRITE8_MEMBER(select_analog_ctrl_w);
	DECLARE_READ8_MEMBER(analog_read_r);
	DECLARE_WRITE8_MEMBER(k007232_bankswitch_w);
	DECLARE_WRITE8_MEMBER(k007232_extvolume_w);
	DECLARE_WRITE8_MEMBER(volume_callback0);
	DECLARE_WRITE8_MEMBER(volume_callback1);
	DECLARE_WRITE_LINE_MEMBER(background_brt_w);
	K051316_CB_MEMBER(zoom_callback_1);
	K051316_CB_MEMBER(zoom_callback_2);
	K051960_CB_MEMBER(sprite_callback);
	uint32_t screen_update_chqflag(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void chqflag(machine_config &config);
	void bank1000_map(address_map &map);
	void chqflag_map(address_map &map);
	void chqflag_sound_map(address_map &map);
protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
private:
	/* misc */
	int        m_k051316_readroms;
	int        m_last_vreg;
	int        m_analog_ctrl;
	int        m_accel;
	int        m_wheel;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<address_map_bank_device> m_bank1000;
	required_device_array<k007232_device, 2> m_k007232;
	required_device<k051960_device> m_k051960;
	required_device_array<k051316_device, 2> m_k051316;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_memory_bank m_rombank;
	void update_background_shadows(uint8_t data);
};

#endif // MAME_INCLUDES_CHQFLAG_H
