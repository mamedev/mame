// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "jvsdev.h"
#include "jvshost.h"

void jvs_device::static_set_jvs_host_tag(device_t &device, const char *jvs_host_tag)
{
	jvs_device &jvsdev = downcast<jvs_device &>(device);
	jvsdev.jvs_host_tag = jvs_host_tag;
}

jvs_device::jvs_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source)
{
	jvs_host_tag = 0;
	next_device = 0;
}

const char *jvs_device::device_id()
{
	return "";
}

UINT8 jvs_device::command_format_version()
{
	return 0x13;
}

UINT8 jvs_device::jvs_standard_version()
{
	return 0x30;
}

UINT8 jvs_device::comm_method_version()
{
	return 0x10;
}

void jvs_device::chain(jvs_device *dev)
{
	if(next_device)
		next_device->chain(dev);
	else
		next_device = dev;
}

void jvs_device::message(UINT8 dest, const UINT8 *send_buffer, UINT32 send_size, UINT8 *recv_buffer, UINT32 &recv_size)
{
	recv_size = 0;

	// Set Address special case
	if(send_size == 2 && send_buffer[0] == 0xf1) {
		if(next_device && !next_device->get_address_set_line())
			next_device->message(dest, send_buffer, send_size, recv_buffer, recv_size);
		else {
			jvs_address = send_buffer[1];
			recv_size = 2;
			recv_buffer[0] = 0x01;
			recv_buffer[1] = 0x01;
		}
		return;
	}

	// dest=0xff is broadcast
	if(dest == 0xff || dest == jvs_address) {
		const UINT8 *s = send_buffer;
		UINT8 *d = recv_buffer;
		*d++ = 0x01;
		while(s < send_buffer + send_size) {
			int len = handle_message(s, send_size-(s-send_buffer), d);
			if(len == -1) {
				// Unknown command
				recv_size = 1;
				recv_buffer[0] = 0x02;
				return;
			} else if(len == 0) {
				// Incorrect parameter
				*d++ = 0x02;
				break;
			} else
				s += len;
		}
		recv_size = d - recv_buffer;
	}

	// Pass along the message if the device hasn't replied
	// Should we cumulate answers instead?
	if(next_device && !recv_size)
		next_device->message(dest, send_buffer, send_size, recv_buffer, recv_size);
}

int jvs_device::handle_message(const UINT8 *send_buffer, UINT32 send_size, UINT8 *&recv_buffer)
{
	UINT32 old_reset_counter = jvs_reset_counter;
	jvs_reset_counter = 0;

	switch(send_buffer[0]) {
	case 0xf0:
		if(send_size < 2 || send_buffer[1] != 0xd9)
			return 0;

		// Reset, must be sent twice
		jvs_reset_counter = old_reset_counter+1;
		if(jvs_reset_counter == 2)
			device_reset();
		return 2;

	case 0x10: {
		const char *id = device_id();
		int len = strlen(id)+1;
		*recv_buffer++ = 0x01;
		memcpy(recv_buffer, id, len);
		recv_buffer += len;
		return 1;
	}

	case 0x11:
		*recv_buffer++ = 0x01;
		*recv_buffer++ = command_format_version();
		return 1;

	case 0x12:
		*recv_buffer++ = 0x01;
		*recv_buffer++ = jvs_standard_version();
		return 1;

	case 0x13:
		*recv_buffer++ = 0x01;
		*recv_buffer++ = comm_method_version();
		return 1;

	case 0x14:
		*recv_buffer++ = 0x01;
		function_list(recv_buffer);
		*recv_buffer++ = 0x00;
		return 1;

	case 0x20:
		if(send_size < 3)
			return 0;
		*recv_buffer++ = 0x01;
		return switches(recv_buffer, send_buffer[1], send_buffer[2]) ? 3 : 0;

	case 0x21:
		if(send_size < 2)
			return 0;
		*recv_buffer++ = 0x01;
		return coin_counters(recv_buffer, send_buffer[1]) ? 2 : 0;

	case 0x22:
		if(send_size < 2)
			return 0;
		*recv_buffer++ = 0x01;
		return analogs(recv_buffer, send_buffer[1]) ? 2 : 0;

	case 0x30:
		if(send_size < 4)
			return 0;
		*recv_buffer++ = 0x01;
		return coin_add(send_buffer[1], -((send_buffer[2] << 8) | send_buffer[3])) ? 4 : 0;

	case 0x31:
		if(send_size < 4)
			return 0;
		*recv_buffer++ = 0x01;
		return coin_add(send_buffer[1],  ((send_buffer[2] << 8) | send_buffer[3])) ? 4 : 0;

	case 0x32:
		if(send_size < 2 || send_size < 2+send_buffer[1])
			return 0;
		*recv_buffer++ = 0x01;
		return swoutputs(send_buffer[1], send_buffer+2) ? 2+send_buffer[1] : 0;

	case 0x38:
		if(send_size < 3)
			return 0;
		*recv_buffer++ = 0x01;
		return swoutputs(send_buffer[1], send_buffer[2]) ? 3 : 0;

	default:
		logerror("JVSDEV: unhandled command %02x\n", send_buffer[0]);
		return 0;
	}

	// never executed
	//return -1;
}

bool jvs_device::get_address_set_line()
{
	return jvs_address != 0xff && (!next_device || next_device->get_address_set_line());
}

void jvs_device::device_start()
{
	jvs_host *host = machine().device<jvs_host>(jvs_host_tag);
	if(!host)
		fatalerror("JVS device %s could not find JVS host %s\n", tag(), jvs_host_tag);
	host->add_device(this);

	save_item(NAME(jvs_address));
	save_item(NAME(jvs_reset_counter));
}

void jvs_device::device_reset()
{
	jvs_address = 0xff;
	jvs_reset_counter = 0;
	jvs_outputs = 0;
}

void jvs_device::function_list(UINT8 *&buf)
{
}

bool jvs_device::coin_counters(UINT8 *&buf, UINT8 count)
{
	return false;
}

bool jvs_device::coin_add(UINT8 slot, INT32 count)
{
	return false;
}


bool jvs_device::switches(UINT8 *&buf, UINT8 count_players, UINT8 bytes_per_switch)
{
	return false;
}

bool jvs_device::analogs(UINT8 *&buf, UINT8 count)
{
	return false;
}

bool jvs_device::swoutputs(UINT8 count, const UINT8 *vals)
{
	return false;
}

bool jvs_device::swoutputs(UINT8 id, UINT8 val)
{
	return false;
}

void jvs_device::handle_output(ioport_port *port, UINT8 id, UINT8 val)
{
	UINT32 m = 1 << id;
	switch(val) {
	case 0: jvs_outputs &= ~m; break;
	case 1: jvs_outputs |=  m;  break;
	case 2: jvs_outputs ^=  m;  break;
	}

	if (port)
	{
		port->write(jvs_outputs, m);
	}
}
