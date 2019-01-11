// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn ANC01 6502 2nd Processor

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_ANC01_65022ndproc.html

**********************************************************************/


#include "emu.h"
#include "tube_6502.h"
#include "softlist_dev.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_TUBE_6502, bbc_tube_6502_device, "bbc_tube_6502", "Acorn 6502 2nd Processor")


//-------------------------------------------------
//  ADDRESS_MAP( tube_6502_mem )
//-------------------------------------------------

void bbc_tube_6502_device::tube_6502_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(this, FUNC(bbc_tube_6502_device::read), FUNC(bbc_tube_6502_device::write));
}

//-------------------------------------------------
//  ROM( tube_6502 )
//-------------------------------------------------

ROM_START( tube_6502 )
	ROM_REGION(0x1000, "rom", 0)
	ROM_LOAD("6502tube.rom", 0x0000, 0x1000, CRC(98b5fe42) SHA1(338269d03cf6bfa28e09d1651c273ea53394323b))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(bbc_tube_6502_device::device_add_mconfig)
	MCFG_DEVICE_ADD("m6502", M65C02, XTAL(12'000'000) / 4)
	MCFG_DEVICE_PROGRAM_MAP(tube_6502_mem)

	MCFG_TUBE_ADD("ula")
	MCFG_TUBE_PNMI_HANDLER(INPUTLINE("m6502", M65C02_NMI_LINE))
	MCFG_TUBE_PIRQ_HANDLER(INPUTLINE("m6502", M65C02_IRQ_LINE))

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
	MCFG_RAM_DEFAULT_VALUE(0x00)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("flop_ls_6502", "bbc_flop_6502")
	MCFG_SOFTWARE_LIST_ADD("flop_ls_65c102", "bbc_flop_65c102")
MACHINE_CONFIG_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_tube_6502_device::device_rom_region() const
{
	return ROM_NAME( tube_6502 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_tube_6502_device - constructor
//-------------------------------------------------

bbc_tube_6502_device::bbc_tube_6502_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_TUBE_6502, tag, owner, clock),
		device_bbc_tube_interface(mconfig, *this),
		m_m6502(*this, "m6502"),
		m_ula(*this, "ula"),
		m_ram(*this, "ram"),
		m_rom(*this, "rom"),
		m_rom_enabled(true)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_tube_6502_device::device_start()
{
	m_slot = dynamic_cast<bbc_tube_slot_device *>(owner());
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_tube_6502_device::device_reset()
{
	m_ula->reset();

	m_rom_enabled = true;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER(bbc_tube_6502_device::host_r)
{
	return m_ula->host_r(space, offset);
}

WRITE8_MEMBER(bbc_tube_6502_device::host_w)
{
	m_ula->host_w(space, offset, data);
}


READ8_MEMBER(bbc_tube_6502_device::read)
{
	uint8_t data;

	if (offset >= 0xfef0 && offset <= 0xfeff)
	{
		if (!machine().side_effects_disabled()) m_rom_enabled = false;
		data = m_ula->parasite_r(space, offset);
	}
	else if (m_rom_enabled && (offset >= 0xf000))
	{
		data = m_rom->base()[offset & 0xfff];
	}
	else
	{
		data = m_ram->pointer()[offset];
	}
	return data;
}

WRITE8_MEMBER(bbc_tube_6502_device::write)
{
	if (offset >= 0xfef0 && offset <= 0xfeff)
	{
		m_ula->parasite_w(space, offset, data);
	}
	else
	{
		m_ram->pointer()[offset] = data;
	}
}
