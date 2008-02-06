/**********************************************************************

    Z80 DMA interface and emulation

    For datasheet http://www.zilog.com/docs/z80/ps0179.pdf

    2008/01     couriersud

        - architecture copied from 8257 DMA
        - significant changes to implementation
        - This is only a minimum implementation to support dkong3 and mario drivers
        - Only memory to memory is tested!

    TODO:
        - implement missing features
        - implement interrupt support (not used in dkong3 and mario)
        - implement more asserts
        - implement a INPUT_LINE_BUSREQ for Z80. As a workaround,
          HALT is used. This implies burst mode.

**********************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "memconv.h"
#include "z80dma.h"

#define VERBOSE 0

#define LOG(x)	do { if (VERBOSE) printf x; } while (0)


#define REGNUM(_m, _s) (((_m)<<3) + (_s))
#define GET_REGNUM(_c, _r) (&(_r) - &(WR0(_c)))
#define REG(_c, _m, _s) (_c)->regs[REGNUM(_m,_s)]
#define WR0(_c)		REG(_c, 0, 0)
#define WR1(_c)		REG(_c, 1, 0)
#define WR2(_c)		REG(_c, 2, 0)
#define WR3(_c)		REG(_c, 3, 0)
#define WR4(_c)		REG(_c, 4, 0)
#define WR5(_c)		REG(_c, 5, 0)
#define WR6(_c)		REG(_c, 6, 0)

#define PORTA_ADDRESS_L(_c)	REG(_c,0,1)
#define PORTA_ADDRESS_H(_c)	REG(_c,0,2)

#define BLOCKLEN_L(_c)		REG(_c,0,3)
#define BLOCKLEN_H(_c)		REG(_c,0,4)

#define PORTA_TIMING(_c)	REG(_c,1,1)
#define PORTB_TIMING(_c)	REG(_c,2,1)

#define MASK_BYTE(_c)		REG(_c,3,1)
#define MATCH_BYTE(_c)		REG(_c,3,2)

#define PORTB_ADDRESS_L(_c)	REG(_c,4,1)
#define PORTB_ADDRESS_H(_c)	REG(_c,4,2)
#define INTERRUPT_CTRL(_c)	REG(_c,4,3)
#define INTERRUPT_VECTOR(_c)	REG(_c,4,4)
#define PULSE_CTRL(_c)		REG(_c,4,5)

#define READ_MASK(_c)		REG(_c,6,1)


#define PORTA_ADDRESS(_c)	((PORTA_ADDRESS_H(_c)<<8) | PORTA_ADDRESS_L(_c))
#define PORTB_ADDRESS(_c)	((PORTB_ADDRESS_H(_c)<<8) | PORTB_ADDRESS_L(_c))
#define BLOCKLEN(_c)		((BLOCKLEN_H(_c)<<8) | BLOCKLEN_L(_c))

#define PORTA_STEP(_c)		(((WR1(_c) >> 4) & 0x03)*2-1)
#define PORTB_STEP(_c)		(((WR2(_c) >> 4) & 0x03)*2-1)
#define PORTA_FIXED(_c)		(((WR1(_c) >> 4) & 0x02) == 0x02)
#define PORTB_FIXED(_c)		(((WR2(_c) >> 4) & 0x02) == 0x02)
#define PORTA_MEMORY(_c)	(((WR1(_c) >> 3) & 0x01) == 0x00)
#define PORTB_MEMORY(_c)	(((WR2(_c) >> 3) & 0x01) == 0x00)

#define PORTA_CYCLE_LEN(_c)	(4-(PORTA_TIMING(_c) & 0x03))
#define PORTB_CYCLE_LEN(_c)	(4-(PORTB_TIMING(_c) & 0x03))

#define PORTA_IS_SOURCE(_c)	((WR0(_c) >> 2) & 0x01)
#define PORTB_IS_SOURCE(_c)	(!PORTA_IS_SOURCE(_c))
#define TRANSFER_MODE(_c)	(WR0(_c) & 0x03)

#define TM_TRANSFER		(0x01)
#define TM_SEARCH		(0x02)
#define TM_SEARCH_TRANSFER	(0x03)

#define READY_ACTIVE_HIGH(_c) ((WR5(_c)>>3) & 0x01)

struct z80dma
{
	const struct z80dma_interface *intf;
	emu_timer *timer;

	UINT16 	regs[REGNUM(6,1)+1];
	UINT8 	num_follow;
	UINT8	cur_follow;
	UINT8 	regs_follow[4];
	UINT8	status;
	UINT8	dma_enabled;

	UINT16 addressA;
	UINT16 addressB;
	UINT16 count;

	UINT8 rdy;

	UINT8 is_read;
	UINT8 cur_cycle;
	UINT8 latch;
};

static struct z80dma *dma;
static int dma_count;

static TIMER_CALLBACK( z80dma_timerproc );
static void z80dma_update_status(int which);

/* ----------------------------------------------------------------------- */

int z80dma_init(int count)
{
	int which;

	dma = auto_malloc(count * sizeof(struct z80dma));
	memset(dma, 0, count * sizeof(struct z80dma));
	dma_count = count;

	for (which = 0; which < dma_count; which++)
	{
		dma[which].timer = timer_alloc(z80dma_timerproc, NULL);

		state_save_register_item_array("Z80DMA", which, dma[which].regs);
		state_save_register_item_array("Z80DMA", which, dma[which].regs_follow);

		state_save_register_item("Z80DMA", which, dma[which].num_follow);
		state_save_register_item("Z80DMA", which, dma[which].cur_follow);
		state_save_register_item("Z80DMA", which, dma[which].status);
		state_save_register_item("Z80DMA", which, dma[which].dma_enabled);

		state_save_register_item("Z80DMA", which, dma[which].addressA);
		state_save_register_item("Z80DMA", which, dma[which].addressB);
		state_save_register_item("Z80DMA", which, dma[which].count);
		state_save_register_item("Z80DMA", which, dma[which].rdy);
		state_save_register_item("Z80DMA", which, dma[which].is_read);
		state_save_register_item("Z80DMA", which, dma[which].cur_cycle);
		state_save_register_item("Z80DMA", which, dma[which].latch);

	}

	return 0;
}



void z80dma_config(int which, const struct z80dma_interface *intf)
{
	dma[which].intf = intf;
}



void z80dma_reset(void)
{
	int which;

	for (which = 0; which < dma_count; which++)
	{
		dma[which].status = 0;
		dma[which].rdy = 0;
		dma[which].num_follow = 0;
		dma[which].dma_enabled = 0;
		z80dma_update_status(which);
	}
}



/* ----------------------------------------------------------------------- */

static void z80dma_do_read(int which)
{
	struct z80dma	*cntx = &dma[which];
	UINT8 mode;

	mode = TRANSFER_MODE(cntx);
	switch(mode) {
		case TM_TRANSFER:
			if (PORTA_IS_SOURCE(cntx))
			{
				if (PORTA_MEMORY(cntx))
					cntx->latch = cntx->intf->memory_read_func(cntx->addressA);
				else
					cntx->latch = cntx->intf->portA_read_func(cntx->addressA);
				cntx->addressA += PORTA_STEP(cntx);
			}
			else
			{
				if (PORTB_MEMORY(cntx))
					cntx->latch = cntx->intf->memory_read_func(cntx->addressB);
				else
					cntx->latch = cntx->intf->portB_read_func(cntx->addressB);
				cntx->addressB += PORTB_STEP(cntx);
			}
			break;

		default:
			fatalerror("z80dma_do_operation: invalid mode %d!\n", mode);
			break;
	}
}

static int z80dma_do_write(int which)
{
	struct z80dma	*cntx = &dma[which];
	int done;
	UINT8 mode;

	mode = TRANSFER_MODE(cntx);
	if (cntx->count == 0x0000)
	{
		//FIXME: Any signal here
	}
	switch(mode) {
		case TM_TRANSFER:
			if (PORTA_IS_SOURCE(cntx))
			{
				if (PORTB_MEMORY(cntx))
					cntx->intf->memory_write_func(cntx->addressB, cntx->latch);
				else
					cntx->intf->portB_write_func(cntx->addressB, cntx->latch);
				cntx->addressB += PORTB_STEP(cntx);
			}
			else
			{
				if (PORTA_MEMORY(cntx))
					cntx->intf->memory_write_func(cntx->addressA, cntx->latch);
				else
					cntx->intf->portB_write_func(cntx->addressA, cntx->latch);
				cntx->addressA += PORTA_STEP(cntx);
			}
			cntx->count--;
			done = (cntx->count == 0xFFFF);
			break;

		default:
			fatalerror("z80dma_do_operation: invalid mode %d!\n", mode);
			break;
	}
	if (done)
	{
		//FIXME: interrupt ?
	}
	return done;
}

static TIMER_CALLBACK( z80dma_timerproc )
{
	int which = param;
	struct z80dma	*cntx = &dma[which];
	int done;

	if (--cntx->cur_cycle)
	{
		return;
	}
	if (cntx->is_read)
	{
		z80dma_do_read(which);
		done = 0;
		cntx->is_read = 0;
		cntx->cur_cycle = (PORTA_IS_SOURCE(cntx) ? PORTA_CYCLE_LEN(cntx) : PORTB_CYCLE_LEN(cntx));
	}
	else
	{
		done = z80dma_do_write(which);
		cntx->is_read = 1;
		cntx->cur_cycle = (PORTB_IS_SOURCE(cntx) ? PORTA_CYCLE_LEN(cntx) : PORTB_CYCLE_LEN(cntx));
	}
	if (done)
	{
		cntx->dma_enabled = 0; //FIXME: Correct?
		z80dma_update_status(which);
	}
}

static void z80dma_update_status(int which)
{
	struct z80dma	*cntx = &dma[which];
	UINT16 pending_transfer;
	attotime next;

	/* no transfer is active right now; is there a transfer pending right now? */
	pending_transfer = dma[which].rdy & dma[which].dma_enabled;

	if (pending_transfer)
	{
		dma[which].is_read = 1;
		dma[which].cur_cycle = (PORTA_IS_SOURCE(cntx) ? PORTA_CYCLE_LEN(cntx) : PORTB_CYCLE_LEN(cntx));
		next = ATTOTIME_IN_HZ(dma[which].intf->clockhz);
		timer_adjust_periodic(dma[which].timer,
			attotime_zero,
			which,
			/* 1 byte transferred in 4 clock cycles */
			next);
	}
	else
	{
		/* no transfers active right now */
		timer_reset(dma[which].timer, attotime_never);
	}

	/* set the halt line */
	if (dma[which].intf && dma[which].intf->cpunum >= 0)
	{
		//FIXME: Synchronization is done by BUSREQ!
		cpunum_set_input_line(Machine, dma[which].intf->cpunum, INPUT_LINE_HALT,
			pending_transfer ? ASSERT_LINE : CLEAR_LINE);
	}
}

/* ----------------------------------------------------------------------- */

static UINT8 z80dma_read(int which, offs_t offset)
{
	fatalerror("z80dma_read: not implemented");
	return 0;
}



static void z80dma_write(int which, offs_t offset, UINT8 data)
{
	struct z80dma	*cntx = &dma[which];

	if (cntx->num_follow == 0)
	{
		if ((data & 0x87) == 0) // WR2
		{
			WR2(cntx) = data;
			if (data & 0x40)
				cntx->regs_follow[cntx->num_follow++] = GET_REGNUM(cntx, PORTB_TIMING(cntx));
		}
		else if ((data & 0x87) == 0x04) // WR1
		{
			WR1(cntx) = data;
			if (data & 0x40)
				cntx->regs_follow[cntx->num_follow++] = GET_REGNUM(cntx, PORTA_TIMING(cntx));
		}
		else if ((data & 0x80) == 0) // WR0
		{
			WR0(cntx) = data;
			if (data & 0x08)
				cntx->regs_follow[cntx->num_follow++] = GET_REGNUM(cntx, PORTA_ADDRESS_L(cntx));
			if (data & 0x10)
				cntx->regs_follow[cntx->num_follow++] = GET_REGNUM(cntx, PORTA_ADDRESS_H(cntx));
			if (data & 0x20)
				cntx->regs_follow[cntx->num_follow++] = GET_REGNUM(cntx, BLOCKLEN_L(cntx));
			if (data & 0x40)
				cntx->regs_follow[cntx->num_follow++] = GET_REGNUM(cntx, BLOCKLEN_H(cntx));
		}
		else if ((data & 0x83) == 0x80) // WR3
		{
			WR3(cntx) = data;
				cntx->regs_follow[cntx->num_follow++] = GET_REGNUM(cntx, MASK_BYTE(cntx));
			if (data & 0x10)
				cntx->regs_follow[cntx->num_follow++] = GET_REGNUM(cntx, MATCH_BYTE(cntx));
		}
		else if ((data & 0x83) == 0x81) // WR4
		{
			WR4(cntx) = data;
			if (data & 0x04)
				cntx->regs_follow[cntx->num_follow++] = GET_REGNUM(cntx, PORTB_ADDRESS_L(cntx));
			if (data & 0x08)
				cntx->regs_follow[cntx->num_follow++] = GET_REGNUM(cntx, PORTB_ADDRESS_H(cntx));
			if (data & 0x10)
				cntx->regs_follow[cntx->num_follow++] = GET_REGNUM(cntx, INTERRUPT_CTRL(cntx));
		}
		else if ((data & 0xC7) == 0x82) // WR5
		{
			WR5(cntx) = data;
		}
		else if ((data & 0x83) == 0x83) // WR6
		{
			WR6(cntx) = data;
			switch (data)
			{
				case 0x88:	/* Reinitialize status byte */
				case 0xA3:	/* Reset and disable interrupts */
				case 0xA7:	/* Initiate read sequence */
				case 0xAB:	/* Enable interrupts */
				case 0xAF:	/* Disable interrupts */
				case 0xB3:	/* Force ready */
				case 0xB7:	/* Enable after rti */
				case 0xBF:	/* Read status byte */
				case 0xD3:	/* Continue */
					fatalerror("Unimplemented WR6 command %02x", data);
					break;
				case 0xC3:	/* Reset */
					LOG(("Reset\n"));
					break;
				case 0xCF:	/* Load */
					cntx->addressA = PORTA_ADDRESS(cntx);
					cntx->addressB = PORTB_ADDRESS(cntx);
					cntx->count = BLOCKLEN(cntx);
					LOG(("Load A: %x B: %x N: %x\n", cntx->addressA, cntx->addressB, cntx->count));
					break;
				case 0x83:	/* Disable dma */
					cntx->dma_enabled = 0;
					z80dma_rdy_write(which, cntx->rdy);
					break;
				case 0x87:	/* Enable dma */
					cntx->dma_enabled = 1;
					z80dma_rdy_write(which, cntx->rdy);
					break;
				case 0xBB:
					cntx->regs_follow[cntx->num_follow++] = GET_REGNUM(cntx, READ_MASK(cntx));
					break;
				default:
					fatalerror("Unknown WR6 command %02x", data);
			}
		}
		else
			fatalerror("Unknown base register %02x", data);
		cntx->cur_follow = 0;
	}
	else
	{
		int nreg = cntx->regs_follow[cntx->cur_follow];
		cntx->regs[nreg] = data;
		cntx->cur_follow++;
		if (cntx->cur_follow>=cntx->num_follow)
			cntx->num_follow = 0;
		if (nreg == REGNUM(4,3))
		{
			cntx->num_follow=0;
			if (data & 0x08)
				cntx->regs_follow[cntx->num_follow++] = GET_REGNUM(cntx, PULSE_CTRL(cntx));
			if (data & 0x10)
				cntx->regs_follow[cntx->num_follow++] = GET_REGNUM(cntx, INTERRUPT_VECTOR(cntx));
			cntx->cur_follow = 0;
		}
	}
}



static TIMER_CALLBACK( z80dma_rdy_write_callback )
{
	int which = param >> 1;
	int state = param & 0x01;
	struct z80dma	*cntx = &dma[which];

	/* normalize state */
	cntx->rdy = 1 ^ state ^ READY_ACTIVE_HIGH(cntx);
	cntx->status = (cntx->status & 0xFD) | (cntx->rdy<<1);

	z80dma_update_status(which);
}



void z80dma_rdy_write(int which, int state)
{
	int param;

	param = (which << 1) | (state ? 1 : 0);
	LOG(("RDY: %d Active High: %d\n", state, READY_ACTIVE_HIGH(&dma[which])));
	timer_call_after_resynch(NULL, param, z80dma_rdy_write_callback);
}


/******************* Standard 8-bit/32-bit/64-bit CPU interfaces *******************/

READ8_HANDLER( z80dma_0_r )	{ return z80dma_read(0, offset); }
READ8_HANDLER( z80dma_1_r )	{ return z80dma_read(1, offset); }
WRITE8_HANDLER( z80dma_0_w ) { z80dma_write(0, offset, data); }
WRITE8_HANDLER( z80dma_1_w ) { z80dma_write(1, offset, data); }

WRITE8_HANDLER( z80dma_0_rdy_w ) { z80dma_rdy_write(0, data); }
WRITE8_HANDLER( z80dma_1_rdy_w ) { z80dma_rdy_write(1, data); }

READ16_HANDLER( z80dma_16le_0_r ) { return read16le_with_read8_handler(z80dma_0_r, offset, mem_mask); }
READ16_HANDLER( z80dma_16le_1_r ) { return read16le_with_read8_handler(z80dma_1_r, offset, mem_mask); }
WRITE16_HANDLER( z80dma_16le_0_w ) { write16le_with_write8_handler(z80dma_0_w, offset, data, mem_mask); }
WRITE16_HANDLER( z80dma_16le_1_w ) { write16le_with_write8_handler(z80dma_1_w, offset, data, mem_mask); }

READ32_HANDLER( z80dma_32le_0_r ) { return read32le_with_read8_handler(z80dma_0_r, offset, mem_mask); }
READ32_HANDLER( z80dma_32le_1_r ) { return read32le_with_read8_handler(z80dma_1_r, offset, mem_mask); }
WRITE32_HANDLER( z80dma_32le_0_w ) { write32le_with_write8_handler(z80dma_0_w, offset, data, mem_mask); }
WRITE32_HANDLER( z80dma_32le_1_w ) { write32le_with_write8_handler(z80dma_1_w, offset, data, mem_mask); }

READ64_HANDLER( z80dma_64be_0_r ) { return read64be_with_read8_handler(z80dma_0_r, offset, mem_mask); }
READ64_HANDLER( z80dma_64be_1_r ) { return read64be_with_read8_handler(z80dma_1_r, offset, mem_mask); }
WRITE64_HANDLER( z80dma_64be_0_w ) { write64be_with_write8_handler(z80dma_0_w, offset, data, mem_mask); }
WRITE64_HANDLER( z80dma_64be_1_w ) { write64be_with_write8_handler(z80dma_1_w, offset, data, mem_mask); }
