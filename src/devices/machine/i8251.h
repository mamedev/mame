// license:BSD-3-Clause
// copyright-holders:smf
/*********************************************************************

    i8251.h

    Intel 8251 Universal Synchronous/Asynchronous Receiver Transmitter code

*********************************************************************/

#ifndef __I8251_H__
#define __I8251_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_I8251_TXD_HANDLER(_devcb) \
	devcb = &i8251_device::set_txd_handler(*device, DEVCB_##_devcb);

#define MCFG_I8251_DTR_HANDLER(_devcb) \
	devcb = &i8251_device::set_dtr_handler(*device, DEVCB_##_devcb);

#define MCFG_I8251_RTS_HANDLER(_devcb) \
	devcb = &i8251_device::set_rts_handler(*device, DEVCB_##_devcb);

#define MCFG_I8251_RXRDY_HANDLER(_devcb) \
	devcb = &i8251_device::set_rxrdy_handler(*device, DEVCB_##_devcb);

#define MCFG_I8251_TXRDY_HANDLER(_devcb) \
	devcb = &i8251_device::set_txrdy_handler(*device, DEVCB_##_devcb);

#define MCFG_I8251_TXEMPTY_HANDLER(_devcb) \
	devcb = &i8251_device::set_txempty_handler(*device, DEVCB_##_devcb);

#define MCFG_I8251_SYNDET_HANDLER(_devcb) \
	devcb = &i8251_device::set_syndet_handler(*device, DEVCB_##_devcb);

class i8251_device :  public device_t,
	public device_serial_interface
{
public:
	// construction/destruction
	i8251_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname);
	i8251_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_txd_handler(device_t &device, _Object object) { return downcast<i8251_device &>(device).m_txd_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_dtr_handler(device_t &device, _Object object) { return downcast<i8251_device &>(device).m_dtr_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_rts_handler(device_t &device, _Object object) { return downcast<i8251_device &>(device).m_rts_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_rxrdy_handler(device_t &device, _Object object) { return downcast<i8251_device &>(device).m_rxrdy_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_txrdy_handler(device_t &device, _Object object) { return downcast<i8251_device &>(device).m_txrdy_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_txempty_handler(device_t &device, _Object object) { return downcast<i8251_device &>(device).m_txempty_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_syndet_handler(device_t &device, _Object object) { return downcast<i8251_device &>(device).m_syndet_handler.set_callback(object); }

	DECLARE_READ8_MEMBER(data_r);
	DECLARE_WRITE8_MEMBER(data_w);
	DECLARE_READ8_MEMBER(status_r);
	DECLARE_WRITE8_MEMBER(control_w);
	DECLARE_WRITE8_MEMBER(command_w);
	DECLARE_WRITE8_MEMBER(mode_w);

	DECLARE_WRITE_LINE_MEMBER( write_rxd );
	DECLARE_WRITE_LINE_MEMBER( write_cts );
	DECLARE_WRITE_LINE_MEMBER( write_dsr );
	DECLARE_WRITE_LINE_MEMBER( write_txc );
	DECLARE_WRITE_LINE_MEMBER( write_rxc );

	/// TODO: REMOVE THIS
	void receive_character(UINT8 ch);

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
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	void update_rx_ready();
	void update_tx_ready();
	void update_tx_empty();
	void transmit_clock();
	void receive_clock();
        bool is_tx_enabled(void) const;
        void check_for_tx_start(void);
        void start_tx(void);


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
	UINT8 m_flags;
	/* offset into sync_bytes used during sync byte transfer */
	UINT8 m_sync_byte_offset;
	/* number of sync bytes written so far */
	UINT8 m_sync_byte_count;
	/* the sync bytes written */
	UINT8 m_sync_bytes[2];
	/* status of i8251 */
	UINT8 m_status;
	UINT8 m_command;
	/* mode byte - bit definitions depend on mode - e.g. synchronous, asynchronous */
	UINT8 m_mode_byte;

	int m_cts;
	int m_dsr;
	int m_rxd;
	int m_rxc;
	int m_txc;
	int m_rxc_count;
	int m_txc_count;
	int m_br_factor;

	/* data being received */
	UINT8 m_rx_data;
        /* tx buffer */
	UINT8 m_tx_data;
};

class v53_scu_device :  public i8251_device
{
public:
	// construction/destruction
	v53_scu_device(const machine_config &mconfig,  const char *tag, device_t *owner, UINT32 clock);
};



// device type definition
extern const device_type I8251;
extern const device_type V53_SCU;


#endif /* __I8251_H__ */
