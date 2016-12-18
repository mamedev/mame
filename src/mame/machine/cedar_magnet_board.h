// license:BSD-3-Clause
// copyright-holders:David Haywood

// just a base device to hold some common functions of the EFO / Cedar Magnet System PCBs

#pragma once

#ifndef CEDAR_MAGNET_BOARD_DEF
#define CEDAR_MAGNET_BOARD_DEF

#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80pio.h"

extern const device_type CEDAR_MAGNET_PLANE;

class cedar_magnet_board_device :  public device_t
{
public:
	// construction/destruction
	cedar_magnet_board_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	uint8_t* m_ram;
	z80_device* m_cpu;

	virtual uint8_t read_cpu_bus(int offset);
	virtual void write_cpu_bus(int offset, uint8_t data);

	TIMER_CALLBACK_MEMBER(halt_assert_callback);
	TIMER_CALLBACK_MEMBER(halt_clear_callback);
	virtual TIMER_CALLBACK_MEMBER(reset_assert_callback);
	TIMER_CALLBACK_MEMBER(reset_clear_callback);


	void halt_assert(void);
	void halt_clear(void);
	void reset_assert(void);
	void reset_clear(void);
	bool is_running(void);
	bool m_is_running;

	INTERRUPT_GEN_MEMBER(irq);
protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
};

#endif
