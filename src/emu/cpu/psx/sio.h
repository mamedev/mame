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

extern const device_type PSX_SIO0;
extern const device_type PSX_SIO1;

typedef void ( *psx_sio_handler )( running_machine &, int );

#define MCFG_PSX_SIO_IRQ_HANDLER(_devcb) \
	devcb = &psxsio_device::set_irq_handler(*device, DEVCB2_##_devcb); \

#define PSX_SIO_OUT_DATA ( 1 )	/* COMMAND */
#define PSX_SIO_OUT_DTR ( 2 )	/* ATT */
#define PSX_SIO_OUT_RTS ( 4 )
#define PSX_SIO_OUT_CLOCK ( 8 )	/* CLOCK */
#define PSX_SIO_IN_DATA ( 1 )	/* DATA */
#define PSX_SIO_IN_DSR ( 2 )	/* ACK */
#define PSX_SIO_IN_CTS ( 4 )

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
	psxsio_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb2_base &set_irq_handler(device_t &device, _Object object) { return downcast<psxsio_device &>(device).m_irq_handler.set_callback(object); }

	void install_handler( psx_sio_handler p_f_sio_handler );

	DECLARE_WRITE32_MEMBER( write );
	DECLARE_READ32_MEMBER( read );

	void input( int n_mask, int n_data );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual void device_post_load();

private:
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
	psx_sio_handler m_fn_handler;

	devcb2_write_line m_irq_handler;
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
