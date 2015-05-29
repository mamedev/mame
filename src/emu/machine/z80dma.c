// license:BSD-3-Clause
// copyright-holders:Couriersud
/**********************************************************************

    Z80 DMA interface and emulation

    For datasheet http://www.zilog.com/docs/z80/ps0179.pdf

    2008/01     couriersud

        - architecture copied from 8257 DMA
        - significant changes to implementation
        - This is only a minimum implementation to support dkong3 and mario drivers
        - Only memory to memory is tested!

    TODO:
        - reset command (C3) is handled improperly
        - rewrite to match documentation
        - implement missing features
        - implement more asserts
        - implement a INPUT_LINE_BUSREQ for Z80. As a workaround,
          HALT is used. This implies burst mode.

**********************************************************************/

#include "emu.h"
#include "z80dma.h"
#include "cpu/z80/z80daisy.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

enum
{
	INT_RDY = 0,
	INT_MATCH,
	INT_END_OF_BLOCK,
	INT_MATCH_END_OF_BLOCK
};

const int COMMAND_RESET                         = 0xc3;
const int COMMAND_RESET_PORT_A_TIMING           = 0xc7;
const int COMMAND_RESET_PORT_B_TIMING           = 0xcb;
const int COMMAND_LOAD                          = 0xcf;
const int COMMAND_CONTINUE                      = 0xd3;
const int COMMAND_DISABLE_INTERRUPTS            = 0xaf;
const int COMMAND_ENABLE_INTERRUPTS             = 0xab;
const int COMMAND_RESET_AND_DISABLE_INTERRUPTS  = 0xa3;
const int COMMAND_ENABLE_AFTER_RETI             = 0xb7;
const int COMMAND_READ_STATUS_BYTE              = 0xbf;
const int COMMAND_REINITIALIZE_STATUS_BYTE      = 0x8b;
const int COMMAND_INITIATE_READ_SEQUENCE        = 0xa7;
const int COMMAND_FORCE_READY                   = 0xb3;
const int COMMAND_ENABLE_DMA                    = 0x87;
const int COMMAND_DISABLE_DMA                   = 0x83;
const int COMMAND_READ_MASK_FOLLOWS             = 0xbb;

const int TM_TRANSFER           = 0x01;
const int TM_SEARCH             = 0x02;
const int TM_SEARCH_TRANSFER    = 0x03;



//**************************************************************************
//  MACROS
//**************************************************************************

#define LOG 0
#define DMA_LOG 0

#define REGNUM(_m, _s)          (((_m)<<3) + (_s))
#define GET_REGNUM(_r)          (&(_r) - &(WR0))
#define REG(_m, _s)             m_regs[REGNUM(_m,_s)]
#define WR0                     REG(0, 0)
#define WR1                     REG(1, 0)
#define WR2                     REG(2, 0)
#define WR3                     REG(3, 0)
#define WR4                     REG(4, 0)
#define WR5                     REG(5, 0)
#define WR6                     REG(6, 0)

#define PORTA_ADDRESS_L         REG(0,1)
#define PORTA_ADDRESS_H         REG(0,2)

#define BLOCKLEN_L              REG(0,3)
#define BLOCKLEN_H              REG(0,4)

#define PORTA_TIMING            REG(1,1)
#define PORTB_TIMING            REG(2,1)

#define MASK_BYTE               REG(3,1)
#define MATCH_BYTE              REG(3,2)

#define PORTB_ADDRESS_L         REG(4,1)
#define PORTB_ADDRESS_H         REG(4,2)
#define INTERRUPT_CTRL          REG(4,3)
#define INTERRUPT_VECTOR        REG(4,4)
#define PULSE_CTRL              REG(4,5)

#define READ_MASK               REG(6,1)

#define PORTA_ADDRESS           ((PORTA_ADDRESS_H<<8) | PORTA_ADDRESS_L)
#define PORTB_ADDRESS           ((PORTB_ADDRESS_H<<8) | PORTB_ADDRESS_L)
#define BLOCKLEN                ((BLOCKLEN_H<<8) | BLOCKLEN_L)

#define PORTA_INC               (WR1 & 0x10)
#define PORTB_INC               (WR2 & 0x10)
#define PORTA_FIXED             (((WR1 >> 4) & 0x02) == 0x02)
#define PORTB_FIXED             (((WR2 >> 4) & 0x02) == 0x02)
#define PORTA_MEMORY            (((WR1 >> 3) & 0x01) == 0x00)
#define PORTB_MEMORY            (((WR2 >> 3) & 0x01) == 0x00)

#define PORTA_CYCLE_LEN         (4-(PORTA_TIMING & 0x03))
#define PORTB_CYCLE_LEN         (4-(PORTB_TIMING & 0x03))

#define PORTA_IS_SOURCE         ((WR0 >> 2) & 0x01)
#define PORTB_IS_SOURCE         (!PORTA_IS_SOURCE)
#define TRANSFER_MODE           (WR0 & 0x03)

#define MATCH_F_SET             (m_status &= ~0x10)
#define MATCH_F_CLEAR           (m_status |= 0x10)
#define EOB_F_SET               (m_status &= ~0x20)
#define EOB_F_CLEAR             (m_status |= 0x20)

#define READY_ACTIVE_HIGH       ((WR5>>3) & 0x01)
#define AUTO_RESTART            ((WR5>>5) & 0x01)

#define INTERRUPT_ENABLE        (WR3 & 0x20)
#define INT_ON_MATCH            (INTERRUPT_CTRL & 0x01)
#define INT_ON_END_OF_BLOCK     (INTERRUPT_CTRL & 0x02)
#define INT_ON_READY            (INTERRUPT_CTRL & 0x40)
#define STATUS_AFFECTS_VECTOR   (INTERRUPT_CTRL & 0x20)



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
const device_type Z80DMA = &device_creator<z80dma_device>;

//-------------------------------------------------
//  z80dma_device - constructor
//-------------------------------------------------

z80dma_device::z80dma_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, Z80DMA, "Z80 DMA", tag, owner, clock, "z80dma", __FILE__),
		device_z80daisy_interface(mconfig, *this),
		m_out_busreq_cb(*this),
		m_out_int_cb(*this),
		m_out_bao_cb(*this),
		m_in_mreq_cb(*this),
		m_out_mreq_cb(*this),
		m_in_iorq_cb(*this),
		m_out_iorq_cb(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void z80dma_device::device_start()
{
	// resolve callbacks
	m_out_busreq_cb.resolve_safe();
	m_out_int_cb.resolve_safe();
	m_out_bao_cb.resolve_safe();
	m_in_mreq_cb.resolve_safe(0);
	m_out_mreq_cb.resolve_safe();
	m_in_iorq_cb.resolve_safe(0);
	m_out_iorq_cb.resolve_safe();

	// allocate timer
	m_timer = machine().scheduler().timer_alloc(FUNC(static_timerproc), (void *)this);

	// register for state saving
	save_item(NAME(m_regs));
	save_item(NAME(m_regs_follow));
	save_item(NAME(m_num_follow));
	save_item(NAME(m_cur_follow));
	save_item(NAME(m_status));
	save_item(NAME(m_dma_enabled));
	save_item(NAME(m_vector));
	save_item(NAME(m_ip));
	save_item(NAME(m_ius));
	save_item(NAME(m_addressA));
	save_item(NAME(m_addressB));
	save_item(NAME(m_count));
	save_item(NAME(m_rdy));
	save_item(NAME(m_force_ready));
	save_item(NAME(m_is_read));
	save_item(NAME(m_cur_cycle));
	save_item(NAME(m_latch));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void z80dma_device::device_reset()
{
	m_status = 0;
	m_rdy = 0;
	m_force_ready = 0;
	m_num_follow = 0;
	m_dma_enabled = 0;
	m_read_num_follow = m_read_cur_follow = 0;
	m_reset_pointer = 0;
	m_is_read = false;
	memset(m_regs, 0, sizeof(m_regs));
	memset(m_regs_follow, 0, sizeof(m_regs_follow));

	// disable interrupts
	WR3 &= ~0x20;
	m_ip = 0;
	m_ius = 0;
	m_vector = 0;

	update_status();
}



//**************************************************************************
//  DAISY CHAIN INTERFACE
//**************************************************************************

//-------------------------------------------------
//  z80daisy_irq_state - return the overall IRQ
//  state for this device
//-------------------------------------------------

int z80dma_device::z80daisy_irq_state()
{
	int state = 0;

	if (m_ip)
	{
		// interrupt pending
		state = Z80_DAISY_INT;
	}
	else if (m_ius)
	{
		// interrupt under service
		state = Z80_DAISY_IEO;
	}

	if (LOG) logerror("Z80DMA '%s' Interrupt State: %u\n", tag(), state);

	return state;
}


//-------------------------------------------------
//  z80daisy_irq_ack - acknowledge an IRQ and
//  return the appropriate vector
//-------------------------------------------------

int z80dma_device::z80daisy_irq_ack()
{
	if (m_ip)
	{
		if (LOG) logerror("Z80DMA '%s' Interrupt Acknowledge\n", tag());

		// clear interrupt pending flag
		m_ip = 0;
		interrupt_check();

		// set interrupt under service flag
		m_ius = 1;

		return m_vector;
	}

	//logerror("z80dma_irq_ack: failed to find an interrupt to ack!\n");

	return 0;
}


//-------------------------------------------------
//  z80daisy_irq_reti - clear the interrupt
//  pending state to allow other interrupts through
//-------------------------------------------------

void z80dma_device::z80daisy_irq_reti()
{
	if (m_ius)
	{
		if (LOG) logerror("Z80DMA '%s' Return from Interrupt\n", tag());

		// clear interrupt under service flag
		m_ius = 0;
		interrupt_check();

		return;
	}

	//logerror("z80dma_irq_reti: failed to find an interrupt to clear IEO on!\n");
}



//**************************************************************************
//  INTERNAL STATE MANAGEMENT
//**************************************************************************

//-------------------------------------------------
//  is_ready - ready for DMA transfer?
//-------------------------------------------------

int z80dma_device::is_ready()
{
	return (m_force_ready) || (m_rdy == READY_ACTIVE_HIGH);
}


//-------------------------------------------------
//  interrupt_check - update IRQ line state
//-------------------------------------------------

void z80dma_device::interrupt_check()
{
	m_out_int_cb(m_ip ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  trigger_interrupt - trigger DMA interrupt
//-------------------------------------------------

void z80dma_device::trigger_interrupt(int level)
{
	if (!m_ius && INTERRUPT_ENABLE)
	{
		// set interrupt pending flag
		m_ip = 1;

		// set interrupt vector
		if (STATUS_AFFECTS_VECTOR)
		{
			m_vector = (INTERRUPT_VECTOR & 0xf9) | (level << 1);
		}
		else
		{
			m_vector = INTERRUPT_VECTOR;
		}

		m_status &= ~0x08;

		if (LOG) logerror("Z80DMA '%s' Interrupt Pending\n", tag());

		interrupt_check();
	}
}


//-------------------------------------------------
//  do_read - perform DMA read
//-------------------------------------------------

void z80dma_device::do_read()
{
	UINT8 mode;

	mode = TRANSFER_MODE;
	switch(mode) {
		case TM_TRANSFER:
		case TM_SEARCH:
		case TM_SEARCH_TRANSFER:
			if (PORTA_IS_SOURCE)
			{
				if (PORTA_MEMORY)
					m_latch = m_in_mreq_cb(m_addressA);
				else
					m_latch = m_in_iorq_cb(m_addressA);

				if (DMA_LOG) logerror("Z80DMA '%s' A src: %04x %s -> data: %02x\n", tag(), m_addressA, PORTA_MEMORY ? "mem" : "i/o", m_latch);
			}
			else
			{
				if (PORTB_MEMORY)
					m_latch = m_in_mreq_cb(m_addressB);
				else
					m_latch = m_in_iorq_cb(m_addressB);

				if (DMA_LOG) logerror("Z80DMA '%s' B src: %04x %s -> data: %02x\n", tag(), m_addressB, PORTB_MEMORY ? "mem" : "i/o", m_latch);
			}
			break;
		default:
			logerror("z80dma_do_operation: invalid mode %d!\n", mode);
			break;
	}
}


//-------------------------------------------------
//  do_write - perform DMA write
//-------------------------------------------------

void z80dma_device::do_transfer_write()
{
	if (PORTA_IS_SOURCE)
	{
		if (PORTB_MEMORY)
			m_out_mreq_cb((offs_t)m_addressB, m_latch);
		else
			m_out_iorq_cb((offs_t)m_addressB, m_latch);

		if (DMA_LOG) logerror("Z80DMA '%s' B dst: %04x %s\n", tag(), m_addressB, PORTB_MEMORY ? "mem" : "i/o");
	}
	else
	{
		if (PORTA_MEMORY)
			m_out_mreq_cb((offs_t)m_addressA, m_latch);
		else
			m_out_iorq_cb((offs_t)m_addressA, m_latch);

		if (DMA_LOG) logerror("Z80DMA '%s' A dst: %04x %s\n", tag(), m_addressA, PORTA_MEMORY ? "mem" : "i/o");
	}
}

void z80dma_device::do_search()
{
	UINT8 load_byte,match_byte;
	load_byte = m_latch | MASK_BYTE;
	match_byte = MATCH_BYTE | MASK_BYTE;
	//if (LOG) logerror("%02x %02x\n",load_byte,match_byte));
	if (load_byte == match_byte)
	{
		if (INT_ON_MATCH)
		{
			trigger_interrupt(INT_MATCH);
		}
	}
}

int z80dma_device::do_write()
{
	int done = 0;
	UINT8 mode;

	mode = TRANSFER_MODE;
	if (m_count == 0x0000)
	{
		//FIXME: Any signal here
	}
	switch(mode) {
		case TM_TRANSFER:
			do_transfer_write();
			break;

		case TM_SEARCH:
			do_search();
			break;

		case TM_SEARCH_TRANSFER:
			do_transfer_write();
			do_search();
			break;

		default:
			logerror("z80dma_do_operation: invalid mode %d!\n", mode);
			break;
	}

	m_addressA += PORTA_FIXED ? 0 : PORTA_INC ? 1 : -1;
	m_addressB += PORTB_FIXED ? 0 : PORTB_INC ? 1 : -1;

	m_count--;
	done = (m_count == 0xFFFF); //correct?

	if (done)
	{
		//FIXME: interrupt ?
	}
	return done;
}


//-------------------------------------------------
//  timerproc
//-------------------------------------------------

void z80dma_device::timerproc()
{
	int done;

	if (--m_cur_cycle)
	{
		return;
	}

	if (m_is_read && !is_ready()) return;

	if (m_is_read)
	{
		/* TODO: there's a nasty recursion bug with Alpha for Sharp X1 Turbo on the transfers with this function! */
		do_read();
		done = 0;
		m_is_read = false;
		m_cur_cycle = (PORTA_IS_SOURCE ? PORTA_CYCLE_LEN : PORTB_CYCLE_LEN);
	}
	else
	{
		done = do_write();
		m_is_read = true;
		m_cur_cycle = (PORTB_IS_SOURCE ? PORTA_CYCLE_LEN : PORTB_CYCLE_LEN);
	}

	if (done)
	{
		m_dma_enabled = 0; //FIXME: Correct?
		m_status = 0x09;

		m_status |= !is_ready() << 1; // ready line status

		if(TRANSFER_MODE == TM_TRANSFER)     m_status |= 0x10;   // no match found

		update_status();
		if (LOG) logerror("Z80DMA '%s' End of Block\n", tag());

		if (INT_ON_END_OF_BLOCK)
		{
			trigger_interrupt(INT_END_OF_BLOCK);
		}

		if (AUTO_RESTART)
		{
			if (LOG) logerror("Z80DMA '%s' Auto Restart\n", tag());

			m_dma_enabled = 1;
			m_addressA = PORTA_ADDRESS;
			m_addressB = PORTB_ADDRESS;
			m_count = BLOCKLEN;
			m_status |= 0x30;
		}
	}
}


//-------------------------------------------------
//  update_status - update DMA status
//-------------------------------------------------

void z80dma_device::update_status()
{
	UINT16 pending_transfer;
	attotime next;

	// no transfer is active right now; is there a transfer pending right now?
	pending_transfer = is_ready() & m_dma_enabled;

	if (pending_transfer)
	{
		m_is_read = true;
		m_cur_cycle = (PORTA_IS_SOURCE ? PORTA_CYCLE_LEN : PORTB_CYCLE_LEN);
		next = attotime::from_hz(clock());
		m_timer->adjust(
			attotime::zero,
			0,
			// 1 byte transferred in 4 clock cycles
			next);
	}
	else
	{
		if (m_is_read)
		{
			// no transfers active right now
			m_timer->reset();
		}
	}

	// set the busreq line
	m_out_busreq_cb(pending_transfer ? ASSERT_LINE : CLEAR_LINE);
}



//**************************************************************************
//  READ/WRITE INTERFACES
//**************************************************************************

//-------------------------------------------------
//  read - register read
//-------------------------------------------------

UINT8 z80dma_device::read()
{
	UINT8 res;

	if(m_read_num_follow == 0) // special case: Legend of Kage on X1 Turbo
		res = m_status;
	else {
		res = m_read_regs_follow[m_read_cur_follow];
	}

	m_read_cur_follow++;

	if(m_read_cur_follow >= m_read_num_follow)
		m_read_cur_follow = 0;

	if (LOG) logerror("Z80DMA '%s' Read %02x\n", tag(), res);

	return res;
}


//-------------------------------------------------
//  write - register write
//-------------------------------------------------

void z80dma_device::write(UINT8 data)
{
	if (m_num_follow == 0)
	{
		m_reset_pointer = 0;

		if ((data & 0x87) == 0) // WR2
		{
			if (LOG) logerror("Z80DMA '%s' WR2 %02x\n", tag(), data);
			WR2 = data;
			if (data & 0x40)
				m_regs_follow[m_num_follow++] = GET_REGNUM(PORTB_TIMING);
		}
		else if ((data & 0x87) == 0x04) // WR1
		{
			if (LOG) logerror("Z80DMA '%s' WR1 %02x\n", tag(), data);
			WR1 = data;
			if (data & 0x40)
				m_regs_follow[m_num_follow++] = GET_REGNUM(PORTA_TIMING);
		}
		else if ((data & 0x80) == 0) // WR0
		{
			if (LOG) logerror("Z80DMA '%s' WR0 %02x\n", tag(), data);
			WR0 = data;
			if (data & 0x08)
				m_regs_follow[m_num_follow++] = GET_REGNUM(PORTA_ADDRESS_L);
			if (data & 0x10)
				m_regs_follow[m_num_follow++] = GET_REGNUM(PORTA_ADDRESS_H);
			if (data & 0x20)
				m_regs_follow[m_num_follow++] = GET_REGNUM(BLOCKLEN_L);
			if (data & 0x40)
				m_regs_follow[m_num_follow++] = GET_REGNUM(BLOCKLEN_H);
		}
		else if ((data & 0x83) == 0x80) // WR3
		{
			if (LOG) logerror("Z80DMA '%s' WR3 %02x\n", tag(), data);
			WR3 = data;
			if (data & 0x08)
				m_regs_follow[m_num_follow++] = GET_REGNUM(MASK_BYTE);
			if (data & 0x10)
				m_regs_follow[m_num_follow++] = GET_REGNUM(MATCH_BYTE);
		}
		else if ((data & 0x83) == 0x81) // WR4
		{
			if (LOG) logerror("Z80DMA '%s' WR4 %02x\n", tag(), data);
			WR4 = data;
			if (data & 0x04)
				m_regs_follow[m_num_follow++] = GET_REGNUM(PORTB_ADDRESS_L);
			if (data & 0x08)
				m_regs_follow[m_num_follow++] = GET_REGNUM(PORTB_ADDRESS_H);
			if (data & 0x10)
				m_regs_follow[m_num_follow++] = GET_REGNUM(INTERRUPT_CTRL);
		}
		else if ((data & 0xC7) == 0x82) // WR5
		{
			if (LOG) logerror("Z80DMA '%s' WR5 %02x\n", tag(), data);
			WR5 = data;
		}
		else if ((data & 0x83) == 0x83) // WR6
		{
			if (LOG) logerror("Z80DMA '%s' WR6 %02x\n", tag(), data);
			m_dma_enabled = 0;

			WR6 = data;

			switch (data)
			{
				case COMMAND_ENABLE_AFTER_RETI:
					fatalerror("Z80DMA '%s' Unimplemented WR6 command %02x\n", tag(), data);
				case COMMAND_READ_STATUS_BYTE:
					if (LOG) logerror("Z80DMA '%s' CMD Read status Byte\n", tag());
					READ_MASK = 1;
					m_read_regs_follow[0] = m_status;
					break;
				case COMMAND_RESET_AND_DISABLE_INTERRUPTS:
					WR3 &= ~0x20;
					m_ip = 0;
					m_ius = 0;
					m_force_ready = 0;
					m_status |= 0x08;
					break;
				case COMMAND_INITIATE_READ_SEQUENCE:
					if (LOG) logerror("Z80DMA '%s' Initiate Read Sequence\n", tag());
					m_read_cur_follow = m_read_num_follow = 0;
					if(READ_MASK & 0x01) { m_read_regs_follow[m_read_num_follow++] = m_status; }
					if(READ_MASK & 0x02) { m_read_regs_follow[m_read_num_follow++] = m_count & 0xff; } //byte counter (low)
					if(READ_MASK & 0x04) { m_read_regs_follow[m_read_num_follow++] = m_count >> 8; } //byte counter (high)
					if(READ_MASK & 0x08) { m_read_regs_follow[m_read_num_follow++] = m_addressA & 0xff; } //port A address (low)
					if(READ_MASK & 0x10) { m_read_regs_follow[m_read_num_follow++] = m_addressA >> 8; } //port A address (high)
					if(READ_MASK & 0x20) { m_read_regs_follow[m_read_num_follow++] = m_addressB & 0xff; } //port B address (low)
					if(READ_MASK & 0x40) { m_read_regs_follow[m_read_num_follow++] = m_addressB >> 8; } //port B address (high)
					break;
				case COMMAND_RESET:
					if (LOG) logerror("Z80DMA '%s' Reset\n", tag());
					m_dma_enabled = 0;
					m_force_ready = 0;
					m_ip = 0;
					m_ius = 0;
					interrupt_check();
					// Needs six reset commands to reset the DMA
					{
						UINT8 WRi;

						for(WRi=0;WRi<7;WRi++)
							REG(WRi,m_reset_pointer) = 0;

						m_reset_pointer++;
						if(m_reset_pointer >= 6) { m_reset_pointer = 0; }
					}
					m_status = 0x38;
					break;
				case COMMAND_LOAD:
					m_force_ready = 0;
					m_addressA = PORTA_ADDRESS;
					m_addressB = PORTB_ADDRESS;
					m_count = BLOCKLEN;
					m_status |= 0x30;

					if (LOG) logerror("Z80DMA '%s' Load A: %x B: %x N: %x\n", tag(), m_addressA, m_addressB, m_count);
					break;
				case COMMAND_DISABLE_DMA:
					if (LOG) logerror("Z80DMA '%s' Disable DMA\n", tag());
					m_dma_enabled = 0;
					break;
				case COMMAND_ENABLE_DMA:
					if (LOG) logerror("Z80DMA '%s' Enable DMA\n", tag());
					m_dma_enabled = 1;
					update_status();
					break;
				case COMMAND_READ_MASK_FOLLOWS:
					if (LOG) logerror("Z80DMA '%s' Set Read Mask\n", tag());
					m_regs_follow[m_num_follow++] = GET_REGNUM(READ_MASK);
					break;
				case COMMAND_CONTINUE:
					if (LOG) logerror("Z80DMA '%s' Continue\n", tag());
					m_count = BLOCKLEN;
					m_dma_enabled = 1;
					//"match not found" & "end of block" status flags zeroed here
					m_status |= 0x30;
					break;
				case COMMAND_RESET_PORT_A_TIMING:
					if (LOG) logerror("Z80DMA '%s' Reset Port A Timing\n", tag());
					PORTA_TIMING = 0;
					break;
				case COMMAND_RESET_PORT_B_TIMING:
					if (LOG) logerror("Z80DMA '%s' Reset Port B Timing\n", tag());
					PORTB_TIMING = 0;
					break;
				case COMMAND_FORCE_READY:
					if (LOG) logerror("Z80DMA '%s' Force Ready\n", tag());
					m_force_ready = 1;
					update_status();
					break;
				case COMMAND_ENABLE_INTERRUPTS:
					if (LOG) logerror("Z80DMA '%s' Enable IRQ\n", tag());
					WR3 |= 0x20;
					break;
				case COMMAND_DISABLE_INTERRUPTS:
					if (LOG) logerror("Z80DMA '%s' Disable IRQ\n", tag());
					WR3 &= ~0x20;
					break;
				case COMMAND_REINITIALIZE_STATUS_BYTE:
					if (LOG) logerror("Z80DMA '%s' Reinitialize status byte\n", tag());
					m_status |= 0x30;
					m_ip = 0;
					break;
				case 0xFB:
				case 0xFF: // TODO: p8k triggers this, it probably crashed.
					if (LOG) logerror("Z80DMA '%s' undocumented command triggered 0x%02X!\n", tag(), data);
					break;
				default:
					printf("Z80DMA '%s' Unknown WR6 command %02x\n", tag(), data);
			}
		}
		else if(data == 0x8e) //newtype on Sharp X1, unknown purpose
			printf("Z80DMA '%s' Unknown base register %02x\n", tag(), data);
		else
			fatalerror("Z80DMA '%s' Unknown base register %02x\n", tag(), data);
		m_cur_follow = 0;
	}
	else
	{
		if (LOG) logerror("Z80DMA '%s' Write %02x\n", tag(), data);

		int nreg = m_regs_follow[m_cur_follow];
		m_regs[nreg] = data;
		m_cur_follow++;
		if (m_cur_follow>=m_num_follow)
			m_num_follow = 0;
		if (nreg == REGNUM(4,3))
		{
			m_num_follow=0;
			if (data & 0x08)
				m_regs_follow[m_num_follow++] = GET_REGNUM(PULSE_CTRL);
			if (data & 0x10)
				m_regs_follow[m_num_follow++] = GET_REGNUM(INTERRUPT_VECTOR);
			m_cur_follow = 0;
		}
		else if(m_regs_follow[m_num_follow] == GET_REGNUM(READ_MASK))
		{
			m_read_cur_follow = m_read_num_follow = 0;

			if(READ_MASK & 0x01) { m_read_regs_follow[m_read_num_follow++] = m_status; }
			if(READ_MASK & 0x02) { m_read_regs_follow[m_read_num_follow++] = m_count & 0xff; } //byte counter (low)
			if(READ_MASK & 0x04) { m_read_regs_follow[m_read_num_follow++] = m_count >> 8; } //byte counter (high)
			if(READ_MASK & 0x08) { m_read_regs_follow[m_read_num_follow++] = m_addressA & 0xff; } //port A address (low)
			if(READ_MASK & 0x10) { m_read_regs_follow[m_read_num_follow++] = m_addressA >> 8; } //port A address (high)
			if(READ_MASK & 0x20) { m_read_regs_follow[m_read_num_follow++] = m_addressB & 0xff; } //port B address (low)
			if(READ_MASK & 0x40) { m_read_regs_follow[m_read_num_follow++] = m_addressB >> 8; } //port B address (high)
		}

		m_reset_pointer++;
		if(m_reset_pointer >= 6) { m_reset_pointer = 0; }
	}
}


//-------------------------------------------------
//  rdy_write_callback - deferred RDY signal write
//-------------------------------------------------

void z80dma_device::rdy_write_callback(int state)
{
	// normalize state
	m_rdy = state;
	m_status = (m_status & 0xFD) | (!is_ready() << 1);

	update_status();

	if (is_ready() && INT_ON_READY)
	{
		trigger_interrupt(INT_RDY);
	}
}


//-------------------------------------------------
//  rdy_w - ready input
//-------------------------------------------------

WRITE_LINE_MEMBER(z80dma_device::rdy_w)
{
	if (LOG) logerror("Z80DMA '%s' RDY: %d Active High: %d\n", tag(), state, READY_ACTIVE_HIGH);
	machine().scheduler().synchronize(FUNC(static_rdy_write_callback), state, (void *)this);
}


//-------------------------------------------------
//  wait_w - wait input
//-------------------------------------------------

WRITE_LINE_MEMBER(z80dma_device::wait_w)
{
}


//-------------------------------------------------
//  bai_w - bus acknowledge input
//-------------------------------------------------

WRITE_LINE_MEMBER(z80dma_device::bai_w)
{
}
