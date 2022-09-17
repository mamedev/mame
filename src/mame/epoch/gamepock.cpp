// license:BSD-3-Clause
// copyright-holders:Wilbert Pol


#include "emu.h"
#include "gamepock.h"

#include "bus/generic/carts.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


void gamepock_state::gamepock_mem(address_map &map)
{
	map.unmap_value_high();
	// 0x0000-0x0fff is internal ROM
	map(0x1000, 0x3fff).noprw();
	map(0x4000, 0xbfff).r("cartslot", FUNC(generic_slot_device::read_rom));
	map(0xc000, 0xc7ff).mirror(0x0800).ram();
	// 0xff80-0xffff is internal RAM
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


void gamepock_state::gamepock(machine_config &config)
{
	UPD78C06(config, m_maincpu, 6_MHz_XTAL); // uPD78C06AG
	m_maincpu->set_addrmap(AS_PROGRAM, &gamepock_state::gamepock_mem);
	m_maincpu->pa_out_cb().set(FUNC(gamepock_state::port_a_w));
	m_maincpu->pb_in_cb().set(FUNC(gamepock_state::port_b_r));
	m_maincpu->pb_out_cb().set(FUNC(gamepock_state::port_b_w));
	m_maincpu->pc_in_cb().set(FUNC(gamepock_state::port_c_r));
	m_maincpu->to_func().set(m_speaker, FUNC(speaker_sound_device::level_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_size(75, 64);
	screen.set_visarea(0, 74, 0, 63);
	screen.set_screen_update(FUNC(gamepock_state::screen_update_gamepock));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	/* cartridge */
	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "gamepock_cart");

	/* Software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("gamepock");
}


ROM_START( gamepock )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "egpcboot.bin", 0x0000, 0x1000, CRC(ee1ea65d) SHA1(9c7731b5ead721d2cc7f7e2655c5fed9e56db8b0) )
ROM_END


CONS( 1984, gamepock, 0, 0, gamepock, gamepock, gamepock_state, empty_init, "Epoch", "Game Pocket Computer", 0 )
