// license:BSD-3-Clause
// copyright-holders:windyfairy
#ifndef MAME_NAMCO_NAMCOS10_EXIO_H
#define MAME_NAMCO_NAMCOS10_EXIO_H

#pragma once

#include "cpu/h8/h83006.h"
#include "cpu/tlcs900/tmp95c061.h"
#include "machine/nvram.h"

class namcos10_exio_base_device : public device_t
{
public:
	namcos10_exio_base_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t ident_code() { return m_ident_code; }

	virtual uint16_t ctrl_r()  { return 0; }
	virtual void ctrl_w(uint16_t data)  {}

	virtual uint16_t bus_req_r()  { return 0; }
	virtual void bus_req_w(uint16_t data)  {}

	virtual uint16_t cpu_status_r() { return 0; }

	virtual uint16_t ram_r(offs_t offset) { return 0xff; }
	virtual void ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) {}

protected:
	namcos10_exio_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t ident_code);

	virtual void device_start() override ATTR_COLD;

	const uint8_t m_ident_code;
};

class namcos10_exio_device : public namcos10_exio_base_device
{
public:
	namcos10_exio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto analog_callback() { return m_analog_cb.bind(); }

	virtual uint16_t ctrl_r() override;
	virtual void ctrl_w(uint16_t data) override;

	virtual uint16_t bus_req_r() override;
	virtual void bus_req_w(uint16_t data) override;

	virtual uint16_t cpu_status_r() override;

	virtual uint16_t ram_r(offs_t offset) override;
	virtual void ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset_after_children() override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void map(address_map &map) ATTR_COLD;

	template <int Port> uint8_t port_read(offs_t offset);
	template <int Port> void port_write(offs_t offset, uint8_t data);

	required_device<tmp95c061_device> m_maincpu;
	required_shared_ptr<uint16_t> m_ram;

	devcb_read16 m_analog_cb;

	bool m_is_active;
	uint32_t m_analog_idx;

	uint16_t m_bus_req;
	uint16_t m_ctrl;
};

class namcos10_mgexio_device : public namcos10_exio_base_device
{
public:
	namcos10_mgexio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto port4_read_callback() { return m_port_read[0].bind(); }
	auto port6_read_callback() { return m_port_read[1].bind(); }
	auto port7_read_callback() { return m_port_read[2].bind(); }
	auto port8_read_callback() { return m_port_read[3].bind(); }
	auto port9_read_callback() { return m_port_read[4].bind(); }
	auto porta_read_callback() { return m_port_read[5].bind(); }
	auto portb_read_callback() { return m_port_read[6].bind(); }

	auto port4_write_callback() { return m_port_write[0].bind(); }
	auto port6_write_callback() { return m_port_write[1].bind(); }
	auto port7_write_callback() { return m_port_write[2].bind(); }
	auto port8_write_callback() { return m_port_write[3].bind(); }
	auto port9_write_callback() { return m_port_write[4].bind(); }
	auto porta_write_callback() { return m_port_write[5].bind(); }
	auto portb_write_callback() { return m_port_write[6].bind(); }

	virtual uint16_t ctrl_r() override;
	virtual void ctrl_w(uint16_t data) override;

	virtual uint16_t bus_req_r() override;
	virtual void bus_req_w(uint16_t data) override;

	virtual uint16_t cpu_status_r() override;

	virtual uint16_t ram_r(offs_t offset) override;
	virtual void ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset_after_children() override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void map(address_map &map) ATTR_COLD;

	template <int Port> uint8_t port_r();
	template <int Port> void port_w(uint8_t data);

	TIMER_CALLBACK_MEMBER(cpu_reset_timeout);

	required_device<h83007_device> m_maincpu;
	required_shared_ptr<uint16_t> m_ram;
	required_device<nvram_device> m_nvram;

	devcb_read8::array<7> m_port_read;
	devcb_write8::array<7> m_port_write;

	emu_timer *m_cpu_reset_timer;

	bool m_is_active;

	uint16_t m_bus_req;
	uint16_t m_ctrl;
};

DECLARE_DEVICE_TYPE(NAMCOS10_EXIO,      namcos10_exio_device)
DECLARE_DEVICE_TYPE(NAMCOS10_EXIO_BASE, namcos10_exio_base_device)
DECLARE_DEVICE_TYPE(NAMCOS10_MGEXIO,    namcos10_mgexio_device)

#endif // MAME_NAMCO_NAMCOS10_EXIO_H
