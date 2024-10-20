// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_SEGA_DC_CTRL_H
#define MAME_SEGA_DC_CTRL_H

#pragma once

#include "mapledev.h"


class dc_common_device : public maple_device
{
public:
	// construction/destruction
	dc_common_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	template <uint8_t Which, typename T>
	void set_port_tag(T &&port_tag) { port[Which].set_tag(std::forward<T>(port_tag)); }
	template <uint8_t First = 0U, typename T, typename... U>
	void set_port_tags(T &&first_tag, U &&... other_tags)
	{
		set_port_tag<First>(std::forward<T>(first_tag));
		set_port_tags<First + 1>(std::forward<U>(other_tags)...);
	}

	// TODO: we probably don't need these setters
	void set_model(const char *new_id) { model = new_id; }
	void set_license(const char *new_license) { license = new_license; }
	void set_versions(const char *new_versions) { versions = new_versions; }

	void maple_w(const uint32_t *data, uint32_t in_size) override;

protected:
	template <uint8_t First> void set_port_tags() { }

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	const char *model = nullptr, *license = nullptr, *versions = nullptr;
	uint32_t id = 0;
	uint32_t electric_current = 0;
	uint32_t region = 0;

	optional_ioport_array<8> port;

	virtual void fixed_status(uint32_t *dest) = 0;
	virtual void free_status(uint32_t *dest) = 0;
	virtual void read(uint32_t *dest) = 0;
};

class dc_controller_device : public dc_common_device
{
public:
	// construction/destruction
	template <typename T>
	dc_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&host_tag, int host_port)
		: dc_controller_device(mconfig, tag, owner, clock)
	{
		host.set_tag(std::forward<T>(host_tag));
		set_host_port(host_port);
	}

	dc_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

private:
	void fixed_status(uint32_t *dest) override;
	void free_status(uint32_t *dest) override;
	void read(uint32_t *dest) override;
};


class dc_keyboard_device : public dc_common_device
{
public:
	// construction/destruction
	template <typename T>
	dc_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&host_tag, int host_port)
		: dc_keyboard_device(mconfig, tag, owner, clock)
	{
		host.set_tag(std::forward<T>(host_tag));
		set_host_port(host_port);
	}

	dc_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

//protected:

private:
	void fixed_status(uint32_t *dest) override;
	void free_status(uint32_t *dest) override;
	void read(uint32_t *dest) override;
};

DECLARE_DEVICE_TYPE(DC_CONTROLLER, dc_controller_device)
DECLARE_DEVICE_TYPE(DC_KEYBOARD, dc_keyboard_device)

#endif // MAME_SEGA_DC_CTRL_H
