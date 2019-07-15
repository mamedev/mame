// license:BSD-3-Clause
// copyright-holders:smf

#ifndef MAME_BUS_RS232_LOOPBACK_H
#define MAME_BUS_RS232_LOOPBACK_H

#pragma once

#include "rs232.h"

class rs232_loopback_device : public device_t, public device_rs232_port_interface
{
public:
	rs232_loopback_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual WRITE_LINE_MEMBER( input_txd ) override;
	virtual WRITE_LINE_MEMBER( input_rts ) override;
	virtual WRITE_LINE_MEMBER( input_dtr ) override;

protected:
	virtual void device_start() override;
};

class dec_rs232_loopback_device : public device_t, public device_rs232_port_interface
{
public:
	dec_rs232_loopback_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual WRITE_LINE_MEMBER( input_txd ) override;
	virtual WRITE_LINE_MEMBER( input_rts ) override;
	virtual WRITE_LINE_MEMBER( input_dtr ) override;
	virtual WRITE_LINE_MEMBER( input_spds ) override;

protected:
	virtual void device_start() override;
};

DECLARE_DEVICE_TYPE(RS232_LOOPBACK, rs232_loopback_device)
DECLARE_DEVICE_TYPE(DEC_RS232_LOOPBACK, dec_rs232_loopback_device)

#endif // MAME_BUS_RS232_LOOPBACK_H
