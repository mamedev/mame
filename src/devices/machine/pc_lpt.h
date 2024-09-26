// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    IBM-PC printer interface

***************************************************************************/

#ifndef MAME_MACHINE_PC_LPT_H
#define MAME_MACHINE_PC_LPT_H

#pragma once

#include "bus/centronics/ctronics.h"


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

class pc_lpt_device : public device_t
{
public:
	pc_lpt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// configuration helpers
	auto irq_handler() { return m_irq_handler.bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	uint8_t data_r();
	void data_w(uint8_t data);
	uint8_t status_r();
	uint8_t control_r( );
	void control_w(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void update_irq();
	void write_irq_enabled(int state);
	void write_centronics_ack(int state);

	enum
	{
		CONTROL_STROBE = 1,
		CONTROL_AUTOFD = 2,
		CONTROL_INIT = 4,
		CONTROL_SELECT = 8,
		CONTROL_IRQ_ENABLED = 16,
		CONTROL_OUTPUT_ENABLED = 32
	};

	enum
	{
		STATUS_FAULT = 8,
		STATUS_SELECT = 16,
		STATUS_PERROR = 32,
		STATUS_ACK = 64,
		STATUS_BUSY = 128
	};

	// internal state

	int m_irq;
	uint8_t m_data;
	uint8_t m_control;
	int m_irq_enabled;
	int m_centronics_ack;

	devcb_write_line m_irq_handler;
	required_device<input_buffer_device> m_cent_data_in;
	required_device<output_latch_device> m_cent_data_out;
	required_device<input_buffer_device> m_cent_status_in;
	required_device<input_buffer_device> m_cent_ctrl_in;
	required_device<output_latch_device> m_cent_ctrl_out;
};

DECLARE_DEVICE_TYPE(PC_LPT, pc_lpt_device)

#endif // MAME_MACHINE_PC_LPT_H
