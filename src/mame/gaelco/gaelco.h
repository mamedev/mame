// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

    Gaelco game hardware from 1991-1996

***************************************************************************/

#include "machine/gen_latch.h"
#include "machine/74259.h"
#include "gaelcrpt.h"
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
		m_vramcrypt(*this, "vramcrypt"),
		m_audiocpu(*this, "audiocpu"),
		m_soundlatch(*this, "soundlatch"),
		m_outlatch(*this, "outlatch"),
		m_okibank(*this, "okibank"),
		m_videoram(*this, "videoram"),
		m_vregs(*this, "vregs"),
		m_spriteram(*this, "spriteram"),
		m_screenram(*this, "screenram"),
		m_sprite_palette_force_high(0x38)
	{ }

	void bigkarnk(machine_config &config);
	void thoop(machine_config &config);
	void maniacsq(machine_config &config);
	void squash(machine_config &config);

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<gaelco_vram_encryption_device> m_vramcrypt;
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
	tilemap_t      *m_tilemap[2]{};

	void coin1_lockout_w(int state);
	void coin2_lockout_w(int state);
	void coin1_counter_w(int state);
	void coin2_counter_w(int state);
	void oki_bankswitch_w(uint8_t data);
	void vram_encrypted_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void encrypted_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void vram_w(offs_t offset, u16 data, u16 mem_mask);
	void irqack_w(uint16_t data);

	template<int Layer> TILE_GET_INFO_MEMBER(get_tile_info);

	virtual void machine_start() override ATTR_COLD;
	DECLARE_VIDEO_START(bigkarnk);
	DECLARE_VIDEO_START(maniacsq);
	DECLARE_VIDEO_START(squash);

	uint32_t screen_update_bigkarnk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_maniacsq(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_thoop(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );

	void bigkarnk_map(address_map &map) ATTR_COLD;
	void bigkarnk_snd_map(address_map &map) ATTR_COLD;
	void maniacsq_map(address_map &map) ATTR_COLD;
	void oki_map(address_map &map) ATTR_COLD;
	void squash_map(address_map &map) ATTR_COLD;
	void thoop_map(address_map &map) ATTR_COLD;

	/* per-game configuration */
	uint8_t m_sprite_palette_force_high = 0;

	static constexpr double FRAMERATE_922804 = 57.42;
};
