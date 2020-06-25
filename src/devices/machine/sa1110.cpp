// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**************************************************************************
 *
 * Intel XScale SA1110 peripheral emulation
 *
 **************************************************************************/

#include "emu.h"
#include "sa1110.h"

#define LOG_UNKNOWN     (1 << 1)
#define LOG_INTC        (1 << 2)
#define LOG_POWER		(1 << 3)
#define LOG_ALL         (LOG_UNKNOWN | LOG_INTC | LOG_POWER)

#define VERBOSE         (LOG_ALL)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(SA1110_PERIPHERALS, sa1110_periphs_device, "sa1110_periphs", "Intel XScale SA1110 Peripherals")

sa1110_periphs_device::sa1110_periphs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SA1110_PERIPHERALS, tag, owner, clock)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
{
}

/*

  Intel SA-1110 Interrupt Controller

  pg. 81 to 88 Intel StrongARM SA-1110 Microprocessor Developer's Manual

*/

void sa1110_periphs_device::update_interrupts()
{
	m_intc_regs.icfp = (m_intc_regs.icpr & m_intc_regs.icmr) & m_intc_regs.iclr;
	m_intc_regs.icip = (m_intc_regs.icpr & m_intc_regs.icmr) & (~m_intc_regs.iclr);
	m_maincpu->set_input_line(ARM7_FIRQ_LINE, m_intc_regs.icfp ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(ARM7_IRQ_LINE,  m_intc_regs.icip ? ASSERT_LINE : CLEAR_LINE);
}

void sa1110_periphs_device::set_irq_line(uint32_t line, int irq_state)
{
	m_intc_regs.icpr &= ~line;
	m_intc_regs.icpr |= irq_state ? line : 0;
	update_interrupts();
}

uint32_t sa1110_periphs_device::intc_r(offs_t offset, uint32_t mem_mask)
{
	switch (offset)
	{
		case REG_ICIP:
			LOGMASKED(LOG_INTC, "sa1110 intc_r: Interrupt Controller IRQ Pending Register: %08x & %08x\n", m_intc_regs.icip, mem_mask);
			return m_intc_regs.icip;
		case REG_ICMR:
			LOGMASKED(LOG_INTC, "sa1110 intc_r: Interrupt Controller Mask Register: %08x & %08x\n", m_intc_regs.icmr, mem_mask);
			return m_intc_regs.icmr;
		case REG_ICLR:
			LOGMASKED(LOG_INTC, "sa1110 intc_r: Interrupt Controller Level Register: %08x & %08x\n", m_intc_regs.iclr, mem_mask);
			return m_intc_regs.iclr;
		case REG_ICFP:
			LOGMASKED(LOG_INTC, "sa1110 intc_r: Interrupt Controller FIQ Pending Register: %08x & %08x\n", m_intc_regs.icfp, mem_mask);
			return m_intc_regs.icfp;
		case REG_ICPR:
			LOGMASKED(LOG_INTC, "sa1110 intc_r: Interrupt Controller Pending Register: %08x & %08x\n", m_intc_regs.icpr, mem_mask);
			return m_intc_regs.icpr;
		case REG_ICCR:
			LOGMASKED(LOG_INTC, "sa1110 intc_r: Interrupt Controller Control Register: %08x & %08x\n", m_intc_regs.iccr, mem_mask);
			return m_intc_regs.iccr;
		default:
			LOGMASKED(LOG_INTC | LOG_UNKNOWN, "sa1110 intc_r: Unknown address: %08x\n", INTC_BASE_ADDR | (offset << 2));
			break;
	}
	return 0;
}

void sa1110_periphs_device::intc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
		case REG_ICIP:
			LOGMASKED(LOG_INTC, "sa1110 intc_w: (Invalid Write) Interrupt Controller IRQ Pending Register: %08x & %08x\n", data, mem_mask);
			break;
		case REG_ICMR:
			LOGMASKED(LOG_INTC, "sa1110 intc_w: Interrupt Controller Mask Register: %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_intc_regs.icmr);
			break;
		case REG_ICLR:
			LOGMASKED(LOG_INTC, "sa1110 intc_w: Interrupt Controller Level Register: %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_intc_regs.iclr);
			break;
		case REG_ICFP:
			LOGMASKED(LOG_INTC, "sa1110 intc_w: (Invalid Write) Interrupt Controller FIQ Pending Register: %08x & %08x\n", data, mem_mask);
			break;
		case REG_ICPR:
			LOGMASKED(LOG_INTC, "sa1110_intc_w: (Invalid Write) Interrupt Controller Pending Register: %08x & %08x\n", data, mem_mask);
			break;
		case REG_ICCR:
			LOGMASKED(LOG_INTC, "sa1110 intc_w: Interrupt Controller Control Register: %08x & %08x\n", data, mem_mask);
			m_intc_regs.iccr = BIT(data, 0);
			break;
		default:
			LOGMASKED(LOG_INTC | LOG_UNKNOWN, "sa1110 intc_w: Unknown address: %08x = %08x & %08x\n", INTC_BASE_ADDR | (offset << 2), data, mem_mask);
			break;
	}
}

/*

  Intel SA-1110 Power Controller

  pg. 104 to 111 Intel StrongARM SA-1110 Microprocessor Developer's Manual

*/

uint32_t sa1110_periphs_device::power_r(offs_t offset, uint32_t mem_mask)
{
	switch (offset)
	{
		case REG_PMCR:
			LOGMASKED(LOG_POWER, "%s: power_r: Power Manager Control Register: %08x\n", machine().describe_context(), m_power_regs.pmcr);
			return m_power_regs.pmcr;
		case REG_PSSR:
			LOGMASKED(LOG_POWER, "%s: power_r: Power Manager Sleep Status Register: %08x\n", machine().describe_context(), m_power_regs.pssr);
			return m_power_regs.pssr;
		case REG_PSPR:
			LOGMASKED(LOG_POWER, "%s: power_r: Power Manager Scratch Pad Register: %08x\n", machine().describe_context(), m_power_regs.pspr);
			return m_power_regs.pspr;
		case REG_PWER:
			LOGMASKED(LOG_POWER, "%s: power_r: Power Manager Wake-up Enable Register: %08x\n", machine().describe_context(), m_power_regs.pwer);
			return m_power_regs.pwer;
		case REG_PCFR:
			LOGMASKED(LOG_POWER, "%s: power_r: Power Manager General Configuration Register: %08x\n", machine().describe_context(), m_power_regs.pcfr);
			return m_power_regs.pcfr;
		case REG_PPCR:
			LOGMASKED(LOG_POWER, "%s: power_r: Power Manager PLL Configuration Register: %08x\n", machine().describe_context(), m_power_regs.ppcr);
			return m_power_regs.ppcr;
		case REG_PGSR:
			LOGMASKED(LOG_POWER, "%s: power_r: Power Manager GPIO Sleep State Register: %08x\n", machine().describe_context(), m_power_regs.pgsr);
			return m_power_regs.pgsr;
		case REG_POSR:
			LOGMASKED(LOG_POWER, "%s: power_r: Power Manager Oscillator Status Register: %08x\n", machine().describe_context(), m_power_regs.posr);
			return m_power_regs.posr;
		default:
			LOGMASKED(LOG_POWER | LOG_UNKNOWN, "%s: power_r: Unknown address: %08x\n", machine().describe_context(), POWER_BASE_ADDR | (offset << 2));
			break;
	}
	return 0;
}

void sa1110_periphs_device::power_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
		case REG_PMCR:
			LOGMASKED(LOG_POWER, "%s: power_w: Power Manager Control Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			COMBINE_DATA(&m_power_regs.pmcr);
			break;
		case REG_PSSR:
			LOGMASKED(LOG_POWER, "%s: power_w: Power Manager Sleep Status Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			m_power_regs.pssr &= ~(data & 0x0000001f);
			break;
		case REG_PSPR:
			LOGMASKED(LOG_POWER, "%s: power_w: Power Manager Scratch Pad Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			COMBINE_DATA(&m_power_regs.pspr);
			break;
		case REG_PWER:
			LOGMASKED(LOG_POWER, "%s: power_w: Power Manager Wake-Up Enable Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			COMBINE_DATA(&m_power_regs.pwer);
			break;
		case REG_PCFR:
			LOGMASKED(LOG_POWER, "%s: power_w: Power Manager General Configuration Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			COMBINE_DATA(&m_power_regs.pcfr);
			break;
		case REG_PPCR:
			LOGMASKED(LOG_POWER, "%s: power_w: Power Manager PLL Configuration Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			COMBINE_DATA(&m_power_regs.ppcr);
			break;
		case REG_PGSR:
			LOGMASKED(LOG_POWER, "%s: power_w: Power Manager GPIO Sleep State Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			COMBINE_DATA(&m_power_regs.pgsr);
			break;
		case REG_POSR:
			LOGMASKED(LOG_POWER, "%s: power_w: Power Manager Oscillator Status Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		default:
			LOGMASKED(LOG_POWER | LOG_UNKNOWN, "%s: power_w: Unknown address: %08x = %08x & %08x\n", machine().describe_context(), POWER_BASE_ADDR | (offset << 2),
				data, mem_mask);
			break;
	}
}

void sa1110_periphs_device::device_start()
{
	save_item(NAME(m_intc_regs.icip), m_intc_regs.icip);
	save_item(NAME(m_intc_regs.icmr), m_intc_regs.icmr);
	save_item(NAME(m_intc_regs.iclr), m_intc_regs.iclr);
	save_item(NAME(m_intc_regs.iccr), m_intc_regs.iccr);
	save_item(NAME(m_intc_regs.icfp), m_intc_regs.icfp);
	save_item(NAME(m_intc_regs.icpr), m_intc_regs.icpr);

	save_item(NAME(m_power_regs.pmcr), m_power_regs.pmcr);
	save_item(NAME(m_power_regs.pssr), m_power_regs.pssr);
	save_item(NAME(m_power_regs.pspr), m_power_regs.pspr);
	save_item(NAME(m_power_regs.pwer), m_power_regs.pwer);
	save_item(NAME(m_power_regs.pcfr), m_power_regs.pcfr);
	save_item(NAME(m_power_regs.ppcr), m_power_regs.ppcr);
	save_item(NAME(m_power_regs.pgsr), m_power_regs.pgsr);
	save_item(NAME(m_power_regs.posr), m_power_regs.posr);
}

void sa1110_periphs_device::device_reset()
{
	memset(&m_intc_regs, 0, sizeof(m_intc_regs));
	memset(&m_power_regs, 0, sizeof(m_power_regs));
}

void sa1110_periphs_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
}

void sa1110_periphs_device::device_add_mconfig(machine_config &config)
{
}
