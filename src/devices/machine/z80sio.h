// license:BSD-3-Clause
// copyright-holders:Curt Coder, Joakim Larsson Edstrom
/***************************************************************************

    Z80-SIO Serial Input/Output

****************************************************************************
             _____   _____               _____               _____
       D1  1|*    \_/     |40 D0       _/     |40 D0       _/     |40 D0
       D3  2|             |39 D2      :       |39 D2      :       |39 D2
       D5  3|             |38 D4      :       |38 D4      :       |38 D4
       D7  4|             |37 D6      :       |37 D6      :       |37 D6
     _INT  5|             |36 _IORQ   :       |36 _IORQ   :       |36 _IORQ
      IEO  6|             |35 _CE     :       |35 _CE     :       |35 _CE
      IEI  7|             |34 B/_A    :       |34 B/_A    :       |34 B/_A
      _M1  8|             |33 C/_D    :       |33 C/_D    :       |33 C/_D
      VDD  9|  DIP40      |32 _RD     : DIP40 |32 _RD     : DIP40 |32 _RD
 _W//RDYA 10|  Z80        |31 GND     : Z80   |31 GND     : Z80   |31 GND
   _SYNCA 11|  SIO/0      |30 _W/_RDYB: SIO/1 |30 _W/_RDYB: SIO/2 |30 _W/_RDYB
     RxDA 12|             |29 _SYNCB  :       |29 _SYNCB  :       |29 _SYNCB
    _RxCA 13|             |28 RxDB    :       |28 RxDB    :       |28 _RxCB
    _TxCA 14|             |27 _RxTxCB :       |27 _RxCB   :       |27 _TxCB
     TxDA 15|             |26 TxDB    :       |26 _TxCB   :       |26 TxDB
    _DTRA 16|             |25 _DTRB   :       |25 TxD_B   :       |25 _DTRB
    _RTSA 17|             |24 _RTSB   :       |24 _RTSB   :       |24 _RTSB
    _CTSA 18|             |23 _CTSB   :       |23 _CTSB   :       |23 _CTSB
    _DCDA 19|             |22 _DCDB   :       |22 _DCDB   :       |22 _DCDB
      CLK 20|_____________|21 _RESET  :_______|21 _RESET  :_______|21 _RESET

                             *I                                   *I
         *I         N         O                 *I                 O
          N D D D D / D D D D R                  N D D D D D D D D R*C
          T 7 5 3 1 C 0 2 4 6 Q                  T 7 5 3 1 0 2 4 6 Q E
         +----------------------+               +----------------------+
      IEI|34                  22| *CE        IEI|6 5 4 3 2 1 44  42  40|B/ *A
      IEO|35                  21| B/ *A      IEO|8             43  41  |C/ *D
      *M1|                      | C/ *D      *M1|9                   37|*RD
      +5v|                      | *RD        +5V|10                  36|GND
*W/ *RDYA|       QFP44          | GND  *W/ *RDYA|11    PLCC44        35|*W/ *RDYB
      N/C|      Z80 SIO/3       | N/C     *SYNCA|12   Z80 SIO/4      34|*SYNCB
   *SYNCA|       Z804C43        | *W/ *RDYB RxDA|13                  33|RxDB
     RxDA|                      | *SYNCB   *RxCA|14                  32|*RxCB
    *RxCA|42                    | RxDB     *TxCA|15                  31|*TxCB
    *TxCA|43                1 1 | *RxCB     TxDA|  19  21  23  25    30|TxDB
     TxDA`. 2 3 4 5 6 7 8 9 0 1 | *TxCB      N/C|18  20  22  24  26  29|N/C
           `--------------------+               +----------------------+
         *D*R*C*D C*R*D*C*R*D*T                 *D*R*C*D C*R*D*C*R*D N
          T T T C L E C*T T T x                  T T T C L E C T T T /
          R S S D K S D S S R D                  R S S D K S D S S R C
          A A A A   E B B B B B                  A A A A   E B B B B
                    T                                      T

***************************************************************************/

#ifndef __Z80SIO_H__
#define __Z80SIO_H__

#include "emu.h"
#include "cpu/z80/z80daisy.h"

//**************************************************************************
//	DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_Z80SIO_ADD(_tag, _clock, _rxa, _txa, _rxb, _txb) \
	MCFG_DEVICE_ADD(_tag, Z80SIO, _clock) \
	MCFG_Z80SIO_OFFSETS(_rxa, _txa, _rxb, _txb)

#define MCFG_Z80SIO_OFFSETS(_rxa, _txa, _rxb, _txb) \
	z80sio_device::configure_channels(*device, _rxa, _txa, _rxb, _txb);

#define MCFG_Z80SIO_OUT_TXDA_CB(_devcb) \
	devcb = &z80sio_device::set_out_txda_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80SIO_OUT_DTRA_CB(_devcb) \
	devcb = &z80sio_device::set_out_dtra_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80SIO_OUT_RTSA_CB(_devcb) \
	devcb = &z80sio_device::set_out_rtsa_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80SIO_OUT_WRDYA_CB(_devcb) \
	devcb = &z80sio_device::set_out_wrdya_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80SIO_OUT_SYNCA_CB(_devcb) \
	devcb = &z80sio_device::set_out_synca_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80SIO_OUT_TXDB_CB(_devcb) \
	devcb = &z80sio_device::set_out_txdb_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80SIO_OUT_DTRB_CB(_devcb) \
	devcb = &z80sio_device::set_out_dtrb_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80SIO_OUT_RTSB_CB(_devcb) \
	devcb = &z80sio_device::set_out_rtsb_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80SIO_OUT_WRDYB_CB(_devcb) \
	devcb = &z80sio_device::set_out_wrdyb_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80SIO_OUT_SYNCB_CB(_devcb) \
	devcb = &z80sio_device::set_out_syncb_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80SIO_OUT_INT_CB(_devcb) \
	devcb = &z80sio_device::set_out_int_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80SIO_OUT_RXDRQA_CB(_devcb) \
	devcb = &z80sio_device::set_out_rxdrqa_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80SIO_OUT_TXDRQA_CB(_devcb) \
	devcb = &z80sio_device::set_out_txdrqa_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80SIO_OUT_RXDRQB_CB(_devcb) \
	devcb = &z80sio_device::set_out_rxdrqb_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80SIO_OUT_TXDRQB_CB(_devcb) \
	devcb = &z80sio_device::set_out_txdrqb_callback(*device, DEVCB_##_devcb);


//**************************************************************************
//	TYPE DEFINITIONS
//**************************************************************************

// ======================> z80sio_channel

class z80sio_device;

class z80sio_channel : public device_t,
		public device_serial_interface
{
	friend class z80sio_device;

public:
	z80sio_channel(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// device_serial_interface overrides
	virtual void tra_callback();
	virtual void tra_complete();
	virtual void rcv_callback();
	virtual void rcv_complete();

	// read register handlers
	UINT8 do_sioreg_rr0();
	UINT8 do_sioreg_rr1();
	UINT8 do_sioreg_rr2();
	
	// write register handlers
	void do_sioreg_wr0(UINT8 data);
	void do_sioreg_wr0_resets(UINT8 data);
	void do_sioreg_wr1(UINT8 data);
	void do_sioreg_wr2(UINT8 data);
	void do_sioreg_wr3(UINT8 data);
	void do_sioreg_wr4(UINT8 data);
	void do_sioreg_wr5(UINT8 data);
	void do_sioreg_wr6(UINT8 data);
	void do_sioreg_wr7(UINT8 data);
	
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

	// Register state
		// read registers	  enum
	UINT8 m_rr0; // REG_RR0_STATUS
	UINT8 m_rr1; // REG_RR1_SPEC_RCV_COND
	UINT8 m_rr2; // REG_RR2_INTERRUPT_VECT
		// write registers	  enum
	UINT8 m_wr0; // REG_WR0_COMMAND_REGPT
	UINT8 m_wr1; // REG_WR1_INT_DMA_ENABLE
	UINT8 m_wr2; // REG_WR2_INT_VECTOR
	UINT8 m_wr3; // REG_WR3_RX_CONTROL
	UINT8 m_wr4; // REG_WR4_RX_TX_MODES
	UINT8 m_wr5; // REG_WR5_TX_CONTROL
	UINT8 m_wr6; // REG_WR6_SYNC_OR_SDLC_A
	UINT8 m_wr7; // REG_WR7_SYNC_OR_SDLC_F

	int m_variant; // Set in device

protected:
	enum
	{
		INT_TRANSMIT = 0,
		INT_EXTERNAL,
		INT_RECEIVE,
		INT_SPECIAL
	};

	// Read registers
	enum
	{
		REG_RR0_STATUS		= 0,
		REG_RR1_SPEC_RCV_COND	= 1,
		REG_RR2_INTERRUPT_VECT	= 2
	};
	
	// Write registers
	enum
	{
		REG_WR0_COMMAND_REGPT	= 0,
		REG_WR1_INT_DMA_ENABLE	= 1,
		REG_WR2_INT_VECTOR	= 2,
		REG_WR3_RX_CONTROL	= 3,
		REG_WR4_RX_TX_MODES	= 4,
		REG_WR5_TX_CONTROL	= 5,
		REG_WR6_SYNC_OR_SDLC_A	= 6,
		REG_WR7_SYNC_OR_SDLC_F	= 7
	};
	
	enum
	{
		RR0_RX_CHAR_AVAILABLE	  = 0x01,
		RR0_INTERRUPT_PENDING	  = 0x02,
		RR0_TX_BUFFER_EMPTY	  = 0x04,
		RR0_DCD			  = 0x08,
		RR0_SYNC_HUNT		  = 0x10,
		RR0_CTS			  = 0x20,
		RR0_TX_UNDERRUN		  = 0x40,
		RR0_BREAK_ABORT		  = 0x80
	};

	enum
	{
		RR1_ALL_SENT		  = 0x01,
		RR1_RESIDUE_CODE_MASK	  = 0x0e,
		RR1_PARITY_ERROR	  = 0x10,
		RR1_RX_OVERRUN_ERROR	  = 0x20,
		RR1_CRC_FRAMING_ERROR	  = 0x40,
		RR1_END_OF_FRAME	  = 0x80
	};

	enum
	{					  // TODO: overload SIO functionality
		RR2_INT_VECTOR_MASK	  = 0xff, // SCC channel A, SIO channel B (special case)
		RR2_INT_VECTOR_V1	  = 0x02, // SIO (special case) /SCC Channel B
		RR2_INT_VECTOR_V2	  = 0x04, // SIO (special case) /SCC Channel B
		RR2_INT_VECTOR_V3	  = 0x08  // SIO (special case) /SCC Channel B
	};
	
	enum
	{
		WR0_REGISTER_MASK	  = 0x07,
		WR0_COMMAND_MASK	  = 0x38,
		WR0_NULL		  = 0x00,
		WR0_SEND_ABORT		  = 0x08, // not supported
		WR0_RESET_EXT_STATUS	  = 0x10,
		WR0_CHANNEL_RESET	  = 0x18,
		WR0_ENABLE_INT_NEXT_RX	  = 0x20,
		WR0_RESET_TX_INT	  = 0x28, // not supported
		WR0_ERROR_RESET		  = 0x30,
		WR0_RETURN_FROM_INT	  = 0x38, // not supported
		WR0_CRC_RESET_CODE_MASK	  = 0xc0, // not supported
		WR0_CRC_RESET_NULL	  = 0x00, // not supported
		WR0_CRC_RESET_RX	  = 0x40, // not supported
		WR0_CRC_RESET_TX	  = 0x80, // not supported
		WR0_CRC_RESET_TX_UNDERRUN = 0xc0  // not supported
	};

	enum
	{
		WR1_EXT_INT_ENABLE	  = 0x01,
		WR1_TX_INT_ENABLE	  = 0x02,
		WR1_STATUS_VECTOR	  = 0x04,
		WR1_RX_INT_MODE_MASK	  = 0x18,
		WR1_RX_INT_DISABLE	  = 0x00,
		WR1_RX_INT_FIRST	  = 0x08,
		WR1_RX_INT_ALL_PARITY	  = 0x10, // not supported
		WR1_RX_INT_ALL		  = 0x18,
		WR1_WRDY_ON_RX_TX	  = 0x20, // not supported
		WR1_WRDY_FUNCTION	  = 0x40, // not supported
		WR1_WRDY_ENABLE		  = 0x80  // not supported
	};

	enum
	{
		WR2_DATA_XFER_INT	  = 0x00, // not supported
		WR2_DATA_XFER_DMA_INT	  = 0x01, // not supported
		WR2_DATA_XFER_DMA	  = 0x02, // not supported
		WR2_DATA_XFER_ILLEGAL	  = 0x03, // not supported
		WR2_DATA_XFER_MASK	  = 0x03, // not supported
		WR2_PRIORITY		  = 0x04, // not supported
		WR2_MODE_8085_1		  = 0x00, // not supported
		WR2_MODE_8085_2		  = 0x08, // not supported
		WR2_MODE_8086_8088	  = 0x10, // not supported
		WR2_MODE_ILLEGAL	  = 0x18, // not supported
		WR2_MODE_MASK		  = 0x18, // not supported
		WR2_VECTORED_INT	  = 0x20, // not supported
		WR2_PIN10_SYNDETB_RTSB	  = 0x80  // not supported
	};

	enum
	{
		WR3_RX_ENABLE		  = 0x01,
		WR3_SYNC_CHAR_LOAD_INHIBIT= 0x02, // not supported
		WR3_ADDRESS_SEARCH_MODE	  = 0x04, // not supported
		WR3_RX_CRC_ENABLE	  = 0x08, // not supported
		WR3_ENTER_HUNT_PHASE	  = 0x10, // not supported
		WR3_AUTO_ENABLES	  = 0x20,
		WR3_RX_WORD_LENGTH_MASK	  = 0xc0,
		WR3_RX_WORD_LENGTH_5	  = 0x00,
		WR3_RX_WORD_LENGTH_7	  = 0x40,
		WR3_RX_WORD_LENGTH_6	  = 0x80,
		WR3_RX_WORD_LENGTH_8	  = 0xc0
	};

	enum
	{
		WR4_PARITY_ENABLE	  = 0x01,
		WR4_PARITY_EVEN		  = 0x02,
		WR4_STOP_BITS_MASK	  = 0x0c,
		WR4_STOP_BITS_1		  = 0x04,
		WR4_STOP_BITS_1_5	  = 0x08, // not supported
		WR4_STOP_BITS_2		  = 0x0c,
		WR4_SYNC_MODE_MASK	  = 0x30, // not supported
		WR4_SYNC_MODE_8_BIT	  = 0x00, // not supported
		WR4_SYNC_MODE_16_BIT	  = 0x10, // not supported
		WR4_SYNC_MODE_SDLC	  = 0x20, // not supported
		WR4_SYNC_MODE_EXT	  = 0x30, // not supported
		WR4_CLOCK_RATE_MASK	  = 0xc0,
		WR4_CLOCK_RATE_X1	  = 0x00,
		WR4_CLOCK_RATE_X16	  = 0x40,
		WR4_CLOCK_RATE_X32	  = 0x80,
		WR4_CLOCK_RATE_X64	  = 0xc0
	};

	enum
	{
		WR5_TX_CRC_ENABLE	  = 0x01, // not supported
		WR5_RTS			  = 0x02,
		WR5_CRC16		  = 0x04, // not supported
		WR5_TX_ENABLE		  = 0x08,
		WR5_SEND_BREAK		  = 0x10,
		WR5_TX_WORD_LENGTH_MASK	  = 0x60,
		WR5_TX_WORD_LENGTH_5	  = 0x00,
		WR5_TX_WORD_LENGTH_6	  = 0x40,
		WR5_TX_WORD_LENGTH_7	  = 0x20,
		WR5_TX_WORD_LENGTH_8	  = 0x60,
		WR5_DTR			  = 0x80
	};

	void update_serial();
	void update_rts();
	void set_dtr(int state);
	void set_rts(int state);

	int get_clock_mode();
	stop_bits_t get_stop_bits();
	int get_rx_word_length();
	int get_tx_word_length();

	// receiver state
	UINT8 m_rx_data_fifo[3];	// receive data FIFO
	UINT8 m_rx_error_fifo[3];	// receive error FIFO
	UINT8 m_rx_error;		// current receive error
	int m_rx_fifo;		// receive FIFO pointer

	int m_rx_clock;		// receive clock pulse count
	int m_rx_first;		// first character received
	int m_rx_break;		// receive break condition
	UINT8 m_rx_rr0_latch;	// read register 0 latched

	int m_rxd;
	int m_sh;			// sync hunt
	int m_cts;			// clear to send latch
	int m_dcd;			// data carrier detect latch

	// transmitter state
	UINT8 m_tx_data;		// transmit data register
	int m_tx_clock;		// transmit clock pulse count

	int m_dtr;			// data terminal ready
	int m_rts;			// request to send

	// synchronous state
	UINT16 m_sync;		// sync character

	int m_index;
	z80sio_device *m_uart;
};


// ======================> z80sio_device

class z80sio_device :  public device_t,
		public device_z80daisy_interface
{
	friend class z80sio_channel;

 public:
	// construction/destruction
	z80sio_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT32 variant, const char *shortname, const char *source);
	z80sio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	template<class _Object> static devcb_base &set_out_txda_callback(device_t &device, _Object object) { return downcast<z80sio_device &>(device).m_out_txda_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_dtra_callback(device_t &device, _Object object) { return downcast<z80sio_device &>(device).m_out_dtra_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_rtsa_callback(device_t &device, _Object object) { return downcast<z80sio_device &>(device).m_out_rtsa_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_wrdya_callback(device_t &device, _Object object) { return downcast<z80sio_device &>(device).m_out_wrdya_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_synca_callback(device_t &device, _Object object) { return downcast<z80sio_device &>(device).m_out_synca_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_txdb_callback(device_t &device, _Object object) { return downcast<z80sio_device &>(device).m_out_txdb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_dtrb_callback(device_t &device, _Object object) { return downcast<z80sio_device &>(device).m_out_dtrb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_rtsb_callback(device_t &device, _Object object) { return downcast<z80sio_device &>(device).m_out_rtsb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_wrdyb_callback(device_t &device, _Object object) { return downcast<z80sio_device &>(device).m_out_wrdyb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_syncb_callback(device_t &device, _Object object) { return downcast<z80sio_device &>(device).m_out_syncb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_int_callback(device_t &device, _Object object) { return downcast<z80sio_device &>(device).m_out_int_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_rxdrqa_callback(device_t &device, _Object object) { return downcast<z80sio_device &>(device).m_out_rxdrqa_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_txdrqa_callback(device_t &device, _Object object) { return downcast<z80sio_device &>(device).m_out_txdrqa_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_rxdrqb_callback(device_t &device, _Object object) { return downcast<z80sio_device &>(device).m_out_rxdrqb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_txdrqb_callback(device_t &device, _Object object) { return downcast<z80sio_device &>(device).m_out_txdrqb_cb.set_callback(object); }

	static void configure_channels(device_t &device, int rxa, int txa, int rxb, int txb)
	{
		z80sio_device &dev = downcast<z80sio_device &>(device);
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
	virtual void device_start();
	virtual void device_reset();
	virtual machine_config_constructor device_mconfig_additions() const;

	// device_z80daisy_interface overrides
	virtual int z80daisy_irq_state();
	virtual int z80daisy_irq_ack();
	virtual void z80daisy_irq_reti();

	// internal interrupt management
	void check_interrupts();
	void reset_interrupts();
	void trigger_interrupt(int index, int state);
	int get_channel_index(z80sio_channel *ch) { return (ch == m_chanA) ? 0 : 1; }

	enum
	{
		TYPE_Z80SIO
	};

	enum
	{
		CHANNEL_A = 0,
		CHANNEL_B
	};

	required_device<z80sio_channel> m_chanA;
	required_device<z80sio_channel> m_chanB;

	// internal state
	int m_rxca;
	int m_txca;
	int m_rxcb;
	int m_txcb;

	devcb_write_line	m_out_txda_cb;
	devcb_write_line	m_out_dtra_cb;
	devcb_write_line	m_out_rtsa_cb;
	devcb_write_line	m_out_wrdya_cb;
	devcb_write_line	m_out_synca_cb;

	devcb_write_line	m_out_txdb_cb;
	devcb_write_line	m_out_dtrb_cb;
	devcb_write_line	m_out_rtsb_cb;
	devcb_write_line	m_out_wrdyb_cb;
	devcb_write_line	m_out_syncb_cb;

	devcb_write_line	m_out_int_cb;
	devcb_write_line	m_out_rxdrqa_cb;
	devcb_write_line	m_out_txdrqa_cb;
	devcb_write_line	m_out_rxdrqb_cb;
	devcb_write_line	m_out_txdrqb_cb;

	int m_int_state[8];	// interrupt state
	int m_variant;
};


// device type definition
extern const device_type Z80SIO;
extern const device_type Z80SIO_CHANNEL;
#endif
