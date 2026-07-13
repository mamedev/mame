// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    h8500_intc.cpp

    H8/500 family interrupt controller.

    Every source except NMI has a 3-bit priority level programmed in the
    IPR registers.  A pending request is passed to the CPU when its level
    is strictly greater than the interrupt mask in SR bits I2-I0, so
    level-0 sources are never accepted.  NMI has fixed level 8 and always
    wins.  Requests that tie on level resolve in vector-number order,
    which matches the "priority among interrupts on the same level"
    column of table 5-2.

	TODO: DTC support

***************************************************************************/

#include "emu.h"
#include "h8500_intc.h"

#include "cpu/h8/h8_cpu_base.h"


DEFINE_DEVICE_TYPE(H8500_INTC, h8500_intc_device, "h8500_intc", "H8/500 interrupt controller")
DEFINE_DEVICE_TYPE(H8520_INTC, h8520_intc_device, "h8520_intc", "H8/520 interrupt controller")
DEFINE_DEVICE_TYPE(H8532_INTC, h8532_intc_device, "h8532_intc", "H8/532 interrupt controller")
DEFINE_DEVICE_TYPE(H8534_INTC, h8534_intc_device, "h8534_intc", "H8/534 interrupt controller")

// External IRQ vectors for each chip type
static const int h8510_irq_vectors[4] = { 32, 36, 37, 38 };
static const int h8520_irq_vectors[8] = { 32, 33, 34, 35, 36, 37, 38, 39 };
static const int h8532_irq_vectors[2] = { 32, 33 };
static const int h8534_irq_vectors[6] = { 32, 36, 40, 41, 44, 45 };

// Vector to IPR mapping table.  Even slots are bits 6-4 of an IPR,
// odd slots are bits 2-0, and -1 means no programmable priority.
static const int h8510_vector_to_slot[80] =
{
	-1, -1, -1, -1, -1, -1, -1, -1, // 0-7: reset, CPU exceptions
	-1, -1, -1, -1, -1, -1, -1, -1, // 8-15: NMI at 11 (fixed level 8)
	-1, -1, -1, -1, -1, -1, -1, -1, // 16-31: TRAPA
	-1, -1, -1, -1, -1, -1, -1, -1,
	 0,  0, -1, -1,  1,  1,  1, -1, // IRQ0, WDT, (reserved x2), IRQ1-IRQ3
	 2,  2,  2,  2,  3,  3,  3,  3, // FRT1 ICI/OCIA/OCIB/FOVI, FRT2 same
	 4,  4,  4, -1,  5,  5,  5, -1, // 8-bit timer CMIA/CMIB/OVI, SCI1 ERI/RXI/TXI
	 6,  6,  6, -1,  7, -1, -1, -1, // SCI2 ERI/RXI/TXI, A/D ADI
	-1, -1, -1, -1, -1, -1, -1, -1, // 64-79: unused
	-1, -1, -1, -1, -1, -1, -1, -1
};

static const int h8520_vector_to_slot[80] =
{
	-1, -1, -1, -1, -1, -1, -1, -1, // 0-7: reset, CPU exceptions
	-1, -1, -1, -1, -1, -1, -1, -1, // 8-15: NMI at 11 (fixed level 8)
	-1, -1, -1, -1, -1, -1, -1, -1, // 16-31: TRAPA
	-1, -1, -1, -1, -1, -1, -1, -1,
	 0,  1,  1,  1,  1,  1,  1,  1, // IRQ0 (+WDT), IRQ1-IRQ7 (one shared level)
	 2,  2,  2,  2,  3,  3,  3,  3, // FRT1 ICI/OCIA/OCIB/FOVI, FRT2 same
	 4,  4,  4, -1,  5,  5,  5, -1, // 8-bit timer CMIA/CMIB/OVI, SCI1 ERI/RXI/TXI
	 6,  6,  6, -1,  7, -1, -1, -1, // SCI2 ERI/RXI/TXI, A/D ADI
	-1, -1, -1, -1, -1, -1, -1, -1, // 64-79: unused
	-1, -1, -1, -1, -1, -1, -1, -1
};

static const int h8532_vector_to_slot[80] =
{
	-1, -1, -1, -1, -1, -1, -1, -1, // 0-7: reset, CPU exceptions
	-1, -1, -1, -1, -1, -1, -1, -1, // 8-15: NMI at 11 (fixed level 8)
	-1, -1, -1, -1, -1, -1, -1, -1, // 16-31: TRAPA
	-1, -1, -1, -1, -1, -1, -1, -1,
	 0,  1, -1, -1,  2,  2,  2,  2, // IRQ0 (+WDT), IRQ1, (reserved x2), FRT1 ICI/OCIA/OCIB/FOVI
	 3,  3,  3,  3,  4,  4,  4,  4, // FRT2 same, FRT3 same
	 5,  5,  5, -1,  6,  6,  6, -1, // 8-bit timer CMIA/CMIB/OVI, SCI ERI/RXI/TXI
	 7, -1, -1, -1, -1, -1, -1, -1, // A/D ADI
	-1, -1, -1, -1, -1, -1, -1, -1, // 64-79: unused
	-1, -1, -1, -1, -1, -1, -1, -1
};

static const int h8534_vector_to_slot[80] =
{
	-1, -1, -1, -1, -1, -1, -1, -1, // 0-7: reset, CPU exceptions
	-1, -1, -1, -1, -1, -1, -1, -1, // 8-15: NMI at 11 (fixed level 8)
	-1, -1, -1, -1, -1, -1, -1, -1, // 16-31: TRAPA
	-1, -1, -1, -1, -1, -1, -1, -1,
	 0,  0, -1, -1,  1, -1, -1, -1, // IRQ0, WDT interval, (reserved x2), IRQ1
	 2,  2, -1, -1,  3,  3, -1, -1, // IRQ2/IRQ3 (one shared level), IRQ4/IRQ5 (same)
	 4,  4,  4,  4,  5,  5,  5,  5, // FRT1 ICI/OCIA/OCIB/FOVI, FRT2 same
	 6,  6,  6,  6,  7,  7,  7, -1, // FRT3 same, 8-bit timer CMIA/CMIB/OVI
	 8,  8,  8, -1,  9,  9,  9, -1, // SCI1 ERI/RXI/TXI, SCI2 ERI/RXI/TXI
	10, -1, -1, -1, -1, -1, -1, -1  // A/D ADI
};


//**************************************************************************
//  base device (H8/510 is basically the base functionality for this family)
//**************************************************************************

h8500_intc_device::h8500_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: h8500_intc_device(mconfig, H8500_INTC, tag, owner, clock)
{
}

h8500_intc_device::h8500_intc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: h8_intc_base(mconfig, type, tag, owner, clock)
	, m_cpu(*this, finder_base::DUMMY_TAG)
	, m_irq_vectors(h8510_irq_vectors)
	, m_irq_pin_count(4)
	, m_vector_to_slot(h8510_vector_to_slot)
	, m_irqcr_bits(0x0f)
	, m_pending_irqs{0, 0, 0}
	, m_ipr_filter(7)
	, m_nmi_input(false)
	, m_irq_input(0)
	, m_irq_req(0)
	, m_ipr{0, 0, 0, 0, 0, 0}
	, m_dte{0, 0, 0, 0, 0, 0}
	, m_nmicr(0)
	, m_irqcr(0)
{
}

void h8500_intc_device::device_start()
{
	save_item(NAME(m_pending_irqs));
	save_item(NAME(m_ipr_filter));
	save_item(NAME(m_nmi_input));
	save_item(NAME(m_irq_input));
	save_item(NAME(m_irq_req));
	save_item(NAME(m_ipr));
	save_item(NAME(m_dte));
	save_item(NAME(m_nmicr));
	save_item(NAME(m_irqcr));
}

void h8500_intc_device::device_reset()
{
	std::fill(std::begin(m_pending_irqs), std::end(m_pending_irqs), 0);
	std::fill(std::begin(m_ipr), std::end(m_ipr), 0);
	std::fill(std::begin(m_dte), std::end(m_dte), 0);
	m_nmicr = 0x00;
	m_irqcr = 0x00;
	m_irq_req = 0;
	m_ipr_filter = 7;
}

void h8500_intc_device::internal_interrupt(int vector)
{
	m_pending_irqs[vector >> 5] |= 1U << (vector & 31);
	update_irq_state();
}

int h8500_intc_device::interrupt_taken(int vector)
{
	// DTC is not yet supported.  Warn if it's being triggered.
	const int dte_slot = (vector >= 0 && vector < MAX_VECTORS) ? m_vector_to_slot[vector] : -1;
	if (dte_slot >= 0 && (m_dte[dte_slot >> 1] & (BIT(dte_slot, 0) ? 0x0f : 0xf0)) != 0)
	{
		logerror("h8500_intc: vector %d taken when it should use DTC (DTE%c=%02X)\n",
				vector, 'A' + (dte_slot >> 1), m_dte[dte_slot >> 1]);
	}

	if (vector == NMI_VECTOR)
	{
		m_pending_irqs[vector >> 5] &= ~(1U << (vector & 31));
		update_irq_state();
		return INPUT_LINE_NMI;
	}

	for (int i = 0; i < m_irq_pin_count; i++)
	{
		if (vector == m_irq_vectors[i])
		{
			// IRQ0 is level triggered
			if (i != 0)
			{
				m_irq_req &= ~(1 << i);
			}
			if (!BIT(m_irq_req, i))
			{
				m_pending_irqs[vector >> 5] &= ~(1U << (vector & 31));
			}
			update_irq_state();
			return i;
		}
	}

	m_pending_irqs[vector >> 5] &= ~(1U << (vector & 31));
	update_irq_state();
	return 8;   // internal sources have no input line
}

// Maintain one external pin's request latch and its pending vector bit.
void h8500_intc_device::set_irq_request(int pin, bool state)
{
	if (state == bool(BIT(m_irq_req, pin)))
	{
		return;
	}

	const int vect = m_irq_vectors[pin];
	if (state)
	{
		m_irq_req |= 1 << pin;
		m_pending_irqs[vect >> 5] |= 1U << (vect & 31);
	}
	else
	{
		m_irq_req &= ~(1 << pin);
		m_pending_irqs[vect >> 5] &= ~(1U << (vect & 31));
	}
}

void h8500_intc_device::set_input(int inputnum, int state)
{
	if (inputnum == INPUT_LINE_NMI)
	{
		// NMICR.NMIEG selects the sensing edge: 0 = falling, 1 = rising
		bool trigger;
		if (BIT(m_nmicr, 0))
		{
			trigger = (state == CLEAR_LINE) && m_nmi_input;
		}
		else
		{
			trigger = (state == ASSERT_LINE) && !m_nmi_input;
		}
		m_nmi_input = (state == ASSERT_LINE);

		if (trigger && machine().time() > attotime::zero)
		{
			m_pending_irqs[NMI_VECTOR >> 5] |= 1U << (NMI_VECTOR & 31);
			update_irq_state();
		}
	}
	else if (inputnum >= 0 && inputnum < m_irq_pin_count)
	{
		const u8 mask = 1 << inputnum;
		const bool was_asserted = (m_irq_input & mask) != 0;
		if (state == ASSERT_LINE)
		{
			m_irq_input |= mask;
		}
		else
		{
			m_irq_input &= ~mask;
		}

		if (inputnum == 0)
		{
			// IRQ0 is level-triggered
			set_irq_request(0, (state == ASSERT_LINE) && BIT(m_irqcr, 0));
			update_irq_state();
		}
		else if ((state == ASSERT_LINE) && !was_asserted && BIT(m_irqcr, inputnum))
		{
			// the other pins latch on the falling edge and hold the
			// request until the exception-handling sequence begins
			set_irq_request(inputnum, true);
			update_irq_state();
		}
	}
}

void h8500_intc_device::set_filter(int ipr_filter)
{
	if (m_ipr_filter != ipr_filter)
	{
		m_ipr_filter = ipr_filter;
		update_irq_state();
	}
}

void h8500_intc_device::check_external_irqs()
{
	// Disabling a pin in IRQCR cancels its request; IRQ0 is level-triggered,
	// so its request tracks the pin state
	set_irq_request(0, BIT(m_irq_input, 0) && BIT(m_irqcr, 0));
	for (int i = 1; i < m_irq_pin_count; i++)
	{
		if (!BIT(m_irqcr, i))
		{
			set_irq_request(i, false);
		}
	}
}

void h8500_intc_device::update_irq_state()
{
	// Select the highest-priority pending request above the CPU's current
	// interrupt mask level.  Ties take the lowest numbered matching vector.
	int cur_vector = 0;
	int cur_level = -1;

	for (int i = 0; i < (MAX_VECTORS + 31) / 32; i++)
	{
		const u32 pending = m_pending_irqs[i];
		if (pending)
		{
			for (int j = 0; j < 32; j++)
			{
				if (BIT(pending, j))
				{
					const int vect = i * 32 + j;
					const int level = get_priority(vect);
					if (level > m_ipr_filter && level > cur_level)
					{
						cur_vector = vect;
						cur_level = level;
					}
				}
			}
		}
	}

	m_cpu->set_irq(cur_vector, cur_level, cur_vector == NMI_VECTOR);
}

int h8500_intc_device::get_priority(int vect) const
{
	if (vect == NMI_VECTOR)
	{
		return 8;
	}

	const int slot = (vect >= 0 && vect < MAX_VECTORS) ? m_vector_to_slot[vect] : -1;
	if (slot < 0)
	{
		return 0;
	}

	return (m_ipr[slot >> 1] >> (BIT(slot, 0) ? 0 : 4)) & 7;
}

u8 h8500_intc_device::ipr_r(int n) const
{
	return m_ipr[n];
}

void h8500_intc_device::ipr_w(int n, u8 data)
{
	m_ipr[n] = data & 0x77;
	update_irq_state();
}

u8 h8500_intc_device::nmicr_r()
{
	return 0xfe | m_nmicr;
}

void h8500_intc_device::nmicr_w(u8 data)
{
	m_nmicr = data & 0x01;
}

u8 h8500_intc_device::irqcr_r()
{
	return ~m_irqcr_bits | m_irqcr;
}

void h8500_intc_device::irqcr_w(u8 data)
{
	m_irqcr = data & m_irqcr_bits;

	check_external_irqs();
	update_irq_state();
}


//**************************************************************************
//  H8/520
//**************************************************************************

h8520_intc_device::h8520_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: h8500_intc_device(mconfig, H8520_INTC, tag, owner, clock)
{
	m_irq_vectors = h8520_irq_vectors;
	m_irq_pin_count = 8;
	m_vector_to_slot = h8520_vector_to_slot;
	m_irqcr_bits = 0xff;
}


//**************************************************************************
//  H8/532
//**************************************************************************

h8532_intc_device::h8532_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: h8500_intc_device(mconfig, H8532_INTC, tag, owner, clock)
	, m_brle(0)
{
	m_irq_vectors = h8532_irq_vectors;
	m_irq_pin_count = 2;
	m_vector_to_slot = h8532_vector_to_slot;
	m_irqcr_bits = 0x03;
}

void h8532_intc_device::device_start()
{
	h8500_intc_device::device_start();

	save_item(NAME(m_brle));
}

void h8532_intc_device::device_reset()
{
	h8500_intc_device::device_reset();

	m_brle = 0;
}

u8 h8532_intc_device::p1cr_r()
{
	return 0x87 | (m_irqcr << 5) | (m_nmicr << 4) | (m_brle << 3);
}

void h8532_intc_device::p1cr_w(u8 data)
{
	m_nmicr = BIT(data, 4);
	m_irqcr = (data >> 5) & 3;
	m_brle = BIT(data, 3);

	check_external_irqs();
	update_irq_state();
}


//**************************************************************************
//  H8/534
//**************************************************************************

h8534_intc_device::h8534_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: h8500_intc_device(mconfig, H8534_INTC, tag, owner, clock)
	, m_brle(0)
	, m_pin_ctl(0)
{
	m_irq_vectors = h8534_irq_vectors;
	m_irq_pin_count = 6;
	m_vector_to_slot = h8534_vector_to_slot;
	m_irqcr_bits = 0x3f;
}

void h8534_intc_device::device_start()
{
	h8500_intc_device::device_start();

	save_item(NAME(m_brle));
	save_item(NAME(m_pin_ctl));
}

void h8534_intc_device::device_reset()
{
	h8500_intc_device::device_reset();

	m_brle = 0;
	m_pin_ctl = 0;
}

u8 h8534_intc_device::syscr1_r()
{
	return 0x87 | ((m_irqcr & 0x03) << 5) | (m_nmicr << 4) | (m_brle << 3);
}

void h8534_intc_device::syscr1_w(u8 data)
{
	m_nmicr = BIT(data, 4);
	m_irqcr = (m_irqcr & 0x3c) | ((data >> 5) & 0x03);
	m_brle = BIT(data, 3);

	check_external_irqs();
	update_irq_state();
}

u8 h8534_intc_device::syscr2_r()
{
	return 0x80 | ((m_irqcr & 0x3c) << 1) | m_pin_ctl;
}

void h8534_intc_device::syscr2_w(u8 data)
{
	m_irqcr = (m_irqcr & 0x03) | ((data >> 1) & 0x3c);
	m_pin_ctl = data & 0x07;

	check_external_irqs();
	update_irq_state();
}
