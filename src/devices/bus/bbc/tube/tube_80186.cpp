// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn ADC08 Intel 80186 Co-processor

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_ADC08_80186Copro.html

**********************************************************************/


#include "emu.h"
#include "tube_80186.h"
#include "softlist_dev.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_TUBE_80186, bbc_tube_80186_device, "bbc_tube_80186", "Acorn 80186 Co-Processor")


//-------------------------------------------------
//  ADDRESS_MAP( tube_80186_mem )
//-------------------------------------------------

void bbc_tube_80186_device::tube_80186_mem(address_map &map)
{
	map.unmap_value_high();
}

//-------------------------------------------------
//  ADDRESS_MAP( tube_80186_io )
//-------------------------------------------------

void bbc_tube_80186_device::tube_80186_io(address_map &map)
{
	map(0x80, 0x8f).rw("ula", FUNC(tube_device::parasite_r), FUNC(tube_device::parasite_w)).umask16(0x00ff);
}

//-------------------------------------------------
//  ROM( tube_80186 )
//-------------------------------------------------

ROM_START( tube_80186 )
	ROM_REGION(0x4000, "bootstrap", 0)
	ROM_LOAD16_BYTE("m512_lo_ic31.rom", 0x0000, 0x2000, CRC(c0df8707) SHA1(7f6d843d5aea6bdb36cbd4623ae942b16b96069d)) // 2201,287-02
	ROM_LOAD16_BYTE("m512_hi_ic32.rom", 0x0001, 0x2000, CRC(e47f10b2) SHA1(45dc8d7e7936afbec6de423569d9005a1c350316)) // 2201,288-02
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_tube_80186_device::device_add_mconfig(machine_config &config)
{
	I80186(config, m_i80186, 20_MHz_XTAL / 2);
	m_i80186->set_addrmap(AS_PROGRAM, &bbc_tube_80186_device::tube_80186_mem);
	m_i80186->set_addrmap(AS_IO, &bbc_tube_80186_device::tube_80186_io);
	m_i80186->tmrout0_handler().set_inputline(m_i80186, INPUT_LINE_HALT).invert();
	m_i80186->tmrout1_handler().set_inputline(m_i80186, INPUT_LINE_NMI).invert();

	TUBE(config, m_ula, 0);
	m_ula->pirq_handler().set(m_i80186, FUNC(i80186_cpu_device::int0_w));
	m_ula->drq_handler().set(m_i80186, FUNC(i80186_cpu_device::drq0_w));

	/* internal ram */
	RAM(config, m_ram).set_default_size("512K");

	/* software lists */
	SOFTWARE_LIST(config, "flop_ls_80186").set_original("bbc_flop_80186");
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_tube_80186_device::device_rom_region() const
{
	return ROM_NAME( tube_80186 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_tube_80186_device - constructor
//-------------------------------------------------

bbc_tube_80186_device::bbc_tube_80186_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_TUBE_80186, tag, owner, clock),
		device_bbc_tube_interface(mconfig, *this),
		m_i80186(*this, "i80186"),
		m_ula(*this, "ula"),
		m_ram(*this, "ram"),
		m_bootstrap(*this, "bootstrap")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_tube_80186_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_tube_80186_device::device_reset()
{
	address_space &program = m_i80186->space(AS_PROGRAM);

	program.install_ram(0x00000, 0x3ffff, m_ram->pointer());
	program.install_ram(0x40000, 0x7ffff, m_ram->pointer() + 0x40000);
	program.install_ram(0x80000, 0xbffff, m_ram->pointer() + 0x40000);
	program.install_rom(0xc0000, 0xc3fff, 0x3c000, m_bootstrap->base());
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_tube_80186_device::host_r(offs_t offset)
{
	return m_ula->host_r(offset);
}

void bbc_tube_80186_device::host_w(offs_t offset, uint8_t data)
{
	m_ula->host_w(offset, data);
}
