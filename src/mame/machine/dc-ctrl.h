// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_MACHINE_DC_CTRL_H
#define MAME_MACHINE_DC_CTRL_H

#pragma once

#include "mapledev.h"

#define MCFG_DC_CONTROLLER_ADD(_tag, _host_tag, _host_port, d0, d1, a0, a1, a2, a3, a4, a5) \
	MCFG_MAPLE_DEVICE_ADD(_tag, DC_CONTROLLER, 0, _host_tag, _host_port) \
	downcast<dc_controller_device &>(*device).set_port_tag(0, d0); \
	downcast<dc_controller_device &>(*device).set_port_tag(1, d1); \
	downcast<dc_controller_device &>(*device).set_port_tag(2, a0); \
	downcast<dc_controller_device &>(*device).set_port_tag(3, a1); \
	downcast<dc_controller_device &>(*device).set_port_tag(4, a2); \
	downcast<dc_controller_device &>(*device).set_port_tag(5, a3); \
	downcast<dc_controller_device &>(*device).set_port_tag(6, a4); \
	downcast<dc_controller_device &>(*device).set_port_tag(7, a5);

#define MCFG_DC_CONTROLLER_SET_ID(id) \
	downcast<dc_controller_device &>(*device).set_id(id);

#define MCFG_DC_CONTROLLER_SET_LICENSE(license) \
	downcast<dc_controller_device &>(*device).set_license(license);

#define MCFG_DC_CONTROLLER_SET_VERSIONS(versions) \
	ddowncast<dc_controller_device &>(*device).set_versions(versions);

class dc_controller_device : public maple_device
{
public:
	// construction/destruction
	dc_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_port_tag(int port, const char *tag) { port_tag[port] = tag; }
	void set_id(const char *new_id) { id = new_id; }
	void set_license(const char *new_license) { license = new_license; }
	void set_versions(const char *new_versions) { versions = new_versions; }

	void maple_w(const uint32_t *data, uint32_t in_size) override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	void fixed_status(uint32_t *dest);
	void free_status(uint32_t *dest);
	void read(uint32_t *dest);

	const char *port_tag[8];
	const char *id, *license, *versions;

	ioport_port *port[8];
};

DECLARE_DEVICE_TYPE(DC_CONTROLLER, dc_controller_device)

#endif // MAME_MACHINE_DC_CTRL_H
