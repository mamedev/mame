// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    CL-PS6700 - Low-Power PC Card Controller for the CL-PS7111

****************************************************************************/

#include "emu.h"
#include "clps6700.h"


#define LOG_REG     (1U << 1)
#define LOG_PCCARD  (1U << 2)
#define LOG_UNKN    (1U << 3)

#define VERBOSE     (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(CLPS6700, clps6700_device, "clps6700", "CL-PS6700 PC Card Controller")

clps6700_device::clps6700_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CLPS6700, tag, owner, clock)
	, m_pccard(*this, finder_base::DUMMY_TAG)
	, m_pirq_handler(*this)
{
}


void clps6700_device::map(address_map &map)
{
	map(0x0000000, 0x3ffffff).rw(m_pccard, FUNC(pccard_slot_device::read_reg), FUNC(pccard_slot_device::write_reg));
	map(0x8000000, 0xbffffff).rw(m_pccard, FUNC(pccard_slot_device::read_memory), FUNC(pccard_slot_device::write_memory));
	map(0xc000000, 0xfffffff).rw(FUNC(clps6700_device::register_r), FUNC(clps6700_device::register_w));
}


void clps6700_device::device_start()
{
	save_item(NAME(m_int_status));
	save_item(NAME(m_int_mask));
	save_item(NAME(m_int_output));
	save_item(NAME(m_int_input));
	save_item(NAME(m_sys_config));
	save_item(NAME(m_pwr_manager));
	save_item(NAME(m_pwr_control));
	save_item(NAME(m_intf_config));
	save_item(NAME(m_intf_timing_0a));
	save_item(NAME(m_intf_timing_0b));
	save_item(NAME(m_intf_timing_1a));
	save_item(NAME(m_intf_timing_1b));
	save_item(NAME(m_dma_control));
	save_item(NAME(m_device_info));
}

void clps6700_device::device_reset()
{
	m_int_status     = 0;
	m_int_mask       = 0;
	m_int_output     = 0;
	m_int_input      = 0;
	m_sys_config     = 0x1f8;
	m_pwr_manager    = 0;
	m_pwr_control    = 0;
	m_intf_config    = 0;
	m_intf_timing_0a = 0x1f00;
	m_intf_timing_0b = 0;
	m_intf_timing_1a = 0x1f00;
	m_intf_timing_1b = 0;
	m_dma_control    = 0;
	m_device_info    = 0x40;
}


void clps6700_device::set_input_level(int bit, int state)
{
	if (state)
		m_int_input |= 1 << bit;
	else
		m_int_input &= ~(1 << bit);
}


uint32_t clps6700_device::register_r(offs_t offset)
{
	uint32_t data = 0xffffffff;

	offset <<= 2;

	switch (offset)
	{
	case 0x0000:
		data = m_int_status;
		LOGMASKED(LOG_REG, "%s: read  PC Card Interrupt Status, %04x = %04x\n", machine().describe_context(), offset, data);
		break;
	case 0x0400:
		data = m_int_mask;
		LOGMASKED(LOG_REG, "%s: read  PC Card Interrupt Mask, %04x = %04x\n", machine().describe_context(), offset, data);
		break;
	case 0x0c00:
		data = m_int_output;
		LOGMASKED(LOG_REG, "%s: read  PC Card Interrupt Output Select, %04x = %04x\n", machine().describe_context(), offset, data);
		break;
	case 0x1c00:
		data = m_int_input;
		LOGMASKED(LOG_REG, "%s: read  PC Card Interrupt Input Level, %04x = %04x\n", machine().describe_context(), offset, data);
		break;
	case 0x2000:
		data = m_sys_config;
		LOGMASKED(LOG_REG, "%s: read  System Interface Configuration, %04x = %04x\n", machine().describe_context(), offset, data);
		break;
	case 0x2400:
		data = m_intf_config;
		LOGMASKED(LOG_REG, "%s: read  Card Interface Configuration, %04x = %04x\n", machine().describe_context(), offset, data);
		break;
	case 0x2800:
		data = m_pwr_manager;
		LOGMASKED(LOG_REG, "%s: read  Power Management, %04x = %04x\n", machine().describe_context(), offset, data);
		break;
	case 0x2c00:
		data = m_pwr_control;
		LOGMASKED(LOG_REG, "%s: read  Card Power Control, %04x = %04x\n", machine().describe_context(), offset, data);
		break;
	case 0x3000:
		data = m_intf_timing_0a;
		LOGMASKED(LOG_REG, "%s: read  Card Interface Timing 0A, %04x = %04x\n", machine().describe_context(), offset, data);
		break;
	case 0x3400:
		data = m_intf_timing_0b;
		LOGMASKED(LOG_REG, "%s: read  Card Interface Timing 0B, %04x = %04x\n", machine().describe_context(), offset, data);
		break;
	case 0x3800:
		data = m_intf_timing_1a;
		LOGMASKED(LOG_REG, "%s: read  Card Interface Timing 1A, %04x = %04x\n", machine().describe_context(), offset, data);
		break;
	case 0x3c00:
		data = m_intf_timing_1b;
		LOGMASKED(LOG_REG, "%s: read  Card Interface Timing 1B, %04x = %04x\n", machine().describe_context(), offset, data);
		break;
	case 0x4000:
		data = m_dma_control;
		LOGMASKED(LOG_REG, "%s: read  DMA Control, %04x = %04x\n", machine().describe_context(), offset, data);
		break;
	case 0x4400:
		data = m_device_info;
		LOGMASKED(LOG_REG, "%s: read  Device Information, %04x = %04x\n", machine().describe_context(), offset, data);
		break;
	default:
		LOGMASKED(LOG_UNKN, "%s: read  Unknown register, %04x = %04x\n", machine().describe_context(), offset, data);
		break;
	}

	return data;
}

void clps6700_device::register_w(offs_t offset, uint32_t data)
{
	offset <<= 2;

	switch (offset)
	{
	case 0x0400:
		LOGMASKED(LOG_REG, "%s: write PC Card Interrupt Mask, %04x = %04x\n", machine().describe_context(), offset, data);
		m_int_mask = data;
		break;
	case 0x0800:
		LOGMASKED(LOG_REG, "%s: write PC Card Interrupt Clear, %04x = %04x\n", machine().describe_context(), offset, data);
		m_int_status &= ~data;
		break;
	case 0x0c00:
		LOGMASKED(LOG_REG, "%s: write PC Card Interrupt Output Select, %04x = %04x\n", machine().describe_context(), offset, data);
		m_int_output = data;
		break;
	case 0x1000:
		LOGMASKED(LOG_REG, "%s: write PC Card Interrupt Reserved 1, %04x = %04x\n", machine().describe_context(), offset, data);
		break;
	case 0x1400:
		LOGMASKED(LOG_REG, "%s: write PC Card Interrupt Reserved 2, %04x = %04x\n", machine().describe_context(), offset, data);
		break;
	case 0x1800:
		LOGMASKED(LOG_REG, "%s: write PC Card Interrupt Reserved 3, %04x = %04x\n", machine().describe_context(), offset, data);
		break;
	case 0x2000:
		LOGMASKED(LOG_REG, "%s: write System Interface Configuration, %04x = %04x\n", machine().describe_context(), offset, data);
		m_sys_config = data;
		break;
	case 0x2400:
		LOGMASKED(LOG_REG, "%s: write Card Interface Configuration, %04x = %04x\n", machine().describe_context(), offset, data);
		m_intf_config = data;
		set_input_level(PCM_RDY, !BIT(data, 8));
		LOGMASKED(LOG_PCCARD, "PC card mode: %s\n", BIT(data, 8) ? "I/O" : "Memory");
		LOGMASKED(LOG_PCCARD, "PC card write protect: %s\n", BIT(data, 9) ? "Yes" : "No");
		LOGMASKED(LOG_PCCARD, "PC card enable: %s\n", BIT(data, 10) ? "Yes" : "No");
		if (BIT(data, 11) && BIT(data, 12))
			m_pccard->reset();
		break;
	case 0x2800:
		LOGMASKED(LOG_REG, "%s: write Power Management, %04x = %04x\n", machine().describe_context(), offset, data);
		m_pwr_manager = data;
		break;
	case 0x2c00:
		LOGMASKED(LOG_REG, "%s: write Card Power Control, %04x = %04x\n", machine().describe_context(), offset, data);
		m_pwr_control = data;
		break;
	case 0x3000:
		LOGMASKED(LOG_REG, "%s: write Card Interface Timing 0A, %04x = %04x\n", machine().describe_context(), offset, data);
		m_intf_timing_0a = data;
		break;
	case 0x3400:
		LOGMASKED(LOG_REG, "%s: write Card Interface Timing 0B, %04x = %04x\n", machine().describe_context(), offset, data);
		m_intf_timing_0b = data;
		break;
	case 0x3800:
		LOGMASKED(LOG_REG, "%s: write Card Interface Timing 1A, %04x = %04x\n", machine().describe_context(), offset, data);
		m_intf_timing_1a = data;
		break;
	case 0x3c00:
		LOGMASKED(LOG_REG, "%s: write Card Interface Timing 1B, %04x = %04x\n", machine().describe_context(), offset, data);
		m_intf_timing_1b = data;
		break;
	case 0x4000:
		LOGMASKED(LOG_REG, "%s: write DMA Control, %04x = %04x\n", machine().describe_context(), offset, data);
		m_dma_control = data;
		break;
	case 0x4400:
		LOGMASKED(LOG_REG, "%s: write Device Information, %04x = %04x\n", machine().describe_context(), offset, data);
		m_device_info = data;
		break;
	default:
		LOGMASKED(LOG_UNKN, "%s: write Unknown register, %04x = %04x\n", machine().describe_context(), offset, data);
		break;
	}
}
