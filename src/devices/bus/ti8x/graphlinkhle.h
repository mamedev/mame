// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
 High-level emulation of a somewhat idealised TI-Graph Link "grey"
 model.  Translates byte-oriented TI-8x link port protocol to/from
 9600 Baud RS232.  A real TI-Graph Link requires some delay between
 bytes.

 The buffer is there so that if you connect two emulated calculators
 together with these it has some chance of working.  The receiving
 calculator can't slow the sending calculator down like it would be able
 to in real life, so you inevitably get overruns without the buffer.
 */
#ifndef MAME_BUS_TI8X_GRAPHLINKHLE_H
#define MAME_BUS_TI8X_GRAPHLINKHLE_H

#pragma once

#include "ti8x.h"

#include "bus/rs232/rs232.h"

#include "diserial.h"

#include <memory>


namespace bus { namespace ti8x {

class graph_link_hle_device
		: public device_t
		, public device_ti8x_link_port_byte_interface
		, public device_serial_interface
{
public:
	graph_link_hle_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void byte_collision() override;
	virtual void byte_send_timeout() override;
	virtual void byte_receive_timeout() override;
	virtual void byte_sent() override;
	virtual void byte_received(u8 data) override;

	virtual void rcv_complete() override;
	virtual void tra_callback() override;
	virtual void tra_complete() override;

private:
	static constexpr unsigned BUFLEN = 1U << 16;

	required_device<rs232_port_device>  m_serial_port;
	std::unique_ptr<u8 []>              m_buffer;
	unsigned                            m_head, m_tail;
	bool                                m_empty, m_ready;
};

} } // namespace bus::ti8x


DECLARE_DEVICE_TYPE_NS(TI8X_GRAPH_LINK_HLE, bus::ti8x, graph_link_hle_device)

#endif // MAME_BUS_TI8X_GRAPHLINKHLE_H
