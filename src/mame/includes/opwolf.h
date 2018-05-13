// license:GPL-2.0+
// copyright-holders:David Graves, Jarek Burczynski
/*************************************************************************

    Operation Wolf

*************************************************************************/
#ifndef MAME_INCLUDES_OPWOLF_H
#define MAME_INCLUDES_OPWOLF_H

#pragma once


#include "machine/taitocchip.h"

#include "sound/msm5205.h"
#include "video/pc080sn.h"
#include "video/pc090oj.h"


class opwolf_state : public driver_device
{
public:
	enum
	{
		TIMER_OPWOLF,
		TIMER_CCHIP
	};

	opwolf_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_cchip_ram(*this, "cchip_ram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_cchip(*this, "cchip"),
		m_pc080sn(*this, "pc080sn"),
		m_pc090oj(*this, "pc090oj"),
		m_msm1(*this, "msm1"),
		m_msm2(*this, "msm2")
	{ }

	DECLARE_CUSTOM_INPUT_MEMBER(opwolf_gun_x_r);
	DECLARE_CUSTOM_INPUT_MEMBER(opwolf_gun_y_r);
	void init_opwolf();
	void init_opwolfb();
	void init_opwolfp();
	void opwolf(machine_config &config);
	void opwolfb(machine_config &config);
	void opwolfp(machine_config &config);

protected:
	DECLARE_READ16_MEMBER(cchip_r);
	DECLARE_WRITE16_MEMBER(cchip_w);
	DECLARE_READ16_MEMBER(opwolf_in_r);
	DECLARE_READ16_MEMBER(opwolf_dsw_r);
	DECLARE_READ16_MEMBER(opwolf_lightgun_r);
	DECLARE_READ8_MEMBER(z80_input1_r);
	DECLARE_READ8_MEMBER(z80_input2_r);
	DECLARE_WRITE8_MEMBER(opwolf_adpcm_d_w);
	DECLARE_WRITE8_MEMBER(opwolf_adpcm_e_w);
	DECLARE_WRITE16_MEMBER(opwolf_cchip_status_w);
	DECLARE_WRITE16_MEMBER(opwolf_cchip_bank_w);
	DECLARE_WRITE16_MEMBER(opwolf_cchip_data_w);
	DECLARE_READ16_MEMBER(opwolf_cchip_status_r);
	DECLARE_READ16_MEMBER(opwolf_cchip_data_r);
	DECLARE_WRITE16_MEMBER(opwolf_spritectrl_w);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_WRITE8_MEMBER(opwolf_adpcm_b_w);
	DECLARE_WRITE8_MEMBER(opwolf_adpcm_c_w);

	virtual void machine_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	DECLARE_MACHINE_RESET(opwolf);
	uint32_t screen_update_opwolf(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(opwolf_timer_callback);
	TIMER_CALLBACK_MEMBER(cchip_timer);
	void updateDifficulty(int mode);
	void opwolf_cchip_init();
	void opwolf_msm5205_vck(msm5205_device *device, int chip);
	DECLARE_WRITE_LINE_MEMBER(opwolf_msm5205_vck_1);
	DECLARE_WRITE_LINE_MEMBER(opwolf_msm5205_vck_2);

	void opwolf_map(address_map &map);
	void opwolf_sound_z80_map(address_map &map);
	void opwolfb_map(address_map &map);
	void opwolfb_sub_z80_map(address_map &map);
	void opwolfp_map(address_map &map);

private:
	/* memory pointers */
	optional_shared_ptr<uint8_t> m_cchip_ram;

	/* video-related */
	uint16_t       m_sprite_ctrl;
	uint16_t       m_sprites_flipscreen;

	/* misc */
	uint8_t        m_adpcm_b[0x08];
	uint8_t        m_adpcm_c[0x08];
	uint32_t       m_adpcm_pos[2];
	uint32_t       m_adpcm_end[2];
	int          m_adpcm_data[2];

	int          m_opwolf_gun_xoffs;
	int          m_opwolf_gun_yoffs;

	emu_timer   *m_opwolf_timer;

	/* c-chip */
	emu_timer   *m_cchip_timer;

	int          m_opwolf_region;

	uint8_t        m_current_bank;
	uint8_t        m_current_cmd;
	uint8_t        m_cchip_last_7a;
	uint8_t        m_cchip_last_04;
	uint8_t        m_cchip_last_05;
	uint8_t        m_cchip_coins_for_credit[2];
	uint8_t        m_cchip_credits_for_coin[2];
	uint8_t        m_cchip_coins[2];
	uint8_t        m_c588;
	uint8_t        m_c589;
	uint8_t        m_c58a; // These variables derived from the bootleg
	uint8_t        m_triggeredLevel1b; // These variables derived from comparison to unprotection version
	uint8_t        m_triggeredLevel2;
	uint8_t        m_triggeredLevel2b;
	uint8_t        m_triggeredLevel2c;
	uint8_t        m_triggeredLevel3b;
	uint8_t        m_triggeredLevel13b;
	uint8_t        m_triggeredLevel4;
	uint8_t        m_triggeredLevel5;
	uint8_t        m_triggeredLevel7;
	uint8_t        m_triggeredLevel8;
	uint8_t        m_triggeredLevel9;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<taito_cchip_device> m_cchip;
	required_device<pc080sn_device> m_pc080sn;
	required_device<pc090oj_device> m_pc090oj;
	required_device<msm5205_device> m_msm1;
	required_device<msm5205_device> m_msm2;
};


#endif // MAME_INCLUDES_OPWOLF_H
