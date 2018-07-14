// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony Playstation 2 disc controller device skeleton
*
*   To Do:
*     Everything
*
*/

#ifndef MAME_MACHINE_IOPCDVD_H
#define MAME_MACHINE_IOPCDVD_H

#pragma once

#include "emu.h"
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

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

protected:
    virtual void device_start() override;
    virtual void device_reset() override;

	enum
	{
		CDVD_STATUS_BOOT = 0x08,
		CDVD_STATUS_IDLE = 0x40,

		CDVD_COMMAND_0x15 = 0x15,
	};

	required_device<iop_intc_device> m_intc;
	uint8_t m_status_0x05;
	uint8_t m_status_0x17;
	uint8_t m_command;
	std::unique_ptr<uint8_t[]> m_out_buf;
	uint8_t m_out_count;
	uint8_t m_out_curr;

	static const size_t BUFFER_SIZE;
};

DECLARE_DEVICE_TYPE(SONYIOP_CDVD, iop_cdvd_device)

#endif // MAME_MACHINE_IOPCDVD_H