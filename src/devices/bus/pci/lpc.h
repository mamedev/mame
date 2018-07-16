// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_MACHINE_LPC_H
#define MAME_MACHINE_LPC_H

#pragma once


class lpc_device : public device_t {
public:
	virtual void map_device(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) = 0;

protected:
	using device_t::device_t;
};

#endif // MAME_MACHINE_LPC_H
