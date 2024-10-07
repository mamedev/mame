// license:BSD-3-Clause
// copyright-holders:Ville Linde, hap, Nicola Salmoria
/******************************************************************************

    Seibu SPI hardware

******************************************************************************/

#include "machine/eepromser.h"
#include "machine/7200fifo.h"
#include "machine/intelfsh.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "tilemap.h"

class seibuspi_state : public driver_device
{
public:
	seibuspi_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_mainram(*this, "mainram")
		, m_z80_rom(*this, "audiocpu")
		, m_eeprom(*this, "eeprom")
		, m_soundfifo(*this, "soundfifo%u", 1)
		, m_oki(*this, "oki%u", 1)
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_key(*this, "KEY.%u", 0)
		, m_special(*this, "SPECIAL")
		, m_z80_bank(*this, "z80_bank")
		, m_soundflash1(*this, "soundflash1")
		, m_soundflash2(*this, "soundflash2")
		, m_soundflash1_region(*this, "soundflash1")
	{ }

	void sys386f(machine_config &config);
	void sxx2f(machine_config &config);
	void rdft2(machine_config &config);
	void ejanhs(machine_config &config);
	void sys386i(machine_config &config);
	void sxx2g(machine_config &config);
	void spi(machine_config &config);
	void sxx2e(machine_config &config);

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

	template <int N> ioport_value ejanhs_encode();

	IRQ_CALLBACK_MEMBER(spi_irq_callback);
	INTERRUPT_GEN_MEMBER(spi_interrupt);

	u32 screen_update_sys386f(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_shared_ptr<u32> m_mainram;
	optional_memory_region m_z80_rom;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_device_array<fifo7200_device, 2> m_soundfifo;
	optional_device_array<okim6295_device, 2> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	optional_ioport_array<5> m_key;
	optional_ioport m_special;

	optional_memory_bank m_z80_bank;

	optional_device<intel_e28f008sa_device> m_soundflash1, m_soundflash2;

	optional_region_ptr<u8> m_soundflash1_region;

	int m_z80_prg_transfer_pos = 0;
	int m_z80_lastbank = 0;
	u8 m_sb_coin_latch = 0;
	u8 m_ejsakura_input_port = 0;
	tilemap_t *m_text_layer = nullptr;
	tilemap_t *m_back_layer = nullptr;
	tilemap_t *m_midl_layer = nullptr;
	tilemap_t *m_fore_layer = nullptr;
	u32 m_video_dma_length = 0;
	u32 m_video_dma_address = 0;
	u16 m_layer_enable = 0;
	u16 m_layer_bank = 0;
	u8 m_rf2_layer_bank = 0;
	u16 m_scrollram[6]{};
	bool m_rowscroll_enable = false;
	int m_midl_layer_offset = 0;
	int m_fore_layer_offset = 0;
	int m_text_layer_offset = 0;
	int m_fore_layer_d13 = 0;
	int m_back_layer_d14 = 0;
	int m_midl_layer_d14 = 0;
	int m_fore_layer_d14 = 0;
	std::unique_ptr<u32[]> m_tilemap_ram;
	std::unique_ptr<u32[]> m_palette_ram;
	std::unique_ptr<u32[]> m_sprite_ram;
	u32 m_tilemap_ram_size = 0;
	u32 m_palette_ram_size = 0;
	u32 m_sprite_ram_size = 0;
	u32 m_bg_fore_layer_position = 0;
	u8 m_alpha_table[0x2000]{};
	int m_sprite_bpp = 0;

	void tile_decrypt_key_w(u16 data);
	void spi_layer_bank_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void spi_layer_enable_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void rf2_layer_bank_w(u8 data);
	void scroll_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void tilemap_dma_start_w(u32 data);
	void palette_dma_start_w(u32 data);
	void sprite_dma_start_w(u16 data);
	void video_dma_length_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void video_dma_address_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u8 spi_status_r();
	u8 spi_ds2404_unknown_r();
	u8 sb_coin_r();
	void spi_coin_w(u8 data);
	u8 sound_fifo_status_r();
	void z80_prg_transfer_w(u8 data);
	void z80_enable_w(u8 data);
	u8 z80_soundfifo_status_r();
	void z80_bank_w(u8 data);
	u32 ejsakura_keyboard_r();
	void ejsakura_input_select_w(u32 data);
	void eeprom_w(u8 data);
	void spi_layerbanks_eeprom_w(u8 data);
	void oki_bank_w(u8 data);

	u32 senkyu_speedup_r();
	u32 senkyua_speedup_r();
	u32 batlball_speedup_r();
	u32 rdft_speedup_r();
	u32 viprp1_speedup_r();
	u32 viprp1o_speedup_r();
	u32 ejanhs_speedup_r();
	u32 rf2_speedup_r();
	u32 rfjet_speedup_r();

	void ymf_irqhandler(int state);

	void set_layer_offsets();
	void drawgfx_blend(bitmap_rgb32 &bitmap, const rectangle &cliprect, gfx_element *gfx, u32 code, u32 color, bool flipx, bool flipy, int sx, int sy, bitmap_ind8 &primap, u8 primask);
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, bitmap_ind8 &primap, int priority);
	void combine_tilemap(bitmap_rgb32 &bitmap, const rectangle &cliprect, tilemap_t *tile, int sx, int sy, int opaque, s16 *rowscroll);

	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	DECLARE_MACHINE_RESET(spi);
	DECLARE_MACHINE_RESET(sxx2e);
	DECLARE_VIDEO_START(ejanhs);
	DECLARE_VIDEO_START(sys386f);
	TILE_GET_INFO_MEMBER(get_text_tile_info);
	TILE_GET_INFO_MEMBER(get_back_tile_info);
	TILE_GET_INFO_MEMBER(get_midl_tile_info);
	TILE_GET_INFO_MEMBER(get_fore_tile_info);
	u32 screen_update_spi(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void register_video_state();
	void init_spi_common();

	void text_decrypt(u8 *rom);
	void bg_decrypt(u8 *rom, int size);

	void rdft2_text_decrypt(u8 *rom);
	void rdft2_bg_decrypt(u8 *rom, int size);

	void rfjet_text_decrypt(u8 *rom);
	void rfjet_bg_decrypt(u8 *rom, int size);

	void base_map(address_map &map) ATTR_COLD;
	void rdft2_map(address_map &map) ATTR_COLD;
	void rise_map(address_map &map) ATTR_COLD;
	void sei252_map(address_map &map) ATTR_COLD;
	void spi_map(address_map &map) ATTR_COLD;
	void spi_soundmap(address_map &map) ATTR_COLD;
	void spi_ymf271_map(address_map &map) ATTR_COLD;
	void sxx2e_map(address_map &map) ATTR_COLD;
	void sxx2e_soundmap(address_map &map) ATTR_COLD;
	void sxx2f_map(address_map &map) ATTR_COLD;
	void sys386f_map(address_map &map) ATTR_COLD;
	void sys386i_map(address_map &map) ATTR_COLD;
};
