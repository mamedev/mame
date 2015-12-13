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
		m_scrollram(*this, "scrollram"),
		m_z80_rom(*this, "audiocpu"),
		m_eeprom(*this, "eeprom"),
		m_soundflash1(*this, "soundflash1"),
		m_soundflash2(*this, "soundflash2"),
		m_soundfifo1(*this, "soundfifo1"),
		m_soundfifo2(*this, "soundfifo2"),
		m_oki1(*this, "oki1"),
		m_oki2(*this, "oki2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_shared_ptr<UINT32> m_mainram;
	optional_shared_ptr<UINT32> m_scrollram;
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

	int m_z80_prg_transfer_pos;
	int m_z80_lastbank;
	UINT8 m_sb_coin_latch;
	UINT8 m_ejsakura_input_port;
	tilemap_t *m_text_layer;
	tilemap_t *m_back_layer;
	tilemap_t *m_midl_layer;
	tilemap_t *m_fore_layer;
	UINT32 m_video_dma_length;
	UINT32 m_video_dma_address;
	UINT32 m_layer_enable;
	UINT32 m_layer_bank;
	UINT8 m_rf2_layer_bank;
	int m_rowscroll_enable;
	int m_midl_layer_offset;
	int m_fore_layer_offset;
	int m_text_layer_offset;
	int m_fore_layer_d13;
	int m_back_layer_d14;
	int m_midl_layer_d14;
	int m_fore_layer_d14;
	UINT32 *m_tilemap_ram;
	UINT32 *m_palette_ram;
	UINT32 *m_sprite_ram;
	UINT32 m_tilemap_ram_size;
	UINT32 m_palette_ram_size;
	UINT32 m_sprite_ram_size;
	UINT32 m_bg_fore_layer_position;
	UINT8 m_alpha_table[0x2000];
	int m_sprite_bpp;

	DECLARE_READ32_MEMBER(spi_layer_bank_r);
	DECLARE_WRITE32_MEMBER(spi_layer_bank_w);
	DECLARE_READ32_MEMBER(spi_layer_enable_r);
	DECLARE_WRITE32_MEMBER(spi_layer_enable_w);
	DECLARE_WRITE8_MEMBER(rf2_layer_bank_w);
	DECLARE_WRITE32_MEMBER(tilemap_dma_start_w);
	DECLARE_WRITE32_MEMBER(palette_dma_start_w);
	DECLARE_WRITE16_MEMBER(sprite_dma_start_w);
	DECLARE_WRITE32_MEMBER(video_dma_length_w);
	DECLARE_WRITE32_MEMBER(video_dma_address_w);
	DECLARE_READ8_MEMBER(spi_status_r);
	DECLARE_READ8_MEMBER(spi_ds2404_unknown_r);
	DECLARE_READ8_MEMBER(sb_coin_r);
	DECLARE_WRITE8_MEMBER(spi_coin_w);
	DECLARE_READ8_MEMBER(sound_fifo_status_r);
	DECLARE_WRITE8_MEMBER(z80_prg_transfer_w);
	DECLARE_WRITE8_MEMBER(z80_enable_w);
	DECLARE_READ8_MEMBER(z80_soundfifo_status_r);
	DECLARE_WRITE8_MEMBER(z80_bank_w);
	DECLARE_READ32_MEMBER(ejsakura_keyboard_r);
	DECLARE_WRITE32_MEMBER(ejsakura_input_select_w);
	DECLARE_WRITE8_MEMBER(eeprom_w);
	DECLARE_WRITE8_MEMBER(spi_layerbanks_eeprom_w);
	DECLARE_WRITE8_MEMBER(oki_bank_w);
	DECLARE_READ8_MEMBER(flashrom_read);
	DECLARE_WRITE8_MEMBER(flashrom_write);

	DECLARE_READ32_MEMBER(ejanhs_speedup_r);
	DECLARE_READ32_MEMBER(senkyu_speedup_r);
	DECLARE_READ32_MEMBER(senkyua_speedup_r);
	DECLARE_READ32_MEMBER(batlball_speedup_r);
	DECLARE_READ32_MEMBER(rdft_speedup_r);
	DECLARE_READ32_MEMBER(viprp1_speedup_r);
	DECLARE_READ32_MEMBER(viprp1o_speedup_r);
	DECLARE_READ32_MEMBER(rf2_speedup_r);
	DECLARE_READ32_MEMBER(rfjet_speedup_r);

	DECLARE_CUSTOM_INPUT_MEMBER(ejanhs_encode);

	DECLARE_WRITE_LINE_MEMBER(ymf_irqhandler);
	IRQ_CALLBACK_MEMBER(spi_irq_callback);
	INTERRUPT_GEN_MEMBER(spi_interrupt);

	void set_layer_offsets();
	void drawgfx_blend(bitmap_rgb32 &bitmap, const rectangle &cliprect, gfx_element *gfx, UINT32 code, UINT32 color, int flipx, int flipy, int sx, int sy, bitmap_ind8 &primap, int primask);
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, bitmap_ind8 &primap, int priority);
	void set_rowscroll(tilemap_t *layer, int scroll, INT16* rows);
	void set_scroll(tilemap_t *layer, int scroll);
	void combine_tilemap(bitmap_rgb32 &bitmap, const rectangle &cliprect, tilemap_t *tile, int sx, int sy, int opaque, INT16 *rowscroll);

	virtual void machine_start() override;
	virtual void video_start() override;
	DECLARE_MACHINE_RESET(spi);
	DECLARE_MACHINE_RESET(sxx2e);
	DECLARE_VIDEO_START(ejanhs);
	DECLARE_VIDEO_START(sys386f);
	TILE_GET_INFO_MEMBER(get_text_tile_info);
	TILE_GET_INFO_MEMBER(get_back_tile_info);
	TILE_GET_INFO_MEMBER(get_midl_tile_info);
	TILE_GET_INFO_MEMBER(get_fore_tile_info);
	UINT32 screen_update_spi(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_sys386f(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void register_video_state();
	void init_spi_common();
	void init_sei252();
	void init_rise10();
	void init_rise11();
	DECLARE_DRIVER_INIT(batlball);
	DECLARE_DRIVER_INIT(senkyu);
	DECLARE_DRIVER_INIT(viprp1);
	DECLARE_DRIVER_INIT(viprp1o);
	DECLARE_DRIVER_INIT(rdft);
	DECLARE_DRIVER_INIT(rfjet);
	DECLARE_DRIVER_INIT(senkyua);
	DECLARE_DRIVER_INIT(rdft2);
	DECLARE_DRIVER_INIT(ejanhs);
	DECLARE_DRIVER_INIT(sys386f);
};

/*----------- defined in machine/spisprit.c -----------*/
void seibuspi_sprite_decrypt(UINT8 *src, int romsize);
