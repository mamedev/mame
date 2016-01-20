// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller
/******************************************************************************

    Atari 400/800

    ANTIC video controller
    GTIA  graphics television interface adapter

    Juergen Buchmueller, June 1998

******************************************************************************/

#ifndef ATARI_H
#define ATARI_H

#include "machine/6821pia.h"
#include "sound/pokey.h"
#include "video/antic.h"
#include "video/gtia.h"


class atari_common_state : public driver_device
{
public:
	atari_common_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gtia(*this, "gtia"),
		m_antic(*this, "antic"),
		m_keyboard(*this, "keyboard"),
		m_keypad(*this, "keypad"),
		m_djoy_b(*this, "djoy_b"),
		m_fake(*this, "fake")
		{ }

	virtual void video_start() override;

	DECLARE_PALETTE_INIT(atari);

	POKEY_INTERRUPT_CB_MEMBER(interrupt_cb);
	POKEY_KEYBOARD_CB_MEMBER(a5200_keypads);
	POKEY_KEYBOARD_CB_MEMBER(a800_keyboard);

protected:
	required_device<cpu_device> m_maincpu;
	required_device<gtia_device> m_gtia;
	required_device<antic_device> m_antic;
	optional_ioport_array<8> m_keyboard;
	optional_ioport_array<4> m_keypad;
	optional_ioport m_djoy_b;
	optional_ioport m_fake;
};

#endif /* ATARI_H */
