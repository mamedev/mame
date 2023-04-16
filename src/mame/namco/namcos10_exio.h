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

	virtual void ctrl_w(uint16_t data) {}

	virtual uint16_t cpu_status_r() { return 0; }

	virtual uint16_t ram_r(offs_t offset) { return 0xff; }
	virtual void ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) {}

protected:
	namcos10_exio_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t ident_code);

	virtual void device_start() override;

	const uint8_t m_ident_code;
};

class namcos10_exio_device : public namcos10_exio_base_device
{
public:
	namcos10_exio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto analog_callback() { return m_analog_cb.bind(); }

	virtual void ctrl_w(uint16_t data) override;

	virtual uint16_t cpu_status_r() override;

	virtual uint16_t ram_r(offs_t offset) override;
	virtual void ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	virtual void device_start() override;
	virtual void device_reset_after_children() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	void map(address_map &map);

	template <int Port> uint8_t port_read(offs_t offset);
	template <int Port> void port_write(offs_t offset, uint8_t data);

	required_device<tmp95c061_device> m_maincpu;
	required_shared_ptr<uint16_t> m_ram;

	devcb_read16 m_analog_cb;

	bool m_is_active;
	uint32_t m_analog_idx;
};

class namcos10_mgexio_device : public namcos10_exio_base_device
{
public:
	namcos10_mgexio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void ctrl_w(uint16_t data) override;

	virtual uint16_t cpu_status_r() override;

	virtual uint16_t ram_r(offs_t offset) override;
	virtual void ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	virtual void device_start() override;
	virtual void device_reset_after_children() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	void map(address_map &map);
	void io_map(address_map &map);

	required_device<h83007_device> m_maincpu;
	required_shared_ptr<uint16_t> m_ram;
	required_device<nvram_device> m_nvram;

	uint32_t m_active_state;
};

DECLARE_DEVICE_TYPE(NAMCOS10_EXIO,      namcos10_exio_device)
DECLARE_DEVICE_TYPE(NAMCOS10_EXIO_BASE, namcos10_exio_base_device)
DECLARE_DEVICE_TYPE(NAMCOS10_MGEXIO,    namcos10_mgexio_device)

#endif // MAME_NAMCO_NAMCOS10_EXIO_H
