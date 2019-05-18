// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    SGI HAL2 Audio Controller emulation

**********************************************************************/

#include "emu.h"
#include "machine/hal2.h"

#define LOG_UNKNOWN     (1 << 0U)
#define LOG_READS       (1 << 1U)
#define LOG_WRITES      (1 << 2U)
#define LOG_ALL         (LOG_UNKNOWN | LOG_READS | LOG_WRITES)

#define VERBOSE         (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(SGI_HAL2, hal2_device, "hal2", "SGI HAL2")

hal2_device::hal2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SGI_HAL2, tag, owner, clock)
	, m_iar(0)
	, m_idr{0, 0, 0, 0}
{
}

void hal2_device::device_start()
{
	save_item(NAME(m_isr));
	save_item(NAME(m_iar));
	save_item(NAME(m_idr));
}

void hal2_device::device_reset()
{
	m_isr = 0;
	m_iar = 0;
	memset(m_idr, 0, sizeof(uint32_t) * 4);
}

READ32_MEMBER(hal2_device::read)
{
	switch (offset)
	{
	case STATUS_REG:
		LOGMASKED(LOG_READS, "%s: HAL2 Status Read: %08x\n", machine().describe_context(), m_isr);
		return m_isr;
	case REVISION_REG:
		LOGMASKED(LOG_READS, "%s: HAL2 Revision Read: 0x4011\n", machine().describe_context());
		return 0x4011;
	}
	LOGMASKED(LOG_READS | LOG_UNKNOWN, "%s: Unknown HAL2 read: %08x & %08x\n", machine().describe_context(), 0x1fbd8000 + offset*4, mem_mask);
	return 0;
}

WRITE32_MEMBER(hal2_device::write)
{
	switch (offset)
	{
	case STATUS_REG:
		LOGMASKED(LOG_WRITES, "%s: HAL2 Status Write: 0x%08x (%08x)\n", machine().describe_context(), data, mem_mask);
		LOGMASKED(LOG_WRITES, "    HAL2 Global Reset %s\n", (data & ISR_GLOBAL_RESET) ? "Inactive" : "Active");
		LOGMASKED(LOG_WRITES, "    HAL2 Codec Reset %s\n", (data & ISR_CODEC_RESET) ? "Inactive" : "Active");
		m_isr &= ~0x1c;
		m_isr |= data & 0x1c;
		break;
	case INDIRECT_ADDRESS_REG:
		LOGMASKED(LOG_WRITES, "%s: HAL2 Indirect Address Register Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_iar = data;
		switch (data & IAR_TYPE)
		{
		case 0x1000:
			LOGMASKED(LOG_WRITES, "    DMA Port\n");
			switch (data & IAR_NUM)
			{
			case 0x0100:
				LOGMASKED(LOG_WRITES, "        Synth In\n");
				break;
			case 0x0200:
				LOGMASKED(LOG_WRITES, "        AES In\n");
				break;
			case 0x0300:
				LOGMASKED(LOG_WRITES, "        AES Out\n");
				break;
			case 0x0400:
				LOGMASKED(LOG_WRITES, "        DAC Out\n");
				break;
			case 0x0500:
				LOGMASKED(LOG_WRITES, "        ADC Out\n");
				break;
			case 0x0600:
				LOGMASKED(LOG_WRITES, "        Synth Control\n");
				break;
			}
			break;
		case 0x2000:
			LOGMASKED(LOG_WRITES, "    Bresenham\n");
			switch (data & IAR_NUM)
			{
			case 0x0100:
				LOGMASKED(LOG_WRITES, "        Bresenham Clock Gen 1\n");
				break;
			case 0x0200:
				LOGMASKED(LOG_WRITES, "        Bresenham Clock Gen 2\n");
				break;
			case 0x0300:
				LOGMASKED(LOG_WRITES, "        Bresenham Clock Gen 3\n");
				break;
			}
			break;

		case 0x3000:
			LOGMASKED(LOG_WRITES, "    Unix Timer\n");
			switch (data & IAR_NUM)
			{
			case 0x0100:
				LOGMASKED(LOG_WRITES, "        Unix Timer\n");
				break;
			}
			break;

		case 0x9000:
			LOGMASKED(LOG_WRITES, "    Global DMA Control\n");
			switch (data & IAR_NUM)
			{
			case 0x0100:
				LOGMASKED(LOG_WRITES, "        DMA Control\n");
				break;
			}
			break;
		}

		switch (data & IAR_ACCESS_SEL)
		{
		case 0x0000:
			LOGMASKED(LOG_WRITES, "    Write\n");
			break;
		case 0x0080:
			LOGMASKED(LOG_WRITES, "    Read\n");
			break;
		}
		LOGMASKED(LOG_WRITES, "    Parameter: %01x\n", (data & IAR_PARAM) >> 2);
		return;

	case INDIRECT_DATA0_REG:
		LOGMASKED(LOG_WRITES, "%s: HAL2 Indirect Data Register 0 Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_idr[0] = data;
		return;

	case INDIRECT_DATA1_REG:
		LOGMASKED(LOG_WRITES, "%s: HAL2 Indirect Data Register 1 Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_idr[1] = data;
		return;

	case INDIRECT_DATA2_REG:
		LOGMASKED(LOG_WRITES, "%s: HAL2 Indirect Data Register 2 Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_idr[2] = data;
		return;

	case INDIRECT_DATA3_REG:
		LOGMASKED(LOG_WRITES, "%s: HAL2 Indirect Data Register 3 Write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_idr[3] = data;
		return;

	default:
		LOGMASKED(LOG_WRITES, "%s: Unknown HAL2 Write: %08x = %08x & %08x\n", machine().describe_context(), 0x1fbd8000 + offset*4, data, mem_mask);
		break;
	}
}
