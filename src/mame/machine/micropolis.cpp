// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    micropolis.c

    by Robbbert, August 2011.

This is a rough implementation of the Micropolis floppy-disk controller
as used for the Exidy Sorcerer. Since there is no documentation, coding
was done by looking at the Z80 code, and supplying the expected values.

Currently, only reading of disks is supported.

ToDo:
- Rewrite to be a standard device able to be used in a general way
- Fix bug where if you run a program on drive B,C,D then exit, you
  get a disk error.
- Enable the ability to write to disk when above bug gets fixed.
- When the controller is reset via command 5, what exactly gets reset?


Ports:
BE00 and BE01 can be used as command registers (they are identical),
              and they are also used as status registers (different).

BE02 and BE03 - read data, write data

***************************************************************************/


#include "emu.h"
#include "machine/micropolis.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/


#define STAT_RFC   0x20
#define STAT_TRACK0     0x08
#define STAT_READY      0x80

#define VERBOSE         0   /* General logging */
#define VERBOSE_DATA    0   /* Logging of each byte during read and write */

/* structure describing a single density track */
#define TRKSIZE_SD      16*270

#if 0
static const UINT8 track_SD[][2] = {
	{ 1, 0xff},     /*  1 * FF (marker)                      */
	{ 1, 0x00},     /*  1 byte, track number (00-4C)         */
	{ 1, 0x01},     /*  1 byte, sector number (00-0F)        */
	{10, 0x00},     /*  10 bytes of zeroes                   */
	{256, 0xe5},    /*  256 bytes of sector data             */
	{ 1, 0xb7},     /*  1 byte, CRC                          */
};
#endif

/***************************************************************************
    MAME DEVICE INTERFACE
***************************************************************************/

const device_type MICROPOLIS = &device_creator<micropolis_device>;

micropolis_device::micropolis_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MICROPOLIS, "MICROPOLIS", tag, owner, clock, "micropolis", __FILE__),
	m_read_dden(*this),
	m_write_intrq(*this),
	m_write_drq(*this),
	m_data(0),
	m_drive_num(0),
	m_track(0),
	m_sector(0),
	m_command(0),
	m_status(0),
	m_write_cmd(0),
	m_data_offset(0),
	m_data_count(0),
	m_sector_length(0),
	m_drive(nullptr)
{
	for (int i = 0; i < 6144; i++)
		m_buffer[i] = 0;

	for (int i = 0; i < 4; i++)
		m_floppy_drive_tags[i] = nullptr;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void micropolis_device::device_start()
{
	m_read_dden.resolve_safe(1);
	m_write_intrq.resolve_safe();
	m_write_drq.resolve_safe();

	save_item(NAME(m_data));
	save_item(NAME(m_drive_num));
	save_item(NAME(m_track));
	save_item(NAME(m_sector));
	save_item(NAME(m_command));
	save_item(NAME(m_status));
	save_item(NAME(m_write_cmd));
	save_item(NAME(m_buffer));
	save_item(NAME(m_data_offset));
	save_item(NAME(m_data_count));
	save_item(NAME(m_sector_length));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void micropolis_device::device_reset()
{
	for (int i = 0; i < 4; i++)
	{
		if (m_floppy_drive_tags[i])
		{
			legacy_floppy_image_device *img = siblingdevice<legacy_floppy_image_device>(m_floppy_drive_tags[i]);

			if (img)
			{
				img->floppy_drive_set_controller(this);
				//img->floppy_drive_set_index_pulse_callback(wd17xx_index_pulse_callback);
				img->floppy_drive_set_rpm(300.);
			}
		}
	}

	set_drive(0);

	m_drive_num = 0;
	m_sector = 0;
	m_track = 0;
	m_sector_length = 270;
	m_status = STAT_TRACK0;
}


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/


/* read a sector */
void micropolis_device::read_sector()
{
	m_data_offset = 0;
	m_data_count = m_sector_length;

	/* read data */
	m_drive->floppy_drive_read_sector_data(0, m_sector, (char *)m_buffer, m_sector_length);
}


void micropolis_device::write_sector()
{
#if 0
	/* at this point, the disc is write enabled, and data
	 * has been transfered into our buffer - now write it to
	 * the disc image or to the real disc
	 */

	/* find sector */
	m_data_count = m_sector_length;

	/* write data */
	m_drive->floppy_drive_write_sector_data(0, m_sector, (char *)m_buffer, m_sector_length, m_write_cmd & 0x01);
#endif
}




/***************************************************************************
    INTERFACE
***************************************************************************/

/* select a drive */
void micropolis_device::set_drive(UINT8 drive)
{
	if (VERBOSE)
		logerror("micropolis_set_drive: $%02x\n", drive);

	if (m_floppy_drive_tags[drive])
		m_drive = siblingdevice<legacy_floppy_image_device>(m_floppy_drive_tags[drive]);
}


/***************************************************************************
    DEVICE HANDLERS
***************************************************************************/


/* read the FDC status register. */
READ8_MEMBER( micropolis_device::status_r )
{
	static int inv = 0;

	if (offset)
		return m_status | m_drive_num;
	else
	{
		// FIXME - find out what controls current sector
		m_sector = (m_sector + 3 + inv) & 15;
		read_sector();
		inv ^= 1;
		return (m_status & STAT_READY) | m_sector;
	}
}


/* read the FDC data register */
READ8_MEMBER( micropolis_device::data_r )
{
	if (m_data_offset >= m_sector_length)
		m_data_offset = 0;

	return m_buffer[m_data_offset++];
}

/* write the FDC command register */
WRITE8_MEMBER( micropolis_device::command_w )
{
/* List of commands:
Command (bits 5,6,7)      Options (bits 0,1,2,3,4)
0    Not used
1    Drive/head select    bits 0,1 select drive 0-3; bit 4 chooses a side
2    INT sector control   bit 0 LO = disable; HI = enable
3    Step                 bit 0 LO step out; HI = step in (increment track number)
4    Set Write
5    Reset controller
6    Not used
7    Not used */

	int direction = 0;

	switch (data >> 5)
	{
	case 1:
		m_drive_num = data & 3;
		m_drive->floppy_mon_w(1); // turn off the old drive
		set_drive(m_drive_num); // select new drive
		m_drive->floppy_mon_w(0); // turn it on
		break;
	case 2:  // not emulated, not used in sorcerer
		break;
	case 3:
		if (BIT(data, 0))
		{
			if (m_track < 77)
			{
				m_track++;
				direction = 1;
			}
		}
		else
		{
			if (m_track)
			{
				m_track--;
				direction = -1;
			}
		}
		break;
	case 4: // not emulated, to be done
		break;
	case 5: // not emulated, to be done
		break;
	}


	m_status = STAT_RFC;

	if (BIT(data, 5))
		m_status |= STAT_READY;

	m_drive->floppy_drive_set_ready_state(1,0);


	if (!m_track)
		m_status |= STAT_TRACK0;

	m_drive->floppy_drive_seek(direction);
}


/* write the FDC data register */
WRITE8_MEMBER( micropolis_device::data_w )
{
	if (m_data_count > 0)
	{
		/* put byte into buffer */
		if (VERBOSE_DATA)
			logerror("micropolis_info buffered data: $%02X at offset %d.\n", data, m_data_offset);

		m_buffer[m_data_offset++] = data;

		if (--m_data_count < 1)
		{
			write_sector();

			m_data_offset = 0;
		}
	}
	else
	{
		if (VERBOSE)
			logerror("%s: micropolis_data_w $%02X\n", space.machine().describe_context(), data);
	}
	m_data = data;
}

READ8_MEMBER( micropolis_device::read )
{
	UINT8 data = 0;

	switch (offset & 0x03)
	{
	case 0: data = status_r(space, 0); break;
	case 1: data = status_r(space, 1); break;
	case 2:
	case 3: data = data_r(space, 0); break;
	}

	return data;
}

WRITE8_MEMBER( micropolis_device::write )
{
	switch (offset & 0x03)
	{
	case 0:
	case 1: command_w(space, 0, data); break;
	case 2:
	case 3: data_w(space, 0, data);    break;
	}
}
