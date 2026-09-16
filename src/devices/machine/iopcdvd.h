// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony PlayStation 2 disc controller device skeleton
*
*   To Do:
*     Everything
*
*/

#ifndef MAME_MACHINE_IOPCDVD_H
#define MAME_MACHINE_IOPCDVD_H

#pragma once

#include "iopintc.h"

class iop_cdvd_device : public device_t
{
public:
	template <typename T>
	iop_cdvd_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&intc_tag)
		: iop_cdvd_device(mconfig, tag, owner, (uint32_t)0)
	{
		m_intc.set_tag(std::forward<T>(intc_tag));
	}

	iop_cdvd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~iop_cdvd_device() override;

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void handle_data_command(uint8_t data);
	void data_fifo_push(uint8_t data);
	uint8_t data_fifo_pop();

	enum
	{
		CDVD_STATUS_IDLE = 0x40,
	};

	required_device<iop_intc_device> m_intc;

	enum
	{
		CHAN_SERVO = 0,
		CHAN_DATA = 1
	};

	struct drive_channel_t
	{
		uint8_t m_buffer[0x10]; // Buffer size is a total guess
		uint8_t m_curr;
		uint8_t m_end;

		uint8_t m_status;
		uint8_t m_command;
	};

	drive_channel_t m_channel[2];

	static const size_t BUFFER_SIZE;
};

DECLARE_DEVICE_TYPE(SONYIOP_CDVD, iop_cdvd_device)

#endif // MAME_MACHINE_IOPCDVD_H
