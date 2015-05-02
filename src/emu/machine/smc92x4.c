/*
    HDC9224 and HDC9234 Hard and Floppy Disk Controller

    This controller handles MFM and FM encoded floppy disks and hard disks.
    The SMC9224 is used in some DEC systems.  The HDC9234 is used in the
    Myarc HFDC card for the TI99/4a.  The main difference between the two
    chips is the way the ECC bytes are computed; there are differences in
    the way seek times are computed, too.

    References:
    * SMC HDC9234 preliminary data book (1988)

    Michael Zapf, April 2010

    First version by Raphael Nabet, 2003
*/

#include "emu.h"
#include "smc92x4.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/*
    Definition of bits in the status register
*/
#define ST_INTPEND  0x80        /* interrupt pending */
#define ST_DMAREQ   0x40        /* DMA request */
#define ST_DONE     0x20        /* command done */
#define ST_TERMCOD  0x18        /* termination code (see below) */
#define ST_RDYCHNG  0x04        /* ready change */
#define ST_OVRUN    0x02        /* overrun/underrun */
#define ST_BADSECT  0x01        /* bad sector */

/*
    Definition of the termination codes (INT_STATUS)
*/
#define ST_TC_SUCCESS   0x00    /* Successful completion */
#define ST_TC_RDIDERR   0x08    /* Error in READ-ID sequence */
#define ST_TC_SEEKERR   0x10    /* Error in SEEK sequence */
#define ST_TC_DATAERR   0x18    /* Error in DATA-TRANSFER seq. */


/*
    Definition of bits in the Termination-Conditions register
*/
#define TC_CRCPRE   0x80        /* CRC register preset, must be 1 */
#define TC_UNUSED   0x40        /* bit 6 is not used and must be 0 */
#define TC_INTDONE  0x20        /* interrupt on done */
#define TC_TDELDAT  0x10        /* terminate on deleted data */
#define TC_TDSTAT3  0x08        /* terminate on drive status 3 change */
#define TC_TWPROT   0x04        /* terminate on write-protect (FDD only) */
#define TC_INTRDCH  0x02        /* interrupt on ready change (FDD only) */
#define TC_TWRFLT   0x01        /* interrupt on write-fault (HDD only) */

/*
    Definition of bits in the Chip-Status register
*/
#define CS_RETREQ   0x80        /* retry required */
#define CS_ECCATT   0x40        /* ECC correction attempted */
#define CS_CRCERR   0x20        /* ECC/CRC error */
#define CS_DELDATA  0x10        /* deleted data mark */
#define CS_SYNCERR  0x08        /* synchronization error */
#define CS_COMPERR  0x04        /* compare error */
#define CS_PRESDRV  0x03        /* present drive selected */

/*
    Definition of bits in the Mode register
*/
#define MO_TYPE     0x80        /* Hard disk (1) or floppy (0) */
#define MO_CRCECC   0x60        /* Values for CRC/ECC handling */
#define MO_DENSITY  0x10        /* FM = 1; MFM = 0 */
#define MO_UNUSED   0x08        /* Unused, 0 */
#define MO_STEPRATE 0x07        /* Step rates */

/*
    hfdc state structure

    status
    ab7     ecc error
    ab6     index pulse
    ab5     seek complete
    ab4     track 0
    ab3     user-defined
    ab2     write-protected
    ab1     drive ready
    ab0     write fault

    output1
    ab7     drive select 3
    ab6     drive select 2
    ab5     drive select 1
    ab4     drive select 0
    ab3     programmable outputs
    ab2     programmable outputs
    ab1     programmable outputs
    ab0     programmable outputs

    output2
    ab7     drive select 3* (active low, used for tape operations)
    ab6     reduce write current
    ab5     step direction
    ab4     step pulse
    ab3     desired head 3
    ab2     desired head 2
    ab1     desired head 1
    ab0     desired head 0
*/

#define OUT1_DRVSEL3    0x80
#define OUT1_DRVSEL2    0x40
#define OUT1_DRVSEL1    0x20
#define OUT1_DRVSEL0    0x10
#define OUT2_DRVSEL3_   0x80
#define OUT2_REDWRT 0x40
#define OUT2_STEPDIR    0x20
#define OUT2_STEPPULSE  0x10
#define OUT2_HEADSEL3   0x08
#define OUT2_HEADSEL2   0x04
#define OUT2_HEADSEL1   0x02
#define OUT2_HEADSEL0   0x01

#define DRIVE_TYPE  0x03
#define TYPE_AT     0x00
#define TYPE_FLOPPY     0x02  /* for testing on any floppy type */
#define TYPE_FLOPPY8    0x02
#define TYPE_FLOPPY5    0x03

#define MAX_SECTOR_LEN 256

#define FORMAT_LONG false

#define ERROR   0
#define DONE    1
#define AGAIN   2
#define UNDEF   3

enum
{
	DATA_TIMER,
	READ_TIMER,
	WRITE_TIMER,
	SEEK_TIMER,
	TRACK_TIMER
};

/*
    Step rates in microseconds for MFM. This is set in the mode register,
    bits 0-2. Single density doubles all values.
*/
static const int step_hd[]  = { 22, 50, 100, 200, 400, 800, 1600, 3200 };
static const int step_flop8[]   = { 218, 500, 1000, 2000, 4000, 8000, 16000, 32000 };
static const int step_flop5[]   = { 436, 1000, 2000, 4000, 8000, 16000, 32000, 64000 };

/*
    0.2 seconds for a revolution (300 rpm), +50% average waiting for index
    hole. Should be properly done with the index hole detection; we're
    simulating this with a timer for now.
*/
#define TRACKTIME_FLOPPY 300000
#define TRACKTIME_HD 1000

/*
    Register names of the HDC. The left part is the set of write registers,
    while the right part are the read registers.
*/
enum
{
	DMA7_0=0,
	DMA15_8=1,
	DMA23_16=2,
	DESIRED_SECTOR=3,
	DESIRED_HEAD=4,     CURRENT_HEAD=4,
	DESIRED_CYLINDER=5,     CURRENT_CYLINDER=5,
	SECTOR_COUNT=6,     CURRENT_IDENT=6,
	RETRY_COUNT=7,      TEMP_STORAGE2=7,
	MODE=8,         CHIP_STATUS=8,
	INT_COMM_TERM=9,    DRIVE_STATUS=9,
	DATA_DELAY=10,      DATA=10,
	COMMAND=11,         INT_STATUS=11
};

#define TRKSIZE_DD      6144
#define TRKSIZE_SD      3172

#define VERBOSE 1
#define LOG logerror

smc92x4_device::smc92x4_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: device_t(mconfig, SMC92X4, "SMC 9224/9234 Hard/Floppy Disk Controller", tag, owner, clock, "smc92x4", __FILE__),
	m_out_intrq(*this),
	m_out_dip(*this),
	m_out_auxbus(*this),
	m_in_auxbus(*this),
	m_in_dma(*this),
	m_out_dma(*this),
	m_full_track_layout(FALSE)
{
}

int smc92x4_device::image_is_single_density()
{
	floppy_image_legacy *image = m_drive->flopimg_get_image();
	return (floppy_get_track_size(image, 0, 0)<4000);
}

bool smc92x4_device::in_single_density_mode()
{
	return ((m_register_w[MODE]&MO_DENSITY)!=0);
}

void smc92x4_device::copyid(chrn_id id1, chrn_id_hd *id2)
{
	id2->C = id1.C & 0xff;
	id2->H = id1.H;
	id2->R = id1.R;
	id2->N = id1.N;
	id2->data_id = id1.data_id;
	id2->flags = id1.flags;
}

/*
    Set IRQ
*/
void smc92x4_device::set_interrupt()
{
	if ((m_register_r[INT_STATUS] & ST_INTPEND) == 0)
	{
		m_register_r[INT_STATUS] |= ST_INTPEND;
		m_out_intrq(ASSERT_LINE);
	}
}

/*
    Assert Command Done status bit, triggering interrupts as needed
*/
void smc92x4_device::set_command_done(int flags)
{
	//assert(! (m_status & ST_DONE))
	if (VERBOSE>7) LOG("smc92x4 command %02x done, flags=%02x\n", m_command, flags);

	m_register_r[INT_STATUS] |= ST_DONE;
	m_register_r[INT_STATUS] &= ~ST_TERMCOD; /* clear the previously set flags */
	m_register_r[INT_STATUS] |= flags;

	/* sm92x4 spec, p. 6 */
	if (m_register_w[INT_COMM_TERM] & TC_INTDONE)
		set_interrupt();
}

/*
    Preserve previously set termination code
*/
void smc92x4_device::set_command_done()
{
	//assert(! (m_status & ST_DONE))
	if (VERBOSE>7) LOG("smc92x4 command %02x done\n", m_command);

	m_register_r[INT_STATUS] |= ST_DONE;

	/* sm92x4 spec, p. 6 */
	if (m_register_w[INT_COMM_TERM] & TC_INTDONE)
		set_interrupt();
}

/*
    Clear IRQ
*/
void smc92x4_device::clear_interrupt()
{
	if ((m_register_r[INT_STATUS] & ST_INTPEND) != 0)
		m_out_intrq(CLEAR_LINE);
}

/*
    Sets the DMA address on the external counter. This counter is attached
    to the auxiliary bus on the PCB.
*/
void smc92x4_device::set_dma_address(int pos2316, int pos1508, int pos0700)
{
	m_out_auxbus((offs_t)OUTPUT_DMA_ADDR, m_register_r[pos2316]);
	m_out_auxbus((offs_t)OUTPUT_DMA_ADDR, m_register_r[pos1508]);
	m_out_auxbus((offs_t)OUTPUT_DMA_ADDR, m_register_r[pos0700]);
}

void smc92x4_device::dma_add_offset(int offset)
{
	int dma_address = (m_register_w[DMA23_16]<<16) + (m_register_w[DMA15_8]<<8) + m_register_w[DMA7_0];
	dma_address += offset;

	m_register_w[DMA23_16] = m_register_r[DMA23_16] = (dma_address & 0xff0000)>>16;
	m_register_w[DMA15_8]  = m_register_r[DMA15_8]  = (dma_address & 0x00ff00)>>8;
	m_register_w[DMA7_0]   = m_register_r[DMA7_0]   = (dma_address & 0x0000ff);
}

/*
    Get the state from outside and latch it in the register.
    There should be a bus driver on the PCB which provides the signals from
    both the hard and floppy drives during S0=S1=0 and STB*=0 times via the
    auxiliary bus.
*/
void smc92x4_device::sync_status_in()
{
	UINT8 prev;
	prev = m_register_r[DRIVE_STATUS];
	m_register_r[DRIVE_STATUS] = m_in_auxbus(0);

	/* Raise interrupt if ready changes. TODO: Check this more closely. */
	if (((m_register_r[DRIVE_STATUS] & DS_READY) != (prev & DS_READY))
		& (m_register_r[INT_STATUS] & ST_RDYCHNG))
	{
		set_interrupt();
	}
}

/*
    Push the output registers over the auxiliary bus. It is expected that
    the PCB contains latches to store the values.
*/
void smc92x4_device::sync_latches_out()
{
	m_output1 = (m_output1 & 0xf0) | (m_register_w[RETRY_COUNT]&0x0f);
	m_out_auxbus((offs_t)OUTPUT_OUTPUT1, m_output1);
	m_out_auxbus((offs_t)OUTPUT_OUTPUT2, m_output2);
}

/*************************************************************
    Timed requests and callbacks
*************************************************************/
#if 0
/* setup a timed data request - data request will be triggered in a few usecs time */
void smc92x4_device::timed_data_request()
{
	int time = in_single_density_mode()? 128 : 32;

	if (!m_use_real_timing)
		time = 1;

	/* set new timer */
	m_timer_data->adjust(attotime::from_usec(time));
}
#endif

/* setup a timed read sector - read sector will be triggered in a few usecs time */
void smc92x4_device::timed_sector_read_request()
{
	int time=0;

	// set new timer
	// Average time from sector to sector.
	if (m_selected_drive_type & TYPE_FLOPPY)
		time = (in_single_density_mode())? 30000 : 15000;
	else
		time = 1000;

	if (!m_use_real_timing)
		time = 1;

	m_timer_rs->adjust(attotime::from_usec(time));
	m_to_be_continued = true;
}

/* setup a timed write sector - write sector will be triggered in a few usecs time */
void smc92x4_device::timed_sector_write_request()
{
	int time=0;

	/* Average time from sector to sector. */
	if (m_selected_drive_type & TYPE_FLOPPY)
		time = (in_single_density_mode())? 30000 : 15000;
	else
		time = 1000;

	if (!m_use_real_timing)
		time = 1;

	m_timer_ws->adjust(attotime::from_usec(time));
	m_to_be_continued = true;
}

/*
    Set up a timed track read/write
*/
void smc92x4_device::timed_track_request()
{
	int time = 0;

	if (m_selected_drive_type & TYPE_FLOPPY)
		time = TRACKTIME_FLOPPY;
	else
		time = TRACKTIME_HD;

	if (!m_use_real_timing)
		time = 1;

	m_timer_track->adjust(attotime::from_usec(time));

	m_to_be_continued = true;
}

/*
    Set up a timed track seek
*/
void smc92x4_device::timed_seek_request()
{
	int time = 0;

	int index = m_register_w[MODE] & MO_STEPRATE;
	int fm = in_single_density_mode();

	/* Get seek time. */
	if ((m_selected_drive_type & DRIVE_TYPE) == TYPE_FLOPPY8)
		time = step_flop8[index];

	else if ((m_selected_drive_type & DRIVE_TYPE) == TYPE_FLOPPY5)
		time = step_flop5[index];
	else
		time = step_hd[index];

	if (fm)
		time = time * 2;

	if (!m_use_real_timing)
	{
		if (VERBOSE>5) LOG("smc92x4 info: Disk access without delays\n");
		time = 1;
	}
	m_timer_seek->adjust(attotime::from_usec(time));
	m_to_be_continued = true;
}

void smc92x4_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	int transfer_enabled;
	int deldata;
	int redcur;
	int precomp;
	int write_long;

	switch (id)
	{
	case DATA_TIMER:
		// Not implemented yet
		break;
	case READ_TIMER:
		transfer_enabled = m_command & 0x01;
		// Now read the sector.
		data_transfer_read(m_recent_id, transfer_enabled);
		sync_status_in();
		break;
	case WRITE_TIMER:
		// The specification is contraditory here :-(
		// For formatting, bit 0x10 = 1 means normal, 0 means deleted
		// while for writing sectors it is the opposite?!
		// We believe the existing drivers which all set the bit for normal writing.
		deldata = 0x10 - (m_command & 0x10);
		redcur =  m_command & 0x08;
		precomp = m_command & 0x07;
		write_long = ((m_register_w[MODE]& MO_CRCECC)==0x40);

		if (deldata)
			if (VERBOSE>0) LOG("smc92x4 warn: Write deleted data mark not supported. Writing normal mark.\n");

		// Now write the sector.
		data_transfer_write(m_recent_id, deldata, redcur, precomp, write_long);
		sync_status_in();
		break;
	case SEEK_TIMER:
		// Callback for seek request.
		/*  int buffered = ((m_register_w[MODE] & MO_STEPRATE)==0); */
		/*  int buffered = (m_command & 0x01); */

		if (m_selected_drive_type & TYPE_FLOPPY)
		{
			if (m_drive==NULL)
			{
				if (VERBOSE>0) LOG("smc92x4 error: seek callback: no floppy\n");
				m_register_r[INT_STATUS] |= ST_TC_SEEKERR;
			}
			else
			{
				if (VERBOSE>5) LOG("smc92x4 step %s direction %d\n", m_drive->tag(), m_step_direction);
				m_drive->floppy_drive_seek(m_step_direction);
			}
		}
		else
		{
			if (VERBOSE>6) LOG("smc92x4 step harddisk direction %d\n", m_step_direction);
			m_harddisk->seek(m_step_direction);
		}
		sync_status_in();
		break;
	case TRACK_TIMER:
		// Callback for track access. Nothing interesting here, just used for
		// delaying.
		break;
	}
	process_after_callback();
}


/*********************************************************************
    Common functions
*********************************************************************/

/*
    Calculate the ident byte from the cylinder. The specification does not
    define idents beyond cylinder 1023, but formatting programs seem to
    continue with 0xfd for cylinders between 1024 and 2047.
*/
UINT8 smc92x4_device::cylinder_to_ident(int cylinder)
{
	if (cylinder < 256) return 0xfe;
	if (cylinder < 512) return 0xff;
	if (cylinder < 768) return 0xfc;
	if (cylinder < 1024) return 0xfd;
	return 0xfd;
}

/*
    Calculate the offset from the ident. This is only useful for AT drives;
    these drives cannot have more than 1024 cylinders.
*/
int smc92x4_device::ident_to_cylinder(UINT8 ident)
{
	switch (ident)
	{
	case 0xfe: return 0;
	case 0xff: return 256;
	case 0xfc: return 512;
	case 0xfd: return 768;
	default: return -1;
	}
}

/*
    Common function to set the read registers from the recently read id.
*/
void smc92x4_device::update_id_regs(chrn_id_hd id)
{
	// Flags for current head register. Note that the sizes are not in
	// sequence (128, 256, 512, 1024). This is only interesting for AT
	// mode.
	static const UINT8 sizeflag[] = { 0x60, 0x00, 0x20, 0x40 };

	m_register_r[CURRENT_CYLINDER] = id.C & 0xff;
	m_register_r[CURRENT_HEAD] = id.H & 0x0f;

	if (id.flags & BAD_SECTOR)
		m_register_r[CURRENT_HEAD] |= 0x80;

	if ((m_selected_drive_type & DRIVE_TYPE) == TYPE_AT)
		m_register_r[CURRENT_HEAD] |= sizeflag[id.N];
	else
		m_register_r[CURRENT_HEAD] |= ((id.C & 0x700)>>4);

	m_register_r[CURRENT_IDENT] = cylinder_to_ident(id.C);
}

/*
    Common procedure: read_id_field (as described in the specification)
*/
void smc92x4_device::read_id_field(int *steps, int *direction, chrn_id_hd *id)
{
	int des_cylinder, cur_cylinder;
	bool found = false;

	sync_latches_out();
	sync_status_in();

	// Set command termination code. The error code is set first, and
	// on success, it is cleared.
	m_register_r[INT_STATUS] |= ST_TC_RDIDERR;

	/* Try to find an ID field. */
	if (m_selected_drive_type & TYPE_FLOPPY)
	{
		chrn_id idflop;
		if (m_drive->flopimg_get_image() == NULL)
		{
			if (VERBOSE>2) LOG("smc92x4 warn: No disk in drive\n");
			m_register_r[CHIP_STATUS] |= CS_SYNCERR;
			return;
		}

		/* Check whether image and controller are set to the same density. */
		if ((image_is_single_density() && in_single_density_mode())
			|| (!image_is_single_density() && !in_single_density_mode()))
		{
			found = m_drive->floppy_drive_get_next_id(m_register_w[DESIRED_HEAD]&0x0f, &idflop);
			copyid(idflop, id); /* Need to use bigger values for HD, but we don't use separate variables here */
		}
		else
		{
			if (VERBOSE>2) LOG("smc92x4 warn: Controller and medium density do not match.\n");
		}
		sync_status_in();

		if (!found)
		{
			if (VERBOSE>1) LOG("smc92x4 error: read_id_field (floppy): sync error\n");
			m_register_r[CHIP_STATUS] |= CS_SYNCERR;
			return;
		}
	}
	else
	{
		m_harddisk->get_next_id(m_register_w[DESIRED_HEAD]&0x0f, id);
		sync_status_in();
		if (!(m_register_r[DRIVE_STATUS]& DS_READY))
		{
			if (VERBOSE>1) LOG("smc92x4 error: read_id_field (harddisk): sync error\n");
			m_register_r[CHIP_STATUS] |= CS_SYNCERR;
			return;
		}
	}

	m_register_r[CHIP_STATUS] &= ~CS_SYNCERR;

	/* Update the registers. */
	update_id_regs(*id);

	if (id->flags & BAD_CRC)
	{
		m_register_r[CHIP_STATUS] |= CS_CRCERR;
		return;
	}

	/* Calculate the steps. */
	if ((m_selected_drive_type & DRIVE_TYPE) == TYPE_AT)
	{
		/* Note: the spec says CURRENT_REGISTER, but that seems wrong. */
		des_cylinder = ((m_register_r[DATA] & 0x03)<<8) | m_register_w[DESIRED_CYLINDER];
		cur_cylinder = ident_to_cylinder(m_register_r[CURRENT_IDENT]) + m_register_r[CURRENT_CYLINDER];
	}
	else
	{
		des_cylinder = ((m_register_w[DESIRED_HEAD] & 0x70)<<4) | m_register_w[DESIRED_CYLINDER];
		cur_cylinder = ((m_register_r[CURRENT_HEAD] & 0x70)<<4) | m_register_r[CURRENT_CYLINDER];
	}

	if (des_cylinder >= cur_cylinder)
	{
		*steps = des_cylinder - cur_cylinder;
		*direction = +1;
	}
	else
	{
		*steps = cur_cylinder - des_cylinder;
		*direction = -1;
	}

	if (VERBOSE>6) LOG("smc92x4 seek required: %d steps\n", *steps);

	m_register_r[INT_STATUS] &= ~ST_TC_RDIDERR;
}

/*
    Common procedure: verify (as described in the specification)
*/
int smc92x4_device::verify(chrn_id_hd *id, bool check_sector)
{
	int maxtry = 132;  /* approx. 33792/(256*32) */
	int pass = 0;
	int found = false;
	int foundsect = false;
	int des_cylinder = 0;

	// Set command termination code. The error code is set first, and
	// on success, it is cleared.
	m_register_r[INT_STATUS] |= ST_TC_SEEKERR;

	while (pass < maxtry && !foundsect)
	{
		pass++;
		/* Try to find an ID field. */
		if (m_selected_drive_type & TYPE_FLOPPY)
		{
			chrn_id idflop;
			found = m_drive->floppy_drive_get_next_id(m_register_w[DESIRED_HEAD]&0x0f, &idflop);
			copyid(idflop, id);
			if (/* pass==1 && */!found)
			{
				m_register_r[CHIP_STATUS] |= CS_SYNCERR;
				if (VERBOSE>1) LOG("smc92x4 error: verify (floppy): sync error\n");
				return ERROR;
			}
		}
		else
		{
			m_harddisk->get_next_id(m_register_w[DESIRED_HEAD]&0x0f, id);
			sync_status_in();
			if (!(m_register_r[DRIVE_STATUS]& DS_READY))
			{
				if (VERBOSE>1) LOG("smc92x4 error: verify (harddisk): sync error\n");
				m_register_r[CHIP_STATUS] |= CS_SYNCERR;
				return ERROR;
			}
		}

		m_register_r[CHIP_STATUS] &= ~CS_SYNCERR;

		/* Compare with the desired sector ID. */
		if ((m_selected_drive_type & DRIVE_TYPE) == TYPE_AT)
		{
			/* Note: the spec says CURRENT_CYLINDER, but that seems wrong. */
			des_cylinder = ((m_register_r[DATA] & 0x03)<<8) | m_register_w[DESIRED_CYLINDER];
		}
		else
		{
			des_cylinder = ((m_register_w[DESIRED_HEAD] & 0x70)<<4) | m_register_w[DESIRED_CYLINDER];
		}
		if (VERBOSE>6) LOG("smc92x4 check id: current = (%d,%d,%d), required = (%d,%d,%d)\n", id->C, id->H, id->R, des_cylinder, m_register_w[DESIRED_HEAD] & 0x0f, m_register_w[DESIRED_SECTOR]);
		if ((des_cylinder == id->C)
			&& ((m_register_w[DESIRED_HEAD] & 0x0f) == id->H))
		{
			if (!check_sector ||  (m_register_w[DESIRED_SECTOR] == id->R))
				foundsect = true;
		}
	}
	if (!foundsect)
	{
		m_register_r[CHIP_STATUS] |= CS_COMPERR;
		if (VERBOSE>1) LOG("smc92x4 error: verify: sector not found, seek error (desired cyl=%d/sec=%d, current cyl=%d/sec=%d)\n", des_cylinder, m_register_w[DESIRED_SECTOR], id->C, id->R);
		return ERROR;
	}

	m_register_r[INT_STATUS] &= ~ST_TC_SEEKERR;
	return DONE;
}

/*
    Common procedure: data_transfer(read) (as described in the specification)
*/
void smc92x4_device::data_transfer_read(chrn_id_hd id, int transfer_enable)
{
	int i, retry, sector_len;

	int sector_data_id;
	dynamic_buffer buf;

	sync_latches_out();
	sync_status_in();

	// Set command termination code. The error code is set first, and
	// on success, it is cleared.
	m_register_r[INT_STATUS] |= ST_TC_DATAERR;

	// Save the value. Note that the retry count is only in the first four
	// bits, and it is stored in one's complement. In this implementation
	// we don't work with it.
	retry = m_register_w[RETRY_COUNT];

	/* We are already at the correct sector (found by the verify sequence) */

	/* Find the data sync mark. Easy, we assume it has been found. */

	if (id.flags & ID_FLAG_DELETED_DATA)
		m_register_r[CHIP_STATUS] |= CS_DELDATA;
	else
		m_register_r[CHIP_STATUS] &= ~CS_DELDATA;

	/* Found. Now update the current cylinder/head registers. */
	update_id_regs(id);

	/* Initiate the DMA. We assume the DMARQ is positive. */
	m_register_r[INT_STATUS] &= ~ST_OVRUN;

	sector_len = 1 << (id.N+7);
	sector_data_id = id.data_id;
	buf.resize(sector_len);

	if (m_selected_drive_type & TYPE_FLOPPY)
	{
		m_drive->floppy_drive_read_sector_data(id.H, sector_data_id, &buf[0], sector_len);
	}
	else
	{
		// TODO: Should we get the sector length from the harddisk?
		m_harddisk->read_sector(id.C, id.H, id.R, &buf[0]);
	}
	sync_status_in();

	if (transfer_enable)
	{
		/* Copy via DMA into controller RAM. */
		set_dma_address(DMA23_16, DMA15_8, DMA7_0);

		m_out_dip(ASSERT_LINE);
		for (i=0; i < sector_len; i++)
		{
			m_out_dma((offs_t)0, buf[i]);
		}
		m_out_dip(CLEAR_LINE);
	}

	/* Check CRC. We assume everything is OK, no retry required. */
	m_register_r[CHIP_STATUS] &= ~CS_RETREQ;
	m_register_r[CHIP_STATUS] &= ~CS_CRCERR;
	m_register_r[CHIP_STATUS] &= ~CS_ECCATT;

	/* Update the DMA registers. */
	if (transfer_enable) dma_add_offset(sector_len);

	/* Decrement sector count. */
	m_register_w[SECTOR_COUNT] = (m_register_w[SECTOR_COUNT]-1)&0xff;

	/* Clear the error bits. */
	m_register_r[INT_STATUS] &= ~ST_TC_DATAERR;

	if (m_register_w[SECTOR_COUNT] == 0)
		return;

	/* Else this is a multi-sector operation. */
	m_register_w[DESIRED_SECTOR] =
	m_register_r[DESIRED_SECTOR] =
		(m_register_w[DESIRED_SECTOR]+1) & 0xff;

	/* Reinitialize retry count. */
	m_register_w[RETRY_COUNT] = retry;
}

/*
    Common procedure: data_transfer(write) (as described in the specification)
*/
void smc92x4_device::data_transfer_write(chrn_id_hd id, int deldata, int redcur, int precomp, bool write_long)
{
	int retry, i, sector_len;
	dynamic_buffer buf;
	int sector_data_id;
	sync_latches_out();
	sync_status_in();

	// Set command termination code. The error code is set first, and
	// on success, it is cleared.
	m_register_r[INT_STATUS] |= ST_TC_DATAERR;

	// Save the value. Note that the retry count is only in the first four
	// bits, and it is stored in one's complement. In this implementation
	// we don't work with it.
	retry = m_register_w[RETRY_COUNT];

	/* Initiate the DMA. We assume the DMARQ is positive. */
	m_register_r[INT_STATUS] &= ~ST_OVRUN;

	sector_len = 1 << (id.N+7);
	sector_data_id = id.data_id;

	buf.resize(sector_len);

	/* Copy via DMA from controller RAM. */
	set_dma_address(DMA23_16, DMA15_8, DMA7_0);

	m_out_dip(ASSERT_LINE);
	for (i=0; i<sector_len; i++)
	{
		buf[i] = m_in_dma(0);
	}
	m_out_dip(CLEAR_LINE);

	if (write_long)
	{
		if (VERBOSE>1) LOG("smc92x4 warn: write sector: Write_long not supported. Performing a normal write.\n");
	}

	if (m_selected_drive_type & TYPE_FLOPPY)
	{
		if (VERBOSE>4) LOG("smc92x4 info: write sector CHS=(%d,%d,%d)\n", id.C, id.H, id.R);
		m_drive->floppy_drive_write_sector_data(id.H, sector_data_id, &buf[0], sector_len, false);
	}
	else
	{
		m_harddisk->write_sector(id.C, id.H, id.R, &buf[0]);
	}
	sync_status_in();

	m_register_r[CHIP_STATUS] &= ~CS_RETREQ;
	m_register_r[CHIP_STATUS] &= ~CS_CRCERR;
	m_register_r[CHIP_STATUS] &= ~CS_ECCATT;

	/* Update the DMA registers. */
	dma_add_offset(sector_len);

	/* Decrement sector count. */
	m_register_w[SECTOR_COUNT] = (m_register_w[SECTOR_COUNT]-1)&0xff;

	/* Clear the error bits. */
	m_register_r[INT_STATUS] &= ~ST_TC_DATAERR;

	if (m_register_w[SECTOR_COUNT] == 0) return;

	/* Else this is a multi-sector operation. */
	m_register_w[DESIRED_SECTOR] =  m_register_r[DESIRED_SECTOR] = (m_register_w[DESIRED_SECTOR]+1) & 0xff;

	/* Reinitialize retry count. */
	m_register_w[RETRY_COUNT] = retry;
}

/*
    Read sectors physical / logical. Physical means that the first, the
    second, the third sector appearing under the head will be read. These
    sectors are usually not in logical sequence. The ordering depends on
    the interleave pattern.
*/
void smc92x4_device::read_write_sectors()
{
	int logical = m_command & 0x04;
	int implied_seek_disabled = m_command & 0x02; /* for read */
	int write = (m_command & 0x80);

	m_after_seek = false;

	m_to_be_continued = false;

	if (write) /* write sectors */
	{
		logical = m_command & 0x20;
		implied_seek_disabled = m_command & 0x40;
	}

	if (!logical)
		implied_seek_disabled = false;

	if (!m_found_id)
	{
		/* First start. */
		read_id_field(&m_seek_count, &m_step_direction, &m_recent_id);
	}

	if ((m_register_r[INT_STATUS] & ST_TC_DATAERR) == ST_TC_SUCCESS)
	{
		m_found_id = true;
		/* Perform the seek for the cylinder. */
		if (!implied_seek_disabled && m_seek_count > 0)
		{
			timed_seek_request();
		}
		else
		{
			if (write) write_sectors_continue();
			else read_sectors_continue();
		}
	}
	else
	{
		set_command_done(ST_TC_RDIDERR);
	}
}


void smc92x4_device::read_sectors_continue()
{
	bool check_sector = true; /* always check the first sector */
	int state = AGAIN;
	int logical = m_command & 0x04;

	/* Needed for the two ways of re-entry: during the seek process, and during sector read */
	if (!m_after_seek)
	{
		if (VERBOSE>7) LOG("smc92x4 continue with sector read\n");
		m_seek_count--;
		if (m_seek_count > 0)
		{
			read_write_sectors();
			return;
		}
		m_after_seek = true;
	}
	else
	{
		/* we are here after the sector read. */
		m_to_be_continued = false;
		if (m_register_r[INT_STATUS] & ST_TERMCOD)
		{
			if (VERBOSE>1) LOG("smc92x4 error: data error during sector read: INTSTATUS=%02x\n", m_register_r[INT_STATUS]);
			set_command_done(ST_TC_DATAERR);
			return;
		}
	}

	m_to_be_continued = false;

	/* Wait for SEEK_COMPLETE. We assume the signal has appeared. */

	if (m_register_w[SECTOR_COUNT] > 0 /* && !(m_register_r[DRIVE_STATUS] & DS_INDEX) */)
	{
		/* Call the verify sequence. */
		state = verify(&m_recent_id, check_sector);
		if (state==ERROR)
		{
			// TODO: set command done?
			if (VERBOSE>0) LOG("smc92x4 error: verify error during sector read\n");
			return;
		}

		/* For read physical, only verify the first sector. */
		if (!logical)
			check_sector = false;

		if (m_recent_id.flags & BAD_SECTOR)
		{
			if (VERBOSE>0) LOG("smc92x4 error: Bad sector, seek error\n");
			set_command_done(ST_TC_SEEKERR);
		}
		else
		{
			timed_sector_read_request();
		}
	}
	else
	{
		set_command_done(ST_TC_SUCCESS);
		if (VERBOSE>7) LOG("smc92x4 read sector command done\n");
	}
}

void smc92x4_device::write_sectors_continue()
{
	bool check_sector = true; /* always check the first sector */
	int state = AGAIN;
	int logical = m_command & 0x20;

	/* Needed for the two ways of re-entry: during the seek process, and during sector write */
	if (!m_after_seek)
	{
		m_seek_count--;
		if (m_seek_count > 0)
		{
			read_write_sectors();
			return;
		}
		m_after_seek = true;
	}
	else
	{
		/* we are here after the sector write. */
		m_to_be_continued = false;
		if (m_register_r[INT_STATUS] & ST_TERMCOD)
		{
			if (VERBOSE>0) LOG("smc92x4 error: data error during sector write\n");
			set_command_done(ST_TC_DATAERR);
			return;
		}
	}

	m_to_be_continued = false;

	if ((m_register_w[RETRY_COUNT] & 0xf0)!= 0xf0)
		if (VERBOSE>1) LOG("smc92x4 warn: RETRY_COUNT in write sector should be set to 0. Ignored.\n");

	/* Wait for SEEK_COMPLETE. We assume the signal has appeared. */
	if (m_register_w[SECTOR_COUNT] > 0)
	{
		/* Call the verify sequence. */
		state = verify(&m_recent_id, check_sector);
		if (state==ERROR)
		{
			if (VERBOSE>0) LOG("smc92x4 error: verify error during sector write\n");
			return;
		}

		/* For write physical, only verify the first sector. */
		if (!logical)
			check_sector = false;

/*      printf("smc92x4 write sectors CYL=%02x, HEA=%02x, SEC=%02x, MOD=%02x, CNT=%02x, TRY=%02x\n",
            m_register_w[DESIRED_CYLINDER],
            m_register_w[DESIRED_HEAD],
            m_register_w[DESIRED_SECTOR],
            m_register_w[MODE],
            m_register_w[SECTOR_COUNT],
            m_register_w[RETRY_COUNT]); */
		timed_sector_write_request();
	}
	else
	{
		set_command_done(ST_TC_SUCCESS);
		if (VERBOSE>7) LOG("smc92x4 write sector command done\n");
	}
}

/*********************************************************************
    Command implementations
*********************************************************************/

/*
    Handle the restore command
*/
void smc92x4_device::restore_drive()
{
	/* TODO: int_after_seek_complete required for buffered seeks */
	sync_status_in();

	if (m_seek_count>=4096 || !(m_register_r[DRIVE_STATUS] & DS_READY))
	{
		if (VERBOSE>0) LOG("smc92x4 error: seek error in restore\n");
		m_register_r[INT_STATUS] |= ST_TC_SEEKERR;
		return;
	}

	if (m_register_r[DRIVE_STATUS] & DS_TRK00)
	{
		m_register_r[INT_STATUS] |= ST_TC_SUCCESS;
		/* Issue interrupt */
		set_interrupt();
		return;
	}

	m_step_direction = -1;
	timed_seek_request();
}

void smc92x4_device::restore_continue()
{
	m_seek_count++;

	/* Next iteration */
	restore_drive();

	m_to_be_continued = false;
}

/*
    Handle the step command. Note that the CURRENT_CYLINDER register is not
    updated (this would break the format procedure).
*/
void smc92x4_device::step_in_out()
{
	int direction = (m_command & 0x02)? -1 : +1;
	// bit 0: 0 -> command ends after last seek pulse, 1 -> command
	// ends when the drive asserts the seek complete pin
	int buffered = m_command & 0x01;

	m_step_direction = direction;
	m_buffered = (buffered!=0);

	timed_seek_request();
	if (VERBOSE>6) LOG("smc92x4 waiting for drive step\n");
}

void smc92x4_device::step_continue()
{
	if (VERBOSE>7) LOG("smc92x4 step continue\n");
	m_to_be_continued = false;
	set_command_done(ST_TC_SUCCESS);
}

/*
    Poll drives
    This command is used to find out which drive has complete a buffered
    seek (RESTORE, SEEK IN/OUT with BUFFERED set to one)
*/
void smc92x4_device::poll_drives()
{
	int mask = 0x08;
	int i;
	int flags = m_command & 0x0f;

/* Spec is unclear: Do we continue to poll the drives after we have checked each
one for the first time? We are not interested in locking up the emulator, so
we decide to poll only once. */
	for (i=3; i>=0; i--)
	{
		if (flags & mask)
		{
			/* Poll drive */
			drive_select(i|((m_types[i])<<2));
			if (m_register_r[DRIVE_STATUS] & DS_SKCOM) return;
		}
		mask = mask>>1;
	}
}

void smc92x4_device::drive_select(int driveparm)
{
	m_output1 = (0x10 << (driveparm & 0x03)) | (m_register_w[RETRY_COUNT]&0x0f);
	m_selected_drive_type = (driveparm>>2) & 0x03;
	m_head_load_delay_enable = (driveparm>>4)&0x01;

	// We need to store the type of the drive for the poll_drives command
	// to be able to correctly select the device (floppy or hard disk).
	m_types[driveparm&0x03] = m_selected_drive_type;

	// Copy the DMA registers to registers CURRENT_HEAD, CURRENT_CYLINDER,
	// and CURRENT_IDENT. This is required during formatting (p. 14) as the
	// format command reuses the registers for formatting parameters.
	m_register_r[CURRENT_HEAD] = m_register_r[DMA7_0];
	m_register_r[CURRENT_CYLINDER] = m_register_r[DMA15_8];
	m_register_r[CURRENT_IDENT] = m_register_r[DMA23_16];

	sync_latches_out();
	sync_status_in();
}

/*
    Command SEEK/READID
*/
void smc92x4_device::seek_read_id()
{
	int step_enable = (m_command & 0x04);
	int verify_id = (m_command & 0x01);
	int wait = (m_command & 0x02);

	m_to_be_continued = false;

	if (!m_found_id)
	{
		/* First start. */
		read_id_field(&m_seek_count, &m_step_direction, &m_recent_id);
		m_found_id = true;
	}

	if ((m_register_r[INT_STATUS] & ST_TC_DATAERR) == ST_TC_SUCCESS)
	{
		/* Perform the seek for the cylinder. */
		if (step_enable && m_seek_count > 0)
		{
			timed_seek_request();
		}
		else
		{
			if (wait)
				if (VERBOSE>1) LOG("smc92x4 warn: seed_read_id: Waiting for seek_complete not implemented.\n");

			if (verify_id)
			{
				verify(&m_recent_id, true);
			}
		}
	}
	else
	{
		set_command_done(ST_TC_RDIDERR);
	}
}

void smc92x4_device::seek_read_id_continue()
{
	m_seek_count--;
	seek_read_id();
	m_to_be_continued = false;
}

/*
    Formats a track starting from the detection of an index mark until the
    detection of another index mark.
    The formatting is done exclusively by the controller; user programs may
    set parameters for gaps and interleaving.

    1. Before starting the command, the user program must have set up a
    sector sequence table in the controller RAM (located on the PCB):
    (ident, cylinder, head, sector1, size)  (5 bytes)
    (ident, cylinder, head, sector2, size)
    (ident, cylinder, head, sector3, size)
    ...
    ident is not required for floppy FM operation. size is not required
    for IBM AT-compatible hard disks.

    2. The DMA registers must point to the beginning of the table

    3. DRIVE_SELECT must be executed (which moves DMA regs to CUR_HEAD ...)

    4. DESIRED_HEAD register must be loaded

    5. The following setup must be done:

    GAP 0 size      DMA7_0          (2s comp)
    GAP 1 size      DMA15_8         (2s comp)
    GAP 2 size      DMA23_16        (2s comp)
    GAP 3 size      DESIRED_SECTOR      (2s comp)
    Sync size       DESIRED_CYLINDER    (1s comp)
    Sector count        SECTOR_COUNT        (1s comp)
    Sector size multiple    RETRY_COUNT         (1s comp)

    GAP4 is variable and fills the rest of the track until the next
    index hole. Usually we have 247 bytes for FM and 598 for MFM.

    6. The step rate and density must be loaded into the MODE register

    7. The drive must be stepped to the desired track.

    8. Now this command may be started.

    All data bytes of a sector are filled with 0xe5. The gaps will be filled
    with 0x4e (MFM) or 0xff (FM).

    To format another track, the sector id table must be updated, and steps
    7 and 8 must be repeated. If the DESIRED_HEAD register must be updated,
    the complete setup process must be done.

    Options: Command = 011m rppp
    m = set data mark (0 = set deleted)
    r = current (1 = reduced)
    ppp = precompensation (for sector size = (2^ppp) * 128; ppp<4 for floppy )

    ===============

    One deviation from the specification: The TI-99 family uses the SDF
    and TDF formats. The TDF formats complies with the formats produced
    by the standard TI disk controller. It does not use index address marks.
    So we could integrate a translation in ti99_dsk when writing to disk,
    but when reading, how can ti99_dsk know whether to blow up the format
    to full length for this controller or keep it as is for the other
    controller? Unless someone comes up with a better idea, we implement
    an "undocumented" option in this controller, allowing to create a
    different track layout.

    So there are two layouts for the floppy track:

    - full format, including the index address mark, according to the
      specification

    - short format without index AM which matches the PC99 format used
      for the TI-99 family.

    The formats are determined by setting the flag in the smc92x4
    interface structure.
*/
void smc92x4_device::format_floppy_track(int flags)
{
	floppy_image_legacy *floppy;
	int i,index,j, exp_size;
	int gap0, gap1, gap2, gap3, gap4, sync1, sync2, count, size, fm;
	int gap_byte, pre_gap, crc, mark, inam;
	UINT8 curr_cyl, curr_head, curr_sect, curr_size;

	int normal_data_mark = flags & 0x10;

	dynamic_buffer buffer;

	/* Determine the track size. We cannot allow different sizes in this design. */
	int data_count = 0;

	sync_status_in();

	floppy = m_drive->flopimg_get_image();

	if (floppy != NULL)
		data_count = floppy_get_track_size(floppy, 0, 0);

	if (data_count==0)
	{
		if (in_single_density_mode())
			data_count = TRKSIZE_SD;
		else
			data_count = TRKSIZE_DD;
	}

	/* Build buffer */
	buffer.resize(data_count);

	fm = in_single_density_mode();

	sync2 = (~m_register_w[DESIRED_CYLINDER])&0xff;
	gap2 = (-m_register_w[DMA23_16])&0xff;
	count = (~m_register_w[SECTOR_COUNT])&0xff;
	size = (~m_register_w[RETRY_COUNT])&0xff;
	gap_byte = (fm)? 0xff : 0x4e;

	if (m_full_track_layout)
	{
		/* Including the index AM. */
		gap0 = (-m_register_w[DMA7_0])&0xff;
		gap1 = (-m_register_w[DMA15_8])&0xff;
		gap3 = (-m_register_w[DESIRED_SECTOR])&0xff;
		gap4 = (fm)? 247 : 598;
		pre_gap = gap_byte;
		sync1 = sync2;
		inam = sync2 + ((fm)? 1 : 4);
	}
	else
	{
		/* Specific overrides for this format. We do not have the index mark. */
		gap0 = (fm)? 16 : 40;
		gap1 = 0;
		gap3 = (fm)? 45 : 24;
		gap4 = (fm)? 231 : 712;
		pre_gap = (fm)? 0x00 : 0x4e;
		sync1 = (fm)? 6 : 10;
		inam = 0;
	}

	index = 0;

	mark = (fm)? 10 : 16;  /* ID/DAM + A1 + CRC */

	exp_size = gap0 + inam + gap1 + count*(sync1 + mark + gap2 + sync2 + size*128 + gap3) + gap4;

	if (exp_size != data_count)
		if (VERBOSE>0) LOG("smc92x4 warn: The current track length in the image (%d) does not match the new track length (%d). Keeping the old length. This will break the image (sorry).\n", data_count, exp_size);

	/* use the backup registers set up during drive_select */
	set_dma_address(CURRENT_IDENT, CURRENT_CYLINDER, CURRENT_HEAD);

	memset(&buffer[index], pre_gap, gap0);
	index += gap0;

	if (m_full_track_layout)
	{
		/* Create the Index AM */
		memset(&buffer[index], 0x00, sync1);
		index += sync1;
		if (!fm)
		{
			memset(&buffer[index], 0xc2, 3);
			index += 3;
		}
		memset(&buffer[index], gap_byte, gap1);
		index += gap1;
	}

	/* for each sector */
	for (j=0; j < count; j++)
	{
		memset(&buffer[index], 0x00, sync1);
		index += sync1;

		if (!fm)
		{
			memset(&buffer[index], 0xa1, 3);
			index += 3;
		}

		buffer[index++] = 0xfe;

		m_out_dip(ASSERT_LINE);
//      if (!fm) curr_ident = m_in_dma();
		if (!fm) m_in_dma(0);
		curr_cyl = m_in_dma(0);
		curr_head = m_in_dma(0);
		curr_sect = m_in_dma(0);
		curr_size = m_in_dma(0);
		m_out_dip(CLEAR_LINE);

		buffer[index++] = curr_cyl;
		buffer[index++] = curr_head;
		buffer[index++] = curr_sect;
		buffer[index++] = curr_size;

		if (j==0)
			if (VERBOSE>6) LOG("current_floppy=%s, format track %d, head %d\n",  m_drive->tag(), curr_cyl, curr_head);

		/* Calculate CRC16 (5 bytes for ID) */
		crc = ccitt_crc16(0xffff, &buffer[index-5], 5);
		buffer[index++] = (crc>>8)&0xff;
		buffer[index++] = crc & 0xff;

		memset(&buffer[index], gap_byte, gap2);
		index += gap2;

		memset(&buffer[index], 0x00, sync2);
		index += sync2;

		if (!fm)
		{
			memset(&buffer[index], 0xa1, 3);
			index += 3;
		}

		if (normal_data_mark) buffer[index++] = 0xfb;
		else buffer[index++] = 0xf8;

		/* Sector data */
		for (i=0; i < 128*size; i++) buffer[index++] = 0xe5;

		/* Calculate CRC16 (128*size+1 bytes for sector) */
		crc = ccitt_crc16(0xffff, &buffer[index-128*size-1], 128*size+1);
		buffer[index++] = (crc>>8)&0xff;
		buffer[index++] = crc & 0xff;

		memset(&buffer[index], gap_byte, gap3);
		index += gap3;
	}

	memset(&buffer[index], gap_byte, gap4);
	index += gap4;

	m_drive->floppy_drive_write_track_data_info_buffer(m_register_w[DESIRED_HEAD]&0x0f, &buffer[0], &data_count);
	sync_status_in();
}

/*
    Create the track layout of a MFM hard disk. Like floppy disks,
    MFM hard disks are interfaced on a track base, that is, we have to
    create a complete track layout.

    For more explanations see the comments to format_floppy_track.
*/
void smc92x4_device::format_harddisk_track(int flags)
{
	int i,index,j;
	int gap1, gap2, gap3, gap4, sync, count, size, gap_byte, crc;
	UINT8 curr_ident, curr_cyl, curr_head, curr_sect, curr_size;

	int normal_data_mark = flags & 0x10;
	int data_count=0;

	dynamic_buffer buffer;

	sync_status_in();

	/* Build buffer */
//  gap0 = (-m_register_w[DMA7_0])&0xff;
	gap1 = (-m_register_w[DMA15_8])&0xff;
	gap2 = (-m_register_w[DMA23_16])&0xff;
	gap3 = (-m_register_w[DESIRED_SECTOR])&0xff;
	gap4 = 340;

	sync = (~m_register_w[DESIRED_CYLINDER])&0xff;
	count = (~m_register_w[SECTOR_COUNT])&0xff;
	size = (~m_register_w[RETRY_COUNT])&0xff;

	data_count = gap1 + count*(sync+12+gap2+sync+size*128+gap3)+gap4;

	buffer.resize(data_count);

	index = 0;
	gap_byte = 0x4e;

	/* use the backup registers set up during drive_select */
	set_dma_address(CURRENT_IDENT, CURRENT_CYLINDER, CURRENT_HEAD);

	for (i=0; i < gap1; i++) buffer[index++] = gap_byte;

	/* Now write the sectors. */
	for (j=0; j < count; j++)
	{
		for (i=0; i < sync; i++) buffer[index++] = 0x00;
		buffer[index++] = 0xa1;

		m_out_dip(ASSERT_LINE);
		curr_ident = m_in_dma(0);
		curr_cyl = m_in_dma(0);
		curr_head = m_in_dma(0);
		curr_sect = m_in_dma(0);
		curr_size = m_in_dma(0);
		m_out_dip(CLEAR_LINE);

		buffer[index++] = curr_ident;
		buffer[index++] = curr_cyl;
		buffer[index++] = curr_head;
		buffer[index++] = curr_sect;
		buffer[index++] = curr_size;

		/* Calculate CRC16 (5 bytes for ID) */
		crc = ccitt_crc16(0xffff, &buffer[index-5], 5);
		buffer[index++] = (crc>>8)&0xff;
		buffer[index++] = crc & 0xff;

		/* GAP 2 */
		for (i=0; i < gap2; i++) buffer[index++] = gap_byte;
		for (i=0; i < sync; i++) buffer[index++] = 0x00;

		buffer[index++] = 0xa1;

		if (normal_data_mark) buffer[index++] = 0xfb;
		else buffer[index++] = 0xf8;

		/* Sector data */
		for (i=0; i < 128*size; i++) buffer[index++] = 0xe5;

		/* Calculate CRC16 (128*size+1 bytes for sector) */
		crc = ccitt_crc16(0xffff, &buffer[index-128*size-1], 128*size+1);
		buffer[index++] = (crc>>8)&0xff;
		buffer[index++] = crc & 0xff;

		/* GAP 3 */
		for (i=0; i < 3; i++) buffer[index++] = 0;  /* check that, unclear in spec */
		for (i=0; i < gap3 - 3; i++) buffer[index++] = gap_byte;
	}
	/* GAP 4 */
	for (i=0; i < gap4; i++) buffer[index++] = gap_byte;

	// Now write the whole track
	m_harddisk->write_track(m_register_w[DESIRED_HEAD]&0x0f, &buffer[0], data_count);

	sync_status_in();
}

/*
    Read a floppy track.
    A complete track is read at the position of the head. It reads the
    track from one index pulse to the next index pulse. (Note that the
    spec talks about "index mark" and "signal from the drive" which is
    a bit confusing, since the index AM is behind Gap0, the index hole
    is before Gap0. We should check with a real device. Also, it does not
    speak about the head, so we assume the head is set in the DESIRED_HEAD
    register.)

    TODO: The TDF format does not support index marks. Need to define TDF
    in a more flexible way. Also consider format variations.
    (Hint: Do a check for the standard "IBM" format. Should probably do
    a translation from IBM to PC99 and back. Requires to parse the track
    image before. Need to decide whether we generally translate to the IBM
    format or generally to the PC99 format between controller and format.
    Format is the image is always PC99.)
*/
void smc92x4_device::read_floppy_track(bool transfer_only_ids)
{
	floppy_image_legacy *floppy;
	/* Determine the track size. We cannot allow different sizes in this design. */
	int data_count = 0;
	int i;
	dynamic_buffer buffer;

	sync_latches_out();

	floppy = m_drive->flopimg_get_image();

	/* Determine the track size. We cannot allow different sizes in this design. */
	if (floppy != NULL)
		data_count = floppy_get_track_size(floppy, 0, 0);

	if (data_count==0)
	{
		if (in_single_density_mode())
			data_count = TRKSIZE_SD;
		else
			data_count = TRKSIZE_DD;
	}

	buffer.resize(data_count);

	m_drive->floppy_drive_read_track_data_info_buffer(m_register_w[DESIRED_HEAD]&0x0f, (char *)&buffer[0], &data_count);
	sync_status_in();

	// Transfer the buffer to the external memory. We assume the memory
	// pointer has been set appropriately in the registers.
	set_dma_address(DMA23_16, DMA15_8, DMA7_0);

	if (transfer_only_ids)
	{
		if (VERBOSE>1) LOG("smc92x4 warn: read track: Ignoring transfer-only-ids. Reading complete track.\n");
	}

	m_out_dip(ASSERT_LINE);
	for (i=0; i < data_count; i++)
	{
		m_out_dma((offs_t)0, buffer[i]);
	}
	m_out_dip(CLEAR_LINE);
}

void smc92x4_device::read_harddisk_track(bool transfer_only_ids)
{
	/* Determine the track size. We cannot allow different sizes in this design. */
	int i;
	dynamic_buffer buffer;
	int data_count=0;
	sync_latches_out();

	data_count = m_harddisk->get_track_length();
	buffer.resize(data_count);

	/* buffer and data_count are allocated and set by the function. */
	m_harddisk->read_track(m_register_w[DESIRED_HEAD]&0x0f, &buffer[0]);
	sync_status_in();

	if (!(m_register_r[DRIVE_STATUS] & DS_READY))
	{
		if (VERBOSE>0) LOG("smc92x4 error: read harddisk track failed.\n");
	}

	// Transfer the buffer to the external memory. We assume the memory
	// pointer has been set appropriately in the registers.
	set_dma_address(DMA23_16, DMA15_8, DMA7_0);

	if (transfer_only_ids)
	{
		if (VERBOSE>1) LOG("smc92x4 warn: read track: Ignoring transfer-only-ids. Reading complete track.\n");
	}

	m_out_dip(ASSERT_LINE);
	for (i=0; i < data_count; i++)
	{
		m_out_dma((offs_t)0, buffer[i]);
	}
	m_out_dip(CLEAR_LINE);
}


void smc92x4_device::read_track()
{
	int transfer_only_ids = m_command & 0x01;

	if (m_selected_drive_type & TYPE_FLOPPY)
	{
		read_floppy_track(transfer_only_ids);
	}
	else
	{
		read_harddisk_track(transfer_only_ids);
	}
	timed_track_request();
}

void smc92x4_device::read_track_continue()
{
	m_to_be_continued = false;
	set_command_done(ST_TC_SUCCESS);
}

void smc92x4_device::format_track()
{
	int flags = m_command & 0x1f;

	if (m_selected_drive_type & TYPE_FLOPPY)
		format_floppy_track(flags);
	else
		format_harddisk_track(flags);

	timed_track_request();
}

void smc92x4_device::format_track_continue()
{
	m_to_be_continued = false;
	set_command_done(ST_TC_SUCCESS);
}

/*
    Continue to process after callback
*/
void smc92x4_device::process_after_callback()
{
	UINT8 opcode = m_command;
	if (opcode >= 0x02 && opcode <= 0x03)
	{
		restore_continue();
	}
	else if (opcode >= 0x04 && opcode <= 0x07)
	{
		step_continue();
	}
	else if (opcode >= 0x50 && opcode <= 0x57)
	{
		seek_read_id_continue();
	}
	else if ((opcode >= 0x58 && opcode <= 0x59)||(opcode >= 0x5C && opcode <= 0x5f))
	{
		read_sectors_continue();
	}
	else if (opcode >= 0x5a && opcode <= 0x5b)
	{
		read_track_continue();
	}
	else if (opcode >= 0x60 && opcode <= 0x7f)
	{
		format_track_continue();
	}
	else if (opcode >= 0x80)
	{
		write_sectors_continue();
	}
	else
	{
		if (VERBOSE>1) LOG("smc92x4 warn: Invalid command %x or command changed while waiting for callback\n", opcode);
	}
}

/*
    Process a command
*/
void smc92x4_device::process_command(UINT8 opcode)
{
	if (VERBOSE>6) LOG("smc92x4 process command %02x\n", opcode);
	if (m_to_be_continued)
	{
		if (VERBOSE>1) LOG("smc92x4 warn: previous command %02x not complete\n", m_command);
	}

	/* Reset DONE and BAD_SECTOR. */
	m_register_r[INT_STATUS] &= ~(ST_DONE | ST_BADSECT);

	// Reset interrupt line (not explicitly mentioned in spec, but seems reasonable
	clear_interrupt();
	m_register_r[INT_STATUS] &= ~(ST_INTPEND | ST_RDYCHNG);

	m_command = opcode;
	m_found_id = false;
	m_seek_count = 0;

	if (opcode == 0x00)
	{
		/* RESET */
		/* same effect as the RST* pin being active */
		if (VERBOSE>0) LOG("smc92x4 info: reset command\n");
		reset();
	}
	else if (opcode == 0x01)
	{
		/* DESELECT DRIVE */
		/* done when no drive is in use */
		if (VERBOSE>2) LOG("smc92x4 info: drdeselect command\n");
		m_output1 &= ~(OUT1_DRVSEL3|OUT1_DRVSEL2|OUT1_DRVSEL1|OUT1_DRVSEL0);
		m_output2 |= OUT2_DRVSEL3_;
		/* sync the latches on the PCB */
		sync_latches_out();
		sync_status_in();
	}
	else if (opcode >= 0x02 && opcode <= 0x03)
	{
		/* RESTORE DRIVE */
		// bit 0: 0 -> command ends after last seek pulse, 1 -> command
		// ends when the drive asserts the seek complete pin
		if (VERBOSE>2) LOG("smc92x4 info: restore command %X\n", opcode);
		restore_drive();
	}
	else if (opcode >= 0x04 && opcode <= 0x07)
	{
		/* STEP IN/OUT ONE CYLINDER */
		if (VERBOSE>2) LOG("smc92x4 info: step in/out command %X\n", opcode);
		step_in_out();
	}
	else if (opcode >= 0x08 && opcode <= 0x0f)
	{
		/* TAPE BACKUP (08-0f)*/
		if (VERBOSE>0) LOG("smc92x4 error: tape backup command %X not implemented\n", opcode);
	}
	else if (opcode >= 0x10 && opcode <= 0x1f)
	{
		/* POLLDRIVE */
		if (VERBOSE>2) LOG("smc92x4 info: polldrive command %X\n", opcode);
		poll_drives();
	}
	else if (opcode >= 0x20 && opcode <= 0x3f)
	{
		/* DRIVE SELECT */
		if (VERBOSE>2) LOG("smc92x4 info: drselect command %X\n", opcode);
		drive_select(opcode&0x1f);
	}
	else if (opcode >= 0x40 && opcode <= 0x4f)
	{
		/* SETREGPTR */
		if (VERBOSE>2) LOG("smc92x4 info: setregptr command %X\n", opcode);
		m_register_pointer = opcode & 0xf;
		// Spec does not say anything about the effect of setting an
		// invalid value (only "care should be taken")
		if (m_register_pointer > 10)
		{
			if (VERBOSE>1) LOG("smc92x4 error: set register pointer: Invalid register number: %d. Setting to 10.\n", m_register_pointer);
			m_register_pointer = 10;
		}
	}
	else if (opcode >= 0x50 && opcode <= 0x57)
	{
		/* SEEK/READ ID */
		if (VERBOSE>2) LOG("smc92x4 seekreadid command %X\n", opcode);
		seek_read_id();
	}
	else if ((opcode >= 0x58 && opcode <= 0x59)
		|| (opcode >= 0x5C && opcode <= 0x5f)
		|| (opcode >= 0x80))
	{
		/* READ/WRITE SECTORS PHYSICAL/LOGICAL */
		if (VERBOSE>2) LOG("smc92x4 info: read/write sector command %X\n", opcode);
		read_write_sectors();
	}
	else if (opcode >= 0x5A && opcode <= 0x5b)
	{
		/* READ TRACK */
		if (VERBOSE>2) LOG("smc92x4 info: read track command %X\n", opcode);
		read_track();
	}
	else if (opcode >= 0x60 && opcode <= 0x7f)
	{
		/* FORMAT TRACK */
		if (VERBOSE>2) LOG("smc92x4 info: format track command %X\n", opcode);
		format_track();
	}
	else
	{
		if (VERBOSE>0) LOG("smc92x4 error: Invalid command %x, ignored\n", opcode);
	}

	if (!m_to_be_continued)
		set_command_done();
}

/***************************************************************************
    Memory accessors
****************************************************************************/

/*
    Read a byte of data from a smc92x4 controller
    The address (offset) encodes the C/D* line (command and /data)
*/
READ8_MEMBER( smc92x4_device::read )
{
	UINT8 reply = 0;

	if ((offset & 1) == 0)
	{
		/* data register */
		reply = m_register_r[m_register_pointer];
		if (VERBOSE>6) LOG("smc92x4 register_r[%d] -> %02x\n", m_register_pointer, reply);
		/* Autoincrement until DATA is reached. */
		if (m_register_pointer < DATA)
			m_register_pointer++;
	}
	else
	{
		/* status register */
		reply = m_register_r[INT_STATUS];
		// Spec (p.3) : The interrupt pin is reset to its inactive state
		// when the UDC interrupt status register is read.
		if (VERBOSE>6) LOG("smc92x4 interrupt status read = %02x\n", reply);
		clear_interrupt();
		/* Clear the bits due to int status register read. */
		m_register_r[INT_STATUS] &= ~(ST_INTPEND | ST_RDYCHNG);
	}
	return reply;
}

/*
    Write a byte to a smc99x4 controller
    The address (offset) encodes the C/D* line (command and /data)
*/
WRITE8_MEMBER( smc92x4_device::write )
{
	data &= 0xff;

	if ((offset & 1) == 0)
	{
		/* data register */
		if (VERBOSE>6) LOG("smc92x4 register_w[%d] <- %X\n", m_register_pointer, data);
		m_register_w[m_register_pointer] = data;

		// The DMA registers and the sector register for read and
		// write are identical.
		if (m_register_pointer < DESIRED_HEAD)
			m_register_r[m_register_pointer] = data;

		/* Autoincrement until DATA is reached. */
		if (m_register_pointer < DATA)
			m_register_pointer++;
	}
	else
		process_command(data);  // command register
}



/***************************************************************************
    DEVICE FUNCTIONS
***************************************************************************/

void smc92x4_device::device_start()
{
	m_out_intrq.resolve_safe();
	m_out_dip.resolve_safe();
	m_out_auxbus.resolve_safe();
	m_in_auxbus.resolve_safe(0);
	m_out_dma.resolve_safe();
	m_in_dma.resolve_safe(0);

	// allocate timers
	// m_timer_data = timer_alloc(DATA_TIMER);
	m_timer_rs = timer_alloc(READ_TIMER);
	m_timer_ws = timer_alloc(WRITE_TIMER);
	m_timer_track = timer_alloc(TRACK_TIMER);
	m_timer_seek = timer_alloc(SEEK_TIMER);

	m_use_real_timing = true;
}

void smc92x4_device::device_reset()
{
	clear_interrupt();
	m_out_dip(CLEAR_LINE);

	for (int i=0; i<=11; i++)
		m_register_r[i] = m_register_w[i] = 0;

	m_to_be_continued = false;
}

void smc92x4_device::set_timing(bool realistic)
{
	m_use_real_timing = realistic;
	if (VERBOSE>0) LOG("smc92x4: use realistic timing: %02x\n", realistic);
}

void smc92x4_device::connect_floppy_drive(legacy_floppy_image_device *drive)
{
	m_drive = drive;
	if (VERBOSE>3)
	{
		if (drive==NULL) LOG("smc92x4: Unselect all drives\n");
		else LOG("smc92x4: Connect drive %s\n", drive->tag());
	}
}
void smc92x4_device::connect_hard_drive(mfm_harddisk_legacy_device *drive)
{
	m_harddisk = drive;
	if (VERBOSE>3)
	{
		if (drive==NULL) LOG("smc92x4: Unselect all drives\n");
		else LOG("smc92x4: Connect drive %s\n", drive->tag());
	}
}

void smc92x4_device::reset()
{
	device_reset();
}

const device_type SMC92X4 = &device_creator<smc92x4_device>;
