// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, Ash Wolf
/***************************************************************************

        Psion 5mx (EPOC R5) series ETNA peripheral

        Skeleton device by Ryan Holtz, ported from work by Ash Wolf

        More info:
            https://github.com/Treeki/WindEmu

****************************************************************************/

#include "etna.h"

#define LOG_READS       (1 << 0)
#define LOG_WRITES      (1 << 1)
#define LOG_UNKNOWNS    (1 << 2)
#define LOG_PROM        (1 << 3)
#define LOG_ALL         (LOG_READS | LOG_WRITES | LOG_UNKNOWNS | LOG_PROM)

#define VERBOSE         (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(ETNA, etna_device, "etna", "Psion 5mx ETNA")

etna_device::etna_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ETNA, tag, owner, clock)
	, m_eeprom_data_out(*this)
{
}

void etna_device::device_start()
{
	save_item(NAME(m_prom_addr_count));
	save_item(NAME(m_prom_addr));
	save_item(NAME(m_prom_value));
	save_item(NAME(m_prom_cs));
	save_item(NAME(m_prom_clk));

	save_item(NAME(m_pending_ints));

	save_item(NAME(m_regs));
	save_item(NAME(m_prom));
}

void etna_device::device_reset()
{
	m_prom_addr_count = 0;
	m_prom_addr = 0;
	m_prom_value = 0;
	m_prom_cs = false;
	m_prom_clk = false;

	m_pending_ints = 0;

	std::fill(std::begin(m_regs), std::end(m_regs), 0);
	std::fill(std::begin(m_prom), std::end(m_prom), 0);

	// defaults expected by the touchscreen code
	m_prom[0x0a] = 20;
	m_prom[0x0b] = 20;
	m_prom[0x0c] = 20;
	m_prom[0x0d] = 30;

	// some basic stuff to begin with
	// set up the Psion's unique ID
	m_prom[0x1b] = 0xde;
	m_prom[0x1a] = 0xad;
	m_prom[0x19] = 0xbe;
	m_prom[0x18] = 0xef;

	// give ourselves a neat custom device name
	const char *key = "PSION";
	const char *name = "MAME!";
	m_prom[0x28] = strlen(name);
	for (int i = 0; i < m_prom[0x28]; i++)
		m_prom[0x29 + i] = name[i] ^ key[i];

	// calculate the checksum
	uint8_t chk = 0;
	for (int i = 0; i < 0x7F; i++)
		chk ^= m_prom[i];

	// EPOC is expecting 66
	m_prom[0x7f] = chk ^ 66;
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
		case REG_WAKE1:
			LOGMASKED(LOG_WRITES, "%s: etna reg write: WAKE1 = %02x\n", machine().describe_context(), data);
			m_regs[REG_WAKE1] = data;
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
			LOGMASKED(LOG_UNKNOWNS, "%s: etna reg write: Unknown = %02x\n", machine().describe_context(), data);
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
			LOGMASKED(LOG_READS, "%s: etna reg read: INT_CLEAR = %02x\n", machine().describe_context(), data);
			break;
		case REG_SKT_VAR_A0:
			LOGMASKED(LOG_READS, "%s: etna reg read: SKT_VAR_A0 = %02x\n", machine().describe_context(), data);
			data = 1;
			break;
		case REG_SKT_VAR_A1:
			LOGMASKED(LOG_READS, "%s: etna reg read: SKT_VAR_A1 = %02x\n", machine().describe_context(), data);
			break;
		case REG_SKT_CTRL:
			LOGMASKED(LOG_READS, "%s: etna reg read: SKT_CTRL = %02x\n", machine().describe_context(), data);
			break;
		case REG_WAKE1:
			data = m_regs[REG_WAKE1];
			LOGMASKED(LOG_READS, "%s: etna reg read: WAKE1 = %02x\n", machine().describe_context(), data);
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
			LOGMASKED(LOG_UNKNOWNS, "%s: etna reg read: Unknown = %02x\n", machine().describe_context(), data);
			break;
	}
	return data;
}

WRITE_LINE_MEMBER(etna_device::eeprom_cs_in)
{
	bool old = m_prom_cs;
	m_prom_cs = state;
	if (!old && m_prom_cs)
	{
		m_prom_addr_count = 0;
		m_prom_addr = 0;
		m_prom_value = 0;
	}
}

WRITE_LINE_MEMBER(etna_device::eeprom_clk_in)
{
	bool old = m_prom_clk;
	m_prom_clk = state;

	if (!old && m_prom_clk)
	{
		if (m_prom_addr_count < 10)
		{
			m_prom_addr <<= 1;
			m_prom_addr |= BIT(m_regs[REG_WAKE1], 2);
			m_prom_addr_count++;
			if (m_prom_addr_count == 10)
			{
				LOGMASKED(LOG_PROM, "PROM Address: %03x\n", m_prom_addr);
				uint16_t byte_addr = (m_prom_addr << 1) % std::size(m_prom);
				m_prom_value = m_prom[byte_addr] | (m_prom[byte_addr + 1] << 8);
				LOGMASKED(LOG_PROM, "PROM Value: %04x\n", m_prom_value);
			}
		}
		else
		{
			m_regs[REG_WAKE1] &= ~0x08;
			m_regs[REG_WAKE1] |= BIT(m_prom_value, 15) << 3;
			LOGMASKED(LOG_PROM, "PROM Bit: %d\n", BIT(m_prom_value, 15));
			m_prom_value <<= 1;
		}
	}
}

WRITE_LINE_MEMBER(etna_device::eeprom_data_in)
{
}
