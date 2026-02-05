// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/**********************************************************************
    Spectrum Next DMA

    Spectrum Next DMA operates in two mode: z80dma compatible and
    N-mode with Next specific fixes.

    z80dma mode is implemented there based on intensive testing of
    the device. Potentially any mismatches not related to N-mode
    covered here must be moved to the z80dma parent.

**********************************************************************/

#include "emu.h"
#include "specnext_dma.h"

// device type definition
DEFINE_DEVICE_TYPE(SPECNEXT_DMA, specnext_dma_device, "specnext_dma", "Spectrum Next DMA")


// TODO: this stuff is copy/pasted from machine/z80dma.cpp - ideally it wouldn't need to be
#define GET_REGNUM(_r)          (&(_r) - &(WR0))
#define WR0                     REG(0, 0)
#define WR1                     REG(1, 0)
#define WR2                     REG(2, 0)
#define WR4                     REG(4, 0)

#define ZXN_PRESCALER           REG(2,2)

#define PORTA_INC               (WR1 & 0x10)
#define PORTB_INC               (WR2 & 0x10)
#define PORTA_FIXED             (((WR1 >> 4) & 0x02) == 0x02)
#define PORTB_FIXED             (((WR2 >> 4) & 0x02) == 0x02)

#define TRANSFER_MODE           (WR0 & 0x03)
#define OPERATING_MODE          ((WR4 >> 5) & 0x03) // 0b00: Byte; 0b01: Continuous; 0b10: Burst; 0b11: Do not program


specnext_dma_device::specnext_dma_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: z80dma_device(mconfig, SPECNEXT_DMA, tag, owner, clock)
{
}

void specnext_dma_device::write(u8 data)
{
	if (num_follow() == 0)
	{
		z80dma_device::write(data);
		if ((data & 0x83) == 0x83) // WR6
		{
			switch (data)
			{
			case COMMAND_ENABLE_DMA:
				m_byte_counter = 0;
				break;
			case COMMAND_RESET:
				m_r2_portB_preescaler_s = 0;
				break;
			default:
				break;
			}
		}
	}
	else
	{
		const u8 nreg = m_regs_follow[m_cur_follow];
		if (nreg == REGNUM(2, 1))
		{
			if (data & 0x20)
				m_regs_follow[m_num_follow++] = GET_REGNUM(ZXN_PRESCALER);
		}
		else if (nreg == REGNUM(2, 2))
		{
			m_r2_portB_preescaler_s = data;
		}
		z80dma_device::write(data);
	}
}

void specnext_dma_device::do_read()
{
	z80dma_device::do_read();
	// zxnDMA?
	if (m_dma_mode == 0 && (m_byte_counter + 1) == m_count)
		m_byte_counter++;
}

TIMER_CALLBACK_MEMBER(specnext_dma_device::clock_w)
{
	if (m_dma_delay)
	{
		if (m_dma_seq == SEQ_WAIT_READY)
			return;

		if (m_dma_seq == SEQ_TRANS1_WRITE_DEST)
		{
			z80dma_device::clock_w(param);
			if (m_dma_seq == SEQ_TRANS1_INC_DEC_SOURCE_ADDRESS)
			{
				set_busrq(CLEAR_LINE);
				m_dma_seq = SEQ_WAIT_READY;
				return;
			}
		}
	}

	if (m_dma_seq == SEQ_TRANS1_INC_DEC_SOURCE_ADDRESS)
	{
		m_dma_timer_0 = machine().time().as_ticks(unscaled_clock()) * 8;
	}

	const bool may_prescaled = m_dma_seq == SEQ_TRANS1_WRITE_DEST && m_r2_portB_preescaler_s;
	z80dma_device::clock_w(param);

	if (may_prescaled && m_dma_seq != SEQ_FINISH)
	{
		const u64 dma_timer_s = machine().time().as_ticks(unscaled_clock()) * 8 - m_dma_timer_0;
		if (m_r2_portB_preescaler_s > BIT(dma_timer_s, 5, 9))
		{
			if (OPERATING_MODE == 0b10) // Burst
				set_busrq(CLEAR_LINE);
			const u64 adj = (dma_timer_s & ~u16(0x3fff)) | (m_r2_portB_preescaler_s << 5);
			m_timer->adjust(attotime::from_ticks(adj - dma_timer_s, unscaled_clock() * 8), 0, clocks_to_attotime(1));
			return;
		}
	}
}

void specnext_dma_device::device_start()
{
	z80dma_device::device_start();

	save_item(NAME(m_dma_mode));
	save_item(NAME(m_dma_delay));
	save_item(NAME(m_r2_portB_preescaler_s));
	save_item(NAME(m_dma_timer_0));
}

void specnext_dma_device::device_reset()
{
	z80dma_device::device_reset();

	m_dma_mode = 0;
	m_dma_delay = 0;
	m_r2_portB_preescaler_s = 0;
	m_dma_timer_0 = 0;
}
