// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    AMS 3" Microdrive Disc System

**********************************************************************/

#include "emu.h"
#include "ams.h"

#include "formats/acorn_dsk.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_AMS3, bbc_ams3_device, "bbc_ams3", "AMS 3\" Microdrive Disc System")


//-------------------------------------------------
//  FLOPPY_FORMATS( ams3 )
//-------------------------------------------------

void bbc_ams3_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_ACORN_SSD_FORMAT);
}


//-------------------------------------------------
//  ROM( ams3 )
//-------------------------------------------------

ROM_START( ams3 )
	ROM_REGION(0x8000, "dfs_rom", 0)
	ROM_LOAD("dfs098.rom",  0x0000, 0x2000, CRC(90852e7d) SHA1(6df3552d5426f3a4625b9a0c7829bdba03f05e84))
	ROM_RELOAD(             0x2000, 0x2000)
	ROM_LOAD("amsdisc.rom", 0x4000, 0x1000, CRC(93876595) SHA1(a9d9123667e65334b28b8588b09642478653e356))
	ROM_RELOAD(             0x5000, 0x1000)
	ROM_RELOAD(             0x6000, 0x1000)
	ROM_RELOAD(             0x7000, 0x1000)
ROM_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_ams3_device::device_add_mconfig(machine_config &config)
{
	I8271(config, m_fdc, DERIVED_CLOCK(1, 2));
	m_fdc->intrq_wr_callback().set(DEVICE_SELF_OWNER, FUNC(bbc_fdc_slot_device::intrq_w));
	m_fdc->hdl_wr_callback().set(FUNC(bbc_ams3_device::motor_w));
	m_fdc->opt_wr_callback().set(FUNC(bbc_ams3_device::side_w));
	// Hitachi HFD 305S
	FLOPPY_CONNECTOR(config, m_floppy[0], "3dssd", FLOPPY_3_DSSD, true, bbc_ams3_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], "3dssd", FLOPPY_3_DSSD, false, bbc_ams3_device::floppy_formats).enable_sound(true);
}


const tiny_rom_entry *bbc_ams3_device::device_rom_region() const
{
	return ROM_NAME( ams3 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_ams3_device - constructor
//-------------------------------------------------

bbc_ams3_device::bbc_ams3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_AMS3, tag, owner, clock)
	, device_bbc_fdc_interface(mconfig, *this)
	, m_fdc(*this, "i8271")
	, m_floppy(*this, "i8271:%u", 0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_ams3_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_ams3_device::read(offs_t offset)
{
	uint8_t data;

	if (offset & 0x04)
	{
		data = m_fdc->data_r();
	}
	else
	{
		data = m_fdc->read(offset & 0x03);
	}
	return data;
}

void bbc_ams3_device::write(offs_t offset, uint8_t data)
{
	if (offset & 0x04)
	{
		m_fdc->data_w(data);
	}
	else
	{
		m_fdc->write(offset & 0x03, data);
	}
}

void bbc_ams3_device::motor_w(int state)
{
	if (m_floppy[0]->get_device()) m_floppy[0]->get_device()->mon_w(!state);
	if (m_floppy[1]->get_device()) m_floppy[1]->get_device()->mon_w(!state);
	m_fdc->ready_w(!state);
}

void bbc_ams3_device::side_w(int state)
{
	if (m_floppy[0]->get_device()) m_floppy[0]->get_device()->ss_w(state);
	if (m_floppy[1]->get_device()) m_floppy[1]->get_device()->ss_w(state);
}
