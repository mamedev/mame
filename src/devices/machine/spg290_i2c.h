// license:BSD-3-Clause
// copyright-holders:Sandro Ronco


#ifndef MAME_MACHINE_SPG290_I2C_H
#define MAME_MACHINE_SPG290_I2C_H

#pragma once

class spg290_i2c_device : public device_t
{
public:
	spg290_i2c_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t read(offs_t offset, uint32_t mem_mask);

	auto irq_cb() { return m_irq_cb.bind(); }
	auto i2c_read_cb() { return m_i2c_read_cb.bind(); }
	auto i2c_write_cb() { return m_i2c_write_cb.bind(); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(i2c_update);

private:
	devcb_write_line m_irq_cb;
	devcb_read16 m_i2c_read_cb;
	devcb_write16 m_i2c_write_cb;

	emu_timer *m_i2c_timer;

	uint32_t m_config;
	uint32_t m_irq_control;
	uint32_t m_clock_conf;
	uint32_t m_id;
	uint32_t m_port_addr;
	uint32_t m_wdata;
	uint32_t m_rdata;
};

DECLARE_DEVICE_TYPE(SPG290_I2C, spg290_i2c_device)

#endif // MAME_MACHINE_SPG290_I2C_H
