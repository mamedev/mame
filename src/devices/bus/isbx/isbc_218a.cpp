// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    ISBX 218a with ISBC configuration

**********************************************************************/

#include "emu.h"
#include "isbc_218a.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define I8272_TAG   "u14"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ISBC_218A, isbc_218a_device, "isbc_218a", "ISBX 218a for ISBC")


//-------------------------------------------------
//  floppy_format_type floppy_formats
//-------------------------------------------------

WRITE_LINE_MEMBER( isbc_218a_device::fdc_irq )
{
	m_slot->mintr1_w(state);
}

WRITE_LINE_MEMBER( isbc_218a_device::fdc_drq )
{
	m_slot->mdrqt_w(state);
}

FLOPPY_FORMATS_MEMBER( isbc_218a_device::floppy_formats )
	FLOPPY_PC_FORMAT
FLOPPY_FORMATS_END

static void isbc_218a_floppies(device_slot_interface &device)
{
	device.option_add("8dd", FLOPPY_8_DSDD);
	device.option_add("525dd", FLOPPY_525_DD);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isbc_218a_device::device_add_mconfig(machine_config &config)
{
	I8272A(config, m_fdc, 8_MHz_XTAL, true);
	m_fdc->intrq_wr_callback().set(FUNC(isbc_218a_device::fdc_irq));
	m_fdc->drq_wr_callback().set(FUNC(isbc_218a_device::fdc_drq));
	FLOPPY_CONNECTOR(config, m_floppy0, isbc_218a_floppies, "525dd", isbc_218a_device::floppy_formats);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isbc_218a_device - constructor
//-------------------------------------------------

isbc_218a_device::isbc_218a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISBC_218A, tag, owner, clock),
	device_isbx_card_interface(mconfig, *this),
	m_fdc(*this, I8272_TAG),
	m_floppy0(*this, I8272_TAG":0"), m_reset(false), m_motor(false)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isbc_218a_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isbc_218a_device::device_reset()
{
	m_reset = false;
	m_motor = false;
	// set from jumper all drives must be same type
	m_fd8 = m_floppy0->get_device()->get_form_factor() == floppy_image::FF_8;
	if(m_fd8)
	{
		m_floppy0->get_device()->mon_w(0);
		m_fdc->set_rate(500000);
	}
}


//-------------------------------------------------
//  mcs0_r - chip select 0 read
//-------------------------------------------------

uint8_t isbc_218a_device::mcs0_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (BIT(offset, 0))
	{
	case 0: data = m_fdc->msr_r(machine().dummy_space(), 0); break;
	case 1: data = m_fdc->fifo_r(machine().dummy_space(), 0); break;
	}

	return data;
}


//-------------------------------------------------
//  mcs0_w - chip select 0 write
//-------------------------------------------------

void isbc_218a_device::mcs0_w(offs_t offset, uint8_t data)
{
	switch (BIT(offset, 0))
	{
	case 1: m_fdc->fifo_w(machine().dummy_space(), 0, data); break;
	}
}


//-------------------------------------------------
//  mcs1_r - chip select 1 read
//-------------------------------------------------

uint8_t isbc_218a_device::mcs1_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset)
	{
	case 4: data = m_motor; break;
	}

	return data;
}


//-------------------------------------------------
//  mcs1_w - chip select 1 write
//-------------------------------------------------

void isbc_218a_device::mcs1_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 2:
		if((data & 1) && m_reset)
			m_fdc->soft_reset();
		m_reset = (data & 1) ? false : true;
		break;
	case 4:
		m_motor = data & 1;
		if(!m_fd8)
			m_floppy0->get_device()->mon_w(!(data & 1));
		break;
	case 6: m_fdc->tc_w(data & 1); break;
	}
}


//-------------------------------------------------
//  mdack_r - DMA acknowledge read
//-------------------------------------------------

uint8_t isbc_218a_device::mdack_r(offs_t offset)
{
	return m_fdc->dma_r();
}


//-------------------------------------------------
//  mdack_w - DMA acknowledge write
//-------------------------------------------------

void isbc_218a_device::mdack_w(offs_t offset, uint8_t data)
{
	m_fdc->dma_w(data);
}


//-------------------------------------------------
//  opt0_w - option 0 write
//-------------------------------------------------

void isbc_218a_device::opt0_w(int state)
{
	m_fdc->tc_w(state);
}
