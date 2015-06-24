// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Haywood
/*************************************************************************

    Diet Go Go

*************************************************************************/

#include "video/decospr.h"
#include "video/deco16ic.h"
#include "video/decocomn.h"
#include "machine/deco104.h"

class dietgo_state : public driver_device
{
public:
	dietgo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_deco104(*this, "ioprot104"),
		m_pf1_rowscroll(*this, "pf1_rowscroll"),
		m_pf2_rowscroll(*this, "pf2_rowscroll"),
		m_spriteram(*this, "spriteram"),
		m_sprgen(*this, "spritegen"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_deco_tilegen1(*this, "tilegen1"),
		m_decocomn(*this, "deco_common"),
		m_decrypted_opcodes(*this, "decrypted_opcodes")
	{ }

	optional_device<deco104_device> m_deco104;
	/* memory pointers */
	required_shared_ptr<UINT16> m_pf1_rowscroll;
	required_shared_ptr<UINT16> m_pf2_rowscroll;
	required_shared_ptr<UINT16> m_spriteram;
	optional_device<decospr_device> m_sprgen;
//  UINT16 *  m_paletteram;    // currently this uses generic palette handling (in decocomn.c)

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<deco16ic_device> m_deco_tilegen1;
	required_device<decocomn_device> m_decocomn;
	required_shared_ptr<UINT16> m_decrypted_opcodes;
	DECLARE_DRIVER_INIT(dietgo);
	virtual void machine_start();
	UINT32 screen_update_dietgo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECO16IC_BANK_CB_MEMBER(bank_callback);

	DECLARE_READ16_MEMBER( dietgo_protection_region_0_104_r );
	DECLARE_WRITE16_MEMBER( dietgo_protection_region_0_104_w );
};
