// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_DEVICES_RS232_SUN_KBD_H
#define MAME_DEVICES_RS232_SUN_KBD_H

#pragma once

#include "rs232.h"
#include "bus/sunkbd/sunkbd.h"


extern device_type const SUN_KBD_ADAPTOR;


class sun_keyboard_adaptor_device : public device_t, public device_rs232_port_interface
{
public:
	sun_keyboard_adaptor_device(machine_config const &mconfig, char const *tag, device_t *owner, UINT32 clock);
	virtual ~sun_keyboard_adaptor_device() override;

	virtual machine_config_constructor device_mconfig_additions() const override;

	virtual DECLARE_WRITE_LINE_MEMBER( input_txd ) override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<sun_keyboard_port_device> m_keyboard_port;
};

#endif // MAME_DEVICES_RS232_SUN_KBD_H
