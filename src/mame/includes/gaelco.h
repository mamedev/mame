// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

    Gaelco game hardware from 1991-1996

***************************************************************************/

#include "machine/gen_latch.h"
#include "machine/74259.h"
#include "emupal.h"
#include "tilemap.h"

class gaelco_state : public driver_device
{
public:
	gaelco_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_audiocpu(*this, "audiocpu"),
		m_soundlatch(*this, "soundlatch"),
		m_outlatch(*this, "outlatch"),
		m_okibank(*this, "okibank"),
		m_videoram(*this, "videoram"),
		m_vregs(*this, "vregs"),
		m_spriteram(*this, "spriteram"),
		m_screenram(*this, "screenram") { }

	void bigkarnk(machine_config &config);
	void thoop(machine_config &config);
	void maniacsq(machine_config &config);
	void squash(machine_config &config);

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<cpu_device> m_audiocpu;
	optional_device<generic_latch_8_device> m_soundlatch;
	optional_device<ls259_device> m_outlatch;
	optional_memory_bank m_okibank;

	/* memory pointers */
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_vregs;
	required_shared_ptr<uint16_t> m_spriteram;
	optional_shared_ptr<uint16_t> m_screenram;

	/* video-related */
	tilemap_t      *m_tilemap[2];

	DECLARE_WRITE_LINE_MEMBER(coin1_lockout_w);
	DECLARE_WRITE_LINE_MEMBER(coin2_lockout_w);
	DECLARE_WRITE_LINE_MEMBER(coin1_counter_w);
	DECLARE_WRITE_LINE_MEMBER(coin2_counter_w);
	DECLARE_WRITE8_MEMBER(oki_bankswitch_w);
	DECLARE_WRITE16_MEMBER(vram_encrypted_w);
	DECLARE_WRITE16_MEMBER(encrypted_w);
	DECLARE_WRITE16_MEMBER(thoop_vram_encrypted_w);
	DECLARE_WRITE16_MEMBER(thoop_encrypted_w);
	void vram_w(offs_t offset, u16 data, u16 mem_mask);

	template<int Layer> TILE_GET_INFO_MEMBER(get_tile_info);

	virtual void machine_start() override;
	DECLARE_VIDEO_START(bigkarnk);
	DECLARE_VIDEO_START(maniacsq);

	uint32_t screen_update_bigkarnk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_maniacsq(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );

	void bigkarnk_map(address_map &map);
	void bigkarnk_snd_map(address_map &map);
	void maniacsq_map(address_map &map);
	void oki_map(address_map &map);
	void squash_map(address_map &map);
	void thoop_map(address_map &map);
};
