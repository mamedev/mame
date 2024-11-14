// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    98036.h

    98036 module (RS232 interface)

*********************************************************************/

#ifndef MAME_BUS_HP9845_IO_98036_H
#define MAME_BUS_HP9845_IO_98036_H

#pragma once

#include "hp9845_io.h"
#include "machine/i8251.h"
#include "machine/mc14411.h"
#include "bus/rs232/rs232.h"

class hp98036_io_card_device : public device_t, public device_hp9845_io_interface
{
public:
	// construction/destruction
	hp98036_io_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~hp98036_io_card_device();

	virtual uint16_t reg_r(address_space &space, offs_t offset) override;
	virtual void reg_w(address_space &space, offs_t offset, uint16_t data) override;

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device<i8251_device> m_uart;
	required_device<mc14411_device> m_clock_gen;
	required_device<rs232_port_device> m_rs232;
	required_ioport m_s1_input;
	required_ioport m_s2_input;
	required_ioport m_s3_input;
	required_ioport m_jumper_input;
	required_ioport m_loopback_en;

	void bit_rate_clock(int state);
	void slow_clock(int state);
	void txd_w(int state);
	void dtr_w(int state);
	void rts_w(int state);
	void txrdy_w(int state);
	void rxrdy_w(int state);
	void rxd_w(int state);
	void dsr_w(int state);
	void cts_w(int state);

	// State of initialization FSM
	// State U11A-Q U11B-Q
	// 0     0      0
	// 1     1      0      Load mode word
	// 2     1      1      Load control word
	// 3     0      1      Done
	uint8_t m_init_state;
	bool m_irq;         // U9-2
	bool m_int_en;      // U10-2
	bool m_tx_int_en;   // U10-10
	bool m_rx_int_en;   // U10-7
	bool m_txrdy;       // U31-15
	bool m_rxrdy;       // U31-14
	bool m_wrrq;        // U19-13
	bool m_rdrq;        // U19-7
	bool m_uart_cmd;    // U10-15
	bool m_uart_wr_mode;// U17-15
	bool m_flg;         // U9-10
	bool m_half_baud;   // U21-13
	bool m_baud_div;    // U21-1
	bool m_r4_dir;      // U19-4
	bool m_loopback;
	uint8_t m_r4in_data;    // U29, U30
	uint8_t m_r4out_data;   // U26, U27
	uint8_t m_lineout;  // U28

	void int_reset();
	void update_flg();
	void update_irq();
	void uart_rd();
	void uart_wr();
	void write_uart(bool cmd, uint8_t data);
};

// device type definitions
DECLARE_DEVICE_TYPE(HP98036_IO_CARD, hp98036_io_card_device)

#endif // MAME_BUS_HP9845_IO_98036_H
