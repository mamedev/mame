// license:BSD-3-Clause
// copyright-holders:David Haywood, Bryan McPhail
/*************************************************************************

    Pocket Gal Deluxe

*************************************************************************/
#ifndef MAME_INCLUDES_PKTGALDX_H
#define MAME_INCLUDES_PKTGALDX_H

#pragma once

#include "sound/okim6295.h"
#include "decospr.h"
#include "deco16ic.h"
#include "deco104.h"
#include "emupal.h"

class pktgaldx_state : public driver_device
{
public:
	pktgaldx_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_deco104(*this, "ioprot104"),
		m_pf1_rowscroll(*this, "pf1_rowscroll"),
		m_pf2_rowscroll(*this, "pf2_rowscroll"),
		m_spriteram(*this, "spriteram"),
		m_pktgaldb_fgram(*this, "pktgaldb_fgram"),
		m_pktgaldb_sprites(*this, "pktgaldb_spr"),
		m_sprgen(*this, "spritegen"),
		m_maincpu(*this, "maincpu"),
		m_oki2(*this, "oki2"),
		m_deco_tilegen(*this, "tilegen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_decrypted_opcodes(*this, "decrypted_opcodes")
	{ }

	void pktgaldx(machine_config &config);
	void pktgaldb(machine_config &config);

	void init_pktgaldx();

private:
	optional_device<deco104_device> m_deco104;

	/* memory pointers */
	optional_shared_ptr<uint16_t> m_pf1_rowscroll;
	optional_shared_ptr<uint16_t> m_pf2_rowscroll;
	optional_shared_ptr<uint16_t> m_spriteram;
	optional_shared_ptr<uint16_t> m_pktgaldb_fgram;
	optional_shared_ptr<uint16_t> m_pktgaldb_sprites;
	optional_device<decospr_device> m_sprgen;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki2;
	optional_device<deco16ic_device> m_deco_tilegen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_shared_ptr<uint16_t> m_decrypted_opcodes;

	uint16_t pckgaldx_unknown_r();
	uint16_t pckgaldx_protection_r();
	void pktgaldx_oki_bank_w(uint16_t data);
	virtual void machine_start() override;
	uint32_t screen_update_pktgaldx(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_pktgaldb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint16_t pktgaldx_protection_region_f_104_r(offs_t offset);
	void pktgaldx_protection_region_f_104_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	DECLARE_WRITE_LINE_MEMBER( vblank_w );
	void vblank_ack_w(uint16_t data);

	DECO16IC_BANK_CB_MEMBER(bank_callback);
	void decrypted_opcodes_map(address_map &map);
	void pktgaldb_map(address_map &map);
	void pktgaldx_map(address_map &map);
};

#endif // MAME_INCLUDES_PKTGALDX_H
