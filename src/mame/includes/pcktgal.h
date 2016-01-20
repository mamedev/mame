// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#include "sound/msm5205.h"
#include "video/decbac06.h"

class pcktgal_state : public driver_device
{
public:
	pcktgal_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_tilegen1(*this, "tilegen1"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm;
	required_device<deco_bac06_device> m_tilegen1;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_spriteram;

	int m_msm5205next;
	int m_toggle;

	DECLARE_WRITE8_MEMBER(bank_w);
	DECLARE_WRITE8_MEMBER(sound_bank_w);
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_WRITE8_MEMBER(adpcm_data_w);
	DECLARE_READ8_MEMBER(adpcm_reset_r);
	DECLARE_WRITE_LINE_MEMBER(adpcm_int);

	DECLARE_DRIVER_INIT(pcktgal);
	DECLARE_PALETTE_INIT(pcktgal);
	virtual void machine_start() override;

	UINT32 screen_update_pcktgal(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_pcktgalb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
};
