// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Labyrinth Runner

*************************************************************************/

#include "video/k007121.h"
#include "video/k051733.h"

class labyrunr_state : public driver_device
{
public:
	labyrunr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_k007121(*this, "k007121"),
		m_maincpu(*this,"maincpu"),
		m_scrollram(*this, "scrollram"),
		m_spriteram(*this, "spriteram"),
		m_videoram1(*this, "videoram1"),
		m_videoram2(*this, "videoram2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* devices */
	required_device<k007121_device> m_k007121;

	required_device<cpu_device> m_maincpu;
	/* memory pointers */
	required_shared_ptr<uint8_t> m_scrollram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram1;
	required_shared_ptr<uint8_t> m_videoram2;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	/* video-related */
	tilemap_t    *m_layer0;
	tilemap_t    *m_layer1;
	rectangle  m_clip0;
	rectangle  m_clip1;

	void labyrunr_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void labyrunr_vram1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void labyrunr_vram2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_tile_info0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void video_start() override;
	void palette_init_labyrunr(palette_device &palette);
	uint32_t screen_update_labyrunr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void labyrunr_vblank_interrupt(device_t &device);
	void labyrunr_timer_interrupt(device_t &device);
};
