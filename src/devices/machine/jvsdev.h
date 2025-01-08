// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_MACHINE_JVSDEV_H
#define MAME_MACHINE_JVSDEV_H

#pragma once

#include "jvshost.h"

class jvs_device : public device_t
{
public:
	void chain(jvs_device *dev);
	void message(uint8_t dest, const uint8_t *send_buffer, uint32_t send_size, uint8_t *recv_buffer, uint32_t &recv_size);
	bool get_address_set_line();

protected:
	uint32_t jvs_outputs;

	void handle_output(ioport_port *port, uint8_t id, uint8_t val);

	jvs_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// JVS device overrides
	virtual const char *device_id();
	virtual uint8_t command_format_version();
	virtual uint8_t jvs_standard_version();
	virtual uint8_t comm_method_version();
	virtual void function_list(uint8_t *&buf);
	virtual bool switches(uint8_t *&buf, uint8_t count_players, uint8_t bytes_per_switch);
	virtual bool coin_counters(uint8_t *&buf, uint8_t count);
	virtual bool coin_add(uint8_t slot, int32_t count);
	virtual bool analogs(uint8_t *&buf, uint8_t count);
	virtual bool swoutputs(uint8_t count, const uint8_t *vals);
	virtual bool swoutputs(uint8_t id, uint8_t val);
	virtual int handle_message(const uint8_t *send_buffer, uint32_t send_size, uint8_t *&recv_buffer);

	required_device<jvs_host> host;

private:
	jvs_device *next_device;
	uint8_t jvs_address;
	uint32_t jvs_reset_counter;

};

#endif // MAME_MACHINE_JVSDEV_H
