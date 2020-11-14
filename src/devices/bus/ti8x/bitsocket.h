// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
 Sends raw assert/release signals over a socket.  Seriously limits
 transfer rates and probably won't work if there's much latency, but it
 allows communication between instances using non-standard protocols.

 bit 0 = data
 bit 1 = set tip
 bit 2 = set ring
 */
#ifndef MAME_BUS_TI8X_BITSOCKET_H
#define MAME_BUS_TI8X_BITSOCKET_H

#pragma once

#include "ti8x.h"

#include "imagedev/bitbngr.h"


namespace bus::ti8x {

class bit_socket_device
		: public device_t
		, public device_ti8x_link_port_interface
{
public:
	bit_socket_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

protected:
	enum
	{
		TIMER_ID_POLL = 1
	};

	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	virtual DECLARE_WRITE_LINE_MEMBER(input_tip) override;
	virtual DECLARE_WRITE_LINE_MEMBER(input_ring) override;

private:
	required_device<bitbanger_device>   m_stream;
	emu_timer *                         m_poll_timer;
	bool                                m_tip_in, m_ring_in;
};

} // namespace bus::ti8x


DECLARE_DEVICE_TYPE_NS(TI8X_BIT_SOCKET, bus::ti8x, bit_socket_device)

#endif // MAME_BUS_TI8X_BITSOCKET_H
