// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "machine/namcoio.h"
#include "sound/dac.h"
#include "sound/namco.h"

class mappy_state : public driver_device
{
public:
	enum
	{
		TIMER_SUPERPAC_IO_RUN,
		TIMER_PACNPAL_IO_RUN,
		TIMER_GROBDA_IO_RUN,
		TIMER_PHOZON_IO_RUN,
		TIMER_MAPPY_IO_RUN,
		TIMER_DIGDUG2_IO_RUN,
		TIMER_MOTOS_IO_RUN
	};

	enum
	{
		GAME_SUPERPAC = 0,
		GAME_PACNPAL,
		GAME_GROBDA,
		GAME_PHOZON,
		GAME_MAPPY,
		GAME_DRUAGA,
		GAME_DIGDUG2,
		GAME_MOTOS
	};

	mappy_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_subcpu2(*this, "sub2"),
		m_namco_15xx(*this, "namco"),
		m_dac(*this, "dac"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")  { }

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	optional_device<cpu_device> m_subcpu2;
	required_device<namco_15xx_device> m_namco_15xx;
	optional_device<dac_byte_interface> m_dac;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	namco56xx_device *m_namco56xx_1;
	namco56xx_device *m_namco56xx_2;
	namco58xx_device *m_namco58xx_1;
	namco58xx_device *m_namco58xx_2;
	namco59xx_device *m_namco59xx;

	// per-game variable to distinguish between the various IO chip config
	int m_type;

	tilemap_t *m_bg_tilemap;
	bitmap_ind16 m_sprite_bitmap;

	uint8_t m_scroll;
	int m_mux;

	uint8_t m_main_irq_mask;
	uint8_t m_sub_irq_mask;
	uint8_t m_sub2_irq_mask;

	void common_latch_w(uint32_t offset);
	void superpac_latch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void phozon_latch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mappy_latch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void superpac_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mappy_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void superpac_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t superpac_flipscreen_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mappy_scroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t dipA_l(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dipA_h(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dipB_mux(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dipB_muxi(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void out_mux(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void out_lamps(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	tilemap_memory_index superpac_tilemap_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	tilemap_memory_index mappy_tilemap_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void superpac_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void phozon_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void mappy_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_mappy();
	void machine_reset_superpac();
	void video_start_superpac();
	void palette_init_superpac(palette_device &palette);
	void machine_reset_phozon();
	void video_start_phozon();
	void palette_init_phozon(palette_device &palette);
	void machine_reset_mappy();
	void video_start_mappy();
	void palette_init_mappy(palette_device &palette);
	uint32_t screen_update_superpac(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_phozon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_mappy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void superpac_main_vblank_irq(device_t &device);
	void pacnpal_main_vblank_irq(device_t &device);
	void grobda_main_vblank_irq(device_t &device);
	void phozon_main_vblank_irq(device_t &device);
	void mappy_main_vblank_irq(device_t &device);
	void digdug2_main_vblank_irq(device_t &device);
	void motos_main_vblank_irq(device_t &device);
	void sub_vblank_irq(device_t &device);
	void sub2_vblank_irq(device_t &device);
	void superpac_io_run(void *ptr, int32_t param);
	void pacnpal_io_run(void *ptr, int32_t param);
	void grobda_io_run(void *ptr, int32_t param);
	void phozon_io_run(void *ptr, int32_t param);
	void mappy_io_run(void *ptr, int32_t param);
	void digdug2_io_run(void *ptr, int32_t param);
	void motos_io_run(void *ptr, int32_t param);
	void init_superpac();
	void init_pacnpal();
	void init_grobda();
	void init_phozon();
	void init_mappy();
	void init_druaga();
	void init_digdug2();
	void init_motos();
	void mappy_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t *spriteram_base);
	void phozon_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t *spriteram_base);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
