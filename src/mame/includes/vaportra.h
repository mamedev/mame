// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*************************************************************************

    Vapor Trail

*************************************************************************/

#include "video/bufsprite.h"
#include "video/deco16ic.h"
#include "video/decmxc06.h"

class vaportra_state : public driver_device
{
public:
	vaportra_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_deco_tilegen1(*this, "tilegen1"),
		m_deco_tilegen2(*this, "tilegen2"),
		m_spritegen(*this, "spritegen"),
		m_spriteram(*this, "spriteram"),
		m_palette(*this, "palette"),
		m_generic_paletteram_16(*this, "paletteram"),
		m_generic_paletteram2_16(*this, "paletteram2") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<deco16ic_device> m_deco_tilegen1;
	required_device<deco16ic_device> m_deco_tilegen2;
	required_device<deco_mxc06_device> m_spritegen;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT16> m_generic_paletteram_16;
	required_shared_ptr<UINT16> m_generic_paletteram2_16;

	/* misc */
	UINT16    m_priority[2];

	DECLARE_WRITE16_MEMBER(vaportra_sound_w);
	DECLARE_READ16_MEMBER(vaportra_control_r);
	DECLARE_READ8_MEMBER(vaportra_soundlatch_r);
	DECLARE_WRITE16_MEMBER(vaportra_priority_w);
	DECLARE_WRITE16_MEMBER(vaportra_palette_24bit_rg_w);
	DECLARE_WRITE16_MEMBER(vaportra_palette_24bit_b_w);

	DECLARE_DRIVER_INIT(vaportra);
	virtual void machine_start() override;
	virtual void machine_reset() override;

	UINT32 screen_update_vaportra(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void update_24bitcol( int offset );

	DECO16IC_BANK_CB_MEMBER(bank_callback);
};
