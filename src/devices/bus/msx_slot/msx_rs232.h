// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_SLOT_MSX_RS232_H
#define MAME_BUS_MSX_SLOT_MSX_RS232_H

#include "rom.h"
#include "bus/rs232/rs232.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"

class msx_slot_rs232_base_device : public msx_slot_rom_device
{
public:

	// configuration helpers
	auto irq_handler() { return m_irq_handler.bind(); }

protected:
	msx_slot_rs232_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	virtual void irq_mask_w(offs_t offset, uint8_t data);
	virtual uint8_t status_r(offs_t offset) { return 0; }
	virtual void update_irq_state() { }
	DECLARE_WRITE_LINE_MEMBER(out2_w);
	DECLARE_WRITE_LINE_MEMBER(cts_w);
	DECLARE_WRITE_LINE_MEMBER(dcd_w);
	DECLARE_WRITE_LINE_MEMBER(ri_w);
	DECLARE_WRITE_LINE_MEMBER(rxrdy_w);
	DECLARE_WRITE_LINE_MEMBER(txrdy_w);

	required_device<i8251_device> m_i8251;
	required_device<pit8253_device> m_i8253;
	required_device<rs232_port_device> m_rs232;

	devcb_write_line m_irq_handler;

	uint8_t m_irq_mask;
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
	msx_slot_rs232_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;

	virtual uint8_t status_r(offs_t offset) override;
	virtual void update_irq_state() override;
};


class msx_slot_rs232_mitsubishi_device : public msx_slot_rs232_base_device
{
public:
	msx_slot_rs232_mitsubishi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual ioport_constructor device_input_ports() const override;

	virtual uint8_t status_r(offs_t offset) override;
	virtual void update_irq_state() override;

	required_ioport m_switch_port;
};


class msx_slot_rs232_sony_device : public msx_slot_rs232_base_device
{
public:
	msx_slot_rs232_sony_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

protected:
	virtual void device_start() override;
	virtual ioport_constructor device_input_ports() const override;

	virtual uint8_t status_r(offs_t offset) override;
	virtual void update_irq_state() override;

	static constexpr uint32_t RAM_SIZE = 0x800;

	required_ioport m_switch_port;
	std::vector<uint8_t> m_ram;
};


class msx_slot_rs232_svi738_device : public msx_slot_rs232_base_device
{
public:
	msx_slot_rs232_svi738_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;

	virtual uint8_t status_r(offs_t offset) override;
	virtual void update_irq_state() override;
};


class msx_slot_rs232_toshiba_device : public msx_slot_rs232_base_device
{
public:
	msx_slot_rs232_toshiba_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual ioport_constructor device_input_ports() const override;

	virtual uint8_t status_r(offs_t offset) override;
	virtual void update_irq_state() override;

	required_ioport m_switch_port;
};


DECLARE_DEVICE_TYPE(MSX_SLOT_RS232, msx_slot_rs232_device)
DECLARE_DEVICE_TYPE(MSX_SLOT_RS232_MITSUBISHI, msx_slot_rs232_mitsubishi_device)
DECLARE_DEVICE_TYPE(MSX_SLOT_RS232_SONY, msx_slot_rs232_sony_device)
DECLARE_DEVICE_TYPE(MSX_SLOT_RS232_SVI738, msx_slot_rs232_svi738_device)
DECLARE_DEVICE_TYPE(MSX_SLOT_RS232_TOSHIBA, msx_slot_rs232_toshiba_device)


#endif // MAME_BUS_MSX_SLOT_MSX_RS232_H
