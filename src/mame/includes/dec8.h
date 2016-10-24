// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#include "machine/gen_latch.h"
#include "sound/msm5205.h"
#include "video/bufsprite.h"
#include "video/decbac06.h"
#include "video/deckarn.h"
#include "video/decmxc06.h"

class dec8_state : public driver_device
{
public:
	enum
	{
		TIMER_DEC8_I8751
	};

	dec8_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_spriteram(*this, "spriteram") ,
		m_msm(*this, "msm"),
		m_tilegen1(*this, "tilegen1"),
		m_tilegen2(*this, "tilegen2"),
		m_spritegen_krn(*this, "spritegen_krn"),
		m_spritegen_mxc(*this, "spritegen_mxc"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_videoram(*this, "videoram"),
		m_bg_data(*this, "bg_data") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_mcu;
	required_device<buffered_spriteram8_device> m_spriteram;
	optional_device<msm5205_device> m_msm;
	optional_device<deco_bac06_device> m_tilegen1;
	optional_device<deco_bac06_device> m_tilegen2;
	optional_device<deco_karnovsprites_device> m_spritegen_krn;
	optional_device<deco_mxc06_device> m_spritegen_mxc;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint8_t> m_bg_data;

	uint8_t *  m_pf1_data;
	uint8_t *  m_row;
	uint16_t   m_buffered_spriteram16[0x800/2]; // for the mxc06 sprite chip emulation (oscar, cobra)

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_pf1_tilemap;
	tilemap_t  *m_fix_tilemap;
	//int      m_scroll1[4];
	int      m_scroll2[4];
	int      m_bg_control[0x20];
	int      m_pf1_control[0x20];
	int      m_game_uses_priority;

	/* misc */
	int      m_i8751_port0;
	int      m_i8751_port1;
	int      m_nmi_enable;
	int      m_i8751_return;
	int      m_i8751_value;
	int      m_coinage_id;
	int      m_coin1;
	int      m_coin2;
	int      m_need1;
	int      m_need2;
	int      m_cred1;
	int      m_cred2;
	int      m_credits;
	int      m_latch;
	int      m_snd;
	int      m_msm5205next;
	int      m_toggle;

	void dec8_mxc06_karn_buffer_spriteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t i8751_h_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t i8751_l_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void i8751_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t gondo_player_1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t gondo_player_2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dec8_i8751_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lastmisn_i8751_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void shackled_i8751_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void csilver_i8751_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void srdarwin_i8751_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dec8_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ghostb_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void csilver_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dec8_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void csilver_adpcm_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void csilver_sound_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void oscar_int_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void shackled_int_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flip_screen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t dec8_mcu_from_main_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dec8_mcu_to_main_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dec8_bg_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t dec8_bg_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dec8_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void srdarwin_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dec8_scroll2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void srdarwin_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lastmisn_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void shackled_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lastmisn_scrollx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lastmisn_scrolly_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gondo_scroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t csilver_adpcm_reset_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void init_dec8();
	void init_deco222();
	void init_meikyuh();
	void init_garyoret();
	void init_shackled();
	void init_cobracom();
	void init_csilver();
	void init_ghostb();
	void init_srdarwin();
	void init_lastmisn();
	void init_gondo();
	void init_oscar();
	void get_cobracom_fix_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_ghostb_fix_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_oscar_fix_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	tilemap_memory_index lastmisn_scan_rows(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void get_lastmisn_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_lastmisn_fix_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_srdarwin_fix_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_srdarwin_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_gondo_fix_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_gondo_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void video_start_lastmisn();
	void video_start_shackled();
	void video_start_gondo();
	void video_start_garyoret();
	void video_start_ghostb();
	void palette_init_ghostb(palette_device &palette);
	void video_start_oscar();
	void video_start_srdarwin();
	void video_start_cobracom();
	uint32_t screen_update_lastmisn(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_shackled(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_gondo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_garyoret(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_ghostb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_oscar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_srdarwin(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_cobracom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_dec8(screen_device &screen, bool state);
	void gondo_interrupt(device_t &device);
	void oscar_interrupt(device_t &device);
	void srdarwin_draw_sprites(  bitmap_ind16 &bitmap, const rectangle &cliprect, int pri );
	void csilver_adpcm_int(int state);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
