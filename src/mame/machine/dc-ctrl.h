// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef _DC_CTRL_H_
#define _DC_CTRL_H_

#include "mapledev.h"

#define MCFG_DC_CONTROLLER_ADD(_tag, _host_tag, _host_port, d0, d1, a0, a1, a2, a3, a4, a5) \
	MCFG_MAPLE_DEVICE_ADD(_tag, DC_CONTROLLER, 0, _host_tag, _host_port) \
	dc_controller_device::static_set_port_tag(*device, 0, d0); \
	dc_controller_device::static_set_port_tag(*device, 1, d1); \
	dc_controller_device::static_set_port_tag(*device, 2, a0); \
	dc_controller_device::static_set_port_tag(*device, 3, a1); \
	dc_controller_device::static_set_port_tag(*device, 4, a2); \
	dc_controller_device::static_set_port_tag(*device, 5, a3); \
	dc_controller_device::static_set_port_tag(*device, 6, a4); \
	dc_controller_device::static_set_port_tag(*device, 7, a5);

#define MCFG_DC_CONTROLLER_SET_ID(id) \
	dc_controller_device::static_set_id(*device, id);

#define MCFG_DC_CONTROLLER_SET_LICENSE(license) \
	dc_controller_device::static_set_license(*device, license);

#define MCFG_DC_CONTROLLER_SET_VERSIONS(versions) \
	dc_controller_device::static_set_versions(*device, versions);

class dc_controller_device : public maple_device
{
public:
	// construction/destruction
	dc_controller_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	static void static_set_port_tag(device_t &device, int port, std::string tag);
	static void static_set_id(device_t &device, const char *id);
	static void static_set_license(device_t &device, const char *license);
	static void static_set_versions(device_t &device, const char *versions);

	void maple_w(const UINT32 *data, UINT32 in_size) override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	void fixed_status(UINT32 *dest);
	void free_status(UINT32 *dest);
	void read(UINT32 *dest);

	std::string port_tag[8];
	const char *id, *license, *versions;

	ioport_port *port[8];
};

extern const device_type DC_CONTROLLER;

#endif // _DC_CTRL_H_
