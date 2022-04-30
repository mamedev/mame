// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8_port.h

    H8 8 bits digital port


***************************************************************************/

#ifndef MAME_CPU_H8_H8_PORT_H
#define MAME_CPU_H8_H8_PORT_H

#pragma once

#include "h8.h"

class h8_port_device : public device_t {
public:
	h8_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	h8_port_device(const machine_config &mconfig, const char *tag, device_t *owner, int address, uint8_t default_ddr, uint8_t mask)
		: h8_port_device(mconfig, tag, owner, 0)
	{
		set_info(address, default_ddr, mask);
	}

	void set_info(int address, uint8_t default_ddr, uint8_t mask);

	void ddr_w(uint8_t data);
	uint8_t ddr_r();
	void dr_w(uint8_t data);
	uint8_t dr_r();
	uint8_t port_r();
	void pcr_w(uint8_t data);
	uint8_t pcr_r();
	void odr_w(uint8_t data);
	uint8_t odr_r();

protected:
	required_device<h8_device> cpu;
	address_space *io;

	int address;
	uint8_t default_ddr, ddr, pcr, odr;
	uint8_t mask;
	uint8_t dr;
	uint8_t last_output;

	virtual void device_start() override;
	virtual void device_reset() override;
	void update_output();
};

DECLARE_DEVICE_TYPE(H8_PORT, h8_port_device)

#endif // MAME_CPU_H8_H8_PORT_H
