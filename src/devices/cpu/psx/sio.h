// license:BSD-3-Clause
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

extern const device_type PSX_SIO0;
extern const device_type PSX_SIO1;

#define MCFG_PSX_SIO_IRQ_HANDLER(_devcb) \
	devcb = &psxsio_device::set_irq_handler(*device, DEVCB_##_devcb);

#define MCFG_PSX_SIO_SCK_HANDLER(_devcb) \
	devcb = &psxsio_device::set_sck_handler(*device, DEVCB_##_devcb);

#define MCFG_PSX_SIO_TXD_HANDLER(_devcb) \
	devcb = &psxsio_device::set_txd_handler(*device, DEVCB_##_devcb);

#define MCFG_PSX_SIO_DTR_HANDLER(_devcb) \
	devcb = &psxsio_device::set_dtr_handler(*device, DEVCB_##_devcb);

#define MCFG_PSX_SIO_RTS_HANDLER(_devcb) \
	devcb = &psxsio_device::set_rts_handler(*device, DEVCB_##_devcb);

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
	template<class _Object> static devcb_base &set_irq_handler(device_t &device, _Object object) { return downcast<psxsio_device &>(device).m_irq_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_sck_handler(device_t &device, _Object object) { return downcast<psxsio_device &>(device).m_sck_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_txd_handler(device_t &device, _Object object) { return downcast<psxsio_device &>(device).m_txd_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_dtr_handler(device_t &device, _Object object) { return downcast<psxsio_device &>(device).m_dtr_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_rts_handler(device_t &device, _Object object) { return downcast<psxsio_device &>(device).m_rts_handler.set_callback(object); }

	DECLARE_WRITE32_MEMBER( write );
	DECLARE_READ32_MEMBER( read );

	DECLARE_WRITE_LINE_MEMBER(write_rxd);
	DECLARE_WRITE_LINE_MEMBER(write_dsr);
	DECLARE_WRITE_LINE_MEMBER(write_cts);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void device_post_load() override;

private:
	void sio_interrupt();
	void sio_timer_adjust();

	UINT32 m_status;
	UINT32 m_mode;
	UINT32 m_control;
	UINT32 m_baud;
	int m_rxd;
	UINT32 m_tx_data;
	UINT32 m_rx_data;
	UINT32 m_tx_shift;
	UINT32 m_rx_shift;
	UINT32 m_tx_bits;
	UINT32 m_rx_bits;

	emu_timer *m_timer;

	devcb_write_line m_irq_handler;
	devcb_write_line m_sck_handler;
	devcb_write_line m_txd_handler;
	devcb_write_line m_dtr_handler;
	devcb_write_line m_rts_handler;
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
