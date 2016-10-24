// license:BSD-3-Clause
// copyright-holders:Uki
/*************************************************************************

    Dr. Micro

*************************************************************************/
#include "sound/msm5205.h"

class drmicro_state : public driver_device
{
public:
	drmicro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_msm(*this, "msm"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	std::unique_ptr<uint8_t[]>       m_videoram;

	/* video-related */
	tilemap_t        *m_bg1;
	tilemap_t        *m_bg2;
	int            m_flipscreen;

	/* misc */
	int            m_nmi_enable;
	int            m_pcm_adr;

	/* devices */
	required_device<msm5205_device> m_msm;
	void nmi_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pcm_set_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void drmicro_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_bg1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg2_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_drmicro(palette_device &palette);
	uint32_t screen_update_drmicro(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void drmicro_interrupt(device_t &device);
	void pcm_w(int state);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
