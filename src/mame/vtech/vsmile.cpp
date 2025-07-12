// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************

    V-Tech V.Smile console emulation

*******************************************************************************/

#include "emu.h"

#include "vsmile.h"

#include "softlist_dev.h"
#include "speaker.h"

#include "vsmile.lh"

#define VERBOSE (1)
#include "logmacro.h"

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

void vsmile_base_state::chip_sel_w(uint8_t data)
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

uint16_t vsmile_base_state::bank3_r(offs_t offset)
{
	return ((uint16_t*)m_system_region->base())[offset];
}

void vsmile_state::machine_start()
{
	vsmile_base_state::machine_start();

	m_redled.resolve();
	m_yellowled.resolve();
	m_blueled.resolve();
	m_greenled.resolve();

	save_item(NAME(m_ctrl_rts));
	save_item(NAME(m_ctrl_select));
}

void vsmile_state::machine_reset()
{
	std::fill(std::begin(m_ctrl_rts), std::end(m_ctrl_rts), false);
	std::fill(std::begin(m_ctrl_select), std::end(m_ctrl_select), false);
}

void vsmile_state::ctrl_tx_w(uint8_t data)
{
	//printf("Ctrl Tx: %02x\n", data);
	m_maincpu->uart_rx(data);
}

template <int Which> void vsmile_state::ctrl_rts_w(int state)
{
	//printf("Ctrl%d RTS: %d\n", Which, state);
	m_ctrl_rts[Which] = state;
	m_maincpu->extint_w(Which, state);
}

void vsmile_state::uart_rx(uint8_t data)
{
	//printf("Ctrl Rx: %02x\n", data);
	m_ctrl[0]->data_w(data);
	m_ctrl[1]->data_w(data);

	//TODO: should be moved to pad code somehow
	if ((data & 0xF0) == 0x60)
	{
		if (m_ctrl_select[0])
		{
			m_redled[0] = BIT(data, 3);
			m_yellowled[0] = BIT(data, 2);
			m_blueled[0] = BIT(data, 1);
			m_greenled[0] = BIT(data, 0);
		}
		if (m_ctrl_select[1])
		{
			m_redled[1] = BIT(data, 3);
			m_yellowled[1] = BIT(data, 2);
			m_blueled[1] = BIT(data, 1);
			m_greenled[1] = BIT(data, 0);
		}
	}
}

uint16_t vsmile_state::portb_r()
{
	uint16_t data = m_dsw_system->read();
	//bit 0 : extra address bit for the cartridge port, access second half of ROM (TODO)
	//bit 1 : Set to 0 to enable cartridge ROM (TODO) -> getCS2
	//bit 2 : Set to 0 to enable internal ROM (TODO)
	//bit 3 : restart (see dipswitch)
	//      VSMILE_PORTB_RESET
	//bit 4 : ADC (TODO)
	//bit 5 : Voltage detect (TODO)
	//bit 6 : ON button, active low (see dipswitch)
	//      VSMILE_PORTB_ON_SW
	//bit 7 : OFF button, active low (see dipswitch)
	//      VSMILE_PORTB_OFF_SW

	//LOG("%s: portb_r: %04x\n", machine().describe_context(), data);

	//On Vsmile, VSMILE_PORTB_RESET, VSMILE_PORTB_OFF_SW and VSMILE_PORTB_ON_SW actives will trigger BIOS test screen
	return data;
}

void vsmile_state::portb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOG("%s: portb_w: %04x & %04x (bit 1: %d & %d)\n", machine().describe_context(), data, mem_mask, BIT(data, 1), BIT(mem_mask, 1));
	if (BIT(mem_mask, 1) && m_cart && m_cart->exists())
		m_cart->set_cs2(BIT(~data, 1));
}

uint16_t vsmile_state::portc_r()
{
	uint16_t data = m_dsw_region->read();
	data |= m_ctrl_rts[0] ? 0 : 0x0400;
	data |= m_ctrl_rts[1] ? 0 : 0x1000;
	data |= 0x0020; //IOC5 - TestPoint
	data |= (m_ctrl_rts[0] && m_ctrl_rts[1]) ? 0x0000 : 0x2000;
	//data = machine().rand() & 0xffff;
	return data;
}

void vsmile_state::portc_w(offs_t offset, uint16_t data, uint16_t mem_mask)
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

void vsmilem_state::porta_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	//printf("Port A write: %04x & %04x\n", data, mem_mask);
}

uint16_t vsmilem_state::porta_r(offs_t offset, uint16_t mem_mask)
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
}

void vsmile_state::banked_map(address_map &map)
{
	map(0x0000000, 0x00fffff).rom().region("sysrom", 0);
	map(0x0100000, 0x01fffff).rom().region("sysrom", 0);
	map(0x0200000, 0x02fffff).rom().region("sysrom", 0);
	map(0x0300000, 0x03fffff).rom().region("sysrom", 0);

	map(0x0400000, 0x04fffff).rom().region("sysrom", 0);
	map(0x0500000, 0x05fffff).rom().region("sysrom", 0);
	map(0x0600000, 0x06fffff).rom().region("sysrom", 0);
	map(0x0700000, 0x07fffff).rom().region("sysrom", 0);

	map(0x0800000, 0x08fffff).rom().region("sysrom", 0);
	map(0x0900000, 0x09fffff).rom().region("sysrom", 0);
	map(0x0a00000, 0x0afffff).rom().region("sysrom", 0);
	map(0x0b00000, 0x0bfffff).rom().region("sysrom", 0);

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
	//based on schematics and BIOS test screen
	PORT_CONFNAME( 0x0f, 0x0f, DEF_STR(Language) )
	PORT_CONFSETTING(    0x02, DEF_STR(Italian) ) //IT
	PORT_CONFSETTING(    0x07, DEF_STR(Chinese) ) //Chinese
	PORT_CONFSETTING(    0x08, "Portuguese" ) //PO
	PORT_CONFSETTING(    0x09, "Dutch" ) //DU
	PORT_CONFSETTING(    0x0b, DEF_STR(German) ) //GE
	PORT_CONFSETTING(    0x0c, DEF_STR(Spanish) ) //SP
	PORT_CONFSETTING(    0x0d, DEF_STR(French) ) //FR
	PORT_CONFSETTING(    0x0e, "English (UK)" ) //UK
	PORT_CONFSETTING(    0x0f, "English (US)" ) //US
	PORT_CONFNAME( 0x10, 0x10, "VTech Intro" )
	PORT_CONFSETTING(    0x00, DEF_STR(Off) )
	PORT_CONFSETTING(    0x10, DEF_STR(On) )
	PORT_BIT( 0xe0, 0x00, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POWER_OFF )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POWER_ON )
	PORT_CONFNAME( 0x08, 0x08, "Restart")
	PORT_CONFSETTING(    0x08, DEF_STR(Off) )
	PORT_CONFSETTING(    0x00, DEF_STR(On) )
	PORT_BIT( 0x37, 0x00, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( vsmilem )
	PORT_START("REGION")
	PORT_CONFNAME( 0x0f, 0x0f, "sysrom Region" )
	PORT_CONFSETTING(    0x02, "Italy" ) // V.Smile Motion logo with "Active Learning System", voice 1, regular cartridge image, "Per favore inserisci una cartuccia di gioco" text (possibly invalid as text on logo is still in English and Italy was previously 0x0a)
	PORT_CONFSETTING(    0x05, "English (1)" ) // V.Smile Motion logo with "Active Learning System", voice 2, regular cartridge image, "Please insert a Learning Game" text
	PORT_CONFSETTING(    0x06, "English (2)" ) // V.Smile Motion logo with "Active Learning System", voice 1, regular cartridge image, "Please insert a Learning Game" text
	PORT_CONFSETTING(    0x07, "China" ) // V.Smile Motion logo with "Active Learning System", voice 1, regular cartridge image, Chinese text
	PORT_CONFSETTING(    0x08, "Mexico" ) // V.Smile Motion logo with "Sistema Educativo", voice 1, regular cartridge image, "TV Learning System" text
	PORT_CONFSETTING(    0x09, "Netherlands?" ) // V.Smile Motion logo with "Active Learning System", voice 3, regular cartridge image, "Plaats een game"
	PORT_CONFSETTING(    0x0b, "Germany" ) // V.Smile Motion logo with "Aktives Lernspiel - System", voice 4, regular cartridge image, "Bitte Lernspiel einstecken"
	PORT_CONFSETTING(    0x0c, "Spain" ) // V.Smile Motion logo with "Aprendizaje Inteligente En Accion", voice 5, regular cartridge image, "Por favor, inserta un cartuncho"
	PORT_CONFSETTING(    0x0d, "France" ) // V.Smile Motion logo with "Apprendre En Mouvements", voice 6, regular cartridge image, "Inserer une cartouche"
	PORT_CONFSETTING(    0x0f, "English (3)" ) // V.Smile Motion logo with "Active Learning System", voice 2, regular cartridge image, "Please insert a Smartridge(tm)" text   (Smartridge must be a region specific term?)

	PORT_CONFNAME( 0x10, 0x10, "VTech Intro" )
	PORT_CONFSETTING(    0x00, "Off" )
	PORT_CONFSETTING(    0x10, "On" )
	PORT_BIT( 0xe0, 0x00, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POWER_OFF )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POWER_ON )
	PORT_CONFNAME( 0x08, 0x08, "Restart")
	PORT_CONFSETTING(    0x08, DEF_STR(Off) )
	PORT_CONFSETTING(    0x00, DEF_STR(On) )
	PORT_BIT( 0x37, 0x00, IPT_UNUSED )
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
	config.set_default_layout(layout_vsmile);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update("maincpu", FUNC(spg2xx_device::screen_update));
	m_screen->screen_vblank().set(m_maincpu, FUNC(spg2xx_device::vblank));

	SPEAKER(config, "speaker", 2).front();

	ADDRESS_MAP_BANK(config, m_bankdev);
	m_bankdev->set_endianness(ENDIANNESS_BIG);
	m_bankdev->set_data_width(16);
	m_bankdev->set_shift(-1);
	m_bankdev->set_stride(0x400000);

	VSMILE_CART_SLOT(config, m_cart, vsmile_cart, nullptr);
}

void vsmile_state::vsmile(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &vsmile_state::mem_map);
	m_maincpu->set_force_no_drc(true);
	m_maincpu->chip_select().set(FUNC(vsmile_state::chip_sel_w));
	m_maincpu->add_route(ALL_OUTPUTS, "speaker", 0.5, 0);
	m_maincpu->add_route(ALL_OUTPUTS, "speaker", 0.5, 1);
	m_maincpu->portb_in().set(FUNC(vsmile_state::portb_r));
	m_maincpu->portb_out().set(FUNC(vsmile_state::portb_w));
	m_maincpu->portc_in().set(FUNC(vsmile_state::portc_r));
	m_maincpu->portc_out().set(FUNC(vsmile_state::portc_w));
	m_maincpu->uart_tx().set(FUNC(vsmile_state::uart_rx));

	vsmile_base(config);

	m_bankdev->set_addrmap(AS_PROGRAM, &vsmile_state::banked_map);

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
	m_maincpu->set_pal(true);
}

void vsmilem_state::vsmilem(machine_config &config)
{
	vsmile(config);
	m_maincpu->porta_out().set(FUNC(vsmilem_state::porta_w));
	m_maincpu->porta_in().set(FUNC(vsmilem_state::porta_r));
}

/************************************
 *
 *  ROM Loading
 *
 ************************************/

// NOTE: many games contain additional spare copies of the BIOS in their own cartridge ROM, reason unknown

ROM_START( vsmile )
	ROM_REGION16_BE( 0x800000, "sysrom", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v103", "v103" )
	ROMX_LOAD( "vsmile_v103.bin", 0x000000, 0x200000, CRC(387fbc24) SHA1(5f2fd211b6ff3a6f5121b14adc6bbf4f49e89f33),  ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(0) ) // this is the earliest version used on the V.Smile Pocket, but it isn't system specific
	ROM_SYSTEM_BIOS( 1, "v102", "v102" )
	ROMX_LOAD( "vsmile_v102.bin", 0x000000, 0x200000, CRC(0cd0bdf5) SHA1(5c8d1eada1b6b545555b8d2b09325d7127681af8),  ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(1) ) // found in all 'fat' model systems
	ROM_SYSTEM_BIOS( 2, "v100", "v100" )
	ROMX_LOAD( "vsmile_v100.bin", 0x000000, 0x200000, CRC(205c5296) SHA1(7fbcf761b5885c8b1524607aabaf364b4559c8cc),  ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(2) )
ROM_END


ROM_START( vsmilem )
	ROM_REGION16_BE( 0x800000, "sysrom", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "bios0", "bios0" )
	ROMX_LOAD( "vsmilemotion.bin", 0x000000, 0x200000, CRC(60fa5426) SHA1(91e0b7b44b975df65095d6ee622436d65fb1aca5), ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(0) ) // from a Spanish unit (but doesn't seem region specific)

	/* This ROM doesn't show the 'Motion' logo at all, but was dumped from a Motion unit

	    Console says "Vtech V.Smile V-motion Active Learning System"
	    "FCC ID 62R-0788, IC 1135D-0788" "53-36600-056-080"
	    melted into plastic "VT8281"
	    The PCB has the code 35-078800-001-103_708979-2.
	*/
	ROM_SYSTEM_BIOS( 1, "bios1", "bios1" )
	ROMX_LOAD( "vmotionbios.bin", 0x000000, 0x200000, CRC(427087ea) SHA1(dc9eaa55f4a0047b6069ef73beea86d26f0f5394), ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(1) ) // from a US unit
ROM_END


//    year, name,    parent, compat, machine, input,   class,         init,       company, fullname,         flags
CONS( 2005, vsmile,  0,      0,      vsmile,  vsmile,  vsmile_state,  empty_init, "VTech", "V.Smile",        MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
CONS( 2008, vsmilem, vsmile, 0,      vsmilem, vsmilem, vsmilem_state, empty_init, "VTech", "V.Smile Motion", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
