// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_SEGA_MAPLEDEV_H
#define MAME_SEGA_MAPLEDEV_H


#include "maple-dc.h"

class maple_device : public device_t
{
public:
	void set_host_port(int _host_port) { host_port = _host_port; }

	virtual void maple_w(const uint32_t *data, uint32_t in_size) = 0;
	void maple_r(uint32_t *data, uint32_t &out_size, bool &partial);
	virtual void maple_reset();

protected:
	maple_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	uint32_t reply_size = 0;
	bool reply_partial = false;
	uint32_t reply_buffer[256]{};

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(reply_ready);
	void reply_ready_with_delay();

	// Copy a string and complete it with spaces up to size len
	void copy_with_spaces(uint8_t *dest, const char *source, int len);

	// Setup the first uint32_t of reply with the type, source and length.
	// Also setup reply_size and clear reply_partial
	void reply_start(uint8_t code, uint8_t source, uint8_t size);

	// Configuration
	required_device<maple_dc_device> host;
	int host_port = 0;

private:
	emu_timer *timer = nullptr;
};

#endif // MAME_SEGA_MAPLEDEV_H
