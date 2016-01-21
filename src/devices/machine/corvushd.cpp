// license:BSD-3-Clause
// copyright-holders:Brett Wyer, Raphael Nabet
//
//  corvus_hd
//
//  Implementation of a Corvus Hard Drive / Host Bus Adapter pair.  The drive
//  being emulated is a Rev B drive, functionally speaking, rather than an Omnidrive.
//
//  The Corvus Flat Cable HBA is a very simplistic device due to the fact that most
//  of the smarts are in the Hard Drive itself.  What's in the hard drive includes a
//  Z80 processor, 4K of EPROM and 5KB of RAM.  Ultimately, a true emulation would include
//  the on-boad controller; however, that is outside the current scope of this code.  Maybe
//  if I could get a Rev. B/H drive, it could be reverse-engineered to do this.
//
//  The Flat Cable controller has two registers:
//
//  Data -              Single byte bidirectional data transfer
//  Status Register -   Bit 7 - Controller Ready -- off = ready, on = not ready
//                      Bit 6 - Bus Direction -- off = host-to-controller, on = controller-to-host
//
//  Layout of a Corvus Hard Disk is as follows:
//
//  Blk Len Description
//  --- --- -----------
//  0   1   Boot Block
//  1   1   Disk Parameter Block
//  2   1   Diagnostic Block (prep code)
//  3   1   Constellation Parameter Block
//  4   2   Dispatcher Code
//  6   2   Pipes and Semaphores code (Semaphore table contained in block 7, bytes 1-256)
//  8   10  Mirror Controller Code
//  18  2   LSI-11 Controller Code
//  20  2   Pipes Controller Code
//  22  3   Reserved for Future Use
//  25  8   Boot Blocks 0-7.  Apple II uses 0-3, Concept uses 4-7
//  33  4   Active User Table
//  37  3   Reserved
//
//  All of the above blocks are initialized by the DDIAG program.  This can be found on the
//  Concept FSYSGEN floppy.
//      - Boot blocks and code blocks are initialized using the "Update Firmware on Disk" function.
//      - Disk Parameter Block is initialized using the "Display/Modify Drive Parameters" function
//
//  An on-disk structure is written with the SYSGEN utility on the same disk.  Password is "HAI"
//
//
//  Corvus Hard Disk performance characteristics (from a 6MB Rev B-E drive)
//
//      Average Latency: 6.25ms
//      Average Access Time: 125ms (and you thought YOUR drive was slow...)
//      Maximum Access Time: 240ms
//      Maximum Access Time (single track): 3ms
//      Data Transfer Rate: 960Kb/sec
//      Rotational Speed: 4800RPM
//
//  Brett Wyer
//
//
//  TODO:
//      Implement READY line glitch after last byte of command (Disk System Tech Ref pp. 3)
//      Implement Read-after-Write (always happens on Rev B/H drives per Mass Storage GTI pp. 12)
//      Implement Drive Illegal Addresses (seek past last sector)
//      Implement Switches on front of drive (LSI-11, MUX, Format, Reset)
//      Implement an inter-sector delay during the FORMAT command (format happens too quickly now)
//

#include "emu.h"
#include "imagedev/harddriv.h"
#include "machine/corvushd.h"
#include <ctype.h>


const device_type CORVUS_HDC = &device_creator<corvus_hdc_t>;

corvus_hdc_t::corvus_hdc_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, CORVUS_HDC, "Corvus Flat Cable HDC", tag, owner, clock, "corvus_hdc", __FILE__),
	m_status(0),
	m_prep_mode(false),
	m_prep_drv(0),
	m_sectors_per_track(0),
	m_tracks_per_cylinder(0),
	m_cylinders_per_drive(0),
	m_offset(0),
	m_awaiting_modifier(false),
	m_recv_bytes(0),
	m_xmit_bytes(0),
	m_last_cylinder(0),
	m_delay(0),
	m_invalid_command_flag(false)
{
}

#define VERBOSE 0
#define VERBOSE_RESPONSES 0
#define ROM_VERSION 1           // Controller ROM version
#define MAX_COMMAND_SIZE 4096   // The maximum size of a command packet (the controller only has 5K of RAM...)
#define SPARE_TRACKS 7          // This is a Rev B drive, so 7 it is
#define CALLBACK_CTH_MODE 1     // Set to Controller-to-Host mode when callback fires
#define CALLBACK_HTC_MODE 2     // Set to Host-to-Controller mode when callback fires
#define CALLBACK_SAME_MODE 3    // Leave mode the same when callback fires
#define CALLBACK_TIMEOUT 4      // Four seconds have elapsed.  We're timing out
#define TRACK_SEEK_TIME 1667    // Track-to-track seek time in microseconds (Maximum Access Time / Total Cylinders)
#define INTERBYTE_DELAY 5       // Inter-byte delay in microseconds communicating between controller and host
#define INTERSECTOR_DELAY 25000 // 25ms delay between sectors (4800 RPM = 80 Rev/Second.  Maximum 2 sectors transferred / Rev)

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)
#define LOG_BUFFER(p,s) do { if (VERBOSE) dump_buffer(p,s); } while (0)



//
// Dump_Buffer
//
// Dump a buffer to the error log in a nice format.
//
// Pass:
//      buffer: Data to be dumped
//      length: Number of bytes to be dumped
//
// Returns:
//      nada
//
void corvus_hdc_t::dump_buffer(UINT8 *buffer, UINT16 length) {
	UINT16  offset;
	char    ascii_dump[16];

	logerror("dump_buffer: Dump of %d bytes:\n", length);
	logerror("Base  00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f ASCII\n");
	logerror("----  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- ----------------");

	for(offset=0; offset < length; offset++) {
		if(offset % 16 == 0) {                  // WHY IS 0 % 16 == 0???
			if(offset > 0 && offset % 16 == 0)
				logerror("%16.16s", ascii_dump);
			logerror("\n%4.4x: %2.2x ", offset, *(buffer + offset));
		} else {
			logerror("%2.2x ", *(buffer + offset));
		}
		ascii_dump[offset % 16] = isprint(*(buffer + offset)) ? *(buffer + offset) : '.';
	}
	if(offset % 16)
		logerror("%.*s", (16 - (offset % 16)) * 3, "                                                     ");
	logerror("%.*s\n", (offset % 16) ? (offset % 16) : 16, ascii_dump);
}



//
// Parse_HDC_Command
//
// Process the first byte received from the host.  Do some initial evaluation and
// return either true or false as to whether the command was invalid or not.
//
// Note that recv_bytes and xmit_bytes in the corvus_hdc structure are updated as
// a side-effect of this command, as is awaiting_modifier.
//
// Pass:
//      data:   Initial byte received from the host in Host to Controller mode
//
// Returns:
//      Whether the command was invalid or not (true = invalid command)
//
bool corvus_hdc_t::parse_hdc_command(UINT8 data) {
	m_awaiting_modifier = false;               // This is the case by definition

	LOG(("parse_hdc_command: Called with data: 0x%2.2x, Prep mode is: %d\n", data, m_prep_mode));

	if(!m_prep_mode) {
		switch(data) {
			//
			// Single-byte commands - Non-Prep mode
			//
			case READ_SECTOR_256:
			case WRITE_SECTOR_256:
			case READ_CHUNK_128:
			case READ_CHUNK_256:
			case READ_CHUNK_512:
			case WRITE_CHUNK_128:
			case WRITE_CHUNK_256:
			case WRITE_CHUNK_512:
			case READTEMPBLOCK:
			case WRITETEMPBLOCK:
			case BOOT:
			case READ_BOOT_BLOCK:
			case GET_DRIVE_PARAMETERS:
		//  case PARK_HEADS_REVH:
			case PARK_HEADS_OMNI:
			case ECHO:
			case PREP_MODE_SELECT:
				m_recv_bytes = corvus_cmd[data][0].recv_bytes;
				m_xmit_bytes = corvus_cmd[data][0].xmit_bytes;
				LOG(("parse_hdc_command: Single byte command recognized: 0x%2.2x, to recv: %d, to xmit: %d\n", data,
					m_recv_bytes, m_xmit_bytes));
				break;
			//
			// Double-byte commands
			//
			case SEMAPHORE_LOCK_CODE:
		//  case SEMAPHORE_UNLOCK_CODE:
			case SEMAPHORE_INIT_CODE:
		//  case PIPE_READ_CODE:
		//  case PIPE_WRITE_CODE:
		//  case PIPE_CLOSE_CODE:
		//  case PIPE_STATUS_CODE:
		//  case SEMAPHORE_STATUS_CODE:
			case PIPE_OPEN_WRITE_CODE:
		//  case PIPE_AREA_INIT_CODE:
		//  case PIPE_OPEN_READ_CODE:
			case ADDACTIVE_CODE:
		//  case DELACTIVEUSR_REVBH_CODE:
		//  case DELACTIVEUSR_OMNI_CODE:
		//  case DELACTIVENUM_OMNI_CODE:
		//  case FINDACTIVE_CODE:
				m_awaiting_modifier = true;
				LOG(("parse_hdc_command: Double byte command recognized: 0x%2.2x\n", data));
				break;

			default:                            // This is an INVALID command
				m_recv_bytes = 1;
				m_xmit_bytes = 1;
				LOG(("parse_hdc_command: Invalid command detected: 0x%2.2x\n", data));
				return true;
		}
	} else {
		switch(data) {
			//
			// Prep Commands
			//
			case PREP_MODE_SELECT:
			case PREP_RESET_DRIVE:
			case PREP_FORMAT_DRIVE:
			case PREP_FILL_DRIVE_OMNI:
			case PREP_VERIFY:
			case PREP_READ_FIRMWARE:
			case PREP_WRITE_FIRMWARE:
				m_recv_bytes = corvus_prep_cmd[data].recv_bytes;
				m_xmit_bytes = corvus_prep_cmd[data].xmit_bytes;
				LOG(("parse_hdc_command: Prep command recognized: 0x%2.2x, to recv: %d, to xmit: %d\n", data,
					m_recv_bytes, m_xmit_bytes));
				break;

			default:                            // This is an INVALID prep command
				m_recv_bytes = 1;
				m_xmit_bytes = 1;
				LOG(("parse_hdc_command: Invalid Prep command detected: 0x%2.2x\n", data));
				return true;
		}
	}   // if(!prep_mode)

	return false;
}



//
// Corvus_Write_Sector
//
// Write a variably-sized chunk of data to the CHD file
//
// Pass:
//      drv:    Corvus drive id (1..15)
//      sector: Physical sector number to write to
//      buffer: Buffer to write
//      len:    Length of the buffer (amount of data to write)
//
// Returns:
//      status: Command status
//
UINT8 corvus_hdc_t::corvus_write_sector(UINT8 drv, UINT32 sector, UINT8 *buffer, int len) {
	hard_disk_file
			*disk;              // Structures for interface to CHD routines
	UINT8   tbuffer[512];       // Buffer to hold an entire sector
	UINT16  cylinder;           // Cylinder this sector resides on

	LOG(("corvus_write_sector: Write Drive: %d, physical sector: 0x%5.5x\n", drv, sector));

	disk = corvus_hdc_file(drv);
	if(!disk) {
		logerror("corvus_write_sector: Failure returned by corvus_hdc_file(%d)\n", drv);
		return STAT_FATAL_ERR | STAT_DRIVE_NOT_ONLINE;
	}

	//
	// Calculate what cylinder the sector resides on for timing purposes
	//
	cylinder = (double) sector / (double) m_sectors_per_track / (double) m_tracks_per_cylinder;
	m_delay = abs(m_last_cylinder - cylinder) * TRACK_SEEK_TIME + INTERSECTOR_DELAY;

	//
	// Corvus supports write sizes of 128, 256 and 512 bytes.  In the case of a write smaller than
	// the sector size of 512 bytes, the sector is read, the provided data is overlayed and then the
	// sector is written back out.  See pp. 5 of the Mass Storage Systems GTI for the details of this
	// wonderful functionality.
	//
	if(len == 512) {
		hard_disk_write(disk, sector, buffer);
	} else {
		hard_disk_read(disk, sector, tbuffer);      // Read the existing data into our temporary buffer
		memcpy(tbuffer, buffer, len);                   // Overlay the data with the buffer passed
		m_delay += INTERSECTOR_DELAY;                  // Add another delay because of the Read / Write
		hard_disk_write(disk, sector, tbuffer);     // Re-write the data
	}

	m_last_cylinder = cylinder;

	LOG(("corvus_write_sector: Full sector dump on a write of %d bytes follows:\n", len));
	LOG_BUFFER(len == 512 ? buffer : tbuffer, 512);

	return STAT_SUCCESS;
}



//
// Corvus_Write_Logical_Sector
//
// Write a variably-sized chunk of data to the user area of the virtual Corvus drive
//
// Pass:
//      dadr:   Corvus-encoded Disk Address -- Logical Sector
//      buffer: Buffer holding the data to be written to the disk
//      len:    Length of the buffer
//
// Returns:
//      status: Corvus status
//
UINT8 corvus_hdc_t::corvus_write_logical_sector(dadr_t *dadr, UINT8 *buffer, int len) {
	UINT8   status;             // Status returned from Physical Sector read
	UINT8   drv;                // Corvus drive id (1..15)
	UINT32  sector;             // Sector number on drive

	//
	// Unencode the first byte of the DADR
	//
	// High-order nibble of first byte is the most-significant nibble of the sector address
	// Low-order nibble of first byte is the drive id
	//
	// For example: 0x23 would decode to Drive ID #3, high-order nibble: 0x02.
	//
	drv = (dadr->address_msn_and_drive & 0x0f);
	sector = (dadr->address_msn_and_drive & 0xf0 << 12) | (dadr->address_mid << 8) | dadr->address_lsb;

	LOG(("corvus_write_logical_sector: Writing based on DADR: 0x%6.6x, logical sector: 0x%5.5x, drive: %d\n",
		dadr->address_msn_and_drive << 16 | dadr->address_lsb << 8 | dadr->address_mid, sector, drv));

	// Set m_tracks_per_cylinder and m_sectors_per_track
	corvus_hdc_file(drv);

	//
	// Shift the logical sector address forward by the number of firmware cylinders (2) + the number of spare tracks (7)
	//
	sector += (m_tracks_per_cylinder * m_sectors_per_track * 2) + (SPARE_TRACKS * m_sectors_per_track);

	status = corvus_write_sector(drv, sector, buffer, len);

	if(status != STAT_SUCCESS)
		m_xmit_bytes = 1;

	return status;
}


//
// Corvus_Read_Sector
//
// Read a variably-sized chunk of data from the CHD file
//
// Pass:
//      drv:    Corvus drive id (1..15)
//      sector: Physical sector number to read from
//      buffer: Buffer to hold the data read from the disk
//      len:    Length of the buffer
//
// Returns:
//      status: Corvus status
//
UINT8 corvus_hdc_t::corvus_read_sector(UINT8 drv, UINT32 sector, UINT8 *buffer, int len) {
	hard_disk_file
			*disk;              // Structures for interface to CHD routines
	UINT8   tbuffer[512];       // Buffer to store full sector results in
	UINT16  cylinder;

	LOG(("corvus_read_sector: Read Drive: %d, physical sector: 0x%5.5x\n", drv, sector));

	disk = corvus_hdc_file(drv);
	if(!disk) {
		logerror("corvus_read_sector: Failure returned by corvus_hdc_file(%d)\n", drv);
		return STAT_FATAL_ERR | STAT_DRIVE_NOT_ONLINE;
	}

	//
	// Calculate what cylinder the sector resides on for timing purposes
	//
	cylinder = (double) sector / (double) m_sectors_per_track / (double) m_tracks_per_cylinder;
	m_delay = abs(m_last_cylinder - cylinder) * TRACK_SEEK_TIME + INTERSECTOR_DELAY;

	hard_disk_read(disk, sector, tbuffer);

	memcpy(buffer, tbuffer, len);

	m_last_cylinder = cylinder;

	LOG(("corvus_read_sector: Data read follows:\n"));
	LOG_BUFFER(tbuffer, len);

	return STAT_SUCCESS;
}



//
// Corvus_Read_Logical_Sector
//
// Read a variably-sized chunk of data from the user area of the virtual Corvus drive
//
// Pass:
//      dadr:   Corvus-encoded Disk Address -- Logical Sector
//      buffer: Buffer to hold the data read from the disk
//      len:    Length of the buffer
//
// Returns:
//      status: Corvus status
//
UINT8 corvus_hdc_t::corvus_read_logical_sector(dadr_t *dadr, UINT8 *buffer, int len) {
	UINT8   status;                             // Status returned from Physical Sector read
	UINT8   drv;                                // Corvus drive id (1..15)
	UINT32  sector;                             // Sector number on drive

	//
	// Unencode the first byte of the DADR
	//
	// High-order nibble of first byte is the most-significant nibble of the sector address
	// Low-order nibble of first byte is the drive id
	//
	// For example: 0x23 would decode to Drive ID #3, high-order nibble: 0x02.
	//
	drv = (dadr->address_msn_and_drive & 0x0f);
	sector = (dadr->address_msn_and_drive & 0xf0 << 12) | (dadr->address_mid << 8) | dadr->address_lsb;

	LOG(("corvus_read_logical_sector: Reading based on DADR: 0x%6.6x, logical sector: 0x%5.5x, drive: %d\n",
		dadr->address_msn_and_drive << 16 | dadr->address_lsb << 8 | dadr->address_mid, sector, drv));

	// Set up m_tracks_per_cylinder and m_sectors_per_track
	corvus_hdc_file(drv);

	//
	// Shift the logical sector address forward by the number of firmware cylinders (2) + the number of spare tracks (7)
	//
	sector += (m_tracks_per_cylinder * m_sectors_per_track * 2) + (SPARE_TRACKS * m_sectors_per_track);

	status = corvus_read_sector(drv, sector, buffer, len);

	if(status != STAT_SUCCESS)
		m_xmit_bytes = 1;

	return status;
}



//
// Corvus_Lock_Semaphore
//
// Lock a semaphore in the semaphore table
//
// Pass:
//      name:   Name of the semaphore to lock
//
// Returns:
//      status: Disk status
//
// Side-effects:
//      Fills in the semaphore result code
//
UINT8 corvus_hdc_t::corvus_lock_semaphore(UINT8 *name) {
	semaphore_table_block_t
			semaphore_table;
	UINT8   offset = 0;
	bool    found = false;
	UINT8   blank_offset = 32;  // Initialize to invalid offset
	UINT8   status;             // Status returned from Physical Sector read

	//
	// Read the semaphore table from the drive
	//
	status = corvus_read_sector(1, 7, semaphore_table.semaphore_block.semaphore_table, 256);
	if(status != STAT_SUCCESS) {
		logerror("corvus_lock_semaphore: Error reading semaphore table, status: 0x%2.2x\n", status);
		m_buffer.semaphore_locking_response.result = SEM_DISK_ERROR;
		return status;
	}

	//
	// Search the semaphore table to see if the semaphore already exists--if so it's locked
	// Also look for the first blank entry to stick the new one into
	//
	do {
		if(blank_offset == 32 && strncmp((char *) &semaphore_table.semaphore_block.semaphore_entry[offset], "        ", 8) == 0)
			blank_offset = offset;
		if(strncmp((char *) &semaphore_table.semaphore_block.semaphore_entry[offset], (char *) name, 8) == 0) {
			found = true;
			break;
		}
	} while( ++offset < 32 );

	//
	// Deal with the found status
	//
	// - Stick it into the table if we didn't find it and there's room
	// - Respond with a "set" status if we did find it
	//
	// Once that's done, write the updated table to the disk
	//
	if(!found) {
		if(blank_offset == 32) {
			m_buffer.semaphore_locking_response.result = SEM_TABLE_FULL;                   // No space for the semaphore!
		} else {
			m_buffer.semaphore_locking_response.result = SEM_PRIOR_STATE_NOT_SET;          // It wasn't there already
			memcpy(&semaphore_table.semaphore_block.semaphore_entry[blank_offset], name, 8);// Stick it into the table
			status = corvus_write_sector(1, 7, semaphore_table.semaphore_block.semaphore_table, 256);
			if(status != STAT_SUCCESS) {
				logerror("corvus_lock_semaphore: Error updating semaphore table, status: 0x%2.2x\n", status);
				m_buffer.semaphore_locking_response.result = SEM_DISK_ERROR;
				return status;
			}
		}
	} else {
		m_buffer.semaphore_locking_response.result = SEM_PRIOR_STATE_SET;                  // It's already locked -- sorry
	}

	return STAT_SUCCESS;
}



//
// Corvus_Unlock_Semaphore
//
// Unock a semaphore in the semaphore table
//
// Pass:
//      name:   Name of the semaphore to unlock
//
// Returns:
//      status: Disk status
//
// Side-effects:
//      Fills in the semaphore result code
//
UINT8 corvus_hdc_t::corvus_unlock_semaphore(UINT8 *name) {
	semaphore_table_block_t
			semaphore_table;
	UINT8   offset = 0;
	bool    found = false;
	UINT8   status;             // Status returned from Physical Sector read

	//
	// Read the semaphore table from the drive
	//
	status = corvus_read_sector(1, 7, semaphore_table.semaphore_block.semaphore_table, 256);
	if(status != STAT_SUCCESS) {
		logerror("corvus_unlock_semaphore: Error reading semaphore table, status: 0x%2.2x\n", status);
		m_buffer.semaphore_locking_response.result = SEM_DISK_ERROR;
		return status;
	}

	//
	// Search the semaphore table to see if the semaphore already exists--if so it's locked
	//
	do {
		if(strncmp((char *) &semaphore_table.semaphore_block.semaphore_entry[offset], (char *) name, 8) == 0) {
			found = true;
			break;
		}
	} while( ++offset < 32 );

	//
	// Deal with the found status
	//
	// - If we didn't find it, just respond that it wasn't there
	// - If we did find it, respond with a "set" status and clear it
	//
	// Once that's done, write the updated table to the disk
	//
	if(!found) {
		m_buffer.semaphore_locking_response.result = SEM_PRIOR_STATE_NOT_SET;              // It wasn't there already
	} else {
		m_buffer.semaphore_locking_response.result = SEM_PRIOR_STATE_SET;                  // It was there
		memcpy(&semaphore_table.semaphore_block.semaphore_entry[offset], "        ", 8);    // Clear it
		status = corvus_write_sector(1, 7, semaphore_table.semaphore_block.semaphore_table, 256);
		if(status != STAT_SUCCESS) {
			logerror("corvus_unlock_semaphore: Error updating semaphore table, status: 0x%2.2x\n", status);
			m_buffer.semaphore_locking_response.result = SEM_DISK_ERROR;
			return status;
		}
	}

	return STAT_SUCCESS;
}



//
// Corvus_Init_Semaphore_Table
//
// Zap all of the semaphores from the table (set them to blanks)
//
// Pass:
//      Nothing
//
// Returns:
//      Disk status
//
//
UINT8 corvus_hdc_t::corvus_init_semaphore_table() {
	semaphore_table_block_t
			semaphore_table;
	UINT8   status;

	memset(semaphore_table.semaphore_block.semaphore_table, 0x20, 256);

	status = corvus_write_sector(1, 7, semaphore_table.semaphore_block.semaphore_table, 256);
	if(status != STAT_SUCCESS) {
		logerror("corvus_init_semaphore_table: Error updating semaphore table, status: 0x%2.2x\n", status);
		return status;
	}

	return STAT_SUCCESS;
}



//
// Corvus_Get_Drive_Parameters
//
// Fills in the Drive Parameter packet based on the opened CHD file
//
// Pass:
//      drv:    Corvus drive id (1..15)
//
// Returns:
//      Status of command
//
UINT8 corvus_hdc_t::corvus_get_drive_parameters(UINT8 drv) {
	UINT16  capacity;                           // Number of usable 512-byte blocks
	UINT16  raw_capacity;                       // Number of actual 512-byte blocks
	union {
		UINT8
			buffer[512];
		disk_parameter_block_t
			dpb;
	} raw_disk_parameter_block;                 // Buffer for the Disk Parameter Block
	union {
		UINT8
			buffer[512];
		constellation_parameter_block_t
			cpb;
	} raw_constellation_parameter_block;        // Buffer for the Constellation Parameter Block
	UINT8   status;                             // Status to return

	//
	// Make sure a valid drive is being accessed
	//
	if ( ! corvus_hdc_file( drv ) )
	{
		logerror("corvus_get_drive_parameters: Attempt to retrieve parameters from non-existant drive: %d\n", drv);
		m_xmit_bytes = 1;
		return STAT_FATAL_ERR | STAT_DRIVE_NOT_ONLINE;
	}

	//
	// Read the Disk Parameter Block (Sector 1) from the drive
	//
	status = corvus_read_sector(drv, 1, raw_disk_parameter_block.buffer, 512);
	if(status != STAT_SUCCESS) {
		logerror("corvus_get_drive_parameters: Error status returned reading Disk Parameter Block -- status: 0x%2.2x\n", status);
		m_xmit_bytes = 1;
		return status;
	}

	//
	// Read the Constellation Parameter Block (Sector 3) from the drive
	//
	status = corvus_read_sector(drv, 3, raw_constellation_parameter_block.buffer, 512);
	if(status != STAT_SUCCESS) {
		logerror("corvus_get_drive_parameters: Error status returned reading Constellation Parameter Block -- status: 0x%2.2x\n", status);
		m_xmit_bytes = 1;
		return status;
	}

	//
	// Build up the parameter packet
	//

	// This firmware string and revision were taken from the Corvus firmware
	// file CORVB184.CLR found on the SSE SoftBox distribution disk.
	strncpy((char *) m_buffer.drive_param_response.firmware_desc, "V18.4     -- CONST II - 11/82  ", sizeof(m_buffer.drive_param_response.firmware_desc));
	m_buffer.drive_param_response.firmware_rev = 37;

	// Controller ROM version
	m_buffer.drive_param_response.rom_version = ROM_VERSION;

	//
	// Track information
	//
	m_buffer.drive_param_response.track_info.sectors_per_track = m_sectors_per_track;
	m_buffer.drive_param_response.track_info.tracks_per_cylinder = m_tracks_per_cylinder;
	m_buffer.drive_param_response.track_info.cylinders_per_drive.msb = (m_cylinders_per_drive & 0xff00) >> 8;
	m_buffer.drive_param_response.track_info.cylinders_per_drive.lsb = (m_cylinders_per_drive & 0x00ff);

	//
	// Calculate the user capacity of the drive based on total capacity less spare tracks and firmware tracks
	//
	raw_capacity = m_tracks_per_cylinder * m_cylinders_per_drive * m_sectors_per_track; // Total capacity
	capacity = raw_capacity - ((m_tracks_per_cylinder * m_sectors_per_track * 2) + (SPARE_TRACKS * m_sectors_per_track));
	m_buffer.drive_param_response.capacity.msb = (capacity & 0xff0000) >> 16;
	m_buffer.drive_param_response.capacity.midb = (capacity & 0x00ff00) >> 8;
	m_buffer.drive_param_response.capacity.lsb = (capacity & 0x0000ff);

	//
	// Fill in the information from the Disk Parameter Block and Constellation Parameter Block
	//
	m_buffer.drive_param_response.interleave = raw_disk_parameter_block.dpb.interleave;
	memcpy(m_buffer.drive_param_response.table_info.mux_parameters, raw_constellation_parameter_block.cpb.mux_parameters, 12);
	memcpy(m_buffer.drive_param_response.table_info.pipe_name_table_ptr,
		raw_constellation_parameter_block.cpb.pipe_name_table_ptr, 2);
	memcpy(m_buffer.drive_param_response.table_info.pipe_ptr_table_ptr,
		raw_constellation_parameter_block.cpb.pipe_ptr_table_ptr, 2);
	memcpy(m_buffer.drive_param_response.table_info.pipe_area_size, raw_constellation_parameter_block.cpb.pipe_area_size, 2);
	memcpy(m_buffer.drive_param_response.table_info.vdo_table, raw_disk_parameter_block.dpb.vdo_table, 14);
	memcpy(m_buffer.drive_param_response.table_info.lsi11_vdo_table, raw_disk_parameter_block.dpb.lsi11_vdo_table, 8);
	memcpy(m_buffer.drive_param_response.table_info.lsi11_spare_table, raw_disk_parameter_block.dpb.lsi11_spare_table, 8);

	m_buffer.drive_param_response.drive_number = drv;
	m_buffer.drive_param_response.physical_capacity.msb = (raw_capacity & 0xff0000) >> 16;
	m_buffer.drive_param_response.physical_capacity.midb = (raw_capacity & 0x00ff00) >> 8;
	m_buffer.drive_param_response.physical_capacity.lsb = (raw_capacity & 0x0000ff);

	LOG(("corvus_get_drive_parameters: Drive Parameter packet follows:\n"));
	LOG_BUFFER(m_buffer.raw_data, 110);

	return STAT_SUCCESS;
}



//
// Corvus_Read_Boot_Block
//
// Old-style Boot (0x14) command boot block reader
//
// Pass:
//      block:  Boot block number to read (0-7)
//
// Returns:
//      status: Status of read operation
//
UINT8 corvus_hdc_t::corvus_read_boot_block(UINT8 block) {
	LOG(("corvus_read_boot_block: Reading boot block: %d\n", block));

	return corvus_read_sector(1, 25 + block, m_buffer.read_512_response.data, 512);
}



//
// corvus_enter_prep_mode
//
// Enter prep mode.  In prep mode, only prep mode commands may be executed.
//
// A "prep block" is 512 bytes of machine code that the host sends to the
// controller.  The controller will jump to this code after receiving it,
// and it is what actually implements prep mode commands.  This HLE ignores
// the prep block from the host.
//
// On the Rev B/H drives (which we emulate), a prep block is Z80 machine
// code and only one prep block can be sent.  Sending the "put drive into
// prep mode" command (0x11) when already in prep mode is an error.  The
// prep block sent by the Corvus program DIAG.COM on the SSE SoftBox
// distribution disk returns error 0x8f (unrecognized command) for this case.
//
// On the OmniDrive and Bank, a prep block is 6801 machine code.  These
// controllers allow multiple prep blocks to be sent.  The first time the
// "put drive into prep mode" command is sent puts the drive into prep mode.
// The command can then be sent again up to 3 times with more prep blocks.
// (Mass Storage GTI, pages 50-51)
//
// Pass:
//      drv:        Corvus drive id (1..15) to be prepped
//      prep_block: 512 bytes of machine code, contents ignored
//
// Returns:
//      Status of command
//
UINT8 corvus_hdc_t::corvus_enter_prep_mode(UINT8 drv, UINT8 *prep_block) {
	// on rev b/h drives, sending the "put drive into prep mode"
	// command when already in prep mode is an error.
	if (m_prep_mode) {
		logerror("corvus_enter_prep_mode: Attempt to enter prep mode while in prep mode\n");
		return STAT_FATAL_ERR | STAT_ILL_CMD_OP_CODE;
	}

	// check if drive is valid
	if (!corvus_hdc_file(drv)) {
		logerror("corvus_enter_prep_mode: Failure returned by corvus_hdc_file(%d)\n", drv);
		return STAT_FATAL_ERR | STAT_DRIVE_NOT_ONLINE;
	}

	LOG(("corvus_enter_prep_mode: Prep mode entered for drive %d, prep block follows:\n", drv));
	LOG_BUFFER(prep_block, 512);

	m_prep_mode = true;
	m_prep_drv = drv;
	return STAT_SUCCESS;
}



//
// corvus_exit_prep_mode (Prep Mode Only)
//
// Exit from prep mode and return to normal command mode.
//
// Returns:
//      Status of command (always success)
//
UINT8 corvus_hdc_t::corvus_exit_prep_mode() {
	LOG(("corvus_exit_prep_mode: Prep mode exited\n"));
	m_prep_mode = false;
	m_prep_drv = 0;
	return STAT_SUCCESS;
}



//
// Corvus_Read_Firmware_Block (Prep Mode Only)
//
// Reads firmware information from the first cylinder of the drive
//
// Pass:
//      head:   Head number
//      sector: Sector number
//
// Returns:
//      Status of command
//
UINT8 corvus_hdc_t::corvus_read_firmware_block(UINT8 head, UINT8 sector) {
	UINT16  relative_sector;    // Relative sector on drive for Physical Read
	UINT8   status;

	relative_sector = head * m_sectors_per_track + sector;

	LOG(("corvus_read_firmware_block: Reading firmware head: 0x%2.2x, sector: 0x%2.2x, relative_sector: 0x%2.2x\n",
		head, sector, relative_sector));

	status = corvus_read_sector(m_prep_drv, relative_sector, m_buffer.read_512_response.data, 512);
	return status;
}



//
// Corvus_Write_Firmware_Block (Prep Mode Only)
//
// Writes firmware information to the first cylinder of the drive
//
// Pass:
//      head:   Head number
//      sector: Sector number
//      buffer: Data to be written
//
// Returns:
//      Status of command
//
UINT8 corvus_hdc_t::corvus_write_firmware_block(UINT8 head, UINT8 sector, UINT8 *buffer) {
	UINT16  relative_sector;    // Relative sector on drive for Physical Read
	UINT8   status;

	relative_sector = head * m_sectors_per_track + sector;

	LOG(("corvus_write_firmware_block: Writing firmware head: 0x%2.2x, sector: 0x%2.2x, relative_sector: 0x%2.2x\n",
		head, sector, relative_sector));

	status = corvus_write_sector(m_prep_drv, relative_sector, buffer, 512);
	return status;
}



//
// Corvus_Format_Drive (Prep Mode Only)
//
// Write the pattern provided across the entire disk
//
// Pass:
//      pattern: 512-byte buffer containing the pattern to write to the whole drive
//
// Returns:
//      Status of command
//
UINT8 corvus_hdc_t::corvus_format_drive(UINT8 *pattern, UINT16 len) {
	UINT32  sector;
	UINT32  max_sector;
	UINT8   status = 0;
	UINT8   tbuffer[512];

	// Set up m_tracks_per_cylinder and m_sectors_per_track
	corvus_hdc_file(m_prep_drv);

	max_sector = m_sectors_per_track * m_tracks_per_cylinder * m_cylinders_per_drive;

	//
	// If we were passed less than 512 bytes, fill the buffer up with the first byte passed (for Omnidrive Format command)
	//
	if(len < 512) {
		memset(tbuffer, *pattern, 512);
		pattern = tbuffer;
	}

	LOG(("corvus_format_drive: Formatting drive with 0x%5.5x sectors, pattern buffer (passed length: %d) follows\n", max_sector, 512));
	LOG_BUFFER(pattern, 512);

	for(sector = 0; sector <= max_sector; sector++) {
		status = corvus_write_sector(m_prep_drv, sector, pattern, 512);
		if(status != STAT_SUCCESS) {
			logerror("corvus_format_drive: Error while formatting drive in corvus_write_sector--sector: 0x%5.5x, status: 0x%x2.2x\n",
				sector, status);
			break;
		}
	}

	return status;
}



//
// Corvus_HDC_File
//
// Returns a hard_disk_file object for a given virtual hard drive device in the concept
//
// Pass:
//      drv:    Corvus drive id (1..15)
//
// Returns:
//      hard_disk_file object
//
hard_disk_file *corvus_hdc_t::corvus_hdc_file(int drv) {
	static const char *const tags[] = {
		"harddisk1", "harddisk2", "harddisk3", "harddisk4"
	};

	// we only support 4 drives, as per the tags[] table, so prevent a crash
	// Corvus drive id numbers are 1-based so we check 1..4 instead of 0..3
	if (drv < 1 || drv > 4)
	{
		return nullptr;
	}

	harddisk_image_device *img = siblingdevice<harddisk_image_device>(tags[drv - 1]);

	if ( !img )
		return nullptr;

	if (!img->exists())
		return nullptr;

	// Pick up the Head/Cylinder/Sector info
	hard_disk_file *file = img->get_hard_disk_file();
	hard_disk_info *info = hard_disk_get_info(file);
	m_sectors_per_track = info->sectors;
	m_tracks_per_cylinder = info->heads;
	m_cylinders_per_drive = info->cylinders;

	LOG(("corvus_hdc_file: Attached to drive %u image: H:%d, C:%d, S:%d\n", drv, info->heads, info->cylinders, info->sectors));

	return file;
}



//
// Corvus_Process_Command_Packet
//
// Having received a complete packet from the host, process it
//
// Pass:
//      Invalid_Command_Flag:   Invalid command flag responses are handled in this routine
//
// Returns:
//      Nothing
//
void corvus_hdc_t::corvus_process_command_packet(bool invalid_command_flag) {
	if (VERBOSE_RESPONSES)
	{
		LOG(("corvus_hdc_data_w: Complete packet received.  Dump follows:\n"));
		LOG_BUFFER(m_buffer.raw_data, m_offset);
	}

	if(!invalid_command_flag) {
		if(!m_prep_mode) {
			switch(m_buffer.command.code) {
				//
				// Read / Write Chunk commands
				//
				case READ_CHUNK_128:
					m_buffer.read_128_response.status =
						corvus_read_logical_sector(&m_buffer.read_sector_command.dadr, m_buffer.read_128_response.data, 128);
					break;
				case READ_SECTOR_256:
				case READ_CHUNK_256:
					m_buffer.read_256_response.status =
						corvus_read_logical_sector(&m_buffer.read_sector_command.dadr, m_buffer.read_256_response.data, 256);
					break;
				case READ_CHUNK_512:
					m_buffer.read_512_response.status =
						corvus_read_logical_sector(&m_buffer.read_sector_command.dadr, m_buffer.read_512_response.data, 512);
					break;
				case WRITE_CHUNK_128:
					m_buffer.single_byte_response.status =
						corvus_write_logical_sector(&m_buffer.write_128_command.dadr, m_buffer.write_128_command.data, 128);
					break;
				case WRITE_SECTOR_256:
				case WRITE_CHUNK_256:
					m_buffer.single_byte_response.status =
						corvus_write_logical_sector(&m_buffer.write_256_command.dadr, m_buffer.write_256_command.data, 256);
					break;
				case WRITE_CHUNK_512:
					m_buffer.single_byte_response.status =
						corvus_write_logical_sector(&m_buffer.write_512_command.dadr, m_buffer.write_512_command.data, 512);
					break;
				//
				// Semaphore commands
				//
				case SEMAPHORE_LOCK_CODE:
			//  case SEMAPHORE_UNLOCK_CODE:
				case SEMAPHORE_INIT_CODE:
			//  case SEMAPHORE_STATUS_CODE:
					switch(m_buffer.command.modifier) {
						case SEMAPHORE_LOCK_MOD:
							m_buffer.semaphore_locking_response.status = corvus_lock_semaphore(m_buffer.lock_semaphore_command.name);
							break;
						case SEMAPHORE_UNLOCK_MOD:
							m_buffer.semaphore_locking_response.status =
								corvus_unlock_semaphore(m_buffer.unlock_semaphore_command.name);
							break;
						case SEMAPHORE_INIT_MOD:
							m_buffer.single_byte_response.status = corvus_init_semaphore_table();
							break;
						case SEMAPHORE_STATUS_MOD:
							m_buffer.semaphore_status_response.status =
								corvus_read_sector(1, 7, m_buffer.semaphore_status_response.table, 256);
							break;
						default:
							invalid_command_flag = true;
					}
					break;
				//
				// Miscellaneous commands
				//
				case BOOT:
					m_buffer.read_512_response.status =
						corvus_read_boot_block(m_buffer.old_boot_command.boot_block);
					break;
				case GET_DRIVE_PARAMETERS:
					m_buffer.drive_param_response.status =
						corvus_get_drive_parameters(m_buffer.get_drive_parameters_command.drive);
					break;
				case PREP_MODE_SELECT:
					m_buffer.single_byte_response.status =
						corvus_enter_prep_mode(m_buffer.prep_mode_command.drive,
							m_buffer.prep_mode_command.prep_block);
					break;
				default:
					m_xmit_bytes = 1;                      // Return a fatal status
					m_buffer.single_byte_response.status = STAT_FAULT | STAT_FATAL_ERR;
					logerror("corvus_hdc_data_w: Unimplemented command, returning FATAL FAULT status!\n");
					break;
			}
		} else {    // In Prep mode
			switch(m_buffer.command.code) {
				case PREP_MODE_SELECT:
					// when already in prep mode, some drives allow this command to
					// be sent again.  see corvus_enter_prep_mode() for details.
					m_buffer.single_byte_response.status =
						corvus_enter_prep_mode(m_buffer.prep_mode_command.drive,
							m_buffer.prep_mode_command.prep_block);
					break;
				case PREP_RESET_DRIVE:
					m_buffer.single_byte_response.status =
						corvus_exit_prep_mode();
					break;
				case PREP_READ_FIRMWARE:
					m_buffer.drive_param_response.status =
						corvus_read_firmware_block((m_buffer.read_firmware_command.encoded_h_s & 0xe0) >> 5,
							m_buffer.read_firmware_command.encoded_h_s & 0x1f);
					break;
				case PREP_WRITE_FIRMWARE:
					m_buffer.drive_param_response.status =
						corvus_write_firmware_block((m_buffer.write_firmware_command.encoded_h_s & 0xe0) >> 5,
							m_buffer.write_firmware_command.encoded_h_s & 0x1f, m_buffer.write_firmware_command.data);
					break;
				case PREP_FORMAT_DRIVE:
					m_buffer.drive_param_response.status =
						corvus_format_drive(m_buffer.format_drive_revbh_command.pattern, m_offset - 512);
					break;
				case PREP_VERIFY:
					m_buffer.verify_drive_response.status = STAT_SUCCESS;
					m_buffer.verify_drive_response.bad_sectors = 0;
					break;
				default:
					m_xmit_bytes = 1;
					m_buffer.single_byte_response.status = STAT_FAULT | STAT_FATAL_ERR;
					logerror("corvus_hdc_data_w: Unimplemented Prep command %02x, returning FATAL FAULT status!\n", m_buffer.command.code);
			}
		}
		if (VERBOSE_RESPONSES)
		{
			LOG(("corvus_hdc_data_w: Command execution complete, status: 0x%2.2x.  Response dump follows:\n",
				m_buffer.single_byte_response.status));
			LOG_BUFFER(m_buffer.raw_data, m_xmit_bytes);
		}

	} // if(!invalid_command_flag)

	//
	// Use a separate "if" in case the Invalid Command Flag was set as a result of a two-byte command
	//
	if(invalid_command_flag) {
		//
		// An Illegal command was detected (Truly invalid, not just unimplemented)
		//
		m_buffer.single_byte_response.status =
			STAT_FATAL_ERR | STAT_ILL_CMD_OP_CODE;      // Respond with an Illegal Op Code

		logerror("corvus_hdc_data_w: Illegal command 0x%2.2x, status: 0x%2.2x\n", m_buffer.command.code, m_buffer.single_byte_response.status);
	}
	//
	// Command execution complete, free up the controller
	//
	m_offset = 0;                                  // Point to beginning of buffer for response

	LOG(("corvus_hdc_data_w: Setting one-time mame timer of %d microseconds to simulate disk function\n", m_delay));

	//
	// Set up timers for command completion and timeout from host
	//
	//machine.scheduler().timer_set(attotime::from_usec(m_delay), FUNC(corvus_hdc_callback), CALLBACK_CTH_MODE);
	m_cmd_timer->adjust(attotime::from_usec(m_delay), CALLBACK_CTH_MODE);
	m_timeout_timer->enable(0);            // We've received enough data, disable the timeout timer

	m_delay = 0;                                   // Reset delay for next function
}



//
// Corvus_HDC_Callback
//
// Callback routine for completion of controller functions
//
// Pass:
//      Callback Function
//
// Returns:
//      Nothing
//
void corvus_hdc_t::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	int function = param;

	switch(function) {
		case CALLBACK_CTH_MODE:
			m_status |= CONTROLLER_DIRECTION;              // Set to Controller-to-Host, Ready mode
			m_status &= ~(CONTROLLER_BUSY);

			LOG(("corvus_hdc_callback: Callback executed with function CALLBACK_CTH_MODE\n"));

			break;
		case CALLBACK_HTC_MODE:
			m_status &= ~(CONTROLLER_DIRECTION |
				CONTROLLER_BUSY);                           // Set to Host-to-Controller, Ready mode

			LOG(("corvus_hdc_callback: Callback executed with function CALLBACK_HTC_MODE\n"));

			break;
		case CALLBACK_SAME_MODE:
			m_status &= ~(CONTROLLER_BUSY);                // Set the controller to Ready mode

			break;
		case CALLBACK_TIMEOUT:                              // We reached a four-second timeout threshold
			if(m_offset < m_recv_bytes || (m_offset > m_recv_bytes && m_recv_bytes != 0)) {
				m_buffer.single_byte_response.status = STAT_TIMEOUT;
				m_status |= CONTROLLER_DIRECTION;
				m_status &= ~(CONTROLLER_BUSY);
				m_recv_bytes = 0;
				m_xmit_bytes = 1;
				logerror("corvus_hdc_callback: Exceeded four-second timeout for data from host, resetting communications\n");
			} else { // if(m_recv_bytes == 0)                 This was a variable-size command
				LOG(("corvus_hdc_callback: Executing variable-length command via four-second timeout\n"));
				corvus_process_command_packet(0);          // Process the command
			}
			break;
		default:
			logerror("corvus_hdc_callback: FATAL ERROR -- Unknown callback function: %d\n", function);
			assert(0);
	}
	if(function != CALLBACK_SAME_MODE) {
		m_timeout_timer->enable(0);                // Disable the four-second timer now that we're done
	}
}



//
// Corvus_HDC_Init
//
// Global routine to initialize the Hard Disk Controller structures and arrays
//
// Pass:
//      Nothing
//
// Returns:
//      NULL if there's no file to attach to
//
void corvus_hdc_t::device_start() {
	m_status &= ~(CONTROLLER_DIRECTION | CONTROLLER_BUSY); // Host-to-controller mode, Idle (awaiting command from Host mode)
	m_prep_mode = false;                       // We're not in Prep Mode
	m_offset = 0;                              // Buffer is empty
	m_awaiting_modifier = false;               // We're not in the middle of a two-byte command
	m_xmit_bytes = 0;                          // We don't have anything to say to the host
	m_recv_bytes = 0;                          // We aren't waiting on additional data from the host

	m_timeout_timer = timer_alloc(TIMER_TIMEOUT);  // Set up a timer to handle the four-second host-to-controller timeout
	m_timeout_timer->adjust(attotime::from_seconds(4), CALLBACK_TIMEOUT);
	m_timeout_timer->enable(0);        // Start this timer out disabled

	m_cmd_timer = timer_alloc(TIMER_COMMAND);

	//
	// Define all of the packet sizes for the commands
	//

	// Read / Write commands
	corvus_cmd[READ_SECTOR_256][0].recv_bytes = 4;
	corvus_cmd[READ_SECTOR_256][0].xmit_bytes = 257;
	corvus_cmd[WRITE_SECTOR_256][0].recv_bytes = 260;
	corvus_cmd[WRITE_SECTOR_256][0].xmit_bytes = 1;
	corvus_cmd[READ_CHUNK_128][0].recv_bytes = 4;
	corvus_cmd[READ_CHUNK_128][0].xmit_bytes = 129;
	corvus_cmd[READ_CHUNK_256][0].recv_bytes = 4;
	corvus_cmd[READ_CHUNK_256][0].xmit_bytes = 257;
	corvus_cmd[READ_CHUNK_512][0].recv_bytes = 4;
	corvus_cmd[READ_CHUNK_512][0].xmit_bytes = 513;
	corvus_cmd[WRITE_CHUNK_128][0].recv_bytes = 132;
	corvus_cmd[WRITE_CHUNK_128][0].xmit_bytes = 1;
	corvus_cmd[WRITE_CHUNK_256][0].recv_bytes = 260;
	corvus_cmd[WRITE_CHUNK_256][0].xmit_bytes = 1;
	corvus_cmd[WRITE_CHUNK_512][0].recv_bytes = 516;
	corvus_cmd[WRITE_CHUNK_512][0].xmit_bytes = 1;

	// Semaphore commands
	corvus_cmd[SEMAPHORE_LOCK_CODE][SEMAPHORE_LOCK_MOD].recv_bytes = 10;
	corvus_cmd[SEMAPHORE_LOCK_CODE][SEMAPHORE_LOCK_MOD].xmit_bytes = 12;
	corvus_cmd[SEMAPHORE_UNLOCK_CODE][SEMAPHORE_UNLOCK_MOD].recv_bytes = 10;
	corvus_cmd[SEMAPHORE_UNLOCK_CODE][SEMAPHORE_UNLOCK_MOD].xmit_bytes = 12;
	corvus_cmd[SEMAPHORE_INIT_CODE][SEMAPHORE_INIT_MOD].recv_bytes = 5;
	corvus_cmd[SEMAPHORE_INIT_CODE][SEMAPHORE_INIT_MOD].xmit_bytes = 1;
	corvus_cmd[SEMAPHORE_STATUS_CODE][SEMAPHORE_STATUS_MOD].recv_bytes = 5;
	corvus_cmd[SEMAPHORE_STATUS_CODE][SEMAPHORE_STATUS_MOD].xmit_bytes = 257;

	// Pipe commands
	corvus_cmd[PIPE_READ_CODE][PIPE_READ_MOD].recv_bytes =  5;
	corvus_cmd[PIPE_READ_CODE][PIPE_READ_MOD].xmit_bytes =  516;
	corvus_cmd[PIPE_WRITE_CODE][PIPE_WRITE_MOD].recv_bytes = 517;
	corvus_cmd[PIPE_WRITE_CODE][PIPE_WRITE_MOD].xmit_bytes = 12;
	corvus_cmd[PIPE_CLOSE_CODE][PIPE_CLOSE_MOD].recv_bytes = 5;
	corvus_cmd[PIPE_CLOSE_CODE][PIPE_CLOSE_MOD].xmit_bytes = 12;
	corvus_cmd[PIPE_STATUS_CODE][PIPE_STATUS_MOD].recv_bytes = 5;
	corvus_cmd[PIPE_STATUS_CODE][PIPE_STATUS_MOD].xmit_bytes = 513; // There are actually two possibilities here
	corvus_cmd[PIPE_OPEN_WRITE_CODE][PIPE_OPEN_WRITE_MOD].recv_bytes = 10;
	corvus_cmd[PIPE_OPEN_WRITE_CODE][PIPE_OPEN_WRITE_MOD].xmit_bytes = 12;
	corvus_cmd[PIPE_AREA_INIT_CODE][PIPE_AREA_INIT_MOD].recv_bytes = 10;
	corvus_cmd[PIPE_AREA_INIT_CODE][PIPE_AREA_INIT_MOD].xmit_bytes = 12;
	corvus_cmd[PIPE_OPEN_READ_CODE][PIPE_OPEN_READ_MOD].recv_bytes = 10;
	corvus_cmd[PIPE_OPEN_READ_CODE][PIPE_OPEN_READ_MOD].xmit_bytes = 12;

	// Active User Table Commands
	corvus_cmd[ADDACTIVE_CODE][ADDACTIVE_MOD].recv_bytes = 18;
	corvus_cmd[ADDACTIVE_CODE][ADDACTIVE_MOD].xmit_bytes = 2;
	corvus_cmd[DELACTIVEUSR_REVBH_CODE][DELACTIVEUSR_REVBH_MOD].recv_bytes = 18;
	corvus_cmd[DELACTIVEUSR_REVBH_CODE][DELACTIVEUSR_REVBH_MOD].xmit_bytes = 2;
	corvus_cmd[DELACTIVENUM_OMNI_CODE][DELACTIVENUM_OMNI_MOD].recv_bytes = 18;
	corvus_cmd[DELACTIVENUM_OMNI_CODE][DELACTIVENUM_OMNI_MOD].xmit_bytes = 2;
	corvus_cmd[DELACTIVEUSR_OMNI_CODE][DELACTIVEUSR_OMNI_MOD].recv_bytes = 18;
	corvus_cmd[DELACTIVEUSR_OMNI_CODE][DELACTIVEUSR_OMNI_MOD].xmit_bytes = 2;
	corvus_cmd[FINDACTIVE_CODE][FINDACTIVE_MOD].recv_bytes = 18;
	corvus_cmd[FINDACTIVE_CODE][FINDACTIVE_MOD].xmit_bytes = 17;
	corvus_cmd[READTEMPBLOCK][0].recv_bytes = 2;
	corvus_cmd[READTEMPBLOCK][0].xmit_bytes = 513;
	corvus_cmd[WRITETEMPBLOCK][0].recv_bytes = 514;
	corvus_cmd[WRITETEMPBLOCK][0].xmit_bytes = 1;

	// Miscellaneous Commands
	corvus_cmd[BOOT][0].recv_bytes = 2;
	corvus_cmd[BOOT][0].xmit_bytes = 513;
	corvus_cmd[READ_BOOT_BLOCK][0].recv_bytes = 3;
	corvus_cmd[READ_BOOT_BLOCK][0].xmit_bytes = 513;
	corvus_cmd[GET_DRIVE_PARAMETERS][0].recv_bytes = 2;
	corvus_cmd[GET_DRIVE_PARAMETERS][0].xmit_bytes = 129;
	corvus_cmd[PARK_HEADS_REVH][0].recv_bytes = 514;
	corvus_cmd[PARK_HEADS_REVH][0].xmit_bytes = 1;
	corvus_cmd[PARK_HEADS_OMNI][0].recv_bytes = 1;
	corvus_cmd[PARK_HEADS_OMNI][0].xmit_bytes = 1;
	corvus_cmd[ECHO][0].recv_bytes = 513;
	corvus_cmd[ECHO][0].xmit_bytes = 513;

	// Put Drive in Prep Mode
	corvus_cmd[PREP_MODE_SELECT][0].recv_bytes = 514;
	corvus_cmd[PREP_MODE_SELECT][0].xmit_bytes = 1;

	// Prep Mode Commands
	corvus_prep_cmd[PREP_MODE_SELECT].recv_bytes = 514;
	corvus_prep_cmd[PREP_MODE_SELECT].xmit_bytes = 1;
	corvus_prep_cmd[PREP_RESET_DRIVE].recv_bytes = 1;
	corvus_prep_cmd[PREP_RESET_DRIVE].xmit_bytes = 1;
	corvus_prep_cmd[PREP_FORMAT_DRIVE].recv_bytes = 0;
	corvus_prep_cmd[PREP_FORMAT_DRIVE].xmit_bytes = 1;
	corvus_prep_cmd[PREP_FILL_DRIVE_OMNI].recv_bytes = 3;
	corvus_prep_cmd[PREP_FILL_DRIVE_OMNI].xmit_bytes = 1;
	corvus_prep_cmd[PREP_VERIFY].recv_bytes = 1;
	corvus_prep_cmd[PREP_VERIFY].xmit_bytes = 2;
	corvus_prep_cmd[PREP_READ_FIRMWARE].recv_bytes = 2;
	corvus_prep_cmd[PREP_READ_FIRMWARE].xmit_bytes = 513;
	corvus_prep_cmd[PREP_WRITE_FIRMWARE].recv_bytes = 514;
	corvus_prep_cmd[PREP_WRITE_FIRMWARE].xmit_bytes = 1;

	LOG(("corvus_hdc_init: Drive structures initialized\n"));
}


//
// Corvus_HDC_Status_R
//
// Global routine to read the Status Register from the Controller (Controller to Host)
//
// Pass:
//      Nothing
//
// Returns:
//      Value in the controller status register
//
READ8_MEMBER ( corvus_hdc_t::status_r ) {
	return m_status;
}



//
// Corvus_HDC_Data_R
//
// Read the Data Register from the Controller (Controller to Host).  If transmission is complete,
// as defined as offset == bytes to transmit, reset the status to Host-to-Controller mode and Idle
// when complete.
//
// Pass:
//      Nothing
//
// Returns:
//      Value in the controller data register
//
READ8_MEMBER ( corvus_hdc_t::read ) {
	UINT8 result;

	if((m_status & CONTROLLER_DIRECTION) == 0) {   // Check to see if we're in Controller-to-Host mode
		logerror("corvus_hdc_data_r: Data register read when in Host-to-Controller mode (status: 0x%2.2x)\n", m_status);
		return 0;
	}

	if((m_status & CONTROLLER_BUSY) != 0) {        // Check to see if we're Busy
		logerror("corvus_hdc_data_r: Data register read when Busy (status: 0x%2.2x)\n", m_status);
		return 0;
	}

	result = m_buffer.raw_data[m_offset++];

	if(m_offset == m_xmit_bytes) {
		LOG(("corvus_hdc_data_r: Finished transmitting %d bytes of data.  Returning to idle mode.\n", m_xmit_bytes));

		m_offset = 0;          // We've reached the end of valid data
		m_xmit_bytes = 0;      // We don't have anything more to say
		m_recv_bytes = 0;      // No active commands

		m_cmd_timer->adjust(attotime::from_usec(INTERBYTE_DELAY), CALLBACK_HTC_MODE);

//      m_status &= ~(CONTROLLER_DIRECTION | CONTROLLER_BUSY); // Put us in Idle, Host-to-Controller mode
	} else {
		//
		// Not finished with this packet.  Insert an interbyte delay and then let the host continue
		//
		m_cmd_timer->adjust(attotime::from_usec(INTERBYTE_DELAY), CALLBACK_SAME_MODE);
	}

	return result;
}



//
// Corvus_HDC_Data_W
//
// Write to the Data Register on the Controller (Host to Controller)
//
// Pass:
//      Value to write to controller data register
//
// Returns:
//      Nothing
//
WRITE8_MEMBER ( corvus_hdc_t::write ) {
	//
	// Received a byte -- check to see if we should really respond
	//
	if((m_status & CONTROLLER_DIRECTION) != 0) {       // System wrote to controller when controller wasn't listening
		logerror("corvus_hdc_data_w: Data register written when in Controller-to-Host mode (status: 0x%2.2x, data: 0x%2.2x)\n",
			m_status, data);
		return;
	}

	if((m_status & CONTROLLER_BUSY) != 0) {            // System wrote to controller when controller was busy
		logerror("corvus_hdc_data_w: Data register written when controller not Ready (status: 0x%2.2x, data: 0x%2.2x)\n",
			m_status, data);
		return;
	}

	//
	// We're supposed to be paying attention.  Make a decision about the data received
	//
	if(m_offset == 0)  {                                                   // First byte of a packet
		LOG(("corvus_hdc_data_w: Received a byte with m_offset == 0.  Processing as command: 0x%2.2x\n", data));
		m_invalid_command_flag = parse_hdc_command(data);
		m_timeout_timer->reset((attotime::from_seconds(4)));
		m_timeout_timer->enable(1);                                // Start our four-second timer
	} else if(m_offset == 1 && m_awaiting_modifier) {                     // Second byte of a packet
		LOG(("corvus_hdc_data_w: Received a byte while awaiting modifier with m_offset == 0.  Processing as modifier: 0x%2.2x\n", data));
		m_awaiting_modifier = false;
		m_recv_bytes = corvus_cmd[m_buffer.command.code][data].recv_bytes;
		m_xmit_bytes = corvus_cmd[m_buffer.command.code][data].xmit_bytes;
	}

	m_buffer.raw_data[m_offset++] = data;

	assert(m_offset <= MAX_COMMAND_SIZE);                                  // Something is wrong, or I undersized the buffer

	//
	// We now have enough information to make a decision whether to execute the command, respond with a fatal response
	// or just wait for more data.  If we can do something, execute the command.  Otherwise, just fall through and return
	// to the user with us Ready for more data and in Host-to-Controller mode.
	//
	if(m_offset == m_recv_bytes) {                        // We've received enough data to process
		corvus_process_command_packet(m_invalid_command_flag);
	} else {
		//
		// Reset the four-second timer since we received some data
		//
		m_timeout_timer->reset((attotime::from_seconds(4)));

		//
		// Make the controller busy for a few microseconds while the command is processed
		//
		m_status |= CONTROLLER_BUSY;
		m_cmd_timer->adjust(attotime::from_usec(INTERBYTE_DELAY), CALLBACK_SAME_MODE);
	}
}
