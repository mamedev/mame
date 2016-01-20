// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef __JVSDEV_H__
#define __JVSDEV_H__

#include "emu.h"

#define MCFG_JVS_DEVICE_ADD(_tag, _type, _host) \
	MCFG_DEVICE_ADD(_tag, _type, 0) \
	jvs_device::static_set_jvs_host_tag(*device, _host);
class jvs_host;

class jvs_device : public device_t
{
public:
	jvs_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	static void static_set_jvs_host_tag(device_t &device, const char *jvs_host_tag);

	void chain(jvs_device *dev);
	void message(UINT8 dest, const UINT8 *send_buffer, UINT32 send_size, UINT8 *recv_buffer, UINT32 &recv_size);
	bool get_address_set_line();

protected:
	UINT32 jvs_outputs;

	void handle_output(ioport_port *port, UINT8 id, UINT8 val);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// JVS device overrides
	virtual const char *device_id();
	virtual UINT8 command_format_version();
	virtual UINT8 jvs_standard_version();
	virtual UINT8 comm_method_version();
	virtual void function_list(UINT8 *&buf);
	virtual bool switches(UINT8 *&buf, UINT8 count_players, UINT8 bytes_per_switch);
	virtual bool coin_counters(UINT8 *&buf, UINT8 count);
	virtual bool coin_add(UINT8 slot, INT32 count);
	virtual bool analogs(UINT8 *&buf, UINT8 count);
	virtual bool swoutputs(UINT8 count, const UINT8 *vals);
	virtual bool swoutputs(UINT8 id, UINT8 val);

private:
	const char *jvs_host_tag;
	jvs_device *next_device;
	UINT8 jvs_address;
	UINT32 jvs_reset_counter;

	int handle_message(const UINT8 *send_buffer, UINT32 send_size, UINT8 *&recv_buffer);
};

#endif
