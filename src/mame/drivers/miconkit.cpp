// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************

SNK Micon-Kit series

Games on this hardware:
- Micon-Kit
- Micon-Kit Part II (aka Yamato)
- Space Micon Kit

The cocktail cabinets were called Table Micon, and the upright version of the
first game was apparently called Micon-Block.

Micon-Kit was SNK's first arcade game, it's a simple Breakout clone. The sequel
adds moving obstacles. The 3rd game in the series, Space Micon Kit, adds a 2nd
row of bricks.

Hardware notes:
- NEC D8080A-C, 18.432MHz XTAL, NEC uPB8224C (/9 divider)
- NEC uPB8228C, NEC D8255C
- 4KB ROM (4*MB8518), 256 bytes RAM (2*D2111AL-4)
- 4KB VRAM (32*MB8102), 1bpp video with color overlay
- video timing: 10MHz XTAL, h/v: ?
- beeper

TODO:
- correct video timing
- there's a 2-player start button on the cocktail cabinet, but where is it
  hooked up, if at all? (when inserting 2 coins, the game automatically starts)

******************************************************************************/

#include "emu.h"

#include "cpu/i8085/i8085.h"
#include "machine/i8255.h"
#include "sound/beep.h"

#include "screen.h"
#include "speaker.h"

#include "micon2.lh"


namespace {

class miconkit_state : public driver_device
{
public:
	miconkit_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ppi(*this, "ppi"),
		m_screen(*this, "screen"),
		m_vram(*this, "vram"),
		m_beeper(*this, "beeper"),
		m_inputs(*this, "IN.%u", 0),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void micon2(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<i8255_device> m_ppi;
	required_device<screen_device> m_screen;
	required_shared_ptr<u8> m_vram;
	required_device<beep_device> m_beeper;
	required_ioport_array<5> m_inputs;
	output_finder<2> m_lamps;

	void main_map(address_map &map);
	void io_map(address_map &map);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u8 paddle_r();
	void sound_w(u8 data);
	u8 vblank_r();
	void select_w(u8 data);
	u8 input_r();

	u8 m_select = 0;
};

void miconkit_state::machine_start()
{
	m_lamps.resolve();

	save_item(NAME(m_select));
}



/******************************************************************************
    Video
******************************************************************************/

u32 miconkit_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			int pixel = BIT(m_vram[(y << 4 & 0xfe0) | (x >> 3 & 0x1f)], x & 7);
			bitmap.pix(y, x) = pixel ? rgb_t::white() : rgb_t::black();
		}
	}

	return 0;
}



/******************************************************************************
    I/O
******************************************************************************/

u8 miconkit_state::paddle_r()
{
	return m_inputs[m_select | 2]->read();
}

void miconkit_state::sound_w(u8 data)
{
	// d0-d3: beeper pitch (0 is off)
	data &= 0xf;
	m_beeper->set_state(data != 0);
	m_beeper->set_clock(248 * data);
}

void miconkit_state::select_w(u8 data)
{
	// d0: input select
	m_select = data & 1;

	// d1: 1 player lamp
	// d2: 2 player (cocktail) lamp
	m_lamps[0] = BIT(data, 1);
	m_lamps[1] = BIT(data, 2);
}

u8 miconkit_state::vblank_r()
{
	// d6: vblank flag
	return ~(m_screen->vblank() << 6);
}

u8 miconkit_state::input_r()
{
	// d0: serve button
	// other: misc inputs
	return (m_inputs[m_select]->read() & 1) | (m_inputs[4]->read() & 0xfe);
}



/******************************************************************************
    Address Maps
******************************************************************************/

void miconkit_state::main_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x4000, 0x40ff).ram();
	map(0x7000, 0x7fff).ram().share("vram");
}

void miconkit_state::io_map(address_map &map)
{
	map(0x00, 0x03).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x04, 0x04).r(FUNC(miconkit_state::input_r));
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( micon2 )
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("IN.2")
	PORT_BIT( 0x7f, 0x38, IPT_PADDLE ) PORT_MINMAX(0x00,0x70) PORT_SENSITIVITY(40) PORT_KEYDELTA(8) PORT_CENTERDELTA(0) PORT_REVERSE

	PORT_START("IN.3")
	PORT_BIT( 0x7f, 0x38, IPT_PADDLE ) PORT_MINMAX(0x00,0x70) PORT_SENSITIVITY(40) PORT_KEYDELTA(8) PORT_CENTERDELTA(0) PORT_COCKTAIL

	PORT_START("IN.4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_CUSTOM ) // button
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_CONFNAME( 0x04, 0x04, DEF_STR( Cabinet ) )
	PORT_CONFSETTING(    0x04, DEF_STR( Upright ) )
	PORT_CONFSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_DIPNAME( 0x30, 0x10, "Replay" )           PORT_DIPLOCATION("DSW:4,3")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x10, "400" )
	PORT_DIPSETTING(    0x20, "500" )
	PORT_DIPSETTING(    0x30, "600" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coinage ) ) PORT_DIPLOCATION("DSW:2")
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Lives ) )   PORT_DIPLOCATION("DSW:1")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x80, "5" )
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void miconkit_state::micon2(machine_config &config)
{
	// basic machine hardware
	I8080A(config, m_maincpu, 18.432_MHz_XTAL / 9);
	m_maincpu->set_addrmap(AS_PROGRAM, &miconkit_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &miconkit_state::io_map);

	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set(FUNC(miconkit_state::paddle_r));
	m_ppi->out_pb_callback().set(FUNC(miconkit_state::sound_w));
	m_ppi->out_pc_callback().set(FUNC(miconkit_state::select_w));
	m_ppi->in_pc_callback().set(FUNC(miconkit_state::vblank_r));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 240-1, 24, 256-24-1);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_screen_update(FUNC(miconkit_state::screen_update));
	m_screen->screen_vblank().set(m_ppi, FUNC(i8255_device::pc4_w));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 0).add_route(ALL_OUTPUTS, "mono", 0.25);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( micon2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ufo_no_0", 0x0000, 0x0400, CRC(3eb5a299) SHA1(5e7de4cb8312be8b84f7e5e035b61a6cb9798bc0) )
	ROM_LOAD( "ufo_no_1", 0x0400, 0x0400, CRC(e796338e) SHA1(86c5f283b4a41e19dd0b624d04e1a62ff2ffbf58) )
	ROM_LOAD( "ufo_no_2", 0x0800, 0x0400, CRC(bf246cd7) SHA1(147fb9b877ee108c9c09461ae7e0d72af9ab3275) )
	ROM_LOAD( "ufo_no_3", 0x0c00, 0x0400, CRC(0e93b4f0) SHA1(9405e85a7e005edd0043cb43ce2ef283b4c1b341) )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME    PARENT  MACHINE  INPUT   CLASS           INIT        SCREEN  COMPANY, FULLNAME, FLAGS
GAMEL(1978, micon2, 0,      micon2,  micon2, miconkit_state, empty_init, ROT90,  "SNK", "Micon-Kit Part II", MACHINE_SUPPORTS_SAVE, layout_micon2 )
