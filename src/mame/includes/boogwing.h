// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Haywood
/*************************************************************************

    Boogie Wings

*************************************************************************/

#include "sound/okim6295.h"
#include "video/deco16ic.h"
#include "video/decocomn.h"
#include "video/bufsprite.h"
#include "video/decocomn.h"
#include "video/decospr.h"
#include "machine/deco104.h"

class boogwing_state : public driver_device
{
public:
	boogwing_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_audiocpu(*this, "audiocpu"),
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
			m_palette(*this, "palette"),
			m_decrypted_opcodes(*this, "decrypted_opcodes")
	{ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<deco104_device> m_deco104;
	required_device<decocomn_device> m_decocomn;
	required_device<deco16ic_device> m_deco_tilegen1;
	required_device<deco16ic_device> m_deco_tilegen2;
	required_device<okim6295_device> m_oki1;
	required_device<okim6295_device> m_oki2;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_device<buffered_spriteram16_device> m_spriteram2;
	/* memory pointers */
	required_shared_ptr<UINT16> m_pf1_rowscroll;
	required_shared_ptr<UINT16> m_pf2_rowscroll;
	required_shared_ptr<UINT16> m_pf3_rowscroll;
	required_shared_ptr<UINT16> m_pf4_rowscroll;
	required_device<decospr_device> m_sprgen1;
	required_device<decospr_device> m_sprgen2;
	required_device<palette_device> m_palette;
	required_shared_ptr<UINT16> m_decrypted_opcodes;

	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_DRIVER_INIT(boogwing);
	virtual void video_start() override;
	UINT32 screen_update_boogwing(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void mix_boogwing(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_READ16_MEMBER( boogwing_protection_region_0_104_r );
	DECLARE_WRITE16_MEMBER( boogwing_protection_region_0_104_w );

	DECO16IC_BANK_CB_MEMBER(bank_callback);
	DECO16IC_BANK_CB_MEMBER(bank_callback2);
};
