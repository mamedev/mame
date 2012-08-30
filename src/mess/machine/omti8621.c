/*
 * omti8621.c - SMS OMTI 8621 disk controller (for Apollo DN3x00)
 *
 *  Created on: August 30, 2010
 *      Author: Hans Ostermeyer
 *
 *  Released for general non-commercial use under the MAME license
 *  Visit http://mamedev.org for licensing and usage restrictions.
 *
 *  see also:
 *  * http://www.bitsavers.org/pdf/sms/pc/OMTI_AT_Controller_Series_Jan87.pdf
 */

#define VERBOSE 0

static int verbose = VERBOSE;

#include "machine/omti8621.h"
#include "image.h"

#define LOG(x)	{ logerror ("%s: ", cpu_context(state->device)); logerror x; logerror ("\n"); }
#define LOG1(x)	{ if (verbose > 0) LOG(x)}
#define LOG2(x)	{ if (verbose > 1) LOG(x)}
#define LOG3(x)	{ if (verbose > 2) LOG(x)}

#define DLOG(x)	{ logerror ("%s: ", cpu_context(disk->device)); logerror x; logerror ("\n"); }
#define DLOG1(x)	{ if (verbose > 0) DLOG(x)}
#define DLOG2(x)	{ if (verbose > 1) DLOG(x)}

#define OMTI_DISK_SECTOR_SIZE 1056

#define OMTI_DISK_TYPE_155_MB 0x607 // Micropolis 1355 (170 MB Dtype = 607)
#define OMTI_DISK_TYPE_348_MB 0x604 // Maxtor EXT-4380-E (380 MB Dtype = 604)
#define OMTI_DISK_TYPE_DEFAULT OMTI_DISK_TYPE_348_MB // new disks will have this type (and size)

#define OMTI_MAX_BLOCK_COUNT 32

#define OMTI_MAX_LUN 1
#define OMTI_DISK0_TAG "omti_disk0"
#define OMTI_DISK1_TAG "omti_disk1"

#define CDB_SIZE 10

/*
 * I/O register offsets
 */

#define OMTI_PORT_DATA_IN    0x00    /* read, 8-bit */
#define OMTI_PORT_DATA_OUT   0x00    /* write, 8-bit */
#define OMTI_PORT_STATUS     0x01    /* read, 8-bit */
#define OMTI_PORT_RESET      0x01    /* write, 8-bit */
#define OMTI_PORT_CONFIG     0x02    /* read, 8-bit */
#define OMTI_PORT_SELECT     0x02    /* write, 8-bit */
#define OMTI_PORT_MASK       0x03    /* write only, 8-bit */

// port status

#define OMTI_STATUS_REQ  0x01 // Request (1 = request transfer of data via data in/out register)
#define OMTI_STATUS_IO   0x02 // In/Out (1 = direction of transfer is from controller to host)
#define OMTI_STATUS_CD   0x04 // Command/Data ( 1 = byte transfered is command or status byte)
#define OMTI_STATUS_BUSY 0x08 // Busy (0 = controller is idle, 1 = controller selected)
#define OMTI_STATUS_DREQ 0x10 // Data Request (0 = no DMA request, 1 = DMA cycle requested)
#define OMTI_STATUS_IREQ 0x20 // Interrupt Request (0 = no interrupt, 1 = command complete)
#define OMTI_STATUS_NU6  0x40 // not used
#define OMTI_STATUS_NU7  0x80 // not used

#define OMTI_CONFIG_W23  0x01 // jumper W23
#define OMTI_CONFIG_W22  0x02 // jumper W22
#define OMTI_CONFIG_W21  0x04 // jumper W21
#define OMTI_CONFIG_W20  0x08 // jumper W20

#define OMTI_MASK_DMAE  0x01 // DMA enable
#define OMTI_MASK_INTE  0x02 // Interrupt enable

#define OMTI_COMMAND_STATUS_ERROR 0x02 // error bit
#define OMTI_COMMAND_STATUS_LUN 0x20 // drive 0 is 0

#define OMTI_SENSE_CODE_NO_ERROR 0x00
#define OMTI_SENSE_CODE_DRIVE_NOT_READY 0x04
#define OMTI_SENSE_CODE_ADDRESS_VALID 0x80
#define OMTI_SENSE_CODE_SECTOR_NOT_FOUND 0x14
#define OMTI_SENSE_CODE_ECC_ERROR 0x11
#define OMTI_SENSE_CODE_BAD_TRACK 0x19
#define OMTI_SENSE_CODE_ALTERNATE_TRACK 0x1C
#define OMTI_SENSE_CODE_INVALID_COMMAND 0x20
#define OMTI_SENSE_CODE_ILLEGAL_ADDRESS 0x21

enum {
	OMTI_STATE_RESET,
	OMTI_STATE_IDLE,
	OMTI_STATE_SELECTION,
	OMTI_STATE_COMMAND,
	OMTI_STATE_DATA,
	OMTI_STATE_STATUS,
};

// OMTI commands

#define OMTI_CMD_TEST_DRIVE_READY 0x00
#define OMTI_CMD_RECALIBRATE 0x01

#define OMTI_CMD_REQUEST_SENSE 0x03
#define OMTI_CMD_FORMAT_DRIVEUNIT 0x04
#define OMTI_CMD_READ_VERIFY 0x05
#define OMTI_CMD_FORMAT_TRACK 0x06
#define OMTI_CMD_FORMAT_BAD_TRACK 0x07
#define OMTI_CMD_READ 0x08

#define OMTI_CMD_WRITE 0x0a
#define OMTI_CMD_SEEK 0x0b

#define OMTI_CMD_READ_SECTOR_BUFFER 0x0e
#define OMTI_CMD_WRITE_SECTOR_BUFFER 0x0f

#define OMTI_CMD_ASSIGN_ALTERNATE_TRACK 0x11

#define OMTI_CMD_READ_DATA_TO_BUFFER 0x1e
#define OMTI_CMD_WRITE_DATA_FROM_BUFFER 0x1f
#define OMTI_CMD_COPY 0x20

#define OMTI_CMD_READ_ESDI_DEFECT_LIST 0x37

#define OMTI_CMD_RAM_DIAGNOSTICS 0xe0
#define OMTI_CMD_CONTROLLER_INT_DIAGNOSTIC 0xe4
#define OMTI_CMD_READ_LONG 0xe5
#define OMTI_CMD_WRITE_LONG 0xe6

#define OMTI_CMD_READ_CONFIGURATION 0xec
#define OMTI_CMD_INVALID_COMMAND 0xff

typedef struct _disk_data disk_data;
struct _disk_data
{
	device_t *device;
    UINT16 type;
    UINT16 cylinders;
    UINT16 heads;
	UINT16 sectors;
	UINT32 sectorbytes;
	UINT32 sector_count;

	device_image_interface *image;

	// configuration data
	UINT8 config_data[10];

	// ESDI defect list data
	UINT8 esdi_defect_list[256];
};

typedef struct _omti8621_state omti8621_state;
struct _omti8621_state {
	device_t *device;
	omti8621_set_irq irq_handler;

	disk_data *disk[OMTI_MAX_LUN+1];

	UINT16 jumper;

	/* stored tc state - state present at pins */
	int tc_state;

	/* stored int state */
	int int_state;

	UINT8 omti_state;

	UINT8 status_port;
	UINT8 config_port;
	UINT8 mask_port;

	// command descriptor block
	UINT8 command_buffer[CDB_SIZE];
	int command_length;
	int command_index;
	int command_status;

	// data buffer
	UINT8 *sector_buffer;
	UINT8 *data_buffer;
	int data_length;
	int data_index;

	// sense data
	UINT8 sense_data[4];

	// these are used only to satisfy dex
	UINT32 diskaddr_ecc_error;
	UINT32 diskaddr_format_bad_track;
	UINT8 alternate_track_buffer[4];
	UINT32 alternate_track_address[2];
};

/***************************************************************************
 get_safe_token - makes sure that the passed
 in device is, in fact, a 8621 controller
 ***************************************************************************/

INLINE omti8621_state *get_safe_token(device_t *device) {
	assert(device != NULL);
	assert(device->type() == OMTI8621);
	return (omti8621_state *) downcast<legacy_device_base *>(device)->token();
}

/***************************************************************************
 cpu_context - return a string describing the current CPU context
 ***************************************************************************/

static const char *cpu_context(const device_t *device) {
	static char statebuf[64]; /* string buffer containing state description */

	device_t *cpu = device->machine().firstcpu;

	/* if we have an executing CPU, output data */
	if (cpu != NULL) {
		osd_ticks_t t = osd_ticks();
		int s = t / osd_ticks_per_second();
		int ms = (t % osd_ticks_per_second()) / 1000;

		sprintf(statebuf, "%d.%03d %s pc=%08x - %s", s, ms, cpu->tag(),
				cpu_get_previouspc(cpu), device->tag());
	} else {
		strcpy(statebuf, "(no context)");
	}
	return statebuf;
}

/*-------------------------------------------------
 set_interrupt - update the IRQ state
 -------------------------------------------------*/

static void set_interrupt(const omti8621_state *state, enum line_state line_state) {
	if (state->irq_handler != NULL) {
		LOG2(("set_interrupt: status_port=%x", state->status_port));
		(*state->irq_handler)(&state->device->machine(), line_state);
	}
}

static TIMER_CALLBACK(set_interrupt_caba ) {
	const omti8621_state *state =(omti8621_state *)ptr;
	set_interrupt(state, ASSERT_LINE);
}

/***************************************************************************
 clear_sense_data - clear the sense data
 ***************************************************************************/

static void clear_sense_data(omti8621_state *state) {
	LOG2(("clear_sense_data"));
	memset(state->sense_data, 0, sizeof(state->sense_data));
}

/***************************************************************************
 set_sense_data - set the sense data from code and command descriptor block
 ***************************************************************************/

static void set_sense_data(omti8621_state *state, UINT8 code, const UINT8 * cdb) {
	LOG2(("set_sense_data code=%x", code));
	state->sense_data[0]=code;
	state->sense_data[1]=cdb[1];
	state->sense_data[2]=cdb[2];
	state->sense_data[3]=cdb[3];
}

/***************************************************************************
 set_configuration_data - set the configuration data for drive lun
 ***************************************************************************/

static void set_configuration_data(omti8621_state *state, UINT8 lun) {
	LOG2(("set_configuration_data lun=%x", lun));

	// initialize the configuration data
	disk_data *disk = state->disk[lun];

	disk->config_data[0] = (disk->cylinders - 1) >> 8; // Number of Cylinders (MSB)
	disk->config_data[1] = (disk->cylinders - 1) & 0xff; // Number of Cylinders (LSB) (-1)
	disk->config_data[2] = disk->heads - 1; // Number of Heads (-1)
	disk->config_data[3] = disk->sectors - 1; // Number of Sectors (-1)
	disk->config_data[4] = 0x02; // Drive Configuration Word (MSB)
	disk->config_data[5] = 0x44; // Drive Configuration Word (LSB)
	disk->config_data[6] = 0x00; // ISG AFTER INDEX
	disk->config_data[7] = 0x00; // PLO SYN Field (ID)
	disk->config_data[8] = 0x00; // PLO SYN Field (DATA)
	disk->config_data[9] = 0x00; // ISG AFTER SECTOR
}

/***************************************************************************
 omti_reset - reset the OMTI controller
 ***************************************************************************/

static void omti_reset(omti8621_state *state) {
	LOG2(("omti_reset"));

	// should go from reset to idle after 100 us
	// state->omti_state = OMTI_STATE_RESET;
	state->omti_state = OMTI_STATE_IDLE;

	state->status_port =  OMTI_STATUS_NU6 | OMTI_STATUS_NU7;
	state->config_port = ~state->jumper;
	state->mask_port = 0;

	// default the sector data buffer with model and status information
	// (i.e. set sector data buffer for cmd=0x0e READ SECTOR BUFFER)

	memset(state->sector_buffer, 0, OMTI_DISK_SECTOR_SIZE);
	memcpy(state->sector_buffer, "8621VB.4060487xx", 0x10);
	state->sector_buffer[0x10] = 0; // ROM Checksum error
	state->sector_buffer[0x11] = 0; // Processor Register error
	state->sector_buffer[0x12] = 0; // Buffer RAM error
	state->sector_buffer[0x13] = 0; // Sequencer Register File error
	state->sector_buffer[0x14] = 0xc0; // 32K buffer size
	// TODO: add missing Default values for LUN 0, 1 and 3

	state->command_length = 0;
	state->command_index = 0;
	state->command_status = 0;

	state->data_index = 0;
	state->data_length = 0;

	clear_sense_data(state) ;

	state->diskaddr_ecc_error = 0;
	state->diskaddr_format_bad_track = 0;
	state->alternate_track_address[0] = 0;
	state->alternate_track_address[1] = 0;
}

/***************************************************************************
 get_lun - get logical unit number from a command descriptor block (in bit 5)
 ***************************************************************************/

static UINT8 get_lun(const UINT8 * cdb)
{
	return   (cdb[1] & 0x20) >> 5;
}

/***************************************************************************
 check_disk_address - check disk address, set sense data and return true for no error
 ***************************************************************************/

static UINT8 check_disk_address(omti8621_state *state, const UINT8 *cdb)
{
	UINT8 sense_code = OMTI_SENSE_CODE_NO_ERROR;
	UINT8 lun = get_lun(cdb);
	UINT16 head = cdb[1] & 0x1f;
	UINT16 sector = cdb[2] & 0x3f;
	UINT32 cylinder = cdb[3] + ((cdb[2] & 0xc0) << 2) + ((cdb[1] & 0x80) << 3);
	UINT8 block_count = cdb[4];
	disk_data *disk = state->disk[lun];

	UINT32 disk_track = cylinder * disk->heads + head;
	UINT32 disk_addr = (disk_track * disk->sectors) + sector;

	if (block_count > OMTI_MAX_BLOCK_COUNT) {
		LOG(("########### check_disk_address: unexpected block count %x", block_count));
		sense_code = OMTI_SENSE_CODE_ILLEGAL_ADDRESS | OMTI_SENSE_CODE_ADDRESS_VALID;
	}

	if (lun > OMTI_MAX_LUN) {
		sense_code = OMTI_SENSE_CODE_DRIVE_NOT_READY;
	} else	if (!disk->image->exists()) {
		sense_code = OMTI_SENSE_CODE_DRIVE_NOT_READY;
	} else	if (sector >= OMTI_MAX_BLOCK_COUNT) {
		sense_code = OMTI_SENSE_CODE_ILLEGAL_ADDRESS | OMTI_SENSE_CODE_ADDRESS_VALID;
	} else if (head >= disk->heads) {
		sense_code = OMTI_SENSE_CODE_ILLEGAL_ADDRESS | OMTI_SENSE_CODE_ADDRESS_VALID;
	} else if (cylinder >= disk->cylinders) {
		sense_code = OMTI_SENSE_CODE_ILLEGAL_ADDRESS | OMTI_SENSE_CODE_ADDRESS_VALID;
	} else if ( disk_track == state->diskaddr_format_bad_track && disk_track != 0) {
		sense_code = OMTI_SENSE_CODE_BAD_TRACK;
	} else if (disk_addr == state->diskaddr_ecc_error && disk_addr != 0) {
		sense_code = OMTI_SENSE_CODE_ECC_ERROR;
	} else if (disk_track == state->alternate_track_address[1] && disk_track != 0) {
		sense_code = OMTI_SENSE_CODE_ALTERNATE_TRACK;
	}

	if (sense_code == OMTI_SENSE_CODE_NO_ERROR) {
		clear_sense_data(state);
	} else {
		state->command_status |= OMTI_COMMAND_STATUS_ERROR;
		set_sense_data(state, sense_code, cdb);
	}
	return sense_code == OMTI_SENSE_CODE_NO_ERROR;
}

/***************************************************************************
 get_disk_track - get disk track from a command descriptor block
 ***************************************************************************/

static UINT32 get_disk_track(const omti8621_state *state, const UINT8 * cdb) {
	UINT8 lun = get_lun(cdb);
	UINT16 head = cdb[1] & 0x1f;
	UINT32 cylinder = cdb[3] + ((cdb[2] & 0xc0) << 2) + ((cdb[1] & 0x80) << 3);
	return cylinder * state->disk[lun]->heads + head;
}

/***************************************************************************
 get_disk_address - get disk address from a command descriptor block
 ***************************************************************************/

static UINT32 get_disk_address(const omti8621_state *state, const UINT8 * cdb) {
	UINT8 lun = get_lun(cdb);
	UINT16 sector = cdb[2] & 0x3f;
	return get_disk_track(state, cdb) * state->disk[lun]->sectors + sector;
}

/***************************************************************************
 set_data_transfer - setup for data transfer from/to data
 ***************************************************************************/

static void set_data_transfer(omti8621_state *state, UINT8 *data, UINT16 length) {
	// set controller for read data transfer
	state->omti_state = OMTI_STATE_DATA;
	state->status_port |= OMTI_STATUS_REQ | OMTI_STATUS_IO | OMTI_STATUS_BUSY;
	state->status_port &= ~OMTI_STATUS_CD;

	state->data_buffer = data;
	state->data_length = length;
	state->data_index = 0;
}

/***************************************************************************
 read_sectors_from_disk - read sectors starting at diskaddr into sector_buffer
 ***************************************************************************/

static void read_sectors_from_disk(omti8621_state *state, INT32 diskaddr,
		UINT8 count, UINT8 lun) {

	UINT8 *data_buffer = state->sector_buffer;
	device_image_interface *image = state->disk[lun]->image;

	while (count-- > 0) {
		LOG2(("read_sectors_from_disk lun=%d diskaddr=%x", lun, diskaddr));

		image->fseek( diskaddr * OMTI_DISK_SECTOR_SIZE, SEEK_SET);
		image->fread( data_buffer, OMTI_DISK_SECTOR_SIZE);

		diskaddr++;
		data_buffer += OMTI_DISK_SECTOR_SIZE;
	}
}

/***************************************************************************
 write_sectors_to_disk - write sectors starting at diskaddr from sector_buffer
 ***************************************************************************/

static void write_sectors_to_disk(omti8621_state *state, INT32 diskaddr,
		UINT8 count, UINT8 lun) {

	UINT8 *data_buffer = state->sector_buffer;
	device_image_interface *image = state->disk[lun]->image;

	while (count-- > 0) {
		LOG2(("write_sectors_to_disk lun=%d diskaddr=%x", lun, diskaddr));

		image->fseek( diskaddr * OMTI_DISK_SECTOR_SIZE, SEEK_SET);
		image->fwrite( data_buffer, OMTI_DISK_SECTOR_SIZE);

		if (diskaddr == state->diskaddr_ecc_error) {
			// reset previous ECC error
			state->diskaddr_ecc_error = 0;
		}

		diskaddr++;
		data_buffer += OMTI_DISK_SECTOR_SIZE;
	}
}

/***************************************************************************
 copy_sectors - copy sectors
 ***************************************************************************/

static void copy_sectors(omti8621_state *state, INT32 dst_addr, INT32 src_addr,
		UINT8 count, UINT8 lun) {
	device_image_interface *image = state->disk[lun]->image;

	LOG2(("copy_sectors lun=%d src_addr=%x dst_addr=%x count=%x", lun, src_addr, dst_addr, count));

	while (count-- > 0) {
		image->fseek( src_addr * OMTI_DISK_SECTOR_SIZE, SEEK_SET);
		image->fread( state->sector_buffer, OMTI_DISK_SECTOR_SIZE);

		image->fseek( dst_addr * OMTI_DISK_SECTOR_SIZE, SEEK_SET);
		image->fwrite( state->sector_buffer, OMTI_DISK_SECTOR_SIZE);

		if (dst_addr == state->diskaddr_ecc_error) {
			// reset previous ECC error
			state->diskaddr_ecc_error = 0;
		}

		src_addr++;
		dst_addr++;
	}
}

/***************************************************************************
 format track - format a track
 ***************************************************************************/

static void format_track(omti8621_state *state, const UINT8 * cdb) {
	UINT8 lun = get_lun(cdb);
	UINT32 disk_addr = get_disk_address(state, cdb);
	UINT32 disk_track = get_disk_track(state, cdb);

	if (state->diskaddr_ecc_error == disk_addr) {
		// reset previous ECC error
		state->diskaddr_ecc_error = 0;
	}

	if (state->diskaddr_format_bad_track == disk_track) {
		// reset previous bad track formatting
		state->diskaddr_format_bad_track = 0;
	}

	if (state->alternate_track_address[0] == disk_track) {
		// reset source of alternate track address
		state->alternate_track_address[0] = 0;
	}

	if (state->alternate_track_address[1] == disk_track) {
		// reset alternate track address
		state->alternate_track_address[1] = 0;
	}

	if (check_disk_address(state, cdb) ) {
		if ((cdb[5] & 0x40) == 0) {
			memset(state->sector_buffer, 0x6C, OMTI_DISK_SECTOR_SIZE * state->disk[lun]->sectors);
		}
		write_sectors_to_disk(state, disk_addr, state->disk[lun]->sectors, lun);
	}

}

/***************************************************************************
 set_esdi_defect_list - setup the (emty) ESDI defect list
 ***************************************************************************/

static void set_esdi_defect_list(omti8621_state *state, UINT8 lun, UINT8 head)
{
	disk_data *disk = state->disk[lun];

	memset(disk->esdi_defect_list, 0, sizeof(disk->esdi_defect_list));
	disk->esdi_defect_list[0] = 1; // month
	disk->esdi_defect_list[1] = 1; // day
	disk->esdi_defect_list[2] = 90; // year
	disk->esdi_defect_list[3] = head;
	memset(disk->esdi_defect_list+6, 0xff, 5); // end of defect list
}

/***************************************************************************
 log_command - log command from a command descriptor block
 ***************************************************************************/

static void log_command(const omti8621_state *state,
		const UINT8 cdb[], const UINT16 cdb_length) {
	if (verbose > 0) {
		int i;
		logerror("%s: OMTI command ", cpu_context(state->device));
		switch (cdb[0]) {
		case OMTI_CMD_TEST_DRIVE_READY: // 0x00
			logerror("Test Drive Ready");
			break;
		case OMTI_CMD_RECALIBRATE: // 0x01
			logerror("Recalibrate");
			break;
		case OMTI_CMD_REQUEST_SENSE: // 0x03
			logerror("Request Sense");
			break;
		case OMTI_CMD_READ_VERIFY: // 0x05
			logerror("Read Verify");
			break;
		case OMTI_CMD_FORMAT_TRACK: // 0x06
			logerror("Format Track");
			break;
		case OMTI_CMD_FORMAT_BAD_TRACK: // 0x07
			logerror("Format Bad Track");
			break;
		case OMTI_CMD_READ: // 0x08
			logerror("Read");
			break;
		case OMTI_CMD_WRITE: // 0x0A
			logerror("Write");
			break;
		case OMTI_CMD_SEEK: // 0x0B
			logerror("Seek");
			break;
		case OMTI_CMD_READ_SECTOR_BUFFER: // 0x0E
			logerror("Read Sector Buffer");
			break;
		case OMTI_CMD_WRITE_SECTOR_BUFFER: // 0x0F
			logerror("Write Sector Buffer");
			break;
		case OMTI_CMD_ASSIGN_ALTERNATE_TRACK: // 0x11
			logerror("Assign Alternate Track");
			break;
		case OMTI_CMD_READ_DATA_TO_BUFFER: // 0x1E
			logerror("Read Data to Buffer");
			break;
		case OMTI_CMD_WRITE_DATA_FROM_BUFFER: // 0x1F
			logerror("Write Data from Buffer");
			break;
		case OMTI_CMD_COPY: // 0x20
			logerror("Copy");
			break;
		case OMTI_CMD_READ_ESDI_DEFECT_LIST: // 0x37
			logerror("Read ESDI Defect List");
			break;
		case OMTI_CMD_RAM_DIAGNOSTICS: // 0xE0
			logerror("RAM. Diagnostic");
			break;
		case OMTI_CMD_CONTROLLER_INT_DIAGNOSTIC: // 0xE4
			logerror("Controller Int. Diagnostic");
			break;
		case OMTI_CMD_READ_LONG: // 0xE5
			logerror("Read Long");
			break;
		case OMTI_CMD_WRITE_LONG: // 0xE6
			logerror("Write Long");
			break;
		case OMTI_CMD_READ_CONFIGURATION: // 0xEC
			logerror("Read Configuration");
			break;
		case OMTI_CMD_INVALID_COMMAND: // 0xFF
			logerror("Invalid Command");
			break;
		default:
			logerror("!!! Unexpected Command !!!");
		}
//      logerror(" (%02x, length=%02x)", cdb[0], cdb_length);
		for (i = 0; i < cdb_length; i++) {
			logerror(" %02x", cdb[i]);
		}

		switch (cdb[0]) {
		case OMTI_CMD_READ_VERIFY: // 0x05
		case OMTI_CMD_READ: // 0x08
		case OMTI_CMD_WRITE: // 0x0a
		case OMTI_CMD_SEEK: // 0x0b
		case OMTI_CMD_READ_DATA_TO_BUFFER: // 0x1E
		case OMTI_CMD_WRITE_DATA_FROM_BUFFER: // 0x1F
		case OMTI_CMD_COPY: // 0x20
			logerror(" (diskaddr=%x count=%x)", get_disk_address(state, cdb), cdb[4]);
			break;
		}
		logerror("\n");
	}
}

/***************************************************************************
 log_data - log data in the common data buffer
 ***************************************************************************/

static void log_data(const omti8621_state *state) {
	if (verbose > 0) {
		int i;
		logerror("%s: OMTI data (length=%02x)", cpu_context(state->device),
				state->data_length);
		for (i = 0; i < state->data_length && i < OMTI_DISK_SECTOR_SIZE; i++) {
			logerror(" %02x", state->data_buffer[i]);
		}

		if (i < state->data_length) {
			logerror(" ...");
		}
		logerror("\n");
	}
}

/***************************************************************************
 do_command
 ***************************************************************************/

static void do_command(omti8621_state *state,
		const UINT8 cdb[], const UINT16 cdb_length) {

	UINT8 lun = get_lun(cdb);
	disk_data *disk = state->disk[lun];
	int command_duration = 0; // ms

	log_command( state, cdb, cdb_length);

	// default to read status and status is successful completion
	state->omti_state = OMTI_STATE_STATUS;
	state->status_port |= OMTI_STATUS_IO | OMTI_STATUS_CD;
	state->command_status = lun ? OMTI_COMMAND_STATUS_LUN : 0;

	if (state->mask_port & OMTI_MASK_INTE) {
		set_interrupt(state, CLEAR_LINE);
	}

	if (!disk->image->exists()) {
		state->command_status |= OMTI_COMMAND_STATUS_ERROR; // no such drive
	}

	switch (cdb[0]) {
	case OMTI_CMD_TEST_DRIVE_READY: // 0x00
		if (!disk->image->exists())
		{
			set_sense_data(state, OMTI_SENSE_CODE_DRIVE_NOT_READY, cdb);
		}
		break;

	case OMTI_CMD_RECALIBRATE: // 0x01
		break;

	case OMTI_CMD_REQUEST_SENSE: // 0x03
		set_data_transfer(state, state->sense_data, sizeof(state->sense_data));
		break;

	case OMTI_CMD_READ_VERIFY: // 0x05
		check_disk_address(state, cdb);
		break;

	case OMTI_CMD_FORMAT_TRACK: // 0x06
		format_track(state, cdb);
		break;

	case OMTI_CMD_FORMAT_BAD_TRACK: // 0x07
		state->diskaddr_format_bad_track = get_disk_address(state, cdb);
		break;

	case OMTI_CMD_READ: // 0x08
		if (check_disk_address(state, cdb)) {
			// read data from controller
			read_sectors_from_disk(state, get_disk_address(state, cdb), cdb[4], lun);
			set_data_transfer(state, state->sector_buffer,	OMTI_DISK_SECTOR_SIZE*cdb[4]);
		}
		break;

	case OMTI_CMD_WRITE: // 0x0A
		log_data(state);
		if (check_disk_address(state, cdb)) {
			write_sectors_to_disk(state, get_disk_address(state, cdb), cdb[4], lun);
		}
		break;

	case OMTI_CMD_SEEK: // 0x0B
		check_disk_address(state, cdb);
		break;

	case OMTI_CMD_READ_SECTOR_BUFFER: // 0x0E
		set_data_transfer(state, state->sector_buffer, OMTI_DISK_SECTOR_SIZE*cdb[4]);
		break;

	case OMTI_CMD_WRITE_SECTOR_BUFFER: // 0x0F
		log_data(state);
		break;

	case OMTI_CMD_COPY: // 0x20
		if (check_disk_address(state, cdb) && check_disk_address(state, cdb+4)) {
			// copy sectors
			copy_sectors (state, get_disk_address(state,  cdb+4), get_disk_address(state, cdb), cdb[4], lun);
		}
		break;

	case OMTI_CMD_READ_ESDI_DEFECT_LIST: // 0x37
		set_esdi_defect_list(state, get_lun(cdb), cdb[1] & 0x1f);
		set_data_transfer(state, disk->esdi_defect_list, sizeof(disk->esdi_defect_list));
		break;

	case OMTI_CMD_ASSIGN_ALTERNATE_TRACK: // 0x11
		log_data(state);
		state->alternate_track_address[0] = get_disk_track(state, cdb);
		state->alternate_track_address[1] = get_disk_track(state, state->alternate_track_buffer-1);;
		break;

	case OMTI_CMD_READ_DATA_TO_BUFFER: // 0x1E
		if (check_disk_address(state, cdb)) {
			// read data from controller
			read_sectors_from_disk (state, get_disk_address(state, cdb), cdb[4], lun);
			// Domain/OS doesn't expect zero access time
			command_duration += 1; // 1 ms is enough, average time would be 30 ms)
		}
		break;

	case OMTI_CMD_WRITE_DATA_FROM_BUFFER: // 0x1F
		log_data(state);
		if (check_disk_address(state, cdb)) {
			write_sectors_to_disk(state, get_disk_address(state, cdb), cdb[4], lun);
		}
		break;

	case  OMTI_CMD_RAM_DIAGNOSTICS: // 0xE0
		break;

	case OMTI_CMD_CONTROLLER_INT_DIAGNOSTIC: // 0xE4
		break;

	case OMTI_CMD_READ_LONG: // 0xE5
		if (check_disk_address(state, cdb)) {
			// read data from controller
			read_sectors_from_disk(state, get_disk_address(state, cdb), cdb[4], lun);
			set_data_transfer(state, state->sector_buffer, OMTI_DISK_SECTOR_SIZE+6);
		}
		break;

	case OMTI_CMD_WRITE_LONG: // 0xE6
		log_data(state);
		if (check_disk_address(state, cdb)) {
			UINT32 diskaddr =  get_disk_address(state, cdb);
			write_sectors_to_disk(state, diskaddr, cdb[4], lun);
			// this will spoil the ECC code
			state->diskaddr_ecc_error = diskaddr;
		}
		break;

	case OMTI_CMD_READ_CONFIGURATION: // 0xEC
		set_configuration_data(state, get_lun(cdb));
		set_data_transfer(state, disk->config_data, sizeof(disk->config_data));
		break;

	case OMTI_CMD_INVALID_COMMAND: // 0xFF
		set_sense_data(state, OMTI_SENSE_CODE_INVALID_COMMAND, cdb);
		state->command_status |= OMTI_COMMAND_STATUS_ERROR;
		break;

	default:
		LOG(("do_command: UNEXPECTED command %02x",cdb[0]));
		set_sense_data(state, OMTI_SENSE_CODE_INVALID_COMMAND, cdb);
		state->command_status |= OMTI_COMMAND_STATUS_ERROR;
		break;
	}

	if (state->mask_port & OMTI_MASK_INTE) {
//      if (state->omti_state != OMTI_STATE_STATUS) {
//          LOG(("do_command: UNEXPECTED omti_state %02x",state->omti_state));
//      }
		state->status_port |= OMTI_STATUS_IREQ;
		if (command_duration == 0) {
			set_interrupt(state, ASSERT_LINE);
		} else {
			// FIXME: should delay state->omti_state and state->status_port as well
			state->device->machine().scheduler().timer_set(attotime::from_msec(command_duration), FUNC(set_interrupt_caba), 0, state);
		}
	}
}

/***************************************************************************
 get_command_length
 ***************************************************************************/

static UINT8 get_command_length(UINT8 command_byte) {
	return command_byte == OMTI_CMD_COPY ? 10 : 6;
}

/***************************************************************************
 get_data
 ***************************************************************************/

static UINT16 get_data(omti8621_state *state) {
	UINT16 data = 0xff;
	if (state->data_index < state->data_length) {
		data = state->data_buffer[state->data_index++] << 8;
		data |= state->data_buffer[state->data_index++];
		if (state->data_index >= state->data_length) {
			state->omti_state = OMTI_STATE_STATUS;
			state->status_port |= OMTI_STATUS_IO | OMTI_STATUS_CD;
			log_data(state);
		}
	} else {
		LOG(("UNEXPECTED reading OMTI 8621 data (buffer length exceeded)"));
	}
    return data;
}

/***************************************************************************
 set_data
 ***************************************************************************/

static void set_data(omti8621_state *state, UINT16 data) {
	if (state->data_index < state->data_length) {
		state->data_buffer[state->data_index++] = data >> 8;
		state->data_buffer[state->data_index++] = data & 0xff;
		if (state->data_index >= state->data_length) {
			do_command(state, state->command_buffer, state->command_index);
		}
	} else {
		LOG(("UNEXPECTED writing OMTI 8621 data (buffer length exceeded)"));
	}
}

/***************************************************************************
 DN3500 Disk Controller-AT Register at 0x4D000
 ***************************************************************************/

static WRITE8_DEVICE_HANDLER( omti8621_w8 ) {
	omti8621_state *state = get_safe_token(device);

	switch (offset) {
	case OMTI_PORT_DATA_OUT: //  0x00
		switch (state->omti_state) {
		case OMTI_STATE_COMMAND:
			LOG2(("writing OMTI 8621 Command Register at offset %02x = %02x", offset, data));
			if (state->command_index == 0) {
				state->command_length = get_command_length(data);
			}

			if (state->command_index < state->command_length) {
				state->command_buffer[state->command_index++] = data;
			} else {
				LOG(("UNEXPECTED writing OMTI 8621 Data Register at offset %02x = %02x (command length exceeded)", offset, data));
			}

			if (state->command_index == state->command_length) {
				switch (state->command_buffer[0]) {
				case OMTI_CMD_WRITE: // 0x0A
					// TODO: check diskaddr
					// Fall through
				case OMTI_CMD_WRITE_SECTOR_BUFFER: // 0x0F
					set_data_transfer(state, state->sector_buffer,
							OMTI_DISK_SECTOR_SIZE * state->command_buffer[4]);
					state->status_port &= ~OMTI_STATUS_IO;
					break;

				case OMTI_CMD_ASSIGN_ALTERNATE_TRACK: // 0x11
					set_data_transfer(state, state->alternate_track_buffer, sizeof(state->alternate_track_buffer));
					state->status_port &= ~OMTI_STATUS_IO;
					break;

				case OMTI_CMD_WRITE_LONG: // 0xE6
					// TODO: check diskaddr
					set_data_transfer(state, state->sector_buffer,
							(OMTI_DISK_SECTOR_SIZE +6) * state->command_buffer[4]);
					state->status_port &= ~OMTI_STATUS_IO;
					break;

				default:
					do_command(state, state->command_buffer, state->command_index);
					break;
				}
			}
			break;

		case OMTI_STATE_DATA:
			LOG(("UNEXPECTED: writing OMTI 8621 Data Register at offset %02x = %02x", offset, data));
			break;

		default:
			LOG(("UNEXPECTED writing OMTI 8621 Data Register at offset %02x = %02x (omti state = %02x)", offset, data, state->omti_state));
			break;
		}
		break;

	case OMTI_PORT_RESET: // 0x01
		LOG2(("writing OMTI 8621 Reset Register at offset %02x = %02x", offset, data));
		omti_reset(state);
		break;

	case OMTI_PORT_SELECT: // 0x02
		LOG2(("writing OMTI 8621 Select Register at offset %02x = %02x (omti state = %02x)", offset, data, state->omti_state));
		state->omti_state = OMTI_STATE_COMMAND;

		state->status_port |= OMTI_STATUS_BUSY | OMTI_STATUS_REQ | OMTI_STATUS_CD;
		state->status_port &= ~OMTI_STATUS_IO;

		state->command_status = 0;
		state->command_index = 0;
		break;

	case OMTI_PORT_MASK: // 0x03
		LOG2(("writing OMTI 8621 Mask Register at offset %02x = %02x", offset, data));
		state->mask_port = data;

		if ((data & OMTI_MASK_INTE) == 0) {
			state->status_port &= ~OMTI_STATUS_IREQ;
			set_interrupt(state, CLEAR_LINE);
		}

		if ((data & OMTI_MASK_DMAE) == 0) {
			state->status_port &= ~OMTI_STATUS_DREQ;
		}
		break;

	default:
		LOG(("UNEXPECTED writing OMTI 8621 Register at offset %02x = %02x", offset, data));
		break;
	}
}

static READ8_DEVICE_HANDLER( omti8621_r8 ) {
	omti8621_state *state = get_safe_token(device);
	UINT8 data = 0xff;

	static UINT8 last_data = 0xff;

	switch (offset) {
	case OMTI_PORT_DATA_IN: // 0x00
		if (state->status_port & OMTI_STATUS_CD) {
			data = state->command_status;
			switch (state->omti_state) {
			case OMTI_STATE_COMMAND:
				LOG2(("reading OMTI 8621 Data Status Register at offset %02x = %02x (omti state = %02x)", offset, data, state->omti_state));
				break;
			case OMTI_STATE_STATUS:
				state->omti_state = OMTI_STATE_IDLE;
				state->status_port &= ~(OMTI_STATUS_BUSY | OMTI_STATUS_CD  | OMTI_STATUS_IO | OMTI_STATUS_REQ);
				LOG2(("reading OMTI 8621 Data Status Register at offset %02x = %02x", offset, data));
				break;
			default:
				LOG(("UNEXPECTED reading OMTI 8621 Data Status Register at offset %02x = %02x (omti state = %02x)", offset, data, state->omti_state));
				break;
			}
		} else {
			LOG(("UNEXPECTED reading OMTI 8621 Data Register at offset %02x = %02x (status bit C/D = 0)", offset, data));
		}
		break;

	case OMTI_PORT_STATUS: // 0x01
		data = state->status_port;
		// omit excessive logging
		if (data != last_data) {
			LOG2(("reading OMTI 8621 Status Register at offset %02x = %02x", offset, data));
//          last_data = data;
		}
		break;

	case OMTI_PORT_CONFIG: // 0x02
		data = state->config_port;
		LOG2(("reading OMTI 8621 Configuration Register at offset %02x = %02x", offset, data));
		break;

	case OMTI_PORT_MASK: // 0x03
		data = state->mask_port ;
		// win.dex will update the mask register with read-modify-write
		// LOG2(("reading OMTI 8621 Mask Register at offset %02x = %02x (UNEXPECTED!)", offset, data));
		break;

	default:
		LOG(("UNEXPECTED reading OMTI 8621 Register at offset %02x = %02x", offset, data));
		break;
	}

	return data;
}

WRITE16_DEVICE_HANDLER(omti8621_w ) {
	omti8621_state *state = get_safe_token(device);

	switch (mem_mask) {
	case 0x00ff:
		omti8621_w8(device, offset*2+1, data);
		break;
	case 0xff00:
		omti8621_w8(device, offset*2, data>>8);
		break;
	default:
		LOG3(("writing OMTI 8621 Data Word Register to %0x = %04x & %04x", offset, data, mem_mask));
		set_data(state, data);
		break;
	}
}

READ16_DEVICE_HANDLER( omti8621_r ) {
	omti8621_state *state = get_safe_token(device);
	UINT16 data;
	switch (mem_mask) {
	case 0x00ff:
		data = omti8621_r8(device, offset*2+1);
		break;
	case 0xff00:
		data = omti8621_r8(device, offset*2) << 8;
		break;
	default:
		data = get_data(state);
		LOG3(("reading OMTI 8621 Data Word Register from %0x = %04x & %04x", offset, data, mem_mask));
		break;
	}
	return data;
}

void omti8621_set_verbose(int on_off)
{
	verbose = on_off == 0 ? 0 : VERBOSE > 1 ? VERBOSE : 1;
}

/***************************************************************************
 omti8621_get_sector - get sector diskaddr of logical unit lun into data_buffer
 ***************************************************************************/

UINT32 omti8621_get_sector(device_t *device, INT32 diskaddr, UINT8 *data_buffer, UINT32 length, UINT8 lun)
{
	omti8621_state *state = get_safe_token(device);
	disk_data *disk = state->disk[lun];

	if (disk->image == NULL || !disk->image->exists())
	{
		return 0;
	}
	else
	{
		LOG1(("omti8621_get_sector %x on lun %d", diskaddr, lun));

		// restrict length to size of 1 sector (i.e. 1024 Byte)
		length = length < OMTI_DISK_SECTOR_SIZE ? length  : OMTI_DISK_SECTOR_SIZE;

		disk->image->fseek(diskaddr * OMTI_DISK_SECTOR_SIZE, SEEK_SET);
		disk->image->fread(data_buffer, length);

		return length;
	}
}


/***************************************************************************
 omti_set_jumper - set OMI jumpers
 ***************************************************************************/

static void omti_set_jumper(omti8621_state *state, UINT16 disk_type)
{
	LOG1(("omti_set_jumper: disk type=%x", disk_type));

	switch (disk_type)
	{
	case OMTI_DISK_TYPE_348_MB: // Maxtor 380 MB (348-MB FA formatted)
		state->jumper = OMTI_CONFIG_W22 | OMTI_CONFIG_W23;
		break;

	case OMTI_DISK_TYPE_155_MB: // Micropolis 170 MB (155-MB formatted)
	default:
		state->jumper = OMTI_CONFIG_W20;
		break;
	}
}


//##########################################################################
class omti_disk_image_device :	public device_t,
								public device_image_interface
{
public:
	// construction/destruction
	omti_disk_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// image-level overrides
	virtual iodevice_t image_type() const { return IO_HARDDISK; }

	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 1; }
	virtual bool is_creatable() const { return 1; }
	virtual bool must_be_loaded() const { return 0; }
	virtual bool is_reset_on_load() const { return 0; }
	virtual const char *image_interface() const { return NULL; }
	virtual const char *file_extensions() const { return "awd"; }
	virtual const option_guide *create_option_guide() const { return NULL; }

	virtual bool call_create(int format_type, option_resolution *format_options);

	disk_data *token() { return &m_token; }
protected:
	// device-level overrides
    virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	disk_data m_token;
};

// device type definition
extern const device_type OMTI_DISK;

const device_type OMTI_DISK = &device_creator<omti_disk_image_device>;

omti_disk_image_device::omti_disk_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, OMTI_DISK, "Winchester", tag, owner, clock),
	  device_image_interface(mconfig, *this)
{
}

void omti_disk_image_device::device_config_complete()
{
	update_names(OMTI_DISK, "disk", "disk");
};


/***************************************************************************
 get_safe_disk_token - makes sure that the passed in device is a OMTI disk
 ***************************************************************************/

INLINE disk_data *get_safe_disk_token(device_t *device) {
	assert(device != NULL);
	assert(device->type() == OMTI_DISK);
	return (disk_data *) downcast<omti_disk_image_device *>(device)->token();
}

/***************************************************************************
 omti_disk_config - configure disk parameters
 ***************************************************************************/

static void omti_disk_config(disk_data *disk, UINT16 disk_type)
{
	DLOG1(("omti_disk_config: configuring disk with type %x", disk_type));

	switch (disk_type)
	{
	case OMTI_DISK_TYPE_348_MB: // Maxtor 380 MB (348-MB FA formatted)
		disk->cylinders = 1223;
		disk->heads = 15;
		disk->sectors = 18;
		break;

	case OMTI_DISK_TYPE_155_MB: // Micropolis 170 MB (155-MB formatted)
	default:
		disk->cylinders = 1023;
		disk->heads = 8;
		disk->sectors = 18;
		break;
	}

	disk->type = disk_type;
	disk->sectorbytes = OMTI_DISK_SECTOR_SIZE;
	disk->sector_count = disk->cylinders * disk->heads * disk->sectors;
}

/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

void omti_disk_image_device::device_start()
{
	disk_data *disk = get_safe_disk_token(this);

	disk->device = this;
	// note: we must have disk->device before we can log

	disk->image = this;

	if (disk->image->image_core_file() == NULL)
	{
		DLOG1(("device_start_omti_disk: no disk"));
	}
	else
	{
		DLOG1(("device_start_omti_disk: with disk image %s",disk->image->basename() ));
	}

	// default disk type
	omti_disk_config(disk, OMTI_DISK_TYPE_DEFAULT);
}

/*-------------------------------------------------
    device reset callback
-------------------------------------------------*/

void omti_disk_image_device::device_reset()
{
	disk_data *disk = get_safe_disk_token(this);
	DLOG1(("device_reset_omti_disk"));

	if (exists() && fseek(0, SEEK_END) == 0)
	{
		UINT32 disk_size = (UINT32)(ftell() / OMTI_DISK_SECTOR_SIZE);
		UINT16 disk_type = disk_size >= 300000 ? OMTI_DISK_TYPE_348_MB : OMTI_DISK_TYPE_155_MB;
		if (disk_type != disk->type) {
			DLOG1(("device_reset_omti_disk: disk size=%d blocks, disk type=%x", disk_size, disk_type ));
			omti_disk_config(disk, disk_type);
		}
	}
}

/*-------------------------------------------------
   disk image create callback
-------------------------------------------------*/

bool omti_disk_image_device::call_create(int format_type, option_resolution *format_options)
{
	disk_data *disk = get_safe_disk_token(this);
	DLOG(("device_create_omti_disk: creating OMTI Disk with %d blocks", disk->sector_count));

	int x;
	unsigned char sectordata[OMTI_DISK_SECTOR_SIZE]; // empty block data


	memset(sectordata, 0x55, sizeof(sectordata));
	for (x = 0; x < disk->sector_count; x++)
	{
		if (fwrite(sectordata, OMTI_DISK_SECTOR_SIZE)
				< OMTI_DISK_SECTOR_SIZE)
		{
			return IMAGE_INIT_FAIL;
		}
	}
	return IMAGE_INIT_PASS;
}

MACHINE_CONFIG_FRAGMENT( omti_disk )
	MCFG_DEVICE_ADD(OMTI_DISK0_TAG, OMTI_DISK, 0)
	MCFG_DEVICE_ADD(OMTI_DISK1_TAG, OMTI_DISK, 0)
MACHINE_CONFIG_END

/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

static DEVICE_START( omti8621 ) {
	omti8621_state *state = get_safe_token(device);

	const omti8621_config *config =
			(const omti8621_config *) device->static_config();

	/* validate basic stuff */
	assert(device != NULL);
	assert(device->type() == OMTI8621);

	/* store a pointer back to the device */
	state->device = device;

	// note: we must have state->device before we can log
	LOG2(("device_start_omti8621"));

	state->irq_handler = config->set_irq;

	state->sector_buffer = auto_alloc_array(device->machine(), UINT8,
			OMTI_DISK_SECTOR_SIZE*OMTI_MAX_BLOCK_COUNT);
	assert(state->sector_buffer != NULL);

	device_t *device0 = device->machine().device(OMTI_DISK0_TAG);
	state->disk[0] = (disk_data *) downcast<omti_disk_image_device *>(device0)->token();

	device_t *device1 = device->machine().device(OMTI_DISK1_TAG);
	state->disk[1] = (disk_data *) downcast<omti_disk_image_device *>(device1)->token();
}

/*-------------------------------------------------
    device reset callback
-------------------------------------------------*/

static DEVICE_RESET( omti8621 )
{
	omti8621_state *state = get_safe_token(device);
	LOG2(("device_reset_omti8621"));

	omti_set_jumper(state, state->disk[0]->type);
	omti_reset(state);
}

/*-------------------------------------------------
    device get info callback
-------------------------------------------------*/

DEVICE_GET_INFO( omti8621 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(omti8621_state);		break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(omti8621); break;
		case DEVINFO_FCT_STOP:					/* Nothing */							break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(omti8621); break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "OMTI 8621");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Winchester Disk Controller");break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");					break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}

DEFINE_LEGACY_DEVICE(OMTI8621, omti8621);
