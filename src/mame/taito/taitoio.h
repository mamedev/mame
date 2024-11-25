// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    taitoio.h

    Implementation of various Taito custom I/O ICs

**************************************************************************/

#ifndef MAME_TAITO_TAITOIO_H
#define MAME_TAITO_TAITOIO_H

#pragma once

#include "machine/watchdog.h"


class tc0040ioc_device : public device_t
{
public:
	tc0040ioc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto read_0_callback() { return m_read_0_cb.bind(); }
	auto read_1_callback() { return m_read_1_cb.bind(); }
	auto read_2_callback() { return m_read_2_cb.bind(); }
	auto read_3_callback() { return m_read_3_cb.bind(); }
	auto write_4_callback() { return m_write_4_cb.bind(); }
	auto read_7_callback() { return m_read_7_cb.bind(); }

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);
	u8 watchdog_r();
	u8 port_r();
	void port_w(u8 data);
	u8 portreg_r();
	void portreg_w(u8 data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	// internal state
	u8      m_regs[8];
	u8      m_port;

	required_device<watchdog_timer_device> m_watchdog;

	devcb_read8 m_read_0_cb;
	devcb_read8 m_read_1_cb;
	devcb_read8 m_read_2_cb;
	devcb_read8 m_read_3_cb;
	devcb_write8 m_write_4_cb;
	devcb_read8 m_read_7_cb;
};

DECLARE_DEVICE_TYPE(TC0040IOC, tc0040ioc_device)

class tc0220ioc_device : public device_t
{
public:
	tc0220ioc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto read_0_callback() { return m_read_0_cb.bind(); }
	auto read_1_callback() { return m_read_1_cb.bind(); }
	auto read_2_callback() { return m_read_2_cb.bind(); }
	auto read_3_callback() { return m_read_3_cb.bind(); }
	auto write_3_callback() { return m_write_3_cb.bind(); }
	auto write_4_callback() { return m_write_4_cb.bind(); }
	auto read_7_callback() { return m_read_7_cb.bind(); }

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	// internal state
	u8      m_regs[8];

	required_device<watchdog_timer_device> m_watchdog;

	devcb_read8 m_read_0_cb;
	devcb_read8 m_read_1_cb;
	devcb_read8 m_read_2_cb;
	devcb_read8 m_read_3_cb;
	devcb_write8 m_write_3_cb;
	devcb_write8 m_write_4_cb;
	devcb_read8 m_read_7_cb;
};

DECLARE_DEVICE_TYPE(TC0220IOC, tc0220ioc_device)

class tc0510nio_device : public device_t
{
public:
	tc0510nio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto read_0_callback() { return m_read_0_cb.bind(); }
	auto read_1_callback() { return m_read_1_cb.bind(); }
	auto read_2_callback() { return m_read_2_cb.bind(); }
	auto read_3_callback() { return m_read_3_cb.bind(); }
	auto write_3_callback() { return m_write_3_cb.bind(); }
	auto write_4_callback() { return m_write_4_cb.bind(); }
	auto read_7_callback() { return m_read_7_cb.bind(); }

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);
	u16 halfword_r(offs_t offset);
	void halfword_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 halfword_wordswap_r(offs_t offset);
	void halfword_wordswap_w(offs_t offset, u16 data, u16 mem_mask = ~0);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	// internal state
	u8   m_regs[8];

	required_device<watchdog_timer_device> m_watchdog;

	devcb_read8 m_read_0_cb;
	devcb_read8 m_read_1_cb;
	devcb_read8 m_read_2_cb;
	devcb_read8 m_read_3_cb;
	devcb_write8 m_write_3_cb;
	devcb_write8 m_write_4_cb;
	devcb_read8 m_read_7_cb;
};

DECLARE_DEVICE_TYPE(TC0510NIO, tc0510nio_device)

class tc0640fio_device : public device_t
{
public:
	tc0640fio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto read_0_callback() { return m_read_0_cb.bind(); }
	auto read_1_callback() { return m_read_1_cb.bind(); }
	auto read_2_callback() { return m_read_2_cb.bind(); }
	auto read_3_callback() { return m_read_3_cb.bind(); }
	auto write_4_callback() { return m_write_4_cb.bind(); }
	auto read_7_callback() { return m_read_7_cb.bind(); }

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);
	u16 halfword_r(offs_t offset);
	void halfword_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 halfword_byteswap_r(offs_t offset);
	void halfword_byteswap_w(offs_t offset, u16 data, u16 mem_mask = ~0);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	private:
	// internal state
	u8   m_regs[8];

	required_device<watchdog_timer_device> m_watchdog;

	devcb_read8 m_read_0_cb;
	devcb_read8 m_read_1_cb;
	devcb_read8 m_read_2_cb;
	devcb_read8 m_read_3_cb;
	devcb_write8 m_write_4_cb;
	devcb_read8 m_read_7_cb;
};

DECLARE_DEVICE_TYPE(TC0640FIO, tc0640fio_device)

#endif // MAME_TAITO_TAITOIO_H
