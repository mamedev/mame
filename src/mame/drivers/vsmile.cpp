// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************

    V-Tech V.Smile console emulation

*******************************************************************************/

#include "emu.h"

#include "includes/vsmile.h"

#include "softlist.h"
#include "speaker.h"

/************************************
 *
 *  Common
 *
 ************************************/

void vsmile_base_state::machine_start()
{
	const int bank = m_cart && m_cart->exists() ? 4 : 0;

	m_bankdev->set_bank(bank);
}

WRITE8_MEMBER(vsmile_base_state::chip_sel_w)
{
	const uint16_t cart_offset = m_cart && m_cart->exists() ? 4 : 0;
	switch (data)
	{
		case 0:
			m_bankdev->set_bank(cart_offset);
			break;
		case 1:
			m_bankdev->set_bank(1 + cart_offset);
			break;
		case 2:
		case 3:
			m_bankdev->set_bank(2 + cart_offset);
			break;
	}
	m_maincpu->invalidate_cache();
}

READ16_MEMBER(vsmile_base_state::bank3_r)
{
	return ((uint16_t*)m_system_region->base())[offset];
}

void vsmile_state::machine_start()
{
	vsmile_base_state::machine_start();

	save_item(NAME(m_ctrl_rts));
	save_item(NAME(m_ctrl_select));
}

void vsmile_state::machine_reset()
{
	std::fill(std::begin(m_ctrl_rts), std::end(m_ctrl_rts), false);
	std::fill(std::begin(m_ctrl_select), std::end(m_ctrl_select), false);
}

WRITE8_MEMBER(vsmile_state::ctrl_tx_w)
{
	//printf("Ctrl Tx: %02x\n", data);
	m_spg->uart_rx(data);
}

template <int Which> WRITE_LINE_MEMBER(vsmile_state::ctrl_rts_w)
{
	//printf("Ctrl%d RTS: %d\n", Which, state);
	m_ctrl_rts[Which] = state;
	m_spg->extint_w(Which, state);
}

WRITE8_MEMBER(vsmile_state::uart_rx)
{
	//printf("Ctrl Rx: %02x\n", data);
	m_ctrl[0]->data_w(data);
	m_ctrl[1]->data_w(data);
}

READ16_MEMBER(vsmile_state::portb_r)
{
	return VSMILE_PORTB_OFF_SW | VSMILE_PORTB_ON_SW | VSMILE_PORTB_RESET;
}

WRITE16_MEMBER(vsmile_state::portb_w)
{
	if (BIT(mem_mask, 4))
		m_cart->set_cs2(BIT(data, 4));
}

READ16_MEMBER(vsmile_state::portc_r)
{
	uint16_t data = m_dsw_region->read();
	data |= m_ctrl_rts[0] ? 0 : 0x0400;
	data |= m_ctrl_rts[1] ? 0 : 0x1000;
	data |= 0x0020;
	data |= (m_ctrl_rts[0] && m_ctrl_rts[1]) ? 0x0000 : 0x2000;
	//data = machine().rand() & 0xffff;
	return data;
}

WRITE16_MEMBER(vsmile_state::portc_w)
{
	if (BIT(mem_mask, 8))
	{
		//printf("Ctrl0 SEL: %d\n", BIT(data, 8));
		m_ctrl_select[0] = BIT(data, 8);
		m_ctrl[0]->select_w(m_ctrl_select[0]);
	}
	if (BIT(mem_mask, 9))
	{
		//printf("Ctrl1 SEL: %d\n", BIT(data, 9));
		m_ctrl_select[1] = BIT(data, 9);
		m_ctrl[1]->select_w(m_ctrl_select[1]);
	}
}

/************************************
 *
 *  V.Smile Motion-specific
 *
 ************************************/

WRITE16_MEMBER(vsmilem_state::porta_w)
{
	//printf("Port A write: %04x & %04x\n", data, mem_mask);
}

READ16_MEMBER(vsmilem_state::porta_r)
{
	const uint16_t data = 0xc000;
	//printf("Port A read: %04x & %04x\n", data, mem_mask);
	return data;
}

/************************************
 *
 *  Address Maps
 *
 ************************************/

void vsmile_base_state::mem_map(address_map &map)
{
	map(0x000000, 0x3fffff).rw(m_bankdev, FUNC(address_map_bank_device::read16), FUNC(address_map_bank_device::write16));
	map(0x000000, 0x003fff).m(m_spg, FUNC(spg2xx_device::map));
}

void vsmile_state::banked_map(address_map &map)
{
	map(0x0000000, 0x00fffff).rom().region("maincpu", 0);
	map(0x0100000, 0x01fffff).rom().region("maincpu", 0);
	map(0x0200000, 0x02fffff).rom().region("maincpu", 0);
	map(0x0300000, 0x03fffff).rom().region("maincpu", 0);

	map(0x0400000, 0x04fffff).rom().region("maincpu", 0);
	map(0x0500000, 0x05fffff).rom().region("maincpu", 0);
	map(0x0600000, 0x06fffff).rom().region("maincpu", 0);
	map(0x0700000, 0x07fffff).rom().region("maincpu", 0);

	map(0x0800000, 0x08fffff).rom().region("maincpu", 0);
	map(0x0900000, 0x09fffff).rom().region("maincpu", 0);
	map(0x0a00000, 0x0afffff).rom().region("maincpu", 0);
	map(0x0b00000, 0x0bfffff).rom().region("maincpu", 0);

	map(0x1000000, 0x13fffff).rw(m_cart, FUNC(vsmile_cart_slot_device::bank0_r), FUNC(vsmile_cart_slot_device::bank0_w));

	map(0x1400000, 0x15fffff).rw(m_cart, FUNC(vsmile_cart_slot_device::bank0_r), FUNC(vsmile_cart_slot_device::bank0_w));
	map(0x1600000, 0x17fffff).rw(m_cart, FUNC(vsmile_cart_slot_device::bank2_r), FUNC(vsmile_cart_slot_device::bank2_w));

	map(0x1800000, 0x18fffff).rw(m_cart, FUNC(vsmile_cart_slot_device::bank0_r), FUNC(vsmile_cart_slot_device::bank0_w));
	map(0x1900000, 0x19fffff).rw(m_cart, FUNC(vsmile_cart_slot_device::bank1_r), FUNC(vsmile_cart_slot_device::bank1_w));
	map(0x1a00000, 0x1afffff).rw(m_cart, FUNC(vsmile_cart_slot_device::bank2_r), FUNC(vsmile_cart_slot_device::bank2_w));
	map(0x1b00000, 0x1bfffff).r(FUNC(vsmile_state::bank3_r));
}

/************************************
 *
 *  Inputs
 *
 ************************************/

static INPUT_PORTS_START( vsmile )
	PORT_START("REGION")
	PORT_DIPNAME( 0x0f, 0x04, "BIOS Region" )
	PORT_DIPSETTING(    0x04, "UK/US" )
	PORT_DIPSETTING(    0x07, "China" )
	PORT_DIPSETTING(    0x08, "Mexico" )
	PORT_DIPSETTING(    0x0a, "Italy" )
	PORT_DIPSETTING(    0x0b, "Germany" )
	PORT_DIPSETTING(    0x0c, "Spain" )
	PORT_DIPSETTING(    0x0d, "France" )
	PORT_DIPNAME( 0x10, 0x10, "VTech Intro" )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x10, "On" )
	PORT_BIT( 0xe0, 0x00, IPT_UNUSED )
INPUT_PORTS_END

/************************************
 *
 *  Machine Configs
 *
 ************************************/

static void vsmile_cart(device_slot_interface &device)
{
	device.option_add_internal("vsmile_rom",   VSMILE_ROM_STD);
	device.option_add_internal("vsmile_nvram", VSMILE_ROM_NVRAM);
}

void vsmile_base_state::vsmile_base(machine_config &config)
{
	UNSP(config, m_maincpu, XTAL(27'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &vsmile_base_state::mem_map);
	m_maincpu->set_force_no_drc(true);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update("spg", FUNC(spg2xx_device::screen_update));
	m_screen->screen_vblank().set(m_spg, FUNC(spg2xx_device::vblank));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	SPG24X(config, m_spg, XTAL(27'000'000), m_maincpu, m_screen);
	m_spg->chip_select().set(FUNC(vsmile_base_state::chip_sel_w));
	m_spg->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
	m_spg->add_route(ALL_OUTPUTS, "rspeaker", 0.5);

	ADDRESS_MAP_BANK(config, m_bankdev);
	m_bankdev->set_endianness(ENDIANNESS_LITTLE);
	m_bankdev->set_data_width(16);
	m_bankdev->set_shift(-1);
	m_bankdev->set_stride(0x400000);

	VSMILE_CART_SLOT(config, m_cart, vsmile_cart, nullptr);
}

void vsmile_state::vsmile(machine_config &config)
{
	vsmile_base(config);

	m_bankdev->set_addrmap(AS_PROGRAM, &vsmile_state::banked_map);

	m_spg->portb_in().set(FUNC(vsmile_state::portb_r));
	m_spg->portc_in().set(FUNC(vsmile_state::portc_r));
	m_spg->portc_out().set(FUNC(vsmile_state::portc_w));
	m_spg->uart_tx().set(FUNC(vsmile_state::uart_rx));

	VSMILE_CTRL_PORT(config, m_ctrl[0], vsmile_controllers, "joy");
	m_ctrl[0]->rts_cb().set(FUNC(vsmile_state::ctrl_rts_w<0>));
	m_ctrl[0]->data_cb().set(FUNC(vsmile_state::ctrl_tx_w));

	VSMILE_CTRL_PORT(config, m_ctrl[1], vsmile_controllers, nullptr);
	m_ctrl[1]->rts_cb().set(FUNC(vsmile_state::ctrl_rts_w<1>));
	m_ctrl[1]->data_cb().set(FUNC(vsmile_state::ctrl_tx_w));

	SOFTWARE_LIST(config, "cart_list").set_original("vsmile_cart");
	SOFTWARE_LIST(config, "cart_list2").set_original("vsmilem_cart");
}

void vsmile_state::vsmilep(machine_config &config)
{
	vsmile(config);
	m_spg->set_pal(true);
}

void vsmilem_state::vsmilem(machine_config &config)
{
	vsmile(config);
	m_spg->porta_out().set(FUNC(vsmilem_state::porta_w));
	m_spg->porta_in().set(FUNC(vsmilem_state::porta_r));
}

/************************************
 *
 *  ROM Loading
 *
 ************************************/

// TODO: decide on a dump endian, these likely differ in endianess due to different dumping technqiues
ROM_START( vsmile )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "vsmilebios.bin", 0x000000, 0x200000, CRC(11f1b416) SHA1(11f77c4973d29c962567390e41879c86a759c93b) )
ROM_END

ROM_START( vsmileg )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "bios german.bin", 0x000000, 0x200000, CRC(205c5296) SHA1(7fbcf761b5885c8b1524607aabaf364b4559c8cc) )
ROM_END

ROM_START( vsmilef )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "sysrom_france", 0x000000, 0x200000, CRC(0cd0bdf5) SHA1(5c8d1eada1b6b545555b8d2b09325d7127681af8) )
ROM_END

ROM_START( vsmilem )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "vsmilebios.bin", 0x000000, 0x200000, BAD_DUMP CRC(11f1b416) SHA1(11f77c4973d29c962567390e41879c86a759c93b) )
ROM_END

//    year, name,    parent, compat, machine, input,   class,         init,       company, fullname,              flags
CONS( 2005, vsmile,  0,      0,      vsmile,  vsmile,  vsmile_state,  empty_init, "VTech", "V.Smile (US)",        MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
CONS( 2005, vsmileg, vsmile, 0,      vsmilep, vsmile,  vsmile_state,  empty_init, "VTech", "V.Smile (Germany)",   MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
CONS( 2005, vsmilef, vsmile, 0,      vsmilep, vsmile,  vsmile_state,  empty_init, "VTech", "V.Smile (France)",    MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
CONS( 2008, vsmilem, vsmile, 0,      vsmilem, vsmile,  vsmilem_state, empty_init, "VTech", "V.Smile Motion (US)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
