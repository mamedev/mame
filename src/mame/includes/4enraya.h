// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina, Roberto Fresca
/*************************************************************************

    IDSA 4 En Raya

*************************************************************************/

#include "sound/ay8910.h"

class _4enraya_state : public driver_device
{
public:
	_4enraya_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ay(*this, "aysnd")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_prom(*this, "pal_prom")
		, m_rom(*this, "maincpu")
	{
	}

	required_device<cpu_device> m_maincpu;
	required_device<ay8910_device> m_ay;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* memory pointers */
	UINT8 m_videoram[0x1000];
	UINT8 m_workram[0x1000];

	optional_region_ptr<UINT8> m_prom;
	optional_region_ptr<UINT8> m_rom;

	/* video-related */
	tilemap_t *m_bg_tilemap;

	/* sound-related */
	UINT8 m_soundlatch;

	DECLARE_WRITE8_MEMBER(sound_data_w);
	DECLARE_READ8_MEMBER(fenraya_custom_map_r);
	DECLARE_WRITE8_MEMBER(fenraya_custom_map_w);
	DECLARE_WRITE8_MEMBER(fenraya_videoram_w);
	DECLARE_WRITE8_MEMBER(sound_control_w);
	DECLARE_DRIVER_INIT(unkpacg);
	TILE_GET_INFO_MEMBER(get_tile_info);
	UINT32 screen_update_4enraya(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
};
