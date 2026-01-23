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

**********************************************************************/

#include "emu.h"
#include "z80dma.h"

#define LOG_DMA     (1U << 1)
#define LOG_INT     (1U << 2)
#define LOG_LINE    (1U << 3)
#define LOG_REGS    (1U << 4)

//#define VERBOSE (LOG_GENERAL | LOG_DMA | LOG_INT | LOG_LINE | LOG_REGS)
#include "logmacro.h"

#define LOGDMA(...) LOGMASKED(LOG_DMA, __VA_ARGS__)
#define LOGINT(...) LOGMASKED(LOG_INT, __VA_ARGS__)
#define LOGLINE(...) LOGMASKED(LOG_LINE, __VA_ARGS__)
#define LOGREGS(...) LOGMASKED(LOG_REGS, __VA_ARGS__)


/****************************************************************************
 * CONSTANTS
 ****************************************************************************/
enum
{
	INT_RDY = 0,
	INT_MATCH,
	INT_END_OF_BLOCK,
	INT_MATCH_END_OF_BLOCK
};

/****************************************************************************
 * MACROS
 ****************************************************************************/
#define GET_REGNUM(_r)          (&(_r) - &(WR0))
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

#define PORTA_IS_SOURCE         ((WR0 >> 2) & 0x01)
#define PORTB_IS_SOURCE         (!PORTA_IS_SOURCE)
#define TRANSFER_MODE           (WR0 & 0x03)

#define OPERATING_MODE          ((WR4 >> 5) & 0x03) // 0b00: Byte; 0b01: Continuous; 0b10: Burst; 0b11: Do not program

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
#define PULSE_GENERATED         (INTERRUPT_CTRL & 0x04)


/****************************************************************************
 * device type definition
 ****************************************************************************/
DEFINE_DEVICE_TYPE(Z80DMA, z80dma_device, "z80dma", "Zilog Z80 DMA Controller")

/****************************************************************************
 * z80dma_device - constructor
 ****************************************************************************/
z80dma_device::z80dma_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: z80dma_device(mconfig, Z80DMA, tag, owner, clock)
{
}

z80dma_device::z80dma_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_z80daisy_interface(mconfig, *this)
	, m_out_busreq_cb(*this)
	, m_out_int_cb(*this)
	, m_out_ieo_cb(*this)
	, m_out_bao_cb(*this)
	, m_in_mreq_cb(*this, 0)
	, m_out_mreq_cb(*this)
	, m_in_iorq_cb(*this, 0)
	, m_out_iorq_cb(*this)
{
}

/****************************************************************************
 * device_start - device-specific startup
 ****************************************************************************/
void z80dma_device::device_start()
{
	// allocate timer
	m_timer = timer_alloc(FUNC(z80dma_device::clock_w), this);

	// register for state saving
	save_item(NAME(m_regs));
	save_item(NAME(m_regs_follow));
	save_item(NAME(m_num_follow));
	save_item(NAME(m_cur_follow));
	save_item(NAME(m_read_cur_follow));
	save_item(NAME(m_status));
	save_item(NAME(m_dma_seq));
	save_item(NAME(m_vector));
	save_item(NAME(m_iei));
	save_item(NAME(m_ip));
	save_item(NAME(m_ius));
	save_item(NAME(m_addressA));
	save_item(NAME(m_addressB));
	save_item(NAME(m_count));
	save_item(NAME(m_byte_counter));
	save_item(NAME(m_rdy));
	save_item(NAME(m_force_ready));
	save_item(NAME(m_wait));
	save_item(NAME(m_waits_extra));
	save_item(NAME(m_busrq));
	save_item(NAME(m_busrq_ack));
	save_item(NAME(m_is_pulse));
	save_item(NAME(m_latch));
}


/****************************************************************************
 * device_reset - device-specific reset
 ****************************************************************************/
void z80dma_device::device_reset()
{
	m_timer->reset();

	m_status = 0;
	m_dma_seq = ~0;
	m_rdy = 0;
	m_force_ready = 0;
	m_wait = 0;
	m_waits_extra = 0;
	m_num_follow = 0;
	m_read_cur_follow = 0;
	m_reset_pointer = 0;
	set_busrq(CLEAR_LINE);
	m_busrq_ack = 0;
	m_is_pulse = false;
	memset(m_regs, 0, sizeof(m_regs));
	memset(m_regs_follow, 0, sizeof(m_regs_follow));

	// disable interrupts
	WR3 &= ~0x20;
	m_ip = 0;
	m_ius = 0;
	m_vector = 0;
}


/****************************************************************************
 * DAISY CHAIN INTERFACE
 ****************************************************************************/

/****************************************************************************
 * z80daisy_irq_state - return the overall IRQ
 * state for this device
 ****************************************************************************/
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
	LOGINT("Z80DMA Interrupt State: %u\n", state);

	return state;
}

/****************************************************************************
 * z80daisy_irq_ack - acknowledge an IRQ and
 * return the appropriate vector
 ****************************************************************************/
int z80dma_device::z80daisy_irq_ack()
{
	if (m_ip)
	{
		LOGINT("Z80DMA Interrupt Acknowledge\n");

		// clear interrupt pending flag
		m_ip = 0;
		interrupt_check();

		// set interrupt under service flag
		m_ius = 1;

		return m_vector;
	}
	else
	{
		LOGINT("z80dma_irq_ack: failed to find an interrupt to ack!\n");
		return 0;
	}
}

/****************************************************************************
 * z80daisy_irq_reti - clear the interrupt
 * pending state to allow other interrupts through
 ****************************************************************************/
void z80dma_device::z80daisy_irq_reti()
{
	if (m_ius)
	{
		LOGINT("Z80DMA Return from Interrupt\n");

		// clear interrupt under service flag
		m_ius = 0;
		interrupt_check();
	}
	else
	{
		LOGINT("z80dma_irq_reti: failed to find an interrupt to clear IEO on!\n");
	}
}


//**************************************************************************
//  INTERNAL STATE MANAGEMENT
//**************************************************************************

void z80dma_device::enable()
{
	const attotime curtime = machine().time();
	m_timer->adjust(attotime::from_ticks(curtime.as_ticks(clock()) + 1, clock()) - curtime, 0, clocks_to_attotime(1));
	m_dma_seq = SEQ_WAIT_READY;
}

void z80dma_device::disable()
{
	m_timer->reset();
	if (m_busrq_ack == 1)
	{
		set_busrq(CLEAR_LINE);
	}
}

void z80dma_device::update_bao()
{
	m_out_bao_cb(!m_busrq && m_busrq_ack);
}

void z80dma_device::set_busrq(int state)
{
	m_busrq = state;
	m_out_busreq_cb(m_busrq);
	update_bao();
}

/****************************************************************************
 * is_ready - ready for DMA transfer?
 ****************************************************************************/
int z80dma_device::is_ready()
{
	return m_force_ready || (m_rdy == READY_ACTIVE_HIGH);
}

/****************************************************************************
 * interrupt_check - update IRQ line state
 ****************************************************************************/
void z80dma_device::interrupt_check()
{
	m_out_int_cb(m_ip ? ASSERT_LINE : CLEAR_LINE);
	m_out_ieo_cb(m_ip ? 0 : m_iei);
}

/****************************************************************************
 * trigger_interrupt - trigger DMA interrupt
 ****************************************************************************/
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

		LOGINT("Z80DMA Interrupt Pending\n");

		interrupt_check();
	}
}

/****************************************************************************
 * do_read - perform DMA read
 ****************************************************************************/
void z80dma_device::do_read()
{
	switch (TRANSFER_MODE)
	{
	case TM_TRANSFER:
	case TM_SEARCH:
	case TM_SEARCH_TRANSFER:
		if (PORTA_IS_SOURCE)
		{
			if (PORTA_MEMORY)
				m_latch = m_in_mreq_cb(m_addressA);
			else
				m_latch = m_in_iorq_cb(m_addressA);

			LOGDMA("Z80DMA A src: %04x %s -> data: %02x\n", m_addressA, PORTA_MEMORY ? "mem" : "i/o", m_latch);
		}
		else
		{
			if (PORTB_MEMORY)
				m_latch = m_in_mreq_cb(m_addressB);
			else
				m_latch = m_in_iorq_cb(m_addressB);

			LOGDMA("Z80DMA B src: %04x %s -> data: %02x\n", m_addressB, PORTB_MEMORY ? "mem" : "i/o", m_latch);
		}
		break;
	default:
		logerror("z80dma_do_operation: invalid mode %d!\n", TRANSFER_MODE);
		break;
	}
}

/****************************************************************************
 * do_write - perform DMA write
 ****************************************************************************/
void z80dma_device::do_transfer_write()
{
	if (PORTA_IS_SOURCE)
	{
		if (PORTB_MEMORY)
			m_out_mreq_cb((offs_t)m_addressB, m_latch);
		else
			m_out_iorq_cb((offs_t)m_addressB, m_latch);

		LOGDMA("Z80DMA B dst: %04x %s\n", m_addressB, PORTB_MEMORY ? "mem" : "i/o");
	}
	else
	{
		if (PORTA_MEMORY)
			m_out_mreq_cb((offs_t)m_addressA, m_latch);
		else
			m_out_iorq_cb((offs_t)m_addressA, m_latch);

		LOGDMA("Z80DMA A dst: %04x %s\n", m_addressA, PORTA_MEMORY ? "mem" : "i/o");
	}
}

void z80dma_device::do_search()
{
	const u8 load_byte = m_latch | MASK_BYTE;
	const u8 match_byte = MATCH_BYTE | MASK_BYTE;
	LOGDMA("SEARCH: %02x %02x\n", load_byte, match_byte);
	if (INT_ON_MATCH && load_byte == match_byte)
	{
		trigger_interrupt(INT_MATCH);
	}
}

void z80dma_device::do_write()
{
	switch (TRANSFER_MODE)
	{
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
		logerror("z80dma_do_operation: invalid mode %d!\n", TRANSFER_MODE);
		break;
	}

	m_addressA += PORTA_FIXED ? 0 : PORTA_INC ? 1 : -1;
	m_addressB += PORTB_FIXED ? 0 : PORTB_INC ? 1 : -1;

	m_byte_counter++;
}

/****************************************************************************
 * clock_w - raising edge
 ****************************************************************************/
TIMER_CALLBACK_MEMBER(z80dma_device::clock_w)
{
	switch (m_dma_seq)
	{
	case SEQ_WAIT_READY:
		if (is_ready())
		{
			// Continuos mode only request BUS during start
			if (OPERATING_MODE != 0b01 || m_byte_counter == 0)
			{
				m_dma_seq = SEQ_REQUEST_BUS;
			}
			else
			{
				m_dma_seq = SEQ_TRANS1_INC_DEC_SOURCE_ADDRESS;
			}
		}
		break;

	case SEQ_REQUEST_BUS:
		if (m_busrq_ack == 0)
		{
			set_busrq(ASSERT_LINE);
		}
		m_dma_seq = SEQ_WAITING_ACK;
		break;

	case SEQ_WAITING_ACK:
		if (m_busrq_ack == 1)
		{
			m_dma_seq = SEQ_TRANS1_INC_DEC_SOURCE_ADDRESS;
		}
		break;

	case SEQ_TRANS1_INC_DEC_SOURCE_ADDRESS:
		{
			if (PULSE_GENERATED && (m_byte_counter & 0xff) == PULSE_CTRL)
			{
				m_is_pulse = true;
				m_out_int_cb(ASSERT_LINE);
			}

			const attotime clock = clocks_to_attotime(1);
			const attotime next = clock * (3 - ((PORTA_IS_SOURCE ? PORTA_TIMING : PORTB_TIMING) & 0x03) + m_waits_extra);
			m_waits_extra = 0;
			m_timer->adjust(next, 0, clock);

			m_dma_seq = SEQ_TRANS1_READ_SOURCE;
		}
		break;

	case SEQ_TRANS1_READ_SOURCE:
		if (!m_wait)
		{
			// TODO: there's a nasty recursion bug with Alpha for Sharp X1 Turbo on the transfers with this function!
			do_read();

			m_dma_seq = SEQ_TRANS1_INC_DEC_DEST_ADDRESS;
		}
		break;

	case SEQ_TRANS1_INC_DEC_DEST_ADDRESS:
		{
			const attotime clock = clocks_to_attotime(1);
			const attotime next = clock * (3 - ((PORTB_IS_SOURCE ? PORTA_TIMING : PORTB_TIMING) & 0x03) + m_waits_extra);
			m_waits_extra = 0;
			m_timer->adjust(next, 0, clock);

			m_dma_seq = SEQ_TRANS1_WRITE_DEST;
		}
		break;

	case SEQ_TRANS1_WRITE_DEST:
		if (!m_wait)
		{
			// hack?: count=0 cause infinite loop in 'x1turbo40 suikoden' and makes it work.
			const bool is_final = m_count && m_byte_counter == m_count;

			do_write();

			if (PULSE_GENERATED && m_is_pulse)
			{
				m_out_int_cb(CLEAR_LINE);
				m_is_pulse = false;
			}

			const u8 mode = is_final ? 0b11 : OPERATING_MODE;
			switch (mode)
			{
			case 0b00: // Byte/Single/byte-at-a-time
				set_busrq(CLEAR_LINE);
				m_dma_seq = SEQ_WAIT_READY;
				break;

			case 0b10: // Burst/Demand
				if (is_ready())
				{
					m_dma_seq = SEQ_TRANS1_INC_DEC_SOURCE_ADDRESS;
				}
				else
				{
					set_busrq(CLEAR_LINE);
					m_dma_seq = SEQ_WAIT_READY;
				}
				break;

			case 0b01: // Continuous/Block
				m_dma_seq = is_ready() ? SEQ_TRANS1_INC_DEC_SOURCE_ADDRESS : SEQ_WAIT_READY;
				break;

			default: // Undefined || final
				m_dma_seq = SEQ_FINISH;
				break;
			}
		}
		break;

	case SEQ_TRANS1_BYTE_MATCH:
	case SEQ_TRANS1_INC_BYTE_COUNTER:
	case SEQ_TRANS1_SET_FLAGS:
	case SEQ_FINISH:
		disable();
		m_status = 0x09;
		m_status |= !is_ready() << 1; // ready line status

		if (TRANSFER_MODE == TM_TRANSFER)
			m_status |= 0x10;   // no match found

		LOGDMA("Z80DMA End of Block\n");

		if (INT_ON_END_OF_BLOCK)
		{
			trigger_interrupt(INT_END_OF_BLOCK);
		}

		if (AUTO_RESTART)
		{
			LOGDMA("Z80DMA Auto Restart\n");

			m_addressA = PORTA_ADDRESS;
			m_addressB = PORTB_ADDRESS;
			m_count = BLOCKLEN;
			m_byte_counter = 0;
			m_status |= 0x30;
			enable();
		}
		break;

	default:
		break;
	}
}


/****************************************************************************
 * READ/WRITE INTERFACES
 ****************************************************************************/

/****************************************************************************
 * read - register read
 ****************************************************************************/
u8 z80dma_device::read()
{
	u8 res = 0;
	switch (m_read_cur_follow)
	{
	case 0:
		res = m_status; //current status
		break;

	case 1:
		res = m_byte_counter & 0xff; //byte counter (low)
		break;

	case 2:
		res = m_byte_counter >> 8; //byte counter (high)
		break;

	case 3:
		res = m_addressA & 0xff; //port A address (low)
		break;

	case 4:
		res = m_addressA >> 8; //port A address (high)
		break;

	case 5:
		res = m_addressB & 0xff; //port B address (low)
		break;

	case 6:
		res = m_addressB >> 8; //port B address (high)
		break;
	}

	if (!machine().side_effects_disabled())
	{
		LOGREGS("%s: RR%d %02x\n", machine().describe_context(), m_read_cur_follow, res);

		if ((READ_MASK & (READ_MASK - 1)) != 0)
			setup_next_read((m_read_cur_follow + 1) & 7);
	}

	return res;
}

void z80dma_device::setup_next_read(int rr)
{
	if (!READ_MASK)
		return;

	while (!BIT(READ_MASK, rr))
		rr = (rr + 1) & 7;
	m_read_cur_follow = rr;
}

/****************************************************************************
 * write - register write
 ****************************************************************************/
void z80dma_device::write(u8 data)
{
	if (m_num_follow == 0)
	{
		m_reset_pointer = 0;

		if ((data & 0x87) == 0) // WR2
		{
			LOGREGS("%s: WR2 %02x\n", machine().describe_context(), data);
			WR2 = data;
			if (data & 0x40)
				m_regs_follow[m_num_follow++] = GET_REGNUM(PORTB_TIMING);
		}
		else if ((data & 0x87) == 0x04) // WR1
		{
			LOGREGS("%s: WR1 %02x\n", machine().describe_context(), data);
			WR1 = data;
			if (data & 0x40)
				m_regs_follow[m_num_follow++] = GET_REGNUM(PORTA_TIMING);
		}
		else if ((data & 0x80) == 0) // WR0
		{
			LOGREGS("%s: WR0 %02x\n", machine().describe_context(), data);
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
			LOGREGS("%s: WR3 %02x\n", machine().describe_context(), data);
			WR3 = data;
			if (data & 0x08)
				m_regs_follow[m_num_follow++] = GET_REGNUM(MASK_BYTE);
			if (data & 0x10)
				m_regs_follow[m_num_follow++] = GET_REGNUM(MATCH_BYTE);

			if (BIT(data, 6))
			{
				enable();
			}
		}
		else if ((data & 0x83) == 0x81) // WR4
		{
			LOGREGS("%s: WR4 %02x\n", machine().describe_context(), data);
			WR4 = data;
			if (data & 0x04)
				m_regs_follow[m_num_follow++] = GET_REGNUM(PORTB_ADDRESS_L);
			if (data & 0x08)
				m_regs_follow[m_num_follow++] = GET_REGNUM(PORTB_ADDRESS_H);
			if (data & 0x10)
				m_regs_follow[m_num_follow++] = GET_REGNUM(INTERRUPT_CTRL);
		}
		else if ((data & 0xc7) == 0x82) // WR5
		{
			LOGREGS("%s: WR5 %02x\n", machine().describe_context(), data);
			WR5 = data;
		}
		else if ((data & 0x83) == 0x83) // WR6
		{
			LOGREGS("%s: WR6 %02x\n", machine().describe_context(), data);
			WR6 = data;

			switch (data)
			{
			case COMMAND_ENABLE_AFTER_RETI:
				fatalerror("Unimplemented WR6 command %02x\n", data);
			case COMMAND_READ_STATUS_BYTE:
				LOGREGS("CMD Read status Byte\n");
				READ_MASK = 1;
				m_read_cur_follow = 0;
				break;
			case COMMAND_RESET_AND_DISABLE_INTERRUPTS:
				LOGREGS("CMD Reset and Disable Interrupts\n");
				WR3 &= ~0x20;
				m_ip = 0;
				m_ius = 0;
				m_force_ready = 0;
				m_status |= 0x08;
				break;
			case COMMAND_INITIATE_READ_SEQUENCE:
				LOGREGS("CMD Initiate Read Sequence\n");
				setup_next_read(0);
				break;
			case COMMAND_RESET:
				LOGREGS("CMD Reset\n");
				disable();
				m_force_ready = 0;
				m_ip = 0;
				m_ius = 0;
				interrupt_check();
				// Needs six reset commands to reset the DMA
				{
					for (u8 WRi = 0; WRi < 7; WRi++)
						REG(WRi,m_reset_pointer) = 0;

					m_reset_pointer++;
					if (m_reset_pointer >= 6)
					{
						m_reset_pointer = 0;
					}
				}
				m_status = 0x38;
				break;
			case COMMAND_LOAD:
				m_force_ready = 0;
				m_addressA = PORTA_ADDRESS;
				m_addressB = PORTB_ADDRESS;
				m_count = BLOCKLEN;
				m_byte_counter = 0;
				m_status |= 0x30;

				LOGREGS("CMD Load A: %x B: %x N: %x\n", m_addressA, m_addressB, m_count);
				break;
			case COMMAND_DISABLE_DMA:
				LOGREGS("CMD Disable DMA\n");
				disable();
				break;
			case COMMAND_ENABLE_DMA:
				LOGREGS("CMD Enable DMA\n");
				enable();
				break;
			case COMMAND_READ_MASK_FOLLOWS:
				LOGREGS("CMD Set Read Mask\n");
				m_regs_follow[m_num_follow++] = GET_REGNUM(READ_MASK);
				break;
			case COMMAND_CONTINUE:
				LOGREGS("CMD Continue\n");
				m_count = BLOCKLEN;
				m_byte_counter = 0;
				//enable(); //???m_dma_enabled = 1;
				//"match not found" & "end of block" status flags zeroed here
				m_status |= 0x30;
				break;
			case COMMAND_RESET_PORT_A_TIMING:
				LOGREGS("CMD Reset Port A Timing\n");
				PORTA_TIMING = 0;
				break;
			case COMMAND_RESET_PORT_B_TIMING:
				LOGREGS("CMD Reset Port B Timing\n");
				PORTB_TIMING = 0;
				break;
			case COMMAND_FORCE_READY:
				LOGREGS("CMD Force Ready\n");
				m_force_ready = 1;
				break;
			case COMMAND_ENABLE_INTERRUPTS:
				LOGREGS("CMD Enable IRQ\n");
				WR3 |= 0x20;
				break;
			case COMMAND_DISABLE_INTERRUPTS:
				LOGREGS("CMD Disable IRQ\n");
				WR3 &= ~0x20;
				break;
			case COMMAND_REINITIALIZE_STATUS_BYTE:
				LOGREGS("CMD Reinitialize status byte\n");
				m_status |= 0x30;
				m_ip = 0;
				break;
			case 0xfB:
			case 0xff: // TODO: p8k triggers this, it probably crashed.
				logerror("Z80DMA undocumented command triggered 0x%02X!\n", data);
				break;
			default:
				logerror("Z80DMA Unknown WR6 command %02x\n", data);
			}
		}
		else
		{
			// 0x8e: newtype on Sharp X1, unknown purpose
			logerror("Z80DMA Unknown base register %02x\n", data);
		}
		m_cur_follow = 0;
	}
	else
	{
		LOGREGS("%s: Write %02x\n", machine().describe_context(), data);

		int nreg = m_regs_follow[m_cur_follow];
		m_regs[nreg] = data;
		m_cur_follow++;
		if (m_cur_follow >= m_num_follow)
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
		else if (m_regs_follow[m_num_follow] == GET_REGNUM(READ_MASK))
		{
			setup_next_read(0);
		}

		m_reset_pointer++;
		if (m_reset_pointer >= 6)
		{
			m_reset_pointer = 0;
		}
	}
}

/****************************************************************************
 * rdy_write_callback - deferred RDY signal write
 ****************************************************************************/
TIMER_CALLBACK_MEMBER(z80dma_device::rdy_write_callback)
{
	// normalize state
	m_rdy = param;
	m_status = (m_status & 0xfd) | (!is_ready() << 1);

	if (is_ready() && INT_ON_READY)
	{
		trigger_interrupt(INT_RDY);
	}
}

/****************************************************************************
 * rdy_w - ready input
 ****************************************************************************/
void z80dma_device::rdy_w(int state)
{
	LOGLINE("Z80DMA RDY: %d Active High: %d\n", state, READY_ACTIVE_HIGH);
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(z80dma_device::rdy_write_callback), this), state);
}

/****************************************************************************
 * bai_w - bus acknowledge input
 ****************************************************************************/
void z80dma_device::bai_w(int state)
{
	m_busrq_ack = state;
	update_bao();
}


DEFINE_DEVICE_TYPE(UA858D, ua858d_device, "ua858d", "Mikroelektronik UA858D DMA Controller")

ua858d_device::ua858d_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: z80dma_device(mconfig, UA858D, tag, owner, clock)
{
}

void ua858d_device::write(u8 data)
{
	if ((m_num_follow == 0) && ((data & 0x83) == 0x80)) // WR3
	{
		LOGREGS("%s: WR3 %02x\n", machine().describe_context(), data);
		WR3 = data;
		if (data & 0x08)
			m_regs_follow[m_num_follow++] = GET_REGNUM(MASK_BYTE);
		if (data & 0x10)
			m_regs_follow[m_num_follow++] = GET_REGNUM(MATCH_BYTE);
	}
	else
	{
		z80dma_device::write(data);
	}
}
