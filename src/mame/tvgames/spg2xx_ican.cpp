// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood

/* Fisher Price / Mattel "I Can..." systems */

#include "emu.h"
#include "spg2xx.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "softlist_dev.h"


namespace {

class icanguit_state : public spg2xx_game_state
{
public:
	icanguit_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_state(mconfig, type, tag),
		m_cart(*this, "cartslot"),
		m_cart_region(nullptr),
		m_porta_in(*this, "P1_%u", 0U),
		m_portc_in(*this, "P3_%u", 0U)
	{ }

	void icanguit(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load_icanguit);

	uint16_t porta_r();
	virtual uint16_t portb_r();
	uint16_t portc_r();

	virtual void porta_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	virtual void portb_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	virtual void portc_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

	required_device<generic_slot_device> m_cart;
	memory_region *m_cart_region;

	uint16_t m_inlatch_a = 0;
	uint16_t m_inlatch_c = 0;
	optional_ioport_array<6> m_porta_in;
	optional_ioport_array<6> m_portc_in;

};

class icanpian_state : public icanguit_state
{
public:
	icanpian_state(const machine_config &mconfig, device_type type, const char *tag) :
		icanguit_state(mconfig, type, tag)
	//  m_eeprom(*this, "eeprom")
	{ }

	void icanpian(machine_config &config);

protected:
	virtual uint16_t portb_r() override;

	virtual void porta_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	virtual void portb_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

	//optional_device<eeprom_serial_93cxx_device> m_eeprom;
};


// there is a speaker volume for the 'guitar' mode, but it's presumably an analog feature, not read by the game.
static INPUT_PORTS_START( icanguit )
	PORT_START("P1")
	// uses multiplexed ports instead, see below

	PORT_START("P1_0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_NAME("Fret 1, Row 1")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_NAME("Fret 2, Row 1")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2) PORT_NAME("Fret 3, Row 1")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_NAME("Fret 4, Row 1")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_NAME("Fret 5, Row 1")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_NAME("Fret 6, Row 1") // Frets 6-12 only have 2 rows (1 and 6)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_NAME("Fret 9, Row 1")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_NAME("Fret 10, Row 1")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_NAME("Fret 11, Row 1")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_NAME("Fret 12, Row 1")
	PORT_BIT( 0xfc00, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P1_1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_NAME("Fret 1, Row 2")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_NAME("Fret 2, Row 2")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_NAME("Fret 3, Row 2")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_NAME("Fret 4, Row 2")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_NAME("Fret 5, Row 2")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_NAME("Fret 7, Row 1")
	PORT_BIT( 0xffc0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P1_2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_NAME("Fret 1, Row 3")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_NAME("Fret 2, Row 3")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_NAME("Fret 3, Row 3")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_NAME("Fret 4, Row 3")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_NAME("Fret 5, Row 3")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Fret 8, Row 1")
	PORT_BIT( 0xffc0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P1_3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_NAME("Fret 1, Row 4")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_NAME("Fret 2, Row 4")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_NAME("Fret 3, Row 4")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_NAME("Fret 4, Row 4")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("Fret 5, Row 4")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("Fret 8, Row 6")
	PORT_BIT( 0xffc0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P1_4")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_NAME("Fret 1, Row 5")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_NAME("Fret 2, Row 5")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_NAME("Fret 3, Row 5")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("Fret 4, Row 5")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("Fret 5, Row 5")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Fret 7, Row 6")
	PORT_BIT( 0xffc0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P1_5")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_NAME("Fret 1, Row 6")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_NAME("Fret 2, Row 6")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_NAME("Fret 3, Row 6")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("Fret 4, Row 6")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("Fret 5, Row 6")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Fret 6, Row 6") // Frets 6-12 only have 2 rows (1 and 6)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_NAME("Fret 9, Row 6")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_NAME("Fret 10, Row 6")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_NAME("Fret 11, Row 6")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_NAME("Fret 12, Row 6")
	PORT_BIT( 0xfc00, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // might be some kind of seeprom in here?

	PORT_START("P3")
	// uses multiplexed ports instead, see below

	PORT_START("P3_0")
	PORT_BIT( 0x0007, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("String 1") // these seem to respond on release, but are definitely active high based on visual indicators
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("String 2")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("String 3")
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("String 4")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("String 5")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("String 6")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_DIPNAME( 0x0400, 0x0000, "TV or Guitar Mode" )
	PORT_DIPSETTING(      0x0000, "TV Mode" )
	PORT_DIPSETTING(      0x0400, "Guitar Mode" )
	PORT_BIT( 0xf800, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P3_1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0xfffc, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P3_2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Home")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Enter")
	PORT_BIT( 0xfffc, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P3_3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Pause")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0xfffc, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P3_4")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) // doesn't highlight during menus, but changes sound in 'Guitar Mode' and switches between levels after selecting song
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) // doesn't highlight during menus, but changes sound in 'Guitar Mode' and switches between levels after selecting song
	PORT_BIT( 0xfff8, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P3_5")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("Whammy Up")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("Whammy Down")
	PORT_BIT( 0xfff8, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

// this has an entire piano keyboard + extras
// there is a volume dial for the internal speakers when used in non-TV mode, but presumably it is not CPU visible
// there should be a metronome key, but nothing seems to have that effect, maybe due to incomplete sound emulation?
static INPUT_PORTS_START( icanpian )
	PORT_START("P1")
	// uses multiplexed ports instead, see below

	PORT_START("P1_0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2)PORT_NAME("Octave 0 F (Green)")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)         PORT_NAME("Octave 0 F# (Purple)")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)         PORT_NAME("Octave 0 G (Yellow)")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)         PORT_NAME("Octave 0 G# (Dark Blue)")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)         PORT_NAME("Octave 0 A (Flesh)")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)         PORT_NAME("Octave 0 A# (Dark Green)")
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)         PORT_NAME("Octave 0 B (Pink)")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)         PORT_NAME("Octave 0 C (White)")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)         PORT_NAME("Octave 0 C# (Black)")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)         PORT_NAME("Octave 0 D (Blue)")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)         PORT_NAME("Octave 0 D# (Red)")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)         PORT_NAME("Octave 0 E (Orange)")
	PORT_BIT( 0xf000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P1_1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)         PORT_NAME("Octave 1 F (Green)")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)         PORT_NAME("Octave 1 F# (Purple)")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)     PORT_NAME("Octave 1 G (Yellow)")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)         PORT_NAME("Octave 1 G# (Dark Blue)")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)      PORT_NAME("Octave 1 A (Flesh)")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)     PORT_NAME("Octave 1 A# (Dark Green)")
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)         PORT_NAME("Octave 1 B (Pink)")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)         PORT_NAME("Octave 1 C (White)")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)         PORT_NAME("Octave 1 C# (Black)")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)         PORT_NAME("Octave 1 D (Blue)")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)         PORT_NAME("Octave 1 D# (Red)")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)         PORT_NAME("Octave 1 E (Orange)")
	PORT_BIT( 0xf000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P1_2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)         PORT_NAME("Octave 2 F (Green)")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6)         PORT_NAME("Octave 2 F# (Purple)")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)         PORT_NAME("Octave 2 G (Yellow)")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7)         PORT_NAME("Octave 2 G# (Dark Blue)")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)         PORT_NAME("Octave 2 A (Flesh)")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8)         PORT_NAME("Octave 2 A# (Dark Green)")
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)         PORT_NAME("Octave 2 B (Pink)")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)         PORT_NAME("Octave 2 C (White)")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0)         PORT_NAME("Octave 2 C# (Black)")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)         PORT_NAME("Octave 2 D (Blue)")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)     PORT_NAME("Octave 2 D# (Red)")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("Octave 2 E (Orange)")
	PORT_BIT( 0xf000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // might be some kind of seeprom in here? (or not? only I Can Play Guitar seems to offer a 'resume', something does get accessed on startup tho? and the machine tells you 'high scores')

	PORT_START("P3")
	// uses multiplexed ports instead, see below

	PORT_START("P3_0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Change Instrument")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Cycle Hands")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Display Mode 1")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Display Mode 2")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Display Mode 3")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Display Mode 4")
	PORT_BIT( 0x07c0, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // unused?
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Power Switch") // presumably power / low battery, kils the game
	PORT_BIT( 0xf000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P3_1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD)  PORT_NAME("Tempo Up")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ASTERISK)  PORT_NAME("Tempo Default")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("Tempo Down")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7_PAD)     PORT_NAME("Pause")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8_PAD)     PORT_NAME("Metronome")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED ) // will skip intro scenes etc. like other buttons but no more physical buttons on KB to map here
	PORT_BIT( 0xffc0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P3_2") // the system ALWAYS requires a cartridge, but has 2 modes of operation depending on a switch.  The only way to use it as a normal keyboard is by flipping this switch.
	PORT_DIPNAME( 0x0001, 0x0000, "System Mode" ) // or implement this as a toggle key? (it's a slider switch)
	PORT_DIPSETTING(      0x0001, "Keyboard Mode (no TV output)" )
	PORT_DIPSETTING(      0x0000, "TV Mode" )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )                       PORT_NAME("Scroll Up")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )                     PORT_NAME("Scroll Down")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON1 )                           PORT_NAME("Enter")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON2 )                           PORT_NAME("Home")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED ) // will skip intro scenes etc. like other buttons but no more physical buttons on KB to map here
	PORT_BIT( 0xffc0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END




uint16_t icanguit_state::porta_r()
{
	logerror("%s: porta_r\n", machine().describe_context());
	return m_inlatch_a;
}


uint16_t icanguit_state::portc_r()
{
	logerror("%s: portc_r\n", machine().describe_context());
	return m_inlatch_c;
}


void icanguit_state::portc_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: portc_w (%04x)\n", machine().describe_context(), data);
}


uint16_t icanguit_state::portb_r()
{
	logerror("%s: portb_r\n", machine().describe_context());
	return m_io_p2->read();
}

void icanguit_state::portb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: portb_w (%04x)\n", machine().describe_context(), data);
}

void icanguit_state::porta_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	//logerror("%s: porta_w (%04x)\n", machine().describe_context(), data);

	if (data == 0x0000)
	{
		m_inlatch_a = m_inlatch_c = 0x0000;
	}
	else if (data == 0x0400)
	{
		m_inlatch_a = m_porta_in[5]->read();
		m_inlatch_c = m_portc_in[5]->read();
	}
	else if (data == 0x0800)
	{
		m_inlatch_a = m_porta_in[4]->read();
		m_inlatch_c = m_portc_in[4]->read();
	}
	else if (data == 0x1000)
	{
		m_inlatch_a = m_porta_in[3]->read();
		m_inlatch_c = m_portc_in[3]->read();
	}
	else if (data == 0x2000)
	{
		m_inlatch_a = m_porta_in[2]->read();
		m_inlatch_c = m_portc_in[2]->read();
	}
	else if (data == 0x4000)
	{
		m_inlatch_a = m_porta_in[1]->read();
		m_inlatch_c = m_portc_in[1]->read();
	}
	else if (data == 0x8000)
	{
		m_inlatch_a = m_porta_in[0]->read();
		m_inlatch_c = m_portc_in[0]->read();
	}
	else
	{
		logerror("%s: unknown porta_w (%04x)\n", machine().describe_context(), data);
	}
}

// icanpian differences

void icanpian_state::porta_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (data == 0x0000)
	{
		m_inlatch_a = m_inlatch_c = 0x0000;
	}
	else if (data == 0x1000)
	{
		m_inlatch_a = m_porta_in[2]->read();
		m_inlatch_c = m_portc_in[2]->read();
	}
	else if (data == 0x2000)
	{
		m_inlatch_a = m_porta_in[1]->read();
		m_inlatch_c = m_portc_in[1]->read();
	}
	else if (data == 0x4000)
	{
		m_inlatch_a = m_porta_in[0]->read();
		m_inlatch_c = m_portc_in[0]->read();
	}
	else
	{
		logerror("%s: unknown porta_w (%04x)\n", machine().describe_context(), data);
	}
}

// accesses are made for what appears to be a serial eeprom on port B, very similar to dreamlif, but beyond blanking it at the start it doesn't
// seem to ever be used, maybe it was never added to hardware, or just never used?
uint16_t icanpian_state::portb_r()
{
/*
    uint16_t ret = 0x0000;
    logerror("%s: portbxx_r\n", machine().describe_context());
    ret |= m_eeprom->do_read() ? 0xffff : 0x0000;
    return ret;
*/
	return 0x0000;
}

void icanpian_state::portb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
/*
    logerror("%s: portbxx_w (%04x)\n", machine().describe_context(), data);
    m_eeprom->di_write(BIT(data, 2));
    m_eeprom->cs_write(BIT(data, 0) ? ASSERT_LINE : CLEAR_LINE);
    m_eeprom->clk_write(BIT(data, 1) ? ASSERT_LINE : CLEAR_LINE);
*/
}

void icanguit_state::machine_start()
{
	spg2xx_game_state::machine_start();

	// if there's a cart, override the standard banking
	if (m_cart && m_cart->exists())
	{
		std::string region_tag;
		m_cart_region = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
		m_bank->configure_entries(0, (m_cart_region->bytes() + 0x7fffff) / 0x800000, m_cart_region->base(), 0x800000);
		m_bank->set_entry(0);
	}
}

void icanguit_state::machine_reset()
{
	spg2xx_game_state::machine_reset();
	m_inlatch_a = 0x0000;
	m_inlatch_c = 0x0000;
}


DEVICE_IMAGE_LOAD_MEMBER(icanguit_state::cart_load_icanguit)
{
	uint32_t const size = m_cart->common_get_size("rom");

	if (size < 0x80'0000)
		return std::make_pair(image_error::INVALIDLENGTH, "Unsupported cartridge size (must be at least 8M)");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}



void icanguit_state::icanguit(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &icanguit_state::mem_map_4m);

	spg2xx_base(config);

	m_maincpu->porta_in().set(FUNC(icanguit_state::porta_r));
	m_maincpu->porta_out().set(FUNC(icanguit_state::porta_w));
	m_maincpu->portb_in().set(FUNC(icanguit_state::portb_r));
	m_maincpu->portb_out().set(FUNC(icanguit_state::portb_w));
	m_maincpu->portc_in().set(FUNC(icanguit_state::portc_r));
	m_maincpu->portc_out().set(FUNC(icanguit_state::portc_w));


	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "icanguit_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(icanguit_state::cart_load_icanguit));
	m_cart->set_must_be_loaded(true);

	SOFTWARE_LIST(config, "icanguit_cart").set_original("icanguit");
}

void icanpian_state::icanpian(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &icanpian_state::mem_map_4m);

	spg2xx_base(config);

	m_maincpu->porta_in().set(FUNC(icanpian_state::porta_r));
	m_maincpu->porta_out().set(FUNC(icanpian_state::porta_w));
	m_maincpu->portb_in().set(FUNC(icanpian_state::portb_r));
	m_maincpu->portb_out().set(FUNC(icanpian_state::portb_w));
	m_maincpu->portc_in().set(FUNC(icanpian_state::portc_r));
	m_maincpu->portc_out().set(FUNC(icanpian_state::portc_w));

//  EEPROM_93C66_16BIT(config, m_eeprom); // unknown part
//  m_eeprom->erase_time(attotime::from_usec(1));
//  m_eeprom->write_time(attotime::from_usec(1));

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "icanpian_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(icanpian_state::cart_load_icanguit));
	m_cart->set_must_be_loaded(true);

	SOFTWARE_LIST(config, "icanpian_cart").set_original("icanpian");
}

ROM_START( icanguit )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	// no internal ROM, requires a cartridge
ROM_END

ROM_START( icanpian )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	// no internal ROM, requires a cartridge
ROM_END

} // anonymous namespace


// Fisher-Price games
CONS( 2007, icanguit,  0,        0, icanguit, icanguit,   icanguit_state, empty_init, "Fisher-Price", "I Can Play Guitar",     MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND ) // how is data saved?
CONS( 2006, icanpian,  0,        0, icanpian, icanpian,   icanpian_state, empty_init, "Fisher-Price", "I Can Play Piano",  MACHINE_IMPERFECT_SOUND ) // 2006 date from Manual

