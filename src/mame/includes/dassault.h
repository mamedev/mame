// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*************************************************************************

    Desert Assault

*************************************************************************/

#include "sound/okim6295.h"
#include "video/deco16ic.h"
#include "video/decocomn.h"
#include "video/bufsprite.h"
#include "video/decospr.h"

class dassault_state : public driver_device
{
public:
	dassault_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_audiocpu(*this, "audiocpu"),
			m_subcpu(*this, "sub"),
			m_decocomn(*this, "deco_common"),
			m_deco_tilegen1(*this, "tilegen1"),
			m_deco_tilegen2(*this, "tilegen2"),
			m_oki2(*this, "oki2"),
			m_spriteram(*this, "spriteram"),
			m_spriteram2(*this, "spriteram2") ,
		m_pf2_rowscroll(*this, "pf2_rowscroll"),
		m_pf4_rowscroll(*this, "pf4_rowscroll"),
		m_ram(*this, "ram"),
		m_shared_ram(*this, "shared_ram"),
		m_ram2(*this, "ram2"),
		m_sprgen1(*this, "spritegen1"),
		m_sprgen2(*this, "spritegen2"),
		m_palette(*this, "palette")
	{ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	required_device<decocomn_device> m_decocomn;
	required_device<deco16ic_device> m_deco_tilegen1;
	required_device<deco16ic_device> m_deco_tilegen2;
	required_device<okim6295_device> m_oki2;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_device<buffered_spriteram16_device> m_spriteram2;

	/* memory pointers */
	required_shared_ptr<UINT16> m_pf2_rowscroll;
	required_shared_ptr<UINT16> m_pf4_rowscroll;
	required_shared_ptr<UINT16> m_ram;
	required_shared_ptr<UINT16> m_shared_ram;
	required_shared_ptr<UINT16> m_ram2;

	optional_device<decospr_device> m_sprgen1;
	optional_device<decospr_device> m_sprgen2;
	required_device<palette_device> m_palette;

	DECLARE_READ16_MEMBER(dassault_control_r);
	DECLARE_WRITE16_MEMBER(dassault_control_w);
	DECLARE_READ16_MEMBER(dassault_sub_control_r);
	DECLARE_WRITE16_MEMBER(dassault_sound_w);
	DECLARE_READ16_MEMBER(dassault_irq_r);
	DECLARE_WRITE16_MEMBER(dassault_irq_w);
	DECLARE_WRITE16_MEMBER(shared_ram_w);
	DECLARE_READ16_MEMBER(shared_ram_r);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_DRIVER_INIT(thndzone);
	DECLARE_DRIVER_INIT(dassault);
	virtual void video_start() override;
	UINT32 screen_update_dassault(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void mixdassaultlayer(bitmap_rgb32 &bitmap, bitmap_ind16* sprite_bitmap, const rectangle &cliprect, UINT16 pri, UINT16 primask, UINT16 penbase, UINT8 alpha);
	DECO16IC_BANK_CB_MEMBER(bank_callback);
};
