// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:bataais
/******************************************************************************

Applied Concepts Great Game Machine (GGM), electronic board game computer.
2nd source distribution: Modular Game System (MGS), by Chafitz.

Hardware notes:
- 6502A 2MHz (unknown XTAL, label "5-80"), SYP6522 VIA
- 2KB RAM(4*HM472114AP-2), no ROM on main PCB
- 2*74164 shift register, 3*6118P VFD driver
- 8-digit 14seg VFD panel (same one as in Speak & Spell)
- 5*4 keypad(unlabeled by default), 1-bit sound

Games are on separate cartridges, each came with a keypad overlay.
There were also some standalone machines, eg. Morphy Encore, Odin Encore.

Known chess cartridges (*denotes not dumped):
- Chess/Boris 2.5 (aka Sargon 2.5)
- *Gruenfeld Edition - Master Chess Openings
- *Morphy Edition - Master Chess
- *Capablanca Edition - Master Chess Endgame
- Sandy Edition - Master Chess (German language version of Morphy)
- Steinitz Edition-4 - Master Chess
- *Monitor Edition - Master Kriegspiel

The opening/endgame cartridges are meant to be ejected/inserted while
playing (put the power switch in "MEM" first).

Other games:
- *Borchek Edition - Master Checkers
- *Odin Edition - Master Reversi
- *Las Vegas 21
- *Wits End (unreleased?)
- *Lunar Lander (unreleased?)

TODO:
- what's VIA PB0 for? game toggles it once per irq
- identify XTAL (2MHz CPU/VIA is correct, compared to video reference)
- confirm display AP segment, is it used anywhere?
- verify cartridge pinout, right now assume A0-A15 (max known cart size is 24KB).
  Boris/Sargon cartridge is A0-A11 and 2 CS lines, Steinitz uses the whole range.
- (probably won't) add chesspieces to artwork? this machine supports more board
  games than just chess: checkers, reversi, and even a blackjack game

******************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/6522via.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "video/pwm.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "speaker.h"
#include "softlist.h"

// internal artwork
#include "aci_ggm.lh" // clickable


namespace {

class ggm_state : public driver_device
{
public:
	ggm_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_via(*this, "via"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_cart(*this, "cartslot"),
		m_ca1_off(*this, "ca1_off"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void ggm(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(reset_switch) { update_reset(newval); }
	DECLARE_CUSTOM_INPUT_MEMBER(overlay_r) { u8 data = m_inputs[5]->read(); return (data == 0xf) ? m_overlay : data; }

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<via6522_device> m_via;
	required_device<pwm_display_device> m_display;
	required_device<dac_bit_interface> m_dac;
	required_device<generic_slot_device> m_cart;
	required_device<timer_device> m_ca1_off;
	required_ioport_array<4+3> m_inputs;

	void main_map(address_map &map);

	void update_reset(ioport_value state);
	void update_display();
	TIMER_DEVICE_CALLBACK_MEMBER(ca1_off) { m_via->write_ca1(0); }

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cartridge);
	DECLARE_READ8_MEMBER(cartridge_r);

	DECLARE_WRITE8_MEMBER(select_w);
	DECLARE_WRITE8_MEMBER(control_w);
	DECLARE_READ8_MEMBER(input_r);

	DECLARE_WRITE_LINE_MEMBER(shift_clock_w);
	DECLARE_WRITE_LINE_MEMBER(shift_data_w);

	u8 m_inp_mux;
	u16 m_digit_data;
	u8 m_shift_data;
	u8 m_shift_clock;
	u32 m_cart_mask;
	u8 m_overlay;
};

void ggm_state::machine_start()
{
	// zerofill
	m_inp_mux = 0;
	m_digit_data = 0;
	m_shift_data = 0;
	m_shift_clock = 0;

	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_digit_data));
	save_item(NAME(m_shift_data));
	save_item(NAME(m_shift_clock));
}

void ggm_state::machine_reset()
{
	// it determines whether it's a cold boot or warm boot ("MEM" switch), with CA1
	if (~m_inputs[4]->read() & 2)
	{
		m_via->write_ca1(1);
		m_ca1_off->adjust(attotime::from_msec(10));
	}
	else
		update_reset(1);
}

void ggm_state::update_reset(ioport_value state)
{
	// assume that the MEM switch puts the system in reset state (just like Boris)
	m_maincpu->set_input_line(INPUT_LINE_RESET, state ? ASSERT_LINE : CLEAR_LINE);

	if (state)
	{
		m_via->reset();
		m_display->clear();
	}
}



/******************************************************************************
    I/O
******************************************************************************/

// cartridge

DEVICE_IMAGE_LOAD_MEMBER(ggm_state::cartridge)
{
	u32 size = m_cart->common_get_size("rom");
	m_cart_mask = ((1 << (31 - count_leading_zeros(size))) - 1) & 0xffff;

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	// keypad overlay
	std::string overlay(image.get_feature("overlay"));
	m_overlay = std::stoul(overlay, nullptr, 0) & 0xf;

	// extra ram (optional)
	if (image.get_feature("ram"))
		m_maincpu->space(AS_PROGRAM).install_ram(0x0800, 0x0fff, nullptr);

	return image_init_result::PASS;
}

READ8_MEMBER(ggm_state::cartridge_r)
{
	return m_cart->read_rom(offset & m_cart_mask);
}


// 6522 ports

void ggm_state::update_display()
{
	u16 data = bitswap<16>(m_digit_data,15,7,2,11,10,3,1,9,6,14,12,5,0,4,13,8);
	m_display->matrix(m_inp_mux, data);
}

WRITE_LINE_MEMBER(ggm_state::shift_clock_w)
{
	// shift display segment data on rising edge
	if (state && !m_shift_clock)
	{
		m_digit_data = m_digit_data << 1 | (m_shift_data & 1);
		update_display();
	}

	m_shift_clock = state;
}

WRITE_LINE_MEMBER(ggm_state::shift_data_w)
{
	m_shift_data = state;
}

WRITE8_MEMBER(ggm_state::select_w)
{
	// PA0-PA7: input mux, digit select
	m_inp_mux = data;
	update_display();
}

WRITE8_MEMBER(ggm_state::control_w)
{
	// PB0: ?

	// PB7: speaker out
	m_dac->write(BIT(data, 7));
}

READ8_MEMBER(ggm_state::input_r)
{
	u8 data = 0;

	// PB1-PB5: multiplexed inputs
	for (int i = 0; i < 4; i++)
		if (BIT(m_inp_mux, i))
			data |= m_inputs[i]->read();

	data = ~data << 1 & 0x3e;

	// PB6: hardware version
	return 0x81 | data | (m_inputs[4]->read() << 6 & 0x40);
}



/******************************************************************************
    Address Maps
******************************************************************************/

void ggm_state::main_map(address_map &map)
{
	// external slot has potential bus conflict with RAM/VIA
	map(0x0000, 0xffff).r(FUNC(ggm_state::cartridge_r));
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x8000, 0x800f).m(m_via, FUNC(via6522_device::map));
}



/******************************************************************************
    Input Ports
******************************************************************************/

#define OVERLAY(val) \
	PORT_CONDITION("IN.6", 0x0f, EQUALS, val)

static INPUT_PORTS_START( overlay_boris ) // actually most of the Chess games have a similar overlay
	PORT_MODIFY("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x01) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x01) PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("A.1 / Pawn")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x01) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("B.2 / Knight")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x01) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("C.3 / Bishop")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x01) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("D.4 / Rook")

	PORT_MODIFY("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x01) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("E.5 / Queen")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x01) PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("F.6 / King")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x01) PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("G.7")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x01) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("H.8")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x01) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")

	PORT_MODIFY("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x01) PORT_CODE(KEYCODE_SPACE) PORT_CODE(KEYCODE_MINUS) PORT_NAME("-")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x01) PORT_CODE(KEYCODE_W) PORT_NAME("B/W")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x01) PORT_CODE(KEYCODE_K) PORT_NAME("Rank")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x01) PORT_CODE(KEYCODE_T) PORT_NAME("Time")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x01) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CE")

	PORT_MODIFY("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x01) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x01) PORT_CODE(KEYCODE_I) PORT_NAME("Halt / Hint")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x01) PORT_CODE(KEYCODE_S) PORT_NAME("Best")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x01) PORT_CODE(KEYCODE_R) PORT_NAME("Restore")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x01) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter")
INPUT_PORTS_END

static INPUT_PORTS_START( overlay_morphy ) // only changed "9" to "Audio"
	PORT_MODIFY("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x02) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x02) PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("A.1 / Pawn")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x02) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("B.2 / Knight")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x02) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("C.3 / Bishop")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x02) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("D.4 / Rook")

	PORT_MODIFY("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x02) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("E.5 / Queen")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x02) PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("F.6 / King")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x02) PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("G.7")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x02) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("H.8")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x02) PORT_CODE(KEYCODE_U) PORT_NAME("Audio")

	PORT_MODIFY("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x02) PORT_CODE(KEYCODE_SPACE) PORT_CODE(KEYCODE_MINUS) PORT_NAME("-")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x02) PORT_CODE(KEYCODE_W) PORT_NAME("B/W")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x02) PORT_CODE(KEYCODE_K) PORT_NAME("Rank")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x02) PORT_CODE(KEYCODE_T) PORT_NAME("Time")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x02) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CE")

	PORT_MODIFY("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x02) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x02) PORT_CODE(KEYCODE_I) PORT_NAME("Halt / Hint")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x02) PORT_CODE(KEYCODE_S) PORT_NAME("Best")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x02) PORT_CODE(KEYCODE_R) PORT_NAME("Restore")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x02) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter")
INPUT_PORTS_END

static INPUT_PORTS_START( overlay_steinitz )
	PORT_MODIFY("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x03) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Display / 0")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x03) PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("A.1 / Pawn")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x03) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("B.2 / Knight")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x03) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("C.3 / Bishop")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x03) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("D.4 / Rook")

	PORT_MODIFY("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x03) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("E.5 / Queen")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x03) PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("F.6 / King")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x03) PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("G.7")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x03) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("H.8")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x03) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CODE(KEYCODE_U) PORT_NAME("Audio / 9")

	PORT_MODIFY("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x03) PORT_CODE(KEYCODE_V) PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Review / Left / Right")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x03) PORT_CODE(KEYCODE_W) PORT_NAME("B/W")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x03) PORT_CODE(KEYCODE_K) PORT_NAME("Rank")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x03) PORT_CODE(KEYCODE_T) PORT_NAME("Time / Change")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x03) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CE")

	PORT_MODIFY("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x03) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x03) PORT_CODE(KEYCODE_I) PORT_NAME("Halt / Hint / Look")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x03) PORT_CODE(KEYCODE_S) PORT_NAME("Best / Score")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x03) PORT_CODE(KEYCODE_R) PORT_NAME("Restore / Timing")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x03) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter")
INPUT_PORTS_END

static INPUT_PORTS_START( ggm )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x00) PORT_CODE(KEYCODE_X) PORT_NAME("Keypad 4-2")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x00) PORT_CODE(KEYCODE_S) PORT_NAME("Keypad 3-2")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x00) PORT_CODE(KEYCODE_D) PORT_NAME("Keypad 3-3")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x00) PORT_CODE(KEYCODE_F) PORT_NAME("Keypad 3-4")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x00) PORT_CODE(KEYCODE_W) PORT_NAME("Keypad 2-2")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x00) PORT_CODE(KEYCODE_E) PORT_NAME("Keypad 2-3")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x00) PORT_CODE(KEYCODE_R) PORT_NAME("Keypad 2-4")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x00) PORT_CODE(KEYCODE_2) PORT_NAME("Keypad 1-2")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x00) PORT_CODE(KEYCODE_3) PORT_NAME("Keypad 1-3")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x00) PORT_CODE(KEYCODE_4) PORT_NAME("Keypad 1-4")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x00) PORT_CODE(KEYCODE_C) PORT_NAME("Keypad 4-3")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x00) PORT_CODE(KEYCODE_V) PORT_NAME("Keypad 4-4")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x00) PORT_CODE(KEYCODE_G) PORT_NAME("Keypad 3-5")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x00) PORT_CODE(KEYCODE_T) PORT_NAME("Keypad 2-5")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x00) PORT_CODE(KEYCODE_5) PORT_NAME("Keypad 1-5")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x00) PORT_CODE(KEYCODE_Z) PORT_NAME("Keypad 4-1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x00) PORT_CODE(KEYCODE_Q) PORT_NAME("Keypad 2-1")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x00) PORT_CODE(KEYCODE_A) PORT_NAME("Keypad 3-1")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x00) PORT_CODE(KEYCODE_1) PORT_NAME("Keypad 1-1")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) OVERLAY(0x00) PORT_CODE(KEYCODE_B) PORT_NAME("Keypad 4-5")

	PORT_INCLUDE( overlay_boris )
	PORT_INCLUDE( overlay_morphy )
	PORT_INCLUDE( overlay_steinitz )

	PORT_START("IN.4")
	PORT_CONFNAME( 0x01, 0x00, "Version" ) // factory-set
	PORT_CONFSETTING(    0x00, "GGS (Applied Concepts)" )
	PORT_CONFSETTING(    0x01, "MGS (Chafitz)" )
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_F1) PORT_TOGGLE PORT_CHANGED_MEMBER(DEVICE_SELF, ggm_state, reset_switch, 0) PORT_NAME("Memory Switch")

	PORT_START("IN.5")
	PORT_CONFNAME( 0x0f, 0x0f, "Keypad Overlay" )
	PORT_CONFSETTING(    0x00, "None" )
	PORT_CONFSETTING(    0x0f, "Auto" ) // get param from softwarelist
	PORT_CONFSETTING(    0x01, "Boris 2.5" )
	PORT_CONFSETTING(    0x02, "Morphy" )
	PORT_CONFSETTING(    0x03, "Steinitz" )

	PORT_START("IN.6")
	PORT_BIT(0x0f, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_CUSTOM_MEMBER(ggm_state, overlay_r)
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void ggm_state::ggm(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 2000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &ggm_state::main_map);

	VIA6522(config, m_via, 2000000); // DDRA = 0xff, DDRB = 0x81
	m_via->writepa_handler().set(FUNC(ggm_state::select_w));
	m_via->writepb_handler().set(FUNC(ggm_state::control_w));
	m_via->readpb_handler().set(FUNC(ggm_state::input_r));
	m_via->irq_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);
	m_via->cb1_handler().set(FUNC(ggm_state::shift_clock_w));
	m_via->cb2_handler().set(FUNC(ggm_state::shift_data_w));
	TIMER(config, m_ca1_off).configure_generic(FUNC(ggm_state::ca1_off));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	PWM_DISPLAY(config, m_display).set_size(8, 16);
	m_display->set_segmask(0xff, 0x3fff);
	m_display->set_bri_levels(0.05);
	config.set_default_layout(layout_aci_ggm);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);

	/* cartridge */
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "ggm", "bin");
	m_cart->set_device_load(FUNC(ggm_state::cartridge), this);
	m_cart->set_must_be_loaded(true);

	SOFTWARE_LIST(config, "cart_list").set_original("ggm");
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( ggm )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	// nothing here, ROM is on cartridge
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME  PARENT CMP MACHINE INPUT CLASS      INIT        COMPANY, FULLNAME, FLAGS
CONS( 1980, ggm,  0,      0, ggm,    ggm,  ggm_state, empty_init, "Applied Concepts", "Great Game Machine", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
