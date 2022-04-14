// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/*****************************************************************************

        SPG290 I2C

*****************************************************************************/

#include "emu.h"
#include "spg290_i2c.h"


DEFINE_DEVICE_TYPE(SPG290_I2C, spg290_i2c_device, "spg290_i2c", "SPG290 I2C")


spg290_i2c_device::spg290_i2c_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPG290_I2C, tag, owner, clock)
	, m_irq_cb(*this)
	, m_i2c_read_cb(*this)
	, m_i2c_write_cb(*this)
{
}

void spg290_i2c_device::device_start()
{
	m_irq_cb.resolve_safe();
	m_i2c_read_cb.resolve_safe(0);
	m_i2c_write_cb.resolve_safe();
	m_i2c_timer = timer_alloc();

	save_item(NAME(m_config));
	save_item(NAME(m_irq_control));
	save_item(NAME(m_clock_conf));
	save_item(NAME(m_id));
	save_item(NAME(m_port_addr));
	save_item(NAME(m_wdata));
	save_item(NAME(m_rdata));
}

void spg290_i2c_device::device_reset()
{
	m_config = 0;
	m_irq_control = 0;
	m_clock_conf = 0;
	m_id = 0;
	m_port_addr = 0;
	m_wdata = 0;
	m_rdata = 0;

	m_i2c_timer->adjust(attotime::never);

	m_irq_cb(CLEAR_LINE);
}

void spg290_i2c_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	if (m_config & 0x40)
		m_rdata = m_i2c_read_cb(m_port_addr);
	else
		m_i2c_write_cb(m_port_addr, m_wdata);

	// I2C ack
	m_config |= (m_config & 7) << 3;

	// I2C IRQ
	if (m_irq_control & 0x02)
	{
		m_irq_control |= 0x01;
		m_irq_cb(ASSERT_LINE);
	}
}

uint32_t spg290_i2c_device::read(offs_t offset, uint32_t mem_mask)
{
	switch (offset << 2)
	{
	case 0x20:      // I2C configuration
		return m_config;
	case 0x24:      // Interrupt status
		return m_irq_control;
	case 0x28:      // Clock setting
		return m_clock_conf;
	case 0x2c:      // ID
		return m_id;
	case 0x30:      // Port address
		return m_port_addr;
	case 0x34:      // Write data
		return m_wdata;
	case 0x38:      // Read data
		return m_rdata;
	default:
		logerror("[%s] %s: unknown read %x\n", tag(), machine().describe_context(), offset << 2);
	}

	return 0;
}

void spg290_i2c_device::write(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset << 2)
	{
	case 0x20:      // I2C configuration
	{
		COMBINE_DATA(&m_config);

		const auto i2c_clk = attotime::from_hz(clock() / 4 / ((m_clock_conf & 0x3ff) + 1));

		if (m_config & 0x0001)       // Start 8
			m_i2c_timer->adjust(i2c_clk * (2 + 9 * 4));
		else if (m_config & 0x0002)  // Start 16
			m_i2c_timer->adjust(i2c_clk * (2 + 9 * 5));
		else if (m_config & 0x0004)  // Start N
			m_i2c_timer->adjust(i2c_clk * (2 + 9 * 4), 0, i2c_clk * (2 + 9 * 4));
		else if (m_config & 0x0100)  // Stop N
			m_i2c_timer->adjust(attotime::never);

		break;
	}
	case 0x24:      // Interrupt status
		COMBINE_DATA(&m_irq_control);

		if (ACCESSING_BITS_0_7)
			m_irq_control &= ~(data & 0x01);  // IRQ ack

		if (!(m_irq_control & 0x01))
			m_irq_cb(CLEAR_LINE);

		break;
	case 0x28:      // Clock setting
		COMBINE_DATA(&m_clock_conf);
		break;
	case 0x2c:      // ID
		COMBINE_DATA(&m_id);
		break;
	case 0x30:      // Port address
		COMBINE_DATA(&m_port_addr);
		break;
	case 0x34:      // Write data
		COMBINE_DATA(&m_wdata);
		break;
	case 0x38:      // Read data
		COMBINE_DATA(&m_rdata);
		break;
	default:
		logerror("[%s] %s: unknown write %x = %x\n", tag(), machine().describe_context(), offset << 2, data);
	}
}
