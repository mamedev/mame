// license:BSD-3-Clause
// copyright-holders:Carlos A. Lozano, Rob Rosenbrock, Phil Stroffolino

#include "machine/gen_latch.h"

class xain_state : public driver_device
{
public:
	xain_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_mcu(*this, "mcu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_charram(*this, "charram"),
		m_bgram0(*this, "bgram0"),
		m_bgram1(*this, "bgram1"),
		m_spriteram(*this, "spriteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	optional_device<cpu_device> m_mcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_charram;
	required_shared_ptr<uint8_t> m_bgram0;
	required_shared_ptr<uint8_t> m_bgram1;
	required_shared_ptr<uint8_t> m_spriteram;

	int m_vblank;
	int m_from_main;
	int m_from_mcu;
	uint8_t m_ddr_a;
	uint8_t m_ddr_b;
	uint8_t m_ddr_c;
	uint8_t m_port_a_out;
	uint8_t m_port_b_out;
	uint8_t m_port_c_out;
	uint8_t m_port_a_in;
	uint8_t m_port_b_in;
	uint8_t m_port_c_in;
	int m_mcu_ready;
	int m_mcu_accept;
	uint8_t m_pri;
	tilemap_t *m_char_tilemap;
	tilemap_t *m_bgram0_tilemap;
	tilemap_t *m_bgram1_tilemap;
	uint8_t m_scrollxP0[2];
	uint8_t m_scrollyP0[2];
	uint8_t m_scrollxP1[2];
	uint8_t m_scrollyP1[2];

	void cpuA_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cpuB_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void main_irq_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void irqA_assert_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void irqB_clear_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t m68705_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void m68705_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t m68705_port_a_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void m68705_port_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void m68705_ddr_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t m68705_port_b_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void m68705_port_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void m68705_ddr_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t m68705_port_c_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void m68705_port_c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void m68705_ddr_c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mcu_comm_reset_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void bgram0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bgram1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void charram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scrollxP0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scrollyP0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scrollxP1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scrollyP1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	ioport_value vblank_r(ioport_field &field, void *param);
	ioport_value mcu_status_r(ioport_field &field, void *param);

	tilemap_memory_index back_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void get_bgram0_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bgram1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_char_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void machine_start() override;
	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	inline int scanline_to_vcount(int scanline);

	void scanline(timer_device &timer, void *ptr, int32_t param);
};
