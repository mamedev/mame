// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    GX800 (Konami Test board)

    driver by Angelo Salese

    Notes:
    Z80, 8KB RAM, 2 * SN76489AN for sound, TTLs for video/misc
    There are 5 x 005273 (Konami custom resistor array (SIL10)) on the PCB,
    also seen on Jail Break HW

    menu translation:
    * screen distortion adjustment normal
    * screen distortion adjustment wide
    * input/output check
    * color check
    * sound check

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/sn76496.h"

#define MAIN_CLOCK XTAL_24MHz

class kontest_state : public driver_device
{
public:
	kontest_state(const machine_config &mconfig, device_type type, const char *tag)
			: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_ram(*this, "ram"),
			m_palette(*this, "palette")
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT8> m_ram;
	required_device<palette_device> m_palette;

	// screen updates
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	// driver state
	UINT8 m_control;

	// member functions
	DECLARE_WRITE8_MEMBER(control_w);

protected:
	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();
public:
	DECLARE_PALETTE_INIT(kontest);
	INTERRUPT_GEN_MEMBER(kontest_interrupt);
};


/***************************************************************************

  Video

***************************************************************************/

PALETTE_INIT_MEMBER(kontest_state, kontest)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int bit0, bit1, bit2 , r, g, b;
	int i;

	for (i = 0; i < 0x20; ++i)
	{
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

void kontest_state::video_start()
{
}

UINT32 kontest_state::screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	int x,y;
	int xi,yi;
	UINT16 tile;
	UINT8 attr;

	for(y=0;y<32;y++)
	{
		for(x=0;x<64;x++)
		{
			tile =  m_ram[(x+y*64)|0x800];
			attr =  m_ram[(x+((y >> 1)*64))|0x000] & 7;
			tile *= 0x10;
			tile += 0x1000;

			for(yi=0;yi<8;yi++)
			{
				for(xi=0;xi<8;xi++)
				{
					UINT8 color,pen[2];
					UINT8 x_step;
					int res_x,res_y;

					x_step = xi >> 2;

					pen[0] =   m_ram[(x_step+yi*2)|(tile)];
					pen[0] >>= 3-((xi & 3));
					pen[0]  &= 1;
					pen[1] =   m_ram[(x_step+yi*2)|(tile)];
					pen[1] >>= 7-((xi & 3));
					pen[1]  &= 1;

					color = pen[0];
					color|= pen[1]<<1;

					res_x = x*8+xi-256;
					res_y = y*8+yi;

					if (cliprect.contains(res_x, res_y))
						bitmap.pix32(res_y, res_x) = m_palette->pen(color|attr*4);
				}
			}
		}
	}

	return 0;
}


/***************************************************************************

  I/O

***************************************************************************/

WRITE8_MEMBER(kontest_state::control_w)
{
	// d3: irq mask
	// d2: ? (reset during 1st grid test and color test)
	// other bits: ?
	m_control = data;

	m_maincpu->set_input_line(0, CLEAR_LINE);
}

static ADDRESS_MAP_START( kontest_map, AS_PROGRAM, 8, kontest_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xe000, 0xffff) AM_RAM AM_SHARE("ram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( kontest_io, AS_IO, 8, kontest_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVWRITE("sn1", sn76489a_device, write)
	AM_RANGE(0x04, 0x04) AM_DEVWRITE("sn2", sn76489a_device, write)
	AM_RANGE(0x08, 0x08) AM_WRITE(control_w)
	AM_RANGE(0x0c, 0x0c) AM_READ_PORT("IN0")
	AM_RANGE(0x0d, 0x0d) AM_READ_PORT("IN1")
	AM_RANGE(0x0e, 0x0e) AM_READ_PORT("IN2")
	AM_RANGE(0x0f, 0x0f) AM_READ_PORT("IN3")
ADDRESS_MAP_END


/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( kontest )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_DIPNAME( 0x80,   0x80, "Orientation" )
	PORT_DIPSETTING(      0x80, "Horizontal" )
	PORT_DIPSETTING(      0x00, "Vertical" )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/***************************************************************************

  Machine Config

***************************************************************************/

INTERRUPT_GEN_MEMBER(kontest_state::kontest_interrupt)
{
	if (m_control & 8)
		device.execute().set_input_line(0, ASSERT_LINE);
}

void kontest_state::machine_start()
{
	// save state
	save_item(NAME(m_control));
}

void kontest_state::machine_reset()
{
	m_control = 0;
}

static MACHINE_CONFIG_START( kontest, kontest_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,MAIN_CLOCK/8)
	MCFG_CPU_PROGRAM_MAP(kontest_map)
	MCFG_CPU_IO_MAP(kontest_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", kontest_state,  kontest_interrupt)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_UPDATE_DRIVER(kontest_state, screen_update)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MCFG_PALETTE_ADD("palette", 32)
	MCFG_PALETTE_INIT_OWNER(kontest_state, kontest)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("sn1", SN76489A, MAIN_CLOCK/16)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)

	MCFG_SOUND_ADD("sn2", SN76489A, MAIN_CLOCK/16)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( kontest )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "800b01.10d",   0x000000, 0x008000, CRC(520f83dc) SHA1(abc23c586864c2ecbc5b16614e27faafc93287de) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "800a02.4f",    0x000000, 0x000020, CRC(6d604171) SHA1(6b1366fb53cecbde6fb651142a77917dd16daf69) )
ROM_END

GAME( 1987?, kontest,  0,   kontest,  kontest, driver_device,  0,       ROT0, "Konami",      "Konami Test Board (GX800, Japan)", MACHINE_SUPPORTS_SAVE ) // late 1987 or early 1988
