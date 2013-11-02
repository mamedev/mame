// license:MAME
// copyright-holders:smf
/*
 * PlayStation Serial I/O emulator
 *
 * Copyright 2003-2011 smf
 *
 */

#pragma once

#ifndef __PSXSIO_H__
#define __PSXSIO_H__

#include "emu.h"
#include "siodev.h"

extern const device_type PSX_SIO0;
extern const device_type PSX_SIO1;

#define MCFG_PSX_SIO_IRQ_HANDLER(_devcb) \
	devcb = &psxsio_device::set_irq_handler(*device, DEVCB2_##_devcb);
#define SIO_BUF_SIZE ( 8 )

#define SIO_STATUS_TX_RDY ( 1 << 0 )
#define SIO_STATUS_RX_RDY ( 1 << 1 )
#define SIO_STATUS_TX_EMPTY ( 1 << 2 )
#define SIO_STATUS_OVERRUN ( 1 << 4 )
#define SIO_STATUS_DSR ( 1 << 7 )
#define SIO_STATUS_IRQ ( 1 << 9 )

#define SIO_CONTROL_TX_ENA ( 1 << 0 )
#define SIO_CONTROL_IACK ( 1 << 4 )
#define SIO_CONTROL_RESET ( 1 << 6 )
#define SIO_CONTROL_TX_IENA ( 1 << 10 )
#define SIO_CONTROL_RX_IENA ( 1 << 11 )
#define SIO_CONTROL_DSR_IENA ( 1 << 12 )
#define SIO_CONTROL_DTR ( 1 << 13 )

class psxsio_device : public device_t
{
public:
	psxsio_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// static configuration helpers
	template<class _Object> static devcb2_base &set_irq_handler(device_t &device, _Object object) { return downcast<psxsio_device &>(device).m_irq_handler.set_callback(object); }

	DECLARE_WRITE32_MEMBER( write );
	DECLARE_READ32_MEMBER( read );

	void input_update();

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual void device_post_load();

private:
	void output( int data, int mask );
	void sio_interrupt();
	void sio_timer_adjust();

	UINT32 m_status;
	UINT32 m_mode;
	UINT32 m_control;
	UINT32 m_baud;
	UINT32 m_tx;
	UINT32 m_rx;
	UINT32 m_tx_prev;
	UINT32 m_rx_prev;
	UINT32 m_tx_data;
	UINT32 m_rx_data;
	UINT32 m_tx_shift;
	UINT32 m_rx_shift;
	UINT32 m_tx_bits;
	UINT32 m_rx_bits;

	emu_timer *m_timer;

	devcb2_write_line m_irq_handler;

	psxsiodev_device *devices[ 10 ];
	int deviceCount;

	int m_outputdata;
	//int m_inputdata;
};

class psxsio0_device : public psxsio_device
{
public:
	psxsio0_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class psxsio1_device : public psxsio_device
{
public:
	psxsio1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

#endif
