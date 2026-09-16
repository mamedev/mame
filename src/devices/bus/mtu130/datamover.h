// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Datamover - 68K-based extension board

#ifndef MAME_BUS_MTU130_DATAMOVER_H
#define MAME_BUS_MTU130_DATAMOVER_H

#include "board.h"
#include "cpu/m68000/m68000.h"

class mtu130_datamover_base_device : public device_t, public mtu130_extension_interface
{
public:
	mtu130_datamover_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t base_address, uint32_t clock = 0);
	virtual ~mtu130_datamover_base_device() = default;

	virtual void write23(offs_t offset, u8 data) override;
	virtual u8 read23(offs_t offset) override;

	virtual void map_io(address_space_installer &space) override;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<m68000_device> m_cpu;
	required_shared_ptr<u16> m_ram;

	bool m_ram_visible;
	bool m_ram_xor;
	u8 m_ram_bank;
	bool m_irq4_req;
	bool m_irq7_req;
	bool m_irq6502_req;
	bool m_irq6502_en;

	uint32_t m_base_address;

	void m6502_map(address_map &map) ATTR_COLD;
	void m68k_map(address_map &map) ATTR_COLD;
	void m68k_cs_map(address_map &map) ATTR_COLD;

	void control_w(u8 data);
	void enable_w(u8 data);
	u8 status_r();

	void irq_req_w(u8 data);
	u8 irq_req_r();
};

class mtu130_datamover0_device : public mtu130_datamover_base_device
{
public:
	mtu130_datamover0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~mtu130_datamover0_device() = default;
};

class mtu130_datamover1_device : public mtu130_datamover_base_device
{
public:
	mtu130_datamover1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~mtu130_datamover1_device() = default;
};

DECLARE_DEVICE_TYPE(MTU130_DATAMOVER0, mtu130_datamover0_device)
DECLARE_DEVICE_TYPE(MTU130_DATAMOVER1, mtu130_datamover1_device)

#endif
