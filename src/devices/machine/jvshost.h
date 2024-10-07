// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_MACHINE_JVSHOST_H
#define MAME_MACHINE_JVSHOST_H

#pragma once


class jvs_device;

class jvs_host : public device_t {
public:
	void add_device(jvs_device *dev);

protected:
	jvs_host(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void push(uint8_t val);
	void commit_raw();
	void commit_encoded();

	void get_raw_reply(const uint8_t *&buffer, uint32_t &size);
	void get_encoded_reply(const uint8_t *&buffer, uint32_t &size);

	bool get_presence_line();
	bool get_address_set_line();

private:
	enum { BUFFER_SIZE = 512 };

	jvs_device *first_device;

	uint32_t send_size, recv_size;
	uint8_t send_buffer[BUFFER_SIZE];
	uint8_t recv_buffer[BUFFER_SIZE];

	bool recv_is_encoded;

	void encode(uint8_t *buffer, uint32_t &size);
	void decode(uint8_t *buffer, uint32_t &size);
};

#endif // MAME_MACHINE_JVSHOST_H
