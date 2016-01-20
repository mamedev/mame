// license:BSD-3-Clause
// copyright-holders:David Haywood
#include "sound/okim6295.h"

class funybubl_state : public driver_device
{
public:
	funybubl_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_paletteram(*this, "paletteram"),
		m_audiocpu(*this, "audiocpu"),
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_paletteram;

	/* devices */
	required_device<cpu_device> m_audiocpu;

	/* memory */
	UINT8      m_banked_vram[0x2000];
	DECLARE_WRITE8_MEMBER(funybubl_vidram_bank_w);
	DECLARE_WRITE8_MEMBER(funybubl_cpurombank_w);
	DECLARE_WRITE8_MEMBER(funybubl_soundcommand_w);
	DECLARE_WRITE8_MEMBER(funybubl_paldatawrite);
	DECLARE_WRITE8_MEMBER(funybubl_oki_bank_sw);
	virtual void machine_start() override;
	virtual void video_start() override;
	UINT32 screen_update_funybubl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
