// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony Playstation 2 SIO2 device skeleton
*
*   To Do:
*     Everything
*
*/

#ifndef MAME_MACHINE_IOPSIO2_H
#define MAME_MACHINE_IOPSIO2_H

#pragma once

#include "emu.h"
#include "iopintc.h"

class iop_sio2_device : public device_t
{
public:
	template <typename T>
    iop_sio2_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&intc_tag)
    	: iop_sio2_device(mconfig, tag, owner, (uint32_t)0)
    {
		m_intc.set_tag(std::forward<T>(intc_tag));
	}

    iop_sio2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ32_MEMBER(read);
	DECLARE_WRITE32_MEMBER(write);

	void receive(uint8_t data);
	uint8_t transmit();

protected:
    virtual void device_start() override;
    virtual void device_reset() override;

	enum
	{
		CTRL_IRQ = 0x01,
	};

	required_device<iop_intc_device> m_intc;

	// HACK: Buffer sizes are guesses.
	uint8_t m_receive_buf[64];
	uint8_t m_receive_curr;
	uint8_t m_receive_end;
	uint8_t m_transmit_buf[64];
	uint8_t m_transmit_curr;
	uint8_t m_transmit_end;
	uint32_t m_ctrl;

	static const size_t BUFFER_SIZE;
};

DECLARE_DEVICE_TYPE(SONYIOP_SIO2, iop_sio2_device)

#endif // MAME_MACHINE_IOPSIO2_H