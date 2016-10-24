// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
#include "cpu/m68000/m68000.h"
#include "video/bufsprite.h"
#include "machine/eepromser.h"

class gaelco2_state : public driver_device
{
public:
	gaelco2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_spriteram(*this,"spriteram"),
		m_vregs(*this, "vregs"),
		m_snowboar_protection(*this, "snowboar_prot"),
		m_eeprom(*this, "eeprom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_generic_paletteram_16(*this, "paletteram"),
		m_shareram(*this, "shareram")
	{ }

	required_device<m68000_device> m_maincpu;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_shared_ptr<uint16_t> m_vregs;
	optional_shared_ptr<uint16_t> m_snowboar_protection;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint16_t> m_generic_paletteram_16;
	optional_shared_ptr<uint16_t> m_shareram;



	uint32_t snowboard_latch;


	uint16_t *m_videoram;
	tilemap_t *m_pant[2];
	int m_dual_monitor;

	uint16_t dallas_kludge_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t maniacsqa_prot_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

	void gaelco2_coin_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gaelco2_coin2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void touchgo_coin_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t snowboar_protection_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void snowboar_protection_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gaelco2_vram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gaelco2_palette_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void init_touchgo();
	void init_touchgop();
	void init_snowboar();
	void init_alighunt();
	void init_maniacsqa();
	void get_tile_info_gaelco2_screen0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_gaelco2_screen1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_gaelco2_screen0_dual(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_gaelco2_screen1_dual(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void video_start_gaelco2();
	void video_start_gaelco2_dual();
	uint32_t screen_update_gaelco2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_gaelco2_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_gaelco2_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void gaelco2_eeprom_cs_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gaelco2_eeprom_sk_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gaelco2_eeprom_data_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int mask, int xoffs);
	uint32_t dual_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int index);
	void gaelco2_ROM16_split_gfx(const char *src_reg, const char *dst_reg, int start, int length, int dest1, int dest2);
};


class bang_state : public gaelco2_state
{
public:
	bang_state(const machine_config &mconfig, device_type type, const char *tag)
		: gaelco2_state(mconfig, type, tag)
		, m_light0_x(*this, "LIGHT0_X")
		, m_light0_y(*this, "LIGHT0_Y")
		, m_light1_x(*this, "LIGHT1_X")
		, m_light1_y(*this, "LIGHT1_Y")
	{}

	required_ioport m_light0_x;
	required_ioport m_light0_y;
	required_ioport m_light1_x;
	required_ioport m_light1_y;

	int m_clr_gun_int;

	uint16_t p1_gun_x(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t p1_gun_y(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t p2_gun_x(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t p2_gun_y(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void bang_clr_gun_int_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bang_irq(timer_device &timer, void *ptr, int32_t param);
	void init_bang();
};


class wrally2_state : public gaelco2_state
{
public:
	wrally2_state(const machine_config &mconfig, device_type type, const char *tag)
		: gaelco2_state(mconfig, type, tag)
		, m_analog0(*this, "ANALOG0")
		, m_analog1(*this, "ANALOG1")
	{}

	required_ioport m_analog0;
	required_ioport m_analog1;

	uint8_t m_analog_ports[2];

	void wrally2_coin_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void wrally2_adc_clk(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void wrally2_adc_cs(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	ioport_value wrally2_analog_bit_r(ioport_field &field, void *param);
};
