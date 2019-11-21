// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************

    V-Tech V.Smile Baby console emulation

*******************************************************************************/

#include "emu.h"

#include "includes/vsmile.h"

#include "softlist.h"
#include "speaker.h"


/************************************
 *
 *  V.Smile Baby
 *
 ************************************/

void vsmileb_state::machine_start()
{
	vsmile_base_state::machine_start();

	save_item(NAME(m_mode));
}

void vsmileb_state::machine_reset()
{
	m_mode = 0x0400;
}

READ16_MEMBER(vsmileb_state::porta_r)
{
	return 0x0302 | (m_io_logo->read() ? 0x0080 : 0x0000);
}

READ16_MEMBER(vsmileb_state::portb_r)
{
	return 0x0080;
}

INPUT_CHANGED_MEMBER(vsmileb_state::pad_button_changed)
{
	uint16_t value = m_mode;
	if (newval == 0)
	{
		value |= 0x0080;
	}
	else
	{
		value |= (uint16_t)param;
	}
	m_maincpu->uart_rx((uint8_t)(value >> 8));
	m_maincpu->uart_rx((uint8_t)value);
}

template <uint16_t V> INPUT_CHANGED_MEMBER(vsmileb_state::sw_mode)
{
	if (!newval && oldval)
	{
		m_mode = V;
		const uint16_t value = m_mode | 0x0080;
		m_maincpu->uart_rx((uint8_t)(value >> 8));
		m_maincpu->uart_rx((uint8_t)value);
	}
}

void vsmileb_state::banked_map(address_map &map)
{
	map(0x0000000, 0x03fffff).rom().region("sysrom", 0);
	map(0x0400000, 0x07fffff).rom().region("sysrom", 0);
	map(0x0800000, 0x0bfffff).rom().region("sysrom", 0);

	map(0x1000000, 0x13fffff).rw(m_cart, FUNC(vsmile_cart_slot_device::bank0_r), FUNC(vsmile_cart_slot_device::bank0_w));

	map(0x1400000, 0x15fffff).rw(m_cart, FUNC(vsmile_cart_slot_device::bank0_r), FUNC(vsmile_cart_slot_device::bank0_w));
	map(0x1600000, 0x17fffff).rw(m_cart, FUNC(vsmile_cart_slot_device::bank2_r), FUNC(vsmile_cart_slot_device::bank2_w));

	map(0x1800000, 0x18fffff).rw(m_cart, FUNC(vsmile_cart_slot_device::bank0_r), FUNC(vsmile_cart_slot_device::bank0_w));
	map(0x1900000, 0x19fffff).rw(m_cart, FUNC(vsmile_cart_slot_device::bank1_r), FUNC(vsmile_cart_slot_device::bank1_w));
	map(0x1a00000, 0x1afffff).rw(m_cart, FUNC(vsmile_cart_slot_device::bank2_r), FUNC(vsmile_cart_slot_device::bank2_w));
	map(0x1b00000, 0x1bfffff).r(FUNC(vsmileb_state::bank3_r));
}

/************************************
 *
 *  Inputs
 *
 ************************************/

static INPUT_PORTS_START( vsmileb )
	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmileb_state, pad_button_changed, vsmileb_state::BUTTON_YELLOW) PORT_NAME("Yellow")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmileb_state, pad_button_changed, vsmileb_state::BUTTON_BLUE)   PORT_NAME("Blue")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmileb_state, pad_button_changed, vsmileb_state::BUTTON_ORANGE) PORT_NAME("Orange")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmileb_state, pad_button_changed, vsmileb_state::BUTTON_GREEN)  PORT_NAME("Green")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmileb_state, pad_button_changed, vsmileb_state::BUTTON_RED)    PORT_NAME("Red")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmileb_state, pad_button_changed, vsmileb_state::BUTTON_CLOUD)  PORT_NAME("Cloud")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmileb_state, pad_button_changed, vsmileb_state::BUTTON_BALL)   PORT_NAME("Ball")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_CHANGED_MEMBER(DEVICE_SELF, vsmileb_state, pad_button_changed, vsmileb_state::BUTTON_EXIT)   PORT_NAME("Exit")

	PORT_START("MODE")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Play Time")       PORT_CHANGED_MEMBER(DEVICE_SELF, vsmileb_state, sw_mode<0x0400>, 0) // three-position function switch
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Watch & Learn")   PORT_CHANGED_MEMBER(DEVICE_SELF, vsmileb_state, sw_mode<0x0800>, 0)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Learn & Explore") PORT_CHANGED_MEMBER(DEVICE_SELF, vsmileb_state, sw_mode<0x0c00>, 0)

	PORT_START("LOGO")
	PORT_DIPNAME( 0x10, 0x10, "VTech Intro" )
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )
	PORT_DIPSETTING(    0x10, DEF_STR(On) )
	PORT_BIT( 0xef, 0x00, IPT_UNUSED )
INPUT_PORTS_END

/************************************
 *
 *  Machine Configs
 *
 ************************************/

void vsmileb_state::vsmileb(machine_config &config)
{
	SPG28X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &vsmileb_state::mem_map);
	m_maincpu->set_force_no_drc(true);
	m_maincpu->chip_select().set(FUNC(vsmileb_state::chip_sel_w));
	m_maincpu->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
	m_maincpu->add_route(ALL_OUTPUTS, "rspeaker", 0.5);
	m_maincpu->porta_in().set(FUNC(vsmileb_state::porta_r));
	m_maincpu->portb_in().set(FUNC(vsmileb_state::portb_r));

	vsmile_base(config);

	m_bankdev->set_addrmap(AS_PROGRAM, &vsmileb_state::banked_map);

	SOFTWARE_LIST(config, "cart_list").set_original("vsmileb_cart");
}

void vsmileb_state::vsmilebp(machine_config &config)
{
	vsmileb(config);
	m_maincpu->set_pal(true);
}

/************************************
 *
 *  ROM Loading
 *
 ************************************/

// TODO: decide on a dump endian, these likely differ in endianess due to different dumping technqiues
ROM_START( vsmileb )
	ROM_REGION16_BE( 0x800000, "sysrom", ROMREGION_ERASEFF )
	ROM_LOAD( "vbabybios.bin", 0x000000, 0x800000, CRC(ddc7f845) SHA1(2c17d0f54200070176d03d44a40c7923636e596a) )
ROM_END

ROM_START( vsmilebsw )
	ROM_REGION16_BE( 0x800000, "sysrom", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "vsmileswedenbios.bin", 0x000000, 0x800000, CRC(8b464b19) SHA1(cea304ba886c39e86906aad3dce17d5fff7cfcbe) )
ROM_END

//    year, name,    parent, compat, machine, input,   class,         init,       company, fullname,            flags
CONS( 2005, vsmileb,   0,       0,      vsmileb, vsmileb, vsmileb_state, empty_init, "VTech", "V.Smile Baby (US)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
CONS( 2005, vsmilebsw ,vsmileb, 0,      vsmilebp,vsmileb, vsmileb_state, empty_init, "VTech", "V.Smile Baby (Sweden)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
