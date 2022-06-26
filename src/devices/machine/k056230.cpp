// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

Konami IC 056230 (LANC)

Device Notes:
-The custom IC itself
-64k shared ram
-LS161 4-bit binary counter
-PAL(056787) for racinfrc's sub board and plygonet.cpp
-PAL(056787A) for zr107.cpp, gticlub.cpp and thunderh's I/O board
-HYC2485S RS485 transceiver

TODO: nearly everything

***************************************************************************/

#include "emu.h"

#include "k056230.h"

#define LOG_REG_READS	(1 << 1U)
#define LOG_REG_WRITES	(1 << 2U)
#define LOG_RAM_READS	(1 << 3U)
#define LOG_RAM_WRITES	(1 << 4U)
#define LOG_UNKNOWNS	(1 << 5U)
#define LOG_ALL (LOG_REG_READS | LOG_REG_WRITES | LOG_RAM_READS | LOG_RAM_WRITES | LOG_UNKNOWNS)

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(K056230, k056230_device, "k056230", "K056230 LANC")


k056230_device::k056230_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, K056230, tag, owner, clock)
	, m_ram(*this, "lanc_ram", 0x800U * 4, ENDIANNESS_BIG)
	, m_irq_cb(*this)
{
}

void k056230_device::device_start()
{
	m_irq_cb.resolve_safe();
	m_irq_state = CLEAR_LINE;

	save_item(NAME(m_irq_state));
}

u8 k056230_device::regs_r(offs_t offset)
{
	u8 data = 0;

	switch (offset)
	{
		case 0:     // Status register
			data = 0x08;
			LOGMASKED(LOG_REG_READS, "%s: regs_r: Status Register: %02x\n", machine().describe_context(), data);
			break;

		case 1:     // CRC Error register
			data = 0x00;
			LOGMASKED(LOG_REG_READS, "%s: regs_r: CRC Error Register: %02x\n", machine().describe_context(), data);
			break;

		default:
			LOGMASKED(LOG_REG_READS, "%s: regs_r: Unknown Register [%02x]: %02x\n", machine().describe_context(), offset, data);
			break;
	}

	return data;
}

void k056230_device::regs_w(offs_t offset, u8 data)
{
	switch (offset)
	{
		case 0:     // Mode register
			LOGMASKED(LOG_REG_WRITES, "%s: regs_w: Mode Register = %02x\n", machine().describe_context(), data);
			break;

		case 1:     // Control register
		{
			LOGMASKED(LOG_REG_WRITES, "%s: regs_w: Control Register = %02x\n", machine().describe_context(), data);
			// TODO: This is a literal translation of the previous device behaviour, and seems pretty likely to be incorrect.
			const int old_state = m_irq_state;
			if (BIT(data, 5))
			{
				LOGMASKED(LOG_REG_WRITES, "%s: regs_w: Asserting IRQ\n", machine().describe_context());
				m_irq_state = ASSERT_LINE;
			}
			if (!BIT(data, 0))
			{
				LOGMASKED(LOG_REG_WRITES, "%s: regs_w: Clearing IRQ\n", machine().describe_context());
				m_irq_state = CLEAR_LINE;
			}
			if (old_state != m_irq_state)
			{
				m_irq_cb(m_irq_state);
			}
		}	break;

		case 2:     // Sub ID register
			LOGMASKED(LOG_REG_WRITES, "%s: regs_w: Sub ID Register = %02x\n", machine().describe_context(), data);
			break;

		default:
			LOGMASKED(LOG_REG_WRITES | LOG_UNKNOWNS, "%s: regs_w: Unknown Register [%02x] = %02x\n", machine().describe_context(), offset, data);
			break;
	}
}

u32 k056230_device::ram_r(offs_t offset, u32 mem_mask)
{
	const auto lanc_ram = util::big_endian_cast<const u32>(m_ram.target());
	u32 data = lanc_ram[offset & 0x7ff];
	LOGMASKED(LOG_RAM_READS, "%s: Network RAM read [%04x (%03x)]: %08x & %08x\n", machine().describe_context(), offset << 2, (offset & 0x7ff) << 2, data, mem_mask);
	return data;
}

void k056230_device::ram_w(offs_t offset, u32 data, u32 mem_mask)
{
	const auto lanc_ram = util::big_endian_cast<u32>(m_ram.target());
	LOGMASKED(LOG_RAM_WRITES, "%s: Network RAM write [%04x (%03x)] = %08x & %08x\n", machine().describe_context(), offset << 2, (offset & 0x7ff) << 2, data, mem_mask);
	COMBINE_DATA(&lanc_ram[offset & 0x7ff]);
}
