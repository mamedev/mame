/***************************************************************************

    Generic (PC-style) IDE controller implementation

***************************************************************************/

#include "driver.h"
#include "idectrl.h"
#include "debugger.h"


/***************************************************************************
    DEBUGGING
***************************************************************************/

#define VERBOSE						0
#define PRINTF_IDE_COMMANDS			0
#define PRINTF_IDE_PASSWORD			0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)

#define LOGPRINT(x)	do { if (VERBOSE) logerror x; if (PRINTF_IDE_COMMANDS) mame_printf_debug x; } while (0)



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define IDE_DISK_SECTOR_SIZE			512

#define MINIMUM_COMMAND_TIME			(ATTOTIME_IN_USEC(10))

#define TIME_PER_SECTOR					(ATTOTIME_IN_USEC(100))
#define TIME_PER_ROTATION				(ATTOTIME_IN_HZ(5400/60))
#define TIME_SECURITY_ERROR				(ATTOTIME_IN_MSEC(1000))

#define TIME_SEEK_MULTISECTOR			(ATTOTIME_IN_MSEC(13))
#define TIME_NO_SEEK_MULTISECTOR		(ATTOTIME_IN_NSEC(16300))

#define IDE_STATUS_ERROR				0x01
#define IDE_STATUS_HIT_INDEX			0x02
#define IDE_STATUS_BUFFER_READY			0x08
#define IDE_STATUS_SEEK_COMPLETE		0x10
#define IDE_STATUS_DRIVE_READY			0x40
#define IDE_STATUS_BUSY					0x80

#define IDE_CONFIG_REGISTERS			0x10

#define IDE_ADDR_CONFIG_UNK				0x034
#define IDE_ADDR_CONFIG_REGISTER		0x038
#define IDE_ADDR_CONFIG_DATA			0x03c

#define IDE_ADDR_DATA					0x1f0
#define IDE_ADDR_ERROR					0x1f1
#define IDE_ADDR_SECTOR_COUNT			0x1f2
#define IDE_ADDR_SECTOR_NUMBER			0x1f3
#define IDE_ADDR_CYLINDER_LSB			0x1f4
#define IDE_ADDR_CYLINDER_MSB			0x1f5
#define IDE_ADDR_HEAD_NUMBER			0x1f6
#define IDE_ADDR_STATUS_COMMAND			0x1f7

#define IDE_ADDR_STATUS_CONTROL			0x3f6

#define IDE_COMMAND_READ_MULTIPLE		0x20
#define IDE_COMMAND_READ_MULTIPLE_ONCE	0x21
#define IDE_COMMAND_WRITE_MULTIPLE		0x30
#define IDE_COMMAND_SET_CONFIG			0x91
#define IDE_COMMAND_READ_MULTIPLE_BLOCK	0xc4
#define IDE_COMMAND_WRITE_MULTIPLE_BLOCK 0xc5
#define IDE_COMMAND_SET_BLOCK_COUNT		0xc6
#define IDE_COMMAND_READ_DMA			0xc8
#define IDE_COMMAND_WRITE_DMA			0xca
#define IDE_COMMAND_GET_INFO			0xec
#define IDE_COMMAND_SET_FEATURES		0xef
#define IDE_COMMAND_SECURITY_UNLOCK		0xf2
#define IDE_COMMAND_UNKNOWN_F9			0xf9
#define IDE_COMMAND_VERIFY_MULTIPLE		0x40
#define IDE_COMMAND_ATAPI_IDENTIFY		0xa1
#define IDE_COMMAND_RECALIBRATE			0x10
#define IDE_COMMAND_IDLE_IMMEDIATE		0xe1

#define IDE_ERROR_NONE					0x00
#define IDE_ERROR_DEFAULT				0x01
#define IDE_ERROR_UNKNOWN_COMMAND		0x04
#define IDE_ERROR_BAD_LOCATION			0x10
#define IDE_ERROR_BAD_SECTOR			0x80

#define IDE_BUSMASTER_STATUS_ACTIVE		0x01
#define IDE_BUSMASTER_STATUS_ERROR		0x02
#define IDE_BUSMASTER_STATUS_IRQ		0x04



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _ide_state ide_state;
struct _ide_state
{
	const device_config *device;

	UINT8			adapter_control;
	UINT8			status;
	UINT8			error;
	UINT8			command;
	UINT8			interrupt_pending;
	UINT8			precomp_offset;

	UINT8			buffer[IDE_DISK_SECTOR_SIZE];
	UINT8			features[IDE_DISK_SECTOR_SIZE];
	UINT16			buffer_offset;
	UINT16			sector_count;

	UINT16			block_count;
	UINT16			sectors_until_int;
	UINT8			verify_only;

	UINT8			dma_active;
	UINT8			dma_cpu;
	UINT8			dma_address_xor;
	UINT8			dma_last_buffer;
	offs_t			dma_address;
	offs_t			dma_descriptor;
	UINT32			dma_bytes_left;

	UINT8			bus_master_command;
	UINT8			bus_master_status;
	UINT32			bus_master_descriptor;

	UINT16			cur_cylinder;
	UINT8			cur_sector;
	UINT8			cur_head;
	UINT8			cur_head_reg;

	UINT32			cur_lba;

	UINT16			num_cylinders;
	UINT8			num_sectors;
	UINT8			num_heads;

	UINT8			config_unknown;
	UINT8			config_register[IDE_CONFIG_REGISTERS];
	UINT8			config_register_num;

	hard_disk_file *disk;
	emu_timer *		last_status_timer;
	emu_timer *		reset_timer;

	UINT8			master_password_enable;
	UINT8			user_password_enable;
	const UINT8 *	master_password;
	const UINT8 *	user_password;
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static TIMER_CALLBACK( reset_callback );

static void ide_build_features(ide_state *ide);

static void continue_read(ide_state *ide);
static void read_sector_done(ide_state *ide);
static TIMER_CALLBACK( read_sector_done_callback );
static void read_first_sector(ide_state *ide);
static void read_next_sector(ide_state *ide);

static UINT32 ide_controller_read(const device_config *device, offs_t offset, int size);
static void ide_controller_write(const device_config *device, offs_t offset, int size, UINT32 data);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    get_safe_token - makes sure that the passed
    in device is, in fact, an IDE controller
-------------------------------------------------*/

INLINE ide_state *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == IDE_CONTROLLER);

	return (ide_state *)device->token;
}


INLINE void signal_interrupt(ide_state *ide)
{
	const ide_config *config = ide->device->inline_config;

	LOG(("IDE interrupt assert\n"));

	/* signal an interrupt */
	if (config->interrupt != NULL)
		(*config->interrupt)(ide->device, ASSERT_LINE);
	ide->interrupt_pending = 1;
	ide->bus_master_status |= IDE_BUSMASTER_STATUS_IRQ;
}


INLINE void clear_interrupt(ide_state *ide)
{
	const ide_config *config = ide->device->inline_config;

	LOG(("IDE interrupt clear\n"));

	/* clear an interrupt */
	if (config->interrupt != NULL)
		(*config->interrupt)(ide->device, CLEAR_LINE);
	ide->interrupt_pending = 0;
}



/***************************************************************************
    DELAYED INTERRUPT HANDLING
***************************************************************************/

static TIMER_CALLBACK( delayed_interrupt )
{
	ide_state *ide = ptr;
	ide->status &= ~IDE_STATUS_BUSY;
	signal_interrupt(ide);
}


static TIMER_CALLBACK( delayed_interrupt_buffer_ready )
{
	ide_state *ide = ptr;
	ide->status &= ~IDE_STATUS_BUSY;
	ide->status |= IDE_STATUS_BUFFER_READY;
	signal_interrupt(ide);
}


INLINE void signal_delayed_interrupt(ide_state *ide, attotime time, int buffer_ready)
{
	/* clear buffer ready and set the busy flag */
	ide->status &= ~IDE_STATUS_BUFFER_READY;
	ide->status |= IDE_STATUS_BUSY;

	/* set a timer */
	if (buffer_ready)
		timer_set(time, ide, 0, delayed_interrupt_buffer_ready);
	else
		timer_set(time, ide, 0, delayed_interrupt);
}



/***************************************************************************
    INITIALIZATION AND RESET
***************************************************************************/

UINT8 *ide_get_features(const device_config *device)
{
	ide_state *ide = get_safe_token(device);
	return ide->features;
}


void ide_set_master_password(const device_config *device, const UINT8 *password)
{
	ide_state *ide = get_safe_token(device);

	ide->master_password = password;
	ide->master_password_enable = (ide->master_password != NULL);
}


void ide_set_user_password(const device_config *device, const UINT8 *password)
{
	ide_state *ide = get_safe_token(device);

	ide->user_password = password;
	ide->user_password_enable = (ide->user_password != NULL);
}


static TIMER_CALLBACK( reset_callback )
{
	device_reset(ptr);
}



/*************************************
 *
 *  Convert offset/mem_mask to offset
 *  and size
 *
 *************************************/

INLINE int convert_to_offset_and_size32(offs_t *offset, UINT32 mem_mask)
{
	int size = 4;

	/* determine which real offset */
	if (!ACCESSING_BITS_0_7)
	{
		(*offset)++, size = 3;
		if (!ACCESSING_BITS_8_15)
		{
			(*offset)++, size = 2;
			if (!ACCESSING_BITS_16_23)
				(*offset)++, size = 1;
		}
	}

	/* determine the real size */
	if (ACCESSING_BITS_24_31)
		return size;
	size--;
	if (ACCESSING_BITS_16_23)
		return size;
	size--;
	if (ACCESSING_BITS_8_15)
		return size;
	size--;
	return size;
}

INLINE int convert_to_offset_and_size16(offs_t *offset, UINT32 mem_mask)
{
	int size = 2;

	/* determine which real offset */
	if (!ACCESSING_BITS_0_7)
		(*offset)++, size = 1;

	if (ACCESSING_BITS_8_15)
		return size;
	size--;
	return size;
}



/*************************************
 *
 *  Compute the LBA address
 *
 *************************************/

INLINE UINT32 lba_address(ide_state *ide)
{
	/* LBA direct? */
	if (ide->cur_head_reg & 0x40)
		return ide->cur_sector + ide->cur_cylinder * 256 + ide->cur_head * 16777216;

	/* standard CHS */
	else
		return (ide->cur_cylinder * ide->num_heads + ide->cur_head) * ide->num_sectors + ide->cur_sector - 1;
}



/*************************************
 *
 *  Advance to the next sector
 *
 *************************************/

INLINE void next_sector(ide_state *ide)
{
	/* LBA direct? */
	if (ide->cur_head_reg & 0x40)
	{
		ide->cur_sector++;
		if (ide->cur_sector == 0)
		{
			ide->cur_cylinder++;
			if (ide->cur_cylinder == 0)
				ide->cur_head++;
		}
	}

	/* standard CHS */
	else
	{
		/* sectors are 1-based */
		ide->cur_sector++;
		if (ide->cur_sector > ide->num_sectors)
		{
			/* heads are 0 based */
			ide->cur_sector = 1;
			ide->cur_head++;
			if (ide->cur_head >= ide->num_heads)
			{
				ide->cur_head = 0;
				ide->cur_cylinder++;
			}
		}
	}

	ide->cur_lba = lba_address(ide);
}



/*************************************
 *
 *  Build a features page
 *
 *************************************/

static void swap_strncpy(UINT8 *dst, const char *src, int field_size_in_words)
{
	int i;

	for (i = 0; i < field_size_in_words * 2 && src[i]; i++)
		dst[i ^ 1] = src[i];
	for ( ; i < field_size_in_words * 2; i++)
		dst[i ^ 1] = ' ';
}


static void ide_build_features(ide_state *ide)
{
	int total_sectors = ide->num_cylinders * ide->num_heads * ide->num_sectors;
	int sectors_per_track = ide->num_heads * ide->num_sectors;

	memset(ide->buffer, 0, IDE_DISK_SECTOR_SIZE);

	/* basic geometry */
	ide->features[ 0*2+0] = 0x5a;						/*  0: configuration bits */
	ide->features[ 0*2+1] = 0x04;
	ide->features[ 1*2+0] = ide->num_cylinders & 0xff;	/*  1: logical cylinders */
	ide->features[ 1*2+1] = ide->num_cylinders >> 8;
	ide->features[ 2*2+0] = 0;							/*  2: reserved */
	ide->features[ 2*2+1] = 0;
	ide->features[ 3*2+0] = ide->num_heads & 0xff;		/*  3: logical heads */
	ide->features[ 3*2+1] = 0;/*ide->num_heads >> 8;*/
	ide->features[ 4*2+0] = 0;							/*  4: vendor specific (obsolete) */
	ide->features[ 4*2+1] = 0;
	ide->features[ 5*2+0] = 0;							/*  5: vendor specific (obsolete) */
	ide->features[ 5*2+1] = 0;
	ide->features[ 6*2+0] = ide->num_sectors & 0xff;	/*  6: logical sectors per logical track */
	ide->features[ 6*2+1] = 0;/*ide->num_sectors >> 8;*/
	ide->features[ 7*2+0] = 0;							/*  7: vendor-specific */
	ide->features[ 7*2+1] = 0;
	ide->features[ 8*2+0] = 0;							/*  8: vendor-specific */
	ide->features[ 8*2+1] = 0;
	ide->features[ 9*2+0] = 0;							/*  9: vendor-specific */
	ide->features[ 9*2+1] = 0;
	swap_strncpy(&ide->features[10*2+0], 				/* 10-19: serial number */
			"00000000000000000000", 10);
	ide->features[20*2+0] = 0;							/* 20: vendor-specific */
	ide->features[20*2+1] = 0;
	ide->features[21*2+0] = 0;							/* 21: vendor-specific */
	ide->features[21*2+1] = 0;
	ide->features[22*2+0] = 4;							/* 22: # of vendor-specific bytes on read/write long commands */
	ide->features[22*2+1] = 0;
	swap_strncpy(&ide->features[23*2+0], 				/* 23-26: firmware revision */
			"1.0", 4);
	swap_strncpy(&ide->features[27*2+0], 				/* 27-46: model number */
			"MAME Compressed Hard Disk", 20);
	ide->features[47*2+0] = 0x01;						/* 47: read/write multiple support */
	ide->features[47*2+1] = 0x80;
	ide->features[48*2+0] = 0;							/* 48: reserved */
	ide->features[48*2+1] = 0;
	ide->features[49*2+0] = 0x03;						/* 49: capabilities */
	ide->features[49*2+1] = 0x0f;
	ide->features[50*2+0] = 0;							/* 50: reserved */
	ide->features[50*2+1] = 0;
	ide->features[51*2+0] = 2;							/* 51: PIO data transfer cycle timing mode */
	ide->features[51*2+1] = 0;
	ide->features[52*2+0] = 2;							/* 52: single word DMA transfer cycle timing mode */
	ide->features[52*2+1] = 0;
	ide->features[53*2+0] = 3;							/* 53: field validity */
	ide->features[53*2+1] = 0;
	ide->features[54*2+0] = ide->num_cylinders & 0xff;	/* 54: number of current logical cylinders */
	ide->features[54*2+1] = ide->num_cylinders >> 8;
	ide->features[55*2+0] = ide->num_heads & 0xff;		/* 55: number of current logical heads */
	ide->features[55*2+1] = 0;/*ide->num_heads >> 8;*/
	ide->features[56*2+0] = ide->num_sectors & 0xff;	/* 56: number of current logical sectors per track */
	ide->features[56*2+1] = 0;/*ide->num_sectors >> 8;*/
	ide->features[57*2+0] = sectors_per_track & 0xff;	/* 57-58: number of current logical sectors per track */
	ide->features[57*2+1] = sectors_per_track >> 8;
	ide->features[58*2+0] = sectors_per_track >> 16;
	ide->features[58*2+1] = sectors_per_track >> 24;
	ide->features[59*2+0] = 0;							/* 59: multiple sector timing */
	ide->features[59*2+1] = 0;
	ide->features[60*2+0] = total_sectors & 0xff;		/* 60-61: total user addressable sectors */
	ide->features[60*2+1] = total_sectors >> 8;
	ide->features[61*2+0] = total_sectors >> 16;
	ide->features[61*2+1] = total_sectors >> 24;
	ide->features[62*2+0] = 0x07;						/* 62: single word dma transfer */
	ide->features[62*2+1] = 0x00;
	ide->features[63*2+0] = 0x07;						/* 63: multiword DMA transfer */
	ide->features[63*2+1] = 0x04;
	ide->features[64*2+0] = 0x03;						/* 64: flow control PIO transfer modes supported */
	ide->features[64*2+1] = 0x00;
	ide->features[65*2+0] = 0x78;						/* 65: minimum multiword DMA transfer cycle time per word */
	ide->features[65*2+1] = 0x00;
	ide->features[66*2+0] = 0x78;						/* 66: mfr's recommended multiword DMA transfer cycle time */
	ide->features[66*2+1] = 0x00;
	ide->features[67*2+0] = 0x4d;						/* 67: minimum PIO transfer cycle time without flow control */
	ide->features[67*2+1] = 0x01;
	ide->features[68*2+0] = 0x78;						/* 68: minimum PIO transfer cycle time with IORDY */
	ide->features[68*2+1] = 0x00;
	ide->features[69*2+0] = 0x00;						/* 69-70: reserved */
	ide->features[69*2+1] = 0x00;
	ide->features[71*2+0] = 0x00;						/* 71: reserved for IDENTIFY PACKET command */
	ide->features[71*2+1] = 0x00;
	ide->features[72*2+0] = 0x00;						/* 72: reserved for IDENTIFY PACKET command */
	ide->features[72*2+1] = 0x00;
	ide->features[73*2+0] = 0x00;						/* 73: reserved for IDENTIFY PACKET command */
	ide->features[73*2+1] = 0x00;
	ide->features[74*2+0] = 0x00;						/* 74: reserved for IDENTIFY PACKET command */
	ide->features[74*2+1] = 0x00;
	ide->features[75*2+0] = 0x00;						/* 75: queue depth */
	ide->features[75*2+1] = 0x00;
	ide->features[76*2+0] = 0x00;						/* 76-79: reserved */
	ide->features[76*2+1] = 0x00;
	ide->features[80*2+0] = 0x00;						/* 80: major version number */
	ide->features[80*2+1] = 0x00;
	ide->features[81*2+0] = 0x00;						/* 81: minor version number */
	ide->features[81*2+1] = 0x00;
	ide->features[82*2+0] = 0x00;						/* 82: command set supported */
	ide->features[82*2+1] = 0x00;
	ide->features[83*2+0] = 0x00;						/* 83: command sets supported */
	ide->features[83*2+1] = 0x00;
	ide->features[84*2+0] = 0x00;						/* 84: command set/feature supported extension */
	ide->features[84*2+1] = 0x00;
	ide->features[85*2+0] = 0x00;						/* 85: command set/feature enabled */
	ide->features[85*2+1] = 0x00;
	ide->features[86*2+0] = 0x00;						/* 86: command set/feature enabled */
	ide->features[86*2+1] = 0x00;
	ide->features[87*2+0] = 0x00;						/* 87: command set/feature default */
	ide->features[87*2+1] = 0x00;
	ide->features[88*2+0] = 0x00;						/* 88: additional DMA modes */
	ide->features[88*2+1] = 0x00;
	ide->features[89*2+0] = 0x00;						/* 89: time required for security erase unit completion */
	ide->features[89*2+1] = 0x00;
	ide->features[90*2+0] = 0x00;						/* 90: time required for enhanced security erase unit completion */
	ide->features[90*2+1] = 0x00;
	ide->features[91*2+0] = 0x00;						/* 91: current advanced power management value */
	ide->features[91*2+1] = 0x00;
	ide->features[92*2+0] = 0x00;						/* 92: master password revision code */
	ide->features[92*2+1] = 0x00;
	ide->features[93*2+0] = 0x00;						/* 93: hardware reset result */
	ide->features[93*2+1] = 0x00;
	ide->features[94*2+0] = 0x00;						/* 94: acoustic management values */
	ide->features[94*2+1] = 0x00;
	ide->features[95*2+0] = 0x00;						/* 95-99: reserved */
	ide->features[95*2+1] = 0x00;
	ide->features[100*2+0] = total_sectors & 0xff;		/* 100-103: maximum 48-bit LBA */
	ide->features[100*2+1] = total_sectors >> 8;
	ide->features[101*2+0] = total_sectors >> 16;
	ide->features[101*2+1] = total_sectors >> 24;
	ide->features[102*2+0] = 0x00;
	ide->features[102*2+1] = 0x00;
	ide->features[103*2+0] = 0x00;
	ide->features[103*2+1] = 0x00;
	ide->features[104*2+0] = 0x00;						/* 104-126: reserved */
	ide->features[104*2+1] = 0x00;
	ide->features[127*2+0] = 0x00;						/* 127: removable media status notification */
	ide->features[127*2+1] = 0x00;
	ide->features[128*2+0] = 0x00;						/* 128: security status */
	ide->features[128*2+1] = 0x00;
	ide->features[129*2+0] = 0x00;						/* 129-159: vendor specific */
	ide->features[129*2+1] = 0x00;
	ide->features[160*2+0] = 0x00;						/* 160: CFA power mode 1 */
	ide->features[160*2+1] = 0x00;
	ide->features[161*2+0] = 0x00;						/* 161-175: reserved for CompactFlash */
	ide->features[161*2+1] = 0x00;
	ide->features[176*2+0] = 0x00;						/* 176-205: current media serial number */
	ide->features[176*2+1] = 0x00;
	ide->features[206*2+0] = 0x00;						/* 206-254: reserved */
	ide->features[206*2+1] = 0x00;
	ide->features[255*2+0] = 0x00;						/* 255: integrity word */
	ide->features[255*2+1] = 0x00;
}



/*************************************
 *
 *  security error handling
 *
 *************************************/

static TIMER_CALLBACK( security_error_done )
{
	ide_state *ide = ptr;

	/* clear error state */
	ide->status &= ~IDE_STATUS_ERROR;
	ide->status |= IDE_STATUS_DRIVE_READY;
}

static void security_error(ide_state *ide)
{
	/* set error state */
	ide->status |= IDE_STATUS_ERROR;
	ide->status &= ~IDE_STATUS_DRIVE_READY;

	/* just set a timer and mark ourselves error */
	timer_set(TIME_SECURITY_ERROR, ide, 0, security_error_done);
}



/*************************************
 *
 *  Sector reading
 *
 *************************************/

static void continue_read(ide_state *ide)
{
	/* reset the totals */
	ide->buffer_offset = 0;

	/* clear the buffer ready and busy flag */
	ide->status &= ~IDE_STATUS_BUFFER_READY;
	ide->status &= ~IDE_STATUS_BUSY;

	if (ide->master_password_enable || ide->user_password_enable)
	{
		security_error(ide);

		ide->sector_count = 0;
		ide->bus_master_status &= ~IDE_BUSMASTER_STATUS_ACTIVE;
		ide->dma_active = 0;

		return;
	}

	/* if there is more data to read, keep going */
	if (ide->sector_count > 0)
		ide->sector_count--;
	if (ide->sector_count > 0)
		read_next_sector(ide);
	else
	{
		ide->bus_master_status &= ~IDE_BUSMASTER_STATUS_ACTIVE;
		ide->dma_active = 0;
	}
}


static void write_buffer_to_dma(ide_state *ide)
{
	int bytesleft = IDE_DISK_SECTOR_SIZE;
	UINT8 *data = ide->buffer;

//  LOG(("Writing sector to %08X\n", ide->dma_address));

	/* loop until we've consumed all bytes */
	while (bytesleft--)
	{
		/* if we're out of space, grab the next descriptor */
		if (ide->dma_bytes_left == 0)
		{
			/* if we're out of buffer space, that's bad */
			if (ide->dma_last_buffer)
			{
				LOG(("DMA Out of buffer space!\n"));
				return;
			}

			/* fetch the address */
			ide->dma_address = cpunum_read_byte(ide->dma_cpu, ide->dma_descriptor++ ^ ide->dma_address_xor);
			ide->dma_address |= cpunum_read_byte(ide->dma_cpu, ide->dma_descriptor++ ^ ide->dma_address_xor) << 8;
			ide->dma_address |= cpunum_read_byte(ide->dma_cpu, ide->dma_descriptor++ ^ ide->dma_address_xor) << 16;
			ide->dma_address |= cpunum_read_byte(ide->dma_cpu, ide->dma_descriptor++ ^ ide->dma_address_xor) << 24;
			ide->dma_address &= 0xfffffffe;

			/* fetch the length */
			ide->dma_bytes_left = cpunum_read_byte(ide->dma_cpu, ide->dma_descriptor++ ^ ide->dma_address_xor);
			ide->dma_bytes_left |= cpunum_read_byte(ide->dma_cpu, ide->dma_descriptor++ ^ ide->dma_address_xor) << 8;
			ide->dma_bytes_left |= cpunum_read_byte(ide->dma_cpu, ide->dma_descriptor++ ^ ide->dma_address_xor) << 16;
			ide->dma_bytes_left |= cpunum_read_byte(ide->dma_cpu, ide->dma_descriptor++ ^ ide->dma_address_xor) << 24;
			ide->dma_last_buffer = (ide->dma_bytes_left >> 31) & 1;
			ide->dma_bytes_left &= 0xfffe;
			if (ide->dma_bytes_left == 0)
				ide->dma_bytes_left = 0x10000;

//          LOG(("New DMA descriptor: address = %08X  bytes = %04X  last = %d\n", ide->dma_address, ide->dma_bytes_left, ide->dma_last_buffer));
		}

		/* write the next byte */
		cpunum_write_byte(ide->dma_cpu, ide->dma_address++, *data++);
		ide->dma_bytes_left--;
	}
}


static void read_sector_done(ide_state *ide)
{
	int lba = lba_address(ide), count = 0;

	/* now do the read */
	if (ide->disk)
		count = hard_disk_read(ide->disk, lba, ide->buffer);

	/* by default, mark the buffer ready and the seek complete */
	if (!ide->verify_only)
		ide->status |= IDE_STATUS_BUFFER_READY;
	ide->status |= IDE_STATUS_SEEK_COMPLETE;

	/* and clear the busy and error flags */
	ide->status &= ~IDE_STATUS_ERROR;
	ide->status &= ~IDE_STATUS_BUSY;

	/* if we succeeded, advance to the next sector and set the nice bits */
	if (count == 1)
	{
		/* advance the pointers, unless this is the last sector */
		/* Gauntlet: Dark Legacy checks to make sure we stop on the last sector */
		if (ide->sector_count != 1)
			next_sector(ide);

		/* clear the error value */
		ide->error = IDE_ERROR_NONE;

		/* signal an interrupt */
		if (!ide->verify_only)
			ide->sectors_until_int--;
		if (ide->sectors_until_int == 0 || ide->sector_count == 1)
		{
			ide->sectors_until_int = ((ide->command == IDE_COMMAND_READ_MULTIPLE_BLOCK) ? ide->block_count : 1);
			signal_interrupt(ide);
		}

		/* handle DMA */
		if (ide->dma_active)
			write_buffer_to_dma(ide);

		/* if we're just verifying or if we DMA'ed the data, we can read the next sector */
		if (ide->verify_only || ide->dma_active)
			continue_read(ide);
	}

	/* if we got an error, we need to report it */
	else
	{
		/* set the error flag and the error */
		ide->status |= IDE_STATUS_ERROR;
		ide->error = IDE_ERROR_BAD_SECTOR;
		ide->bus_master_status |= IDE_BUSMASTER_STATUS_ERROR;
		ide->bus_master_status &= ~IDE_BUSMASTER_STATUS_ACTIVE;

		/* signal an interrupt */
		signal_interrupt(ide);
	}
}


static TIMER_CALLBACK( read_sector_done_callback )
{
	read_sector_done(ptr);
}


static void read_first_sector(ide_state *ide)
{
	/* mark ourselves busy */
	ide->status |= IDE_STATUS_BUSY;

	/* just set a timer */
	if (ide->command == IDE_COMMAND_READ_MULTIPLE_BLOCK)
	{
		int new_lba = lba_address(ide);
		attotime seek_time;

		if (new_lba == ide->cur_lba || new_lba == ide->cur_lba + 1)
			seek_time = TIME_NO_SEEK_MULTISECTOR;
		else
			seek_time = TIME_SEEK_MULTISECTOR;

		ide->cur_lba = new_lba;
		timer_set(seek_time, ide, 0, read_sector_done_callback);
	}
	else
		timer_set(TIME_PER_SECTOR, ide, 0, read_sector_done_callback);
}


static void read_next_sector(ide_state *ide)
{
	/* mark ourselves busy */
	ide->status |= IDE_STATUS_BUSY;

	if (ide->command == IDE_COMMAND_READ_MULTIPLE_BLOCK)
	{
		if (ide->sectors_until_int != 1)
			/* make ready now */
			read_sector_done(ide);
		else
			/* just set a timer */
			timer_set(ATTOTIME_IN_USEC(1), ide, 0, read_sector_done_callback);
	}
	else
		/* just set a timer */
		timer_set(TIME_PER_SECTOR, ide, 0, read_sector_done_callback);
}



/*************************************
 *
 *  Sector writing
 *
 *************************************/

static void write_sector_done(ide_state *ide);
static TIMER_CALLBACK( write_sector_done_callback );

static void continue_write(ide_state *ide)
{
	/* reset the totals */
	ide->buffer_offset = 0;

	/* clear the buffer ready flag */
	ide->status &= ~IDE_STATUS_BUFFER_READY;
	ide->status |= IDE_STATUS_BUSY;

	if (ide->command == IDE_COMMAND_WRITE_MULTIPLE_BLOCK)
	{
		if (ide->sectors_until_int != 1)
		{
			/* ready to write now */
			write_sector_done(ide);
		}
		else
		{
			/* set a timer to do the write */
			timer_set(TIME_PER_SECTOR, ide, 0, write_sector_done_callback);
		}
	}
	else
	{
		/* set a timer to do the write */
		timer_set(TIME_PER_SECTOR, ide, 0, write_sector_done_callback);
	}
}


static void read_buffer_from_dma(ide_state *ide)
{
	int bytesleft = IDE_DISK_SECTOR_SIZE;
	UINT8 *data = ide->buffer;

//  LOG(("Reading sector from %08X\n", ide->dma_address));

	/* loop until we've consumed all bytes */
	while (bytesleft--)
	{
		/* if we're out of space, grab the next descriptor */
		if (ide->dma_bytes_left == 0)
		{
			/* if we're out of buffer space, that's bad */
			if (ide->dma_last_buffer)
			{
				LOG(("DMA Out of buffer space!\n"));
				return;
			}

			/* fetch the address */
			ide->dma_address = cpunum_read_byte(ide->dma_cpu, ide->dma_descriptor++ ^ ide->dma_address_xor);
			ide->dma_address |= cpunum_read_byte(ide->dma_cpu, ide->dma_descriptor++ ^ ide->dma_address_xor) << 8;
			ide->dma_address |= cpunum_read_byte(ide->dma_cpu, ide->dma_descriptor++ ^ ide->dma_address_xor) << 16;
			ide->dma_address |= cpunum_read_byte(ide->dma_cpu, ide->dma_descriptor++ ^ ide->dma_address_xor) << 24;
			ide->dma_address &= 0xfffffffe;

			/* fetch the length */
			ide->dma_bytes_left = cpunum_read_byte(ide->dma_cpu, ide->dma_descriptor++ ^ ide->dma_address_xor);
			ide->dma_bytes_left |= cpunum_read_byte(ide->dma_cpu, ide->dma_descriptor++ ^ ide->dma_address_xor) << 8;
			ide->dma_bytes_left |= cpunum_read_byte(ide->dma_cpu, ide->dma_descriptor++ ^ ide->dma_address_xor) << 16;
			ide->dma_bytes_left |= cpunum_read_byte(ide->dma_cpu, ide->dma_descriptor++ ^ ide->dma_address_xor) << 24;
			ide->dma_last_buffer = (ide->dma_bytes_left >> 31) & 1;
			ide->dma_bytes_left &= 0xfffe;
			if (ide->dma_bytes_left == 0)
				ide->dma_bytes_left = 0x10000;

//          LOG(("New DMA descriptor: address = %08X  bytes = %04X  last = %d\n", ide->dma_address, ide->dma_bytes_left, ide->dma_last_buffer));
		}

		/* read the next byte */
		*data++ = cpunum_read_byte(ide->dma_cpu, ide->dma_address++);
		ide->dma_bytes_left--;
	}
}


static void write_sector_done(ide_state *ide)
{
	int lba = lba_address(ide), count = 0;

	/* now do the write */
	if (ide->disk)
		count = hard_disk_write(ide->disk, lba, ide->buffer);

	/* by default, mark the buffer ready and the seek complete */
	ide->status |= IDE_STATUS_BUFFER_READY;
	ide->status |= IDE_STATUS_SEEK_COMPLETE;

	/* and clear the busy adn error flags */
	ide->status &= ~IDE_STATUS_ERROR;
	ide->status &= ~IDE_STATUS_BUSY;

	/* if we succeeded, advance to the next sector and set the nice bits */
	if (count == 1)
	{
		/* advance the pointers, unless this is the last sector */
		/* Gauntlet: Dark Legacy checks to make sure we stop on the last sector */
		if (ide->sector_count != 1)
			next_sector(ide);

		/* clear the error value */
		ide->error = IDE_ERROR_NONE;

		/* signal an interrupt */
		if (--ide->sectors_until_int == 0 || ide->sector_count == 1)
		{
			ide->sectors_until_int = ((ide->command == IDE_COMMAND_WRITE_MULTIPLE_BLOCK) ? ide->block_count : 1);
			signal_interrupt(ide);
		}

		/* signal an interrupt if there's more data needed */
		if (ide->sector_count > 0)
			ide->sector_count--;
		if (ide->sector_count == 0)
			ide->status &= ~IDE_STATUS_BUFFER_READY;

		/* keep going for DMA */
		if (ide->dma_active && ide->sector_count != 0)
		{
			read_buffer_from_dma(ide);
			continue_write(ide);
		}
		else
			ide->dma_active = 0;
	}

	/* if we got an error, we need to report it */
	else
	{
		/* set the error flag and the error */
		ide->status |= IDE_STATUS_ERROR;
		ide->error = IDE_ERROR_BAD_SECTOR;
		ide->bus_master_status |= IDE_BUSMASTER_STATUS_ERROR;
		ide->bus_master_status &= ~IDE_BUSMASTER_STATUS_ACTIVE;

		/* signal an interrupt */
		signal_interrupt(ide);
	}
}


static TIMER_CALLBACK( write_sector_done_callback )
{
	write_sector_done(ptr);
}



/*************************************
 *
 *  Handle IDE commands
 *
 *************************************/

static void handle_command(ide_state *ide, UINT8 command)
{
	/* implicitly clear interrupts here */
	clear_interrupt(ide);

	ide->command = command;
	switch (command)
	{
		case IDE_COMMAND_READ_MULTIPLE:
		case IDE_COMMAND_READ_MULTIPLE_ONCE:
			LOGPRINT(("IDE Read multiple: C=%d H=%d S=%d LBA=%d count=%d\n",
				ide->cur_cylinder, ide->cur_head, ide->cur_sector, lba_address(ide), ide->sector_count));

			/* reset the buffer */
			ide->buffer_offset = 0;
			ide->sectors_until_int = 1;
			ide->dma_active = 0;
			ide->verify_only = 0;

			/* start the read going */
			read_first_sector(ide);
			break;

		case IDE_COMMAND_READ_MULTIPLE_BLOCK:
			LOGPRINT(("IDE Read multiple block: C=%d H=%d S=%d LBA=%d count=%d\n",
				ide->cur_cylinder, ide->cur_head, ide->cur_sector, lba_address(ide), ide->sector_count));

			/* reset the buffer */
			ide->buffer_offset = 0;
			ide->sectors_until_int = 1;
			ide->dma_active = 0;
			ide->verify_only = 0;

			/* start the read going */
			read_first_sector(ide);
			break;

		case IDE_COMMAND_VERIFY_MULTIPLE:
			LOGPRINT(("IDE Read verify multiple with retries: C=%d H=%d S=%d LBA=%d count=%d\n",
				ide->cur_cylinder, ide->cur_head, ide->cur_sector, lba_address(ide), ide->sector_count));

			/* reset the buffer */
			ide->buffer_offset = 0;
			ide->sectors_until_int = 1;
			ide->dma_active = 0;
			ide->verify_only = 1;

			/* start the read going */
			read_first_sector(ide);
			break;

		case IDE_COMMAND_READ_DMA:
			LOGPRINT(("IDE Read multiple DMA: C=%d H=%d S=%d LBA=%d count=%d\n",
				ide->cur_cylinder, ide->cur_head, ide->cur_sector, lba_address(ide), ide->sector_count));

			/* reset the buffer */
			ide->buffer_offset = 0;
			ide->sectors_until_int = ide->sector_count;
			ide->dma_active = 1;
			ide->verify_only = 0;

			/* start the read going */
			if (ide->bus_master_command & 1)
				read_first_sector(ide);
			break;

		case IDE_COMMAND_WRITE_MULTIPLE:
			LOGPRINT(("IDE Write multiple: C=%d H=%d S=%d LBA=%d count=%d\n",
				ide->cur_cylinder, ide->cur_head, ide->cur_sector, lba_address(ide), ide->sector_count));

			/* reset the buffer */
			ide->buffer_offset = 0;
			ide->sectors_until_int = 1;
			ide->dma_active = 0;

			/* mark the buffer ready */
			ide->status |= IDE_STATUS_BUFFER_READY;
			break;

		case IDE_COMMAND_WRITE_MULTIPLE_BLOCK:
			LOGPRINT(("IDE Write multiple block: C=%d H=%d S=%d LBA=%d count=%d\n",
				ide->cur_cylinder, ide->cur_head, ide->cur_sector, lba_address(ide), ide->sector_count));

			/* reset the buffer */
			ide->buffer_offset = 0;
			ide->sectors_until_int = 1;
			ide->dma_active = 0;

			/* mark the buffer ready */
			ide->status |= IDE_STATUS_BUFFER_READY;
			break;

		case IDE_COMMAND_WRITE_DMA:
			LOGPRINT(("IDE Write multiple DMA: C=%d H=%d S=%d LBA=%d count=%d\n",
				ide->cur_cylinder, ide->cur_head, ide->cur_sector, lba_address(ide), ide->sector_count));

			/* reset the buffer */
			ide->buffer_offset = 0;
			ide->sectors_until_int = ide->sector_count;
			ide->dma_active = 1;

			/* start the read going */
			if (ide->bus_master_command & 1)
			{
				read_buffer_from_dma(ide);
				continue_write(ide);
			}
			break;

		case IDE_COMMAND_SECURITY_UNLOCK:
			LOGPRINT(("IDE Security Unlock\n"));

			/* reset the buffer */
			ide->buffer_offset = 0;
			ide->sectors_until_int = 0;
			ide->dma_active = 0;

			/* mark the buffer ready */
			ide->status |= IDE_STATUS_BUFFER_READY;
			signal_interrupt(ide);
			break;

		case IDE_COMMAND_GET_INFO:
			LOGPRINT(("IDE Read features\n"));

			/* reset the buffer */
			ide->buffer_offset = 0;
			ide->sector_count = 1;

			/* build the features page */
			memcpy(ide->buffer, ide->features, sizeof(ide->buffer));

			/* indicate everything is ready */
			ide->status |= IDE_STATUS_BUFFER_READY;
			ide->status |= IDE_STATUS_SEEK_COMPLETE;
			ide->status |= IDE_STATUS_DRIVE_READY;

			/* and clear the busy adn error flags */
			ide->status &= ~IDE_STATUS_ERROR;
			ide->status &= ~IDE_STATUS_BUSY;

			/* clear the error too */
			ide->error = IDE_ERROR_NONE;

			/* signal an interrupt */
			signal_delayed_interrupt(ide, MINIMUM_COMMAND_TIME, 1);
			break;

		case IDE_COMMAND_SET_CONFIG:
			LOGPRINT(("IDE Set configuration (%d heads, %d sectors)\n", ide->cur_head + 1, ide->sector_count));

			ide->num_sectors = ide->sector_count;
			ide->num_heads = ide->cur_head + 1;

			/* signal an interrupt */
			signal_interrupt(ide);
			break;

		case IDE_COMMAND_UNKNOWN_F9:
			/* only used by Killer Instinct AFAICT */
			LOGPRINT(("IDE unknown command (F9)\n"));

			/* signal an interrupt */
			signal_interrupt(ide);
			break;

		case IDE_COMMAND_SET_FEATURES:
			LOGPRINT(("IDE Set features (%02X %02X %02X %02X %02X)\n", ide->precomp_offset, ide->sector_count & 0xff, ide->cur_sector, ide->cur_cylinder & 0xff, ide->cur_cylinder >> 8));

			/* signal an interrupt */
			signal_delayed_interrupt(ide, MINIMUM_COMMAND_TIME, 0);
			break;

		case IDE_COMMAND_SET_BLOCK_COUNT:
			LOGPRINT(("IDE Set block count (%02X)\n", ide->sector_count));

			ide->block_count = ide->sector_count;
			// judge dredd wants 'drive ready' on this command
			ide->status |= IDE_STATUS_DRIVE_READY;

			/* signal an interrupt */
			signal_interrupt(ide);
			break;

		default:
			LOGPRINT(("IDE unknown command (%02X)\n", command));
			DEBUGGER_BREAK;
			break;
	}
}



/*************************************
 *
 *  IDE controller read
 *
 *************************************/

static UINT32 ide_controller_read(const device_config *device, offs_t offset, int size)
{
	ide_state *ide = get_safe_token(device);
	UINT32 result = 0;

	/* logit */
//  if (offset != IDE_ADDR_DATA && offset != IDE_ADDR_STATUS_COMMAND && offset != IDE_ADDR_STATUS_CONTROL)
		LOG(("%08X:IDE read at %03X, size=%d\n", activecpu_get_previouspc(), offset, size));

	switch (offset)
	{
		/* unknown config register */
		case IDE_ADDR_CONFIG_UNK:
			return ide->config_unknown;

		/* active config register */
		case IDE_ADDR_CONFIG_REGISTER:
			return ide->config_register_num;

		/* data from active config register */
		case IDE_ADDR_CONFIG_DATA:
			if (ide->config_register_num < IDE_CONFIG_REGISTERS)
				return ide->config_register[ide->config_register_num];
			return 0;

		/* read data if there's data to be read */
		case IDE_ADDR_DATA:
			if (ide->status & IDE_STATUS_BUFFER_READY)
			{
				/* fetch the correct amount of data */
				result = ide->buffer[ide->buffer_offset++];
				if (size > 1)
					result |= ide->buffer[ide->buffer_offset++] << 8;
				if (size > 2)
				{
					result |= ide->buffer[ide->buffer_offset++] << 16;
					result |= ide->buffer[ide->buffer_offset++] << 24;
				}

				/* if we're at the end of the buffer, handle it */
				if (ide->buffer_offset >= IDE_DISK_SECTOR_SIZE)
				{
					LOG(("%08X:IDE completed PIO read\n", activecpu_get_previouspc()));
					continue_read(ide);
				}
			}
			break;

		/* return the current error */
		case IDE_ADDR_ERROR:
			return ide->error;

		/* return the current sector count */
		case IDE_ADDR_SECTOR_COUNT:
			return ide->sector_count;

		/* return the current sector */
		case IDE_ADDR_SECTOR_NUMBER:
			return ide->cur_sector;

		/* return the current cylinder LSB */
		case IDE_ADDR_CYLINDER_LSB:
			return ide->cur_cylinder & 0xff;

		/* return the current cylinder MSB */
		case IDE_ADDR_CYLINDER_MSB:
			return ide->cur_cylinder >> 8;

		/* return the current head */
		case IDE_ADDR_HEAD_NUMBER:
			return ide->cur_head_reg;

		/* return the current status and clear any pending interrupts */
		case IDE_ADDR_STATUS_COMMAND:
		/* return the current status but don't clear interrupts */
		case IDE_ADDR_STATUS_CONTROL:
			result = ide->status;
			if (attotime_compare(timer_timeelapsed(ide->last_status_timer), TIME_PER_ROTATION) > 0)
			{
				result |= IDE_STATUS_HIT_INDEX;
				timer_adjust_oneshot(ide->last_status_timer, attotime_never, 0);
			}

			/* clear interrutps only when reading the real status */
			if (offset == IDE_ADDR_STATUS_COMMAND)
			{
				if (ide->interrupt_pending)
					clear_interrupt(ide);
			}

			/* take a bit of time to speed up people who poll hard */
			activecpu_adjust_icount(-100);
			break;

		/* log anything else */
		default:
			logerror("%08X:unknown IDE read at %03X, size=%d\n", activecpu_get_previouspc(), offset, size);
			break;
	}

	/* return the result */
	return result;
}



/*************************************
 *
 *  IDE controller write
 *
 *************************************/

static void ide_controller_write(const device_config *device, offs_t offset, int size, UINT32 data)
{
	ide_state *ide = get_safe_token(device);

	/* logit */
	if (offset != IDE_ADDR_DATA)
		LOG(("%08X:IDE write to %03X = %08X, size=%d\n", activecpu_get_previouspc(), offset, data, size));

	switch (offset)
	{
		/* unknown config register */
		case IDE_ADDR_CONFIG_UNK:
			ide->config_unknown = data;
			break;

		/* active config register */
		case IDE_ADDR_CONFIG_REGISTER:
			ide->config_register_num = data;
			break;

		/* data from active config register */
		case IDE_ADDR_CONFIG_DATA:
			if (ide->config_register_num < IDE_CONFIG_REGISTERS)
				ide->config_register[ide->config_register_num] = data;
			break;

		/* write data */
		case IDE_ADDR_DATA:
			if (ide->status & IDE_STATUS_BUFFER_READY)
			{
				/* store the correct amount of data */
				ide->buffer[ide->buffer_offset++] = data;
				if (size > 1)
					ide->buffer[ide->buffer_offset++] = data >> 8;
				if (size > 2)
				{
					ide->buffer[ide->buffer_offset++] = data >> 16;
					ide->buffer[ide->buffer_offset++] = data >> 24;
				}

				/* if we're at the end of the buffer, handle it */
				if (ide->buffer_offset >= IDE_DISK_SECTOR_SIZE)
				{
					LOG(("%08X:IDE completed PIO write\n", activecpu_get_previouspc()));
					if (ide->command != IDE_COMMAND_SECURITY_UNLOCK)
						continue_write(ide);
					else
					{
						if (ide->user_password_enable && memcmp(ide->buffer, ide->user_password, 2 + 32) == 0)
						{
							LOGPRINT(("IDE Unlocked user password\n"));
							ide->user_password_enable = 0;
						}
						if (ide->master_password_enable && memcmp(ide->buffer, ide->master_password, 2 + 32) == 0)
						{
							LOGPRINT(("IDE Unlocked master password\n"));
							ide->master_password_enable = 0;
						}
#if PRINTF_IDE_PASSWORD
						{
							int i;

							for (i = 0; i < 34; i += 2)
							{
								if (i % 8 == 2)
									mame_printf_debug("\n");

								mame_printf_debug("0x%02x, 0x%02x, ", ide->buffer[i], ide->buffer[i + 1]);
								//mame_printf_debug("0x%02x%02x, ", ide->buffer[i], ide->buffer[i + 1]);
							}
							mame_printf_debug("\n");
						}
#endif

						/* clear the busy adn error flags */
						ide->status &= ~IDE_STATUS_ERROR;
						ide->status &= ~IDE_STATUS_BUSY;
						ide->status &= ~IDE_STATUS_BUFFER_READY;

						if (ide->master_password_enable || ide->user_password_enable)
							security_error(ide);
						else
							ide->status |= IDE_STATUS_DRIVE_READY;
					}
				}
			}
			break;

		/* precompensation offset?? */
		case IDE_ADDR_ERROR:
			ide->precomp_offset = data;
			break;

		/* sector count */
		case IDE_ADDR_SECTOR_COUNT:
			ide->sector_count = data ? data : 256;
			break;

		/* current sector */
		case IDE_ADDR_SECTOR_NUMBER:
			ide->cur_sector = data;
			break;

		/* current cylinder LSB */
		case IDE_ADDR_CYLINDER_LSB:
			ide->cur_cylinder = (ide->cur_cylinder & 0xff00) | (data & 0xff);
			break;

		/* current cylinder MSB */
		case IDE_ADDR_CYLINDER_MSB:
			ide->cur_cylinder = (ide->cur_cylinder & 0x00ff) | ((data & 0xff) << 8);
			break;

		/* current head */
		case IDE_ADDR_HEAD_NUMBER:
			ide->cur_head = data & 0x0f;
			ide->cur_head_reg = data;
			// drive index = data & 0x10
			// LBA mode = data & 0x40
			break;

		/* command */
		case IDE_ADDR_STATUS_COMMAND:
			handle_command(ide, data);
			break;

		/* adapter control */
		case IDE_ADDR_STATUS_CONTROL:
			ide->adapter_control = data;

			/* handle controller reset */
			//if (data == 0x04)
			if (data & 0x04)
			{
				ide->status |= IDE_STATUS_BUSY;
				ide->status &= ~IDE_STATUS_DRIVE_READY;
				timer_adjust_oneshot(ide->reset_timer, ATTOTIME_IN_MSEC(5), 0);
			}
			break;
	}
}



/*************************************
 *
 *  Bus master read
 *
 *************************************/

static UINT32 ide_bus_master_read(const device_config *device, offs_t offset, int size)
{
	ide_state *ide = get_safe_token(device);

	LOG(("%08X:ide_bus_master_read(%d, %d)\n", activecpu_get_previouspc(), offset, size));

	/* command register */
	if (offset == 0)
		return ide->bus_master_command | (ide->bus_master_status << 16);

	/* status register */
	if (offset == 2)
		return ide->bus_master_status;

	/* descriptor table register */
	if (offset == 4)
		return ide->bus_master_descriptor;

	return 0xffffffff;
}



/*************************************
 *
 *  Bus master write
 *
 *************************************/

static void ide_bus_master_write(const device_config *device, offs_t offset, int size, UINT32 data)
{
	ide_state *ide = get_safe_token(device);

	LOG(("%08X:ide_bus_master_write(%d, %d, %08X)\n", activecpu_get_previouspc(), offset, size, data));

	/* command register */
	if (offset == 0)
	{
		UINT8 old = ide->bus_master_command;
		UINT8 val = data & 0xff;

		/* save the read/write bit and the start/stop bit */
		ide->bus_master_command = (old & 0xf6) | (val & 0x09);
		ide->bus_master_status = (ide->bus_master_status & ~IDE_BUSMASTER_STATUS_ACTIVE) | (val & 0x01);

		/* handle starting a transfer */
		if (!(old & 1) && (val & 1))
		{
			/* reset all the DMA data */
			ide->dma_bytes_left = 0;
			ide->dma_last_buffer = 0;
			ide->dma_descriptor = ide->bus_master_descriptor;
			ide->dma_cpu = cpu_getactivecpu();
			ide->dma_address_xor = (activecpu_endianness() == CPU_IS_LE) ? 0 : 3;

			/* if we're going live, start the pending read/write */
			if (ide->dma_active)
			{
				if (ide->bus_master_command & 8)
					read_next_sector(ide);
				else
				{
					read_buffer_from_dma(ide);
					continue_write(ide);
				}
			}
		}
	}

	/* status register */
	if (offset <= 2 && offset + size > 2)
	{
		UINT8 old = ide->bus_master_status;
		UINT8 val = data >> (8 * (2 - offset));

		/* save the DMA capable bits */
		ide->bus_master_status = (old & 0x9f) | (val & 0x60);

		/* clear interrupt and error bits */
		if (val & IDE_BUSMASTER_STATUS_IRQ)
			ide->bus_master_status &= ~IDE_BUSMASTER_STATUS_IRQ;
		if (val & IDE_BUSMASTER_STATUS_ERROR)
			ide->bus_master_status &= ~IDE_BUSMASTER_STATUS_ERROR;
	}

	/* descriptor table register */
	if (offset == 4)
		ide->bus_master_descriptor = data & 0xfffffffc;
}



/*************************************
 *
 *  IDE direct handlers (16-bit)
 *
 *************************************/

/*
    ide_bus_r()

    Read a 16-bit word from the IDE bus directly.

    select: 0->CS1Fx active, 1->CS3Fx active
    offset: register offset (state of DA2-DA0)
*/
int ide_bus_r(const device_config *device, int select, int offset)
{
	offset += select ? 0x3f0 : 0x1f0;
	return ide_controller_read(device, offset, (offset == 0x1f0) ? 2 : 1);
}

/*
    ide_bus_w()

    Write a 16-bit word to the IDE bus directly.

    select: 0->CS1Fx active, 1->CS3Fx active
    offset: register offset (state of DA2-DA0)
    data: data written (state of D0-D15 or D0-D7)
*/
void ide_bus_w(const device_config *device, int select, int offset, int data)
{
	offset += select ? 0x3f0 : 0x1f0;
	if (offset == 0x1f0)
		ide_controller_write(device, offset, 2, data);
	else
		ide_controller_write(device, offset, 1, data & 0xff);
}

int ide_controller_r(const device_config *device, int reg)
{
	return ide_controller_read(device, reg, 1);
}

void ide_controller_w(const device_config *device, int reg, int data)
{
	ide_controller_write(device, reg, 1, data);
}


/*************************************
 *
 *  32-bit IDE handlers
 *
 *************************************/

READ32_DEVICE_HANDLER( ide_controller32_r )
{
	int size;

	offset *= 4;
	size = convert_to_offset_and_size32(&offset, mem_mask);

	return ide_controller_read(device, offset, size) << ((offset & 3) * 8);
}


WRITE32_DEVICE_HANDLER( ide_controller32_w )
{
	int size;

	offset *= 4;
	size = convert_to_offset_and_size32(&offset, mem_mask);

	ide_controller_write(device, offset, size, data >> ((offset & 3) * 8));
}


READ32_DEVICE_HANDLER( ide_bus_master32_r )
{
	int size;

	offset *= 4;
	size = convert_to_offset_and_size32(&offset, mem_mask);

	return ide_bus_master_read(device, offset, size) << ((offset & 3) * 8);
}


WRITE32_DEVICE_HANDLER( ide_bus_master32_w )
{
	int size;

	offset *= 4;
	size = convert_to_offset_and_size32(&offset, mem_mask);

	ide_bus_master_write(device, offset, size, data >> ((offset & 3) * 8));
}



/*************************************
 *
 *  16-bit IDE handlers
 *
 *************************************/

READ16_DEVICE_HANDLER( ide_controller16_r )
{
	int size;

	offset *= 2;
	size = convert_to_offset_and_size16(&offset, mem_mask);

	return ide_controller_read(device, offset, size) << ((offset & 1) * 8);
}


WRITE16_DEVICE_HANDLER( ide_controller16_w )
{
	int size;

	offset *= 2;
	size = convert_to_offset_and_size16(&offset, mem_mask);

	ide_controller_write(device, offset, size, data >> ((offset & 1) * 8));
}



/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

static DEVICE_START( ide_controller )
{
	ide_state *ide = get_safe_token(device);
	const hard_disk_info *hdinfo;
	const ide_config *config;
	char unique_tag[50];

	/* validate some basic stuff */
	assert(device != NULL);
	assert(device->static_config == NULL);
	assert(device->inline_config != NULL);
	assert(device->machine != NULL);
	assert(device->machine->config != NULL);

	/* store a pointer back to the device */
	ide->device = device;

	/* set MAME harddisk handle */
	config = device->inline_config;
	ide->disk = hard_disk_open(get_disk_handle(config->disknum));

	/* get and copy the geometry */
	if (ide->disk != NULL)
	{
		hdinfo = hard_disk_get_info(ide->disk);
		if (hdinfo->sectorbytes == IDE_DISK_SECTOR_SIZE)
		{
			ide->num_cylinders = hdinfo->cylinders;
			ide->num_sectors = hdinfo->sectors;
			ide->num_heads = hdinfo->heads;
#if PRINTF_IDE_COMMANDS
			mame_printf_debug("CHS: %d %d %d\n", ide->num_cylinders, ide->num_heads, ide->num_sectors);
#endif
		}
	}

	/* build the features page */
	ide_build_features(ide);

	/* create a timer for timing status */
	ide->last_status_timer = timer_alloc(NULL, NULL);
	ide->reset_timer = timer_alloc(reset_callback, (void *)device);

	/* create the name for save states */
	assert(strlen(device->tag) < 30);
	state_save_combine_module_and_tag(unique_tag, "ide_controller", device->tag);

	/* register ide states */
	state_save_register_item(unique_tag, 0, ide->adapter_control);
	state_save_register_item(unique_tag, 0, ide->status);
	state_save_register_item(unique_tag, 0, ide->error);
	state_save_register_item(unique_tag, 0, ide->command);
	state_save_register_item(unique_tag, 0, ide->interrupt_pending);
	state_save_register_item(unique_tag, 0, ide->precomp_offset);

	state_save_register_item_array(unique_tag, 0, ide->buffer);
	state_save_register_item_array(unique_tag, 0, ide->features);
	state_save_register_item(unique_tag, 0, ide->buffer_offset);
	state_save_register_item(unique_tag, 0, ide->sector_count);

	state_save_register_item(unique_tag, 0, ide->block_count);
	state_save_register_item(unique_tag, 0, ide->sectors_until_int);

	state_save_register_item(unique_tag, 0, ide->dma_active);
	state_save_register_item(unique_tag, 0, ide->dma_cpu);
	state_save_register_item(unique_tag, 0, ide->dma_address_xor);
	state_save_register_item(unique_tag, 0, ide->dma_last_buffer);
	state_save_register_item(unique_tag, 0, ide->dma_address);
	state_save_register_item(unique_tag, 0, ide->dma_descriptor);
	state_save_register_item(unique_tag, 0, ide->dma_bytes_left);

	state_save_register_item(unique_tag, 0, ide->bus_master_command);
	state_save_register_item(unique_tag, 0, ide->bus_master_status);
	state_save_register_item(unique_tag, 0, ide->bus_master_descriptor);

	state_save_register_item(unique_tag, 0, ide->cur_cylinder);
	state_save_register_item(unique_tag, 0, ide->cur_sector);
	state_save_register_item(unique_tag, 0, ide->cur_head);
	state_save_register_item(unique_tag, 0, ide->cur_head_reg);

	state_save_register_item(unique_tag, 0, ide->cur_lba);

	state_save_register_item(unique_tag, 0, ide->num_cylinders);
	state_save_register_item(unique_tag, 0, ide->num_sectors);
	state_save_register_item(unique_tag, 0, ide->num_heads);

	state_save_register_item(unique_tag, 0, ide->config_unknown);
	state_save_register_item_array(unique_tag, 0, ide->config_register);
	state_save_register_item(unique_tag, 0, ide->config_register_num);

	state_save_register_item(unique_tag, 0, ide->master_password_enable);
	state_save_register_item(unique_tag, 0, ide->user_password_enable);
}


/*-------------------------------------------------
    device exit callback
-------------------------------------------------*/

static DEVICE_STOP( ide_controller )
{
	ide_state *ide = get_safe_token(device);

	/* close the hard disk */
	if (ide->disk != NULL)
		hard_disk_close(ide->disk);
}


/*-------------------------------------------------
    device reset callback
-------------------------------------------------*/

static DEVICE_RESET( ide_controller )
{
	ide_state *ide = get_safe_token(device);

	LOG(("IDE controller reset performed\n"));

	/* reset the drive state */
	ide->status = IDE_STATUS_DRIVE_READY | IDE_STATUS_SEEK_COMPLETE;
	ide->error = IDE_ERROR_DEFAULT;
	ide->buffer_offset = 0;
	ide->master_password_enable = (ide->master_password != NULL);
	ide->user_password_enable = (ide->user_password != NULL);
	clear_interrupt(ide);
}


/*-------------------------------------------------
    device set info callback
-------------------------------------------------*/

static DEVICE_SET_INFO( ide_controller )
{
	switch (state)
	{
		/* no parameters to set */
	}
}


/*-------------------------------------------------
    device get info callback
-------------------------------------------------*/

DEVICE_GET_INFO( ide_controller )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(ide_state);			break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:	info->i = sizeof(ide_config);			break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_PERIPHERAL;		break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_SET_INFO:				info->set_info = DEVICE_SET_INFO_NAME(ide_controller); break;
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(ide_controller); break;
		case DEVINFO_FCT_STOP:					info->stop = DEVICE_STOP_NAME(ide_controller); break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(ide_controller);break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					info->s = "IDE Controller";				break;
		case DEVINFO_STR_FAMILY:				info->s = "Disk Controller";			break;
		case DEVINFO_STR_VERSION:				info->s = "1.0";						break;
		case DEVINFO_STR_SOURCE_FILE:			info->s = __FILE__;						break;
		case DEVINFO_STR_CREDITS:				info->s = "Copyright Nicola Salmoria and the MAME Team"; break;
	}
}
