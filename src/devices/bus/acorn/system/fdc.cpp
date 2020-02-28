// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Floppy Disc Controller Board

    Part No. 200,004

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_fdc.html

**********************************************************************/


#include "emu.h"
#include "fdc.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ACORN_FDC, acorn_fdc_device, "acorn_fdc", "Acorn Floppy Disc Controller Board")

//-------------------------------------------------
//  MACHINE_DRIVER( fdc )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER(acorn_fdc_device::floppy_formats )
	FLOPPY_ACORN_SSD_FORMAT
FLOPPY_FORMATS_END

static void acorn_floppies(device_slot_interface &device)
{
	device.option_add("525sssd", FLOPPY_525_SSSD);
	device.option_add("525sd", FLOPPY_525_SD);
	device.option_add("525qd", FLOPPY_525_QD);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void acorn_fdc_device::device_add_mconfig(machine_config &config)
{
	I8271(config, m_fdc, 4_MHz_XTAL / 2);
	m_fdc->intrq_wr_callback().set(FUNC(acorn_fdc_device::bus_nmi_w));
	m_fdc->hdl_wr_callback().set(FUNC(acorn_fdc_device::motor_w));
	m_fdc->opt_wr_callback().set(FUNC(acorn_fdc_device::side_w));
	FLOPPY_CONNECTOR(config, m_floppy[0], acorn_floppies, "525qd", acorn_fdc_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], acorn_floppies, "525qd", acorn_fdc_device::floppy_formats).enable_sound(true);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  acorn_fdc_device - constructor
//-------------------------------------------------

acorn_fdc_device::acorn_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ACORN_FDC, tag, owner, clock)
	, device_acorn_bus_interface(mconfig, *this)
	, m_fdc(*this, "i8271")
	, m_floppy(*this, "i8271:%u", 0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void acorn_fdc_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void acorn_fdc_device::device_reset()
{
	address_space &space = m_bus->memspace();

	space.install_device(0x0a00, 0x0a03, *m_fdc, &i8271_device::map);
	space.install_readwrite_handler(0x0a04, 0x0a04, 0, 0x1f8, 0, read8smo_delegate(*m_fdc, FUNC(i8271_device::data_r)), write8smo_delegate(*m_fdc, FUNC(i8271_device::data_w)));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

WRITE_LINE_MEMBER(acorn_fdc_device::motor_w)
{
	if (m_floppy[0]->get_device()) m_floppy[0]->get_device()->mon_w(!state);
	if (m_floppy[1]->get_device()) m_floppy[1]->get_device()->mon_w(!state);
	m_fdc->ready_w(!state);
}

WRITE_LINE_MEMBER(acorn_fdc_device::side_w)
{
	if (m_floppy[0]->get_device()) m_floppy[0]->get_device()->ss_w(state);
	if (m_floppy[1]->get_device()) m_floppy[1]->get_device()->ss_w(state);
}

WRITE_LINE_MEMBER(acorn_fdc_device::bus_nmi_w)
{
	m_bus->nmi_w(state);
}
