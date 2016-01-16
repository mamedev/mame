// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "mapledev.h"
#include "maple-dc.h"

void maple_device::static_set_host(device_t &device, const char *_host_tag, int _host_port)
{
	maple_device &dev = downcast<maple_device &>(device);
	dev.host_tag = _host_tag;
	dev.host_port = _host_port;
}


maple_device::maple_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source) : device_t(mconfig, type, name, tag, owner, clock, shortname, source)
{
	host_tag = nullptr;
	host_port = 0;
}


void maple_device::device_start()
{
	host = machine().device<maple_dc_device>(host_tag);
	host->register_port(host_port, this);

	timer = timer_alloc(TIMER_ID);

	save_item(NAME(reply_buffer));
	save_item(NAME(reply_size));
	save_item(NAME(reply_partial));
}

void maple_device::maple_reset()
{
	device_reset();
}

void maple_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if(id != TIMER_ID)
		return;

	timer.adjust(attotime::never);
	reply_ready();
}

void maple_device::reply_ready_with_delay()
{
	// Arbitrary delay to avoid instant replies
	// Makes the maple dma code easier to write
	// There's usually something z80 in there after all, so it even makes sense
	timer->adjust(attotime::from_usec(100));
}

void maple_device::reply_ready()
{
	host->end_of_reply();
}

void maple_device::copy_with_spaces(UINT8 *dest, const char *source, int len)
{
	int i;
	for(i=0; i<len && source[i]; i++)
		*dest++ = source[i];
	for(; i<len; i++)
		*dest++ = 0x20;
}

void maple_device::maple_r(UINT32 *data, UINT32 &out_size, bool &partial)
{
	out_size = reply_size;
	memcpy(data, reply_buffer, out_size*4);
	partial = reply_partial;
}

void maple_device::reply_start(UINT8 code, UINT8 source, UINT8 size)
{
	reply_buffer[0] = ((size-1) << 24) | (host_port << 22) | (source << 16) | (host_port << 8) | code;
	reply_size = size;
	reply_partial = false;
}
