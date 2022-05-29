// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Memotech Compact Flash System

**********************************************************************/


#include "emu.h"
#include "cfx.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MTX_CFX, mtx_cfx_device, "mtx_cfx", "MTX CFX System")


//-------------------------------------------------
//  ROM( cfx )
//-------------------------------------------------

ROM_START( cfx )
	ROM_REGION(0x4000, "flash", ROMREGION_ERASE00) // SST39SF010
	ROM_SYSTEM_BIOS(0, "153", "CFX Duo 15 (Build 153, 02 April 2018)")
	ROMX_LOAD("cfx015-b153,280.bin", 0x0000, 0x3afc, CRC(77b31543) SHA1(3973016bbbd36ac958915e187f26816ec431ef87), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "151", "CFX Duo 15 (Build 151, 03 December 2017)")
	ROMX_LOAD("cfx015-b151.bin", 0x0000, 0x3afc, CRC(4c7366c7) SHA1(33a7f503a84602aaf93ba61f55a3e4d5dbb223ce), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "147", "CFX Duo 14 (Build 147, 28 December 2016)")
	ROMX_LOAD("cfx014-b147,512.bin", 0x0000, 0x3afc, CRC(ef9d0e1f) SHA1(b6e5ba79f804f509f0d028f00d5972e07e2cb9cf), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "144", "CFX Duo 14 (Build 144, 17 October 2015)")
	ROMX_LOAD("cfx0014,512.bin", 0x0000, 0x3a3f, CRC(23ab4f0e) SHA1(e01874ba5392518be2313fb3161bcef6a72c9662), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "132", "CFX Duo 13 (Build 132, 11 October 2015)")
	ROMX_LOAD("cfx013,512.bin", 0x0000, 0x3c31, CRC(2d1c4ebc) SHA1(5f894bdc9fc20d37d11a2771b8e8fbcacd3f938e), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(5, "103", "CFX Duo 12 (Build 103, 21 August 2015)")
	ROMX_LOAD("cfx_duo12.bin", 0x0000, 0x4000, CRC(4fbe5ab7) SHA1(3e8dcc3d88158e0dc30d7d9ecec7e427883f7171), ROM_BIOS(5))
ROM_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void mtx_cfx_device::device_add_mconfig(machine_config &config)
{
	I8255(config, m_pia);
	m_pia->in_pa_callback().set([this]() { return m_ide_data & 0xff; });
	m_pia->out_pa_callback().set([this](uint8_t data) { m_ide_data = (m_ide_data & 0xff00) | data; });
	m_pia->in_pb_callback().set([this]() { return m_ide_data >> 8; });
	m_pia->out_pb_callback().set([this](uint8_t data) { m_ide_data = (m_ide_data & 0x00ff) | (data << 8); });
	m_pia->out_pc_callback().set(FUNC(mtx_cfx_device::portc_w));

	ATA_INTERFACE(config, m_ide).options(ata_devices, "hdd", nullptr, false);
}


const tiny_rom_entry *mtx_cfx_device::device_rom_region() const
{
	return ROM_NAME( cfx );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mtx_cfx_device - constructor
//-------------------------------------------------

mtx_cfx_device::mtx_cfx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MTX_CFX, tag, owner, clock)
	, device_mtx_exp_interface(mconfig, *this)
	, m_flash(*this, "flash")
	, m_pia(*this, "pia")
	, m_ide(*this, "ide")
	, m_ide_data(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mtx_cfx_device::device_start()
{
	save_item(NAME(m_ide_data));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mtx_cfx_device::device_reset()
{
	machine().root_device().membank("rommap_bank1")->configure_entries(4, 2, m_flash->base(), 0x2000);

	io_space().install_readwrite_handler(0x6c, 0x6f, read8sm_delegate(*m_pia, FUNC(i8255_device::read)), write8sm_delegate(*m_pia, FUNC(i8255_device::write)));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void mtx_cfx_device::portc_w(uint8_t data)
{
	/*
	    b0 A0
	    b1 A1
	    b2 A2
	    b3 CS0
	    b4 CS1
	    b5 WRITE
	    b6 READ
	    b7 RESET
	*/

	if (BIT(data, 7))
		m_ide->reset();

	switch (BIT(data, 3, 2))
	{
	case 0x01: // CS0
		switch (BIT(data, 5, 2))
		{
		case 0x01: // WRITE
			m_ide->cs0_w(data & 0x07, m_ide_data);
			break;
		case 0x02: // READ
			m_ide_data = m_ide->cs0_r(data & 0x07);
			break;
		}
		break;

	case 0x02: // CS1
		switch (BIT(data, 5, 2))
		{
		case 0x01: // WRITE
			m_ide->cs1_w(data & 0x07, m_ide_data);
			break;
		case 0x02: // READ
			m_ide_data = m_ide->cs1_r(data & 0x07);
			break;
		}
		break;
	}
}
