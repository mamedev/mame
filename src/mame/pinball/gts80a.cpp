// license:BSD-3-Clause
// copyright-holders:Robbbert
/****************************************************************************************************************

PINBALL
Gottlieb System 80A

Same as system 80, except that the displays have 7 digits.

Caveman is missing its joystick. Need the manual. The video-pinball interface has not been written.
 If you turn on DIPS6,7,8, you can enter test mode, insert coins and start a game.
 But once in-game, no inputs work.

Note: If DIP28 is set to Novelty, then Match doesn't work.

Here are the key codes to enable play: (may need to hit X to start a ball)

Game                 NUM  Start game                                       End ball (X often works, even when not connected)
-----------------------------------------------------------------------------------------------------------------
Devil's Dare         670  1, then hold .enter hit pad-                     .enter hit pad-
Rocky                672  1                                                X
Spirit               673  1, then Y and \                                  X then \ (wait for sound)
Punk!                674  1, then - and num*                               - then num* (wait for sound)
Striker              675  1, then S and num2                               X
Krull                676  1, then jiggle X and Y until you hear a sound    X
Qbert's Quest        677  1, then X (wait for sound), then Z               X
Super Orbit          680  1                                                X
Royal Flush Deluxe   681  1                                                X
Going Nuts           682  1 then num- then num*                            num- then num*
Amazon Hunt          684  1                                                X
Rack 'Em Up          685  1 then hold num-enter, hit X                     X
Ready Aim Fire       686  1                                                X
Jacks to Open        687  1                                                X
Touchdown            688  1                                                X
Alien Star           689  1, then K and \                                  \ then K
The Games            691  1                                                X
El Dorado            692  1                                                X
Ice Fever            695  1 then unknown                                   X
Caveman            PV810  1 then unknown                                   X
**** Other Manufacturer ****
Commandos
Fast Draw
Mythology



Status:
- All games (except caveman) are playable
- Lots of issues with the sound

*****************************************************************************************************************/

#include "emu.h"
#include "genpin.h"

#include "gottlieb_a.h"

#include "cpu/i86/i86.h"
#include "machine/input_merger.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "gts80a.lh"
#include "gts80a_caveman.lh"

namespace {

class gts80a_state : public genpin_class
{
public:
	gts80a_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_riot1(*this, "riot1")
		, m_riot2(*this, "riot2")
		, m_riot3(*this, "riot3")
		, m_io_dips(*this, "DSW%d", 0U)
		, m_io_keyboard(*this, "X%d", 0U)
		, m_p2_sound(*this, "p2sound")
		, m_p3_sound(*this, "p3sound")
		, m_r1_sound(*this, "r1sound")
		, m_digits(*this, "digit%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	void p0(machine_config &config);   // base config
	void p2(machine_config &config);   // multi-mode card
	void p3(machine_config &config);   // unknown card
	void r1(machine_config &config);   // r1
	void r1v(machine_config &config);  // r1 with votrax
	DECLARE_INPUT_CHANGED_MEMBER(slam_w);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

private:
	u8 port1a_r();
	u8 port2a_r();
	void port1b_w(u8 data);
	void port2a_w(u8 data);
	void port2b_w(u8 data);
	void port3a_w(u8 data);
	void port3b_w(u8 data);
	void gts80a_map(address_map &map) ATTR_COLD;

	u8 m_segment = 0U;
	u8 m_lamprow = 0U;
	u8 m_swrow = 0U;
	u8 m_soundex = 0U;
	u8 m_sol_state[9][2]{};

	required_device<m6502_device> m_maincpu;
	required_device<mos6532_device> m_riot1;
	required_device<mos6532_device> m_riot2;
	required_device<mos6532_device> m_riot3;
	required_ioport_array<4> m_io_dips;
	required_ioport_array<9> m_io_keyboard;
	optional_device<gottlieb_sound_p2_device> m_p2_sound;
	optional_device<gottlieb_sound_p3_device> m_p3_sound;
	optional_device<gottlieb_sound_r1_device> m_r1_sound;
	output_finder<80> m_digits;
	output_finder<57> m_io_outputs;   // 8 solenoids, 1 outhole, 48 lamps
};

void gts80a_state::gts80a_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x007f).m(m_riot1, FUNC(mos6532_device::ram_map));
	map(0x0080, 0x00ff).m(m_riot2, FUNC(mos6532_device::ram_map));
	map(0x0100, 0x017f).m(m_riot3, FUNC(mos6532_device::ram_map));
	map(0x01cb, 0x01cb).lr8(NAME([] () { return 0xff; }));  // continual read
	map(0x0200, 0x021f).mirror(0x0060).m(m_riot1, FUNC(mos6532_device::io_map));
	map(0x0280, 0x029f).mirror(0x0060).m(m_riot2, FUNC(mos6532_device::io_map));
	map(0x0300, 0x031f).mirror(0x0060).m(m_riot3, FUNC(mos6532_device::io_map));
	map(0x1000, 0x17ff).rom();
	map(0x1800, 0x18ff).ram().share("nvram"); // 5101L-1 256x4
	map(0x2000, 0x2fff).rom();
	map(0x3000, 0x3fff).rom();
}

static INPUT_PORTS_START( gts80a )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x80, 0x80, "SW 01")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x80, DEF_STR(Off))
	PORT_DIPNAME( 0x40, 0x40, "SW 02")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x40, DEF_STR(Off))
	PORT_DIPNAME( 0x20, 0x20, "SW 03")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x20, DEF_STR(Off))
	PORT_DIPNAME( 0x10, 0x10, "SW 04")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x10, DEF_STR(Off))
	PORT_DIPNAME( 0x08, 0x08, "SW 05")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x08, DEF_STR(Off))
	PORT_DIPNAME( 0x04, 0x04, "SW 06")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x04, DEF_STR(Off))
	PORT_DIPNAME( 0x02, 0x02, "SW 07")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x02, DEF_STR(Off))
	PORT_DIPNAME( 0x01, 0x01, "SW 08")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x01, DEF_STR(Off))

	PORT_START("DSW1")
	PORT_DIPNAME( 0x80, 0x80, "SW 09")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x80, DEF_STR(Off))
	PORT_DIPNAME( 0x40, 0x40, "SW 10")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x40, DEF_STR(Off))
	PORT_DIPNAME( 0x20, 0x20, "SW 11")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x20, DEF_STR(Off))
	PORT_DIPNAME( 0x10, 0x10, "SW 12")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x10, DEF_STR(Off))
	PORT_DIPNAME( 0x08, 0x08, "SW 13")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x08, DEF_STR(Off))
	PORT_DIPNAME( 0x04, 0x00, "SW 14")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x04, DEF_STR(Off))
	PORT_DIPNAME( 0x02, 0x00, "SW 15")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x02, DEF_STR(Off))
	PORT_DIPNAME( 0x01, 0x00, "SW 16")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x01, DEF_STR(Off))

	PORT_START("DSW2")
	PORT_DIPNAME( 0x80, 0x80, "SW 17")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x80, DEF_STR(Off))
	PORT_DIPNAME( 0x40, 0x40, "SW 18")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x40, DEF_STR(Off))
	PORT_DIPNAME( 0x20, 0x20, "SW 19")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x20, DEF_STR(Off))
	PORT_DIPNAME( 0x10, 0x10, "SW 20")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x10, DEF_STR(Off))
	PORT_DIPNAME( 0x08, 0x08, "SW 21")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x08, DEF_STR(Off))
	PORT_DIPNAME( 0x04, 0x04, "SW 22")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x04, DEF_STR(Off))
	PORT_DIPNAME( 0x02, 0x00, "SW 23")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x02, DEF_STR(Off))
	PORT_DIPNAME( 0x01, 0x00, "SW 24")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x01, DEF_STR(Off))

	PORT_START("DSW3")
	PORT_DIPNAME( 0x80, 0x00, "SW 25")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x80, DEF_STR(Off))
	PORT_DIPNAME( 0x40, 0x00, "SW 26")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x40, DEF_STR(Off))
	PORT_DIPNAME( 0x20, 0x20, "SW 27")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x20, DEF_STR(Off))
	PORT_DIPNAME( 0x10, 0x10, "SW 28")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x10, DEF_STR(Off))
	PORT_DIPNAME( 0x08, 0x08, "SW 29")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x08, DEF_STR(Off))
	PORT_DIPNAME( 0x04, 0x04, "SW 30")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x04, DEF_STR(Off))
	PORT_DIPNAME( 0x02, 0x02, "SW 31")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x02, DEF_STR(Off))
	PORT_DIPNAME( 0x01, 0x01, "SW 32")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x01, DEF_STR(Off))

	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP00")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP01")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP02")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP03")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP04")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP05")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("INP06")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Play/Test")

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP10")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP11")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP12")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP13")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP14")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP15")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("INP16")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP20")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP21")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP22")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP23")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP24")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP25")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP26")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP30")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP31")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP32")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP33")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP34")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP35")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("INP36")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("INP40")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("INP41")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP42")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP43")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("INP44")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP45")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP46")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START )

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP50")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP51")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP52")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP53")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME("INP54")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("INP55")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("INP56")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt") // won't boot if closed

	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME("INP60")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("INP61")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGUP) PORT_NAME("INP62")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGDN) PORT_NAME("INP63")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_HOME) PORT_NAME("INP64")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_END) PORT_NAME("INP65")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_NAME("INP66")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")

	PORT_START("X7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP) PORT_NAME("INP70")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME("INP71")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("INP72")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("INP73")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F1) PORT_NAME("INP74")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F2) PORT_NAME("INP75")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F4) PORT_NAME("INP76")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X8")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Slam") PORT_CHANGED_MEMBER(DEVICE_SELF, gts80a_state, slam_w, 0)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( gts80a_state::slam_w )
{
	m_riot2->pa_bit_w<7>(newval);
}

u8 gts80a_state::port1a_r()
{
	u8 data = 0xff;
	if ((m_lamprow < 4) && BIT(m_segment, 7))
		data = m_io_dips[m_lamprow]->read();

	for (u8 i = 0; i < 8; i++)
		if (!BIT(m_swrow, i))
			data &= m_io_keyboard[i]->read();

	return data ^ 0xff;  // inverted by Z14 (7400)
}

u8 gts80a_state::port2a_r()
{
	return m_io_keyboard[8]->read(); // slam tilt
}

// sw strobes
void gts80a_state::port1b_w(u8 data)
{
	m_swrow = data ^ 0xff;  // inverted by Z11 (7404)
}

// schematic and pinmame say '1' is indicated by m_segment !bits 4,5,6, but it is !bit 7
void gts80a_state::port2a_w(u8 data)
{
	static const u8 patterns[16] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x58,0x4c,0x62,0x69,0x78,0 }; // 7448
	uint16_t seg1 = (uint16_t)patterns[m_segment & 15];
	uint16_t seg2 = bitswap<10>(seg1, 7, 7, 6, 6, 5, 4, 3, 2, 1, 0);
	switch (data & 0x70)
	{
		case 0x10: // player 1&2
			if (!BIT(m_segment, 7)) seg2 |= 0x300; // put '1' in the middle
			m_digits[data & 15] = seg2;
			break;
		case 0x20: // player 3&4
			if (!BIT(m_segment, 7)) seg2 |= 0x300; // put '1' in the middle
			m_digits[(data & 15)+20] = seg2;
			break;
		case 0x40: // credits & balls
			if (!BIT(m_segment, 7)) m_segment = 1; // turn '1' back to normal
			m_digits[(data & 15)+60] = patterns[m_segment & 15];
			if (!BIT(m_segment, 7)) seg2 |= 0x300; // put '1' in the middle
			m_digits[(data & 15)+40] = seg2;
			break;
	}
}

//d0-3 bcd data; d4-6 = centre segment; d7 = dipsw enable
void gts80a_state::port2b_w(u8 data)
{
	m_segment = data;
}

// solenoids and sound
void gts80a_state::port3a_w(u8 data)
{
	u8 i;
	data ^= 0x1f;   // Z27 inverter
	// Sound
	u8 sndcmd = data & 15;
	if (!BIT(data, 4))  // Z31
		sndcmd = 0;

	sndcmd ^= 15;  // inverted again by Z13 on the A3 board
	if (m_p2_sound)
		m_p2_sound->write(sndcmd | 0x40);
	else
	if (m_r1_sound)
		m_r1_sound->write(sndcmd | m_soundex);
	else
	if (m_p3_sound)
		m_p3_sound->write(sndcmd);

	// Solenoids group 1
	if (!BIT(data, 5))
		for (i = 0; i < 4;i++)
			m_sol_state[i][0] = (BIT(data, 0, 2) == i) ? 1 : 0;
	else
		for (i = 0; i < 4;i++)
			m_sol_state[i][0] = 0;

	// Solenoids group 2
	if (!BIT(data, 6))
		for (i = 0; i < 4;i++)
			m_sol_state[i+4][0] = (BIT(data, 2, 2) == i) ? 1 : 0;
	else
		for (i = 4; i < 8;i++)
			m_sol_state[i][0] = 0;

	// Outhole
	m_sol_state[8][0] = BIT(data, 7) ^ 1;

	// Smooth solenoids
	// Some solenoids get continuously pulsed, which is absorbed by the real thing, but
	// causes issues for us. So we need to use only the first occurrence of a particular sound.
	for (i = 0; i < 9; i++)
	{
		switch (m_sol_state[i][1])
		{
			case 0:   // was off
				if (m_sol_state[i][0] == 1) // was off, coming on
				{
					m_io_outputs[i] = 1;
					m_sol_state[i][1] = 1;  // remember
				}
				break;
			case 1:   // was on
				if (m_sol_state[i][0] == 0) // was on, going off
				{
					m_io_outputs[i] = 0;
					m_sol_state[i][1] = 0;  // remember
				}
				else
					m_io_outputs[i] = 0;  // still on from before
				break;
			default:
				m_sol_state[i][1] = 0;
				break;
		}
	}
	// Activate solenoids
	for (i = 0; i < 9; i++)
	{
		bool state = m_io_outputs[i] ? 1 : 0;
		switch (i)
		{
			case 2:
				machine().bookkeeping().coin_counter_w(0, state);
				break;
			case 3:
				machine().bookkeeping().coin_counter_w(1, state);
				break;
			case 6:
				machine().bookkeeping().coin_counter_w(2, state);
				break;
			case 7:
				if (state)
					m_samples->start(0, 6);  // knocker
				break;
			case 8:
				if (state)
					m_samples->start(0, 9);  // outhole
				break;
			default:
				break;
		}
	}
}

// Lamps
void gts80a_state::port3b_w(u8 data)
{
	m_lamprow = BIT(data, 4, 4);
	if (m_lamprow && (m_lamprow < 13))
		for (u8 i = 0; i < 4; i++)
			m_io_outputs[m_lamprow*4+i+5] = BIT(data, i);
	m_soundex = m_io_outputs[18] << 4;   // Sound16 line
	m_soundex = m_soundex ? 32 : 16;
}

void gts80a_state::machine_start()
{
	m_digits.resolve();
	m_io_outputs.resolve();

	save_item(NAME(m_lamprow));
	save_item(NAME(m_swrow));
	save_item(NAME(m_segment));
	save_item(NAME(m_soundex));
	save_item(NAME(m_sol_state));
}

void gts80a_state::machine_reset()
{
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;

	m_lamprow = 0;
	m_swrow = 0;
	m_segment = 0;
	m_soundex = 0;
}


/* with Sound Board */
void gts80a_state::p0(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, XTAL(3'579'545)/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &gts80a_state::gts80a_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Video */
	config.set_default_layout(layout_gts80a);

	/* Devices */
	MOS6532(config, m_riot1, XTAL(3'579'545)/4);
	m_riot1->pa_rd_callback().set(FUNC(gts80a_state::port1a_r)); // sw_r
	m_riot1->pb_wr_callback().set(FUNC(gts80a_state::port1b_w)); // sw_w
	m_riot1->irq_wr_callback().set("irq", FUNC(input_merger_device::in_w<0>));

	MOS6532(config, m_riot2, XTAL(3'579'545)/4);
	m_riot2->pa_rd_callback().set(FUNC(gts80a_state::port2a_r)); // pa7 - slam tilt
	m_riot2->pa_wr_callback().set(FUNC(gts80a_state::port2a_w)); // digit select
	m_riot2->pb_wr_callback().set(FUNC(gts80a_state::port2b_w)); // seg
	m_riot2->irq_wr_callback().set("irq", FUNC(input_merger_device::in_w<1>));

	MOS6532(config, m_riot3, XTAL(3'579'545)/4);
	m_riot3->pa_wr_callback().set(FUNC(gts80a_state::port3a_w)); // sol, snd
	m_riot3->pb_wr_callback().set(FUNC(gts80a_state::port3b_w)); // lamps
	m_riot3->irq_wr_callback().set("irq", FUNC(input_merger_device::in_w<2>));

	INPUT_MERGER_ANY_HIGH(config, "irq").output_handler().set_inputline("maincpu", m6502_device::IRQ_LINE); // wire-or'd

	/* Sound */
	genpin_audio(config);
	SPEAKER(config, "mono").front_center();
}

void gts80a_state::p2(machine_config &config)
{
	p0(config);
	GOTTLIEB_SOUND_PIN2(config, m_p2_sound).add_route(ALL_OUTPUTS, "mono", 0.75);
}

void gts80a_state::p3(machine_config &config)
{
	p0(config);
	GOTTLIEB_SOUND_PIN3(config, m_p3_sound).add_route(ALL_OUTPUTS, "mono", 0.75);
}

void gts80a_state::r1(machine_config &config)
{
	p0(config);
	GOTTLIEB_SOUND_REV1(config, m_r1_sound).add_route(ALL_OUTPUTS, "mono", 0.75);
}

void gts80a_state::r1v(machine_config &config)
{
	p0(config);
	GOTTLIEB_SOUND_SPEECH_REV1A(config, m_r1_sound).add_route(ALL_OUTPUTS, "mono", 0.75);
}

//******************* CAVEMAN ****************************************************************

class caveman_state : public gts80a_state
{
public:
	caveman_state(const machine_config &mconfig, device_type type, const char *tag)
		: gts80a_state(mconfig, type, tag)
		, m_videocpu(*this, "video_cpu")
		, m_vram(*this, "vram")
	{ }

	void caveman(machine_config &config);

private:
	uint32_t screen_update_caveman(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void video_io_map(address_map &map) ATTR_COLD;
	void video_map(address_map &map) ATTR_COLD;
	required_device<cpu_device> m_videocpu;
	required_shared_ptr<uint8_t> m_vram;
};

uint32_t caveman_state::screen_update_caveman(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int count = 0;
	for (int y = 0; y < 256; y++)
	{
		for (int x = 0; x < 256; x += 4)
		{
			uint8_t pix = m_vram[count];

			bitmap.pix(y, x+0) = BIT(pix, 6, 2);
			bitmap.pix(y, x+1) = BIT(pix, 4, 2);
			bitmap.pix(y, x+2) = BIT(pix, 2, 2);
			bitmap.pix(y, x+3) = BIT(pix, 0, 2);

			count++;
		}
	}

	return 0;
}


void caveman_state::video_map(address_map &map)
{
	map.global_mask(0xffff);
	map(0x0000, 0x07ff).ram();
	map(0x2000, 0x5fff).ram().share("vram");
	map(0x8000, 0xffff).rom();
}

void caveman_state::video_io_map(address_map &map)
{
//  map(0x000, 0x002).rw(FUNC(caveman_state::), FUNC(caveman_state::)); // 8259 irq controller
//  map(0x100, 0x102).rw(FUNC(caveman_state::), FUNC(caveman_state::)); // HD46505
//  map(0x200, 0x200).rw(FUNC(caveman_state::), FUNC(caveman_state::)); // 8212 in, ?? out
//  map(0x300, 0x300).rw(FUNC(caveman_state::), FUNC(caveman_state::)); // soundlatch (command?) in, ?? out

//  map(0x400, 0x400).r(FUNC(caveman_state::)); // joystick inputs
//  map(0x500, 0x506).w(FUNC(caveman_state::)); // palette

}

void caveman_state::caveman(machine_config &config)
{
	r1v(config);
	I8088(config, m_videocpu, 5000000);
	m_videocpu->set_addrmap(AS_PROGRAM, &caveman_state::video_map);
	m_videocpu->set_addrmap(AS_IO, &caveman_state::video_io_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 0, 248-1);
	screen.set_screen_update(FUNC(caveman_state::screen_update_caveman));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_entries(16);

	config.set_default_layout(layout_gts80a_caveman);
}

static INPUT_PORTS_START( caveman )
	PORT_INCLUDE(gts80a)
INPUT_PORTS_END


#define GTS80A_BIOS \
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASEFF ) \
	ROM_LOAD("u2_80a.bin", 0x2000, 0x1000, CRC(241de1d4) SHA1(9d5942704cbdec6565d6335e33e9f7e4c60a41ac) ) \
	ROM_LOAD("u3_80a.bin", 0x3000, 0x1000, CRC(2d77ccdc) SHA1(47241ccd365e8d74d5aa5b775acf6445cc95b8a8) )

/*-------------------------------------------------------------------
/ Alien Star (#689)
/-------------------------------------------------------------------*/
ROM_START(alienstr)
	GTS80A_BIOS
	ROM_LOAD("689.cpu", 0x1000, 0x0800, CRC(4262006b) SHA1(66520b66c31efd0dc654630b2d3567da799b4d89))

	ROM_REGION(0x1000, "p3sound:audiocpu", 0)
	ROM_LOAD("689-s.snd", 0x0800, 0x0800, CRC(e1e7a610) SHA1(d4eddfc970127cf3a7d086ad46cbc7b95fdc269d))
ROM_END

/*-------------------------------------------------------------------
/ Amazon Hunt (#684)
/-------------------------------------------------------------------*/
ROM_START(amazonh)
	GTS80A_BIOS
	ROM_LOAD("684-2.cpu", 0x1000, 0x0800, CRC(b0d0c4af) SHA1(e81f568983d95cecb62d34598c40c5a5e6dcb3e2))

	ROM_REGION(0x10000, "r1sound:audiocpu", 0)
	ROM_LOAD("684-s1.snd", 0x7000, 0x0800, CRC(86d239df) SHA1(f18efdc6b84d18b1cf01e79224284c5180c57d22))
	ROM_LOAD("684-s2.snd", 0x7800, 0x0800, CRC(4d8ea26c) SHA1(d76d535bf29297247f1e5abd080a52b7dfc3811b))
ROM_END

ROM_START(amazonha)
	GTS80A_BIOS
	ROM_LOAD("684-1.cpu", 0x1000, 0x0800, CRC(7fac5132) SHA1(2fbcda45935c1817b2230598921b86c6f52564c8))

	ROM_REGION(0x10000, "r1sound:audiocpu", 0)
	ROM_LOAD("684-s1.snd", 0x7000, 0x0800, CRC(86d239df) SHA1(f18efdc6b84d18b1cf01e79224284c5180c57d22))
	ROM_LOAD("684-s2.snd", 0x7800, 0x0800, CRC(4d8ea26c) SHA1(d76d535bf29297247f1e5abd080a52b7dfc3811b))
ROM_END

/*-------------------------------------------------------------------
/ Caveman (#PV-810) Pinball/Video Combo
/-------------------------------------------------------------------*/
//ROM_LOAD("pv810_2.cpu", 0x1000, 0x0800, CRC(341697b9) SHA1(c7ca7227dd655380043b083f580baf2eaaedc034) )  // Enhanced Caveman rom

ROM_START(caveman)
	GTS80A_BIOS
	ROM_LOAD("pv810-1.cpu", 0x1000, 0x0800, CRC(dd8d516c) SHA1(011d8744a7984ed4c7ceb1f57dcbd8fdb22e21fe))

	ROM_REGION(0x10000, "r1sound:audiocpu", 0)
	ROM_LOAD("pv810-s1.snd", 0x7000, 0x0800, CRC(a491664d) SHA1(45031bcbddb75b4f3a5c3b623a0f2723fb95f92f))
	ROM_LOAD("pv810-s2.snd", 0x7800, 0x0800, CRC(d8654e6e) SHA1(75d4f1f966ed5a1632536723229166b9cc7d77c7))

	ROM_REGION(0x10000, "video_cpu", 0)
	ROM_LOAD16_BYTE("v810-u8.bin", 0x8000, 0x1000, CRC(514aa152) SHA1(f61a98bbc95f202417cf97b35fe9835108200477))
	ROM_LOAD16_BYTE("v810-u7.bin", 0x8001, 0x1000, CRC(74c6533e) SHA1(8fe373c28dc4089bd9e573c69682113315236c72))
	ROM_LOAD16_BYTE("v810-u6.bin", 0xa000, 0x1000, CRC(2fd0ee95) SHA1(8374b7729b2de9e73784617ada6f9d895f54cc8d))
	ROM_LOAD16_BYTE("v810-u5.bin", 0xa001, 0x1000, CRC(2fb15da3) SHA1(ba2927bc88c1ee1b8dd682234b2616d2013c7e7c))
	ROM_LOAD16_BYTE("v810-u4.bin", 0xc000, 0x1000, CRC(2dfe8492) SHA1(a29604cda968504f95577e36c715ae97034bb5f8))
	ROM_LOAD16_BYTE("v810-u3.bin", 0xc001, 0x1000, CRC(740e9ec3) SHA1(ba4839680694bf5acff540147af4319c64c313e8))
	ROM_LOAD16_BYTE("v810-u2.bin", 0xe000, 0x1000, CRC(b793baf9) SHA1(cf1618cd0134529d057bc8245b9b366c3aae2326))
	ROM_LOAD16_BYTE("v810-u1.bin", 0xe001, 0x1000, CRC(0a283b15) SHA1(4a57ae5be36500c22b55ac17dc71968bd833298b))
ROM_END

ROM_START(cavemana)
	GTS80A_BIOS
	ROM_LOAD("pv810-1.cpu", 0x1000, 0x0800, CRC(dd8d516c) SHA1(011d8744a7984ed4c7ceb1f57dcbd8fdb22e21fe))

	ROM_REGION(0x10000, "r1sound:audiocpu", 0)
	ROM_LOAD("pv810-s1.snd", 0x7000, 0x0800, CRC(a491664d) SHA1(45031bcbddb75b4f3a5c3b623a0f2723fb95f92f))
	ROM_LOAD("pv810-s2.snd", 0x7800, 0x0800, CRC(d8654e6e) SHA1(75d4f1f966ed5a1632536723229166b9cc7d77c7))

	ROM_REGION(0x10000, "video_cpu", 0)
	ROM_LOAD16_BYTE("v810-u8.bin", 0x8000, 0x1000, CRC(514aa152) SHA1(f61a98bbc95f202417cf97b35fe9835108200477))
	ROM_LOAD16_BYTE("v810-u7.bin", 0x8001, 0x1000, CRC(74c6533e) SHA1(8fe373c28dc4089bd9e573c69682113315236c72))
	ROM_LOAD16_BYTE("v810-u6.bin", 0xa000, 0x1000, CRC(2fd0ee95) SHA1(8374b7729b2de9e73784617ada6f9d895f54cc8d))
	ROM_LOAD16_BYTE("v810-u5.bin", 0xa001, 0x1000, CRC(2fb15da3) SHA1(ba2927bc88c1ee1b8dd682234b2616d2013c7e7c))
	ROM_LOAD16_BYTE("v810-u4a.bin", 0xc000, 0x1000, CRC(3437c697) SHA1(e35822ed04eeb7f8a54a0bfdd2b63d54fa9b2263))
	ROM_LOAD16_BYTE("v810-u3a.bin", 0xc001, 0x1000, CRC(729819f6) SHA1(6f684d05d1dcdbb975d3b97cfa0b1d657e7a98a5))
	ROM_LOAD16_BYTE("v810-u2a.bin", 0xe000, 0x1000, CRC(ab6193c2) SHA1(eb898b3a3dfef15f992f7ef6f2d636a3e124ca13))
	ROM_LOAD16_BYTE("v810-u1a.bin", 0xe001, 0x1000, CRC(7c6410fb) SHA1(6606d853d4955ce18ace71814bd2ae3d25e0c046))
ROM_END

/*-------------------------------------------------------------------
/ Devil's Dare (#670)
/-------------------------------------------------------------------*/
ROM_START(dvlsdre)
	GTS80A_BIOS
	ROM_LOAD("670-1.cpu", 0x1000, 0x0800, CRC(6318bce2) SHA1(1b13a87d18693fe7986fdd79bd00a80d877940c3))

	ROM_REGION(0x10000, "r1sound:audiocpu", 0)
	ROM_LOAD("670-s1.snd", 0x7000, 0x0800, CRC(506bc22a) SHA1(3c69f8d0c38c51796c31fb38c02d00afe8a4b8c5))
	ROM_LOAD("670-s2.snd", 0x7800, 0x0800, CRC(f662ee4b) SHA1(0f63e01672b7c07a4913e150f0bbe07ecfc06e7c))
ROM_END

ROM_START(dvlsdre2)
	GTS80A_BIOS
	ROM_LOAD("670-a.cpu", 0x1000, 0x0800, CRC(353b2e18) SHA1(270365ea8276b64e38939f0bf88ddb955d59cd4d))

	ROM_REGION(0x1000, "p2sound:audiocpu", 0)
	ROM_LOAD("670-a-s.snd", 0x0400, 0x0400, CRC(f141d535) SHA1(91e4ab9ce63b5ff3e395b6447a104286327b5533))

	ROM_REGION( 0x0400, "p2sound:r6530", 0 )
	ROM_LOAD("6530sy80.bin", 0x0000, 0x0400, CRC(c8ba951d) SHA1(e4aa152b36695a0205c19a8914e4d77373f64c6c))
ROM_END

/*-------------------------------------------------------------------
/ El Dorado City of Gold (#692)
/-------------------------------------------------------------------*/
ROM_START(eldorado)
	GTS80A_BIOS
	ROM_LOAD("692-2.cpu", 0x1000, 0x0800, CRC(4ee6d09b) SHA1(5da0556204e76029380366f9fbb5662715cc3257))

	ROM_REGION(0x1000, "p3sound:audiocpu", 0)
	ROM_LOAD("692-s.snd", 0x0800, 0x0800, CRC(9bfbf400) SHA1(58aed9c0b1f52bcd0b53edcdf7af576bb175e3d6))
ROM_END

/*-------------------------------------------------------------------
/ Goin' Nuts (#682)
/-------------------------------------------------------------------*/
ROM_START(goinnuts)
	GTS80A_BIOS
	ROM_LOAD("682.cpu", 0x1000, 0x0800, CRC(51c7c6de) SHA1(31accbc8d29038679f2b0396202490233657e538))

	ROM_REGION(0x10000, "r1sound:audiocpu", 0)
	ROM_LOAD("682-s1.snd", 0x7000, 0x0800, CRC(f00dabf3) SHA1(a6e3078220ab23dc41fd48fd528e679aefec3693))
	ROM_LOAD("682-s2.snd", 0x7800, 0x0800, CRC(3be8ac5f) SHA1(0112d3417c0793e672733eff58058d8c9ad10421))
ROM_END

/*-------------------------------------------------------------------
/ Ice Fever (#695)
/-------------------------------------------------------------------*/
ROM_START(icefever)
	GTS80A_BIOS
	ROM_LOAD("695.cpu", 0x1000, 0x0800, CRC(2f6e9caf) SHA1(4f9eeafcbaf758ee6bbad74611b4912ff75b8576))

	ROM_REGION(0x1000, "p3sound:audiocpu", 0)
	ROM_LOAD("695-s.snd", 0x0800, 0x0800, CRC(daededc2) SHA1(b43303c1e39b21f3fcbc339d440ea051ced1ea26))
ROM_END

/*-------------------------------------------------------------------
/ Jacks To Open (#687)
/-------------------------------------------------------------------*/
ROM_START(jack2opn)
	GTS80A_BIOS
	ROM_LOAD("687.cpu", 0x1000, 0x0800, CRC(0080565e) SHA1(c08412ba24d2ffccf11431e80bd2fc95fc4ce02b))

	ROM_REGION(0x1000, "p3sound:audiocpu", 0)
	ROM_LOAD("687-s.snd", 0x0800, 0x0800, CRC(f9d10b7a) SHA1(db255711ed6cb46d183c0ae3894df447f3d8a8e3))
ROM_END

/*-------------------------------------------------------------------
/ Krull (#676)
/-------------------------------------------------------------------*/
ROM_START(krullp)
	GTS80A_BIOS
	ROM_LOAD("676-3.cpu", 0x1000, 0x0800, CRC(71507430) SHA1(cbd7dd186ec928829585d3166ec10956d708d850))

	ROM_REGION(0x10000, "r1sound:audiocpu", 0)
	ROM_LOAD("676-s1.snd", 0x7000, 0x0800, CRC(b1989d8f) SHA1(f1a7eac8aa9c7685f4d37f1c73bba27f4fa8b6ae))
	ROM_LOAD("676-s2.snd", 0x7800, 0x0800, CRC(05fade11) SHA1(538f6225235b5338504597acdf6bafd1de24284e))
ROM_END

/*-------------------------------------------------------------------
/ Punk! (#674)
/-------------------------------------------------------------------*/
ROM_START(punk)
	GTS80A_BIOS
	ROM_LOAD("674.cpu", 0x1000, 0x0800, CRC(70cccc57) SHA1(c2446ecf072174ce3e8524c1a01b1eea72875226))

	ROM_REGION(0x10000, "r1sound:audiocpu", 0)
	ROM_LOAD("674-s1.snd", 0x7000, 0x0800, CRC(b75f79d5) SHA1(921774dacccb025c9465ea7e24534aca2d29d6f1))
	ROM_LOAD("674-s2.snd", 0x7800, 0x0800, CRC(005d123a) SHA1(ebe258786de09488ec0a104a47e208c66b3613b5))
ROM_END

/*-------------------------------------------------------------------
/ Q*Bert's Quest (#677)
/-------------------------------------------------------------------*/
ROM_START(qbquest)
	GTS80A_BIOS
	ROM_LOAD("677.cpu", 0x1000, 0x0800, CRC(fd885874) SHA1(d4414949eca45fd063c4f31079e9fa095044ab9c))

	ROM_REGION(0x10000, "r1sound:audiocpu", 0)
	ROM_LOAD("677-s1.snd", 0x7000, 0x0800, CRC(af7bc8b7) SHA1(33100d63629be7a5b768efd82a1ed1280c845d25))
	ROM_LOAD("677-s2.snd", 0x7800, 0x0800, CRC(820aa26f) SHA1(7181ceedcf61204277d7b9fdba621915960999ad))
ROM_END

/*-------------------------------------------------------------------
/ Rack 'Em Up (#685)
/-------------------------------------------------------------------*/
ROM_START(rackempp)
	GTS80A_BIOS
	ROM_LOAD("685.cpu", 0x1000, 0x0800, CRC(4754d68d) SHA1(2af743287c1a021f3e130d3d6e191ec9724d640c))

	ROM_REGION(0x1000, "p3sound:audiocpu", 0)
	ROM_LOAD("685-s.snd", 0x0800, 0x0800, CRC(d4219987) SHA1(7385d8723bdc937e7c9d6bf7f26ca06f64a9a212))
ROM_END

/*-------------------------------------------------------------------
/ Ready...Aim...Fire! (#686)
/-------------------------------------------------------------------*/
ROM_START(raimfire)
	GTS80A_BIOS
	ROM_LOAD("686.cpu", 0x1000, 0x0800, CRC(d1e7a0de) SHA1(b9af2fcaadc55d37c7d9d22621c3817eb751de6b))

	ROM_REGION(0x1000, "p3sound:audiocpu", 0)
	ROM_LOAD("686-s.snd", 0x0800, 0x0800, CRC(09740682) SHA1(4f36d78207bd5b8e7abb7118f03acbb3885173c2))
ROM_END

/*-------------------------------------------------------------------
/ Rocky (#672)
/-------------------------------------------------------------------*/
ROM_START(rocky)
	GTS80A_BIOS
	ROM_LOAD("672-2x.cpu", 0x1000, 0x0800, CRC(8e2f0d39) SHA1(eb0982d2bfa910b3c95d6d55c04dc58395789411))

	ROM_REGION(0x10000, "r1sound:audiocpu", 0)
	ROM_LOAD("672-s1.snd", 0x7000, 0x0800, CRC(10ba523c) SHA1(4289acd1437d7bf69fb442884a98290dc1b5f493))
	ROM_LOAD("672-s2.snd", 0x7800, 0x0800, CRC(5e77117a) SHA1(7836b1ee0b2afe621ae414d5710111b550db0e63))
ROM_END

ROM_START(rockyf)
	GTS80A_BIOS
	ROM_LOAD("672-2x.cpu", 0x1000, 0x0800, CRC(8e2f0d39) SHA1(eb0982d2bfa910b3c95d6d55c04dc58395789411))

	ROM_REGION(0x10000, "r1sound:audiocpu", 0)
	ROM_LOAD("f672-s1.snd", 0x7000, 0x0800, CRC(57a0ce22) SHA1(cdc167b5eb72e8c3235d3ffd9143faf8e6c0a2ef))
	ROM_LOAD("f672-s2.snd", 0x7800, 0x0800, CRC(87a0474f) SHA1(62fe995f3bc7fe23422d75b043d508c2f84f745a))
ROM_END

/*-------------------------------------------------------------------
/ Royal Flush Deluxe (#681)
/-------------------------------------------------------------------*/
ROM_START(rflshdlx)
	GTS80A_BIOS
	ROM_LOAD("681-2.cpu", 0x1000, 0x0800, CRC(0b048658) SHA1(c68ce525cbb44194090df17401b220d6a070eccb))

	ROM_REGION(0x10000, "r1sound:audiocpu", 0)
	ROM_LOAD("681-s1.snd", 0x7000, 0x0800, CRC(33455bbd) SHA1(04db645060d93d7d9faff56ead9fa29a9c4723ec))
	ROM_LOAD("681-s2.snd", 0x7800, 0x0800, CRC(639c93f9) SHA1(1623fea6681a009e7a755357fa85206cf2ce6897))
ROM_END

/*-------------------------------------------------------------------
/ Spirit (#673)
/-------------------------------------------------------------------*/
ROM_START(spirit)
	GTS80A_BIOS
	ROM_LOAD("673-2.cpu", 0x1000, 0x0800, CRC(a7dc2207) SHA1(9098e740639af364a12857f89bdc4e2c7c89ff23))

	ROM_REGION(0x10000, "r1sound:audiocpu", 0)
	ROM_LOAD("673-s1.snd", 0x7000, 0x0800, CRC(fd3062ae) SHA1(6eae04ec470afd4363ca448ee106e3e89fbf471e))
	ROM_LOAD("673-s2.snd", 0x7800, 0x0800, CRC(7cf923f1) SHA1(2182324c30e8cb22735e59b74d4f6b268d3750e6))
ROM_END

/*-------------------------------------------------------------------
/ Striker (#675)
/-------------------------------------------------------------------*/
ROM_START(striker)
	GTS80A_BIOS
	ROM_LOAD("675.cpu", 0x1000, 0x0800, CRC(06b66ce8) SHA1(399d98753e2da5c835c629a673069e853a4ce3c3))

	ROM_REGION(0x10000, "r1sound:audiocpu", 0)
	ROM_LOAD("675-s1.snd", 0x7000, 0x0800, CRC(cc11c487) SHA1(fe880dd7dc03f368b2c7ea81059c4b176018b86e))
	ROM_LOAD("675-s2.snd", 0x7800, 0x0800, CRC(ec30a3d9) SHA1(895be373598786d618bed635fe43daae7245c8ac))
ROM_END

/*-------------------------------------------------------------------
/ Super Orbit (#680)
/-------------------------------------------------------------------*/
ROM_START(sorbit)
	GTS80A_BIOS
	ROM_LOAD("680.cpu", 0x1000, 0x0800, CRC(decf84e6) SHA1(0c6f5e1abac58aede15016b5e30db72d1a3f6c11))

	ROM_REGION(0x10000, "r1sound:audiocpu", 0)
	ROM_LOAD("680-s1.snd", 0x7000, 0x0800, CRC(fccbbbdd) SHA1(089f2b15ab1cc46550351614e18d8915b3d6a8bf))
	ROM_LOAD("680-s2.snd", 0x7800, 0x0800, CRC(d883d63d) SHA1(1777a16bc9df7e5be2643ed18754ba120c7a954b))
ROM_END


/*-------------------------------------------------------------------
/ The Games (#691)
/-------------------------------------------------------------------*/
ROM_START(thegames)
	GTS80A_BIOS
	ROM_LOAD("691.cpu", 0x1000, 0x0800, CRC(50f620ea) SHA1(2f997a637eba4eb362586d3aa8caac44acccc795))

	ROM_REGION(0x1000, "p3sound:audiocpu", 0)
	ROM_LOAD("691-s.snd", 0x0800, 0x0800, CRC(d7011a31) SHA1(edf5de6cf5ddc1eb577dd1d8dcc9201522df8315))
ROM_END

/*-------------------------------------------------------------------
/ Touchdown (#688)
/-------------------------------------------------------------------*/
ROM_START(touchdn)
	GTS80A_BIOS
	ROM_LOAD("688.cpu", 0x1000, 0x0800, CRC(e531ab3f) SHA1(695aef0dd911fee27ac2d1493a9646b5430a07d5))

	ROM_REGION(0x1000, "p3sound:audiocpu", 0)
	ROM_LOAD("688-s.snd", 0x0800, 0x0800, CRC(5e9988a6) SHA1(5f531491722d3c30cf4a7c17982813a7c548387a))
ROM_END

} // Anonymous namespace

GAME( 1981, dvlsdre,  0,       r1v, gts80a, gts80a_state, empty_init, ROT0, "Gottlieb", "Devil's Dare",                MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1981, dvlsdre2, 0,       p2,  gts80a, gts80a_state, empty_init, ROT0, "Gottlieb", "Devil's Dare (Sound Only)",   MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1982, rocky,    0,       r1v, gts80a, gts80a_state, empty_init, ROT0, "Gottlieb", "Rocky",                       MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1982, rockyf,   rocky,   r1v, gts80a, gts80a_state, empty_init, ROT0, "Gottlieb", "Rocky (French speech)",       MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1982, spirit,   0,       r1v, gts80a, gts80a_state, empty_init, ROT0, "Gottlieb", "Spirit",                      MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1982, punk,     0,       r1v, gts80a, gts80a_state, empty_init, ROT0, "Gottlieb", "Punk!",                       MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1982, striker,  0,       r1v, gts80a, gts80a_state, empty_init, ROT0, "Gottlieb", "Striker (Pinball)",           MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1983, krullp,   0,       r1v, gts80a, gts80a_state, empty_init, ROT0, "Gottlieb", "Krull (Pinball)",             MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1983, qbquest,  0,       r1v, gts80a, gts80a_state, empty_init, ROT0, "Gottlieb", "Q*Bert's Quest",              MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1983, sorbit,   0,       r1,  gts80a, gts80a_state, empty_init, ROT0, "Gottlieb", "Super Orbit",                 MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1983, rflshdlx, 0,       r1,  gts80a, gts80a_state, empty_init, ROT0, "Gottlieb", "Royal Flush Deluxe",          MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1983, goinnuts, 0,       r1,  gts80a, gts80a_state, empty_init, ROT0, "Gottlieb", "Goin' Nuts",                  MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1983, amazonh,  0,       r1,  gts80a, gts80a_state, empty_init, ROT0, "Gottlieb", "Amazon Hunt",                 MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1983, amazonha, amazonh, r1,  gts80a, gts80a_state, empty_init, ROT0, "Gottlieb", "Amazon Hunt (alternate set)", MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1983, rackempp, 0,       p3,  gts80a, gts80a_state, empty_init, ROT0, "Gottlieb", "Rack 'em Up! (Pinball)",      MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1983, raimfire, 0,       p3,  gts80a, gts80a_state, empty_init, ROT0, "Gottlieb", "Ready...Aim...Fire!",         MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1984, jack2opn, 0,       p3,  gts80a, gts80a_state, empty_init, ROT0, "Gottlieb", "Jacks to Open",               MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1984, touchdn,  0,       p3,  gts80a, gts80a_state, empty_init, ROT0, "Gottlieb", "Touchdown",                   MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1984, alienstr, 0,       p3,  gts80a, gts80a_state, empty_init, ROT0, "Gottlieb", "Alien Star",                  MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1984, thegames, 0,       p3,  gts80a, gts80a_state, empty_init, ROT0, "Gottlieb", "The Games",                   MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1984, eldorado, 0,       p3,  gts80a, gts80a_state, empty_init, ROT0, "Gottlieb", "El Dorado City of Gold",      MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1985, icefever, 0,       p3,  gts80a, gts80a_state, empty_init, ROT0, "Gottlieb", "Ice Fever",                   MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )

/* custom (+video) */
GAME( 1981, caveman,  0,       caveman, caveman, caveman_state, empty_init, ROT0, "Gottlieb", "Caveman (Pinball/Video Combo, set 1)", MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1981, cavemana, caveman, caveman, caveman, caveman_state, empty_init, ROT0, "Gottlieb", "Caveman (Pinball/Video Combo, set 2)", MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
