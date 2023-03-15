// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    IQ151 STAPER (STAndard PERipheral) module emulation

    STAPER module includes cables for connect:
    - a printer (CONSUL 2112 or 2113)
    - a paper tape puncher (DT-105S)
    - a paper tape reader (FS-1503)

    Currently only the printer is emulated

***************************************************************************/

#include "emu.h"
#include "staper.h"


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(IQ151_STAPER, iq151_staper_device, "iq151_staper", "IQ151 STAPER")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  iq151_staper_device - constructor
//-------------------------------------------------

iq151_staper_device::iq151_staper_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, IQ151_STAPER, tag, owner, clock)
	, device_iq151cart_interface(mconfig, *this)
	, m_ppi(*this, "ppi8255")
	, m_printer(*this, "printer")
	, m_printer_timer(nullptr)
	, m_ppi_portc(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void iq151_staper_device::device_start()
{
	m_printer_timer = timer_alloc(FUNC(iq151_staper_device::pc2_low_tick), this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void iq151_staper_device::device_reset()
{
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void iq151_staper_device::device_add_mconfig(machine_config &config)
{
	I8255A(config, m_ppi);
	m_ppi->in_pa_callback().set(FUNC(iq151_staper_device::ppi_porta_r));
	m_ppi->out_pb_callback().set(FUNC(iq151_staper_device::ppi_portb_w));
	m_ppi->out_pc_callback().set(FUNC(iq151_staper_device::ppi_portc_w));

	PRINTER(config, "printer", 0);
}

//-------------------------------------------------
//  pc2_low_tick - lower PPI Port C bit 2
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(iq151_staper_device::pc2_low_tick)
{
	m_ppi->pc2_w(0);
}


//-------------------------------------------------
//  IO read
//-------------------------------------------------

void iq151_staper_device::io_read(offs_t offset, uint8_t &data)
{
	if (offset >= 0xf8 && offset < 0xfc)
		data = m_ppi->read(offset & 0x03);
}

//-------------------------------------------------
//  IO write
//-------------------------------------------------

void iq151_staper_device::io_write(offs_t offset, uint8_t data)
{
	if (offset >= 0xf8 && offset < 0xfc)
		m_ppi->write(offset & 0x03, data);
}


//**************************************************************************
//  I8255  interface
//**************************************************************************

uint8_t iq151_staper_device::ppi_porta_r()
{
	// TODO: paper tape reader input
	return 0;
}

void iq151_staper_device::ppi_portb_w(uint8_t data)
{
	if (m_ppi_portc & 0x80)
	{
		// printer out
		m_printer->output(data);

		// CONSUL 2112/3 usually print 65/70 cps
		m_printer_timer->adjust(attotime::from_msec(15));
	}
	if (m_ppi_portc & 0x40)
	{
		// TODO: paper tape puncher out
	}
}

void iq151_staper_device::ppi_portc_w(uint8_t data)
{
	/*
	    x--- ----   printer select
	    -x-- ----   punchtape select
	*/

	m_ppi_portc = data;
}
