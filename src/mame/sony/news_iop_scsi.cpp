// license:BSD-3-Clause
// copyright-holders:AJR

/*
 * Sony NEWS 68k IOP SCSI DMA helper
 *
 * The dual-processor 68k Sony NEWS computers use some tricks similar to the Apple Macintosh
 * for handling SCSI DMA. The I/O Processor (IOP) is the DMA controller for most peripherals
 * including SCSI. The SCSI controller feeds DRQ into the BERR signal, which in most cases will
 * cause the IOP to stall in the bus error handler of iopboot (still need to confirm for '030 IOPs
 * running rtx or mrx) until more data is avaliable. NEWS-OS 4's iopboot mini-OS uses longword
 * transactions to read from the 8-bit pseudo-DMA port. This relies on how the 68k handles bus
 * transactions when reading from a narrower device, where the processor will retry reading from the port
 * after the bus error handler finishes. With how the current 68k core handles bus reads, this
 * does not really work. Therefore, the dual-processor 68k NEWS systems use this handler to
 * snoop on the IOP's interactions with the SCSI controller. When the IOP starts a SCSI transaction,
 * this handler halts the IOP and primes the read buffer. Once a longword has been read, the IOP is resumed.
 * This cycle of pause/resume continues until the SCSI transaction is complete. iopboot uses byte-transactions
 * until the count is longword aligned before starting this process, so this more or less emulates (at a high level)
 * what iopboot expects when the bus error handler and processor routines successfully read in a longword.
 *
 * Thanks to AJR - the macscsi helper was easy to modify for this!
 * There are two key differences between this and the macscsi helper.
 * 1. The IRQ signal is used by NEWS, and the timing is important. Therefore, IRQs are suppressed until the FIFO is drained by the IOP
 *    so that the timing of IRQ can be preserved. Phase change interrupts, for example, will happen as soon as the FIFO is filled and
 *    there are no bytes left to transfer, which will interrupt the IOP too early.
 * 2. iopboot does not check DRQ before trying to read from the SCSI controller, because its BERR handler
 *    has a retry loop built in, so it doesn't care if the SCSI chip is ready or not. Because of this, the helper will
 *    automatically transition to filling the FIFO once DRQ is active if iopboot primed the controller for reading.
 */

#include "emu.h"
#include "news_iop_scsi.h"

#define LOG_DATA (1U << 1)
#define LOG_DRQ (1U << 2)
#define LOG_IRQ (1U << 3)

// #define VERBOSE (LOG_GENERAL|LOG_DRQ|LOG_IRQ)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(NEWS_IOP_SCSI_HELPER, news_iop_scsi_helper_device, "news_iop_scsi_dma", "Sony NEWS SCSI DMA Helper")

ALLOW_SAVE_TYPE(news_iop_scsi_helper_device::mode);

// TODO: adjustments to the below for NEWS?
static constexpr u8 BAD_BYTE = 0xbb;

news_iop_scsi_helper_device::news_iop_scsi_helper_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, NEWS_IOP_SCSI_HELPER, tag, owner, clock),
	m_scsi_read_callback(*this, BAD_BYTE),
	m_scsi_write_callback(*this),
	m_scsi_dma_read_callback(*this, BAD_BYTE),
	m_scsi_dma_write_callback(*this),
	m_iop_halt_callback(*this),
	m_bus_error_callback(*this),
	m_irq_out_callback(*this),
	m_timeout(attotime::from_usec(16)), // TODO: proper value for NEWS
	m_pseudo_dma_timer(nullptr),
	m_mode(mode::NON_DMA),
	m_read_fifo_bytes(0),
	m_write_fifo_bytes(0),
	m_read_fifo_data(0),
	m_write_fifo_data(0)
{
}

void news_iop_scsi_helper_device::device_start()
{
	m_pseudo_dma_timer = timer_alloc(FUNC(news_iop_scsi_helper_device::timer_callback), this);

	save_item(NAME(m_mode));
	save_item(NAME(m_read_fifo_bytes));
	save_item(NAME(m_write_fifo_bytes));
	save_item(NAME(m_read_fifo_data));
	save_item(NAME(m_write_fifo_data));
	save_item(NAME(m_irq));
}

void news_iop_scsi_helper_device::device_reset()
{
	dma_stop();

	m_iop_halt_callback(CLEAR_LINE);
	m_pseudo_dma_timer->enable(false);
}

void news_iop_scsi_helper_device::read_fifo_process()
{
	assert(m_read_fifo_bytes < 4);

	u8 data = m_scsi_dma_read_callback(6);
	++m_read_fifo_bytes;
	LOGMASKED(LOG_DATA, "Read byte %02X into FIFO (%d/4 filled)\n", data, m_read_fifo_bytes);
	m_read_fifo_data |= u32(data) << (32 - m_read_fifo_bytes * 8);
	if (m_read_fifo_bytes != 4)
	{
		m_pseudo_dma_timer->adjust(m_timeout);
	}
	else
	{
		m_iop_halt_callback(CLEAR_LINE);
		m_pseudo_dma_timer->enable(false);
	}
}

void news_iop_scsi_helper_device::write_fifo_process()
{
	assert(m_write_fifo_bytes != 0);
	--m_write_fifo_bytes;
	u8 data = BIT(m_write_fifo_data, m_write_fifo_bytes * 8, 8);
	LOGMASKED(LOG_DATA, "Write byte %02X from FIFO (%d left)\n", data, m_write_fifo_bytes);
	m_scsi_dma_write_callback(0, data);

	if (m_write_fifo_bytes != 0)
	{
		m_pseudo_dma_timer->adjust(m_timeout);
	}
	else
	{
		m_iop_halt_callback(CLEAR_LINE);
		m_pseudo_dma_timer->enable(false);
	}
}

void news_iop_scsi_helper_device::drq_w(int state)
{
	LOGMASKED(LOG_DRQ, "drq_w(0x%x)\n", state);
	if (state)
	{
		if (m_mode == mode::READ_DMA && m_read_fifo_bytes < 4)
		{
			read_fifo_process();
		}
		else if ((m_mode == mode::WRITE_DMA || m_mode == mode::BAD_DMA) && m_write_fifo_bytes != 0)
		{
			write_fifo_process();
		}
		else if (m_mode == mode::READ_WAIT_DRQ)
		{
			m_mode = mode::READ_DMA;
			m_read_fifo_data = u32(m_scsi_dma_read_callback(6)) << 24;
			LOGMASKED(LOG_DATA, "%s: Pseudo-DMA read started from DRQ: first byte = %02X\n", machine().describe_context(), m_read_fifo_data >> 24);
			m_read_fifo_bytes = 1;
			m_pseudo_dma_timer->adjust(m_timeout);
			m_iop_halt_callback(ASSERT_LINE);
		}
	}
}

void news_iop_scsi_helper_device::irq_w(int state)
{
	LOGMASKED(LOG_IRQ, "irq_w(0x%x)\n", state);
	m_irq = state > 0;
	if (m_irq && (m_read_fifo_bytes != 0 || m_write_fifo_bytes != 0))
	{
		LOGMASKED(LOG_IRQ, "Gating IRQ\n");
		m_mode = mode::IRQ_FIFO_DRAIN;
		m_irq_out_callback(false);
	}
	else
	{
		m_irq_out_callback(m_irq);
	}
}

TIMER_CALLBACK_MEMBER(news_iop_scsi_helper_device::timer_callback)
{
	if (m_mode == mode::WRITE_DMA && m_write_fifo_bytes != 0 && BIT(m_scsi_read_callback(5), 6))
	{
		write_fifo_process();
	}
	else if (m_mode == mode::READ_DMA || m_mode == mode::WRITE_DMA)
	{
		LOG("DMA timed out\n");
		m_iop_halt_callback(CLEAR_LINE);
	}
	else
	{
		dma_stop();
	}
}

void news_iop_scsi_helper_device::dma_stop()
{
	if (m_read_fifo_bytes != 0)
	{
		logerror("%s: %d unread byte(s) lost (%08X)\n", machine().describe_context(), m_read_fifo_bytes, m_read_fifo_data);
	}

	if (m_write_fifo_bytes != 0)
	{
		logerror("%s: %d unwritten byte(s) lost (%08X)\n", machine().describe_context(), m_write_fifo_bytes, m_write_fifo_data);
	}

	m_mode = mode::NON_DMA;
	m_read_fifo_bytes = 0;
	m_write_fifo_bytes = 0;
	m_read_fifo_data = 0;
	m_write_fifo_data = 0;
}

u8 news_iop_scsi_helper_device::read_wrapper(bool pseudo_dma, offs_t offset)
{
	LOGMASKED(LOG_DATA, "read_wrapper(%s, 0x%x)\n", pseudo_dma ? "true" : "false", offset);
	u8 data = BAD_BYTE;
	switch (offset & 7)
	{
	case 2:
		data = m_scsi_read_callback(2);
		if (!machine().side_effects_disabled() && !BIT(data, 1))
		{
			dma_stop();
		}
		break;

	case 5:
		data = m_scsi_read_callback(5);
		if (!machine().side_effects_disabled())
		{
			if (!BIT(m_scsi_read_callback(2), 1))
			{
				dma_stop();
			}

			if (m_mode == mode::READ_WAIT_DRQ && BIT(data, 6))
			{
				m_mode = mode::READ_DMA;
				m_read_fifo_data = u32(m_scsi_dma_read_callback(6)) << 24;
				LOGMASKED(LOG_DATA, "%s: Pseudo-DMA read started: first byte = %02X\n", machine().describe_context(), m_read_fifo_data >> 24);
				m_read_fifo_bytes = 1;
				m_pseudo_dma_timer->adjust(m_timeout);
				m_iop_halt_callback(ASSERT_LINE);
			}
		}

		if ((m_mode == mode::READ_DMA && m_read_fifo_bytes != 0) || (m_mode == mode::WRITE_DMA && m_write_fifo_bytes < 4))
		{
			data |= 0x40;
		}
		break;

	case 6:
		if (!machine().side_effects_disabled() && !BIT(m_scsi_read_callback(2), 1))
		{
			dma_stop();
		}

		if (m_read_fifo_bytes != 0)
		{
			data = BIT(m_read_fifo_data, 24, 8);
			if (!machine().side_effects_disabled())
			{
				--m_read_fifo_bytes;
				LOGMASKED(LOG_DATA, "%s: CPU read byte %02X from FIFO (%d left)\n", machine().describe_context(), data, m_read_fifo_bytes);
				m_read_fifo_data <<= 8;
				if (BIT(m_scsi_read_callback(5), 6) && m_mode != mode::IRQ_FIFO_DRAIN)
				{
					read_fifo_process();
				}
				else if (!m_pseudo_dma_timer->enabled() && m_mode != mode::IRQ_FIFO_DRAIN)
				{
					m_pseudo_dma_timer->adjust(m_timeout);
					m_iop_halt_callback(ASSERT_LINE);
				}
				else if (m_read_fifo_bytes == 0 && m_mode == mode::IRQ_FIFO_DRAIN)
				{
					m_irq_out_callback(m_irq);
				}
			}
		}
		else if (m_mode != mode::NON_DMA && pseudo_dma)
		{
			if (m_mode != mode::BAD_DMA)
			{
				fatalerror("%s: Read underflow on SCSI pseudo-DMA\n", machine().describe_context());
				m_mode = mode::BAD_DMA;
			}
			m_bus_error_callback(READ_ERROR);
			m_pseudo_dma_timer->enable(false);
		}
		else
		{
			data = pseudo_dma ? m_scsi_dma_read_callback(6) : m_scsi_read_callback(6);
		}
		break;

	default:
		if (!machine().side_effects_disabled() && !BIT(m_scsi_read_callback(2), 1))
		{
			dma_stop();
		}
		data = m_scsi_read_callback(offset & 7);
		break;
	}

	return data;
}

void news_iop_scsi_helper_device::write_wrapper(bool pseudo_dma, offs_t offset, u8 data)
{
	switch (offset & 7)
	{
	case 0:
		if (m_mode == mode::BAD_DMA && pseudo_dma)
		{
			m_bus_error_callback(WRITE_ERROR);
		}
		else if (m_mode == mode::WRITE_DMA && (m_write_fifo_bytes != 0 || !BIT(m_scsi_read_callback(5), 6)))
		{
			if (m_write_fifo_bytes < 4)
			{
				m_write_fifo_data = (m_write_fifo_data << 8) | data;
				++m_write_fifo_bytes;
				LOGMASKED(LOG_DATA, "%s: CPU writing byte %02X into FIFO (%d/4 filled)\n", machine().describe_context(), data, m_write_fifo_bytes);
				if (!m_pseudo_dma_timer->enabled())
				{
					m_pseudo_dma_timer->adjust(m_timeout);
					m_iop_halt_callback(ASSERT_LINE);
				}
			}
			else if (pseudo_dma)
			{
				logerror("%s: Write overflow on SCSI pseudo-DMA\n", machine().describe_context());
				m_bus_error_callback(WRITE_ERROR);
				m_mode = mode::BAD_DMA;
			}
		}
		else if (pseudo_dma)
		{
			m_scsi_dma_write_callback(0, data);
		}
		else
		{
			m_scsi_write_callback(0, data);
		}
		break;

	case 2:
		if (!BIT(data, 1))
		{
			dma_stop();
		}
		m_scsi_write_callback(2, data);
		break;

	case 5:
		if (m_mode == mode::NON_DMA && BIT(m_scsi_read_callback(2), 1))
		{
			m_mode = mode::WRITE_DMA;
		}
		m_scsi_write_callback(5, data);
		break;

	case 6:
	case 7:
		if (m_mode == mode::NON_DMA && BIT(m_scsi_read_callback(2), 1))
		{
			m_mode = mode::READ_WAIT_DRQ;
		}
		m_scsi_write_callback(offset & 7, data);
		break;

	default:
		m_scsi_write_callback(offset & 7, data);
		break;
	}
}
