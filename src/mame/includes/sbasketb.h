// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari

#include "sound/sn76496.h"
#include "sound/vlm5030.h"

class sbasketb_state : public driver_device
{
public:
	sbasketb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_palettebank(*this, "palettebank"),
		m_spriteram_select(*this, "spriteramsel"),
		m_scroll(*this, "scroll"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_sn(*this, "snsnd"),
		m_vlm(*this, "vlm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_palettebank;
	required_shared_ptr<uint8_t> m_spriteram_select;
	required_shared_ptr<uint8_t> m_scroll;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<sn76489_device> m_sn;
	required_device<vlm5030_device> m_vlm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* video-related */
	tilemap_t  *m_bg_tilemap;

	uint8_t    m_irq_mask;
	void sbasketb_sh_irqtrigger_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sbasketb_coin_counter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void irq_mask_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sbasketb_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sbasketb_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sbasketb_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_sbasketb();

	uint8_t m_SN76496_latch;
	void konami_SN76496_latch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) { m_SN76496_latch = data; };
	void konami_SN76496_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) { m_sn->write(space, offset, m_SN76496_latch); };
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void video_start() override;
	void palette_init_sbasketb(palette_device &palette);
	uint32_t screen_update_sbasketb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(device_t &device);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
};
