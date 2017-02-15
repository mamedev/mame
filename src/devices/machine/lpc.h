// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef LPC_H
#define LPC_H


class lpc_device : public device_t {
public:
	lpc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	virtual void map_device(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) = 0;
};

#endif
