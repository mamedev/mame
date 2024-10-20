// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

Western Digital WD2010 Winchester Disk Controller

Portions (2015, 2017) : Karl-Ludwig Deisenhofer
**********************************************************************

Implements WD2010 / WD1010 controller basics for a single hard disk.

 Provides IRQ / (B)DRQ signals needed for early MFM cards.
 Honors DRIVE_READY and WRITE FAULT (DRDY / WF).

 Single sector read / write (format) confirmed to work with
 Rainbow-100 controller (WD1010, quite compatible to WD2010, see **)

UNIMPLEMENTED FEATURES :
        - more than 1 drive (untested)
    - multi sector transfers (M = 1)
        - seek and index timers / ID not found.
        - implied seeks / implied writes / retries
        - edge or level triggered seek complete (SC)
        - set_parameter / compute_correction
      (the DWC flag is not usable in this context).

 Pseudo code (from datasheet) left in to illustrate
 the intended instruction flow. Some loops were omitted!

 USAGE:  tie WF (write fault) to ground if not needed:
 in_wf_callback().set_constant(0)

 Other signals should be set to VCC if not serviced:
 in_drdy_callback().set_constant(1)   // DRIVE READY = VCC
 in_sc_callback().set_constant(1)     // SEEK COMPLETE = VCC
 **********************************************************************/

#include "emu.h"
#include "wd2010.h"

//#define VERBOSE 1
#include "logmacro.h"

#include <cmath>
//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

// WD 2010 CONFIGURATION (2048 cylinder limit)
#define STEP_LIMIT 2048
#define CYLINDER_HIGH_MASK 0x07

// DEC RD51 chip; different STEP / CYLINDER LIMIT (**):

// WD 1010 CONFIGURATION (1024 cylinder limit)
// #define  STEP_LIMIT 1024
// #define CYLINDER_HIGH_MASK 0x03

// --------------------------------------------------------
// Maximum sector number before ID not found is returned.
#define MAX_MFM_SECTORS 32
// --------------------------------------------------------

// Typical access times for MFM drives (as listed in ST412_OEM Manual_Apr82)
#define SETTLING_MS 15.0
#define LATENCY_MS 8.33

// Step rates in ms for 5 Mhz WCLK (35 uS when zero)
#define STEP_RATE_MS \
	(float) ( (data & 0x0f) ? ((data & 0x0f) * 0.5) : 0.035 )


// task file
enum
{
	TASK_FILE_ERROR = 1,
	TASK_FILE_WRITE_PRECOMP_CYLINDER = TASK_FILE_ERROR,
	TASK_FILE_SECTOR_COUNT,
	TASK_FILE_SECTOR_NUMBER,
	TASK_FILE_CYLINDER_LOW,
	TASK_FILE_CYLINDER_HIGH,
	TASK_FILE_SDH_REGISTER,
	TASK_FILE_STATUS,
	TASK_FILE_COMMAND = TASK_FILE_STATUS
};

#define WRITE_PRECOMP_CYLINDER \
	(m_task_file[TASK_FILE_WRITE_PRECOMP_CYLINDER] * 4)

#define SECTOR_COUNT \
	((m_task_file[TASK_FILE_SECTOR_COUNT] + 1) * 256)

#define SECTOR_NUMBER \
	(m_task_file[TASK_FILE_SECTOR_NUMBER])

#define CYLINDER \
	(((m_task_file[TASK_FILE_CYLINDER_HIGH] & CYLINDER_HIGH_MASK) << 8) | m_task_file[TASK_FILE_CYLINDER_LOW])

#define HEAD \
	(m_task_file[TASK_FILE_SDH_REGISTER] & 0x07)

#define DRIVE \
	((m_task_file[TASK_FILE_SDH_REGISTER] >> 3) & 0x03)

#define SECTOR_SIZE \
	SECTOR_SIZES[(m_task_file[TASK_FILE_SDH_REGISTER] >> 5) & 0x03]

static constexpr int SECTOR_SIZES[4] = { 256, 512, 1024, 128 };

// status register
#define STATUS_BSY      0x80
#define STATUS_RDY      0x40
#define STATUS_WF       0x20
#define STATUS_SC       0x10
#define STATUS_DRQ      0x08
#define STATUS_DWC      0x04
#define STATUS_CIP      0x02
#define STATUS_ERR      0x01


// error register
#define ERROR_BB        0x80
#define ERROR_CRC_ECC   0x40
#define ERROR_ID        0x10
#define ERROR_AC        0x04
#define ERROR_TK        0x02
#define ERROR_DM        0x01


// command register
#define COMMAND_MASK                0xf0
#define COMMAND_RESTORE             0x10
#define COMMAND_SEEK                0x70
#define COMMAND_READ_SECTOR         0x20
#define COMMAND_WRITE_SECTOR        0x30
#define COMMAND_SCAN_ID             0x40
#define COMMAND_WRITE_FORMAT        0x50
#define COMMAND_COMPUTE_CORRECTION  0x08
#define COMMAND_SET_PARAMETER_MASK  0xfe
#define COMMAND_SET_PARAMETER       0x00

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(WD2010, wd2010_device, "wd2010", "Western Digital WD2010 Winchester Disk Controller")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  wd2010_device - constructor
//-------------------------------------------------

wd2010_device::wd2010_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, WD2010, tag, owner, clock)
	, m_out_intrq_cb(*this)
	, m_out_bdrq_cb(*this)
	, m_out_bcr_cb(*this)
	, m_in_bcs_cb(*this, 0)
	, m_in_brdy_cb(*this, 0)
	, m_out_bcs_cb(*this)
	, m_out_dirin_cb(*this)
	, m_out_step_cb(*this)
	, m_out_rwc_cb(*this)
	, m_out_wg_cb(*this)
	, m_in_drdy_cb(*this, 0)
	, m_in_index_cb(*this, 0)
	, m_in_wf_cb(*this, 0)
	, m_in_tk000_cb(*this, 0)
	, m_in_sc_cb(*this, 0)
	, m_status(0)
	, m_error(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void wd2010_device::device_start()
{
	// allocate a timer for commands
	m_cmd_timer = timer_alloc(FUNC(wd2010_device::command_complete), this);
	m_complete_write_timer = timer_alloc(FUNC(wd2010_device::complete_write), this);
	m_deassert_write_timer = timer_alloc(FUNC(wd2010_device::deassert_write), this);
	m_deassert_read_timer = timer_alloc(FUNC(wd2010_device::deassert_read), this);
	m_next_sector_timer =  timer_alloc(FUNC(wd2010_device::next_sector), this);
	m_present_cylinder = 0; // start somewhere
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void wd2010_device::device_reset()
{
	m_out_intrq_cb(CLEAR_LINE);

	buffer_ready(false);
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t wd2010_device::read(offs_t offset)
{
	uint8_t data;

	switch (offset)
	{
	case TASK_FILE_ERROR:
		if (m_status & STATUS_CIP) // "if other registers are read while CIP, the status register contents are returned."
			data = (m_in_drdy_cb() ? 0x40 : 0) | (m_in_wf_cb() ? 0x20 : 0) | (m_in_sc_cb() ? 0x10 : 0) | m_status;// see STATUS register
		else
			data = m_error;
		break;

	case TASK_FILE_STATUS:
			m_out_intrq_cb(CLEAR_LINE); // "reading the status register clears INTRQ" (-> datasheet)
			data = (m_in_drdy_cb() ? 0x40 : 0) | (m_in_wf_cb() ? 0x20 : 0) | (m_in_sc_cb() ? 0x10 : 0) | m_status;// see ERROR register
		break;

	default:
		data = m_task_file[offset];

		if (offset == TASK_FILE_SDH_REGISTER)
		{
			LOG("(READ) %s WD2010 SDH: %u\n", machine().describe_context(), data);
			LOG("(READ) %s WD2010 Head: %u\n", machine().describe_context(), HEAD);
			LOG("(READ) %s WD2010 Drive: %u\n", machine().describe_context(), DRIVE);
			LOG("(READ) %s WD2010 Sector Size: %u\n", machine().describe_context(), SECTOR_SIZE);
		}

		break;
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void wd2010_device::write(offs_t offset, uint8_t data)
{
	m_task_file[offset] = data;

	switch (offset)
	{
	case TASK_FILE_WRITE_PRECOMP_CYLINDER:
		LOG("%s WD2010 Write Precomp Cylinder: %u\n", machine().describe_context(), WRITE_PRECOMP_CYLINDER);
		break;

	case TASK_FILE_SECTOR_COUNT:
		LOG("%s WD2010 Sector Count: %u\n", machine().describe_context(), SECTOR_COUNT);
		break;

	case TASK_FILE_SECTOR_NUMBER:
		LOG("%s WD2010 Sector Number: %u\n", machine().describe_context(), SECTOR_NUMBER);
		break;

	case TASK_FILE_CYLINDER_LOW:
		LOG("%s WD2010 Cylinder (lower bits set): %u\n", machine().describe_context(), CYLINDER);
		break;

	case TASK_FILE_CYLINDER_HIGH:
		LOG("%s WD2010 Cylinder (MSB bits set): %u\n", machine().describe_context(), CYLINDER);
		break;

	case TASK_FILE_SDH_REGISTER:
		LOG("(WRITE) %s WD2010 SDH: %u\n", machine().describe_context(), data);
		LOG("(WRITE) %s WD2010 Head: %u\n", machine().describe_context(), HEAD);
		LOG("(WRITE) %s WD2010 Drive: %u\n", machine().describe_context(), DRIVE);
		LOG("(WRITE) %s WD2010 Sector Size: %u\n", machine().describe_context(), SECTOR_SIZE);
		break;

	case TASK_FILE_COMMAND:
		m_out_intrq_cb(CLEAR_LINE); // "either reading the status register or writing a new command clears INTRQ"
		m_status &= ~(STATUS_ERR | STATUS_BSY | STATUS_CIP);  // "Reset ERR bit in STATUS upon new cmd" (see datasheet)
		m_error = 0;

		if (data == COMMAND_COMPUTE_CORRECTION)
		{
			LOG("%s WD2010 COMPUTE CORRECTION\n", machine().describe_context());
			compute_correction(data);
		}
		else if ((data & COMMAND_SET_PARAMETER_MASK) == COMMAND_SET_PARAMETER)
		{
			LOG("%s WD2010 SET PARAMETER\n", machine().describe_context());
			set_parameter(data);
		}
		else
		{
			switch (data & COMMAND_MASK)
			{
			case COMMAND_RESTORE:
				LOG("%s WD2010 RESTORE\n", machine().describe_context());
				restore(data);
				break;

			case COMMAND_SEEK:
				LOG("%s WD2010 SEEK\n", machine().describe_context());
				seek(data);
				break;

			case COMMAND_READ_SECTOR:
				LOG("%s WD2010 READ SECTOR (I = %u) (M = %u)\n", machine().describe_context(), ((data & 8)>0), ((data & 4)>0));
				read_sector(data);
				break;

			case COMMAND_WRITE_SECTOR:
				LOG("%s WD2010 WRITE SECTOR (M = %u)\n", machine().describe_context(), ((data & 4) > 0));
				write_sector(data);
				break;

			case COMMAND_SCAN_ID:
				LOG("%s WD2010 SCAN ID\n", machine().describe_context());
				scan_id(data);
				break;

			case COMMAND_WRITE_FORMAT:
				LOG("%s WD2010 WRITE FORMAT\n", machine().describe_context());
				format(data);
				break;
			}

		}

		break;
	} // switch
}


//-------------------------------------------------
//  compute_correction -
//-------------------------------------------------
void wd2010_device::compute_correction(uint8_t data)
{
	uint8_t newstatus = STATUS_RDY | STATUS_SC;
	complete_cmd(newstatus);
}


//-------------------------------------------------
//  set_parameter -
//-------------------------------------------------
void wd2010_device::set_parameter(uint8_t data)
{
	uint8_t newstatus = STATUS_RDY | STATUS_SC;
	complete_cmd(newstatus);
}


//-------------------------------------------------
//  restore -
//-------------------------------------------------
void wd2010_device::restore(uint8_t data)
{
	uint8_t newstatus = STATUS_RDY | STATUS_SC;

	m_out_intrq_cb(CLEAR_LINE); // reset INTRQ, errors, set BUSY, CIP
	m_error = 0;
	m_status = STATUS_BSY | STATUS_CIP;

	m_out_rwc_cb(0); // reset RWC, set direction = OUT

	// Datasheet: DIRIN HIGH = in ;  LOW = out
	m_out_dirin_cb(0); // 0 = heads move away from the spindle, towards track O.

	// Omitted: store step rate for later (implied seeks).

	int step_pulses = 0;
	while (step_pulses <= STEP_LIMIT)
	{
		while (!m_in_sc_cb())
		{
			if (!m_in_drdy_cb() || m_in_wf_cb()) // drive not ready or write fault?
			{
				m_out_bcr_cb(0); // pulse BCR
				m_out_bcr_cb(1);

				m_error = ERROR_AC; // ERROR : ABORTED COMMAND
				complete_cmd(newstatus | STATUS_ERR);
				return;
			}
		}

		if ( m_in_tk000_cb() || (step_pulses == STEP_LIMIT) ) // Simulate TRACK 00 signal (normally from DRIVE)
		{
			m_present_cylinder = 0;
			m_task_file[TASK_FILE_CYLINDER_HIGH] = 0;
			m_task_file[TASK_FILE_CYLINDER_LOW] = 0;

			m_out_bcr_cb(0); // pulse BCR
			m_out_bcr_cb(1);
			newstatus &= ~(STATUS_BSY | STATUS_CIP); // prepare new status; (INTRQ later) reset BSY, CIP

			// NOTE: calculation needs 'data' (extracted from command register)
			float step_ms = SETTLING_MS + LATENCY_MS + ( (float)sqrt(1.0 * step_pulses) * STEP_RATE_MS );

			m_cmd_timer->adjust(attotime::from_usec(1000 * step_ms), newstatus);
			return;
		}

		m_out_step_cb(1); // issue a step pulse
		m_out_step_cb(0);
		step_pulses++;
	}

	assert(1);
}

//-------------------------------------------------
//  seek -
//-------------------------------------------------

// FIXME : drive change (untested)

// Not implemented: IMPLIED SEEK ("wait until rising edge of SC signal")
// Also, step rate for implied seeks _should be_ taken from previous restore.

void wd2010_device::seek(uint8_t data)
{
	uint8_t newstatus = STATUS_RDY | STATUS_SC;

	m_out_intrq_cb(CLEAR_LINE); // reset INTRQ, errors, set BUSY, CIP
	m_error = 0;
	m_status = STATUS_BSY | STATUS_CIP;

	auto_scan_id(data); // has drive number changed?

	int direction; // 0 = towards 0
	int step_pulses;

	// Calculate number of steps by comparing the cylinder registers
	//           HI/LO with the internally stored position.
	uint32_t cylinder_registers = CYLINDER;
	if (m_present_cylinder > cylinder_registers)
	{
		step_pulses = m_present_cylinder - cylinder_registers;
		direction = 0;
	}
	else
	{
		step_pulses = cylinder_registers - m_present_cylinder;
		direction = 1;
	}

	// NOTE: calculation needs 'step_pulses' and 'data' (taken from command register)
	float step_ms = SETTLING_MS + LATENCY_MS + ( (float)sqrt(1.0 * step_pulses) * STEP_RATE_MS );

	m_out_dirin_cb(direction);

	if (!m_in_drdy_cb() || m_in_wf_cb()) // DRDY de-asserted or WF asserted?
	{
		m_error = ERROR_AC;
		complete_cmd(newstatus | STATUS_ERR);
		return;
	}
	else
	{
		while (step_pulses > 0) // issue STEP PULSES
		{
			if (direction == 0)
			{
				m_out_step_cb(1); // issue a step pulse
				m_out_step_cb(0);

				if (m_present_cylinder > 0)
					m_present_cylinder--;
			}
			else
			{
				m_out_step_cb(0);
				m_out_step_cb(1);

				m_present_cylinder++;
			}
			step_pulses--;

			// TODO: delay according to rate field
		}

		// ALL STEPS ISSUED NOW

		if (!m_in_drdy_cb()) // DRDY not asserted = > ABORTED COMMAND
		{
			m_error = ERROR_AC;
			complete_cmd(newstatus | STATUS_ERR);
			return;
		}
	}

	// AFTER ALL STEPS ARE ISSUED ...
	// UPDATE INTERNAL CYLINDER POSITION REGISTER (from WD1010 spec -> "SEEK COMMAND")
	m_present_cylinder = cylinder_registers;

	// ...update CYLINDER registers with cylinder found -
	m_task_file[TASK_FILE_CYLINDER_HIGH] = (m_present_cylinder >> 8) & 0xff;
	m_task_file[TASK_FILE_CYLINDER_LOW] = (m_present_cylinder - ((m_task_file[TASK_FILE_CYLINDER_HIGH] << 8) )) & 0xff;

	//LOGERROR("SEEK (END) - m_present_cylinder = %u SDH CYL L/H %02x / %02x\n", m_present_cylinder,m_task_file[TASK_FILE_CYLINDER_LOW],m_task_file[TASK_FILE_CYLINDER_HIGH]);
	m_cmd_timer->adjust(attotime::from_usec(1000 * step_ms), newstatus);
}

//-------------------------------------------------
//  read_sector -
//-------------------------------------------------
// FIXME: multiple sector transfers, ID / CYL / HEAD / SIZE match
//        + ERROR HANDLING (...)
void wd2010_device::read_sector(uint8_t data)
{
	uint8_t newstatus = STATUS_RDY | STATUS_SC;
	int intrq_at_end = 0; // (default) : (I = 1  INTRQ occurs when the command

	m_out_intrq_cb(CLEAR_LINE); // reset INTRQ, errors, set BUSY, CIP
	m_error = 0;
	m_status = STATUS_BSY | STATUS_CIP;

	// Assume: drive NO # has not changed... (else: SCAN_ID; GET CYL#)
	auto_scan_id(data); // has drive number changed?

	// CYL REGISTERS and INTERNAL CYL. SAME ?
	// TODO:  < NOT SAME? THEN _SEEK_ >

	// DRIVE NOT READY?  OR  WF?
	if ( (!m_in_drdy_cb()) || m_in_wf_cb() )
	{
		m_error = ERROR_AC; // ABORTED_COMMAND
		complete_cmd(newstatus | STATUS_ERR);
		return;
	}
	else
	{
		m_out_bcs_cb(1); // activate BCS (!)

		m_out_bcr_cb(0); // strobe BCR
		m_out_bcr_cb(1);

		if (!m_in_drdy_cb()) // DRIVE NOT READY?
		{
			m_error = ERROR_AC; // ABORTED_COMMAND
			complete_cmd(newstatus | STATUS_ERR);
			return;
		}
		else
		{
			// < SEARCH FOR ID FIELD >
			// < CYL / HEAD / SEC.SIZE MATCH ? >

			// < ID NOT FOUND >
			if (SECTOR_NUMBER > MAX_MFM_SECTORS)
			{
				// prepare new status; (later IRQ +) reset BSY, CIP
				m_error = ERROR_ID;
				complete_cmd(newstatus | STATUS_ERR);
				return;
			}

			// LOOP OVER 10 INDEXES : SCAN_ID / GET CYL.# (not implemented: ID NOT FOUND)
			m_present_cylinder = CYLINDER;

			// CYL / HEAD / SEC.SIZE MATCH ? => (ID FOUND)
			//
			// NO "BAD BLOCK DETECT" (** NOT IMPLEMENTED **)
			// NO "CRC ERROR"  (** NOT IMPLEMENTED **)
			// AND "DAM FOUND" (** NOT IMPLEMENTED **)

			// ====> THEN "TRANSFER SECTOR TO BUFFER" <====

			m_out_bcr_cb(0);  // strobe BCR
			m_out_bcr_cb(1);

			// NO "CRC ERROR"

			// FLAG "M" SET? (MULTIPLE SECTOR TRANSFERS)
			if (data & 4)
				LOG("WD2010 (READ): MULTIPLE SECTOR READ (M = 1).\n");

			m_out_bcs_cb(0); // deactivate BCS (!)

			m_out_bcr_cb(0); // strobe BCR
			m_out_bcr_cb(1);

			// set BDRQ (NOTE: DRQ status bit 3 reflects state of BDRQ)
			m_status |= STATUS_DRQ;
			m_out_bdrq_cb(1);

			// reset BUSY (* after * TRANSFER OF SECTOR in READ)
			m_status &= ~(STATUS_BSY);

			// FLAG "I" SET?
			if (!(data & 8))  // (I = 0    INTRQ occurs with BDRQ/DRQ indicating the Sector Buffer is full...)
			{
				m_out_intrq_cb(ASSERT_LINE);
				if (!(data & 4))  //  (...valid only when M = 0)
					intrq_at_end = STATUS_DWC;  // 'reuse' unused DWC bit!
			}
			else
			{
				intrq_at_end = 0;  // (default): (I = 1  INTRQ occurs when the command is completed and the Host has read the Sector Buffer)
			}

			// (WAIT FOR): BRDY LOW TO HIGH?   (see -> TIMER)

		} // DRIVE_READY ? (inner)

	} // DRIVE_READY ? (outer)

	// NOTE : (intrq_at_end = 0) - INTRQ occurs when the command is completed
	newstatus |= (m_status & ~(STATUS_CIP | STATUS_DRQ)) | intrq_at_end; // de-assert CIP + DRQ (BSY already reset)

	m_deassert_read_timer->adjust(attotime::from_usec(1), newstatus); // complete command ON  *RISING EDGE * OF BUFFER_READY
}


//-------------------------------------------------
//  write_sector (stage I)
//-------------------------------------------------
// FIXME: SEEK, SEEK_COMPLETE, Drive # change (!)
// as well as CYL.register + internal CYL.register comparisons
void wd2010_device::write_sector(uint8_t data)
{
	m_error = 0; // De-assert ERROR + DRQ
	m_status &= ~(STATUS_DRQ);

	m_status = STATUS_BSY | STATUS_CIP; // Assert BUSY + CIP

	// (When drive changed) : SCAN_ID / GET CYL#
	auto_scan_id(data); // has drive number changed?

	// Assume YES : CYL.register + internal CYL.register SAME?  (if NO => SEEK!)
	// Assume : SEEK_COMPLETE = YES
	m_present_cylinder = CYLINDER;

	m_status |= STATUS_DRQ; // Assert BDRQ + DRQ (= status bit 3)
	m_out_bdrq_cb(1);

	//  WAIT UNTIL BRDY ASSERTED (-> timer):
	m_complete_write_timer->adjust(attotime::from_usec(1), data); // 1 usec
}


//-------------------------------------------------
//  write_sector (stage II)
//-------------------------------------------------
void wd2010_device::complete_write_sector(uint8_t data)
{
	uint8_t newstatus = STATUS_RDY | STATUS_SC;

	m_out_bdrq_cb(0); // DE-Assert BDRQ (...and DRQ !)
	m_status &= ~(STATUS_DRQ);

	if (!m_in_drdy_cb() || m_in_wf_cb())  //  DRIVE IS READY / NO WF?
	{
		m_error = ERROR_AC; // ABORTED_COMMAND
		complete_cmd(newstatus | STATUS_ERR);
		return;
	}
	else
	{ // --------------------------------------------------------
		// (*** UNIMPLEMENTED ***) Search for ID field...

		// < Correct ID found >

		// (*** UNIMPLEMENTED ***) : 'ID NOT FOUND' - set bit 4 error register
		// ........................:   => SCAN_ID => RE-SEEK (2-10 INDEX PULSES) / Set ERR bit 0 status register ..

		m_status &= ~(STATUS_SC); // "WRITE_GATE valid when SEEK_COMPLETE = 0" (see Rainbow 100 Addendum!)

		m_out_bcs_cb(1);
		m_out_wg_cb(1); // (!)

		m_out_bcr_cb(0); // strobe BCR
		m_out_bcr_cb(1);

		// Assume: DRIVE IS READY / NO WF

		if (!m_in_drdy_cb() || m_in_wf_cb()) // DRDY de-asserted or WF asserted?
		{
			m_error = ERROR_AC; // ABORTED_COMMAND
			complete_cmd(newstatus | STATUS_ERR);
			return;
		}
		else
		{
			// ====> WRITE DATA TO SECTOR <====

			m_out_wg_cb(0); // (!)

			// Assume: (single sector transfer; M = 0)

		} // (INNER IF): No WF and DRIVE IS READY.
	} // --------------------------------------------------------

	// 'complete_cmd' ON THE FALLING EDGE OF _BUFFER_READY_ ( set by WRITE_SECTOR ) !
	m_deassert_write_timer->adjust(attotime::from_usec(1), newstatus);
}

// ******************************************************
// AUTO SCAN-ID (whenever DRIVE # changes):
// ******************************************************
void wd2010_device::auto_scan_id(uint8_t data)
{
	static int last_drive;
	if (DRIVE != last_drive)
	{
		// FIXME: geometry of disk not available here. Assume sector size already set (?)
		update_sdh( SECTOR_SIZE, 0, 0, 1 ); // new sector_size, head, cylinder, sector

		logerror("\n(WD2010) : UNSUPPORTED DRIVE CHANGE (old = %02x, new = %02x) Sector size assumed: %d !\n", last_drive, DRIVE, SECTOR_SIZES[SECTOR_SIZE]);
	}
	last_drive = DRIVE;
	return; // (see NOTES)
}
// ******************************************************

// Update SDH register / update present_cylinder.
void wd2010_device::update_sdh(uint8_t new_sector_size, uint8_t new_head, uint16_t new_cylinder, uint8_t new_sectornr)
{
	// Update SECTOR_SIZE, HEAD in SDH with the ID found -
	m_task_file[TASK_FILE_SDH_REGISTER] &= 0x98; // mask 10011000 (size | head)
	m_task_file[TASK_FILE_SDH_REGISTER] = ((new_sector_size & 3) << 5) | (new_head & 7);

	// ...update CYLINDER registers with cylinder given -
	m_task_file[TASK_FILE_CYLINDER_HIGH] = (new_cylinder >> 8) & 0xff;
	m_task_file[TASK_FILE_CYLINDER_LOW] = (new_cylinder - ((m_task_file[TASK_FILE_CYLINDER_HIGH] << 8) )) & 0xff;

	// ...update SECTOR_NUMBER with sector nr. given -
	m_task_file[TASK_FILE_SECTOR_NUMBER] = new_sectornr;

	m_present_cylinder = CYLINDER;
	logerror("UPDATE_SDH - m_present_cylinder = %u\n", m_present_cylinder);
}

//-------------------------------------------------
//  scan_id -
//-------------------------------------------------

//  Reads the cylinder number from the track on which the heads are presently located

//  FIXME: NO ID HANDLING (ID FOUND / NOT FOUND), NO BAD BLOCK; NO CRC
void wd2010_device::scan_id(uint8_t data)
{
	uint8_t newstatus = STATUS_RDY;

	m_out_intrq_cb(CLEAR_LINE);
	m_error = 0;
	m_status = STATUS_BSY | STATUS_CIP;

	// Assume DRIVE READY.
	// < TODO: Search for ANY ID FIELD. >

	// Assume ID FOUND :
	m_task_file[TASK_FILE_CYLINDER_HIGH] = (m_present_cylinder >> 8) & 0xff;
	m_task_file[TASK_FILE_CYLINDER_LOW] = (m_present_cylinder - ((m_task_file[TASK_FILE_CYLINDER_HIGH] << 8) )) & 0xff;

	// NO BAD BLOCK.
	// NO CRC ERROR.

	complete_cmd(newstatus);
}

//--------------------------------------------------------
// FORMAT ENTIRE TRACK using the task file + sector buffer

// On real hardware, data fields are filled with FF.
// Sector buffer is used for track layout (- datasheet).

// This routine does just enough to keep formatter
// programs happy (no need to low level format a CHD).

// < UNIMPLEMENTED: (IMPLIED) SEEKs, INDEX, CRC and GAPs >
//--------------------------------------------------------
// SECTOR_COUNT REG.= 'total # of sectors to be formatted'
// (raw number; no multiplication)   = 16 decimal on RD51

// SECTOR NUMBER REG.= number of bytes - 3 (for GAP 1 + 3)
// = 40 decimal on DEC RD51 with WUTIL 3.2
//--------------------------------------------------------
void wd2010_device::format(uint8_t data)
{
	uint8_t newstatus = STATUS_RDY;

	m_out_intrq_cb(CLEAR_LINE);
	m_error = 0;
	m_status = STATUS_BSY | STATUS_CIP;

	m_status |= STATUS_DRQ;
	m_out_bdrq_cb(1);

	// < WAIT UNTIL BRDY ASSERTED >

	// Datasheet says [DRQ] must go LOW...
	// ...delayed here _until BRDY goes high_ (=> TIMER EVENT <=):

	// m_out_bdrq_cb(0);
	// m_status &= ~(STATUS_DRQ);

	auto_scan_id(data); // has drive number changed?

	// TODO: Seek to desired cylinder
	// Assume : SEEK COMPLETE.

	m_out_bcr_cb(0); // strobe BCR
	m_out_bcr_cb(1);

	m_out_bcs_cb(1); // activate BCS (!)

	if (!m_in_drdy_cb() || m_in_wf_cb())
	{
		m_error = ERROR_AC; // ABORTED_COMMAND
		complete_cmd(newstatus | STATUS_ERR);
		return;
	}

	// WAIT FOR INDEX

	m_out_wg_cb(1); // Have Index, activate WRITE GATE

	if (m_in_wf_cb()) // Check for WRITE FAULT (WF)
	{
		m_error = ERROR_AC; // ABORTED_COMMAND
		complete_cmd(newstatus | STATUS_ERR);
		return;
	}

	m_out_wg_cb(0);   // (transition from WG 1 -> 0). Actual write.

	//  ** DELAY INTRQ UNTIL WRITE IS COMPLETE :
	m_complete_write_timer->adjust(attotime::from_usec(1), newstatus | STATUS_DRQ); // 1 USECs
}


// *************************************
// INTERNAL
// *************************************
void wd2010_device::buffer_ready(bool state)
{
	is_buffer_ready = state;
}


TIMER_CALLBACK_MEMBER(wd2010_device::command_complete)
{
	m_cmd_timer->adjust(attotime::never);
	complete_immediate(param);
}

TIMER_CALLBACK_MEMBER(wd2010_device::complete_write)
{
	// when BUFFER_READY -> HIGH
	if (is_buffer_ready)
	{
		m_complete_write_timer->adjust(attotime::never);
		complete_write_sector(param);
	}
	else
	{
		m_complete_write_timer->reset();
		m_complete_write_timer->adjust(attotime::from_usec(1), param); // DELAY ANOTHER 1 USEC (!)
	}
}

TIMER_CALLBACK_MEMBER(wd2010_device::deassert_write)
{
	//  waiting for BUFFER_READY -> LOW
	if (!(is_buffer_ready))
	{
		m_deassert_write_timer->adjust(attotime::never);
		complete_immediate(param);
	}
	else
	{
		m_deassert_write_timer->reset();
		m_deassert_write_timer->adjust(attotime::from_usec(1), param); // DELAY ANOTHER 1 USEC (!)
	}
}

TIMER_CALLBACK_MEMBER(wd2010_device::deassert_read)
{
	// when BUFFER_READY -> HIGH
	if (is_buffer_ready)
	{
		m_deassert_read_timer->adjust(attotime::never);

		m_error &= ~ERROR_ID;
		param &= ~STATUS_ERR;

		m_out_bdrq_cb(0);
		complete_immediate(param);
	}
	else
	{
		m_deassert_read_timer->reset();
		m_deassert_read_timer->adjust(attotime::from_usec(1), param); // DELAY ANOTHER 1 USEC (!)
	}
}

TIMER_CALLBACK_MEMBER(wd2010_device::next_sector)
{
	uint8_t cmd = m_task_file[TASK_FILE_COMMAND];

	switch (cmd & 0xf0)
	{
	case 0x20:
		read_sector(cmd);
		break;
	case 0x30:
		write_sector(cmd);
		break;
	default:
		break;
	}
}

// Called by timer callbacks -
void wd2010_device::complete_immediate(uint8_t status)
{
	// re-evaluate external signals at end of command
	status &= ~(STATUS_RDY | STATUS_WF | STATUS_SC); // RDY  0x40  / WF 0x20 /  SC 0x10
	status |= (m_in_drdy_cb() ? 0x40 : 0) | (m_in_wf_cb() ? 0x20 : 0) | (m_in_sc_cb() ? 0x10 : 0);

	if (status & STATUS_DRQ) // if DRQ was set, reset
	{
		status &= ~(STATUS_DRQ);
		m_out_bdrq_cb(0);
	}

	uint8_t cmd = m_task_file[TASK_FILE_COMMAND] & 0xf4;
	if ((cmd == 0x24) || (cmd == 0x34))
	{
		if (--m_task_file[TASK_FILE_SECTOR_COUNT] > 1)
		{
			m_task_file[TASK_FILE_SECTOR_NUMBER]++;
			m_next_sector_timer->adjust(attotime::from_usec(100));
			return;
		}
	}

	// Set current status (M_STATUS)
	m_status = status & (255 - STATUS_DWC); // minus "unused" bit 2 (DWC)

	m_status &= ~(STATUS_BSY | STATUS_CIP); // de-assert BUSY + CIP

	// "IRQ AT END OF COMMAND"  when  BIT 2 set  (DWC 'data was corrected' - unused in this context!)
	if (!(status & STATUS_DWC)) // interrupt at END OF COMMAND ?
		m_out_intrq_cb(ASSERT_LINE); // Assert INTRQ (callback).

	m_out_bcs_cb(0); // de-assert BCS (needed)
	m_out_wg_cb(0);  // deactivate WG  (required by write / format)

	m_out_bcr_cb(0); // strobe BCR
	m_out_bcr_cb(1);
}

void wd2010_device::complete_cmd(uint8_t status)
{
	m_cmd_timer->adjust(attotime::from_msec(1), status);
}
