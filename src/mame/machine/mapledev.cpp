// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "mapledev.h"

maple_device::maple_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) : device_t(mconfig, type, tag, owner, clock)
, host(*this, finder_base::DUMMY_TAG)
{
	host_port = 0;
}


void maple_device::device_start()
{
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

void maple_device::device_timer(emu_timer &timer, device_timer_id id, int param)
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

void maple_device::copy_with_spaces(uint8_t *dest, const char *source, int len)
{
	int i;
	for(i=0; i<len && source[i]; i++)
		*dest++ = source[i];
	for(; i<len; i++)
		*dest++ = 0x20;
}

void maple_device::maple_r(uint32_t *data, uint32_t &out_size, bool &partial)
{
	out_size = reply_size;
	memcpy(data, reply_buffer, out_size*4);
	partial = reply_partial;
}

void maple_device::reply_start(uint8_t code, uint8_t source, uint8_t size)
{
	reply_buffer[0] = ((size-1) << 24) | (host_port << 22) | (source << 16) | (host_port << 8) | code;
	reply_size = size;
	reply_partial = false;
}
