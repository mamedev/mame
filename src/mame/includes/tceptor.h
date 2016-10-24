// license:BSD-3-Clause
// copyright-holders:BUT
#include "sound/namco.h"
#include "video/c45.h"

class tceptor_state : public driver_device
{
public:
	tceptor_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cus30(*this, "namco"),
		m_tile_ram(*this, "tile_ram"),
		m_tile_attr(*this, "tile_attr"),
		m_bg_ram(*this, "bg_ram"),
		m_m68k_shared_ram(*this, "m68k_shared_ram"),
		m_sprite_ram(*this, "sprite_ram"),
		m_c45_road(*this, "c45_road"),
		m_2dscreen(*this, "2dscreen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	uint8_t m_m6809_irq_enable;
	uint8_t m_m68k_irq_enable;
	uint8_t m_mcu_irq_enable;
	required_device<cpu_device> m_maincpu;
	required_device<namco_cus30_device> m_cus30;
	required_shared_ptr<uint8_t> m_tile_ram;
	required_shared_ptr<uint8_t> m_tile_attr;
	required_shared_ptr<uint8_t> m_bg_ram;
	required_shared_ptr<uint8_t> m_m68k_shared_ram;
	required_shared_ptr<uint16_t> m_sprite_ram;
	int m_sprite16;
	int m_sprite32;
	int m_bg;
	tilemap_t *m_tx_tilemap;
	tilemap_t *m_bg1_tilemap;
	tilemap_t *m_bg2_tilemap;
	int32_t m_bg1_scroll_x;
	int32_t m_bg1_scroll_y;
	int32_t m_bg2_scroll_x;
	int32_t m_bg2_scroll_y;
	bitmap_ind16 m_temp_bitmap;
	std::unique_ptr<uint16_t[]> m_sprite_ram_buffered;
	std::unique_ptr<uint8_t[]> m_decoded_16;
	std::unique_ptr<uint8_t[]> m_decoded_32;
	int m_is_mask_spr[1024/16];
	uint8_t m68k_shared_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void m68k_shared_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void m6809_irq_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void m6809_irq_disable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void m68k_irq_enable_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void mcu_irq_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcu_irq_disable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t dsw0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dsw1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t input0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t input1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t readFF(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void tceptor_tile_ram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tceptor_tile_attr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tceptor_bg_ram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tceptor_bg_scroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tile_mark_dirty(int offset);

	required_device<namco_c45_road_device> m_c45_road;
	required_device<screen_device> m_2dscreen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	void get_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg2_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_tceptor(palette_device &palette);
	uint32_t screen_update_tceptor_2d(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_tceptor_3d_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_tceptor_3d_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_tceptor(screen_device &screen, bool state);
	void m6809_vb_interrupt(device_t &device);
	void m68k_vb_interrupt(device_t &device);
	void mcu_vb_interrupt(device_t &device);
	inline int get_tile_addr(int tile_index);
	void decode_bg(const char * region);
	void decode_sprite(int gfx_index, const gfx_layout *layout, const void *data);
	void decode_sprite16(const char * region);
	void decode_sprite32(const char * region);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int sprite_priority);
	inline uint8_t fix_input0(uint8_t in1, uint8_t in2);
	inline uint8_t fix_input1(uint8_t in1, uint8_t in2);
};
