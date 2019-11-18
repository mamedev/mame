// license:BSD-3-Clause
// copyright-holders:David Haywood

// device interface to hold some common functions of the EFO / Cedar Magnet System PCBs
#ifndef MAME_MACHINE_CEDAR_MAGNET_BOARD_H
#define MAME_MACHINE_CEDAR_MAGNET_BOARD_H

#pragma once


#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"

class cedar_magnet_board_interface : public device_interface
{
public:
	// construction/destruction
	cedar_magnet_board_interface(const machine_config &mconfig, device_t &device, const char *cputag, const char *ramtag);

	z80_device &cpu() { assert(m_cpu != nullptr); return *m_cpu; }
	u8 *ram() { return &m_ram[0]; }

	virtual u8 read_cpu_bus(int offset);
	virtual void write_cpu_bus(int offset, u8 data);

	void irq_hold();
	void halt_assert();
	void halt_clear();
	void reset_assert();
	void reset_clear();
	bool is_running() const;

protected:
	virtual void interface_pre_reset() override;
	virtual void interface_pre_start() override;

	virtual TIMER_CALLBACK_MEMBER(reset_assert_callback);

	required_device<z80_device> m_cpu;
	optional_shared_ptr<u8> m_ram;

private:
	TIMER_CALLBACK_MEMBER(halt_assert_callback);
	TIMER_CALLBACK_MEMBER(halt_clear_callback);
	TIMER_CALLBACK_MEMBER(reset_clear_callback);

	bool m_is_running;
	emu_timer *m_halt_assert_timer;
	emu_timer *m_halt_clear_timer;
	emu_timer *m_reset_assert_timer;
	emu_timer *m_reset_clear_timer;
};

#endif // MAME_MACHINE_CEDAR_MAGNET_BOARD_H
