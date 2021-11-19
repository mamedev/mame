// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Sharp Scoop peripheral chip emulation skeleton

***************************************************************************/

#include "emu.h"
#include "scoop.h"

#define LOG_UNKNOWN     (1 << 1)
#define LOG_READS       (1 << 2)
#define LOG_WRITES      (1 << 3)
#define LOG_ALL         (LOG_UNKNOWN | LOG_READS | LOG_WRITES)

#define VERBOSE         (LOG_ALL)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(SCOOP, scoop_device, "scoop", "Sharp SCOOP peripheral interface")

scoop_device::scoop_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SCOOP, tag, owner, clock)
	, m_gpio_out(*this)
{
}

void scoop_device::device_start()
{
	m_gpio_out.resolve_all_safe();

	save_item(NAME(m_gpwr));
	save_item(NAME(m_gpio_in_latch));
	save_item(NAME(m_gpcr));
}

void scoop_device::device_reset()
{
	m_gpwr = 0;
	m_gpio_in_latch = 0;
	m_gpcr = 0;
}

void scoop_device::gpio_in(const uint16_t line, const int state)
{
	m_gpio_in_latch &= ~(1 << line);
	m_gpio_in_latch |= (state << line);
}

void scoop_device::update_gpio_direction(const uint16_t old_dir)
{
	const uint16_t new_outputs = ~old_dir & m_gpcr;
	if (new_outputs)
	{
		for (uint32_t line = 0; line < 13; line++)
		{
			if (BIT(new_outputs, line))
			{
				m_gpio_out[line](BIT(m_gpwr, line));
			}
		}
	}
}

void scoop_device::update_gpio_outputs(const uint16_t old_latch, const uint16_t changed)
{
	uint16_t remaining_changed = changed;

	for (uint32_t line = 0; line < 13 && remaining_changed != 0; line++)
	{
		if (BIT(remaining_changed, line))
		{
			m_gpio_out[line](BIT(m_gpwr, line));
			remaining_changed &= ~(1 << line);
		}
	}
}

uint32_t scoop_device::read(offs_t offset)
{
	switch (offset)
	{
	case 0x00/4:
		LOGMASKED(LOG_READS, "%s: read: MCR: %04x\n", machine().describe_context(), 0);
		return 0;
	case 0x04/4:
		LOGMASKED(LOG_READS, "%s: read: CDR: %04x\n", machine().describe_context(), 0);
		return 0;
	case 0x08/4:
		LOGMASKED(LOG_READS, "%s: read: CSR: %04x\n", machine().describe_context(), 0);
		return 0;
	case 0x0c/4:
		LOGMASKED(LOG_READS, "%s: read: CPR: %04x\n", machine().describe_context(), 0);
		return 0;
	case 0x10/4:
		LOGMASKED(LOG_READS, "%s: read: CCR: %04x\n", machine().describe_context(), 0);
		return 0;
	case 0x14/4:
		LOGMASKED(LOG_READS, "%s: read: IRR: %04x\n", machine().describe_context(), 0);
		return 0;
	case 0x18/4:
		LOGMASKED(LOG_READS, "%s: read: IMR: %04x\n", machine().describe_context(), 0);
		return 0;
	case 0x1c/4:
		LOGMASKED(LOG_READS, "%s: read: ISR: %04x\n", machine().describe_context(), 0);
		return 0;
	case 0x20/4:
		LOGMASKED(LOG_READS, "%s: read: GPCR: %04x\n", machine().describe_context(), m_gpcr);
		return m_gpcr;
	case 0x24/4:
		LOGMASKED(LOG_READS, "%s: read: GPWR: %04x\n", machine().describe_context(), m_gpwr);
		return m_gpwr;
	case 0x28/4:
	{
		const uint16_t combined = (m_gpwr & m_gpcr) | (m_gpio_in_latch & ~m_gpcr);
		LOGMASKED(LOG_READS, "%s: read: GPRR: %04x\n", machine().describe_context(), combined);
		return combined;
	}
	default:
		LOGMASKED(LOG_READS | LOG_UNKNOWN, "%s: read: Unknown Register: %04x\n", machine().describe_context(), offset << 2);
		return 0;
	}
}

void scoop_device::write(offs_t offset, uint32_t data)
{
	switch (offset)
	{
	case 0x00/4:
		LOGMASKED(LOG_WRITES, "%s: write: MCR: %04x\n", machine().describe_context(), (uint16_t)data);
		break;
	case 0x04/4:
		LOGMASKED(LOG_WRITES, "%s: write: CDR: %04x\n", machine().describe_context(), (uint16_t)data);
		break;
	case 0x08/4:
		LOGMASKED(LOG_WRITES, "%s: write: CSR: %04x\n", machine().describe_context(), (uint16_t)data);
		break;
	case 0x0c/4:
		LOGMASKED(LOG_WRITES, "%s: write: CPR: %04x\n", machine().describe_context(), (uint16_t)data);
		break;
	case 0x10/4:
		LOGMASKED(LOG_WRITES, "%s: write: CCR: %04x\n", machine().describe_context(), (uint16_t)data);
		break;
	case 0x14/4:
		LOGMASKED(LOG_WRITES, "%s: write: IRR: %04x\n", machine().describe_context(), (uint16_t)data);
		break;
	case 0x18/4:
		LOGMASKED(LOG_WRITES, "%s: write: IMR: %04x\n", machine().describe_context(), (uint16_t)data);
		break;
	case 0x1c/4:
		LOGMASKED(LOG_WRITES, "%s: write: ISR: %04x\n", machine().describe_context(), (uint16_t)data);
		break;
	case 0x20/4:
	{
		LOGMASKED(LOG_WRITES, "%s: write: GPCR: %04x\n", machine().describe_context(), (uint16_t)data);
		const uint16_t old = m_gpcr;
		m_gpcr = data;
		if (old != m_gpcr)
			update_gpio_direction(old);
		break;
	}
	case 0x24/4:
	{
		LOGMASKED(LOG_WRITES, "%s: write: GPWR: %04x\n", machine().describe_context(), (uint16_t)data);
		const uint16_t old = m_gpwr;
		m_gpwr = data;
		if (old != m_gpwr)
			update_gpio_outputs(old, old ^ m_gpwr);
		break;
	}
	case 0x28/4:
		LOGMASKED(LOG_WRITES, "%s: write: GPRR (ignored): %04x\n", machine().describe_context(), (uint16_t)data);
		break;
	default:
		LOGMASKED(LOG_WRITES | LOG_UNKNOWN, "%s: write: Unknown Register: %04x = %04x\n", machine().describe_context(), offset << 2, (uint16_t)data);
		break;
	}
}
