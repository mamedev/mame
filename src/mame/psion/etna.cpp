// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, Ash Wolf
/***************************************************************************

    Psion ASIC12 (ETNA) peripheral

    ETNA is a peripheral ASIC incorporating a PCMCIA controller.

    More info:
        https://github.com/Treeki/WindEmu

****************************************************************************/

#include "emu.h"
#include "etna.h"


#define LOG_READS       (1U << 1)
#define LOG_WRITES      (1U << 2)
#define LOG_UNKNOWN     (1U << 3)
#define LOG_ALL         (LOG_READS | LOG_WRITES | LOG_UNKNOWN)

#define VERBOSE         (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(ETNA, etna_device, "etna", "Psion ASIC12 (ETNA)")

etna_device::etna_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ETNA, tag, owner, clock)
	, m_porta_r(*this, 0x00)
	, m_porta_w(*this)
{
}

void etna_device::device_start()
{
	save_item(NAME(m_pending_ints));
	save_item(NAME(m_regs));
}

void etna_device::device_reset()
{
	m_pending_ints = 0;

	std::fill(std::begin(m_regs), std::end(m_regs), 0);
}

void etna_device::regs_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case REG_UNK0:
			LOGMASKED(LOG_WRITES, "%s: etna reg write: UNK0 = %02x\n", machine().describe_context(), data);
			break;
		case REG_UNK1:
			LOGMASKED(LOG_WRITES, "%s: etna reg write: UNK1 = %02x\n", machine().describe_context(), data);
			break;
		case REG_UART_INT_STATUS:
			LOGMASKED(LOG_WRITES, "%s: etna reg write: INT_STATUS = %02x\n", machine().describe_context(), data);
			break;
		case REG_UART_INT_MASK:
			LOGMASKED(LOG_WRITES, "%s: etna reg write: INT_MASK = %02x\n", machine().describe_context(), data);
			break;
		case REG_UART_BAUD_LO:
			LOGMASKED(LOG_WRITES, "%s: etna reg write: BAUD_LO = %02x\n", machine().describe_context(), data);
			break;
		case REG_UART_BAUD_HI:
			LOGMASKED(LOG_WRITES, "%s: etna reg write: BAUD_HI = %02x\n", machine().describe_context(), data);
			break;
		case REG_PCCD_INT_STATUS:
			LOGMASKED(LOG_WRITES, "%s: etna reg write: PCCD_INT_STATUS = %02x\n", machine().describe_context(), data);
			break;
		case REG_PCCD_INT_MASK:
			LOGMASKED(LOG_WRITES, "%s: etna reg write: PCCD_INT_MASK = %02x\n", machine().describe_context(), data);
			break;
		case REG_INT_CLEAR:
			LOGMASKED(LOG_WRITES, "%s: etna reg write: INT_CLEAR = %02x\n", machine().describe_context(), data);
			m_pending_ints &= ~data;
			break;
		case REG_SKT_VAR_A0:
			LOGMASKED(LOG_WRITES, "%s: etna reg write: SKT_VAR_A0 = %02x\n", machine().describe_context(), data);
			break;
		case REG_SKT_VAR_A1:
			LOGMASKED(LOG_WRITES, "%s: etna reg write: SKT_VAR_A1 = %02x\n", machine().describe_context(), data);
			break;
		case REG_SKT_CTRL:
			LOGMASKED(LOG_WRITES, "%s: etna reg write: SKT_CTRL = %02x\n", machine().describe_context(), data);
			break;
		case REG_PORTA:
			LOGMASKED(LOG_WRITES, "%s: etna reg write: PORTA = %02x\n", machine().describe_context(), data);
			//m_regs[REG_PORTA] = data;
			m_porta_w(data);
			break;
		case REG_SKT_VAR_B0:
			LOGMASKED(LOG_WRITES, "%s: etna reg write: SKT_VAR_B0 = %02x\n", machine().describe_context(), data);
			break;
		case REG_SKT_VAR_B1:
			LOGMASKED(LOG_WRITES, "%s: etna reg write: SKT_VAR_B1 = %02x\n", machine().describe_context(), data);
			break;
		case REG_WAKE2:
			LOGMASKED(LOG_WRITES, "%s: etna reg write: WAKE2 = %02x\n", machine().describe_context(), data);
			m_regs[REG_WAKE2] = data;
			break;
		default:
			LOGMASKED(LOG_UNKNOWN, "%s: etna reg write: Unknown = %02x\n", machine().describe_context(), data);
			break;
	}
}

uint8_t etna_device::regs_r(offs_t offset)
{
	uint8_t data = 0;

	switch (offset)
	{
		case REG_UNK0:
			LOGMASKED(LOG_READS, "%s: etna reg read: UNK0 = %02x\n", machine().describe_context(), data);
			break;
		case REG_UNK1:
			LOGMASKED(LOG_READS, "%s: etna reg read: UNK1 = %02x\n", machine().describe_context(), data);
			break;
		case REG_UART_INT_STATUS:
			LOGMASKED(LOG_READS, "%s: etna reg read: INT_STATUS = %02x\n", machine().describe_context(), data);
			break;
		case REG_UART_INT_MASK:
			LOGMASKED(LOG_READS, "%s: etna reg read: INT_MASK = %02x\n", machine().describe_context(), data);
			break;
		case REG_UART_BAUD_LO:
			LOGMASKED(LOG_READS, "%s: etna reg read: BAUD_LO = %02x\n", machine().describe_context(), data);
			break;
		case REG_UART_BAUD_HI:
			LOGMASKED(LOG_READS, "%s: etna reg read: BAUD_HI = %02x\n", machine().describe_context(), data);
			break;
		case REG_PCCD_INT_STATUS:
			LOGMASKED(LOG_READS, "%s: etna reg read: PCCD_INT_STATUS = %02x\n", machine().describe_context(), data);
			break;
		case REG_PCCD_INT_MASK:
			LOGMASKED(LOG_READS, "%s: etna reg read: PCCD_INT_MASK = %02x\n", machine().describe_context(), data);
			break;
		case REG_INT_CLEAR:
			data = 0;
			LOGMASKED(LOG_READS, "%s: etna reg read: INT_CLEAR = %02x\n", machine().describe_context(), data);
			break;
		case REG_SKT_VAR_A0:
			data = 1;
			LOGMASKED(LOG_READS, "%s: etna reg read: SKT_VAR_A0 = %02x\n", machine().describe_context(), data);
			break;
		case REG_SKT_VAR_A1:
			data = 0;
			LOGMASKED(LOG_READS, "%s: etna reg read: SKT_VAR_A1 = %02x\n", machine().describe_context(), data);
			break;
		case REG_SKT_CTRL:
			LOGMASKED(LOG_READS, "%s: etna reg read: SKT_CTRL = %02x\n", machine().describe_context(), data);
			break;
		case REG_PORTA:
			//data = m_regs[REG_PORTA];
			data = m_porta_r();
			LOGMASKED(LOG_READS, "%s: etna reg read: PORTA = %02x\n", machine().describe_context(), data);
			break;
		case REG_SKT_VAR_B0:
			LOGMASKED(LOG_READS, "%s: etna reg read: SKT_VAR_B0 = %02x\n", machine().describe_context(), data);
			break;
		case REG_SKT_VAR_B1:
			LOGMASKED(LOG_READS, "%s: etna reg read: SKT_VAR_B1 = %02x\n", machine().describe_context(), data);
			break;
		case REG_WAKE2:
			data = m_regs[REG_WAKE2];
			LOGMASKED(LOG_READS, "%s: etna reg read: WAKE2 = %02x\n", machine().describe_context(), data);
			break;
		default:
			LOGMASKED(LOG_UNKNOWN, "%s: etna reg read: Unknown = %02x\n", machine().describe_context(), data);
			break;
	}
	return data;
}
