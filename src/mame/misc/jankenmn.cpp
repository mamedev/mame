// license:BSD-3-Clause
// copyright-holders:Roberto Fresca, hap
/***************************************************************************

  JANKENMAN UNIT

  Driver by Roberto Fresca.
  With further improvements by MAME Team.


  Coin-operated Z80-CTC + DAC system. No screen, just artwork with lamps + LEDs.
  Janken man is coming from Jankenpon, the rock-paper-scissors game.
  There are several versions of this game, the most notable difference being in
  the artwork and bonus lamps. The kid's voice and hand are the same among all.

  Control panel is composed of buttons:
  Guu (rock), Choki (scissors), Paa (paper).
  Some cabs have a Start button and/or Payout button too.

  Info:
  http://dgm.hmc6.net/museum/jyankenman.html
  http://dgm.hmc6.net/museum/jyankenman_kattaraageru.html
  (and many videos on Youtube)


  Janken Man series (working on this hardware):

  * Janken Man (Pretty Carnival) (3 station prize game), (c) 1985, Sunwise.
  * New Janken Man,                                      (c) 1986, Sunwise.
  * Janken Man Fever,                                    (c) 1988, Sunwise.
  * Janken Man Fever JP,                                 (c) 1991, Sunwise.
  * Janken Man JP,                                       (c) 1991, Sunwise.
  * Janken Man Kattara Ageru,                            (c) 1991, Sunwise.
  * Janken Man Bubbles,                                  (c) 1992, Sunwise.
  * Janken Man Big,                                      (c) 19??, Sunwise.
  * Janken Man Lucky!,                                   (c) 199?, Sunwise.


****************************************************************************

  General Notes...

  For better experience, use the hi-res external artwork I made.

  Preview: http://www.robertofresca.com/imagenes/jankenmn_full.png
  Artwork: http://mrdo.mameworld.info/artwork/jankenmn.zip
  (mirror) http://www.progettoemma.net/mw/jankenmn.zip

****************************************************************************

  Hardware Notes...

  1x LH0080A          ; Sharp, Z80A CPU.
  1x LH0082A          ; Sharp, Z80 CTC Counter Timer Circuit.
  2x M5L8255AP-5      ; Mitsubishi, PPI 8255 (I/O).
  1x HM6116P-3        ; Hitachi, SRAM.

  1x 27C128           ; Program, labeled 'PCG1'.
  1x 27C020           ; Waveform (8bit mono unsigned 8192Hz), labeled 'PCG2'.

  1x AD7523JN         ; InterSil, D/A Converter, 8-Bit, Multiplying, 6.7MHz.
  1x LA8358           ; Sanyo, ???. (near volume knob)
  1x 386D NJR         ; New Japan Radio, LM386D IC (500mW, 1-Channel Mono Audio AMP).
  3x M54562P          ; 8-Unit 500mA source type Darlington Transistor Array with clamp diode.

  1x 2.500 MHz Xtal.
  1x 8 DIP Switches bank.


****************************************************************************

  Other specs...

  Name of game:       Janken Man.
  Manufacturer:       Sanwaizu Co., Ltd. (Bankruptcy on March 6, 1998)
  Year:               May 1985 (1985)
  Body dimensions:    Depth: 355mm,  Width: 340mm, Height: 855mm (body only)
  Weight:             30kg.
  Power:              AC 100V 50/60Hz.
  Power consumption:  32W
  Capacity:           400 game tokens, 200 commemorative tokens
  Safe capacity:      6000 coins 10 yen, 500 coins 100 yen.
  Coin acceptor:      10 and 100 yens, Manufactured by Asahi Seiko 730-A/BW.
  Coin selector:      KWM/740 made by Asahi Seiko.
  Hopper:             MP04975 made by MAX.
  Solenoid:           AES-112 manufactured by Asahi Seiko.


****************************************************************************

  The waveform is 8bit mono unsigned at 8192Hz.
  Sampleset has sounds, music and voice at approximate rom offsets:

  $00c58-$038a4: "jan ken pon!"                   --> Is the call for rock paper and scissors.
  $04d2e-$05a4b: "zuko"                           --> Is just used for sound effect when player loses.
  $05b2d-$08207: "ai ko desho"                    --> Is the call for rematch when you've drawn.
  $08410-$0a9ec: "ooatari"                        --> "you got it! / perfect!".
  $0a9ec-$0c008: "yappii"                         --> Is just an exclamation of happiness.
  $0c008-$0dac0: "attarii"                        --> "you got it".

  unused PCM data:

  $15db7-$18628: "kakariin o oyobi kudasai"       --> "please call the attendant".
  $18628-$1a4f3: "keihin ga deru yo"              --> "your prize is incoming".
  $3c26d-$3f677: "keihin o sentaku shite kudasai" --> "please select your prize".


****************************************************************************

  About lamps...

  The internal layout has lamps mapped the following way:

  digits:

    0 = Left
    1 = Right

  lamps:

    00 = Multiplier 1 "attarii" (pays x1)
    01 = Multiplier 2 "ooatari" (pays x2)

    02 = Rock button LED
    03 = Scissors button LED
    04 = Paper button LED

    05 = Lose
    06 = Draw
    07 = Win

    08 = Base Hand
    09 = Paper components
    10 = Paper/Scissors common components
    11 = Rock components
    12 = Scissors components
    13 = Rock/Scissors common components

    14 = Payout error LED

  Not implemented in the internal layout/artwork:

    15 = Rotating blue lamp


***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/i8255.h"
#include "sound/dac.h"
#include "speaker.h"

#include "jankenmn.lh"


namespace {

#define MASTER_CLOCK        XTAL(2'500'000)


class jankenmn_state : public driver_device
{
public:
	jankenmn_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_digits(*this, "digit%u", 0U)
		, m_lamps(*this, "lamp%u", 0U)
	{ }

	int hopper_status_r();

	void jankenmn(machine_config &config);

private:
	void lamps1_w(uint8_t data);
	void lamps2_w(uint8_t data);
	void lamps3_w(uint8_t data);

	virtual void machine_start() override;

	void jankenmn_map(address_map &map);
	void jankenmn_port_map(address_map &map);

	required_device<z80_device> m_maincpu;
	output_finder<2> m_digits;
	output_finder<16> m_lamps;
};


/*********************************************
*            Read/Write Handlers             *
*********************************************/

static constexpr uint8_t led_map[16] = // 7748 IC?
	{ 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0x00 };

void jankenmn_state::lamps1_w(uint8_t data)
{
	// hand state: d0: rock, d1: scissors, d2: paper
	m_lamps[8] = (data & 7) != 0;
	m_lamps[11] = BIT(data, 0);
	m_lamps[12] = BIT(data, 1);
	m_lamps[9] = BIT(data, 2);
	m_lamps[10] = (data & 6) != 0;
	m_lamps[13] = (data & 3) != 0;

	// d4-d7: led7seg (remaining credits) right digit
	m_digits[1] = led_map[(data >> 4) & 0x0f];

	// d3: ? (only set if game is over)
}

void jankenmn_state::lamps2_w(uint8_t data)
{
	// button LEDs: d1: paper, d2: scissors, d3: rock
	m_lamps[2] = BIT(data, 3);
	m_lamps[3] = BIT(data, 2);
	m_lamps[4] = BIT(data, 1);

	// lamps: d5: draw, d6: lose, d7: win
	m_lamps[5] = BIT(data, 6);
	m_lamps[6] = BIT(data, 5);
	m_lamps[7] = BIT(data, 7);

	// d4: payout error LED
	m_lamps[14] = BIT(data, 4);

	// d0: led7seg (remaining credits) left digit
	m_digits[0] = led_map[data & 1];
}

void jankenmn_state::lamps3_w(uint8_t data)
{
	// d1: blue rotating lamp on top of cab
	m_lamps[15] = BIT(data, 1);

	// d2: payout (waits for hopper status)
	machine().bookkeeping().coin_counter_w(2, data & 0x04);

	// d3: right multiplier lamp(2), d4: left multiplier lamp(1)
	m_lamps[0] = BIT(data, 4);
	m_lamps[1] = BIT(data, 3);

	// d5: assume coin lockout
	machine().bookkeeping().coin_lockout_global_w(~data & 0x20);

	// d0, d6, d7: N/C?
	if (data & 0x04)
		logerror("payout: %02X\n", (data & 0x04));
}

int jankenmn_state::hopper_status_r()
{
	// temp workaround, needs hopper
	return machine().rand();
}

void jankenmn_state::machine_start()
{
	m_digits.resolve();
	m_lamps.resolve();
}


/*********************************************
*           Memory Map Definition            *
*********************************************/

void jankenmn_state::jankenmn_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc7ff).ram();
	map(0xe000, 0xffff).rom();
}

void jankenmn_state::jankenmn_port_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x10, 0x13).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x20, 0x23).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x30, 0x30).nopw(); // ???
}

/*
  Writes to port 30h....

  They are coming from different code chunks, but seems that at least
  they have different functions. Writes from 00B6h are unknown, whilst
  others coming from 00D6h are counters. Sometimes whilst one increase,
  the other one decrease. Writes coming from 0103h seems to clear (00)
  or just end whatever the command sent.

  Other behaviours could point to be different counters.

  Also when you win, and the multipliers start to run, a lot of data
  is written to the port. Maybe is a leftover, or just a connector to
  hook the multiplier's 'roulette style' matrix lamps for other Janken
  Man games...

  ':maincpu' (00B6): unmapped io memory write to 0030 = 01 & FF
  ':maincpu' (00D6): unmapped io memory write to 0030 = 2F & FF
  ':maincpu' (0103): unmapped io memory write to 0030 = 00 & FF

  ':maincpu' (00B6): unmapped io memory write to 0030 = F4 & FF
  ':maincpu' (00D6): unmapped io memory write to 0030 = 30 & FF
  ':maincpu' (0103): unmapped io memory write to 0030 = 00 & FF

  ':maincpu' (00B6): unmapped io memory write to 0030 = E7 & FF
  ':maincpu' (00D6): unmapped io memory write to 0030 = 31 & FF
  ':maincpu' (0103): unmapped io memory write to 0030 = 00 & FF

  ':maincpu' (00B6): unmapped io memory write to 0030 = DA & FF
  ':maincpu' (00D6): unmapped io memory write to 0030 = 32 & FF
  ':maincpu' (0103): unmapped io memory write to 0030 = 00 & FF

  ':maincpu' (00B6): unmapped io memory write to 0030 = CD & FF
  ':maincpu' (00D6): unmapped io memory write to 0030 = 33 & FF
  ':maincpu' (0103): unmapped io memory write to 0030 = 00 & FF


  Need more analysis...

*/

/*********************************************
*          Input Ports Definitions           *
*********************************************/

static INPUT_PORTS_START( jankenmn )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Guu (Rock)")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Choki (Scissors)")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Paa (Paper)")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN3 ) // 100 yen coin
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(jankenmn_state, hopper_status_r)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN2 ) // 10 yen coin
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 ) // 10 yen coin

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )  PORT_DIPLOCATION("DSW:7,8")
	PORT_DIPSETTING( 0x00, "Lamp Test" )
	PORT_DIPSETTING( 0x02, DEF_STR( 3C_1C ) )       // or 4 credits on 100 yen
	PORT_DIPSETTING( 0x01, DEF_STR( 2C_1C ) )       // or 6 credits on 100 yen
	PORT_DIPSETTING( 0x03, DEF_STR( 1C_1C ) )       // or 11 credits on 100 yen
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END


/*********************************************
*   Daisy Chain Interrupts Interface   *
*********************************************/

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ nullptr }
};


/*********************************************
*               Machine Config               *
*********************************************/

void jankenmn_state::jankenmn(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK);  /* 2.5 MHz */
	m_maincpu->set_daisy_config(daisy_chain);
	m_maincpu->set_addrmap(AS_PROGRAM, &jankenmn_state::jankenmn_map);
	m_maincpu->set_addrmap(AS_IO, &jankenmn_state::jankenmn_port_map);

	i8255_device &ppi0(I8255(config, "ppi8255_0"));
	/* (10-13) Mode 0 - Ports A & B set as input, high C & low C as output. */
	ppi0.in_pa_callback().set_ioport("DSW");
	ppi0.in_pb_callback().set_ioport("IN0");
	ppi0.out_pc_callback().set(FUNC(jankenmn_state::lamps3_w));

	i8255_device &ppi1(I8255(config, "ppi8255_1"));
	/* (20-23) Mode 0 - Ports A, B, high C & low C set as output. */
	ppi1.out_pa_callback().set("dac", FUNC(dac_byte_interface::data_w));
	ppi1.out_pb_callback().set(FUNC(jankenmn_state::lamps1_w));
	ppi1.out_pc_callback().set(FUNC(jankenmn_state::lamps2_w));

	z80ctc_device& ctc(Z80CTC(config, "ctc", MASTER_CLOCK));
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	/* NO VIDEO */

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	AD7523(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.5);
}


/*********************************************
*                  Rom Load                  *
*********************************************/

ROM_START( jankenmn )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "pcg2.bin",   0x0000, 0xc000, CRC(48a8f769) SHA1(656346ca0a83fd8ff5c8683152e4c5e1a1c797fa) )
	ROM_CONTINUE(           0xe000, 0x2000 )
	ROM_IGNORE( 0x32000 ) // lots of unused PCM data

	// Z80 code/data in $0000-$2b4f, rest is empty. CTC probably at 58-5b.
	// Can't find any sign of 8255 PPI. Is this an unused leftover or testrom?
	ROM_REGION( 0x4000, "temp", 0 )
	ROM_LOAD( "pcg1.bin",   0x0000, 0x4000,  CRC(a9c5aa2e) SHA1(c3b81eeefa5c442231cd26615aaf6c682063b26f) )
ROM_END

} // anonymous namespace


/*********************************************
*                Game Drivers                *
*********************************************/

//     YEAR  NAME      PARENT  MACHINE   INPUT     CLASS           INIT        ROT   COMPANY    FULLNAME                    FLAGS                  LAYOUT
GAMEL( 1991, jankenmn, 0,      jankenmn, jankenmn, jankenmn_state, empty_init, ROT0, "Sunwise", "Janken Man Kattara Ageru", MACHINE_SUPPORTS_SAVE, layout_jankenmn )
