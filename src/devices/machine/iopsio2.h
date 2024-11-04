// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony PlayStation 2 IOP SIO2 device skeleton
*
*   To Do:
*     Everything
*
*/

#ifndef MAME_MACHINE_IOPSIO2_H
#define MAME_MACHINE_IOPSIO2_H

#pragma once

#include "iopintc.h"
#include "ps2pad.h"
#include "ps2mc.h"

class iop_sio2_device : public device_t
{
public:
	template <typename T, typename U, typename V, typename W>
	iop_sio2_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&intc_tag, U &&pad0_tag, V &&pad1_tag, W &&mc0_tag)
		: iop_sio2_device(mconfig, tag, owner, (uint32_t)0)
	{
		m_intc.set_tag(std::forward<T>(intc_tag));
		m_pad0.set_tag(std::forward<U>(pad0_tag));
		m_pad1.set_tag(std::forward<V>(pad1_tag));
		m_mc0.set_tag(std::forward<W>(mc0_tag));
	}

	iop_sio2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~iop_sio2_device() override;

	uint32_t read(offs_t offset, uint32_t mem_mask = ~0);
	void write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void transmit(uint8_t data);
	uint8_t receive();

	void receive_from_device_hack(uint8_t data); // TODO: Turn me into a bus interface!

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(response_timer);

	void select_device_hack(uint8_t data);
	void transmit_to_device_hack(uint8_t data); // TODO: Turn me into a bus interface!
	ps2_pad_device* pad(uint8_t port);

	enum : uint32_t
	{
		CTRL_IRQ = 0x01,
	};

	required_device<iop_intc_device> m_intc;
	required_device<ps2_pad_device> m_pad0;
	required_device<ps2_pad_device> m_pad1;
	required_device<ps2_mc_device> m_mc0;

	// HACK: Buffer size is a guess.
	uint8_t m_buffer[512];
	uint32_t m_curr_byte;
	uint32_t m_end_byte;
	uint32_t m_ctrl;

	uint32_t m_unknown_0x6c; // Device status?
	uint32_t m_unknown_0x70;
	uint32_t m_unknown_0x74;

	uint32_t m_cmdbuf[16];
	uint32_t m_curr_port;
	uint32_t m_cmd_size;
	uint32_t m_cmd_length;
	uint32_t m_databuf[4][2];

	uint32_t m_target_device;

	emu_timer *m_response_timer;

	static const size_t BUFFER_SIZE;
};

DECLARE_DEVICE_TYPE(SONYIOP_SIO2, iop_sio2_device)

#endif // MAME_MACHINE_IOPSIO2_H
