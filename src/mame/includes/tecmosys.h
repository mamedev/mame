// license:BSD-3-Clause
// copyright-holders:Farfetch'd,David Haywood,Tomasz Slanina
/***************************************************************************

    tecmosys protection simulation

***************************************************************************/

#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"

class tecmosys_state : public driver_device
{
public:
	tecmosys_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_eeprom(*this, "eeprom"),
		m_watchdog(*this, "watchdog"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_soundlatch2(*this, "soundlatch2"),
		m_spriteram(*this, "spriteram"),
		m_tilemap_paletteram16(*this, "tmap_palette"),
		m_bg2tilemap_ram(*this, "bg2tilemap_ram"),
		m_bg1tilemap_ram(*this, "bg1tilemap_ram"),
		m_bg0tilemap_ram(*this, "bg0tilemap_ram"),
		m_fgtilemap_ram(*this, "fgtilemap_ram"),
		m_bg0tilemap_lineram(*this, "bg0_lineram"),
		m_bg1tilemap_lineram(*this, "bg1_lineram"),
		m_bg2tilemap_lineram(*this, "bg2_lineram"),
		m_a80000regs(*this, "a80000regs"),
		m_b00000regs(*this, "b00000regs"),
		m_c00000regs(*this, "c00000regs"),
		m_c80000regs(*this, "c80000regs"),
		m_880000regs(*this, "880000regs") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch2;

	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_tilemap_paletteram16;
	required_shared_ptr<uint16_t> m_bg2tilemap_ram;
	required_shared_ptr<uint16_t> m_bg1tilemap_ram;
	required_shared_ptr<uint16_t> m_bg0tilemap_ram;
	required_shared_ptr<uint16_t> m_fgtilemap_ram;
	required_shared_ptr<uint16_t> m_bg0tilemap_lineram;
	required_shared_ptr<uint16_t> m_bg1tilemap_lineram;
	required_shared_ptr<uint16_t> m_bg2tilemap_lineram;
	required_shared_ptr<uint16_t> m_a80000regs;
	required_shared_ptr<uint16_t> m_b00000regs;
	required_shared_ptr<uint16_t> m_c00000regs;
	required_shared_ptr<uint16_t> m_c80000regs;
	required_shared_ptr<uint16_t> m_880000regs;

	int m_spritelist;
	bitmap_ind16 m_sprite_bitmap;
	bitmap_ind16 m_tmp_tilemap_composebitmap;
	bitmap_ind16 m_tmp_tilemap_renderbitmap;
	tilemap_t *m_bg0tilemap;
	tilemap_t *m_bg1tilemap;
	tilemap_t *m_bg2tilemap;
	tilemap_t *m_txt_tilemap;
	uint8_t m_device_read_ptr;
	uint8_t m_device_status;
	const struct prot_data* m_device_data;
	uint8_t m_device_value;

	uint16_t sound_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void sound_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void unk880000_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t unk880000_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void z80_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void oki_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t prot_status_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void prot_status_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t prot_data_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void prot_data_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bg0_tilemap_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bg1_tilemap_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bg2_tilemap_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void fg_tilemap_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tilemap_paletteram16_xGGGGGRRRRRBBBBB_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bg0_tilemap_lineram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bg1_tilemap_lineram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bg2_tilemap_lineram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t eeprom_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void eeprom_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void init_tkdensha();
	void init_deroon();
	void init_tkdensho();
	virtual void machine_start() override;
	virtual void video_start() override;

	void get_bg0tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg1tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg2tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void prot_init(int which);
	void prot_reset();
	inline void set_color_555(pen_t color, int rshift, int gshift, int bshift, uint16_t data);
	void render_sprites_to_bitmap(bitmap_rgb32 &bitmap, uint16_t extrax, uint16_t extray );
	void tilemap_copy_to_compose(uint16_t pri);
	void do_final_mix(bitmap_rgb32 &bitmap);
	void descramble();
};
