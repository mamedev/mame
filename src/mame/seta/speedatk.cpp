// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Pierpaolo Prazzoli, Takahiro Nogi
/**************************************************************************************************

Dai-Fugo (c) 1983 Sega / Seta
Speed Attack! (c) 1984 Seta Kikaku Corp.

driver by Pierpaolo Prazzoli & Angelo Salese, based on early work by David Haywood

TODO:
 - Improve IOX device for daifugo (many hardwired reads);
 - It's possible that there is only one coin chute and not two,needs a real board to know
   more about it.

How to play:
 - A to D selects a card.
 - Turn takes one or more cards into your hand (depends on how many cards you
   putted on the stacks).
 - Left & right puts a card on one of the two stacks.

Notes:
 - According to the text gfx rom, there are also a Taito and a KKK versions out there.

===================================================================================================

SPEED ATTACK!
(c)SETA

CPU :NEC D780C x 1
SOUND   :AY-3-8910 x 1
XTAL    :12MHZ

SETA CUSTOM ?
AC-002 , AC-003

CB1-1   :1C
CB0-2   :1D
CB1-3   :1F
CB0-4   :1H
CB0-5   :7C
CB0-6   :7D
CB0-7   :7E

CB1.BPR :7L TBP18S030
CB2.BPR :6K 82S129

---------------------------------------------------------------------------------------------------

DIP SWITCH 8BIT (Default: ALL ON)

SW 1,2 : COIN CREDIT   LL:1-1 HL:1-2 LH:1-5 HH:1-10
SW 3,4 : LEVEL LL:EASY -> LH -> HL -> HH:HARD
SW 5,6 : NOT USE
SW 7   : FLIP SCREEN H:FLIP
SW 8   : TEST MODE H:TEST

   PARTS SIDE | SOLDIER SIDE
  ----------------------------
      GND   | 1|    GND
      GND   | 2|    GND
      +5V   | 3|    +5V
            | 4|
     +12V   | 5|   +12V
  SPEAKER(+)| 6|  SPEAKER(-)
     SYNC   | 7| COIN COUNTER
       B    | 8|  SERVICE
       G    | 9|  COIN SW
       R    |10|
     PD 6   |11|   PS 6 (NOT USE)
     PD 5   |12|   PS 5 (NOT USE)
     PD 4   |13|   PS 4
     PD 3   |14|   PS 3
     PD 1   |15|   PS 1
     PD 2   |16|   PS 2
            |17|
            |18|

PS / PD :  key matrix

===================================================================================================

Dai-Fugo (C)1983 ESCO TRADING CO.,INC. / SEGA ENTERPRISES LTD. / SETA KIKAKU LTD.

Developed by SETA
Manufactured by SEGA
Exclusively licensed to ESCO

Model No.834-5206

CA2_1.1c    PRG 0x2000
CA2_2.1d    PRG 0x2000
CA2_3.1e    PRG 0x2000
CA2_4.1h    PRG 0x1000
CA1_5.7c    CHR 0x2000
CA1_6.7d    CHR 0x2000
CA1_7.7e    CHR 0x2000
tbp24s10.6k COLOR   0x100
tbp18s030.7l    CLUT    0x20

X'tal       12.000MHz
CPU         D780C(Z80 1.5MHz?)
SOUND       AY-3-8910(1.5MHz?)
CRTC        HD46505SP-1 1.5MHz
WRAM        M58725P(16Kbit SRAM)
VRAM        M5L2114LP-3(4Kbit SRAM) x4
CUSTOM      AC001(14pin DIP)
            AC002(40pin DIP)
            AC003(40pin DIP)
DIPSW       8 Elements Switch Array x1

**************************************************************************************************/

#include "emu.h"
#include "speedatk.h"

#include "cpu/z80/z80.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"
#include "screen.h"
#include "speaker.h"


#define MASTER_CLOCK XTAL(12'000'000)

void speedatk_state::machine_start()
{
	save_item(NAME(m_mux_data));
	save_item(NAME(m_km_status));
	save_item(NAME(m_coin_settings));
	save_item(NAME(m_coin_impulse));

	m_coin_impulse = 0;
}

uint8_t speedatk_state::iox_key_matrix_calc(uint8_t p_side)
{
	static const char *const keynames[] = { "P1_ROW0", "P1_ROW1", "P2_ROW0", "P2_ROW1" };

	int i, j, t;

	for (i = 0x00 ; i < 0x10 ; i += 8)
	{
		j = (i / 0x08);

		for (t = 0 ; t < 8 ; t ++)
		{
			if (!(ioport(keynames[j+p_side])->read() & ( 1 << t )))
			{
				return (i + t) | (p_side ? 0x20 : 0x00);
			}
		}
	}

	return 0;
}

uint8_t speedatk_state::key_matrix_r()
{
	if(m_coin_impulse > 0)
	{
		m_coin_impulse--;
		return 0x80;
	}

	if((ioport("COINS")->read() & 1) || (ioport("COINS")->read() & 2))
	{
		m_coin_impulse = m_coin_settings;
		m_coin_impulse--;
		return 0x80;
	}

	if(m_mux_data != 1 && m_mux_data != 2 && m_mux_data != 4)
		return 0xff; //unknown command

	/* both side checks */
	if(m_mux_data == 1)
	{
		uint8_t p1_side = iox_key_matrix_calc(0);
		uint8_t p2_side = iox_key_matrix_calc(2);

		if(p1_side != 0)
			return p1_side;

		return p2_side;
	}

	/* check individual input side */
	return iox_key_matrix_calc((m_mux_data == 2) ? 0 : 2);
}

uint8_t speedatk_state::daifugo_key_matrix_r()
{
	if(m_coin_impulse > 0)
	{
		m_coin_impulse--;
		return 0x80;
	}

	if(ioport("COINS")->read() & 1)
	{
		m_coin_impulse = m_coin_settings;
		m_coin_impulse--;
		return 0x80;
	}

	static const char *const keynames[] = { "PLAYER1", "PLAYER2" };

	int i;
	const uint8_t player_side = (m_mux_data >> 2) & 1;

	// key matrix check and change binary digit to decimal number
	for (i = 0 ; i < 16 ; i++)
	{
		if (ioport(keynames[player_side])->read() & (1 << i))
		{
			return (i + 1);
		}
	}

	return 0;
}

void speedatk_state::key_matrix_w(uint8_t data)
{
	m_mux_data = data;
}

/* Key matrix status,used for coin settings and I don't know what else... */
uint8_t speedatk_state::key_matrix_status_r()
{
	/* bit 0: busy flag,active low */
	return (m_km_status & 0xfe) | 1;
}

/*
xxxx ---- command
---- xxxx param
My guess is that the other commands configs the key matrix, it probably needs some tests on the real thing.
1f
3f
41
61
8x coinage setting command
a1
*/
void speedatk_state::key_matrix_status_w(uint8_t data)
{
	m_km_status = data;
	if((m_km_status & 0xf0) == 0x80) //coinage setting command
		m_coin_settings = m_km_status & 0xf;
}

void speedatk_state::speedatk_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x8000).rw(FUNC(speedatk_state::key_matrix_r), FUNC(speedatk_state::key_matrix_w));
	map(0x8001, 0x8001).rw(FUNC(speedatk_state::key_matrix_status_r), FUNC(speedatk_state::key_matrix_status_w));
	map(0x8588, 0x858f).nopr(); // speedatk only
	map(0x8800, 0x8fff).ram();
	map(0xa000, 0xa3ff).ram().share("videoram");
	map(0xb000, 0xb3ff).ram().share("colorram");
}

void speedatk_state::daifugo_mem(address_map &map)
{
	speedatk_mem(map);
	map(0x8000, 0x8000).rw(FUNC(speedatk_state::daifugo_key_matrix_r), FUNC(speedatk_state::key_matrix_w));
	map(0x8001, 0x8001).lr8(NAME([] (offs_t offset) {
		// TODO: bit 1 seems to be a busy flag, will throw a "BAD CSTM 2" if that is high.
		return 0x00;
	}));
	// Protection?
	map(0x8464, 0x8464).lr8(NAME([] (offs_t offset) {
		return 0x00;    // this value is never referenced in game
	}));
	map(0x8465, 0x8465).lr8(NAME([] (offs_t offset) {
		return 0xb6;    // strange behavior if value other than 0xb6 is set
	}));
	map(0x8469, 0x8469).lr8(NAME([] (offs_t offset) {
		return 0x84;    // 1000_0100b(084h) game stops if other value is set
	}));
	map(0x8471, 0x8471).lr8(NAME([] (offs_t offset) {
		return 0x00;    // 0x00 game stops if other value is set
	}));
	map(0x8630, 0x8630).lr8(NAME([] (offs_t offset) {
		return 0x4d;    // 0x4D accept I/O and sound process
	}));
}

void speedatk_state::speedatk_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w(FUNC(speedatk_state::m6845_w)); //h46505 address / data routing
	map(0x24, 0x24).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x40, 0x40).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x40, 0x41).w("aysnd", FUNC(ay8910_device::address_data_w));
	map(0x60, 0x68).noprw();
	//what's 60-6f for? Seems used only in attract mode and read back when a 2p play ends ...
}

static INPUT_PORTS_START( daifugo )
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Time/CPU Card" )
	PORT_DIPSETTING(    0x10, "300/Not Open" )
	PORT_DIPSETTING(    0x00, "400/Open" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("COINS")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)

	PORT_START("PLAYER1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_HANAFUDA_A )      PORT_PLAYER(1)  // 1
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_HANAFUDA_B )      PORT_PLAYER(1)  // 2
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_HANAFUDA_C )      PORT_PLAYER(1)  // 3
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_HANAFUDA_D )      PORT_PLAYER(1)  // 4
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED )          PORT_PLAYER(1)  // 5
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )          PORT_PLAYER(1)  // 6
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_HANAFUDA_E )      PORT_PLAYER(1)  // 7
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_HANAFUDA_H )      PORT_PLAYER(1)  // 8
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_HANAFUDA_F )      PORT_PLAYER(1)  // 9
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_HANAFUDA_G )      PORT_PLAYER(1)  // 10
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )          PORT_PLAYER(1)  // 11
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_START1 )          PORT_PLAYER(1)  // 12
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_START2 )          PORT_PLAYER(1)  // 13
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_HANAFUDA_YES )    PORT_PLAYER(1)  // 14
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_HANAFUDA_NO ) PORT_NAME("P1 Hanafuda No/Pass")    PORT_PLAYER(1)  // 15
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNUSED )          PORT_PLAYER(1)  // 16

	PORT_START("PLAYER2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_HANAFUDA_A )      PORT_PLAYER(2)  // 1
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_HANAFUDA_B )      PORT_PLAYER(2)  // 2
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_HANAFUDA_C )      PORT_PLAYER(2)  // 3
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_HANAFUDA_D )      PORT_PLAYER(2)  // 4
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED )          PORT_PLAYER(2)  // 5
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )          PORT_PLAYER(2)  // 6
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_HANAFUDA_E )      PORT_PLAYER(2)  // 7
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_HANAFUDA_H )      PORT_PLAYER(2)  // 8
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_HANAFUDA_F )      PORT_PLAYER(2)  // 9
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_HANAFUDA_G )      PORT_PLAYER(2)  // 10
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )          PORT_PLAYER(2)  // 11
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNUSED )          PORT_PLAYER(2)  // 12
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNUSED )          PORT_PLAYER(2)  // 13
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_HANAFUDA_YES )    PORT_PLAYER(2)  // 14
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_HANAFUDA_NO ) PORT_NAME("P2 Hanafuda No/Pass")    PORT_PLAYER(2)  // 15
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNUSED )          PORT_PLAYER(2)  // 16
INPUT_PORTS_END

static INPUT_PORTS_START( speedatk )
	PORT_START("DSW")
	PORT_SERVICE( 0x01, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, "1 Coin/10 Credits" )

	PORT_START("P1_ROW0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 B") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 A") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 C") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 D") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("P1 Left") PORT_CODE(KEYCODE_A)

	PORT_START("P1_ROW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("P1 Right") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1) PORT_NAME("P1 Turn") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2_ROW0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 B")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 A")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 C")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 D")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("P2 Left")

	PORT_START("P2_ROW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME("P2 Right")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(2) PORT_NAME("P2 Turn")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COINS")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1)
INPUT_PORTS_END

static const gfx_layout charlayout_1bpp =
{
	8,8,
	RGN_FRAC(1,1),
	3,
	{ 0, 0, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( gfx_speedatk )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout_1bpp,   0, 32 )
	GFXDECODE_ENTRY( "gfx2", 0, gfx_8x8x3_planar,  0, 32 )
GFXDECODE_END

void speedatk_state::output_w(uint8_t data)
{
	m_flip_scr = data & 0x80;

	if((data & 0x7f) != 0x7f)
		logerror("%02x\n",data);
}

void speedatk_state::speedatk(machine_config &config)
{
	Z80(config, m_maincpu, MASTER_CLOCK/4); //divider is unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &speedatk_state::speedatk_mem);
	m_maincpu->set_addrmap(AS_IO, &speedatk_state::speedatk_io);
	m_maincpu->set_vblank_int("screen", FUNC(speedatk_state::irq0_line_hold));

	WATCHDOG_TIMER(config, "watchdog"); // timing is unknown

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(320, 256);
	screen.set_visarea(0*8, 32*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(speedatk_state::screen_update));

	HD6845S(config, m_crtc, MASTER_CLOCK/16);   /* HD46505SP/HD6845SP; hand tuned to get ~60 fps */
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_speedatk);
	PALETTE(config, m_palette, FUNC(speedatk_state::speedatk_palette), 0x100, 16);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", MASTER_CLOCK/8)); //divider is unknown
	aysnd.port_b_read_callback().set_ioport("DSW");
	aysnd.port_a_write_callback().set(FUNC(speedatk_state::output_w));
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.5);
}

void speedatk_state::daifugo(machine_config &config)
{
	speedatk(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &speedatk_state::daifugo_mem);
}

ROM_START( daifugo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ca2_1.1c",      0x0000, 0x2000, CRC(cef063a1) SHA1(851c8f7f723822c19c02a35908b6246bd5d5c806) )
	ROM_LOAD( "ca2_2.1d",      0x2000, 0x2000, CRC(29e17bb9) SHA1(7028dca9b794af561eb86fc9d3e5ad7c7c9c6050) )
	ROM_LOAD( "ca2_3.1e",      0x4000, 0x2000, CRC(570dc665) SHA1(f473aeeaa536ed476bd107fd6b594e3dfd5d9cf4) )
	ROM_LOAD( "ca2_4.1h",      0x6000, 0x1000, CRC(85a1707c) SHA1(de95bb11918ed15f0061041f4a2ccbc38832280a) )

	ROM_REGION( 0x6000, "gfx1", ROMREGION_ERASE00 )
	// unused in daifugo?

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "ca1_5.7c",      0x0000, 0x2000, CRC(36f08de2) SHA1(0852b52dbc309addbf81e4e6692b02bf6fea3f95) )
	ROM_LOAD( "ca1_6.7d",      0x2000, 0x2000, CRC(c70b8f3d) SHA1(d8d9f0db53b89800a46e6ba2b6cd8b5669563cac) )
	ROM_LOAD( "ca1_7.7e",      0x4000, 0x2000, CRC(12a6ba09) SHA1(f9a133774b29d061168e247d74deed8fcc561d0c) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "tbp18s030.7l",  0x0000, 0x0020, CRC(bd674823) SHA1(c664b9959c939900dde3f86722404253b0e3f3f6) )  // color PROM
	ROM_LOAD( "tbp24s10.6k",   0x0020, 0x0100, CRC(6bd28c7a) SHA1(6840481a9b496cb37a45895b73d3270e49212a3e) )  // lookup table
ROM_END

ROM_START( speedatk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cb1-1",        0x0000, 0x2000, CRC(df988e05) SHA1(0ec91c5f2e1adf952a4fe7aede591e763773a75b) )
	ROM_LOAD( "cb0-2",        0x2000, 0x2000, CRC(be949154) SHA1(8a594a7ebdc8456290919163f7ea4ccb0d1f4edb) )
	ROM_LOAD( "cb1-3",        0x4000, 0x2000, CRC(741a5949) SHA1(7f7bebd4fb73fef9aa28549d100f632c442ac9b3) )
	ROM_LOAD( "cb0-4",        0x6000, 0x2000, CRC(53a9c0c8) SHA1(cd0fd94411dabf09828c1f629891158c40794127) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "cb0-7",        0x0000, 0x2000, CRC(a86007b5) SHA1(8e5cab76c37a8d53e1355000cd1a0a85ffae0e8c) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "cb0-5",        0x0000, 0x2000, CRC(47a966e7) SHA1(fdaa0f88656afc431bae367679ce6298fa962e0f) )
	ROM_LOAD( "cb0-6",        0x2000, 0x2000, CRC(cc1da937) SHA1(1697bb008bfa5c33a282bd470ac39c324eea7509) )
	ROM_COPY( "gfx2",         0x0000, 0x4000, 0x1000 ) /* Fill the blank space with cards gfx */
	ROM_COPY( "gfx1",         0x1000, 0x5000, 0x1000 ) /* Gfx from cb0-7 */

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "cb1.bpr",      0x0000, 0x0020, CRC(a0176c23) SHA1(133fb9eef8a6595cac2dcd7edce4789899a59e84) ) /* color PROM */
	ROM_LOAD( "cb2.bpr",      0x0020, 0x0100, CRC(a604cf96) SHA1(a4ef6e77dcd3abe4c27e8e636222a5ee711a51f5) ) /* lookup table */
ROM_END

GAME( 1983, daifugo, 0,  daifugo,  daifugo,  speedatk_state, empty_init, ROT90, "Seta Kikaku / Sega (Esco Trading Co license)", "Daifugo (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION )
GAME( 1984, speedatk, 0, speedatk, speedatk, speedatk_state, empty_init, ROT0,  "Seta Kikaku", "Speed Attack! (Japan)", MACHINE_SUPPORTS_SAVE )
