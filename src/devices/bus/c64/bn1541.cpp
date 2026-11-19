// license:BSD-3-Clause
// copyright-holders:Curt Coder, smf
/**********************************************************************

    SpeedDOS / Burst Nibbler 1541/1571 Parallel Cable emulation

    http://sta.c64.org/cbmparc2.html

**********************************************************************/

#include "emu.h"
#include "bn1541.h"

#include <algorithm>



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_BN1541, c64_bn1541_device, "c64_bn1541", "C64 Burst Nibbler 1541/1571 Parallel Cable")



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


//-------------------------------------------------
//  parallel_select_w -
//-------------------------------------------------

void device_c64_floppy_parallel_interface::parallel_select_w(int state)
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_bn1541_device - constructor
//-------------------------------------------------

c64_bn1541_device::c64_bn1541_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_BN1541, tag, owner, clock),
	device_pet_user_port_interface(mconfig, *this),
	m_drive{ { mconfig, *this, 0 }, { mconfig, *this, 1 } },
	m_parallel_output(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_bn1541_device::device_start()
{
	std::fill(std::begin(m_drive_data), std::end(m_drive_data), 0xff);
	std::fill(std::begin(m_drive_strobe), std::end(m_drive_strobe), 1);

	int drive = 0;

	for (device_t &device : device_enumerator(machine().root_device()))
	{
		if (drive < 2 && device.interface(m_drive[drive].m_other))
		{
			if (LOG) logerror("Parallel device %s\n", device.tag());

			m_drive[drive].m_other->m_other = &m_drive[drive];
			drive++;
		}
	}

	save_item(NAME(m_parallel_output));
	save_item(NAME(m_drive_data));
	save_item(NAME(m_drive_strobe));
}


//-------------------------------------------------
//  drive_port::parallel_data_w -
//-------------------------------------------------

void c64_bn1541_device::drive_port::parallel_data_w(uint8_t data)
{
	m_host.drive_data_w(m_index, data);
}


//-------------------------------------------------
//  drive_port::parallel_strobe_w -
//-------------------------------------------------

void c64_bn1541_device::drive_port::parallel_strobe_w(int state)
{
	m_host.drive_strobe_w(m_index, state);
}


//-------------------------------------------------
//  drive_data_w -
//-------------------------------------------------

void c64_bn1541_device::drive_data_w(int drive, uint8_t data)
{
	if (LOG) logerror("Drive %d parallel data %02x\n", drive, data);

	m_drive_data[drive] = data;

	uint8_t const bus = m_drive_data[0] & m_drive_data[1];

	output_c(BIT(bus, 0));
	output_d(BIT(bus, 1));
	output_e(BIT(bus, 2));
	output_f(BIT(bus, 3));
	output_h(BIT(bus, 4));
	output_j(BIT(bus, 5));
	output_k(BIT(bus, 6));
	output_l(BIT(bus, 7));

	update_output();
}


//-------------------------------------------------
//  drive_strobe_w -
//-------------------------------------------------

void c64_bn1541_device::drive_strobe_w(int drive, int state)
{
	if (LOG) logerror("Drive %d parallel strobe %u\n", drive, state);

	m_drive_strobe[drive] = state;

	output_b(m_drive_strobe[0] && m_drive_strobe[1]);
}


//-------------------------------------------------
//  update_output
//-------------------------------------------------

void c64_bn1541_device::update_output()
{
	for (int drive = 0; drive < 2; drive++)
	{
		if (m_drive[drive].m_other != nullptr)
		{
			m_drive[drive].m_other->parallel_data_w(m_parallel_output & m_drive_data[drive ^ 1]);
		}
	}
}


//-------------------------------------------------
//  input_8 - CIA2 PC write
//-------------------------------------------------

void c64_bn1541_device::input_8(int state)
{
	if (LOG) logerror("C64 parallel strobe %u\n", state);

	for (auto &drive : m_drive)
	{
		if (drive.m_other != nullptr)
		{
			drive.m_other->parallel_strobe_w(state);
		}
	}
}
