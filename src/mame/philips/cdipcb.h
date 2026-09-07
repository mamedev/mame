// license:BSD-3-Clause
// copyright-holders:Vincent Halver
/******************************************************************************

    CD-i PCB Test Device described in the 220 Service Manual.

    The service PCB has a seven segment display and three buttons.

*******************************************************************************/

#ifndef MAME_PHILIPS_CDIPCB_H
#define MAME_PHILIPS_CDIPCB_H

#pragma once

#include "bus/rs232/rs232.h"
#include "diserial.h"

class cdi_service_pcb_device : public device_t,
	public device_serial_interface,
	public device_rs232_port_interface
{
public:
	cdi_service_pcb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	DECLARE_INPUT_CHANGED_MEMBER(button_changed);

protected:
	// device_rs232_port_interface implementation
	virtual void input_txd(int state) override { device_serial_interface::rx_w(state); }

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_serial_interface implementation
	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_complete() override;

private:
	void put_char(uint8_t data);
	void send_char(uint8_t data);
	void refresh_display();

	output_finder<8> m_digits;

	bool m_answered;
	uint8_t m_cursor;
	uint8_t m_display[8];
};

DECLARE_DEVICE_TYPE(CDI_SERVICE_PCB, cdi_service_pcb_device)

#endif // MAME_PHILIPS_CDIPCB_H
