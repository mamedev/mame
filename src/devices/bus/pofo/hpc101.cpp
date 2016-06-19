// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Portfolio HPC-101 parallel interface emulation

**********************************************************************/

#include "hpc101.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0

#define M82C55A_TAG     "u1"
#define CENTRONICS_TAG  "centronics"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type HPC101 = &device_creator<hpc101_t>;


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( hpc101 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( hpc101 )
	MCFG_DEVICE_ADD(M82C55A_TAG, I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(DEVWRITE8("cent_data_out", output_latch_device, write))
	MCFG_I8255_OUT_PORTB_CB(DEVWRITE8("cent_ctrl_out", output_latch_device, write))
	MCFG_I8255_IN_PORTC_CB(DEVREAD8("cent_status_in", input_buffer_device, read))

	MCFG_CENTRONICS_ADD(CENTRONICS_TAG, centronics_devices, "printer")
	MCFG_CENTRONICS_ACK_HANDLER(DEVWRITELINE("cent_status_in", input_buffer_device, write_bit5))
	MCFG_CENTRONICS_BUSY_HANDLER(DEVWRITELINE("cent_status_in", input_buffer_device, write_bit4))
	MCFG_CENTRONICS_FAULT_HANDLER(DEVWRITELINE("cent_status_in", input_buffer_device, write_bit3))
	MCFG_CENTRONICS_SELECT_HANDLER(DEVWRITELINE("cent_status_in", input_buffer_device, write_bit1))
	MCFG_CENTRONICS_PERROR_HANDLER(DEVWRITELINE("cent_status_in", input_buffer_device, write_bit0))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", CENTRONICS_TAG)
	MCFG_DEVICE_ADD("cent_status_in", INPUT_BUFFER, 0)

	MCFG_DEVICE_ADD("cent_ctrl_out", OUTPUT_LATCH, 0)
	MCFG_OUTPUT_LATCH_BIT0_HANDLER(DEVWRITELINE(CENTRONICS_TAG, centronics_device, write_strobe))
	MCFG_OUTPUT_LATCH_BIT1_HANDLER(DEVWRITELINE(CENTRONICS_TAG, centronics_device, write_autofd))
	MCFG_OUTPUT_LATCH_BIT2_HANDLER(DEVWRITELINE(CENTRONICS_TAG, centronics_device, write_init))
	MCFG_OUTPUT_LATCH_BIT3_HANDLER(DEVWRITELINE(CENTRONICS_TAG, centronics_device, write_select_in))
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor hpc101_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( hpc101 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  hpc101_t - constructor
//-------------------------------------------------

hpc101_t::hpc101_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, HPC101, "Atari Portfolio HPC-101", tag, owner, clock, "hpc101", __FILE__),
	device_portfolio_expansion_slot_interface(mconfig, *this),
	m_ppi(*this, M82C55A_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void hpc101_t::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void hpc101_t::device_reset()
{
	m_ppi->reset();
}


//-------------------------------------------------
//  nrdi_r - read
//-------------------------------------------------

UINT8 hpc101_t::nrdi_r(address_space &space, offs_t offset, UINT8 data, bool iom, bool bcom, bool ncc1)
{
	if (!bcom)
	{
		if ((offset & 0x0f) == 0x0f)
		{
			data = 0x02;
		}

		if ((offset & 0x0c) == 0x08)
		{
			data = m_ppi->read(space, offset & 0x03);
		}
	}

	return data;
}


//-------------------------------------------------
//  nwri_w - write
//-------------------------------------------------

void hpc101_t::nwri_w(address_space &space, offs_t offset, UINT8 data, bool iom, bool bcom, bool ncc1)
{
	if (!bcom)
	{
		if ((offset & 0x0c) == 0x08)
		{
			m_ppi->write(space, offset & 0x03, data);
		}
	}
}
