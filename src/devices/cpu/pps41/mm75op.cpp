// license:BSD-3-Clause
// copyright-holders:hap

// MM75 opcode handlers

#include "emu.h"
#include "mm75.h"


// opcodes (differences with mm76_device)

void mm75_device::op_ios()
{
	// IOS: does not have serial I/O
	op_illegal();
}
