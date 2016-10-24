// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#include "sound/flt_rc.h"
#include "sound/k007232.h"
#include "sound/k005289.h"
#include "sound/vlm5030.h"

class nemesis_state : public driver_device
{
public:
	nemesis_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_charram(*this, "charram"),
		m_xscroll1(*this, "xscroll1"),
		m_xscroll2(*this, "xscroll2"),
		m_yscroll2(*this, "yscroll2"),
		m_yscroll1(*this, "yscroll1"),
		m_videoram1(*this, "videoram1"),
		m_videoram2(*this, "videoram2"),
		m_colorram1(*this, "colorram1"),
		m_colorram2(*this, "colorram2"),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_gx400_shared_ram(*this, "gx400_shared"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_filter1(*this, "filter1"),
		m_filter2(*this, "filter2"),
		m_filter3(*this, "filter3"),
		m_filter4(*this, "filter4"),
		m_k007232(*this, "k007232"),
		m_k005289(*this, "k005289"),
		m_vlm(*this, "vlm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint16_t> m_charram;
	required_shared_ptr<uint16_t> m_xscroll1;
	required_shared_ptr<uint16_t> m_xscroll2;
	required_shared_ptr<uint16_t> m_yscroll2;
	required_shared_ptr<uint16_t> m_yscroll1;
	required_shared_ptr<uint16_t> m_videoram1;
	required_shared_ptr<uint16_t> m_videoram2;
	required_shared_ptr<uint16_t> m_colorram1;
	required_shared_ptr<uint16_t> m_colorram2;
	required_shared_ptr<uint16_t> m_spriteram;
	optional_shared_ptr<uint16_t> m_paletteram;
	optional_shared_ptr<uint8_t> m_gx400_shared_ram;

	/* video-related */
	tilemap_t *m_background;
	tilemap_t *m_foreground;
	int       m_spriteram_words;
	int       m_tilemap_flip;
	int       m_flipscreen;
	uint8_t     m_irq_port_last;
	uint8_t     m_blank_tile[8*8];
	uint8_t     m_palette_lookup[32];

	/* misc */
	int       m_irq_on;
	int       m_irq1_on;
	int       m_irq2_on;
	int       m_irq4_on;
	uint16_t    m_selected_ip; /* Copied from WEC Le Mans 24 driver, explicity needed for Hyper Crash */
	int       m_gx400_irq1_cnt;
	uint8_t     m_frame_counter;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<filter_rc_device> m_filter1;
	optional_device<filter_rc_device> m_filter2;
	optional_device<filter_rc_device> m_filter3;
	optional_device<filter_rc_device> m_filter4;
	optional_device<k007232_device> m_k007232;
	optional_device<k005289_device> m_k005289;
	optional_device<vlm5030_device> m_vlm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	void gx400_irq1_enable_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gx400_irq2_enable_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gx400_irq4_enable_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void nemesis_irq_enable_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void konamigt_irq_enable_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void konamigt_irq2_enable_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t gx400_sharedram_word_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void gx400_sharedram_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t konamigt_input_word_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void selected_ip_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t selected_ip_word_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint8_t wd_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void nemesis_gfx_flipx_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void nemesis_gfx_flipy_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void salamand_control_port_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void nemesis_palette_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void nemesis_videoram1_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void nemesis_videoram2_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void nemesis_colorram1_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void nemesis_colorram2_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void nemesis_charram_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void nemesis_filter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gx400_speech_start_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void salamand_speech_start_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t nemesis_portA_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void city_sound_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_nemesis(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void nemesis_interrupt(device_t &device);
	void blkpnthr_interrupt(device_t &device);
	void konamigt_interrupt(timer_device &timer, void *ptr, int32_t param);
	void gx400_interrupt(timer_device &timer, void *ptr, int32_t param);
	void create_palette_lookups();
	void nemesis_postload();
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void volume_callback(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
};
