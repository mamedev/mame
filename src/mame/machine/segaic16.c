/***************************************************************************

    Sega 16-bit common hardware

***************************************************************************/

#include "driver.h"
#include "segaic16.h"
#include "video/resnet.h"

extern void *fd1089_get_decrypted_base(void);
extern void fd1094_machine_init(void);
extern void *fd1094_get_decrypted_base(void);


/*************************************
 *
 *  Debugging
 *
 *************************************/

#define LOG_MEMORY_MAP	(1)
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
	int		cpunum;
	const struct segaic16_memory_map_entry *map;
	void	(*sound_w)(UINT8);
	UINT8	(*sound_r)(void);
};


struct multiply_chip
{
	UINT16	regs[4];
};


struct divide_chip
{
	UINT16 	regs[8];
};


struct compare_timer_chip
{
	UINT16	regs[16];
	UINT16	counter;
	UINT8	bit;
	void	(*sound_w)(UINT8);
	void	(*timer_ack)(void);
};



/*************************************
 *
 *  Statics
 *
 *************************************/

static struct memory_mapper_chip memory_mapper;
static struct multiply_chip multiply[3];
static struct divide_chip divide[3];
static struct compare_timer_chip compare_timer[2];



/*************************************
 *
 *  Prototypes
 *
 *************************************/

static void update_memory_mapping(struct memory_mapper_chip *chip);



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
	result = program_read_word_16be(activecpu_get_pc());
	recurse = 0;
	return result;
}



/*************************************
 *
 *  Memory mapping chip
 *
 *************************************/

void segaic16_memory_mapper_init(int cpunum, const struct segaic16_memory_map_entry *entrylist, void (*sound_w_callback)(UINT8), UINT8 (*sound_r_callback)(void))
{
	struct memory_mapper_chip *chip = &memory_mapper;

	/* reset the chip structure */
	memset(chip, 0, sizeof(*chip));
	chip->cpunum = cpunum;
	chip->map = entrylist;
	chip->sound_w = sound_w_callback;
	chip->sound_r = sound_r_callback;

	/* create the initial regions */
	segaic16_memory_mapper_reset();
}


void segaic16_memory_mapper_reset(void)
{
	struct memory_mapper_chip *chip = &memory_mapper;

	/* zap to 0 and remap everything */
	memset(chip->regs, 0, sizeof(chip->regs));
	update_memory_mapping(chip);
}


void segaic16_memory_mapper_config(const UINT8 *map_data)
{
	struct memory_mapper_chip *chip = &memory_mapper;

	/* zap to 0 and remap everything */
	memcpy(&chip->regs[0x10], map_data, 0x10);
	update_memory_mapping(chip);
}


void segaic16_memory_mapper_set_decrypted(UINT8 *decrypted)
{
	struct memory_mapper_chip *chip = &memory_mapper;
	offs_t romsize = memory_region_length(REGION_CPU1 + chip->cpunum);
	int rgnum;

	/* loop over the regions */
	for (rgnum = 0; chip->map[rgnum].regbase != 0; rgnum++)
	{
		static const offs_t region_size_map[4] = { 0x00ffff, 0x01ffff, 0x07ffff, 0x1fffff };
		const struct segaic16_memory_map_entry *rgn = &chip->map[rgnum];
		offs_t region_size = region_size_map[chip->regs[rgn->regbase] & 3];
		offs_t region_base = (chip->regs[rgn->regbase + 1] << 16) & ~region_size;
		offs_t region_start = region_base + (rgn->regoffs & region_size);
		read16_handler read = rgn->read;
		int banknum = 0;

		/* skip non-ROM regions */
		if (read == NULL || rgn->romoffset == ~0)
			continue;

		/* check for mapping to banks */
		if ((FPTR)read >= STATIC_BANK1 && (FPTR)read <= STATIC_BANKMAX)
			banknum = ((FPTR)read - STATIC_BANK1) + 1;

		/* skip any mappings beyond the ROM size */
		if (region_start >= romsize)
			continue;

		memory_configure_bank_decrypted(banknum, 0, 1, decrypted + region_start, 0);
		memory_set_bank(banknum, 0);
	}
}


static void memory_mapper_w(struct memory_mapper_chip *chip, offs_t offset, UINT8 data)
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
				cpunum_set_input_line(chip->cpunum, INPUT_LINE_RESET, (chip->regs[offset] & 3) == 3 ? ASSERT_LINE : CLEAR_LINE);
				if ((chip->regs[offset] & 3) == 3)
					fd1094_machine_init();
			}
			break;

		case 0x03:
			if (chip->sound_w)
				(*chip->sound_w)(data);
			break;

		case 0x04:
			/* controls IRQ lines to 68000, negative logic -- write $B to signal IRQ4 */
			if ((chip->regs[offset] & 7) != 7)
				cpunum_set_input_line(chip->cpunum, (~chip->regs[offset] & 7), HOLD_LINE);
			break;

		case 0x05:
			/* read/write control */
			/*   01 - write data latched in 00,01 to 2 * (address in 0A,0B,0C) */
			/*   02 - read data into latches 00,01 from 2 * (address in 07,08,09) */
			if (data == 0x01)
			{
				offs_t addr = (chip->regs[0x0a] << 17) | (chip->regs[0x0b] << 9) | (chip->regs[0x0c] << 1);
				cpuintrf_push_context(chip->cpunum);
				program_write_word_16be(addr, (chip->regs[0x00] << 8) | chip->regs[0x01]);
				cpuintrf_pop_context();
			}
			else if (data == 0x02)
			{
				offs_t addr = (chip->regs[0x07] << 17) | (chip->regs[0x08] << 9) | (chip->regs[0x09] << 1);
				UINT16 result;
				cpuintrf_push_context(chip->cpunum);
				result = program_read_word_16be(addr);
				cpuintrf_pop_context();
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
				update_memory_mapping(chip);
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
				return (*chip->sound_r)();
			return 0xff;

		default:
			logerror("Unknown memory_mapper_r from address %02X\n", offset);
			break;
	}
	return unmapped_val;
}


static void update_memory_mapping(struct memory_mapper_chip *chip)
{
	int rgnum;

	if (LOG_MEMORY_MAP) mame_printf_debug("----\nRemapping:\n");

	/* first reset everything back to the beginning */
	memory_install_read16_handler (chip->cpunum, ADDRESS_SPACE_PROGRAM, 0x000000, 0xffffff, 0, 0, segaic16_memory_mapper_lsb_r);
	memory_install_write16_handler(chip->cpunum, ADDRESS_SPACE_PROGRAM, 0x000000, 0xffffff, 0, 0, segaic16_memory_mapper_lsb_w);

	/* loop over the regions */
	for (rgnum = 0; chip->map[rgnum].regbase != 0; rgnum++)
	{
		static const offs_t region_size_map[4] = { 0x00ffff, 0x01ffff, 0x07ffff, 0x1fffff };
		const struct segaic16_memory_map_entry *rgn = &chip->map[rgnum];
		offs_t region_size = region_size_map[chip->regs[rgn->regbase] & 3];
		offs_t region_base = (chip->regs[rgn->regbase + 1] << 16) & ~region_size;
		offs_t region_mirror = rgn->mirror & region_size;
		offs_t region_start = region_base + (rgn->regoffs & region_size);
		offs_t region_end = region_start + ((rgn->length - 1 < region_size) ? rgn->length - 1 : region_size);
		write16_handler write = rgn->write;
		read16_handler read = rgn->read;
		int banknum = 0;

		/* check for mapping to banks */
		if ((FPTR)read >= STATIC_BANK1 && (FPTR)read <= STATIC_BANKMAX)
			banknum = ((FPTR)read - STATIC_BANK1) + 1;
		if ((FPTR)write >= STATIC_BANK1 && (FPTR)write <= STATIC_BANKMAX)
			banknum = ((FPTR)write - STATIC_BANK1) + 1;

		/* ROM areas need extra clamping */
		if (rgn->romoffset != ~0)
		{
			offs_t romsize = memory_region_length(REGION_CPU1 + chip->cpunum);
			if (region_start >= romsize)
				read = NULL;
			else if (region_start + rgn->length > romsize)
				region_end = romsize - 1;
		}

		/* map it */
		if (read)
			memory_install_read16_handler(chip->cpunum, ADDRESS_SPACE_PROGRAM, region_start, region_end, 0, region_mirror, read);
		if (write)
			memory_install_write16_handler(chip->cpunum, ADDRESS_SPACE_PROGRAM, region_start, region_end, 0, region_mirror, write);

		/* set the bank pointer */
		if (banknum && read)
		{
			if (rgn->base)
			{
				memory_configure_bank(banknum, 0, 1, *rgn->base, 0);
				memory_set_bank(banknum, 0);
			}
			else if (rgn->romoffset != ~0)
			{
				UINT8 *decrypted;

				decrypted = fd1094_get_decrypted_base();
				if (!decrypted)
					decrypted = fd1089_get_decrypted_base();

				memory_configure_bank(banknum, 0, 1, memory_region(REGION_CPU1 + chip->cpunum) + region_start, 0);
				if (decrypted)
					memory_configure_bank_decrypted(banknum, 0, 1, decrypted ? (decrypted + region_start) : 0, 0);
				memory_set_bank(banknum, 0);
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
	memory_mapper_w(&memory_mapper, offset, data);
}


READ16_HANDLER( segaic16_memory_mapper_lsb_r )
{
	return memory_mapper_r(&memory_mapper, offset, segaic16_open_bus_r(0,0));
}


WRITE16_HANDLER( segaic16_memory_mapper_lsb_w )
{
	if (ACCESSING_LSB)
		memory_mapper_w(&memory_mapper, offset, data & 0xff);
}



/*************************************
 *
 *  Multiply chip
 *
 *************************************/

static UINT16 multiply_r(int which, offs_t offset, UINT16 mem_mask)
{
	offset &= 3;
	switch (offset)
	{
		case 0:	return multiply[which].regs[0];
		case 1:	return multiply[which].regs[1];
		case 2:	return ((INT16)multiply[which].regs[0] * (INT16)multiply[which].regs[1]) >> 16;
		case 3:	return ((INT16)multiply[which].regs[0] * (INT16)multiply[which].regs[1]) & 0xffff;
	}
	return 0xffff;
}


static void multiply_w(int which, offs_t offset, UINT16 data, UINT16 mem_mask)
{
	offset &= 3;
	switch (offset)
	{
		case 0:	COMBINE_DATA(&multiply[which].regs[0]);	break;
		case 1:	COMBINE_DATA(&multiply[which].regs[1]);	break;
		case 2:	COMBINE_DATA(&multiply[which].regs[0]);	break;
		case 3:	COMBINE_DATA(&multiply[which].regs[1]);	break;
	}
}


READ16_HANDLER( segaic16_multiply_0_r )  { return multiply_r(0, offset, mem_mask); }
READ16_HANDLER( segaic16_multiply_1_r )  { return multiply_r(1, offset, mem_mask); }
READ16_HANDLER( segaic16_multiply_2_r )  { return multiply_r(2, offset, mem_mask); }
WRITE16_HANDLER( segaic16_multiply_0_w ) { multiply_w(0, offset, data, mem_mask); }
WRITE16_HANDLER( segaic16_multiply_1_w ) { multiply_w(1, offset, data, mem_mask); }
WRITE16_HANDLER( segaic16_multiply_2_w ) { multiply_w(2, offset, data, mem_mask); }



/*************************************
 *
 *  Divide chip
 *
 *************************************/

static void update_divide(int which, int mode)
{
	/* clear the flags by default */
	divide[which].regs[6] = 0;

	/* if mode 0, store quotient/remainder */
	if (mode == 0)
	{
		INT32 dividend = (INT32)((divide[which].regs[0] << 16) | divide[which].regs[1]);
		INT32 divisor = (INT16)divide[which].regs[2];
		INT32 quotient, remainder;

		/* perform signed divide */
		if (divisor == 0)
		{
			quotient = dividend;//((INT32)(dividend ^ divisor) < 0) ? 0x8000 : 0x7fff;
			divide[which].regs[6] |= 0x4000;
		}
		else
			quotient = dividend / divisor;
		remainder = dividend - quotient * divisor;

		/* clamp to 16-bit signed */
		if (quotient < -32768)
		{
			quotient = -32768;
			divide[which].regs[6] |= 0x8000;
		}
		else if (quotient > 32767)
		{
			quotient = 32767;
			divide[which].regs[6] |= 0x8000;
		}

		/* store quotient and remainder */
		divide[which].regs[4] = quotient;
		divide[which].regs[5] = remainder;
	}

	/* if mode 1, store 32-bit quotient */
	else
	{
		UINT32 dividend = (UINT32)((divide[which].regs[0] << 16) | divide[which].regs[1]);
		UINT32 divisor = (UINT16)divide[which].regs[2];
		UINT32 quotient;

		/* perform unsigned divide */
		if (divisor == 0)
		{
			quotient = dividend;//0x7fffffff;
			divide[which].regs[6] |= 0x4000;
		}
		else
			quotient = dividend / divisor;

		/* store 32-bit quotient */
		divide[which].regs[4] = quotient >> 16;
		divide[which].regs[5] = quotient & 0xffff;
	}
}

static UINT16 divide_r(int which, offs_t offset, UINT16 mem_mask)
{
	/* 8 effective read registers */
	offset &= 7;
	switch (offset)
	{
		case 0:	return divide[which].regs[0];	/* dividend high */
		case 1:	return divide[which].regs[1];	/* dividend low */
		case 2:	return divide[which].regs[2];	/* divisor */
		case 4: return divide[which].regs[4];	/* quotient (mode 0) or quotient high (mode 1) */
		case 5:	return divide[which].regs[5];	/* remainder (mode 0) or quotient low (mode 1) */
		case 6: return divide[which].regs[6];	/* flags */
	}
	return 0xffff;
}


static void divide_w(int which, offs_t offset, UINT16 data, UINT16 mem_mask)
{
	int a4 = offset & 8;
	int a3 = offset & 4;

	if (LOG_DIVIDE) logerror("%06X:divide%d_w(%X) = %04X\n", activecpu_get_pc(), which, offset, data);

	/* only 4 effective write registers */
	offset &= 3;
	switch (offset)
	{
		case 0:	COMBINE_DATA(&divide[which].regs[0]); break;	/* dividend high */
		case 1:	COMBINE_DATA(&divide[which].regs[1]); break;	/* dividend low */
		case 2:	COMBINE_DATA(&divide[which].regs[2]); break;	/* divisor/trigger */
		case 3:	break;
	}

	/* if a4 line is high, divide, using a3 as the mode */
	if (a4) update_divide(which, a3);
}


READ16_HANDLER( segaic16_divide_0_r )  { return divide_r(0, offset, mem_mask); }
READ16_HANDLER( segaic16_divide_1_r )  { return divide_r(1, offset, mem_mask); }
READ16_HANDLER( segaic16_divide_2_r )  { return divide_r(2, offset, mem_mask); }
WRITE16_HANDLER( segaic16_divide_0_w ) { divide_w(0, offset, data, mem_mask); }
WRITE16_HANDLER( segaic16_divide_1_w ) { divide_w(1, offset, data, mem_mask); }
WRITE16_HANDLER( segaic16_divide_2_w ) { divide_w(2, offset, data, mem_mask); }



/*************************************
 *
 *  Compare/timer chip
 *
 *************************************/

void segaic16_compare_timer_init(int which, void (*sound_write_callback)(UINT8), void (*timer_ack_callback)(void))
{
	compare_timer[which].sound_w = sound_write_callback;
	compare_timer[which].timer_ack = timer_ack_callback;
	compare_timer[which].counter = 0;
}


int segaic16_compare_timer_clock(int which)
{
	int old_counter = compare_timer[which].counter;
	int result = 0;

	/* if we're enabled, clock the upcounter */
	if (compare_timer[which].regs[10] & 1)
		compare_timer[which].counter++;

	/* regardless of the enable, a value of 0xfff will generate the IRQ */
	if (old_counter == 0xfff)
	{
		result = 1;
		compare_timer[which].counter = compare_timer[which].regs[8] & 0xfff;
	}
	return result;
}


static void update_compare(int which, int update_history)
{
	int bound1 = (INT16)compare_timer[which].regs[0];
	int bound2 = (INT16)compare_timer[which].regs[1];
	int value = (INT16)compare_timer[which].regs[2];
	int min = (bound1 < bound2) ? bound1 : bound2;
	int max = (bound1 > bound2) ? bound1 : bound2;

	if (value < min)
	{
		compare_timer[which].regs[7] = min;
		compare_timer[which].regs[3] = 0x8000;
	}
	else if (value > max)
	{
		compare_timer[which].regs[7] = max;
		compare_timer[which].regs[3] = 0x4000;
	}
	else
	{
		compare_timer[which].regs[7] = value;
		compare_timer[which].regs[3] = 0x0000;
	}

	if (update_history)
		compare_timer[which].regs[4] |= (compare_timer[which].regs[3] == 0) << compare_timer[which].bit++;
}


static void timer_interrupt_ack(int which)
{
	if (compare_timer[which].timer_ack)
		(*compare_timer[which].timer_ack)();
}


static UINT16 compare_timer_r(int which, offs_t offset, UINT16 mem_mask)
{
	offset &= 0xf;
	if (LOG_COMPARE) logerror("%06X:compare%d_r(%X) = %04X\n", activecpu_get_pc(), which, offset, compare_timer[which].regs[offset]);
	switch (offset)
	{
		case 0x0:	return compare_timer[which].regs[0];
		case 0x1:	return compare_timer[which].regs[1];
		case 0x2:	return compare_timer[which].regs[2];
		case 0x3:	return compare_timer[which].regs[3];
		case 0x4:	return compare_timer[which].regs[4];
		case 0x5:	return compare_timer[which].regs[1];
		case 0x6:	return compare_timer[which].regs[2];
		case 0x7:	return compare_timer[which].regs[7];
		case 0x9:
		case 0xd:	timer_interrupt_ack(which); break;
	}
	return 0xffff;
}


static void compare_timer_w(int which, offs_t offset, UINT16 data, UINT16 mem_mask)
{
	offset &= 0xf;
	if (LOG_COMPARE) logerror("%06X:compare%d_w(%X) = %04X\n", activecpu_get_pc(), which, offset, data);
	switch (offset)
	{
		case 0x0:	COMBINE_DATA(&compare_timer[which].regs[0]); update_compare(which, 0); break;
		case 0x1:	COMBINE_DATA(&compare_timer[which].regs[1]); update_compare(which, 0); break;
		case 0x2:	COMBINE_DATA(&compare_timer[which].regs[2]); update_compare(which, 1); break;
		case 0x4:	compare_timer[which].regs[4] = 0; compare_timer[which].bit = 0; break;
		case 0x6:	COMBINE_DATA(&compare_timer[which].regs[2]); update_compare(which, 0); break;
		case 0x8:
		case 0xc:	COMBINE_DATA(&compare_timer[which].regs[8]); break;
		case 0x9:
		case 0xd:	timer_interrupt_ack(which); break;
		case 0xa:
		case 0xe:	COMBINE_DATA(&compare_timer[which].regs[10]); break;
		case 0xb:
		case 0xf:
			COMBINE_DATA(&compare_timer[which].regs[11]);
			if (compare_timer[which].sound_w)
				(*compare_timer[which].sound_w)(compare_timer[which].regs[11]);
			break;
	}
}


READ16_HANDLER( segaic16_compare_timer_0_r )  { return compare_timer_r(0, offset, mem_mask); }
READ16_HANDLER( segaic16_compare_timer_1_r )  { return compare_timer_r(1, offset, mem_mask); }
WRITE16_HANDLER( segaic16_compare_timer_0_w ) { compare_timer_w(0, offset, data, mem_mask); }
WRITE16_HANDLER( segaic16_compare_timer_1_w ) { compare_timer_w(1, offset, data, mem_mask); }
