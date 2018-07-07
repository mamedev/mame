// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*************************************************************************

    Street Fighter hardware

*************************************************************************/

#include "machine/gen_latch.h"
#include "sound/msm5205.h"
#include "emupal.h"

class sf_state : public driver_device
{
public:
	sf_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm%u", 1U),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_videoram(*this, "videoram"),
		m_objectram(*this, "objectram"),
		m_tilerom(*this, "tilerom"),
		m_audiobank(*this, "audiobank")
	{ }

	void sfp(machine_config &config);
	void sfjp(machine_config &config);
	void sfus(machine_config &config);
	void sfan(machine_config &config);

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device_array<msm5205_device, 2> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	/* memory pointers */
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_objectram;
	required_region_ptr<uint8_t> m_tilerom;

	required_memory_bank m_audiobank;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_tx_tilemap;
	int m_active;
	uint16_t m_bgscroll;
	uint16_t m_fgscroll;

	DECLARE_WRITE8_MEMBER(coin_w);
	DECLARE_WRITE8_MEMBER(soundcmd_w);
	DECLARE_WRITE16_MEMBER(protection_w);
	DECLARE_WRITE8_MEMBER(sound2_bank_w);
	DECLARE_WRITE16_MEMBER(videoram_w);
	DECLARE_WRITE16_MEMBER(bg_scroll_w);
	DECLARE_WRITE16_MEMBER(fg_scroll_w);
	DECLARE_WRITE16_MEMBER(gfxctrl_w);
	template<int Chip> DECLARE_WRITE8_MEMBER(msm_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	inline int invert( int nb );
	void draw_sprites( bitmap_ind16 &bitmap,const rectangle &cliprect );
	void write_dword( address_space &space, offs_t offset, uint32_t data );

	void sfan_map(address_map &map);
	void sfjp_map(address_map &map);
	void sfus_map(address_map &map);
	void sound2_io_map(address_map &map);
	void sound2_map(address_map &map);
	void sound_map(address_map &map);
};
