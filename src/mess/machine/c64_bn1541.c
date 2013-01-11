/**********************************************************************

    SpeedDOS / Burst Nibbler 1541/1571 Parallel Cable emulation

    http://sta.c64.org/cbmparc2.html

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "c64_bn1541.h"



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
	m_other(NULL)
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

c64_bn1541_device::c64_bn1541_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_BN1541, "C64 Burst Nibbler 1541/1571 Parallel Cable", tag, owner, clock),
	device_c64_user_port_interface(mconfig, *this),
	device_c64_floppy_parallel_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_bn1541_device::device_start()
{
	device_iterator iter(machine().root_device());

	for (device_t *device = iter.first(); device != NULL; device = iter.next())
	{
		device_iterator subiter(*device);

		for (device_t *subdevice = subiter.first(); subdevice != NULL; subdevice = iter.next())
		{
			if (subdevice->interface(m_other) && subdevice != this)
			{
				if (LOG) logerror("Parallel device %s\n", subdevice->tag());

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

	m_parallel_data = data;
}


//-------------------------------------------------
//  parallel_strobe_w -
//-------------------------------------------------

void c64_bn1541_device::parallel_strobe_w(int state)
{
	if (LOG) logerror("1541 parallel strobe %u\n", state);

	m_slot->flag2_w(state);
}


//-------------------------------------------------
//  c64_pb_r - port B read
//-------------------------------------------------

UINT8 c64_bn1541_device::c64_pb_r(address_space &space, offs_t offset)
{
	return m_parallel_data;
}


//-------------------------------------------------
//  c64_pb_w - port B write
//-------------------------------------------------

void c64_bn1541_device::c64_pb_w(address_space &space, offs_t offset, UINT8 data)
{
	if (LOG) logerror("C64 parallel data %02x\n", data);

	if (m_other != NULL)
	{
		m_other->parallel_data_w(data);
	}
}


//-------------------------------------------------
//  c64_pc2_w - CIA2 PC write
//-------------------------------------------------

void c64_bn1541_device::c64_pc2_w(int state)
{
	if (LOG) logerror("C64 parallel strobe %u\n", state);

	if (m_other != NULL)
	{
		m_other->parallel_strobe_w(state);
	}
}
