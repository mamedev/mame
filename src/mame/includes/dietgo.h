// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Haywood
/*************************************************************************

    Diet Go Go

*************************************************************************/
#ifndef MAME_INCLUDES_DIETGO_H
#define MAME_INCLUDES_DIETGO_H

#pragma once

#include "cpu/h6280/h6280.h"
#include "video/decospr.h"
#include "video/deco16ic.h"
#include "machine/deco104.h"

class dietgo_state : public driver_device
{
public:
	dietgo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_deco104(*this, "ioprot104")
		, m_pf_rowscroll(*this, "pf%u_rowscroll", 1)
		, m_spriteram(*this, "spriteram")
		, m_sprgen(*this, "spritegen")
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_deco_tilegen(*this, "tilegen")
		, m_decrypted_opcodes(*this, "decrypted_opcodes")
	{ }

	void dietgo(machine_config &config);

	void init_dietgo();

private:
	optional_device<deco104_device> m_deco104;
	/* memory pointers */
	required_shared_ptr_array<uint16_t, 2> m_pf_rowscroll;
	required_shared_ptr<uint16_t> m_spriteram;
	optional_device<decospr_device> m_sprgen;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<h6280_device> m_audiocpu;
	required_device<deco16ic_device> m_deco_tilegen;
	required_shared_ptr<uint16_t> m_decrypted_opcodes;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECO16IC_BANK_CB_MEMBER(bank_callback);

	DECLARE_READ16_MEMBER( dietgo_protection_region_0_104_r );
	DECLARE_WRITE16_MEMBER( dietgo_protection_region_0_104_w );
	void decrypted_opcodes_map(address_map &map);
	void dietgo_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_DIETGO_H
