// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/*
BMC Bowling (c) 1994.05 BMC, Ltd

- TS 2004.10.22

  Game is almost playable, especially with NVRAM_HACK (undefine it
  to get real nvram  access, but sometimes it's impossible to get back to
  title screen ).

 Controls:

 press START(1) OR BUTTON1 to start game , also START(1) or BUTTON1 to bowl / start
        ( 5 to insert coin(s) , B to bet , D to pay out (?)  etc...)

 press ANALYZER(0) during boot to enter test menu, then :
            STOP1+STOP2 - sound test menu
        BIG(G) - cycle options ,
        DOUBLE(H) - play
            STOP1(X),STOP2(C) - change
            TAKE(A) - color test
            START(1) - exit
        BET(B)+START(1) - other tests
        START(1) - next test

 press START(1)+HP(S) during boot to see stats

 press CONFIRM(N) during boot, to enter    settings
        BET(B) - change page
        STOP1(X)/STOP3(V) - modify
        START(1)/SMALL(F) - move
        KEY DOWN(D) - default ?

TODO:

 - scroll (writes to $91800 and VIA port A - not used in game (only in test mode))
 - VIA 6522(ports)
 - Crt
 - interrupts
 - missing gfx elements
 - erratic music tempo

---

Chips:
MC68000P10
Goldstar GM68B45S (same as Hitachi HD6845 CTR Controller)*
Winbond WF19054 (same as AY3-8910)
MK28 (appears to be a AD-65, AKA OKI6295) next to rom 10
Synertek SY6522 VIA
9325-AG (Elliptical Filter)
KDA0476BCN-66 (RAMDAC)
part # scratched 64 pin PLCC next to 7EX & 8 EX

Ram:
Goldstar GM76C28A (2Kx8 SRAM) x3
HM62256LP-12 x6

OSC:
GREAT1 21.47727
13.3M
3.579545

DIPS:
Place for 4 8 switch dips
dips 1 & 3 are all connected via resistors
dips 2 & 4 are standard 8 switch dips

EPROM        Label         Use
----------------------------------------
ST M27C1001  bmc_8ex.bin - 68K code 0x00
ST M27C1001  bmc_7ex.bin - 68K code 0x01
ST M27C512   bmc_3.bin\
ST M27C512   bmc_4.bin | Graphics
ST M27C512   bmc_5.bin |
ST M27C512   bmc_6.bin/
HM27C101AG   bmc_10.bin - Sound samples

BrianT

Top board:
          --- Edge Connection ---
GS9403      7EX    8EX   Part #      Misc HD74LS374p
                         Scratched   HM62256   HM62256
                         PLC
  OKI                                ST T74L6245BI
                  PEEl 18CV8P
          10



Main board:
          --- Edge Connection ---              JAMMA Connection
  68K     GM76C28A         3   5   GMC76C28A
          GM76C28A         4   6   GM68B45S   BATTERY

21.47727MHz                                 WF19054
                                                      DIP1
13.3Mhz                                               DIP2
  SY6522            HM62256 x4  3.579545MHz 9325-AG   DIP3
                                   KDA0476BCN-66      DIP4
*/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "machine/6522via.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"
#include "video/ramdac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

#define NVRAM_HACK


class bmcbowl_state : public driver_device
{
public:
	bmcbowl_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_stats_ram(*this, "nvram"),
		m_vid1(*this, "vid1"),
		m_vid2(*this, "vid2"),
		m_palette(*this, "palette"),
		m_input(*this, "IN%u", 1)
	{ }

	void bmcbowl(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	uint8_t random_read();
	uint16_t protection_r();
	void scroll_w(uint16_t data);
	void via_a_out(uint8_t data);
	void via_b_out(uint8_t data);
	void via_ca2_out(int state);
	uint8_t dips1_r();
	void input_mux_w(uint8_t data);
	void int_ack_w(uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void init_stats(const uint8_t *table, int table_len, int address);
	void main_mem(address_map &map) ATTR_COLD;
	void ramdac_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint16_t> m_stats_ram;
	required_shared_ptr<uint16_t> m_vid1;
	required_shared_ptr<uint16_t> m_vid2;
	required_device<palette_device> m_palette;

	required_ioport_array<2> m_input;
	uint8_t m_selected_input = 0;
};


uint32_t bmcbowl_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
/*
      280x230,4 bitmap layers, 8bpp,
        missing scroll and priorities   (maybe fixed ones)
*/

	bitmap.fill(rgb_t::black(), cliprect);

	int z = 0;
	for (int y = 0; y < 230; y++)
	{
		for (int x = 0; x < 280; x += 2)
		{
			int pixdat;

			pixdat = m_vid2[0x8000 + z];

			if (pixdat & 0xff)
				bitmap.pix(y, x + 1) = m_palette->pen(pixdat & 0xff);
			if (pixdat >> 8)
				bitmap.pix(y, x) = m_palette->pen(pixdat >> 8);

			pixdat = m_vid2[z];

			if (pixdat & 0xff)
				bitmap.pix(y, x + 1) = m_palette->pen(pixdat & 0xff);
			if (pixdat >> 8)
				bitmap.pix(y, x) = m_palette->pen(pixdat >> 8);

			pixdat = m_vid1[0x8000 + z];

			if (pixdat & 0xff)
				bitmap.pix(y, x + 1) = m_palette->pen(pixdat & 0xff);
			if (pixdat >> 8)
				bitmap.pix(y, x) = m_palette->pen(pixdat >> 8);

			pixdat = m_vid1[z];

			if (pixdat & 0xff)
				bitmap.pix(y, x + 1) = m_palette->pen(pixdat & 0xff);
			if (pixdat >> 8)
				bitmap.pix(y, x) = m_palette->pen(pixdat >> 8);

			z++;
		}
	}
	return 0;
}

uint8_t bmcbowl_state::random_read()
{
	return machine().rand();
}

uint16_t bmcbowl_state::protection_r()
{
	switch (m_maincpu->pcbase())
	{
		case 0xca68:
			switch (m_maincpu->state_int(M68K_D2))
			{
				case 0x0000: return 0x37<<8;
				case 0x1013: return 0;
				default:     return 0x46<<8;
			}
	}
	logerror("Protection read @ %X\n",m_maincpu->pcbase());
	return machine().rand();
}

void bmcbowl_state::scroll_w(uint16_t data)
{
	// TODO - scroll
}

void bmcbowl_state::via_a_out(uint8_t data)
{
	// related to video hw ? BG scroll ?
}

void bmcbowl_state::via_b_out(uint8_t data)
{
	// used
}

void bmcbowl_state::via_ca2_out(int state)
{
	// used
}


// 'working' NVRAM

#ifdef NVRAM_HACK
static const uint8_t bmc_nv1[]=
{
	0x00,0x00,0x55,0x55,0x00,0x00,0x55,0x55,0x00,0x00,0x55,0x55,0x00,0x00,0x55,0x55,0x13,0x88,0x46,0xDD,0x0F,0xA0,
	0x5A,0xF5,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x6E,0x55,
	0x55,0x55,0x3B,0x00,0x00,0x00,0x06,0x55,0x55,0x55,0x53,0x00,0x00,0x00,0x00,0x55,0x55,0x55,0x55,0x00,0x00,0x00,
	0x00,0x55,0x55,0x55,0x55,0x00,0x00,0x00,0x6E,0x55,0x55,0x55,0x3B,0x00,0x00,0x00,0x06,0x55,0x55,0x55,0x53,0x00,
	0x00,0x00,0x00,0x55,0x55,0x55,0x55,0x00,0x00,0x00,0x00,0x55,0x55,0x55,0x55,0x00,0x00,0x00,0x00,0x55,0x55,0x55,
	0x55,0x00,0x00,0x00,0x00,0x55,0x55,0x55,0x55,0x00,0x00,0x00,0x00,0x55,0x55,0x55,0x55,0xFF,0x00,0x0A,0x00,0x0A,
	0x00,0x32,0x00,0x02,0x28,0x32,0x5C,0x0A,0x03,0x03,0xD6,0x66,0xFF,0xFF,0xFF,0xFF,0x5D,0xED,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x25,0xD5,0x25,0x1C,0x00,0x00,0x00,0x00,
	0x00,0x96,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x6E,
	0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x96,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0xDC,0x00,0xFF,0xFF,0xFF,0xFF
};

static const uint8_t bmc_nv2[]=
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x03,0x00,0x09,0x00,0x00,0x2B,0xF1,
	0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
};

static const uint8_t bmc_nv3[]=
{
	0xFA,0xFF,0x01,0x02,0x04,0x0A,0x1E,0xC8,0x02,0x01,0xFF,0xFF,0xFF,0xFF,0xFF
};


void bmcbowl_state::init_stats(const uint8_t *table, int table_len, int address)
{
	for (int i = 0; i < table_len; i++)
		m_stats_ram[address+i] = 0xff00 | table[i];
}
#endif

void bmcbowl_state::int_ack_w(uint8_t data)
{
	m_maincpu->set_input_line(M68K_IRQ_2, CLEAR_LINE);
}

void bmcbowl_state::machine_start()
{
	save_item(NAME(m_selected_input));
}

void bmcbowl_state::machine_reset()
{
#ifdef NVRAM_HACK
	for (int i = 0; i < m_stats_ram.bytes()/2; i++)
		m_stats_ram[i] = 0xffff;

	init_stats(bmc_nv1,std::size(bmc_nv1), 0);
	init_stats(bmc_nv2,std::size(bmc_nv2), 0x3b0/2);
	init_stats(bmc_nv3,std::size(bmc_nv3), 0xfe2/2);
#endif
}

void bmcbowl_state::main_mem(address_map &map)
{
	map(0x000000, 0x01ffff).rom();

	map(0x090001, 0x090001).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x090003, 0x090003).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x090005, 0x090005).w("ramdac", FUNC(ramdac_device::mask_w));

	map(0x090800, 0x090803).nopw();
	map(0x091000, 0x091001).nopw();
	map(0x091800, 0x091801).w(FUNC(bmcbowl_state::scroll_w));

	map(0x092000, 0x09201f).m("via6522", FUNC(via6522_device::map)).umask16(0x00ff);

	map(0x093000, 0x093003).w("ymsnd", FUNC(ym2413_device::write)).umask16(0x00ff);
	map(0x092800, 0x092803).w("aysnd", FUNC(ay8910_device::data_address_w)).umask16(0xff00);
	map(0x092802, 0x092802).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x093802, 0x093803).portr("IN0");
	map(0x095000, 0x095fff).ram().share("nvram"); /* 8 bit */
	map(0x097000, 0x097001).nopr();
	map(0x140000, 0x1bffff).rom();
	map(0x1c0000, 0x1effff).ram().share("vid1");
	map(0x1f0000, 0x1fffff).ram();
	map(0x200000, 0x21ffff).ram().share("vid2");

	map(0x28c000, 0x28c000).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));

	/* protection device*/
	map(0x30c000, 0x30c001).nopw();
	map(0x30c040, 0x30c041).nopw();
	map(0x30c080, 0x30c081).nopw();
	map(0x30c0c0, 0x30c0c1).nopw();
	map(0x30c100, 0x30c101).r(FUNC(bmcbowl_state::protection_r));
	map(0x30c140, 0x30c141).nopw();
	map(0x30ca01, 0x30ca01).rw(FUNC(bmcbowl_state::random_read), FUNC(bmcbowl_state::int_ack_w));
}


static INPUT_PORTS_START( bmcbowl )
	PORT_START("IN0")   /* DSW 1 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Note")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Analyzer")

	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Pay") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Stop") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Stop 1") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Stop 2") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Stop 3") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Bet") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Confirm") PORT_CODE(KEYCODE_N)


	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Start")
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Take") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("HP") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Key Down") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Small") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Big") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Double") PORT_CODE(KEYCODE_H)

	PORT_START("IN1")   /* DSW 2 */
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x20, "1 COIN 10 CREDITS" )
	PORT_DIPSETTING(    0x00, "2 COINS 10 CREDITS" )

	PORT_DIPNAME( 0x01, 0x00, "DSW2 8" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DSW2 7" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DSW2 6" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DSW2 5" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DSW2 4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_DIPNAME( 0x40, 0x00, "DSW2 2" )
	PORT_DIPSETTING(    0x040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "DSW2 1" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")   /* DSW 4 */
	PORT_DIPNAME( 0x01, 0x00, "DSW4 8" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DSW4 7" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DSW4 6" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DSW4 5" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DSW4 4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DSW4 3" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DSW4 2" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "DSW4 1" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN3")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
INPUT_PORTS_END

uint8_t bmcbowl_state::dips1_r()
{
	switch (m_selected_input)
	{
		case 0x00: return m_input[0]->read();
		case 0x40: return m_input[1]->read();
	}
	logerror("%s: unknown input - %X\n", machine().describe_context(), m_selected_input);
	return 0xff;
}


void bmcbowl_state::input_mux_w(uint8_t data)
{
	m_selected_input = data;
}

void bmcbowl_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}

void bmcbowl_state::bmcbowl(machine_config &config)
{
	M68000(config, m_maincpu, 21.477272_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &bmcbowl_state::main_mem);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(35*8, 30*8);
	screen.set_visarea(0*8, 35*8-1, 0*8, 29*8-1);
	screen.set_screen_update(FUNC(bmcbowl_state::screen_update));
	screen.screen_vblank().set_inputline(m_maincpu, M68K_IRQ_2, ASSERT_LINE); // probably not the source of this interrupt
	screen.screen_vblank().append("via6522", FUNC(via6522_device::write_cb1));

	PALETTE(config, m_palette).set_entries(256);
	ramdac_device &ramdac(RAMDAC(config, "ramdac", 0, m_palette));
	ramdac.set_addrmap(0, &bmcbowl_state::ramdac_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ym2413_device &ymsnd(YM2413(config, "ymsnd", 3.579545_MHz_XTAL)); // guessed chip type
	ymsnd.add_route(ALL_OUTPUTS, "lspeaker", 0.50);
	ymsnd.add_route(ALL_OUTPUTS, "rspeaker", 0.50);

	ay8910_device &aysnd(AY8910(config, "aysnd", 21.477272_MHz_XTAL / 16)); // matches PCB recording
	aysnd.port_a_read_callback().set(FUNC(bmcbowl_state::dips1_r));
	aysnd.port_b_write_callback().set(FUNC(bmcbowl_state::input_mux_w));
	aysnd.add_route(ALL_OUTPUTS, "lspeaker", 0.50);
	aysnd.add_route(ALL_OUTPUTS, "rspeaker", 0.50);

	okim6295_device &oki(OKIM6295(config, "oki", 21.477272_MHz_XTAL / 16, okim6295_device::PIN7_LOW)); // matches PCB recording
	oki.add_route(ALL_OUTPUTS, "lspeaker", 0.50);
	oki.add_route(ALL_OUTPUTS, "rspeaker", 0.50);

	/* via */
	via6522_device &via(MOS6522(config, "via6522", 13.3_MHz_XTAL / 16)); // clock not verified (controls music tempo)
	via.readpb_handler().set_ioport("IN3");
	via.writepa_handler().set(FUNC(bmcbowl_state::via_a_out));
	via.writepb_handler().set(FUNC(bmcbowl_state::via_b_out));
	via.ca2_handler().set(FUNC(bmcbowl_state::via_ca2_out));
	via.irq_handler().set_inputline(m_maincpu, M68K_IRQ_4);
}

ROM_START( bmcbowl )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "bmc_8ex.bin", 0x000000, 0x10000, CRC(8b1aa5db) SHA1(879df950bedf2c163ba89d983ca4a0691b01c46e) )
	ROM_LOAD16_BYTE( "bmc_7ex.bin", 0x000001, 0x10000, CRC(7726d47a) SHA1(8438c3345847c2913c640a29145ec8502f6b01e7) )

	ROM_LOAD16_BYTE( "bmc_4.bin", 0x140000, 0x10000, CRC(f43880d6) SHA1(9e73a29baa84d417ff88026896d852567a38e714) )
	ROM_RELOAD(0x160000, 0x10000)
	ROM_LOAD16_BYTE( "bmc_3.bin", 0x140001, 0x10000, CRC(d1af9410) SHA1(e66b3ddd9d9e3c567fdb140c4c8972c766f2b975) )
	ROM_RELOAD(0x160001, 0x10000)

	ROM_LOAD16_BYTE( "bmc_6.bin", 0x180000, 0x20000, CRC(7b9e0d77) SHA1(1ec1c92c6d4c512f7292b77e9663518085684ba9) )
	ROM_LOAD16_BYTE( "bmc_5.bin", 0x180001, 0x20000, CRC(708b6f8b) SHA1(4a910126d87c11fed99f44b61d51849067eddc02) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "bmc_10.bin", 0x00000, 0x20000,  CRC(f840c17f) SHA1(82891a85c8dc60f727b5a8c8e8ab09e8e4bd8af4) )
ROM_END

} // anonymous namespace


GAME( 1994, bmcbowl, 0, bmcbowl, bmcbowl, bmcbowl_state, empty_init, ROT0, "BMC", "Konkyuu no Hoshi", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE)
