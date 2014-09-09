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
	atari_common_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_gtia(*this, "gtia"),
		m_antic(*this, "antic")
		{ }

	virtual void video_start();

	TIMER_DEVICE_CALLBACK_MEMBER( a400_interrupt );
	TIMER_DEVICE_CALLBACK_MEMBER( a800_interrupt );
	TIMER_DEVICE_CALLBACK_MEMBER( a800xl_interrupt );
	TIMER_DEVICE_CALLBACK_MEMBER( a5200_interrupt );

	DECLARE_PALETTE_INIT(atari);

	POKEY_INTERRUPT_CB_MEMBER(interrupt_cb);
	POKEY_KEYBOARD_CB_MEMBER(a5200_keypads);
	POKEY_KEYBOARD_CB_MEMBER(a800_keyboard);

private:
	required_device<gtia_device> m_gtia;
	required_device<antic_device> m_antic;
};

#endif /* ATARI_H */
