// license:BSD-3-Clause
// copyright-holders:Acho A. Tang, Alex W. Jackson
/***************************************************************************

    B-Wings

***************************************************************************/

#include "machine/bankdev.h"
#include "machine/gen_latch.h"

#define BW_DEBUG 0

class bwing_state : public driver_device
{
public:
	bwing_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_vrambank(*this, "vrambank"),
		m_soundlatch(*this, "soundlatch"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_fgscrollram(*this, "fgscrollram"),
		m_bgscrollram(*this, "bgscrollram"),
		m_gfxram(*this, "gfxram") { }

	/* device */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<address_map_bank_device> m_vrambank;
	required_device<generic_latch_8_device> m_soundlatch;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_paletteram;
	required_shared_ptr<uint8_t> m_fgscrollram;
	required_shared_ptr<uint8_t> m_bgscrollram;
	required_shared_ptr<uint8_t> m_gfxram;

	/* video-related */
	tilemap_t *m_charmap;
	tilemap_t *m_fgmap;
	tilemap_t *m_bgmap;
	unsigned m_sreg[8];
	unsigned m_palatch;
	unsigned m_mapmask;

	/* sound-related */
	int m_bwp3_nmimask;
	int m_bwp3_u8F_d;

	void bwp3_u8F_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bwp3_nmimask_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bwp3_nmiack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bwp1_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bwp2_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fgscrollram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bgscrollram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gfxram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scrollreg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void paletteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void coin_inserted(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void tilt_pressed(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);

	void get_fgtileinfo(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bgtileinfo(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_charinfo(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	tilemap_memory_index scan_cols(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);

	void init_bwing();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void bwing_postload();

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bmp, const rectangle &clip, uint8_t *ram, int pri );

	void bwp3_interrupt(device_t &device);
};
