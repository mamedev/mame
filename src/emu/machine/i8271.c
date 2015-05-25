// license:BSD-3-Clause
// copyright-holders:Kevin Thacker
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

/* data request */
#define I8271_FLAGS_DATA_REQUEST 0x01
/* data direction. If 0x02, then it is from fdc to cpu, else
it is from cpu to fdc */
#define I8271_FLAGS_DATA_DIRECTION 0x02

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

#define VERBOSE 0

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)
#define FDC_LOG(x) do { if (VERBOSE) logerror("I8271: %s\n",x); } while (0)
#define FDC_LOG_COMMAND(x) do { if (VERBOSE) logerror("I8271: COMMAND %s\n",x); } while (0)


/* Device Interface */

const device_type I8271 = &device_creator<i8271_device>;

i8271_device::i8271_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, I8271, "Intel 8271", tag, owner, clock, "i8271", __FILE__),
	m_write_irq(*this),
	m_write_drq(*this),
	m_floppy_tag1(NULL),
	m_floppy_tag2(NULL),
	m_flags(0),
	m_state(0),
	m_Command(0),
	m_StatusRegister(0),
	m_CommandRegister(0),
	m_ResultRegister(0),
	m_ParameterRegister(0),
	m_ResetRegister(0),
	m_data(0),
	m_ParameterCount(0),
	m_ParameterCountWritten(0),
	m_Mode(0),
	m_drive(0),
	m_side(0),
	m_drive_control_output(0),
	m_drive_control_input(0),
	m_StepRate(0),
	m_HeadSettlingTime(0),
	m_IndexCountBeforeHeadUnload(0),
	m_HeadLoadTime(0),
	//m_ID_C(0),
	//m_ID_H(0),
	m_ID_R(0),
	m_ID_N(0),
	m_data_id(0),
	m_ExecutionPhaseTransferCount(0),
	m_ExecutionPhaseCount(0),
	m_Counter(0)
	//m_data_direction(0)
{
	for (int i = 0; i < 8; i++ )
	{
		m_CommandParameters[i] = 0;
	}

	for (int i = 0; i < 2; i++ )
	{
		m_CurrentTrack[i] = 0;
	}

	for (int i = 0; i < 4; i++ )
	{
		m_BadTracks[i] = 0;
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8271_device::device_start()
{
	m_write_irq.resolve_safe();
	m_write_drq.resolve_safe();

	m_data_timer = timer_alloc(TIMER_DATA_CALLBACK);
	m_command_complete_timer = timer_alloc(TIMER_TIMED_COMMAND_COMPLETE);
	m_drive = 0;
	m_pExecutionPhaseData = auto_alloc_array(machine(), char, 0x4000);

	m_floppy[0] = machine().device<legacy_floppy_image_device>(m_floppy_tag1);
	m_floppy[1] = machine().device<legacy_floppy_image_device>(m_floppy_tag2);

	// register for state saving
	/*save_item(NAME(m_flags));
	save_item(NAME(m_state));
	save_item(NAME(m_Command));
	save_item(NAME(m_StatusRegister));
	save_item(NAME(m_CommandRegister));
	save_item(NAME(m_ResultRegister));
	save_item(NAME(m_ParameterRegister));
	save_item(NAME(m_ResetRegister));
	save_item(NAME(m_data));
	//save_item(NAME(m_ParameterCount));
	//save_item(NAME(m_ParameterCountWritten));
	save_item(NAME(m_CommandParameters));
	//save_item(NAME(m_CurrentTrack));
	//save_item(NAME(m_BadTracks));
	//save_item(NAME(m_Mode));
	save_item(NAME(m_drive));
	save_item(NAME(m_side));
	save_item(NAME(m_drive_control_output));
	save_item(NAME(m_drive_control_input));
	//save_item(NAME(m_StepRate));
	//save_item(NAME(m_HeadSettlingTime));
	//save_item(NAME(m_IndexCountBeforeHeadUnload));
	//save_item(NAME(m_HeadLoadTime));
	save_item(NAME(m_ID_C));
	save_item(NAME(m_ID_H));
	save_item(NAME(m_ID_R));
	save_item(NAME(m_ID_N));
	save_item(NAME(m_data_id));
	save_item(NAME(m_ExecutionPhaseTransferCount));
	save_item(NAME(m_ExecutionPhaseCount));
	save_item(NAME(m_Counter));
	save_item(NAME(m_data_direction));
	save_pointer(NAME(m_pExecutionPhaseData), 0x4000);*/
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i8271_device::device_reset()
{
	m_StatusRegister = 0;  //I8271_STATUS_INT_REQUEST | I8271_STATUS_NON_DMA_REQUEST;
	m_Mode = 0x0c0; /* bits 0, 1 are initialized to zero */
	m_ParameterCountWritten = 0;
	m_ParameterCount = 0;

	/* if timer is active remove */
	m_command_complete_timer->reset();
	m_data_timer->reset();

	/* clear irq */
	set_irq_state(0);
	/* clear dma */
	set_dma_drq();
}

void i8271_device::seek_to_track(int track)
{
	if (track==0)
	{
		/* seek to track 0 */
		unsigned char StepCount = 0x0ff;

		/*logerror("step\n"); */

		/* track 0 not set, not seeked more than 255 tracks */
		while (m_floppy[m_drive]->floppy_tk00_r() && (StepCount != 0))
		{
/*            logerror("step\n"); */
			StepCount--;
			m_floppy[m_drive]->floppy_drive_seek(-1);
		}

		m_CurrentTrack[m_drive] = 0;

		/* failed to find track 0? */
		if (StepCount==0)
		{
			/* Completion Type: operator intervation probably required for recovery */
			/* Completion code: track 0 not found */
			m_ResultRegister |= (2<<3) | 2<<1;
		}

		/* step out - towards track 0 */
		m_drive_control_output &=~(1<<2);
	}
	else
	{
		signed int SignedTracks;

		/* calculate number of tracks to seek */
		SignedTracks = track - m_CurrentTrack[m_drive];

		/* step towards 0 */
		m_drive_control_output &= ~(1<<2);

		if (SignedTracks>0)
		{
			/* step away from 0 */
			m_drive_control_output |= (1<<2);
		}


		/* seek to track 0 */
		m_floppy[m_drive]->floppy_drive_seek(SignedTracks);

		m_CurrentTrack[m_drive] = track;
	}
}

void i8271_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_DATA_CALLBACK:
		/* ok, trigger data request now */
		data_request();

		/* stop it */
		m_data_timer->reset();
		break;

	case TIMER_TIMED_COMMAND_COMPLETE:
		command_complete(1,1);

		/* stop it, but don't allow it to be free'd */
		m_command_complete_timer->reset();
		break;

	default:
		break;
	}
}

/* setup a timed data request - data request will be triggered in a few usecs time */
void i8271_device::timed_data_request()
{
	int usecs;
	/* 64 for single density */
	usecs = 64;

	/* set timers */
	m_command_complete_timer->reset();
	m_data_timer->adjust(attotime::from_usec(usecs));
}


/* setup a irq to occur 128us later - in reality this would be much later, because the int would
come after reading the two CRC bytes at least! This function is used when a irq is required at
command completion. Required for read data and write data, where last byte could be missed! */
void i8271_device::timed_command_complete()
{
	int usecs;

	/* 64 for single density - 2 crc bytes later*/
	usecs = 64*2;

	/* set timers */
	m_data_timer->reset();
	m_command_complete_timer->adjust(attotime::from_usec(usecs));
}

void i8271_device::set_irq_state(int state)
{
	m_StatusRegister &= ~I8271_STATUS_INT_REQUEST;
	if (state)
	{
		m_StatusRegister |= I8271_STATUS_INT_REQUEST;
	}

	m_write_irq((m_StatusRegister & I8271_STATUS_INT_REQUEST) ? ASSERT_LINE : CLEAR_LINE);
}

void i8271_device::set_dma_drq()
{
	m_write_drq((m_flags & I8271_FLAGS_DATA_REQUEST) ? 1 : 0);
}

void i8271_device::load_bad_tracks(int surface)
{
	m_BadTracks[(surface<<1) + 0] = m_CommandParameters[1];
	m_BadTracks[(surface<<1) + 1] = m_CommandParameters[2];
	m_CurrentTrack[surface] = m_CommandParameters[3];
}

void i8271_device::write_bad_track(int surface, int track, int data)
{
	m_BadTracks[(surface<<1) + (track-1)] = data;
}

void i8271_device::write_current_track(int surface, int track)
{
	m_CurrentTrack[surface] = track;
}

int i8271_device::read_current_track(int surface)
{
	return m_CurrentTrack[surface];
}

int i8271_device::read_bad_track(int surface, int track)
{
	return m_BadTracks[(surface<<1) + (track-1)];
}

void i8271_device::get_drive()
{
	/* &40 = drive 0 side 0 */
	/* &80 = drive 1 side 0 */



	if (m_CommandRegister & (1<<6))
	{
		m_drive = 0;
	}

	if (m_CommandRegister & (1<<7))
	{
		m_drive = 1;
	}

}

void i8271_device::check_all_parameters_written()
{
	if (m_ParameterCount == m_ParameterCountWritten)
	{
		m_StatusRegister &= ~I8271_STATUS_COMMAND_FULL;

		command_execute();
	}
}


void i8271_device::update_state()
{
	switch (m_state)
	{
		/* fdc reading data and passing it to cpu which must read it */
		case I8271_STATE_EXECUTION_READ:
		{
	//      /* if data request has been cleared, i.e. caused by a read of the register */
	//      if ((m_flags & I8271_FLAGS_DATA_REQUEST)==0)
			{
				/* setup data with byte */
				m_data = m_pExecutionPhaseData[m_ExecutionPhaseCount];

/*              logerror("read data %02x\n", m_data); */

				/* update counters */
				m_ExecutionPhaseCount++;
				m_ExecutionPhaseTransferCount--;

			//  logerror("Count: %04x\n", m_ExecutionPhaseCount);
			//  logerror("Remaining: %04x\n", m_ExecutionPhaseTransferCount);

				/* completed? */
				if (m_ExecutionPhaseTransferCount==0)
				{
					/* yes */

			//      logerror("sector read complete!\n");
					/* continue command */
					command_continue();
				}
				else
				{
					/* no */

					/* issue data request */
					timed_data_request();
				}
			}
		}
		break;

		/* fdc reading data and passing it to cpu which must read it */
		case I8271_STATE_EXECUTION_WRITE:
		{
			/* setup data with byte */
			m_pExecutionPhaseData[m_ExecutionPhaseCount] = m_data;
			/* update counters */
			m_ExecutionPhaseCount++;
			m_ExecutionPhaseTransferCount--;

			/* completed? */
			if (m_ExecutionPhaseTransferCount==0)
			{
				/* yes */

				/* continue command */
				command_continue();
			}
			else
			{
				/* no */

				/* issue data request */
				timed_data_request();
			}
		}
		break;

		default:
			break;
	}
}

void i8271_device::initialise_execution_phase_read(int transfer_size)
{
	/* read */
	m_flags |= I8271_FLAGS_DATA_DIRECTION;
	m_ExecutionPhaseCount = 0;
	m_ExecutionPhaseTransferCount = transfer_size;
	m_state = I8271_STATE_EXECUTION_READ;
}


void i8271_device::initialise_execution_phase_write(int transfer_size)
{
	/* write */
	m_flags &= ~I8271_FLAGS_DATA_DIRECTION;
	m_ExecutionPhaseCount = 0;
	m_ExecutionPhaseTransferCount = transfer_size;
	m_state = I8271_STATE_EXECUTION_WRITE;
}

/* for data transfers */
void i8271_device::data_request()
{
	m_flags |= I8271_FLAGS_DATA_REQUEST;

	if ((m_Mode & 0x01)!=0)
	{
		/* non-dma */
		m_StatusRegister |= I8271_STATUS_NON_DMA_REQUEST;
		/* set int */
		set_irq_state(1);
	}
	else
	{
		/* dma */
		m_StatusRegister &= ~I8271_STATUS_NON_DMA_REQUEST;

		set_dma_drq();
	}
}

void i8271_device::command_complete(int result, int int_rq)
{
	/* not busy, and not a execution phase data request in non-dma mode */
	m_StatusRegister &= ~(I8271_STATUS_COMMAND_BUSY | I8271_STATUS_NON_DMA_REQUEST);

	if (result)
	{
		m_StatusRegister |= I8271_STATUS_RESULT_FULL;
	}

	if (int_rq)
	{
		/* trigger an int */
		set_irq_state(1);
	}

	/* correct?? */
	m_drive_control_output &=~1;
}


/* for data transfers */
void i8271_device::clear_data_request()
{
	m_flags &= ~I8271_FLAGS_DATA_REQUEST;

	if ((m_Mode & 0x01)!=0)
	{
		/* non-dma */
		m_StatusRegister &= ~I8271_STATUS_NON_DMA_REQUEST;
		/* set int */
		set_irq_state(0);
	}
	else
	{
		/* dma */
		set_dma_drq();
	}
}


void i8271_device::command_continue()
{
	switch (m_Command)
	{
		case I8271_COMMAND_READ_DATA_MULTI_RECORD:
		case I8271_COMMAND_READ_DATA_SINGLE_RECORD:
		{
			/* completed all sectors? */
			m_Counter--;
			/* increment sector id */
			m_ID_R++;

			/* end command? */
			if (m_Counter==0)
			{
				timed_command_complete();
				return;
			}

			do_read();
		}
		break;

		case I8271_COMMAND_WRITE_DATA_MULTI_RECORD:
		case I8271_COMMAND_WRITE_DATA_SINGLE_RECORD:
		{
			/* put the buffer to the sector */
			m_floppy[m_drive]->floppy_drive_write_sector_data(m_side, m_data_id, m_pExecutionPhaseData, 1<<(m_ID_N+7),0);

			/* completed all sectors? */
			m_Counter--;
			/* increment sector id */
			m_ID_R++;

			/* end command? */
			if (m_Counter==0)
			{
				timed_command_complete();
				return;
			}

			do_write();
		}
		break;

		case I8271_COMMAND_READ_ID:
		{
			m_Counter--;

			if (m_Counter==0)
			{
				timed_command_complete();
				return;
			}

			do_read_id();
		}
		break;

		default:
			break;
	}
}

void i8271_device::do_read()
{
	/* find the sector */
	if (find_sector())
	{
		/* get the sector into the buffer */
		m_floppy[m_drive]->floppy_drive_read_sector_data(m_side, m_data_id, m_pExecutionPhaseData, 1<<(m_ID_N+7));

		/* initialise for reading */
		initialise_execution_phase_read(1<<(m_ID_N+7));

		/* update state - gets first byte and triggers a data request */
		timed_data_request();
		return;
	}
	LOG(("error getting sector data\n"));

	timed_command_complete();
}

void i8271_device::do_read_id()
{
	chrn_id id;

	/* get next id from disc */
	m_floppy[m_drive]->floppy_drive_get_next_id(m_side,&id);

	m_pExecutionPhaseData[0] = id.C;
	m_pExecutionPhaseData[1] = id.H;
	m_pExecutionPhaseData[2] = id.R;
	m_pExecutionPhaseData[3] = id.N;

	initialise_execution_phase_read(4);
}


void i8271_device::do_write()
{
	/* find the sector */
	if (find_sector())
	{
		/* initialise for reading */
		initialise_execution_phase_write(1<<(m_ID_N+7));

		/* update state - gets first byte and triggers a data request */
		timed_data_request();
		return;
	}
	LOG(("error getting sector data\n"));

	timed_command_complete();
}



int i8271_device::find_sector()
{
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
		if (m_floppy[m_drive]->floppy_drive_get_next_id(m_side,&id))
		{
			/* tested on Amstrad CPC - All bytes must match, otherwise
			a NO DATA error is reported */
			if (id.R == m_ID_R)
			{
				/* TODO: Is this correct? What about bad tracks? */
				if (id.C == m_CurrentTrack[m_drive])
				{
					m_data_id = id.data_id;
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
		if (m_floppy[m_drive]->floppy_drive_get_flag_state(FLOPPY_DRIVE_INDEX))
		{
			index_count++;
		}

	}
	while (index_count!=2);

	/* completion type: command/drive error */
	/* completion code: sector not found */
	m_ResultRegister |= (3<<3);

	return 0;
}

void i8271_device::command_execute()
{
	/* clear it = good completion status */
	/* this will be changed if anything bad happens! */
	m_ResultRegister = 0;

	switch (m_Command)
	{
		case I8271_COMMAND_SPECIFY:
		{
			switch (m_CommandParameters[0])
			{
				case 0x0d:
				{
					LOG(("Initialization\n"));
					m_StepRate = m_CommandParameters[1];
					m_HeadSettlingTime = m_CommandParameters[2];
					m_IndexCountBeforeHeadUnload = (m_CommandParameters[3]>>4) & 0x0f;
					m_HeadLoadTime = (m_CommandParameters[3] & 0x0f);
				}
				break;

				case 0x010:
				{
					LOG(("Load bad Tracks Surface 0\n"));
					load_bad_tracks(0);

				}
				break;

				case 0x018:
				{
					LOG(("Load bad Tracks Surface 1\n"));
					load_bad_tracks(1);

				}
				break;
			}

			/* no result */
			command_complete(0,0);
		}
		break;

		case I8271_COMMAND_READ_SPECIAL_REGISTER:
		{
			/* unknown - what is read when a special register that isn't allowed is specified? */
			int data = 0x0ff;

			switch (m_CommandParameters[0])
			{
				case I8271_SPECIAL_REGISTER_MODE_REGISTER:
				{
					data = m_Mode;
				}
				break;

				case I8271_SPECIAL_REGISTER_SURFACE_0_CURRENT_TRACK:
				{
					data = read_current_track(0);

				}
				break;

				case I8271_SPECIAL_REGISTER_SURFACE_1_CURRENT_TRACK:
				{
					data = read_current_track(1);
				}
				break;

				case I8271_SPECIAL_REGISTER_SURFACE_0_BAD_TRACK_1:
				{
					data = read_bad_track(0,1);
				}
				break;

				case I8271_SPECIAL_REGISTER_SURFACE_0_BAD_TRACK_2:
				{
					data = read_bad_track(0,2);
				}
				break;

				case I8271_SPECIAL_REGISTER_SURFACE_1_BAD_TRACK_1:
				{
					data = read_bad_track(1,1);
				}
				break;

				case I8271_SPECIAL_REGISTER_SURFACE_1_BAD_TRACK_2:
				{
					data = read_bad_track(1,2);
				}
				break;

				case I8271_SPECIAL_REGISTER_DRIVE_CONTROL_OUTPUT_PORT:
				{
					FDC_LOG_COMMAND("Read Drive Control Output port\n");

					get_drive();

					/* assumption: select bits reflect the select bits from the previous
					command. i.e. read drive status */
					data = (m_drive_control_output & ~0x0c0) | (m_CommandRegister & 0x0c0);
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


					m_drive_control_input = (1<<6) | (1<<2);

					/* bit 3 = 0 if write protected */
					m_drive_control_input |= m_floppy[m_drive]->floppy_wpt_r() << 3;

					/* bit 1 = 0 if head at track 0 */
					m_drive_control_input |= m_floppy[m_drive]->floppy_tk00_r() << 1;

					/* need to setup this register based on drive selected */
					data = m_drive_control_input;
				}
				break;

			}

			m_ResultRegister = data;

			command_complete(1,0);
		}
		break;


		case I8271_COMMAND_WRITE_SPECIAL_REGISTER:
		{
			switch (m_CommandParameters[0])
			{
				case I8271_SPECIAL_REGISTER_MODE_REGISTER:
				{
					/* TODO: Check bits 6-7 and 5-2 are valid */
					m_Mode = m_CommandParameters[1];

					if (m_Mode & 0x01)
					{
						LOG(("Mode: Non-DMA\n"));
					}
					else
					{
						LOG(("Mode: DMA\n"));
					}

					if (m_Mode & 0x02)
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
					write_current_track(0, m_CommandParameters[1]);
				}
				break;

				case I8271_SPECIAL_REGISTER_SURFACE_1_CURRENT_TRACK:
				{
					LOG(("Surface 1 Current Track\n"));
					write_current_track(1, m_CommandParameters[1]);
				}
				break;

				case I8271_SPECIAL_REGISTER_SURFACE_0_BAD_TRACK_1:
				{
					LOG(("Surface 0 Bad Track 1\n"));
					write_bad_track(0, 1, m_CommandParameters[1]);
				}
				break;

				case I8271_SPECIAL_REGISTER_SURFACE_0_BAD_TRACK_2:
				{
					LOG(("Surface 0 Bad Track 2\n"));
					write_bad_track(0, 2,m_CommandParameters[1]);
				}
				break;

				case I8271_SPECIAL_REGISTER_SURFACE_1_BAD_TRACK_1:
				{
					LOG(("Surface 1 Bad Track 1\n"));


					write_bad_track(1, 1, m_CommandParameters[1]);
				}
				break;

				case I8271_SPECIAL_REGISTER_SURFACE_1_BAD_TRACK_2:
				{
					LOG(("Surface 1 Bad Track 2\n"));

					write_bad_track(1, 2, m_CommandParameters[1]);
				}
				break;

				case I8271_SPECIAL_REGISTER_DRIVE_CONTROL_OUTPUT_PORT:
				{
//                  /* get drive selected */
//                  m_drive = (m_CommandParameters[1]>>6) & 0x03;

					FDC_LOG_COMMAND("Write Drive Control Output port\n");


					if (m_CommandParameters[1] & 0x01)
					{
						LOG(("Write Enable\n"));
					}
					if (m_CommandParameters[1] & 0x02)
					{
						LOG(("Seek/Step\n"));
					}
					if (m_CommandParameters[1] & 0x04)
					{
						LOG(("Direction\n"));
					}
					if (m_CommandParameters[1] & 0x08)
					{
						LOG(("Load Head\n"));
					}
					if (m_CommandParameters[1] & 0x010)
					{
						LOG(("Low head current\n"));
					}
					if (m_CommandParameters[1] & 0x020)
					{
						LOG(("Write Fault Reset\n"));
					}

					LOG(("Select %02x\n", (m_CommandParameters[1] & 0x0c0)>>6));

					/* get drive */
					get_drive();

					/* on bbc dfs 09 this is the side select output */
					m_side = (m_CommandParameters[1]>>5) & 0x01;

					/* load head - on mini-sized drives this turns on the disc motor,
					on standard-sized drives this loads the head and turns the motor on */
					m_floppy[m_drive]->floppy_mon_w(!BIT(m_CommandParameters[1], 3));
					m_floppy[m_drive]->floppy_drive_set_ready_state(1, 1);

					/* step pin changed? if so perform a step in the direction indicated */
					if (((m_drive_control_output^m_CommandParameters[1]) & (1<<1))!=0)
					{
						/* step pin changed state? */

						if (BIT(m_CommandParameters[1], 1))
						{
							signed int signed_tracks;

							if (BIT(m_CommandParameters[1], 2))
							{
								signed_tracks = 1;
							}
							else
							{
								signed_tracks = -1;
							}

							m_floppy[m_drive]->floppy_drive_seek(signed_tracks);
						}
					}

					m_drive_control_output = m_CommandParameters[1];


				}
				break;

				case I8271_SPECIAL_REGISTER_DRIVE_CONTROL_INPUT_PORT:
				{
					FDC_LOG_COMMAND("Write Drive Control Input port\n");

					//                  m_drive_control_input = m_CommandParameters[1];
				}
				break;

			}

			/* write doesn't supply a result */
			command_complete(0,0);
		}
		break;

		case I8271_COMMAND_READ_DRIVE_STATUS:
		{
			unsigned char status;

			get_drive();

			/* no write fault */
			status = 0;

			status |= (1<<2) | (1<<6);

			/* these two do not appear to be set at all! ?? */

			if (m_floppy[0])
			{
				if (m_floppy[0]->floppy_drive_get_flag_state(FLOPPY_DRIVE_READY))
				{
					status |= (1 << 2);
				}
			}

			if (m_floppy[1])
			{
				if (m_floppy[1]->floppy_drive_get_flag_state(FLOPPY_DRIVE_READY))
				{
					status |= (1 << 6);
				}
			}

			/* bit 3 = 1 if write protected */
			status |= !m_floppy[m_drive]->floppy_wpt_r() << 3;

			/* bit 1 = 1 if head at track 0 */
			status |= !m_floppy[m_drive]->floppy_tk00_r() << 1;

			m_ResultRegister = status;
			command_complete(1,0);

		}
		break;

		case I8271_COMMAND_SEEK:
		{
			get_drive();

			seek_to_track(m_CommandParameters[0]);

			/* check for bad seek */
			timed_command_complete();

		}
		break;

		case I8271_COMMAND_READ_DATA_MULTI_RECORD:
		{
			/* N value as stored in ID field */
			m_ID_N = (m_CommandParameters[2]>>5) & 0x07;

			/* starting sector id */
			m_ID_R = m_CommandParameters[1];

			/* number of sectors to transfer */
			m_Counter = m_CommandParameters[2] & 0x01f;


			FDC_LOG_COMMAND("READ DATA MULTI RECORD");

			LOG(("Sector Count: %02x\n", m_Counter));
			LOG(("Track: %02x\n",m_CommandParameters[0]));
			LOG(("Sector: %02x\n", m_CommandParameters[1]));
			LOG(("Sector Length: %02x bytes\n", 1<<(m_ID_N+7)));

			get_drive();

			if (!m_floppy[m_drive]->floppy_drive_get_flag_state(FLOPPY_DRIVE_READY))
			{
				/* Completion type: operation intervention probably required for recovery */
				/* Completion code: Drive not ready */
				m_ResultRegister = (2<<3);
				timed_command_complete();
			}
			else
			{
				seek_to_track(m_CommandParameters[0]);


				do_read();
			}

		}
		break;

		case I8271_COMMAND_READ_DATA_SINGLE_RECORD:
		{
			FDC_LOG_COMMAND("READ DATA SINGLE RECORD");

			m_ID_N = 0;
			m_Counter = 1;
			m_ID_R = m_CommandParameters[1];

			LOG(("Sector Count: %02x\n", m_Counter));
			LOG(("Track: %02x\n",m_CommandParameters[0]));
			LOG(("Sector: %02x\n", m_CommandParameters[1]));
			LOG(("Sector Length: %02x bytes\n", 1<<(m_ID_N+7)));

			get_drive();

			if (!m_floppy[m_drive]->floppy_drive_get_flag_state(FLOPPY_DRIVE_READY))
			{
				/* Completion type: operation intervention probably required for recovery */
				/* Completion code: Drive not ready */
				m_ResultRegister = (2<<3);
				timed_command_complete();
			}
			else
			{
				seek_to_track(m_CommandParameters[0]);

				do_read();
			}

		}
		break;

		case I8271_COMMAND_WRITE_DATA_MULTI_RECORD:
		{
			/* N value as stored in ID field */
			m_ID_N = (m_CommandParameters[2]>>5) & 0x07;

			/* starting sector id */
			m_ID_R = m_CommandParameters[1];

			/* number of sectors to transfer */
			m_Counter = m_CommandParameters[2] & 0x01f;

			FDC_LOG_COMMAND("READ DATA MULTI RECORD");

			LOG(("Sector Count: %02x\n", m_Counter));
			LOG(("Track: %02x\n",m_CommandParameters[0]));
			LOG(("Sector: %02x\n", m_CommandParameters[1]));
			LOG(("Sector Length: %02x bytes\n", 1<<(m_ID_N+7)));

			get_drive();

			m_drive_control_output &=~1;

			if (!m_floppy[m_drive]->floppy_drive_get_flag_state(FLOPPY_DRIVE_READY))
			{
				/* Completion type: operation intervention probably required for recovery */
				/* Completion code: Drive not ready */
				m_ResultRegister = (2<<3);
				timed_command_complete();
			}
			else
			{
				if (m_floppy[m_drive]->floppy_wpt_r() == CLEAR_LINE)
				{
					/* Completion type: operation intervention probably required for recovery */
					/* Completion code: Drive write protected */
					m_ResultRegister = (2<<3) | (1<<1);
					timed_command_complete();
				}
				else
				{
					m_drive_control_output |=1;

					seek_to_track(m_CommandParameters[0]);

					do_write();
				}
			}
		}
		break;

		case I8271_COMMAND_WRITE_DATA_SINGLE_RECORD:
		{
			FDC_LOG_COMMAND("WRITE DATA SINGLE RECORD");

			m_ID_N = 0;
			m_Counter = 1;
			m_ID_R = m_CommandParameters[1];


			LOG(("Sector Count: %02x\n", m_Counter));
			LOG(("Track: %02x\n",m_CommandParameters[0]));
			LOG(("Sector: %02x\n", m_CommandParameters[1]));
			LOG(("Sector Length: %02x bytes\n", 1<<(m_ID_N+7)));
			get_drive();

			m_drive_control_output &=~1;

			if (!m_floppy[m_drive]->floppy_drive_get_flag_state(FLOPPY_DRIVE_READY))
			{
				/* Completion type: operation intervention probably required for recovery */
				/* Completion code: Drive not ready */
				m_ResultRegister = (2<<3);
				timed_command_complete();
			}
			else
			{
				if (m_floppy[m_drive]->floppy_wpt_r() == CLEAR_LINE)
				{
					/* Completion type: operation intervention probably required for recovery */
					/* Completion code: Drive write protected */
					m_ResultRegister = (2<<3) | (1<<1);
					timed_command_complete();
				}
				else
				{
					m_drive_control_output |=1;

					seek_to_track(m_CommandParameters[0]);

					do_write();
				}
			}

		}
		break;


		case I8271_COMMAND_READ_ID:
		{
			FDC_LOG_COMMAND("READ ID");

			LOG(("Track: %02x\n",m_CommandParameters[0]));
			LOG(("ID Field Count: %02x\n", m_CommandParameters[2]));

			get_drive();

			if (!m_floppy[m_drive]->floppy_drive_get_flag_state(FLOPPY_DRIVE_READY))
			{
				/* Completion type: operation intervention probably required for recovery */
				/* Completion code: Drive not ready */
				m_ResultRegister = (2<<3);
				timed_command_complete();
			}
			else
			{
				m_Counter = m_CommandParameters[2];

				seek_to_track(m_CommandParameters[0]);

				do_read_id();
			}
		}
		break;

		default:
			LOG(("ERROR Unrecognised Command\n"));
			break;
	}
}



WRITE8_MEMBER(i8271_device::write)
{
	switch (offset & 3)
	{
		case 0:
		{
			LOG(("I8271 W Command Register: %02x\n", data));

			m_CommandRegister = data;
			m_Command = m_CommandRegister & 0x03f;

			m_StatusRegister |= I8271_STATUS_COMMAND_BUSY | I8271_STATUS_COMMAND_FULL;
			m_StatusRegister &= ~I8271_STATUS_PARAMETER_FULL | I8271_STATUS_RESULT_FULL;
			m_ParameterCountWritten = 0;

			switch (m_Command)
			{
				case I8271_COMMAND_SPECIFY:
				{
					FDC_LOG_COMMAND("SPECIFY");

					m_ParameterCount = 4;
				}
				break;

				case I8271_COMMAND_SEEK:
				{
					FDC_LOG_COMMAND("SEEK");

					m_ParameterCount = 1;
				}
				break;

				case I8271_COMMAND_READ_DRIVE_STATUS:
				{
					FDC_LOG_COMMAND("READ DRIVE STATUS");

					m_ParameterCount = 0;
				}
				break;

				case I8271_COMMAND_READ_SPECIAL_REGISTER:
				{
					FDC_LOG_COMMAND("READ SPECIAL REGISTER");

					m_ParameterCount = 1;
				}
				break;

				case I8271_COMMAND_WRITE_SPECIAL_REGISTER:
				{
					FDC_LOG_COMMAND("WRITE SPECIAL REGISTER");

					m_ParameterCount = 2;
				}
				break;

				case I8271_COMMAND_FORMAT:
				{
					m_ParameterCount = 5;
				}
				break;

				case I8271_COMMAND_READ_ID:
				{
					m_ParameterCount = 3;

				}
				break;


				case I8271_COMMAND_READ_DATA_SINGLE_RECORD:
				case I8271_COMMAND_READ_DATA_AND_DELETED_DATA_SINGLE_RECORD:
				case I8271_COMMAND_WRITE_DATA_SINGLE_RECORD:
				case I8271_COMMAND_WRITE_DELETED_DATA_SINGLE_RECORD:
				case I8271_COMMAND_VERIFY_DATA_AND_DELETED_DATA_SINGLE_RECORD:
				{
					m_ParameterCount = 2;
				}
				break;

				case I8271_COMMAND_READ_DATA_MULTI_RECORD:
				case I8271_COMMAND_READ_DATA_AND_DELETED_DATA_MULTI_RECORD:
				case I8271_COMMAND_WRITE_DATA_MULTI_RECORD:
				case I8271_COMMAND_WRITE_DELETED_DATA_MULTI_RECORD:
				case I8271_COMMAND_VERIFY_DATA_AND_DELETED_DATA_MULTI_RECORD:
				{
					m_ParameterCount = 3;
				}
				break;

				case I8271_COMMAND_SCAN_DATA:
				case I8271_COMMAND_SCAN_DATA_AND_DELETED_DATA:
				{
					m_ParameterCount = 5;
				}
				break;






			}

			check_all_parameters_written();
		}
		break;

		case 1:
		{
			LOG(("I8271 W Parameter Register: %02x\n",data));
			m_ParameterRegister = data;

			if (m_ParameterCount!=0)
			{
				m_CommandParameters[m_ParameterCountWritten] = data;
				m_ParameterCountWritten++;
			}

			check_all_parameters_written();
		}
		break;

		case 2:
		{
			LOG(("I8271 W Reset Register: %02x\n", data));
			if (((data ^ m_ResetRegister) & 0x01)!=0)
			{
				if ((data & 0x01)==0)
				{
					reset();
				}
			}

			m_ResetRegister = data;


		}
		break;

		default:
			break;
	}
}

READ8_MEMBER(i8271_device::read)
{
	switch (offset & 3)
	{
		case 0:
		{
			/* bit 1,0 are zero other bits contain status data */
			m_StatusRegister &= ~0x03;
			LOG(("I8271 R Status Register: %02x\n",m_StatusRegister));
			return m_StatusRegister;
		}

		case 1:
		{
			if ((m_StatusRegister & I8271_STATUS_COMMAND_BUSY)==0)
			{
				/* clear IRQ */
				set_irq_state(0);

				m_StatusRegister &= ~I8271_STATUS_RESULT_FULL;
				LOG(("I8271 R Result Register %02x\n",m_ResultRegister));
				return m_ResultRegister;
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
READ8_MEMBER(i8271_device::dack_r)
{
	return data_r(space, offset);
}

/* to be completed! */
WRITE8_MEMBER(i8271_device::dack_w)
{
	data_w(space, offset, data);
}

READ8_MEMBER(i8271_device::data_r)
{
	clear_data_request();

	update_state();

	//  logerror("I8271 R data: %02x\n",m_data);


	return m_data;
}

WRITE8_MEMBER(i8271_device::data_w)
{
	m_data = data;

//    logerror("I8271 W data: %02x\n",m_data);

	clear_data_request();

	update_state();
}
