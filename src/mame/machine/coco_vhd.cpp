// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    coco_vhd.c

    Color Computer Virtual Hard Drives

****************************************************************************

    Technical specs on the Virtual Hard Disk interface

    Address       Description
    -------       -----------
    FF80          Logical record number (high byte)
    FF81          Logical record number (middle byte)
    FF82          Logical record number (low byte)
    FF83          Command/status register
    FF84          Buffer address (high byte)
    FF85          Buffer address (low byte)

    Set the other registers, and then issue a command to FF83 as follows:

     0 = read 256-byte sector at LRN
     1 = write 256-byte sector at LRN
     2 = flush write cache (Closes and then opens the image file)

    Error values:

     0 = no error
    -1 = power-on state (before the first command is received)
    -2 = invalid command
     2 = VHD image does not exist
     4 = Unable to open VHD image file
     5 = access denied (may not be able to write to VHD image)

    IMPORTANT: The I/O buffer must NOT cross an 8K MMU bank boundary.

 ***************************************************************************/

#include "emu.h"
#include "coco_vhd.h"
#include "includes/coco.h"
#include "machine/ram.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define VERBOSE 1

#define VHDSTATUS_OK                    0x00
#define VHDSTATUS_NO_VHD_ATTACHED       0x02
#define VHDSTATUS_ACCESS_DENIED         0x05
#define VHDSTATUS_UNKNOWN_COMMAND       0xFE
#define VHDSTATUS_POWER_ON_STATE        0xFF

#define VHDCMD_READ     0
#define VHDCMD_WRITE    1
#define VHDCMD_FLUSH    2


/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

const device_type COCO_VHD = &device_creator<coco_vhd_image_device>;

//-------------------------------------------------
//  coco_vhd_image_device - constructor
//-------------------------------------------------

coco_vhd_image_device::coco_vhd_image_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, COCO_VHD, "Virtual Hard Disk", tag, owner, clock, "coco_vhd_image", __FILE__),
		device_image_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  coco_vhd_image_device - destructor
//-------------------------------------------------

coco_vhd_image_device::~coco_vhd_image_device()
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void coco_vhd_image_device::device_config_complete()
{
	// set brief and instance name
	update_names();
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void coco_vhd_image_device::device_start()
{
	m_status = VHDSTATUS_NO_VHD_ATTACHED;
	m_cpu = machine().device<cpu_device>(MAINCPU_TAG);
	m_cpu_space = &m_cpu->space(AS_PROGRAM);
}



//-------------------------------------------------
//  call_load
//-------------------------------------------------

bool coco_vhd_image_device::call_load()
{
	m_status = VHDSTATUS_POWER_ON_STATE;
	m_logical_record_number = 0;
	m_buffer_address = 0;
	return IMAGE_INIT_PASS;
}



//-------------------------------------------------
//  coco_vhd_readwrite
//-------------------------------------------------

void coco_vhd_image_device::coco_vhd_readwrite(UINT8 data)
{
	int result, i;
	UINT32 bytes_to_read;
	UINT32 bytes_to_write;
	UINT64 seek_position;
	UINT64 total_size;
	char buffer[1024];

	/* access the image */
	if (!exists())
	{
		m_status = VHDSTATUS_NO_VHD_ATTACHED;
		return;
	}

	/* perform the seek */
	seek_position = ((UINT64) 256) * m_logical_record_number;
	total_size = length();
	result = fseek(MIN(seek_position, total_size), SEEK_SET);
	if (result < 0)
	{
		m_status = VHDSTATUS_ACCESS_DENIED;
		return;
	}

	/* expand the disk, if necessary */
	if (data == VHDCMD_WRITE)
	{
		while(total_size < seek_position)
		{
			memset(buffer, 0, sizeof(buffer));

			bytes_to_write = (UINT32) MIN(seek_position - total_size, (UINT64) sizeof(buffer));
			result = fwrite(buffer, bytes_to_write);
			if (result != bytes_to_write)
			{
				m_status = VHDSTATUS_ACCESS_DENIED;
				return;
			}

			total_size += bytes_to_write;
		}
	}

	switch(data)
	{
		case VHDCMD_READ: /* Read sector */
			memset(buffer, 0, 256);
			if (total_size > seek_position)
			{
				bytes_to_read = (UINT32) MIN((UINT64) 256, total_size - seek_position);
				result = fread(buffer, bytes_to_read);
				if (result != bytes_to_read)
				{
					m_status = VHDSTATUS_ACCESS_DENIED;
					return;
				}
			}

			/* write the bytes to memory */
			for (i = 0; i < 256; i++)
				m_cpu_space->write_byte(m_buffer_address + i, buffer[i]);

			m_status = VHDSTATUS_OK;
			break;

		case VHDCMD_WRITE: /* Write Sector */
			/* read the bytes from memory */
			for (i = 0; i < 256; i++)
				buffer[i] = m_cpu_space->read_byte(m_buffer_address + i);

			/* and write to disk */
			result = fwrite(buffer, 256);
			if (result != 256)
			{
				m_status = VHDSTATUS_ACCESS_DENIED;
				return;
			}

			m_status = VHDSTATUS_OK;
			break;

		case VHDCMD_FLUSH: /* Flush file cache */
			m_status = VHDSTATUS_OK;
			break;

		default:
			m_status = VHDSTATUS_UNKNOWN_COMMAND;
			break;
	}
}



UINT8 coco_vhd_image_device::read(offs_t offset)
{
	UINT8 result = 0;

	switch(offset)
	{
		case 0xff83 - 0xff80:
			if (VERBOSE)
				logerror("vhd: Status read: %d\n", m_status);
			result = m_status;
			break;
	}
	return result;
}



void coco_vhd_image_device::write(offs_t offset, UINT8 data)
{
	int pos;

	switch(offset)
	{
		case 0xff80 - 0xff80:
		case 0xff81 - 0xff80:
		case 0xff82 - 0xff80:
			pos = ((0xff82 - 0xff80) - offset) * 8;
			m_logical_record_number &= ~(0xFF << pos);
			m_logical_record_number += data << pos;
			if (VERBOSE)
				logerror("vhd: LRN write: %6.6X\n", m_logical_record_number);
			break;

		case 0xff83 - 0xff80:
			coco_vhd_readwrite(data);
			if (VERBOSE)
				logerror("vhd: Command: %d\n", data);
			break;

		case 0xff84 - 0xff80:
			m_buffer_address &= 0xFFFF00FF;
			m_buffer_address += data << 8;
			if (VERBOSE)
				logerror("vhd: BA write: %X (%2.2X..)\n", m_buffer_address, data);
			break;

		case 0xff85 - 0xff80:
			m_buffer_address &= 0xFFFFFF00;
			m_buffer_address += data;
			if (VERBOSE)
				logerror("vhd: BA write: %X (..%2.2X)\n", m_buffer_address, data);
			break;
	}
}
