// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Torch Z80 Communicator (ZEP100)

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Torch_Z802ndproc.html

**********************************************************************/


#include "emu.h"
#include "tube_zep100.h"
#include "softlist_dev.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_TUBE_ZEP100, bbc_tube_zep100_device, "bbc_tube_zep100", "Torch Z80 Communicator")


//-------------------------------------------------
//  ADDRESS_MAP( tube_zep100_mem )
//-------------------------------------------------

void bbc_tube_zep100_device::tube_zep100_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(this, FUNC(bbc_tube_zep100_device::mem_r), FUNC(bbc_tube_zep100_device::mem_w));
}

//-------------------------------------------------
//  ADDRESS_MAP( tube_zep100_io )
//-------------------------------------------------

void bbc_tube_zep100_device::tube_zep100_io(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x07).mirror(0xff00).rw(this, FUNC(bbc_tube_zep100_device::io_r), FUNC(bbc_tube_zep100_device::io_w));
}

//-------------------------------------------------
//  ROM( tube_zep100 )
//-------------------------------------------------

ROM_START( tube_zep100 )
	ROM_REGION(0x2000, "rom", 0)
	ROM_DEFAULT_BIOS("cccp102")
	ROM_SYSTEM_BIOS(0, "cccp102", "CCCP 1.02")
	ROMX_LOAD("cccp102.rom", 0x0000, 0x2000, CRC(2eb40a21) SHA1(e6ee738e5f2f8556002b79d18caa8ef21f14e08d), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "cccp094", "CCCP 0.94")
	ROMX_LOAD("cccp094.rom", 0x0000, 0x2000, CRC(49989bd4) SHA1(62b57c858a3baa4ff943c31f77d331c414772a61), ROM_BIOS(2))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(bbc_tube_zep100_device::device_add_mconfig)
	MCFG_DEVICE_ADD("z80", Z80, XTAL(4'000'000))
	MCFG_DEVICE_PROGRAM_MAP(tube_zep100_mem)
	MCFG_DEVICE_IO_MAP(tube_zep100_io)

	MCFG_DEVICE_ADD("via", VIA6522, XTAL(4'000'000) / 2)
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(*this, bbc_tube_zep100_device, via_pb_w))
	MCFG_VIA6522_CB2_HANDLER(WRITELINE("ppi", i8255_device, pc2_w))
	MCFG_VIA6522_CA2_HANDLER(WRITELINE("ppi", i8255_device, pc6_w))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(DEVICE_SELF_OWNER, bbc_tube_slot_device, irq_w))

	MCFG_DEVICE_ADD("ppi", I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8("via", via6522_device, write_pa))
	MCFG_I8255_IN_PORTB_CB(READ8(*this, bbc_tube_zep100_device, ppi_pb_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(*this, bbc_tube_zep100_device, ppi_pc_w))

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
	MCFG_RAM_DEFAULT_VALUE(0x00)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("flop_ls_torch", "bbc_flop_torch")
MACHINE_CONFIG_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_tube_zep100_device::device_rom_region() const
{
	return ROM_NAME( tube_zep100 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_tube_zep100_device - constructor
//-------------------------------------------------

bbc_tube_zep100_device::bbc_tube_zep100_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_TUBE_ZEP100, tag, owner, clock),
		device_bbc_tube_interface(mconfig, *this),
		m_z80(*this, "z80"),
		m_via(*this, "via"),
		m_ppi(*this, "ppi"),
		m_ram(*this, "ram"),
		m_rom(*this, "rom"),
		m_port_b(0),
		m_rom_enabled(true)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_tube_zep100_device::device_start()
{
	m_slot = dynamic_cast<bbc_tube_slot_device *>(owner());
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_tube_zep100_device::device_reset()
{
	m_via->reset();

	m_rom_enabled = true;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER(bbc_tube_zep100_device::host_r)
{
	return m_via->read(space, offset & 0x0f);
}

WRITE8_MEMBER(bbc_tube_zep100_device::host_w)
{
	if (offset & 0x10)
		m_z80->reset();

	m_via->write(space, offset & 0x0f, data);
}


READ8_MEMBER(bbc_tube_zep100_device::mem_r)
{
	uint8_t data;

	if (m_rom_enabled)
		data = m_rom->base()[offset];
	else
		data = m_ram->pointer()[offset];

	return data;
}

WRITE8_MEMBER(bbc_tube_zep100_device::mem_w)
{
	m_ram->pointer()[offset] = data;
}


READ8_MEMBER(bbc_tube_zep100_device::io_r)
{
	uint8_t data = 0xff;

	if (!machine().side_effects_disabled())
		m_rom_enabled = !BIT(offset, 2);

	data = m_ppi->read(space, offset & 0x03);

	return data;
}

WRITE8_MEMBER(bbc_tube_zep100_device::io_w)
{
	m_ppi->write(space, offset & 0x03, data);
}


WRITE8_MEMBER(bbc_tube_zep100_device::via_pb_w)
{
	m_port_b = data;
}

READ8_MEMBER(bbc_tube_zep100_device::ppi_pb_r)
{
	return m_port_b;
}

WRITE8_MEMBER(bbc_tube_zep100_device::ppi_pc_w)
{
	m_via->write_ca1(BIT(data, 7));
	m_via->write_cb1(BIT(data, 1));
}
