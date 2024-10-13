// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/***************************************************************************

  IMOLA GP by RB Bologna (Alberici?)

TODO:
- document remaining dips
- need better mappings for shifter, currently 3 buttons
- cpu/audio clocks, the XTAL on pcb is unlabeled
- correct video timing, see sprites disappear partially
- vreg[0xf] autosteering, the car should probably only auto-steer when
  it gets on the grass(road edges) at high speed. How does the hardware
  know the sprite position then?
- verify colors

========================================
www.andys-arcade.com

Dumped by Andy Welburn on a windy and rainy day 07/07/04

Possibly has clk/dir type steering.

Shows RB BO ITALY on the title screen and is a top-down driving game,
a lot like monaco GP, it even has stages where you have headlights.
Board colour, screening, track patterns, and most importantly
component type and colour of sockets indicates to me a pcb made in
the same factory as 'Sidam' and some 'Olympia' games. There is no
manufacturer name, no game name, all i see is : AA20/80 etched
on the underside of the pcb.

I have had this pcb for a number of years, i always thought it was
some sort of pinball logic pcb so didn't treat it too well. When it
came to clearing out my boxes of junk i took another look at it, and
it was the bank of 4116 rams that made me take a closer look.

I hooked it up and saw some video on my scope, then it died.
The +12v had shorted.. Suspecting the annoying tantalum capacitors
(often short out for no reason) i found a shorted one, removed
it and away we went. It had separate H + V sync, so i loaded
a 74ls08 into a spare ic space and AND'ed the two signals to get
composite, voila, i got a stable picture. The colours aren't right,
and maybe the video isn't quite right either, but it worked enough
for me to realise i had never seen a game like it, so i dumped it.

I couldn't get any sound out of it, could be broken, or not
hooked up right, i would suspect the latter is the case.


Hardware :
==========
2x  z80's
1x  AY-3-8910
1x  8255
2   pairs of 2114 RAM (512 bytes each)
16x 4116 RAM (2k each)
4x  2716 (ROM)
12x 2708 (ROM)


ROMS layout:
------------
(add .bin to the end to get the filenames)
YC,YD,YA and YB are all 2716 eproms, everything else is 2708's.

(tabulation used here, see photo for clarity)

YC  YD      XX
YA  YB
            XC
        XI  XM
        XY  XD
        XP  XS
        XA  XE
        XR  XO

Andy Welburn
www.andys-arcade.com

========================================
Falgas and Videotronic (both from Spain) did a clone of "Imola GP" called "Ferrari 1".
The hardware is almost the same, being the only notable difference is that it uses bigger
ROMs (just three, labeled "P1/1", "P2/1" and "P2/2"), but the contents are the same:

p2-1.bin     [1/4]      12.bin                  IDENTICAL
p2-2.bin     [1/4]      10.bin                  IDENTICAL
p1-1.bin     [1/4]      03.bin                  IDENTICAL
p2-1.bin     [2/4]      08.bin                  IDENTICAL
p2-2.bin     [2/4]      06.bin                  IDENTICAL
p1-1.bin     [2/4]      01.bin                  IDENTICAL
p2-1.bin     [3/4]      11.bin                  IDENTICAL
p2-2.bin     [3/4]      09.bin                  IDENTICAL
p1-1.bin     [3/4]      04.bin                  IDENTICAL
p2-1.bin     [4/4]      07.bin                  IDENTICAL
p2-2.bin     [4/4]      05.bin                  IDENTICAL
p1-1.bin     [4/4]      02.bin                  IDENTICAL

The only xtal on this PCB is 16.00000 MHz.
This "Ferrari 1" was legally registered by Videotronic on Spain on 1985. The PCB is
silkscreened by Falgas and the cab contains Falgas logos with a small note that reads
"Manufactured by Videotronic for Falgas" (in Spanish).

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/timer.h"
#include "sound/ay8910.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "imolagp.lh"


namespace {

class imolagp_state : public driver_device
{
public:
	imolagp_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_slavecpu(*this, "slave"),
		m_steer_pot_timer(*this, "pot"),
		m_steer_inp(*this, "STEER"),
		m_digits(*this, "digit%u%u", 0U, 0U)
	{ }

	void imolagp(machine_config &config);

	ioport_value imolagp_steerlatch_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_slavecpu;
	required_device<timer_device> m_steer_pot_timer;
	required_ioport m_steer_inp;
	output_finder<5, 6> m_digits;

	uint8_t m_videoram[2][0x4000]; // 2 layers of 16KB
	uint8_t m_comms_latch[2];
	uint8_t m_vcontrol;
	uint8_t m_vreg[0x10];
	uint8_t m_scroll;
	uint8_t m_steerlatch;
	uint8_t m_draw_mode;

	void transmit_data_w(offs_t offset, uint8_t data);
	uint8_t trigger_slave_nmi_r();
	uint8_t receive_data_r(offs_t offset);
	void imola_led_board_w(offs_t offset, uint8_t data);
	uint8_t vreg_data_r();
	void screenram_w(offs_t offset, uint8_t data);
	uint8_t imola_draw_mode_r(offs_t offset);
	void vreg_control_w(uint8_t data);
	void vreg_data_w(uint8_t data);
	void vblank_irq(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(imolagp_pot_callback);

	void imolagp_palette(palette_device &palette) const;
	uint32_t screen_update_imolagp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void imolagp_master_io(address_map &map) ATTR_COLD;
	void imolagp_master_map(address_map &map) ATTR_COLD;
	void imolagp_slave_io(address_map &map) ATTR_COLD;
	void imolagp_slave_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Video

***************************************************************************/

void imolagp_state::imolagp_palette(palette_device &palette) const
{
	// palette seems like 3bpp + intensity
	// this still needs to be verified
	for (int i = 0; i < 8; i++)
	{
		palette.set_pen_color(i*4 + 0, 0, 0, 0);
		palette.set_pen_color(i*4 + 1, pal1bit(i >> 2) / 2, pal1bit(i >> 1) / 2, pal1bit(i >> 0) / 2);
		palette.set_pen_color(i*4 + 2, 0, 0, 0);
		palette.set_pen_color(i*4 + 3, pal1bit(i >> 2), pal1bit(i >> 1), pal1bit(i >> 0));
	}
}

void imolagp_state::video_start()
{
	memset(m_videoram, 0, sizeof(m_videoram));
	save_item(NAME(m_videoram));
}


uint32_t imolagp_state::screen_update_imolagp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// draw solid background layer first, then sprites on top
	for (int layer = 0; layer < 2; layer++)
	{
		for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
		{
			uint8_t const *const source = &m_videoram[layer][(y & 0xff) * 0x40];
			uint16_t *const dest = &bitmap.pix(y & 0xff);
			for (int i = 0; i < 0x40; i++)
			{
				uint8_t const data = source[i];
				if (data || !layer)
				{
					// one color per each 4 pixels
					uint8_t const color = (data & 0xf0) >> 3;
					uint8_t const x = (i << 2) - (m_scroll ^ 3);
					for (int x2 = 0; x2 < 4; x2++)
					{
						uint8_t const offset = x + x2;
						if (offset >= cliprect.min_x && offset <= cliprect.max_x)
							dest[offset] = color | (data >> x2 & 1);
					}
				}
			}
		}
	}

	return 0;
}



/***************************************************************************

  Interrupts

***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(imolagp_state::imolagp_pot_callback)
{
	int steer = m_steer_inp->read();
	if (steer & 0x7f)
	{
		if (~steer & 0x80)
		{
			// shift register when steering left
			steer = -steer;
			m_steerlatch = (m_steerlatch << 1) | (~m_steerlatch >> 1 & 1);
		}

		// steering speed is determined by timer period
		// these values(in usec) may need tweaking:
		const int base = 6500;
		const int range = 100000;
		m_steer_pot_timer->adjust(attotime::from_usec(base + range * (1.0 / (double)(steer & 0x7f))));
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	}
	else
		m_steer_pot_timer->adjust(attotime::from_msec(20));
}

void imolagp_state::vblank_irq(int state)
{
	if (state)
	{
		m_scroll = m_vreg[0xe]; // latch scroll
		m_maincpu->set_input_line(0, HOLD_LINE);
		m_slavecpu->set_input_line(0, HOLD_LINE);
	}
}



/***************************************************************************

  I/O and Memory Maps

***************************************************************************/

/* The master CPU transmits data to the slave CPU one word at a time using a rapid sequence of triggered NMIs.
 * The slave CPU pauses as it enters its vblank irq, awaiting this burst of data.
 * Handling the NMI takes more time than triggering the NMI, implying that the slave CPU either runs at
 * a higher clock, or has a way to force the main CPU to wait.
 */
void imolagp_state::transmit_data_w(offs_t offset, uint8_t data)
{
	m_comms_latch[offset] = data;
}

uint8_t imolagp_state::receive_data_r(offs_t offset)
{
	return m_comms_latch[offset];
}

uint8_t imolagp_state::trigger_slave_nmi_r()
{
	if (!machine().side_effects_disabled())
		m_slavecpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	return 0;
}


void imolagp_state::imola_led_board_w(offs_t offset, uint8_t data)
{
	// not sure what chip is used here, this is copied from turbo.c
	static const uint8_t ls48_map[16] =
		{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x58,0x4c,0x62,0x69,0x78,0x00 };

	int i = offset >> 3;
	int j = offset & 7;
	if (i < 5 && j < 6)
		m_digits[i][j] = ls48_map[data & 0x0f];
/*
    score:        00, 01, 02, 03
    time:         04, 05
    result:       12, 13
    credits:      14, 15
    highscore 1:  40, 41, 42, 43
    highscore 2:  44, 45, 30, 31
    highscore 3:  32, 33, 34, 35
    highscore 4:  20, 21, 22, 23
    highscore 5:  24, 25, 10, 11
*/
}


void imolagp_state::vreg_control_w(uint8_t data)
{
	m_vcontrol = data & 0xf;
}

uint8_t imolagp_state::vreg_data_r()
{
	// auto-steer related
	return 0;
	//return 0xf7; // -> go left?
	//return 0x17; // it checks for this too
}

void imolagp_state::vreg_data_w(uint8_t data)
{
	// $07: always $ff?
	// $0e: x scroll
	// $0f: auto-steer related
	m_vreg[m_vcontrol] = data;
}


void imolagp_state::screenram_w(offs_t offset, uint8_t data)
{
	// when in tunnel: $81/$82 -> sprite ram?
	if (m_draw_mode & 0x80)
		m_videoram[1][offset] = data;

	// sprites: $05
	else if (m_draw_mode & 0x01)
		m_videoram[1][offset] = data;

	// background: $06
	else
		m_videoram[0][offset] = data;
}

uint8_t imolagp_state::imola_draw_mode_r(offs_t offset)
{
	// the game reads a port before and after writing to screen ram
	m_draw_mode = offset;
	return 0;
}

void imolagp_state::imolagp_master_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x23ff).ram();
	map(0x2800, 0x2803).rw("ppi8255", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x3000, 0x3000).w(FUNC(imolagp_state::vreg_control_w));
	map(0x37f0, 0x37f0).w("aysnd", FUNC(ay8910_device::address_w));
//  map(0x37f7, 0x37f7).noprw();
	map(0x3800, 0x3800).rw(FUNC(imolagp_state::vreg_data_r), FUNC(imolagp_state::vreg_data_w));
	map(0x3810, 0x3810).w("aysnd", FUNC(ay8910_device::data_w));
	map(0x4000, 0x4000).portr("DSWA");
	map(0x47ff, 0x4800).w(FUNC(imolagp_state::transmit_data_w));
	map(0x5000, 0x50ff).w(FUNC(imolagp_state::imola_led_board_w));
	map(0x5800, 0x5800).portr("DSWA"); // assume mirror
	map(0x6000, 0x6000).portr("DSWB");
}

void imolagp_state::imolagp_master_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r(FUNC(imolagp_state::trigger_slave_nmi_r));
}


void imolagp_state::imolagp_slave_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x43ff).ram();
	map(0x9fff, 0xa000).r(FUNC(imolagp_state::receive_data_r));
	map(0xc000, 0xffff).w(FUNC(imolagp_state::screenram_w));
}

void imolagp_state::imolagp_slave_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0xff).r(FUNC(imolagp_state::imola_draw_mode_r));
}



/***************************************************************************

  Inputs

***************************************************************************/

ioport_value imolagp_state::imolagp_steerlatch_r()
{
	return m_steerlatch & 0xf;
}

static INPUT_PORTS_START( imolagp )
	PORT_START("DSWA") /* 0x4000 */
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0xe0, 0x00, DEF_STR( Test ) )
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )
	PORT_DIPSETTING(    0x20, "TEST A" )
	PORT_DIPSETTING(    0x40, "TEST C" )
	PORT_DIPSETTING(    0x60, "TEST D" )
	PORT_DIPSETTING(    0x80, "Memory" )
	PORT_DIPSETTING(    0xa0, "Color Test" )
	PORT_DIPSETTING(    0xc0, "Grid Test" )
	PORT_DIPSETTING(    0xe0, DEF_STR( Unused ) )

	PORT_START("DSWB") /* 0x6000 */
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(imolagp_state, imolagp_steerlatch_r)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("STEER")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x01, 0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(24)
INPUT_PORTS_END


static INPUT_PORTS_START( imolagpo )
	PORT_INCLUDE( imolagp )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(imolagp_state, imolagp_steerlatch_r)
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void imolagp_state::machine_start()
{
	m_digits.resolve();

	save_item(NAME(m_vcontrol));
	save_item(NAME(m_vreg));
	save_item(NAME(m_scroll));
	save_item(NAME(m_steerlatch));
	save_item(NAME(m_draw_mode));
	save_item(NAME(m_comms_latch));
}

void imolagp_state::machine_reset()
{
	// reset steering wheel
	m_steerlatch = 0;
	m_steer_pot_timer->adjust(attotime::from_msec(20));
}


void imolagp_state::imolagp(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 3000000); // ? (assume slower than slave)
	m_maincpu->set_addrmap(AS_PROGRAM, &imolagp_state::imolagp_master_map);
	m_maincpu->set_addrmap(AS_IO, &imolagp_state::imolagp_master_io);
	TIMER(config, m_steer_pot_timer).configure_generic(FUNC(imolagp_state::imolagp_pot_callback)); // maincpu nmi

	Z80(config, m_slavecpu, 4000000); // ?
	m_slavecpu->set_addrmap(AS_PROGRAM, &imolagp_state::imolagp_slave_map);
	m_slavecpu->set_addrmap(AS_IO, &imolagp_state::imolagp_slave_io);

	config.set_perfect_quantum(m_maincpu);

	i8255_device &ppi(I8255A(config, "ppi8255", 0));
	// mode $91 - ports A & C-lower as input, ports B & C-upper as output
	ppi.in_pa_callback().set_ioport("IN0");
	ppi.out_pb_callback().set([this](uint8_t data) { logerror("%s PPI write port B: %02X\n", machine().describe_context(), data); });
	ppi.in_pc_callback().set_ioport("IN1");

	/* video hardware */
	// Part of the screen is obscured by the cabinet - this is handled by the visible area and physical aspect ratio here
	// It would be better to move this into the layout so that the video output can be used with a restored cabinet.
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256,256);
	screen.set_visarea(0+48,255,0+16,255);
	screen.set_screen_update(FUNC(imolagp_state::screen_update_imolagp));
	screen.set_video_attributes(VIDEO_UPDATE_SCANLINE);
	screen.set_palette("palette");
	screen.screen_vblank().set(FUNC(imolagp_state::vblank_irq));
	screen.set_physical_aspect(13, 12);

	PALETTE(config, "palette", FUNC(imolagp_state::imolagp_palette), 0x20);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	AY8910(config, "aysnd", 2000000).add_route(ALL_OUTPUTS, "mono", 0.5); // ?
}


ROM_START( imolagp ) // same hardware, but larger video roms
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 code */
	ROM_LOAD( "03.bin",   0x0000, 0x0800, CRC(a8be1c83) SHA1(e3147515f9f44e435192db838ffbb5c592f6e8d7) )
	ROM_LOAD( "01.bin",   0x0800, 0x0800, CRC(aa6fc1ea) SHA1(3f9c559aaba7b00ffd0210c6977dd4f966451a4b) )
	ROM_LOAD( "04.bin",   0x1000, 0x0800, CRC(c45c459c) SHA1(e51bbfe79bcd66d80b9179067611ea2029c9fd7a) )
	ROM_LOAD( "02.bin",   0x1800, 0x0800, CRC(a80e193b) SHA1(b31bf30dfe1bc498a4324719e4a6656fb94b8d96) )

	ROM_REGION( 0x10000, "slave", 0 ) /* Z80 code */
	ROM_LOAD( "12.bin",   0x0000, 0x0800, CRC(f9658100) SHA1(00fe32ef6b7cd909e8b69f0f8431c78591318aff) )
	ROM_LOAD( "08.bin",   0x0800, 0x0800, CRC(3a23a90e) SHA1(3a9ce5717147f2cf8c58432dd5ddcf70c2a041aa) )
	ROM_LOAD( "11.bin",   0x1000, 0x0800, CRC(5252e22e) SHA1(bb032af93f5a027235b35467c8a2c2c6fe6d1461) )
	ROM_LOAD( "07.bin",   0x1800, 0x0800, CRC(ce3459ff) SHA1(e336f9411cff71d85cdcc30af7405eca02c8c8f8) )
	ROM_LOAD( "10.bin",   0x2000, 0x0800, CRC(a043ded9) SHA1(0e3b53897da98ef3953622f8bd7dc379916ac0c0) )
	ROM_LOAD( "06.bin",   0x2800, 0x0800, CRC(3ff5997d) SHA1(95ac9dbda782b94b9b2dc4c9baea86113968077f) )
	ROM_LOAD( "09.bin",   0x3000, 0x0800, CRC(b5eb210e) SHA1(81340b1797f2401fbf0485091bf3b309c153476a) )
	ROM_LOAD( "05.bin",   0x3800, 0x0800, CRC(f59e426e) SHA1(ec9bdeed74e2450acb7f00abd13cb5ceb3205016) )
ROM_END

ROM_START( imolagpo )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 code */
	ROM_LOAD( "yd.bin",   0x0000, 0x0800, CRC(5eb61bb7) SHA1(b897ecc7fa9aa1ae4e095d22d16a901b9d439a8e) )
	ROM_LOAD( "yc.bin",   0x0800, 0x0800, CRC(f7468a3b) SHA1(af1664e30b732b3d5321e76659961af3ebeb1237) )
	ROM_LOAD( "yb.bin",   0x1000, 0x0800, CRC(9f21506e) SHA1(6b46ff4815b8a02b190ec13e067f9a6687980774) )
	ROM_LOAD( "ya.bin",   0x1800, 0x0800, CRC(23fbcf14) SHA1(e8b5a9b01f715356c14aa41dbc9ca26732d3a4e4) )

	ROM_REGION( 0x10000, "slave", ROMREGION_ERASEFF ) /* Z80 code */
	ROM_LOAD( "xx.bin",   0x0000, 0x0400, CRC(059d6294) SHA1(38f075753e7a9fcabb857e5587e8a5966052cbcd) )
	// empty socket, please don't put NO_DUMP unless 100% sure
	ROM_LOAD( "xm.bin",   0x0800, 0x0400, CRC(64ebb7de) SHA1(fc5477bbedf44e93a578a71d2ff376f6f0b51a71) ) // ? gfx: B
	// empty socket, "
	ROM_LOAD( "xc.bin",   0x1000, 0x0400, CRC(397fd1f3) SHA1(e6b927933847ddcdbbcbeb5e5f37fea063356b24) )
	// empty socket, "
	// empty socket, "
	ROM_LOAD( "xi.bin",   0x1c00, 0x0400, CRC(ef54efa2) SHA1(c8464f11ccfd9eaf9aefb2cd3ac2b9e8bc2d11b6) ) // contains bitmap for "R.B."
	ROM_LOAD( "xy.bin",   0x2000, 0x0400, CRC(fea8e31e) SHA1(f85eac74d32ebd28170b466b136faf21a8ab220f) )
	ROM_LOAD( "xd.bin",   0x2400, 0x0400, CRC(0c601fc9) SHA1(e655f292b502a14068f5c35428001f8ceedf3637) )
	ROM_LOAD( "xs.bin",   0x2800, 0x0400, CRC(5d15ac52) SHA1(b4f97854018f72e4086c7d830d1b312aea1420a7) )
	ROM_LOAD( "xa.bin",   0x2c00, 0x0400, CRC(a95f5461) SHA1(2645fb93bc4ad5354eef5a385fa94021fb7291dc) ) // ? car - good?
	ROM_LOAD( "xp.bin",   0x3000, 0x0400, CRC(4b6d63ef) SHA1(16f9e31e588b989f5259ab59c0a3a2c7787f3a16) ) // ? gfx: AEIOSXTDNMVGYRPL
	ROM_LOAD( "xo.bin",   0x3400, 0x0400, CRC(c1d7f67c) SHA1(2ddfe9e59e323cd041fd760531b9e15ccd050058) ) // ? gfx: C
	ROM_LOAD( "xr.bin",   0x3800, 0x0400, CRC(8a8667aa) SHA1(53f34b6c5327d4398de644d7f318d460da56c2de) ) // ? gfx: sign+explosion
	ROM_LOAD( "xe.bin",   0x3c00, 0x0400, CRC(e0e81120) SHA1(14a77dfd069be342df4dbb1b747443c6d121d3fe) ) // ? car+misc
ROM_END

} // anonymous namespace


//    YEAR,  NAME,     PARENT,  MACHINE, INPUT,    CLASS,         INIT,       MONITOR, COMPANY,      FULLNAME,                   FLAGS
GAMEL(1983?, imolagp,  0,       imolagp, imolagp,  imolagp_state, empty_init, ROT90,   "RB Bologna", "Imola Grand Prix (set 1)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE, layout_imolagp ) // made by Alberici? year not shown, PCB labels suggests it's from 1983
GAMEL(1983?, imolagpo, imolagp, imolagp, imolagpo, imolagp_state, empty_init, ROT90,   "RB Bologna", "Imola Grand Prix (set 2)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE, layout_imolagp ) // "
