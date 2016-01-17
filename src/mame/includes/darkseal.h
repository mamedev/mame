// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#include "video/deco16ic.h"
#include "video/bufsprite.h"
#include "video/decospr.h"

class darkseal_state : public driver_device
{
public:
	darkseal_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_palette(*this, "palette"),
		m_deco_tilegen1(*this, "tilegen1"),
		m_deco_tilegen2(*this, "tilegen2"),
		m_sprgen(*this, "spritegen"),
		m_spriteram(*this, "spriteram"),
		m_ram(*this, "ram"),
		m_pf1_rowscroll(*this, "pf1_rowscroll"),
		m_pf3_rowscroll(*this, "pf3_rowscroll"),
		m_generic_paletteram_16(*this, "paletteram"),
		m_generic_paletteram2_16(*this, "paletteram2") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<palette_device> m_palette;
	required_device<deco16ic_device> m_deco_tilegen1;
	required_device<deco16ic_device> m_deco_tilegen2;
	required_device<decospr_device> m_sprgen;
	required_device<buffered_spriteram16_device> m_spriteram;

	required_shared_ptr<UINT16> m_ram;
	required_shared_ptr<UINT16> m_pf1_rowscroll;
	//UINT16 *m_pf2_rowscroll;
	required_shared_ptr<UINT16> m_pf3_rowscroll;
	//UINT16 *m_pf4_rowscroll;
	required_shared_ptr<UINT16> m_generic_paletteram_16;
	required_shared_ptr<UINT16> m_generic_paletteram2_16;

	DECLARE_WRITE16_MEMBER(control_w);
	DECLARE_READ16_MEMBER(control_r);
	DECLARE_WRITE16_MEMBER(palette_24bit_rg_w);
	DECLARE_WRITE16_MEMBER(palette_24bit_b_w);

	DECLARE_DRIVER_INIT(darkseal);
	virtual void video_start() override;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void update_24bitcol(int offset);
};
