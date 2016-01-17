// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*************************************************************************

    Data East 'Rohga' era hardware

*************************************************************************/

#include "sound/okim6295.h"
#include "video/deco16ic.h"
#include "video/decocomn.h"
#include "video/bufsprite.h"
#include "video/decospr.h"
#include "machine/deco146.h"
#include "machine/deco104.h"

class rohga_state : public driver_device
{
public:
	rohga_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_deco146(*this, "ioprot"),
		m_deco104(*this, "ioprot104"),
		m_decocomn(*this, "deco_common"),
		m_deco_tilegen1(*this, "tilegen1"),
		m_deco_tilegen2(*this, "tilegen2"),
		m_oki1(*this, "oki1"),
		m_oki2(*this, "oki2"),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2") ,
		m_pf1_rowscroll(*this, "pf1_rowscroll"),
		m_pf2_rowscroll(*this, "pf2_rowscroll"),
		m_pf3_rowscroll(*this, "pf3_rowscroll"),
		m_pf4_rowscroll(*this, "pf4_rowscroll"),
		m_sprgen1(*this, "spritegen1"),
		m_sprgen2(*this, "spritegen2"),
		m_palette(*this, "palette")
	{ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<deco146_device> m_deco146;
	optional_device<deco104_device> m_deco104;
	required_device<decocomn_device> m_decocomn;
	required_device<deco16ic_device> m_deco_tilegen1;
	required_device<deco16ic_device> m_deco_tilegen2;
	required_device<okim6295_device> m_oki1;
	required_device<okim6295_device> m_oki2;
	required_device<buffered_spriteram16_device> m_spriteram;
	optional_device<buffered_spriteram16_device> m_spriteram2;

	/* memory pointers */
	optional_shared_ptr<UINT16> m_pf1_rowscroll;
	optional_shared_ptr<UINT16> m_pf2_rowscroll;
	required_shared_ptr<UINT16> m_pf3_rowscroll;
	required_shared_ptr<UINT16> m_pf4_rowscroll;

	optional_device<decospr_device> m_sprgen1;
	optional_device<decospr_device> m_sprgen2;

	required_device<palette_device> m_palette;

	DECLARE_READ16_MEMBER(rohga_irq_ack_r);
	DECLARE_WRITE16_MEMBER(wizdfire_irq_ack_w);
	DECLARE_WRITE16_MEMBER(rohga_buffer_spriteram16_w);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_DRIVER_INIT(wizdfire);
	DECLARE_DRIVER_INIT(nitrobal);
	DECLARE_DRIVER_INIT(schmeisr);
	DECLARE_DRIVER_INIT(rohga);
	DECLARE_VIDEO_START(wizdfire);
	UINT32 screen_update_rohga(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_wizdfire(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_nitrobal(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void mixwizdfirelayer(bitmap_rgb32 &bitmap, const rectangle &cliprect, int gfxregion, UINT16 pri, UINT16 primask);
	DECO16IC_BANK_CB_MEMBER(bank_callback);
	DECOSPR_PRIORITY_CB_MEMBER(rohga_pri_callback);
	DECOSPR_COLOUR_CB_MEMBER(rohga_col_callback);
	DECOSPR_COLOUR_CB_MEMBER(schmeisr_col_callback);

	READ16_MEMBER( nb_protection_region_0_146_r );
	WRITE16_MEMBER( nb_protection_region_0_146_w );
	READ16_MEMBER( wf_protection_region_0_104_r );
	WRITE16_MEMBER( wf_protection_region_0_104_w );
};
