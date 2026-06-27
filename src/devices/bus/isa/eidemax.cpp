// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Promise EIDEMAX

    16-bit ISA Enhanced IDE controller

    Notes:
    - To enable large drive support for drives connected to the
      motherboard set the drive type to "1" in the MB BIOS

***************************************************************************/

#include "emu.h"
#include "eidemax.h"

//#define VERBOSE 1
#include "logmacro.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA16_EIDEMAX, eidemax_device, "eidemax", "Promise EIDEMAX")

eidemax_device::eidemax_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA16_EIDEMAX, tag, owner, clock),
	device_isa16_card_interface(mconfig, *this),
	m_ata(*this, "ide"),
	m_config(*this, "config")
{
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( eidemax )
	PORT_START("config")
	PORT_DIPNAME(0x03, 0x00, "JP2: IRQ")
	PORT_DIPSETTING(0x00, "IRQ 15")
	PORT_DIPSETTING(0x01, "IRQ 14")
	PORT_DIPSETTING(0x02, "IRQ 12")
	PORT_DIPNAME(0x04, 0x00, "JP3: IDE Enable")
	PORT_DIPSETTING(0x00, "IDE Enable")
	PORT_DIPSETTING(0x04, "IDE Disable")
	PORT_DIPNAME(0x08, 0x00, "JP3: Secondary/Primary")
	PORT_DIPSETTING(0x00, "Secondary")
	PORT_DIPSETTING(0x08, "Primary")
	PORT_DIPNAME(0x30, 0x00, "JP4: BIOS Address")
	PORT_DIPSETTING(0x00, "D000")
	PORT_DIPSETTING(0x10, "D400")
	PORT_DIPSETTING(0x20, "D800")
	PORT_DIPSETTING(0x30, "DC00")
	PORT_DIPNAME(0x40, 0x00, "JP5: BIOS")
	PORT_DIPSETTING(0x00, "BIOS Enable")
	PORT_DIPSETTING(0x40, "BIOS Disable")
INPUT_PORTS_END

ioport_constructor eidemax_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( eidemax );
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( eidemax )
	ROM_REGION(0x4000, "option", 0)
	ROM_DEFAULT_BIOS("pti007")

	ROM_SYSTEM_BIOS(0, "r113", "EIDE MAX V1.13")
	// - EIDE MAX V1.13 08/07/1995
	// - Second half empty
	ROMX_LOAD("emax_x_r113.u2", 0x0000, 0x4000, CRC(530e397c) SHA1(20c4be8ca7163613861db213ae7a5cf3c6fe1766), ROM_BIOS(0))
	ROM_IGNORE(0x4000)

	ROM_SYSTEM_BIOS(1, "pti004", "EIDE MAX V1.14")
	// - EIDE MAX V1.14 11/24/1995
	ROMX_LOAD("pti-004_jul_1995.u2", 0x0000, 0x4000, CRC(92bf52c4) SHA1(ef1b7a8de2fd12a19c3f2a6ff66779233f273651), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "pti007", "EIDE MAX V1.17")
	// contains the firmware for four different Promise IDE controllers:
	// - EIDE Pro V1.03 11/24/1995
	// - DC4030VL-2 V1.03
	// - EIDE MAX V1.17 11/24/1995 <- we want this one
	// - EIDE2300 V3.31
	ROMX_LOAD("pti-007_oct_1996.u2", 0x0000, 0x4000, CRC(2630489f) SHA1(925340c30eba42d0ff326722592201f2598eb38a), ROM_BIOS(2))
	ROM_CONTINUE(0x0000, 0x4000)
	ROM_CONTINUE(0x0000, 0x4000)
	ROM_IGNORE(0x4000)

	// from the r113 card. "Fatal error: Bad transmission checksum" when trying to convert with jedutil
	ROM_REGION(0x1000, "gals", 0)
	ROM_LOAD("eidemax_gal16v8b_u4.jed", 0x000, 0x2e2, CRC(d71644f5) SHA1(e75e3c381bcbf7dbbdec5fc70dcc9d94066b7f80))
	ROM_LOAD("eidemax_gal16v8b_u5.jed", 0x400, 0x465, CRC(7583ab80) SHA1(e105f258f68ffe62acf2544f2f474850ce07a0a3))
ROM_END

const tiny_rom_entry *eidemax_device::device_rom_region() const
{
	return ROM_NAME( eidemax );
}


//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

void eidemax_device::device_add_mconfig(machine_config &config)
{
	ATA_INTERFACE(config, m_ata).options(ata_devices, nullptr, nullptr, false);
	m_ata->irq_handler().set(FUNC(eidemax_device::ide_interrupt));
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void eidemax_device::device_start()
{
	set_isa_device();
}

void eidemax_device::device_reset()
{
	uint8_t cfg = m_config->read();

	switch (cfg & 0x03)
	{
		case 0x00: m_irq_level = 15; break;
		case 0x01: m_irq_level = 14; break;
		case 0x02: m_irq_level = 12; break;
	}

	// ide enabled?
	if ((cfg & 0x04) == 0x00)
	{
		if ((cfg & 0x08) == 0x00)
			m_base_reg = 0x170;
		else
			m_base_reg = 0x1f0;
	}
	else
	{
		m_base_reg = 0x000;
	}

	// bios enabled?
	if ((cfg & 0x40) == 0x00)
	{
		switch (cfg & 0x30)
		{
			case 0x00: m_rom_base = 0xd0000; break;
			case 0x10: m_rom_base = 0xd4000; break;
			case 0x20: m_rom_base = 0xd8000; break;
			case 0x30: m_rom_base = 0xdc000; break;
		}
	}
	else
	{
		m_rom_base = 0;
	}

	remap(AS_PROGRAM, 0x00000, 0xfffff);
	remap(AS_IO, 0x0000, 0xffff);
}

void eidemax_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_IO)
	{
		// install ide registers
		if (m_base_reg)
		{
			m_isa->install16_device(m_base_reg + 0x000, m_base_reg + 0x007, read16s_delegate(*this, FUNC(eidemax_device::cs0_r)), write16s_delegate(*this, FUNC(eidemax_device::cs0_w)));
			m_isa->install_device(m_base_reg + 0x206, m_base_reg + 0x206, read8sm_delegate(*this, FUNC(eidemax_device::cs1_r)), write8sm_delegate(*this, FUNC(eidemax_device::cs1_w)));
		}
	}

	if (space_id == AS_PROGRAM)
	{
		// install bios rom
		if (m_rom_base)
			m_isa->install_rom(this, m_rom_base, m_rom_base | 0x3fff, "option");
	}
}

uint16_t eidemax_device::cs0_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0;

	if (ACCESSING_BITS_0_7 && (offset == 0))
		data |= m_ata->cs0_r(offset * 2 + 0, mem_mask);
	else if (ACCESSING_BITS_0_7)
		data |= m_ata->cs0_r(offset * 2 + 0, 0x00ff) << 0;
	else if (ACCESSING_BITS_8_15)
		data |= m_ata->cs0_r(offset * 2 + 1, 0x00ff) << 8;

	// clear index bit, otherwise drives aren't detected (compares status with 0x58)
	if (ACCESSING_BITS_8_15 && offset == 3)
		data &= 0xfd00;

	LOG("cs0_r: %04x = %04x (%04x)\n", offset, data, mem_mask);

	return data;
}

void eidemax_device::cs0_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOG("cs0_w: %04x = %04x (%04x)\n", offset, data, mem_mask);

	if (ACCESSING_BITS_0_7 && (offset == 0))
		m_ata->cs0_w(offset * 2 + 0, data, mem_mask);
	else if (ACCESSING_BITS_0_7)
		m_ata->cs0_w(offset * 2 + 0, data >> 0, 0x00ff);
	else if (ACCESSING_BITS_8_15)
		m_ata->cs0_w(offset * 2 + 1, data >> 8, 0x00ff);
}

uint8_t eidemax_device::cs1_r(offs_t offset)
{
	uint8_t data = m_ata->cs1_r(6, 0x00ff);

	LOG("cs1_r: %04x\n", data);

	return data;
}

void eidemax_device::cs1_w(offs_t offset, uint8_t data)
{
	LOG("cs1_w: %02x\n", data);

	m_ata->cs1_w(6, data, 0x00ff);
}

void eidemax_device::ide_interrupt(int state)
{
	switch (m_irq_level)
	{
		case 15: m_isa->irq15_w(state); break;
		case 14: m_isa->irq14_w(state); break;
		case 12: m_isa->irq12_w(state); break;
	}
}
