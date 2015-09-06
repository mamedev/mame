// license:BSD-3-Clause
// copyright-holders:David Haywood, Bryan McPhail
/*************************************************************************

    Pocket Gal Deluxe

*************************************************************************/
#include "sound/okim6295.h"
#include "video/decospr.h"
#include "video/deco16ic.h"
#include "video/decocomn.h"
#include "machine/deco104.h"

class pktgaldx_state : public driver_device
{
public:
	pktgaldx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_deco104(*this, "ioprot104"),
		m_pf1_rowscroll(*this, "pf1_rowscroll"),
		m_pf2_rowscroll(*this, "pf2_rowscroll"),
		m_spriteram(*this, "spriteram"),
		m_pktgaldb_fgram(*this, "pktgaldb_fgram"),
		m_pktgaldb_sprites(*this, "pktgaldb_spr"),
		m_sprgen(*this, "spritegen"),
		m_maincpu(*this, "maincpu"),
		m_oki2(*this, "oki2"),
		m_deco_tilegen1(*this, "tilegen1"),
		m_decocomn(*this, "deco_common"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_decrypted_opcodes(*this, "decrypted_opcodes") { }

	optional_device<deco104_device> m_deco104;

	/* memory pointers */
	optional_shared_ptr<UINT16> m_pf1_rowscroll;
	optional_shared_ptr<UINT16> m_pf2_rowscroll;
	optional_shared_ptr<UINT16> m_spriteram;
	optional_shared_ptr<UINT16> m_pktgaldb_fgram;
	optional_shared_ptr<UINT16> m_pktgaldb_sprites;
	optional_device<decospr_device> m_sprgen;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki2;
	optional_device<deco16ic_device> m_deco_tilegen1;
	optional_device<decocomn_device> m_decocomn;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_shared_ptr<UINT16> m_decrypted_opcodes;

	DECLARE_READ16_MEMBER(pckgaldx_unknown_r);
	DECLARE_READ16_MEMBER(pckgaldx_protection_r);
	DECLARE_WRITE16_MEMBER(pktgaldx_oki_bank_w);
	DECLARE_DRIVER_INIT(pktgaldx);
	virtual void machine_start();
	UINT32 screen_update_pktgaldx(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_pktgaldb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	READ16_MEMBER( pktgaldx_protection_region_f_104_r );
	WRITE16_MEMBER( pktgaldx_protection_region_f_104_w );

	DECO16IC_BANK_CB_MEMBER(bank_callback);
};
