// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

  Fast Freddie/Jump Coaster hardware
  driver by Zsolt Vasvari

***************************************************************************/

#include "includes/galaxold.h"

class fastfred_state : public galaxold_state
{
public:
	fastfred_state(const machine_config &mconfig, device_type type, const char *tag)
		: galaxold_state(mconfig, type, tag),
			m_gfxdecode(*this, "gfxdecode"),
			m_palette(*this, "palette"),
			m_videoram(*this, "videoram"),
			m_spriteram(*this, "spriteram"),
			m_attributesram(*this, "attributesram"),
			m_background_color(*this, "bgcolor"),
			m_imago_fg_videoram(*this, "imago_fg_vram") { }

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_attributesram;
	optional_shared_ptr<uint8_t> m_background_color;
	optional_shared_ptr<uint8_t> m_imago_fg_videoram;

	int m_hardware_type;
	uint16_t m_charbank;
	uint8_t m_colorbank;
	uint8_t m_nmi_mask;
	uint8_t m_sound_nmi_mask;
	uint8_t m_imago_sprites[0x800*3];
	uint16_t m_imago_sprites_address;
	uint8_t m_imago_sprites_bank;

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_web_tilemap;

	uint8_t fastfred_custom_io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t flyboy_custom1_io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t flyboy_custom2_io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t jumpcoas_custom_io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t boggy84_custom_io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void imago_dma_irq_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void imago_sprites_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void imago_sprites_dma_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t imago_sprites_offset_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void nmi_mask_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_nmi_mask_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fastfred_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fastfred_attributes_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fastfred_charbank1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fastfred_charbank2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fastfred_colorbank1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fastfred_colorbank2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fastfred_flip_screen_x_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fastfred_flip_screen_y_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void imago_fg_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void imago_charbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void init_fastfred();
	void init_flyboy();
	void init_flyboyb();
	void init_imago();
	void init_boggy84();
	void init_jumpcoas();
	void init_boggy84b();

	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void imago_get_tile_info_bg(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void imago_get_tile_info_fg(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void imago_get_tile_info_web(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	void vblank_irq(device_t &device);
	void sound_timer_irq(device_t &device);

	virtual void machine_start() override;
	void palette_init_fastfred(palette_device &palette);
	void machine_start_imago();
	void video_start_fastfred();
	void video_start_imago();

	uint32_t screen_update_fastfred(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_imago(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
};
