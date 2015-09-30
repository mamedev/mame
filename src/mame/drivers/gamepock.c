// license:BSD-3-Clause
// copyright-holders:Wilbert Pol


#include "emu.h"
#include "cpu/upd7810/upd7810.h"
#include "sound/speaker.h"
#include "bus/generic/carts.h"
#include "includes/gamepock.h"
#include "rendlay.h"


static ADDRESS_MAP_START(gamepock_mem, AS_PROGRAM, 8, gamepock_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000,0x0fff) AM_ROM
	AM_RANGE(0x1000,0x3fff) AM_NOP
	//AM_RANGE(0x4000,0xbfff) AM_ROM        // mapped by the cartslot
	AM_RANGE(0xc000,0xc7ff) AM_MIRROR(0x0800) AM_RAM
	AM_RANGE(0xff80,0xffff) AM_RAM              /* 128 bytes microcontroller RAM */
ADDRESS_MAP_END


static ADDRESS_MAP_START(gamepock_io, AS_IO, 8, gamepock_state )
	AM_RANGE( 0x00, 0x00 ) AM_WRITE( port_a_w )
	AM_RANGE( 0x01, 0x01 ) AM_READWRITE( port_b_r, port_b_w )
	AM_RANGE( 0x02, 0x02 ) AM_READ( port_c_r )
ADDRESS_MAP_END


static INPUT_PORTS_START( gamepock )
	PORT_START("IN0")
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_SELECT )
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 )

	PORT_START("IN1")
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 )
INPUT_PORTS_END


static MACHINE_CONFIG_START( gamepock, gamepock_state )
	MCFG_CPU_ADD("maincpu", UPD78C06, XTAL_6MHz)    /* uPD78C06AG */
	MCFG_CPU_PROGRAM_MAP( gamepock_mem)
	MCFG_CPU_IO_MAP( gamepock_io)
	MCFG_UPD7810_TO(WRITELINE(gamepock_state,gamepock_to_w))

	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE( 60 )
	MCFG_SCREEN_SIZE( 75, 64 )
	MCFG_SCREEN_VISIBLE_AREA( 0, 74, 0, 63 )
	MCFG_SCREEN_UPDATE_DRIVER(gamepock_state, screen_update_gamepock)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEFAULT_LAYOUT(layout_lcd)

	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "gamepock_cart")

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","gamepock")
MACHINE_CONFIG_END


ROM_START( gamepock )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "egpcboot.bin", 0x0000, 0x1000, CRC(ee1ea65d) SHA1(9c7731b5ead721d2cc7f7e2655c5fed9e56db8b0) )
ROM_END


CONS( 1984, gamepock, 0, 0, gamepock, gamepock, driver_device, 0, "Epoch", "Game Pocket Computer", 0 )
