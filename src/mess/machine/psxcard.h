#pragma once

#ifndef _PSXCARD_
#define _PSXCARD_

#include "emu.h"

class psx_controller_port_device;

#define MCFG_PSXCARD_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, PSXCARD, 0)

class psxcard_device : public device_t
{
public:
	psxcard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

private:
	unsigned char pkt[0x8b], pkt_ptr, pkt_sz, cmd, *cache;
	unsigned short addr;
	int state;

	UINT8 m_odata;
	UINT8 m_idata;
	int m_bit;
	int m_count;
	bool m_pad;

	bool m_clock;
	bool m_sel;
	bool m_ack;
	bool m_rx;

	emu_timer *m_ack_timer;
	psx_controller_port_device *m_owner;

	void read_card(const unsigned short addr, unsigned char *buf);
	void write_card(const unsigned short addr, unsigned char *buf);
	unsigned char checksum_data(const unsigned char *buf, const unsigned int sz);
	void do_card();
	bool transfer(UINT8 to, UINT8 *from);
	void ack_timer(void *ptr, int param);

public:
	virtual void device_start();
	virtual void device_reset();

	void clock_w(bool state) { if(m_clock && !m_sel && !state && !m_pad) do_card(); m_clock = state; }
	void sel_w(bool state);
	bool rx_r() { return m_rx; }
	bool ack_r() { return m_ack; }
};

// device type definition
extern const device_type PSXCARD;

#endif
