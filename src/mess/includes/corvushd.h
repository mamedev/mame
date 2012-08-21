/*****************************************************************************
 *
 * includes/corvushd.h
 *
 * Implementation of the Corvus Systems Flat Cable Hard Disk controller
 *
 ****************************************************************************/

#ifndef CORVUSHD_H_
#define CORVUSHD_H_


//
// Controller Commands
//

// Read/Write Commands

#define READ_SECTOR_256			0x02
#define WRITE_SECTOR_256		0x03
#define READ_CHUNK_128			0x12
#define READ_CHUNK_256			0x22
#define READ_CHUNK_512			0x32
#define WRITE_CHUNK_128			0x13
#define WRITE_CHUNK_256			0x23
#define WRITE_CHUNK_512			0x33

// Semaphore Commands

#define SEMAPHORE_LOCK_CODE		0x0b
#define SEMAPHORE_LOCK_MOD			0x01
#define SEMAPHORE_UNLOCK_CODE	0x0b
#define SEMAPHORE_UNLOCK_MOD		0x11
#define SEMAPHORE_INIT_CODE		0x1a
#define SEMAPHORE_INIT_MOD			0x10
#define SEMAPHORE_STATUS_CODE	0x1a
#define SEMAPHORE_STATUS_MOD		0x41

// Pipe Commands

#define PIPE_READ_CODE			0x1a
#define PIPE_READ_MOD				0x20
#define PIPE_WRITE_CODE			0x1a
#define PIPE_WRITE_MOD				0x21
#define PIPE_CLOSE_CODE			0x1a
#define PIPE_CLOSE_MOD				0x40
#define PIPE_STATUS_CODE		0x1a
#define PIPE_STATUS_MOD				0x41
#define PIPE_OPEN_WRITE_CODE	0x1b
#define PIPE_OPEN_WRITE_MOD			0x80
#define PIPE_AREA_INIT_CODE		0x1b
#define PIPE_AREA_INIT_MOD			0xa0
#define PIPE_OPEN_READ_CODE		0x1b
#define PIPE_OPEN_READ_MOD			0xc0

// Active User Table Commands

#define ADDACTIVE_CODE			0x34
#define ADDACTIVE_MOD				0x03
#define DELACTIVEUSR_REVBH_CODE	0x34
#define DELACTIVEUSR_REVBH_MOD		0x00
#define DELACTIVENUM_OMNI_CODE	0x34
#define DELACTIVENUM_OMNI_MOD		0x00
#define DELACTIVEUSR_OMNI_CODE	0x34
#define DELACTIVEUSR_OMNI_MOD		0x31
#define FINDACTIVE_CODE			0x34
#define FINDACTIVE_MOD				0x05
#define READTEMPBLOCK			0xc4
#define WRITETEMPBLOCK			0xb4

// Miscellaneous Commands

#define BOOT					0x14
#define READ_BOOT_BLOCK			0x44
#define GET_DRIVE_PARAMETERS	0x10
#define PARK_HEADS_REVH			0x11
#define PARK_HEADS_OMNI			0x80
#define ECHO					0xf4

// Put drive in Prep Mode

#define PREP_MODE_SELECT		0x11

// Prep Mode Commands

#define PREP_RESET_DRIVE		0x00
#define PREP_FORMAT_DRIVE		0x01
#define PREP_FILL_DRIVE_OMNI	0x81
#define PREP_VERIFY				0x07
#define PREP_READ_FIRMWARE		0x32
#define PREP_WRITE_FIRMWARE		0x33

//
// Controller Status Codes
//

// Disk status codes

#define STAT_SUCCESS			0x00

#define STAT_HEADER_FAULT		0x00
#define STAT_SEEK_TIMEOUT		0x01
#define STAT_SEEK_FAULT			0x02
#define STAT_SEEK_ERROR			0x03
#define STAT_HEADER_CRC_ERROR	0x04
#define STAT_REZERO_FAULT		0x05
#define STAT_REZERO_TIMEOUT		0x06
#define	STAT_DRIVE_NOT_ONLINE	0x07
#define	STAT_WRITE_FAULT		0x08
#define STAT_NOT_USED			0x09
#define STAT_READ_DATA_FAULT	0x0a
#define STAT_DATA_CRC_ERROR		0x0b
#define STAT_SECTOR_LOCATE_ERR	0x0c
#define STAT_WRITE_PROTECTED	0x0d
#define STAT_ILL_SECTOR_ADDRESS	0x0e
#define STAT_ILL_CMD_OP_CODE	0x0f
#define STAT_DRIVE_NOT_ACK		0x10
#define STAT_ACK_STUCK_ACTIVE	0x11
#define STAT_TIMEOUT			0x12
#define STAT_FAULT				0x13
#define STAT_CRC				0x14
#define STAT_SEEK				0x15
#define STAT_VERIFICATION		0x16
#define STAT_SPEED_ERROR		0x17
#define STAT_ILL_ADDRESS		0x18
#define STAT_RW_FAULT_ERROR		0x19
#define STAT_SERVO_ERROR		0x1a
#define STAT_GUARD_BAND			0x1b
#define STAT_PLO_ERROR			0x1c
#define STAT_RW_UNSAFE			0x1d

// Disk status modifiers (added to status code)

#define STAT_RECOVERABLE_ERR	0x20
#define STAT_VERIFY_ERR			0x40
#define STAT_FATAL_ERR			0x80

// Semaphore status codes

#define SEM_PRIOR_STATE_NOT_SET	0x00
#define SEM_PRIOR_STATE_SET		0x80
#define SEM_TABLE_FULL			0xfd
#define SEM_DISK_ERROR			0xfe

// Pipe Status codes

#define PIPE_REQ_SUCCESSFUL		0x00
#define PIPE_EMPTY_PIPE_READ	0x08
#define PIPE_NOT_OPEN			0x09
#define PIPE_WRITE_TO_FULL_PIPE	0x0a
#define PIPE_OPEN_OPEN_PIPE		0x0b
#define PIPE_PIPE_NOT_EXIST		0x0c
#define PIPE_NO_ROOM_FOR_NEW	0x0d
#define PIPE_ILLEGAL_COMMAND	0x0e
#define PIPE_AREA_NOT_INIT		0x0f

// Pipe State codes

#define PIPE_OPEN_WRITE_EMPTY	0x01
#define PIPE_OPEN_READ_EMPTY	0x02
#define PIPE_NOT_OPEN_FULL		0x80
#define PIPE_OPEN_WRITE_FULL	0x81
#define PIPE_OPEN_READ_FULL		0x82

// Status Register Bits

#define CONTROLLER_BUSY			0x80	// Set = Busy, Clear = Ready
#define CONTROLLER_DIRECTION	0x40	// Set = Controller->Host, Clear = Host->Controller


/*----------- defined in machine/corvushd.c -----------*/

//
// Prototypes
//
UINT8 corvus_hdc_init( running_machine &machine );
READ8_HANDLER ( corvus_hdc_status_r );
READ8_HANDLER ( corvus_hdc_data_r );
WRITE8_HANDLER ( corvus_hdc_data_w );


#endif /* CORVUSHD_H_ */
