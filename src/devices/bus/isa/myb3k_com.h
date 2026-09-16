// license:BSD-3-Clause
// copyright-holders: Joakim Larsson Edstrom
#ifndef MAME_BUS_ISA_MYB3K_COM_H
#define MAME_BUS_ISA_MYB3K_COM_H

#pragma once

/*
 * This card is part of Matsushita MyBrain 3000 and branded Panasonic JB-3000
 * and Ericsson Step/One computers. These are not really PC compatibles but
 * mimic the IBM PC in feature set and stay close on hardware but not 100%
 *
 * Serial connector from Step/One service manual
 * ---------------------------------------------
 * Pin numbering------====------------------+  1 - FG (AA) Chassis GND
 * | 25 23 21 19 17 15 13 11  9  7  5  3  1 |  3 - SD (BA) TxD
 * | 26 24 22 20 18 16 14 12 10  8  6  4  2 |  4 - ST (DB) TxC
 * +----------------------------------------+  5 - RD (BB) RxD
 *  7 - RS (AC) RTS   8 - RT (DD) RxC          9 - CS (CB) CTS
 * 11 - DR (CC) DSR  13 - SG (AB) Signal GND  14 - ER (CD) DTR
 * 15 - CD (CF) DCD  18 - CI (CE) RI
 *
 */

#include "isa.h"
#include "machine/i8251.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa8_myb3k_com_device

class isa8_myb3k_com_device :
		public device_t,
		public device_isa8_card_interface
{
public:
	// construction/destruction
	isa8_myb3k_com_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void pit_txc(int state);
	void pit_rxc(int state);
	void rem_txc(int state);
	void rem_rxc(int state);
	void com_int_rx(int state);
	void com_int_tx(int state);
	void dcd_w(int state);
	void ri_w(int state);
	void dce_control(uint8_t data);
	uint8_t dce_status();

protected:
	isa8_myb3k_com_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// helpers
	void com_int();

	required_ioport m_iobase;
	required_ioport m_isairq;
	required_device<i8251_device> m_usart;
	bool m_installed;
	int m_irq;
	int m_irq_tx;
	int m_irq_rx;
	int m_control;
	int m_status;
};


// device type definition
DECLARE_DEVICE_TYPE(ISA8_MYB3K_COM, isa8_myb3k_com_device)

#endif  // MAME_BUS_ISA_MYB3K_COM_H
