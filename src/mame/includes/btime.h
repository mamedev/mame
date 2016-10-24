// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari, Couriersud
/***************************************************************************

    Burger Time hardware

***************************************************************************/

#include "machine/gen_latch.h"

class btime_state : public driver_device
{
public:
	btime_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_rambase(*this, "rambase")
		, m_videoram(*this, "videoram")
		, m_colorram(*this, "colorram")
		, m_bnj_backgroundram(*this, "bnj_bgram")
		, m_zoar_scrollram(*this, "zoar_scrollram")
		, m_lnc_charbank(*this, "lnc_charbank")
		, m_deco_charram(*this, "deco_charram")
		, m_spriteram(*this, "spriteram")
		, m_audio_rambase(*this, "audio_rambase")
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch")
		, m_prom_region(*this, "proms")
	{
	}

	/* memory pointers */
	optional_shared_ptr<uint8_t> m_rambase;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	optional_shared_ptr<uint8_t> m_bnj_backgroundram;
	optional_shared_ptr<uint8_t> m_zoar_scrollram;
	optional_shared_ptr<uint8_t> m_lnc_charbank;
	optional_shared_ptr<uint8_t> m_deco_charram;
	optional_shared_ptr<uint8_t> m_spriteram;     // used by disco
//  uint8_t *  m_decrypted;
	optional_shared_ptr<uint8_t> m_audio_rambase;

	/* video-related */
	std::unique_ptr<bitmap_ind16> m_background_bitmap;
	uint8_t    m_btime_palette;
	uint8_t    m_bnj_scroll1;
	uint8_t    m_bnj_scroll2;
	uint8_t    m_btime_tilemap[4];

	/* audio-related */
	uint8_t    m_audio_nmi_enable_type;
	uint8_t    m_audio_nmi_enabled;
	uint8_t    m_audio_nmi_state;

	/* protection-related (for mmonkey) */
	int      m_protection_command;
	int      m_protection_status;
	int      m_protection_value;
	int      m_protection_ret;

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch;
	optional_memory_region m_prom_region;

	void audio_nmi_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void audio_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t audio_command_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t zoar_dsw1_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t wtennis_reset_hack_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mmonkey_protection_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mmonkey_protection_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lnc_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t btime_mirrorvideoram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t btime_mirrorcolorram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void btime_mirrorvideoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lnc_mirrorvideoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void btime_mirrorcolorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void deco_charram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bnj_background_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bnj_scroll1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bnj_scroll2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void btime_video_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bnj_video_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void zoar_video_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void disco_video_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void coin_inserted_irq_hi(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void coin_inserted_irq_lo(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void coin_inserted_nmi_lo(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void ay_audio_nmi_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void init_btime();
	void init_tisland();
	void init_cookrace();
	void init_zoar();
	void init_sdtennis();
	void init_wtennis();
	void init_bnj();
	void init_protennb();
	void init_disco();
	void init_lnc();
	void machine_start_btime();
	void machine_reset_btime();
	void palette_init_btime(palette_device &palette);
	void machine_reset_lnc();
	void palette_init_lnc(palette_device &palette);
	void machine_start_mmonkey();
	void machine_reset_mmonkey();
	void video_start_bnj();
	void video_start_disco();
	uint32_t screen_update_btime(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_cookrace(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_lnc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_eggs(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_bnj(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_zoar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_disco(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void audio_nmi_gen(timer_device &timer, void *ptr, int32_t param);
	void draw_chars( bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t transparency, uint8_t color, int priority );
	void draw_background( bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t* tmap, uint8_t color );
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t color,
							uint8_t sprite_y_adjust, uint8_t sprite_y_adjust_flip_screen,
							uint8_t *sprite_ram, offs_t interleave );

};
