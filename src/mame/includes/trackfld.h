// license:BSD-3-Clause
// copyright-holders:Chris Hardy
/***************************************************************************

    Track'n'Field

***************************************************************************/

#include "sound/sn76496.h"
#include "sound/vlm5030.h"

class trackfld_state : public driver_device
{
public:
	trackfld_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram2(*this, "spriteram2"),
		m_scroll(*this, "scroll"),
		m_spriteram(*this, "spriteram"),
		m_scroll2(*this, "scroll2"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_sn(*this, "snsnd"),
		m_vlm(*this, "vlm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_spriteram2;
	required_shared_ptr<uint8_t> m_scroll;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_scroll2;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<sn76496_device> m_sn;
	optional_device<vlm5030_device> m_vlm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	int      m_bg_bank;
	int      m_sprite_bank1;
	int      m_sprite_bank2;
	int      m_old_gfx_bank;                    // needed by atlantol
	int      m_sprites_gfx_banked;

	uint8_t    m_irq_mask;
	uint8_t    m_yieartf_nmi_mask;
	void coin_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void questions_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void irq_mask_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void yieartf_nmi_mask_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void trackfld_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void trackfld_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void trackfld_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void atlantol_gfxbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t trackfld_SN76496_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t trackfld_speech_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void trackfld_VLM5030_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void init_trackfld();
	void init_atlantol();
	void init_wizzquiz();
	void init_mastkin();
	void init_trackfldnz();

	uint8_t m_SN76496_latch;
	void konami_SN76496_latch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) { m_SN76496_latch = data; };
	void konami_SN76496_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) { m_sn->write(space, offset, m_SN76496_latch); };
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_trackfld();
	void machine_reset_trackfld();
	void video_start_trackfld();
	void palette_init_trackfld(palette_device &palette);
	void video_start_atlantol();
	uint32_t screen_update_trackfld(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(device_t &device);
	void vblank_nmi(device_t &device);
	void yieartf_timer_irq(device_t &device);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
};
