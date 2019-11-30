// license:GPL-2.0+
// copyright-holders:Jarek Burczynski
/*************************************************************************

    Rastan

*************************************************************************/
#ifndef MAME_INCLUDES_RASTAN_H
#define MAME_INCLUDES_RASTAN_H

#pragma once

#include "machine/74157.h"
#include "sound/msm5205.h"
#include "video/pc080sn.h"
#include "video/pc090oj.h"

class rastan_state : public driver_device
{
public:
	rastan_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_audiobank(*this, "audiobank"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_adpcm_sel(*this, "adpcm_sel"),
		m_adpcm_data(*this, "adpcm"),
		m_pc080sn(*this, "pc080sn"),
		m_pc090oj(*this, "pc090oj")
	{ }

	void rastan(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_memory_bank m_audiobank;

	/* video-related */
	u16         m_sprite_ctrl;
	u16         m_sprites_flipscreen;

	/* misc */
	u16         m_adpcm_pos;
	bool        m_adpcm_ff;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm;
	required_device<ls157_device> m_adpcm_sel;
	required_region_ptr<u8> m_adpcm_data;
	required_device<pc080sn_device> m_pc080sn;
	required_device<pc090oj_device> m_pc090oj;

	void msm5205_address_w(u8 data);
	void spritectrl_w(u16 data);
	void sound_bankswitch_w(u8 data);
	void msm5205_start_w(u8 data);
	void msm5205_stop_w(u8 data);
	void rastan_colpri_cb(u32 &sprite_colbank, u32 &pri_mask, u16 sprite_ctrl);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(msm5205_vck);
	void main_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_RASTAN_H
