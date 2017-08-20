// license:BSD-3-Clause
// copyright-holder:FelipeSanches

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "video/nt7534.h"
#include "screen.h"

class controlidx628_state : public driver_device
{
public:
	controlidx628_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_lcdc(*this, "nt7534") { }
	DECLARE_WRITE8_MEMBER( p0_w );
	DECLARE_WRITE8_MEMBER( p1_w );
        DECLARE_PALETTE_INIT( controlidx628 );

	required_device<nt7534_device> m_lcdc;
private:
	uint8_t p0_data;
	uint8_t p1_data;
};


/*************************
* Memory map information *
*************************/

static ADDRESS_MAP_START( prog_map, AS_PROGRAM, 8, controlidx628_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_IO, 8, controlidx628_state )
	AM_RANGE(0x8000, 0xffff) AM_RAM

//	/* Ports start here */
	AM_RANGE(MCS51_PORT_P0, MCS51_PORT_P0) AM_WRITE(p0_w)
	AM_RANGE(MCS51_PORT_P1, MCS51_PORT_P1) AM_WRITE(p1_w)
//	AM_RANGE(MCS51_PORT_P2, MCS51_PORT_P2) AM_RAM
//	AM_RANGE(MCS51_PORT_P3, MCS51_PORT_P3) AM_RAM
ADDRESS_MAP_END


WRITE8_MEMBER( controlidx628_state::p0_w )
{
	p0_data = data;
}

WRITE8_MEMBER( controlidx628_state::p1_w )
{
	if ((BIT(p1_data, 6) == 0) && (BIT(data, 6) == 1)){
		m_lcdc->write(space, BIT(data, 7), p0_data);
	}
	p1_data = data;
}

/*************************
*      Input ports       *
*************************/

//static INPUT_PORTS_START( controlidx628 )
//INPUT_PORTS_END

PALETTE_INIT_MEMBER(controlidx628_state, controlidx628)
{
        palette.set_pen_color(0, rgb_t(0xCA, 0xE7, 0xEB));
        palette.set_pen_color(1, rgb_t(0x78, 0xAB, 0xA8));
}

/*************************
*     Machine Driver     *
*************************/

static MACHINE_CONFIG_START( controlidx628 )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", I80C32, XTAL_11_0592MHz) /* Actually the board has an Atmel AT89S52 mcu. */
	MCFG_CPU_PROGRAM_MAP(prog_map)
	MCFG_CPU_IO_MAP(io_map)

        /* video hardware */
        MCFG_SCREEN_ADD("screen", LCD)
        MCFG_SCREEN_REFRESH_RATE(50)
        MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
        MCFG_SCREEN_SIZE(132, 65)
        MCFG_SCREEN_VISIBLE_AREA(4 + 0, 4 + 127, 0, 63)
//        MCFG_DEFAULT_LAYOUT(layout_lcd)
        MCFG_SCREEN_UPDATE_DEVICE("nt7534", nt7534_device, screen_update)
        MCFG_SCREEN_PALETTE("palette")

        MCFG_PALETTE_ADD("palette", 2)
        MCFG_PALETTE_INIT_OWNER(controlidx628_state, controlidx628)

        MCFG_NT7534_ADD("nt7534")
MACHINE_CONFIG_END


/*************************
*        Rom Load        *
*************************/

ROM_START( cidx628 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "controlid_x628.u1",   0x0000, 0x2000, CRC(500d79b4) SHA1(5522115f2da622db389e067fcdd4bccb7aa8561a) )
ROM_END

COMP(200?, cidx628, 0, 0, controlidx628, 0, controlidx628_state, 0, "ControlID", "X628", MACHINE_NOT_WORKING|MACHINE_NO_SOUND)
