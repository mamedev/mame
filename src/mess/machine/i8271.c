/* Intel 8271 Floppy Disc Controller */
/* used in BBC Micro B,Acorn Atom */
/* Jun 2000. Kev Thacker */

/* TODO:

    - Scan commands
    - Check the commands work properly using a BBC disc copier program
    - check if 0 is specified as number of sectors, how many sectors
    is actually transfered
    - deleted data functions (error if data finds deleted data?)
*/


#include "emu.h"
#include "i8271.h"
#include "imagedev/flopdrv.h"

/* data request */
#define I8271_FLAGS_DATA_REQUEST 0x01
/* data direction. If 0x02, then it is from fdc to cpu, else
it is from cpu to fdc */
#define I8271_FLAGS_DATA_DIRECTION 0x02

struct i8271_t
{
	int flags;
	int state;
	unsigned char Command;
	unsigned char StatusRegister;
	unsigned char CommandRegister;
	unsigned char ResultRegister;
	unsigned char ParameterRegister;
	unsigned char ResetRegister;
	unsigned char data;

	/* number of parameters required after command is specified */
	unsigned long ParameterCount;
	/* number of parameters written so far */
	unsigned long ParameterCountWritten;

	unsigned char CommandParameters[8];

	/* current track for each drive */
	unsigned long   CurrentTrack[2];

	/* 2 bad tracks for drive 0, followed by 2 bad tracks for drive 1 */
	unsigned long   BadTracks[4];

	/* mode special register */
	unsigned long Mode;


	/* drive outputs */
	int drive;
	int side;

	/* drive control output special register */
	int drive_control_output;
	/* drive control input special register */
	int drive_control_input;

	unsigned long StepRate;
	unsigned long HeadSettlingTime;
	unsigned long IndexCountBeforeHeadUnload;
	unsigned long HeadLoadTime;

	/* id on disc to find */
	int ID_C;
	int ID_H;
	int ID_R;
	int ID_N;

	/* id of data for read/write */
	int data_id;

	int ExecutionPhaseTransferCount;
	char *pExecutionPhaseData;
	int ExecutionPhaseCount;

	/* sector counter and id counter */
	int Counter;

	/* ==0, to cpu, !=0 =from cpu */
	int data_direction;
	const i8271_interface *intf;

	emu_timer *data_timer;
	emu_timer *command_complete_timer;
};

enum I8271_STATE_t
{
	I8271_STATE_EXECUTION_READ = 0,
	I8271_STATE_EXECUTION_WRITE
};

/* commands accepted */
#define I8271_COMMAND_SPECIFY                                       0x035
#define I8271_COMMAND_SEEK                                          0x029
#define I8271_COMMAND_READ_DRIVE_STATUS                             0x02c
#define I8271_COMMAND_READ_SPECIAL_REGISTER                         0x03d
#define I8271_COMMAND_WRITE_SPECIAL_REGISTER                        0x03a
#define I8271_COMMAND_FORMAT                                        0x023
#define I8271_COMMAND_READ_ID                                       0x01b
#define I8271_COMMAND_READ_DATA_SINGLE_RECORD                       0x012
#define I8271_COMMAND_READ_DATA_AND_DELETED_DATA_SINGLE_RECORD      0x016
#define I8271_COMMAND_WRITE_DATA_SINGLE_RECORD                      0x00a
#define I8271_COMMAND_WRITE_DELETED_DATA_SINGLE_RECORD              0x00e
#define I8271_COMMAND_VERIFY_DATA_AND_DELETED_DATA_SINGLE_RECORD    0x01e
#define I8271_COMMAND_READ_DATA_MULTI_RECORD                        0x013
#define I8271_COMMAND_READ_DATA_AND_DELETED_DATA_MULTI_RECORD       0x017
#define I8271_COMMAND_WRITE_DATA_MULTI_RECORD                       0x00b
#define I8271_COMMAND_WRITE_DELETED_DATA_MULTI_RECORD               0x00f
#define I8271_COMMAND_VERIFY_DATA_AND_DELETED_DATA_MULTI_RECORD     0x01f
#define I8271_COMMAND_SCAN_DATA                                     0x000
#define I8271_COMMAND_SCAN_DATA_AND_DELETED_DATA                    0x004

/*
#define I8271_COMMAND_READ_OPERATION                            (1<<4)
#define I8271_COMMAND_DELETED_DATA                              (1<<2)
#define I8271_COMMAND_MULTI_RECORD                              (1<<0)
*/



/* first parameter for specify command */
#define I8271_SPECIFY_INITIALIZATION                                0x0d
#define I8271_SPECIFY_LOAD_BAD_TRACKS_SURFACE_0                     0x010
#define I8271_SPECIFY_LOAD_BAD_TRACKS_SURFACE_1                     0x018

/* first parameter for read/write special register */
#define I8271_SPECIAL_REGISTER_SCAN_SECTOR_NUMBER                   0x06
#define I8271_SPECIAL_REGISTER_SCAN_MSB_OF_COUNT                    0x014
#define I8271_SPECIAL_REGISTER_SCAN_LSB_OF_COUNT                    0x013
#define I8271_SPECIAL_REGISTER_SURFACE_0_CURRENT_TRACK              0x012
#define I8271_SPECIAL_REGISTER_SURFACE_1_CURRENT_TRACK              0x01a
#define I8271_SPECIAL_REGISTER_MODE_REGISTER                        0x017
#define I8271_SPECIAL_REGISTER_DRIVE_CONTROL_OUTPUT_PORT            0x023
#define I8271_SPECIAL_REGISTER_DRIVE_CONTROL_INPUT_PORT             0x022
#define I8271_SPECIAL_REGISTER_SURFACE_0_BAD_TRACK_1                0x010
#define I8271_SPECIAL_REGISTER_SURFACE_0_BAD_TRACK_2                0x011
#define I8271_SPECIAL_REGISTER_SURFACE_1_BAD_TRACK_1                0x018
#define I8271_SPECIAL_REGISTER_SURFACE_1_BAD_TRACK_2                0x019


/* status register bits */
#define I8271_STATUS_COMMAND_BUSY   0x080
#define I8271_STATUS_COMMAND_FULL   0x040
#define I8271_STATUS_PARAMETER_FULL 0x020
#define I8271_STATUS_RESULT_FULL    0x010
#define I8271_STATUS_INT_REQUEST    0x008
#define I8271_STATUS_NON_DMA_REQUEST    0x004

static void i8271_command_execute(device_t *device);
static void i8271_command_continue(device_t *device);
static void i8271_command_complete(device_t *device,int result, int int_rq);
static void i8271_data_request(device_t *device);
static void i8271_timed_data_request(device_t *device);
/* locate sector for read/write operation */
static int i8271_find_sector(device_t *device);
/* do a read operation */
static void i8271_do_read(device_t *device);
static void i8271_do_write(device_t *device);
static void i8271_do_read_id(device_t *device);
static void i8271_set_irq_state(device_t *device,int);
static void i8271_set_dma_drq(device_t *device);

static TIMER_CALLBACK(i8271_data_timer_callback);
static TIMER_CALLBACK(i8271_timed_command_complete_callback);

#define VERBOSE 0

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)
#define FDC_LOG(x) do { if (VERBOSE) logerror("I8271: %s\n",x); } while (0)
#define FDC_LOG_COMMAND(x) do { if (VERBOSE) logerror("I8271: COMMAND %s\n",x); } while (0)

static DEVICE_RESET( i8271 );

INLINE i8271_t *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == I8271);

	return (i8271_t *)downcast<i8271_device *>(device)->token();
}


static device_t *current_image(device_t *device)
{
	i8271_t *i8271 = get_safe_token(device);
	if (i8271->intf->floppy_drive_tags[i8271->drive]!=NULL) {
		return device->machine().device(i8271->intf->floppy_drive_tags[i8271->drive]);
	} else {
		return NULL;
	}
}


static void i8271_seek_to_track(device_t *device,int track)
{
	device_t *img = current_image(device);
	i8271_t *i8271 = get_safe_token(device);
	if (track==0)
	{
		/* seek to track 0 */
		unsigned char StepCount = 0x0ff;

		/*logerror("step\n"); */

		/* track 0 not set, not seeked more than 255 tracks */
		while (floppy_tk00_r(img) && (StepCount != 0))
		{
/*            logerror("step\n"); */
			StepCount--;
			floppy_drive_seek(img, -1);
		}

		i8271->CurrentTrack[i8271->drive] = 0;

		/* failed to find track 0? */
		if (StepCount==0)
		{
			/* Completion Type: operator intervation probably required for recovery */
			/* Completion code: track 0 not found */
			i8271->ResultRegister |= (2<<3) | 2<<1;
		}

		/* step out - towards track 0 */
		i8271->drive_control_output &=~(1<<2);
	}
	else
	{

		signed int SignedTracks;

		/* calculate number of tracks to seek */
		SignedTracks = track - i8271->CurrentTrack[i8271->drive];

		/* step towards 0 */
		i8271->drive_control_output &= ~(1<<2);

		if (SignedTracks>0)
		{
			/* step away from 0 */
			i8271->drive_control_output |= (1<<2);
		}


		/* seek to track 0 */
		floppy_drive_seek(img, SignedTracks);

		i8271->CurrentTrack[i8271->drive] = track;
	}
}


static TIMER_CALLBACK(i8271_data_timer_callback)
{
	device_t *device = (device_t *)ptr;
	i8271_t *i8271 = get_safe_token(device);

	/* ok, trigger data request now */
	i8271_data_request(device);

	/* stop it */
	i8271->data_timer->reset();
}

/* setup a timed data request - data request will be triggered in a few usecs time */
static void i8271_timed_data_request(device_t *device)
{
	int usecs;
	i8271_t *i8271 = get_safe_token(device);
	/* 64 for single density */
	usecs = 64;

	/* set timers */
	i8271->command_complete_timer->reset();
	i8271->data_timer->adjust(attotime::from_usec(usecs));
}


static TIMER_CALLBACK(i8271_timed_command_complete_callback)
{
	device_t *device = (device_t *)ptr;
	i8271_t *i8271 = get_safe_token(device);

	i8271_command_complete(device,1,1);

	/* stop it, but don't allow it to be free'd */
	i8271->command_complete_timer->reset();
}

/* setup a irq to occur 128us later - in reality this would be much later, because the int would
come after reading the two CRC bytes at least! This function is used when a irq is required at
command completion. Required for read data and write data, where last byte could be missed! */
static void i8271_timed_command_complete(device_t *device)
{
	int usecs;
	i8271_t *i8271 = get_safe_token(device);

	/* 64 for single density - 2 crc bytes later*/
	usecs = 64*2;

	/* set timers */
	i8271->data_timer->reset();
	i8271->command_complete_timer->adjust(attotime::from_usec(usecs));
}

static void i8271_set_irq_state(device_t *device,int state)
{
	i8271_t *i8271 = get_safe_token(device);
	i8271->StatusRegister &= ~I8271_STATUS_INT_REQUEST;
	if (state)
	{
		i8271->StatusRegister |= I8271_STATUS_INT_REQUEST;
	}

	if (i8271->intf->interrupt)
	{
		i8271->intf->interrupt(device, (i8271->StatusRegister & I8271_STATUS_INT_REQUEST));
	}
}

static void i8271_set_dma_drq(device_t *device)
{
	i8271_t *i8271 = get_safe_token(device);
	if (i8271->intf->dma_request)
	{
		i8271->intf->dma_request(device, (i8271->flags & I8271_FLAGS_DATA_REQUEST), (i8271->flags & I8271_FLAGS_DATA_DIRECTION));
	}
}

static void i8271_load_bad_tracks(device_t *device, int surface)
{
	i8271_t *i8271 = get_safe_token(device);
	i8271->BadTracks[(surface<<1) + 0] = i8271->CommandParameters[1];
	i8271->BadTracks[(surface<<1) + 1] = i8271->CommandParameters[2];
	i8271->CurrentTrack[surface] = i8271->CommandParameters[3];
}

static void i8271_write_bad_track(device_t *device, int surface, int track, int data)
{
	i8271_t *i8271 = get_safe_token(device);
	i8271->BadTracks[(surface<<1) + (track-1)] = data;
}

static void i8271_write_current_track(device_t *device,int surface, int track)
{
	i8271_t *i8271 = get_safe_token(device);
	i8271->CurrentTrack[surface] = track;
}

static int i8271_read_current_track(device_t *device,int surface)
{
	i8271_t *i8271 = get_safe_token(device);
	return i8271->CurrentTrack[surface];
}

static int i8271_read_bad_track(device_t *device,int surface, int track)
{
	i8271_t *i8271 = get_safe_token(device);
	return i8271->BadTracks[(surface<<1) + (track-1)];
}

static void i8271_get_drive(device_t *device)
{
	i8271_t *i8271 = get_safe_token(device);
	/* &40 = drive 0 side 0 */
	/* &80 = drive 1 side 0 */



	if (i8271->CommandRegister & (1<<6))
	{
		i8271->drive = 0;
	}

	if (i8271->CommandRegister & (1<<7))
	{
		i8271->drive = 1;
	}

}

static void i8271_check_all_parameters_written(device_t *device)
{
	i8271_t *i8271 = get_safe_token(device);
	if (i8271->ParameterCount == i8271->ParameterCountWritten)
	{
		i8271->StatusRegister &= ~I8271_STATUS_COMMAND_FULL;

		i8271_command_execute(device);
	}
}


static void i8271_update_state(device_t *device)
{
	i8271_t *i8271 = get_safe_token(device);
	switch (i8271->state)
	{
		/* fdc reading data and passing it to cpu which must read it */
		case I8271_STATE_EXECUTION_READ:
		{
	//      /* if data request has been cleared, i.e. caused by a read of the register */
	//      if ((i8271->flags & I8271_FLAGS_DATA_REQUEST)==0)
			{
				/* setup data with byte */
				i8271->data = i8271->pExecutionPhaseData[i8271->ExecutionPhaseCount];

/*              logerror("read data %02x\n", i8271->data); */

				/* update counters */
				i8271->ExecutionPhaseCount++;
				i8271->ExecutionPhaseTransferCount--;

			//  logerror("Count: %04x\n", i8271->ExecutionPhaseCount);
			//  logerror("Remaining: %04x\n", i8271->ExecutionPhaseTransferCount);

				/* completed? */
				if (i8271->ExecutionPhaseTransferCount==0)
				{
					/* yes */

			//      logerror("sector read complete!\n");
					/* continue command */
					i8271_command_continue(device);
				}
				else
				{
					/* no */

					/* issue data request */
					i8271_timed_data_request(device);
				}
			}
		}
		break;

		/* fdc reading data and passing it to cpu which must read it */
		case I8271_STATE_EXECUTION_WRITE:
		{
			/* setup data with byte */
			i8271->pExecutionPhaseData[i8271->ExecutionPhaseCount] = i8271->data;
			/* update counters */
			i8271->ExecutionPhaseCount++;
			i8271->ExecutionPhaseTransferCount--;

			/* completed? */
			if (i8271->ExecutionPhaseTransferCount==0)
			{
				/* yes */

				/* continue command */
				i8271_command_continue(device);
			}
			else
			{
				/* no */

				/* issue data request */
				i8271_timed_data_request(device);
			}
		}
		break;

		default:
			break;
	}
}

static void i8271_initialise_execution_phase_read(device_t *device,int transfer_size)
{
	i8271_t *i8271 = get_safe_token(device);
	/* read */
	i8271->flags |= I8271_FLAGS_DATA_DIRECTION;
	i8271->ExecutionPhaseCount = 0;
	i8271->ExecutionPhaseTransferCount = transfer_size;
	i8271->state = I8271_STATE_EXECUTION_READ;
}


static void i8271_initialise_execution_phase_write(device_t *device,int transfer_size)
{
	i8271_t *i8271 = get_safe_token(device);
	/* write */
	i8271->flags &= ~I8271_FLAGS_DATA_DIRECTION;
	i8271->ExecutionPhaseCount = 0;
	i8271->ExecutionPhaseTransferCount = transfer_size;
	i8271->state = I8271_STATE_EXECUTION_WRITE;
}

/* for data transfers */
static void i8271_data_request(device_t *device)
{
	i8271_t *i8271 = get_safe_token(device);
	i8271->flags |= I8271_FLAGS_DATA_REQUEST;

	if ((i8271->Mode & 0x01)!=0)
	{
		/* non-dma */
		i8271->StatusRegister |= I8271_STATUS_NON_DMA_REQUEST;
		/* set int */
		i8271_set_irq_state(device,1);
	}
	else
	{
		/* dma */
		i8271->StatusRegister &= ~I8271_STATUS_NON_DMA_REQUEST;

		i8271_set_dma_drq(device);
	}
}

static void i8271_command_complete(device_t *device,int result, int int_rq)
{
	i8271_t *i8271 = get_safe_token(device);
	/* not busy, and not a execution phase data request in non-dma mode */
	i8271->StatusRegister &= ~(I8271_STATUS_COMMAND_BUSY | I8271_STATUS_NON_DMA_REQUEST);

	if (result)
	{
		i8271->StatusRegister |= I8271_STATUS_RESULT_FULL;
	}

	if (int_rq)
	{
		/* trigger an int */
		i8271_set_irq_state(device,1);
	}

	/* correct?? */
	i8271->drive_control_output &=~1;
}


/* for data transfers */
static void i8271_clear_data_request(device_t *device)
{
	i8271_t *i8271 = get_safe_token(device);
	i8271->flags &= ~I8271_FLAGS_DATA_REQUEST;

	if ((i8271->Mode & 0x01)!=0)
	{
		/* non-dma */
		i8271->StatusRegister &= ~I8271_STATUS_NON_DMA_REQUEST;
		/* set int */
		i8271_set_irq_state(device,0);
	}
	else
	{
		/* dma */
		i8271_set_dma_drq(device);
	}
}


static void i8271_command_continue(device_t *device)
{
	i8271_t *i8271 = get_safe_token(device);
	switch (i8271->Command)
	{
		case I8271_COMMAND_READ_DATA_MULTI_RECORD:
		case I8271_COMMAND_READ_DATA_SINGLE_RECORD:
		{
			/* completed all sectors? */
			i8271->Counter--;
			/* increment sector id */
			i8271->ID_R++;

			/* end command? */
			if (i8271->Counter==0)
			{

				i8271_timed_command_complete(device);
				return;
			}

			i8271_do_read(device);
		}
		break;

		case I8271_COMMAND_WRITE_DATA_MULTI_RECORD:
		case I8271_COMMAND_WRITE_DATA_SINGLE_RECORD:
		{
			/* put the buffer to the sector */
			floppy_drive_write_sector_data(current_image(device), i8271->side, i8271->data_id, i8271->pExecutionPhaseData, 1<<(i8271->ID_N+7),0);

			/* completed all sectors? */
			i8271->Counter--;
			/* increment sector id */
			i8271->ID_R++;

			/* end command? */
			if (i8271->Counter==0)
			{

				i8271_timed_command_complete(device);
				return;
			}

			i8271_do_write(device);
		}
		break;

		case I8271_COMMAND_READ_ID:
		{
			i8271->Counter--;

			if (i8271->Counter==0)
			{
				i8271_timed_command_complete(device);
				return;
			}

			i8271_do_read_id(device);
		}
		break;

		default:
			break;
	}
}

static void i8271_do_read(device_t *device)
{
	i8271_t *i8271 = get_safe_token(device);
	/* find the sector */
	if (i8271_find_sector(device))
	{
		/* get the sector into the buffer */
		floppy_drive_read_sector_data(current_image(device), i8271->side, i8271->data_id, i8271->pExecutionPhaseData, 1<<(i8271->ID_N+7));

		/* initialise for reading */
		i8271_initialise_execution_phase_read(device, 1<<(i8271->ID_N+7));

		/* update state - gets first byte and triggers a data request */
		i8271_timed_data_request(device);
		return;
	}
	LOG(("error getting sector data\n"));

	i8271_timed_command_complete(device);
}

static void i8271_do_read_id(device_t *device)
{
	chrn_id id;
	i8271_t *i8271 = get_safe_token(device);

	/* get next id from disc */
	floppy_drive_get_next_id(current_image(device), i8271->side,&id);

	i8271->pExecutionPhaseData[0] = id.C;
	i8271->pExecutionPhaseData[1] = id.H;
	i8271->pExecutionPhaseData[2] = id.R;
	i8271->pExecutionPhaseData[3] = id.N;

	i8271_initialise_execution_phase_read(device, 4);
}


static void i8271_do_write(device_t *device)
{
	i8271_t *i8271 = get_safe_token(device);
	/* find the sector */
	if (i8271_find_sector(device))
	{
		/* initialise for reading */
		i8271_initialise_execution_phase_write(device,1<<(i8271->ID_N+7));

		/* update state - gets first byte and triggers a data request */
		i8271_timed_data_request(device);
		return;
	}
	LOG(("error getting sector data\n"));

	i8271_timed_command_complete(device);
}



static int i8271_find_sector(device_t *device)
{
	device_t *img = current_image(device);
	i8271_t *i8271 = get_safe_token(device);
//  int track_count_attempt;

//  track_count_attempt
	/* find sector within one revolution of the disc - 2 index pulses */

	/* number of times we have seen index hole */
	int index_count = 0;

	/* get sector id's */
	do
	{
		chrn_id id;

		/* get next id from disc */
		if (floppy_drive_get_next_id(img, i8271->side,&id))
		{
			/* tested on Amstrad CPC - All bytes must match, otherwise
			a NO DATA error is reported */
			if (id.R == i8271->ID_R)
			{
				/* TODO: Is this correct? What about bad tracks? */
				if (id.C == i8271->CurrentTrack[i8271->drive])
				{
					i8271->data_id = id.data_id;
					return 1;
				}
				else
				{
					/* TODO: if track doesn't match, the real 8271 does a step */


					return 0;
				}
			}
		}

			/* index set? */
		if (floppy_drive_get_flag_state(img, FLOPPY_DRIVE_INDEX))
		{
			index_count++;
		}

	}
	while (index_count!=2);

	/* completion type: command/drive error */
	/* completion code: sector not found */
	i8271->ResultRegister |= (3<<3);

	return 0;
}

static void i8271_command_execute(device_t *device)
{
	i8271_t *i8271 = get_safe_token(device);
	device_t *img = current_image(device);

	/* clear it = good completion status */
	/* this will be changed if anything bad happens! */
	i8271->ResultRegister = 0;

	switch (i8271->Command)
	{
		case I8271_COMMAND_SPECIFY:
		{
			switch (i8271->CommandParameters[0])
			{
				case 0x0d:
				{
					LOG(("Initialization\n"));
					i8271->StepRate = i8271->CommandParameters[1];
					i8271->HeadSettlingTime = i8271->CommandParameters[2];
					i8271->IndexCountBeforeHeadUnload = (i8271->CommandParameters[3]>>4) & 0x0f;
					i8271->HeadLoadTime = (i8271->CommandParameters[3] & 0x0f);
				}
				break;

				case 0x010:
				{
					LOG(("Load bad Tracks Surface 0\n"));
					i8271_load_bad_tracks(device,0);

				}
				break;

				case 0x018:
				{
					LOG(("Load bad Tracks Surface 1\n"));
					i8271_load_bad_tracks(device,1);

				}
				break;
			}

			/* no result */
			i8271_command_complete(device,0,0);
		}
		break;

		case I8271_COMMAND_READ_SPECIAL_REGISTER:
		{
			/* unknown - what is read when a special register that isn't allowed is specified? */
			int data = 0x0ff;

			switch (i8271->CommandParameters[0])
			{
				case I8271_SPECIAL_REGISTER_MODE_REGISTER:
				{
					data = i8271->Mode;
				}
				break;

				case I8271_SPECIAL_REGISTER_SURFACE_0_CURRENT_TRACK:
				{
					data = i8271_read_current_track(device, 0);

				}
				break;

				case I8271_SPECIAL_REGISTER_SURFACE_1_CURRENT_TRACK:
				{
					data = i8271_read_current_track(device, 1);
				}
				break;

				case I8271_SPECIAL_REGISTER_SURFACE_0_BAD_TRACK_1:
				{
					data = i8271_read_bad_track(device, 0,1);
				}
				break;

				case I8271_SPECIAL_REGISTER_SURFACE_0_BAD_TRACK_2:
				{
					data = i8271_read_bad_track(device, 0,2);
				}
				break;

				case I8271_SPECIAL_REGISTER_SURFACE_1_BAD_TRACK_1:
				{
					data = i8271_read_bad_track(device, 1,1);
				}
				break;

				case I8271_SPECIAL_REGISTER_SURFACE_1_BAD_TRACK_2:
				{
					data = i8271_read_bad_track(device, 1,2);
				}
				break;

				case I8271_SPECIAL_REGISTER_DRIVE_CONTROL_OUTPUT_PORT:
				{
					FDC_LOG_COMMAND("Read Drive Control Output port\n");

					i8271_get_drive(device);

					/* assumption: select bits reflect the select bits from the previous
					command. i.e. read drive status */
					data = (i8271->drive_control_output & ~0x0c0)
						| (i8271->CommandRegister & 0x0c0);


				}
				break;

				case I8271_SPECIAL_REGISTER_DRIVE_CONTROL_INPUT_PORT:
				{
					/* bit 7: not used */
					/* bit 6: ready 1 */
					/* bit 5: write fault */
					/* bit 4: index */
					/* bit 3: wr prot */
					/* bit 2: rdy 0 */
					/* bit 1: track 0 */
					/* bit 0: cnt/opi */

					FDC_LOG_COMMAND("Read Drive Control Input port\n");


					i8271->drive_control_input = (1<<6) | (1<<2);

					/* bit 3 = 0 if write protected */
					i8271->drive_control_input |= floppy_wpt_r(img) << 3;

					/* bit 1 = 0 if head at track 0 */
					i8271->drive_control_input |= floppy_tk00_r(img) << 1;

					/* need to setup this register based on drive selected */
					data = i8271->drive_control_input;
				}
				break;

			}

			i8271->ResultRegister = data;

			i8271_command_complete(device,1,0);
		}
		break;


		case I8271_COMMAND_WRITE_SPECIAL_REGISTER:
		{
			switch (i8271->CommandParameters[0])
			{
				case I8271_SPECIAL_REGISTER_MODE_REGISTER:
				{
					/* TODO: Check bits 6-7 and 5-2 are valid */
					i8271->Mode = i8271->CommandParameters[1];

					if (i8271->Mode & 0x01)
					{
						LOG(("Mode: Non-DMA\n"));
					}
					else
					{
						LOG(("Mode: DMA\n"));
					}

					if (i8271->Mode & 0x02)
					{
						LOG(("Single actuator\n"));
					}
					else
					{
						LOG(("Double actuator\n"));
					}
				}
				break;

				case I8271_SPECIAL_REGISTER_SURFACE_0_CURRENT_TRACK:
				{
					LOG(("Surface 0 Current Track\n"));
					i8271_write_current_track(device, 0, i8271->CommandParameters[1]);
				}
				break;

				case I8271_SPECIAL_REGISTER_SURFACE_1_CURRENT_TRACK:
				{
					LOG(("Surface 1 Current Track\n"));
					i8271_write_current_track(device, 1, i8271->CommandParameters[1]);
				}
				break;

				case I8271_SPECIAL_REGISTER_SURFACE_0_BAD_TRACK_1:
				{
					LOG(("Surface 0 Bad Track 1\n"));
					i8271_write_bad_track(device, 0, 1, i8271->CommandParameters[1]);
				}
				break;

				case I8271_SPECIAL_REGISTER_SURFACE_0_BAD_TRACK_2:
				{
					LOG(("Surface 0 Bad Track 2\n"));
					i8271_write_bad_track(device, 0, 2,i8271->CommandParameters[1]);
				}
				break;

				case I8271_SPECIAL_REGISTER_SURFACE_1_BAD_TRACK_1:
				{
					LOG(("Surface 1 Bad Track 1\n"));


					i8271_write_bad_track(device, 1, 1, i8271->CommandParameters[1]);
				}
				break;

				case I8271_SPECIAL_REGISTER_SURFACE_1_BAD_TRACK_2:
				{
					LOG(("Surface 1 Bad Track 2\n"));

					i8271_write_bad_track(device, 1, 2, i8271->CommandParameters[1]);
				}
				break;

				case I8271_SPECIAL_REGISTER_DRIVE_CONTROL_OUTPUT_PORT:
				{
//                  /* get drive selected */
//                  i8271->drive = (i8271->CommandParameters[1]>>6) & 0x03;

					FDC_LOG_COMMAND("Write Drive Control Output port\n");


					if (i8271->CommandParameters[1] & 0x01)
					{
						LOG(("Write Enable\n"));
					}
					if (i8271->CommandParameters[1] & 0x02)
					{
						LOG(("Seek/Step\n"));
					}
					if (i8271->CommandParameters[1] & 0x04)
					{
						LOG(("Direction\n"));
					}
					if (i8271->CommandParameters[1] & 0x08)
					{
						LOG(("Load Head\n"));
					}
					if (i8271->CommandParameters[1] & 0x010)
					{
						LOG(("Low head current\n"));
					}
					if (i8271->CommandParameters[1] & 0x020)
					{
						LOG(("Write Fault Reset\n"));
					}

					LOG(("Select %02x\n", (i8271->CommandParameters[1] & 0x0c0)>>6));

					/* get drive */
					i8271_get_drive(device);

					/* on bbc dfs 09 this is the side select output */
					i8271->side = (i8271->CommandParameters[1]>>5) & 0x01;

					/* load head - on mini-sized drives this turns on the disc motor,
					on standard-sized drives this loads the head and turns the motor on */
					floppy_mon_w(img, !BIT(i8271->CommandParameters[1], 3));
					floppy_drive_set_ready_state(img, 1, 1);

					/* step pin changed? if so perform a step in the direction indicated */
					if (((i8271->drive_control_output^i8271->CommandParameters[1]) & (1<<1))!=0)
					{
						/* step pin changed state? */

						if ((i8271->CommandParameters[1] & (1<<1))!=0)
						{
							signed int signed_tracks;

							if ((i8271->CommandParameters[1] & (1<<2))!=0)
							{
								signed_tracks = 1;
							}
							else
							{
								signed_tracks = -1;
							}

							floppy_drive_seek(img, signed_tracks);
						}
					}

					i8271->drive_control_output = i8271->CommandParameters[1];


				}
				break;

				case I8271_SPECIAL_REGISTER_DRIVE_CONTROL_INPUT_PORT:
				{

					FDC_LOG_COMMAND("Write Drive Control Input port\n");

					//                  i8271->drive_control_input = i8271->CommandParameters[1];
				}
				break;

			}

			/* write doesn't supply a result */
			i8271_command_complete(device,0,0);
		}
		break;

		case I8271_COMMAND_READ_DRIVE_STATUS:
		{
			unsigned char status;

			i8271_get_drive(device);

			/* no write fault */
			status = 0;

			status |= (1<<2) | (1<<6);

			/* these two do not appear to be set at all! ?? */

			if (i8271->intf->floppy_drive_tags[0]!=NULL) {
				if (floppy_drive_get_flag_state(device->machine().device(i8271->intf->floppy_drive_tags[0]), FLOPPY_DRIVE_READY))
				{
					status |= (1<<2);
				}
			}

			if (i8271->intf->floppy_drive_tags[1]!=NULL) {
				if (floppy_drive_get_flag_state(device->machine().device(i8271->intf->floppy_drive_tags[1]), FLOPPY_DRIVE_READY))
				{
					status |= (1<<6);
				}
			}

			/* bit 3 = 1 if write protected */
			status |= !floppy_wpt_r(img) << 3;

			/* bit 1 = 1 if head at track 0 */
			status |= !floppy_tk00_r(img) << 1;

			i8271->ResultRegister = status;
			i8271_command_complete(device,1,0);

		}
		break;

		case I8271_COMMAND_SEEK:
		{
			i8271_get_drive(device);


			i8271_seek_to_track(device,i8271->CommandParameters[0]);

			/* check for bad seek */
			i8271_timed_command_complete(device);

		}
		break;

		case I8271_COMMAND_READ_DATA_MULTI_RECORD:
		{
			/* N value as stored in ID field */
			i8271->ID_N = (i8271->CommandParameters[2]>>5) & 0x07;

			/* starting sector id */
			i8271->ID_R = i8271->CommandParameters[1];

			/* number of sectors to transfer */
			i8271->Counter = i8271->CommandParameters[2] & 0x01f;


			FDC_LOG_COMMAND("READ DATA MULTI RECORD");

			LOG(("Sector Count: %02x\n", i8271->Counter));
			LOG(("Track: %02x\n",i8271->CommandParameters[0]));
			LOG(("Sector: %02x\n", i8271->CommandParameters[1]));
			LOG(("Sector Length: %02x bytes\n", 1<<(i8271->ID_N+7)));

			i8271_get_drive(device);

			if (!floppy_drive_get_flag_state(img, FLOPPY_DRIVE_READY))
			{
				/* Completion type: operation intervention probably required for recovery */
				/* Completion code: Drive not ready */
				i8271->ResultRegister = (2<<3);
				i8271_timed_command_complete(device);
			}
			else
			{
				i8271_seek_to_track(device,i8271->CommandParameters[0]);


				i8271_do_read(device);
			}

		}
		break;

		case I8271_COMMAND_READ_DATA_SINGLE_RECORD:
		{
			FDC_LOG_COMMAND("READ DATA SINGLE RECORD");

			i8271->ID_N = 0;
			i8271->Counter = 1;
			i8271->ID_R = i8271->CommandParameters[1];

			LOG(("Sector Count: %02x\n", i8271->Counter));
			LOG(("Track: %02x\n",i8271->CommandParameters[0]));
			LOG(("Sector: %02x\n", i8271->CommandParameters[1]));
			LOG(("Sector Length: %02x bytes\n", 1<<(i8271->ID_N+7)));

			i8271_get_drive(device);

			if (!floppy_drive_get_flag_state(img, FLOPPY_DRIVE_READY))
			{
				/* Completion type: operation intervention probably required for recovery */
				/* Completion code: Drive not ready */
				i8271->ResultRegister = (2<<3);
				i8271_timed_command_complete(device);
			}
			else
			{
				i8271_seek_to_track(device,i8271->CommandParameters[0]);

				i8271_do_read(device);
			}

		}
		break;

		case I8271_COMMAND_WRITE_DATA_MULTI_RECORD:
		{
			/* N value as stored in ID field */
			i8271->ID_N = (i8271->CommandParameters[2]>>5) & 0x07;

			/* starting sector id */
			i8271->ID_R = i8271->CommandParameters[1];

			/* number of sectors to transfer */
			i8271->Counter = i8271->CommandParameters[2] & 0x01f;

			FDC_LOG_COMMAND("READ DATA MULTI RECORD");

			LOG(("Sector Count: %02x\n", i8271->Counter));
			LOG(("Track: %02x\n",i8271->CommandParameters[0]));
			LOG(("Sector: %02x\n", i8271->CommandParameters[1]));
			LOG(("Sector Length: %02x bytes\n", 1<<(i8271->ID_N+7)));

			i8271_get_drive(device);

			i8271->drive_control_output &=~1;

			if (!floppy_drive_get_flag_state(img, FLOPPY_DRIVE_READY))
			{
				/* Completion type: operation intervention probably required for recovery */
				/* Completion code: Drive not ready */
				i8271->ResultRegister = (2<<3);
				i8271_timed_command_complete(device);
			}
			else
			{
				if (floppy_wpt_r(img) == CLEAR_LINE)
				{
					/* Completion type: operation intervention probably required for recovery */
					/* Completion code: Drive write protected */
					i8271->ResultRegister = (2<<3) | (1<<1);
					i8271_timed_command_complete(device);
				}
				else
				{
					i8271->drive_control_output |=1;

					i8271_seek_to_track(device,i8271->CommandParameters[0]);

					i8271_do_write(device);
				}
			}
		}
		break;

		case I8271_COMMAND_WRITE_DATA_SINGLE_RECORD:
		{
			FDC_LOG_COMMAND("WRITE DATA SINGLE RECORD");

			i8271->ID_N = 0;
			i8271->Counter = 1;
			i8271->ID_R = i8271->CommandParameters[1];


			LOG(("Sector Count: %02x\n", i8271->Counter));
			LOG(("Track: %02x\n",i8271->CommandParameters[0]));
			LOG(("Sector: %02x\n", i8271->CommandParameters[1]));
			LOG(("Sector Length: %02x bytes\n", 1<<(i8271->ID_N+7)));
			i8271_get_drive(device);

			i8271->drive_control_output &=~1;

			if (!floppy_drive_get_flag_state(img, FLOPPY_DRIVE_READY))
			{
				/* Completion type: operation intervention probably required for recovery */
				/* Completion code: Drive not ready */
				i8271->ResultRegister = (2<<3);
				i8271_timed_command_complete(device);
			}
			else
			{
				if (floppy_wpt_r(img) == CLEAR_LINE)
				{
					/* Completion type: operation intervention probably required for recovery */
					/* Completion code: Drive write protected */
					i8271->ResultRegister = (2<<3) | (1<<1);
					i8271_timed_command_complete(device);
				}
				else
				{

					i8271->drive_control_output |=1;

					i8271_seek_to_track(device,i8271->CommandParameters[0]);

					i8271_do_write(device);
				}
			}

		}
		break;


		case I8271_COMMAND_READ_ID:
		{
			FDC_LOG_COMMAND("READ ID");

			LOG(("Track: %02x\n",i8271->CommandParameters[0]));
			LOG(("ID Field Count: %02x\n", i8271->CommandParameters[2]));

			i8271_get_drive(device);

			if (!floppy_drive_get_flag_state(img, FLOPPY_DRIVE_READY))
			{
				/* Completion type: operation intervention probably required for recovery */
				/* Completion code: Drive not ready */
				i8271->ResultRegister = (2<<3);
				i8271_timed_command_complete(device);
			}
			else
			{

				i8271->Counter = i8271->CommandParameters[2];

				i8271_seek_to_track(device,i8271->CommandParameters[0]);

				i8271_do_read_id(device);
			}
		}
		break;

		default:
			LOG(("ERROR Unrecognised Command\n"));
			break;
	}
}



WRITE8_DEVICE_HANDLER(i8271_w)
{
	i8271_t *i8271 = get_safe_token(device);
	switch (offset & 3)
	{
		case 0:
		{
			LOG(("I8271 W Command Register: %02x\n", data));

			i8271->CommandRegister = data;
			i8271->Command = i8271->CommandRegister & 0x03f;

			i8271->StatusRegister |= I8271_STATUS_COMMAND_BUSY | I8271_STATUS_COMMAND_FULL;
			i8271->StatusRegister &= ~I8271_STATUS_PARAMETER_FULL | I8271_STATUS_RESULT_FULL;
			i8271->ParameterCountWritten = 0;

			switch (i8271->Command)
			{
				case I8271_COMMAND_SPECIFY:
				{
					FDC_LOG_COMMAND("SPECIFY");

					i8271->ParameterCount = 4;
				}
				break;

				case I8271_COMMAND_SEEK:
				{
					FDC_LOG_COMMAND("SEEK");

					i8271->ParameterCount = 1;
				}
				break;

				case I8271_COMMAND_READ_DRIVE_STATUS:
				{
					FDC_LOG_COMMAND("READ DRIVE STATUS");

					i8271->ParameterCount = 0;
				}
				break;

				case I8271_COMMAND_READ_SPECIAL_REGISTER:
				{
					FDC_LOG_COMMAND("READ SPECIAL REGISTER");

					i8271->ParameterCount = 1;
				}
				break;

				case I8271_COMMAND_WRITE_SPECIAL_REGISTER:
				{
					FDC_LOG_COMMAND("WRITE SPECIAL REGISTER");

					i8271->ParameterCount = 2;
				}
				break;

				case I8271_COMMAND_FORMAT:
				{
					i8271->ParameterCount = 5;
				}
				break;

				case I8271_COMMAND_READ_ID:
				{
					i8271->ParameterCount = 3;

				}
				break;


				case I8271_COMMAND_READ_DATA_SINGLE_RECORD:
				case I8271_COMMAND_READ_DATA_AND_DELETED_DATA_SINGLE_RECORD:
				case I8271_COMMAND_WRITE_DATA_SINGLE_RECORD:
				case I8271_COMMAND_WRITE_DELETED_DATA_SINGLE_RECORD:
				case I8271_COMMAND_VERIFY_DATA_AND_DELETED_DATA_SINGLE_RECORD:
				{
					i8271->ParameterCount = 2;
				}
				break;

				case I8271_COMMAND_READ_DATA_MULTI_RECORD:
				case I8271_COMMAND_READ_DATA_AND_DELETED_DATA_MULTI_RECORD:
				case I8271_COMMAND_WRITE_DATA_MULTI_RECORD:
				case I8271_COMMAND_WRITE_DELETED_DATA_MULTI_RECORD:
				case I8271_COMMAND_VERIFY_DATA_AND_DELETED_DATA_MULTI_RECORD:
				{
					i8271->ParameterCount = 3;
				}
				break;

				case I8271_COMMAND_SCAN_DATA:
				case I8271_COMMAND_SCAN_DATA_AND_DELETED_DATA:
				{
					i8271->ParameterCount = 5;
				}
				break;






			}

			i8271_check_all_parameters_written(device);
		}
		break;

		case 1:
		{
			LOG(("I8271 W Parameter Register: %02x\n",data));
			i8271->ParameterRegister = data;

			if (i8271->ParameterCount!=0)
			{
				i8271->CommandParameters[i8271->ParameterCountWritten] = data;
				i8271->ParameterCountWritten++;
			}

			i8271_check_all_parameters_written(device);
		}
		break;

		case 2:
		{
			LOG(("I8271 W Reset Register: %02x\n", data));
			if (((data ^ i8271->ResetRegister) & 0x01)!=0)
			{
				if ((data & 0x01)==0)
				{
					DEVICE_RESET_CALL( i8271 );
				}
			}

			i8271->ResetRegister = data;


		}
		break;

		default:
			break;
	}
}

READ8_DEVICE_HANDLER(i8271_r)
{
	i8271_t *i8271 = get_safe_token(device);
	switch (offset & 3)
	{
		case 0:
		{
			/* bit 1,0 are zero other bits contain status data */
			i8271->StatusRegister &= ~0x03;
			LOG(("I8271 R Status Register: %02x\n",i8271->StatusRegister));
			return i8271->StatusRegister;
		}

		case 1:
		{

			if ((i8271->StatusRegister & I8271_STATUS_COMMAND_BUSY)==0)
			{
				/* clear IRQ */
				i8271_set_irq_state(device,0);

				i8271->StatusRegister &= ~I8271_STATUS_RESULT_FULL;
				LOG(("I8271 R Result Register %02x\n",i8271->ResultRegister));
				return i8271->ResultRegister;
			}

			/* not useful information when command busy */
			return 0x0ff;
		}


		default:
			break;
	}

	return 0x0ff;
}


/* to be completed! */
READ8_DEVICE_HANDLER(i8271_dack_r)
{
	return i8271_data_r(device, space, offset);
}

/* to be completed! */
WRITE8_DEVICE_HANDLER(i8271_dack_w)
{
	i8271_data_w(device, space, offset, data);
}

	READ8_DEVICE_HANDLER(i8271_data_r)
{
	i8271_t *i8271 = get_safe_token(device);

	i8271_clear_data_request(device);

	i8271_update_state(device);

	//  logerror("I8271 R data: %02x\n",i8271->data);


	return i8271->data;
}

WRITE8_DEVICE_HANDLER(i8271_data_w)
{
	i8271_t *i8271 = get_safe_token(device);
	i8271->data = data;

//    logerror("I8271 W data: %02x\n",i8271->data);

	i8271_clear_data_request(device);

	i8271_update_state(device);
}


/* Device Interface */

static DEVICE_START( i8271 )
{
	i8271_t *i8271 = get_safe_token(device);
	// validate arguments

	assert(device != NULL);
	assert(device->tag() != NULL);
	assert(device->static_config() != NULL);

	i8271->intf = (const i8271_interface*)device->static_config();

	i8271->data_timer = device->machine().scheduler().timer_alloc(FUNC(i8271_data_timer_callback), (void *)device);
	i8271->command_complete_timer = device->machine().scheduler().timer_alloc(FUNC(i8271_timed_command_complete_callback), (void *)device);
	i8271->drive = 0;
	i8271->pExecutionPhaseData = auto_alloc_array(device->machine(), char, 0x4000);

	// register for state saving
	//state_save_register_item(device->machine(), "i8271", device->tag(), 0, i8271->number);
}

static DEVICE_RESET( i8271 )
{
	i8271_t *i8271 = get_safe_token(device);

	i8271->StatusRegister = 0;  //I8271_STATUS_INT_REQUEST | I8271_STATUS_NON_DMA_REQUEST;
	i8271->Mode = 0x0c0; /* bits 0, 1 are initialized to zero */
	i8271->ParameterCountWritten = 0;
	i8271->ParameterCount = 0;

	/* if timer is active remove */
	i8271->command_complete_timer->reset();
	i8271->data_timer->reset();

	/* clear irq */
	i8271_set_irq_state(device,0);
	/* clear dma */
	i8271_set_dma_drq(device);

}

const device_type I8271 = &device_creator<i8271_device>;

i8271_device::i8271_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, I8271, "Intel 8271", tag, owner, clock)
{
	m_token = global_alloc_clear(i8271_t);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void i8271_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8271_device::device_start()
{
	DEVICE_START_NAME( i8271 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i8271_device::device_reset()
{
	DEVICE_RESET_NAME( i8271 )(this);
}
