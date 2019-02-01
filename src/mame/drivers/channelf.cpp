// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller, Frank Palazzolo, Sean Riddle
/******************************************************************
 *
 *  Fairchild Channel F driver
 *
 *  Juergen Buchmueller
 *  Frank Palazzolo
 *  Sean Riddle
 *
 *  Fredric "e5frog" Blaoholtz, added support large cartridges
 *    also spanning from $3000 to $FFFF. Added clones
 *  Fabio "etabeta" Priuli, moved carts to be slot devices
 *
 ******************************************************************/

#include "emu.h"
#include "includes/channelf.h"

#include "screen.h"
#include "softlist.h"
#include "speaker.h"

#define MASTER_CLOCK_PAL    2000000  /* PAL unit has a separate crystal at 4.000 MHz */
#define PAL_VBLANK_TIME     4623


/* The F8 has latches on its port pins
 * These mimic's their behavior
 * [0]=port0, [1]=port1, [2]=port4, [3]=port5
 *
 * Note: this should really be moved into f8.c,
 * but it's complicated by possible peripheral usage.
 *
 * If the read/write operation is going external to the F3850
 * (or F3853, etc.), then the latching applies.  However, relaying the
 * port read/writes from the F3850 to a peripheral like the F3853
 * should not be latched in this way. (See mk1 driver)
 *
 * The f8 cannot determine how its ports are mapped at runtime,
 * so it can't easily decide to latch or not.
 *
 * ...so it stays here for now.
 */

uint8_t channelf_state::port_read_with_latch(uint8_t ext, uint8_t latch_state)
{
	return (~ext | latch_state);
}

READ8_MEMBER( channelf_state::port_0_r )
{
	return port_read_with_latch(ioport("PANEL")->read(), m_latch[0]);
}

READ8_MEMBER( channelf_state::port_1_r )
{
	uint8_t ext_value;

	if ((m_latch[0] & 0x40) == 0)
		ext_value = ioport("RIGHT_C")->read();
	else
		ext_value = 0xc0 | ioport("RIGHT_C")->read();

	return port_read_with_latch(ext_value,m_latch[1]);
}

READ8_MEMBER( channelf_state::port_4_r )
{
	uint8_t ext_value;

	if ((m_latch[0] & 0x40) == 0)
		ext_value = ioport("LEFT_C")->read();
	else
		ext_value = 0xff;

	return port_read_with_latch(ext_value,m_latch[2]);
}

READ8_MEMBER( channelf_state::port_5_r )
{
	return port_read_with_latch(0xff, m_latch[3]);
}

WRITE8_MEMBER( channelf_state::port_0_w )
{
	int offs;

	m_latch[0] = data;

	if (data & 0x20)
	{
		offs = m_row_reg*128+m_col_reg;
		m_p_videoram[offs] = m_val_reg;
	}
}

WRITE8_MEMBER( channelf_state::port_1_w )
{
	m_latch[1] = data;
	m_val_reg = ((data ^ 0xff) >> 6) & 0x03;
}

WRITE8_MEMBER( channelf_state::port_4_w )
{
	m_latch[2] = data;
	m_col_reg = (data | 0x80) ^ 0xff;
}

WRITE8_MEMBER( channelf_state::port_5_w )
{
	m_latch[3] = data;
	m_custom->sound_w((data>>6)&3);
	m_row_reg = (data | 0xc0) ^ 0xff;
}

void channelf_state::channelf_map(address_map &map)
{
	map(0x0000, 0x07ff).rom();
	map(0x0800, 0xffff).r(m_cart, FUNC(channelf_cart_slot_device::read_rom));
}

void channelf_state::channelf_io(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(channelf_state::port_0_r), FUNC(channelf_state::port_0_w)); /* Front panel switches */
	map(0x01, 0x01).rw(FUNC(channelf_state::port_1_r), FUNC(channelf_state::port_1_w)); /* Right controller     */
	map(0x04, 0x04).rw(FUNC(channelf_state::port_4_r), FUNC(channelf_state::port_4_w)); /* Left controller      */
	map(0x05, 0x05).rw(FUNC(channelf_state::port_5_r), FUNC(channelf_state::port_5_w));
}



static INPUT_PORTS_START( channelf )
	PORT_START("PANEL")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("TIME (Button 1)") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("HOLD (Button 2)") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("MODE (Button 3)") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("START (Button 4)") PORT_CODE(KEYCODE_4)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("RIGHT_C")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT) PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_PLAYER(1)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("P1 Twist Counterclockwise") PORT_PLAYER(1)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("P1 Twist Clockwise")PORT_PLAYER(1)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("P1 Pull Up") PORT_PLAYER(1)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("P1 Push Down") PORT_PLAYER(1)

	PORT_START("LEFT_C")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT) PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("P2 Twist Counterclockwise") PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("P2 Twist Clockwise")PORT_PLAYER(2)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("P2 Pull Up") PORT_PLAYER(2)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("P2 Push Down") PORT_PLAYER(2)
INPUT_PORTS_END


void channelf_state::machine_start()
{
	if (m_cart->exists())
	{
		switch (m_cart->get_type())
		{
			case CF_MAZE:
				m_maincpu->space(AS_IO).install_readwrite_handler(0x24, 0x25, read8_delegate(FUNC(channelf_cart_slot_device::read_ram),(channelf_cart_slot_device*)m_cart), write8_delegate(FUNC(channelf_cart_slot_device::write_ram),(channelf_cart_slot_device*)m_cart));
				break;
			case CF_HANGMAN:
				m_maincpu->space(AS_IO).install_readwrite_handler(0x20, 0x21, read8_delegate(FUNC(channelf_cart_slot_device::read_ram),(channelf_cart_slot_device*)m_cart), write8_delegate(FUNC(channelf_cart_slot_device::write_ram),(channelf_cart_slot_device*)m_cart));
				break;
			case CF_CHESS:
				m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x2800, 0x2fff, read8_delegate(FUNC(channelf_cart_slot_device::read_ram),(channelf_cart_slot_device*)m_cart), write8_delegate(FUNC(channelf_cart_slot_device::write_ram),(channelf_cart_slot_device*)m_cart));
				break;
			case CF_MULTI:
			case CF_MULTI_OLD:
				m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x2800, 0x2fff, read8_delegate(FUNC(channelf_cart_slot_device::read_ram),(channelf_cart_slot_device*)m_cart), write8_delegate(FUNC(channelf_cart_slot_device::write_ram),(channelf_cart_slot_device*)m_cart));
				m_maincpu->space(AS_PROGRAM).install_write_handler(0x3000, 0x3fff, write8_delegate(FUNC(channelf_cart_slot_device::write_bank),(channelf_cart_slot_device*)m_cart));
				break;
		}

		m_cart->save_ram();
	}
}

static void cf_cart(device_slot_interface &device)
{
	device.option_add_internal("std",      CHANF_ROM_STD);
	device.option_add_internal("maze",     CHANF_ROM_MAZE);
	device.option_add_internal("hangman",  CHANF_ROM_HANGMAN);
	device.option_add_internal("chess",    CHANF_ROM_CHESS);
	device.option_add_internal("multi_old",CHANF_ROM_MULTI_OLD);
	device.option_add_internal("multi",    CHANF_ROM_MULTI_FINAL);
}


void channelf_state::channelf_cart(machine_config &config)
{
	/* cartridge */
	CHANF_CART_SLOT(config, m_cart, cf_cart, nullptr);

	/* Software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("channelf");
}

MACHINE_CONFIG_START(channelf_state::channelf)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", F8, 3579545/2)        /* Colorburst/2 */
	MCFG_DEVICE_PROGRAM_MAP(channelf_map)
	MCFG_DEVICE_IO_MAP(channelf_io)
	config.m_minimum_quantum = attotime::from_hz(60);

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(128, 64)
	MCFG_SCREEN_VISIBLE_AREA(4, 112 - 7, 4, 64 - 3)
	MCFG_SCREEN_UPDATE_DRIVER(channelf_state, screen_update_channelf)
	MCFG_SCREEN_PALETTE("palette")

	PALETTE(config, "palette", FUNC(channelf_state::channelf_palette), 8);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD("custom", CHANNELF_SOUND)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	channelf_cart(config);
MACHINE_CONFIG_END

MACHINE_CONFIG_START(channelf_state::sabavdpl)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", F8, MASTER_CLOCK_PAL)        /* PAL speed */
	MCFG_DEVICE_PROGRAM_MAP(channelf_map)
	MCFG_DEVICE_IO_MAP(channelf_io)
	config.m_minimum_quantum = attotime::from_hz(50);

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(PAL_VBLANK_TIME)) /* approximate */
	MCFG_SCREEN_SIZE(128, 64)
	MCFG_SCREEN_VISIBLE_AREA(4, 112 - 7, 4, 64 - 3)
	MCFG_SCREEN_UPDATE_DRIVER(channelf_state, screen_update_channelf)
	MCFG_SCREEN_PALETTE("palette")

	PALETTE(config, "palette", FUNC(channelf_state::channelf_palette), 8);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD("custom", CHANNELF_SOUND)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	channelf_cart(config);
MACHINE_CONFIG_END


MACHINE_CONFIG_START(channelf_state::channlf2)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", F8, 3579545/2)        /* Colorburst / 2 */
	MCFG_DEVICE_PROGRAM_MAP(channelf_map)
	MCFG_DEVICE_IO_MAP(channelf_io)
	config.m_minimum_quantum = attotime::from_hz(60);

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(128, 64)
	MCFG_SCREEN_VISIBLE_AREA(4, 112 - 7, 4, 64 - 3)
	MCFG_SCREEN_UPDATE_DRIVER(channelf_state, screen_update_channelf)
	MCFG_SCREEN_PALETTE("palette")

	PALETTE(config, "palette", FUNC(channelf_state::channelf_palette), 8);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD("custom", CHANNELF_SOUND)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	channelf_cart(config);
MACHINE_CONFIG_END


MACHINE_CONFIG_START(channelf_state::sabavpl2)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", F8, MASTER_CLOCK_PAL)        /* PAL speed */
	MCFG_DEVICE_PROGRAM_MAP(channelf_map)
	MCFG_DEVICE_IO_MAP(channelf_io)
	config.m_minimum_quantum = attotime::from_hz(50);

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(PAL_VBLANK_TIME)) /* not accurate */
	MCFG_SCREEN_SIZE(128, 64)
	MCFG_SCREEN_VISIBLE_AREA(4, 112 - 7, 4, 64 - 3)
	MCFG_SCREEN_UPDATE_DRIVER(channelf_state, screen_update_channelf)
	MCFG_SCREEN_PALETTE("palette")

	PALETTE(config, "palette", FUNC(channelf_state::channelf_palette), 8);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD("custom", CHANNELF_SOUND)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	channelf_cart(config);
MACHINE_CONFIG_END

ROM_START( channelf )
	ROM_REGION(0x10000,"maincpu",0)
	ROM_SYSTEM_BIOS( 0, "sl90025", "Luxor Video Entertainment System" )
	ROMX_LOAD("sl90025.rom",  0x0000, 0x0400, CRC(015c1e38) SHA1(759e2ed31fbde4a2d8daf8b9f3e0dffebc90dae2), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "sl31253", "Channel F" )
	ROMX_LOAD("sl31253.rom",  0x0000, 0x0400, CRC(04694ed9) SHA1(81193965a374d77b99b4743d317824b53c3e3c78), ROM_BIOS(1))
	ROM_LOAD("sl31254.rom",   0x0400, 0x0400, CRC(9c047ba3) SHA1(8f70d1b74483ba3a37e86cf16c849d601a8c3d2c))
	ROM_REGION(0x2000, "vram", ROMREGION_ERASE00)
ROM_END

#define rom_sabavdpl rom_channelf
#define rom_luxorves rom_channelf
#define rom_channlf2 rom_channelf
#define rom_sabavdpl rom_channelf
#define rom_sabavpl2 rom_channelf
#define rom_luxorvec rom_channelf
#define rom_itttelma rom_channelf
#define rom_ingtelma rom_channelf

/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR  NAME      PARENT    COMPAT    MACHINE   INPUT     CLASS           INIT        COMPANY         FULLNAME                                FLAGS
CONS( 1976, channelf, 0,        0,        channelf, channelf, channelf_state, empty_init, "Fairchild",    "Channel F",                            0 )
CONS( 1977, sabavdpl, channelf, 0,        sabavdpl, channelf, channelf_state, empty_init, "SABA",         "SABA Videoplay",                       0 )
CONS( 197?, luxorves, channelf, 0,        sabavdpl, channelf, channelf_state, empty_init, "Luxor",        "Luxor Video Entertainment System",     0 )
CONS( 1978, channlf2, 0,        channelf, channlf2, channelf, channelf_state, empty_init, "Fairchild",    "Channel F II",                         0 )
CONS( 1978, sabavpl2, channlf2, 0,        sabavpl2, channelf, channelf_state, empty_init, "SABA",         "SABA Videoplay 2",                     0 )
CONS( 197?, luxorvec, channlf2, 0,        sabavpl2, channelf, channelf_state, empty_init, "Luxor",        "Luxor Video Entertainment Computer",   0 )
CONS( 197?, itttelma, channlf2, 0,        sabavpl2, channelf, channelf_state, empty_init, "ITT",          "ITT Tele-Match Processor",             0 )
CONS( 1978, ingtelma, channlf2, 0,        sabavpl2, channelf, channelf_state, empty_init, "Ingelen",      "Ingelen Tele-Match Processor",         0 )
