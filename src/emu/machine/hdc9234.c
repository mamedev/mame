/**************************************************************************

    HDC9234 Hard and Floppy Disk Controller
    Standard Microsystems Corporation (SMC)

    This controller handles MFM and FM encoded floppy disks and hard disks.
    A variant, the SMC9224, is used in some DEC systems.

    The HDC9234 is used in the Myarc HFDC card for the TI99/4A.

    References:
    [1] SMC HDC9234 preliminary data book (1988)

    The HDC9234 controller is also referred to as the "Universal Disk Controller" (UDC)
    by the data book

    Michael Zapf, June 2014

***************************************************************************/

#include "emu.h"
#include "hdc9234.h"

#define TRACE_REG 0
#define TRACE_ACT 0
#define TRACE_SHIFT 0
#define TRACE_LIVE 0
#define TRACE_SYNC 0

#define UNRELIABLE_MEDIA 0

// Seek complete?

// Untested:
// Multi-sector read
// Seek complete
// read sectors physical
//

/*
    Register names of the HDC. The left part is the set of write registers,
    while the right part are the read registers.

            +------+------+------+------+------+------+------+------+
    DHEAD:  |   0  | Sector size |  0   |   Desired head  (OUTPUT2) |  AT mode
            +------+------+------+------+------+------+------+------+
            |   0  |  Desired cylinder  |   Desired head  (OUTPUT2) |  SMC mode
            +------+------+------+------+------+------+------+------+
            +------+------+------+------+------+------+------+------+
    RETRY:  |  Retry count (ones comp!) |   Progr. output (OUTPUT1) |
            +------+------+------+------+------+------+------+------+
            +------+------+------+------+------+------+------+------+
    MODE:   | HD   | use CRC/ECC |  FM  |  0   |      step rate     |
            +------+------+------+------+------+------+------+------+
            +------+------+------+------+------+------+------+------+
    INTCOMM:|   1  |  0   | Done | DelD | User | WrPrt| Ready|Wfault|
            +------+------+------+------+------+------+------+------+
            +------+------+------+------+------+------+------+------+
    DDELAY: |   0  |  0   | Sector size |  0   |  0   |    Zone     | AT mode
            +------+------+------+------+------+------+------+------+
            |          Data to be written on disk                   | writing
            +------+------+------+------+------+------+------+------+
            |          Head load timer count                        | drselect
            +------+------+------+------+------+------+------+------+

    Read registers
            +------+------+------+------+------+------+------+------+
    CHIP_ST:| Retry|  ECC |  CRC | DelD | Sync | Comp | Current Drv |
            +------+------+------+------+------+------+------+------+
            +------+------+------+------+------+------+------+------+
    INT_ST: | Pend | DMARQ| Done |  Termcode   | RdyCh| Ovrun| BdSec|
            +------+------+------+------+------+------+------+------+
            +------+------+------+------+------+------+------+------+
    DRV_ST: | ECC  | Index| SeekC| Trk00| User | WrPrt| Ready|Wfault|
            +------+------+------+------+------+------+------+------+

*/
enum
{
	// Write registers   |   Read registers
	//--------------------------------------
	DMA7_0=0,
	DMA15_8=1,
	DMA23_16=2,
	DESIRED_SECTOR=3,       CURRENT_SECTOR=3,
	DESIRED_HEAD=4,         CURRENT_HEAD=4,
	DESIRED_CYLINDER=5,     CURRENT_CYLINDER=5,
	SECTOR_COUNT=6,         CURRENT_IDENT=6,
	RETRY_COUNT=7,          TEMP_STORAGE2=7,
	MODE=8,                 CHIP_STATUS=8,
	INT_COMM_TERM=9,        DRIVE_STATUS=9,
	DATA_DELAY=10,          DATA=10,
	COMMAND=11,             INT_STATUS=11,

	//======================
	// Internal registers
	CURRENT_SIZE=12,
	CURRENT_CRC1=13,
	CURRENT_CRC2=14
};

/*
    Definition of bits in the status register [1] p.7
*/
enum
{
	ST_INTPEND = 0x80,       // interrupt pending
	ST_DMAREQ  = 0x40,       // DMA request
	ST_DONE    = 0x20,       // command done
	ST_TERMCOD = 0x18,       // termination code (see below)
		TC_SUCCESS = 0x00,   // Successful completion
		TC_RDIDERR = 0x08,   // Error in READ-ID sequence
		TC_VRFYERR = 0x10,   // Error in VERIFY sequence
		TC_DATAERR = 0x18,   // Error in DATA-TRANSFER seq.
	ST_RDYCHNG = 0x04,       // ready change
	ST_OVRUN   = 0x02,       // overrun/underrun
	ST_BADSECT = 0x01        // bad sector
};

/*
    Definition of bits in the Termination-Conditions register
*/
enum
{
	TC_CRCPRE  = 0x80,       // CRC register preset, must be 1
	TC_UNUSED  = 0x40,       // bit 6 is not used and must be 0
	TC_INTDONE = 0x20,       // interrupt on command completion
	TC_TDELDAT = 0x10,       // terminate on deleted data mark detection
	TC_TDUSER  = 0x08,       // user-defined condition
	TC_TWPROT  = 0x04,       // terminate on write protection
	TC_INTRDCH = 0x02,       // interrupt on ready change
	TC_TWRFLT  = 0x01        // interrupt on write fault
};

/*
    Definition of bits in the chip status register.
*/
enum
{
	CS_RETREQ  = 0x80,        // retry required
	CS_ECCATT  = 0x40,        // ECC correction attempted
	CS_CRCERR  = 0x20,        // ECC/CRC error
	CS_DELDATA = 0x10,        // deleted data mark
	CS_SYNCERR = 0x08,        // synchronization error
	CS_COMPERR = 0x04,        // compare error
	CS_PRESDRV = 0x03         // present drive selected
};

/*
    Bits in the internal output registers. The registers are output via the
    auxiliary bus (AB)

    OUTPUT1
    AB7     drive select 3
    AB6     drive select 2
    AB5     drive select 1
    AB4     drive select 0
    AB3     programmable outputs
    AB2     programmable outputs
    AB1     programmable outputs
    AB0     programmable outputs

    OUTPUT2
    AB7     drive select 3* (active low, used for tape operations)
    AB6     reduce write current
    AB5     step direction       (0=towards TRK00)
    AB4     step pulse           (1=active)
    AB3     desired head 3
    AB2     desired head 2
    AB1     desired head 1
    AB0     desired head 0
*/
enum
{
	OUT1_DRVSEL3    = 0x80,
	OUT1_DRVSEL2    = 0x40,
	OUT1_DRVSEL1    = 0x20,
	OUT1_DRVSEL0    = 0x10,
	OUT2_DRVSEL3I   = 0x80,
	OUT2_REDWRT     = 0x40,
	OUT2_STEPDIR    = 0x20,
	OUT2_STEPPULSE  = 0x10,
	OUT2_HEADSEL3   = 0x08,
	OUT2_HEADSEL2   = 0x04,
	OUT2_HEADSEL1   = 0x02,
	OUT2_HEADSEL0   = 0x01
};

enum
{
	TYPE_AT = 0x00,
	TYPE_HD = 0x01,
	TYPE_FLOPPY8 = 0x02,
	TYPE_FLOPPY5 = 0x03
};

#define DRIVE_TYPE  0x03

/*
    Timers
*/
enum
{
	GEN_TIMER = 1,
	COM_TIMER,
	LIVE_TIMER
};

/*
    Definition of bits in the Mode register
*/
enum {
	MO_TYPE     = 0x80,        // Hard disk (1) or floppy (0)
	MO_CRCECC   = 0x60,        // Values for CRC/ECC handling
	MO_DENSITY  = 0x10,        // FM = 1; MFM = 0
	MO_UNUSED   = 0x08,        // Unused, 0
	MO_STEPRATE = 0x07         // Step rates
};

/*
    Step rates in microseconds for MFM. This is set in the mode register,
    bits 0-2. FM mode doubles all values.
*/
static const int step_hd[]      = { 22, 50, 100, 200, 400, 800, 1600, 3200 };
static const int step_flop8[]   = { 218, 500, 1000, 2000, 4000, 8000, 16000, 32000 };
static const int step_flop5[]   = { 436, 1000, 2000, 4000, 8000, 16000, 32000, 64000 };

/*
    ID fields association to registers
*/
static const int id_field[] = { CURRENT_CYLINDER, CURRENT_HEAD, CURRENT_SECTOR, CURRENT_SIZE, CURRENT_CRC1, CURRENT_CRC2 };

/*
    Pulse widths for stepping in ??s
*/
enum
{
	pulse_hd = 11,
	pulse_flop8 = 112,
	pulse_flop5 = 224
};

/*
    Times for UDC's acceptance of command and register write accesses (ns).
*/
enum
{
	REGISTER_COMMIT = 1000,
	COMMAND_COMMIT = 1000
};

enum
{
	UNDEF,
	IDLE,
	DONE,
	STEP_ON,
	STEP_OFF,
	RESTORE_CHECK1,
	RESTORE_CHECK2,
	SEEK_COMPLETE,

	READ_ID,
	READ_ID1,

	VERIFY,
	VERIFY1,
	VERIFY2,
	VERIFY3,

	DATA_TRANSFER,
	DATA_TRANSFER1,

	// Live states
	SEARCH_IDAM,
	SEARCH_IDAM_FAILED,
	READ_TWO_MORE_A1_IDAM,
	READ_ID_FIELDS_INTO_REGS,
	SEARCH_DAM,
	READ_TWO_MORE_A1_DAM,
	SEARCH_DAM_FAILED,
	READ_SECTOR_DATA,
	READ_SECTOR_DATA1
};

enum
{
	NOCMD,
	RESET,
	DESELECT,
	RESTORE,
	STEP,
	SELECT,
	SETREG,
	READSECL,
	READSECP
};

const hdc9234_device::cmddef hdc9234_device::s_command[] =
{
	{ 0x00, 0xff, RESET, &hdc9234_device::device_reset },
	{ 0x01, 0xff, DESELECT, &hdc9234_device::drive_deselect },
	{ 0x02, 0xfe, RESTORE, &hdc9234_device::restore_drive },
	{ 0x04, 0xfc, STEP, &hdc9234_device::step_drive },
	{ 0x20, 0xe0, SELECT, &hdc9234_device::drive_select },
	{ 0x40, 0xf0, SETREG, &hdc9234_device::set_register_pointer },
	{ 0x58, 0xfe, READSECP, &hdc9234_device::read_sector_physical },
	{ 0x5c, 0xfc, READSECL, &hdc9234_device::read_sector_logical },
	{ 0, 0, 0, 0 }
};

/*
    Standard constructor.
*/
hdc9234_device::hdc9234_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, HDC9234, "SMC HDC9234 Universal Disk Controller", tag, owner, clock, "hdc9234", __FILE__),
	m_out_intrq(*this),
	m_out_dmarq(*this),
	m_out_dip(*this),
	m_out_auxbus(*this),
	m_in_dma(*this),
	m_out_dma(*this),
	m_initialized(false)
{
}

/*
    Set or reset some bits.
*/
void hdc9234_device::set_bits(UINT8& byte, int mask, bool set)
{
	if (set) byte |= mask;
	else byte &= ~mask;
}

/*
    Tell whether the controller is in FM mode.
*/
bool hdc9234_device::fm_mode()
{
	return ((m_register_w[MODE]&MO_DENSITY)!=0);
}

/*
    Set/clear INT

    Interupts are generated in the following occasions:
    - when the DONE bit is set to 1 in the ISR and ST_DONE is set to 1
    - when the READY_CHANGE bit is set to 1 in the ISR and ST_RDYCHNG is set to 1
    (ready change: 1->0 or 0->1)
*/
void hdc9234_device::set_interrupt(line_state intr)
{
	if (intr == ASSERT_LINE)
	{
		// Only if there is not already a pending interrupt
		if ((m_register_r[INT_STATUS] & ST_INTPEND) == 0)
		{
			m_register_r[INT_STATUS] |= ST_INTPEND;
			m_out_intrq(intr);
		}
	}
	else
	{
		// if there is a pending interrupt
		if ((m_register_r[INT_STATUS] & ST_INTPEND) != 0)
			m_out_intrq(intr);
	}
}

/*
    Assert Command Done status bit, triggering interrupts as needed
*/
void hdc9234_device::set_command_done(int flags)
{
	set_bits(m_register_r[INT_STATUS], ST_DONE, true);

	if (flags != -1)
	{
		set_bits(m_register_r[INT_STATUS], ST_TERMCOD, false); // clear the previously set flags
		m_register_r[INT_STATUS] |= flags;
		if (TRACE_ACT) logerror("%s: command %02x done, flags=%02x\n", tag(), m_command, flags);
	}
	else
	{
		if (TRACE_ACT) logerror("%s: command %02x done\n", tag(), m_command);
	}

	// [1], p. 6
	if (TRACE_ACT) logerror("%s: Raise interrupt DONE\n", tag());
	set_interrupt(ASSERT_LINE);

	m_substate = IDLE;
	m_main_state = IDLE;
	m_command = NOCMD;
}

/*
    Preserve previously set termination code
*/
void hdc9234_device::set_command_done()
{
	set_command_done(-1);
}

void hdc9234_device::wait_time(emu_timer *tm, int microsec, int next_substate)
{
	if (TRACE_ACT) logerror("%s: Delay by %d microsec\n", tag(), microsec);
	tm->adjust(attotime::from_usec(microsec));
	m_substate = next_substate;
}

void hdc9234_device::wait_time(emu_timer *tm, attotime delay, int param)
{
	if (TRACE_ACT) logerror("%s: [%s] Delaying by %4.2f microsecs\n", tag(), ttsn().cstr(), delay.as_double()*1000000);
	tm->adjust(delay);
	m_substate = param;
}

// ===========================================================================
//     States
// ===========================================================================

/*
    DESELECT DRIVE
    done when no drive is in use
*/
void hdc9234_device::drive_deselect()
{
	if (TRACE_ACT) logerror("%s: deselect command\n", tag());
	set_bits(m_output1, OUT1_DRVSEL3|OUT1_DRVSEL2|OUT1_DRVSEL1|OUT1_DRVSEL0, false);
	sync_latches_out();
	set_command_done(TC_SUCCESS);
}

/*
    "Restore" command
    // RESTORE DRIVE
    // bit 0:
    // 0 -> command ends after last seek pulse,
    // 1 -> command ends when the drive asserts the seek complete pin
*/
void hdc9234_device::restore_drive()
{
	if (TRACE_ACT) logerror("%s: restore command %02x\n", tag(), m_command);

	m_seek_count = 0;
	m_substate = RESTORE_CHECK1;
	step_drive_continue();
}

/*
    STEP IN / OUT 1 CYLINDER
*/
void hdc9234_device::step_drive()
{
	if (TRACE_ACT) logerror("%s: step in/out command %02x\n", tag(), m_command);
	m_substate = STEP_ON;
	step_drive_continue();
}

void hdc9234_device::step_drive_continue()
{
	while (true)
	{
		switch (m_substate)
		{
		case DONE:
			set_command_done(TC_SUCCESS);
			return;

		case STEP_ON:
			if (TRACE_ACT) logerror("%s: substate STEP_ON\n", tag());
			// STEPDIR = 0 -> towards TRK00
			set_bits(m_output2, OUT2_STEPDIR, (m_command & 0x02)==0);
			// Raising edge (note that all signals must be inverted before leading them to the drive)
			set_bits(m_output2, OUT2_STEPPULSE, true);
			sync_latches_out();
			wait_time(m_timer, pulse_width(), STEP_OFF);
			return;

		case STEP_OFF:
			if (TRACE_ACT) logerror("%s: substate STEP_OFF\n", tag());
			set_bits(m_output2, OUT2_STEPPULSE, false);
			sync_latches_out();
			if (m_main_state==RESTORE)
			{
				wait_time(m_timer, get_step_time(), RESTORE_CHECK1);
			}
			else
			{
				wait_time(m_timer, get_step_time(), DONE);
			}
			return;

		case RESTORE_CHECK1:
			if (TRACE_ACT) logerror("%s: substate RESTORE_CHECK; seek count = %d\n", tag(), m_seek_count);
			// If the drive is on track 0 or not ready (no drive), terminate the command
			if (on_track00())
			{
				if (TRACE_ACT) logerror("%s: restore command TRK00 reached\n", tag());
				if (m_command & 1)
				{
					// Buffered seek; wait for SEEK_COMPLETE
					wait_line(SEEK_COMPLETE);
					return;
				}
				else
				{
					m_substate = DONE;
					break;
				}
			}
			m_substate = RESTORE_CHECK2;
			break;

		case RESTORE_CHECK2:
			// Track 0 has not been reached yet
			if ((m_register_r[DRIVE_STATUS] & HDC_DS_READY)==0)
			{
				if (TRACE_ACT) logerror("%s: restore command: drive not ready\n", tag());
				m_substate = DONE;
				break;
			}

			// Increase step count
			m_seek_count++;
			if (m_seek_count>=4096)
			{
				if (TRACE_ACT) logerror("%s: restore command: giving up\n", tag());
				set_command_done(TC_VRFYERR);
				return;
			}

			m_substate = STEP_ON;
			break;

		case SEEK_COMPLETE:
			m_substate = RESTORE_CHECK2;
			break;
		}
	}
}

void hdc9234_device::drive_select()
{
	int driveparm = m_command & 0x1f;

	// Command word
	//
	//        7     6     5     4     3     2     1     0
	//     +-----+-----+-----+-----+-----+-----+-----+-----+
	//     |  0  |  0  |  1  |Delay|    Type   |   Drive   |
	//     +-----+-----+-----+-----+-----+-----+-----+-----+
	//
	// [1] p.5: lower 4 bits of retry count register is put on OUTPUT1

	m_output1 = (0x10 << (driveparm & 0x03)) | (m_register_w[RETRY_COUNT]&0x0f);
	// Bit 7 of OUTPUT2 is complement of Bit 7 of OUTPUT1

	// The drive type is used to configure DMA burst mode ([1], p.12)
	// and to select the timing parameters
	m_selected_drive_type = (driveparm>>2) & 0x03;
	m_head_load_delay_enable = (driveparm>>4)&0x01;

	if (TRACE_ACT) logerror("%s: drive select command (%02x): head load delay=%d, type=%d, drive=%d\n", tag(), m_command, m_head_load_delay_enable, m_selected_drive_type, driveparm&3);

	// We need to store the type of the drive for the poll_drives command
	// to be able to correctly select the device (floppy or hard disk).
	// m_types[driveparm&0x03] = m_selected_drive_type;

	// Copy the DMA registers to registers CURRENT_HEAD, CURRENT_CYLINDER,
	// and CURRENT_IDENT. This is required during formatting ([1], p. 14)
	// as the format command reuses the registers for formatting parameters.
	m_register_r[CURRENT_HEAD] = m_register_r[DMA7_0];
	m_register_r[CURRENT_CYLINDER] = m_register_r[DMA15_8];
	m_register_r[CURRENT_IDENT] = m_register_r[DMA23_16];

	// Copy the selected drive number to the chip status register
	m_register_r[CHIP_STATUS] = (m_register_r[CHIP_STATUS] & 0xfc) | (driveparm & 0x03);

	sync_latches_out();
	set_command_done(TC_SUCCESS);
}

void hdc9234_device::set_register_pointer()
{
	m_register_pointer = m_command & 0xf;
	if (TRACE_ACT) logerror("%s: setregptr command; start reg=%d\n", tag(), m_register_pointer);
	// Spec does not say anything about the effect of setting an
	// invalid value (only "care should be taken")
	if (m_register_pointer > 10)
	{
		logerror("%s: set register pointer: Invalid register number: %d. Setting to 10.\n", tag(), m_register_pointer);
		m_register_pointer = 10;
	}
	set_command_done(TC_SUCCESS);
}

/*
    Read the desired sector. For multiple sectors, read the sectors in
    the order as they appear on the track. The command terminates with the
    next index pulse or when all sectors have been read before.
    Opcodes:
    58 = transfer disabled
    59 = transfer enabled
*/
void hdc9234_device::read_sector_physical()
{
	if (TRACE_ACT) logerror("%s: read sectors physical command %02x\n", tag(), m_command);
	if (TRACE_ACT) logerror("%s: sector: C=%d H=%d S=%d\n", tag(), m_register_w[DESIRED_CYLINDER], m_register_w[DESIRED_HEAD],m_register_w[DESIRED_SECTOR]);

	m_retry_save = m_register_w[RETRY_COUNT];
	m_multi_sector = (m_register_w[SECTOR_COUNT] != 1);

	m_substate = READ_ID;
	read_sector_continue();
}

/*
    Read the desired sector. For multiple sectors, read the sectors in
    ascending order (sector n, n+1, n+2 ...).
    Opcodes:
    5c = implied seek / transfer disabled
    5d = implied seek / transfer enabled
    5e = no implied seek / transfer disabled
    5f = no implied seek / transfer enabled
*/
void hdc9234_device::read_sector_logical()
{
	if (TRACE_ACT) logerror("%s: read sectors logical command %02x\n", tag(), m_command);
	if (TRACE_ACT) logerror("%s: sector: C=%d H=%d S=%d\n", tag(), m_register_w[DESIRED_CYLINDER], m_register_w[DESIRED_HEAD],m_register_w[DESIRED_SECTOR]);

	m_retry_save = m_register_w[RETRY_COUNT];
	m_multi_sector = (m_register_w[SECTOR_COUNT] != 1);

	m_substate = READ_ID;
	read_sector_continue();
}

void hdc9234_device::read_sector_continue()
{
	while (true)
	{
		switch (m_substate)
		{
		/*
		    READ ID FIELD ([1] p. 9)
		    The controller
		    - scans for the next IDAM
		    - reads the ID field values into the CURRENT_HEAD/CYLINDER/SECTOR registers
		    - checks the CRC
		    - calculates the number of steps and the direction towards DESIRED_CYLINDER
		    (must have saved that value before!)
		    - steps to that location during OUTPUT2 times

		    When an error occurs, the COMMAND_TERMINATION bits are set to 01
		*/
		case READ_ID:
			// Implied seek: Enter the READ_ID subprogram.
			if (TRACE_ACT) logerror("%s: substate READ_ID\n", tag());

			// Bit 1 = 0: enable implied seek (i.e. the controller will seek the desired track)
			// Bit 1 = 1: disable implied seek (controller will stay on the current track)
			// Also, do implied seek for read physical
			if ((m_command & 0x0e)==0x0e)
				m_substate = VERIFY;
			else
				m_substate = READ_ID1;

			// First step: Search the next IDAM, and if found, read the
			// ID values into the registers
			m_live_state.bit_count_total = 0;
			live_start(SEARCH_IDAM);
			return;

		case READ_ID1:
			// If an error occured (no IDAM found), terminate the command
			if ((m_register_r[CHIP_STATUS] & CS_SYNCERR) != 0)
			{
				if (TRACE_ACT) logerror("%s: READ_ID: No IDAM found\n", tag());
				set_command_done(TC_RDIDERR);
				return;
			}

			if (TRACE_ACT)
			{
				logerror("%s: substate READ_ID1\n", tag());
				logerror("%s: DESIRED_CYL = %d; CURRENT_CYL = %d\n", tag(), m_register_w[DESIRED_CYLINDER], m_register_r[CURRENT_CYLINDER]);
			}

			// The CRC has been updated automatically with each read_one_bit during the live_run.
			// We just need to check whether it ended in 0000
			if (m_live_state.crc != 0)
			{
				logerror("%s: CRC error in sector header\n", tag());
				set_bits(m_register_r[CHIP_STATUS], CS_CRCERR, true);
				set_command_done(TC_RDIDERR);
				return;
			}

			// Calculate the direction and number of step pulses
			// positive -> towards inner cylinders
			// negative -> towards outer cylinders
			// zero -> we're already there
			m_track_delta = m_register_w[DESIRED_CYLINDER] - m_register_r[CURRENT_CYLINDER];
			m_substate = STEP_ON;
			break;

		case STEP_ON:
			// Any more steps left?
			if (m_track_delta == 0)
			{
				m_substate = VERIFY;
				break;
			}

			if (TRACE_ACT) logerror("%s: substate STEP_ON\n", tag());
			// STEPDIR = 0 -> towards TRK00
			set_bits(m_output2, OUT2_STEPDIR, (m_track_delta>0));
			set_bits(m_output2, OUT2_STEPPULSE, true);
			sync_latches_out();
			wait_time(m_timer, pulse_width(), STEP_OFF);
			return;

		case STEP_OFF:
			if (TRACE_ACT) logerror("%s: substate STEP_OFF\n", tag());
			set_bits(m_output2, OUT2_STEPPULSE, false);
			sync_latches_out();
			m_track_delta += (m_track_delta<0)? 1 : -1;
			// Return to STEP_ON, check whether there are more steps
			wait_time(m_timer, get_step_time(), STEP_ON);
			return;

		case VERIFY:
		/*
		    VERIFY ([1] p. 10)
		    The controller
		    - continues to read the next ID field until the current values match the
		      contents of the DESIRED_HEAD/CYLINDER/SECTOR registers
		    - checks the CRC

		    When an error occurs, the COMMAND_TERMINATION bits are set to 10
		*/
			// After seeking (or immediately when implied seek has been disabled),
			// find the desired sector.

			if (TRACE_ACT) logerror("%s: substate VERIFY\n", tag());

			// If an error occured (no IDAM found), terminate the command
			// (This test is only relevant when we did not have a seek phase before)
			if ((m_register_r[CHIP_STATUS] & CS_SYNCERR) != 0)
			{
				if (TRACE_ACT) logerror("%s: READ_ID: No IDAM found\n", tag());
				set_command_done(TC_VRFYERR);
				return;
			}

			// Count from 0 again
			m_live_state.bit_count_total = 0;
			m_substate = VERIFY1;
			break;

		case VERIFY1:
			// Check whether we are already there
			if ((m_register_w[DESIRED_CYLINDER] == m_register_r[CURRENT_CYLINDER])
				&& (m_register_w[DESIRED_HEAD] == m_register_r[CURRENT_HEAD])
				&& (m_register_w[DESIRED_SECTOR] == m_register_r[CURRENT_SECTOR]))
			{
				if (TRACE_ACT) logerror("%s: Found the desired sector\n", tag());
				m_substate = DATA_TRANSFER;
			}
			else
			{
				if (TRACE_ACT) logerror("%s: Current CHS=(%d,%d,%d), desired CHS=(%d,%d,%d).\n", tag(),
					m_register_r[CURRENT_CYLINDER] & 0xff,
					m_register_r[CURRENT_HEAD] & 0xff,
					m_register_r[CURRENT_SECTOR] & 0xff,
					m_register_w[DESIRED_CYLINDER] & 0xff,
					m_register_w[DESIRED_HEAD] & 0xff,
					m_register_w[DESIRED_SECTOR] & 0xff);
				m_substate = VERIFY2;
			}
			break;

		case VERIFY2:
			// Search the next ID
			m_substate = VERIFY3;
			live_start(SEARCH_IDAM);
			return;

		case VERIFY3:
			if ((m_register_r[CHIP_STATUS] & CS_SYNCERR) != 0)
			{
				if (TRACE_ACT) logerror("%s: VERIFY: Desired sector not found\n", tag());
				// live_run has set the sync error; clear it
				set_bits(m_register_r[CHIP_STATUS], CS_SYNCERR, false);
				// and set the compare error bit instead
				set_bits(m_register_r[CHIP_STATUS], CS_COMPERR, true);
				set_command_done(TC_VRFYERR);
				return;
			}

			// Continue with the loop
			if ((m_command & 0x0c)==0x0c)
			{
				// this is for the logical sector reading
				m_substate = VERIFY1;
			}
			else
			{
				// this is for the physical sector reading
				// do not verify the next ID field
				m_substate = DATA_TRANSFER;
				m_wait_for_index = true;
			}
			break;

		case DATA_TRANSFER:
		/*
		    DATA TRANSFER ([1], p. 10)
		    only during READ PHYSICAL/LOGICAL
		    The controller
		    - scans for the next DAM
		    - initiates a DMA request and waits for ACK from the system processor
		    - transfers the contents of the current sector into memory via DMA

		    When an error occurs, the COMMAND_TERMINATION bits are set to 11
		*/
			if (TRACE_ACT) logerror("%s: substate DATA_TRANSFER\n", tag());

			// Search the DAM and transfer the contents via DMA
			m_substate = DATA_TRANSFER1;

			// Count from 0 again
			m_live_state.bit_count_total = 0;

			dma_address_out();
			live_start(SEARCH_DAM);
			return;

		case DATA_TRANSFER1:
			// OK, sector has been read.
			// Check CRC
			if (m_live_state.crc != 0)
			{
				if (TRACE_ACT) logerror("%s: CRC error in sector data\n", tag());
				// Set Retry Required flag
				set_bits(m_register_r[CHIP_STATUS], CS_RETREQ, true);

				// Decrement the retry register (one's complemented value; 0000 = 15)
				int retry = 15-((m_register_w[RETRY_COUNT] >> 4)&0x0f);
				if (TRACE_ACT) logerror("%s: CRC error; retries = %d\n", tag(), retry);
				m_register_w[RETRY_COUNT] = (m_register_w[RETRY_COUNT] & 0x0f) | ((15-(retry-1))<<4);

				if (retry == 0)
				{
					if (TRACE_ACT) logerror("%s: CRC error; no retries left\n", tag());
					set_bits(m_register_r[CHIP_STATUS], CS_CRCERR, true);
					set_command_done(TC_DATAERR);
					return;
				}
				else
				{
					// Go back to VERIFY and try again
					// Note that the specs recommend to set the retry to 0 (1111)
					// for physical reading; failing to do so will result in
					// unpredictable behavior.
					// We'll rely on the properly written software as well.
					m_live_state.bit_count_total = 0;
					m_substate = VERIFY2;
				}
			}
			else
			{
				if (TRACE_ACT) logerror("%s: Sector successfully read\n", tag());

				// Update the DMA registers for multi-sector operations
				if (m_multi_sector)
				{
					int dma_address = (m_register_w[DMA23_16] & 0xff) << 16 |
						(m_register_w[DMA15_8] & 0xff) << 8 |
						(m_register_w[DMA7_0] & 0xff);

					dma_address = (dma_address + get_sector_size()) & 0xffffff;

					m_register_w[DMA23_16] = m_register_r[DMA23_16] = (dma_address & 0xff0000) >> 16;
					m_register_w[DMA15_8] = m_register_r[DMA15_8] = (dma_address & 0x00ff00) >> 16;
					m_register_w[DMA7_0] = m_register_r[DMA7_0] = (dma_address & 0x0000ff) >> 16;
				}

				// Decrement the count
				m_register_w[SECTOR_COUNT] = (m_register_w[SECTOR_COUNT]-1) & 0xff;

				// Do we have more sectors to read?
				// Surprisingly, the manual does not say what happens when
				// the sector count is zero for the first access.
				// It explicitly states that the check is done after the access.
				// If we take it (and especially the state charts) seriously, zero means 256.
				// m_stop_after_index is important for physical reading
				if (m_register_w[SECTOR_COUNT] != 0 && !m_stop_after_index)
				{
					// Increment the sector number
					// What happens when we exceed the highest sector number
					// in the track? We have to assume that this is possible
					// and that in this case the VERIFY routine fails.
					m_register_w[DESIRED_SECTOR] = (m_register_w[DESIRED_SECTOR] + 1) & 0xff;
					m_substate = VERIFY1;
					m_live_state.bit_count_total = 0;
				}
				else
				{
					set_command_done(TC_SUCCESS);
					return;
				}
			}
			break;

		default:
			if (TRACE_ACT) logerror("%s: unknown substate %d in read_sector\n", tag(), m_substate);
		}
	}
}

void hdc9234_device::general_continue()
{
	// Do we have a live run on the track?
	if (m_live_state.state != IDLE)
	{
		// Continue with it
		live_run();
		if (m_live_state.state != IDLE)  return;
	}

	// We're here when there is no live_run anymore
	// Where were we last time?
	switch (m_main_state)
	{
	case IDLE:
		break;
	case RESTORE:
	case STEP:
		step_drive_continue();
		break;
	case SELECT:
		// During drive_select there is no need to continue
		break;
	case READSECL:
		read_sector_continue();
		break;
	default:
		logerror("%s: [%s] general_continue on unknown main_state %d\n", tag(), ttsn().cstr(), m_main_state);
		break;
	}
}

// ===========================================================================

/*
    Delivers the step time (in microseconds) minus the pulse width
*/
int hdc9234_device::get_step_time()
{
	int time = 0;
	int index = m_register_w[MODE] & MO_STEPRATE;
	// Get seek time.
	if ((m_selected_drive_type & DRIVE_TYPE) == TYPE_FLOPPY8)
		time = step_flop8[index] - pulse_flop8;

	else if ((m_selected_drive_type & DRIVE_TYPE) == TYPE_FLOPPY5)
		time = step_flop5[index] - pulse_flop5;
	else
		time = step_hd[index] - pulse_hd;

	if (fm_mode()) time = time * 2;
	return time;
}

/*
    Delivers the pulse width time (in microseconds)
*/
int hdc9234_device::pulse_width()
{
	int time = 0;
	// Get seek time.
	if ((m_selected_drive_type & DRIVE_TYPE) == TYPE_FLOPPY8)
		time = pulse_flop8;

	else if ((m_selected_drive_type & DRIVE_TYPE) == TYPE_FLOPPY5)
		time = pulse_flop5;
	else
		time = pulse_hd;

	if (fm_mode()) time = time * 2;
	return time;
}

/*
    Delivers the sector size
*/
int hdc9234_device::get_sector_size()
{
	return 128 << (m_register_r[CURRENT_SIZE] & 3);
}

/*
===========================================================================

    Live state machine

    We follow a very similar approach to track access like in wd_fdc. The live
    state machine attempts to find marks on the track, starting from the current
    position. When found, it waits for the machine to catch up. When an event
    happens in the meantime, the state machine is rolled back, and the actions
    are replayed until the position where the event occured.

    Lots of code is taken from wd_fdc, with some minor restructuring and renaming.
    Same ideas, though. More comments.

===========================================================================
*/

astring hdc9234_device::tts(const attotime &t)
{
	char buf[256];
	int nsec = t.attoseconds / ATTOSECONDS_PER_NANOSECOND;
	sprintf(buf, "%4d.%03d,%03d,%03d", int(t.seconds), nsec/1000000, (nsec/1000)%1000, nsec % 1000);
	return buf;
}

astring hdc9234_device::ttsn()
{
	return tts(machine().time());
}

/*
    The controller starts to read bits from the disk. This method takes an
    argument for the state machine called at the end.
*/
void hdc9234_device::live_start(int state)
{
	if (TRACE_LIVE) logerror("%s: [%s] Live start substate=%d\n", tag(), ttsn().cstr(), state);
	m_live_state.time = machine().time();
	m_live_state.state = state;
	m_live_state.next_state = -1;

	m_live_state.shift_reg = 0;
	m_live_state.crc = 0xffff;
	m_live_state.bit_counter = 0;
	m_live_state.data_separator_phase = false;
	m_live_state.data_reg = 0;

	pll_reset(m_live_state.time);
	m_checkpoint_state = m_live_state;

	// Save checkpoint
	m_checkpoint_pll = m_pll;

	live_run();
	m_last_live_state = UNDEF;
}

void hdc9234_device::live_run()
{
	live_run_until(attotime::never);
}

/*
    The main method of the live state machine. We stay in this method until
    the requested data are read.
    limit: if unlimited (attotime::never), run up to the end of the track and wait there
    otherwise, used to replay the read/write operation up to the point where the event happened
*/
void hdc9234_device::live_run_until(attotime limit)
{
	int slot = 0;

	if (TRACE_LIVE) logerror("%s: [%s] live_run, live_state=%d, fm_mode=%d\n", tag(), tts(m_live_state.time).cstr(), m_live_state.state, fm_mode()? 1:0);

	if (m_live_state.state == IDLE || m_live_state.next_state != -1)
	{
		if (TRACE_LIVE) logerror("%s: [%s] live_run, state=%d, next_state=%d\n", tag(), tts(m_live_state.time).cstr(), m_live_state.state,m_live_state.next_state);
		return;
	}

	if (limit == attotime::never)
	{
		// We did not specify an upper time bound, so we take the next index pulse
		if (m_floppy != NULL) limit = m_floppy->time_next_index();

		if (limit == attotime::never)
		{
			// We don't have an index pulse? (no disk?)
			// See wd_fdc: Force a sync from time to time in that case
			// so that the main cpu timeout isn't too painful.  Avoids
			// looping into infinity looking for data too.
			limit = machine().time() + attotime::from_msec(1);
			m_timer->adjust(attotime::from_msec(1));
		}
	}

	while (true)
	{
		switch (m_live_state.state)
		{
		case SEARCH_IDAM:

			// We're doing this complicated logerror check to avoid
			// repeated logging in the same state. This can be found for the
			// other live states as well. m_last_live_state is only used to
			// control this loggind.

			if (TRACE_LIVE && m_last_live_state != SEARCH_IDAM)
				logerror("%s: [%s] SEARCH_IDAM [limit %s]\n", tag(),tts(m_live_state.time).cstr(), tts(limit).cstr());
			m_last_live_state = m_live_state.state;

			// This bit will be set when the IDAM cannot be found
			set_bits(m_register_r[CHIP_STATUS], CS_SYNCERR, false);

			if (read_one_bit(limit))
			{
				if (TRACE_LIVE) logerror("%s: [%s] SEARCH_IDAM limit reached\n", tag(), tts(m_live_state.time).cstr());
				return;
			}
			// logerror("%s: SEARCH_IDAM\n", tts(m_live_state.time).cstr());
			if (TRACE_SHIFT) logerror("%s: shift = %04x data=%02x c=%d\n", tts(m_live_state.time).cstr(), m_live_state.shift_reg,
					(m_live_state.shift_reg & 0x4000 ? 0x80 : 0x00) |
					(m_live_state.shift_reg & 0x1000 ? 0x40 : 0x00) |
					(m_live_state.shift_reg & 0x0400 ? 0x20 : 0x00) |
					(m_live_state.shift_reg & 0x0100 ? 0x10 : 0x00) |
					(m_live_state.shift_reg & 0x0040 ? 0x08 : 0x00) |
					(m_live_state.shift_reg & 0x0010 ? 0x04 : 0x00) |
					(m_live_state.shift_reg & 0x0004 ? 0x02 : 0x00) |
					(m_live_state.shift_reg & 0x0001 ? 0x01 : 0x00),
					m_live_state.bit_counter);

			// [1] p. 9: The ID field sync mark must be found within 33,792 byte times
			if (m_live_state.bit_count_total > 33792*16)
			{
				wait_for_realtime(SEARCH_IDAM_FAILED);
				return;
			}

			if (!fm_mode())
			{
				// MFM case
				if (m_live_state.shift_reg == 0x4489)
				{
					if (TRACE_LIVE) logerror("%s: [%s] Found an A1 mark\n", tag(),tts(m_live_state.time).cstr());
					m_live_state.crc = 0x443b;
					m_live_state.data_separator_phase = false;
					m_live_state.bit_counter = 0;
					// Next task: find the next two A1 marks
					m_live_state.state = READ_TWO_MORE_A1_IDAM;
				}
			}
			else
			{
				// FM case
				if (m_live_state.shift_reg == 0xf57e)
				{
					if (TRACE_LIVE) logerror("%s: SEARCH_IDAM: IDAM found\n", tag());
					m_live_state.crc = 0xef21;
					m_live_state.data_separator_phase = false;
					m_live_state.bit_counter = 0;
					m_live_state.state = READ_ID_FIELDS_INTO_REGS;
				}
			}
			break;

		case SEARCH_IDAM_FAILED:
			set_bits(m_register_r[CHIP_STATUS], CS_SYNCERR, true);
			m_live_state.state = IDLE;
			return;

		case READ_TWO_MORE_A1_IDAM:     // This state only applies for MFM mode.

			if (TRACE_LIVE && m_last_live_state != READ_TWO_MORE_A1_IDAM)
				logerror("%s: [%s] READ_TWO_MORE_A1\n", tag(),tts(m_live_state.time).cstr());
			m_last_live_state = m_live_state.state;

			// Beyond time limit?
			if (read_one_bit(limit)) return;

			if (TRACE_SHIFT) logerror("%s: shift = %04x data=%02x c=%d\n", tts(m_live_state.time).cstr(), m_live_state.shift_reg,
					(m_live_state.shift_reg & 0x4000 ? 0x80 : 0x00) |
					(m_live_state.shift_reg & 0x1000 ? 0x40 : 0x00) |
					(m_live_state.shift_reg & 0x0400 ? 0x20 : 0x00) |
					(m_live_state.shift_reg & 0x0100 ? 0x10 : 0x00) |
					(m_live_state.shift_reg & 0x0040 ? 0x08 : 0x00) |
					(m_live_state.shift_reg & 0x0010 ? 0x04 : 0x00) |
					(m_live_state.shift_reg & 0x0004 ? 0x02 : 0x00) |
					(m_live_state.shift_reg & 0x0001 ? 0x01 : 0x00),
					m_live_state.bit_counter);

			if (m_live_state.bit_count_total > 33792*16)
			{
				wait_for_realtime(SEARCH_IDAM_FAILED);
				return;
			}

			// Repeat until we have collected 16 bits
			if(m_live_state.bit_counter & 15) break;

			// So we now got 16 bits. Fill this value into the next slot. We expect two more A1 values.
			slot = m_live_state.bit_counter >> 4;
			if (slot < 3)
			{
				if (m_live_state.shift_reg != 0x4489)
				{
					// This ain't A1. Step back into the previous state (look for the next IDAM).
					m_live_state.state = SEARCH_IDAM;
				}
				else
					if (TRACE_LIVE) logerror("%s: [%s] Found an A1 mark\n", tag(),tts(m_live_state.time).cstr());
				// Continue
				break;
			}

			if (TRACE_LIVE) logerror("%s: [%s] Found data value %02X\n", tag(),tts(m_live_state.time).cstr(), m_live_state.data_reg);
			if (m_live_state.data_reg != 0xfe)
			{
				// This may happen when we accidentally locked onto the DAM. Look for the next IDAM.
				if (TRACE_LIVE) logerror("%s: Missing FE data after A1A1A1\n", tag());
				m_live_state.state = SEARCH_IDAM;
				break;
			}

			// We're here after we got the three A1 and FE
			m_live_state.bit_counter = 0;
			m_live_state.state = READ_ID_FIELDS_INTO_REGS;
			break;

		case READ_ID_FIELDS_INTO_REGS:
			if (TRACE_LIVE && m_last_live_state != READ_ID_FIELDS_INTO_REGS)
				logerror("%s: [%s] READ_ID_FIELDS_INTO_REGS\n", tag(),tts(m_live_state.time).cstr());
			m_last_live_state = m_live_state.state;

			if (read_one_bit(limit))
			{
				return;
			}
			// Already got 16 bits?
			if (m_live_state.bit_counter & 15) break;

			slot = (m_live_state.bit_counter >> 4)-1;

			if (TRACE_LIVE) logerror("%s: slot %d = %02x, crc=%04x\n", tag(), slot, m_live_state.data_reg, m_live_state.crc);

			// The id_field is an array of indexes into the chip registers.
			// Thus we get the values properly assigned to the registers.
			m_register_r[id_field[slot]] = m_live_state.data_reg;

			if(slot > 4)
			{
				// We successfully read the ID fields; let's wait for the machine time to catch up.
				// Live run is done here; it is the main state machine's turn again.
				m_live_state.bit_count_total = 0;
				wait_for_realtime(IDLE);
				return;
			}
			break;

		case SEARCH_DAM:
			if (TRACE_LIVE && m_last_live_state != SEARCH_DAM)
				logerror("%s: [%s] SEARCH_DAM\n", tag(),tts(m_live_state.time).cstr());
			m_last_live_state = m_live_state.state;

			set_bits(m_register_r[CHIP_STATUS], CS_DELDATA, false);

			if(read_one_bit(limit))
				return;

			if (TRACE_SHIFT) logerror("%s: shift = %04x data=%02x c=%d\n", tts(m_live_state.time).cstr(), m_live_state.shift_reg,
					(m_live_state.shift_reg & 0x4000 ? 0x80 : 0x00) |
					(m_live_state.shift_reg & 0x1000 ? 0x40 : 0x00) |
					(m_live_state.shift_reg & 0x0400 ? 0x20 : 0x00) |
					(m_live_state.shift_reg & 0x0100 ? 0x10 : 0x00) |
					(m_live_state.shift_reg & 0x0040 ? 0x08 : 0x00) |
					(m_live_state.shift_reg & 0x0010 ? 0x04 : 0x00) |
					(m_live_state.shift_reg & 0x0004 ? 0x02 : 0x00) |
					(m_live_state.shift_reg & 0x0001 ? 0x01 : 0x00),
					m_live_state.bit_counter);

			if (!fm_mode())
			{   // MFM
				if(m_live_state.bit_counter > 43*16)
				{
					logerror("%s: SEARCH_DAM failed\n", tag());
					wait_for_realtime(SEARCH_DAM_FAILED);
					return;
				}

				if (m_live_state.bit_counter >= 28*16 && m_live_state.shift_reg == 0x4489)
				{
					if (TRACE_LIVE) logerror("%s: [%s] Found an A1 mark\n", tag(),tts(m_live_state.time).cstr());
					m_live_state.crc = 0x443b;
					m_live_state.data_separator_phase = false;
					m_live_state.bit_counter = 0;
					m_live_state.state = READ_TWO_MORE_A1_DAM;
				}
			}
			else
			{   // FM
				if (m_live_state.bit_counter > 23*16)
				{
					logerror("%s: SEARCH_DAM failed\n", tag());
					wait_for_realtime(SEARCH_DAM_FAILED);
					return;
				}

				if (m_live_state.bit_counter >= 11*16 && (m_live_state.shift_reg == 0xf56a || m_live_state.shift_reg == 0xf56b ||
														m_live_state.shift_reg == 0xf56e || m_live_state.shift_reg == 0xf56f)) {
					if (TRACE_LIVE) logerror("%s: SEARCH_DAM: found DAM = %04x\n", tag(), m_live_state.shift_reg);
					m_live_state.crc =
						m_live_state.shift_reg == 0xf56a ? 0x8fe7 :
						m_live_state.shift_reg == 0xf56b ? 0x9fc6 :
						m_live_state.shift_reg == 0xf56e ? 0xafa5 :
						0xbf84;
					m_live_state.data_separator_phase = false;
					m_live_state.bit_counter = 0;
					m_live_state.state = READ_SECTOR_DATA;
				}
			}
			break;

		case READ_TWO_MORE_A1_DAM: {
			if (TRACE_LIVE && m_last_live_state != READ_TWO_MORE_A1_DAM)
				logerror("%s: [%s] READ_TWO_MORE_A1_DAM\n", tag(),tts(m_live_state.time).cstr());
			m_last_live_state = m_live_state.state;

			if(read_one_bit(limit))
				return;

			if (TRACE_SHIFT) logerror("%s: shift = %04x data=%02x c=%d\n", tts(m_live_state.time).cstr(), m_live_state.shift_reg,
					(m_live_state.shift_reg & 0x4000 ? 0x80 : 0x00) |
					(m_live_state.shift_reg & 0x1000 ? 0x40 : 0x00) |
					(m_live_state.shift_reg & 0x0400 ? 0x20 : 0x00) |
					(m_live_state.shift_reg & 0x0100 ? 0x10 : 0x00) |
					(m_live_state.shift_reg & 0x0040 ? 0x08 : 0x00) |
					(m_live_state.shift_reg & 0x0010 ? 0x04 : 0x00) |
					(m_live_state.shift_reg & 0x0004 ? 0x02 : 0x00) |
					(m_live_state.shift_reg & 0x0001 ? 0x01 : 0x00),
					m_live_state.bit_counter);

			// Repeat until we have collected 16 bits
			if (m_live_state.bit_counter & 15) break;

			// Fill this value into the next slot. We expect three A1 values.
			int slot = m_live_state.bit_counter >> 4;

			if (slot < 3)
			{
				if (m_live_state.shift_reg != 0x4489)
				{
					wait_for_realtime(SEARCH_DAM_FAILED);
					return;
				}
				else
					if (TRACE_LIVE) logerror("%s: [%s] Found an A1 mark\n", tag(),tts(m_live_state.time).cstr());
				// Continue
				break;
			}

			if (TRACE_LIVE) logerror("%s: [%s] Found data value %02X\n", tag(),tts(m_live_state.time).cstr(), m_live_state.data_reg);

			if ((m_live_state.data_reg & 0xff) == 0xf8)
			{
				if (TRACE_LIVE) logerror("%s: Found deleted data mark F8 after DAM sync\n", tag());
				set_bits(m_register_r[CHIP_STATUS], CS_DELDATA, true);
			}
			else
			{
				if ((m_live_state.data_reg & 0xff) != 0xfb)
				{
					if (TRACE_LIVE) logerror("%s: Missing FB/F8 data mark after DAM sync\n", tag());
					wait_for_realtime(SEARCH_DAM_FAILED);
					return;
				}
			}

			m_live_state.bit_counter = 0;
			m_live_state.state = READ_SECTOR_DATA;
			break;
		}
		case SEARCH_DAM_FAILED:
			if (TRACE_LIVE) logerror("%s: SEARCH_DAM failed\n", tag());
			m_live_state.state = IDLE;
			return;

		case READ_SECTOR_DATA:
		{
			if (TRACE_LIVE && m_last_live_state != READ_SECTOR_DATA)
				logerror("%s: [%s] READ_SECTOR_DATA\n", tag(),tts(m_live_state.time).cstr());
			m_last_live_state = m_live_state.state;

			if(read_one_bit(limit))
				return;

			// Request bus release at the first bit of each byte (floppy; [1], fig 5 and 6)
			if ((m_command & 0x01)!=0) // transfer enabled
			{
				if ((m_live_state.bit_counter & 15)== 1)
				{
					set_bits(m_register_r[INT_STATUS], ST_OVRUN, true);
					m_out_dmarq(ASSERT_LINE);
				}
			}

			// Repeat until we have collected 16 bits
			if (m_live_state.bit_counter & 15) break;

			if (TRACE_LIVE) logerror("%s: [%s] Found data value %02X, CRC=%04x\n", tag(),tts(m_live_state.time).cstr(), m_live_state.data_reg, m_live_state.crc);
			int slot = (m_live_state.bit_counter >> 4)-1;

			if (slot < get_sector_size())
			{
				// Sector data
				wait_for_realtime(READ_SECTOR_DATA1);
				return;
			}
			else if (slot < get_sector_size()+2)
			{
				// CRC
				if (slot == get_sector_size()+1)
				{
					if (TRACE_LIVE) logerror("%s: [%s] Sector read completed\n", tag(),tts(m_live_state.time).cstr());
					wait_for_realtime(IDLE);
					return;
				}
			}
			break;
		}

		case READ_SECTOR_DATA1:
			if (TRACE_LIVE && m_last_live_state != READ_SECTOR_DATA1)
				logerror("%s: [%s] READ_SECTOR_DATA1\n", tag(),tts(m_live_state.time).cstr());
			m_last_live_state = m_live_state.state;

			// Did the system CPU send the DMA ACK in the meantime?
			if ((m_register_r[INT_STATUS] & ST_OVRUN)!=0)
			{
				if (TRACE_LIVE) logerror("%s: No DMA ACK - buffer overrun\n", tag());
				set_bits(m_register_r[INT_STATUS], TC_DATAERR, true);
				m_live_state.state = IDLE;
				return;
			}

			if ((m_command & 0x01)!=0)  // transfer enabled
			{
				m_out_dip(ASSERT_LINE);
				m_out_dma(0, m_live_state.data_reg, 0xff);
				m_out_dip(CLEAR_LINE);

				m_out_dmarq(CLEAR_LINE);
			}

			m_live_state.state = READ_SECTOR_DATA;
			checkpoint();
			break;

		default:
			logerror("%s: Unknown live state: %02x\n", tag(), m_live_state.state);
			m_last_live_state = m_live_state.state;
			return;
		}
	}
	m_last_live_state = UNDEF;
}

/*
    Synchronize the live position on the track with the real time.
    Results in a new checkpoint and a live position at machine time or behind.
    As a side effect, portions of the track may be re-read
*/
void hdc9234_device::live_sync()
{
	// Do we have some time set?
	if (!m_live_state.time.is_never())
	{
		// Are we ahead of the machine time?
		if(m_live_state.time > machine().time())
		{
			// If so, we must roll back to the last checkpoint
			if (TRACE_SYNC) logerror("%s: [%s] Rolling back and replaying (%s)\n", tag(), ttsn().cstr(), tts(m_live_state.time).cstr());
			rollback();
			// and replay until we reach the machine time
			live_run_until(machine().time());
			// Caught up, commit that
			m_pll.commit(m_floppy, m_live_state.time);
		}
		else
		{
			// We are behind machine time, so we will never get back to that
			// time, thus we can commit that position
			if (TRACE_SYNC) logerror("%s: [%s] Committing (%s)\n", tag(), ttsn().cstr(), tts(m_live_state.time).cstr());
			m_pll.commit(m_floppy, m_live_state.time);

			if (m_live_state.next_state != -1)
			{
				m_live_state.state = m_live_state.next_state;
			}

			if (m_live_state.state == IDLE)
			{
				m_pll.stop_writing(m_floppy, m_live_state.time);
				m_live_state.time = attotime::never;
			}
		}

		m_live_state.next_state = -1;
		checkpoint();
	}
}

void hdc9234_device::rollback()
{
	m_live_state = m_checkpoint_state;
	m_pll = m_checkpoint_pll;
}

/*
    Wait for real time to catch up. This way we pretend that the last
    operation actually needed the real time.
*/
void hdc9234_device::wait_for_realtime(int state)
{
	m_live_state.next_state = state;
	m_timer->adjust(m_live_state.time - machine().time());
	if (TRACE_LIVE) logerror("%s: [%s] Waiting for real time [%s] to catch up\n", tag(), tts(m_live_state.time).cstr(), tts(machine().time()).cstr());
}

/*
    Read the next bit from the disk.
    Return true: the time limit has been reached
    Return false: The next bit is read into the shift register as the
    rightmost bit; the shift register is a member of m_live_state. Also,
    the CRC is updated.
*/
bool hdc9234_device::read_one_bit(const attotime &limit)
{
	// Get the next bit from the phase-locked loop.
	int bit = m_pll.get_next_bit(m_live_state.time, m_floppy, limit);

	// We have reached the time limit
	if (bit < 0) return true;

	// For test purposes: Drop a bit at some occasions
	// value > 1000: rare occasions
	// value = 500: can cope with
	// value < 100: big trouble for controller, will fail
	if (UNRELIABLE_MEDIA)
	{
		if ((machine().time().attoseconds % 1009)==0) bit = 0;
	}

	// Push into shift register
	m_live_state.shift_reg = (m_live_state.shift_reg << 1) | bit;
	m_live_state.bit_counter++;

	// Used for timeout handling
	m_live_state.bit_count_total++;

	// Clock bit (false) or data bit (true)?
	if (m_live_state.data_separator_phase==true)
	{
		m_live_state.data_reg = (m_live_state.data_reg << 1) | bit;
		// Update CRC
		if ((m_live_state.crc ^ (bit ? 0x8000 : 0x0000)) & 0x8000)
			m_live_state.crc = (m_live_state.crc << 1) ^ 0x1021;
		else
			m_live_state.crc = m_live_state.crc << 1;
	}

	m_live_state.data_separator_phase = !m_live_state.data_separator_phase;
	return false;
}

void hdc9234_device::pll_reset(const attotime &when)
{
	m_pll.reset(when);
	// In FM mode, cells are 4 ??s long; in MFM they are 2 ??s long.
	m_pll.set_clock(attotime::from_usec(fm_mode()? 4 : 2));
}

void hdc9234_device::checkpoint()
{
	m_pll.commit(m_floppy, m_live_state.time);
	m_checkpoint_state = m_live_state;
	m_checkpoint_pll = m_pll;
}

// ===========================================================================

/*
    This is pretty simple here, compared to wd17xx, because index and ready
    callbacks have to be tied to the controller board outside the chip.
*/
void hdc9234_device::connect_floppy_drive(floppy_image_device* floppy)
{
	m_floppy = floppy;
}

/*
    Read a byte of data from the controller
    The address (offset) encodes the C/D* line (command and /data)
*/
READ8_MEMBER( hdc9234_device::read )
{
	UINT8 reply = 0;

	if ((offset & 1) == 0)
	{
		// Data register
		reply = m_register_r[m_register_pointer];
		if (TRACE_REG) logerror("%s: read register[%d] -> %02x\n", tag(), m_register_pointer, reply);

		// Autoincrement until DATA is reached.
		if (m_register_pointer < DATA)  m_register_pointer++;
	}
	else
	{
		// Status register
		reply = m_register_r[INT_STATUS];

		// "The interrupt pin is reset to its inactive state
		// when the UDC interrupt status register is read." [1] (p.3)
		if (TRACE_REG) logerror("%s: read interrupt status register -> %02x\n", tag(), reply);
		set_interrupt(CLEAR_LINE);

		// Clear the bits due to interrupt status register read.
		m_register_r[INT_STATUS] &= ~(ST_INTPEND | ST_RDYCHNG);
	}
	return reply;
}

/*
    Write a byte to the controller
    The address (offset) encodes the C/D* line (command and /data), so there
    are only two addresses: 0 (register) and 1 (command).
    The operation terminates immediately, and the controller picks up the
    values stored in this phase at a later time.
*/
WRITE8_MEMBER( hdc9234_device::write )
{
	m_data = data & 0xff;

	if ((offset & 1) == 0)
	{
		wait_time(m_cmd_timer, attotime::from_nsec(REGISTER_COMMIT), 0);
	}
	else
	{
		if (m_command != NOCMD)
		{
			logerror("%s: [%s] Error - previous command %02x not completed; new command %02x ignored\n", tag(), ttsn().cstr(), m_command, m_data);
		}
		else
		{
			wait_time(m_cmd_timer, attotime::from_nsec(COMMAND_COMMIT), 1);
		}
	}
}

/*
    When the commit period has passed, process the command.
*/
void hdc9234_device::command_continue()
{
	// Reset DONE and BAD_SECTOR [1], p.7
	set_bits(m_register_r[INT_STATUS], ST_DONE | ST_BADSECT, false);

	// Reset interrupt line (not explicitly mentioned in spec, but seems reasonable
	set_interrupt(CLEAR_LINE);

	// Clear Interrupt Pending and Ready Change
	set_bits(m_register_r[INT_STATUS], ST_INTPEND | ST_RDYCHNG, false);

	m_command = m_data;
	m_stop_after_index = false;
	m_wait_for_index = false;

	int index = 0;
	m_main_state = UNDEF;
	while (s_command[index].mask!=0 && m_main_state == UNDEF)
	{
		if ((m_command & s_command[index].mask) == s_command[index].baseval)
		{
			// Invoke command
			m_main_state = s_command[index].state;
			(this->*s_command[index].command)();
		}
		index++;
	}
	if (m_main_state==UNDEF)
	{
		logerror("%s: Command %02x not defined\n", tag(), m_command);
	}
}

/*
    When the commit period has passed, process the register write operation.
*/
void hdc9234_device::register_write_continue()
{
	// Writing data to registers
	// Data register
	if (TRACE_REG)
	{
		if (m_register_pointer == INT_COMM_TERM)
			logerror("%s: Setting interrupt trigger DONE=%d READY=%d\n", tag(), (m_data & TC_INTDONE)? 1:0, (m_data & TC_INTRDCH)? 1:0);
		else
			logerror("%s: register[%d] <- %02x\n", tag(), m_register_pointer, m_data);
	}
	m_register_w[m_register_pointer] = m_data;

	// The DMA registers and the sector register for read and
	// write are identical, so in that case we copy the contents
	if (m_register_pointer < DESIRED_HEAD) m_register_r[m_register_pointer] = m_data;

	// Autoincrement until DATA is reached.
	if (m_register_pointer < DATA)  m_register_pointer++;
}

/*
    Auxiliary bus operation.

    The auxbus of the HDC9234 is used to poll the drive status of the cur-
    rently selected drive, to transmit DMA address bytes, to output the
    OUTPUT1 register, and to output the OUTPUT2 register.

    The specification is not really precise on the times when this bus is
    used, but at least we can rely on this information:

    - Whenever there is no output of data, the bus is sampled. ([1], p.8,
      Drive status register). Data is sampled at the rising edge of STB*.
      As the minimum STB* pulse is 800ns with min 100ns S0/S1 settling time
      and min 100ns hold time we can say that the bus is polled at a maximum
      rate of 1 MHz.

    - Data for the DMA address is output only when the address is initially
      set; also when the address must be set again on error ([1], p.5,
      DMA registers). The external memory system has to take care of the
      addressing for subsequent bytes. The address will be increased by the
      length of a sector during multiple sector read/write operations.

    We may assume that the OUTPUT1 and OUTPUT2 operations only occur on
    changes to the registers in the controller. The values showing up on the
    auxiliary bus must be latched anyway.

    For the sampling of drive status values, the emulation would have to
    invoke a callback to the hosting board at a rate of about 1 MHz. Since
    the devices like floppy or hard disks are pushing their status changes,
    it makes much more sense to allow for an incoming call to the controller
    instead of a polling. This also allows to raise interrupts as soon
    as the drive status changes. The difference to the real controller
    would be less than 3 microseconds (in the worst case when the auxbus is
    currently outputting data as the drive status change occurs).

    Drive status read
    S0 = 0, S1 = 0
    +------+------+------+------+------+------+------+------+
    | ECC  |Index | SeekC| Tr00 | User | WrPrt| Ready|Fault |
    +------+------+------+------+------+------+------+------+


    OUTPUT1 register contents
    S0 = 0, S1 = 1
    +------+------+------+------+------+------+------+------+
    | Drv3 | Drv2 | Drv1 | Drv0 |  PO3 |  PO2 |  PO1 |  PO0 |
    +------+------+------+------+------+------+------+------+

    DrvX = select Drive X (only one bit allowed)
    POX = Programmable output X (contents from low 4 bits of register RETRY_COUNT)


    OUTPUT2 register contents
    S0 = 1, S1 = 1
    +------+------+------+------+------+------+------+------+
    | Drv3*| WCur | Dir  | Step |           Head            |
    +------+------+------+------+------+------+------+------+

    Drv3* = inverted Drv3 signal of OUTPUT1
    WCur = Reduced write current
    Dir = Step direction (0 = towards track 0)
    Step = Step pulse
    Head = desired head
*/

/*
    Read the drive status over the auxbus
    (as said, let the controller board push the values into the controller)
*/
void hdc9234_device::auxbus_in(UINT8 data)
{
	// Kill unwanted input via auxbus until we are initialized.
	if (!m_initialized)
		return;

	if (TRACE_ACT) logerror("%s: Got value %02x via auxbus: ecc=%d index=%d seek_comp=%d tr00=%d user=%d writeprot=%d ready=%d fault=%d\n",
				tag(), data,
				(data&HDC_DS_ECCERR)? 1:0, (data&HDC_DS_INDEX)? 1:0,
				(data&HDC_DS_SKCOM)? 1:0, (data&HDC_DS_TRK00)? 1:0,
				(data&HDC_DS_UDEF)? 1:0, (data&HDC_DS_WRPROT)? 1:0,
				(data&HDC_DS_READY)? 1:0, (data&HDC_DS_WRFAULT)? 1:0);
	UINT8 prev = m_register_r[DRIVE_STATUS];
	m_register_r[DRIVE_STATUS] = data;

	if ((prev & HDC_DS_INDEX) != (data &  HDC_DS_INDEX))
	{
		// Check whether index value changed
		index_callback((data & HDC_DS_INDEX)? ASSERT_LINE : CLEAR_LINE);
	}

	if ((prev & HDC_DS_READY) != (data & HDC_DS_READY))
	{
		// Check whether ready value changed
		ready_callback((data & HDC_DS_READY)? ASSERT_LINE : CLEAR_LINE);
	}

	if ((prev & HDC_DS_SKCOM) != (data & HDC_DS_SKCOM))
	{
		// Check whether seek complete value changed
		seek_complete_callback((data & HDC_DS_SKCOM)? ASSERT_LINE : CLEAR_LINE);
	}
}

void hdc9234_device::index_callback(int level)
{
	if (TRACE_ACT) logerror("%s: [%s] Index callback level=%d\n", tag(), ttsn().cstr(), level);

	// Synchronize our position on the track
	live_sync();

	if (level==CLEAR_LINE) {
		general_continue();
		return;
	}
	else
	{
		// ...
		if (m_wait_for_index) m_stop_after_index = true;
		general_continue();
	}
}

void hdc9234_device::ready_callback(int level)
{
	if (TRACE_ACT) logerror("%s: [%s] Ready callback level=%d\n", tag(), ttsn().cstr(), level);

	// Set the interrupt status flag
	set_bits(m_register_r[INT_STATUS], ST_RDYCHNG, true);

	// Synchronize our position on the track
	live_sync();

	// Raise an interrupt if desired
	if (m_register_w[INT_COMM_TERM] & TC_INTRDCH)
	{
		if (TRACE_ACT) logerror("%s: Raise interrupt READY change\n", tag());
		set_interrupt(ASSERT_LINE);
	}
}

void hdc9234_device::seek_complete_callback(int level)
{
	if (TRACE_ACT) logerror("%s: [%s] Seek complete callback level=%d\n", tag(), ttsn().cstr(), level);

	// Synchronize our position on the track
	live_sync();

	if (level==ASSERT_LINE && m_next_state != UNDEF)
	{
		m_substate = m_next_state;
		general_continue();
	}
}

void hdc9234_device::wait_line(int substate)
{
	m_next_state = substate;
}

bool hdc9234_device::on_track00()
{
	return (m_register_r[DRIVE_STATUS] & HDC_DS_TRK00)!=0;
}

/*
    Push the output registers over the auxiliary bus. It is expected that
    the PCB contains latches to store the values.
*/
void hdc9234_device::sync_latches_out()
{
	if (TRACE_ACT) logerror("%s: Setting OUTPUT1 to %02x\n", tag(), m_output1);
	m_out_auxbus((offs_t)HDC_OUTPUT_1, m_output1);
	set_bits(m_output2, OUT2_DRVSEL3I, (m_output1 & 0x80)==0);
	if (TRACE_ACT) logerror("%s: Setting OUTPUT2 to %02x\n", tag(), m_output2);
	m_out_auxbus((offs_t)HDC_OUTPUT_2, m_output2);
}

void hdc9234_device::dma_address_out()
{
	if (TRACE_ACT) logerror("%s: Setting DMA address %06x\n", tag(), (m_register_w[DMA23_16]<<16 | m_register_w[DMA15_8]<<8 | m_register_w[DMA7_0])&0xffffff);
	m_out_auxbus((offs_t)HDC_OUTPUT_DMA_ADDR, m_register_w[DMA23_16]);
	m_out_auxbus((offs_t)HDC_OUTPUT_DMA_ADDR, m_register_w[DMA15_8]);
	m_out_auxbus((offs_t)HDC_OUTPUT_DMA_ADDR, m_register_w[DMA7_0]);
}

/*
    This is reached when a timer has expired
*/
void hdc9234_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (TRACE_ACT) logerror("%s: [%s] Timer id=%d expired\n", tag(), ttsn().cstr(), id);
	live_sync();

	switch (id)
	{
	case GEN_TIMER:
		general_continue();
		break;
	case COM_TIMER:
		if (m_substate==1) command_continue();
		else register_write_continue();
		break;
	case LIVE_TIMER:
		live_run();
		break;
	}
}

/*
    DMA acknowledge line.
*/
WRITE_LINE_MEMBER( hdc9234_device::dmaack )
{
	if (state==ASSERT_LINE)
	{
		if (TRACE_ACT) logerror("%s: [%s] DMA acknowledged\n", tag(), ttsn().cstr());
		set_bits(m_register_r[INT_STATUS], ST_OVRUN, false);
	}
}

/*
    Reset the controller. Negative logic, but we use ASSERT_LINE.
*/
WRITE_LINE_MEMBER( hdc9234_device::reset )
{
	if (state == ASSERT_LINE)
	{
		if (TRACE_ACT) logerror("%s: Reset via RST line\n", tag());
		device_reset();
	}
}

void hdc9234_device::device_start()
{
	logerror("%s: Start\n", tag());
	m_out_intrq.resolve_safe();
	m_out_dip.resolve_safe();
	m_out_auxbus.resolve_safe();
	m_out_dmarq.resolve_safe();
	m_out_dma.resolve_safe();
	m_in_dma.resolve_safe(0);

	// allocate timers
	m_timer = timer_alloc(GEN_TIMER);
	m_cmd_timer = timer_alloc(COM_TIMER);
	m_live_timer = timer_alloc(LIVE_TIMER);

	m_live_state.state = IDLE;
}

void hdc9234_device::device_reset()
{
	logerror("%s: Reset\n", tag());

	m_selected_drive_type = 0;
	m_head_load_delay_enable = false;
	m_register_pointer = 0;
	m_output1 = 0;
	m_output2 = 0x80;

	set_interrupt(CLEAR_LINE);
	m_out_dip(CLEAR_LINE);
	m_out_dmarq(CLEAR_LINE);

	for (int i=0; i<=11; i++)
		m_register_r[i] = m_register_w[i] = 0;

	m_step_direction = 0;

	m_next_state = IDLE;
	m_seek_count = 0;

	m_live_state.time = attotime::never;
	m_live_state.state = IDLE;
	m_initialized = true;

	m_track_delta = 0;

	m_multi_sector = false;
	m_retry_save = 0;

	m_substate = IDLE;
	m_main_state = IDLE;
	m_command = NOCMD;

	m_stop_after_index = false;
	m_wait_for_index = false;

	m_data = 0;
}

const device_type HDC9234 = &device_creator<hdc9234_device>;
