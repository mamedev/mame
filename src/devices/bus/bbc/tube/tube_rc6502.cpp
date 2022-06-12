// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
// thanks-to:John Kortink (ReCo6502 developer)
/**********************************************************************

    ReCo6502

    http://www.zeridajh.org/hardware/reco6502/index.htm

    ReCo6502 is a full featured remake of the Acorn 6502 Second Processor
    for the Acorn BBC. In addition to offering backwards compatibility,
    it has the ability to run a modern (WDC manufactured) 65C02 or 65C816
    at 14 MHz, and offers 512 KB of memory.

**********************************************************************/


#include "emu.h"
#include "tube_rc6502.h"
#include "softlist_dev.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_TUBE_RC6502,  bbc_tube_rc6502_device,  "bbc_tube_rc6502",  "ReCo6502 (65C02)")
DEFINE_DEVICE_TYPE(BBC_TUBE_RC65816, bbc_tube_rc65816_device, "bbc_tube_rc65816", "ReCo6502 (65C816)")


//-------------------------------------------------
//  INPUT_PORTS( tube_rc6502 )
//-------------------------------------------------

INPUT_PORTS_START( tube_rc6502 )
	PORT_START("config")
	PORT_CONFNAME(0x04, 0x04, "Link 1: Processor clock frequency")
	PORT_CONFSETTING(0x00, "3.15 MHz")
	PORT_CONFSETTING(0x04, "14.7 MHz")
	PORT_CONFNAME(0x02, 0x00, "Link 2: Auto-load HiBASIC")
	PORT_CONFSETTING(0x00, "Disable")
	PORT_CONFSETTING(0x02, "Enable")
	PORT_CONFNAME(0x01, 0x01, "Link 3: Soft config")
	PORT_CONFSETTING(0x00, "Disable")
	PORT_CONFSETTING(0x01, "Enable")
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor bbc_tube_rc6502_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( tube_rc6502 );
}

//-------------------------------------------------
//  ADDRESS_MAP( tube_rc6502_mem )
//-------------------------------------------------

void bbc_tube_rc6502_device::tube_rc6502_bank(address_map &map)
{
	// Bank 0: ROM disabled with banked RAM (65C02)
	map(0x000000, 0x07ffff).ram().share("ram");
	map(0x004000, 0x007fff).bankrw(m_bank1);
	map(0x008000, 0x00bfff).bankrw(m_bank2);
	// Bank 1: ROM disabled with linear RAM (65C816)
	map(0x100000, 0x17ffff).ram().share("ram");
	// Bank 2: ROM enabled with banked RAM (65C02)
	map(0x200000, 0x27ffff).ram().share("ram");
	map(0x204000, 0x207fff).bankrw(m_bank1);
	map(0x208000, 0x20bfff).bankrw(m_bank2);
	map(0x208000, 0x20ffff).rom().region("maincpu", 0);
	// Bank 3: ROM enabled with linear RAM (65C816)
	map(0x300000, 0x37ffff).ram().share("ram");
	map(0x308000, 0x30ffff).rom().region("maincpu", 0);
}

void bbc_tube_rc6502_device::tube_rc6502_mem(address_map &map)
{
	map(0x00000, 0x0ffff).m(m_bankdev, FUNC(address_map_bank_device::amap8));
	map(0x0fef0, 0x0fef0).rw(FUNC(bbc_tube_rc6502_device::config_r), FUNC(bbc_tube_rc6502_device::register_w));
	map(0x0fef8, 0x0feff).rw(m_ula, FUNC(tube_device::parasite_r), FUNC(tube_device::parasite_w));
}

void bbc_tube_rc65816_device::tube_rc65816_mem(address_map &map)
{
	map(0x00000, 0x7ffff).m(m_bankdev, FUNC(address_map_bank_device::amap8));
	map(0x0fef0, 0x0fef0).rw(FUNC(bbc_tube_rc65816_device::config_r), FUNC(bbc_tube_rc65816_device::register_w));
	map(0x0fef8, 0x0feff).rw(m_ula, FUNC(tube_device::parasite_r), FUNC(tube_device::parasite_w));
}

//-------------------------------------------------
//  ROM( tube_rc6502 )
//-------------------------------------------------

ROM_START( tube_rc6502 )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_DEFAULT_BIOS("hb84")
	ROM_SYSTEM_BIOS(0, "hb84", "HiBASIC '84'")
	ROMX_LOAD("reco6502tube.rom", 0x0000, 0x8000, CRC(6bf531b5) SHA1(83f51b7e1120e14807ed4e24a2fe69c2a3efa416), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "hb02", "HiBASIC 4r32 65C02")
	ROMX_LOAD("reco6502tube_02.rom", 0x0000, 0x8000, CRC(5ce3d04f) SHA1(70b190e251e8807c4120c2ec7b930e62c19e3ea1), ROM_BIOS(1))
ROM_END

ROM_START( tube_rc65816 )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_DEFAULT_BIOS("hb84")
	ROM_SYSTEM_BIOS(0, "hb84", "HiBASIC '84")
	ROMX_LOAD("reco6502tube.rom", 0x0000, 0x8000, CRC(6bf531b5) SHA1(83f51b7e1120e14807ed4e24a2fe69c2a3efa416), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "hb816", "HiBASIC 4r32 65C816")
	ROMX_LOAD("reco6502tube_816.rom", 0x0000, 0x8000, CRC(169a2b8c) SHA1(b9943843f753470b636ee17ad820fa3ccad68249), ROM_BIOS(1))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_tube_rc6502_device::add_common_devices(machine_config &config)
{
	ADDRESS_MAP_BANK(config, m_bankdev).set_map(&bbc_tube_rc6502_device::tube_rc6502_bank).set_options(ENDIANNESS_LITTLE, 8, 22, 0x100000);

	/* internal ram */
	RAM(config, m_ram).set_default_size("512K").set_default_value(0);

	/* software lists */
	SOFTWARE_LIST(config, "flop_ls_6502").set_original("bbc_flop_6502");
}

void bbc_tube_rc6502_device::device_add_mconfig(machine_config &config)
{
	W65C02S(config, m_maincpu, 44.2368_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &bbc_tube_rc6502_device::tube_rc6502_mem);

	add_common_devices(config);

	TUBE(config, m_ula);
	m_ula->pnmi_handler().set_inputline(m_maincpu, M65C02_NMI_LINE);
	m_ula->pirq_handler().set_inputline(m_maincpu, M65C02_IRQ_LINE);
	m_ula->prst_handler().set(FUNC(bbc_tube_rc6502_device::prst_w));
}

void bbc_tube_rc65816_device::device_add_mconfig(machine_config &config)
{
	G65816(config, m_maincpu, 44.2368_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &bbc_tube_rc65816_device::tube_rc65816_mem);

	add_common_devices(config);

	TUBE(config, m_ula);
	m_ula->pnmi_handler().set_inputline(m_maincpu, G65816_LINE_NMI);
	m_ula->pirq_handler().set_inputline(m_maincpu, G65816_LINE_IRQ);
	m_ula->prst_handler().set(FUNC(bbc_tube_rc65816_device::prst_w));
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_tube_rc6502_device::device_rom_region() const
{
	return ROM_NAME( tube_rc6502 );
}

const tiny_rom_entry *bbc_tube_rc65816_device::device_rom_region() const
{
	return ROM_NAME( tube_rc65816 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_tube_rc6502_device - constructor
//-------------------------------------------------

bbc_tube_rc6502_device::bbc_tube_rc6502_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_bbc_tube_interface(mconfig, *this)
	, m_maincpu(*this, "maincpu")
	, m_bankdev(*this, "bankdev")
	, m_bank1(*this, "bank1")
	, m_bank2(*this, "bank2")
	, m_ula(*this, "ula")
	, m_ram(*this, "ram")
	, m_config(*this, "config")
{
}

bbc_tube_rc6502_device::bbc_tube_rc6502_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_tube_rc6502_device(mconfig, BBC_TUBE_RC6502, tag, owner, clock)
{
}

bbc_tube_rc65816_device::bbc_tube_rc65816_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_tube_rc6502_device(mconfig, BBC_TUBE_RC65816, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_tube_rc6502_device::device_start()
{
	save_item(NAME(m_default));
	save_item(NAME(m_divider));
	save_item(NAME(m_banking));
	save_item(NAME(m_banknum));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_tube_rc6502_device::device_reset()
{
	m_bank1->configure_entries(0, 8, m_ram->pointer() + 0x4000, 0x10000);
	m_bank1->set_entry(0);
	m_bank2->configure_entries(0, 8, m_ram->pointer() + 0x8000, 0x10000);
	m_bank2->set_entry(0);
	m_bankdev->set_bank(2);

	// Default hardware registers
	m_default = 0x00;
	m_divider = 0x00;
	m_banking = 0x00;
	m_banknum = m_config->read();
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

WRITE_LINE_MEMBER(bbc_tube_rc6502_device::prst_w)
{
	device_reset();

	m_maincpu->set_input_line(INPUT_LINE_RESET, state);
}

uint8_t bbc_tube_rc6502_device::host_r(offs_t offset)
{
	return m_ula->host_r(offset);
}

void bbc_tube_rc6502_device::host_w(offs_t offset, uint8_t data)
{
	m_ula->host_w(offset, data);
}


uint8_t bbc_tube_rc6502_device::config_r()
{
	return m_banknum;
}

void bbc_tube_rc6502_device::register_w(uint8_t data)
{
	switch (data & 0x06)
	{
	case 0x00:
		// Default - 1-bit register
		m_default = BIT(data, 0);
		m_maincpu->set_clock_scale(1.0 / double(m_divider + 2));
		if (m_default)
		{
			m_bankdev->set_bank(2);
			m_bank1->set_entry(0);
			m_bank2->set_entry(0);
		}
		else
		{
			m_bankdev->set_bank(m_banking >> 2);
			m_bank1->set_entry(BIT(m_banking, 0) ? (m_banknum >> 0) & 0x07 : 0);
			m_bank2->set_entry(BIT(m_banking, 1) ? (m_banknum >> 3) & 0x07 : 0);
		}
		break;

	case 0x02:
		// Divider - 4-bit register
		m_divider >>= 1;
		m_divider |= (BIT(data, 0) << 3);
		break;

	case 0x04:
		// Banking - 4-bit register
		m_banking >>= 1;
		m_banking |= (BIT(data, 0) << 3);
		break;

	case 0x06:
		// Banknum - 6-bit register
		m_banknum >>= 1;
		m_banknum |= (BIT(data, 0) << 5);
		break;
	}
}
