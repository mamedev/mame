/***************************************************************************

    Z80 DART Dual Asynchronous Receiver/Transmitter implementation

    Copyright (c) 2008, The MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************
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
                  _RIA  11 |             | 30  _W/RDYB
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
                _SYNCA  11 |             | 30  _W/RDYB
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
                _SYNCA  11 |             | 30  _W/RDYB
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
                _SYNCA  11 |             | 30  _W/RDYB
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

#include "cpu/z80/z80daisy.h"



//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_Z80DART_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, Z80DART, _clock) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_Z80SIO0_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, Z80SIO0, _clock) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_Z80SIO1_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, Z80SIO1, _clock) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_Z80SIO2_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, Z80SIO2, _clock) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_Z80SIO3_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, Z80SIO3, _clock) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_Z80SIO4_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, Z80SIO4, _clock) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_Z80DART_REMOVE(_tag) \
	MCFG_DEVICE_REMOVE(_tag)

#define Z80DART_INTERFACE(_name) \
	const z80dart_interface (_name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> z80dart_interface

struct z80dart_interface
{
	int m_rx_clock_a;			// channel A receive clock
	int m_tx_clock_a;			// channel A transmit clock
	int m_rx_clock_b;			// channel B receive clock
	int m_tx_clock_b;			// channel B transmit clock

	devcb_read_line		m_in_rxda_cb;
	devcb_write_line	m_out_txda_cb;
	devcb_write_line	m_out_dtra_cb;
	devcb_write_line	m_out_rtsa_cb;
	devcb_write_line	m_out_wrdya_cb;
	devcb_write_line	m_out_synca_cb;

	devcb_read_line		m_in_rxdb_cb;
	devcb_write_line	m_out_txdb_cb;
	devcb_write_line	m_out_dtrb_cb;
	devcb_write_line	m_out_rtsb_cb;
	devcb_write_line	m_out_wrdyb_cb;
	devcb_write_line	m_out_syncb_cb;

	devcb_write_line	m_out_int_cb;
};



// ======================> z80dart_device

class z80dart_device :	public device_t,
						public device_z80daisy_interface,
						public z80dart_interface
{
	friend class dart_channel;

public:
	// construction/destruction
	z80dart_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// control register access
	UINT8 control_read(int which) { return m_channel[which].control_read(); }
	void control_write(int which, UINT8 data) { return m_channel[which].control_write(data); }

	// data register access
	UINT8 data_read(int which) { return m_channel[which].data_read(); }
	void data_write(int which, UINT8 data) { return m_channel[which].data_write(data); }

	// put data on the input lines
	void receive_data(int which, UINT8 data) { m_channel[which].receive_data(data); }

	// interrupt acknowledge
	int m1_r();

	// control line access
	void cts_w(int which, int state) { m_channel[which].cts_w(state); }
	void dcd_w(int which, int state) { m_channel[which].dcd_w(state); }
	void ri_w(int which, int state) { m_channel[which].ri_w(state); }
	void rx_w(int which, int state) { m_channel[which].rx_w(state); }
	void tx_w(int which, int state) { m_channel[which].tx_w(state); }
	void sync_w(int which, int state) { m_channel[which].sync_w(state); }

private:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	// device_z80daisy_interface overrides
	virtual int z80daisy_irq_state();
	virtual int z80daisy_irq_ack();
	virtual void z80daisy_irq_reti();

	// internal interrupt management
	void check_interrupts();
	void take_interrupt(int priority);

	// a single channel on the DART
	class dart_channel
	{
		friend class z80dart_device;

	public:
		dart_channel();

		void start(z80dart_device *device, int index, const devcb_read_line &in_rxd, const devcb_write_line &out_txd, const devcb_write_line &out_dtr, const devcb_write_line &out_rts, const devcb_write_line &out_wrdy, const devcb_write_line &out_sync);
		void reset();

		UINT8 control_read();
		void control_write(UINT8 data);

		UINT8 data_read();
		void data_write(UINT8 data);

		void receive_data(UINT8 data);

		void cts_w(int state);
		void dcd_w(int state);
		void ri_w(int state);
		void rx_w(int state);
		void tx_w(int state);
		void sync_w(int state);

	private:
		void take_interrupt(int level);
		int get_clock_mode();
		float get_stop_bits();
		int get_rx_word_length();
		int get_tx_word_length();
		int detect_start_bit();
		void shift_data_in();
		bool character_completed();
		void detect_parity_error();
		void detect_framing_error();
		void receive();
		void transmit();

		static TIMER_CALLBACK( static_rxc_tick ) { reinterpret_cast<dart_channel *>(ptr)->rx_w(1); }
		static TIMER_CALLBACK( static_txc_tick ) { reinterpret_cast<dart_channel *>(ptr)->tx_w(1); }

		z80dart_device *m_device;
		int	m_index;

		devcb_resolved_read_line	m_in_rxd_func;
		devcb_resolved_write_line	m_out_txd_func;
		devcb_resolved_write_line	m_out_dtr_func;
		devcb_resolved_write_line	m_out_rts_func;
		devcb_resolved_write_line	m_out_wrdy_func;
		devcb_resolved_write_line	m_out_sync_func;

		// register state
		UINT8 m_rr[3];				// read register
		UINT8 m_wr[6];				// write register

		// receiver state
		UINT8 m_rx_data_fifo[3];	// receive data FIFO
		UINT8 m_rx_error_fifo[3];	// receive error FIFO
		UINT8 m_rx_shift;			// 8-bit receive shift register
		UINT8 m_rx_error;			// current receive error
		int m_rx_fifo;				// receive FIFO pointer

		int m_rx_clock;				// receive clock pulse count
		int m_rx_state;				// receive state
		int m_rx_bits;				// bits received
		int m_rx_first;				// first character received
		int m_rx_parity;			// received data parity
		int m_rx_break;				// receive break condition
		UINT8 m_rx_rr0_latch;		// read register 0 latched

		int m_ri;					// ring indicator latch
		int m_cts;					// clear to send latch
		int m_dcd;					// data carrier detect latch

		// transmitter state
		UINT8 m_tx_data;			// transmit data register
		UINT8 m_tx_shift;			// transmit shift register

		int m_tx_clock;				// transmit clock pulse count
		int m_tx_state;				// transmit state
		int m_tx_bits;				// bits transmitted
		int m_tx_parity;			// transmitted data parity

		int m_dtr;					// data terminal ready
		int m_rts;					// request to send

		// synchronous state
		UINT16 m_sync;				// sync character
	};

	// internal state
	devcb_resolved_write_line		m_out_int_func;
	dart_channel					m_channel[2];		// channels
	int 							m_int_state[8];		// interrupt state

	// timers
	emu_timer *						m_rxca_timer;
	emu_timer *						m_txca_timer;
	emu_timer *						m_rxcb_timer;
	emu_timer *						m_txcb_timer;
};


// device type definition
extern const device_type Z80DART;
extern const device_type Z80SIO0;
extern const device_type Z80SIO1;
extern const device_type Z80SIO2;
extern const device_type Z80SIO3;
extern const device_type Z80SIO4;



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

// register access
DECLARE_READ8_DEVICE_HANDLER( z80dart_cd_ba_r );
DECLARE_WRITE8_DEVICE_HANDLER( z80dart_cd_ba_w );

DECLARE_READ8_DEVICE_HANDLER( z80dart_ba_cd_r );
DECLARE_WRITE8_DEVICE_HANDLER( z80dart_ba_cd_w );

// control register access
DECLARE_WRITE8_DEVICE_HANDLER( z80dart_c_w );
DECLARE_READ8_DEVICE_HANDLER( z80dart_c_r );

// data register access
DECLARE_WRITE8_DEVICE_HANDLER( z80dart_d_w );
DECLARE_READ8_DEVICE_HANDLER( z80dart_d_r );

// serial clocks
WRITE_LINE_DEVICE_HANDLER( z80dart_rxca_w );
WRITE_LINE_DEVICE_HANDLER( z80dart_txca_w );
WRITE_LINE_DEVICE_HANDLER( z80dart_rxcb_w );
WRITE_LINE_DEVICE_HANDLER( z80dart_txcb_w );
WRITE_LINE_DEVICE_HANDLER( z80dart_rxtxcb_w );

// ring indicator
WRITE_LINE_DEVICE_HANDLER( z80dart_ria_w );
WRITE_LINE_DEVICE_HANDLER( z80dart_rib_w );

// data carrier detected
WRITE_LINE_DEVICE_HANDLER( z80dart_dcda_w );
WRITE_LINE_DEVICE_HANDLER( z80dart_dcdb_w );

// clear to send
WRITE_LINE_DEVICE_HANDLER( z80dart_ctsa_w );
WRITE_LINE_DEVICE_HANDLER( z80dart_ctsb_w );

// sync
WRITE_LINE_DEVICE_HANDLER( z80dart_synca_w );
WRITE_LINE_DEVICE_HANDLER( z80dart_syncb_w );

#endif
