// license:BSD-3-Clause
// copyright-holders:smf
/*
 * PlayStation Serial I/O emulator
 *
 * Copyright 2003-2011 smf
 *
 */

#ifndef MAME_CPU_PSX_SIO_H
#define MAME_CPU_PSX_SIO_H

#pragma once


DECLARE_DEVICE_TYPE(PSX_SIO0, psxsio0_device)
DECLARE_DEVICE_TYPE(PSX_SIO1, psxsio1_device)

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
	// configuration helpers
	auto irq_handler() { return m_irq_handler.bind(); }
	auto sck_handler() { return m_sck_handler.bind(); }
	auto txd_handler() { return m_txd_handler.bind(); }
	auto dtr_handler() { return m_dtr_handler.bind(); }
	auto rts_handler() { return m_rts_handler.bind(); }

	void write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t read(offs_t offset, uint32_t mem_mask = ~0);

	void write_rxd(int state);
	void write_dsr(int state);

protected:
	psxsio_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_post_load() override;

	TIMER_CALLBACK_MEMBER( sio_tick );

private:
	void sio_interrupt();
	void sio_timer_adjust();

	uint32_t m_status;
	uint32_t m_mode;
	uint32_t m_control;
	uint32_t m_baud;
	int m_rxd;
	uint32_t m_tx_data;
	uint32_t m_rx_data;
	uint32_t m_tx_shift;
	uint32_t m_rx_shift;
	uint32_t m_tx_bits;
	uint32_t m_rx_bits;

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
	psxsio0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class psxsio1_device : public psxsio_device
{
public:
	psxsio1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

#endif // MAME_CPU_PSX_SIO_H
