// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_BUS_RS232_SUN_KBD_H
#define MAME_BUS_RS232_SUN_KBD_H

#pragma once

#include "rs232.h"
#include "bus/sunkbd/sunkbd.h"


DECLARE_DEVICE_TYPE(SUN_KBD_ADAPTOR, sun_keyboard_adaptor_device)


class sun_keyboard_adaptor_device : public device_t, public device_rs232_port_interface
{
public:
	sun_keyboard_adaptor_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);
	virtual ~sun_keyboard_adaptor_device() override;

	virtual DECLARE_WRITE_LINE_MEMBER( input_txd ) override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<sun_keyboard_port_device> m_keyboard_port;
};

#endif // MAME_BUS_RS232_SUN_KBD_H
