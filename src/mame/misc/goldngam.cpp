// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/******************************************************************************

  GOLDEN GAMES / C+M TECHNICS AG
  ------------------------------

  Driver by Roberto Fresca.


  Games running on this hardware:

  * Swiss Poker ('50 SG-.10', V2.5), 1990, Golden Games / C+M Technics AG.
  * Movie Card,                      1998, Golden Games / C+M Technics AG.


*******************************************************************************


  Hardware Notes:
  ---------------

  Swiss Card CPU Board...
  Stickered "GOLDEN GAMES 288"

  CPU: 1x MC68000P8.               [IC2]
  SND: 1x Microchip AY38910A.      [IC18]
       1x LM380N (audio amp).

  RAM: 2x HM62256LP-10             [IC4, IC5]

  OTHER:
  1x EF6840P (timer)               [IC3]
  1x unknown GAL labeled "CM 1.2"  [IC8]
  1x 8.000 MHz Xtal.               [Q1]


  CPU Board layout:
  ,----------------------------------------------------------------------------------------.
  |  |!!!!!!!!|   |!!!!!!!!|    |!!!!!!!!|   |!!!!!!!!|    |!!!!!!!!!!|     |||||||||||    |
  |  '--------'   '--------'    '--------'   '--------'    '----------'     '---------'    |
  |                                                                                        |
  |                                                                                  .--.  |
  | .--------.    .--------.   .--------.    .--------.                              |LM|  |
  | |ULN2803A|    |ULN2803A|   |ULN2803A|    |ULN2803A|                              |38|  |
  | '--------'    '--------'   '--------'    '--------'                              |0N|  |
  |  .---------.     .---------.       .---------.  .--------. .--------. .--------. '--'  |
  |  |74HC273AP|     |74HC251AP|       |74HC273AP|  |ULN2003A| |ULN2803A| |HC251AP |       |
  |  '---------'     '---------'       '---------'  '--------' '--------' '--------'       |
  |                                                   .--------.  .---------.   .--------. |
  |                                                   |HC259AP |  |74HC273AP|   |ZH 436 E| |
  |                                                   '--------'  '---------'   '--------' |
  |                      GOLDEN GAMES 238                                                  |
  |                         CPU 68000                                     .--------------. |
  |                      .----.  .--.    .--.       .--------. .--.       |              | |
  |                      |ICL1|  |74|    |74|       |        | |..|       |  AY38912A/P  | |
  |                      '----'  |HC|    |HC|       |        | |..|       |              | |
  |  .---------. .---------..--. |13|    |14|       |        | |..|       '--------------' |
  |  |74HC273P | |74HC273AP||74| |8A|    |B1|       |        | |..|                        |
  |  '---------' '---------'|HC| |N |    |  |       |        | |..|       .--------------. |
  |                         |13| '--'    '--'       |        | |..|       |              | |
  |  .---------.            |8A|                    | MC     | |..|       | HM62256LP_10 | |
  |  |ULN2803A |            |N |         .--.       |        | |..|       |              | |
  |  '---------'            '--' .--.    |74|       | 68000  | |..|       '--------------' |
  |             .------.         |G |    |HC|       |        | |..|                        |
  |             |HC238B|    .--. |A |    |20|       |    P8  | |..|                        |
  |             '------'    |74| |L |    |P |       |        | |..|                        |
  |                         |HC| |  |    '--'       |        | |..|                        |
  |                         |13| |20|               |        | |..|       .--------------. |
  |                         |9N| |V8|    .--.       |        | |..|       |              | |
  |                         '--' |  |    |74|       |        | |..|       | HM62256LP_10 | |
  |                              '--'    |HC|       |        | |..|       |              | |
  |                                      |05|       |        | |..|       '--------------' |
  |                                      '--'       |        | |..|                        |
  |                       .--.                      |        | |..|       .--------------. |
  |                       |HC|   .------.           |        | |..|       |              | |
  |                       |16|   |HC32AP|           |        | |..|       |   EF6840P    | |
  |                       |1A|   '------'           '--------' '--'       |              | |
  |                       '--'   .------.                                 '--------------' |
  |                              |HC4066|   .-------.                                      |
  |                              '------'   |HC251AP|    .--.  .-------.                   |
  |                     .------. .------.   '-------'    |SG|  |74HC93P|    .------------. |
  |                     |HC251A| |HC251A|                |51|  '-------'    |            | |
  |                     '------' '------'                |  |               |  MC6850P   | |
  |                                                      '--'  .-------.    |            | |
  |                                                            |74HC08A|    '------------' |
  |                                                            '-------'    .---.          |
  |                                         .--------.                      |DS1|          |
  |                                         |HC244AP |                      '---'          |
  |                                         '--------'                                     |
  |   .------------------.   .---------.   .----------.   .---------.    .----------.      |
  |   |!!!!!!!!!!!!!!!!!!|   |!!!!!!!!!|   |!!!!!!!!!!|   |!!!!!!!!!|    |!!!!!!!!!!|      |
  '----------------------------------------------------------------------------------------'

  ICL1 = ICL7673CPA
  DS1  = DS1232
  SG51 = SG51K 0382 80000 MHz



  Swiss Card Graphics Board...
  Stickered "960210"

  2x Xilinx XC7336-15
  3x TC551001BPL-70L (128K x 8 static RAM).
  2x ROMs.
  1x 3x32 Female connector.
  1x 6.000 MHz. Xtal.


  Graphics Board layout:

  .-------------------------------------------------------------------------.
  |                                                                         |
  |                                                               .-------. |
  | .--. .---------.                                              |V2.5_HI| |
  | |74| | XILINX  |                                              |.PR    | |
  | |HC| |XC7336-15|                                              |       | |
  | |12| |  PC44C  |                                              |       | |
  | |5N| | X64654M |  .-----------------------------------------. |       | |
  | |  | | ACG9539 |  |:::::::::::::::::::::::::::::::::::::::::| |       | |
  | '--' '---------'  '-----------------------------------------' |       | |
  |                                                               |       | |
  |      .--.                                                     |27C256 | |
  |      |74|                                                     '-------' |
  |      |HC|                                                               |
  |.--.  |27|         .---------.    .--. .--.  .--. .--.                   |
  ||..|  |3N|         |IQXO-100C|    |74| |74|  |74| |74|         .-------. |
  ||..|  |  |         |6.000 MHz|    |HC| |HC|  |HC| |HC|         |V2.5_LO| |
  ||..|  |  |         '---------'    |24| |24|  |24| |24|         |.PR    | |
  ||..|  |  |                        |4N| |5N|  |4N| |5N|         |       | |
  |'--'  '--'                   .--. |  | |  |  |  | |  |  .--.   |       | |
  |                             |74| |  | |  |  |  | |  |  |74|   |       | |
  | .-------.   .---------.     |HC| '--' '--'  '--' '--'  |HC|   |       | |
  | |TOSHIBA|   | XILINX  |     |24|                       |24|   |       | |
  | |       |   |XC7336-15|     |4N|   .-------..-------.  |4N|   |       | |
  | |TC     |   |  PC44C  |     |  |   |TOSHIBA||TOSHIBA|  |  |   |27C256 | |
  | |551001 |   | A40490A |     |  |   |       ||       |  '--'   '-------' |
  | |BPL-70L|   | ACG9541 |     '--'   |TC     ||TC     |                   |
  | |       |   '---------'            |551001 ||551001 |                   |
  | |       |                   .--.   |BPL-70L||BPL-70L|  .--.             |
  | |       |                   |74|   |       ||       |  |74|             |
  | |       |                   |HC|   |       ||       |  |HC|             |
  | |       |                   |24|   |       ||       |  |24|             |
  | |       |                   |4N|   |       ||       |  |4N|             |
  | '-------'                   |  |   |       ||       |  |  |             |
  |                             |  |   |       ||       |  |  |             |
  |                             '--'   '-------''-------'  '--'             |
  '-------------------------------------------------------------------------'


  Swiss Card Program Module...

  2x (LO-HI) ROMs.
  1x PALC22V10D.
  1x 3x32 Male connector.

  Program Board layout
  .--------------------------------------.
  |                                      |
  | .--------------.    .--------------. |
  | |V1.0_LO.GR    |    |V1.0_HI.GR    | |
  | |              |    |              | |
  | |        27C512|    |        27C512| |
  | '--------------'    '--------------' |
  |                                      |
  |          .---------------.           |
  |    LOW   |PALC22V10D-15PC|    HIGH   |
  |          '---------------'           |
  |--------------------------------------|
  ||||||||||||||||||||||||||||||||||||||||
  '--------------------------------------'


*******************************************************************************


  Movie Card CPU Board...
  Etched: "Golden Games"
  Stickered "JQ 6.01"

  CPU: 1x MC68000P8.
  SND: 1x YM2149F.
       1x LM380N (audio amp).

  RAM: 2x 84256A-10

  OTHER:
  1x EF6840P (timer)
  1x Xilinx XC9572 PC84AEM9917 (In-System Programmable CPLD)
  2x Philips SCN68681C1A44 (Dual asynchronous receiver/transmitter)

  1x DS1307

  1x 8.000 MHz Xtal.
  1x Unreadable Xtal.
  1x Unreadable Xtal.


*******************************************************************************


  *** Game Notes ***




*******************************************************************************

  --------------------
  ***  Memory Map  ***
  --------------------


*******************************************************************************


  DRIVER UPDATES:


  [2010-09-12]

  - Initial release.
  - Added technical notes.


  TODO:



*******************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/6840ptm.h"
#include "machine/6850acia.h"
#include "machine/mc68681.h"
#include "sound/ay8910.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

#define MASTER_CLOCK    XTAL(8'000'000)   /* from CPU Board */
#define SECONDARY_CLOCK XTAL(6'000'000)   /* from GFX Board */


class goldngam_state : public driver_device
{
public:
	goldngam_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_ptm(*this, "ptm"),
		m_duart(*this, "duart%u", 1U) { }

	void swisspkr(machine_config &config);
	void moviecrd(machine_config &config);

protected:
	void base(machine_config &config);

private:
	static constexpr int MOVIECRD_DUART1_IRQ = M68K_IRQ_2;
	static constexpr int MOVIECRD_DUART2_IRQ = M68K_IRQ_4;

	uint8_t unk_r();
	virtual void video_start() override ATTR_COLD;
	void palette_init(palette_device &palette);
	uint32_t screen_update_goldngam(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void cpu_space_map(address_map &map) ATTR_COLD;

	void moviecrd_map(address_map &map) ATTR_COLD;
	void swisspkr_map(address_map &map) ATTR_COLD;

	required_shared_ptr<uint16_t> m_videoram;
	required_device<cpu_device> m_maincpu;
	required_device<ptm6840_device> m_ptm;
	optional_device_array<mc68681_device, 2> m_duart;
};

constexpr int goldngam_state::MOVIECRD_DUART1_IRQ;
constexpr int goldngam_state::MOVIECRD_DUART2_IRQ;

/*************************
*     Video Hardware     *
*************************/

void goldngam_state::video_start()
{
}

uint32_t goldngam_state::screen_update_goldngam(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int index = 0;

	for(int y = 0; y < 512; ++y)
	{
		for(int x = 0; x < 384; x += 2)
		{
			uint16_t word = m_videoram[index];
			bitmap.pix(y, x) = word >> 8;
			bitmap.pix(y, x+1) = word & 0xff;
			++index;
		}
	}

	return 0;
}


void goldngam_state::palette_init(palette_device &palette)
{
}



/*************************
* Memory Map Information *
*************************/

uint8_t goldngam_state::unk_r()
{
	// hopper status read ?
	return 1;
}

void goldngam_state::swisspkr_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x200000, 0x20ffff).ram();
	map(0x400000, 0x40000f).rw("ptm", FUNC(ptm6840_device::read), FUNC(ptm6840_device::write)).umask16(0xff00);
	map(0x401000, 0x401003).rw("acia", FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0xff00);
	map(0x402001, 0x402001).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x402000, 0x402003).w("aysnd", FUNC(ay8910_device::address_data_w)).umask16(0x00ff); //wrong
	map(0x500100, 0x500101).nopr(); //?
	map(0x500200, 0x500201).portr("IN0");
	map(0x500202, 0x500203).portr("IN1");
	map(0x500204, 0x500205).portr("IN2");
	map(0x500206, 0x500207).portr("IN3");
	map(0x500208, 0x500209).nopr(); //?
	map(0x50020c, 0x50020d).nopr(); //?
	map(0x500300, 0x500301).nopr(); //?
	map(0x50030f, 0x50030f).r(FUNC(goldngam_state::unk_r));
	map(0x501500, 0x501501).nopw(); //?
	map(0x503000, 0x503001).ram(); //int ack ?
	map(0x503002, 0x503003).ram(); //int ack ?
	map(0x503006, 0x503007).ram(); //int ack ?
	map(0xc00000, 0xc3ffff).ram().share("videoram");
	map(0xe00000, 0xe00001).noprw(); //?
}


/* unknown R/W:

'maincpu' (0000C8): unmapped program memory word write to 501500 = 0040 & 00FF
'maincpu' (01BDB6): unmapped program memory word write to 401000 = 0300 & FF00
'maincpu' (01BDBE): unmapped program memory word write to 401000 = 5500 & FF00
'maincpu' (01BDC4): unmapped program memory word read from 401002 & FF00
'maincpu' (01BB82): unmapped program memory word write to 402000 = 0008 & 00FF
'maincpu' (01BB88): unmapped program memory word write to 402002 = 0000 & 00FF
'maincpu' (01BB82): unmapped program memory word write to 402000 = 0009 & 00FF
'maincpu' (01BB88): unmapped program memory word write to 402002 = 0000 & 00FF
'maincpu' (01BB82): unmapped program memory word write to 402000 = 000A & 00FF
'maincpu' (01BB88): unmapped program memory word write to 402002 = 0000 & 00FF
'maincpu' (01BA5E): unmapped program memory word write to 402000 = 0007 & 00FF
'maincpu' (01BA66): unmapped program memory word write to 402002 = 007F & 00FF
'maincpu' (01C072): unmapped program memory word write to 400002 = 0100 & FF00
'maincpu' (01C076): unmapped program memory word write to 400000 = 8200 & FF00
'maincpu' (01C07E): unmapped program memory word write to 400004 = 0B00 & FF00
'maincpu' (01C07E): unmapped program memory word write to 400006 = B700 & FF00
'maincpu' (01C084): unmapped program memory word write to 400002 = 0000 & FF00
'maincpu' (01C088): unmapped program memory word write to 400000 = 8300 & FF00
'maincpu' (01C090): unmapped program memory word write to 40000C = FF00 & FF00
'maincpu' (01C090): unmapped program memory word write to 40000E = FF00 & FF00
'maincpu' (01C096): unmapped program memory word write to 400002 = C200 & FF00
'maincpu' (01C09E): unmapped program memory word write to 400008 = 0600 & FF00
'maincpu' (01C09E): unmapped program memory word write to 40000A = 4000 & FF00
'maincpu' (0160DC): unmapped program memory word write to 501500 = 0040 & 00FF
'maincpu' (015102): unmapped program memory word read from 500400 & 00FF
'maincpu' (0151BE): unmapped program memory word write to 401000 = 0300 & FF00
'maincpu' (0151C6): unmapped program memory word write to 401000 = 1500 & FF00
'maincpu' (0151CC): unmapped program memory word read from 401002 & FF00

*/

void goldngam_state::cpu_space_map(address_map &map)
{
	map(0xfffff0, 0xffffff).m(m_maincpu, FUNC(m68000_base_device::autovectors_map));
	map(0xfffff4, 0xfffff5).lr16(NAME([this] () -> u16 { return m_duart[0]->get_irq_vector(); }));
	map(0xfffff8, 0xfffff9).lr16(NAME([this] () -> u16 { return m_duart[1]->get_irq_vector(); }));
}

void goldngam_state::moviecrd_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x200000, 0x20ffff).ram();
	map(0x502100, 0x502103).w("aysnd", FUNC(ym2149_device::address_data_w)).umask16(0x00ff);
	map(0x503000, 0x50301f).rw("duart1", FUNC(mc68681_device::read), FUNC(mc68681_device::write)).umask16(0x00ff);
	map(0x503100, 0x50311f).rw("duart2", FUNC(mc68681_device::read), FUNC(mc68681_device::write)).umask16(0x00ff);
	map(0x504000, 0x50400f).rw("ptm", FUNC(ptm6840_device::read), FUNC(ptm6840_device::write)).umask16(0x00ff);
	map(0x505000, 0x505001).nopw(); //int ack ?
	map(0xc00000, 0xc3ffff).ram().share("videoram");
}

/*************************
*      Input Ports       *
*************************/

static INPUT_PORTS_START( goldngam )

	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q) PORT_NAME("IN0-01")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W) PORT_NAME("IN0-02")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("IN0-04")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_NAME("IN0-08")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("IN0-10")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y) PORT_NAME("IN0-20")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U) PORT_NAME("IN0-40")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I) PORT_NAME("IN0-80")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A) PORT_NAME("IN1-01")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("IN1-02")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("IN1-04")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) PORT_NAME("IN1-08")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G) PORT_NAME("IN1-10")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("IN1-20")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J) PORT_NAME("IN1-40")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K) PORT_NAME("IN1-80")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z) PORT_NAME("IN2-01")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X) PORT_NAME("IN2-02")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C) PORT_NAME("IN2-04")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V) PORT_NAME("IN2-08")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B) PORT_NAME("IN2-10")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N) PORT_NAME("IN2-20")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M) PORT_NAME("IN2-40")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L) PORT_NAME("IN2-80")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1) PORT_NAME("IN3-01")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2) PORT_NAME("IN3-02")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("IN3-04")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("IN3-08")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5) PORT_NAME("IN3-10")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6) PORT_NAME("IN3-20")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7) PORT_NAME("IN3-40")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8) PORT_NAME("IN3-80")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0001, 0x0001, "switch 2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

INPUT_PORTS_END


/*************************
*    Graphics Layouts    *
*************************/

/* Used 1bpp gfx just to see the tiles.
   See tiles 1d30 onward...
*/

static const gfx_layout charlayout =
{
	8, 8,
	RGN_FRAC(1,2),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( gfx_goldngam )
	GFXDECODE_ENTRY( "maincpu", 0, charlayout, 0, 16 )
GFXDECODE_END


/*************************
*    Machine Drivers     *
*************************/

void goldngam_state::base(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, MASTER_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &goldngam_state::swisspkr_map);

	PTM6840(config, m_ptm, 2'000'000);
	m_ptm->irq_callback().set_inputline("maincpu", M68K_IRQ_2);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(4*8, 43*8-1, 1*8, 37*8-1); // 312x288
	screen.set_screen_update(FUNC(goldngam_state::screen_update_goldngam));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_goldngam);

	PALETTE(config, "palette", FUNC(goldngam_state::palette_init), 512);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
}

void goldngam_state::swisspkr(machine_config &config)
{
	base(config);

	ACIA6850(config, "acia", 0).irq_handler().set_inputline("maincpu", M68K_IRQ_4);
	AY8912(config, "aysnd", MASTER_CLOCK/4).add_route(ALL_OUTPUTS, "mono", 1.00);
}

void goldngam_state::moviecrd(machine_config &config)
{
	base(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &goldngam_state::moviecrd_map);
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &goldngam_state::cpu_space_map);

	m_ptm->irq_callback().set_inputline("maincpu", M68K_IRQ_1);

	MC68681(config, m_duart[0], 3'686'400);
	m_duart[0]->irq_cb().set_inputline("maincpu", MOVIECRD_DUART1_IRQ);

	MC68681(config, m_duart[1], 3'686'400);
	m_duart[1]->irq_cb().set_inputline("maincpu", MOVIECRD_DUART2_IRQ);

	YM2149(config, "aysnd", MASTER_CLOCK/4).add_route(ALL_OUTPUTS, "mono", 1.00);
}


/*************************
*        Rom Load        *
*************************/

ROM_START( swisspkr )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "v2.5_hi.pr", 0x00000, 0x10000, CRC(a7f85661) SHA1(aa307bcfe0dfb07120b9711d65916b8689626b00) )
	ROM_LOAD16_BYTE( "v2.5_lo.pr", 0x00001, 0x10000, CRC(142db5d0) SHA1(cc6481a206ed1b0f19cccaab7d6158e81e483c9b) )

	ROM_REGION( 0x40000, "gfx1", 0 )    /* The following ROMs have code for 'Super Cherry' */
	ROM_LOAD16_BYTE( "v1.0_hi.gr", 0x20000, 0x10000, CRC(ea750ab1) SHA1(d1284e7f2628c3aa3de9246e475d45e6be48890e) )
	ROM_LOAD16_BYTE( "v1.0_lo.gr", 0x20001, 0x10000, CRC(d885b965) SHA1(5f2ae3e21cf4e0d20c99cec2dfd3a6f72358535a) )
ROM_END

ROM_START( moviecrd )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "v1.2_hi.pro", 0x20000, 0x20000, CRC(2210dd79) SHA1(007e9930965d6281889fca487f00a1edaac54d85) )
	ROM_CONTINUE(                   0x00000, 0x20000)
	ROM_LOAD16_BYTE( "v1.2_lo.pro", 0x20001, 0x20000, CRC(adb6060f) SHA1(1aff8be7830f61f97720e773eab7985956c7569d) )
	ROM_CONTINUE(                   0x00001, 0x20000)

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "v1.2_hi.gfx", 0x00000, 0x10000, CRC(0b62d1a8) SHA1(4156379cc000cbea997b1c21cebea9021fa697b2) )
	ROM_LOAD16_BYTE( "v1.2_lo.gfx", 0x00001, 0x10000, CRC(70e8e9d5) SHA1(c026493b4bd302d389219ba564aafa42fca86491) )
ROM_END

} // anonymous namespace


/*************************
*      Game Drivers      *
*************************/

//    YEAR  NAME      PARENT    MACHINE    INPUT     STATE           INIT        ROT   COMPANY                           FULLNAME                           FLAGS
GAME( 1990, swisspkr, 0,        swisspkr,  goldngam, goldngam_state, empty_init, ROT0, "Golden Games / C+M Technics AG", "Swiss Poker ('50 SG-.10', V2.5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 1998, moviecrd, 0,        moviecrd,  goldngam, goldngam_state, empty_init, ROT0, "Golden Games / C+M Technics AG", "Movie Card",                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
