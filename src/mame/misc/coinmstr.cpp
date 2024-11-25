// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Roberto Fresca, David Haywood
/*==================================================================================

  Coinmaster trivia and poker games.

  Preliminary driver by Pierpaolo Prazzoli
  Additional work by Roberto Fresca.


  TODO:
  - is there extra colour info in attr3 for suprnudge 2 and pokeroul?
  - is there colour intensity control?
  - finish inputs
  - finish question roms reading
  - hook up all the PIAs


====================================================================================

  Technical notes....


  There are at least 2 different boards.

  A) Unknown, with 2x 6264 RAM, mapped $C000-$DFFF and $E000-$FFFF.
  B) 'PCB-001-POK', with 3x 6116 RAM, mapped $E000-$E7FF, $E800-$EFFF, and $F000-$F7FF.

  - All of them seems to have a connector for expansion (as Question Boards).

  - The I/O ports are a mess. Devices could be mapped in different ways.

  - Some trivia seems to accept 2 type of eproms for question roms:
    0x4000 or 0x8000 bytes long. This check is done with the 1st read
    from the rom (I think from offset 0) and if it's 0x10, it means a
    0x4000 bytes eprom or if it's 0x20, it means a 0x8000 one.
    Also supnudg2 only tests 0x20 as 1st byte, so accepting only
    the 2nd type of eproms.


  * Coinmaster Joker Poker (PCB-001-POK)

  AY-3-8912
  ---------

  - IO pins connected to DIP switches bank.
  - Data pin connected to CPU data pin.
  - BC1 goes to PAL IC12 pin 12
  - BC2 +5V
  - BDIR goes to PAL IC12 pin 13
  - A8 +5V

  PIAs
  ----

  PIA0 (IC24) Port A --> Input.
  PIA0 (IC24) Port B --> Output.

  PB output
  PB0 to PB6 go to ULN2003 (IC19) then on PCB connector.
  PB7 goes to ULM2003 (IC40) pin 1 then on PCB connector.


  PIA1 (IC39) Port A --> Output.
  PIA1 (IC39) Port B --> Output.

  PA0-PA7 go to 22 KOhm resistor, then a "pull up" capacitor, and then into the base of a transistor.
          Collector connected to +5V, emitter is the output (1 KOhm pulldown), it goes into an ULN2803
          and then to PCB connector. 3 of that transistors outputs (input of the ULN) are connected
          together and connected to another circuit that generate 2 more outputs on PCB connector.
         (them seem unused no solder on the pcb connector)

  PB0 to PB6 goes to ULN2003 (IC34) then on PCB connector.
  PB7 goes to ULM2003 (IC40) pin 2 then on PCB connector.


  PIA2 (IC32) Port A --> Input.
  PIA2 (IC32) Port B --> Output.

  PB0 to PB6 go to ULN2003 (IC31) then on PCB connector.
  PB7 goes to ULM2003 (IC40) pin 3 then on PCB connector.


====================================================================================

  Notes by game....

  * Coinmaster Joker Poker

  - For Joker Poker (set 2), to start, pulse the KEY OUT (W) to wipe
    the credits set at boot stage and reset the game. Otherwise you'll
    get 116 credits due to input inconsistencies.

  DIP switch #1 changes the minimal hand between "Jacks or Better" and
  "Pair of Aces".

  There are two bookkeeping modes. I think these are for different levels
  like operator and manager/supervisor. With the DIP switch #4 you can
  switch between them. Is possible that this input would be meant to be
  routed to a real supervisor key.

  Bookkeeping Types:

  "Weekly Meters"

  Just the operator bookkeeping mode. Only weekly credits in and out.
  Erasable with CANCEL button. HOLD 5 shows the settings status.

  "Meter Totals"

  A complete bookkeeping, not erasable. Credits in/out, gamble in/out,
  and complete statistics. In the principal bookkeeping screen, HOLD 1
  brings up a sort of values, and HOLD 4 shows the current stake limit
  (keep pressing HOLD 4, and press HIGH and LOW to set the stake limit
  between 10-100).

  Pressing DEAL/START, you can get the winning hands, occurrence of
  spades, diamonds, clubs and hearts. also number of jokers dealt.

  With DIP switch #8 ON, you can enter a sort of test mode, where you
  can set the cards using the HOLD buttons, and test the winning hands.

  DIP switch #7 is the 'Factory Install Switch'. It behaves like a PC
  CMOS eraser jumper. Turning it ON and then OFF, you will erase the
  'Meter Totals' (all permanent meters and statistics).


==================================================================================*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/6821pia.h"
#include "video/mc6845.h"
#include "sound/ay8910.h"
#include "machine/nvram.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

#define MASTER_CLOCK    XTAL(14'000'000)
#define CPU_CLOCK      (MASTER_CLOCK/4)
#define SND_CLOCK      (MASTER_CLOCK/8)


class coinmstr_state : public driver_device
{
public:
	coinmstr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_attr_ram1(*this, "attr_ram1"),
		m_attr_ram2(*this, "attr_ram2"),
		m_attr_ram3(*this, "attr_ram3"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	void coinmstr(machine_config &config);
	void pokeroul(machine_config &config);
	void supnudg2(machine_config &config);
	void jpcoin(machine_config &config);
	void jpjcoin(machine_config &config);
	void quizmstr(machine_config &config);
	void trailblz(machine_config &config);

	void init_coinmstr();

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_attr_ram1;
	required_shared_ptr<uint8_t> m_attr_ram2;
	required_shared_ptr<uint8_t> m_attr_ram3;

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_question_adr[4]{};

	void quizmstr_bg_w(offs_t offset, uint8_t data);
	void quizmstr_attr1_w(offs_t offset, uint8_t data);
	void quizmstr_attr2_w(offs_t offset, uint8_t data);
	void quizmstr_attr3_w(offs_t offset, uint8_t data);
	uint8_t question_r();
	void question_w(offs_t offset, uint8_t data);
	uint8_t ff_r();
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void coinmstr_map(address_map &map) ATTR_COLD;
	void jpcoin_io_map(address_map &map) ATTR_COLD;
	void jpcoin_map(address_map &map) ATTR_COLD;
	void jpjcoin_io_map(address_map &map) ATTR_COLD;
	void pokeroul_io_map(address_map &map) ATTR_COLD;
	void quizmstr_io_map(address_map &map) ATTR_COLD;
	void supnudg2_io_map(address_map &map) ATTR_COLD;
	void trailblz_io_map(address_map &map) ATTR_COLD;
};


void coinmstr_state::quizmstr_bg_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;

	if(offset >= 0x0240)
		m_bg_tilemap->mark_tile_dirty(offset - 0x0240);
}


static void coinmstr_set_pal(palette_device &palette, uint32_t paldat, int col)
{
	col = col *4;

	{
		int r0, r1, r2, r3;
		int g0, g1, g2, g3;
		int b0, b1, b2, b3;

		r0 = (paldat & 0x1000) >> 12 ;
		g0 = (paldat & 0x0800) >> 11 ;
		b0 = (paldat & 0x0400) >> 10 ;
		r1 = (paldat & 0x0200) >> 9 ;
		g1 = (paldat & 0x0100) >> 8 ;
		b1 = (paldat & 0x0080) >> 7 ;

		r2 = (paldat & 0x0020) >> 5 ;
		g2 = (paldat & 0x0010) >> 4 ;
		b2 = (paldat & 0x0008) >> 3 ;
		r3 = (paldat & 0x0004) >> 2 ;
		g3 = (paldat & 0x0002) >> 1 ;
		b3 = (paldat & 0x0001) >> 0 ;


		palette.set_pen_color(col+0, (b0 * 255) << 5, (g0 * 255) << 5, (r0 * 255) << 5);
		palette.set_pen_color(col+2, (b1 * 255) << 5, (g1 * 255) << 5, (r1 * 255) << 5);
		palette.set_pen_color(col+1, (b2 * 255) << 5, (g2 * 255) << 5, (r2 * 255) << 5);
		palette.set_pen_color(col+3, (b3 * 255) << 5, (g3 * 255) << 5, (r3 * 255) << 5);

	}
}


void coinmstr_state::quizmstr_attr1_w(offs_t offset, uint8_t data)
{
	m_attr_ram1[offset] = data;

	if(offset >= 0x0240)
	{
		// the later games also use attr3 for something..
		uint32_t  paldata = (m_attr_ram1[offset] & 0x7f) | ((m_attr_ram2[offset] & 0x7f) << 7);
		m_bg_tilemap->mark_tile_dirty(offset - 0x0240);

		coinmstr_set_pal(*m_palette, paldata, offset - 0x240);

	}
}

void coinmstr_state::quizmstr_attr2_w(offs_t offset, uint8_t data)
{
	m_attr_ram2[offset] = data;

	if(offset >= 0x0240)
	{
		// the later games also use attr3 for something..
		uint32_t  paldata = (m_attr_ram1[offset] & 0x7f) | ((m_attr_ram2[offset] & 0x7f) << 7);
		m_bg_tilemap->mark_tile_dirty(offset - 0x0240);

		coinmstr_set_pal(*m_palette, paldata, offset - 0x240);

	}
}

void coinmstr_state::quizmstr_attr3_w(offs_t offset, uint8_t data)
{
	m_attr_ram3[offset] = data;

	if(offset >= 0x0240)
		m_bg_tilemap->mark_tile_dirty(offset - 0x0240);

}


uint8_t coinmstr_state::question_r()
{
	int address;
	uint8_t *questions = memregion("user1")->base();

	switch(m_question_adr[2])
	{
		case 0x38: address = 0x00000; break; // m_question_adr[3] == 7
		case 0x39: address = 0x08000; break; // m_question_adr[3] == 7
		case 0x3a: address = 0x10000; break; // m_question_adr[3] == 7
		case 0x3b: address = 0x18000; break; // m_question_adr[3] == 7
		case 0x3c: address = 0x20000; break; // m_question_adr[3] == 7
		case 0x3d: address = 0x28000; break; // m_question_adr[3] == 7
		case 0x3e: address = 0x30000; break; // m_question_adr[3] == 7
		case 0x07: address = 0x38000; break; // m_question_adr[3] == 7
		case 0x0f: address = 0x40000; break; // m_question_adr[3] == 7
		case 0x17: address = 0x48000; break; // m_question_adr[3] == 7
		case 0x1f: address = 0x50000; break; // m_question_adr[3] == 7
		case 0x27: address = 0x58000; break; // m_question_adr[3] == 7
		case 0x2f: address = 0x60000; break; // m_question_adr[3] == 7
		case 0x37: address = 0x68000; break; // m_question_adr[3] == 7
		case 0x3f: address = 0x70000 + m_question_adr[3] * 0x8000; break;

		default:
			address = 0;
			logerror("unknown question rom # = %02X\n",m_question_adr[2]);
	}

	if(m_question_adr[3] == 6 || m_question_adr[3] > 7)
		logerror("question_adr[3] = %02X\n",m_question_adr[3]);

/*
    in these offsets they set 0x80... why?

    if( (m_question_adr[0] & 0x5f) == 0x00 ||
        (m_question_adr[0] & 0x5f) == 0x01 ||
        (m_question_adr[0] & 0x5f) == 0x0f ||
        (m_question_adr[0] & 0x5f) == 0x56 )
*/


//  don't know...
//  address |= ((m_question_adr[0] & 0x7f) << 8) | m_question_adr[1];
	address |= (m_question_adr[1] << 7) | (m_question_adr[0] & 0x7f);

	return questions[address];
}

void coinmstr_state::question_w(offs_t offset, uint8_t data)
{
	if(data != m_question_adr[offset])
	{
		logerror("offset = %d data = %02X\n",offset,data);
	}

	m_question_adr[offset] = data;
}

uint8_t coinmstr_state::ff_r()
{
	return 0xff;
}

// Common memory map

void coinmstr_state::coinmstr_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xe7ff).ram().w(FUNC(coinmstr_state::quizmstr_bg_w)).share("videoram");
	map(0xe800, 0xefff).ram().w(FUNC(coinmstr_state::quizmstr_attr1_w)).share("attr_ram1");
	map(0xf000, 0xf7ff).ram().w(FUNC(coinmstr_state::quizmstr_attr2_w)).share("attr_ram2");
	map(0xf800, 0xffff).ram().w(FUNC(coinmstr_state::quizmstr_attr3_w)).share("attr_ram3");
}

/* 2x 6462 hardware C000-DFFF & E000-FFFF */
void coinmstr_state::jpcoin_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xdfff).ram();     /* 2x 6462 hardware */
	map(0xe000, 0xe7ff).ram().w(FUNC(coinmstr_state::quizmstr_bg_w)).share("videoram");
	map(0xe800, 0xefff).ram().w(FUNC(coinmstr_state::quizmstr_attr1_w)).share("attr_ram1");
	map(0xf000, 0xf7ff).ram().w(FUNC(coinmstr_state::quizmstr_attr2_w)).share("attr_ram2");
	map(0xf800, 0xffff).ram().w(FUNC(coinmstr_state::quizmstr_attr3_w)).share("attr_ram3");
}

// Different I/O mappping for every game

void coinmstr_state::quizmstr_io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x00).r(FUNC(coinmstr_state::question_r));
	map(0x00, 0x03).w(FUNC(coinmstr_state::question_w));
	map(0x40, 0x41).w("aysnd", FUNC(ay8910_device::address_data_w));
	map(0x41, 0x41).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x48, 0x4b).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x50, 0x53).nopr();
	map(0x50, 0x53).nopw();
	map(0x58, 0x5b).rw("pia2", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x70, 0x70).w("crtc", FUNC(mc6845_device::address_w));
	map(0x71, 0x71).w("crtc", FUNC(mc6845_device::register_w));
	map(0xc0, 0xc3).nopr();
	map(0xc0, 0xc3).nopw();
}

void coinmstr_state::trailblz_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r(FUNC(coinmstr_state::question_r));
	map(0x00, 0x03).w(FUNC(coinmstr_state::question_w));
	map(0x40, 0x40).w("crtc", FUNC(mc6845_device::address_w));
	map(0x41, 0x41).w("crtc", FUNC(mc6845_device::register_w));
	map(0x48, 0x49).w("aysnd", FUNC(ay8910_device::address_data_w));
	map(0x49, 0x49).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x50, 0x53).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write)); //?
	map(0x60, 0x63).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x70, 0x73).rw("pia2", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xc1, 0xc3).nopw();
}

void coinmstr_state::supnudg2_io_map(address_map &map)
{
/*
out 40
in  40
in  43
out 43
out 42

out 48  CRTC
out 49  CRTC

in  53
out 53
out 52
out 50

in  69
out 69
out 68
in  6B
out 6B
out 6A

out C1
out C2
out C3

E0-E1 CRTC
*/
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x00).r(FUNC(coinmstr_state::question_r));
	map(0x00, 0x03).w(FUNC(coinmstr_state::question_w));
	map(0x48, 0x48).w("crtc", FUNC(mc6845_device::address_w));
	map(0x49, 0x49).w("crtc", FUNC(mc6845_device::register_w));
	map(0x40, 0x43).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x50, 0x53).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x68, 0x6b).rw("pia2", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x78, 0x79).w("aysnd", FUNC(ay8910_device::address_data_w));
	map(0x79, 0x79).r("aysnd", FUNC(ay8910_device::data_r));
	map(0xc0, 0xc1).nopr();
	map(0xc0, 0xc3).nopw();
}

void coinmstr_state::pokeroul_io_map(address_map &map)
{
/*
out 68
in  69

in  6B
out 6B
out 6A

in  59
out 59
out 58
out 5B
out 5A

in  7B
out 7B
out 7A

E0-E1 CRTC
*/
	map.global_mask(0xff);
	map(0x40, 0x40).w("crtc", FUNC(mc6845_device::address_w));
	map(0x41, 0x41).w("crtc", FUNC(mc6845_device::register_w));
	map(0x48, 0x49).w("aysnd", FUNC(ay8910_device::address_data_w));
	map(0x49, 0x49).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x58, 0x5b).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write)); /* confirmed */
	map(0x68, 0x6b).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write)); /* confirmed */
	map(0x78, 0x7b).rw("pia2", FUNC(pia6821_device::read), FUNC(pia6821_device::write)); /* confirmed */
	map(0xc0, 0xc1).r(FUNC(coinmstr_state::ff_r));  /* needed to boot */
}

void coinmstr_state::jpcoin_io_map(address_map &map)
{
/*
out C0
in  C1

in  C8
out CA

in  D0
out D1
out D2

in  DA
out DA

E0-E1 CRTC
*/
	map.global_mask(0xff);
	map(0xe0, 0xe0).w("crtc", FUNC(mc6845_device::address_w));           /* confirmed */
	map(0xe1, 0xe1).w("crtc", FUNC(mc6845_device::register_w));          /* confirmed */
	map(0xc0, 0xc1).w("aysnd", FUNC(ay8910_device::address_data_w));
	map(0xc1, 0xc1).r("aysnd", FUNC(ay8910_device::data_r));
	map(0xc8, 0xcb).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));    /* confirmed */
	map(0xd0, 0xd3).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xd8, 0xdb).rw("pia2", FUNC(pia6821_device::read), FUNC(pia6821_device::write));    /* confirmed */
//  map(0xc0, 0xc1).r(FUNC(coinmstr_state::ff_r));  /* needed to boot */
	map(0xc4, 0xc4).r(FUNC(coinmstr_state::ff_r));  /* needed to boot */
}

void coinmstr_state::jpjcoin_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x10, 0x13).rw("pia2", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x20, 0x23).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x28, 0x28).w("crtc", FUNC(mc6845_device::address_w));
	map(0x29, 0x29).w("crtc", FUNC(mc6845_device::register_w));
	map(0x38, 0x39).w("aysnd", FUNC(ay8910_device::address_data_w));
	map(0x39, 0x39).r("aysnd", FUNC(ay8910_device::data_r));
}


static INPUT_PORTS_START( quizmstr )
	PORT_START("PIA0.A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1) PORT_NAME("Bookkeeping") PORT_TOGGLE /* Button 2 for second page, Button 3 erases data */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE) PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 )

	PORT_START("PIA0.B")
	PORT_DIPNAME( 0x01, 0x01, "PIA0.B" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("PIA1.A")
	PORT_DIPNAME( 0x01, 0x01, "PIA1.A" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("PIA1.B")
	PORT_DIPNAME( 0x01, 0x01, "PIA1.B" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("PIA2.A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	/* If 0x40 is HIGH the DIP Test Mode does work but bookkeeping shows always 0's */
	/* If 0x40 is LOW Bookkeeping does work, but the second page (selected categories) is missing */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("PIA2.B")
	PORT_DIPNAME( 0x01, 0x01, "PIA2.B" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x02, 0x02, "4-02" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Test Mode" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "NVRAM Reset?" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Self Test" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ))
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ))
INPUT_PORTS_END

static INPUT_PORTS_START( trailblz )
	PORT_START("PIA0.A")
	PORT_DIPNAME( 0x01, 0x01, "PIA0.A" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("PIA0.B")
	PORT_DIPNAME( 0x01, 0x01, "PIA0.B" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("PIA1.A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x02, 0x02, "1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Cont")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Pass")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Sel")
	PORT_DIPNAME( 0x40, 0x40, "Show Refill" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Show Stats" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("PIA1.B")
	PORT_DIPNAME( 0x01, 0x01, "PIA1.B" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("PIA2.A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("PIA2.B")
	PORT_DIPNAME( 0x01, 0x01, "PIA2.B" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "4" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Tests" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "NVRAM Reset?" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( supnudg2 )    /* need to find the button 'B' to be playable */
		PORT_INCLUDE ( trailblz )

	PORT_MODIFY("PIA0.A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )  PORT_NAME("1 Pound (5 credits)")    // coin x 5
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )  PORT_NAME("50 Pence (2.5 credits)") // coin x 2.5
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )  PORT_NAME("20 Pence (1 credit)")    // coin x 1
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )  PORT_NAME("10 Pence (0.5 credit)")  // coin x 0.5
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )  PORT_CODE(KEYCODE_A)  PORT_NAME("PIA0.A_0x10")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )  PORT_CODE(KEYCODE_S)  PORT_NAME("PIA0.A_0x20")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )  PORT_CODE(KEYCODE_D)  PORT_NAME("PIA0.A_0x40")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )  PORT_CODE(KEYCODE_F)  PORT_NAME("PIA0.A_0x80")

	PORT_MODIFY("PIA1.A")
	PORT_DIPNAME( 0x01, 0x01, "PIA1.A" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("PIA2.A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 )  PORT_CODE(KEYCODE_N)  PORT_NAME("Pass")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )  PORT_CODE(KEYCODE_Z)  PORT_NAME("Button A")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )  PORT_CODE(KEYCODE_C)  PORT_NAME("Button C")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )   PORT_CODE(KEYCODE_1)  PORT_NAME("Start")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )  PORT_CODE(KEYCODE_T)  PORT_NAME("Test/Check Question Board")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )  PORT_CODE(KEYCODE_0)  PORT_NAME("Short Term Meters")  PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE )  PORT_CODE(KEYCODE_9)  PORT_NAME("Refill")             PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )    PORT_CODE(KEYCODE_Q)  PORT_NAME("Remote Credits x5")

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "4" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Tests?" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "NVRAM Reset?" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "First Install (DIL 8)" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( pokeroul )
	PORT_START("PIA0.A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Cancel / Collect")                           PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Hold 1 & 5 (auto?)")                         PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Hold 2 / Bet / Half Gamble / Previous Hand") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Deal / Draw / Gamble")                       PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Auto Hold")                                  PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Short Term Meters")              PORT_TOGGLE PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Refill Mode")                    PORT_TOGGLE PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )   PORT_NAME("0-8")                                        PORT_CODE(KEYCODE_S)

	PORT_START("PIA0.B")
	PORT_DIPNAME( 0x01, 0x01, "PIA0.B" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("PIA1.A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE (2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE (2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )   PORT_NAME("PIA1.A_3")                                        PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )   PORT_NAME("PIA1.A_4")                                        PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )   PORT_NAME("PIA1.A_5")                                        PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )   PORT_NAME("PIA1.A_6")                                        PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )   PORT_NAME("PIA1.A_7")                                        PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )   PORT_NAME("PIA1.A_8")                                        PORT_CODE(KEYCODE_Y)

	PORT_START("PIA1.B")
	PORT_DIPNAME( 0x01, 0x01, "PIA1.B" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("PIA2.A")
	PORT_DIPNAME( 0x01, 0x01, "PIA2.A" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
/*  PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
*/
	PORT_START("PIA2.B")
	PORT_DIPNAME( 0x01, 0x01, "PIA2.B" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Factory Install Switch" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( jpcoin )
	PORT_START("PIA0.A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Hold 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )   PORT_NAME("Bookkeeping")  PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("credits x10")  // credits x10
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("credits x1")   // credits x1
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("credits x5")   // credits x5
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   PORT_NAME("Remote x100")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Deal/Start")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Hold 5")

	PORT_START("PIA0.B")
	PORT_DIPNAME( 0x01, 0x01, "PIA0.B" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("PIA1.A")
	PORT_DIPNAME( 0x01, 0x01, "PIA1.A" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("PIA1.B")
	PORT_DIPNAME( 0x01, 0x01, "PIA1.B" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("PIA2.A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Hold 4")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Hold 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Hold 2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )     PORT_NAME("Low")     PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )    PORT_NAME("High")    PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_NAME("Cancel")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  PORT_NAME("Key Out")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BET )     PORT_NAME("Bet")     PORT_IMPULSE(2)

	PORT_START("PIA2.B")
	PORT_DIPNAME( 0x01, 0x01, "PIA2.B" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Minimum Hand" )
	PORT_DIPSETTING(    0x01, "Pair of Aces" )
	PORT_DIPSETTING(    0x00, "Jacks or Better" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Bookkeeping Type" )
	PORT_DIPSETTING(    0x08, "Weekly Meters" )
	PORT_DIPSETTING(    0x00, "Meter Totals" )
	PORT_DIPNAME( 0x10, 0x10, "Bonus 7s and 9s" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Factory Install Switch (Erase Meter Totals)" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Test Mode" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( gfx_coinmstr )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 46*32 )
GFXDECODE_END


TILE_GET_INFO_MEMBER(coinmstr_state::get_bg_tile_info)
{
	uint8_t *videoram = m_videoram;
	int tile = videoram[tile_index + 0x0240];
	int color = tile_index;

	tile |= (m_attr_ram1[tile_index + 0x0240] & 0x80) << 1;
	tile |= (m_attr_ram2[tile_index + 0x0240] & 0x80) << 2;

	tile |= (m_attr_ram3[tile_index + 0x0240] & 0x03) << (6+4);

	tileinfo.set(0, tile, color, 0);
}

void coinmstr_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(coinmstr_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 46, 32);
}

uint32_t coinmstr_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

void coinmstr_state::coinmstr(machine_config &config)
{
	Z80(config, m_maincpu, CPU_CLOCK); // 7 MHz.
	m_maincpu->set_addrmap(AS_PROGRAM, &coinmstr_state::coinmstr_map);
	m_maincpu->set_vblank_int("screen", FUNC(coinmstr_state::irq0_line_hold));

	pia6821_device &pia0(PIA6821(config, "pia0"));
	pia0.readpa_handler().set_ioport("PIA0.A");
	pia0.readpb_handler().set_ioport("PIA0.B");

	pia6821_device &pia1(PIA6821(config, "pia1"));
	pia1.readpa_handler().set_ioport("PIA1.A");
	pia1.set_port_a_input_overrides_output_mask(0xff);
	pia1.readpb_handler().set_ioport("PIA1.B");

	pia6821_device &pia2(PIA6821(config, "pia2"));
	pia2.readpa_handler().set_ioport("PIA2.A");
	pia2.readpb_handler().set_ioport("PIA2.B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(0*8, 46*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(coinmstr_state::screen_update));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_coinmstr);
	PALETTE(config, m_palette).set_entries(46*32*4);

	mc6845_device &crtc(MC6845(config, "crtc", 14000000 / 16));
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", SND_CLOCK));
	aysnd.port_a_read_callback().set_ioport("DSW1");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.25);
}

void coinmstr_state::quizmstr(machine_config &config)
{
	coinmstr(config);
	m_maincpu->set_addrmap(AS_IO, &coinmstr_state::quizmstr_io_map);
}

void coinmstr_state::trailblz(machine_config &config)
{
	coinmstr(config);
	m_maincpu->set_addrmap(AS_IO, &coinmstr_state::trailblz_io_map);
}

void coinmstr_state::supnudg2(machine_config &config)
{
	coinmstr(config);
	m_maincpu->set_addrmap(AS_IO, &coinmstr_state::supnudg2_io_map);
}

void coinmstr_state::pokeroul(machine_config &config)
{
	coinmstr(config);
	m_maincpu->set_addrmap(AS_IO, &coinmstr_state::pokeroul_io_map);
}

void coinmstr_state::jpcoin(machine_config &config)
{
	coinmstr(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &coinmstr_state::jpcoin_map);
	m_maincpu->set_addrmap(AS_IO, &coinmstr_state::jpcoin_io_map);
//  NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}

void coinmstr_state::jpjcoin(machine_config &config)
{
	jpcoin(config);
	m_maincpu->set_addrmap(AS_IO, &coinmstr_state::jpjcoin_io_map);
}

/*

Quizmaster
Coinmaster, 1985

PCB Layout
----------


PCB 001-POK                                            5MBYTE MEMORY EXPANSION
|--------------------------------------------------|   |------------------------------|
|   ULN2803      6821                              |   |                              |
|                                                  |   |                              |
|                                NPC_QM4_11.IC45   |   |GESCHICH2.IC19  GESCHICH1.IC20|
|   ULN2003      6821                              |   |                              |
|     ULN2003                    NPC_QM4_21.IC41   |   |      *         GESCHICH3.IC18|
|                                                  |   |                              |
|7               6821                              |   |POPMUSIK2.IC15  POPMUSIK1.IC16|
|2                               HD465055          |   |                              |
|W                                                 |   |      *               *       |
|A                                                 |   |                              |
|Y           AY3-8912                       6116   |   |SPORT2.IC11     SPORT1.IC12   |
|                                                  |   |                              |
|                                                  |   |SPORT4.IC9      SPORT3.IC10   |
|                BATTERY                    6116   |   |                              |
|     DSW1(8)              NM_QM4_11.IC9           |   |GEOGRAPH2.IC7   GEOGRAPH1.IC8 |
|                  NE555                           |   |                              |
|            VOL           NP_QM4_21.IC6    6116   |   |      *               *       |
| LM380                                  PAL       |   |                              |
|                                                  |   |ALLGEME2.IC3    ALLGEME1.IC4  |
|                                        PAL       |   |                              |
|                                                  |   |      *         ALLGEME3.IC2  |
|                                        PAL       |   |                              |
|                                            14MHz |   |                              |
|                              Z80       CN1       |   |                       CN2    |
|                                                  |   |     PAL                      |
|--------------------------------------------------|   |------------------------------|
CN1/2 is connector for top ROM board                    * - unpopulated socket


*/

ROM_START( quizmstr )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "nm_qm4_11.ic9", 0x0000, 0x4000, CRC(3a233bf0) SHA1(7b91b6f19093e67dd5513a000138421d4ef6f0af) )
	ROM_LOAD( "np_qm4_21.ic6", 0x4000, 0x4000, CRC(a1cd39e4) SHA1(420b0726577471c762ae470bc2138c035f295ad9) )
	/* 0x8000 - 0xbfff empty */

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "npc_qm4_11.ic45", 0x0000, 0x2000, CRC(ed48582a) SHA1(0aa2434a43af2990b8ad1cd3fc9f2e5e962f99c7) )
	ROM_LOAD( "npc_qm4_21.ic41", 0x2000, 0x2000, CRC(b67b0183) SHA1(018cabace593e795edfe914cdaedb9ebdf158567) )

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* Question roms */
	/* empty ic1 */
	ROM_LOAD( "allgeme3.ic2",   0x08000, 0x8000, CRC(e9ead7f0) SHA1(c0b8e4e7905f31b74c8d217f0afc91f73d52927b) )
	ROM_LOAD( "allgeme2.ic3",   0x10000, 0x8000, CRC(ac4d2ee8) SHA1(3a64fba8a24ae2e8bfd9d1c27804342e1779bcf6) )
	ROM_LOAD( "allgeme1.ic4",   0x18000, 0x8000, CRC(896e619b) SHA1(6f0faf0ae206f20387024a4a426b3e92429b3b1d) )
	/* empty ic5 */
	/* empty ic6 */
	ROM_LOAD( "geograph2.ic7",  0x30000, 0x8000, CRC(d809eeb6) SHA1(c557cecd3dd641a9c293f1865a423dafcd71af82) )
	ROM_LOAD( "geograph1.ic8",  0x38000, 0x8000, CRC(8984e83c) SHA1(d22c02e9297f804f8560e2e46793e4b6654d0785) )
	ROM_LOAD( "sport4.ic9",     0x40000, 0x8000, CRC(3c37de48) SHA1(bee26e9b15cec0b8e81af59810db17a8f2bdc299) )
	ROM_LOAD( "sport3.ic10",    0x48000, 0x8000, CRC(24abe1e7) SHA1(77373b1fafa4b117b3a1e4c6e8b530e0bb3b4f42) )
	ROM_LOAD( "sport2.ic11",    0x50000, 0x8000, CRC(26645e8e) SHA1(4922dcd417f7d098aaaa6a0320ed1d3e488d3e63) )
	ROM_LOAD( "sport1.ic12",    0x58000, 0x8000, CRC(7be41758) SHA1(8e6452fd902d25a73d3fa89bd7b4c5563669cc92) )
	/* empty ic13 */
	/* empty ic14 */
	ROM_LOAD( "popmusik2.ic15", 0x70000, 0x8000, CRC(d3b9ea70) SHA1(0a92ecdc4e2ddd3c0f40682a46a88bc617829481) )
	ROM_LOAD( "popmusik1.ic16", 0x78000, 0x8000, CRC(685f047e) SHA1(c0254130d57f60435a70effe6376e0cb3f50223f) )
	/* empty ic17 */
	ROM_LOAD( "geschich3.ic18", 0x88000, 0x8000, CRC(26c3ceec) SHA1(bf6fd24576c6159bf7730b04d2ac451bfcf3f757) )
	ROM_LOAD( "geschich2.ic19", 0x90000, 0x8000, CRC(387d166e) SHA1(14edac9ef550ce64fd81567520f3009612aa7221) )
	ROM_LOAD( "geschich1.ic20", 0x98000, 0x8000, CRC(bf4c097f) SHA1(eb14e7bad713d3b03fa3978a7f0087312517cf9e) )
ROM_END


ROM_START( trailblz )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "1-4.09",       0x0000, 0x4000, CRC(7c34749c) SHA1(3847188a734b32979f376f51f74dff050b610dfb) )
	ROM_LOAD( "2-4.06",       0x4000, 0x4000, CRC(81a9809b) SHA1(4d2bfd5223713a9e2e15130a3176118d400ee63e) )
	/* 0x8000 - 0xbfff empty */

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "1-2.45",       0x0000, 0x2000, CRC(b4a807b1) SHA1(f00a4790adb0c25917a0dc8c98c9b65526304fd3) )
	ROM_LOAD( "2-2.41",       0x2000, 0x2000, CRC(756dd230) SHA1(6d6f440bf1f48cc33d5e46cfc645809d5f8b1f3a) )

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* Question roms */
	ROM_LOAD( "questions.bin", 0x00000, 0xa0000, NO_DUMP )
ROM_END


ROM_START( supnudg2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u3.bin",       0x0000, 0x8000, CRC(ed04e2cc) SHA1(7d90a588cca2d113487710e897771f9d99e37e62) )
	ROM_LOAD( "u4.bin",       0x8000, 0x8000, CRC(0551e859) SHA1(b71640097cc75b78f3013f0e77de328bf1a205b1) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "u25.bin",      0x0000, 0x8000, CRC(1f7cef5e) SHA1(3abc31d400a0f5dc29c70d8aac42fd6302290cc9) )
	ROM_LOAD( "u23.bin",      0x8000, 0x8000, CRC(726a48ac) SHA1(cd17840067294812edf5bfa88d71fc967388df8e) )

	ROM_REGION( 0xa0000, "user1", 0 ) /* Question roms */
	ROM_LOAD( "q1.bin",       0x00000, 0x8000, CRC(245d679a) SHA1(2d3fbed8c1b3d0bffe7f3bd9088e0a5d207654c7) )
	ROM_LOAD( "q2.bin",       0x08000, 0x8000, CRC(e41ae8fb) SHA1(526c7b60e6ee4dfe05bbabf0e1e986e04ac2f544) )
	ROM_LOAD( "q3.bin",       0x10000, 0x8000, CRC(692218a2) SHA1(b9548dd835d9f3fb3e09bd018c7f9cbecafaee28) )
	ROM_LOAD( "q4.bin",       0x18000, 0x8000, CRC(ce4be482) SHA1(4fd8f24d22d3f1789fc728445cbc5339ed454bb4) )
	ROM_LOAD( "q5.bin",       0x20000, 0x8000, CRC(805672bf) SHA1(0fa68cad0d1c2b11a04a364b5ff64facfa573bbc) )
	ROM_LOAD( "q6.bin",       0x28000, 0x8000, CRC(b4405848) SHA1(5f8ca8b017966e6f358f603efde83f45897f3476) )
	ROM_LOAD( "q7.bin",       0x30000, 0x8000, CRC(32329b78) SHA1(114f097678be734355b8f36f6af7f1cb75ece191) )
	ROM_LOAD( "q8.bin",       0x38000, 0x8000, CRC(25c2aa26) SHA1(7f95553bf98381ced086b6606345bef62fe89a3a) )
	ROM_LOAD( "q9.bin",       0x40000, 0x8000, CRC(c98cb15a) SHA1(7d12064c2bcb34668299cadae3072c7f8434c405) )
	ROM_LOAD( "q10.bin",      0x48000, 0x8000, CRC(0c6c2df5) SHA1(49c92e498a0556032bb8ca56ff5afb9f69a80b3f) )
	ROM_LOAD( "q11.bin",      0x50000, 0x8000, CRC(1c53a264) SHA1(c10cc32b032bd4f890497bdc942e7e8c75ea1d6f) )
	ROM_LOAD( "q12.bin",      0x58000, 0x8000, CRC(c9535bff) SHA1(9c9873642c62971f805dc629f8d1006e35a675f9) )
	ROM_LOAD( "q13.bin",      0x60000, 0x8000, CRC(7a9b9f61) SHA1(7e39fef67fc3c29604ae68358e01330cf5130c06) )
	ROM_LOAD( "q14.bin",      0x68000, 0x8000, CRC(ec35e800) SHA1(0e0ca6fec760f31f464b282a1d7341cc4a29c064) )
	ROM_LOAD( "q15.bin",      0x70000, 0x8000, CRC(9f3738eb) SHA1(e841958f37167e7f9adcd3c965d31e2b7e02f52c) )
	ROM_LOAD( "q16.bin",      0x78000, 0x8000, CRC(af92277c) SHA1(093079fab28e3de443b640d2777cc2980b20af6c) )
	ROM_LOAD( "q17.bin",      0x80000, 0x8000, CRC(522fd485) SHA1(6c2a2626c00015962c460eac0dcb46ea263a4a23) )
	ROM_LOAD( "q18.bin",      0x88000, 0x8000, CRC(54d50510) SHA1(2a8ad2a2e1735f9c7d606b99b3653f823f09d1e8) )
	ROM_LOAD( "q19.bin",      0x90000, 0x8000, CRC(30aa2ff5) SHA1(4a2b4fc9c0c5cab3d374ee4738152209589e0807) )
	ROM_LOAD( "q20.bin",      0x98000, 0x8000, CRC(0845b450) SHA1(c373839ee1ad983e2df41cb22f625c14972372b0) )
ROM_END

/*

  Poker Roulette (c) 1990 Coinmaster


  Hardware notes:

  CPU:  1x z80
  RAM:  2x 6264
  I/O:  3x 6821 PIAs
  CRTC: 1x 6845
  SND:  1x ay-3-8912
  Xtal: 1x 14MHz.


  Dev notes:

  2x gfx banks, switched by bit4 of attr RAM.

*/

ROM_START( pokeroul )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "poker1.ic3", 0x0000, 0x8000, CRC(bfe78d09) SHA1(7cc0f57714ff808a41ce20027a283e5dff60f752) )
	ROM_LOAD( "poker2.ic4", 0x8000, 0x8000, CRC(34c1b55c) SHA1(fa562d230a57dce3fff176c21c86b461a02749f6) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "027c1.00_e14.7.88.ic25", 0x0000, 0x8000, CRC(719f9879) SHA1(122e2689a21a22713f938e3bf6cfb72c60fb9a16) )
	ROM_LOAD( "027c1.01_e14.7.88.ic23", 0x8000, 0x8000, CRC(71e5a2fc) SHA1(c28efcea1cf6c9872e70ff191932e3cdb5618917) )
ROM_END

/*
 Looks like the 2x 6264, since checks C000-DFFF

 BP 1D0 (PIAS init)
 BP 1102 (calls)

 Output C0
 Input C1

 Input C8
 Output C9 (masked)
 Output CA

 Input D0
 Output D1
 Output D2
 Input D3
 Output DA

 Output E0  CRTC
 Output E1  CRTC

*/
ROM_START( jpcoin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2.bin", 0x0000, 0x2000, CRC(67e1aa60) SHA1(32301f60a7325f23047d84bb1e9416ea05753493) )
	ROM_LOAD( "1.bin", 0x4000, 0x2000, CRC(6c79e430) SHA1(56e026329ea6aba122d1f66c375bf4c3cc829feb) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "4.bin", 0x0000, 0x4000, CRC(2bac1c0b) SHA1(3e45fc38ed6d332e1d49b2b66bf8001610f914c5) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "3.bin", 0x4000, 0x4000, CRC(4984053e) SHA1(e0f7c56160f48f7b1c2c407f448c13a191770adc) ) // 1ST AND 2ND HALF IDENTICAL
ROM_END

/*
Joker Poker from Coinmaster.
It boots to an error screen, seems that PIAs are mapped at different offsets

Original Coinmaster PCB
Silkscreened: COINMASTER (c) 1884
and VIDEO BOARD STOCK No PCB-001-POK

1x SGS Z8400AB1 (Z80 A CPU).
1x MC68A45P CRTC
3x HD68B21P PIAs.

3x Hitachi HM6116 (2048 X 8bit) RAM

1x AY-3-8912.
1x LM380 AMP

1x 3.6 Volts, 100 mAh battery.
1x 8 DIP switches bank.
1x 14 MHz. Xtal.

CPU CLK 3.5Mhz               14/4
AY3-89-12 CLK 1.75Mhz        14/8

*/

ROM_START( jpcoin2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jp88-1.ic9", 0x0000, 0x4000, CRC(60d31daf) SHA1(204537887388f1a174d1a09331186182be31e8ee) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "jp88-3.ic45", 0x0000, 0x2000, CRC(f2f92a7e) SHA1(ce6f6fd5af0049269357527650b51a1016caf636) )
	ROM_LOAD( "jp88-2.ic41", 0x2000, 0x2000, CRC(57db61b2) SHA1(a3bc2056866cbb9fdca52e62f2ff4a952d1d7484) )
ROM_END


// Silkscreened: COINMASTER (c) 1984
// ROMs' labels had Nero Poker overwritten with 'Jackpot' with a pen, so the ROMs were probably recycled for a newer game
ROM_START( jpjcoin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nero_poker_1.ic9", 0x0000, 0x4000, CRC(d80a3286) SHA1(dbcfac0e055e6b3965d43015399c9ee1f8442109) )
	ROM_LOAD( "nero_poker_2.ic6", 0x4000, 0x4000, CRC(98a384ee) SHA1(64c32a2483561b91abab55687151be1a452a5953) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "nero_poker_char_2.ic45", 0x0000, 0x2000, CRC(57df89b0) SHA1(4bb34806a438d0d82bb7ce0026de9e62a8169a2d) )
	ROM_LOAD( "nero_poker_char_1.ic41", 0x2000, 0x2000, CRC(512c14b8) SHA1(462ad3163bc63d8fbd0d79776f49cd21656963d3) )

	ROM_REGION( 0x100, "plds", 0 )
	ROM_LOAD( "pal10l8cn.1.ic5",  0x00, 0x2c, CRC(cb037476) SHA1(64f55a998ac0dd64e02f3cf2d1457f3d9d804962) )
	ROM_LOAD( "pal10h8cn.2.ic8",  0x30, 0x2c, CRC(ce530d03) SHA1(e2547ac21749cf2362d032b86fdd492a800a3019) )
	ROM_LOAD( "pal10h8cn.3.ic12", 0x60, 0x2c, CRC(1eb10fe5) SHA1(5ee435c87f376940a15d6c80b25b4970d56d6013) )
ROM_END


/*************************
*      Driver Init       *
*************************/

void coinmstr_state::init_coinmstr()
{
	uint8_t *rom = memregion("user1")->base();
	int length = memregion("user1")->bytes();
	std::vector<uint8_t> buf(length);

	memcpy(&buf[0],rom,length);

	for (int i = 0; i < length; i++)
	{
		int adr = bitswap<24>(i, 23,22,21,20,19,18,17,16,15, 14,8,7,2,5,12,10,9,11,13,3,6,0,1,4);
		rom[i] = bitswap<8>(buf[adr],3,2,4,1,5,0,6,7);
	}
}

} // anonymous namespace


/*************************
*      Game Drivers      *
*************************/

//    YEAR  NAME      PARENT    MACHINE   INPUT     STATE           INIT           ROT   COMPANY                  FULLNAME                                    FLAGS
GAME( 1985, quizmstr, 0,        quizmstr, quizmstr, coinmstr_state, init_coinmstr, ROT0, "Loewen Spielautomaten", "Quizmaster (German)",                      MACHINE_UNEMULATED_PROTECTION )
GAME( 1987, trailblz, 0,        trailblz, trailblz, coinmstr_state, init_coinmstr, ROT0, "Coinmaster",            "Trail Blazer",                             MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING ) // or Trail Blazer 2 ?
GAME( 1989, supnudg2, 0,        supnudg2, supnudg2, coinmstr_state, init_coinmstr, ROT0, "Coinmaster",            "Super Nudger II - P173 (Version 5.21)",    MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME( 1990, pokeroul, 0,        pokeroul, pokeroul, coinmstr_state, empty_init,    ROT0, "Coinmaster",            "Poker Roulette (Version 8.22)",            MACHINE_NOT_WORKING )
GAME( 1985, jpcoin,   0,        jpcoin,   jpcoin,   coinmstr_state, empty_init,    ROT0, "Coinmaster",            "Joker Poker (Coinmaster set 1)",           0 )
GAME( 1990, jpcoin2,  0,        jpcoin,   jpcoin,   coinmstr_state, empty_init,    ROT0, "Coinmaster",            "Joker Poker (Coinmaster, Amusement Only)", 0 )
GAME( 1988, jpjcoin, 0,         jpjcoin,  jpcoin,   coinmstr_state, empty_init,    ROT0, "<unknown>",             "Jackpot Joker Poker (Version 88V 01)",     0 )
