// license:BSD-3-Clause
// copyright-holders:Robbbert,Stephane Humbert
/********************************************************************************************************

PINBALL
Peyper/Sonic

Machines:
  Odin (Peyper)
  Odin Deluxe (Sonic)
  Solar Wars (Sonic)
  Gammatron (Sonic)
  Pole Position (Sonic)
  Star Wars (Sonic)
  Wolf Man (Peyper)
  Nemesis (Peyper)
  Odisea Paris-Dakar (Peyper)
  Hang-On (Sonic)
  Ator (Video Dens)
  Petaco (Juegos Populares)(Game #1101)
  Sir Lancelot (Peyper) has its own driver.

Status:
- odin, odin_dlx, solarwap, poleposn, wolfman, nemesisp, odisea, sonstwar, sonstwr2, hangonp: Playable.
- When the machine is started, wait several seconds for the startup tune - then you can insert a coin.
- Most games require a ball in the outhole before starting a game (hold down X and press 1).

ToDo:
- Gammatron: unable to start a game - it's multiball, so need a key combination.
- ator: playfield inputs not working. Not sure if to use init_1 or init_2, won't know until playfield works.
- ator3bmp: rom missing
- Mechanical sounds
- Some sounds can go missing, for example odin/odin_dlx loses sounds while counting the bonuses
- Any machine with inputs "sonstwar" have mistakes in the dip switches. Therefore sonstwar, sonstwr2
   and hangonp don't show a match number at game end.
- Petaco: not working. Display, Inputs, Outputs, Dips to be done.

*********************************************************************************************************/

#include "emu.h"
#include "machine/genpin.h"

#include "cpu/z80/z80.h"
#include "machine/i8279.h"
#include "sound/ay8910.h"
#include "speaker.h"

#include "peyper.lh"

namespace {

class peyper_state : public genpin_class
{
public:
	peyper_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_io_keyboard(*this, "X%d", 0U)
		, m_leds(*this, "led_%u", 1U)
		, m_digits(*this, "dpl_%u", 0U)
		, m_io_outputs(*this, "out%d", 0U)
		{ }

	template <int Mask> DECLARE_CUSTOM_INPUT_MEMBER(wolfman_replay_hs_r);
	void init_0() { m_game = 0; }
	void init_1() { m_game = 1; }
	void init_2() { m_game = 2; }
	void init_3() { m_game = 0; m_outhole_solenoid = 11; }

	void peyper(machine_config &config);
	void petaco(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	u8 sw_r();
	void col_w(u8 data);
	void disp_w(u8 data);
	void lamp_w(offs_t offset, u8 data);
	void lamp7_w(u8 data) { for (u8 i = 0; i < 8; i++) m_io_outputs[32U+i] = BIT(data, i); }
	void sol_w(u8 data);
	void p1a_w(u8 data) { for (u8 i = 0; i < 8; i++) m_io_outputs[40U+i] = BIT(data, i); }
	void p1b_w(u8 data) { for (u8 i = 0; i < 8; i++) m_io_outputs[48U+i] = BIT(data, i); }
	void p2a_w(u8 data) { for (u8 i = 0; i < 8; i++) m_io_outputs[56U+i] = BIT(data, i); }
	void p2b_w(u8 data) { for (u8 i = 0; i < 8; i++) m_io_outputs[64U+i] = BIT(data, i); }

	void io_map(address_map &map);
	void mem_map(address_map &map);
	void petaco_map(address_map &map);
	void petaco_io_map(address_map &map);

	void petaco_sol0(u8 data) {}
	void petaco_sol1(u8 data) {}
	void ay1_a_w(u8 data) {}
	void ay1_b_w(u8 data) {}
	void ay2_b_w(u8 data) {}
	u8 m_row = 0U;
	u8 m_game = 0U;
	u8 m_outhole_solenoid = 8U;
	required_device<cpu_device> m_maincpu;
	required_ioport_array<4> m_io_keyboard;
	output_finder<4> m_leds;
	output_finder<34> m_digits; // 0 used as black hole
	output_finder<72> m_io_outputs;   // 8 solenoids + 64 lamps
};

void peyper_state::col_w(u8 data)
{
	m_row = data;
}

u8 peyper_state::sw_r()
{
	if ((m_row & 7) < 4)
		return m_io_keyboard[m_row & 7]->read();

	return 0xff;
}

void peyper_state::disp_w(u8 data)
{
	static const u8 patterns[16] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x58,0x4c,0x62,0x69,0x78,0 }; // 7448
	u8 laydigits[3][36]{
		{ 25, 27, 26, 28, 23,  5, 22,  4, 21,  3, 20,  2, 19,  1, 33, 30, 34, 36, 35, 29, 17, 11, 16, 10, 15,  9, 14,  8, 13,  7, 32, 31,  6, 12, 18, 24 },   // peyper
		{ 25, 27, 26, 28, 40, 37, 23,  5, 22,  4, 21,  3, 20,  2, 19,  1,  0, 36, 35, 29, 39, 38, 17, 11, 16, 10, 15,  9, 14,  8, 13,  7,  6, 12, 18, 24 },   // odin
		{ 25, 27, 26, 28, 40, 37, 19,  1, 23,  5, 22,  4, 21,  3, 20,  2,  0, 36, 35, 29, 39, 38, 13,  7, 17, 11, 16, 10, 15,  9, 14,  8,  6, 12, 18, 24 }};  // wolfman
/*
0 -> XA0 DPL25,DPL27
1 -> XA1 DPL26,DPL28
2 -> DPL23,DPL5
3 -> DPL22,DPL4
4 -> DPL21,DPL3
5 -> DPL20,DPL2
6 -> DPL19,DPL1
7 -> DPL30,DPL33
8 ->  XB0
9 ->  XB1
10 -> DPL11,DPL17
11 -> DPL10,DPL16
12 -> DPL09,DPL15
13 -> DPL08,DPL14
14 -> DPL07,DPL13
15 -> DPL31,DPL32
*/

	u8 const p = m_row << 1;
	for (u8 i = 0; i < 2; i++, data >>= 4)
	{
		u8 const q = laydigits[m_game][p | i]; // get control code or digit
		u8 const a = data & 15; // get bcd
		u8 const hex_a = patterns[a]; // get segments

		// special codes
		switch (q)
		{
			case 34: // player indicator lights (7-digit only)
				m_leds[0] = BIT(a, 0); // PLAYER 1
				m_leds[1] = BIT(a, 1); // PLAYER 2
				m_leds[2] = BIT(a, 2); // PLAYER 3
				m_leds[3] = BIT(a, 3); // PLAYER 4
				break;

			case 35: // units digits show 0
				if (!BIT(a, 0)) m_digits[laydigits[m_game][32]] = 0x3f;
				if (!BIT(a, 1)) m_digits[laydigits[m_game][33]] = 0x3f;
				if (!BIT(a, 2)) m_digits[laydigits[m_game][34]] = 0x3f;
				if (!BIT(a, 3)) m_digits[laydigits[m_game][35]] = 0x3f;
				break;

			case 36: // game status indicators
				/*
				if (BIT(a,3)) logerror("TILT\n");
				if (BIT(a,2)) logerror("ONC\n");
				if (BIT(a,1)) logerror("GAME OVER\n");
				if (BIT(a,0)) logerror("BALL IN PLAY\n");
				*/
				break;

			case 37: // player 1 indicators (6-digit only)
			case 38: // player 2 indicators (6-digit only)
			case 39: // player 3 indicators (6-digit only)
			case 40: // player 4 indicators (6-digit only)
				m_leds[q - 37] = BIT(a, 1); // player indicator
				m_digits[q - 7] = BIT(a, 2) ? 6 : 0; // million led (we show blank or 1 in millions digit)
				// bit 3, looks like it turns on all the decimal points, reason unknown
				break;

			default: // display a digit
				m_digits[q] = hex_a;
		}
	}
}

void peyper_state::lamp_w(offs_t offset, u8 data)
{
	u8 t = 8*(BIT(offset, 2, 2)+1);  // t = 8,16,24
	for (u8 i = 0; i < 8; i++)
		m_io_outputs[t+i] = BIT(data, i);
}

void peyper_state::sol_w(u8 data)
{
	data &= 15;
	for (u8 i = 0; i < 8; i++)
		m_io_outputs[i] = (data == i);
	if (data == m_outhole_solenoid)
		m_samples->start(0, 5); // outhole
}


template <int Mask>
CUSTOM_INPUT_MEMBER(peyper_state::wolfman_replay_hs_r)
{
	switch (Mask)
	{
		case 0x03:
			return ((ioport("REPLAY")->read() & Mask) >> 0);
		case 0x40:
			return ((ioport("REPLAY")->read() & Mask) >> 6);
		default:
			logerror("wolfman_replay_hs_r : invalid %02X bit_mask\n",Mask);
			return 0;
	}
}


void peyper_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x5FFF).rom();
	map(0x6000, 0x67FF).ram().share("nvram");
}

void peyper_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x01).rw("i8279", FUNC(i8279_device::read), FUNC(i8279_device::write));
	map(0x04, 0x04).w("ay1", FUNC(ay8910_device::address_w));
	map(0x05, 0x05).r("ay1", FUNC(ay8910_device::data_r)); // only read by Ator?
	map(0x06, 0x06).w("ay1", FUNC(ay8910_device::data_w));
	map(0x08, 0x08).w("ay2", FUNC(ay8910_device::address_w));
	map(0x09, 0x09).r("ay2", FUNC(ay8910_device::data_r)); // never actually read?
	map(0x0a, 0x0a).w("ay2", FUNC(ay8910_device::data_w));
	map(0x0c, 0x0c).w(FUNC(peyper_state::sol_w));
	map(0x10, 0x18).w(FUNC(peyper_state::lamp_w));
	map(0x20, 0x20).portr("DSW0");
	map(0x24, 0x24).portr("DSW1");
	map(0x28, 0x28).portr("SYSTEM");
	map(0x2c, 0x2c).w(FUNC(peyper_state::lamp7_w));
}

void peyper_state::petaco_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x6000, 0x67ff).mirror(0x1800).ram().share("nvram");
	map(0x8000, 0x8001).rw("i8279", FUNC(i8279_device::read), FUNC(i8279_device::write));
}

void peyper_state::petaco_io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x08, 0x08).portr("DSW0");
	map(0x09, 0x09).portr("SYSTEM");
	map(0x0a, 0x0a).portr("DSW1");
	map(0x0c, 0x0c).w("ay1", FUNC(ay8910_device::address_w));
	map(0x0d, 0x0d).r("ay1", FUNC(ay8910_device::data_r));
	map(0x0e, 0x0e).w("ay1", FUNC(ay8910_device::data_w));
	map(0x10, 0x10).w("ay2", FUNC(ay8910_device::address_w));
	map(0x11, 0x11).r("ay2", FUNC(ay8910_device::data_r));
	map(0x12, 0x12).w("ay2", FUNC(ay8910_device::data_w));
	map(0x14, 0x17).w(FUNC(peyper_state::petaco_sol0));
	map(0x18, 0x1b).w(FUNC(peyper_state::petaco_sol1));
}

static INPUT_PORTS_START( pbsonic_generic )
	/* SYSTEM : port 0x28 (cpl'ed) */
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )             // N.C.
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )             // N.C.
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )            // Reset
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )             // Start game - Might also display next screen in "Test Mode"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )              // Small coin (25 pesetas)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )              // Medium coin (50 pesetas but 100 pesetas in 'sonstwar') - never mentioned in the manuals
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )              // Big coin (100 pesetas)

	/* DSW0 : port 0x20 - DSW0-1 is bit 7 ... DSW0-8 is bit 0 */
	PORT_START("DSW0")
	PORT_DIPUNKNOWN( 0x80, IP_ACTIVE_LOW )                  // game specific settings
	PORT_DIPNAME( 0x40, 0x00, "Match Feature" )             // Loteria
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )      // Reclamo
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Coinage ) )          // Partidas/Moneda
	PORT_DIPSETTING(    0x18, "A 2/1 B 1/1 C 1/3" )
	PORT_DIPSETTING(    0x00, "A 1/1 B 1/2 C 1/5" )
	PORT_DIPSETTING(    0x08, "A 1/1 B 1/3 C 1/6" )
	PORT_DIPSETTING(    0x10, "A 1/2 B 1/4 C 1/8" )
	PORT_DIPNAME( 0x04, 0x00, "Balls" )                     // Bolas/Partida
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x03, 0x00, "Replay at / High-score" )    // Puntos/Premios - high-score not mentioned in some manuals - values are game specific
	PORT_DIPSETTING(    0x00, "0k 0k and 0k / 0k" )
	PORT_DIPSETTING(    0x01, "0k 0k and 0k / 0k" )
	PORT_DIPSETTING(    0x02, "0k 0k and 0k / 0k" )
	PORT_DIPSETTING(    0x03, "0k 0k and 0k / 0k" )

	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP01")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP02")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP03")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP04")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP05")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP06")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP07")

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP10")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP11")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP12")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP13")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP14")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP15")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP16")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP17")

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP20")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP21")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP22")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP23")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP24")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP25")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP26")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP27")

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP30")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP31")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP32")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP33")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP34")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP35")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP36")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP37")
INPUT_PORTS_END


/* verified from Z80 code - NO manual found ! */
static INPUT_PORTS_START( odin_dlx )
	PORT_INCLUDE( pbsonic_generic )

	/* SYSTEM : port 0x28 (cpl'ed) -> 0x00 in 0x6014..0x601b - 0x11 when pressed (code at 0x0190) */

	/* SW0 : port 0x00 (after 0x40 written to port 0x01) -> 0x00 in 0x601c..0x6023 - 0x11 when pressed (code at 0x019d) */
	/* SW1 : port 0x00 (after 0x41 written to port 0x01) -> 0x00 in 0x6024..0x602b - 0x11 when pressed (code at 0x01af) */
	/* SW2 : port 0x00 (after 0x42 written to port 0x01) -> 0x00 in 0x602c..0x6033 - 0x11 when pressed (code at 0x01bf) */
	/* SW3 : port 0x00 (after 0x43 written to port 0x01) -> 0x00 in 0x6034..0x603b - 0x11 when pressed (code at 0x01cf) */

	/* DSW0 : port 0x20 - DSW0-1 is bit 7 ... DSW0-8 is bit 0 */
	PORT_MODIFY("DSW0")
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )
//  PORT_DIPNAME( 0x40, 0x00, "Match Feature" )             // Loteria - code at 0x0aa5 - stored at 0x609e (0x00 YES / 0x01 NO) - then code at 0x0986
//  PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )      // Reclamo - code at 0x0ab2 - stored at 0x609d (0x00 ON / 0x01 OFF)
//  PORT_DIPNAME( 0x18, 0x00, DEF_STR( Coinage ) )          // Partidas/Moneda - code at 0x0a84 - tables at 0x0b2f (4 * 3) - credits BCD stored at 0x6103
//  PORT_DIPNAME( 0x04, 0x00, "Balls" )                     // Bolas/Partida - code at 0x0a77 - stored at 0x6099
	PORT_DIPNAME( 0x03, 0x00, "Replay at / High-score" )    // Puntos/Premios - code at 0x0a34 and 0x0a57 - tables at 0x0aff (4 * 9) and 0x0b23 (4 * 3)
	PORT_DIPSETTING(    0x00, "800k 1200k and 8000k / 1400k" )
	PORT_DIPSETTING(    0x01, "900k 1300k and 8000k / 1500k" )
	PORT_DIPSETTING(    0x02, "1000k 1400k and 8000k / 1600k" )
	PORT_DIPSETTING(    0x03, "1100k 1500k and 8000k / 1700k" )

	/* DSW1 : port 0x24 - DSW1-1 is bit 7 ... DSW1-8 is bit 0 */
	PORT_START("DSW1")
	PORT_DIPNAME( 0x80, 0x00, "Speed-up ?" )                // from Visual Pinball - code at 0x0abe - stored at 0x6009 (0x02 YES / 0x04 NO)
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )                        // mostly affects music (samples ?)
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x20, 0x00, "Clear RAM on Reset" )        // Borrador RAM - code at 0x0ace - range 0x60ff..0x6121 - 0x611f = 0x55 and 0x6120 = 0xaa
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0f, 0x08, "Test Mode" )                 // Visualizacion - code at 0x1542 (similar code in 'solarwap')
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x03, "Displays Replays and Extra Balls" )    // (0x6116++) on player 1, (0x6119++) on player 3 - I can't tell ATM which is which :(
	PORT_DIPSETTING(    0x05, "Displays Coins" )                      // Coin A (0x6106++) on player 1, Coin B (0x6109++) on player 3 and coin C (0x610c++) on player 2
	PORT_DIPSETTING(    0x06, "Displays Hours and Games" )            // hours since boot (0x6110++) on player 1, games started on player 3 (0x6113++)
INPUT_PORTS_END

/* verified from Z80 code - manual found */
static INPUT_PORTS_START( solarwap )
	PORT_INCLUDE( pbsonic_generic )

	/* SYSTEM : port 0x28 (cpl'ed) -> 0x00 in 0x6065..0x606c - 0x11 when pressed (code at 0x0193) */

	/* SW0 : port 0x00 (after 0x40 written to port 0x01) -> 0x00 in 0x606d..0x6074 - 0x11 when pressed (code at 0x01a0) */
	/* SW1 : port 0x00 (after 0x41 written to port 0x01) -> 0x00 in 0x6075..0x605c - 0x11 when pressed (code at 0x01b0) */
	/* SW2 : port 0x00 (after 0x42 written to port 0x01) -> 0x00 in 0x607d..0x6084 - 0x11 when pressed (code at 0x01c0) */
	/* SW3 : port 0x00 (after 0x43 written to port 0x01) -> 0x00 in 0x6085..0x608c - 0x11 when pressed (code at 0x01d0) */

	/* DSW0 : port 0x20 - DSW0-1 is bit 7 ... DSW0-8 is bit 0 */
	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x80, 0x00, "DSW0-1 Unknown" )            // code at 0x1818 - stored at 0x6097 - manual says "unused"
	PORT_DIPSETTING(    0x80, "00" )
	PORT_DIPSETTING(    0x00, "FF" )
//  PORT_DIPNAME( 0x40, 0x00, "Match Feature" )             // Loteria - code at 0x0a27 - stored at 0x60fc (0x00 YES / 0x01 NO) - then code at 0x0937
//  PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )      // Reclamo - code at 0x0a32 - stored at 0x60fb (0x00 ON / 0x01 OFF)
//  PORT_DIPNAME( 0x18, 0x00, DEF_STR( Coinage ) )          // Partidas/Moneda - code at 0x0a06 - tables at 0x0ac2 (4 * 3) - credits BCD stored at 0x6162
//  PORT_DIPNAME( 0x04, 0x00, "Balls" )                     // Bolas/Partida - code at 0x09f9 - stored at 0x60f7
	PORT_DIPNAME( 0x03, 0x00, "Replay at / High-score" )    // Puntos/Premios - code at 0x09b6 and 0x09d9 - tables at 0x0a92 (4 * 9) and 0x0ab6 (4 * 3)
	PORT_DIPSETTING(    0x01, "1900k and 2500k / 3500k" )
	PORT_DIPSETTING(    0x00, "2200k and 2800k / 3500k" )
	PORT_DIPSETTING(    0x02, "2500k and 3100k / 4000k" )
	PORT_DIPSETTING(    0x03, "2800k and 3400k / 4500k" )

	/* DSW1 : port 0x24 - DSW1-1 is bit 7 ... DSW1-8 is bit 0 */
	PORT_START("DSW1")
	PORT_DIPNAME( 0x80, 0x00, "Extra Ball Light" )          // code at 0x0a3c - stored at 0x60ca (0x10 Flashing / 0x20 Always Lit)
	PORT_DIPSETTING(    0x00, "Flashing" )
	PORT_DIPSETTING(    0x80, "Always Lit" )
	PORT_DIPNAME( 0x40, 0x40, "Medium Left Lane" )          // code at 0x0a49 - stored at 0x60fd (when "Lit Value")
	PORT_DIPSETTING(    0x40, "Lit Value" )
	PORT_DIPSETTING(    0x00, "100k when 100k Lit" )
	PORT_DIPNAME( 0x20, 0x00, "Clear RAM on Reset" )        // Borrador RAM - code at 0x0a61 - range 0x615e..0x6180 - 0x617e = 0x55 and 0x617f = 0xaa
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x00, "Background Music" )          // code at 0x0a52 - stored at 0x6010 (0x00 ON / 0xff No Hit)
	PORT_DIPSETTING(    0x10, "When hitting nothing" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0f, 0x08, "Test Mode" )                 // Visualizacion - code at 0x0eef
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x03, "Displays Replays and Extra Balls" )    // (0x6175++) on player 1, (0x6178++) on player 3 - I can't tell ATM which is which :(
	PORT_DIPSETTING(    0x05, "Displays Coins" )                      // Coin A (0x6165++) on player 1, Coin B (0x6168++) on player 3 and coin C (0x616b++) on player 2
	PORT_DIPSETTING(    0x06, "Displays Hours and Games" )            // hours since boot (0x616f++) on player 1, games started on player 3 (0x6172++)
INPUT_PORTS_END

/* verified from Z80 code - manual found */
static INPUT_PORTS_START( poleposn )
	PORT_INCLUDE( pbsonic_generic )

	/* SYSTEM : port 0x28 (cpl'ed) -> 0x00 in 0x604b..0x6052 - 0x11 when pressed (code at 0x019c) */

	/* SW0 : port 0x00 (after 0x40 written to port 0x01) -> 0x00 in 0x6053..0x605a - 0x11 when pressed (code at 0x01a9) */
	/* SW1 : port 0x00 (after 0x41 written to port 0x01) -> 0x00 in 0x605b..0x6062 - 0x11 when pressed (code at 0x01b9) */
	/* SW2 : port 0x00 (after 0x42 written to port 0x01) -> 0x00 in 0x6063..0x606a - 0x11 when pressed (code at 0x01c9) */
	/* SW3 : port 0x00 (after 0x43 written to port 0x01) -> 0x00 in 0x606b..0x6072 - 0x11 when pressed (code at 0x01d9) */

	/* DSW0 : port 0x20 - DSW0-1 is bit 7 ... DSW0-8 is bit 0 */
	PORT_MODIFY("DSW0")
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )
//  PORT_DIPNAME( 0x40, 0x00, "Match Feature" )             // Loteria - code at 0x092f - stored at 0x60f5 (0x00 YES / 0x01 NO) - then code at 0x0859
//  PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )      // Reclamo - code at 0x093a - stored at 0x60f4 (0x00 ON / 0x01 OFF)
//  PORT_DIPNAME( 0x18, 0x00, DEF_STR( Coinage ) )          // Partidas/Moneda - code at 0x0915 - tables at 0x09ed (4 * 3) - credits BCD stored at 0x618c
//  PORT_DIPNAME( 0x04, 0x00, "Balls" )                     // Bolas/Partida - code at 0x0901 - stored at 0x60f0
	PORT_DIPNAME( 0x03, 0x00, "Replay at / High-score" )    // Puntos/Premios - code at 0x08b4 and 0x08e1 - tables at 0x09bd (4 * 9) and 0x09e1 (4 * 3)
	PORT_DIPSETTING(    0x00, "1500k 2200k and 8000k / 3000k" )
	PORT_DIPSETTING(    0x01, "1700k 2400k and 8000k / 3200k" )
	PORT_DIPSETTING(    0x02, "2000k 2600k and 8000k / 3500k" )
	PORT_DIPSETTING(    0x03, "2400k 3000k and 8000k / 3900k" )

	/* DSW1 : port 0x24 - DSW1-1 is bit 7 ... DSW1-8 is bit 0 */
	PORT_START("DSW1")
	PORT_DIPNAME( 0x80, 0x00, "Test Mode" )                 // Visualizacion - code at 0x0a4c
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x20, 0x00, "Clear RAM on Reset" )        // Borrador RAM - code at 0x098c - range 0x6188..0x61aa - 0x61a8 = 0x55 and 0x61a9 = 0xaa
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x03, 0x00, "Double Playfield" )          // Puntos/xBonus - code at 0x0944 - stored +1 at 0x60f9
	PORT_DIPSETTING(    0x00, "When Bonus x3" )
	PORT_DIPSETTING(    0x01, "When Bonus x3 and x6" )
	PORT_DIPSETTING(    0x02, "When Bonus x3, x6 and x9" )
	PORT_DIPSETTING(    0x03, "When Bonus x3, x6, x9 and x12" )
INPUT_PORTS_END

/* verified from Z80 code - manual found but with incorrect "Coinage" and "Replay at / High-score" settings (another revision floating around ?) */
static INPUT_PORTS_START( sonstwar )
	PORT_INCLUDE( pbsonic_generic )

	/* SYSTEM : port 0x28 (cpl'ed) -> 0x00 in 0x6013..0x601a - 0x11 when pressed (code at 0x0187) */

	/* SW0 : port 0x00 (after 0x40 written to port 0x01) -> 0x00 in 0x601b..0x6022 - 0x11 when pressed (code at 0x0194) */
	/* SW1 : port 0x00 (after 0x41 written to port 0x01) -> 0x00 in 0x6023..0x602a - 0x11 when pressed (code at 0x01a4) */
	/* SW2 : port 0x00 (after 0x42 written to port 0x01) -> 0x00 in 0x602b..0x6032 - 0x11 when pressed (code at 0x01b4) */
	/* SW3 : port 0x00 (after 0x43 written to port 0x01) -> 0x00 in 0x6033..0x603a - 0x11 when pressed (code at 0x01c4) */

	/* DSW0 : port 0x20 - DSW0-1 is bit 7 ... DSW0-8 is bit 0 */
	PORT_MODIFY("DSW0")
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )
//  PORT_DIPNAME( 0x40, 0x00, "Match Feature" )             // Loteria - code at 0x08c3 - stored at 0x6096 (0x00 YES / 0x01 NO) - then code at 0x07cb
//  PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )      // Reclamo - code at 0x09cf - stored at 0x6094 (0x00 ON / 0x01 OFF)
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Coinage ) )          // Partidas/Moneda - code at 0x08a2 - tables at 0x0961 (4 * 3) - credits BCD stored at 0x617f
	PORT_DIPSETTING(    0x18, "A 3/1 B 1/2 C 1/2" )                   // manual says "A 2/1 C 1/3"
	PORT_DIPSETTING(    0x08, "A 2/1 B 1/3 C 1/3" )                   // manual says "A 1/1 C 1/6"
	PORT_DIPSETTING(    0x10, "A 3/2 B 1/4 C 1/4" )                   // manual says "A 1/2 C 1/8"
	PORT_DIPSETTING(    0x00, "A 1/1 B 1/5 C 1/5" )                   // manual says "A 1/1 C 1/5"
//  PORT_DIPNAME( 0x04, 0x00, "Balls" )                     // Bolas/Partida - code at 0x0901 - stored at 0x60f0
	PORT_DIPNAME( 0x03, 0x00, "Replay at / High-score" )    // Puntos/Premios - code at 0x0867 and 0x096d - tables at 0x0931 (4 * 9) and 0x0955 (4 * 3)
	PORT_DIPSETTING(    0x00, "1400k 2200k and 9000k / 3300k" )       // manual says "1700k and 2500k / 3600k"
	PORT_DIPSETTING(    0x01, "1700k 2500k and 9000k / 3600k" )       // manual says "2000k and 2800k / 3900k"
	PORT_DIPSETTING(    0x02, "2000k 2800k and 9000k / 3900k" )       // manual says "2300k and 3100k / 4200k"
	PORT_DIPSETTING(    0x03, "2300k 3100k and 9000k / 4200k" )       // manual says "2600k and 3400k / 4500k"

	/* DSW1 : port 0x24 - DSW1-1 is bit 7 ... DSW1-8 is bit 0 */
	PORT_START("DSW1")
	PORT_DIPNAME( 0x80, 0x00, "Clear RAM on Reset" )        // Borrador RAM - code at 0x083c - range 0x6176..0x6194 - 0x6194 = 0x5a and 0x6195 = 0xa5
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x60, 0x00, "Special" )                   // code at 0x08f0 - stored +1 at 0x6098
	PORT_DIPSETTING(    0x00, "1 Attack" )
	PORT_DIPSETTING(    0x01, "2 Attacks" )
	PORT_DIPSETTING(    0x02, "3 Attacks" )
	PORT_DIPSETTING(    0x03, "4 Attacks" )
	PORT_DIPNAME( 0x18, 0x00, "Extra Ball" )                // code at 0x08da - stored +1 at 0x6097
	PORT_DIPSETTING(    0x00, "1 Star" )
	PORT_DIPSETTING(    0x01, "2 Stars" )
	PORT_DIPSETTING(    0x02, "3 Stars" )
	PORT_DIPSETTING(    0x03, "4 Stars" )
	PORT_DIPNAME( 0x04, 0x00, "Test Mode" )                 // Visualizacion - code at 0x0e31
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x02, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x01, IP_ACTIVE_LOW )
INPUT_PORTS_END

/* verified from Z80 code - manual found */
static INPUT_PORTS_START( wolfman )
	PORT_INCLUDE( pbsonic_generic )

	/* SYSTEM : port 0x28 (cpl'ed) -> 0x00 in 0x602a..0x6031 - 0x11 when pressed (code at 0x0173) */

	/* SW0 : port 0x00 (after 0x40 written to port 0x01) -> 0x00 in 0x6032..0x6039 - 0x11 when pressed (code at 0x0180) */
	/* SW1 : port 0x00 (after 0x41 written to port 0x01) -> 0x00 in 0x603a..0x6041 - 0x11 when pressed (code at 0x0190) */
	/* SW2 : port 0x00 (after 0x42 written to port 0x01) -> 0x00 in 0x6042..0x6049 - 0x11 when pressed (code at 0x01a0) */
	/* SW3 : port 0x00 (after 0x43 written to port 0x01) -> 0x00 in 0x604a..0x6051 - 0x11 when pressed (code at 0x01b0) */

	/* COIN2 doesn't give any credits and isn't even tested due to code at 0x01dd : only contents of 0x602f and 0x6031 are tested */

	/* DSW0 : port 0x20 - DSW0-1 is bit 7 ... DSW0-8 is bit 0 */
	PORT_MODIFY("DSW0")
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x40, 0x40, "Match" )                     // Loteria - code at 0x0a8a - stored at 0x60c2 (0x00 Score / 0x01 Replay) - then code at 0x0e41
	PORT_DIPSETTING(    0x40, "Awards Replay" )                       // Partida
	PORT_DIPSETTING(    0x00, "Doubles Score" )                       // Dobla Tanteo
	PORT_DIPNAME( 0x20, 0x00, "DSW0-3 Unknown" )            // Bola Reclamo - code at 0x0a94 - stored at 0x60c1 (0x00 ON / 0x01 OFF) - then code at 0x072e - sound related ?
	PORT_DIPSETTING(    0x00, "00" )
	PORT_DIPSETTING(    0x20, "01" )
//  PORT_DIPNAME( 0x18, 0x00, DEF_STR( Coinage ) )          // Partidas/Moneda - code at 0x0a69 - tables at 0x0b30 (4 * 3) - credits BCD stored at 0x6151
//  PORT_DIPNAME( 0x04, 0x00, "Balls" )                     // Bolas/Partida - code at 0x0a5c - stored at 0x60bd
	PORT_BIT( 0x03, 0x00, IPT_CUSTOM) PORT_CUSTOM_MEMBER(peyper_state, wolfman_replay_hs_r<0x03>)

	/* DSW1 : port 0x24 - DSW1-1 is bit 7 ... DSW1-8 is bit 0 */
	PORT_START("DSW1")
	PORT_DIPNAME( 0x80, 0x00, "Adjust Replay" )             // Premios por Puntuacion - code at 0x0aa3 - stored at 0x60c4 and 0x60cc (0x00 NO / 0x05 YES)
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
	PORT_BIT( 0x40, 0x00, IPT_CUSTOM) PORT_CUSTOM_MEMBER(peyper_state, wolfman_replay_hs_r<0x40>)
	PORT_DIPNAME( 0x20, 0x00, "Clear RAM on Reset" )        // Borrador RAM - code at 0x0ace - range 0x6141..0x616f - 0x616d = 0x5a and 0x616e = 0xa5
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0f, 0x08, "Test Mode" )                 // Visualizacion - code at 0x0f35
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x03, "Displays Replays and Extra Balls" )    // (0x6164++) on player 1, (0x6167++) on player 3 - I can't tell ATM which is which :(
	PORT_DIPSETTING(    0x05, "Displays Coins" )                      // Coin A (0x6154++) on player 1, Coin B (0x6157++) on player 3 and coin C (0x615a++) on player 2
	PORT_DIPSETTING(    0x06, "Displays Hours and Games" )            // hours since boot (0x615e++) on player 1, games started on player 3 (0x6161++)

	/* Fake port to handle settings via multiple input ports */
	PORT_START("REPLAY")
	PORT_DIPNAME( 0x43, 0x40, "Replay at / High-score" )    // Puntos/Premios - code at 0x09f4 and 0x0a25 - tables at 0x0af4 (4 * 9) and 0x0b24 (4 * 3) or 0x0b18 (1 * 9) and 0x0b21 (1 * 3)
	PORT_DIPSETTING(    0x40, "800k and 1200k and 8000k / 1410k" )    // manual says "800k and 1200k / 1400k"
	PORT_DIPSETTING(    0x41, "1000k and 1400k and 8000k / 1610k" )   // manual says "1000k and 1400k / 1600k"
	PORT_DIPSETTING(    0x42, "1200k and 1600k and 8000k / 1810k" )   // manual says "1200k and 1600k / 1800k"
	PORT_DIPSETTING(    0x43, "1400k and 1800k and 8000k / 2010k" )   // manual says "1400k and 1800k / 2000k"
	PORT_DIPSETTING(    0x00, "2400k and 2800k and 6800k / 3610k" )   // not mentioned in the manual where it is "conenecte" to DSW1-3 ("Clear RAM on Reset")
INPUT_PORTS_END

/* verified from Z80 code - NO manual found ! */
static INPUT_PORTS_START( odisea )
	PORT_INCLUDE( pbsonic_generic )

	/* SYSTEM : port 0x28 (cpl'ed) -> 0x00 in 0x6030..0x6037 - 0x11 when pressed (code at 0x0153) */

	/* SW0 : port 0x00 (after 0x40 written to port 0x01) -> 0x00 in 0x6038..0x603f - 0x11 when pressed (code at 0x0160) */
	/* SW1 : port 0x00 (after 0x41 written to port 0x01) -> 0x00 in 0x6040..0x6047 - 0x11 when pressed (code at 0x0170) */
	/* SW2 : port 0x00 (after 0x42 written to port 0x01) -> 0x00 in 0x6048..0x604f - 0x11 when pressed (code at 0x0180) */
	/* SW3 : port 0x00 (after 0x43 written to port 0x01) -> 0x00 in 0x6050..0x6057 - 0x11 when pressed (code at 0x0190) */

	/* DSW0 : port 0x20 - DSW0-1 is bit 7 ... DSW0-8 is bit 0 */
	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x80, 0x00, "DSW0-1 Unknown" )            // code at 0x0a99 - stored at 0x60c5
	PORT_DIPSETTING(    0x00, "02" )
	PORT_DIPSETTING(    0x80, "01" )
	PORT_DIPNAME( 0x40, 0x40, "Match" )                     // code at 0x0a83 - stored at 0x60c1 (0x00 Score / 0x01 Replay) - then code at 0x0de7
	PORT_DIPSETTING(    0x40, "Awards Replay" )
	PORT_DIPSETTING(    0x00, "Doubles Score" )
	PORT_DIPNAME( 0x20, 0x00, "DSW0-3 Unknown" )            // Bola Reclamo - code at 0x0a8f - stored at 0x60bf - then code at 0x06d8 (similar code in 'wolfman') - sound related ?
	PORT_DIPSETTING(    0x00, "00" )
	PORT_DIPSETTING(    0x20, "01" )
//  PORT_DIPNAME( 0x18, 0x00, DEF_STR( Coinage ) )          // Partidas/Moneda - code at 0x0a62 - tables at 0x0b17 (4 * 3) - credits BCD stored at 0x6173
//  PORT_DIPNAME( 0x04, 0x00, "Balls" )                     // Bolas/Partida - code at 0x0a55 - stored at 0x60bb
	/* Replay at 2400k 2800k and 3200k / High-score 3210k - code at 0x09f3 and 0x0b23 - tables at 0x0aff (1 * 9) and 0x0b08 (1 * 3) */
	PORT_DIPNAME( 0x02, 0x00, "DSW0-7 Unknown" )            // code at 0x0342 - stored at 0x60a7
	PORT_DIPSETTING(    0x00, "14" )
	PORT_DIPSETTING(    0x02, "28" )
	PORT_DIPUNUSED( 0x01, IP_ACTIVE_LOW )

	/* DSW1 : port 0x24 - DSW1-1 is bit 7 ... DSW1-8 is bit 0 */
	PORT_START("DSW1")
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x28, 0x28, "Test Mode" )                 // code at 0x0a0b (RAM) - range 0x6161..0x618d - 0x618c = 0x5a and 0x618d = 0xa5 - code at 0x0fc8 (Test)
	PORT_DIPSETTING(    0x28, DEF_STR( Off ) )                        // does NOT clear RAM on reset
	PORT_DIPSETTING(    0x20, "Displays Coins" )                      // Coin A (0x6176++) on player 3 and coin C (0x617c++) on player 1 - also clears RAM on reset
	PORT_DIPSETTING(    0x08, "Displays Games, Replays & EB" )        // games started (0x617f++) on player 2, replays (0x6182++) on player 3 and EB (0x6185++) on player 4 - also clears RAM on reset
//  PORT_DIPSETTING(    0x00, "INVALID" )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x02, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x01, IP_ACTIVE_LOW )
INPUT_PORTS_END


void peyper_state::machine_start()
{
	genpin_class::machine_start();

	m_leds.resolve();
	m_digits.resolve();
	m_io_outputs.resolve();

	save_item(NAME(m_row));
	save_item(NAME(m_game));
	save_item(NAME(m_outhole_solenoid));
}

void peyper_state::machine_reset()
{
	genpin_class::machine_reset();
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;

	m_row = 0;
}


void peyper_state::peyper(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 2'500'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &peyper_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &peyper_state::io_map);
	m_maincpu->set_periodic_int(FUNC(peyper_state::irq0_line_hold), attotime::from_hz(1250));
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	config.set_default_layout(layout_peyper);

	/* Sound */
	genpin_audio(config);
	SPEAKER(config, "ayvol").front_center();
	ay8910_device &ay1(AY8910(config, "ay1", 2500000));
	ay1.port_a_write_callback().set(FUNC(peyper_state::p1a_w));
	ay1.port_b_write_callback().set(FUNC(peyper_state::p1b_w));
	ay1.add_route(ALL_OUTPUTS, "ayvol", 1.0);
	ay8910_device &ay2(AY8910(config, "ay2", 2500000));
	ay2.port_a_write_callback().set(FUNC(peyper_state::p2a_w));
	ay2.port_b_write_callback().set(FUNC(peyper_state::p2b_w));
	ay2.add_route(ALL_OUTPUTS, "ayvol", 1.0);

	/* Devices */
	i8279_device &kbdc(I8279(config, "i8279", 2500000));
	kbdc.out_sl_callback().set(FUNC(peyper_state::col_w));      // scan SL lines
	kbdc.out_disp_callback().set(FUNC(peyper_state::disp_w));   // display A&B
	kbdc.in_rl_callback().set(FUNC(peyper_state::sw_r));        // kbd RL lines
	kbdc.in_shift_callback().set_constant(1);                   // Shift key
	kbdc.in_ctrl_callback().set_constant(1);
}

void peyper_state::petaco(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 5_MHz_XTAL / 2);  // sch shows 5MHz for a normal Z80
	m_maincpu->set_addrmap(AS_PROGRAM, &peyper_state::petaco_map);
	m_maincpu->set_addrmap(AS_IO, &peyper_state::petaco_io_map);
	m_maincpu->set_periodic_int(FUNC(peyper_state::irq0_line_hold), attotime::from_hz(400));  // missing from sch

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	//I8279
	i8279_device &kbdc(I8279(config, "i8279", 2500000));
	kbdc.out_sl_callback().set(FUNC(peyper_state::col_w));      // scan SL lines
	kbdc.out_disp_callback().set(FUNC(peyper_state::disp_w));   // display A&B
	kbdc.in_rl_callback().set(FUNC(peyper_state::sw_r));        // kbd RL lines
	kbdc.in_shift_callback().set_constant(1);                   // Shift key
	kbdc.in_ctrl_callback().set_constant(1);

	/* Video */
	config.set_default_layout(layout_peyper);

	/* Sound */
	genpin_audio(config);
	SPEAKER(config, "ayvol").front_center();
	ay8910_device &ay1(AY8910(config, "ay1", 5_MHz_XTAL / 2));
	ay1.port_a_write_callback().set(FUNC(peyper_state::ay1_a_w));
	ay1.port_b_write_callback().set(FUNC(peyper_state::ay1_b_w));
	ay1.add_route(ALL_OUTPUTS, "ayvol", 0.9);

	ay8910_device &ay2(AY8910(config, "ay2", 5_MHz_XTAL / 2));
	ay2.port_a_write_callback().set_nop();    // not connected
	ay2.port_b_write_callback().set(FUNC(peyper_state::ay2_b_w));
	ay2.add_route(ALL_OUTPUTS, "ayvol", 0.9);
}


/*-------------------------------------------------------------------
/ Odin (1985)
/-------------------------------------------------------------------*/
ROM_START(odin)
	ROM_REGION(0x6000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("odin_a.bin", 0x0000, 0x2000, CRC(ac3a7770) SHA1(2409629d3adbae0d7e6e5f9fe6f137c1e5a1bb86))
	ROM_LOAD("odin_b.bin", 0x2000, 0x2000, CRC(46744695) SHA1(fdbd8a93b3e4a9697e77e7d381759829b86fe28b))
ROM_END

/*-------------------------------------------------------------------
/ Odin De Luxe (1985)
/-------------------------------------------------------------------*/
ROM_START(odin_dlx)
	ROM_REGION(0x6000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("1a.bin", 0x0000, 0x2000, CRC(4fca9bfc) SHA1(05dce75919375d01a306aef385bcaac042243695))
	ROM_LOAD("2a.bin", 0x2000, 0x2000, CRC(46744695) SHA1(fdbd8a93b3e4a9697e77e7d381759829b86fe28b))
ROM_END

/*-------------------------------------------------------------------
/ Solar Wars (1986)
/-------------------------------------------------------------------*/
ROM_START(solarwap)
	ROM_REGION(0x6000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("solarw1c.bin", 0x0000, 0x2000, CRC(aa6bf0cd) SHA1(7332a4b1679841283d846f3e4f1792cb8e9529bf))
	ROM_LOAD("solarw2.bin",  0x2000, 0x2000, CRC(95e2cbb1) SHA1(f9ab3222ca0b9e0796030a7a618847a4e8f77957))
ROM_END

/*-------------------------------------------------------------------
/ Gamatron (1986)
/-------------------------------------------------------------------*/
ROM_START(gamatros)
	ROM_REGION(0x6000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gama_a.bin", 0x0000, 0x2000, CRC(1dc2841c) SHA1(27c6a07b1f8bd5e73b425e7dbdcfb1d5233c18b2))
	ROM_LOAD("gama_b.bin", 0x2000, 0x2000, CRC(56125890) SHA1(8b30a2282df264d798df1b031ecade999d135f81))
ROM_END

/*-------------------------------------------------------------------
/ Pole Position (1987)
/-------------------------------------------------------------------*/
ROM_START(poleposn)
	ROM_REGION(0x6000, "maincpu", 0)
	ROM_LOAD("1.bin", 0x0000, 0x2000, CRC(fdd37f6d) SHA1(863fef32ab9b5f3aca51788b6be9373a01fa0698))
	ROM_LOAD("2.bin", 0x2000, 0x2000, CRC(967cb72b) SHA1(adef17018e2caf65b64bbfef72fe159b9704c409))
	ROM_LOAD("3.bin", 0x4000, 0x2000, CRC(461fe9ca) SHA1(01bf35550e2c55995f167293746f355cfd484af1))
ROM_END

/*-------------------------------------------------------------------
/ Star Wars (1987)
/-------------------------------------------------------------------*/
ROM_START(sonstwar)
	ROM_REGION(0x6000, "maincpu", 0)
	ROM_LOAD("sw1.bin", 0x0000, 0x2000, CRC(a2555d92) SHA1(5c82be85bf097e94953d11c0d902763420d64de4))
	ROM_LOAD("sw2.bin", 0x2000, 0x2000, CRC(c2ae34a7) SHA1(0f59242e3aec5da7111e670c4d7cf830d0030597))
	ROM_LOAD("sw3.bin", 0x4000, 0x2000, CRC(aee516d9) SHA1(b50e54d4d5db59e3fb71fb000f9bc5e34ff7de9c))
ROM_END

ROM_START(sonstwr2)
	ROM_REGION(0x6000, "maincpu", 0)
	ROM_LOAD("stw1i.bin", 0x0000, 0x2000, CRC(416e2a0c) SHA1(74ca550ee9eb83d9762ffab0f085dffae569d4a9))
	ROM_LOAD("stw2i.bin", 0x2000, 0x2000, CRC(ccbbec46) SHA1(4fd0e48916e8761a7e70300d3ede166f5f04f8ae))
	ROM_LOAD("sw3.bin",   0x4000, 0x2000, CRC(aee516d9) SHA1(b50e54d4d5db59e3fb71fb000f9bc5e34ff7de9c))
ROM_END
/*-------------------------------------------------------------------
/ Hang-On (1988)
/-------------------------------------------------------------------*/
ROM_START(hangonp)
	ROM_REGION(0x6000, "maincpu", 0)
	ROM_LOAD("hangon1.bin", 0x0000, 0x2000, CRC(b0672137) SHA1(e0bd0808a3a8c6df200b0edc7b5e8cf293a659b7))
	ROM_LOAD("hangon2.bin", 0x2000, 0x2000, CRC(6e1e55c0) SHA1(473c882a0eb68807969894b82be2b86d7c463c93))
	ROM_LOAD("hangon3.bin", 0x4000, 0x2000, CRC(26949f2f) SHA1(e3e1a436ce59c7f1c2904cd8f50f2ba4a4e37638))
ROM_END
/*-------------------------------------------------------------------
/ Odisea Paris-Dakar (1987)
/-------------------------------------------------------------------*/
ROM_START(odisea)
	ROM_REGION(0x6000, "maincpu", 0)
	ROM_LOAD("odiseaa.bin", 0x0000, 0x2000, CRC(29a40242) SHA1(321e8665df424b75112589fc630a438dc6f2f459))
	ROM_LOAD("odiseab.bin", 0x2000, 0x2000, CRC(8bdf7c17) SHA1(7202b4770646fce5b2ba9e3b8ca097a993123b14))
	ROM_LOAD("odiseac.bin", 0x4000, 0x2000, CRC(832dee5e) SHA1(9b87ffd768ab2610f2352adcf22c4a7880de47ab))
ROM_END
// Note that odisea roms of size 0x4000 exist on the web, the first half of each is identical to what we have, the 2nd half is all 0xFF.

/*-------------------------------------------------------------------
/ Nemesis (1986)
/-------------------------------------------------------------------*/
ROM_START(nemesisp)
	ROM_REGION(0x6000, "maincpu", 0)
	ROM_LOAD("nemesisa.bin", 0x0000, 0x2000, CRC(56f13350) SHA1(30907c362f88b48d634e8aaa1e1161852886645c))
	ROM_LOAD("nemesisb.bin", 0x2000, 0x2000, CRC(a8f3e6c7) SHA1(c25b2271c4de6f4b57c3c850d28a0878ea081c26))
	ROM_LOAD("memoriac.bin", 0x4000, 0x2000, CRC(468f16f0) SHA1(66ce0464d82331cfc0ac1f6fbd871066e4e57262))
ROM_END

/*-------------------------------------------------------------------
/ Wolf Man (1987)
/-------------------------------------------------------------------*/
ROM_START(wolfman)
	ROM_REGION(0x6000, "maincpu", 0)
	ROM_LOAD("memoriaa.bin", 0x0000, 0x2000, CRC(1fec83fe) SHA1(5dc887d0fa00129ae31451c03bfe442f87dd2f54))
	ROM_LOAD("memoriab.bin", 0x2000, 0x2000, CRC(62a1e3ec) SHA1(dc472c7c9d223820f8f1031c92e36890c1fcba7d))
	ROM_LOAD("memoriac.bin", 0x4000, 0x2000, CRC(468f16f0) SHA1(66ce0464d82331cfc0ac1f6fbd871066e4e57262))
ROM_END

/*-------------------------------------------------------------------
/ Ator (1985)
/-------------------------------------------------------------------*/
ROM_START(ator) // Version with 2 bumpers, probably older
	ROM_REGION(0x6000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("ator1.bin", 0x0000, 0x2000, CRC(87967577) SHA1(6586401a4b1a837bacd61d156b44eedaab271479))
	ROM_LOAD("ator2.bin", 0x2000, 0x2000, CRC(832b06ea) SHA1(c70c613fd29d1d560951890bce072cbf45525526))
	// No ROM 3
ROM_END

ROM_START(ator3bmp) // Version with 3 bumpers, probably newer
	ROM_REGION(0x6000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("1.bin", 0x0000, 0x2000, NO_DUMP)
	ROM_LOAD("ator 2 _0xba29.bin", 0x2000, 0x2000, CRC(21aad5c4) SHA1(e78da5d80682710db34cbbfeae5af54241c73371))
	// probably no ROM 3 (PCB photo shows location unpopulated)
ROM_END

/*-------------------------------------------------------------------
/ Petaco (1984)
/-------------------------------------------------------------------*/
ROM_START(petaco)
	ROM_REGION(0x6000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("petaco1.cpu", 0x0000, 0x2000, CRC(f4e09939) SHA1(dcc4220b269d271eb0b6ad0a5d3c1a240587a01b))
	ROM_LOAD("petaco2.cpu", 0x2000, 0x2000, CRC(d29a59ea) SHA1(bb7891e9597bbf5ae6a3276abf2b1247e082d828))
ROM_END


} // Anonymous namespace

GAME( 1984, petaco,   0,        petaco,   odin_dlx, peyper_state, init_1,     ROT0, "Juegos Populares", "Petaco",             MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1985, odin,     0,        peyper,   odin_dlx, peyper_state, init_1,     ROT0, "Peyper",     "Odin",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1985, odin_dlx, 0,        peyper,   odin_dlx, peyper_state, init_1,     ROT0, "Sonic",      "Odin De Luxe",             MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1986, solarwap, 0,        peyper,   solarwap, peyper_state, init_0,     ROT0, "Sonic",      "Solar Wars (Sonic)",       MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1986, gamatros, 0,        peyper,   solarwap, peyper_state, init_0,     ROT0, "Sonic",      "Gamatron (Sonic)",         MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1987, poleposn, 0,        peyper,   poleposn, peyper_state, init_0,     ROT0, "Sonic",      "Pole Position (Sonic)",    MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1987, sonstwar, 0,        peyper,   sonstwar, peyper_state, init_3,     ROT0, "Sonic",      "Star Wars (Sonic, set 1)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1987, sonstwr2, sonstwar, peyper,   sonstwar, peyper_state, init_3,     ROT0, "Sonic",      "Star Wars (Sonic, set 2)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1987, wolfman,  0,        peyper,   wolfman,  peyper_state, init_2,     ROT0, "Peyper",     "Wolf Man",                 MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1986, nemesisp, 0,        peyper,   wolfman,  peyper_state, init_2,     ROT0, "Peyper",     "Nemesis",                  MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1987, odisea,   0,        peyper,   odisea,   peyper_state, init_2,     ROT0, "Peyper",     "Odisea Paris-Dakar",       MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1988, hangonp,  0,        peyper,   sonstwar, peyper_state, init_3,     ROT0, "Sonic",      "Hang-On (Sonic)",          MACHINE_IS_SKELETON_MECHANICAL ) // inputs to be checked
GAME( 1985, ator,     0,        peyper,   sonstwar, peyper_state, init_1,     ROT0, "Video Dens", "Ator (set 1, 2 bumpers)",  MACHINE_IS_SKELETON_MECHANICAL ) // inputs to be checked
GAME( 1985, ator3bmp, ator,     peyper,   sonstwar, peyper_state, init_1,     ROT0, "Video Dens", "Ator (set 2, 3 bumpers)",  MACHINE_IS_SKELETON_MECHANICAL ) // initial program ROM missing; no manual found
