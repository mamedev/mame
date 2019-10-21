// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_MACHINE_68307TMU_H
#define MAME_MACHINE_68307TMU_H

#pragma once

#include "68307.h"


class m68307_cpu_device::m68307_timer
{
public:
	struct single_timer
	{
		uint16_t regs[0x8];
		bool enabled;
		emu_timer *mametimer;
	};


	single_timer singletimer[2];

	emu_timer *wd_mametimer;
	m68307_cpu_device *parent;

	void write_tmr(uint16_t data, uint16_t mem_mask, int which);
	void write_trr(uint16_t data, uint16_t mem_mask, int which);
	void write_ter(uint16_t data, uint16_t mem_mask, int which);
	uint16_t read_tcn(uint16_t mem_mask, int which);
	bool timer_int_pending(int which) const;

	void init(m68307_cpu_device *device);
	void reset();

	TIMER_CALLBACK_MEMBER(timer0_callback);
	TIMER_CALLBACK_MEMBER(timer1_callback);
	TIMER_CALLBACK_MEMBER(wd_timer_callback);
};

#endif // MAME_MACHINE_68307TMU_H
