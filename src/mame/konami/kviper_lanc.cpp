// license:BSD-3-Clause
// copyright-holders:
/***************************************************************************

Konami Viper LAN Controller

TODO: nearly everything

***************************************************************************/

#include "emu.h"

#include "kviper_lanc.h"

#define LOG_READS   (1U << 1)
#define LOG_WRITES  (1U << 2)
#define LOG_RAM_READS   (1U << 3)
#define LOG_RAM_WRITES  (1U << 4)
#define LOG_ALL (LOG_READS | LOG_WRITES | LOG_RAM_READS | LOG_RAM_WRITES | LOG_UNKNOWNS)

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(KVIPER_LANC, kviper_lanc_device, "kviper_lanc", "Konami Viper LAN Controller")


kviper_lanc_device::kviper_lanc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, KVIPER_LANC, tag, owner, clock)
	, m_irq_cb(*this)
{
}

void kviper_lanc_device::device_start()
{
	save_item(NAME(m_irq_state));
	save_item(NAME(m_network_id));
	save_item(NAME(m_control));
	save_item(NAME(m_status));
	save_item(NAME(m_unk1));
	save_item(NAME(m_unk2));
}

void kviper_lanc_device::device_reset()
{
	m_irq_enable = false;
	m_irq_state = 0;
	m_network_id = 0;
	m_control = 0;
	m_status = 0;
	m_unk1 = 0;
	m_unk2 = 0;
}

void kviper_lanc_device::map(address_map &map)
{
	map(0x0, 0x0).w(FUNC(kviper_lanc_device::network_id_w));
	map(0x1, 0x1).w(FUNC(kviper_lanc_device::control_w));
	map(0x2, 0x2).r(FUNC(kviper_lanc_device::status_r));
	map(0x3, 0x3).rw(FUNC(kviper_lanc_device::unk1_r), FUNC(kviper_lanc_device::unk1_w));
	map(0x4, 0x4).rw(FUNC(kviper_lanc_device::unk2_r), FUNC(kviper_lanc_device::unk2_w));
	map(0x5, 0x5).w(FUNC(kviper_lanc_device::start_w));
}

void kviper_lanc_device::network_id_w(uint8_t data)
{
	m_network_id = data;
	LOGMASKED(LOG_WRITES, "%s: network_id_w: %02x\n", machine().describe_context(), m_network_id);
}

void kviper_lanc_device::control_w(uint8_t data)
{
	if (!BIT(data, 0))
	{
		m_status = 0;
		m_irq_state = 0;
	}
	else
	{
		if(m_irq_enable)
			m_irq_state = 1;
	}

	if (BIT(data, 3))
		m_status = 0x10;

	m_irq_cb(m_irq_state);

	m_control = data;
	LOGMASKED(LOG_WRITES, "%s: control_w: %02x\n", machine().describe_context(), m_control);
}

uint8_t kviper_lanc_device::status_r()
{
	return m_status;
	LOGMASKED(LOG_READS, "%s: status_r: %02x\n", machine().describe_context(), m_status);
}

uint8_t kviper_lanc_device::unk1_r()
{
	return m_unk1;
	LOGMASKED(LOG_READS, "%s: unk1_r: %02x\n", machine().describe_context(), m_unk1);
}

void kviper_lanc_device::unk1_w(uint8_t data)
{
	m_unk1 = data;
	LOGMASKED(LOG_WRITES, "%s: unk1_w: %02x\n", machine().describe_context(), m_unk1);
}

uint8_t kviper_lanc_device::unk2_r()
{
	return m_unk2;
	LOGMASKED(LOG_READS, "%s: unk2_r: %02x\n", machine().describe_context(), m_unk2);
}

void kviper_lanc_device::unk2_w(uint8_t data)
{
	m_unk2 = data;
	LOGMASKED(LOG_WRITES, "%s: unk2_w: %02x\n", machine().describe_context(), m_unk2);
}

void kviper_lanc_device::start_w(uint8_t data)
{
	m_irq_enable = bool(BIT(data, 0));
}

uint8_t kviper_lanc_device::ram_r(offs_t offset)
{
	uint8_t data = m_ram[offset];
	LOGMASKED(LOG_RAM_READS, "ram_r: %02x %02x\n", offset, data);
	return data;
}

void kviper_lanc_device::ram_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_RAM_WRITES, "ram_w: %02x %02x\n", offset, data);
	m_ram[offset] = data;
}