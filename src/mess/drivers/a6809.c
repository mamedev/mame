/***************************************************************************

    Acorn 6809

    12/05/2009 Skeleton driver.

    Acorn System 3 update?
    http://acorn.chriswhy.co.uk/8bit_Upgrades/Acorn_6809_CPU.html

****************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/6522via.h"
#include "machine/keyboard.h"
#include "video/saa5050.h"
#include "video/mc6845.h"


class a6809_state : public driver_device
{
public:
	a6809_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_via(*this, "via")
	{ }

	required_device<via6522_device> m_via;

	DECLARE_READ8_MEMBER( via_pb_r );
	DECLARE_WRITE8_MEMBER( kb_w );

	UINT8 m_keydata;
};


static ADDRESS_MAP_START(a6809_mem, AS_PROGRAM, 8, a6809_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000,0x03ff) AM_RAM
	AM_RANGE(0x0400,0x07ff) AM_DEVREADWRITE_LEGACY("saa5050", saa5050_videoram_r, saa5050_videoram_w)
	AM_RANGE(0x0800,0x0800) AM_DEVWRITE("mc6845", mc6845_device, address_w)
	AM_RANGE(0x0801,0x0801) AM_DEVREADWRITE("mc6845", mc6845_device, register_r, register_w)
	AM_RANGE(0x0900,0x090f) AM_MIRROR(0xf0) AM_DEVREADWRITE("via", via6522_device, read, write)
	AM_RANGE(0xf000,0xf7ff) // optional ROM
	AM_RANGE(0xf800,0xffff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( a6809_io, AS_IO, 8, a6809_state)
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( a6809 )
INPUT_PORTS_END


static MACHINE_RESET(a6809)
{
}

static SCREEN_UPDATE_IND16( a6809 )
{
	device_t *saa5050 = screen.machine().device("saa5050");

	saa5050_update(saa5050, bitmap, cliprect);
	saa5050_frame_advance(saa5050);

	return 0;
}

static MC6845_UPDATE_ROW( a6809_update_row )
{
}

static const mc6845_interface a6809_crtc6845_interface =
{
	"screen",
	12 /*?*/,
	NULL,
	a6809_update_row,
	NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	NULL
};

static const saa5050_interface a6809_saa5050_intf =
{
	"screen",
	0,	/* starting gfxnum */
	40, 25, 40,  /* x, y, size */
	0	/* rev y order */
};

READ8_MEMBER( a6809_state::via_pb_r )
{
	return m_keydata;
}

static const via6522_interface via_intf =
{
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(a6809_state, via_pb_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0)
};

WRITE8_MEMBER( a6809_state::kb_w )
{
	m_keydata = data;

	m_via->write_cb1(1);
	m_via->write_cb1(0);
}

static ASCII_KEYBOARD_INTERFACE( kb_intf )
{
	DEVCB_DRIVER_MEMBER(a6809_state, kb_w)
};

static MACHINE_CONFIG_START( a6809, a6809_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M6809E, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(a6809_mem)
	MCFG_CPU_IO_MAP(a6809_io)

	MCFG_MACHINE_RESET(a6809)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(SAA5050_VBLANK))
	MCFG_SCREEN_SIZE(40 * 12, 24 * 20)
	MCFG_SCREEN_VISIBLE_AREA(0, 40 * 12 - 1, 0, 24 * 20 - 1)
	MCFG_SCREEN_UPDATE_STATIC(a6809)

	MCFG_GFXDECODE(saa5050)
	MCFG_PALETTE_LENGTH(128)
	MCFG_PALETTE_INIT(saa5050)

	MCFG_SAA5050_ADD("saa5050", a6809_saa5050_intf)

	MCFG_VIA6522_ADD("via", XTAL_4MHz / 4, via_intf)

	MCFG_MC6845_ADD("mc6845", MC6845, XTAL_4MHz / 2, a6809_crtc6845_interface)

	MCFG_ASCII_KEYBOARD_ADD("keyboard", kb_intf)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( a6809 )
	ROM_REGION( 0x800, "maincpu", 0 )
	ROM_LOAD( "acorn6809.bin", 0x000, 0x800, CRC(5fa5b632) SHA1(b14a884bf82a7a8c23bc03c2e112728dd1a74896) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "acorn6809.ic11", 0x0000, 0x0100, CRC(7908317d) SHA1(e0f1e5bd3a8598d3b62bc432dd1f3892ed7e66d8) ) // address decoder

	ROM_REGION(0x01000, "gfx1",0)
	ROM_LOAD( "p2000.chr", 0x0140, 0x08c0, BAD_DUMP CRC(78c17e3e) SHA1(4e1c59dc484505de1dc0b1ba7e5f70a54b0d4ccc) ) // copied from p2000t.c
ROM_END

/* Driver */

/*    YEAR   NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1980, a6809,  0,      0,       a6809,     a6809, driver_device,   0,      "Acorn",  "System 3 (6809 CPU)", GAME_NOT_WORKING | GAME_NO_SOUND )
