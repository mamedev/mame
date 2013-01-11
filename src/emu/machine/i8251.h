/*********************************************************************

    i8251.h

    Intel 8251 Universal Synchronous/Asynchronous Receiver Transmitter code

*********************************************************************/

#ifndef __I8251_H__
#define __I8251_H__


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define I8251_EXPECTING_MODE        0x01
#define I8251_EXPECTING_SYNC_BYTE   0x02

#define I8251_STATUS_FRAMING_ERROR  0x20
#define I8251_STATUS_OVERRUN_ERROR  0x10
#define I8251_STATUS_PARITY_ERROR   0x08
#define I8251_STATUS_TX_EMPTY       0x04
#define I8251_STATUS_RX_READY       0x02
#define I8251_STATUS_TX_READY       0x01

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_I8251_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, I8251, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_I8251_REMOVE(_tag) \
	MCFG_DEVICE_REMOVE(_tag)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/
// ======================> i8251_interface

struct i8251_interface
{
	devcb_read_line     m_in_rxd_cb;
	devcb_write_line    m_out_txd_cb;
	devcb_read_line     m_in_dsr_cb;
	devcb_write_line    m_out_dtr_cb;
	devcb_write_line    m_out_rts_cb;
	devcb_write_line    m_out_rxrdy_cb;
	devcb_write_line    m_out_txrdy_cb;
	devcb_write_line    m_out_txempty_cb;
	devcb_write_line    m_out_syndet_cb;
};

// ======================> i8251_device

class i8251_device :  public device_t,
						public device_serial_interface,
						public i8251_interface
{
public:
	// construction/destruction
	i8251_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	/* read data register */
	DECLARE_READ8_MEMBER(data_r);

	/* read status register */
	DECLARE_READ8_MEMBER(status_r);

	/* write data register */
	DECLARE_WRITE8_MEMBER(data_w);

	/* write control word */
	DECLARE_WRITE8_MEMBER(control_w);

	/* The 8251 has seperate transmit and receive clocks */
	/* use these two functions to update the i8251 for each clock */
	/* on NC100 system, the clocks are the same */
	void transmit_clock();
	void receive_clock();

	void receive_character(UINT8 ch);

	virtual void input_callback(UINT8 state);
protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete();
	virtual void device_reset();

	void update_rx_ready();
	void update_tx_ready();
	void update_tx_empty();
private:
	devcb_resolved_read_line    m_in_rxd_func;
	devcb_resolved_write_line   m_out_txd_func;
	devcb_resolved_read_line    m_in_dsr_func;
	devcb_resolved_write_line   m_out_dtr_func;
	devcb_resolved_write_line   m_out_rts_func;
	devcb_resolved_write_line   m_out_rxrdy_func;
	devcb_resolved_write_line   m_out_txrdy_func;
	devcb_resolved_write_line   m_out_txempty_func;
	devcb_resolved_write_line   m_out_syndet_func;

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

	/* data being received */
	UINT8 m_data;
};

// device type definition
extern const device_type I8251;

extern const i8251_interface default_i8251_interface;

#endif /* __I8251_H__ */
