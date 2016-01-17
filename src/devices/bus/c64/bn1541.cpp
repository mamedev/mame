// license:BSD-3-Clause
// copyright-holders:Curt Coder, smf
/**********************************************************************

    SpeedDOS / Burst Nibbler 1541/1571 Parallel Cable emulation

    http://sta.c64.org/cbmparc2.html

**********************************************************************/

#include "bn1541.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_BN1541 = &device_creator<c64_bn1541_device>;



//**************************************************************************
//  FLOPPY DRIVE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_c64_floppy_parallel_interface - constructor
//-------------------------------------------------

device_c64_floppy_parallel_interface::device_c64_floppy_parallel_interface(const machine_config &mconfig, device_t &device) :
	m_other(nullptr), m_parallel_data(0)
{
}


//-------------------------------------------------
//  ~device_c64_floppy_parallel_interface - destructor
//-------------------------------------------------

device_c64_floppy_parallel_interface::~device_c64_floppy_parallel_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_bn1541_device - constructor
//-------------------------------------------------

c64_bn1541_device::c64_bn1541_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_BN1541, "C64 Burst Nibbler 1541/1571 Parallel Cable", tag, owner, clock, "c64_bn1541", __FILE__),
	device_pet_user_port_interface(mconfig, *this),
	device_c64_floppy_parallel_interface(mconfig, *this), m_parallel_output(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_bn1541_device::device_start()
{
	device_iterator iter(machine().root_device());

	for (device_t *device = iter.first(); device != nullptr; device = iter.next())
	{
		device_iterator subiter(*device);

		for (device_t *subdevice = subiter.first(); subdevice != nullptr; subdevice = iter.next())
		{
			if (subdevice->interface(m_other) && subdevice != this)
			{
				if (LOG) logerror("Parallel device %s\n", subdevice->tag().c_str());

				// grab the first 1541/1571 and run to the hills
				m_other->m_other = this;
				return;
			}
		}
	}
}


//-------------------------------------------------
//  parallel_data_w -
//-------------------------------------------------

void c64_bn1541_device::parallel_data_w(UINT8 data)
{
	if (LOG) logerror("1541 parallel data %02x\n", data);

	output_c((data>>0)&1);
	output_d((data>>1)&1);
	output_e((data>>2)&1);
	output_f((data>>3)&1);
	output_h((data>>4)&1);
	output_j((data>>5)&1);
	output_k((data>>6)&1);
	output_l((data>>7)&1);
}


//-------------------------------------------------
//  parallel_strobe_w -
//-------------------------------------------------

void c64_bn1541_device::parallel_strobe_w(int state)
{
	if (LOG) logerror("1541 parallel strobe %u\n", state);

	output_b(state);
}


//-------------------------------------------------
//  update_output
//-------------------------------------------------

void c64_bn1541_device::update_output()
{
	if (m_other != nullptr)
	{
		m_other->parallel_data_w(m_parallel_output);
	}
}


//-------------------------------------------------
//  input_8 - CIA2 PC write
//-------------------------------------------------

WRITE_LINE_MEMBER(c64_bn1541_device::input_8)
{
	if (LOG) logerror("C64 parallel strobe %u\n", state);

	if (m_other != nullptr)
	{
		m_other->parallel_strobe_w(state);
	}
}
