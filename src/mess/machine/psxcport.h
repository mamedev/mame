#pragma once

#ifndef __PSXCPORT_H__
#define __PSXCPORT_H__

#include "cpu/psx/siodev.h"

extern const device_type PSXCONTROLLERPORTS;

struct pad_t
{
	UINT8 n_shiftin;
	UINT8 n_shiftout;
	int n_bits;
	int n_state;
	int n_byte;
	int b_lastclock;
	int b_ack;
};

class psxcontrollerports_device : public psxsiodev_device
{
public:
	psxcontrollerports_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	void device_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	void psx_pad( int n_port, int n_data );
	virtual void data_in( int data, int mask );

	pad_t m_pad[ 2 ];
	emu_timer *m_ack_timer;
};

#endif
