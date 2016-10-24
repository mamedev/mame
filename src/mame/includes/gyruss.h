// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Couriersud
/*************************************************************************

    Gyruss

*************************************************************************/

#include "sound/discrete.h"

class gyruss_state : public driver_device
{
public:
	gyruss_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_audiocpu(*this, "audiocpu"),
		m_audiocpu_2(*this, "audio2"),
		m_discrete(*this, "discrete"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_flipscreen(*this, "flipscreen"),
		m_spriteram(*this, "spriteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	/* devices/memory pointers */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_audiocpu_2;
	required_device<discrete_device> m_discrete;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_flipscreen;
	required_shared_ptr<uint8_t> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	tilemap_t *m_tilemap;
	uint8_t m_master_nmi_mask;
	uint8_t m_slave_irq_mask;

	void gyruss_irq_clear_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gyruss_sh_irqtrigger_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gyruss_i8039_irq_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void master_nmi_mask_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void slave_irq_mask_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gyruss_spriteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t gyruss_scanline_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t gyruss_portA_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void gyruss_dac_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gyruss_filter0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gyruss_filter1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_gyruss();
	void gyruss_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void video_start() override;
	void palette_init_gyruss(palette_device &palette);
	uint32_t screen_update_gyruss(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void master_vblank_irq(device_t &device);
	void slave_vblank_irq(device_t &device);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void filter_w(address_space &space, int chip, int data );
};
