// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    ISBX 218a with ISBC configuration

**********************************************************************/

#include "isbc_218a.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define I8272_TAG   "u14"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ISBC_218A = &device_creator<isbc_218a_device>;


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

static SLOT_INTERFACE_START( isbc_218a_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END


//-------------------------------------------------
//  MACHINE_DRIVER( isbc_218a )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( isbc_218a )
	MCFG_I8272A_ADD(I8272_TAG, true)
	MCFG_UPD765_INTRQ_CALLBACK(WRITELINE(isbc_218a_device, fdc_irq))
	MCFG_UPD765_DRQ_CALLBACK(WRITELINE(isbc_218a_device, fdc_drq))
	MCFG_FLOPPY_DRIVE_ADD(I8272_TAG":0", isbc_218a_floppies, "525dd", isbc_218a_device::floppy_formats)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor isbc_218a_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( isbc_218a );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isbc_218a_device - constructor
//-------------------------------------------------

isbc_218a_device::isbc_218a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, ISBC_218A, "ISBX 218a for ISBC", tag, owner, clock, "isbc_218a", __FILE__),
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
}


//-------------------------------------------------
//  mcs0_r - chip select 0 read
//-------------------------------------------------

UINT8 isbc_218a_device::mcs0_r(address_space &space, offs_t offset)
{
	UINT8 data = 0xff;

	switch (BIT(offset, 0))
	{
	case 0: data = m_fdc->msr_r(space, 0); break;
	case 1: data = m_fdc->fifo_r(space, 0); break;
	}

	return data;
}


//-------------------------------------------------
//  mcs0_w - chip select 0 write
//-------------------------------------------------

void isbc_218a_device::mcs0_w(address_space &space, offs_t offset, UINT8 data)
{
	switch (BIT(offset, 0))
	{
	case 1: m_fdc->fifo_w(space, 0, data); break;
	}
}


//-------------------------------------------------
//  mcs1_r - chip select 1 read
//-------------------------------------------------

UINT8 isbc_218a_device::mcs1_r(address_space &space, offs_t offset)
{
	UINT8 data = 0xff;

	switch (offset)
	{
	case 4: data = m_motor; break;
	}

	return data;
}


//-------------------------------------------------
//  mcs1_w - chip select 1 write
//-------------------------------------------------

void isbc_218a_device::mcs1_w(address_space &space, offs_t offset, UINT8 data)
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
		m_floppy0->get_device()->mon_w(!(data & 1));
		break;
	case 6: m_fdc->tc_w(data & 1); break;
	}
}


//-------------------------------------------------
//  mdack_r - DMA acknowledge read
//-------------------------------------------------

UINT8 isbc_218a_device::mdack_r(address_space &space, offs_t offset)
{
	return m_fdc->dma_r();
}


//-------------------------------------------------
//  mdack_w - DMA acknowledge write
//-------------------------------------------------

void isbc_218a_device::mdack_w(address_space &space, offs_t offset, UINT8 data)
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
