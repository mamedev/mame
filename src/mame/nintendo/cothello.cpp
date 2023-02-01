// license:BSD-3-Clause
// copyright-holders:hap
/*******************************************************************************

Nintendo Computer Othello

x

Hardware notes:
- x

TODO:
- WIP

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
		m_vram(*this, "vram")
	{ }

	void cothello(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_shared_ptr<u8> m_vram;

	void main_map(address_map &map);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

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
			bitmap.pix(y, x) = pixel ? rgb_t::white() : rgb_t::black();
		}
	}

	return 0;
}



/*******************************************************************************
    I/O
*******************************************************************************/

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
	map(0x6000, 0x6000).portr("IN.0");
	map(0x8000, 0x8000).w(FUNC(cothello_state::sound_w));
	map(0xa000, 0xa000).portr("IN.1");
	map(0xc000, 0xffff).writeonly().share("vram");
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( cothello )
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_8)

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_I)
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
