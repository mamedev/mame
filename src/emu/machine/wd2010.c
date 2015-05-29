// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

Western Digital WD2010 Winchester Disk Controller

Portions (2015) : Karl-Ludwig Deisenhofer
**********************************************************************

Implements WD2010 / WD1010 controller basics.

 Provides IRQ / (B)DRQ signals needed for early MFM cards.
 Honors DRIVE_READY and WRITE FAULT (DRDY / WF).

 Single sector read / write (format) confirmed to work with
 Rainbow-100 controller (WD1010, largely compatible to WD2010, see **)

 LIST OF UNIMPLEMENTED FEATURES :
        - MULTI SECTOR TRANSFERS (M = 1); MULTIPLE DRIVES
        - AUTO_SCAN_ID / SEEK + INDEX TIMERS / ID NOT FOUND
        - IMPLIED SEEKS / IMPLIED WRITES / RETRIES
        - EDGE or LEVEL TRIGGERED SEEK_COMPLETE (SC)
        - SET_PARAMETER / COMPUTE_CORRECTION (DWC flag!)

 Pseudo code (from datasheet) left in to illustrate
 the intended instruction flow. Some loops were omitted!

 USAGE:  tie WF (write fault) to ground if not needed:
 MCFG_WD2010_IN_WF_CB(GND)

 Other signals should be set to VCC if not serviced:
 MCFG_WD2010_IN_DRDY_CB(VCC)  // DRIVE READY = VCC
 MCFG_WD2010_IN_SC_CB(VCC)    // SEEK COMPLETE = VCC
 **********************************************************************/

// WD 2010 CONFIGURATION (2048 cylinder limit)
#define STEP_LIMIT 2048
#define CYLINDER_HIGH_MASK 0x07

// DEC RD51 chip; different STEP / CYLINDER LIMIT (**):

// WD 1010 CONFIGURATION (1024 cylinder limit)
// #define  STEP_LIMIT 1024
// #define CYLINDER_HIGH_MASK 0x03

// --------------------------------------------------------
#define MAX_MFM_SECTORS 17      // STANDARD MFM SECTORS/TRACK
// --------------------------------------------------------


#include "machine/wd2010.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 1

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

static const int SECTOR_SIZES[4] = { 256, 512, 1024, 128 };

#define SECTOR_SIZE \
	SECTOR_SIZES[(m_task_file[TASK_FILE_SDH_REGISTER] >> 5) & 0x03]

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

const device_type WD2010 = &device_creator<wd2010_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  wd2010_device - constructor
//-------------------------------------------------

wd2010_device::wd2010_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: device_t(mconfig, WD2010, "Western Digital WD2010", tag, owner, clock, "wd2010", __FILE__),
m_out_intrq_cb(*this),
m_out_bdrq_cb(*this),
m_out_bcr_cb(*this),
m_in_bcs_cb(*this),
m_in_brdy_cb(*this),
m_out_bcs_cb(*this),
m_out_dirin_cb(*this),
m_out_step_cb(*this),
m_out_rwc_cb(*this),
m_out_wg_cb(*this),
m_in_drdy_cb(*this),
m_in_index_cb(*this),
m_in_wf_cb(*this),
m_in_tk000_cb(*this),
m_in_sc_cb(*this),
m_status(0),
m_error(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void wd2010_device::device_start()
{
	// resolve callbacks
	m_out_intrq_cb.resolve_safe();
	m_out_bdrq_cb.resolve_safe();
	m_out_bcr_cb.resolve_safe();
	m_in_bcs_cb.resolve_safe(0);

	m_in_brdy_cb.resolve_safe(0);

	m_out_bcs_cb.resolve_safe();
	m_out_dirin_cb.resolve_safe();
	m_out_step_cb.resolve_safe();
	m_out_rwc_cb.resolve_safe();
	m_out_wg_cb.resolve_safe();
	m_in_drdy_cb.resolve_safe(0);
	m_in_index_cb.resolve_safe(0);
	m_in_wf_cb.resolve_safe(0);
	m_in_tk000_cb.resolve_safe(0);
	m_in_sc_cb.resolve_safe(0);

	/* allocate a timer for commands */
	cmd_timer = timer_alloc(0);
	complete_write_when_buffer_ready_high = timer_alloc(1);
	deassert_write_when_buffer_ready_low = timer_alloc(2);
	deassert_read_when_buffer_ready_high = timer_alloc(3);
}

// timers
#define COMMAND_TIMER 0
#define COMPLETE_WRITE_SECTOR 1
#define DE_ASSERT_WRITE 2
#define DE_ASSERT_READ 3


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void wd2010_device::device_reset()
{
	m_out_intrq_cb(CLEAR_LINE);

	buffer_ready(false);

	m_present_cylinder = 0; // start somewhere
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER(wd2010_device::read)
{
	UINT8 data = 0;

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
			logerror("(READ) %s WD2010 '%s' SDH: %u\n", machine().describe_context(), tag(), data);
			logerror("(READ) %s WD2010 '%s' Head: %u\n", machine().describe_context(), tag(), HEAD);
			logerror("(READ) %s WD2010 '%s' Drive: %u\n", machine().describe_context(), tag(), DRIVE);
			logerror("(READ) %s WD2010 '%s' Sector Size: %u\n", machine().describe_context(), tag(), SECTOR_SIZE);
		}

		break;
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER(wd2010_device::write)
{
	m_task_file[offset] = data;

	switch (offset)
	{
	case TASK_FILE_WRITE_PRECOMP_CYLINDER:
		if (LOG) logerror("%s WD2010 '%s' Write Precomp Cylinder: %u\n", machine().describe_context(), tag(), WRITE_PRECOMP_CYLINDER);
		break;

	case TASK_FILE_SECTOR_COUNT:
		if (LOG) logerror("%s WD2010 '%s' Sector Count: %u\n", machine().describe_context(), tag(), SECTOR_COUNT);
		break;

	case TASK_FILE_SECTOR_NUMBER:
		if (LOG) logerror("%s WD2010 '%s' Sector Number: %u\n", machine().describe_context(), tag(), SECTOR_NUMBER);
		break;

	case TASK_FILE_CYLINDER_LOW:
		if (LOG) logerror("%s WD2010 '%s' Cylinder (lower bits set): %u\n", machine().describe_context(), tag(), CYLINDER);
		break;

	case TASK_FILE_CYLINDER_HIGH:
		if (LOG) logerror("%s WD2010 '%s' Cylinder (MSB bits set): %u\n", machine().describe_context(), tag(), CYLINDER);
		break;

	case TASK_FILE_SDH_REGISTER:
		if (LOG)
		{
			logerror("(WRITE) %s WD2010 '%s' SDH: %u\n", machine().describe_context(), tag(), data);
			logerror("(WRITE) %s WD2010 '%s' Head: %u\n", machine().describe_context(), tag(), HEAD);
			logerror("(WRITE) %s WD2010 '%s' Drive: %u\n", machine().describe_context(), tag(), DRIVE);
			logerror("(WRITE) %s WD2010 '%s' Sector Size: %u\n", machine().describe_context(), tag(), SECTOR_SIZE);
		}
		break;

	case TASK_FILE_COMMAND:
		m_out_intrq_cb(CLEAR_LINE); // "either reading the status register or writing a new command clears INTRQ"
		m_status &= ~(STATUS_ERR | STATUS_BSY | STATUS_CIP);  // "Reset ERR bit in STATUS upon new cmd" (see datasheet)
		m_error = 0;

		if (data == COMMAND_COMPUTE_CORRECTION)
		{
			if (LOG) logerror("%s WD2010 '%s' COMPUTE CORRECTION\n", machine().describe_context(), tag());
			compute_correction(data);
		}
		else if ((data & COMMAND_SET_PARAMETER_MASK) == COMMAND_SET_PARAMETER)
		{
			if (LOG) logerror("%s WD2010 '%s' SET PARAMETER\n", machine().describe_context(), tag());
			set_parameter(data);
		}
		else
		{
			switch (data & COMMAND_MASK)
			{
			case COMMAND_RESTORE:
				if (LOG) logerror("%s WD2010 '%s' RESTORE\n", machine().describe_context(), tag());
				restore(data);
				break;

			case COMMAND_SEEK:
				if (LOG) logerror("%s WD2010 '%s' SEEK\n", machine().describe_context(), tag());
				seek(data);
				break;

			case COMMAND_READ_SECTOR:
				if (LOG) logerror("%s WD2010 '%s' READ SECTOR (I = %u) (M = %u)\n", machine().describe_context(), tag(), ((data & 8)>0), ((data & 4)>0));
				read_sector(data);
				break;

			case COMMAND_WRITE_SECTOR:
				if (LOG) logerror("%s WD2010 '%s' WRITE SECTOR (M = %u)\n", machine().describe_context(), tag(), ((data & 4) > 0));
				write_sector(data);
				break;

			case COMMAND_SCAN_ID:
				if (LOG) logerror("%s WD2010 '%s' SCAN ID\n", machine().describe_context(), tag());
				scan_id(data);
				break;

			case COMMAND_WRITE_FORMAT:
				if (LOG) logerror("%s WD2010 '%s' WRITE FORMAT\n", machine().describe_context(), tag());
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
void wd2010_device::compute_correction(UINT8 data)
{
	UINT8 newstatus = STATUS_RDY | STATUS_SC;
	complete_cmd(newstatus);
}


//-------------------------------------------------
//  set_parameter -
//-------------------------------------------------
void wd2010_device::set_parameter(UINT8 data)
{
	UINT8 newstatus = STATUS_RDY | STATUS_SC;
	complete_cmd(newstatus);
}


//-------------------------------------------------
//  restore -
//-------------------------------------------------
void wd2010_device::restore(UINT8 data)
{
	UINT8 newstatus = STATUS_RDY | STATUS_SC;

	m_out_intrq_cb(CLEAR_LINE); // reset INTRQ, errors, set BUSY, CIP
	m_error = 0;
	m_status = STATUS_BSY | STATUS_CIP;

	m_out_rwc_cb(0); // reset RWC, set direction = OUT

	// datasheet: DIRIN HIGH = in ;  LOW = out
	m_out_dirin_cb(0); // 0 = heads move away from the spindle, towards track O.

	// TODO: store step rate

	m_present_cylinder = 0; // (sse WD2010-05 datasheet)
	m_task_file[TASK_FILE_CYLINDER_HIGH] = 0;
	m_task_file[TASK_FILE_CYLINDER_LOW] = 0;

	int step_pulses = 0;
	while (step_pulses < STEP_LIMIT)
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

		//if (m_in_tk000_cb())
		if (step_pulses == STEP_LIMIT - 2) // Simulate TRACK 00 signal (normally from DRIVE)
		{
			m_out_bcr_cb(0); // pulse BCR
			m_out_bcr_cb(1);
			newstatus &= ~(STATUS_BSY | STATUS_CIP); // prepare new status; (INTRQ later) reset BSY, CIP
			complete_cmd(newstatus);
			return;
		}

		if (step_pulses == STEP_LIMIT - 1) // NOTE: STEP_LIMIT - differs - between WD2010 and WD1010
		{
			m_error = ERROR_TK; // ERROR: track 0 not reached within limit
			newstatus = newstatus | STATUS_ERR;

			m_out_bcr_cb(0); // pulse BCR
			m_out_bcr_cb(1);
			newstatus &= ~(STATUS_BSY | STATUS_CIP); // prepare new status; (INTRQ later) reset BSY, CIP
			complete_cmd(newstatus);
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

// FIXME : step rate, drive change (!)

// NOT IMPLEMENTED: IMPLIED SEEK ("wait until rising edge of SC signal")
void wd2010_device::seek(UINT8 data)
{
	UINT8 newstatus = STATUS_RDY | STATUS_SC;

	m_out_intrq_cb(CLEAR_LINE); // reset INTRQ, errors, set BUSY, CIP
	m_error = 0;
	m_status = STATUS_BSY | STATUS_CIP;

	// TODO : store STEP RATE.

	auto_scan_id(data); // has drive number changed?

	int direction = 0; // 0 = towards 0
	int step_pulses = 0;

	// Calculate number of steps by comparing the cylinder registers
	//           HI/LO with the internally stored position.
	UINT32 cylinder_registers = CYLINDER;
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
	logerror("SEEK - direction = %u, step_pulses = %u\n", direction, step_pulses);
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

	logerror("SEEK (END) - m_present_cylinder = %u\n", m_present_cylinder);

	cmd_timer->adjust(attotime::from_msec(35), newstatus);  // 35 msecs makes "SEEK_TIMING" test happy.
}

//-------------------------------------------------
//  read_sector -
//-------------------------------------------------
// FIXME: multiple sector transfers, ID / CYL / HEAD / SIZE match
//        + ERROR HANDLING (...)
void wd2010_device::read_sector(UINT8 data)
{
	UINT8 newstatus = STATUS_RDY | STATUS_SC;
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
				logerror("WD2010 (READ): MULTIPLE SECTOR READ (M = 1).\n");

			// Assume: NO "M" (MULTIPLE SECTOR TRANSFERS)

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

	deassert_read_when_buffer_ready_high->adjust(attotime::from_usec(1), newstatus); // complete command ON  *RISING EDGE * OF BUFFER_READY
}


//-------------------------------------------------
//  write_sector (stage I)
//-------------------------------------------------
// FIXME: SEEK, SEEK_COMPLETE, Drive # change (!)
// as well as CYL.register + internal CYL.register comparisons
void wd2010_device::write_sector(UINT8 data)
{
	m_error = 0; // De-assert ERROR + DRQ
	m_status &= ~(STATUS_DRQ);

	m_status = STATUS_BSY | STATUS_CIP; // Assert BUSY + CIP

	m_status |= STATUS_DRQ; // Assert BDRQ + DRQ (= status bit 3)
	m_out_bdrq_cb(1);

	//  WAIT UNTIL BRDY ASSERTED (-> timer):
	complete_write_when_buffer_ready_high->adjust(attotime::from_usec(1), data); // 1 usec
}


//-------------------------------------------------
//  write_sector (stage II)
//-------------------------------------------------
void wd2010_device::complete_write_sector(UINT8 data)
{
	UINT8 newstatus = STATUS_RDY | STATUS_SC;

	m_out_bdrq_cb(0); // DE-Assert BDRQ (...and DRQ !)
	m_status &= ~(STATUS_DRQ);

	// (When drive changed) : SCAN_ID / GET CYL#
	auto_scan_id(data); // has drive number changed? (*** UNIMPLEMENTED ***)

	// Assume YES : CYL.register + internal CYL.register SAME?  (if NO => SEEK!)
	// Assume : SEEK_COMPLETE = YES

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
	deassert_write_when_buffer_ready_low->adjust(attotime::from_usec(1), newstatus);
}

// ******************************************************
// AUTO SCAN-ID (whenever DRIVE # changes):

	// * does nothing right now *
// ******************************************************
void wd2010_device::auto_scan_id(UINT8 data)
{
	static int last_drive;

	if (DRIVE != last_drive)
	{
		printf("\n(WD2010) : UNSUPPORTED DRIVE CHANGE !\n");
		logerror("\n(WD2010) : UNSUPPORTED DRIVE CHANGE !\n");

		//update_sdh(new_sector_size, new_head, new_cylinder, new_sectornr);
	}
	last_drive = DRIVE;

	return; // AUTO-SCAN CURRENTLY DISABLED (see NOTES)
}
// ******************************************************

// What to do here (just update present_cylinder with CYLINDER)...?
void wd2010_device::update_sdh(UINT8 new_sector_size, UINT8 new_head, UINT16 new_cylinder, UINT8 new_sectornr)
{
	// "Update SDH"
	/*
	// Update SECTOR_SIZE, HEAD in SDH with the ID found -
	m_task_file[TASK_FILE_SDH_REGISTER] = ???

	// ...update CYLINDER registers with cylinder found -
	m_task_file[TASK_FILE_CYLINDER_LOW] = (new_cylinder >> 4) & 0x0f;
	m_task_file[TASK_FILE_CYLINDER_HIGH] = (new_cylinder - ((new_cylinder >> 4) << 4)) & 0x0f;

	// ...update SECTOR_NUMBER with sector nr. found -
	m_task_file[TASK_FILE_SECTOR_NUMBER] = new_sectornr;
	*/

	m_present_cylinder = CYLINDER;
	logerror("UPDATE_SDH - m_present_cylinder = %u\n", m_present_cylinder);
}

//-------------------------------------------------
//  scan_id -
//-------------------------------------------------

//  Reads the cylinder number from the track on which the heads are PRESENTLY located,
//  and writes this into the Present Cylinder Position Register.

//  FIXME: NO ID HANDLING (ID FOUND / NOT FOUND), NO BAD BLOCK; NO CRC
void wd2010_device::scan_id(UINT8 data)
{
	UINT8 newstatus = STATUS_RDY;

	m_out_intrq_cb(CLEAR_LINE);
	m_error = 0;
	m_status = STATUS_BSY | STATUS_CIP;

	// Assume DRIVE READY.
	// < TODO: Search for ANY ID FIELD. >

	// Assume ID FOUND :
	update_sdh( 32, 0, 0, 1 ); // (NEW:) SECTOR_SIZE,  HEAD,  CYLINDER,  SECTOR_NR

	// NO BAD BLOCK.
	// NO CRC ERROR.

	complete_cmd(newstatus);
}

//--------------------------------------------------------
// FORMAT ENTIRE TRACK using the task file + sector buffer

// On real hardware, data fields are filled with FF.
// Sector buffer is used for track layout (see datasheet).

// Routine simulates one single write on each track
//  - just enough to keep formatter programs happy -

// < UNIMPLEMENTED: (IMPLIED) SEEKs, INDEX, CRC and GAPs >
//--------------------------------------------------------
// SECTOR_COUNT REG.= 'total # of sectors to be formatted'
// (raw number; no multiplication)   = 16 decimal on RD51

// SECTOR NUMBER REG.= number of bytes - 3 (for GAP 1 + 3)
// = 40 decimal on DEC RD51 with WUTIL 3.2
//--------------------------------------------------------
void wd2010_device::format(UINT8 data)
{
	UINT8 newstatus = STATUS_RDY;

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

	// Check for WRITE FAULT (WF)
	if (m_in_wf_cb())
	{
		m_error = ERROR_AC; // ABORTED_COMMAND
		complete_cmd(newstatus | STATUS_ERR);
		return;
	}

	//  UINT8 format_sector_count = m_task_file[TASK_FILE_SECTOR_COUNT];
	//  do
	//  {
	//      < WRITE GAP 1 or GAP 3 >

	//      < Wait for SEEK_COMPLETE=1 (extend GAP if SEEK_COMPLETE = 0) >
	//      < Assume SEEK COMPLETE >

	//      format_sector_count--;
	//      if (format_sector_count != 0)
			{
				// The Rainbow 100 driver does ignore multiple sector
				// transfers so WRITE FORMAT does not actually write -

				m_out_wg_cb(0);   // (transition from WG 1 -> 0)

				// NOTE: decrementing TASK_FILE_SECTOR_COUNT does * NOT WORK *
			}
	//      else
	//      {       //  < Write 4Es until INDEX  (*** UNIMPLEMENTED ****) >
	//      }
	//  } while (format_sector_count > 0);

	//  ** DELAY INTRQ UNTIL WRITE IS COMPLETE :
	complete_write_when_buffer_ready_high->adjust(attotime::from_usec(1), newstatus | STATUS_DRQ); // 1 USECs
}


// *************************************
// INTERNAL
// *************************************
void wd2010_device::buffer_ready(bool state)
{
	is_buffer_ready = state;
}


void wd2010_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
{
	switch (tid)
	{
	case COMMAND_TIMER:
		cmd_timer->adjust(attotime::never);
		complete_immediate(param);
		break;

	case COMPLETE_WRITE_SECTOR:  // when BUFFER_READY -> HIGH
		if (is_buffer_ready)
		{
			complete_write_when_buffer_ready_high->adjust(attotime::never);
			complete_write_sector(param);
		}
		else
		{
			complete_write_when_buffer_ready_high->reset();
			complete_write_when_buffer_ready_high->adjust(attotime::from_usec(1), param); // DELAY ANOTHER 1 USEC (!)
		}
		break;

	case DE_ASSERT_WRITE: //  waiting for BUFFER_READY -> LOW
		if (!(is_buffer_ready))
		{
			deassert_write_when_buffer_ready_low->adjust(attotime::never);
			complete_immediate(param);
		}
		else
		{
			deassert_write_when_buffer_ready_low->reset();
			deassert_write_when_buffer_ready_low->adjust(attotime::from_usec(1), param); // DELAY ANOTHER 1 USEC (!)
		}
		break;

	case DE_ASSERT_READ: // when BUFFER_READY -> HIGH
		if (is_buffer_ready)
		{
			deassert_read_when_buffer_ready_high->adjust(attotime::never);

			m_error &= ~ERROR_ID;
			param &= ~STATUS_ERR;

			m_out_bdrq_cb(0);
			complete_immediate(param);
		}
		else
		{
			deassert_read_when_buffer_ready_high->reset();
			deassert_read_when_buffer_ready_high->adjust(attotime::from_usec(1), param); // DELAY ANOTHER 1 USEC (!)
		}
		break;

	default:
		break;
	}
}

// Called by 'device_timer' -
void wd2010_device::complete_immediate(UINT8 status)
{
	// re-evaluate external signals at end of command
	status &= ~(STATUS_RDY | STATUS_WF | STATUS_SC); // RDY  0x40  / WF 0x20 /  SC 0x10
	status |= (m_in_drdy_cb() ? 0x40 : 0) | (m_in_wf_cb() ? 0x20 : 0) | (m_in_sc_cb() ? 0x10 : 0);

	if (status & STATUS_DRQ) // if DRQ was set, reset
	{
		status &= ~(STATUS_DRQ);
		m_out_bdrq_cb(0);
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

void wd2010_device::complete_cmd(UINT8 status)
{
	cmd_timer->adjust(attotime::from_msec(1), status);
}
