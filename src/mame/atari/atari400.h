// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller, Angelo Salese
/******************************************************************************

    Atari 400/800

    ANTIC video controller
    GTIA  graphics television interface adapter

    Juergen Buchmueller, June 1998

******************************************************************************/

#ifndef MAME_ATARI_ATARI400_H
#define MAME_ATARI_ATARI400_H

#pragma once

#include "machine/6821pia.h"
#include "machine/ram.h"
#include "sound/pokey.h"
#include "antic.h"
#include "gtia.h"

#include "emupal.h"
#include "screen.h"

#include <algorithm>


class atari_common_state : public driver_device
{
public:
	atari_common_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gtia(*this, "gtia")
		, m_antic(*this, "antic")
		, m_pokey(*this, "pokey")
		, m_screen(*this, "screen")
		, m_keyboard(*this, "keyboard.%u", 0)
		, m_keypad(*this, "keypad.%u", 0)
		, m_djoy_b(*this, "djoy_b")
		, m_fake(*this, "fake")
	{ }

protected:
	virtual void video_start() override ATTR_COLD;

	void atari_palette(palette_device &palette) const;

	POKEY_KEYBOARD_CB_MEMBER(a5200_keypads);
	POKEY_KEYBOARD_CB_MEMBER(a800_keyboard);

	required_device<cpu_device> m_maincpu;
	required_device<gtia_device> m_gtia;
	required_device<antic_device> m_antic;
	required_device<pokey_device> m_pokey;
	required_device<screen_device> m_screen;
	optional_ioport_array<8> m_keyboard;
	optional_ioport_array<4> m_keypad;
	optional_ioport m_djoy_b;
	optional_ioport m_fake;

	void config_ntsc_screen(machine_config &config);
	void config_pal_screen(machine_config &config);
};

#endif // MAME_ATARI_ATARI400_H
