// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Portfolio HPC-101 parallel interface emulation

**********************************************************************/

#include "emu.h"
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

DEFINE_DEVICE_TYPE(POFO_HPC101, pofo_hpc101_device, "pofo_hpc101", "Atari Portfolio HPC-101")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void pofo_hpc101_device::device_add_mconfig(machine_config &config)
{
	I8255A(config, m_ppi);
	m_ppi->out_pa_callback().set("cent_data_out", FUNC(output_latch_device::bus_w));
	m_ppi->out_pb_callback().set("cent_ctrl_out", FUNC(output_latch_device::bus_w));
	m_ppi->in_pc_callback().set("cent_status_in", FUNC(input_buffer_device::bus_r));

	centronics_device &centronics(CENTRONICS(config, CENTRONICS_TAG, centronics_devices, "printer"));
	centronics.ack_handler().set("cent_status_in", FUNC(input_buffer_device::write_bit5));
	centronics.busy_handler().set("cent_status_in", FUNC(input_buffer_device::write_bit4));
	centronics.fault_handler().set("cent_status_in", FUNC(input_buffer_device::write_bit3));
	centronics.select_handler().set("cent_status_in", FUNC(input_buffer_device::write_bit1));
	centronics.perror_handler().set("cent_status_in", FUNC(input_buffer_device::write_bit0));

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	centronics.set_output_latch(cent_data_out);

	INPUT_BUFFER(config, "cent_status_in");

	output_latch_device &cent_ctrl_out(OUTPUT_LATCH(config, "cent_ctrl_out"));
	cent_ctrl_out.bit_handler<0>().set(CENTRONICS_TAG, FUNC(centronics_device::write_strobe));
	cent_ctrl_out.bit_handler<1>().set(CENTRONICS_TAG, FUNC(centronics_device::write_autofd));
	cent_ctrl_out.bit_handler<2>().set(CENTRONICS_TAG, FUNC(centronics_device::write_init));
	cent_ctrl_out.bit_handler<3>().set(CENTRONICS_TAG, FUNC(centronics_device::write_select_in));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pofo_hpc101_device - constructor
//-------------------------------------------------

pofo_hpc101_device::pofo_hpc101_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, POFO_HPC101, tag, owner, clock),
	device_portfolio_expansion_slot_interface(mconfig, *this),
	m_ppi(*this, M82C55A_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pofo_hpc101_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pofo_hpc101_device::device_reset()
{
	m_ppi->reset();
}


//-------------------------------------------------
//  nrdi_r - read
//-------------------------------------------------

uint8_t pofo_hpc101_device::nrdi_r(address_space &space, offs_t offset, uint8_t data, bool iom, bool bcom, bool ncc1)
{
	if (!bcom)
	{
		if ((offset & 0x0f) == 0x0f)
		{
			data = 0x02;
		}

		if ((offset & 0x0c) == 0x08)
		{
			data = m_ppi->read(offset & 0x03);
		}
	}

	return data;
}


//-------------------------------------------------
//  nwri_w - write
//-------------------------------------------------

void pofo_hpc101_device::nwri_w(address_space &space, offs_t offset, uint8_t data, bool iom, bool bcom, bool ncc1)
{
	if (!bcom)
	{
		if ((offset & 0x0c) == 0x08)
		{
			m_ppi->write(offset & 0x03, data);
		}
	}
}
