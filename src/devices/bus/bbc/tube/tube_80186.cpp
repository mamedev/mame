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

MACHINE_CONFIG_START(bbc_tube_80186_device::device_add_mconfig)
	MCFG_DEVICE_ADD("i80186", I80186, XTAL(20'000'000) / 2)
	MCFG_DEVICE_PROGRAM_MAP(tube_80186_mem)
	MCFG_DEVICE_IO_MAP(tube_80186_io)
	//MCFG_80186_CHIP_SELECT_CB(WRITE16(*this, bbc_tube_80186_device, chip_select_cb))
	MCFG_80186_TMROUT0_HANDLER(INPUTLINE("i80186", INPUT_LINE_HALT)) MCFG_DEVCB_INVERT
	MCFG_80186_TMROUT1_HANDLER(INPUTLINE("i80186", INPUT_LINE_NMI)) MCFG_DEVCB_INVERT

	MCFG_TUBE_ADD("ula")
	MCFG_TUBE_PIRQ_HANDLER(WRITELINE("i80186", i80186_cpu_device, int0_w))
	MCFG_TUBE_DRQ_HANDLER(WRITELINE("i80186", i80186_cpu_device, drq0_w))

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("512K")

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("flop_ls_80186", "bbc_flop_80186")
MACHINE_CONFIG_END

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
	m_slot = dynamic_cast<bbc_tube_slot_device *>(owner());
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_tube_80186_device::device_reset()
{
	m_ula->reset();

	address_space &program = m_i80186->space(AS_PROGRAM);

	program.install_ram(0x00000, 0x3ffff, m_ram->pointer());
	program.install_ram(0x40000, 0x7ffff, m_ram->pointer() + 0x40000);
	program.install_ram(0x80000, 0xbffff, m_ram->pointer() + 0x40000);
	program.install_rom(0xc0000, 0xc3fff, 0x3c000, m_bootstrap->base());
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER(bbc_tube_80186_device::host_r)
{
	return m_ula->host_r(space, offset);
}

WRITE8_MEMBER(bbc_tube_80186_device::host_w)
{
	m_ula->host_w(space, offset, data);
}
