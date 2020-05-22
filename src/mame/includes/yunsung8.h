// license:BSD-3-Clause
// copyright-holders:Luca Elia
/*************************************************************************

    Yun Sung 8 Bit Games

*************************************************************************/

#include "machine/74157.h"
#include "sound/msm5205.h"
#include "emupal.h"
#include "tilemap.h"

class yunsung8_state : public driver_device
{
public:
	yunsung8_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_adpcm_select(*this, "adpcm_select"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{
	}

	void yunsung8(machine_config &config);

private:
	/* video-related */
	tilemap_t     *m_bg_tilemap;
	tilemap_t     *m_fg_tilemap;
	uint8_t       *m_bg_vram;
	uint8_t       *m_fg_vram;
	int         m_layers_ctrl;
	int         m_videobank;

	/* misc */
	bool        m_toggle;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm;
	required_device<ls157_device> m_adpcm_select;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* memory */
	uint8_t      m_videoram[0x4000];

	void bankswitch_w(uint8_t data);
	void main_irq_ack_w(uint8_t data);
	void videobank_w(uint8_t data);
	uint8_t videoram_r(offs_t offset);
	void videoram_w(offs_t offset, uint8_t data);
	void flipscreen_w(uint8_t data);
	void sound_bankswitch_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(adpcm_int);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map);
	void port_map(address_map &map);
	void sound_map(address_map &map);
};
