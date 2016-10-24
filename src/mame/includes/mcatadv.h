// license:BSD-3-Clause
// copyright-holders:Paul Priest, David Haywood

#include "machine/gen_latch.h"
#include "machine/watchdog.h"

class mcatadv_state : public driver_device
{
public:
	mcatadv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram1(*this, "videoram1"),
		m_videoram2(*this, "videoram2"),
		m_scroll1(*this, "scroll1"),
		m_scroll2(*this, "scroll2"),
		m_spriteram(*this, "spriteram"),
		m_vidregs(*this, "vidregs"),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_watchdog(*this, "watchdog"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch") { }

	/* memory pointers */
	required_shared_ptr<uint16_t> m_videoram1;
	required_shared_ptr<uint16_t> m_videoram2;
	required_shared_ptr<uint16_t> m_scroll1;
	required_shared_ptr<uint16_t> m_scroll2;
	required_shared_ptr<uint16_t> m_spriteram;
	std::unique_ptr<uint16_t[]>     m_spriteram_old;
	required_shared_ptr<uint16_t> m_vidregs;
	std::unique_ptr<uint16_t[]>     m_vidregs_old;

	/* video-related */
	tilemap_t    *m_tilemap1;
	tilemap_t    *m_tilemap2;
	int m_palette_bank1;
	int m_palette_bank2;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	void mcat_soundlatch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t mcat_wd_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void mcatadv_sound_bw_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcatadv_videoram1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void mcatadv_videoram2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void get_mcatadv_tile_info1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_mcatadv_tile_info2(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void video_start() override;
	uint32_t screen_update_mcatadv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_mcatadv(screen_device &screen, bool state);
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void mcatadv_draw_tilemap_part( screen_device &screen, uint16_t* current_scroll, uint16_t* current_videoram1, int i, tilemap_t* current_tilemap, bitmap_ind16 &bitmap, const rectangle &cliprect );
};
