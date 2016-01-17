// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    Konami Target Panic (cabinet test PCB)

    driver by Phil Bennett

    TODO: Determine correct clock frequencies, fix 'stuck' inputs in
    test mode

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"


class tgtpanic_state : public driver_device
{
public:
	tgtpanic_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_ram(*this, "ram") { }

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	required_shared_ptr<UINT8> m_ram;

	UINT8 m_color;

	DECLARE_WRITE8_MEMBER(color_w);

	virtual void machine_start() override;

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};


void tgtpanic_state::machine_start()
{
	save_item(NAME(m_color));
}

/*************************************
 *
 *  Video hardware
 *
 *************************************/

UINT32 tgtpanic_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT32 colors[4];
	UINT32 offs;
	UINT32 x, y;

	colors[0] = 0;
	colors[1] = 0xffffffff;
	colors[2] = rgb_t(pal1bit(m_color >> 2), pal1bit(m_color >> 1), pal1bit(m_color >> 0));
	colors[3] = rgb_t(pal1bit(m_color >> 6), pal1bit(m_color >> 5), pal1bit(m_color >> 4));

	for (offs = 0; offs < 0x2000; ++offs)
	{
		UINT8 val = m_ram[offs];

		y = (offs & 0x7f) << 1;
		x = (offs >> 7) << 2;

		/* I'm guessing the hardware doubles lines */
		bitmap.pix32(y + 0, x + 0) = colors[val & 3];
		bitmap.pix32(y + 1, x + 0) = colors[val & 3];
		val >>= 2;
		bitmap.pix32(y + 0, x + 1) = colors[val & 3];
		bitmap.pix32(y + 1, x + 1) = colors[val & 3];
		val >>= 2;
		bitmap.pix32(y + 0, x + 2) = colors[val & 3];
		bitmap.pix32(y + 1, x + 2) = colors[val & 3];
		val >>= 2;
		bitmap.pix32(y + 0, x + 3) = colors[val & 3];
		bitmap.pix32(y + 1, x + 3) = colors[val & 3];
	}

	return 0;
}

WRITE8_MEMBER(tgtpanic_state::color_w)
{
	m_screen->update_partial(m_screen->vpos());
	m_color = data;
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( prg_map, AS_PROGRAM, 8, tgtpanic_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_RAM AM_SHARE("ram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_IO, 8, tgtpanic_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("IN0") AM_WRITE(color_w)
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1")
ADDRESS_MAP_END


/*************************************
 *
 *  Inputs
 *
 *************************************/

static INPUT_PORTS_START( tgtpanic )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( tgtpanic, tgtpanic_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(prg_map)
	MCFG_CPU_IO_MAP(io_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(tgtpanic_state, irq0_line_hold,  20) /* Unverified */

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60) /* Unverified */
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* Unverified */
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 192 - 1, 0, 192 - 1)
	MCFG_SCREEN_UPDATE_DRIVER(tgtpanic_state, screen_update)
MACHINE_CONFIG_END


	/*************************************
	*
	*  ROM definition
	*
	*************************************/

ROM_START( tgtpanic )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "601_ja_a01.13e", 0x0000, 0x8000, CRC(ece71952) SHA1(0f9cbd8adac2b1950bc608d51f0f122399c8f00f) )
ROM_END


/*************************************
 *
 *  Game driver
 *
 *************************************/

GAME( 1996, tgtpanic, 0, tgtpanic, tgtpanic, driver_device, 0, ROT0, "Konami", "Target Panic", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
