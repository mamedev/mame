// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Nicola Salmoria
/*
    buggychl
*/

#include "machine/buggychl.h"
#include "machine/gen_latch.h"
#include "sound/msm5232.h"

class buggychl_state : public driver_device
{
public:
	buggychl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_charram(*this, "charram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_scrollv(*this, "scrollv"),
		m_scrollh(*this, "scrollh"),
		m_audiocpu(*this, "audiocpu"),
		m_maincpu(*this, "maincpu"),
		m_mcu(*this, "mcu"),
		m_bmcu(*this, "bmcu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") ,
		m_soundlatch(*this, "soundlatch") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_charram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_scrollv;
	required_shared_ptr<uint8_t> m_scrollh;

	/* video-related */
	bitmap_ind16 m_tmp_bitmap1;
	bitmap_ind16 m_tmp_bitmap2;
	tilemap_t     *m_bg_tilemap;
	int         m_sl_bank;
	int         m_bg_on;
	int         m_sky_on;
	int         m_sprite_color_base;
	int         m_bg_scrollx;
	uint8_t       m_sprite_lookup[0x2000];

	/* sound-related */
	int         m_sound_nmi_enable;
	int         m_pending_nmi;

	/* devices */
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_mcu;
	required_device<buggychl_mcu_device> m_bmcu;
	required_device<msm5232_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	void bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nmi_disable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nmi_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void buggychl_chargen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void buggychl_sprite_lookup_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void buggychl_sprite_lookup_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void buggychl_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void buggychl_bg_scrollx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void port_a_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void port_b_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void port_a_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void port_b_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_buggychl(palette_device &palette);
	uint32_t screen_update_buggychl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void nmi_callback(void *ptr, int32_t param);
	void draw_sky( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_bg( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_fg( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
};
