// license:BSD-3-Clause
// copyright-holders:hap
/*

  Sharp SM511 MCU family cores

*/

#ifndef MAME_CPU_SM510_SM511_H
#define MAME_CPU_SM510_SM511_H

#pragma once

#include "sm510base.h"


// pinout reference

/*

SM511: identical to SM510 (see sm510.h)

SM512: can't be bothered to draw one here, it's 80 pins QFP, a lot of LCD segment pins

*/

class sm511_device : public sm510_base_device
{
public:
	sm511_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 32768);

protected:
	sm511_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int stack_levels, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);

	void program_4k(address_map &map) ATTR_COLD;
	void data_96_32x4(address_map &map) ATTR_COLD;

	virtual void device_post_load() override { notify_clock_changed(); }
	virtual void device_reset() override ATTR_COLD;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void execute_one() override;
	virtual bool op_argument() override;

	virtual void clock_melody() override;
	virtual void init_melody() override;
	virtual u16 melody_step_mask() { return 0x7f; }
};

class sm512_device : public sm511_device
{
public:
	sm512_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 32768);

protected:
	void data_80_48x4(address_map &map) ATTR_COLD;
};


DECLARE_DEVICE_TYPE(SM511, sm511_device)
DECLARE_DEVICE_TYPE(SM512, sm512_device)

#endif // MAME_CPU_SM510_SM511_H
