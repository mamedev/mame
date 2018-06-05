// license:BSD-3-Clause
// copyright-holders:Wilbert Pol


#include "emu.h"
#include "includes/gamepock.h"

#include "bus/generic/carts.h"
#include "cpu/upd7810/upd7810.h"

#include "rendlay.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"


void gamepock_state::gamepock_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).rom();
	map(0x1000, 0x3fff).noprw();
	//AM_RANGE(0x4000,0xbfff) AM_ROM        // mapped by the cartslot
	map(0xc000, 0xc7ff).mirror(0x0800).ram();
	map(0xff80, 0xffff).ram();              /* 128 bytes microcontroller RAM */
}


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


MACHINE_CONFIG_START(gamepock_state::gamepock)
	MCFG_DEVICE_ADD("maincpu", UPD78C06, XTAL(6'000'000))    /* uPD78C06AG */
	MCFG_DEVICE_PROGRAM_MAP( gamepock_mem)
	MCFG_UPD7810_PORTA_WRITE_CB(WRITE8(*this, gamepock_state, port_a_w))
	MCFG_UPD7810_PORTB_READ_CB(READ8(*this, gamepock_state, port_b_r))
	MCFG_UPD7810_PORTB_WRITE_CB(WRITE8(*this, gamepock_state, port_b_w))
	MCFG_UPD7810_PORTC_READ_CB(READ8(*this, gamepock_state, port_c_r))
	MCFG_UPD7810_TO(WRITELINE(*this, gamepock_state,gamepock_to_w))

	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE( 60 )
	MCFG_SCREEN_SIZE( 75, 64 )
	MCFG_SCREEN_VISIBLE_AREA( 0, 74, 0, 63 )
	MCFG_SCREEN_UPDATE_DRIVER(gamepock_state, screen_update_gamepock)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEFAULT_LAYOUT(layout_lcd)

	MCFG_PALETTE_ADD_MONOCHROME("palette")

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD("speaker", SPEAKER_SOUND)
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


CONS( 1984, gamepock, 0, 0, gamepock, gamepock, gamepock_state, empty_init, "Epoch", "Game Pocket Computer", 0 )
