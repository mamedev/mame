// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_MACHINE_DC_CTRL_H
#define MAME_MACHINE_DC_CTRL_H

#pragma once

#include "mapledev.h"

#define MCFG_DC_CONTROLLER_ADD(_tag, _host_tag, _host_port, d0, d1, a0, a1, a2, a3, a4, a5) \
	MCFG_MAPLE_DEVICE_ADD(_tag, DC_CONTROLLER, 0, _host_tag, _host_port) \
	downcast<dc_common_device &>(*device).set_port_tag(0, d0); \
	downcast<dc_common_device &>(*device).set_port_tag(1, d1); \
	downcast<dc_common_device &>(*device).set_port_tag(2, a0); \
	downcast<dc_common_device &>(*device).set_port_tag(3, a1); \
	downcast<dc_common_device &>(*device).set_port_tag(4, a2); \
	downcast<dc_common_device &>(*device).set_port_tag(5, a3); \
	downcast<dc_common_device &>(*device).set_port_tag(6, a4); \
	downcast<dc_common_device &>(*device).set_port_tag(7, a5);

#define MCFG_DC_CONTROLLER_SET_MODEL(name) \
	downcast<dc_common_device &>(*device).set_model(name);

#define MCFG_DC_CONTROLLER_SET_LICENSE(license) \
	downcast<dc_common_device &>(*device).set_license(license);

#define MCFG_DC_CONTROLLER_SET_VERSIONS(versions) \
	ddowncast<dc_common_device &>(*device).set_versions(versions);

class dc_common_device : public maple_device
{
public:
	// construction/destruction
	dc_common_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	void set_port_tag(int port, const char *tag) { port_tag[port] = tag; }
	// TODO: we probably don't need these setters
	void set_model(const char *new_id) { model = new_id; }
	void set_license(const char *new_license) { license = new_license; }
	void set_versions(const char *new_versions) { versions = new_versions; }

	void maple_w(const uint32_t *data, uint32_t in_size) override;

protected:
	// device-level overrides
	virtual void device_start() override;

	const char *port_tag[8];
	const char *model, *license, *versions;
	uint32_t id;
	uint32_t electric_current;
	uint32_t region;

	ioport_port *port[8];

	virtual void fixed_status(uint32_t *dest) = 0;
	virtual void free_status(uint32_t *dest) = 0;
	virtual void read(uint32_t *dest) = 0;
};

class dc_controller_device : public dc_common_device
{
public:
	// construction/destruction
	dc_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

private:
	void fixed_status(uint32_t *dest) override;
	void free_status(uint32_t *dest) override;
	void read(uint32_t *dest) override;
};

#define MCFG_DC_KEYBOARD_ADD(_tag, _host_tag, _host_port, d0, d1, a0, a1, a2, a3, a4, a5) \
	MCFG_MAPLE_DEVICE_ADD(_tag, DC_KEYBOARD, 0, _host_tag, _host_port) \
	downcast<dc_common_device &>(*device).set_port_tag(0, d0); \
	downcast<dc_common_device &>(*device).set_port_tag(1, d1); \
	downcast<dc_common_device &>(*device).set_port_tag(2, a0); \
	downcast<dc_common_device &>(*device).set_port_tag(3, a1); \
	downcast<dc_common_device &>(*device).set_port_tag(4, a2); \
	downcast<dc_common_device &>(*device).set_port_tag(5, a3); \
	downcast<dc_common_device &>(*device).set_port_tag(6, a4); \
	downcast<dc_common_device &>(*device).set_port_tag(7, a5);


class dc_keyboard_device : public dc_common_device
{
public:
	// construction/destruction
	dc_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

//protected:

private:
	void fixed_status(uint32_t *dest) override;
	void free_status(uint32_t *dest) override;
	void read(uint32_t *dest) override;
};

DECLARE_DEVICE_TYPE(DC_CONTROLLER, dc_controller_device)
DECLARE_DEVICE_TYPE(DC_KEYBOARD, dc_keyboard_device)

#endif // MAME_MACHINE_DC_CTRL_H
