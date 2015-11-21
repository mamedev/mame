// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "jvshost.h"
#include "jvsdev.h"

void jvs_host::add_device(jvs_device *dev)
{
	if(first_device)
		first_device->chain(dev);
	else
		first_device = dev;
}

void jvs_host::device_start()
{
	save_item(NAME(send_size));
	save_item(NAME(recv_size));
	save_item(NAME(send_buffer));
	save_item(NAME(recv_buffer));
	save_item(NAME(recv_is_encoded));
}

void jvs_host::device_reset()
{
	send_size = recv_size = 0;
	recv_is_encoded = false;
	memset(send_buffer, 0, sizeof(send_buffer));
	memset(recv_buffer, 0, sizeof(recv_buffer));
}

jvs_host::jvs_host(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source), send_size(0), recv_size(0), recv_is_encoded(false)
{
	first_device = 0;
}

void jvs_host::push(UINT8 val)
{
	send_buffer[send_size++] = val;
}

void jvs_host::commit_raw()
{
	recv_size = 0;
	if(!send_size)
		return;

	// Message must:
	// - have a non-zero destination in the first byte
	// - have the message length without the two header bytes but with the checksum byte in the second byte
	// - have at least one command byte
	if(send_size < 3 || send_buffer[0] == 0x00 || send_buffer[1] != send_size-1) {
				logerror("JVS checksum error\n");
				// "This message is crap" doesn't exist so call it checksum error
				recv_buffer[0] = 0x00;
				recv_buffer[1] = 0x02;
				recv_buffer[2] = 0x03;
				recv_size = 3;
		} else {
		if(first_device) {
			first_device->message(send_buffer[0], send_buffer+2, send_size-2, recv_buffer+2, recv_size);
			recv_is_encoded = false;
			if(recv_size) {
				// Add the reply header, host is always destination 0x00
				recv_buffer[0] = 0x00;
				recv_buffer[1] = recv_size+1;
				recv_size += 2;
			}
		} else
			recv_size = 0;
	}
	send_size = 0;
}

void jvs_host::commit_encoded()
{
	recv_size = 0;
	if(!send_size)
		return;
	decode(send_buffer, send_size);
	commit_raw();
}


void jvs_host::get_raw_reply(const UINT8 *&buffer, UINT32 &size)
{
	if(recv_is_encoded) {
		decode(recv_buffer, recv_size);
		recv_is_encoded = false;
	}
	buffer = recv_buffer;
	size = recv_size;
}

void jvs_host::get_encoded_reply(const UINT8 *&buffer, UINT32 &size)
{
	if(!recv_is_encoded) {
		encode(recv_buffer, recv_size);
		recv_is_encoded = true;
	}
	buffer = recv_buffer;
	size = recv_size;
}

bool jvs_host::get_presence_line()
{
	return first_device != 0;
}

bool jvs_host::get_address_set_line()
{
	return first_device && first_device->get_address_set_line();
}


void jvs_host::encode(UINT8 *buffer, UINT32 &size)
{
	if(!size)
		return;
	UINT32 add = 1;
	UINT8 sum = 0;
	for(UINT32 i=0; i<size; i++)
		sum += buffer[i];
	buffer[size++] = sum;
	for(UINT32 i=0; i<size; i++)
		if(buffer[i] == 0xd0 || buffer[i] == 0xe0)
			add++;
	for(UINT32 i=size; i; i--) {
		UINT8 t = buffer[i-1];
		if(t == 0xd0 || t == 0xe0) {
			buffer[i+add-1] = t-1;
			buffer[i+add-2] = 0xd0;
			add--;
		} else
			buffer[i+add-1] = t;
	}
	buffer[0] = 0xe0;
	size += add;
}

void jvs_host::decode(UINT8 *buffer, UINT32 &size)
{
	if(!size)
		return;
	UINT32 pos = 0;
	for(UINT32 i=0; i<size; i++) {
		UINT8 t = buffer[i];
		if(!i && t == 0xe0)
			continue;
		if(t == 0xd0) {
			i++;
			t = buffer[i]+1;
		}
		buffer[pos++] = t;
	}
	size = pos ? pos - 1 : 0;
}
