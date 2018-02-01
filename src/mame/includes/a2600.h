// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Wilbert Pol, David Shah
/*****************************************************************************

    a2600.h

    Atari 2600

 ****************************************************************************/

#ifndef MAME_INCLUDES_A2600_H
#define MAME_INCLUDES_A2600_H

#pragma once

#include "bus/vcs/compumat.h"
#include "bus/vcs/dpc.h"
#include "bus/vcs/harmony_melody.h"
#include "bus/vcs/rom.h"
#include "bus/vcs/scharger.h"
#include "bus/vcs/vcs_slot.h"
#include "bus/vcs_ctrl/ctrl.h"
#include "cpu/m6502/m6507.h"
#include "sound/tiaintf.h"
#include "video/tia.h"

#define USE_NEW_RIOT 0

#if USE_NEW_RIOT
#include "machine/mos6530n.h"
#else
#include "machine/6532riot.h"
#endif

#define CONTROL1_TAG    "joyport1"
#define CONTROL2_TAG    "joyport2"


class a2600_state : public driver_device
{
public:
	a2600_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_riot_ram(*this, "riot_ram"),
		m_joy1(*this, CONTROL1_TAG),
		m_joy2(*this, CONTROL2_TAG),
		m_cart(*this, "cartslot"),
		m_tia(*this, "tia_video"),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_swb(*this, "SWB"),
		m_riot(*this, "riot")
	{ }

	required_shared_ptr<uint8_t> m_riot_ram;
	uint16_t m_current_screen_height;

	DECLARE_MACHINE_START(a2600);
	DECLARE_WRITE8_MEMBER(switch_A_w);
	DECLARE_READ8_MEMBER(switch_A_r);
	DECLARE_WRITE8_MEMBER(switch_B_w);
	DECLARE_WRITE_LINE_MEMBER(irq_callback);
	DECLARE_READ8_MEMBER(riot_input_port_8_r);
	DECLARE_READ16_MEMBER(a2600_read_input_port);
	DECLARE_READ8_MEMBER(a2600_get_databus_contents);
	DECLARE_WRITE16_MEMBER(a2600_tia_vsync_callback);
	DECLARE_WRITE16_MEMBER(a2600_tia_vsync_callback_pal);
	DECLARE_WRITE8_MEMBER(cart_over_tia_w);
	// investigate how the carts mapped here (Mapper JVP) interact with the RIOT device
	DECLARE_READ8_MEMBER(cart_over_riot_r);
	DECLARE_WRITE8_MEMBER(cart_over_riot_w);
	DECLARE_READ8_MEMBER(cart_over_all_r);
	DECLARE_WRITE8_MEMBER(cart_over_all_w);

	void a2600p(machine_config &config);
	void a2600(machine_config &config);
	void a2600_cartslot(machine_config &config);
	void a2600_mem(address_map &map);
protected:
	required_device<vcs_control_port_device> m_joy1;
	required_device<vcs_control_port_device> m_joy2;
	optional_device<vcs_cart_slot_device> m_cart;
	required_device<tia_video_device> m_tia;

	required_device<m6507_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_ioport m_swb;
#if USE_NEW_RIOT
	required_device<mos6532_t> m_riot;
#else
	required_device<riot6532_device> m_riot;
#endif
};


#endif // MAME_INCLUDES_A2600_H
