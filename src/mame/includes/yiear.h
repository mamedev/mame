// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
// thanks-to:Enrique Sanchez
#include "sound/sn76496.h"
#include "sound/vlm5030.h"

class yiear_state : public driver_device
{
public:
	yiear_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2"),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_sn(*this, "snsnd"),
		m_vlm(*this, "vlm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_spriteram2;
	required_shared_ptr<uint8_t> m_videoram;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<sn76489a_device> m_sn;
	required_device<vlm5030_device> m_vlm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* video-related */
	tilemap_t  *m_bg_tilemap;

	uint8_t      m_yiear_nmi_enable;
	uint8_t      m_yiear_irq_enable;
	void yiear_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void yiear_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t yiear_speech_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void yiear_VLM5030_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t m_SN76496_latch;
	void konami_SN76496_latch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) { m_SN76496_latch = data; };
	void konami_SN76496_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) { m_sn->write(space, offset, m_SN76496_latch); };
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_yiear(palette_device &palette);
	uint32_t screen_update_yiear(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void yiear_vblank_interrupt(device_t &device);
	void yiear_nmi_interrupt(device_t &device);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
};
