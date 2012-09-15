/*
    Intel 28F640J5 Flash ROM emulation (could also handle 28F320J5 with minor
    changes, and possibly 28F256J3, 28F128J3, 28F640J3 and 28F320J3)

    The 28F640J5 is a 64Mbit FEEPROM that can be accessed either on an 8-bit or
    a 16-bit bus.

    References:
    Datasheets were found on Intel's site (www.intel.com)

    Raphael Nabet 2004, based on MAME's intelfsh.c core
*/

#include "emu.h"
#include "strata.h"

#define MAX_STRATA	1

#define FEEPROM_SIZE		0x800000	// 64Mbit
#define BLOCK_SIZE			0x020000

#define BLOCKLOCK_SIZE ((FEEPROM_SIZE/BLOCK_SIZE+7)/8)
#define WRBUF_SIZE 32
#define PROT_REGS_SIZE 18

#define ADDRESS_MASK        0x7fffff
#define BLOCK_ADDRESS_MASK	0x7e0000
#define BLOCK_ADDRESS_SHIFT	17
#define BYTE_ADDRESS_MASK	0x01ffff

enum fm_mode_t
{
	FM_NORMAL,		// normal read/write
	FM_READID,		// read ID
	FM_READQUERY,	// read query
	FM_READSTATUS,	// read status
	FM_WRITEPART1,	// first half of programming, awaiting second
	FM_WRBUFPART1,	// first part of write to buffer, awaiting second
	FM_WRBUFPART2,	// second part of write to buffer, awaiting third
	FM_WRBUFPART3,	// third part of write to buffer, awaiting fourth
	FM_WRBUFPART4,	// fourth part of write to buffer
	FM_CLEARPART1,	// first half of clear, awaiting second
	FM_SETLOCK,		// first half of set master lock/set block lock
	FM_CONFPART1,	// first half of configuration, awaiting second
	FM_WRPROTPART1	// first half of protection program, awaiting second
};
struct strata_t 
{
	fm_mode_t mode;				// current operation mode
	int hard_unlock;	// 1 if RP* pin is at Vhh (not fully implemented)
	int status;			// current status
	int master_lock;	// master lock flag
	offs_t wrbuf_base;	// start address in write buffer command
	int wrbuf_len;		// count converted into byte length in write buffer command
	int wrbuf_count;	// current count in write buffer command
	UINT8 *wrbuf;		// write buffer used by write buffer command
	UINT8 *data_ptr;	// main FEEPROM area
	UINT8 *blocklock;	// block lock flags
	UINT8 *prot_regs;	// protection registers
};

/* accessors for individual block lock flags */
#define READ_BLOCKLOCK(strata, block) (((strata)->blocklock[(block) >> 3] >> ((block) & 7)) & 1)
#define SET_BLOCKLOCK(strata, block) ((strata)->blocklock[(block) >> 3] |= 1 << ((block) & 7))
#define CLEAR_BLOCKLOCK(strata, block) ((strata)->blocklock[(block) >> 3] &= ~(1 << ((block) & 7)))

/*
    Initialize one FEEPROM chip: may be called at driver init or image load
    time (or machine init time if you don't use MESS image core)
*/

INLINE strata_t *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == STRATAFLASH);

	return (strata_t *)downcast<strataflash_device *>(device)->token();
}

static DEVICE_START( strataflash )
{
	strata_t *strata = get_safe_token(device);
	int i;

	strata->mode = FM_NORMAL;
	strata->status = 0x80;
	strata->master_lock = 0;
	strata->data_ptr = auto_alloc_array(device->machine(), UINT8, FEEPROM_SIZE + WRBUF_SIZE + PROT_REGS_SIZE + BLOCKLOCK_SIZE);
	strata->wrbuf = strata->data_ptr + FEEPROM_SIZE;
	strata->prot_regs = strata->wrbuf + WRBUF_SIZE;
	strata->blocklock = strata->prot_regs + PROT_REGS_SIZE;
	/* clear various FEEPROM areas */
	memset(strata->prot_regs, 0xff, 18);
	memset(strata->data_ptr, 0xff, FEEPROM_SIZE);
	memset(strata->blocklock, 0x00, BLOCKLOCK_SIZE);
	/* set-up factory-programmed protection register segment */
	strata->prot_regs[BYTE_XOR_LE(0)] &= 0xfe;
	for (i=2; i<10; i++)
		strata->prot_regs[i] = device->machine().rand();
}

const device_type STRATAFLASH = &device_creator<strataflash_device>;

strataflash_device::strataflash_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, STRATAFLASH, "Intel 28F640J5", tag, owner, clock)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(strata_t));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void strataflash_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void strataflash_device::device_start()
{
	DEVICE_START_NAME( strataflash )(this);
}



/*
    load the FEEPROM contents from file
*/
int strataflash_load(device_t *device, emu_file *file)
{
	strata_t *strata = get_safe_token(device);
	UINT8 buf;
	int i;

	/* version flag */
	if (file->read(& buf, 1) != 1)
		return 1;
	if (buf != 0)
		return 1;

	/* chip state: master lock */
	if (file->read(& buf, 1) != 1)
		return 1;
	strata->master_lock = buf & 1;

	/* main FEEPROM area */
	if (file->read(strata->data_ptr, FEEPROM_SIZE) != FEEPROM_SIZE)
		return 1;
	for (i = 0; i < FEEPROM_SIZE; i += 2)
	{
		UINT16 *ptr = (UINT16 *) (&strata->data_ptr[i]);
		*ptr = LITTLE_ENDIANIZE_INT16(*ptr);
	}

	/* protection registers */
	if (file->read(strata->prot_regs, PROT_REGS_SIZE) != PROT_REGS_SIZE)
		return 1;
	for (i = 0; i < PROT_REGS_SIZE; i += 2)
	{
		UINT16 *ptr = (UINT16 *) (&strata->prot_regs[i]);
		*ptr = LITTLE_ENDIANIZE_INT16(*ptr);
	}

	/* block lock flags */
	if (file->read(strata->blocklock, BLOCKLOCK_SIZE) != BLOCKLOCK_SIZE)
		return 1;

	return 0;
}

/*
    save the FEEPROM contents to file
*/
int strataflash_save(device_t *device, emu_file *file)
{
	strata_t *strata = get_safe_token(device);
	UINT8 buf;
	int i;

	/* version flag */
	buf = 0;
	if (file->write(& buf, 1) != 1)
		return 1;

	/* chip state: lower boot block lockout, higher boot block lockout,
    software data protect */
	buf = strata->master_lock;
	if (file->write(& buf, 1) != 1)
		return 1;

	/* main FEEPROM area */
	for (i = 0; i < FEEPROM_SIZE; i += 2)
	{
		UINT16 *ptr = (UINT16 *) (&strata->data_ptr[i]);
		*ptr = LITTLE_ENDIANIZE_INT16(*ptr);
	}
	if (file->write(strata->data_ptr, FEEPROM_SIZE) != FEEPROM_SIZE)
		return 1;
	for (i = 0; i < FEEPROM_SIZE; i += 2)
	{
		UINT16 *ptr = (UINT16 *) (&strata->data_ptr[i]);
		*ptr = LITTLE_ENDIANIZE_INT16(*ptr);
	}

	/* protection registers */
	for (i = 0; i < PROT_REGS_SIZE; i += 2)
	{
		UINT16 *ptr = (UINT16 *) (&strata->prot_regs[i]);
		*ptr = LITTLE_ENDIANIZE_INT16(*ptr);
	}
	if (file->write(strata->prot_regs, PROT_REGS_SIZE) != PROT_REGS_SIZE)
		return 1;
	for (i = 0; i < PROT_REGS_SIZE; i += 2)
	{
		UINT16 *ptr = (UINT16 *) (&strata->prot_regs[i]);
		*ptr = LITTLE_ENDIANIZE_INT16(*ptr);
	}

	/* block lock flags */
	if (file->write(strata->blocklock, BLOCKLOCK_SIZE) != BLOCKLOCK_SIZE)
		return 1;

	return 0;
}

/* bus width for 8/16-bit handlers */
typedef enum bus_width_t
{
	bw_8,
	bw_16
} bus_width_t;

/*
    read a 8/16-bit word from FEEPROM
*/
static int strataflash_r(device_t *device, offs_t offset, bus_width_t bus_width)
{
	strata_t *strata = get_safe_token(device);

	switch (bus_width)
	{
	case bw_8:
		offset &= ADDRESS_MASK;
		break;
	case bw_16:
		offset &= ADDRESS_MASK & ~1;
	}

	switch (strata->mode)
	{
	default:
	case FM_NORMAL:
		switch (bus_width)
		{
		case bw_8:
			return strata->data_ptr[BYTE_XOR_LE(offset)];
		case bw_16:
			return *(UINT16*)(strata->data_ptr+offset);
		}
		break;
	case FM_READSTATUS:
		return strata->status;
	case FM_WRBUFPART1:
		return 0x80;
	case FM_READID:
		if ((offset >= 0x100) && (offset < 0x112))
		{	/* protection registers */
			switch (bus_width)
			{
			case bw_8:
				return strata->prot_regs[BYTE_XOR_LE(offset)];
			case bw_16:
				return *(UINT16*)(strata->prot_regs+offset);
			}
		}
		else
			switch (offset >> 1)
			{
			case 0:	// maker ID
				return 0x89;	// Intel
			case 1:	// chip ID
				return 0x15;	// 64 Mbit
			default:
				if (((offset & BYTE_ADDRESS_MASK) >> 1) == 2)
				{	// block lock config
					return READ_BLOCKLOCK(strata, offset >> BLOCK_ADDRESS_SHIFT);
				}
				return 0;	// default case
			case 3: // master lock config
				if (strata->master_lock)
					return 1;
				else
					return 0;
			}
		break;
	case FM_READQUERY:
		switch (offset >> 1)
		{
		case 0x00:	// maker ID
			return 0x89;	// Intel
		case 0x01:	// chip ID
			return 0x15;	// 64 Mbit
		default:
			if (((offset & BYTE_ADDRESS_MASK) >> 1) == 2)
			{	// block lock config
				return READ_BLOCKLOCK(strata, offset >> BLOCK_ADDRESS_SHIFT);
			}
			return 0;	// default case
#if 0
		case 0x03: // master lock config
			if (strata->flash_master_lock)
				return 1;
			else
				return 0;
#endif

		/* CFI query identification string */
		case 0x10:
			return 'Q';
		case 0x11:
			return 'R';
		case 0x12:
			return 'Y';
		case 0x13:
			return 0x01;
		case 0x14:
			return 0x00;
		case 0x15:
			return 0x31;
		case 0x16:
			return 0x00;
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1a:
			return 0x00;

		/* system interface information: voltage */
		case 0x1b:
			return 0x45;
		case 0x1c:
			return 0x55;
		case 0x1d:
			return 0x00;
		case 0x1e:
			return 0x00;

		/* system interface information: timings */
		case 0x1f:
			return /*0x07*/0x00;
		case 0x20:
			return /*0x07*/0x00;
		case 0x21:
			return /*0x0a*/0x00;
		case 0x22:
			return 0x00;
		case 0x23:
			return /*0x04*/0x00;
		case 0x24:
			return /*0x04*/0x00;
		case 0x25:
			return /*0x04*/0x00;
		case 0x26:
			return 0x00;

		/* device geometry definition */
		case 0x27:
			return 0x17;
		case 0x28:
			return 0x02;
		case 0x29:
			return 0x00;
		case 0x2a:
			return 0x05;
		case 0x2b:
			return 0x00;
		case 0x2c:
			return 0x01;
		case 0x2d:
			return 0x3f;
		case 0x2e:
			return 0x00;
		case 0x2f:
			return 0x00;
		case 0x30:
			return 0x02;

		/* primary vendor-specific extended query */
		case 0x31:
			return 'P';
		case 0x32:
			return 'R';
		case 0x33:
			return 'I';
		case 0x34:
			return '1';
		case 0x35:
			return '1';
		case 0x36:
			return 0x0a;
		case 0x37:
			return 0x00;
		case 0x38:
			return 0x00;
		case 0x39:
			return 0x00;
		case 0x3a:
			return 0x01;
		case 0x3b:
			return 0x01;
		case 0x3c:
			return 0x00;
		case 0x3d:
			return 0x50;
		case 0x3e:
			return 0x00;
		case 0x3f:
			return 0x00;
		}
		break;
	}

	return 0;
}

/*
    write a 8/16-bit word to FEEPROM
*/
static void strataflash_w(device_t *device, offs_t offset, int data, bus_width_t bus_width)
{
	strata_t *strata = get_safe_token(device);

	switch (bus_width)
	{
	case bw_8:
		offset &= ADDRESS_MASK;
		break;
	case bw_16:
		offset &= ADDRESS_MASK & ~1;
	}

	switch (strata->mode)
	{
	case FM_NORMAL:
	case FM_READID:
	case FM_READQUERY:
	case FM_READSTATUS:
		switch (data)
		{
		case 0xff:	// read array
			strata->mode = FM_NORMAL;
			break;
		case 0x90:	// read identifier codes
			strata->mode = FM_READID;
			break;
		case 0x98:	// read query
			strata->mode = FM_READQUERY;
			break;
		case 0x70:	// read status register
			strata->mode = FM_READSTATUS;
			break;
		case 0x50:	// clear status register
			strata->mode = FM_READSTATUS;
			strata->status &= 0xC5;
			break;
		case 0xe8:	// write to buffer
			strata->mode = FM_WRBUFPART1;
			strata->wrbuf_base = offset & BLOCK_ADDRESS_MASK;
			/*strata->status &= 0xC5;*/
			break;
		case 0x40:
		case 0x10:	// program
			strata->mode = FM_WRITEPART1;
			strata->status &= 0xC5;
			break;
		case 0x20:	// block erase
			strata->mode = FM_CLEARPART1;
			strata->status &= 0xC5;
			break;
		case 0xb0:	// block erase, program suspend
			/* not emulated (erase is instantaneous) */
			break;
		case 0xd0:	// block erase, program resume
			/* not emulated (erase is instantaneous) */
			break;
		case 0xb8:	// configuration
			strata->mode = FM_CONFPART1;
			strata->status &= 0xC5;
			break;
		case 0x60:	// set master lock
			strata->mode = FM_SETLOCK;
			strata->status &= 0xC5;
			break;
		case 0xc0:	// protection program
			strata->mode = FM_WRPROTPART1;
			strata->status &= 0xC5;
			break;
		default:
			logerror("Unknown flash mode byte %x\n", data);
			break;
		}
		break;
	case FM_WRBUFPART1:
		strata->mode = FM_WRBUFPART2;
		if (((offset & BLOCK_ADDRESS_MASK) != strata->wrbuf_base) || (data >= 0x20))
		{
			strata->status |= 0x30;
			strata->wrbuf_len = 0;
			strata->wrbuf_count = data;
		}
		else
		{
			switch (bus_width)
			{
			case bw_8:
				strata->wrbuf_len = data+1;
				break;
			case bw_16:
				strata->wrbuf_len = (data+1) << 1;
				break;
			}
			strata->wrbuf_count = data;
		}
		break;
	case FM_WRBUFPART2:
		strata->mode = FM_WRBUFPART3;
		if (((offset & BLOCK_ADDRESS_MASK) != strata->wrbuf_base)
			|| (((offset & BYTE_ADDRESS_MASK) + strata->wrbuf_len) > BLOCK_SIZE))
		{
			strata->status |= 0x30;
			strata->wrbuf_len = 0;
			strata->wrbuf_base = 0;
		}
		else
			strata->wrbuf_base = offset;
		memset(strata->wrbuf, 0xff, strata->wrbuf_len);	/* right??? */
	case FM_WRBUFPART3:
		if ((offset < strata->wrbuf_base) || (offset >= (strata->wrbuf_base + strata->wrbuf_len)))
			strata->status |= 0x30;
		else
		{
			switch (bus_width)
			{
			case bw_8:
				strata->wrbuf[offset-strata->wrbuf_base] = data;
				break;
			case bw_16:
				strata->wrbuf[offset-strata->wrbuf_base] = data & 0xff;
				strata->wrbuf[offset-strata->wrbuf_base+1] = data >> 8;
				break;
			}
		}
		if (strata->wrbuf_count == 0)
			strata->mode = FM_WRBUFPART4;
		else
			strata->wrbuf_count--;
		break;
	case FM_WRBUFPART4:
		if (((offset & BLOCK_ADDRESS_MASK) != (strata->wrbuf_base & BLOCK_ADDRESS_MASK)) || (data != 0xd0))
		{
			strata->status |= 0x30;
		}
		else if (READ_BLOCKLOCK(strata, offset >> BLOCK_ADDRESS_SHIFT) && !strata->hard_unlock)
		{
			strata->status |= 0x12;
		}
		else if (!(strata->status & 0x30))
		{
			int i;
			for (i=0; i<strata->wrbuf_len; i++)
				strata->data_ptr[BYTE_XOR_LE(strata->wrbuf_base+i)] &= strata->wrbuf[i];
			strata->mode = FM_READSTATUS;
		}
		break;
	case FM_WRITEPART1:
		if (READ_BLOCKLOCK(strata, offset >> BLOCK_ADDRESS_SHIFT) && !strata->hard_unlock)
		{
			strata->status |= 0x12;
		}
		else
		{
			switch (bus_width)
			{
			case bw_8:
				strata->data_ptr[BYTE_XOR_LE(offset)] &= data;
				break;
			case bw_16:
				*(UINT16*)(strata->data_ptr+offset) &= data;
				break;
			}
		}
		strata->mode = FM_READSTATUS;
		break;
	case FM_CLEARPART1:
		if (data == 0xd0)
		{
			// clear the 128k block containing the current address
			// to all 0xffs
			if (READ_BLOCKLOCK(strata, offset >> BLOCK_ADDRESS_SHIFT) && !strata->hard_unlock)
			{
				strata->status |= 0x22;
			}
			else
			{
				offset &= BLOCK_ADDRESS_MASK;
				memset(&strata->data_ptr[offset], 0xff, BLOCK_SIZE);
			}
			strata->mode = FM_READSTATUS;
		}
		break;
	case FM_SETLOCK:
		switch (data)
		{
		case 0xf1:
			if (!strata->hard_unlock)
				strata->status |= 0x12;
			else
				strata->master_lock = 1;
			break;
		case 0x01:
			if (strata->master_lock && !strata->hard_unlock)
				strata->status |= 0x12;
			else
				SET_BLOCKLOCK(strata, offset >> BLOCK_ADDRESS_SHIFT);
			break;
		case 0xd0:
			if (strata->master_lock && !strata->hard_unlock)
				strata->status |= 0x22;
			else
				CLEAR_BLOCKLOCK(strata, offset >> BLOCK_ADDRESS_SHIFT);
			break;
		case 0x03:	// Set Read configuration
			/* ignore command */
			break;
		default:
			strata->status |= 0x30;
			break;
		}
		strata->mode = FM_READSTATUS;
		break;
	case FM_CONFPART1:
		/* configuration register is not emulated because the sts pin is not */
		//strata->configuration = data;
		strata->mode = FM_READSTATUS;	/* right??? */
		break;
	case FM_WRPROTPART1:
		if ((offset < 0x100) || (offset >= 0x112))
			strata->status |= 0x10;
		else if ((offset >= 0x102) && !((strata->prot_regs[BYTE_XOR_LE(0)] >> ((offset - 0x102) >> 3)) & 1))
			strata->status |= 0x12;
		else
		{
			switch (bus_width)
			{
			case bw_8:
				strata->prot_regs[BYTE_XOR_LE(offset-0x100)] &= data;
				break;
			case bw_16:
				*(UINT16*)(strata->prot_regs+(offset-0x100)) &= data;
				break;
			}
		}
		strata->mode = FM_READSTATUS;	/* right??? */
		break;
	}
}

/*
    read a 8-bit word from FEEPROM
*/
UINT8 strataflash_8_r(device_t *device, offs_t offset)
{
	return strataflash_r(device, offset, bw_8);
}

/*
    write a 8-bit word to FEEPROM
*/
void strataflash_8_w(device_t *device, offs_t offset, UINT8 data)
{
	strataflash_w(device, offset, data, bw_8);
}

/*
    read a 16-bit word from FEEPROM
*/
UINT16 strataflash_16_r(device_t *device, offs_t offset)
{
	return strataflash_r(device, offset, bw_16);
}

/*
    write a 16-bit word to FEEPROM
*/
void strataflash_16_w(device_t *device, offs_t offset, UINT16 data)
{
	strataflash_w(device, offset, data, bw_16);
}
