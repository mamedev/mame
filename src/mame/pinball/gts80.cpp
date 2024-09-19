// license:BSD-3-Clause
// copyright-holders:Robbbert
/***********************************************************************************************************

PINBALL
Gottlieb System 80

An evolution of the original system 1, again plagued by numerous problems. The R6502 and R6532 replaced the
ancient 4-bit spider chips, but the problems of poor-quality PCBs, bad connectors and no common grounding were
still there. Like most pinball machines, the leaky battery could destroy everything, as could an unnoticed
stuck-on solenoid. If the machine was left on for a while, it could lock up, so a reset board (watchdog) was
added.

Here are the key codes to enable play: (may need to hit X to start a ball)

Game                              NUM  Start game                            End ball
-------------------------------------------------------------------------------------------------------------
Panthera                          652  1                                     X
Le Grand 8 (Panthera conversion)  ---  1                                     X
The Amazing Spider-man            653  1                                     X
Circus                            654  1                                     X
Counterforce                      656  1                                     X
Star Race                         657  1                                     X
James Bond                        658  1                                     X
Timeline                          659  1                                     X
Force II                          661  1 then PgDn and Down                  Down then PgDn
Pink Panther                      664  1 then PgDn and Down                  PgDn
Mars God of War                   666  1 them Home and \                     Home then \ (wait for flash or match)
Volcano                           667  1 then M=                             M then = (wait for flash or match)
Black Hole                        668  1 then R (wait for score to flash)    L then R (wait for flash or match)
Haunted House                     669  1                                     X
Eclipse                           671  1 then RW                             X
Critical Mass

Status:
- All machines are playable but read notes below.
- jamesb is a timed game
- If 'tones' is selected, sound might not work.


ToDO:
- Mars (all versions): no sound, audio cpu runs into the weeds
- Volcano (all versions): Unstable, certain inputs will freeze the machine (E for example)
- Grand8: Z80-based sound board

************************************************************************************************************/


#include "emu.h"
#include "genpin.h"

#include "gottlieb_a.h"

#include "machine/input_merger.h"
#include "speaker.h"

#include "gts80.lh"

namespace {

class gts80_state : public genpin_class
{
public:
	gts80_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_riot1(*this, "riot1")
		, m_riot2(*this, "riot2")
		, m_riot3(*this, "riot3")
		, m_io_dips(*this, "DSW%d", 0U)
		, m_io_keyboard(*this, "X%d", 0U)
		, m_p2_sound(*this, "p2sound")
		, m_r1_sound(*this, "r1sound")
		, m_digits(*this, "digit%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	void p0(machine_config &config);  // no sound card
	void p2(machine_config &config);  // multi-mode card
	void r1v(machine_config &config); // r1 with votrax
	void marspp(machine_config &config); // marspp has either SC-01 or SC-01-A
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
	void gts80_map(address_map &map) ATTR_COLD;

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
	optional_device<gottlieb_sound_r1_device> m_r1_sound;
	output_finder<76> m_digits;
	output_finder<57> m_io_outputs;   // 8 solenoids, 1 outhole, 48 lamps
};

void gts80_state::gts80_map(address_map &map)
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

static INPUT_PORTS_START( gts80 )
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
	PORT_DIPNAME( 0x80, 0x00, "SW 17")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x80, DEF_STR(Off))
	PORT_DIPNAME( 0x40, 0x00, "SW 18")
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
	PORT_DIPNAME( 0x20, 0x00, "SW 27")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x20, DEF_STR(Off))
	PORT_DIPNAME( 0x10, 0x00, "SW 28")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x10, DEF_STR(Off))
	PORT_DIPNAME( 0x08, 0x00, "SW 29")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x08, DEF_STR(Off))
	PORT_DIPNAME( 0x04, 0x00, "SW 30")
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
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Slam") PORT_CHANGED_MEMBER(DEVICE_SELF, gts80_state, slam_w, 0)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( gts80_state::slam_w )
{
	m_riot2->pa_bit_w<7>(newval);
}

u8 gts80_state::port1a_r()
{
	u8 data = 0xff;
	if ((m_lamprow < 4) && BIT(m_segment, 7))
		data = m_io_dips[m_lamprow]->read();

	for (u8 i = 0; i < 8; i++)
		if (!BIT(m_swrow, i))
			data &= m_io_keyboard[i]->read();

	return data ^ 0xff;  // inverted by Z14 (7400)
}

u8 gts80_state::port2a_r()
{
	return m_io_keyboard[8]->read(); // slam tilt
}

// sw strobes
void gts80_state::port1b_w(u8 data)
{
	m_swrow = data ^ 0xff;  // inverted by Z11 (7404)
}

// schematic and pinmame say '1' is indicated by m_segment !bits 4,5,6, but it is !bit 7
void gts80_state::port2a_w(u8 data)
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
void gts80_state::port2b_w(u8 data)
{
	m_segment = data;
}

// solenoids and sound
void gts80_state::port3a_w(u8 data)
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
void gts80_state::port3b_w(u8 data)
{
	m_lamprow = BIT(data, 4, 4);
	if (m_lamprow && (m_lamprow < 13))
		for (u8 i = 0; i < 4; i++)
			m_io_outputs[m_lamprow*4+i+5] = BIT(data, i);
	m_soundex = m_io_outputs[18] << 4;   // Sound16 line
	m_soundex = m_soundex ? 32 : 16;   // This gives working speech for Black Hole
}

void gts80_state::machine_start()
{
	m_digits.resolve();
	m_io_outputs.resolve();

	save_item(NAME(m_lamprow));
	save_item(NAME(m_swrow));
	save_item(NAME(m_segment));
	save_item(NAME(m_soundex));
	save_item(NAME(m_sol_state));
}

void gts80_state::machine_reset()
{
	m_lamprow = 0;
	m_swrow = 0;
	m_segment = 0;
	m_soundex = 0;
}


/* with Sound Board */
void gts80_state::p0(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, XTAL(3'579'545)/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &gts80_state::gts80_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Video */
	config.set_default_layout(layout_gts80);

	/* Devices */
	MOS6532(config, m_riot1, XTAL(3'579'545)/4);
	m_riot1->pa_rd_callback().set(FUNC(gts80_state::port1a_r)); // sw_r
	m_riot1->pb_wr_callback().set(FUNC(gts80_state::port1b_w)); // sw_w
	m_riot1->irq_wr_callback().set("irq", FUNC(input_merger_device::in_w<0>));

	MOS6532(config, m_riot2, XTAL(3'579'545)/4);
	m_riot2->pa_rd_callback().set(FUNC(gts80_state::port2a_r)); // pa7 - slam tilt
	m_riot2->pa_wr_callback().set(FUNC(gts80_state::port2a_w)); // digit select
	m_riot2->pb_wr_callback().set(FUNC(gts80_state::port2b_w)); // seg
	m_riot2->irq_wr_callback().set("irq", FUNC(input_merger_device::in_w<1>));

	MOS6532(config, m_riot3, XTAL(3'579'545)/4);
	m_riot3->pa_wr_callback().set(FUNC(gts80_state::port3a_w)); // sol, snd
	m_riot3->pb_wr_callback().set(FUNC(gts80_state::port3b_w)); // lamps
	m_riot3->irq_wr_callback().set("irq", FUNC(input_merger_device::in_w<2>));

	INPUT_MERGER_ANY_HIGH(config, "irq").output_handler().set_inputline("maincpu", m6502_device::IRQ_LINE);

	/* Sound */
	genpin_audio(config);
	SPEAKER(config, "mono").front_center();
}

void gts80_state::p2(machine_config &config)
{
	p0(config);
	GOTTLIEB_SOUND_PIN2(config, m_p2_sound).add_route(ALL_OUTPUTS, "mono", 0.75);
}

void gts80_state::r1v(machine_config &config)
{
	p0(config);
	GOTTLIEB_SOUND_SPEECH_REV1A(config, m_r1_sound).add_route(ALL_OUTPUTS, "mono", 0.75);
}

void gts80_state::marspp(machine_config &config)
{
	p0(config);
	GOTTLIEB_SOUND_SPEECH_REV1(config, m_r1_sound).add_route(ALL_OUTPUTS, "mono", 0.75);
}

/* SYSTEM-80 ALTERNATE ROMS =======================================================================

This is a list of known alternate roms. Nothing has been tested. Comments are derived from the roms.

All original games in this driver can be upgraded to 7 digits per player by replacing
ROM_LOAD("u2_80.bin", 0x2000, 0x1000, CRC(4f0bc7b1) SHA1(612cbacdca5cfa6ad23940796df3b7c385be79fe))
ROM_LOAD("u3_80.bin", 0x3000, 0x1000, CRC(1e69f9d0) SHA1(ad738cac2555830257b531e5e533b15362f624b9))
with
ROM_LOAD("u2g807dc.bin", 0x2000, 0x1000, CRC(f8a687b3) SHA1(ba7747c04a5967df760ace102e47c91d42e07a12))
ROM_LOAD("u3g807dc.bin", 0x3000, 0x1000, CRC(6e31242e) SHA1(14e371a0352a6068dec20af1f2b344e34a5b9011))


Haunted House
ROM_LOAD( "669-3.cpu",    0x1000, 0x0800, CRC(cf178411) SHA1(284e709ff3a569e84d1499a23f41adcbd8553930) )   Hack set 669_3
ROM_LOAD( "669-3a.cpu",   0x1000, 0x0800, CRC(effe6851) SHA1(fdf2fdddfebdf9c871d4395c307bf1f3ca2b2d10) )   Hack set 669_3a
ROM_LOAD( "669-3b.cpu",   0x1000, 0x0800, CRC(2bfceb85) SHA1(9635cd29d5b53a4641b69f1648c1201924edd486) )   Hack set 669_3b
ROM_LOAD( "669-4.cpu",    0x1000, 0x0800, CRC(8724cdf8) SHA1(9c8d7433ea14b8b50014deb7574c9d5043a794cc) )   Hack set 669_4 (speaking if sc01 added)
ROM_LOAD( "669-4a.cpu",   0x1000, 0x0800, CRC(7b906d4b) SHA1(b22c5c94e52190fbf514b531674887be42a1f559) )   Hack set 669_4a (speaking if sc01 added)
ROM_LOAD( "669-4b.cpu",   0x1000, 0x0800, CRC(71025a8a) SHA1(5a682a6dbff825217e000df4f824dea6ad89223b) )   Hack set 669_4b (speaking if sc01 added)


Mars, God of War
ROM_LOAD("666-2.cpu", 0x1000, 0x0800, CRC(6fb6d10b) SHA1(bd6fcebb52733e56f6fb66ce527cbbe3573b7250))   Hack


Panthera
ROM_LOAD( "652-1.bin",    0x1000, 0x0200, CRC(0213164b) SHA1(c65c4cb6fb1fbddaf7a09adfd38e984a193219b8) )    'Game Rom 1'
ROM_LOAD( "652-2.bin",    0x1200, 0x0200, CRC(bd7bc39f) SHA1(9d0ac37bb3ec8c95990fd37a962a17a95ce97aa0) )    'Game Rom 2'


Pink Panther
ROM_LOAD( "664_flipprojets.snd",    0x0000, 0x0400, CRC(48aeb325) SHA1(49dae08c635f191841188565bd89f07c4ad44c08) )


Spiderman
ROM_LOAD( "653.snd",      0x0400, 0x0400, CRC(dfa35ede) SHA1(740bf1e02098b85783374c583b88d2fefe98ede4) )
ROM_LOAD( "653-cpu3.bin", 0x1000, 0x0800, CRC(10289804) SHA1(13d743ce3f97ffc99470dd9b612836c98034e919) )
ROM_LOAD( "653cpu3a.bin", 0x1000, 0x0800, CRC(b2b1514a) SHA1(2f8179d171d411080ed5bc18a6c4c737a45a796c) )

*/

#define GTS80_BIOS \
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASEFF ) \
	ROM_LOAD("u2_80.bin", 0x2000, 0x1000, CRC(4f0bc7b1) SHA1(612cbacdca5cfa6ad23940796df3b7c385be79fe) ) \
	ROM_LOAD("u3_80.bin", 0x3000, 0x1000, CRC(1e69f9d0) SHA1(ad738cac2555830257b531e5e533b15362f624b9) )

/*-------------------------------------------------------------------
/ Black Hole #668
/-------------------------------------------------------------------*/
ROM_START(blckhole)
	GTS80_BIOS
	ROM_LOAD("668-4.cpu", 0x1000, 0x0800, CRC(01b53045) SHA1(72d73bbb09358b331696cd1cc44fc4958feffbe2))

	ROM_REGION(0x10000, "r1sound:audiocpu", 0)
	ROM_LOAD("668-s1.snd", 0x7000, 0x0800, CRC(23d5045d) SHA1(a20bf02ece97e8238d1dbe8d35ca63d82b62431e))
	ROM_LOAD("668-s2.snd", 0x7800, 0x0800, CRC(d63da498) SHA1(84dd87783f47fbf64b1830284c168501f9b455e2))
ROM_END

ROM_START(blckhole2)
	GTS80_BIOS
	ROM_LOAD("668-2.cpu", 0x1000, 0x0800, CRC(df03ffea) SHA1(7ca8fc321f74b9193104c282c7b4b92af93694c9))

	ROM_REGION(0x10000, "r1sound:audiocpu", 0)
	ROM_LOAD("668-s1.snd", 0x7000, 0x0800, CRC(23d5045d) SHA1(a20bf02ece97e8238d1dbe8d35ca63d82b62431e))
	ROM_LOAD("668-s2.snd", 0x7800, 0x0800, CRC(d63da498) SHA1(84dd87783f47fbf64b1830284c168501f9b455e2))
ROM_END

ROM_START(blckhols)
	GTS80_BIOS
	ROM_LOAD("668-a2.cpu", 0x1000, 0x0800, CRC(df56f896) SHA1(1ec945a7ed8d25064476791adab2b554371dadbe))

	ROM_REGION(0x1000, "p2sound:audiocpu", 0)
	ROM_LOAD("668-a-s.snd", 0x0400, 0x0400, CRC(5175f307) SHA1(97be8f2bbc393cc45a07fa43daec4bbba2336af8))

	ROM_REGION( 0x0400, "p2sound:r6530", 0 )
	ROM_LOAD("6530sy80.bin", 0x0000, 0x0400, CRC(c8ba951d) SHA1(e4aa152b36695a0205c19a8914e4d77373f64c6c))
ROM_END

/*-------------------------------------------------------------------
/ Circus #654
/-------------------------------------------------------------------*/
ROM_START(circusp)
	GTS80_BIOS
	ROM_LOAD("654-1.cpu", 0x1000, 0x0200, CRC(0eeb2731) SHA1(087cd6400bf0775bda0264422b3f790a77852bc4))
	ROM_RELOAD(0x1400, 0x0200)
	ROM_LOAD("654-2.cpu", 0x1200, 0x0200, CRC(01e23569) SHA1(47088421254e487aa1d1e87ea911dc1634e7d9ad))
	ROM_RELOAD(0x1600, 0x0200)

	ROM_REGION(0x1000, "p2sound:audiocpu", 0)
	ROM_LOAD("654.snd", 0x0400, 0x0400, CRC(75c3ad67) SHA1(4f59c451b8659d964d5242728814c2d97f68445b))

	ROM_REGION( 0x0400, "p2sound:r6530", 0 )
	ROM_LOAD("6530sy80.bin", 0x0000, 0x0400, CRC(c8ba951d) SHA1(e4aa152b36695a0205c19a8914e4d77373f64c6c))
ROM_END

/*-------------------------------------------------------------------
/ Counterforce #656
/-------------------------------------------------------------------*/
ROM_START(cntforce)
	GTS80_BIOS
	ROM_LOAD("656-1.cpu", 0x1000, 0x0200, CRC(42baf51d) SHA1(6c7947df6e4d7ed2fd48410705018bde91db3356))
	ROM_RELOAD(0x1400, 0x0200)
	ROM_LOAD("656-2.cpu", 0x1200, 0x0200, CRC(0e185c30) SHA1(01d9fb5d335c24bed9f747d6e23f57adb6ef09a5))
	ROM_RELOAD(0x1600, 0x0200)

	ROM_REGION(0x1000, "p2sound:audiocpu", 0)
	ROM_LOAD("656.snd", 0x0400, 0x0400, CRC(0be2cbe9) SHA1(306a3e7d93733562360285de35b331b5daae7250))

	ROM_REGION( 0x0400, "p2sound:r6530", 0 )
	ROM_LOAD("6530sy80.bin", 0x0000, 0x0400, CRC(c8ba951d) SHA1(e4aa152b36695a0205c19a8914e4d77373f64c6c))
ROM_END

/*-------------------------------------------------------------------
/ Critical Mass
/-------------------------------------------------------------------*/
// Prototype, no number

/*-------------------------------------------------------------------
/ Eclipse #671
/-------------------------------------------------------------------*/
ROM_START(eclipse)
	GTS80_BIOS
	ROM_LOAD("671-a.cpu", 0x1000, 0x0800, CRC(efad7312) SHA1(fcfd5e5c7924d65ac42561994797156a80018667))

	ROM_REGION(0x1000, "p2sound:audiocpu", 0)
	ROM_LOAD("671-a-s.snd", 0x0400, 0x0400, CRC(5175f307) SHA1(97be8f2bbc393cc45a07fa43daec4bbba2336af8))

	ROM_REGION( 0x0400, "p2sound:r6530", 0 )
	ROM_LOAD("6530sy80.bin", 0x0000, 0x0400, CRC(c8ba951d) SHA1(e4aa152b36695a0205c19a8914e4d77373f64c6c))
ROM_END

/*-------------------------------------------------------------------
/ Force II #661
/-------------------------------------------------------------------*/
ROM_START(forceii)
	GTS80_BIOS
	ROM_LOAD("661-2.cpu", 0x1000, 0x0800, CRC(a4fa42a4) SHA1(c17af4f0da6d5630e43db44655bece0e26b0112a))

	ROM_REGION(0x1000, "p2sound:audiocpu", 0) // no sound
	ROM_LOAD("661.snd", 0x0400, 0x0400, CRC(650158a7) SHA1(c7a9d521d1e7de1e00e7abc3a97aaaee04f8052e))

	ROM_REGION( 0x0400, "p2sound:r6530", 0 )
	ROM_LOAD("6530sy80.bin", 0x0000, 0x0400, CRC(c8ba951d) SHA1(e4aa152b36695a0205c19a8914e4d77373f64c6c))
ROM_END

/*-------------------------------------------------------------------
/ Haunted House #669, since serial no. 5000
/-------------------------------------------------------------------*/
ROM_START(hh)
	GTS80_BIOS
	ROM_LOAD("669-2.cpu", 0x1000, 0x0800, CRC(f3085f77) SHA1(ebd43588401a735d9c941d06d67ac90183139e90))

	ROM_REGION(0x10000, "r1sound:audiocpu", 0)
	ROM_LOAD("669-s1.snd", 0x7000, 0x0800, CRC(52ec7335) SHA1(2b08dd8a89057c9c8c184d5b723ecad01572129f))
	ROM_LOAD("669-s2.snd", 0x7800, 0x0800, CRC(a3317b4b) SHA1(c3b14aa58fd4588c8b8fa3540ea6331a9ee40f1f))
ROM_END

ROM_START(hh_1)
	GTS80_BIOS
	ROM_LOAD("669-1.cpu", 0x1000, 0x0800, CRC(96e72b93) SHA1(3eb3d3e064ba2fe637bba2a93ffd07f00edfa0f2))

	ROM_REGION(0x10000, "r1sound:audiocpu", 0)
	ROM_LOAD("669-s1.snd", 0x7000, 0x0800, CRC(52ec7335) SHA1(2b08dd8a89057c9c8c184d5b723ecad01572129f))
	ROM_LOAD("669-s2.snd", 0x7800, 0x0800, CRC(a3317b4b) SHA1(c3b14aa58fd4588c8b8fa3540ea6331a9ee40f1f))
ROM_END

/*-------------------------------------------------------------------
/ James Bond #658
/-------------------------------------------------------------------*/
ROM_START(jamesb)
	GTS80_BIOS
	ROM_LOAD("658-1.cpu", 0x1000, 0x0800, CRC(b841ad7a) SHA1(3396e82351c975781cac9112bfa341a3b799f296))

	ROM_REGION(0x1000, "p2sound:audiocpu", 0)
	ROM_LOAD("658.snd", 0x0400, 0x0400, CRC(962c03df) SHA1(e8ff5d502a038531a921380b75c27ef79b6feac8))

	ROM_REGION( 0x0400, "p2sound:r6530", 0 )
	ROM_LOAD("6530sy80.bin", 0x0000, 0x0400, CRC(c8ba951d) SHA1(e4aa152b36695a0205c19a8914e4d77373f64c6c))
ROM_END

ROM_START(jamesb2)
	GTS80_BIOS
	ROM_LOAD("658-x.cpu", 0x1000, 0x0800, CRC(e7e0febf) SHA1(2c101a88b61229f30ed15d38f395bc538999d766))

	ROM_REGION(0x1000, "p2sound:audiocpu", 0)
	ROM_LOAD("658.snd", 0x0400, 0x0400, CRC(962c03df) SHA1(e8ff5d502a038531a921380b75c27ef79b6feac8))

	ROM_REGION( 0x0400, "p2sound:r6530", 0 )
	ROM_LOAD("6530sy80.bin", 0x0000, 0x0400, CRC(c8ba951d) SHA1(e4aa152b36695a0205c19a8914e4d77373f64c6c))
ROM_END

/*-------------------------------------------------------------------
/ Mars - God of War #666
/-------------------------------------------------------------------*/
ROM_START(marsp)
	GTS80_BIOS
	ROM_LOAD("666-1.cpu", 0x1000, 0x0800, CRC(bb7d476a) SHA1(22d5d7f0e52c5180f73a1ca0b3c6bd4b7d0843d6))

	ROM_REGION(0x10000, "r1sound:audiocpu", 0)
	ROM_LOAD("666-s1.snd", 0x7000, 0x0800, CRC(d33dc8a5) SHA1(8d071c392996a74c3cdc2cf5ea3be3c86553ce89))
	ROM_LOAD("666-s2.snd", 0x7800, 0x0800, CRC(e5616f3e) SHA1(a6b5ebd0b456a555db0889cd63ce79aafc64dbe5))
ROM_END

ROM_START(marspf)
	GTS80_BIOS
	ROM_LOAD("666-1.cpu", 0x1000, 0x0800, CRC(bb7d476a) SHA1(22d5d7f0e52c5180f73a1ca0b3c6bd4b7d0843d6))

	ROM_REGION(0x10000, "r1sound:audiocpu", 0)
	ROM_LOAD("f666-s1.snd", 0x7000, 0x0800, CRC(f9f782c5) SHA1(83438fcf3475bc2cb24c828036d94063c263a031))
	ROM_LOAD("f666-s2.snd", 0x7800, 0x0800, CRC(7bd64d94) SHA1(a52492820e69f2072fd1dffb5cbb48fb960e19ce))
ROM_END

ROM_START(marspp)
	GTS80_BIOS
	ROM_LOAD("666s-1.cpu", 0x1000, 0x0800, CRC(029e0bcf) SHA1(20764464ede38bee2a726fc2ae98a60375b3cb1c))

	ROM_REGION(0x10000, "r1sound:audiocpu", 0)
	ROM_LOAD("666-s1.snd", 0x7000, 0x0800, CRC(d33dc8a5) SHA1(8d071c392996a74c3cdc2cf5ea3be3c86553ce89))
	ROM_LOAD("666-s2.snd", 0x7800, 0x0800, CRC(e5616f3e) SHA1(a6b5ebd0b456a555db0889cd63ce79aafc64dbe5))
ROM_END

/*-------------------------------------------------------------------
/ Panthera #652
/-------------------------------------------------------------------*/
ROM_START(panthera)
	GTS80_BIOS
	ROM_LOAD("652.cpu", 0x1000, 0x0800, CRC(5386e5fb) SHA1(822f47951b702f9c6a1ce674baaab0a596f34413))

	ROM_REGION(0x1000, "p2sound:audiocpu", 0)
	ROM_LOAD("652.snd", 0x0400, 0x0400, CRC(4d0cf2c0) SHA1(0da5d118ffd19b1e78dfaaee3e31c43750d45c8d))

	ROM_REGION( 0x0400, "p2sound:r6530", 0 )
	ROM_LOAD("6530sy80.bin", 0x0000, 0x0400, CRC(c8ba951d) SHA1(e4aa152b36695a0205c19a8914e4d77373f64c6c))
ROM_END

ROM_START(grand8)
	GTS80_BIOS
	ROM_LOAD("652.cpu", 0x1000, 0x0800, CRC(5386e5fb) SHA1(822f47951b702f9c6a1ce674baaab0a596f34413))

	ROM_REGION(0x2000, "audiocpu", 0)
	// Z80 code (0000-048E), unknown (048F-060C), FF (060D-1FFF)
	ROM_LOAD("grand8.bin", 0, 0x2000, CRC(b7cfaaae) SHA1(60eb4f9bc7b7d11ec6d353b0ae02484cf1c0c9ee))
ROM_END

/*-------------------------------------------------------------------
/ Pink Panther #664
/-------------------------------------------------------------------*/
ROM_START(pnkpnthr)
	GTS80_BIOS
	ROM_LOAD("664-1.cpu", 0x1000, 0x0800, CRC(a0d3e69a) SHA1(590e68dc28067e61832927cd4b3eefcc066f0a92))

	ROM_REGION(0x1000, "p2sound:audiocpu", 0)
	ROM_LOAD("664.snd", 0x0400, 0x0400, CRC(18f4abfd) SHA1(9e85eb7e9b1e2fe71be828ff1b5752424ed42588))

	ROM_REGION( 0x0400, "p2sound:r6530", 0 )
	ROM_LOAD("6530sy80.bin", 0x0000, 0x0400, CRC(c8ba951d) SHA1(e4aa152b36695a0205c19a8914e4d77373f64c6c))
ROM_END

/*-------------------------------------------------------------------
/ Star Race #657
/-------------------------------------------------------------------*/
ROM_START(starrace)
	GTS80_BIOS
	ROM_LOAD("657-1.cpu", 0x1000, 0x0200, CRC(27081372) SHA1(2d9cd81ffa44c389c4895043fa1e93b899544158))
	ROM_RELOAD(0x1400, 0x0200)
	ROM_LOAD("657-2.cpu", 0x1200, 0x0200, CRC(c56e31c8) SHA1(1e129fb6309e015a16f2bdb1e389cbc85d1919a7))
	ROM_RELOAD(0x1600, 0x0200)

	ROM_REGION(0x1000, "p2sound:audiocpu", 0)
	ROM_LOAD("657.snd", 0x0400, 0x0400, CRC(3a1d3995) SHA1(6f0bdb34c4fa11d5f8ecbb98ae55bafeb5d62c9e))

	ROM_REGION( 0x0400, "p2sound:r6530", 0 )
	ROM_LOAD("6530sy80.bin", 0x0000, 0x0400, CRC(c8ba951d) SHA1(e4aa152b36695a0205c19a8914e4d77373f64c6c))
ROM_END

/*-------------------------------------------------------------------
/ The Amazing Spider-Man #653
/-------------------------------------------------------------------*/
ROM_START(spidermn)
	GTS80_BIOS
	ROM_LOAD("653-1.cpu", 0x1000, 0x0200, CRC(674ddc58) SHA1(c9be45391b8dd58a0836801807d593d4c7da9904))
	ROM_RELOAD(0x1400, 0x0200)
	ROM_LOAD("653-2.cpu", 0x1200, 0x0200, CRC(ff1ddfd7) SHA1(dd7b98e491045916153b760f36432506277a4093))
	ROM_RELOAD(0x1600, 0x0200)

	ROM_REGION(0x1000, "p2sound:audiocpu", 0)
	ROM_LOAD("653.snd", 0x0400, 0x0400, CRC(f5650c46) SHA1(2d0e50fa2f4b3d633daeaa7454630e3444453cb2))

	ROM_REGION( 0x0400, "p2sound:r6530", 0 )
	ROM_LOAD("6530sy80.bin", 0x0000, 0x0400, CRC(c8ba951d) SHA1(e4aa152b36695a0205c19a8914e4d77373f64c6c))
ROM_END

/*-------------------------------------------------------------------
/ The Incredible Hulk (#500)
/-------------------------------------------------------------------*/
// Prototype to test System 80 before production.

/*-------------------------------------------------------------------
/ Time Line #659
/-------------------------------------------------------------------*/
ROM_START(timeline)
	GTS80_BIOS
	ROM_LOAD("659.cpu", 0x1000, 0x0800, CRC(d6950e3b) SHA1(939b45a9ee4bb122fbea534ad728ec6b85120416))

	ROM_REGION(0x1000, "p2sound:audiocpu", 0)
	ROM_LOAD("659.snd", 0x0400, 0x0400, CRC(28185568) SHA1(2fd26e7e0a8f050d67159f17634df2b1fc47cbd3))

	ROM_REGION( 0x0400, "p2sound:r6530", 0 )
	ROM_LOAD("6530sy80.bin", 0x0000, 0x0400, CRC(c8ba951d) SHA1(e4aa152b36695a0205c19a8914e4d77373f64c6c))
ROM_END

/*-------------------------------------------------------------------
/ Volcano (Sound and Speech) #667
/-------------------------------------------------------------------*/
ROM_START(vlcno_ax)
	GTS80_BIOS
	ROM_LOAD("667-a-x.cpu", 0x1000, 0x0800, CRC(1f51c351) SHA1(8e1850808faab843ac324040ca665a83809cdc7b))

	ROM_REGION(0x10000, "r1sound:audiocpu", 0)
	ROM_LOAD("667-s1.snd", 0x7000, 0x0800, CRC(ba9d40b7) SHA1(3d6640b259cd8ae87b998cbf1ae2dc13a2913e4f))
	ROM_LOAD("667-s2.snd", 0x7800, 0x0800, CRC(b54bd123) SHA1(3522ccdcb28bfacff2287f5537d52f22879249ab))
ROM_END

ROM_START(vlcno_1c)
	GTS80_BIOS
	ROM_LOAD("667-1c.cpu", 0x1000, 0x0800, CRC(e364202d) SHA1(128eaa5b390e309f4cf89f3631da0341f1419ffe))

	ROM_REGION(0x1000, "p2sound:audiocpu", 0)
	ROM_LOAD("667-a-s.snd", 0x0400, 0x0400, CRC(894b4e2e) SHA1(d888f8e00b2b50cef5cc916d46e4c5e6699914a1))

	ROM_REGION( 0x0400, "p2sound:r6530", 0 )
	ROM_LOAD("6530sy80.bin", 0x0000, 0x0400, CRC(c8ba951d) SHA1(e4aa152b36695a0205c19a8914e4d77373f64c6c))
ROM_END

ROM_START(vlcno_1b)
	GTS80_BIOS
	ROM_LOAD("667-1b.cpu", 0x1000, 0x0800, CRC(a422d862) SHA1(2785388eb43c08405774a9413ffa52c1591a84f2))

	ROM_REGION(0x1000, "p2sound:audiocpu", 0)
	ROM_LOAD("667-a-s.snd", 0x0400, 0x0400, CRC(894b4e2e) SHA1(d888f8e00b2b50cef5cc916d46e4c5e6699914a1))

	ROM_REGION( 0x0400, "p2sound:r6530", 0 )
	ROM_LOAD("6530sy80.bin", 0x0000, 0x0400, CRC(c8ba951d) SHA1(e4aa152b36695a0205c19a8914e4d77373f64c6c))
ROM_END

ROM_START(vlcno_1a)
	GTS80_BIOS
	ROM_LOAD("667-1a.cpu", 0x1000, 0x0800, CRC(5931c6f7) SHA1(e104a6c3ca2175bb49199e06963e26185dd563d2))

	ROM_REGION(0x1000, "p2sound:audiocpu", 0)
	ROM_LOAD("667-a-s.snd", 0x0400, 0x0400, CRC(894b4e2e) SHA1(d888f8e00b2b50cef5cc916d46e4c5e6699914a1))

	ROM_REGION( 0x0400, "p2sound:r6530", 0 )
	ROM_LOAD("6530sy80.bin", 0x0000, 0x0400, CRC(c8ba951d) SHA1(e4aa152b36695a0205c19a8914e4d77373f64c6c))
ROM_END

/*-------------------------------------------------------------------
/ System 80 Test Fixture
/-------------------------------------------------------------------*/
ROM_START(s80tst)
	GTS80_BIOS
	ROM_LOAD("80tst.cpu", 0x1000, 0x0800, CRC(a0f9e56b) SHA1(5146745ab61fea4b3070c6cf4324a9e77a7cee36))

	ROM_REGION(0x10000, "r1sound:audiocpu", 0)
	ROM_LOAD("80tst-s1.snd", 0x7000, 0x0800, CRC(b9dbdd21) SHA1(dfe42c9e6e02f82ffd0cafe164df3211cdc2d966))
	ROM_LOAD("80tst-s2.snd", 0x7800, 0x0800, CRC(1a4b1e9d) SHA1(18e7ffbdbdaf83ab1c8daa5fa5201d9f54390758))
ROM_END

} // Anonymous namespace

GAME(1981, s80tst,    0,        r1v,    gts80, gts80_state, empty_init, ROT0, "Gottlieb",         "System 80 Test",                    MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )

GAME(1980, panthera,  0,        p2,     gts80, gts80_state, empty_init, ROT0, "Gottlieb",         "Panthera",                          MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1980, grand8,    panthera, p0,     gts80, gts80_state, empty_init, ROT0, "Christian Tabart", "Le Grand 8",                        MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1980, spidermn,  0,        p2,     gts80, gts80_state, empty_init, ROT0, "Gottlieb",         "The Amazing Spider-Man",            MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1980, circusp,   0,        p2,     gts80, gts80_state, empty_init, ROT0, "Gottlieb",         "Circus",                            MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1980, cntforce,  0,        p2,     gts80, gts80_state, empty_init, ROT0, "Gottlieb",         "Counterforce",                      MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1980, starrace,  0,        p2,     gts80, gts80_state, empty_init, ROT0, "Gottlieb",         "Star Race",                         MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1980, jamesb,    0,        p2,     gts80, gts80_state, empty_init, ROT0, "Gottlieb",         "James Bond (Timed Play)",           MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1980, jamesb2,   jamesb,   p2,     gts80, gts80_state, empty_init, ROT0, "Gottlieb",         "James Bond (3/5-Ball)",             MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1980, timeline,  0,        p2,     gts80, gts80_state, empty_init, ROT0, "Gottlieb",         "Time Line",                         MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1981, forceii,   0,        p2,     gts80, gts80_state, empty_init, ROT0, "Gottlieb",         "Force II",                          MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1981, pnkpnthr,  0,        p2,     gts80, gts80_state, empty_init, ROT0, "Gottlieb",         "Pink Panther",                      MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1981, marsp,     0,        r1v,    gts80, gts80_state, empty_init, ROT0, "Gottlieb",         "Mars - God of War",                 MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1981, marspf,    marsp,    r1v,    gts80, gts80_state, empty_init, ROT0, "Gottlieb",         "Mars - God of War (French speech)", MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1981, marspp,    marsp,    marspp, gts80, gts80_state, empty_init, ROT0, "Gottlieb",         "Mars - God of War (Prototype)",     MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1981, vlcno_ax,  0,        r1v,    gts80, gts80_state, empty_init, ROT0, "Gottlieb",         "Volcano",                           MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1981, vlcno_1c,  0,        p2,     gts80, gts80_state, empty_init, ROT0, "Gottlieb",         "Volcano (Sound Only set 1)",        MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1981, vlcno_1b,  vlcno_1c, p2,     gts80, gts80_state, empty_init, ROT0, "Gottlieb",         "Volcano (Sound Only set 2)",        MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1981, vlcno_1a,  vlcno_1c, p2,     gts80, gts80_state, empty_init, ROT0, "Gottlieb",         "Volcano (Sound Only set 3)",        MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1981, blckhole,  0,        r1v,    gts80, gts80_state, empty_init, ROT0, "Gottlieb",         "Black Hole (Rev. 4)",               MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1981, blckhole2, blckhole, r1v,    gts80, gts80_state, empty_init, ROT0, "Gottlieb",         "Black Hole (Rev. 2)",               MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1981, blckhols,  0,        p2,     gts80, gts80_state, empty_init, ROT0, "Gottlieb",         "Black Hole (Sound Only)",           MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1982, hh,        0,        r1v,    gts80, gts80_state, empty_init, ROT0, "Gottlieb",         "Haunted House (Rev. 2)",            MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1982, hh_1,      hh,       r1v,    gts80, gts80_state, empty_init, ROT0, "Gottlieb",         "Haunted House (Rev. 1)",            MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1981, eclipse,   0,        p2,     gts80, gts80_state, empty_init, ROT0, "Gottlieb",         "Eclipse",                           MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
