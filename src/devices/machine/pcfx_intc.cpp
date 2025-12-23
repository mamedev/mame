// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Angelo Salese
/**************************************************************************************************

PC-FX INTC (unknown part number)

**************************************************************************************************/

#include "emu.h"
#include "pcfx_intc.h"

DEFINE_DEVICE_TYPE(PCFX_INTC, pcfx_intc_device, "pcfx_intc", "NEC PC-FX INTC")

pcfx_intc_device::pcfx_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PCFX_INTC, tag, owner, clock)
	, m_int_w(*this)
{
}

void pcfx_intc_device::device_start()
{
	save_item(NAME(m_irq_mask));
	save_item(NAME(m_irq_pending));
	save_pointer(NAME(m_irq_priority), 8);
}

void pcfx_intc_device::device_reset()
{
	m_irq_mask = m_irq_pending = 0;
	std::fill_n(m_irq_priority, 8, 0);
}

u16 pcfx_intc_device::read(offs_t offset)
{
	u16 data = 0;

	switch( offset )
	{
		// Interrupts pending
		// Same bit order as mask
		case 0x00/4:
			data = m_irq_pending;
			break;

		// Interrupt mask
		case 0x40/4:
			data = m_irq_mask;
			break;

		// Interrupt priority 0
		case 0x80/4:
			data = m_irq_priority[4] | ( m_irq_priority[5] << 3 ) | ( m_irq_priority[6] << 6 ) | ( m_irq_priority[7] << 9 );
			break;

		// Interrupt priority 1
		case 0xC0/4:
			data = m_irq_priority[0] | ( m_irq_priority[1] << 3 ) | ( m_irq_priority[2] << 6 ) | ( m_irq_priority[3] << 9 );
			break;
	}

	return data;
}

void pcfx_intc_device::write(offs_t offset, uint16_t data)
{
	switch( offset )
	{
		// Interrupts pending
		case 0x00/4:
			logerror("irq_write: Attempt to write to irq pending register\n");
			break;

		// Interrupt mask
		// ---- ---- x--- ---- Mask interrupt level 8  (Unknown)
		// ---- ---- -x-- ---- Mask interrupt level 9  (Timer)
		// ---- ---- --x- ---- Mask interrupt level 10 (Unknown)
		// ---- ---- ---x ---- Mask interrupt level 11 (Pad)
		// ---- ---- ---- x--- Mask interrupt level 12 (HuC6270-A)
		// ---- ---- ---- -x-- Mask interrupt level 13 (HuC6272)
		// ---- ---- ---- --x- Mask interrupt level 14 (HuC6270-B)
		// ---- ---- ---- ---x Mask interrupt level 15 (HuC6273)
		// 0 - allow, 1 - ignore interrupt
		case 0x40/4:
			m_irq_mask = data;
			check_irqs();
			break;

		// Interrupt priority 0
		// ----xxx--------- Priority level interrupt 12
		// -------xxx------ Priority level interrupt 13
		// ----------xxx--- Priority level interrupt 14
		// -------------xxx Priority level interrupt 15
		case 0x80/4:
			m_irq_priority[7] = ( data >> 0 ) & 0x07;
			m_irq_priority[6] = ( data >> 3 ) & 0x07;
			m_irq_priority[5] = ( data >> 6 ) & 0x07;
			m_irq_priority[4] = ( data >> 9 ) & 0x07;
			check_irqs();
			break;

		// Interrupt priority 1
		// ----xxx--------- Priority level interrupt 8
		// -------xxx------ Priority level interrupt 9
		// ----------xxx--- Priority level interrupt 10
		// -------------xxx Priority level interrupt 11
		case 0xC0/4:
			m_irq_priority[3] = ( data >> 0 ) & 0x07;
			m_irq_priority[2] = ( data >> 3 ) & 0x07;
			m_irq_priority[1] = ( data >> 6 ) & 0x07;
			m_irq_priority[0] = ( data >> 9 ) & 0x07;
			check_irqs();
			break;
	}
}

inline void pcfx_intc_device::check_irqs()
{
	uint16_t active_irqs = m_irq_pending & ~m_irq_mask;
	int highest_prio = -1;

	for (auto & elem : m_irq_priority)
	{
		if ( active_irqs & 0x80 )
		{
			if ( elem >= highest_prio )
			{
				highest_prio = elem;
			}
		}
		active_irqs <<= 1;
	}

	if ( highest_prio >= 0 )
	{
		m_int_w(8 + highest_prio, ASSERT_LINE );
	}
	else
	{
		m_int_w(0, CLEAR_LINE );
	}
}

void pcfx_intc_device::set_irq_line(int line, int state)
{
	if ( state )
	{
//printf("Setting irq line %d\n", line);
		m_irq_pending |= ( 1 << ( 15 - line ) );
	}
	else
	{
//printf("Clearing irq line %d\n", line);
		m_irq_pending &= ~( 1 << ( 15 - line ) );
	}
	check_irqs();
}

