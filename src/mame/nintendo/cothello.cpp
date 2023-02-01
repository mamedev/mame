// license:BSD-3-Clause
// copyright-holders:hap
/*******************************************************************************

Nintendo Computer Othello

This is Nintendo's 1st microprocessor-based arcade game.
It's a cocktail cabinet, P1 side has 10 buttons, and P2 side has 4.

In 1980, they also sold a home version. It was called Computer TV Game (model CTG-HC10)
and is presumed to be on the same base hardware as the arcade version.

Hardware notes:
- PCB label: COG CPU
- M58710S (8080A), ?MHz XTAL
- 3*1KB M58732S 2708 ROM, 4th socket is empty
- 256 bytes RAM (2*M58722P 2111A), 0.5KB DRAM (M58755S)
- M58741P Color TV Interface, 64*64 1bpp video
- beeper

TODO:
- verify XTAL
- verify video timing
- coin
- sound
- is there a button select, or is current input emulation correct where for
  example P1 can move P2 cursor? (the only unique P2 button is the Set button)
- unknown if the screen is color or B&W + overlay, but since the video chip is
  meant for a color tv, let's assume the green tint is from the screen itself

*******************************************************************************/

#include "emu.h"

#include "cpu/i8085/i8085.h"

#include "screen.h"
#include "speaker.h"

#include "cothello.lh"


namespace {

class cothello_state : public driver_device
{
public:
	cothello_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_vram(*this, "vram"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void cothello(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_shared_ptr<u8> m_vram;
	required_ioport_array<3> m_inputs;

	void main_map(address_map &map);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u8 input_r();
	u8 coin_r();
	void sound_w(u8 data);
};



/*******************************************************************************
    Video
*******************************************************************************/

u32 cothello_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			int pixel = m_vram[(y << 8 | x) & 0x3fff] & 1;
			bitmap.pix(y, x) = pixel ? rgb_t(0x00, 0xff, 0x80) : rgb_t::black();
		}
	}

	return 0;
}



/*******************************************************************************
    I/O
*******************************************************************************/

u8 cothello_state::input_r()
{
	u8 data = 0xff;

	for (int i = 0; i < 3; i++)
		data &= m_inputs[i]->read();

	return data;
}

u8 cothello_state::coin_r()
{
	return 0xfb;
}

void cothello_state::sound_w(u8 data)
{
	//printf("%X ",data);
}



/******************************************************************************
    Address Maps
******************************************************************************/

void cothello_state::main_map(address_map &map)
{
	map(0x0000, 0x0bff).rom();
	map(0x4000, 0x40ff).ram();
	map(0x6000, 0x6000).r(FUNC(cothello_state::input_r));
	map(0x8000, 0x8000).w(FUNC(cothello_state::sound_w));
	map(0xa000, 0xa000).r(FUNC(cothello_state::coin_r));
	map(0xc000, 0xffff).writeonly().share("vram");
	map(0xc040, 0xc07f).mirror(0x3f00).nopw();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( cothello )
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("1 Player Start Sente")  // 4
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("1 Player Start Gote")   // 3
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START3 ) PORT_NAME("2 Players Start Sente") // 2
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START4 ) PORT_NAME("2 Players Start Gote")  // 1
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Reset")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Abort")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE )

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Pass")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Set")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Pass")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Set")
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void cothello_state::cothello(machine_config &config)
{
	// basic machine hardware
	I8080A(config, m_maincpu, 750000);
	m_maincpu->set_vblank_int("screen", FUNC(cothello_state::irq0_line_hold));
	m_maincpu->set_addrmap(AS_PROGRAM, &cothello_state::main_map);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(64, 64);
	m_screen->set_visarea(0, 64-1, 0, 64-1);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_screen_update(FUNC(cothello_state::screen_update));

	// sound hardware
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( cothello )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "13.ic13", 0x0000, 0x0400, CRC(c4b2802b) SHA1(b7e568c3503722143815b051de2dd3b60cde635a) )
	ROM_LOAD( "12.ic12", 0x0400, 0x0400, CRC(293eee03) SHA1(d3bf755104f2fcbc99ebdc9556b3f42cdfacf94e) )
	ROM_LOAD( "11.ic11", 0x0800, 0x0400, CRC(dea6486e) SHA1(4e11699dfee0e34c67872427372ea0b33bd16d09) )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME      PARENT  MACHINE    INPUT     CLASS           INIT        SCREEN  COMPANY, FULLNAME, FLAGS
GAMEL(1978, cothello, 0,      cothello,  cothello, cothello_state, empty_init, ROT0,   "Nintendo", "Computer Othello", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING | MACHINE_NO_SOUND, layout_cothello )
