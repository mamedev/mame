// license:GPL-2.0+
// copyright-holders:David Graves, Jarek Burczynski
/*************************************************************************

    Operation Wolf

*************************************************************************/
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

	opwolf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_cchip_ram(*this, "cchip_ram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_pc080sn(*this, "pc080sn"),
		m_pc090oj(*this, "pc090oj"),
		m_msm1(*this, "msm1"),
		m_msm2(*this, "msm2") { }

	/* memory pointers */
	optional_shared_ptr<UINT8> m_cchip_ram;

	/* video-related */
	UINT16       m_sprite_ctrl;
	UINT16       m_sprites_flipscreen;

	/* misc */
	UINT8        m_adpcm_b[0x08];
	UINT8        m_adpcm_c[0x08];
	UINT32       m_adpcm_pos[2];
	UINT32       m_adpcm_end[2];
	int          m_adpcm_data[2];

	int          m_opwolf_gun_xoffs;
	int          m_opwolf_gun_yoffs;

	/* c-chip */
	emu_timer   *m_cchip_timer;

	int          m_opwolf_region;

	UINT8        m_current_bank;
	UINT8        m_current_cmd;
	UINT8        m_cchip_last_7a;
	UINT8        m_cchip_last_04;
	UINT8        m_cchip_last_05;
	UINT8        m_cchip_coins_for_credit[2];
	UINT8        m_cchip_credits_for_coin[2];
	UINT8        m_cchip_coins[2];
	UINT8        m_c588;
	UINT8        m_c589;
	UINT8        m_c58a; // These variables derived from the bootleg

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<pc080sn_device> m_pc080sn;
	required_device<pc090oj_device> m_pc090oj;
	required_device<msm5205_device> m_msm1;
	required_device<msm5205_device> m_msm2;
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
	DECLARE_DRIVER_INIT(opwolf);
	DECLARE_DRIVER_INIT(opwolfb);
	DECLARE_DRIVER_INIT(opwolfp);


	DECLARE_CUSTOM_INPUT_MEMBER(opwolf_gun_x_r);
	DECLARE_CUSTOM_INPUT_MEMBER(opwolf_gun_y_r);


	virtual void machine_start() override;
	DECLARE_MACHINE_RESET(opwolf);
	UINT32 screen_update_opwolf(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(opwolf_timer_callback);
	TIMER_CALLBACK_MEMBER(cchip_timer);
	void updateDifficulty( int mode );
	void opwolf_cchip_init(  );
	void opwolf_msm5205_vck(msm5205_device *device, int chip);
	DECLARE_WRITE_LINE_MEMBER(opwolf_msm5205_vck_1);
	DECLARE_WRITE_LINE_MEMBER(opwolf_msm5205_vck_2);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
