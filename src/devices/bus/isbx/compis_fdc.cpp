// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    TeleNova Compis Floppy Disk Controller (119 106/1) emulation

**********************************************************************/

#include "emu.h"
#include "compis_fdc.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define I8272_TAG   "ic13"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(COMPIS_FDC, compis_fdc_device, "compis_fdc", "Compis FDC")


//-------------------------------------------------
//  floppy_format_type floppy_formats
//-------------------------------------------------

WRITE_LINE_MEMBER( compis_fdc_device::fdc_irq )
{
	m_slot->mintr1_w(state);
}

WRITE_LINE_MEMBER( compis_fdc_device::fdc_drq )
{
	m_slot->mdrqt_w(state);
}

FLOPPY_FORMATS_MEMBER( compis_fdc_device::floppy_formats )
	FLOPPY_CPIS_FORMAT
FLOPPY_FORMATS_END

static void compis_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
	device.option_add("525hd", FLOPPY_525_HD);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void compis_fdc_device::device_add_mconfig(machine_config &config)
{
	I8272A(config, m_fdc, 8'000'000, true);
	m_fdc->intrq_wr_callback().set(FUNC(compis_fdc_device::fdc_irq));
	m_fdc->drq_wr_callback().set(FUNC(compis_fdc_device::fdc_drq));
	FLOPPY_CONNECTOR(config, m_floppy0, compis_floppies, "525qd", compis_fdc_device::floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy1, compis_floppies, "525qd", compis_fdc_device::floppy_formats);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  compis_fdc_device - constructor
//-------------------------------------------------

compis_fdc_device::compis_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, COMPIS_FDC, tag, owner, clock),
	device_isbx_card_interface(mconfig, *this),
	m_fdc(*this, I8272_TAG),
	m_floppy0(*this, I8272_TAG":0"),
	m_floppy1(*this, I8272_TAG":1")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void compis_fdc_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void compis_fdc_device::device_reset()
{
	m_fdc->reset();
}


//-------------------------------------------------
//  mcs0_r - chip select 0 read
//-------------------------------------------------

uint8_t compis_fdc_device::mcs0_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (BIT(offset, 0))
	{
	case 0: data = m_fdc->msr_r(); break;
	case 1: data = m_fdc->fifo_r(); break;
	}

	return data;
}


//-------------------------------------------------
//  mcs0_w - chip select 0 write
//-------------------------------------------------

void compis_fdc_device::mcs0_w(offs_t offset, uint8_t data)
{
	switch (BIT(offset, 0))
	{
	case 1: m_fdc->fifo_w(data); break;
	}
}


//-------------------------------------------------
//  mdack_r - DMA acknowledge read
//-------------------------------------------------

uint8_t compis_fdc_device::mdack_r(offs_t offset)
{
	return m_fdc->dma_r();
}


//-------------------------------------------------
//  mdack_w - DMA acknowledge write
//-------------------------------------------------

void compis_fdc_device::mdack_w(offs_t offset, uint8_t data)
{
	m_fdc->dma_w(data);
}


//-------------------------------------------------
//  opt0_w - option 0 write
//-------------------------------------------------

void compis_fdc_device::opt0_w(int state)
{
	m_fdc->tc_w(state);
}


//-------------------------------------------------
//  opt1_w - option 1 write
//-------------------------------------------------

void compis_fdc_device::opt1_w(int state)
{
	m_floppy0->get_device()->mon_w(state);
	m_floppy1->get_device()->mon_w(state);
}
