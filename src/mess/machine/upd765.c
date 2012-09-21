/***************************************************************************

    machine/upd765.c

    Functions to emulate a UPD765/Intel 8272 compatible floppy disk controller

    Code by Kevin Thacker.

    TODO:

    - overrun condition
    - Scan Commands
    - crc error in id field and crc error in data field errors
    - disc not present, and no sectors on track for data, deleted data, write, write deleted,
        read a track etc
        - end of cylinder condition - almost working, needs fixing  with
                PCW and PC drivers
    - resolve "ready" state stuff (ready state when reset for PC, ready state change while processing command AND
    while idle)

    Changes:
    091006 (Mariusz Wojcieszek, changes needed by QX-10):
    - allowed "Sense Interrupt Status" command when Seek is active
    - DIO bit in status register (0x40) is cleared when "Read Data" command is executed,
      it is later set during result phase.

***************************************************************************/

#include "emu.h"
#include "machine/upd765.h"


enum UPD765_PHASE
{
	UPD765_COMMAND_PHASE_FIRST_BYTE,
	UPD765_COMMAND_PHASE_BYTES,
	UPD765_RESULT_PHASE,
	UPD765_EXECUTION_PHASE_READ,
	UPD765_EXECUTION_PHASE_WRITE
};

/* supported versions */
enum UPD765_VERSION
{
	TYPE_UPD765A = 0,
	TYPE_UPD765B = 1,
	TYPE_SMC37C78 = 2,
	TYPE_UPD72065 = 3
};


/* uncomment the following line for verbose information */
#define LOG_VERBOSE		0
#define LOG_COMMAND		0
#define LOG_EXTRA		0
#define LOG_INTERRUPT	0

/* uncomment this to not allow end of cylinder "error" */
#define NO_END_OF_CYLINDER



/* state of upd765 Interrupt (INT) output */
#define UPD765_INT	0x02
/* data rate for floppy discs (MFM data) */
#define UPD765_DATA_RATE	32
/* state of upd765 terminal count input*/
#define UPD765_TC	0x04

#define UPD765_DMA_MODE 0x08

#define UPD765_SEEK_OPERATION_IS_RECALIBRATE 0x01

#define UPD765_SEEK_ACTIVE 0x010
/* state of upd765 DMA DRQ output */
#define UPD765_DMA_DRQ 0x020
/* state of upd765 FDD READY input */
#define UPD765_FDD_READY 0x040

#define UPD765_MF	0x40
#define UPD765_MT	0x80

#define UPD765_RESET 0x080

#define UPD765_BAD_MEDIA 0x100

struct upd765_t
{
	devcb_resolved_write_line	out_int_func;
	devcb_resolved_write_line	out_drq_func;

	unsigned long	sector_counter;
	/* version of fdc to emulate */
	UPD765_VERSION version;

	/* main status register */
	unsigned char    FDC_main;
	/* data register */
	unsigned char	upd765_data_reg;

	unsigned char c,h,r,n;

	int sector_id;

	int data_type;

	char format_data[4];

	UPD765_PHASE    upd765_phase;
	unsigned int    upd765_command_bytes[16];
	unsigned int    upd765_result_bytes[16];
	unsigned int    upd765_transfer_bytes_remaining;
	unsigned int    upd765_transfer_bytes_count;
	unsigned int    upd765_status[4];
	/* present cylinder number per drive */
	unsigned int    pcn[4];

	/* drive being accessed. drive outputs from fdc */
	unsigned int    drive;
	/* side being accessed: side output from fdc */
	unsigned int	side;


	/* step rate time in us */
	unsigned long	srt_in_ms;

	unsigned int	ncn;

//  unsigned int    upd765_id_index;
	char *execution_phase_data;
	unsigned int	upd765_flags;

//  unsigned char specify[2];
//  unsigned char perpendicular_mode[1];

	int command;

	UINT8			ready_changed;

	emu_timer *seek_timer;
	emu_timer *timer;
	int timer_type;

	emu_timer *command_timer;

	char *data_buffer;
	const upd765_interface *intf;

	bool pool;
};

//static void upd765_setup_data_request(unsigned char Data);
static void upd765_setup_command(device_t *device);
static TIMER_CALLBACK(upd765_continue_command);
static int upd765_sector_count_complete(device_t *device);
static void upd765_increment_sector(device_t *device);
static void upd765_update_state(device_t *device);
static void upd765_set_dma_drq(device_t *device,int state);
static void upd765_set_int(device_t *device,int state);

static const INT8 upd765_cmd_size[32] =
{
	1,1,9,3,2,9,9,2,1,9,2,1,9,6,1,3,
	1,9,1,1,1,1,9,1,1,9,1,1,1,9,1,1
};

INLINE upd765_t *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == UPD765A || device->type() == UPD765B ||
		device->type() == SMC37C78 || device->type() == UPD72065);

	return (upd765_t *)downcast<upd765a_device *>(device)->token();
}

static device_t *current_image(device_t *device)
{
	device_t *image = NULL;
	upd765_t *fdc = get_safe_token(device);

	if (!fdc->intf->get_image)
	{
		if (fdc->intf->floppy_drive_tags[fdc->drive] != NULL)
		{
			if (device->owner() != NULL)
				image = device->owner()->subdevice(fdc->intf->floppy_drive_tags[fdc->drive]);
			else
				image = device->machine().device(fdc->intf->floppy_drive_tags[fdc->drive]);
		}
	}
	else
	{
		image = fdc->intf->get_image(device, fdc->drive);
	}
	return image;
}

static void upd765_setup_drive_and_side(device_t *device)
{
	upd765_t *fdc = get_safe_token(device);
	/* drive index upd765 sees */
	fdc->drive = fdc->upd765_command_bytes[1] & 0x03;
	/* side index upd765 sees */
	fdc->side = (fdc->upd765_command_bytes[1]>>2) & 0x01;
}


/* setup status register 0 based on data in status register 1 and 2 */
static void upd765_setup_st0(device_t *device)
{
	upd765_t *fdc = get_safe_token(device);
	/* clear completition status bits, drive bits and side bits */
	fdc->upd765_status[0] &= ~((1<<7) | (1<<6) | (1<<2) | (1<<1) | (1<<0));
	/* fill in drive */
	fdc->upd765_status[0] |= fdc->drive | (fdc->side<<2);

	/* fill in completion status bits based on bits in st0, st1, st2 */
	/* no error bits set */
	if ((fdc->upd765_status[1] | fdc->upd765_status[2])==0)
	{
		return;
	}

	fdc->upd765_status[0] |= 0x040;
}


static int upd765_n_to_bytes(int n)
{
	/* 0-> 128 bytes, 1->256 bytes, 2->512 bytes etc */
	/* data_size = ((1<<(N+7)) */
	return 1<<(n+7);
}

static void upd765_set_data_request(device_t *device)
{
	upd765_t *fdc = get_safe_token(device);
	fdc->FDC_main |= 0x080;
}

static void upd765_clear_data_request(device_t *device)
{
	upd765_t *fdc = get_safe_token(device);
	fdc->FDC_main &= ~0x080;
}

static int upd765_get_rdy(device_t *device)
{
	upd765_t *fdc = get_safe_token(device);

	if (fdc->intf->rdy_pin == UPD765_RDY_PIN_CONNECTED)
	{
		device_t *img = current_image(device);
		if (img!=NULL) {
			return floppy_drive_get_flag_state(img, FLOPPY_DRIVE_READY);
		} else {
			return 0;
		}
	}
	else
		return 1;
}

static void upd765_seek_complete(device_t *device)
{
	/* tested on Amstrad CPC */

	/* if a seek is done without drive connected: */
	/*  abnormal termination of command,
        seek complete,
        not ready
    */

	/* if a seek is done with drive connected, but disc missing: */
	/* abnormal termination of command,
        seek complete,
        not ready */

	/* if a seek is done with drive connected and disc in drive */
	/* seek complete */


	/* On the PC however, it appears that recalibrates and seeks can be performed without
    a disc in the drive. */

	/* Therefore, the above output is dependant on the state of the drive */

	/* In the Amstrad CPC, the drive select is provided by the UPD765. A single port is also
    assigned for setting the drive motor state. The motor state controls the motor of the selected
    drive */

	/* On the PC the drive can be selected with the DIGITAL OUTPUT REGISTER, and the motor of each
    of the 4 possible drives is also settable using the same register */

	/* Assumption for PC: (NOT TESTED - NEEDS VERIFICATION) */

	/* If a seek is done without drive connected: */
	/* abnormal termination of command,
        seek complete,
        fault
        */

	/* if a seek is done with drive connected, but disc missing: */
	/* seek complete */

	/* if a seek is done with drive connected and disc in drive: */
	/* seek complete */

	/* On Amstrad CPC:
        If drive not connected, or drive connected but disc not in drive, not ready!
        If drive connected and drive motor on, ready!
       On PC:
        Drive is always ready!

        In 37c78 docs, the ready bits of the upd765 are marked as unused.
        This indicates it is always ready!!!!!
    */

	device_t *img = current_image(device);
	upd765_t *fdc = get_safe_token(device);

	fdc->pcn[fdc->drive] = fdc->ncn;

	fdc->upd765_status[0] = 0x20;

	/* drive ready? */
	if (img != NULL && upd765_get_rdy(device))
	{
		/* recalibrate? */
		if (fdc->upd765_flags & UPD765_SEEK_OPERATION_IS_RECALIBRATE)
		{
			/* not at track 0? */
			if (fdc->pcn[fdc->drive] != 0)
				/* no, track 0 failed after 77 steps */
				fdc->upd765_status[0] |= 0x40 | 0x10;
		}
	}
	else
	{
		/* abnormal termination, not ready */
		fdc->upd765_status[0] |= 0x40 | 0x08;
	}

	/* set drive and side. note: commented out side to avoid problems with the tf20 */
	fdc->upd765_status[0] |= fdc->drive; //| (fdc->side<<2);

	upd765_set_int(device,0);
	upd765_set_int(device,1);

	fdc->upd765_flags &= ~UPD765_SEEK_ACTIVE;

	upd765_idle(device);
}

static TIMER_CALLBACK(upd765_seek_timer_callback)
{
	device_t *device = (device_t *)ptr;
	upd765_t *fdc = get_safe_token(device);
	/* seek complete */
	upd765_seek_complete(device);

	fdc->seek_timer->reset();
}

static void upd765_timer_func(device_t *device, int timer_type)
{
	upd765_t *fdc = get_safe_token(device);
	/* type 0 = data transfer mode in execution phase */
	if (fdc->timer_type == 0)
	{
		/* set data request */
		upd765_set_data_request(device);

		fdc->timer_type = 4;

		if (!(fdc->upd765_flags & UPD765_DMA_MODE))
		{
			if (fdc->upd765_command_bytes[0] & UPD765_MF)
			{
				/* MFM */
				fdc->timer->reset(attotime::from_usec(13));
			}
			else
			{
				/* FM */
				fdc->timer->reset(attotime::from_usec(27));
			}
		}
		else
		{
			upd765_timer_func(device, fdc->timer_type);
		}
	}
	else if (fdc->timer_type==2)
	{
		/* result phase begin */

		/* generate a int for specific commands */
		switch (fdc->command) {
		case 2:		/* read a track */
		case 5:		/* write data */
		case 6:		/* read data */
		case 9:		/* write deleted data */
		case 10:	/* read id */
		case 12:	/* read deleted data */
		case 13:	/* format at track */
		case 17:	/* scan equal */
		case 19:	/* scan low or equal */
		case 29:	/* scan high or equal */
			upd765_set_int(device,1);
			break;

		default:
			break;
		}

		upd765_set_data_request(device);

		fdc->timer->reset();
	}
	else if (fdc->timer_type == 4)
	{
		/* if in dma mode, a int is not generated per byte. If not in  DMA mode
        a int is generated per byte */
		if (fdc->upd765_flags & UPD765_DMA_MODE)
		{
			upd765_set_dma_drq(device,1);
		}
		else
		{
			if (fdc->FDC_main & (1<<7))
			{
				/* set int to indicate data is ready */
				upd765_set_int(device,1);
			}
		}

		fdc->timer->reset();
	}
}

static TIMER_CALLBACK(upd765_timer_callback)
{
	device_t *device = (device_t *)ptr;
	upd765_timer_func(device,param);
}

/* after (32-27) the DRQ is set, then 27 us later, the int is set.
I don't know if this is correct, but it is required for the PCW driver.
In this driver, the first NMI calls the handler function, furthur NMI's are
effectively disabled by reading the data before the NMI int can be set.
*/

static void upd765_setup_timed_generic(device_t *device, int timer_type, attotime duration)
{
	upd765_t *fdc = get_safe_token(device);

	fdc->timer_type = timer_type;

	if (!(fdc->upd765_flags & UPD765_DMA_MODE))
	{
		fdc->timer->adjust(duration);
	}
	else
	{
		upd765_timer_func(device,fdc->timer_type);
		fdc->timer->reset();
	}
}

/* setup data request */
static void upd765_setup_timed_data_request(device_t *device, int bytes)
{
	/* setup timer to trigger in UPD765_DATA_RATE us */
	upd765_setup_timed_generic(device, 0, attotime::from_usec(32-27)	/*UPD765_DATA_RATE)*bytes*/);
}

/* setup result data request */
static void upd765_setup_timed_result_data_request(device_t *device)
{
	upd765_setup_timed_generic(device, 2, attotime::from_usec(UPD765_DATA_RATE*2));
}


/* sets up a timer to issue a seek complete in signed_tracks time */
static void upd765_setup_timed_int(device_t *device,int signed_tracks)
{
	upd765_t *fdc = get_safe_token(device);
	/* setup timer to signal after seek time is complete */
	fdc->seek_timer->adjust(attotime::from_double(fdc->srt_in_ms*abs(signed_tracks)*0.001));
}

static void upd765_seek_setup(device_t *device, int is_recalibrate)
{
	device_t *img;
	int signed_tracks;
	upd765_t *fdc = get_safe_token(device);

	fdc->upd765_flags |= UPD765_SEEK_ACTIVE;

	if (is_recalibrate)
	{
		/* head cannot be specified with recalibrate */
		fdc->upd765_command_bytes[1] &=~0x04;
	}

	upd765_setup_drive_and_side(device);

	img = current_image(device);

	fdc->FDC_main |= (1<<fdc->drive);
	fdc->FDC_main |= 0x20;  // execution phase
	fdc->FDC_main &= ~0x10;  // not busy, can send another seek/recalibrate
	                         // for a different drive, or sense int status

	/* recalibrate command? */
	if (is_recalibrate)
	{
		fdc->upd765_flags |= UPD765_SEEK_OPERATION_IS_RECALIBRATE;

		fdc->ncn = 0;

		/* if drive is already at track 0, or drive is not ready */
		if (img == NULL || floppy_tk00_r(img) == CLEAR_LINE || (!upd765_get_rdy(device)))
		{
			/* seek completed */
//          upd765_seek_complete(device);
			// delay for the time of 1 step, the PCW does not like immediate recalibrates
			upd765_setup_timed_int(device,1);
		}
		else
		{
			/* is drive present? */
			if (1) //image_slotexists(img)) //fix me
			{
				/* yes - calculate real number of tracks to seek */

				int current_track;

				/* get current track */
				current_track = floppy_drive_get_current_track(img);

				/* get number of tracks to seek */
				signed_tracks = -current_track;
			}
			else
			{
				/* no, seek 77 tracks and then stop */
				/* true for UPD765A, but not for other variants */
				signed_tracks = -77;
			}

			/* perform seek - if drive isn't present it will not do anything */
			floppy_drive_seek(img, signed_tracks);

			if (signed_tracks!=0)
			{
				upd765_setup_timed_int(device,signed_tracks);
			}
			else
			{
				upd765_seek_complete(device);
			}
		}
	}
	else
	{

		fdc->upd765_flags &= ~UPD765_SEEK_OPERATION_IS_RECALIBRATE;

		fdc->ncn = fdc->upd765_command_bytes[2];

		/* get signed tracks */
		signed_tracks = fdc->ncn - fdc->pcn[fdc->drive];

		/* perform seek - if drive isn't present it will not do anything */
		floppy_drive_seek(img, signed_tracks);

		/* if no tracks to seek, or drive is not ready, seek is complete */
		if (img == NULL || (signed_tracks==0) || (!upd765_get_rdy(device)))
		{
			upd765_seek_complete(device);
		}
		else
		{
			/* seek complete - issue an interrupt */
			upd765_setup_timed_int(device,signed_tracks);
		}
	}
//    upd765_idle(device);
}



static void upd765_setup_execution_phase_read(device_t *device, char *ptr, int size)
{
	upd765_t *fdc = get_safe_token(device);

//  fdc->FDC_main &= ~0x040;                     /* FDC->CPU */
	fdc->FDC_main |= 0x040;                      /* FDC->CPU */

	fdc->upd765_transfer_bytes_count = 0;
	fdc->upd765_transfer_bytes_remaining = size;
	fdc->execution_phase_data = ptr;
	fdc->upd765_phase = UPD765_EXECUTION_PHASE_READ;

	upd765_setup_timed_data_request(device, 1);
}

static void upd765_setup_execution_phase_write(device_t *device, char *ptr, int size)
{
	upd765_t *fdc = get_safe_token(device);

	fdc->FDC_main &= ~0x040;                     /* FDC->CPU */

	fdc->upd765_transfer_bytes_count = 0;
	fdc->upd765_transfer_bytes_remaining = size;
	fdc->execution_phase_data = ptr;
	fdc->upd765_phase = UPD765_EXECUTION_PHASE_WRITE;

	/* setup a data request with first byte */
	upd765_setup_timed_data_request(device,1);
}


static void upd765_setup_result_phase(device_t *device, int byte_count)
{
	upd765_t *fdc = get_safe_token(device);

	fdc->FDC_main |= 0x040;                     /* FDC->CPU */
	fdc->FDC_main &= ~0x020;                    /* not execution phase */

	fdc->upd765_transfer_bytes_count = 0;
	fdc->upd765_transfer_bytes_remaining = byte_count;
	fdc->upd765_phase = UPD765_RESULT_PHASE;

	upd765_setup_timed_result_data_request(device);
}

void upd765_idle(device_t *device)
{
	upd765_t *fdc = get_safe_token(device);

	fdc->FDC_main &= ~0x040;                     /* CPU->FDC */
	fdc->FDC_main &= ~0x020;                    /* not execution phase */
	fdc->FDC_main &= ~0x010;                     /* not busy */
	fdc->upd765_phase = UPD765_COMMAND_PHASE_FIRST_BYTE;

	upd765_set_data_request(device);
}



/* change flags */
static void upd765_change_flags(device_t *device,unsigned int flags, unsigned int mask)
{
	unsigned int new_flags;
	unsigned int changed_flags;
	upd765_t *fdc = get_safe_token(device);

	assert((flags & ~mask) == 0);

	/* compute the new flags and which ones have changed */
	new_flags = fdc->upd765_flags & ~mask;
	new_flags |= flags;
	changed_flags = fdc->upd765_flags ^ new_flags;
	fdc->upd765_flags = new_flags;

	/* if interrupt changed, call the handler */
	if (changed_flags & UPD765_INT)
		fdc->out_int_func((fdc->upd765_flags & UPD765_INT) ? 1 : 0);

	/* if DRQ changed, call the handler */
	if (changed_flags & UPD765_DMA_DRQ)
		fdc->out_drq_func((fdc->upd765_flags & UPD765_DMA_DRQ) ? 1 : 0);
}



/* set int output */
static void upd765_set_int(device_t *device, int state)
{
	if (LOG_INTERRUPT)
		logerror("upd765_set_int(): state=%d\n", state);
	upd765_change_flags(device, state ? UPD765_INT : 0, UPD765_INT);
}



/* set dma request output */
static void upd765_set_dma_drq(device_t *device, int state)
{
	upd765_change_flags(device, state ? UPD765_DMA_DRQ : 0, UPD765_DMA_DRQ);
}

READ_LINE_DEVICE_HANDLER( upd765_int_r )
{
	upd765_t *fdc = get_safe_token(device);

	return (fdc->upd765_flags & UPD765_INT) ? 1 : 0;
}

READ_LINE_DEVICE_HANDLER( upd765_drq_r )
{
	upd765_t *fdc = get_safe_token(device);

	return (fdc->upd765_flags & UPD765_DMA_DRQ) ? 1 : 0;
}


/* Drive ready */

/*

A drive will report ready if:
- drive is selected
- disc is in the drive
- disk is rotating at a constant speed (normally 300rpm)

On more modern PCs, a ready signal is not provided by the drive.
This signal is not used in the PC design and was eliminated to save costs
If you look at the datasheets for the modern UPD765 variants, you will see the Ready
signal is not mentioned.

On the original UPD765A, ready signal is required, and some commands will fail if the drive
is not ready.




*/




/* done when ready state of drive changes */
/* this ignores if command is active, in which case command should terminate immediatly
with error */
static void upd765_set_ready_change_callback(device_t *controller, device_t *img, int state)
{
	upd765_t *fdc = get_safe_token(controller);
	int drive = floppy_get_drive(img);

	if (LOG_EXTRA)
		logerror("upd765: ready state change\n");

	/* drive that changed state */
	fdc->upd765_status[0] = 0x0c0 | drive;

	/* not ready */
	if (state==0 && fdc->intf->rdy_pin == UPD765_RDY_PIN_CONNECTED )
		fdc->upd765_status[0] |= 8;

	/* trigger an int */
	upd765_set_int(controller, 1);
}


/* terminal count input */
WRITE_LINE_DEVICE_HANDLER( upd765_tc_w )
{
	int old_state;
	upd765_t *fdc = get_safe_token(device);

	old_state = fdc->upd765_flags;

	/* clear drq */
	upd765_set_dma_drq(device, 0);

	fdc->upd765_flags &= ~UPD765_TC;
	if (state)
	{
		fdc->upd765_flags |= UPD765_TC;
	}

	/* changed state? */
	if (((fdc->upd765_flags^old_state) & UPD765_TC)!=0)
	{
		/* now set? */
		if ((fdc->upd765_flags & UPD765_TC)!=0)
		{
			/* yes */
			if (fdc->timer)
			{
				if (fdc->timer_type==0)
				{
					fdc->timer->reset();


				}
			}

#ifdef NO_END_OF_CYLINDER
			fdc->command_timer->adjust(attotime::zero);
#else
			upd765_update_state(device);
#endif
		}
	}
}

READ8_DEVICE_HANDLER( upd765_status_r )
{
	upd765_t *fdc = get_safe_token(device);
	if (LOG_EXTRA)
		logerror("%s: upd765_status_r: %02x\n", space.machine().describe_context(), fdc->FDC_main);
	return fdc->FDC_main;
}


/* control mark handling code */

/* if SK==1, and we are executing a read data command, and a deleted data mark is found,
skip it.
if SK==1, and we are executing a read deleted data command, and a data mark is found,
skip it. */

static int upd765_read_skip_sector(device_t *device)
{
	upd765_t *fdc = get_safe_token(device);
	/* skip set? */
	if ((fdc->upd765_command_bytes[0] & (1<<5))!=0)
	{
		/* read data? */
		if (fdc->command == 0x06)
		{
			/* did we just find a sector with deleted data mark? */
			if (fdc->data_type == UPD765_DAM_DELETED_DATA)
			{
				/* skip it */
				return TRUE;
			}
		}
		/* deleted data? */
		else
		if (fdc->command == 0x0c)
		{
			/* did we just find a sector with data mark ? */
			if (fdc->data_type == UPD765_DAM_DATA)
			{
				/* skip it */
				return TRUE;
			}
		}
	}

	/* do not skip */
	return FALSE;
}

/* this is much closer to how the upd765 actually gets sectors */
/* used by read data, read deleted data, write data, write deleted data */
/* What the upd765 does:

  - get next sector id from disc
  - if sector id matches id specified in command, it will
    search for next data block and read data from it.

  - if the index is seen twice while it is searching for a sector, then the sector cannot be found
*/

static void upd765_get_next_id(device_t *device, chrn_id *id)
{
	upd765_t *fdc = get_safe_token(device);
	device_t *img = current_image(device);

	/* get next id from disc */
	floppy_drive_get_next_id(img, fdc->side,id);

	fdc->sector_id = id->data_id;

	/* set correct data type */
	fdc->data_type = UPD765_DAM_DATA;
	if (id->flags & ID_FLAG_DELETED_DATA)
	{
		fdc->data_type = UPD765_DAM_DELETED_DATA;
	}
}

static int upd765_get_matching_sector(device_t *device)
{
	upd765_t *fdc = get_safe_token(device);
	device_t *img = current_image(device);
	chrn_id id;

	/* number of times we have seen index hole */
	int index_count = 0;

	if (fdc->upd765_flags & UPD765_BAD_MEDIA) {
		fdc->upd765_status[1] |= 1;
		return FALSE;
	}

	/* get sector id's */
	do
    {
		upd765_get_next_id(device, &id);

		/* tested on Amstrad CPC - All bytes must match, otherwise
        a NO DATA error is reported */
		if (id.R == fdc->upd765_command_bytes[4])
		{
			if (id.C == fdc->upd765_command_bytes[2])
			{
				if (id.H == fdc->upd765_command_bytes[3])
				{
					if (id.N == fdc->upd765_command_bytes[5])
					{
						/* end of cylinder is set if:
                        1. sector data is read completely (i.e. no other errors occur like
                        no data.
                        2. sector being read is same specified by EOT
                        3. terminal count is not received */
						if (fdc->upd765_command_bytes[4]==fdc->upd765_command_bytes[6])
						{
							/* set end of cylinder */
							fdc->upd765_status[1] |= UPD765_ST1_END_OF_CYLINDER;
						}

						return TRUE;
					}
				}
			}
			else
			{
				/* the specified sector ID was found, however, the C value specified
                in the read/write command did not match the C value read from the disc */

				/* no data - checked on Amstrad CPC */
				fdc->upd765_status[1] |= UPD765_ST1_NO_DATA;
				/* bad C value */
				fdc->upd765_status[2] |= UPD765_ST2_WRONG_CYLINDER;

				if (id.C == 0x0ff)
				{
					/* the C value is 0x0ff which indicates a bad track in the IBM soft-sectored
                    format */
					fdc->upd765_status[2] |= UPD765_ST2_BAD_CYLINDER;
				}

				return FALSE;
			}
		}

		 /* index set? */
		if (floppy_drive_get_flag_state(img, FLOPPY_DRIVE_INDEX))
		{
			index_count++;
		}

	}
	while (index_count!=2);

	if (fdc->upd765_command_bytes[4] != fdc->upd765_command_bytes[6])
	{
		/* no data - specified sector ID was not found */
		fdc->upd765_status[1] |= UPD765_ST1_NO_DATA;
	}

	return 0;
}

static void upd765_read_complete(device_t *device)
{
	upd765_t *fdc = get_safe_token(device);
/* causes problems!!! - need to fix */
#ifdef NO_END_OF_CYLINDER
	/* set end of cylinder */
	fdc->upd765_status[1] &= ~UPD765_ST1_END_OF_CYLINDER;
#else
	/* completed read command */

	/* end of cylinder is set when:
        - a whole sector has been read
        - terminal count input is not set
        - AND the the sector specified by EOT was read
        */

	/* if end of cylinder is set, and we did receive a terminal count, then clear it */
	if ((fdc->upd765_flags & UPD765_TC)!=0)
	{
		/* set end of cylinder */
		fdc->upd765_status[1] &= ~UPD765_ST1_END_OF_CYLINDER;
	}
#endif

	upd765_setup_st0(device);

	fdc->upd765_result_bytes[0] = fdc->upd765_status[0];
	fdc->upd765_result_bytes[1] = fdc->upd765_status[1];
	fdc->upd765_result_bytes[2] = fdc->upd765_status[2];
	fdc->upd765_result_bytes[3] = fdc->upd765_command_bytes[2]; /* C */
	fdc->upd765_result_bytes[4] = fdc->upd765_command_bytes[3]; /* H */
	fdc->upd765_result_bytes[5] = fdc->upd765_command_bytes[4]; /* R */
	fdc->upd765_result_bytes[6] = fdc->upd765_command_bytes[5]; /* N */

	upd765_setup_result_phase(device,7);
}

static void upd765_read_data(device_t *device)
{
	upd765_t *fdc = get_safe_token(device);
	device_t *img = current_image(device);

	if (!upd765_get_rdy(device))
	{
		fdc->upd765_status[0] = 0x0c0 | (1<<4) | fdc->drive | (fdc->side<<2);
		fdc->upd765_status[1] = 0x00;
		fdc->upd765_status[2] = 0x00;

		fdc->upd765_result_bytes[0] = fdc->upd765_status[0];
		fdc->upd765_result_bytes[1] = fdc->upd765_status[1];
		fdc->upd765_result_bytes[2] = fdc->upd765_status[2];
		fdc->upd765_result_bytes[3] = fdc->upd765_command_bytes[2]; /* C */
		fdc->upd765_result_bytes[4] = fdc->upd765_command_bytes[3]; /* H */
		fdc->upd765_result_bytes[5] = fdc->upd765_command_bytes[4]; /* R */
		fdc->upd765_result_bytes[6] = fdc->upd765_command_bytes[5]; /* N */
		upd765_setup_result_phase(device,7);
		return;
	}

	if (LOG_VERBOSE)
		logerror("sector c: %02x h: %02x r: %02x n: %02x\n",fdc->upd765_command_bytes[2], fdc->upd765_command_bytes[3],fdc->upd765_command_bytes[4], fdc->upd765_command_bytes[5]);

	/* find a sector to read data from */
	{
		int found_sector_to_read;

		found_sector_to_read = 0;
		/* check for finished reading sectors */
		do
		{
			/* get matching sector */
			if (upd765_get_matching_sector(device))
			{

				/* skip it? */
				if (upd765_read_skip_sector(device))
				{
					/* yes */

					/* check that we haven't finished reading all sectors */
					if (upd765_sector_count_complete(device))
					{
						/* read complete */
						upd765_read_complete(device);
						return;
					}

					/* read not finished */

					/* increment sector count */
					upd765_increment_sector(device);
				}
				else
				{
					/* found a sector to read */
					found_sector_to_read = 1;
				}
			}
			else
			{
				/* error in finding sector */
				upd765_read_complete(device);
				return;
			}
		}
		while (found_sector_to_read==0);
	}

	{
		int data_size;

		data_size = upd765_n_to_bytes(fdc->upd765_command_bytes[5]);

		floppy_drive_read_sector_data(img, fdc->side, fdc->sector_id,fdc->data_buffer,data_size);

        upd765_setup_execution_phase_read(device,fdc->data_buffer, data_size);
	}
}


static void upd765_format_track(device_t *device)
{
	upd765_t *fdc = get_safe_token(device);
	device_t *img = current_image(device);

	/* write protected? */
	if (floppy_wpt_r(img) == CLEAR_LINE)
	{
		fdc->upd765_status[1] |= UPD765_ST1_NOT_WRITEABLE;

		upd765_setup_st0(device);
		/* TODO: Check result is correct */
			fdc->upd765_result_bytes[0] = fdc->upd765_status[0];
            fdc->upd765_result_bytes[1] = fdc->upd765_status[1];
            fdc->upd765_result_bytes[2] = fdc->upd765_status[2];
			fdc->upd765_result_bytes[3] = fdc->format_data[0];
			fdc->upd765_result_bytes[4] = fdc->format_data[1];
			fdc->upd765_result_bytes[5] = fdc->format_data[2];
			fdc->upd765_result_bytes[6] = fdc->format_data[3];
			upd765_setup_result_phase(device,7);

		return;
	}

    upd765_setup_execution_phase_write(device, &fdc->format_data[0], 4);
}

static void upd765_read_a_track(device_t *device)
{
	upd765_t *fdc = get_safe_token(device);
	int data_size;

	/* SKIP not allowed with this command! */

	/* get next id */
	chrn_id id;

	upd765_get_next_id(device, &id);

	/* TO BE CONFIRMED! */
	/* check id from disc */
	if (id.C==fdc->upd765_command_bytes[2])
	{
		if (id.H==fdc->upd765_command_bytes[3])
		{
			if (id.R==fdc->upd765_command_bytes[4])
			{
				if (id.N==fdc->upd765_command_bytes[5])
				{
					/* if ID found, then no data is not set */
					/* otherwise no data will remain set */
					fdc->upd765_status[1] &=~UPD765_ST1_NO_DATA;
				}
			}
		}
	}


	data_size = upd765_n_to_bytes(id.N);

	floppy_drive_read_sector_data(current_image(device), fdc->side, fdc->sector_id,fdc->data_buffer,data_size);

	upd765_setup_execution_phase_read(device,fdc->data_buffer, data_size);
}

static int upd765_just_read_last_sector_on_track(device_t *device)
{
	if (floppy_drive_get_flag_state(current_image(device), FLOPPY_DRIVE_INDEX))
		return 1;
	return 0;
}

static void upd765_write_complete(device_t *device)
{
	upd765_t *fdc = get_safe_token(device);

/* causes problems!!! - need to fix */
#ifdef NO_END_OF_CYLINDER
        /* set end of cylinder */
        fdc->upd765_status[1] &= ~UPD765_ST1_END_OF_CYLINDER;
#else
	/* completed read command */

	/* end of cylinder is set when:
     - a whole sector has been read
     - terminal count input is not set
     - AND the the sector specified by EOT was read
     */

	/* if end of cylinder is set, and we did receive a terminal count, then clear it */
	if ((fdc->upd765_flags & UPD765_TC)!=0)
	{
		/* set end of cylinder */
		fdc->upd765_status[1] &= ~UPD765_ST1_END_OF_CYLINDER;
	}
#endif

	upd765_setup_st0(device);

    fdc->upd765_result_bytes[0] = fdc->upd765_status[0];
    fdc->upd765_result_bytes[1] = fdc->upd765_status[1];
    fdc->upd765_result_bytes[2] = fdc->upd765_status[2];
    fdc->upd765_result_bytes[3] = fdc->upd765_command_bytes[2]; /* C */
    fdc->upd765_result_bytes[4] = fdc->upd765_command_bytes[3]; /* H */
    fdc->upd765_result_bytes[5] = fdc->upd765_command_bytes[4]; /* R */
    fdc->upd765_result_bytes[6] = fdc->upd765_command_bytes[5]; /* N */

    upd765_setup_result_phase(device,7);
}


static void upd765_write_data(device_t *device)
{
	upd765_t *fdc = get_safe_token(device);
	if (!upd765_get_rdy(device))
	{
		fdc->upd765_status[0] = 0x0c0 | (1<<4) | fdc->drive | (fdc->side<<2);
        fdc->upd765_status[1] = 0x00;
        fdc->upd765_status[2] = 0x00;

        fdc->upd765_result_bytes[0] = fdc->upd765_status[0];
        fdc->upd765_result_bytes[1] = fdc->upd765_status[1];
        fdc->upd765_result_bytes[2] = fdc->upd765_status[2];
        fdc->upd765_result_bytes[3] = fdc->upd765_command_bytes[2]; /* C */
        fdc->upd765_result_bytes[4] = fdc->upd765_command_bytes[3]; /* H */
        fdc->upd765_result_bytes[5] = fdc->upd765_command_bytes[4]; /* R */
        fdc->upd765_result_bytes[6] = fdc->upd765_command_bytes[5]; /* N */
		upd765_setup_result_phase(device,7);
		return;
	}

	/* write protected? */
	if (floppy_wpt_r(current_image(device)) == CLEAR_LINE)
	{
		fdc->upd765_status[1] |= UPD765_ST1_NOT_WRITEABLE;

		upd765_write_complete(device);
		return;
	}

	if (upd765_get_matching_sector(device))
	{
		int data_size;

		data_size = upd765_n_to_bytes(fdc->upd765_command_bytes[5]);

        upd765_setup_execution_phase_write(device,fdc->data_buffer, data_size);
	}
    else
    {
        upd765_setup_result_phase(device,7);
    }
}


/* return true if we have read all sectors, false if not */
static int upd765_sector_count_complete(device_t *device)
{
	upd765_t *fdc = get_safe_token(device);
/* this is not correct?? */
#if 1
	/* if terminal count has been set - yes */
	if (fdc->upd765_flags & UPD765_TC)
	{
		/* completed */
		return 1;
	}



	/* multi-track? */
	if (fdc->upd765_command_bytes[0] & UPD765_MT)
	{
		/* it appears that in multi-track mode,
        the EOT parameter of the command is ignored!? -
        or is it ignored the first time and not the next, so that
        if it is started on side 0, it will end at EOT on side 1,
        but if started on side 1 it will end at end of track????

        PC driver requires this to end at last sector on side 1, and
        ignore EOT parameter.

        To be checked!!!!
        */

		/* if just read last sector and on side 1 - finish */
		if (upd765_just_read_last_sector_on_track(device))
		{
			if (floppy_get_heads_per_disk(flopimg_get_image(current_image(device)))==1) {
				return 2;
			} else {
				if (fdc->side==1)
					return 2;
				return 1; // do not advance to next cylinder
			}
		}

		/* if not on second side then we haven't finished yet */
		if (fdc->side!=1)
		{
			/* haven't finished yet */
			return 0;
		}
	}
	else
	{
		/* sector id == EOT? */
		if (fdc->upd765_command_bytes[4]==fdc->upd765_command_bytes[6])
		{

			/* completed */
			return 2;
		}
	}
#else

	/* if terminal count has been set - yes */
	if (fdc->upd765_flags & UPD765_TC)
	{
		/* completed */
		return 1;
	}

	/* Multi-Track operation:

    Verified on Amstrad CPC.

        disc format used:
            9 sectors per track
            2 sides
            Sector IDs: &01, &02, &03, &04, &05, &06, &07, &08, &09

        Command specified:
            SIDE = 0,
            C = 0,H = 0,R = 1, N = 2, EOT = 1
        Sectors read:
            Sector 1 side 0
            Sector 1 side 1

        Command specified:
            SIDE = 0,
            C = 0,H = 0,R = 1, N = 2, EOT = 3
        Sectors read:
            Sector 1 side 0
            Sector 2 side 0
            Sector 3 side 0
            Sector 1 side 1
            Sector 2 side 1
            Sector 3 side 1


        Command specified:
            SIDE = 0,
            C = 0, H = 0, R = 7, N = 2, EOT = 3
        Sectors read:
            Sector 7 side 0
            Sector 8 side 0
            Sector 9 side 0
            Sector 10 not found. Error "No Data"

        Command specified:
            SIDE = 1,
            C = 0, H = 1, R = 1, N = 2, EOT = 1
        Sectors read:
            Sector 1 side 1

        Command specified:
            SIDE = 1,
            C = 0, H = 1, R = 1, N = 2, EOT = 2
        Sectors read:
            Sector 1 side 1
            Sector 1 side 2

  */

	/* sector id == EOT? */
	if ((fdc->upd765_command_bytes[4]==fdc->upd765_command_bytes[6]))
	{
		/* multi-track? */
		if (fdc->upd765_command_bytes[0] & 0x080)
		{
			/* if we have reached EOT (fdc->upd765_command_bytes[6])
            on side 1, then read is complete */
			if (fdc->side==1)
				return 1;

			return 0;

		}

		/* completed */
		return 1;
	}
#endif
	/* not complete */
	return 0;
}

static void	upd765_increment_sector(device_t *device)
{
	upd765_t *fdc = get_safe_token(device);

	/* multi-track? */
	if (fdc->upd765_command_bytes[0] & UPD765_MT)
	{
		/* reached EOT? */
		if (fdc->upd765_command_bytes[4] == fdc->upd765_command_bytes[6])
		{
			if (fdc->side == 1)
			{
				fdc->upd765_command_bytes[2]++;
			}

			fdc->upd765_command_bytes[3] ^= 0x01;
			fdc->upd765_command_bytes[4] = 1;
			fdc->side = fdc->upd765_command_bytes[3] & 0x01;
		}
		else
		{
			fdc->upd765_command_bytes[4]++;
		}
	}
	else
	{
		if (fdc->upd765_command_bytes[4] == fdc->upd765_command_bytes[6])
		{
			fdc->upd765_command_bytes[2]++;
			fdc->upd765_command_bytes[4] = 1;
		}
		else
		{
			fdc->upd765_command_bytes[4]++;
		}
	}
}

/* control mark handling code */

/* if SK==0, and we are executing a read data command, and a deleted data sector is found,
the data is not skipped. The data is read, but the control mark is set and the read is stopped */
/* if SK==0, and we are executing a read deleted data command, and a data sector is found,
the data is not skipped. The data is read, but the control mark is set and the read is stopped */
static int upd765_read_data_stop(device_t *device)
{
	upd765_t *fdc = get_safe_token(device);
	/* skip not set? */
	if ((fdc->upd765_command_bytes[0] & (1<<5))==0)
	{
		/* read data? */
		if (fdc->command == 0x06)
		{
			/* did we just read a sector with deleted data? */
			if (fdc->data_type == UPD765_DAM_DELETED_DATA)
			{
				/* set control mark */
				fdc->upd765_status[2] |= UPD765_ST2_CONTROL_MARK;

				/* quit */
				return TRUE;
			}
		}
		/* deleted data? */
		else
		if (fdc->command == 0x0c)
		{
			/* did we just read a sector with data? */
			if (fdc->data_type == UPD765_DAM_DATA)
			{
				/* set control mark */
				fdc->upd765_status[2] |= UPD765_ST2_CONTROL_MARK;

				/* quit */
				return TRUE;
			}
		}
	}

	/* continue */
	return FALSE;
}

static TIMER_CALLBACK(upd765_continue_command)
{
	device_t *device = (device_t *)ptr;
	upd765_t *fdc = get_safe_token(device);
	if ((fdc->upd765_phase == UPD765_EXECUTION_PHASE_READ) ||
		(fdc->upd765_phase == UPD765_EXECUTION_PHASE_WRITE))
	{
		switch (fdc->command)
        {
			/* read a track */
			case 0x02:
			{
				fdc->sector_counter++;

				/* sector counter == EOT */
				if (fdc->sector_counter==fdc->upd765_command_bytes[6])
				{
					/* TODO: Add correct info here */

					fdc->upd765_status[1] |= UPD765_ST1_END_OF_CYLINDER;

					upd765_setup_st0(device);

					fdc->upd765_result_bytes[0] = fdc->upd765_status[0];
					fdc->upd765_result_bytes[1] = fdc->upd765_status[1];
					fdc->upd765_result_bytes[2] = fdc->upd765_status[2];
					fdc->upd765_result_bytes[3] = fdc->upd765_command_bytes[2]; /* C */
					fdc->upd765_result_bytes[4] = fdc->upd765_command_bytes[3]; /* H */
					fdc->upd765_result_bytes[5] = fdc->upd765_command_bytes[4]; /* R */
					fdc->upd765_result_bytes[6] = fdc->upd765_command_bytes[5]; /* N */

					upd765_setup_result_phase(device,7);
				}
				else
				{
					upd765_read_a_track(device);
				}
			}
			break;

			/* format track */
			case 0x0d:
			{
				floppy_drive_format_sector(current_image(device), fdc->side, fdc->sector_counter,
					fdc->format_data[0], fdc->format_data[1],
					fdc->format_data[2], fdc->format_data[3],
					fdc->upd765_command_bytes[5]);

				fdc->sector_counter++;

				/* sector_counter = SC */
				if (fdc->sector_counter == fdc->upd765_command_bytes[3])
				{
					/* TODO: Check result is correct */
					fdc->upd765_result_bytes[0] = fdc->upd765_status[0];
					fdc->upd765_result_bytes[1] = fdc->upd765_status[1];
					fdc->upd765_result_bytes[2] = fdc->upd765_status[2];
					fdc->upd765_result_bytes[3] = fdc->format_data[0];
					fdc->upd765_result_bytes[4] = fdc->format_data[1];
					fdc->upd765_result_bytes[5] = fdc->format_data[2];
					fdc->upd765_result_bytes[6] = fdc->format_data[3];
					upd765_setup_result_phase(device,7);
				}
				else
				{
					upd765_format_track(device);
				}
			}
			break;

			/* write data, write deleted data */
			case 0x09:
			case 0x05:
			{
				/* sector id == EOT */
				UINT8 ddam;

				ddam = 0;
				if (fdc->command == 0x09)
				{
					ddam = 1;
				}

				/* write data to disc */
				// logerror("floppy_drive_write_sector_data side=%d sector=%d tc=%x\n", fdc->side, fdc->sector_id, fdc->upd765_flags & UPD765_TC);
				floppy_drive_write_sector_data(current_image(device), fdc->side, fdc->sector_id,fdc->data_buffer,upd765_n_to_bytes(fdc->upd765_command_bytes[5]),ddam);

				/* nothing to write */
				if ((fdc->upd765_transfer_bytes_remaining==0) && (fdc->upd765_flags & UPD765_DMA_MODE))
				{
					if (fdc->upd765_flags & UPD765_TC)
					{
						upd765_write_complete(device);
						break;
					}
				}

				if (upd765_sector_count_complete(device))
				{
					upd765_increment_sector(device);
					upd765_write_complete(device);
				}
				else
				{
					upd765_increment_sector(device);
					upd765_write_data(device);
				}
			}
			break;

			/* read data, read deleted data */
			case 0x0c:
			case 0x06:
			{

				int cause;
				/* read all sectors? */

				/* sector id == EOT */
				if ((cause = upd765_sector_count_complete(device)) || upd765_read_data_stop(device))
				{
					if (cause == 2)
						upd765_increment_sector(device);  // advance to next cylinder if EOT

					upd765_read_complete(device);
				}
				else
				{
					upd765_increment_sector(device);
					upd765_read_data(device);
				}
			}
			break;

			default:
				break;
		}
	}
}


static int upd765_get_command_byte_count(device_t *device)
{
	upd765_t *fdc = get_safe_token(device);
	fdc->command = fdc->upd765_command_bytes[0] & 0x01f;

	if (fdc->version==TYPE_UPD765A)
	{
		 return upd765_cmd_size[fdc->command];
    }
	else
	{
		if (fdc->version==TYPE_UPD72065)
		{
			switch(fdc->upd765_command_bytes[0])
			{
				case 0x34: // Reset Standby
				case 0x35: // Set Standby
				case 0x36: // Software Reset
					return 1;
				default:
					return upd765_cmd_size[fdc->command];
			}
		}
		if (fdc->version==TYPE_SMC37C78)
		{
			switch (fdc->command)
			{
				/* version */
				case 0x010:
					return 1;

				/* verify */
				case 0x016:
					return 9;

				/* configure */
				case 0x013:
					return 4;

				/* dumpreg */
				case 0x0e:
					return 1;

				/* perpendicular mode */
				case 0x012:
					return 1;

				/* lock */
				case 0x014:
					return 1;

				/* seek/relative seek are together! */

				default:
					return upd765_cmd_size[fdc->command];
			}
		}
	}

	return upd765_cmd_size[fdc->command];
}





void upd765_update_state(device_t *device)
{
	upd765_t *fdc = get_safe_token(device);
	switch (fdc->upd765_phase) {
	case UPD765_RESULT_PHASE:
		/* set data reg */
		fdc->upd765_data_reg = fdc->upd765_result_bytes[fdc->upd765_transfer_bytes_count];

		if (fdc->upd765_transfer_bytes_count == 0)
		{
			/* clear int for specific commands */
			switch (fdc->command) {
			case 2:	/* read a track */
			case 5:	/* write data */
			case 6:	/* read data */
			case 9:	/* write deleted data */
			case 10:	/* read id */
			case 12:	/* read deleted data */
			case 13:	/* format at track */
			case 17:	/* scan equal */
			case 19:	/* scan low or equal */
			case 29:	/* scan high or equal */
				upd765_set_int(device, 0);
				break;

			default:
				break;
			}
		}

		if (LOG_VERBOSE)
			logerror("UPD765: RESULT: %02x\n", fdc->upd765_data_reg);

		fdc->upd765_transfer_bytes_count++;
		fdc->upd765_transfer_bytes_remaining--;

		if (fdc->upd765_transfer_bytes_remaining==0)
		{
			upd765_idle(device);
		}
		else
		{
			upd765_set_data_request(device);
		}
		break;

	case UPD765_EXECUTION_PHASE_READ:
		/* setup data register */
		fdc->upd765_data_reg = fdc->execution_phase_data[fdc->upd765_transfer_bytes_count];
		fdc->upd765_transfer_bytes_count++;
		fdc->upd765_transfer_bytes_remaining--;

		if (LOG_EXTRA)
			logerror("EXECUTION PHASE READ: %02x\n", fdc->upd765_data_reg);

		if ((fdc->upd765_transfer_bytes_remaining==0) || (fdc->upd765_flags & UPD765_TC))
		{
			fdc->command_timer->adjust(attotime::zero);
		}
		else
		{
			/* trigger int */
			upd765_setup_timed_data_request(device,1);
		}
		break;

	case UPD765_COMMAND_PHASE_FIRST_BYTE:
		fdc->FDC_main |= 0x10;                      /* set BUSY */

		if (LOG_VERBOSE)
			logerror("%s: upd765(): command=0x%02x\n", device->machine().describe_context(), fdc->upd765_data_reg);

		/* seek in progress? */
		if (fdc->upd765_flags & UPD765_SEEK_ACTIVE)
		{
			/* any command results in a invalid - I think that seek, recalibrate and
            sense interrupt status may work*/
			if (fdc->upd765_data_reg != 8)	/* Sense Interrupt Status */
				fdc->upd765_data_reg = 0;
		}

		fdc->upd765_command_bytes[0] = fdc->upd765_data_reg;

		fdc->upd765_transfer_bytes_remaining = upd765_get_command_byte_count(device);

		fdc->upd765_transfer_bytes_count = 1;
		fdc->upd765_transfer_bytes_remaining--;

		if (fdc->upd765_transfer_bytes_remaining==0)
		{
			upd765_setup_command(device);
		}
		else
		{
			/* request more data */
			upd765_set_data_request(device);
			fdc->upd765_phase = UPD765_COMMAND_PHASE_BYTES;
		}
        break;

    case UPD765_COMMAND_PHASE_BYTES:
		if (LOG_VERBOSE)
			logerror("%s: upd765(): command=0x%02x\n", device->machine().describe_context(), fdc->upd765_data_reg);

		fdc->upd765_command_bytes[fdc->upd765_transfer_bytes_count] = fdc->upd765_data_reg;
		fdc->upd765_transfer_bytes_count++;
		fdc->upd765_transfer_bytes_remaining--;

		if (fdc->upd765_transfer_bytes_remaining==0)
		{
			upd765_setup_command(device);
		}
		else
		{
			/* request more data */
			upd765_set_data_request(device);
		}
		break;

    case UPD765_EXECUTION_PHASE_WRITE:
		fdc->execution_phase_data[fdc->upd765_transfer_bytes_count]=fdc->upd765_data_reg;
		fdc->upd765_transfer_bytes_count++;
		fdc->upd765_transfer_bytes_remaining--;

		if ((fdc->upd765_transfer_bytes_remaining == 0) || (fdc->upd765_flags & UPD765_TC))
		{
			fdc->command_timer->adjust(attotime::zero);
		}
		else
		{
			upd765_setup_timed_data_request(device,1);
		}
		break;
	}
}


READ8_DEVICE_HANDLER(upd765_data_r)
{
	upd765_t *fdc = get_safe_token(device);

//  if ((fdc->FDC_main & 0x0c0) == 0x0c0)
	if ((fdc->FDC_main & 0x080) == 0x080)
	{
		if (
			(fdc->upd765_phase == UPD765_EXECUTION_PHASE_READ) ||
			(fdc->upd765_phase == UPD765_EXECUTION_PHASE_WRITE))
		{

			/* reading the data byte clears the interrupt */
			upd765_set_int(device,CLEAR_LINE);
		}

		/* reset data request */
		upd765_clear_data_request(device);

		/* update state */
		upd765_update_state(device);
	}

	if (LOG_EXTRA)
		logerror("DATA R: %02x\n", fdc->upd765_data_reg);

	return fdc->upd765_data_reg;
}

WRITE8_DEVICE_HANDLER(upd765_data_w)
{
	upd765_t *fdc = get_safe_token(device);

	if (LOG_EXTRA)
		logerror("DATA W: %02x\n", data);

	/* write data to data reg */
	fdc->upd765_data_reg = data;

	if ((fdc->FDC_main & 0x0c0)==0x080)
	{
		if (
			(fdc->upd765_phase == UPD765_EXECUTION_PHASE_READ) ||
			(fdc->upd765_phase == UPD765_EXECUTION_PHASE_WRITE))
		{

			/* reading the data byte clears the interrupt */
			upd765_set_int(device, CLEAR_LINE);
		}

		/* reset data request */
		upd765_clear_data_request(device);

		/* update state */
		upd765_update_state(device);
	}
}

static void upd765_setup_invalid(device_t *device)
{
	upd765_t *fdc = get_safe_token(device);

	fdc->command = 0;
	fdc->upd765_result_bytes[0] = 0x080;
	upd765_setup_result_phase(device,1);
}

static void upd765_setup_command(device_t *device)
{
	upd765_t *fdc = get_safe_token(device);

	static const char *const commands[] =
	{
		NULL,						/* [00] */
		NULL,						/* [01] */
		"Read Track",				/* [02] */
		"Specify",					/* [03] */
		"Sense Drive Status",		/* [04] */
		"Write Data",				/* [05] */
		"Read Data",				/* [06] */
		"Recalibrate",				/* [07] */
		"Sense Interrupt Status",	/* [08] */
		"Write Deleted Data",		/* [09] */
		"Read ID",					/* [0A] */
		NULL,						/* [0B] */
		"Read Deleted Data",		/* [0C] */
		"Format Track",				/* [0D] */
		"Dump Registers",			/* [0E] */
		"Seek",						/* [0F] */
		"Version",					/* [10] */
		NULL,						/* [11] */
		"Perpendicular Mode",		/* [12] */
		"Configure",				/* [13] */
		"Lock"						/* [14] */
	};

	device_t *img;
	const char *cmd = NULL;
	chrn_id id;

	/* if not in dma mode set execution phase bit */
	if (!(fdc->upd765_flags & UPD765_DMA_MODE))
	{
        fdc->FDC_main |= 0x020;              /* execution phase */
	}

	if (LOG_COMMAND)
	{
		if ((fdc->upd765_command_bytes[0] & 0x1f) < ARRAY_LENGTH(commands))
			cmd = commands[fdc->upd765_command_bytes[0] & 0x1f];
		logerror("upd765_setup_command(): Setting up command 0x%02X (%s)\n",
			fdc->upd765_command_bytes[0] & 0x1f, cmd ? cmd : "???");
	}

	switch (fdc->upd765_command_bytes[0] & 0x1f)
	{
		case 0x03:      /* specify */
			/* setup step rate */
			fdc->srt_in_ms = 16-((fdc->upd765_command_bytes[1]>>4) & 0x0f);

			fdc->upd765_flags &= ~UPD765_DMA_MODE;

			if ((fdc->upd765_command_bytes[2] & 0x01)==0)
			{
				fdc->upd765_flags |= UPD765_DMA_MODE;
			}

			upd765_idle(device);
			break;

		case 0x04:  /* sense drive status */
			upd765_setup_drive_and_side(device);
			img = current_image(device);

			fdc->upd765_status[3] = fdc->drive | (fdc->side<<2);

			if (img)
			{
				fdc->upd765_status[3] |= !floppy_tk00_r(img) << 4;
				fdc->upd765_status[3] |= !floppy_wpt_r(img) << 6;

				if (upd765_get_rdy(device))
				{
					fdc->upd765_status[3] |= 0x20;
				}
			}

			fdc->upd765_status[3] |= 0x08;

			/* two side and fault not set but should be? */
			fdc->upd765_result_bytes[0] = fdc->upd765_status[3];
			upd765_setup_result_phase(device,1);
			break;

		case 0x07:          /* recalibrate */
			upd765_seek_setup(device, 1);
			break;

		case 0x0f:          /* seek */
			upd765_seek_setup(device, 0);
			break;

		case 0x0a:      /* read id */
			upd765_setup_drive_and_side(device);
			img = current_image(device);

			fdc->upd765_status[0] = fdc->drive | (fdc->side<<2);
			fdc->upd765_status[1] = 0;
			fdc->upd765_status[2] = 0;

			/* drive ready? */
			if (upd765_get_rdy(device))
			{
				/* is disk inserted? */
				device_image_interface *image = dynamic_cast<device_image_interface *>( img);
				if (image!=NULL && image->exists())
				{
					int index_count = 0;

					/* floppy drive is ready and disc is inserted */

					/* this is the id that appears when a disc is not formatted */
					/* to be checked on Amstrad */
					id.C = 0;
					id.H = 0;
					id.R = 0x01;
					id.N = 0x02;

					/* repeat for two index counts before quitting */
					do
					{
						/* get next id from disc */
						if (floppy_drive_get_next_id(img, fdc->side, &id))
						{
							/* got an id */
							/* if bad media keep going until failure */
							if (!(fdc->upd765_flags & UPD765_BAD_MEDIA)) break;
						}

						if (floppy_drive_get_flag_state(img, FLOPPY_DRIVE_INDEX))
						{
							/* update index count */
							index_count++;
						}
					}
					while (index_count!=2);

					if (fdc->upd765_flags & UPD765_BAD_MEDIA)
					{
							fdc->upd765_status[0] |= 0x40;
							fdc->upd765_status[1] |= 1;
					}

					/* at this point, we have seen a id or two index pulses have occurred! */
					fdc->upd765_result_bytes[0] = fdc->upd765_status[0];
					fdc->upd765_result_bytes[1] = fdc->upd765_status[1];
					fdc->upd765_result_bytes[2] = fdc->upd765_status[2];
					fdc->upd765_result_bytes[3] = id.C; /* C */
					fdc->upd765_result_bytes[4] = id.H; /* H */
					fdc->upd765_result_bytes[5] = id.R; /* R */
					fdc->upd765_result_bytes[6] = id.N; /* N */

					upd765_setup_result_phase(device,7);
				}
				else
				{
					/* floppy drive is ready, but no disc is inserted */
					/* this occurs on the PC */
					/* in this case, the command never quits! */
					/* there are no index pulses to stop the command! */
				}
			}
			else
			{
				/* what are id values when drive not ready? */

				/* not ready, abnormal termination */
				fdc->upd765_status[0] |= (1<<3) | (1<<6);
				fdc->upd765_result_bytes[0] = fdc->upd765_status[0];
				fdc->upd765_result_bytes[1] = fdc->upd765_status[1];
				fdc->upd765_result_bytes[2] = fdc->upd765_status[2];
				fdc->upd765_result_bytes[3] = 0; /* C */
				fdc->upd765_result_bytes[4] = 0; /* H */
				fdc->upd765_result_bytes[5] = 0; /* R */
				fdc->upd765_result_bytes[6] = 0; /* N */
			}
			break;


		case 0x08: /* sense interrupt status */
			/* interrupt pending? */
			if (fdc->upd765_flags & UPD765_INT)
			{
				/* clear ready changed bit */
				fdc->ready_changed &= ~(1 << fdc->drive);

				if (!fdc->pool) {
					fdc->ready_changed = 0;
				}

				/* clear drive seek bits */
				fdc->FDC_main &= ~(1 | 2 | 4 | 8);

				/* return status */
				fdc->upd765_result_bytes[0] = fdc->upd765_status[0];
				/* return pcn */
				fdc->upd765_result_bytes[1] = fdc->pcn[fdc->drive];

				/* return result */
				upd765_setup_result_phase(device,2);

				if (fdc->ready_changed)
				{
					fdc->drive++;
					fdc->upd765_status[0] = 0xc0 | fdc->drive;
				}
				else
				{
					/* Clear int */
					upd765_set_int(device, CLEAR_LINE);
				}
			}
			else
			{
				if(fdc->version == TYPE_UPD72065 && (fdc->FDC_main & 0x0f) == 0x00)
				{  // based on XM6
					upd765_setup_invalid(device);
				}
				else
				{
					/* no int */
					fdc->upd765_result_bytes[0] = 0x80;
					/* return pcn */
					fdc->upd765_result_bytes[1] = fdc->pcn[fdc->drive];

					/* return result */
					upd765_setup_result_phase(device,2);
				}
			}
			break;

		case 0x06:  /* read data */
			upd765_setup_drive_and_side(device);

			fdc->upd765_status[0] = fdc->drive | (fdc->side<<2);
			fdc->upd765_status[1] = 0;
			fdc->upd765_status[2] = 0;

			upd765_read_data(device);
			break;

		case 0x0c:
			/* read deleted data */
			upd765_setup_drive_and_side(device);

			fdc->upd765_status[0] = fdc->drive | (fdc->side<<2);
			fdc->upd765_status[1] = 0;
			fdc->upd765_status[2] = 0;

			/* .. for now */
			upd765_read_data(device);
			break;

		case 0x09:
			/* write deleted data */
			upd765_setup_drive_and_side(device);

			fdc->upd765_status[0] = fdc->drive | (fdc->side<<2);
			fdc->upd765_status[1] = 0;
			fdc->upd765_status[2] = 0;

			/* ... for now */
			upd765_write_data(device);
			break;

		case 0x02:
			/* read a track */
			upd765_setup_drive_and_side(device);
			img = current_image(device);
			fdc->upd765_status[0] = fdc->drive | (fdc->side<<2);
			fdc->upd765_status[1] = 0;
			fdc->upd765_status[2] = 0;

			fdc->upd765_status[0] |= UPD765_ST1_NO_DATA;

			/* wait for index */
			do
			{
				/* get next id from disc */
				floppy_drive_get_next_id(img, fdc->side,&id);
			}
			while ((floppy_drive_get_flag_state(img, FLOPPY_DRIVE_INDEX))==0);

			fdc->sector_counter = 0;

			upd765_read_a_track(device);
			break;

		case 0x05:  /* write data */
			upd765_setup_drive_and_side(device);

			fdc->upd765_status[0] = fdc->drive | (fdc->side<<2);
			fdc->upd765_status[1] = 0;
			fdc->upd765_status[2] = 0;

			upd765_write_data(device);
			break;

		case 0x0d:	/* format a track */
			upd765_setup_drive_and_side(device);

			fdc->upd765_status[0] = fdc->drive | (fdc->side<<2);
			fdc->upd765_status[1] = 0;
			fdc->upd765_status[2] = 0;

			fdc->sector_counter = 0;

			upd765_format_track(device);
			break;

		default:	/* invalid */
			switch (fdc->version)
			{
				case TYPE_UPD765A:
					upd765_setup_invalid(device);
					break;

				case TYPE_UPD765B:
					/* from upd765b data sheet */
					if ((fdc->upd765_command_bytes[0] & 0x01f)==0x010)
					{
						/* version */
						fdc->upd765_status[0] = 0x090;
						fdc->upd765_result_bytes[0] = fdc->upd765_status[0];
						upd765_setup_result_phase(device,1);
					}
					break;

				case TYPE_UPD72065:
					switch(fdc->upd765_command_bytes[0] & 0x3f)
					{
						case 0x36:  // Software Reset
							upd765_reset(device,0);
							upd765_idle(device);
							if(LOG_COMMAND)
								logerror("upd72065: command - Software Reset\n");
							break;
						default:
							upd765_setup_invalid(device);
							break;
					}
					break;

				case TYPE_SMC37C78:
					/* TO BE COMPLETED!!! !*/
					switch (fdc->upd765_command_bytes[0] & 0x1f)
					{
						case 0x10:		/* version */
							fdc->upd765_status[0] = 0x90;
							fdc->upd765_result_bytes[0] = fdc->upd765_status[0];
							upd765_setup_result_phase(device,1);
							break;

						case 0x13:		/* configure */
							fdc->pool = fdc->upd765_command_bytes[1] & 0x10;
							upd765_idle(device);
							break;

						case 0x0e:		/* dump reg */
							fdc->upd765_result_bytes[0] = fdc->pcn[0];
							fdc->upd765_result_bytes[1] = fdc->pcn[1];
							fdc->upd765_result_bytes[2] = fdc->pcn[2];
							fdc->upd765_result_bytes[3] = fdc->pcn[3];

							upd765_setup_result_phase(device,10);
							break;

						case 0x12:		/* perpendicular mode */
							upd765_idle(device);
							break;

						case 0x14:		/* lock */
							upd765_setup_result_phase(device,1);
							break;
					}
					break;
			}
	}
}


/* dma acknowledge write */
WRITE8_DEVICE_HANDLER(upd765_dack_w)
{
	/* clear request */
	upd765_set_dma_drq(device, CLEAR_LINE);

	/* write data */
	upd765_data_w(device, space, offset, data);
}

READ8_DEVICE_HANDLER(upd765_dack_r)
{
	/* clear data request */
	upd765_set_dma_drq(device,CLEAR_LINE);

	/* read data */
	return upd765_data_r(device, space, offset);
}

static TIMER_CALLBACK( interrupt_callback )
{
	device_t* device = (device_t*)ptr;
	upd765_set_int(device, 1);
}

void upd765_set_bad(device_t *device, int state)
{
	upd765_t *fdc = get_safe_token(device);
	if (state) fdc->upd765_flags |= UPD765_BAD_MEDIA;
	else fdc->upd765_flags &= ~UPD765_BAD_MEDIA;
}

void upd765_reset(device_t *device, int offset)
{
	upd765_t *fdc = get_safe_token(device);

	/* upd765 in idle state - ready to accept commands */
	upd765_idle(device);

	/* set int low */
	upd765_set_int(device, 0);
	/* set dma drq output */
	upd765_set_dma_drq(device,0);

	/* tandy 100hx assumes that after NEC is reset, it is in DMA mode */
	fdc->upd765_flags |= UPD765_DMA_MODE;

	/* if ready input is set during reset generate an int */
	if (upd765_get_rdy(device))
	{
		int i;
		int a_drive_is_ready;

		fdc->upd765_status[0] = 0x080 | 0x040;

		// HACK signal ready changed for all drives
		fdc->ready_changed = 0x0f;
		fdc->drive = 0;

		/* for the purpose of pc-xt. If any of the drives have a disk inserted,
        do not set not-ready - need to check with pc_fdc.c whether all drives
        are checked or only the drive selected with the drive select bits?? */

		a_drive_is_ready = 0;
		for (i = 0; i < 4; i++)
		{
			if (fdc->intf->floppy_drive_tags[i]!=NULL)
			{
				device_t *img;

				if (device->owner() != NULL)
					img = device->owner()->subdevice(fdc->intf->floppy_drive_tags[i]);
				else
					img = device->machine().device(fdc->intf->floppy_drive_tags[i]);

				device_image_interface *image = dynamic_cast<device_image_interface *>( img);
				if (image->exists())
				{
					a_drive_is_ready = 1;
					break;
				}
			}

		}

		if (!a_drive_is_ready && fdc->intf->rdy_pin == UPD765_RDY_PIN_CONNECTED )
		{
			fdc->upd765_status[0] |= 0x08;
		}

		device->machine().scheduler().timer_set(attotime::from_usec(5), FUNC(interrupt_callback),0,device);
	}
}

WRITE_LINE_DEVICE_HANDLER( upd765_reset_w )
{
	upd765_t *fdc = get_safe_token(device);

	int flags;

	/* get previous reset state */
	flags = fdc->upd765_flags;

	/* set new reset state */
	/* clear reset */
	fdc->upd765_flags &= ~UPD765_RESET;

	/* reset */
	if (state)
	{
		fdc->upd765_flags |= UPD765_RESET;

		upd765_set_int(device, 0);
	}

	/* reset changed state? */
	if (((flags^fdc->upd765_flags) & UPD765_RESET)!=0)
	{
		/* yes */

		/* no longer reset */
		if ((fdc->upd765_flags & UPD765_RESET)==0)
		{
			/* reset nec */
			upd765_reset(device, 0);
		}
	}
}


WRITE_LINE_DEVICE_HANDLER( upd765_ready_w )
{
	upd765_t *fdc = get_safe_token(device);

	/* clear ready state */
	fdc->upd765_flags &= ~UPD765_FDD_READY;

	if (state)
	{
		fdc->upd765_flags |= UPD765_FDD_READY;
	}
}


/* Device Interface */

static void common_start(device_t *device, int device_type)
{
	upd765_t *fdc = get_safe_token(device);
	// validate arguments

	assert(device != NULL);
	assert(device->tag() != NULL);
	assert(device->static_config() != NULL);

	fdc->intf = (const upd765_interface*)device->static_config();

	fdc->version = (UPD765_VERSION)device_type;
	fdc->timer = device->machine().scheduler().timer_alloc(FUNC(upd765_timer_callback), (void*)device);
	fdc->seek_timer = device->machine().scheduler().timer_alloc(FUNC(upd765_seek_timer_callback), (void*)device);
	fdc->command_timer = device->machine().scheduler().timer_alloc(FUNC(upd765_continue_command), (void*)device);

	fdc->upd765_flags &= UPD765_FDD_READY;
	fdc->data_buffer = auto_alloc_array(device->machine(), char, 32*1024);

	fdc->out_int_func.resolve(fdc->intf->out_int_func, *device);
	fdc->out_drq_func.resolve(fdc->intf->out_drq_func, *device);

	// plain upd765 is doing pooling
	fdc->pool = true;
	// register for state saving
	//state_save_register_item(device->machine(), "upd765", device->tag(), 0, upd765->number);
}

static DEVICE_START( upd765a )
{
	common_start(device, TYPE_UPD765A);
}

static DEVICE_START( upd765b )
{
	common_start(device, TYPE_UPD765B);
}

static DEVICE_START( smc37c78 )
{
	common_start(device, TYPE_SMC37C78);
	// specified in documentation that by default is off
	upd765_t *fdc = get_safe_token(device);
	fdc->pool = false;
}

static DEVICE_START( upd72065 )
{
	common_start(device, TYPE_UPD72065);
}

static DEVICE_RESET( upd765 )
{
	int i;
	upd765_t *fdc = get_safe_token(device);
	for (i = 0; i < 4; i++) {
		if (fdc->intf->floppy_drive_tags[i]!=NULL)
		{
			device_t *img;

			if (device->owner() != NULL)
				img = device->owner()->subdevice(fdc->intf->floppy_drive_tags[i]);
			else
				img = device->machine().device(fdc->intf->floppy_drive_tags[i]);

			floppy_drive_set_controller(img, device);
			floppy_drive_set_ready_state_change_callback(img, upd765_set_ready_change_callback);
		}
	}

	upd765_reset(device,0);
}

const device_type UPD765A = &device_creator<upd765a_device>;

upd765a_device::upd765a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, UPD765A, "UPD765A", tag, owner, clock)
{
	m_token = global_alloc_clear(upd765_t);
}
upd765a_device::upd765a_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, type, name, tag, owner, clock)
{
	m_token = global_alloc_clear(upd765_t);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void upd765a_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd765a_device::device_start()
{
	DEVICE_START_NAME( upd765a )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void upd765a_device::device_reset()
{
	DEVICE_RESET_NAME( upd765 )(this);
}


const device_type UPD765B = &device_creator<upd765b_device>;

upd765b_device::upd765b_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: upd765a_device(mconfig, UPD765B, "UPD765B", tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd765b_device::device_start()
{
	DEVICE_START_NAME( upd765b )(this);
}


const device_type SMC37C78 = &device_creator<smc37c78_device>;

smc37c78_device::smc37c78_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: upd765a_device(mconfig, SMC37C78, "SMC37C78", tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void smc37c78_device::device_start()
{
	DEVICE_START_NAME( smc37c78 )(this);
}


const device_type UPD72065 = &device_creator<upd72065_device>;

upd72065_device::upd72065_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: upd765a_device(mconfig, UPD72065, "UPD72065", tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd72065_device::device_start()
{
	DEVICE_START_NAME( upd72065 )(this);
}


