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

#ifndef __Z80DART_H__
#define __Z80DART_H__

#include "emu.h"
#include "cpu/z80/z80daisy.h"


//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_Z80DART_ADD(_tag, _clock, _rxa, _txa, _rxb, _txb) \
	MCFG_DEVICE_ADD(_tag, Z80DART, _clock) \
	MCFG_Z80DART_OFFSETS(_rxa, _txa, _rxb, _txb)

#define MCFG_Z80SIO0_ADD(_tag, _clock, _rxa, _txa, _rxb, _txb) \
	MCFG_DEVICE_ADD(_tag, Z80SIO0, _clock) \
	MCFG_Z80DART_OFFSETS(_rxa, _txa, _rxb, _txb)

#define MCFG_Z80SIO1_ADD(_tag, _clock, _rxa, _txa, _rxb, _txb) \
	MCFG_DEVICE_ADD(_tag, Z80SIO1, _clock) \
	MCFG_Z80DART_OFFSETS(_rxa, _txa, _rxb, _txb)

#define MCFG_Z80SIO2_ADD(_tag, _clock, _rxa, _txa, _rxb, _txb) \
	MCFG_DEVICE_ADD(_tag, Z80SIO2, _clock) \
	MCFG_Z80DART_OFFSETS(_rxa, _txa, _rxb, _txb)

#define MCFG_Z80SIO3_ADD(_tag, _clock, _rxa, _txa, _rxb, _txb) \
	MCFG_DEVICE_ADD(_tag, Z80SIO3, _clock) \
	MCFG_Z80DART_OFFSETS(_rxa, _txa, _rxb, _txb)

#define MCFG_Z80SIO4_ADD(_tag, _clock, _rxa, _txa, _rxb, _txb) \
	MCFG_DEVICE_ADD(_tag, Z80SIO4, _clock) \
	MCFG_Z80DART_OFFSETS(_rxa, _txa, _rxb, _txb)

#define MCFG_I8274_ADD(_tag, _clock, _rxa, _txa, _rxb, _txb) \
	MCFG_DEVICE_ADD(_tag, I8274, _clock) \
	MCFG_Z80DART_OFFSETS(_rxa, _txa, _rxb, _txb)

#define MCFG_UPD7201_ADD(_tag, _clock, _rxa, _txa, _rxb, _txb) \
	MCFG_DEVICE_ADD(_tag, UPD7201, _clock) \
	MCFG_Z80DART_OFFSETS(_rxa, _txa, _rxb, _txb)


#define MCFG_Z80DART_OFFSETS(_rxa, _txa, _rxb, _txb) \
	z80dart_device::configure_channels(*device, _rxa, _txa, _rxb, _txb);

#define MCFG_Z80DART_OUT_TXDA_CB(_devcb) \
	devcb = &z80dart_device::set_out_txda_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80DART_OUT_DTRA_CB(_devcb) \
	devcb = &z80dart_device::set_out_dtra_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80DART_OUT_RTSA_CB(_devcb) \
	devcb = &z80dart_device::set_out_rtsa_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80DART_OUT_WRDYA_CB(_devcb) \
	devcb = &z80dart_device::set_out_wrdya_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80DART_OUT_SYNCA_CB(_devcb) \
	devcb = &z80dart_device::set_out_synca_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80DART_OUT_TXDB_CB(_devcb) \
	devcb = &z80dart_device::set_out_txdb_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80DART_OUT_DTRB_CB(_devcb) \
	devcb = &z80dart_device::set_out_dtrb_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80DART_OUT_RTSB_CB(_devcb) \
	devcb = &z80dart_device::set_out_rtsb_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80DART_OUT_WRDYB_CB(_devcb) \
	devcb = &z80dart_device::set_out_wrdyb_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80DART_OUT_SYNCB_CB(_devcb) \
	devcb = &z80dart_device::set_out_syncb_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80DART_OUT_INT_CB(_devcb) \
	devcb = &z80dart_device::set_out_int_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80DART_OUT_RXDRQA_CB(_devcb) \
	devcb = &z80dart_device::set_out_rxdrqa_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80DART_OUT_TXDRQA_CB(_devcb) \
	devcb = &z80dart_device::set_out_txdrqa_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80DART_OUT_RXDRQB_CB(_devcb) \
	devcb = &z80dart_device::set_out_rxdrqb_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80DART_OUT_TXDRQB_CB(_devcb) \
	devcb = &z80dart_device::set_out_txdrqb_callback(*device, DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> z80dart_channel

class z80dart_device;

class z80dart_channel : public device_t,
						public device_serial_interface
{
	friend class z80dart_device;

public:
	z80dart_channel(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_serial_interface overrides
	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_callback() override;
	virtual void rcv_complete() override;

	UINT8 control_read();
	void control_write(UINT8 data);

	UINT8 data_read();
	void data_write(UINT8 data);

	void receive_data(UINT8 data);

	DECLARE_WRITE_LINE_MEMBER( write_rx );
	DECLARE_WRITE_LINE_MEMBER( cts_w );
	DECLARE_WRITE_LINE_MEMBER( dcd_w );
	DECLARE_WRITE_LINE_MEMBER( ri_w );
	DECLARE_WRITE_LINE_MEMBER( rxc_w );
	DECLARE_WRITE_LINE_MEMBER( txc_w );
	DECLARE_WRITE_LINE_MEMBER( sync_w );

	int m_rxc;
	int m_txc;

	// register state
	UINT8 m_rr[3];              // read register
	UINT8 m_wr[6];              // write register

protected:
	enum
	{
		INT_TRANSMIT = 0,
		INT_EXTERNAL,
		INT_RECEIVE,
		INT_SPECIAL
	};

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
	UINT8 m_rx_data_fifo[3];    // receive data FIFO
	UINT8 m_rx_error_fifo[3];   // receive error FIFO
	UINT8 m_rx_error;           // current receive error
	int m_rx_fifo;              // receive FIFO pointer

	int m_rx_clock;             // receive clock pulse count
	int m_rx_first;             // first character received
	int m_rx_break;             // receive break condition
	UINT8 m_rx_rr0_latch;       // read register 0 latched

	int m_rxd;
	int m_ri;                   // ring indicator latch
	int m_cts;                  // clear to send latch
	int m_dcd;                  // data carrier detect latch

	// transmitter state
	UINT8 m_tx_data;            // transmit data register
	int m_tx_clock;             // transmit clock pulse count

	int m_dtr;                  // data terminal ready
	int m_rts;                  // request to send

	// synchronous state
	UINT16 m_sync;              // sync character

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
	z80dart_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, UINT32 variant, std::string shortname, std::string source);
	z80dart_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_out_txda_callback(device_t &device, _Object object) { return downcast<z80dart_device &>(device).m_out_txda_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_dtra_callback(device_t &device, _Object object) { return downcast<z80dart_device &>(device).m_out_dtra_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_rtsa_callback(device_t &device, _Object object) { return downcast<z80dart_device &>(device).m_out_rtsa_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_wrdya_callback(device_t &device, _Object object) { return downcast<z80dart_device &>(device).m_out_wrdya_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_synca_callback(device_t &device, _Object object) { return downcast<z80dart_device &>(device).m_out_synca_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_txdb_callback(device_t &device, _Object object) { return downcast<z80dart_device &>(device).m_out_txdb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_dtrb_callback(device_t &device, _Object object) { return downcast<z80dart_device &>(device).m_out_dtrb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_rtsb_callback(device_t &device, _Object object) { return downcast<z80dart_device &>(device).m_out_rtsb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_wrdyb_callback(device_t &device, _Object object) { return downcast<z80dart_device &>(device).m_out_wrdyb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_syncb_callback(device_t &device, _Object object) { return downcast<z80dart_device &>(device).m_out_syncb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_int_callback(device_t &device, _Object object) { return downcast<z80dart_device &>(device).m_out_int_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_rxdrqa_callback(device_t &device, _Object object) { return downcast<z80dart_device &>(device).m_out_rxdrqa_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_txdrqa_callback(device_t &device, _Object object) { return downcast<z80dart_device &>(device).m_out_txdrqa_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_rxdrqb_callback(device_t &device, _Object object) { return downcast<z80dart_device &>(device).m_out_rxdrqb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_txdrqb_callback(device_t &device, _Object object) { return downcast<z80dart_device &>(device).m_out_txdrqb_cb.set_callback(object); }

	static void configure_channels(device_t &device, int rxa, int txa, int rxb, int txb)
	{
		z80dart_device &dev = downcast<z80dart_device &>(device);
		dev.m_rxca = rxa;
		dev.m_txca = txa;
		dev.m_rxcb = rxb;
		dev.m_txcb = txb;
	}

	DECLARE_READ8_MEMBER( cd_ba_r );
	DECLARE_WRITE8_MEMBER( cd_ba_w );
	DECLARE_READ8_MEMBER( ba_cd_r );
	DECLARE_WRITE8_MEMBER( ba_cd_w );

	DECLARE_READ8_MEMBER( da_r ) { return m_chanA->data_read(); }
	DECLARE_WRITE8_MEMBER( da_w ) { m_chanA->data_write(data); }
	DECLARE_READ8_MEMBER( db_r ) { return m_chanB->data_read(); }
	DECLARE_WRITE8_MEMBER( db_w ) { m_chanB->data_write(data); }

	DECLARE_READ8_MEMBER( ca_r ) { return m_chanA->control_read(); }
	DECLARE_WRITE8_MEMBER( ca_w ) { m_chanA->control_write(data); }
	DECLARE_READ8_MEMBER( cb_r ) { return m_chanB->control_read(); }
	DECLARE_WRITE8_MEMBER( cb_w ) { m_chanB->control_write(data); }

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
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

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

	int m_variant;
};


// ======================> z80sio0_device

class z80sio0_device :  public z80dart_device
{
public:
	// construction/destruction
	z80sio0_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};


// ======================> z80sio1_device

class z80sio1_device :  public z80dart_device
{
public:
	// construction/destruction
	z80sio1_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};


// ======================> z80sio2_device

class z80sio2_device :  public z80dart_device
{
public:
	// construction/destruction
	z80sio2_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};


// ======================> z80sio3_device

class z80sio3_device :  public z80dart_device
{
public:
	// construction/destruction
	z80sio3_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};


// ======================> z80sio4_device

class z80sio4_device :  public z80dart_device
{
public:
	// construction/destruction
	z80sio4_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};


// ======================> i8274_device

class i8274_device :  public z80dart_device
{
public:
	// construction/destruction
	i8274_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( inta_r ) { return m1_r(); };
};


// ======================> upd7201_device

class upd7201_device :  public z80dart_device
{
public:
	// construction/destruction
	upd7201_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};


// device type definition
extern const device_type Z80DART_CHANNEL;
extern const device_type Z80DART;
extern const device_type Z80SIO0;
extern const device_type Z80SIO1;
extern const device_type Z80SIO2;
extern const device_type Z80SIO3;
extern const device_type Z80SIO4;
extern const device_type I8274;
extern const device_type UPD7201;


#endif
