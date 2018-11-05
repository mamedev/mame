// license:BSD-3-Clause
// copyright-holders:smf
/*********************************************************************

    i8251.h

    Intel 8251 Universal Synchronous/Asynchronous Receiver Transmitter code

*********************************************************************/

#ifndef MAME_MACHINE_I8251_H
#define MAME_MACHINE_I8251_H

#pragma once

#include "diserial.h"

class i8251_device :  public device_t,
	public device_serial_interface
{
public:
	// construction/destruction
	i8251_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	template <class Object> devcb_base &set_txd_handler(Object &&cb) { return m_txd_handler.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_dtr_handler(Object &&cb) { return m_dtr_handler.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_rts_handler(Object &&cb) { return m_rts_handler.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_rxrdy_handler(Object &&cb) { return m_rxrdy_handler.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_txrdy_handler(Object &&cb) { return m_txrdy_handler.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_txempty_handler(Object &&cb) { return m_txempty_handler.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_syndet_handler(Object &&cb) { return m_syndet_handler.set_callback(std::forward<Object>(cb)); }
	auto txd_handler() { return m_txd_handler.bind(); }
	auto dtr_handler() { return m_dtr_handler.bind(); }
	auto rts_handler() { return m_rts_handler.bind(); }
	auto rxrdy_handler() { return m_rxrdy_handler.bind(); }
	auto txrdy_handler() { return m_txrdy_handler.bind(); }
	auto txempty_handler() { return m_txempty_handler.bind(); }
	auto syndet_handler() { return m_syndet_handler.bind(); }

	uint8_t data_r();
	void data_w(uint8_t data);
	uint8_t status_r();
	void control_w(uint8_t data);

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	DECLARE_WRITE_LINE_MEMBER( write_rxd );
	DECLARE_WRITE_LINE_MEMBER( write_cts );
	DECLARE_WRITE_LINE_MEMBER( write_dsr );
	DECLARE_WRITE_LINE_MEMBER( write_txc );
	DECLARE_WRITE_LINE_MEMBER( write_rxc );

	DECLARE_READ_LINE_MEMBER(txrdy_r);

	/// TODO: REMOVE THIS
	void receive_character(uint8_t ch);

	/// TODO: this shouldn't be public
	enum
	{
		I8251_STATUS_FRAMING_ERROR = 0x20,
		I8251_STATUS_OVERRUN_ERROR = 0x10,
		I8251_STATUS_PARITY_ERROR = 0x08,
		I8251_STATUS_TX_EMPTY = 0x04,
		I8251_STATUS_RX_READY = 0x02,
		I8251_STATUS_TX_READY = 0x01
	};

protected:
	i8251_device(
			const machine_config &mconfig,
			device_type type,
			const char *tag,
			device_t *owner,
			uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	void command_w(uint8_t data);
	void mode_w(uint8_t data);

	void update_rx_ready();
	void update_tx_ready();
	void update_tx_empty();
	void transmit_clock();
	void receive_clock();
	bool is_tx_enabled() const;
	void check_for_tx_start();
	void start_tx();


	enum
	{
		I8251_EXPECTING_MODE = 0x01,
		I8251_EXPECTING_SYNC_BYTE = 0x02,
				I8251_DELAYED_TX_EN = 0x04
	};

private:
	devcb_write_line m_txd_handler;
	devcb_write_line m_dtr_handler;
	devcb_write_line m_rts_handler;
	devcb_write_line m_rxrdy_handler;
	devcb_write_line m_txrdy_handler;
	devcb_write_line m_txempty_handler;
	devcb_write_line m_syndet_handler;

	/* flags controlling how i8251_control_w operates */
	uint8_t m_flags;
	/* offset into sync_bytes used during sync byte transfer */
	uint8_t m_sync_byte_offset;
	/* number of sync bytes written so far */
	uint8_t m_sync_byte_count;
	/* the sync bytes written */
	uint8_t m_sync_bytes[2];
	/* status of i8251 */
	uint8_t m_status;
	uint8_t m_command;
	/* mode byte - bit definitions depend on mode - e.g. synchronous, asynchronous */
	uint8_t m_mode_byte;

	int m_cts;
	int m_dsr;
	int m_rxd;
	int m_rxc;
	int m_txc;
	int m_rxc_count;
	int m_txc_count;
	int m_br_factor;

	/* data being received */
	uint8_t m_rx_data;
		/* tx buffer */
	uint8_t m_tx_data;
};

class v53_scu_device :  public i8251_device
{
public:
	// construction/destruction
	v53_scu_device(const machine_config &mconfig,  const char *tag, device_t *owner, uint32_t clock);

	void command_w(uint8_t data) { i8251_device::command_w(data); }
	void mode_w(uint8_t data) { i8251_device::mode_w(data); }
};



// device type definition
DECLARE_DEVICE_TYPE(I8251,   i8251_device)
DECLARE_DEVICE_TYPE(V53_SCU, v53_scu_device)

#endif // MAME_MACHINE_I8251_H
