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
	uint8_t m_videoram[0x1000];
	uint8_t m_workram[0x1000];

	optional_region_ptr<uint8_t> m_prom;
	optional_region_ptr<uint8_t> m_rom;

	/* video-related */
	tilemap_t *m_bg_tilemap;

	/* sound-related */
	uint8_t m_soundlatch;

	void sound_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t fenraya_custom_map_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void fenraya_custom_map_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fenraya_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_unkpacg();
	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	uint32_t screen_update_4enraya(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
};
