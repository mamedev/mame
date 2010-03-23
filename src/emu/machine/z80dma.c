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
        - implement more asserts
        - implement a INPUT_LINE_BUSREQ for Z80. As a workaround,
          HALT is used. This implies burst mode.

**********************************************************************/

#include "emu.h"
#include "memconv.h"
#include "z80dma.h"
#include "cpu/z80/z80daisy.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define LOG 0

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
#define PORTA_INC(_c)		(WR1(_c) & 0x10)
#define PORTB_INC(_c)		(WR2(_c) & 0x10)
#define PORTA_FIXED(_c)		(((WR1(_c) >> 4) & 0x02) == 0x02)
#define PORTB_FIXED(_c)		(((WR2(_c) >> 4) & 0x02) == 0x02)
#define PORTA_MEMORY(_c)	(((WR1(_c) >> 3) & 0x01) == 0x00)
#define PORTB_MEMORY(_c)	(((WR2(_c) >> 3) & 0x01) == 0x00)

#define PORTA_CYCLE_LEN(_c)	(4-(PORTA_TIMING(_c) & 0x03))
#define PORTB_CYCLE_LEN(_c)	(4-(PORTB_TIMING(_c) & 0x03))

#define PORTA_IS_SOURCE(_c)	((WR0(_c) >> 2) & 0x01)
#define PORTB_IS_SOURCE(_c)	(!PORTA_IS_SOURCE(_c))
#define TRANSFER_MODE(_c)	(WR0(_c) & 0x03)

#define MATCH_F_SET(_c)		(_c->status &= ~0x10)
#define MATCH_F_CLEAR(_c)	(_c->status |= 0x10)
#define EOB_F_SET(_c)		(_c->status &= ~0x20)
#define EOB_F_CLEAR(_c)		(_c->status |= 0x20)

#define TM_TRANSFER		(0x01)
#define TM_SEARCH		(0x02)
#define TM_SEARCH_TRANSFER	(0x03)

#define READY_ACTIVE_HIGH(_c) ((WR5(_c)>>3) & 0x01)

#define INTERRUPT_ENABLE(_c)		(WR3(_c) & 0x20)
#define INT_ON_MATCH(_c)			(INTERRUPT_CTRL(_c) & 0x01)
#define INT_ON_END_OF_BLOCK(_c)		(INTERRUPT_CTRL(_c) & 0x02)
#define INT_ON_READY(_c)			(INTERRUPT_CTRL(_c) & 0x40)
#define STATUS_AFFECTS_VECTOR(_c)	(INTERRUPT_CTRL(_c) & 0x20)

enum
{
	INT_RDY = 0,
	INT_MATCH,
	INT_END_OF_BLOCK,
	INT_MATCH_END_OF_BLOCK
};

#define COMMAND_RESET							0xc3
#define COMMAND_RESET_PORT_A_TIMING				0xc7
#define COMMAND_RESET_PORT_B_TIMING				0xcb
#define COMMAND_LOAD							0xcf
#define COMMAND_CONTINUE						0xd3
#define COMMAND_DISABLE_INTERRUPTS				0xaf
#define COMMAND_ENABLE_INTERRUPTS				0xab
#define COMMAND_RESET_AND_DISABLE_INTERRUPTS	0xa3
#define COMMAND_ENABLE_AFTER_RETI				0xb7
#define COMMAND_READ_STATUS_BYTE				0xbf
#define COMMAND_REINITIALIZE_STATUS_BYTE		0x8b
#define COMMAND_INITIATE_READ_SEQUENCE			0xa7
#define COMMAND_FORCE_READY						0xb3
#define COMMAND_ENABLE_DMA						0x87
#define COMMAND_DISABLE_DMA						0x83
#define COMMAND_READ_MASK_FOLLOWS				0xbb

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _z80dma_t z80dma_t;
struct _z80dma_t
{
	devcb_resolved_write_line	out_busreq_func;
	devcb_resolved_write_line	out_int_func;
	devcb_resolved_write_line	out_bao_func;
	devcb_resolved_read8		in_mreq_func;
	devcb_resolved_write8		out_mreq_func;
	devcb_resolved_read8		in_iorq_func;
	devcb_resolved_write8		out_iorq_func;

	emu_timer *timer;

	UINT16	regs[REGNUM(6,1)+1];
	UINT8	num_follow;
	UINT8	cur_follow;
	UINT8	regs_follow[4];
	UINT8	read_num_follow;
	UINT8	read_cur_follow;
	UINT8	read_regs_follow[7];
	UINT8	status;
	UINT8	dma_enabled;

	UINT16 addressA;
	UINT16 addressB;
	UINT16 count;

	int rdy;
	int force_ready;
	UINT8 reset_pointer;

	UINT8 is_read;
	UINT8 cur_cycle;
	UINT8 latch;

	/* interrupts */
	int ip;						/* interrupt pending */
	int ius;					/* interrupt under service */
	UINT8 vector;				/* interrupt vector */
};

static TIMER_CALLBACK( z80dma_timerproc );
static void z80dma_update_status(running_device *device);
static int z80dma_irq_state(running_device *device);

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE z80dma_t *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == Z80DMA);
	return (z80dma_t *) device->token;
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    interrupt_check - update IRQ line state
-------------------------------------------------*/

static void interrupt_check(z80dma_t *z80dma)
{
	devcb_call_write_line(&z80dma->out_int_func, z80dma->ip ? ASSERT_LINE : CLEAR_LINE);
}

/*-------------------------------------------------
    trigger_interrupt - trigger DMA interrupt
-------------------------------------------------*/

static void trigger_interrupt(running_device *device, int level)
{
	z80dma_t *z80dma = get_safe_token(device);

	if (!z80dma->ius && INTERRUPT_ENABLE(z80dma))
	{
		/* set interrupt pending flag */
		z80dma->ip = 1;

		/* set interrupt vector */
		if (STATUS_AFFECTS_VECTOR(z80dma))
		{
			z80dma->vector = (INTERRUPT_VECTOR(z80dma) & 0xf9) | (level << 1);
		}
		else
		{
			z80dma->vector = INTERRUPT_VECTOR(z80dma);
		}

		z80dma->status &= ~0x08;

		if (LOG) logerror("Z80DMA '%s' Interrupt Pending\n", device->tag());

		interrupt_check(z80dma);
	}
}

static int is_ready(z80dma_t *z80dma)
{
	return (z80dma->force_ready) || (z80dma->rdy == READY_ACTIVE_HIGH(z80dma));
}

/*-------------------------------------------------
    z80dma_do_read - perform DMA read
-------------------------------------------------*/

static void z80dma_do_read(running_device *device)
{
	z80dma_t *z80dma = get_safe_token(device);
	UINT8 mode;

	mode = TRANSFER_MODE(z80dma);
	switch(mode) {
		case TM_TRANSFER:
		case TM_SEARCH:
			if (PORTA_IS_SOURCE(z80dma))
			{
				if (PORTA_MEMORY(z80dma))
					z80dma->latch = devcb_call_read8(&z80dma->in_mreq_func, z80dma->addressA);
				else
					z80dma->latch = devcb_call_read8(&z80dma->in_iorq_func, z80dma->addressA);

				if (LOG) logerror("A src: %04x \n",z80dma->addressA);
				z80dma->addressA += PORTA_FIXED(z80dma) ? 0 : PORTA_INC(z80dma) ? PORTA_STEP(z80dma) : -PORTA_STEP(z80dma);
			}
			else
			{
				if (PORTB_MEMORY(z80dma))
					z80dma->latch = devcb_call_read8(&z80dma->in_mreq_func, z80dma->addressB);
				else
					z80dma->latch = devcb_call_read8(&z80dma->in_iorq_func, z80dma->addressB);

				if (LOG) logerror("B src: %04x \n",z80dma->addressB);
				z80dma->addressB += PORTB_FIXED(z80dma) ? 0 : PORTB_INC(z80dma) ? PORTB_STEP(z80dma) : -PORTB_STEP(z80dma);
			}
			break;
		case TM_SEARCH_TRANSFER:
			fatalerror("z80dma_do_operation: unhandled search & transfer mode !\n");
			break;
		default:
			fatalerror("z80dma_do_operation: invalid mode %d!\n", mode);
			break;
	}
}

/*-------------------------------------------------
    z80dma_do_write - perform DMA write
-------------------------------------------------*/

static int z80dma_do_write(running_device *device)
{
	z80dma_t *z80dma = get_safe_token(device);
	int done;
	UINT8 mode;

	mode = TRANSFER_MODE(z80dma);
	if (z80dma->count == 0x0000)
	{
		//FIXME: Any signal here
	}
	switch(mode) {
		case TM_TRANSFER:
			if (PORTA_IS_SOURCE(z80dma))
			{
				if (PORTB_MEMORY(z80dma))
					devcb_call_write8(&z80dma->out_mreq_func, z80dma->addressB, z80dma->latch);
				else
					devcb_call_write8(&z80dma->out_iorq_func, z80dma->addressB, z80dma->latch);

				if (LOG) logerror("B dst: %04x \n",z80dma->addressB);
				z80dma->addressB += PORTB_FIXED(z80dma) ? 0 : PORTB_INC(z80dma) ? PORTB_STEP(z80dma) : -PORTB_STEP(z80dma);
			}
			else
			{
				if (PORTA_MEMORY(z80dma))
					devcb_call_write8(&z80dma->out_mreq_func, z80dma->addressA, z80dma->latch);
				else
					devcb_call_write8(&z80dma->out_iorq_func, z80dma->addressA, z80dma->latch);

				if (LOG) logerror("A dst: %04x \n",z80dma->addressA);
				z80dma->addressA += PORTA_FIXED(z80dma) ? 0 : PORTA_INC(z80dma) ? PORTA_STEP(z80dma) : -PORTA_STEP(z80dma);
			}
			z80dma->count--;
			done = (z80dma->count == 0xFFFF);
			break;

		case TM_SEARCH:
			{
				UINT8 load_byte,match_byte;
				load_byte = z80dma->latch | MASK_BYTE(z80dma);
				match_byte = MATCH_BYTE(z80dma) | MASK_BYTE(z80dma);
				//if (LOG) logerror("%02x %02x\n",load_byte,match_byte));
				if (load_byte == match_byte)
				{
					if (INT_ON_MATCH(z80dma))
					{
						trigger_interrupt(device, INT_MATCH);
					}
				}

				z80dma->count--;
				done = (z80dma->count == 0xFFFF); //correct?
			}
			break;

		case TM_SEARCH_TRANSFER:
			fatalerror("z80dma_do_operation: unhandled search & transfer mode !\n");
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

/*-------------------------------------------------
    TIMER_CALLBACK( z80dma_timerproc )
-------------------------------------------------*/

static TIMER_CALLBACK( z80dma_timerproc )
{
	running_device *device = (running_device *)ptr;
	z80dma_t *z80dma = get_safe_token(device);
	int done;

	if (--z80dma->cur_cycle)
	{
		return;
	}

	if (z80dma->is_read)
	{
		z80dma_do_read(device);
		done = 0;
		z80dma->is_read = 0;
		z80dma->cur_cycle = (PORTA_IS_SOURCE(z80dma) ? PORTA_CYCLE_LEN(z80dma) : PORTB_CYCLE_LEN(z80dma));
	}
	else
	{
		done = z80dma_do_write(device);
		z80dma->is_read = 1;
		z80dma->cur_cycle = (PORTB_IS_SOURCE(z80dma) ? PORTA_CYCLE_LEN(z80dma) : PORTB_CYCLE_LEN(z80dma));
	}

	if (done)
	{
		z80dma->dma_enabled = 0; //FIXME: Correct?
        z80dma->status = 0x19;

		z80dma->status |= !is_ready(z80dma) << 1; // ready line status

		if(TRANSFER_MODE(z80dma) == TM_TRANSFER)     z80dma->status |= 0x10;   // no match found

		z80dma_update_status(device);
		if (LOG) logerror("Z80DMA '%s' End of Block\n", device->tag());

		if (INT_ON_END_OF_BLOCK(z80dma))
        {
			trigger_interrupt(device, INT_END_OF_BLOCK);
        }
	}
}

/*-------------------------------------------------
    z80dma_update_status - update DMA status
-------------------------------------------------*/

static void z80dma_update_status(running_device *device)
{
	z80dma_t *z80dma = get_safe_token(device);
	UINT16 pending_transfer;
	attotime next;

	/* no transfer is active right now; is there a transfer pending right now? */
	pending_transfer = is_ready(z80dma) & z80dma->dma_enabled;

	if (pending_transfer)
	{
		z80dma->is_read = 1;
		z80dma->cur_cycle = (PORTA_IS_SOURCE(z80dma) ? PORTA_CYCLE_LEN(z80dma) : PORTB_CYCLE_LEN(z80dma));
		next = ATTOTIME_IN_HZ(device->clock);
		timer_adjust_periodic(z80dma->timer,
			attotime_zero,
			0,
			/* 1 byte transferred in 4 clock cycles */
			next);
	}
	else
	{
		if (z80dma->is_read)
		{
			/* no transfers active right now */
			timer_reset(z80dma->timer, attotime_never);
		}
	}

	/* set the busreq line */
	devcb_call_write_line(&z80dma->out_busreq_func, pending_transfer ? ASSERT_LINE : CLEAR_LINE);
}

/*-------------------------------------------------
    z80dma_r - register read
-------------------------------------------------*/

READ8_DEVICE_HANDLER( z80dma_r )
{
	z80dma_t *z80dma = get_safe_token(device);
	UINT8 res;

	res = z80dma->read_regs_follow[z80dma->read_cur_follow];
	z80dma->read_cur_follow++;

	if(z80dma->read_cur_follow >= z80dma->read_num_follow)
		z80dma->read_cur_follow = 0;

	return res;
}

/*-------------------------------------------------
    z80dma_w - register write
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( z80dma_w )
{
	z80dma_t *z80dma = get_safe_token(device);

	if (z80dma->num_follow == 0)
	{
		if ((data & 0x87) == 0) // WR2
		{
			WR2(z80dma) = data;
			if (data & 0x40)
				z80dma->regs_follow[z80dma->num_follow++] = GET_REGNUM(z80dma, PORTB_TIMING(z80dma));
		}
		else if ((data & 0x87) == 0x04) // WR1
		{
			WR1(z80dma) = data;
			if (data & 0x40)
				z80dma->regs_follow[z80dma->num_follow++] = GET_REGNUM(z80dma, PORTA_TIMING(z80dma));
		}
		else if ((data & 0x80) == 0) // WR0
		{
			WR0(z80dma) = data;
			if (data & 0x08)
				z80dma->regs_follow[z80dma->num_follow++] = GET_REGNUM(z80dma, PORTA_ADDRESS_L(z80dma));
			if (data & 0x10)
				z80dma->regs_follow[z80dma->num_follow++] = GET_REGNUM(z80dma, PORTA_ADDRESS_H(z80dma));
			if (data & 0x20)
				z80dma->regs_follow[z80dma->num_follow++] = GET_REGNUM(z80dma, BLOCKLEN_L(z80dma));
			if (data & 0x40)
				z80dma->regs_follow[z80dma->num_follow++] = GET_REGNUM(z80dma, BLOCKLEN_H(z80dma));
		}
		else if ((data & 0x83) == 0x80) // WR3
		{
			WR3(z80dma) = data;
			if (data & 0x08)
				z80dma->regs_follow[z80dma->num_follow++] = GET_REGNUM(z80dma, MASK_BYTE(z80dma));
			if (data & 0x10)
				z80dma->regs_follow[z80dma->num_follow++] = GET_REGNUM(z80dma, MATCH_BYTE(z80dma));
		}
		else if ((data & 0x83) == 0x81) // WR4
		{
			WR4(z80dma) = data;
			if (data & 0x04)
				z80dma->regs_follow[z80dma->num_follow++] = GET_REGNUM(z80dma, PORTB_ADDRESS_L(z80dma));
			if (data & 0x08)
				z80dma->regs_follow[z80dma->num_follow++] = GET_REGNUM(z80dma, PORTB_ADDRESS_H(z80dma));
			if (data & 0x10)
				z80dma->regs_follow[z80dma->num_follow++] = GET_REGNUM(z80dma, INTERRUPT_CTRL(z80dma));
		}
		else if ((data & 0xC7) == 0x82) // WR5
		{
			WR5(z80dma) = data;
		}
		else if ((data & 0x83) == 0x83) // WR6
		{
			z80dma->dma_enabled = 0;

			WR6(z80dma) = data;

			switch (data)
			{
				case COMMAND_ENABLE_AFTER_RETI:
					fatalerror("Unimplemented WR6 command %02x", data);
					break;
				case COMMAND_READ_STATUS_BYTE:
					if (LOG) logerror("CMD Read status Byte\n");
        			READ_MASK(z80dma) = 0;
        			break;
				case COMMAND_RESET_AND_DISABLE_INTERRUPTS:
					WR3(z80dma) &= ~0x20;
					z80dma->ip = 0;
					z80dma->ius = 0;
					z80dma->force_ready = 0;
					z80dma->status |= 0x08;
					break;
				case COMMAND_INITIATE_READ_SEQUENCE:
					if (LOG) logerror("Initiate Read Sequence\n");
					z80dma->read_cur_follow = z80dma->read_num_follow = 0;
					if(READ_MASK(z80dma) & 0x01) { z80dma->read_regs_follow[z80dma->read_num_follow++] = z80dma->status; }
					if(READ_MASK(z80dma) & 0x02) { z80dma->read_regs_follow[z80dma->read_num_follow++] = BLOCKLEN_L(z80dma); } //byte counter (low)
					if(READ_MASK(z80dma) & 0x04) { z80dma->read_regs_follow[z80dma->read_num_follow++] = BLOCKLEN_H(z80dma); } //byte counter (high)
					if(READ_MASK(z80dma) & 0x08) { z80dma->read_regs_follow[z80dma->read_num_follow++] = PORTA_ADDRESS_L(z80dma); } //port A address (low)
					if(READ_MASK(z80dma) & 0x10) { z80dma->read_regs_follow[z80dma->read_num_follow++] = PORTA_ADDRESS_H(z80dma); } //port A address (high)
					if(READ_MASK(z80dma) & 0x20) { z80dma->read_regs_follow[z80dma->read_num_follow++] = PORTB_ADDRESS_L(z80dma); } //port B address (low)
					if(READ_MASK(z80dma) & 0x40) { z80dma->read_regs_follow[z80dma->read_num_follow++] = PORTA_ADDRESS_H(z80dma); } //port B address (high)
					break;
				case COMMAND_RESET:
					if (LOG) logerror("Reset\n");
					z80dma->dma_enabled = 0;
					z80dma->force_ready = 0;
					/* Needs six reset commands to reset the DMA */
					{
						UINT8 WRi;

						for(WRi=0;WRi<7;WRi++)
							REG(z80dma,WRi,z80dma->reset_pointer) = 0;

						z80dma->reset_pointer++;
						if(z80dma->reset_pointer >= 6) { z80dma->reset_pointer = 0; }
					}
					z80dma->status = 0x38;
					break;
				case COMMAND_LOAD:
					z80dma->force_ready = 0;
					z80dma->addressA = PORTA_ADDRESS(z80dma);
					z80dma->addressB = PORTB_ADDRESS(z80dma);
					z80dma->count = BLOCKLEN(z80dma);
					z80dma->status |= 0x30;
					if (LOG) logerror("Load A: %x B: %x N: %x\n", z80dma->addressA, z80dma->addressB, z80dma->count);
					break;
				case COMMAND_DISABLE_DMA:
					if (LOG) logerror("Disable DMA\n");
					z80dma->dma_enabled = 0;
					break;
				case COMMAND_ENABLE_DMA:
					if (LOG) logerror("Enable DMA\n");
					z80dma->dma_enabled = 1;
					z80dma_update_status(device);
					break;
				case COMMAND_READ_MASK_FOLLOWS:
					if (LOG) logerror("Set Read Mask\n");
					z80dma->regs_follow[z80dma->num_follow++] = GET_REGNUM(z80dma, READ_MASK(z80dma));
					break;
				case COMMAND_CONTINUE:
					if (LOG) logerror("Continue\n");
					z80dma->count = BLOCKLEN(z80dma);
					z80dma->dma_enabled = 1;
					//"match not found" & "end of block" status flags zeroed here
					z80dma->status |= 0x30;
					break;
				case COMMAND_RESET_PORT_A_TIMING:
					if (LOG) logerror("Reset Port A Timing\n");
					PORTA_TIMING(z80dma) = 0;
					break;
				case COMMAND_RESET_PORT_B_TIMING:
					if (LOG) logerror("Reset Port B Timing\n");
					PORTB_TIMING(z80dma) = 0;
					break;
				case COMMAND_FORCE_READY:
					if (LOG) logerror("Force ready\n");
					z80dma->force_ready = 1;
					z80dma_update_status(device);
					break;
				case COMMAND_ENABLE_INTERRUPTS:
					if (LOG) logerror("Enable IRQ\n");
					WR3(z80dma) |= 0x20;
					break;
				case COMMAND_DISABLE_INTERRUPTS:
					if (LOG) logerror("Disable IRQ\n");
					WR3(z80dma) &= ~0x20;
					break;
				case COMMAND_REINITIALIZE_STATUS_BYTE:
					if (LOG) logerror("Reinitialize status byte\n");
					z80dma->status |= 0x30;
					z80dma->ip = 0;
					break;
				case 0xFB:
					if (LOG) logerror("Z80DMA undocumented command triggered 0x%02X!\n",data);
					break;
				default:
					fatalerror("Unknown WR6 command %02x", data);
			}
		}
		else
			fatalerror("Unknown base register %02x", data);
		z80dma->cur_follow = 0;
	}
	else
	{
		int nreg = z80dma->regs_follow[z80dma->cur_follow];
		z80dma->regs[nreg] = data;
		z80dma->cur_follow++;
		if (z80dma->cur_follow>=z80dma->num_follow)
			z80dma->num_follow = 0;
		if (nreg == REGNUM(4,3))
		{
			z80dma->num_follow=0;
			if (data & 0x08)
				z80dma->regs_follow[z80dma->num_follow++] = GET_REGNUM(z80dma, PULSE_CTRL(z80dma));
			if (data & 0x10)
				z80dma->regs_follow[z80dma->num_follow++] = GET_REGNUM(z80dma, INTERRUPT_VECTOR(z80dma));
			z80dma->cur_follow = 0;
		}
	}
}

/*-------------------------------------------------
    TIMER_CALLBACK( z80dma_rdy_write_callback )
-------------------------------------------------*/

static TIMER_CALLBACK( z80dma_rdy_write_callback )
{
	running_device *device = (running_device *)ptr;
	z80dma_t *z80dma = get_safe_token(device);

	z80dma->rdy = param;
	z80dma->status = (z80dma->status & 0xFD) | (!is_ready(z80dma) << 1);

	z80dma_update_status(device);

	if (is_ready(z80dma) && INT_ON_READY(z80dma))
    {
		trigger_interrupt(device, INT_RDY);
    }
}

/*-------------------------------------------------
    z80dma_rdy_w - ready input
-------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER( z80dma_rdy_w )
{
	z80dma_t *z80dma = get_safe_token(device);

	if (LOG) logerror("RDY: %d Active High: %d\n", state, READY_ACTIVE_HIGH(z80dma));

	timer_call_after_resynch(device->machine, (void *) device, state, z80dma_rdy_write_callback);
}

/*-------------------------------------------------
    z80dma_wait_w - wait input
-------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER( z80dma_wait_w )
{
}

/*-------------------------------------------------
    z80dma_bai_w - bus acknowledge input
-------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER( z80dma_bai_w )
{
}

/*-------------------------------------------------
    z80dma_irq_state - read interrupt state
-------------------------------------------------*/

static int z80dma_irq_state(running_device *device)
{
	z80dma_t *z80dma = get_safe_token(device);
	int state = 0;

	if (z80dma->ip)
	{
		/* interrupt pending */
		state = Z80_DAISY_INT;
	}
	else if (z80dma->ius)
	{
		/* interrupt under service */
		state = Z80_DAISY_IEO;
	}

	if (LOG) logerror("Z80DMA '%s' Interrupt State: %u\n", device->tag(), state);

	return state;
}

/*-------------------------------------------------
    z80dma_irq_ack - interrupt acknowledge
-------------------------------------------------*/

static int z80dma_irq_ack(running_device *device)
{
	z80dma_t *z80dma = get_safe_token(device);

	if (z80dma->ip)
	{
	    if (LOG) logerror("Z80DMA '%s' Interrupt Acknowledge\n", device->tag());

		/* clear interrupt pending flag */
		z80dma->ip = 0;
	    interrupt_check(z80dma);

		/* set interrupt under service flag */
		z80dma->ius = 1;

		/* disable DMA */
		z80dma->dma_enabled = 0;

		return z80dma->vector;
	}

	logerror("z80dma_irq_ack: failed to find an interrupt to ack!\n");

	return 0;
}

/*-------------------------------------------------
    z80dma_irq_reti - return from interrupt
-------------------------------------------------*/

static void z80dma_irq_reti(running_device *device)
{
	z80dma_t *z80dma = get_safe_token(device);

	if (z80dma->ius)
	{
	    if (LOG) logerror("Z80DMA '%s' Return from Interrupt\n", device->tag());

		/* clear interrupt under service flag */
		z80dma->ius = 0;
	    interrupt_check(z80dma);

		return;
	}

	logerror("z80dma_irq_reti: failed to find an interrupt to clear IEO on!\n");
}

/*-------------------------------------------------
    DEVICE_START( z80dma )
-------------------------------------------------*/

static DEVICE_START( z80dma )
{
	z80dma_t *z80dma = get_safe_token(device);
	z80dma_interface *intf = (z80dma_interface *)device->baseconfig().static_config;

	/* resolve callbacks */
	devcb_resolve_write_line(&z80dma->out_busreq_func, &intf->out_busreq_func, device);
	devcb_resolve_write_line(&z80dma->out_int_func, &intf->out_int_func, device);
	devcb_resolve_write_line(&z80dma->out_bao_func, &intf->out_bao_func, device);
	devcb_resolve_read8(&z80dma->in_mreq_func, &intf->in_mreq_func, device);
	devcb_resolve_write8(&z80dma->out_mreq_func, &intf->out_mreq_func, device);
	devcb_resolve_read8(&z80dma->in_iorq_func, &intf->in_iorq_func, device);
	devcb_resolve_write8(&z80dma->out_iorq_func, &intf->out_iorq_func, device);

	/* allocate timer */
	z80dma->timer = timer_alloc(device->machine, z80dma_timerproc, (void *) device);

	/* register for state saving */
	state_save_register_device_item_array(device, 0, z80dma->regs);
	state_save_register_device_item_array(device, 0, z80dma->regs_follow);
	state_save_register_device_item(device, 0, z80dma->num_follow);
	state_save_register_device_item(device, 0, z80dma->cur_follow);
	state_save_register_device_item(device, 0, z80dma->status);
	state_save_register_device_item(device, 0, z80dma->dma_enabled);
	state_save_register_device_item(device, 0, z80dma->vector);
	state_save_register_device_item(device, 0, z80dma->ip);
	state_save_register_device_item(device, 0, z80dma->ius);
	state_save_register_device_item(device, 0, z80dma->addressA);
	state_save_register_device_item(device, 0, z80dma->addressB);
	state_save_register_device_item(device, 0, z80dma->count);
	state_save_register_device_item(device, 0, z80dma->rdy);
	state_save_register_device_item(device, 0, z80dma->force_ready);
	state_save_register_device_item(device, 0, z80dma->is_read);
	state_save_register_device_item(device, 0, z80dma->cur_cycle);
	state_save_register_device_item(device, 0, z80dma->latch);
}

/*-------------------------------------------------
    DEVICE_RESET( z80dma )
-------------------------------------------------*/

static DEVICE_RESET( z80dma )
{
	z80dma_t *z80dma = get_safe_token(device);

	z80dma->status = 0;
	z80dma->rdy = 0;
	z80dma->force_ready = 0;
	z80dma->num_follow = 0;
	z80dma->dma_enabled = 0;
	z80dma->read_num_follow = z80dma->read_cur_follow = 0;
	z80dma->reset_pointer = 0;

	/* disable interrupts */
	WR3(z80dma) &= ~0x20;
	z80dma->ip = 0;
	z80dma->ius = 0;
	z80dma->vector = 0;

	z80dma_update_status(device);
}

/*-------------------------------------------------
    DEVICE_GET_INFO( z80dma )
-------------------------------------------------*/

DEVICE_GET_INFO( z80dma )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(z80dma_t);				break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = 0;							break;
		case DEVINFO_INT_CLASS:							info->i = DEVICE_CLASS_PERIPHERAL;		break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(z80dma);break;
		case DEVINFO_FCT_STOP:							/* Nothing */							break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(z80dma);break;
		case DEVINFO_FCT_IRQ_STATE:						info->f = (genf *)z80dma_irq_state;		break;
		case DEVINFO_FCT_IRQ_ACK:						info->f = (genf *)z80dma_irq_ack;		break;
		case DEVINFO_FCT_IRQ_RETI:						info->f = (genf *)z80dma_irq_reti;		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Z8410");				break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "Z80");					break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");					break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:						strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}
