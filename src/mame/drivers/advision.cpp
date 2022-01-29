// license:BSD-3-Clause
// copyright-holders:Dan Boris
/*************************************************************************

    Driver for the Entex Adventure Vision

**************************************************************************/

/*

    TODO:

    - convert to discrete sound
    - screen pincushion distortion

*/

#include "emu.h"
#include "includes/advision.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


/* Memory Maps */

uint8_t advision_state::rom_r(offs_t offset)
{
	offset += 0x400;
	return m_cart->read_rom(offset & 0xfff);
}

void advision_state::program_map(address_map &map)
{
	map(0x0000, 0x03ff).bankr("bank1");
	map(0x0400, 0x0fff).r(FUNC(advision_state::rom_r));
}

void advision_state::io_map(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(advision_state::ext_ram_r), FUNC(advision_state::ext_ram_w));
}

/* Input Ports */

static INPUT_PORTS_START( advision )
	PORT_START("joystick")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4 )       PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 )       PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 )       PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )       PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_8WAY
INPUT_PORTS_END

/* Machine Driver */

void advision_state::advision(machine_config &config)
{
	/* basic machine hardware */
	I8048(config, m_maincpu, XTAL(11'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &advision_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &advision_state::io_map);
	m_maincpu->p1_in_cb().set(FUNC(advision_state::controller_r));
	m_maincpu->p1_out_cb().set(FUNC(advision_state::bankswitch_w));
	m_maincpu->p2_out_cb().set(FUNC(advision_state::av_control_w));
	m_maincpu->t1_in_cb().set(FUNC(advision_state::vsync_r));

	COP411(config, m_soundcpu, 52631*4); // COP411L-KCN/N, R11=82k, C8=56pF
	m_soundcpu->set_config(COP400_CKI_DIVISOR_4, COP400_CKO_RAM_POWER_SUPPLY, false);
	m_soundcpu->read_l().set(FUNC(advision_state::sound_cmd_r));
	m_soundcpu->write_g().set(FUNC(advision_state::sound_g_w));
	m_soundcpu->write_d().set(FUNC(advision_state::sound_d_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(4*15);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(advision_state::screen_update));
	screen.set_size(320, 200);
	screen.set_visarea(84, 235, 60, 142);
	screen.set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(advision_state::advision_palette), 8);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_2BIT_BINARY_WEIGHTED(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.25); // unknown DAC

	/* cartridge */
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "advision_cart");

	/* Software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("advision");
}

/* ROMs */

ROM_START( advision )
	ROM_REGION( 0x1000, I8048_TAG, ROMREGION_ERASE00 )
	ROM_LOAD( "b225__ins8048-11kdp_n.u5", 0x000, 0x400, CRC(279e33d1) SHA1(bf7b0663e9125c9bfb950232eab627d9dbda8460) ) // "<natsemi logo> /B225 \\ INS8048-11KDP/N"

	ROM_REGION( 0x200, COP411_TAG, 0 )
	ROM_LOAD( "b8223__cop411l-kcn_n.u8", 0x000, 0x200, CRC(81e95975) SHA1(8b6f8c30dd3e9d8e43f1ea20fba2361b383790eb) ) // "<natsemi logo> /B8223 \\ COP411L-KCN/N"
ROM_END

/* Game Driver */

/*    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY  FULLNAME            FLAGS */
CONS( 1982, advision, 0,      0,      advision, advision, advision_state, empty_init, "Entex", "Adventure Vision", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
