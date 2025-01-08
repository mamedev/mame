// license:BSD-3-Clause
// copyright-holders:hap
/*******************************************************************************

Sun Electronics / Gifu Tokki G.T. Block Challenger
(is "G.T." Gifu Tokki or an indication of table cabinet type like Taito with T.T.?)

Hardware notes:
- NEC 8080A @ 2MHz (18MHz XTAL, 8224 clock divider)
- NEC 8228, 2*NEC 8255C
- 4KB ROM (4*MB8518)
- 256 bytes SRAM(2*MB8101), 1KB SRAM(2*M58754S)
- 1bpp bitmap + ball sprite, paddle sprite
- discrete sound
- 15 switches (not the usual small dipswitches, but separate large switches)

TV Game 8080 hardware is pretty much the same, but on a completely different
cheaper looking PCB. It has 13 switches instead of 15.

To test score-related things: P1 BCD score LSB is in RAM 0x2056, MSB 0x2057.
Every 480 points, the bricks get rebuilt. The game forcibly ends at 1920 points.

TODO:
- Is blockch really Block Challenger, or an older game on the same hardware?
  The flyer has more bricks, and has the score panel at the bottom. Although
  the PCB was purchased as Sun Electronics Block Challenger.
- verify tvgm8080 title, the only reference is from the instruction card which
  said: TV.GAME -8080-
- the flyer photo shows a green screen, assumed to be an overlay on a B&W CRT
- video timing is wrong
- 2nd irq timing is guessed (it's not vblank-out irq, that will cause strange
  delays when the ball hits a brick)
- paddle/ball sprite drawing is guessed
- unknown bits in ppi1_c_r
- identify remaining switches
- cocktail mode
- sound emulation

*******************************************************************************/

#include "emu.h"

#include "cpu/i8085/i8085.h"
#include "machine/i8255.h"
#include "machine/timer.h"

#include "screen.h"

#include "blockch.lh"


namespace {

class blockch_state : public driver_device
{
public:
	blockch_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ppi(*this, "ppi%u", 0),
		m_screen(*this, "screen"),
		m_vram(*this, "vram"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void blockch(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device_array<i8255_device, 2> m_ppi;
	required_device<screen_device> m_screen;
	required_shared_ptr<u8> m_vram;
	required_ioport_array<5> m_inputs;

	u8 m_sound = 0;
	u8 m_ball_x = 0;
	u8 m_ball_y = 0;
	u8 m_vctrl = 0;

	void main_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	void ppi0_c_w(u8 data);
	void ppi1_a_w(u8 data);
	void ppi1_b_w(u8 data);
	void ppi1_c_w(u8 data);
	u8 ppi1_c_r();
};

void blockch_state::machine_start()
{
	save_item(NAME(m_sound));
	save_item(NAME(m_ball_x));
	save_item(NAME(m_ball_y));
	save_item(NAME(m_vctrl));
}



/*******************************************************************************
    Video
*******************************************************************************/

u32 blockch_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	// 96x64 background
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			// VRAM is only 1-bit
			int pixel = m_vram[(x << 4 & 0x1fc0) | (y >> 2 & 0x3f)] & 1;
			bitmap.pix(y, x) = pixel ? rgb_t::white() : rgb_t::black();
		}
	}

	// draw ball sprite
	int bx = (m_vctrl << 8 & 0x100) | m_ball_x;
	bx = 0x200 - bx - 2;
	int by = 0x100 - m_ball_y;

	for (int y = by; y < (by + 4); y++)
		for (int x = bx; x < (bx + 4); x++)
			if (cliprect.contains(x, y))
				bitmap.pix(y, x) = rgb_t::white();

	// draw paddle
	int num_p = (m_inputs[3]->read() & 1) ? 1 : 2;
	const int px[2] = { 44, 184 };
	int py, plen;

	if (m_sound & 0x80)
	{
		py = 0;
		plen = 256;
	}
	else
	{
		py = m_inputs[4]->read();
		if (m_vctrl & 8)
			plen = 8;
		else if (m_vctrl & 4)
			plen = 16;
		else
			plen = 32;
	}

	for (int y = py; y < (py + plen); y++)
		for (int i = 0; i < num_p; i++)
			for (int x = px[i]; x < (px[i] + 4); x++)
				if (cliprect.contains(x, y))
					bitmap.pix(y, x) = rgb_t::white();

	return 0;
}



/*******************************************************************************
    I/O
*******************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(blockch_state::scanline)
{
	int scanline = param;

	// vblank irq
	if (scanline == 248)
		m_maincpu->set_input_line(0, HOLD_LINE);

	// unknown irq
	if (scanline == 64)
		m_maincpu->set_input_line(0, HOLD_LINE);
}

void blockch_state::ppi0_c_w(u8 data)
{
	// sound/lamps?
	// d7 also appears to be for hiding the paddles
	m_sound = data;
}

void blockch_state::ppi1_a_w(u8 data)
{
	m_ball_y = data;
}

void blockch_state::ppi1_b_w(u8 data)
{
	m_ball_x = data;
}

void blockch_state::ppi1_c_w(u8 data)
{
	// d0: ball x hi
	// d1: flip screen
	// d2: shorter paddles (8 points)
	// d3: shorter paddles (hit the ceiling)
	m_vctrl = data;
}

u8 blockch_state::ppi1_c_r()
{
	// d4: ? (game won't boot when 0)
	// d5: ? (paddle collision problem when 1)
	// d6: vblank?
	// d7: upright/cocktail switch
	u8 d6 = m_screen->vblank() ? 0x40 : 0;
	u8 d7 = m_inputs[2]->read() & 0x80;
	return d7 | d6 | 0x1f;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void blockch_state::main_map(address_map &map)
{
	map(0x0000, 0x0fff).mirror(0x1000).rom();
	map(0x2000, 0x20ff).ram();
	map(0x8000, 0x9fff).writeonly().share("vram");
}

void blockch_state::io_map(address_map &map)
{
	map(0x10, 0x13).rw(m_ppi[0], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x14, 0x17).rw(m_ppi[1], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x18, 0x18).portr("IN.4");
	map(0x1c, 0x1c).portr("IN.3");
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( blockch )
	PORT_START("IN.0")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x02, "7" )
	PORT_DIPSETTING(    0x03, "9")
	PORT_DIPNAME( 0x04, 0x04, "Replay" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, "399" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 0_10" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 0_20" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 0_40" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 0_80" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_DIPNAME( 0x10, 0x00, "Obstacles" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Barriers" )
	PORT_DIPSETTING(    0x20, "481, 1441" ) PORT_CONDITION("IN.1", 0x10, EQUALS, 0x00)
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) ) PORT_CONDITION("IN.1", 0x10, EQUALS, 0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Oil" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, "961" ) PORT_CONDITION("IN.1", 0x10, EQUALS, 0x00)
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) ) PORT_CONDITION("IN.1", 0x10, EQUALS, 0x10)
	PORT_DIPNAME( 0x80, 0x80, "Unknown 1_80" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN.2")
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("IN.3")
	PORT_DIPNAME( 0x01, 0x01, "Paddles" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_BIT( 0x7e, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 3_80" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN.4")
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(40) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_MINMAX(0x00, 0xf0)
INPUT_PORTS_END

static INPUT_PORTS_START( tvgm8080 )
	PORT_INCLUDE( blockch )

	PORT_MODIFY("IN.1")
	PORT_DIPNAME( 0x40, 0x40, "Oil" )
	PORT_DIPSETTING(    0x40, "961" ) PORT_CONDITION("IN.1", 0x10, EQUALS, 0x00)
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) ) PORT_CONDITION("IN.1", 0x10, EQUALS, 0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("IN.3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void blockch_state::blockch(machine_config &config)
{
	// basic machine hardware
	I8080A(config, m_maincpu, 18_MHz_XTAL/9);
	m_maincpu->set_addrmap(AS_PROGRAM, &blockch_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &blockch_state::io_map);

	TIMER(config, "scantimer").configure_scanline(FUNC(blockch_state::scanline), "screen", 0, 1);

	I8255(config, m_ppi[0]); // 0x92: A & B = input, C = output
	m_ppi[0]->in_pa_callback().set_ioport("IN.0");
	m_ppi[0]->in_pb_callback().set_ioport("IN.1");
	m_ppi[0]->out_pc_callback().set(FUNC(blockch_state::ppi0_c_w));

	I8255(config, m_ppi[1]); // 0x88: A & B = output, Clow = output, Chigh = input
	m_ppi[1]->out_pa_callback().set(FUNC(blockch_state::ppi1_a_w));
	m_ppi[1]->out_pb_callback().set(FUNC(blockch_state::ppi1_b_w));
	m_ppi[1]->out_pc_callback().set(FUNC(blockch_state::ppi1_c_w));
	m_ppi[1]->in_pc_callback().set(FUNC(blockch_state::ppi1_c_r));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(512, 256);
	m_screen->set_visarea(4, 384-12-1, 4, 248-1);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_screen_update(FUNC(blockch_state::screen_update));

	config.set_default_layout(layout_blockch);

	// sound hardware
	// TODO: discrete?
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( blockch )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("1.62", 0x0000, 0x0400, CRC(94edc2dc) SHA1(3d3b9d996099aa82d2e9b3bed57fe1ce28971e8a))
	ROM_LOAD("2.61", 0x0400, 0x0400, CRC(e29d4d60) SHA1(2766374286ed2a20c772e005bf45b4079c35acd5))
	ROM_LOAD("3.60", 0x0800, 0x0400, CRC(9b15f0fc) SHA1(1ff965cfbccfdee7622f21b33e5a7e3d6292d0bb))
	ROM_LOAD("4.59", 0x0c00, 0x0400, CRC(edf84910) SHA1(292aa16b5f23cae16c03fa7c0e711a5c2a04c27b))
ROM_END

ROM_START( tvgm8080 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("1", 0x0000, 0x0400, CRC(1665afec) SHA1(f4ce9d834a396962d86cf8afd3c2d06ee1709ab6))
	ROM_LOAD("2", 0x0400, 0x0400, CRC(86314a5e) SHA1(b1cf040fceee99d6e192a3506ce71a7a82e426bc))
	ROM_LOAD("3", 0x0800, 0x0400, CRC(7c8d1319) SHA1(37613d4b8e2da7e91d436015124f1ef37011f910))
	ROM_LOAD("4", 0x0c00, 0x0400, CRC(bf3423a8) SHA1(acfe811e2c2a20a306b054e58b0a7493d5d90ba6))
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT   MACHINE  INPUT     CLASS          INIT        SCREEN  COMPANY, FULLNAME, FLAGS
GAME( 1978, blockch,  0,       blockch, blockch,  blockch_state, empty_init, ROT270, "Sun Electronics / Gifu Tokki", "G.T. Block Challenger", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL )
GAME( 1978, tvgm8080, blockch, blockch, tvgm8080, blockch_state, empty_init, ROT270, "bootleg?", "TV Game 8080", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL )
