// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_SLOT_MSX_RS232_H
#define MAME_BUS_MSX_SLOT_MSX_RS232_H

#pragma once

#include "rom.h"
#include "bus/rs232/rs232.h"
#include "machine/i8251.h"
#include "machine/nvram.h"
#include "machine/pit8253.h"

class msx_slot_rs232_base_device : public msx_slot_rom_device
{
public:

	// configuration helpers
	auto irq_handler() { return m_irq_handler.bind(); }

protected:
	msx_slot_rs232_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void irq_mask_w(offs_t offset, u8 data);
	virtual u8 status_r(offs_t offset) { return 0; }
	virtual void update_irq_state() { }
	void out2_w(int state);
	void cts_w(int state);
	void dcd_w(int state);
	void ri_w(int state);
	void rxrdy_w(int state);
	void txrdy_w(int state);

	required_device<i8251_device> m_i8251;
	required_device<pit8253_device> m_i8253;
	required_device<rs232_port_device> m_rs232;

	devcb_write_line m_irq_handler;

	u8 m_irq_mask;
	bool m_out2;
	bool m_cts;
	bool m_dcd;
	bool m_ri;
	bool m_rxrdy;
	bool m_txrdy;
};


class msx_slot_rs232_device : public msx_slot_rs232_base_device
{
public:
	msx_slot_rs232_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;

	virtual u8 status_r(offs_t offset) override;
	virtual void update_irq_state() override;
};


class msx_slot_rs232_mitsubishi_device : public msx_slot_rs232_base_device
{
public:
	msx_slot_rs232_mitsubishi_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual u8 status_r(offs_t offset) override;
	virtual void update_irq_state() override;

	required_ioport m_switch_port;
};


class msx_slot_rs232_sony_device : public msx_slot_rs232_base_device
{
public:
	msx_slot_rs232_sony_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t status_r(offs_t offset) override;
	virtual void update_irq_state() override;

	static constexpr size_t RAM_SIZE = 0x800;

	required_ioport m_switch_port;
	std::unique_ptr<u8[]> m_ram;
};


class msx_slot_rs232_svi738_device : public msx_slot_rs232_base_device
{
public:
	msx_slot_rs232_svi738_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;

	virtual u8 status_r(offs_t offset) override;
	virtual void update_irq_state() override;
};


class msx_slot_rs232_toshiba_device : public msx_slot_rs232_base_device
{
public:
	msx_slot_rs232_toshiba_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t status_r(offs_t offset) override;
	virtual void update_irq_state() override;

	required_ioport m_switch_port;
};


class msx_slot_rs232_toshiba_hx3x_device : public msx_slot_rs232_base_device
{
public:
	msx_slot_rs232_toshiba_hx3x_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	static constexpr size_t SRAM_SIZE = 0x800;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual uint8_t status_r(offs_t offset) override;
	virtual void update_irq_state() override;

	void bank_w(u8 data);
	u8 bank_r();

	required_ioport m_switch_port;
	required_ioport m_copy_port;
	required_device<nvram_device> m_nvram;
	memory_bank_creator m_rombank;
	memory_view m_view;
	std::unique_ptr<u8[]> m_sram;
	u8 m_bank_reg;
};


DECLARE_DEVICE_TYPE(MSX_SLOT_RS232, msx_slot_rs232_device)
DECLARE_DEVICE_TYPE(MSX_SLOT_RS232_MITSUBISHI, msx_slot_rs232_mitsubishi_device)
DECLARE_DEVICE_TYPE(MSX_SLOT_RS232_SONY, msx_slot_rs232_sony_device)
DECLARE_DEVICE_TYPE(MSX_SLOT_RS232_SVI738, msx_slot_rs232_svi738_device)
DECLARE_DEVICE_TYPE(MSX_SLOT_RS232_TOSHIBA, msx_slot_rs232_toshiba_device)
DECLARE_DEVICE_TYPE(MSX_SLOT_RS232_TOSHIBA_HX3X, msx_slot_rs232_toshiba_hx3x_device)


#endif // MAME_BUS_MSX_SLOT_MSX_RS232_H
