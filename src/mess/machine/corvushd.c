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
//      Implement Multiple physical drive support - Up to four
//      Implement Drive Illegal Addresses (seek past last sector)
//      Implement Switches on front of drive (LSI-11, MUX, Format, Reset)
//      Implement an inter-sector delay during the FORMAT command (format happens too quickly now)
//

#include "emu.h"
#include "imagedev/harddriv.h"
#include "includes/corvushd.h"
#include <ctype.h>


#define VERBOSE 0
#define VERBOSE_RESPONSES 0
#define VERSION 1
#define MAX_COMMAND_SIZE 4096	// The maximum size of a command packet (the controller only has 5K of RAM...)
#define SPARE_TRACKS 7			// This is a Rev B drive, so 7 it is
#define CALLBACK_CTH_MODE 1		// Set to Controller-to-Host mode when callback fires
#define CALLBACK_HTC_MODE 2		// Set to Host-to-Controller mode when callback fires
#define CALLBACK_SAME_MODE 3	// Leave mode the same when callback fires
#define CALLBACK_TIMEOUT 4		// Four seconds have elapsed.  We're timing out
#define TRACK_SEEK_TIME 1667	// Track-to-track seek time in microseconds (Maximum Access Time / Total Cylinders)
#define INTERBYTE_DELAY 5		// Inter-byte delay in microseconds communicating between controller and host
#define INTERSECTOR_DELAY 25000	// 25ms delay between sectors (4800 RPM = 80 Rev/Second.  Maximum 2 sectors transferred / Rev)

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)
#define LOG_BUFFER(p,s) do { if (VERBOSE) dump_buffer(p,s); } while (0)
//
// Structures
//

// Sector addressing scheme for Rev B/H drives used in various commands (Called a DADR in the docs)
struct dadr_t {
	UINT8 address_msn_and_drive;// Most significant nibble: Most signficant nibble of sector address, Least significant nibble: Drive #
	UINT8 address_lsb;			// Least significant byte of sector address
	UINT8 address_mid;			// Middle byte of sector address
};

// Controller structure
struct corvus_hdc_t {
	UINT8	status;				// Controller status byte (DIRECTION + BUSY/READY)
	char	prep_mode;			// Whether the controller is in Prep Mode or not
	// Physical drive info
	UINT8	sectors_per_track;	// Number of sectors per track for this drive
	UINT8	tracks_per_cylinder;// Number of tracks per cylinder (heads)
	UINT16	cylinders_per_drive;// Number of cylinders per drive
	// Command Processing
	UINT16	offset;				// Current offset into raw_data buffer
	char	awaiting_modifier;	// We've received a two-byte command and we're waiting for the mod
	UINT16	recv_bytes;			// Number of bytes expected to be received from Host
	UINT16	xmit_bytes;			// Number of bytes expected to be transmitted to host
	// Timing-related values
	UINT16	last_cylinder;		// Last cylinder accessed - for calculating seek times
	UINT32	delay;				// Delay in microseconds for callback
	emu_timer	*timeout_timer;	// Four-second timer for timeouts
	UINT8	invalid_command_flag;		// I hate this, but it saves a lot more tests

	//
	// Union below represents both an input and output buffer and interpretations of it
	//
	union {
		//
		// Raw Buffer
		//
		UINT8		raw_data[MAX_COMMAND_SIZE];
		//
		// Basic interpretation of code and modifier
		//
		struct {
			UINT8	code;		// First byte of data is the code (command)
			UINT8	modifier;	// Second byte of data is the modifier
		} command;
		//
		// Basic response code
		//
		struct {
			UINT8	status;		// Status code returned by the command executed
		} single_byte_response;
		//
		// Read sector command
		//
		struct {
			UINT8	code;		// Command code
			dadr_t	dadr;		// Encoded drive and sector to read
		} read_sector_command;
		//
		// 128-byte Read Sector response
		//
		struct {
			UINT8	status;		// Status code returned by command executed
			UINT8	data[128];	// Data returned from read
		} read_128_response;
		//
		// 256-byte Read Sector response
		//
		struct {
			UINT8	status;		// Status code returned by command executed
			UINT8	data[256];	// Data returned from read
		} read_256_reponse;
		//
		// 512-byte Read Sector response
		//
		struct {
			UINT8	status;		// Status code returned by command executed
			UINT8	data[512];	// Data returned by read
		} read_512_response;
		//
		// Write 128-byte sector command
		//
		struct {
			UINT8	code;		// Command code
			dadr_t	dadr;		// Encoded drive and sector to write
			UINT8	data[128];	// Data to be written
		} write_128_command;
		//
		// Write 256-byte sector command
		//
		struct {
			UINT8	code;		// Command code
			dadr_t	dadr;		// Encoded drive and sector to write
			UINT8	data[256];	// Data to be written
		} write_256_command;
		//
		// Write 512-byte sector command
		//
		struct {
			UINT8	code;		// Command Code
			dadr_t	dadr;		// Encoded drive and sector to write
			UINT8	data[512];	// Data to be written
		} write_512_command;
		//
		// Semaphore Lock command
		//
		struct {
			UINT8	code;		// Command code
			UINT8	modifier;	// Command code modifier
			UINT8	name[8];	// Semaphore name
		} lock_semaphore_command;
		//
		// Semaphore Unlock command
		//
		struct {
			UINT8	code;		// Command code
			UINT8	modifier;	// Command code modifier
			UINT8	name[8];	// Semaphore name
		} unlock_semaphore_command;
		//
		// Semaphore Lock/Unlock response
		//
		struct {
			UINT8	status;		// Disk access status
			UINT8	result;		// Semaphore action status
			UINT8	unused[10];	// Unused
		} semaphore_locking_response;
		//
		// Initialize Semaphore table command
		//
		struct {
			UINT8	code;		// Command code
			UINT8	modifier;	// Command code modifier
			UINT8	unused[3];	// Unused
		} init_semaphore_command;
		//
		// Semaphore Status command
		//
		struct {
			UINT8	code;		// Command code
			UINT8	modifier;	// Command code modifier
			UINT8	zero_three;	// Don't ask me...
			UINT8	unused[2];	// Unused
		} semaphore_status_command;
		//
		// Semaphore Status response
		//
		struct {
			UINT8	status;		// Disk access status
			UINT8	table[256];	// Contents of the semaphore table
		} semaphore_status_response;
		//
		// Get Drive Parameters command (0x10)
		//
		struct {
			UINT8	code;		// Command code
			UINT8	drive;		// Drive number (starts at 1)
		} get_drive_parameters_command;
		//
		// Get Drive Parameters command response
		//
		struct {
			UINT8	status;						// Status code returned by command executed
			UINT8	firmware[33];				// Firmware message
			UINT8	rom_version;				// ROM Version
			struct {
				UINT8	sectors_per_track;		// Sectors/Track
				UINT8	tracks_per_cylinder;	// Tracks/Cylinder (heads)
				struct {
					UINT8	lsb;
					UINT8	msb;
				} cylinders_per_drive;			// Byte-flipped Cylinders/Drive
			} track_info;
			struct {
				UINT8	lsb;					// Least significant byte
				UINT8	midb;					// Middle byte
				UINT8	msb;					// Most significant byte
			} capacity;							// 24-bit value, byte-flipped (lsb..msb)
			UINT8	unused[16];
			UINT8	interleave;					// Interleave factor
			struct {
				UINT8	mux_parameters[12];
				UINT8	pipe_name_table_ptr[2];	// Pointer to table of 64 entries, 8 bytes each (table of names)
				UINT8	pipe_ptr_table_ptr[2];	// Pointer to table of 64 entries, 8 bytes each.  See pp. 29 - Mass Storage GTI
				UINT8	pipe_area_size[2];		// Size of pipe area (lsb, msb)
				struct {
					UINT8	track_offset[2];
				} vdo_table[7];					// Virtual drive table
				UINT8	lsi11_vdo_table[8];
				UINT8	lsi11_spare_table[8];
			} table_info;
			UINT8	drive_number;				// Physical drive number
			struct {
				UINT8	lsb;					// Least
				UINT8	midb;					// Middle
				UINT8	msb;					// Most
			} physical_capacity;				// Physical capacity of drive
		} drive_param_response;
		//
		// 2-byte Boot command (0x14)
		//
		struct {
			UINT8	code;		// Command code
			UINT8	boot_block;	// Which boot block to read (0-7)
		} old_boot_command;
		//
		// Read Firmware command (Prep Mode 0x32)
		//
		struct {
			UINT8	code;		// Command Code
			UINT8	encoded_h_s;// Encoded Head (bits 7-5) / Sector (bits 4-0)
		} read_firmware_command;
		//
		// Write Firmware command (Prep Mode 0x33)
		//
		struct {
			UINT8	code;		// Command Code
			UINT8	encoded_h_s; // Encoded Head (bits 7-5) / Sector (bits 4-0)
			UINT8	data[512];	// Data to be written
		} write_firmware_command;
		//
		// Format Drive command (Prep Mode 0x01)
		//
		// Note that the following is a BLATANT ASSUMPTION.  Technically, the Format Drive command
		// uses a variable-length buffer for the pattern.  Unfortunately, the docs don't explain how to determine the
		// length of the buffer passed.  I assume it's a timeout; however, the docs happen to say that
		// all Corvus diagnostic programs send 513 bytes total, including the command, so I'm going with that.
		//
		struct {
			UINT8	code;		// Command Code
			UINT8	pattern[512]; // Pattern to be written
		} format_drive_revbh_command;
	} buffer;
};

// Structure of Block #1, the Disk Parameter Block
struct disk_parameter_block_t {
	struct {
		UINT8	lsb;
		UINT8	msb;
	} spared_track[8];			// Spared track table (0xffff indicates end)
	UINT8	interleave;			// Interleave factor
	UINT8	reserved;
	struct {
		UINT8 track_offset[2];	// Virtual drive offsets (lsb, msb) 0xffff indicates unused
	} vdo_table[7];
	UINT8	lsi11_vdo_table[8];
	UINT8	lsi11_spare_table[8];
	UINT8	reserved2[432];
	struct {
		UINT8	lsb;
		UINT8	msb;
	} revh_spare_table[16];
};

// Structure of Block #3, the Constellation Parameter Block
struct constellation_parameter_block_t {
	UINT8	mux_parameters[12];
	UINT8	pipe_name_table_ptr[2];
	UINT8	pipe_ptr_table_ptr[2];
	UINT8	pipe_area_size[2];
	UINT8	reserved[470];
	UINT8	software_protection[12];
	UINT8	serial_number[12];
};

// Structure of Block #7, the Semaphore Table Block
struct semaphore_table_block_t {
	union {
		UINT8	semaphore_table[256];			// Table consists of 256 bytes
		struct {
			UINT8	semaphore_name[8];			// Each semaphore name is 8 bytes
		} semaphore_entry[32];					// 32 Entries
	} semaphore_block;
	UINT8	unused[256];						// Remaining half of block is unused
};

// Command size structure (number of bytes to xmit and recv for each command)
struct corvus_cmd_t {
	UINT16	recv_bytes;							// Number of bytes from host for this command
	UINT16	xmit_bytes;							// Number of bytes to return to host
};

//
// Prototypes
//
static hard_disk_file *corvus_hdc_file(running_machine &machine, int id);
static TIMER_CALLBACK(corvus_hdc_callback);

//
// Globals
//
static corvus_hdc_t	corvus_hdc;					// The controller itself
static corvus_cmd_t	corvus_cmd[0xf5][0xc1];		// Command sizes and their return sizes
static corvus_cmd_t	corvus_prep_cmd[0x82];		// Prep Command sizes and their return sizes



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
static void dump_buffer(UINT8 *buffer, UINT16 length) {

	UINT16	offset;
	char	ascii_dump[16];

	logerror("dump_buffer: Dump of %d bytes:\n", length);
	logerror("Base  00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f ASCII\n");
	logerror("----  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- ----------------");

	for(offset=0; offset < length; offset++) {
		if(offset % 16 == 0) {					// WHY IS 0 % 16 == 0???
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
// return either TRUE or FALSE as to whether the command was invalid or not.
//
// Note that recv_bytes and xmit_bytes in the corvus_hdc structure are updated as
// a side-effect of this command, as is awaiting_modifier.
//
// Pass:
//      data:   Initial byte received from the host in Host to Controller mode
//
// Returns:
//      Whether the command was invalid or not (TRUE = invalid command)
//
static UINT8 parse_hdc_command(UINT8 data) {

	corvus_hdc_t *c = &corvus_hdc;

	c->awaiting_modifier = FALSE;				// This is the case by definition

	LOG(("parse_hdc_command: Called with data: 0x%2.2x, Prep mode is: %d\n", data, c->prep_mode));

	if(!c->prep_mode) {
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
				c->recv_bytes = corvus_cmd[data][0].recv_bytes;
				c->xmit_bytes = corvus_cmd[data][0].xmit_bytes;
				LOG(("parse_hdc_command: Single byte command recoginized: 0x%2.2x, to recv: %d, to xmit: %d\n", data,
					c->recv_bytes, c->xmit_bytes));
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
				c->awaiting_modifier = TRUE;
				LOG(("parse_hdc_command: Double byte command recoginized: 0x%2.2x\n", data));
				break;

			default:							// This is an INVALID command
				c->recv_bytes = 1;
				c->xmit_bytes = 1;
				LOG(("parse_hdc_command: Invalid command detected: 0x%2.2x\n", data));
				return TRUE;
		}
	} else {
		switch(data) {
			//
			// Prep Commands
			//
			case PREP_MODE_SELECT:				// Apparently I need to be able to do this while in Prep mode
			case PREP_RESET_DRIVE:
			case PREP_FORMAT_DRIVE:
			case PREP_FILL_DRIVE_OMNI:
			case PREP_VERIFY:
			case PREP_READ_FIRMWARE:
			case PREP_WRITE_FIRMWARE:
				c->recv_bytes = corvus_prep_cmd[data].recv_bytes;
				c->xmit_bytes = corvus_prep_cmd[data].xmit_bytes;
				LOG(("parse_hdc_command: Prep command recognized: 0x%2.2x, to recv: %d, to xmit: %d\n", data,
					c->recv_bytes, c->xmit_bytes));
				break;

			default:							// This is an INVALID prep command
				c->recv_bytes = 1;
				c->xmit_bytes = 1;
				LOG(("parse_hdc_command: Invalid Prep command detected: 0x%2.2x\n", data));
				return TRUE;
		}
	}	// if(!prep_mode)

	return FALSE;
}



//
// Corvus_Write_Sector
//
// Write a variably-sized chunk of data to the CHD file
//
// Pass:
//      drv:    Drive number to write to
//      sector: Physical sector number to write to
//      buffer: Buffer to write
//      len:    Length of the buffer (amount of data to write)
//
// Returns:
//      status: Command status
//
static UINT8 corvus_write_sector(running_machine &machine, UINT8 drv, UINT32 sector, UINT8 *buffer, int len) {

	corvus_hdc_t
			*c = &corvus_hdc;
	hard_disk_file
			*disk;				// Structures for interface to CHD routines
	UINT8	tbuffer[512];		// Buffer to hold an entire sector
	UINT16	cylinder;			// Cylinder this sector resides on

	LOG(("corvus_write_sector: Write Drive: %d, physical sector: 0x%5.5x\n", drv, sector));

	disk = corvus_hdc_file(machine, drv);
	if(!disk) {
		logerror("corvus_write_sector: Failure returned by corvus_hdc_file(%d)\n", drv);
		return STAT_FATAL_ERR | STAT_DRIVE_NOT_ONLINE;
	}

	//
	// Calculate what cylinder the sector resides on for timing purposes
	//
	cylinder = (double) sector / (double) c->sectors_per_track / (double) c->tracks_per_cylinder;
	c->delay = abs(c->last_cylinder - cylinder) * TRACK_SEEK_TIME + INTERSECTOR_DELAY;

	//
	// Corvus supports write sizes of 128, 256 and 512 bytes.  In the case of a write smaller than
	// the sector size of 512 bytes, the sector is read, the provided data is overlayed and then the
	// sector is written back out.  See pp. 5 of the Mass Storage Systems GTI for the details of this
	// wonderful functionality.
	//
	if(len == 512) {
		hard_disk_write(disk, sector, buffer);
	} else {
		hard_disk_read(disk, sector, tbuffer);		// Read the existing data into our temporary buffer
		memcpy(tbuffer, buffer, len);					// Overlay the data with the buffer passed
		c->delay += INTERSECTOR_DELAY;					// Add another delay because of the Read / Write
		hard_disk_write(disk, sector, tbuffer);		// Re-write the data
	}

	c->last_cylinder = cylinder;

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
static UINT8 corvus_write_logical_sector(running_machine &machine, dadr_t *dadr, UINT8 *buffer, int len) {

	corvus_hdc_t
			*c = &corvus_hdc;
	UINT8	status;				// Status returned from Physical Sector read
	UINT8	drv;				// Drive number (1 - 15)
	UINT32	sector;				// Sector number on drive

	//
	// Unencode the first byte of the DADR
	//
	// High-order nibble of first byte is the most-significant nibble of the sector address
	// Low-order nibble of first byte is the drive id
	//
	// For example: 0x23 would decode to Drive ID #3, high-order nibble: 0x02.
	//
	drv = (dadr->address_msn_and_drive & 0x0f) - 1;
	sector = (dadr->address_msn_and_drive & 0xf0 << 12) | (dadr->address_mid << 8) | dadr->address_lsb;

	LOG(("corvus_write_logical_sector: Writing based on DADR: 0x%6.6x, logical sector: 0x%5.5x, drive: %d\n",
		dadr->address_msn_and_drive << 16 | dadr->address_lsb << 8 | dadr->address_mid, sector, drv));

	//
	// Shift the logical sector address forward by the number of firmware cylinders (2) + the number of spare tracks (7)
	//
	sector += (c->tracks_per_cylinder * c->sectors_per_track * 2) + (SPARE_TRACKS * c->sectors_per_track);

	status = corvus_write_sector(machine, drv, sector, buffer, len);

	if(status != STAT_SUCCESS)
		c->xmit_bytes = 1;

	return status;
}


//
// Corvus_Read_Sector
//
// Read a variably-sized chunk of data from the CHD file
//
// Pass:
//      drv:    Drive number to read from
//      sector: Physical sector number to read from
//      buffer: Buffer to hold the data read from the disk
//      len:    Length of the buffer
//
// Returns:
//      status: Corvus status
//
static UINT8 corvus_read_sector(running_machine &machine, UINT8 drv, UINT32 sector, UINT8 *buffer, int len) {

	corvus_hdc_t
			*c = &corvus_hdc;
	hard_disk_file
			*disk;				// Structures for interface to CHD routines
	UINT8	tbuffer[512];		// Buffer to store full sector results in
	UINT16	cylinder;

	LOG(("corvus_read_sector: Read Drive: %d, physical sector: 0x%5.5x\n", drv, sector));

	disk = corvus_hdc_file(machine, drv);
	if(!disk) {
		logerror("corvus_read_sector: Failure returned by corvus_hdc_file(%d)\n", drv);
		return STAT_FATAL_ERR | STAT_DRIVE_NOT_ONLINE;
	}

	//
	// Calculate what cylinder the sector resides on for timing purposes
	//
	cylinder = (double) sector / (double) c->sectors_per_track / (double) c->tracks_per_cylinder;
	c->delay = abs(c->last_cylinder - cylinder) * TRACK_SEEK_TIME + INTERSECTOR_DELAY;

	hard_disk_read(disk, sector, tbuffer);

	memcpy(buffer, tbuffer, len);

	c->last_cylinder = cylinder;

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
static UINT8 corvus_read_logical_sector(running_machine &machine, dadr_t *dadr, UINT8 *buffer, int len) {

	corvus_hdc_t
			*c = &corvus_hdc;
	UINT8	status;								// Status returned from Physical Sector read
	UINT8	drv;								// Drive number (1 - 15)
	UINT32	sector;								// Sector number on drive

	//
	// Unencode the first byte of the DADR
	//
	// High-order nibble of first byte is the most-significant nibble of the sector address
	// Low-order nibble of first byte is the drive id
	//
	// For example: 0x23 would decode to Drive ID #3, high-order nibble: 0x02.
	//
	drv = (dadr->address_msn_and_drive & 0x0f) - 1;
	sector = (dadr->address_msn_and_drive & 0xf0 << 12) | (dadr->address_mid << 8) | dadr->address_lsb;

	LOG(("corvus_read_logical_sector: Reading based on DADR: 0x%6.6x, logical sector: 0x%5.5x, drive: %d\n",
		dadr->address_msn_and_drive << 16 | dadr->address_lsb << 8 | dadr->address_mid, sector, drv));

	//
	// Shift the logical sector address forward by the number of firmware cylinders (2) + the number of spare tracks (7)
	//
	sector += (c->tracks_per_cylinder * c->sectors_per_track * 2) + (SPARE_TRACKS * c->sectors_per_track);

	status = corvus_read_sector(machine, drv, sector, buffer, len);

	if(status != STAT_SUCCESS)
		c->xmit_bytes = 1;

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
static UINT8 corvus_lock_semaphore(running_machine &machine, UINT8 *name) {

	corvus_hdc_t
			*c = &corvus_hdc;
	semaphore_table_block_t
			semaphore_table;
	UINT8	offset = 0;
	UINT8	found = FALSE;
	UINT8	blank_offset = 32;	// Initialize to invalid offset
	UINT8	status;				// Status returned from Physical Sector read

	//
	// Read the semaphore table from the drive
	//
	status = corvus_read_sector(machine, 0, 7, semaphore_table.semaphore_block.semaphore_table, 256);
	if(status != STAT_SUCCESS) {
		logerror("corvus_lock_semaphore: Error reading semaphore table, status: 0x%2.2x\n", status);
		c->buffer.semaphore_locking_response.result = SEM_DISK_ERROR;
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
			found = TRUE;
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
			c->buffer.semaphore_locking_response.result = SEM_TABLE_FULL;					// No space for the semaphore!
		} else {
			c->buffer.semaphore_locking_response.result = SEM_PRIOR_STATE_NOT_SET;			// It wasn't there already
			memcpy(&semaphore_table.semaphore_block.semaphore_entry[blank_offset], name, 8);// Stick it into the table
			status = corvus_write_sector(machine, 0, 7, semaphore_table.semaphore_block.semaphore_table, 256);
			if(status != STAT_SUCCESS) {
				logerror("corvus_lock_semaphore: Error updating semaphore table, status: 0x%2.2x\n", status);
				c->buffer.semaphore_locking_response.result = SEM_DISK_ERROR;
				return status;
			}
		}
	} else {
		c->buffer.semaphore_locking_response.result = SEM_PRIOR_STATE_SET;					// It's already locked -- sorry
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
static UINT8 corvus_unlock_semaphore(running_machine &machine, UINT8 *name) {

	corvus_hdc_t
			*c = &corvus_hdc;
	semaphore_table_block_t
			semaphore_table;
	UINT8	offset = 0;
	UINT8	found = FALSE;
	UINT8	status;				// Status returned from Physical Sector read

	//
	// Read the semaphore table from the drive
	//
	status = corvus_read_sector(machine, 0, 7, semaphore_table.semaphore_block.semaphore_table, 256);
	if(status != STAT_SUCCESS) {
		logerror("corvus_unlock_semaphore: Error reading semaphore table, status: 0x%2.2x\n", status);
		c->buffer.semaphore_locking_response.result = SEM_DISK_ERROR;
		return status;
	}

	//
	// Search the semaphore table to see if the semaphore already exists--if so it's locked
	//
	do {
		if(strncmp((char *) &semaphore_table.semaphore_block.semaphore_entry[offset], (char *) name, 8) == 0) {
			found = TRUE;
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
		c->buffer.semaphore_locking_response.result = SEM_PRIOR_STATE_NOT_SET;				// It wasn't there already
	} else {
		c->buffer.semaphore_locking_response.result = SEM_PRIOR_STATE_SET;					// It was there
		memcpy(&semaphore_table.semaphore_block.semaphore_entry[offset], "        ", 8);	// Clear it
		status = corvus_write_sector(machine, 0, 7, semaphore_table.semaphore_block.semaphore_table, 256);
		if(status != STAT_SUCCESS) {
			logerror("corvus_unlock_semaphore: Error updating semaphore table, status: 0x%2.2x\n", status);
			c->buffer.semaphore_locking_response.result = SEM_DISK_ERROR;
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
static UINT8 corvus_init_semaphore_table( running_machine &machine ) {

	semaphore_table_block_t
			semaphore_table;
	UINT8	status;

	memset(semaphore_table.semaphore_block.semaphore_table, 0x20, 256);

	status = corvus_write_sector(machine, 0, 7, semaphore_table.semaphore_block.semaphore_table, 256);
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
//      drv:    Drive number to get parameters from
//
// Returns:
//      Status of command
//
static UINT8 corvus_get_drive_parameters(running_machine &machine, UINT8 drv) {

	corvus_hdc_t
			*c = &corvus_hdc;
	UINT16	capacity;							// Number of usable 512-byte blocks
	UINT16	raw_capacity;						// Number of actual 512-byte blocks
	union {
		UINT8
			buffer[512];
		disk_parameter_block_t
			dpb;
	} raw_disk_parameter_block;					// Buffer for the Disk Parameter Block
	union {
		UINT8
			buffer[512];
		constellation_parameter_block_t
			cpb;
	} raw_constellation_parameter_block;		// Buffer for the Constellation Parameter Block
	UINT8	status;								// Status to return

	//
	// Make sure a valid drive is being accessed
	//
	drv -= 1;									// Internally, drives start at 0

	if ( ! corvus_hdc_file( machine, drv ) )
	{
		logerror("corvus_get_drive_parameters: Attempt to retrieve parameters from non-existant drive: %d\n", drv);
		c->xmit_bytes = 1;
		return STAT_FATAL_ERR | STAT_DRIVE_NOT_ONLINE;
	}

	//
	// Read the Disk Parameter Block (Sector 1) from the drive
	//
	status = corvus_read_sector(machine, drv, 1, raw_disk_parameter_block.buffer, 512);
	if(status != STAT_SUCCESS) {
		logerror("corvus_get_drive_parameters: Error status returned reading Disk Parameter Block -- status: 0x%2.2x\n", status);
		c->xmit_bytes = 1;
		return status;
	}

	//
	// Read the Constellation Parameter Block (Sector 3) from the drive
	//
	status = corvus_read_sector(machine, drv, 3, raw_constellation_parameter_block.buffer, 512);
	if(status != STAT_SUCCESS) {
		logerror("corvus_get_drive_parameters: Error status returned reading Constellation Parameter Block -- status: 0x%2.2x\n", status);
		c->xmit_bytes = 1;
		return status;
	}

	//
	// Build up the parameter packet
	//
	strcpy((char *) c->buffer.drive_param_response.firmware, "V18.4AP   -- CONST II - 11/82  %");	// Pulled from some firmware...
	c->buffer.drive_param_response.rom_version = VERSION;
	c->buffer.drive_param_response.track_info.sectors_per_track = c->sectors_per_track;
	c->buffer.drive_param_response.track_info.tracks_per_cylinder = c->tracks_per_cylinder;
	c->buffer.drive_param_response.track_info.cylinders_per_drive.msb = (c->cylinders_per_drive & 0xff00) >> 8;
	c->buffer.drive_param_response.track_info.cylinders_per_drive.lsb = (c->cylinders_per_drive & 0x00ff);

	//
	// Calculate the user capacity of the drive based on total capacity less spare tracks and firmware tracks
	//
	raw_capacity = c->tracks_per_cylinder * c->cylinders_per_drive * c->sectors_per_track; // Total capacity
	capacity = raw_capacity - ((c->tracks_per_cylinder * c->sectors_per_track * 2) + (SPARE_TRACKS * c->sectors_per_track));
	c->buffer.drive_param_response.capacity.msb = (capacity & 0xff0000) >> 16;
	c->buffer.drive_param_response.capacity.midb = (capacity & 0x00ff00) >> 8;
	c->buffer.drive_param_response.capacity.lsb = (capacity & 0x0000ff);

	//
	// Fill in the information from the Disk Parameter Block and Constellation Parameter Block
	//
	c->buffer.drive_param_response.interleave = raw_disk_parameter_block.dpb.interleave;
	memcpy(c->buffer.drive_param_response.table_info.mux_parameters, raw_constellation_parameter_block.cpb.mux_parameters, 12);
	memcpy(c->buffer.drive_param_response.table_info.pipe_name_table_ptr,
		raw_constellation_parameter_block.cpb.pipe_name_table_ptr, 2);
	memcpy(c->buffer.drive_param_response.table_info.pipe_ptr_table_ptr,
		raw_constellation_parameter_block.cpb.pipe_ptr_table_ptr, 2);
	memcpy(c->buffer.drive_param_response.table_info.pipe_area_size, raw_constellation_parameter_block.cpb.pipe_area_size, 2);
	memcpy(c->buffer.drive_param_response.table_info.vdo_table, raw_disk_parameter_block.dpb.vdo_table, 14);
	memcpy(c->buffer.drive_param_response.table_info.lsi11_vdo_table, raw_disk_parameter_block.dpb.lsi11_vdo_table, 8);
	memcpy(c->buffer.drive_param_response.table_info.lsi11_spare_table, raw_disk_parameter_block.dpb.lsi11_spare_table, 8);

	c->buffer.drive_param_response.drive_number = drv + 1;
	c->buffer.drive_param_response.physical_capacity.msb = (raw_capacity & 0xff0000) >> 16;
	c->buffer.drive_param_response.physical_capacity.midb = (raw_capacity & 0x00ff00) >> 8;
	c->buffer.drive_param_response.physical_capacity.lsb = (raw_capacity & 0x0000ff);

	LOG(("corvus_get_drive_parameters: Drive Parameter packet follows:\n"));
	LOG_BUFFER(c->buffer.raw_data, 110);

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
static UINT8 corvus_read_boot_block(running_machine &machine, UINT8 block) {

	corvus_hdc_t	*c = &corvus_hdc;			// Pick up global controller structure

	LOG(("corvus_read_boot_block: Reading boot block: %d\n", block));

	return corvus_read_sector(machine, 0, 25 + block, c->buffer.read_512_response.data, 512);

}



//
// Corvus_Read_Firmware_Block
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
static UINT8 corvus_read_firmware_block(running_machine &machine, UINT8 head, UINT8 sector) {

	corvus_hdc_t
			*c = &corvus_hdc;	// Pick up global controller structure
	UINT16	relative_sector;	// Relative sector on drive for Physical Read
	UINT8	status;

	relative_sector = head * c->sectors_per_track + sector;

	LOG(("corvus_read_firmware_block: Reading firmware head: 0x%2.2x, sector: 0x%2.2x, relative_sector: 0x%2.2x\n",
		head, sector, relative_sector));

	status = corvus_read_sector(machine, 0, relative_sector, c->buffer.read_512_response.data, 512);		// TODO: Which drive should Prep Mode talk to ???
	return status;
}



//
// Corvus_Write_Firmware_Block
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
static UINT8 corvus_write_firmware_block(running_machine &machine, UINT8 head, UINT8 sector, UINT8 *buffer) {

	corvus_hdc_t
			*c = &corvus_hdc;	// Pick up global controller structure
	UINT16	relative_sector;	// Relative sector on drive for Physical Read
	UINT8	status;

	relative_sector = head * c->sectors_per_track + sector;

	LOG(("corvus_write_firmware_block: Writing firmware head: 0x%2.2x, sector: 0x%2.2x, relative_sector: 0x%2.2x\n",
		head, sector, relative_sector));

	status = corvus_write_sector(machine, 0, relative_sector, buffer, 512);	// TODO: Which drive should Prep Mode talk to ???
	return status;
}



//
// Corvus_Format_Drive
//
// Write the pattern provided across the entire disk
//
// Pass:
//      pattern: 512-byte buffer containing the pattern to write to the whole drive
//
// Returns:
//      Status of command
//
static UINT8 corvus_format_drive(running_machine &machine, UINT8 *pattern, UINT16 len) {

	corvus_hdc_t
			*c = &corvus_hdc;
	UINT32	sector;
	UINT32	max_sector;
	UINT8	status = 0;
	UINT8	tbuffer[512];

	max_sector = c->sectors_per_track * c->tracks_per_cylinder * c->cylinders_per_drive;

	//
	// If we were passed less than 512 bytes, fill the buffer up with the first byte passed (for Omnidrive Format command)
	//
	if(len < 512) {
		memset(tbuffer, *pattern, 512);
		pattern = tbuffer;
	}

	LOG(("corvus_format_drive: Formatting drive with 0x%5.5x sectors, pattern buffer (passed length: %d)follows\n", max_sector, 512));
	LOG_BUFFER(pattern, 512);

	for(sector = 0; sector <= max_sector; sector++) {
		status = corvus_write_sector(machine, 0, sector, pattern, 512);
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
//      id:     Drive number (1 - 15)
//
// Returns:
//      hard_disk_file object
//
static hard_disk_file *corvus_hdc_file(running_machine &machine, int id) {
	static const char *const tags[] = {
		"harddisk1"
	};
	harddisk_image_device *img;

	/* Only one harddisk supported right now */
	assert ( id == 0 );

	img = dynamic_cast<harddisk_image_device *>(machine.device(tags[id]));

	if ( !img )
		return NULL;

	if (!img->exists())
		return NULL;

	return img->get_hard_disk_file();
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
static void corvus_process_command_packet(running_machine &machine, UINT8 invalid_command_flag) {

	corvus_hdc_t	*c = &corvus_hdc;

	if (VERBOSE_RESPONSES)
	{
		LOG(("corvus_hdc_data_w: Complete packet received.  Dump follows:\n"));
		LOG_BUFFER(c->buffer.raw_data, c->offset);
	}

	if(!invalid_command_flag) {
		if(!c->prep_mode) {
			switch(c->buffer.command.code) {
				//
				// Read / Write Chunk commands
				//
				case READ_CHUNK_128:
					c->buffer.read_128_response.status =
						corvus_read_logical_sector(machine, &c->buffer.read_sector_command.dadr, c->buffer.read_128_response.data, 128);
					break;
				case READ_SECTOR_256:
				case READ_CHUNK_256:
					c->buffer.read_256_reponse.status =
						corvus_read_logical_sector(machine, &c->buffer.read_sector_command.dadr, c->buffer.read_256_reponse.data, 256);
					break;
				case READ_CHUNK_512:
					c->buffer.read_512_response.status =
						corvus_read_logical_sector(machine, &c->buffer.read_sector_command.dadr, c->buffer.read_512_response.data, 512);
					break;
				case WRITE_CHUNK_128:
					c->buffer.single_byte_response.status =
						corvus_write_logical_sector(machine, &c->buffer.write_128_command.dadr, c->buffer.write_128_command.data, 128);
					break;
				case WRITE_SECTOR_256:
				case WRITE_CHUNK_256:
					c->buffer.single_byte_response.status =
						corvus_write_logical_sector(machine, &c->buffer.write_256_command.dadr, c->buffer.write_256_command.data, 256);
					break;
				case WRITE_CHUNK_512:
					c->buffer.single_byte_response.status =
						corvus_write_logical_sector(machine, &c->buffer.write_512_command.dadr, c->buffer.write_512_command.data, 512);
					break;
				//
				// Semaphore commands
				//
				case SEMAPHORE_LOCK_CODE:
			//  case SEMAPHORE_UNLOCK_CODE:
				case SEMAPHORE_INIT_CODE:
			//  case SEMAPHORE_STATUS_CODE:
					switch(c->buffer.command.modifier) {
						case SEMAPHORE_LOCK_MOD:
							c->buffer.semaphore_locking_response.status = corvus_lock_semaphore(machine, c->buffer.lock_semaphore_command.name);
							break;
						case SEMAPHORE_UNLOCK_MOD:
							c->buffer.semaphore_locking_response.status =
								corvus_unlock_semaphore(machine, c->buffer.unlock_semaphore_command.name);
							break;
						case SEMAPHORE_INIT_MOD:
							c->buffer.single_byte_response.status = corvus_init_semaphore_table(machine);
							break;
						case SEMAPHORE_STATUS_MOD:
							c->buffer.semaphore_status_response.status =
								corvus_read_sector(machine, 0, 7, c->buffer.semaphore_status_response.table, 256);
							break;
						default:
							invalid_command_flag = TRUE;
					}
					break;
				//
				// Miscellaneous commands
				//
				case BOOT:
					c->buffer.read_512_response.status =
						corvus_read_boot_block(machine, c->buffer.old_boot_command.boot_block);
					break;
				case GET_DRIVE_PARAMETERS:
					c->buffer.drive_param_response.status =
						corvus_get_drive_parameters(machine, c->buffer.get_drive_parameters_command.drive);
					break;
				case PREP_MODE_SELECT:
					c->prep_mode = TRUE;
					c->buffer.single_byte_response.status = STAT_SUCCESS;
					break;
				default:
					c->xmit_bytes = 1;						// Return a fatal status
					c->buffer.single_byte_response.status = STAT_FAULT | STAT_FATAL_ERR;
					logerror("corvus_hdc_data_w: Unimplemented command, returning FATAL FAULT status!\n");
					break;
			}
		} else {	// In Prep mode
			switch(c->buffer.command.code) {
				case PREP_MODE_SELECT:
					c->prep_mode = TRUE;
					c->buffer.single_byte_response.status = STAT_SUCCESS;
					break;
				case PREP_RESET_DRIVE:
					c->prep_mode = FALSE;
					c->buffer.single_byte_response.status = STAT_SUCCESS;
					break;
				case PREP_READ_FIRMWARE:
					c->buffer.drive_param_response.status =
						corvus_read_firmware_block(machine, (c->buffer.read_firmware_command.encoded_h_s & 0xe0) >> 5,
							c->buffer.read_firmware_command.encoded_h_s & 0x1f);
					break;
				case PREP_WRITE_FIRMWARE:
					c->buffer.drive_param_response.status =
						corvus_write_firmware_block(machine, (c->buffer.write_firmware_command.encoded_h_s & 0xe0) >> 5,
							c->buffer.write_firmware_command.encoded_h_s & 0x1f, c->buffer.write_firmware_command.data);
					break;
				case PREP_FORMAT_DRIVE:
					c->buffer.drive_param_response.status =
						corvus_format_drive(machine, c->buffer.format_drive_revbh_command.pattern, c->offset - 512);
					break;
				default:
					c->xmit_bytes = 1;
					c->buffer.single_byte_response.status = STAT_FAULT | STAT_FATAL_ERR;
					logerror("corvus_hdc_data_w: Unimplemented Prep command, returning FATAL FAULT status!\n");
			}
		}
		if (VERBOSE_RESPONSES)
		{
			LOG(("corvus_hdc_data_w: Command execution complete, status: 0x%2.2x.  Response dump follows:\n",
				c->buffer.single_byte_response.status));
			LOG_BUFFER(c->buffer.raw_data, c->xmit_bytes);
		}

	} // if(!invalid_command_flag)

	//
	// Use a separate "if" in case the Invalid Command Flag was set as a result of a two-byte command
	//
	if(invalid_command_flag) {
		//
		// An Illegal command was detected (Truly invalid, not just unimplemented)
		//
		c->buffer.single_byte_response.status =
			STAT_FATAL_ERR | STAT_ILL_CMD_OP_CODE;		// Respond with an Illegal Op Code

		logerror("corvus_hdc_data_w: Illegal Command, status: 0x%2.2x\n", c->buffer.single_byte_response.status);
	}
	//
	// Command execution complete, free up the controller
	//
	c->offset = 0;									// Point to beginning of buffer for response

	LOG(("corvus_hdc_data_w: Setting one-time mame timer of %d microseconds to simulate disk function\n", c->delay));

	//
	// Set up timers for command completion and timeout from host
	//
	machine.scheduler().timer_set(attotime::from_usec(c->delay), FUNC(corvus_hdc_callback), CALLBACK_CTH_MODE);
	c->timeout_timer->enable(0);			// We've received enough data, disable the timeout timer

	c->delay = 0;									// Reset delay for next function
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
static TIMER_CALLBACK(corvus_hdc_callback)
{
	int function = param;
	corvus_hdc_t *c = &corvus_hdc;

	switch(function) {
		case CALLBACK_CTH_MODE:
			c->status |= CONTROLLER_DIRECTION;				// Set to Controller-to-Host, Ready mode
			c->status &= ~(CONTROLLER_BUSY);

			LOG(("corvus_hdc_callback: Callback executed with function CALLBACK_CTH_MODE\n"));

			break;
		case CALLBACK_HTC_MODE:
			c->status &= ~(CONTROLLER_DIRECTION |
				CONTROLLER_BUSY);							// Set to Host-to-Controller, Ready mode

			LOG(("corvus_hdc_callback: Callback executed with function CALLBACK_HTC_MODE\n"));

			break;
		case CALLBACK_SAME_MODE:
			c->status &= ~(CONTROLLER_BUSY);				// Set the controller to Ready mode

			break;
		case CALLBACK_TIMEOUT:								// We reached a four-second timeout threshold
			if(c->offset < c->recv_bytes || (c->offset > c->recv_bytes && c->recv_bytes != 0)) {
				c->buffer.single_byte_response.status = STAT_TIMEOUT;
				c->status |= CONTROLLER_DIRECTION;
				c->status &= ~(CONTROLLER_BUSY);
				c->recv_bytes = 0;
				c->xmit_bytes = 1;
				logerror("corvus_hdc_callback: Exceeded four-second timeout for data from host, resetting communications\n");
			} else { // if(c->recv_bytes == 0)                 This was a variable-size command
				LOG(("corvus_hdc_callback: Executing variable-length command via four-second timeout\n"));
				corvus_process_command_packet(machine, 0);			// Process the command
			}
			break;
		default:
			logerror("corvus_hdc_callback: FATAL ERROR -- Unknown callback function: %d\n", function);
			assert(0);
	}
	if(function != CALLBACK_SAME_MODE) {
		c->timeout_timer->enable(0);				// Disable the four-second timer now that we're done
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
UINT8 corvus_hdc_init(running_machine &machine) {

	corvus_hdc_t			*c = &corvus_hdc;	// Pick up global controller structure
	hard_disk_file	*disk;				// Structures for interface to CHD routines
	hard_disk_info	*info;

	if((disk = corvus_hdc_file(machine, 0)))				// Attach to the CHD file
		info = hard_disk_get_info(disk);		// Pick up the Head/Cylinder/Sector info
	else
		return 0;

	c->status &= ~(CONTROLLER_DIRECTION | CONTROLLER_BUSY);	// Host-to-controller mode, Idle (awaiting command from Host mode)
	c->prep_mode = FALSE;						// We're not in Prep Mode
	c->sectors_per_track = info->sectors;
	c->tracks_per_cylinder = info->heads;
	c->cylinders_per_drive = info->cylinders;
	c->offset = 0;								// Buffer is empty
	c->awaiting_modifier = FALSE;				// We're not in the middle of a two-byte command
	c->xmit_bytes = 0;							// We don't have anything to say to the host
	c->recv_bytes = 0;							// We aren't waiting on additional data from the host

	c->timeout_timer = machine.scheduler().timer_alloc(FUNC(corvus_hdc_callback));	// Set up a timer to handle the four-second host-to-controller timeout
	c->timeout_timer->adjust(attotime::from_seconds(4), CALLBACK_TIMEOUT);
	c->timeout_timer->enable(0);		// Start this timer out disabled

	LOG(("corvus_hdc_init: Attached to drive image: H:%d, C:%d, S:%d\n", info->heads, info->cylinders, info->sectors));

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
	corvus_cmd[PIPE_READ_CODE][PIPE_READ_MOD].recv_bytes =	5;
	corvus_cmd[PIPE_READ_CODE][PIPE_READ_MOD].xmit_bytes =	516;
	corvus_cmd[PIPE_WRITE_CODE][PIPE_WRITE_MOD].recv_bytes = 517;
	corvus_cmd[PIPE_WRITE_CODE][PIPE_WRITE_MOD].xmit_bytes = 12;
	corvus_cmd[PIPE_CLOSE_CODE][PIPE_CLOSE_MOD].recv_bytes = 5;
	corvus_cmd[PIPE_CLOSE_CODE][PIPE_CLOSE_MOD].xmit_bytes = 12;
	corvus_cmd[PIPE_STATUS_CODE][PIPE_STATUS_MOD].recv_bytes = 5;
	corvus_cmd[PIPE_STATUS_CODE][PIPE_STATUS_MOD].xmit_bytes = 513;	// There are actually two possibilities here
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
	corvus_prep_cmd[PREP_VERIFY].xmit_bytes = 0;
	corvus_prep_cmd[PREP_READ_FIRMWARE].recv_bytes = 2;
	corvus_prep_cmd[PREP_READ_FIRMWARE].xmit_bytes = 513;
	corvus_prep_cmd[PREP_WRITE_FIRMWARE].recv_bytes = 514;
	corvus_prep_cmd[PREP_WRITE_FIRMWARE].xmit_bytes = 1;

	LOG(("corvus_hdc_init: Drive structures initialized\n"));

	return TRUE;
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
READ8_HANDLER ( corvus_hdc_status_r ) {

	corvus_hdc_t *c = &corvus_hdc;

	return c->status;
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
READ8_HANDLER ( corvus_hdc_data_r ) {

	corvus_hdc_t *c = &corvus_hdc;
	UINT8 result;

	if((c->status & CONTROLLER_DIRECTION) == 0) {	// Check to see if we're in Controller-to-Host mode
		logerror("corvus_hdc_data_r: Data register read when in Host-to-Controller mode (status: 0x%2.2x)\n", c->status);
		return 0;
	}

	if((c->status & CONTROLLER_BUSY) != 0) {		// Check to see if we're Busy
		logerror("corvus_hdc_data_r: Data register read when Busy (status: 0x%2.2x)\n", c->status);
		return 0;
	}

	result = c->buffer.raw_data[c->offset++];

	if(c->offset == c->xmit_bytes) {
		LOG(("corvus_hdc_data_r: Finished transmitting %d bytes of data.  Returning to idle mode.\n", c->xmit_bytes));

		c->offset = 0;			// We've reached the end of valid data
		c->xmit_bytes = 0;		// We don't have anything more to say
		c->recv_bytes = 0;		// No active commands

		space.machine().scheduler().timer_set((attotime::from_usec(INTERBYTE_DELAY)), FUNC(corvus_hdc_callback), CALLBACK_HTC_MODE);

//      c->status &= ~(CONTROLLER_DIRECTION | CONTROLLER_BUSY); // Put us in Idle, Host-to-Controller mode
	} else {
		//
		// Not finished with this packet.  Insert an interbyte delay and then let the host continue
		//
		space.machine().scheduler().timer_set((attotime::from_usec(INTERBYTE_DELAY)), FUNC(corvus_hdc_callback), CALLBACK_SAME_MODE);
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
WRITE8_HANDLER ( corvus_hdc_data_w ) {

	corvus_hdc_t	*c = &corvus_hdc;

	//
	// Received a byte -- check to see if we should really respond
	//
	if((c->status & CONTROLLER_DIRECTION) != 0) {		// System wrote to controller when controller wasn't listening
		logerror("corvus_hdc_data_w: Data register written when in Controller-to-Host mode (status: 0x%2.2x, data: 0x%2.2x)\n",
			c->status, data);
		return;
	}

	if((c->status & CONTROLLER_BUSY) != 0) {			// System wrote to controller when controller was busy
		logerror("corvus_hdc_data_w: Data register written when controller not Ready (status: 0x%2.2x, data: 0x%2.2x)\n",
			c->status, data);
		return;
	}

	//
	// We're supposed to be paying attention.  Make a decision about the data received
	//
	if(c->offset == 0)	{													// First byte of a packet
		LOG(("corvus_hdc_data_w: Received a byte with c->offset == 0.  Processing as command: 0x%2.2x\n", data));
		c->invalid_command_flag = parse_hdc_command(data);
		c->timeout_timer->reset((attotime::from_seconds(4)));
		c->timeout_timer->enable(1);								// Start our four-second timer
	} else if(c->offset == 1 && c->awaiting_modifier) {						// Second byte of a packet
		LOG(("corvus_hdc_data_w: Received a byte while awaiting modifier with c->offset == 0.  Processing as modifier: 0x%2.2x\n", data));
		c->awaiting_modifier = FALSE;
		c->recv_bytes = corvus_cmd[c->buffer.command.code][data].recv_bytes;
		c->xmit_bytes = corvus_cmd[c->buffer.command.code][data].xmit_bytes;
	}

	c->buffer.raw_data[c->offset++] = data;

	assert(c->offset <= MAX_COMMAND_SIZE);									// Something is wrong, or I undersized the buffer

	//
	// We now have enough information to make a decision whether to execute the command, respond with a fatal response
	// or just wait for more data.  If we can do something, execute the command.  Otherwise, just fall through and return
	// to the user with us Ready for more data and in Host-to-Controller mode.
	//
	if(c->offset == c->recv_bytes) {						// We've received enough data to process
		corvus_process_command_packet(space.machine(), c->invalid_command_flag);
	} else {
		//
		// Reset the four-second timer since we received some data
		//
		c->timeout_timer->reset((attotime::from_seconds(4)));

		//
		// Make the controller busy for a few microseconds while the command is processed
		//
		c->status |= CONTROLLER_BUSY;
		space.machine().scheduler().timer_set((attotime::from_usec(INTERBYTE_DELAY)), FUNC(corvus_hdc_callback), CALLBACK_SAME_MODE);
	}
}
