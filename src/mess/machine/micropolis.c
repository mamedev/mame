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
#include "imagedev/flopdrv.h"
#include "machine/micropolis.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/


#define STAT_RFC   0x20
#define STAT_TRACK0     0x08
#define STAT_READY      0x80

#define VERBOSE			0	/* General logging */
#define VERBOSE_DATA	0	/* Logging of each byte during read and write */

/* structure describing a single density track */
#define TRKSIZE_SD		16*270
#if 0
static const UINT8 track_SD[][2] = {
	{ 1, 0xff}, 	/*  1 * FF (marker)                      */
	{ 1, 0x00}, 	/*  1 byte, track number (00-4C)         */
	{ 1, 0x01}, 	/*  1 byte, sector number (00-0F)        */
	{10, 0x00},     /*  10 bytes of zeroes                   */
	{256, 0xe5},	/*  256 bytes of sector data             */
	{ 1, 0xb7}, 	/*  1 byte, CRC                          */
};
#endif


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _micropolis_state micropolis_state;
struct _micropolis_state
{
	/* register */
	UINT8 data;
	UINT8 drive_num;
	UINT8 track;
	UINT8 sector;
	UINT8 command;
	UINT8 status;

	UINT8	write_cmd;				/* last write command issued */

	UINT8	buffer[6144];			/* I/O buffer (holds up to a whole track) */
	UINT32	data_offset;			/* offset into I/O buffer */
	INT32	data_count; 			/* transfer count from/into I/O buffer */

	UINT32	sector_length;			/* sector length (byte) */

	/* this is the drive currently selected */
	device_t *drive;

	/* Pointer to interface */
	const micropolis_interface *intf;
};


/***************************************************************************
    DEFAULT INTERFACES
***************************************************************************/

const micropolis_interface default_micropolis_interface =
{
	DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, { FLOPPY_0, FLOPPY_1, FLOPPY_2, FLOPPY_3}
};

const micropolis_interface default_micropolis_interface_2_drives =
{
	DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, { FLOPPY_0, FLOPPY_1, NULL, NULL}
};


/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE micropolis_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == MICROPOLIS);

	return (micropolis_state *)downcast<micropolis_device *>(device)->token();
}


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/


/* read a sector */
static void micropolis_read_sector(device_t *device)
{
	micropolis_state *w = get_safe_token(device);
	w->data_offset = 0;
	w->data_count = w->sector_length;

	/* read data */
	floppy_drive_read_sector_data(w->drive, 0, w->sector, (char *)w->buffer, w->sector_length);
}


static void micropolis_write_sector(device_t *device)
{
#if 0
	micropolis_state *w = get_safe_token(device);
	/* at this point, the disc is write enabled, and data
     * has been transfered into our buffer - now write it to
     * the disc image or to the real disc
     */

	/* find sector */
	w->data_count = w->sector_length;

	/* write data */
	floppy_drive_write_sector_data(w->drive, 0, w->sector, (char *)w->buffer, w->sector_length, w->write_cmd & 0x01);
#endif
}




/***************************************************************************
    INTERFACE
***************************************************************************/

/* select a drive */
void micropolis_set_drive(device_t *device, UINT8 drive)
{
	micropolis_state *w = get_safe_token(device);

	if (VERBOSE)
		logerror("micropolis_set_drive: $%02x\n", drive);

	if (w->intf->floppy_drive_tags[drive])
		w->drive = device->siblingdevice(w->intf->floppy_drive_tags[drive]);
}


/***************************************************************************
    DEVICE HANDLERS
***************************************************************************/


/* read the FDC status register. */
READ8_DEVICE_HANDLER( micropolis_status_r )
{
	micropolis_state *w = get_safe_token(device);
	static int inv = 0;

	if (offset)
		return w->status | w->drive_num;
	else
	{
		// FIXME - find out what controls current sector
		w->sector = (w->sector + 3 + inv) & 15;
		micropolis_read_sector(device);
		inv ^= 1;
		return (w->status & STAT_READY) | w->sector;
	}
}


/* read the FDC data register */
READ8_DEVICE_HANDLER( micropolis_data_r )
{
	micropolis_state *w = get_safe_token(device);

	if (w->data_offset >= w->sector_length)
		w->data_offset = 0;

	return w->buffer[w->data_offset++];
}

/* write the FDC command register */
WRITE8_DEVICE_HANDLER( micropolis_command_w )
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

	micropolis_state *w = get_safe_token(device);
	int direction = 0;

	switch (data >> 5)
	{
	case 1:
		w->drive_num = data & 3;
		floppy_mon_w(w->drive, 1); // turn off the old drive
		micropolis_set_drive(device, w->drive_num); // select new drive
		floppy_mon_w(w->drive, 0); // turn it on
		break;
	case 2:  // not emulated, not used in sorcerer
		break;
	case 3:
		if (BIT(data, 0))
		{
			if (w->track < 77)
			{
				w->track++;
				direction = 1;
			}
		}
		else
		{
			if (w->track)
			{
				w->track--;
				direction = -1;
			}
		}
		break;
	case 4: // not emulated, to be done
		break;
	case 5: // not emulated, to be done
		break;
	}


	w->status = STAT_RFC;

	if (BIT(data, 5))
		w->status |= STAT_READY;

	floppy_drive_set_ready_state(w->drive, 1,0);


	if (!w->track)
		w->status |= STAT_TRACK0;

	floppy_drive_seek(w->drive, direction);
}


/* write the FDC data register */
WRITE8_DEVICE_HANDLER( micropolis_data_w )
{
	micropolis_state *w = get_safe_token(device);

	if (w->data_count > 0)
	{
		/* put byte into buffer */
		if (VERBOSE_DATA)
			logerror("micropolis_info buffered data: $%02X at offset %d.\n", data, w->data_offset);

		w->buffer[w->data_offset++] = data;

		if (--w->data_count < 1)
		{
			micropolis_write_sector(device);

			w->data_offset = 0;
		}
	}
	else
	{
		if (VERBOSE)
			logerror("%s: micropolis_data_w $%02X\n", device->machine().describe_context(), data);
	}
	w->data = data;
}

READ8_DEVICE_HANDLER( micropolis_r )
{
	UINT8 data = 0;

	switch (offset & 0x03)
	{
	case 0: data = micropolis_status_r(device, 0); break;
	case 1:	data = micropolis_status_r(device, 1); break;
	case 2:
	case 3:	data = micropolis_data_r(device, 0); break;
	}

	return data;
}

WRITE8_DEVICE_HANDLER( micropolis_w )
{
	switch (offset & 0x03)
	{
	case 0:
	case 1:	micropolis_command_w(device, 0, data); break;
	case 2:
	case 3: micropolis_data_w(device, 0, data);    break;
	}
}


/***************************************************************************
    MAME DEVICE INTERFACE
***************************************************************************/

static DEVICE_START( micropolis )
{
	micropolis_state *w = get_safe_token(device);

	assert(device->static_config() != NULL);

	w->intf = (const micropolis_interface*)device->static_config();
}

static DEVICE_RESET( micropolis )
{
	micropolis_state *w = get_safe_token(device);
	int i;

	for (i = 0; i < 4; i++)
	{
		if(w->intf->floppy_drive_tags[i])
		{
			device_t *img = NULL;

			img = device->siblingdevice(w->intf->floppy_drive_tags[i]);

			if (img)
			{
				floppy_drive_set_controller(img,device);
				//floppy_drive_set_index_pulse_callback(img, wd17xx_index_pulse_callback);
				floppy_drive_set_rpm( img, 300.);
			}
		}
	}

	micropolis_set_drive(device, 0);

	w->drive_num = 0;
	w->sector = 0;
	w->track = 0;
	w->sector_length = 270;
	w->status = STAT_TRACK0;
}

void micropolis_reset(device_t *device)
{
	DEVICE_RESET_CALL( micropolis );
}

const device_type MICROPOLIS = &device_creator<micropolis_device>;

micropolis_device::micropolis_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MICROPOLIS, "MICROPOLIS", tag, owner, clock)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(micropolis_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void micropolis_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void micropolis_device::device_start()
{
	DEVICE_START_NAME( micropolis )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void micropolis_device::device_reset()
{
	DEVICE_RESET_NAME( micropolis )(this);
}


