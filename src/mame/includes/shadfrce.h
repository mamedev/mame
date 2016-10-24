// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "machine/gen_latch.h"
#include "sound/okim6295.h"

class shadfrce_state : public driver_device
{
public:
	shadfrce_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_io_p1(*this, "P1"),
		m_io_p2(*this, "P2"),
		m_io_dsw1(*this, "DSW1"),
		m_io_dsw2(*this, "DSW2"),
		m_io_other(*this, "OTHER"),
		m_io_extra(*this, "EXTRA"),
		m_io_misc(*this, "MISC"),
		m_io_system(*this, "SYSTEM"),
		m_fgvideoram(*this, "fgvideoram"),
		m_bg0videoram(*this, "bg0videoram"),
		m_bg1videoram(*this, "bg1videoram"),
		m_spvideoram(*this, "spvideoram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_ioport m_io_p1;
	required_ioport m_io_p2;
	required_ioport m_io_dsw1;
	required_ioport m_io_dsw2;
	required_ioport m_io_other;
	required_ioport m_io_extra;
	required_ioport m_io_misc;
	required_ioport m_io_system;

	required_shared_ptr<uint16_t> m_fgvideoram;
	required_shared_ptr<uint16_t> m_bg0videoram;
	required_shared_ptr<uint16_t> m_bg1videoram;
	required_shared_ptr<uint16_t> m_spvideoram;

	std::unique_ptr<uint16_t[]> m_spvideoram_old;
	tilemap_t *m_fgtilemap;
	tilemap_t *m_bg0tilemap;
	tilemap_t *m_bg1tilemap;
	int m_video_enable;
	int m_irqs_enable;
	int m_raster_scanline;
	int m_raster_irq_enable;
	int m_vblank;
	int m_prev_value;

	void flip_screen(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t input_ports_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void sound_brt_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void irq_ack_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void irq_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void scanline_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void fgvideoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bg0videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bg1videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bg0scrollx_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bg0scrolly_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bg1scrollx_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bg1scrolly_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void oki_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void get_fgtile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg0tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg1tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void video_start() override;

	void scanline(timer_device &timer, void *ptr, int32_t param);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof(screen_device &screen, bool state);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
};
