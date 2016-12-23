// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina

#include "machine/gen_latch.h"
#include "sound/msm5232.h"

class nycaptor_state : public driver_device
{
public:
	nycaptor_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_scrlram(*this, "scrlram"),
		m_sharedram(*this, "sharedram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_mcu(*this, "mcu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_scrlram;
	required_shared_ptr<uint8_t> m_sharedram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	std::vector<uint8_t> m_paletteram;
	std::vector<uint8_t> m_paletteram_ext;
	uint8_t m_gfxctrl;
	uint8_t m_char_bank;
	uint8_t m_palette_bank;

	/* mcu */
	uint8_t m_from_main;
	uint8_t m_from_mcu;
	int m_mcu_sent;
	int m_main_sent;
	uint8_t m_port_a_in;
	uint8_t m_port_a_out;
	uint8_t m_ddr_a;
	uint8_t m_port_b_in;
	uint8_t m_port_b_out;
	uint8_t m_ddr_b;
	uint8_t m_port_c_in;
	uint8_t m_port_c_out;
	uint8_t m_ddr_c;

	/* misc */
	int m_generic_control_reg;
	int m_sound_nmi_enable;
	int m_pending_nmi;
	uint8_t m_snd_data;
	int m_vol_ctrl[16];
	int  m_gametype;
	int m_mask;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	optional_device<cpu_device> m_mcu;
	required_device<msm5232_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch;

	DECLARE_WRITE8_MEMBER(sub_cpu_halt_w);
	DECLARE_READ8_MEMBER(from_snd_r);
	DECLARE_WRITE8_MEMBER(to_main_w);
	DECLARE_READ8_MEMBER(nycaptor_b_r);
	DECLARE_READ8_MEMBER(nycaptor_by_r);
	DECLARE_READ8_MEMBER(nycaptor_bx_r);
	DECLARE_WRITE8_MEMBER(sound_cpu_reset_w);
	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(nmi_disable_w);
	DECLARE_WRITE8_MEMBER(nmi_enable_w);
	DECLARE_READ8_MEMBER(nycaptor_generic_control_r);
	DECLARE_WRITE8_MEMBER(nycaptor_generic_control_w);
	DECLARE_READ8_MEMBER(cyclshtg_mcu_status_r);
	DECLARE_READ8_MEMBER(cyclshtg_mcu_r);
	DECLARE_WRITE8_MEMBER(cyclshtg_mcu_w);
	DECLARE_READ8_MEMBER(cyclshtg_mcu_status_r1);
	DECLARE_WRITE8_MEMBER(cyclshtg_generic_control_w);
	DECLARE_READ8_MEMBER(unk_r);
	DECLARE_READ8_MEMBER(nycaptor_68705_port_a_r);
	DECLARE_WRITE8_MEMBER(nycaptor_68705_port_a_w);
	DECLARE_WRITE8_MEMBER(nycaptor_68705_ddr_a_w);
	DECLARE_READ8_MEMBER(nycaptor_68705_port_b_r);
	DECLARE_WRITE8_MEMBER(nycaptor_68705_port_b_w);
	DECLARE_WRITE8_MEMBER(nycaptor_68705_ddr_b_w);
	DECLARE_READ8_MEMBER(nycaptor_68705_port_c_r);
	DECLARE_WRITE8_MEMBER(nycaptor_68705_port_c_w);
	DECLARE_WRITE8_MEMBER(nycaptor_68705_ddr_c_w);
	DECLARE_WRITE8_MEMBER(nycaptor_mcu_w);
	DECLARE_READ8_MEMBER(nycaptor_mcu_r);
	DECLARE_READ8_MEMBER(nycaptor_mcu_status_r1);
	DECLARE_READ8_MEMBER(nycaptor_mcu_status_r2);
	DECLARE_WRITE8_MEMBER(nycaptor_videoram_w);
	DECLARE_WRITE8_MEMBER(nycaptor_palette_w);
	DECLARE_READ8_MEMBER(nycaptor_palette_r);
	DECLARE_WRITE8_MEMBER(nycaptor_gfxctrl_w);
	DECLARE_READ8_MEMBER(nycaptor_gfxctrl_r);
	DECLARE_WRITE8_MEMBER(nycaptor_scrlram_w);
	DECLARE_WRITE8_MEMBER(unk_w);
	DECLARE_DRIVER_INIT(cyclshtg);
	DECLARE_DRIVER_INIT(colt);
	DECLARE_DRIVER_INIT(bronx);
	DECLARE_DRIVER_INIT(nycaptor);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_MACHINE_RESET(ta7630);
	uint32_t screen_update_nycaptor(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(nmi_callback);
	int nycaptor_spot(  );
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int pri );
};
