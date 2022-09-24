// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_SLOT_MSX_RS232_H
#define MAME_BUS_MSX_SLOT_MSX_RS232_H

#include "rom.h"
#include "bus/rs232/rs232.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"

class msx_slot_rs232_device : public msx_slot_rom_device
{
public:
	msx_slot_rs232_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto irq_handler() { return m_irq_handler.bind(); }

protected:
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

private:
	void irq_mask_w(offs_t offset, uint8_t data);
	uint8_t status_r(offs_t offset);
	DECLARE_WRITE_LINE_MEMBER(out2_w);
	DECLARE_WRITE_LINE_MEMBER(cts_w);
	DECLARE_WRITE_LINE_MEMBER(dcd_w);
	DECLARE_WRITE_LINE_MEMBER(ri_w);
	DECLARE_WRITE_LINE_MEMBER(rxrdy_w);

	required_device<i8251_device> m_i8251;
	required_device<pit8253_device> m_i8253;
	required_ioport m_switch_port;
	required_device<rs232_port_device> m_rs232;

	devcb_write_line m_irq_handler;

	uint8_t m_irq_mask;
	bool m_out2;
	bool m_cts;
	bool m_dcd;
	bool m_ri;
};

DECLARE_DEVICE_TYPE(MSX_SLOT_RS232, msx_slot_rs232_device)

#endif // MAME_BUS_MSX_SLOT_MSX_RS232_H
