// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn ADC06 65C102 Co-processor

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_ADC06_65C102CoPro.html

**********************************************************************/


#include "emu.h"
#include "tube_65c102.h"
#include "softlist_dev.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_TUBE_65C102, bbc_tube_65c102_device, "bbc_tube_65c102", "Acorn 65C102 Co-Processor")


//-------------------------------------------------
//  ADDRESS_MAP( tube_6502_mem )
//-------------------------------------------------

void bbc_tube_65c102_device::tube_6502_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(bbc_tube_65c102_device::read), FUNC(bbc_tube_65c102_device::write));
}

//-------------------------------------------------
//  ROM( tube_65c102 )
//-------------------------------------------------

ROM_START( tube_65c102 )
	ROM_REGION(0x1000, "rom", 0)
	ROM_LOAD("65c102_boot_110.rom", 0x0000, 0x1000, CRC(ad5b70cc) SHA1(0ac9a1c70e55a79e2c81e102afae1d016af229fa)) // 2201,243-02
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_tube_65c102_device::device_add_mconfig(machine_config &config)
{
	M65C02(config, m_maincpu, 16_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &bbc_tube_65c102_device::tube_6502_mem);

	TUBE(config, m_ula);
	m_ula->pnmi_handler().set_inputline(m_maincpu, M65C02_NMI_LINE);
	m_ula->pirq_handler().set_inputline(m_maincpu, M65C02_IRQ_LINE);

	/* internal ram */
	RAM(config, m_ram).set_default_size("64K").set_default_value(0);

	/* software lists */
	SOFTWARE_LIST(config, "flop_ls_6502").set_original("bbc_flop_6502");
	SOFTWARE_LIST(config, "flop_ls_65c102").set_original("bbc_flop_65c102");
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_tube_65c102_device::device_rom_region() const
{
	return ROM_NAME( tube_65c102 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_tube_65c102_device - constructor
//-------------------------------------------------

bbc_tube_65c102_device::bbc_tube_65c102_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_TUBE_65C102, tag, owner, clock),
		device_bbc_tube_interface(mconfig, *this),
		m_maincpu(*this, "maincpu"),
		m_ula(*this, "ula"),
		m_ram(*this, "ram"),
		m_rom(*this, "rom"),
		m_rom_enabled(true)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_tube_65c102_device::device_start()
{
	m_slot = dynamic_cast<bbc_tube_slot_device *>(owner());
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_tube_65c102_device::device_reset()
{
	m_ula->reset();

	m_rom_enabled = true;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER(bbc_tube_65c102_device::host_r)
{
	return m_ula->host_r(space, offset);
}

WRITE8_MEMBER(bbc_tube_65c102_device::host_w)
{
	m_ula->host_w(space, offset, data);
}


READ8_MEMBER(bbc_tube_65c102_device::read)
{
	uint8_t data;

	if ((offset >= 0xfef0) && (offset <= 0xfeff))
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

WRITE8_MEMBER(bbc_tube_65c102_device::write)
{
	if ((offset >= 0xfef0) && (offset <= 0xfeff))
	{
		m_ula->parasite_w(space, offset, data);
	}
	else
	{
		m_ram->pointer()[offset] = data;
	}
}
