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
#include "tia.h"

#define USE_NEW_RIOT 0

#if USE_NEW_RIOT
#include "machine/mos6530n.h"
#else
#include "machine/6532riot.h"
#endif

#define CONTROL1_TAG    "joyport1"
#define CONTROL2_TAG    "joyport2"


class a2600_base_state : public driver_device
{
protected:
	a2600_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_riot_ram(*this, "riot_ram"),
		m_tia(*this, "tia_video"),
		m_maincpu(*this, "maincpu"),
		m_riot(*this, "riot"),
		m_joy1(*this, CONTROL1_TAG),
		m_joy2(*this, CONTROL2_TAG),
		m_screen(*this, "screen")
	{ }

	virtual void machine_start() override;

	void a2600_base_ntsc(machine_config &config);

	void switch_A_w(uint8_t data);
	uint8_t switch_A_r();
	void switch_B_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(irq_callback);
	uint16_t a2600_read_input_port(offs_t offset);
	uint8_t a2600_get_databus_contents(offs_t offset);
	void a2600_tia_vsync_callback(uint16_t data);
	void a2600_tia_vsync_callback_pal(uint16_t data);

	void a2600_mem(address_map &map);

	required_shared_ptr<uint8_t> m_riot_ram;
	required_device<tia_video_device> m_tia;
	required_device<m6507_device> m_maincpu;
#if USE_NEW_RIOT
	required_device<mos6532_new_device> m_riot;
#else
	required_device<riot6532_device> m_riot;
#endif
	required_device<vcs_control_port_device> m_joy1;
	required_device<vcs_control_port_device> m_joy2;
	required_device<screen_device> m_screen;

private:
	uint16_t m_current_screen_height = 0U;
};


class a2600_state : public a2600_base_state
{
public:
	a2600_state(const machine_config &mconfig, device_type type, const char *tag) :
		a2600_base_state(mconfig, type, tag),
		m_cart(*this, "cartslot")
	{ }

	void a2600(machine_config &config);
	void a2600p(machine_config &config);

protected:
	virtual void machine_start() override;

	void cart_over_tia_w(address_space &space, offs_t offset, uint8_t data);
	// investigate how the carts mapped here (Mapper JVP) interact with the RIOT device
	uint8_t cart_over_riot_r(address_space &space, offs_t offset);
	void cart_over_riot_w(address_space &space, offs_t offset, uint8_t data);
	uint8_t cart_over_all_r(address_space &space, offs_t offset);
	void cart_over_all_w(address_space &space, offs_t offset, uint8_t data);

	void a2600_cartslot(machine_config &config);

private:
	required_device<vcs_cart_slot_device> m_cart;
};


class a2600_pop_state : public a2600_base_state
{
public:
	a2600_pop_state(const machine_config &mconfig, device_type type, const char *tag)
		: a2600_base_state(mconfig, type, tag)
		, m_bank(*this, "bank")
		, m_a8(*this, "A8")
		, m_swb(*this, "SWB")
	{ }

	void a2600_pop(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	void memory_map(address_map &map);

	uint8_t rom_switch_r(offs_t offset);
	void rom_switch_w(offs_t offset, uint8_t data);
	TIMER_CALLBACK_MEMBER(reset_timer_callback);
	TIMER_CALLBACK_MEMBER(game_select_button_timer_callback);

	required_memory_bank m_bank;
	required_ioport m_a8;
	required_ioport m_swb;
	emu_timer *m_reset_timer = nullptr;
	emu_timer *m_game_select_button_timer = nullptr;
};


#endif // MAME_INCLUDES_A2600_H
