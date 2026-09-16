// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    82919.h

    82919 module (RS232 interface)

*********************************************************************/

#ifndef MAME_BUS_HP_IPC_IO_82919_H
#define MAME_BUS_HP_IPC_IO_82919_H

#pragma once

#include "hp_ipc_io.h"
#include "machine/mc68681.h"
#include "bus/rs232/rs232.h"

class hp82919_io_card_device : public device_t, public device_hp_ipc_io_interface
{
public:
	// construction/destruction
	hp82919_io_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~hp82919_io_card_device();

	uint8_t read(offs_t addr);
	void write(offs_t addr , uint8_t data);

	void uart_irq(int state);
	void uart_a_tx(int state);
	void uart_b_tx(int state);
	void uart_output(uint8_t data);
	void prim_rxd(int state);
	void prim_dcd(int state);
	void prim_dsr(int state);
	void prim_ri(int state);
	void prim_cts(int state);
	void sec_rxd(int state);
	void sec_dcd(int state);
	void sec_cts(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void install_read_write_handlers(address_space& space , uint32_t base_addr) override;

private:
	required_device<rs232_port_device> m_rs232_prim;
	required_device<rs232_port_device> m_rs232_sec;
	required_device<mc68681_device> m_uart;
	required_ioport m_loopback_en;

	uint8_t m_int_level;
	bool m_int_en;
	bool m_int_forced;
	bool m_int_pending;
	bool m_uart_int;
	bool m_irq;
	bool m_loopback;

	void update_irq();
};

// device type definition
DECLARE_DEVICE_TYPE(HP82919_IO_CARD, hp82919_io_card_device)

#endif // MAME_BUS_HP_IPC_IO_82919_H
