// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

	Sharp LoCoMo peripheral chip emulation skeleton

***************************************************************************/

#include "emu.h"
#include "locomo.h"

#define LOG_UNKNOWN     (1 << 1)
#define LOG_READS		(1 << 2)
#define LOG_WRITES		(1 << 3)
#define LOG_ALL         (LOG_UNKNOWN | LOG_READS | LOG_WRITES)

#define VERBOSE         (LOG_ALL)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(LOCOMO, locomo_device, "locomo", "Sharp LoCoMo Peripheral")

locomo_device::locomo_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, LOCOMO, tag, owner, clock)
{
}

void locomo_device::device_start()
{
	save_item(NAME(m_kbd_cmd));
	save_item(NAME(m_kbd_row));
	save_item(NAME(m_kbd_col));
	save_item(NAME(m_kbd_level));
}

void locomo_device::device_reset()
{
	m_kbd_cmd = 0;
	m_kbd_row = 0;
	m_kbd_col = 0;
	m_kbd_level = 0;
}

uint32_t locomo_device::read(offs_t offset, uint32_t mem_mask)
{
	switch (offset)
	{
	case 0x00/4:
		LOGMASKED(LOG_READS, "%s: read: Version Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0x04/4:
		LOGMASKED(LOG_READS, "%s: read: Pin Status Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0x08/4:
		LOGMASKED(LOG_READS, "%s: read: C32K(?) Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0x0c/4:
		LOGMASKED(LOG_READS, "%s: read: Interrupt Control Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0x10/4:
		LOGMASKED(LOG_READS, "%s: read: Memory Chip Select 0 Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0x14/4:
		LOGMASKED(LOG_READS, "%s: read: Memory Chip Select 1 Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0x18/4:
		LOGMASKED(LOG_READS, "%s: read: Memory Chip Select 2 Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0x1c/4:
		LOGMASKED(LOG_READS, "%s: read: Memory Chip Select 3 Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0x20/4:
		LOGMASKED(LOG_READS, "%s: read: A/D Start Delay Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0x24/4:
		LOGMASKED(LOG_READS, "%s: read: HSYS Delay Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0x28/4:
		LOGMASKED(LOG_READS, "%s: read: HSYS Period Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0x30/4:
		LOGMASKED(LOG_READS, "%s: read: Tablet ADC Clock Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0x38/4:
		LOGMASKED(LOG_READS, "%s: read: TFT Backlight Control Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0x3c/4:
		LOGMASKED(LOG_READS, "%s: read: TFT CPS Delay Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0x40/4:
		LOGMASKED(LOG_READS, "%s: read: Keyboard Level Register: %08x & %08x\n", machine().describe_context(), m_kbd_level, mem_mask);
		return m_kbd_level;
	case 0x44/4:
		LOGMASKED(LOG_READS, "%s: read: Keyboard Strobe Control Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0x48/4:
		LOGMASKED(LOG_READS, "%s: read: Keyboard Strobe Command Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0x4c/4:
		LOGMASKED(LOG_READS, "%s: read: Keyboard Interrupt Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0x54/4:
		LOGMASKED(LOG_READS, "%s: read: Audio Clock Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0x60/4:
		LOGMASKED(LOG_READS, "%s: read: SPI Mode Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0x64/4:
		LOGMASKED(LOG_READS, "%s: read: SPI Control Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0x68/4:
		LOGMASKED(LOG_READS, "%s: read: SPI Status Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0x70/4:
		LOGMASKED(LOG_READS, "%s: read: SPI Interrupt Status Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0x74/4:
		LOGMASKED(LOG_READS, "%s: read: SPI Interrupt Status Write-Enable Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0x78/4:
		LOGMASKED(LOG_READS, "%s: read: SPI Interrupt Enable Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0x7c/4:
		LOGMASKED(LOG_READS, "%s: read: SPI Interrupt Request Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0x80/4:
		LOGMASKED(LOG_READS, "%s: read: SPI Transmit Data Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0x84/4:
		LOGMASKED(LOG_READS, "%s: read: SPI Receive Data Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0x88/4:
		LOGMASKED(LOG_READS, "%s: read: SPI Transmit Shift Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0x8c/4:
		LOGMASKED(LOG_READS, "%s: read: SPI Receive Shift Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0x90/4:
		LOGMASKED(LOG_READS, "%s: read: GPIO Direction Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0x94/4:
		LOGMASKED(LOG_READS, "%s: read: GPIO Input Enable Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0x98/4:
		LOGMASKED(LOG_READS, "%s: read: GPIO Level Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0x9c/4:
		LOGMASKED(LOG_READS, "%s: read: GPIO Output Latch Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0xa0/4:
		LOGMASKED(LOG_READS, "%s: read: GPIO Rising-Edge Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0xa4/4:
		LOGMASKED(LOG_READS, "%s: read: GPIO Falling-Edge Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0xa8/4:
		LOGMASKED(LOG_READS, "%s: read: GPIO Edge-Detect Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0xac/4:
		LOGMASKED(LOG_READS, "%s: read: GPIO Status Write-Enable Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0xb0/4:
		LOGMASKED(LOG_READS, "%s: read: GPIO Interrupt Enable Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0xb4/4:
		LOGMASKED(LOG_READS, "%s: read: GPIO Interrupt Status Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0xc8/4:
		LOGMASKED(LOG_READS, "%s: read: Front Light Cycle Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0xcc/4:
		LOGMASKED(LOG_READS, "%s: read: Front Light Duty Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0xd8/4:
		LOGMASKED(LOG_READS, "%s: read: Long-Time Clock Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0xdc/4:
		LOGMASKED(LOG_READS, "%s: read: Long-Time Clock Interrupt Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0xe0/4:
		LOGMASKED(LOG_READS, "%s: read: DAC Control Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0xe8/4:
		LOGMASKED(LOG_READS, "%s: read: LED 0 Control Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	case 0xec/4:
		LOGMASKED(LOG_READS, "%s: read: LED 1 Control Register: %08x & %08x\n", machine().describe_context(), 0, mem_mask);
		return 0;
	default:
		LOGMASKED(LOG_READS | LOG_UNKNOWN, "%s: read: Unknown Register: %08x & %08x\n", machine().describe_context(), offset << 2, mem_mask);
		return 0;
	}
}

void locomo_device::write(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
	case 0x00/4:
		LOGMASKED(LOG_WRITES, "%s: write: Version Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x04/4:
		LOGMASKED(LOG_WRITES, "%s: write: Pin Status Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x08/4:
		LOGMASKED(LOG_WRITES, "%s: write: C32K(?) Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x0c/4:
		LOGMASKED(LOG_WRITES, "%s: write: Interrupt Control Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x10/4:
		LOGMASKED(LOG_WRITES, "%s: write: Memory Chip Select 0 Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x14/4:
		LOGMASKED(LOG_WRITES, "%s: write: Memory Chip Select 1 Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x18/4:
		LOGMASKED(LOG_WRITES, "%s: write: Memory Chip Select 2 Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x1c/4:
		LOGMASKED(LOG_WRITES, "%s: write: Memory Chip Select 3 Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x20/4:
		LOGMASKED(LOG_WRITES, "%s: write: A/D Start Delay Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x24/4:
		LOGMASKED(LOG_WRITES, "%s: write: HSYS Delay Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x28/4:
		LOGMASKED(LOG_WRITES, "%s: write: HSYS Period Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x30/4:
		LOGMASKED(LOG_WRITES, "%s: write: Tablet ADC Clock Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x38/4:
		LOGMASKED(LOG_WRITES, "%s: write: TFT Backlight Control Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x3c/4:
		LOGMASKED(LOG_WRITES, "%s: write: TFT CPS Delay Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x40/4:
		LOGMASKED(LOG_WRITES, "%s: write: Keyboard Level Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x44/4:
		LOGMASKED(LOG_WRITES, "%s: write: Keyboard Strobe Control Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		if (m_kbd_cmd == 1)
		{
			if (m_kbd_row == 0)
			{
				m_kbd_row = (uint16_t)data;
			}
			else if (m_kbd_col == 0)
			{
				m_kbd_col = (uint16_t)data;
				if (m_kbd_col == 0xffff)
				{
					m_kbd_cmd = 0;
					m_kbd_level = 0;
				}
				else
				{
					// HACK: Something about this value causes the Zaurus updater to begin booting.
					// TODO: Work out the proper keyboard matrix.
					m_kbd_level = 0xffff;
				}
				m_kbd_row = 0;
				m_kbd_col = 0;
			}
		}
		break;
	case 0x48/4:
	{
		LOGMASKED(LOG_WRITES, "%s: write: Keyboard Strobe Command Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		const uint16_t old = m_kbd_cmd;
		m_kbd_cmd = (uint16_t)data;
		if (old == 0)
		{
			m_kbd_row = 0;
			m_kbd_col = 0;
		}
		break;
	}
	case 0x4c/4:
		LOGMASKED(LOG_WRITES, "%s: write: Keyboard Interrupt Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x54/4:
		LOGMASKED(LOG_WRITES, "%s: write: Audio Clock Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x60/4:
		LOGMASKED(LOG_WRITES, "%s: write: SPI Mode Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x64/4:
		LOGMASKED(LOG_WRITES, "%s: write: SPI Control Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x68/4:
		LOGMASKED(LOG_WRITES, "%s: write: SPI Status Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x70/4:
		LOGMASKED(LOG_WRITES, "%s: write: SPI Interrupt Status Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x74/4:
		LOGMASKED(LOG_WRITES, "%s: write: SPI Interrupt Status Write-Enable Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x78/4:
		LOGMASKED(LOG_WRITES, "%s: write: SPI Interrupt Enable Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x7c/4:
		LOGMASKED(LOG_WRITES, "%s: write: SPI Interrupt Request Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x80/4:
		LOGMASKED(LOG_WRITES, "%s: write: SPI Transmit Data Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x84/4:
		LOGMASKED(LOG_WRITES, "%s: write: SPI Receive Data Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x88/4:
		LOGMASKED(LOG_WRITES, "%s: write: SPI Transmit Shift Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x8c/4:
		LOGMASKED(LOG_WRITES, "%s: write: SPI Receive Shift Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x90/4:
		LOGMASKED(LOG_WRITES, "%s: write: GPIO Direction Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x94/4:
		LOGMASKED(LOG_WRITES, "%s: write: GPIO Input Enable Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x98/4:
		LOGMASKED(LOG_WRITES, "%s: write: GPIO Level Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x9c/4:
		LOGMASKED(LOG_WRITES, "%s: write: GPIO Output Latch Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0xa0/4:
		LOGMASKED(LOG_WRITES, "%s: write: GPIO Rising-Edge Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0xa4/4:
		LOGMASKED(LOG_WRITES, "%s: write: GPIO Falling-Edge Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0xa8/4:
		LOGMASKED(LOG_WRITES, "%s: write: GPIO Edge-Detect Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0xac/4:
		LOGMASKED(LOG_WRITES, "%s: write: GPIO Status Write-Enable Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0xb0/4:
		LOGMASKED(LOG_WRITES, "%s: write: GPIO Interrupt Enable Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0xb4/4:
		LOGMASKED(LOG_WRITES, "%s: write: GPIO Interrupt Status Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0xc8/4:
		LOGMASKED(LOG_WRITES, "%s: write: Front Light Cycle Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0xcc/4:
		LOGMASKED(LOG_WRITES, "%s: write: Front Light Duty Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0xd8/4:
		LOGMASKED(LOG_WRITES, "%s: write: Long-Time Clock Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0xdc/4:
		LOGMASKED(LOG_WRITES, "%s: write: Long-Time Clock Interrupt Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0xe0/4:
		LOGMASKED(LOG_WRITES, "%s: write: DAC Control Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0xe8/4:
		LOGMASKED(LOG_WRITES, "%s: write: LED 0 Control Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0xec/4:
		LOGMASKED(LOG_WRITES, "%s: write: LED 1 Control Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	default:
		LOGMASKED(LOG_WRITES | LOG_UNKNOWN, "%s: write: Unknown Register: %08x = %08x & %08x\n", machine().describe_context(), offset << 2, data, mem_mask);
		break;
	}
}
