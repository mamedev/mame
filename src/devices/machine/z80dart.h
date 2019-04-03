// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Intel 8274 Multi-Protocol Serial Controller emulation
    NEC uPD7201 Multiprotocol Serial Communications Controller emulation
    Z80-DART Dual Asynchronous Receiver/Transmitter emulation
    Z80-SIO/0/1/2/3/4 Serial Input/Output Controller emulation

****************************************************************************
                            _____   _____
                   CLK   1 |*    \_/     | 40  Vcc
                _RESET   2 |             | 39  _CTSA
                  _CDA   3 |             | 38  _RTSA
                 _RxCB   4 |             | 37  TxDA
                  _CDB   5 |             | 36  _TxCA
                 _CTSB   6 |             | 35  _RxCA
                 _TxCB   7 |             | 34  RxDA
                  TxDB   8 |             | 33  _SYNDETA
                  RxDB   9 |             | 32  RDYA/RxDRQA
        _RTSB/_SYNDETB  10 |    I8274    | 31  _DTRA
          RDYB/_TxDRQA  11 |             | 30  _IPO/TxDRQB
                    D7  12 |             | 29  _IPI/RxDRQB
                    D6  13 |             | 28  _INT
                    D5  14 |             | 27  _INTA
                    D4  15 |             | 26  _DTRB
                    D3  16 |             | 25  A0
                    D2  17 |             | 24  A1
                    D1  18 |             | 23  _CS
                    D0  19 |             | 22  _RD
                   Vss  20 |_____________| 21  _WR

                            _____   _____
                   CLK   1 |*    \_/     | 40  Vcc
                _RESET   2 |             | 39  _CTSA
                 _DCDA   3 |             | 38  _RTSA
                 _RxCB   4 |             | 37  TxDA
                 _DCDB   5 |             | 36  _TxCA
                 _CTSB   6 |             | 35  _RxCA
                 _TxCB   7 |             | 34  RxDA
                  TxDB   8 |             | 33  _SYNCA
                  RxDB   9 |             | 32  _WAITA/DRQRxA
          _RTSB/_SYNCB  10 |   UPD7201   | 31  _DTRA/_HAO
        _WAITB/_DRQTxA  11 |             | 30  _PRO/DRQTxB
                    D7  12 |             | 29  _PRI/DRQRxB
                    D6  13 |             | 28  _INT
                    D5  14 |             | 27  _INTAK
                    D4  15 |             | 26  _DTRB/_HAI
                    D3  16 |             | 25  B/_A
                    D2  17 |             | 24  C/_D
                    D1  18 |             | 23  _CS
                    D0  19 |             | 22  _RD
                   Vss  20 |_____________| 21  _WR

                            _____   _____
                    D1   1 |*    \_/     | 40  D0
                    D3   2 |             | 39  D2
                    D5   3 |             | 38  D4
                    D7   4 |             | 37  D6
                  _INT   5 |             | 36  _IORQ
                   IEI   6 |             | 35  _CE
                   IEO   7 |             | 34  B/_A
                   _M1   8 |             | 33  C/_D
                   Vdd   9 |             | 32  _RD
               _W/RDYA  10 |   Z80-DART  | 31  GND
                  _RIA  11 |    Z8470    | 30  _W/RDYB
                  RxDA  12 |             | 29  _RIB
                 _RxCA  13 |             | 28  RxDB
                 _TxCA  14 |             | 27  _RxTxCB
                  TxDA  15 |             | 26  TxDB
                 _DTRA  16 |             | 25  _DTRB
                 _RTSA  17 |             | 24  _RTSB
                 _CTSA  18 |             | 23  _CTSB
                 _DCDA  19 |             | 22  _DCDB
                   CLK  20 |_____________| 21  _RESET

                            _____   _____
                    D1   1 |*    \_/     | 40  D0
                    D3   2 |             | 39  D2
                    D5   3 |             | 38  D4
                    D7   4 |             | 37  D6
                  _INT   5 |             | 36  _IORQ
                   IEI   6 |             | 35  _CE
                   IEO   7 |             | 34  B/_A
                   _M1   8 |             | 33  C/_D
                   Vdd   9 |             | 32  _RD
               _W/RDYA  10 |  Z80-SIO/0  | 31  GND
                _SYNCA  11 |    Z8440    | 30  _W/RDYB
                  RxDA  12 |             | 29  _SYNCB
                 _RxCA  13 |             | 28  RxDB
                 _TxCA  14 |             | 27  _RxTxCB
                  TxDA  15 |             | 26  TxDB
                 _DTRA  16 |             | 25  _DTRB
                 _RTSA  17 |             | 24  _RTSB
                 _CTSA  18 |             | 23  _CTSB
                 _DCDA  19 |             | 22  _DCDB
                   CLK  20 |_____________| 21  _RESET

                            _____   _____
                    D1   1 |*    \_/     | 40  D0
                    D3   2 |             | 39  D2
                    D5   3 |             | 38  D4
                    D7   4 |             | 37  D6
                  _INT   5 |             | 36  _IORQ
                   IEI   6 |             | 35  _CE
                   IEO   7 |             | 34  B/_A
                   _M1   8 |             | 33  C/_D
                   Vdd   9 |             | 32  _RD
               _W/RDYA  10 |  Z80-SIO/1  | 31  GND
                _SYNCA  11 |    Z8441    | 30  _W/RDYB
                  RxDA  12 |             | 29  _SYNCB
                 _RxCA  13 |             | 28  RxDB
                 _TxCA  14 |             | 27  _RxCB
                  TxDA  15 |             | 26  _TxCB
                 _DTRA  16 |             | 25  TxDB
                 _RTSA  17 |             | 24  _RTSB
                 _CTSA  18 |             | 23  _CTSB
                 _DCDA  19 |             | 22  _DCDB
                   CLK  20 |_____________| 21  _RESET

                            _____   _____
                    D1   1 |*    \_/     | 40  D0
                    D3   2 |             | 39  D2
                    D5   3 |             | 38  D4
                    D7   4 |             | 37  D6
                  _INT   5 |             | 36  _IORQ
                   IEI   6 |             | 35  _CE
                   IEO   7 |             | 34  B/_A
                   _M1   8 |             | 33  C/_D
                   Vdd   9 |             | 32  _RD
               _W/RDYA  10 |  Z80-SIO/2  | 31  GND
                _SYNCA  11 |    Z8442    | 30  _W/RDYB
                  RxDA  12 |             | 29  _RxDB
                 _RxCA  13 |             | 28  _RxCB
                 _TxCA  14 |             | 27  _TxCB
                  TxDA  15 |             | 26  TxDB
                 _DTRA  16 |             | 25  _DTRB
                 _RTSA  17 |             | 24  _RTSB
                 _CTSA  18 |             | 23  _CTSB
                 _DCDA  19 |             | 22  _DCDB
                   CLK  20 |_____________| 21  _RESET

***************************************************************************/

#ifndef MAME_MACHINE_Z80DART_H
#define MAME_MACHINE_Z80DART_H

#pragma once

#include "machine/z80daisy.h"
#include "diserial.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> z80dart_channel

class z80dart_device;

class z80dart_channel : public device_t,
						public device_serial_interface
{
	friend class z80dart_device; // FIXME: still accesses m_rr and m_wr directly in a couple of places
public:
	enum
	{
		INT_TRANSMIT = 0,
		INT_EXTERNAL,
		INT_RECEIVE,
		INT_SPECIAL
	};

	z80dart_channel(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE_LINE_MEMBER( write_rx );
	DECLARE_WRITE_LINE_MEMBER( cts_w );
	DECLARE_WRITE_LINE_MEMBER( dcd_w );
	DECLARE_WRITE_LINE_MEMBER( ri_w );
	DECLARE_WRITE_LINE_MEMBER( rxc_w );
	DECLARE_WRITE_LINE_MEMBER( txc_w );
	DECLARE_WRITE_LINE_MEMBER( sync_w );

	uint8_t control_read();
	void control_write(uint8_t data);

	uint8_t data_read();
	void data_write(uint8_t data);

	void set_rxc(int rxc) { m_rxc = rxc; }
	void set_txc(int txc) { m_txc = txc; }

	void clr_interrupt_pending() { m_rr[0] &= ~RR0_INTERRUPT_PENDING; }
	void set_interrupt_pending() { m_rr[0] |= RR0_INTERRUPT_PENDING; }

	uint8_t get_vector() const { return m_rr[2]; }
	void set_vector(uint8_t vector) { m_rr[2] = vector; }

	bool get_status_vector() const { return m_wr[1] & WR1_STATUS_VECTOR; }
	bool get_priority() const { return m_wr[2] & WR2_PRIORITY; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_serial_interface overrides
	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_callback() override;
	virtual void rcv_complete() override;

	void receive_data(uint8_t data);

	int m_rxc;
	int m_txc;

	// register state
	uint8_t m_rr[3];              // read register
	uint8_t m_wr[6];              // write register

	enum
	{
		RR0_RX_CHAR_AVAILABLE     = 0x01,
		RR0_INTERRUPT_PENDING     = 0x02,
		RR0_TX_BUFFER_EMPTY       = 0x04,
		RR0_DCD                   = 0x08,
		RR0_RI                    = 0x10,
		RR0_SYNC_HUNT             = 0x10, // not supported
		RR0_CTS                   = 0x20,
		RR0_TX_UNDERRUN           = 0x40, // not supported
		RR0_BREAK_ABORT           = 0x80  // not supported
	};

	enum
	{
		RR1_ALL_SENT              = 0x01,
		RR1_RESIDUE_CODE_MASK     = 0x0e, // not supported
		RR1_PARITY_ERROR          = 0x10,
		RR1_RX_OVERRUN_ERROR      = 0x20,
		RR1_CRC_FRAMING_ERROR     = 0x40,
		RR1_END_OF_FRAME          = 0x80  // not supported
	};

	enum
	{
		WR0_REGISTER_MASK         = 0x07,
		WR0_COMMAND_MASK          = 0x38,
		WR0_NULL                  = 0x00,
		WR0_SEND_ABORT            = 0x08, // not supported
		WR0_RESET_EXT_STATUS      = 0x10,
		WR0_CHANNEL_RESET         = 0x18,
		WR0_ENABLE_INT_NEXT_RX    = 0x20,
		WR0_RESET_TX_INT          = 0x28, // not supported
		WR0_ERROR_RESET           = 0x30,
		WR0_RETURN_FROM_INT       = 0x38, // not supported
		WR0_CRC_RESET_CODE_MASK   = 0xc0, // not supported
		WR0_CRC_RESET_NULL        = 0x00, // not supported
		WR0_CRC_RESET_RX          = 0x40, // not supported
		WR0_CRC_RESET_TX          = 0x80, // not supported
		WR0_CRC_RESET_TX_UNDERRUN = 0xc0  // not supported
	};

	enum
	{
		WR1_EXT_INT_ENABLE        = 0x01,
		WR1_TX_INT_ENABLE         = 0x02,
		WR1_STATUS_VECTOR         = 0x04,
		WR1_RX_INT_MODE_MASK      = 0x18,
		WR1_RX_INT_DISABLE        = 0x00,
		WR1_RX_INT_FIRST          = 0x08,
		WR1_RX_INT_ALL_PARITY     = 0x10, // not supported
		WR1_RX_INT_ALL            = 0x18,
		WR1_WRDY_ON_RX_TX         = 0x20, // not supported
		WR1_WRDY_FUNCTION         = 0x40, // not supported
		WR1_WRDY_ENABLE           = 0x80  // not supported
	};

	enum
	{
		WR2_DATA_XFER_INT         = 0x00, // not supported
		WR2_DATA_XFER_DMA_INT     = 0x01, // not supported
		WR2_DATA_XFER_DMA         = 0x02, // not supported
		WR2_DATA_XFER_ILLEGAL     = 0x03, // not supported
		WR2_DATA_XFER_MASK        = 0x03, // not supported
		WR2_PRIORITY              = 0x04, // not supported
		WR2_MODE_8085_1           = 0x00, // not supported
		WR2_MODE_8085_2           = 0x08, // not supported
		WR2_MODE_8086_8088        = 0x10, // not supported
		WR2_MODE_ILLEGAL          = 0x18, // not supported
		WR2_MODE_MASK             = 0x18, // not supported
		WR2_VECTORED_INT          = 0x20, // not supported
		WR2_PIN10_SYNDETB_RTSB    = 0x80  // not supported
	};

	enum
	{
		WR3_RX_ENABLE             = 0x01,
		WR3_SYNC_CHAR_LOAD_INHIBIT= 0x02, // not supported
		WR3_ADDRESS_SEARCH_MODE   = 0x04, // not supported
		WR3_RX_CRC_ENABLE         = 0x08, // not supported
		WR3_ENTER_HUNT_PHASE      = 0x10, // not supported
		WR3_AUTO_ENABLES          = 0x20,
		WR3_RX_WORD_LENGTH_MASK   = 0xc0,
		WR3_RX_WORD_LENGTH_5      = 0x00,
		WR3_RX_WORD_LENGTH_7      = 0x40,
		WR3_RX_WORD_LENGTH_6      = 0x80,
		WR3_RX_WORD_LENGTH_8      = 0xc0
	};

	enum
	{
		WR4_PARITY_ENABLE         = 0x01,
		WR4_PARITY_EVEN           = 0x02,
		WR4_STOP_BITS_MASK        = 0x0c,
		WR4_STOP_BITS_1           = 0x04,
		WR4_STOP_BITS_1_5         = 0x08, // not supported
		WR4_STOP_BITS_2           = 0x0c,
		WR4_SYNC_MODE_MASK        = 0x30, // not supported
		WR4_SYNC_MODE_8_BIT       = 0x00, // not supported
		WR4_SYNC_MODE_16_BIT      = 0x10, // not supported
		WR4_SYNC_MODE_SDLC        = 0x20, // not supported
		WR4_SYNC_MODE_EXT         = 0x30, // not supported
		WR4_CLOCK_RATE_MASK       = 0xc0,
		WR4_CLOCK_RATE_X1         = 0x00,
		WR4_CLOCK_RATE_X16        = 0x40,
		WR4_CLOCK_RATE_X32        = 0x80,
		WR4_CLOCK_RATE_X64        = 0xc0
	};

	enum
	{
		WR5_TX_CRC_ENABLE         = 0x01, // not supported
		WR5_RTS                   = 0x02,
		WR5_CRC16                 = 0x04, // not supported
		WR5_TX_ENABLE             = 0x08,
		WR5_SEND_BREAK            = 0x10,
		WR5_TX_WORD_LENGTH_MASK   = 0x60,
		WR5_TX_WORD_LENGTH_5      = 0x00,
		WR5_TX_WORD_LENGTH_6      = 0x40,
		WR5_TX_WORD_LENGTH_7      = 0x20,
		WR5_TX_WORD_LENGTH_8      = 0x60,
		WR5_DTR                   = 0x80
	};

	void update_serial();
	void set_dtr(int state);
	void set_rts(int state);

	int get_clock_mode();
	stop_bits_t get_stop_bits();
	int get_rx_word_length();
	int get_tx_word_length();

	// receiver state
	util::fifo<uint8_t, 3> m_rx_data_fifo;
	util::fifo<uint8_t, 3> m_rx_error_fifo;

	uint8_t m_rx_error;           // current receive error
	int m_rx_clock;             // receive clock pulse count
	int m_rx_first;             // first character received
	int m_rx_break;             // receive break condition
	uint8_t m_rx_rr0_latch;       // read register 0 latched

	int m_rxd;
	int m_ri;                   // ring indicator latch
	int m_cts;                  // clear to send latch
	int m_dcd;                  // data carrier detect latch

	// transmitter state
	uint8_t m_tx_data;            // transmit data register
	int m_tx_clock;             // transmit clock pulse count

	int m_dtr;                  // data terminal ready
	int m_rts;                  // request to send

	// synchronous state
	uint16_t m_sync;              // sync character

	int m_index;
	z80dart_device *m_uart;
};


// ======================> z80dart_device

class z80dart_device :  public device_t,
						public device_z80daisy_interface
{
	friend class z80dart_channel;

public:
	// construction/destruction
	z80dart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto out_txda_callback() { return m_out_txda_cb.bind(); }
	auto out_dtra_callback() { return m_out_dtra_cb.bind(); }
	auto out_rtsa_callback() { return m_out_rtsa_cb.bind(); }
	auto out_wrdya_callback() { return m_out_wrdya_cb.bind(); }
	auto out_synca_callback() { return m_out_synca_cb.bind(); }
	auto out_txdb_callback() { return m_out_txdb_cb.bind(); }
	auto out_dtrb_callback() { return m_out_dtrb_cb.bind(); }
	auto out_rtsb_callback() { return m_out_rtsb_cb.bind(); }
	auto out_wrdyb_callback() { return m_out_wrdyb_cb.bind(); }
	auto out_syncb_callback() { return m_out_syncb_cb.bind(); }
	auto out_int_callback() { return m_out_int_cb.bind(); }
	auto out_rxdrqa_callback() { return m_out_rxdrqa_cb.bind(); }
	auto out_txdrqa_callback() { return m_out_txdrqa_cb.bind(); }
	auto out_rxdrqb_callback() { return m_out_rxdrqb_cb.bind(); }
	auto out_txdrqb_callback() { return m_out_txdrqb_cb.bind(); }

	void configure_channels(int rxa, int txa, int rxb, int txb)
	{
		m_rxca = rxa;
		m_txca = txa;
		m_rxcb = rxb;
		m_txcb = txb;
	}

	uint8_t cd_ba_r(offs_t offset);
	void cd_ba_w(offs_t offset, uint8_t data);
	uint8_t ba_cd_r(offs_t offset);
	void ba_cd_w(offs_t offset, uint8_t data);

	uint8_t da_r() { return m_chanA->data_read(); }
	void da_w(uint8_t data) { m_chanA->data_write(data); }
	uint8_t db_r() { return m_chanB->data_read(); }
	void db_w(uint8_t data) { m_chanB->data_write(data); }

	uint8_t ca_r() { return m_chanA->control_read(); }
	void ca_w(uint8_t data) { m_chanA->control_write(data); }
	uint8_t cb_r() { return m_chanB->control_read(); }
	void cb_w(uint8_t data) { m_chanB->control_write(data); }

	// interrupt acknowledge
	int m1_r();

	DECLARE_WRITE_LINE_MEMBER( rxa_w ) { m_chanA->write_rx(state); }
	DECLARE_WRITE_LINE_MEMBER( rxb_w ) { m_chanB->write_rx(state); }
	DECLARE_WRITE_LINE_MEMBER( ctsa_w ) { m_chanA->cts_w(state); }
	DECLARE_WRITE_LINE_MEMBER( ctsb_w ) { m_chanB->cts_w(state); }
	DECLARE_WRITE_LINE_MEMBER( dcda_w ) { m_chanA->dcd_w(state); }
	DECLARE_WRITE_LINE_MEMBER( dcdb_w ) { m_chanB->dcd_w(state); }
	DECLARE_WRITE_LINE_MEMBER( ria_w ) { m_chanA->ri_w(state); }
	DECLARE_WRITE_LINE_MEMBER( rib_w ) { m_chanB->ri_w(state); }
	DECLARE_WRITE_LINE_MEMBER( rxca_w ) { m_chanA->rxc_w(state); }
	DECLARE_WRITE_LINE_MEMBER( rxcb_w ) { m_chanB->rxc_w(state); }
	DECLARE_WRITE_LINE_MEMBER( txca_w ) { m_chanA->txc_w(state); }
	DECLARE_WRITE_LINE_MEMBER( txcb_w ) { m_chanB->txc_w(state); }
	DECLARE_WRITE_LINE_MEMBER( rxtxcb_w ) { m_chanB->rxc_w(state); m_chanB->txc_w(state); }
	DECLARE_WRITE_LINE_MEMBER( synca_w ) { m_chanA->sync_w(state); }
	DECLARE_WRITE_LINE_MEMBER( syncb_w ) { m_chanB->sync_w(state); }

protected:
	z80dart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t variant);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	// device_z80daisy_interface overrides
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;

	// internal interrupt management
	void check_interrupts();
	void reset_interrupts();
	void trigger_interrupt(int index, int state);
	int get_channel_index(z80dart_channel *ch) { return (ch == m_chanA) ? 0 : 1; }

	enum
	{
		TYPE_DART,
		TYPE_SIO0,
		TYPE_SIO1,
		TYPE_SIO2,
		TYPE_SIO3,
		TYPE_SIO4,
		TYPE_I8274,
		TYPE_UPD7201
	};

	enum
	{
		CHANNEL_A = 0,
		CHANNEL_B
	};

	required_device<z80dart_channel> m_chanA;
	required_device<z80dart_channel> m_chanB;

	// internal state
	int m_rxca;
	int m_txca;
	int m_rxcb;
	int m_txcb;

	devcb_write_line    m_out_txda_cb;
	devcb_write_line    m_out_dtra_cb;
	devcb_write_line    m_out_rtsa_cb;
	devcb_write_line    m_out_wrdya_cb;
	devcb_write_line    m_out_synca_cb;

	devcb_write_line    m_out_txdb_cb;
	devcb_write_line    m_out_dtrb_cb;
	devcb_write_line    m_out_rtsb_cb;
	devcb_write_line    m_out_wrdyb_cb;
	devcb_write_line    m_out_syncb_cb;

	devcb_write_line    m_out_int_cb;
	devcb_write_line    m_out_rxdrqa_cb;
	devcb_write_line    m_out_txdrqa_cb;
	devcb_write_line    m_out_rxdrqb_cb;
	devcb_write_line    m_out_txdrqb_cb;

	int m_int_state[8];     // interrupt state

	int const m_variant;
};


// ======================> z80sio0_device

class z80sio0_device : public z80dart_device
{
public:
	// construction/destruction
	z80sio0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> z80sio1_device

class z80sio1_device :  public z80dart_device
{
public:
	// construction/destruction
	z80sio1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> z80sio2_device

class z80sio2_device :  public z80dart_device
{
public:
	// construction/destruction
	z80sio2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> z80sio3_device

class z80sio3_device :  public z80dart_device
{
public:
	// construction/destruction
	z80sio3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> z80sio4_device

class z80sio4_device :  public z80dart_device
{
public:
	// construction/destruction
	z80sio4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> i8274_device

class i8274_device :  public z80dart_device
{
public:
	// construction/destruction
	i8274_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t inta_r() { return m1_r(); };
};


// ======================> upd7201_device

class upd7201_device :  public z80dart_device
{
public:
	// construction/destruction
	upd7201_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// device type definition
DECLARE_DEVICE_TYPE(Z80DART_CHANNEL, z80dart_channel)
DECLARE_DEVICE_TYPE(Z80DART,         z80dart_device)
DECLARE_DEVICE_TYPE(Z80SIO0,         z80sio0_device)
DECLARE_DEVICE_TYPE(Z80SIO1,         z80sio1_device)
DECLARE_DEVICE_TYPE(Z80SIO2,         z80sio2_device)
DECLARE_DEVICE_TYPE(Z80SIO3,         z80sio3_device)
DECLARE_DEVICE_TYPE(Z80SIO4,         z80sio4_device)
DECLARE_DEVICE_TYPE(I8274,           i8274_device)
DECLARE_DEVICE_TYPE(UPD7201,         upd7201_device)

#endif // MAME_MACHINE_Z80DART_H
