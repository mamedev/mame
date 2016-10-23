// license:BSD-3-Clause
// copyright-holders:Brad Oliver
/***************************************************************************

  Mr. Do's Castle hardware

***************************************************************************/

#include "video/mc6845.h"
#include "sound/msm5205.h"

class docastle_state : public driver_device
{
public:
	docastle_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_slave(*this, "slave"),
		m_cpu3(*this, "cpu3"),
		m_crtc(*this, "crtc"),
		m_msm(*this, "msm"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_slave;
	required_device<cpu_device> m_cpu3;
	required_device<h46505_device> m_crtc;
	optional_device<msm5205_device> m_msm;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* video-related */
	tilemap_t  *m_do_tilemap;

	/* misc */
	int      m_prev_ma6;
	int      m_adpcm_pos;
	int      m_adpcm_idle;
	int      m_adpcm_data;
	int      m_adpcm_status;
	uint8_t    m_buffer0[9];
	uint8_t    m_buffer1[9];

	uint8_t docastle_shared0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t docastle_shared1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void docastle_shared0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void docastle_shared1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void docastle_nmitrigger_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void docastle_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void docastle_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t flipscreen_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t idsoccer_adpcm_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void idsoccer_adpcm_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(docastle);
	void video_start_dorunrun();
	uint32_t screen_update_docastle(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void video_start_common( uint32_t tile_transmask );
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	DECLARE_WRITE_LINE_MEMBER(docastle_tint);
	DECLARE_WRITE_LINE_MEMBER(idsoccer_adpcm_int);
};
