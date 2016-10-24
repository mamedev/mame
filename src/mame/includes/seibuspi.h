// license:BSD-3-Clause
// copyright-holders:Ville Linde, hap, Nicola Salmoria
/******************************************************************************

    Seibu SPI hardware

******************************************************************************/

#include "machine/intelfsh.h"
#include "machine/eepromser.h"
#include "machine/7200fifo.h"
#include "sound/okim6295.h"

class seibuspi_state : public driver_device
{
public:
	seibuspi_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_mainram(*this, "mainram"),
		m_z80_rom(*this, "audiocpu"),
		m_eeprom(*this, "eeprom"),
		m_soundflash1(*this, "soundflash1"),
		m_soundflash2(*this, "soundflash2"),
		m_soundfifo1(*this, "soundfifo1"),
		m_soundfifo2(*this, "soundfifo2"),
		m_oki1(*this, "oki1"),
		m_oki2(*this, "oki2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_key(*this, "KEY.%u", 0),
		m_special(*this, "SPECIAL")
	{ }

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_shared_ptr<uint32_t> m_mainram;
	optional_memory_region m_z80_rom;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_device<intel_e28f008sa_device> m_soundflash1;
	optional_device<intel_e28f008sa_device> m_soundflash2;
	optional_device<fifo7200_device> m_soundfifo1;
	optional_device<fifo7200_device> m_soundfifo2;
	optional_device<okim6295_device> m_oki1;
	optional_device<okim6295_device> m_oki2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	optional_ioport_array<5> m_key;
	optional_ioport m_special;

	int m_z80_prg_transfer_pos;
	int m_z80_lastbank;
	uint8_t m_sb_coin_latch;
	uint8_t m_ejsakura_input_port;
	tilemap_t *m_text_layer;
	tilemap_t *m_back_layer;
	tilemap_t *m_midl_layer;
	tilemap_t *m_fore_layer;
	uint32_t m_video_dma_length;
	uint32_t m_video_dma_address;
	uint16_t m_layer_enable;
	uint16_t m_layer_bank;
	uint8_t m_rf2_layer_bank;
	uint16_t m_scrollram[6];
	int m_rowscroll_enable;
	int m_midl_layer_offset;
	int m_fore_layer_offset;
	int m_text_layer_offset;
	int m_fore_layer_d13;
	int m_back_layer_d14;
	int m_midl_layer_d14;
	int m_fore_layer_d14;
	std::unique_ptr<uint32_t[]> m_tilemap_ram;
	std::unique_ptr<uint32_t[]> m_palette_ram;
	std::unique_ptr<uint32_t[]> m_sprite_ram;
	uint32_t m_tilemap_ram_size;
	uint32_t m_palette_ram_size;
	uint32_t m_sprite_ram_size;
	uint32_t m_bg_fore_layer_position;
	uint8_t m_alpha_table[0x2000];
	int m_sprite_bpp;

	void tile_decrypt_key_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void spi_layer_bank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void spi_layer_enable_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void rf2_layer_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tilemap_dma_start_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void palette_dma_start_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void sprite_dma_start_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void video_dma_length_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void video_dma_address_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint8_t spi_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t spi_ds2404_unknown_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t sb_coin_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void spi_coin_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t sound_fifo_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void z80_prg_transfer_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void z80_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t z80_soundfifo_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void z80_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint32_t ejsakura_keyboard_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void ejsakura_input_select_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void eeprom_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void spi_layerbanks_eeprom_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void oki_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t flashrom_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void flashrom_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint32_t senkyu_speedup_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t senkyua_speedup_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t batlball_speedup_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t rdft_speedup_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t viprp1_speedup_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t viprp1o_speedup_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t rf2_speedup_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t rfjet_speedup_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);

	ioport_value ejanhs_encode(ioport_field &field, void *param);

	void ymf_irqhandler(int state);
	int spi_irq_callback(device_t &device, int irqline);
	void spi_interrupt(device_t &device);

	void set_layer_offsets();
	void drawgfx_blend(bitmap_rgb32 &bitmap, const rectangle &cliprect, gfx_element *gfx, uint32_t code, uint32_t color, int flipx, int flipy, int sx, int sy, bitmap_ind8 &primap, int primask);
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, bitmap_ind8 &primap, int priority);
	void combine_tilemap(bitmap_rgb32 &bitmap, const rectangle &cliprect, tilemap_t *tile, int sx, int sy, int opaque, int16_t *rowscroll);

	virtual void machine_start() override;
	virtual void video_start() override;
	void machine_reset_spi();
	void machine_reset_sxx2e();
	void video_start_ejanhs();
	void video_start_sys386f();
	void get_text_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_back_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_midl_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fore_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	uint32_t screen_update_spi(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_sys386f(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void register_video_state();
	void init_spi_common();
	void init_sei252();
	void init_batlball();
	void init_senkyu();
	void init_viprp1();
	void init_viprp1o();
	void init_rdft();
	void init_rfjet();
	void init_senkyua();
	void init_rdft2();
	void init_ejanhs();
	void init_sys386f();

	void text_decrypt(uint8_t *rom);
	void bg_decrypt(uint8_t *rom, int size);

	void rdft2_text_decrypt(uint8_t *rom);
	void rdft2_bg_decrypt(uint8_t *rom, int size);

	void rfjet_text_decrypt(uint8_t *rom);
	void rfjet_bg_decrypt(uint8_t *rom, int size);
};
