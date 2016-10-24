// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Fire Trap

***************************************************************************/

#include "machine/gen_latch.h"
#include "sound/msm5205.h"

class firetrap_state : public driver_device
{
public:
	firetrap_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_bg1videoram(*this, "bg1videoram"),
		m_bg2videoram(*this, "bg2videoram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_bg1videoram;
	required_shared_ptr<uint8_t> m_bg2videoram;
	required_shared_ptr<uint8_t> m_fgvideoram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t       *m_fg_tilemap;
	tilemap_t       *m_bg1_tilemap;
	tilemap_t       *m_bg2_tilemap;
	uint8_t         m_scroll1_x[2];
	uint8_t         m_scroll1_y[2];
	uint8_t         m_scroll2_x[2];
	uint8_t         m_scroll2_y[2];

	/* misc */
	int           m_sound_irq_enable;
	int           m_nmi_enable;
	int           m_i8751_return;
	int           m_i8751_current_command;
	int           m_i8751_init_ptr;
	int           m_msm5205next;
	int           m_adpcm_toggle;
	int           m_coin_command_pending;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	void firetrap_nmi_disable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void firetrap_bankselect_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t firetrap_8751_bootleg_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t firetrap_8751_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void firetrap_8751_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void firetrap_sound_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void firetrap_sound_2400_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void firetrap_sound_bankselect_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void firetrap_adpcm_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flip_screen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void firetrap_fgvideoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void firetrap_bg1videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void firetrap_bg2videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void firetrap_bg1_scrollx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void firetrap_bg1_scrolly_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void firetrap_bg2_scrollx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void firetrap_bg2_scrolly_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void coin_inserted(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	tilemap_memory_index get_fg_memory_offset(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	tilemap_memory_index get_bg_memory_offset(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg2_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_firetrap(palette_device &palette);
	uint32_t screen_update_firetrap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void firetrap_irq(device_t &device);
	inline void get_bg_tile_info(tile_data &tileinfo, int tile_index, uint8_t *bgvideoram, int gfx_region);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void firetrap_adpcm_int(int state);
};
