/***************************************************************************

    Sega 16-bit common hardware

***************************************************************************/

#include "emu.h"
#include "segaic16.h"
#include "video/resnet.h"
#include "machine/fd1089.h"
#include "includes/segas16.h"		// needed for fd1094 calls


/*************************************
 *
 *  Debugging
 *
 *************************************/

#define LOG_MEMORY_MAP	(0)
#define LOG_MULTIPLY	(0)
#define LOG_DIVIDE		(0)
#define LOG_COMPARE		(0)



/*************************************
 *
 *  Types
 *
 *************************************/

struct memory_mapper_chip
{
	UINT8	regs[0x20];
	device_t *cpu;
	const segaic16_memory_map_entry *map;
	void	(*sound_w)(running_machine &, UINT8);
	UINT8	(*sound_r)(running_machine &);
};


/*************************************
 *
 *  Statics
 *
 *************************************/

static struct memory_mapper_chip memory_mapper;


/*************************************
 *
 *  Prototypes
 *
 *************************************/

static void update_memory_mapping(running_machine &machine, struct memory_mapper_chip *chip, int decrypt);


/*************************************
 *
 *  Misc helpers
 *
 *************************************/

READ16_HANDLER( segaic16_open_bus_r )
{
	static UINT8 recurse = 0;
	UINT16 result;

	/* Unmapped memory returns the last word on the data bus, which is almost always the opcode */
	/* of the next instruction due to prefetch; however, since we may be encrypted, we actually */
	/* need to return the encrypted opcode, not the last decrypted data. */

	/* Believe it or not, this is actually important for Cotton, which has the following evil */
	/* code: btst #0,$7038f7, which tests the low bit of an unmapped address, which thus should */
	/* return the prefetched value. */

	/* prevent recursion */
	if (recurse)
		return 0xffff;

	/* read original encrypted memory at that address */
	recurse = 1;
	result = space->read_word(cpu_get_pc(&space->device()));
	recurse = 0;
	return result;
}



/*************************************
 *
 *  Memory mapping chip
 *
 *************************************/

void segaic16_memory_mapper_init(device_t *cpu, const segaic16_memory_map_entry *entrylist, void (*sound_w_callback)(running_machine &, UINT8), UINT8 (*sound_r_callback)(running_machine &))
{
	struct memory_mapper_chip *chip = &memory_mapper;

	/* reset the chip structure */
	memset(chip, 0, sizeof(*chip));
	chip->cpu = cpu;
	chip->map = entrylist;
	chip->sound_w = sound_w_callback;
	chip->sound_r = sound_r_callback;

	/* create the initial regions */
	update_memory_mapping(cpu->machine(), chip, 0);

	state_save_register_item_array(cpu->machine(), "segaic16_mapper", NULL, 0, chip->regs);
}

void segaic16_memory_mapper_reset(running_machine &machine)
{
	struct memory_mapper_chip *chip = &memory_mapper;

	/* zap to 0 and remap everything */
	memset(chip->regs, 0, sizeof(chip->regs));
	update_memory_mapping(machine, chip, 1);
}


void segaic16_memory_mapper_config(running_machine &machine, const UINT8 *map_data)
{
	struct memory_mapper_chip *chip = &memory_mapper;

	/* zap to 0 and remap everything */
	memcpy(&chip->regs[0x10], map_data, 0x10);
	update_memory_mapping(machine, chip, 1);
}


void segaic16_memory_mapper_set_decrypted(running_machine &machine, UINT8 *decrypted)
{
	struct memory_mapper_chip *chip = &memory_mapper;
	offs_t romsize = chip->cpu->region()->bytes();
	int rgnum;

	/* loop over the regions */
	for (rgnum = 0; chip->map[rgnum].regbase != 0; rgnum++)
	{
		static const offs_t region_size_map[4] = { 0x00ffff, 0x01ffff, 0x07ffff, 0x1fffff };
		const segaic16_memory_map_entry *rgn = &chip->map[rgnum];
		offs_t region_size = region_size_map[chip->regs[rgn->regbase] & 3];
		offs_t region_base = (chip->regs[rgn->regbase + 1] << 16) & ~region_size;
		offs_t region_start = region_base + (rgn->regoffs & region_size);
		const char *readbank = rgn->readbank;

		/* skip non-ROM regions */
		if (readbank == NULL || rgn->romoffset == ~0)
			continue;

		/* skip any mappings beyond the ROM size */
		if (region_start >= romsize)
			continue;

		machine.root_device().membank(readbank)->configure_decrypted_entry(0, decrypted + region_start);
		machine.root_device().membank(readbank)->set_entry(0);
	}
}


static void memory_mapper_w(address_space *space, struct memory_mapper_chip *chip, offs_t offset, UINT8 data)
{
	UINT8 oldval;

	/* wraps every 32 bytes */
	offset &= 0x1f;

	/* remember the previous value and swap in the new one */
	oldval = chip->regs[offset];
	chip->regs[offset] = data;

	/* switch off the offset */
	switch (offset)
	{
		case 0x02:
			/* misc commands */
			/*   00 - resume execution after 03 */
			/*   03 - maybe controls halt and reset lines together? */
			if ((oldval ^ chip->regs[offset]) & 3)
			{
				if ((chip->regs[offset] & 3) == 3)
					fd1094_machine_init(chip->cpu);

				/* fd1094_machine_init calls device_reset on the CPU, so we must do this afterwards */
				device_set_input_line(chip->cpu, INPUT_LINE_RESET, (chip->regs[offset] & 3) == 3 ? ASSERT_LINE : CLEAR_LINE);
			}
			break;

		case 0x03:
			if (chip->sound_w)
				(*chip->sound_w)(space->machine(), data);
			break;

		case 0x04:
			/* controls IRQ lines to 68000, negative logic -- write $B to signal IRQ4 */
			if ((chip->regs[offset] & 7) != 7)
			{
				int irqnum;
				for (irqnum = 0; irqnum < 8; irqnum++)
					device_set_input_line(chip->cpu, irqnum, (irqnum == (~chip->regs[offset] & 7)) ? HOLD_LINE : CLEAR_LINE);
			}
			break;

		case 0x05:
			/* read/write control */
			/*   01 - write data latched in 00,01 to 2 * (address in 0A,0B,0C) */
			/*   02 - read data into latches 00,01 from 2 * (address in 07,08,09) */
			if (data == 0x01)
			{
				address_space *targetspace = chip->cpu->memory().space(AS_PROGRAM);
				offs_t addr = (chip->regs[0x0a] << 17) | (chip->regs[0x0b] << 9) | (chip->regs[0x0c] << 1);
				targetspace->write_word(addr, (chip->regs[0x00] << 8) | chip->regs[0x01]);
			}
			else if (data == 0x02)
			{
				address_space *targetspace = chip->cpu->memory().space(AS_PROGRAM);
				offs_t addr = (chip->regs[0x07] << 17) | (chip->regs[0x08] << 9) | (chip->regs[0x09] << 1);
				UINT16 result;
				result = targetspace->read_word(addr);
				chip->regs[0x00] = result >> 8;
				chip->regs[0x01] = result;
			}
			break;

		case 0x07:	case 0x08:	case 0x09:
			/* writes here latch a 68000 address for writing */
			break;

		case 0x0a:	case 0x0b:	case 0x0c:
			/* writes here latch a 68000 address for reading */
			break;

		case 0x10:	case 0x11:
		case 0x12:	case 0x13:
		case 0x14:	case 0x15:
		case 0x16:	case 0x17:
		case 0x18:	case 0x19:
		case 0x1a:	case 0x1b:
		case 0x1c:	case 0x1d:
		case 0x1e:	case 0x1f:
			if (oldval != data)
				update_memory_mapping(space->machine(), chip, 1);
			break;

		default:
			logerror("Unknown memory_mapper_w to address %02X = %02X\n", offset, data);
			break;
	}
}


static UINT16 memory_mapper_r(struct memory_mapper_chip *chip, offs_t offset, UINT16 unmapped_val)
{
	/* wraps every 32 bytes */
	offset &= 0x1f;

	/* switch off the offset */
	switch (offset)
	{
		case 0x00:
		case 0x01:
			/* data latches - return the values latched */
			return chip->regs[offset];

		case 0x02:
			/* various input bits from the 68000 */
			/*   01 - ???? */
			/*   02 - ???? */
			/*   04 - ???? */
			/*   08 - ???? */
			/*   40 - set if busy processing a read/write request */
			/* Together, 01+02 == 00 if the 68000 is halted */
			/* Together, 01+02+04+08 == 0F if the 68000 is executing */
			return (chip->regs[0x02] & 3) == 3 ? 0x00 : 0x0f;

		case 0x03:
			/* this returns data that the sound CPU writes */
			if (chip->sound_r)
				return (*chip->sound_r)(chip->cpu->machine());
			return 0xff;

		default:
			logerror("Unknown memory_mapper_r from address %02X\n", offset);
			break;
	}
	return unmapped_val;
}


static void update_memory_mapping(running_machine &machine, struct memory_mapper_chip *chip, int decrypt)
{
	int rgnum;
	address_space *space = chip->cpu->memory().space(AS_PROGRAM);

	if (LOG_MEMORY_MAP) mame_printf_debug("----\nRemapping:\n");

	/* first reset everything back to the beginning */
	space->install_legacy_readwrite_handler(0x000000, 0xffffff, FUNC(segaic16_memory_mapper_lsb_r), FUNC(segaic16_memory_mapper_lsb_w));

	/* loop over the regions */
	for (rgnum = 0; chip->map[rgnum].regbase != 0; rgnum++)
	{
		static const offs_t region_size_map[4] = { 0x00ffff, 0x01ffff, 0x07ffff, 0x1fffff };
		const segaic16_memory_map_entry *rgn = &chip->map[rgnum];
		offs_t region_size = region_size_map[chip->regs[rgn->regbase] & 3];
		offs_t region_base = (chip->regs[rgn->regbase + 1] << 16) & ~region_size;
		offs_t region_mirror = rgn->mirror & region_size;
		offs_t region_start = region_base + (rgn->regoffs & region_size);
		offs_t region_end = region_start + ((rgn->length - 1 < region_size) ? rgn->length - 1 : region_size);
		const char *writebank = rgn->writebank;
		write16_space_func write = rgn->write;
		const char *writename = rgn->writename;
		const char *readbank = rgn->readbank;
		read16_space_func read = rgn->read;
		const char *readname = rgn->readname;

		/* ROM areas need extra clamping */
		if (rgn->romoffset != ~0)
		{
			offs_t romsize = chip->cpu->region()->bytes();
			if (region_start >= romsize)
				read = NULL, readname = "(null)";
			else if (region_start + rgn->length > romsize)
				region_end = romsize - 1;
		}

		/* map it */
		if (read != NULL)
			space->install_legacy_read_handler(region_start, region_end, 0, region_mirror, read, readname);
		else if (readbank != NULL)
			space->install_read_bank(region_start, region_end, 0, region_mirror, readbank);
		else
			space->install_legacy_read_handler(region_start, region_end, 0, region_mirror, FUNC(segaic16_open_bus_r));

		if (write != NULL)
			space->install_legacy_write_handler(region_start, region_end, 0, region_mirror, write, writename);
		else if (writebank != NULL)
			space->install_write_bank(region_start, region_end, 0, region_mirror, writebank);
		else
			space->unmap_write(region_start, region_end, 0, region_mirror);

		/* set the bank pointer */
		if (readbank != NULL)
		{
			if (rgn->base != NULL)
			{
				machine.root_device().membank(readbank)->configure_entry(0, *rgn->base);
				machine.root_device().membank(readbank)->set_entry(0);
			}
			else if (rgn->romoffset != ~0)
			{
				UINT8 *decrypted = NULL;

				if (decrypt)
				{
					decrypted = (UINT8 *)fd1094_get_decrypted_base();
					if (!decrypted)
						decrypted = (UINT8 *)fd1089_get_decrypted_base();
				}

				machine.root_device().membank(readbank)->configure_entry(0, chip->cpu->region()->base() + region_start);
				if (decrypted)
					machine.root_device().membank(readbank)->configure_decrypted_entry(0, decrypted + region_start);

				machine.root_device().membank(readbank)->set_entry(0);
			}
		}

		if (LOG_MEMORY_MAP) mame_printf_debug("  %06X-%06X (%06X) = %s\n", region_start, region_end, region_mirror, rgn->name);
	}
}


READ8_HANDLER( segaic16_memory_mapper_r )
{
	return memory_mapper_r(&memory_mapper, offset, 0xff);
}


WRITE8_HANDLER( segaic16_memory_mapper_w )
{
	memory_mapper_w(space, &memory_mapper, offset, data);
}


READ16_HANDLER( segaic16_memory_mapper_lsb_r )
{
	return memory_mapper_r(&memory_mapper, offset, segaic16_open_bus_r(space,0,0xffff));
}


WRITE16_HANDLER( segaic16_memory_mapper_lsb_w )
{
	if (ACCESSING_BITS_0_7)
		memory_mapper_w(space, &memory_mapper, offset, data & 0xff);
}


/*************************************
 *
 *  Multiply chip - 315-5248
 *
 *************************************/

typedef struct _ic_315_5248_state ic_315_5248_state ;
struct _ic_315_5248_state
{
	UINT16	regs[4];
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE ic_315_5248_state *_315_5248_get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == _315_5248);

	return (ic_315_5248_state *)downcast<legacy_device_base *>(device)->token();
}

/*****************************************************************************
    IMPLEMENTATION
*****************************************************************************/

READ16_DEVICE_HANDLER ( segaic16_multiply_r )
{
	ic_315_5248_state *ic_315_5248 = _315_5248_get_safe_token(device);

	offset &= 3;
	switch (offset)
	{
		case 0:	return ic_315_5248->regs[0];
		case 1:	return ic_315_5248->regs[1];
		case 2:	return ((INT16)ic_315_5248->regs[0] * (INT16)ic_315_5248->regs[1]) >> 16;
		case 3:	return ((INT16)ic_315_5248->regs[0] * (INT16)ic_315_5248->regs[1]) & 0xffff;
	}
	return 0xffff;
}


WRITE16_DEVICE_HANDLER( segaic16_multiply_w )
{
	ic_315_5248_state *ic_315_5248 = _315_5248_get_safe_token(device);

	offset &= 3;
	switch (offset)
	{
		case 0:	COMBINE_DATA(&ic_315_5248->regs[0]);	break;
		case 1:	COMBINE_DATA(&ic_315_5248->regs[1]);	break;
		case 2:	COMBINE_DATA(&ic_315_5248->regs[0]);	break;
		case 3:	COMBINE_DATA(&ic_315_5248->regs[1]);	break;
	}
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( ic_315_5248 )
{
	ic_315_5248_state *ic_315_5248 = _315_5248_get_safe_token(device);

	device->save_item(NAME(ic_315_5248->regs));
}

static DEVICE_RESET( ic_315_5248 )
{
	ic_315_5248_state *ic_315_5248 = _315_5248_get_safe_token(device);
	int i;

	for (i = 0; i < 4; i++)
		ic_315_5248->regs[i] = 0;
}

/*************************************
 *
 *  Divide chip - 315-5249
 *
 *************************************/

typedef struct _ic_315_5249_state ic_315_5249_state ;
struct _ic_315_5249_state
{
	UINT16	regs[8];
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE ic_315_5249_state *_315_5249_get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == _315_5249);

	return (ic_315_5249_state *)downcast<legacy_device_base *>(device)->token();
}

/*****************************************************************************
    IMPLEMENTATION
*****************************************************************************/

static void update_divide( device_t *device, int mode )
{
	ic_315_5249_state *ic_315_5249 = _315_5249_get_safe_token(device);

	/* clear the flags by default */
	ic_315_5249->regs[6] = 0;

	/* if mode 0, store quotient/remainder */
	if (mode == 0)
	{
		INT32 dividend = (INT32)((ic_315_5249->regs[0] << 16) | ic_315_5249->regs[1]);
		INT32 divisor = (INT16)ic_315_5249->regs[2];
		INT32 quotient, remainder;

		/* perform signed divide */
		if (divisor == 0)
		{
			quotient = dividend;//((INT32)(dividend ^ divisor) < 0) ? 0x8000 : 0x7fff;
			ic_315_5249->regs[6] |= 0x4000;
		}
		else
			quotient = dividend / divisor;

		remainder = dividend - quotient * divisor;

		/* clamp to 16-bit signed */
		if (quotient < -32768)
		{
			quotient = -32768;
			ic_315_5249->regs[6] |= 0x8000;
		}
		else if (quotient > 32767)
		{
			quotient = 32767;
			ic_315_5249->regs[6] |= 0x8000;
		}

		/* store quotient and remainder */
		ic_315_5249->regs[4] = quotient;
		ic_315_5249->regs[5] = remainder;
	}

	/* if mode 1, store 32-bit quotient */
	else
	{
		UINT32 dividend = (UINT32)((ic_315_5249->regs[0] << 16) | ic_315_5249->regs[1]);
		UINT32 divisor = (UINT16)ic_315_5249->regs[2];
		UINT32 quotient;

		/* perform unsigned divide */
		if (divisor == 0)
		{
			quotient = dividend;//0x7fffffff;
			ic_315_5249->regs[6] |= 0x4000;
		}
		else
			quotient = dividend / divisor;

		/* store 32-bit quotient */
		ic_315_5249->regs[4] = quotient >> 16;
		ic_315_5249->regs[5] = quotient & 0xffff;
	}
}

READ16_DEVICE_HANDLER ( segaic16_divide_r )
{
	ic_315_5249_state *ic_315_5249 = _315_5249_get_safe_token(device);

	/* 8 effective read registers */
	offset &= 7;
	switch (offset)
	{
		case 0:	return ic_315_5249->regs[0];	/* dividend high */
		case 1:	return ic_315_5249->regs[1];	/* dividend low */
		case 2:	return ic_315_5249->regs[2];	/* divisor */
		case 4: 	return ic_315_5249->regs[4];	/* quotient (mode 0) or quotient high (mode 1) */
		case 5:	return ic_315_5249->regs[5];	/* remainder (mode 0) or quotient low (mode 1) */
		case 6: 	return ic_315_5249->regs[6];	/* flags */
	}

	return 0xffff;
}


WRITE16_DEVICE_HANDLER( segaic16_divide_w )
{
	ic_315_5249_state *ic_315_5249 = _315_5249_get_safe_token(device);
	int a4 = offset & 8;
	int a3 = offset & 4;

	if (LOG_DIVIDE) logerror("divide_w(%X) = %04X\n", offset, data);

	/* only 4 effective write registers */
	offset &= 3;
	switch (offset)
	{
		case 0:	COMBINE_DATA(&ic_315_5249->regs[0]); break;	/* dividend high */
		case 1:	COMBINE_DATA(&ic_315_5249->regs[1]); break;	/* dividend low */
		case 2:	COMBINE_DATA(&ic_315_5249->regs[2]); break;	/* divisor/trigger */
		case 3:	break;
	}

	/* if a4 line is high, divide, using a3 as the mode */
	if (a4) update_divide(device, a3);
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( ic_315_5249 )
{
	ic_315_5249_state *ic_315_5249 = _315_5249_get_safe_token(device);

	device->save_item(NAME(ic_315_5249->regs));
}

static DEVICE_RESET( ic_315_5249 )
{
	ic_315_5249_state *ic_315_5249 = _315_5249_get_safe_token(device);
	int i;

	for (i = 0; i < 8; i++)
		ic_315_5249->regs[i] = 0;
}

/*************************************
 *
 *  Compare/timer chip - 315-5250
 *
 *************************************/

typedef struct _ic_315_5250_state ic_315_5250_state ;
struct _ic_315_5250_state
{
	UINT16	regs[16];
	UINT16	counter;
	UINT8	      bit;
	_315_5250_sound_callback       sound_w;
	_315_5250_timer_ack_callback   timer_ack;
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE ic_315_5250_state *_315_5250_get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == _315_5250);

	return (ic_315_5250_state *)downcast<legacy_device_base *>(device)->token();
}

INLINE const ic_315_5250_interface *_315_5250_get_interface( device_t *device )
{
	assert(device != NULL);
	assert((device->type() == _315_5250));
	return (const ic_315_5250_interface *) device->static_config();
}

/*****************************************************************************
    IMPLEMENTATION
*****************************************************************************/

int segaic16_compare_timer_clock( device_t *device )
{
	ic_315_5250_state *ic_315_5250 = _315_5250_get_safe_token(device);
	int old_counter = ic_315_5250->counter;
	int result = 0;

	/* if we're enabled, clock the upcounter */
	if (ic_315_5250->regs[10] & 1)
		ic_315_5250->counter++;

	/* regardless of the enable, a value of 0xfff will generate the IRQ */
	if (old_counter == 0xfff)
	{
		result = 1;
		ic_315_5250->counter = ic_315_5250->regs[8] & 0xfff;
	}
	return result;
}


static void update_compare( device_t *device, int update_history )
{
	ic_315_5250_state *ic_315_5250 = _315_5250_get_safe_token(device);
	int bound1 = (INT16)ic_315_5250->regs[0];
	int bound2 = (INT16)ic_315_5250->regs[1];
	int value = (INT16)ic_315_5250->regs[2];
	int min = (bound1 < bound2) ? bound1 : bound2;
	int max = (bound1 > bound2) ? bound1 : bound2;

	if (value < min)
	{
		ic_315_5250->regs[7] = min;
		ic_315_5250->regs[3] = 0x8000;
	}
	else if (value > max)
	{
		ic_315_5250->regs[7] = max;
		ic_315_5250->regs[3] = 0x4000;
	}
	else
	{
		ic_315_5250->regs[7] = value;
		ic_315_5250->regs[3] = 0x0000;
	}

	if (update_history)
		ic_315_5250->regs[4] |= (ic_315_5250->regs[3] == 0) << ic_315_5250->bit++;
}


static void timer_interrupt_ack( device_t *device )
{
	ic_315_5250_state *ic_315_5250 = _315_5250_get_safe_token(device);

	if (ic_315_5250->timer_ack)
		(*ic_315_5250->timer_ack)(device->machine());
}


READ16_DEVICE_HANDLER ( segaic16_compare_timer_r )
{
	ic_315_5250_state *ic_315_5250 = _315_5250_get_safe_token(device);

	offset &= 0xf;
	if (LOG_COMPARE) logerror("compare_r(%X) = %04X\n", offset, ic_315_5250->regs[offset]);
	switch (offset)
	{
		case 0x0:	return ic_315_5250->regs[0];
		case 0x1:	return ic_315_5250->regs[1];
		case 0x2:	return ic_315_5250->regs[2];
		case 0x3:	return ic_315_5250->regs[3];
		case 0x4:	return ic_315_5250->regs[4];
		case 0x5:	return ic_315_5250->regs[1];
		case 0x6:	return ic_315_5250->regs[2];
		case 0x7:	return ic_315_5250->regs[7];
		case 0x9:
		case 0xd:	timer_interrupt_ack(device); break;
	}
	return 0xffff;
}


WRITE16_DEVICE_HANDLER ( segaic16_compare_timer_w )
{
	ic_315_5250_state *ic_315_5250 = _315_5250_get_safe_token(device);

	offset &= 0xf;
	if (LOG_COMPARE) logerror("compare_w(%X) = %04X\n", offset, data);
	switch (offset)
	{
		case 0x0:	COMBINE_DATA(&ic_315_5250->regs[0]); update_compare(device, 0); break;
		case 0x1:	COMBINE_DATA(&ic_315_5250->regs[1]); update_compare(device, 0); break;
		case 0x2:	COMBINE_DATA(&ic_315_5250->regs[2]); update_compare(device, 1); break;
		case 0x4:	ic_315_5250->regs[4] = 0; ic_315_5250->bit = 0; break;
		case 0x6:	COMBINE_DATA(&ic_315_5250->regs[2]); update_compare(device, 0); break;
		case 0x8:
		case 0xc:	COMBINE_DATA(&ic_315_5250->regs[8]); break;
		case 0x9:
		case 0xd:	timer_interrupt_ack(device); break;
		case 0xa:
		case 0xe:	COMBINE_DATA(&ic_315_5250->regs[10]); break;
		case 0xb:
		case 0xf:
			COMBINE_DATA(&ic_315_5250->regs[11]);
			if (ic_315_5250->sound_w)
				(*ic_315_5250->sound_w)(device->machine(), ic_315_5250->regs[11]);
			break;
	}
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( ic_315_5250 )
{
	ic_315_5250_state *ic_315_5250 = _315_5250_get_safe_token(device);
	const ic_315_5250_interface *intf = _315_5250_get_interface(device);

	ic_315_5250->sound_w = intf->sound_write_callback;
	ic_315_5250->timer_ack = intf->timer_ack_callback;

	device->save_item(NAME(ic_315_5250->counter));
	device->save_item(NAME(ic_315_5250->bit));
	device->save_item(NAME(ic_315_5250->regs));
}

static DEVICE_RESET( ic_315_5250 )
{
	ic_315_5250_state *ic_315_5250 = _315_5250_get_safe_token(device);

	memset(&ic_315_5250, 0, sizeof(ic_315_5250));
}



DEVICE_GET_INFO( ic_315_5248 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(ic_315_5248_state);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(ic_315_5248);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(ic_315_5248);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Sega 315-5248");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Sega Custom IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}

DEVICE_GET_INFO( ic_315_5249 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(ic_315_5249_state);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(ic_315_5249);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(ic_315_5249);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Sega 315-5249");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Sega Custom IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}

DEVICE_GET_INFO( ic_315_5250 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(ic_315_5250_state);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(ic_315_5250);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(ic_315_5250);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Sega 315-5250");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Sega Custom IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}


DEFINE_LEGACY_DEVICE(_315_5248, ic_315_5248);
DEFINE_LEGACY_DEVICE(_315_5249, ic_315_5249);
DEFINE_LEGACY_DEVICE(_315_5250, ic_315_5250);
