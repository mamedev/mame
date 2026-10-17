// license:BSD-3-Clause
// copyright-holders:Niko Nieminen
/***************************************************************************

    RS-232 bit-level cable devices

    Unlike the "null_modem" device (a byte-oriented terminal that frames
    characters with its own data bits/parity/stop bit settings), these devices
    carry the actual line levels, so word length, parity, stop bits, baud rate,
    break and framing errors all pass through exactly as the attached UART
    produces them.  No software or configuration change is needed on the
    emulated machine.

    null_cable   - a null-modem cable between two emulated machines
                   (TxD<->RxD, RTS->CTS, DTR->DSR+DCD), carried over a
                   bitbanger socket: -rs232a null_cable -bitb socket.127.0.0.1:5000
    serial_link  - raw port: sends TxD/RTS/DTR as they are and takes RxD/CTS/
                   DSR/DCD from the peer.  The peer is a host-side bridge
                   (sarjasilta.py) connected to a real USB serial port.

    Wire protocol (all records 10 bytes): type, value, 64-bit little endian
    timestamp in nanoseconds of the sender's clock.  Heartbeat records
    (type 0) are sent every 2 ms so that the receiver can learn the clock
    offset and replay the edges with a fixed latency.

***************************************************************************/

#ifndef MAME_BUS_RS232_CABLE_H
#define MAME_BUS_RS232_CABLE_H

#pragma once

#include "rs232.h"
#include "imagedev/bitbngr.h"

#include <deque>
#include <vector>


class rs232_cable_device : public device_t, public device_rs232_port_interface
{
public:
	virtual void input_txd(int state) override;
	virtual void input_rts(int state) override;
	virtual void input_dtr(int state) override;

protected:
	rs232_cable_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool crossed);

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	struct event
	{
		int64_t time;
		uint8_t type;
		uint8_t value;
	};

	TIMER_CALLBACK_MEMBER(poll);
	TIMER_CALLBACK_MEMBER(play);

	int64_t now_ns() const;
	void send(uint8_t type, int value);
	void parse();
	void schedule(int64_t remote_time, uint8_t type, uint8_t value);
	void apply(uint8_t type, uint8_t value);
	void arm_play();

	required_device<bitbanger_device> m_stream;
	required_ioport m_latency;

	emu_timer *m_poll;
	emu_timer *m_play;
	bool const m_crossed;

	std::vector<uint8_t> m_rxbuf;
	std::deque<event> m_events;
	int64_t m_offset;        // local time - remote time (ns), applied to new edges
	int64_t m_offset_est;    // running worst-case estimate, adopted while the line is idle
	int64_t m_last_play;     // play time of the last scheduled event
	bool m_synced;
	uint32_t m_tick;
	uint32_t m_late;
	int m_txd;
	int m_rts;
	int m_dtr;
};

class null_cable_device : public rs232_cable_device
{
public:
	null_cable_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class serial_link_device : public rs232_cable_device
{
public:
	serial_link_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(NULL_CABLE, null_cable_device)
DECLARE_DEVICE_TYPE(SERIAL_LINK, serial_link_device)

#endif // MAME_BUS_RS232_CABLE_H
