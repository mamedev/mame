// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef LPC_H
#define LPC_H

#include "emu.h"

class lpc_device : public device_t {
public:
	lpc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	virtual void map_device(UINT64 memory_window_start, UINT64 memory_window_end, UINT64 memory_offset, address_space *memory_space,
							UINT64 io_window_start, UINT64 io_window_end, UINT64 io_offset, address_space *io_space) = 0;
};

#endif
