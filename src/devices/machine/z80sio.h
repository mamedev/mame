// license:BSD-3-Clause
// copyright-holders:Curt Coder, Joakim Larsson Edstrom
/***************************************************************************

    Z80-SIO Serial Input/Output
    Z80-DART Dual Asynchronous Receiver/Transmitter
    Intel 8274 Multi-Protocol Serial Controller
    NEC µPD7201 Multiprotocol Serial Communications Controller

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
                        CLK   1 |*    \_/     | 40  Vcc
                     _RESET   2 |             | 39  _CTSA
                       _CDA   3 |             | 38  _RTSA
                      _RxCB   4 |             | 37  TxDA
                       _CDB   5 |             | 36  _TxCA
                      _CTSB   6 |             | 35  _RxCA
                      _TxCB   7 |             | 34  RxDA
                       TxDB   8 |             | 33  _SYNDETA
                       RxDB   9 |    DIP40    | 32  RDYA/RxDRQA
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
                       RxDB   9 |   DIP40     | 32  _WAITA/DRQRxA
               _RTSB/_SYNCB  10 |   D7201     | 31  _DTRA/_HAO
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
                           D1  1|*    \_/     |48 D0
                           D3  2|             |47 D2
                           D5  3|             |46 D4
                           D7  4|             |45 D6
                        _INTR  5|             |44 R/_W
                          CLK  6|             |43 _IACK
                        XTAL1  7|             |42 _DTACK
                        XTAL2  8|             |41 _CS
                       _RESET  9|             |40 _RxRDYB
                      _RxRDYA 10|             |39 _TxRDYB
                      _TxRDYA 11|  DIP48      |38 GND
                          Vcc 12|  MK68564    |37 _IEI
                         _IEO 13|  SIO        |36 _SYNCB
                       _SYNCA 14|             |35 _TxCB
                        _TxCA 15|             |34 _RxCB
                        _RxCA 16|             |29 RxDB
                         RxDA 17|             |28 TxDB
                         TxDA 18|             |27 _DTRB
                        _DTRA 19|             |26 _RTSB
                        _RTSA 20|             |25 _CTSB
                        _CTSA 21|             |24 _DCDB
                        _DCDA 22|             |23 A1
                           A2 23|             |22 A3
                           A4 24|_____________|21 A5

***************************************************************************/

#ifndef MAME_MACHINE_Z80SIO_H
#define MAME_MACHINE_Z80SIO_H

#pragma once

#include "machine/z80daisy.h"

//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define SIO_CHANA_TAG   "cha"
#define SIO_CHANB_TAG   "chb"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> z80sio_channel

class z80sio_device;

class z80sio_channel : public device_t
{
	friend class z80sio_device;
	friend class z80dart_device;
	friend class i8274_device;
	friend class upd7201_device;
	friend class mk68564_device;

public:
	z80sio_channel(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// read register handlers
	uint8_t do_sioreg_rr0();
	uint8_t do_sioreg_rr1();
	uint8_t do_sioreg_rr2();

	// write register handlers
	void do_sioreg_wr0(uint8_t data);
	void do_sioreg_wr0_resets(uint8_t data);
	void do_sioreg_wr1(uint8_t data);
	void do_sioreg_wr2(uint8_t data);
	void do_sioreg_wr3(uint8_t data);
	void do_sioreg_wr4(uint8_t data);
	void do_sioreg_wr5(uint8_t data);
	void do_sioreg_wr6(uint8_t data);
	void do_sioreg_wr7(uint8_t data);

	uint8_t control_read();
	void control_write(uint8_t data);

	uint8_t data_read();
	void data_write(uint8_t data);

	void write_rx(int state) { m_rxd = state; }
	void cts_w(int state);
	void dcd_w(int state);
	void rxc_w(int state);
	void txc_w(int state);
	void sync_w(int state);

	// Register state
	// read registers     enum
	uint8_t m_rr0; // REG_RR0_STATUS
	uint8_t m_rr1; // REG_RR1_SPEC_RCV_COND
	// write registers    enum
	uint8_t m_wr0; // REG_WR0_COMMAND_REGPT
	uint8_t m_wr1; // REG_WR1_INT_DMA_ENABLE
	uint8_t m_wr2; // REG_WR2_INT_VECTOR
	uint8_t m_wr3; // REG_WR3_RX_CONTROL
	uint8_t m_wr4; // REG_WR4_RX_TX_MODES
	uint8_t m_wr5; // REG_WR5_TX_CONTROL
	uint8_t m_wr6; // REG_WR6_SYNC_OR_SDLC_A
	uint8_t m_wr7; // REG_WR7_SYNC_OR_SDLC_F

protected:
	enum
	{
		INT_TRANSMIT = 0,
		INT_EXTERNAL,
		INT_RECEIVE
	};

	enum
	{
		INT_RCV_SPC_PRI_LVL  = 0,
		INT_TRANSMIT_PRI_LVL = 1,
		INT_EXTERNAL_PRI_LVL = 2
	};

	// Read registers
	enum
	{
		REG_RR0_STATUS          = 0,
		REG_RR1_SPEC_RCV_COND   = 1,
		REG_RR2_INTERRUPT_VECT  = 2
	};

	// Write registers
	enum
	{
		REG_WR0_COMMAND_REGPT   = 0,
		REG_WR1_INT_DMA_ENABLE  = 1,
		REG_WR2_INT_VECTOR      = 2,
		REG_WR3_RX_CONTROL      = 3,
		REG_WR4_RX_TX_MODES     = 4,
		REG_WR5_TX_CONTROL      = 5,
		REG_WR6_SYNC_OR_SDLC_A  = 6,
		REG_WR7_SYNC_OR_SDLC_F  = 7
	};

	// used in a flag bitmap variable
	enum : uint8_t
	{
		TX_FLAG_CRC     = 1U << 0,  // include in checksum calculation
		TX_FLAG_FRAMING = 1U << 1,  // transmitting framing bits
		TX_FLAG_ABORT_TX= 1U << 2,  // transmitting abort sequence
		TX_FLAG_CRC_TX  = 1U << 3,  // transmitting CRC value
		TX_FLAG_DATA_TX = 1U << 4   // transmitting frame data
	};

	// Sync/SDLC FSM states
	enum
	{
		SYNC_FSM_HUNT = 0,  // Hunt for start sync/flag
		SYNC_FSM_EVICT = 1, // Evict flag from sync SR
		SYNC_FSM_1ST_CHAR = 2,  // Receiving 1st character
		SYNC_FSM_IN_FRAME = 3   // Inside a frame
	};

	z80sio_channel(
			const machine_config &mconfig,
			device_type type,
			const char *tag,
			device_t *owner,
			uint32_t clock,
			uint8_t rr1_auto_reset);

	// device_t implementation
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void update_dtr_rts_break();
	void set_dtr(int state);
	void set_rts(int state);

	int get_clock_mode() const;
	int get_rx_word_length() const;
	int get_tx_word_length() const;

	// receiver state
	int m_rx_fifo_depth;
	uint32_t m_rx_data_fifo;
	uint32_t m_rx_error_fifo;

	int m_rx_clock;     // receive clock line state
	int m_rx_count;     // clocks until next sample
	bool m_dlyd_rxd;    // delayed RxD
	int m_rx_bit;       // receive data bit (0 = start bit, 1 = LSB, etc.)
	int m_rx_bit_limit; // bits to assemble for next character (sync/SDLC)
	int m_rx_sync_fsm;  // Sync/SDLC FSM state
	uint8_t m_rx_one_cnt;   // SDLC: counter to delete stuffed zeros
	uint16_t m_rx_sr;   // receive shift register
	uint8_t m_rx_sync_sr;   // rx sync SR
	uint8_t m_rx_crc_delay; // rx CRC delay SR
	uint16_t m_rx_crc;  // rx CRC accumulator
	bool m_rx_crc_en;   // rx CRC enabled
	bool m_rx_parity;   // accumulated parity

	bool m_rx_first;    // first character received

	int m_rxd;

	// transmitter state
	uint8_t m_tx_data;

	int m_tx_clock;     // transmit clock line state
	int m_tx_count;     // clocks until next bit transition
	bool m_tx_phase;    // phase of bit clock
	bool m_tx_parity;   // accumulated parity
	bool m_tx_in_pkt;   // In active part of packet (sync mode)
	bool m_tx_forced_sync;  // Force sync/flag
	uint32_t m_tx_sr;   // transmit shift register
	uint16_t m_tx_crc;  // calculated transmit checksum
	uint8_t m_tx_hist;  // transmit history (for bitstuffing)
	uint8_t m_tx_flags; // internal transmit control flags
	uint8_t m_tx_delay; // 2-bit tx delay (4 half-bits)
	uint8_t m_all_sent_delay;   // SR for all-sent delay

	int m_txd;
	int m_dtr;          // data terminal ready
	int m_rts;          // request to send

	// external/status monitoring
	bool m_ext_latched; // changed data lines
	bool m_brk_latched; // break status latched
	int m_cts;          // clear to send line state
	int m_dcd;          // data carrier detect line state
	int m_sync;         // sync line state

	// synchronous state

	int m_index;
	z80sio_device *m_uart;

protected:
	// helpers
	void out_txd_cb(int state);
	void out_rts_cb(int state);
	void out_dtr_cb(int state);
	void set_ready(bool ready);
	bool receive_allowed() const;
	virtual bool transmit_allowed() const;

	void receive_enabled();
	virtual void enter_hunt_mode();
	virtual void sync_receive();
	virtual void sdlc_receive();
	void receive_data();
	void queue_received(uint16_t data, uint32_t error);
	void advance_rx_fifo();
	uint8_t get_special_rx_mask() const;

	bool is_tx_idle() const;
	void transmit_enable();
	void transmit_complete();
	void async_tx_setup();
	virtual void sync_tx_sr_empty();
	void tx_setup(uint16_t data, int bits, bool framing, bool crc_tx, bool abort_tx);
	virtual void tx_setup_idle();
	bool get_tx_empty() const;
	void set_tx_empty(bool prev_state, bool new_state);
	void update_crc(uint16_t& crc , bool bit);

	virtual void sync_save_state();
	void reset_ext_status();
	void read_ext();
	void trigger_ext_int();

	uint8_t const m_rr1_auto_reset;
};


// ======================> z80dart_channel

class z80dart_channel : public z80sio_channel
{
public:
	z80dart_channel(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void enter_hunt_mode() override;
	virtual void sync_receive() override;
	virtual void sdlc_receive() override;
	virtual void sync_tx_sr_empty() override;
	virtual void tx_setup_idle() override;
	virtual void sync_save_state() override;
};


// ======================> i8274_channel

class i8274_channel : public z80sio_channel
{
public:
	i8274_channel(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> mk68564_channel

class mk68564_channel : public z80sio_channel
{
	friend class mk68564_device;

public:
	mk68564_channel(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual bool transmit_allowed() const override;

private:
	uint8_t cmdreg_r();
	void cmdreg_w(uint8_t data);
	uint8_t modectl_r();
	void modectl_w(uint8_t data);
	uint8_t intctl_r();
	void intctl_w(uint8_t data);
	uint8_t sync1_r();
	void sync1_w(uint8_t data);
	uint8_t sync2_r();
	void sync2_w(uint8_t data);
	uint8_t rcvctl_r();
	void rcvctl_w(uint8_t data);
	uint8_t xmtctl_r();
	void xmtctl_w(uint8_t data);
	uint8_t tcreg_r();
	void tcreg_w(uint8_t data);
	uint8_t brgctl_r();
	void brgctl_w(uint8_t data);

	void brg_update();
	TIMER_CALLBACK_MEMBER(brg_timeout);

	bool m_tx_auto_enable;
	uint8_t m_brg_tc;
	uint8_t m_brg_control;
	bool m_brg_state;
	emu_timer *m_brg_timer;
};


// ======================> z80sio_device

class z80sio_device :  public device_t,
		public device_z80daisy_interface
{
	friend class z80sio_channel;

public:
	// construction/destruction
	z80sio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto out_txda_callback() { return m_out_txd_cb[0].bind(); }
	auto out_txdb_callback() { return m_out_txd_cb[1].bind(); }
	auto out_dtra_callback() { return m_out_dtr_cb[0].bind(); }
	auto out_dtrb_callback() { return m_out_dtr_cb[1].bind(); }
	auto out_rtsa_callback() { return m_out_rts_cb[0].bind(); }
	auto out_rtsb_callback() { return m_out_rts_cb[1].bind(); }
	auto out_wrdya_callback() { return m_out_wrdy_cb[0].bind(); }
	auto out_wrdyb_callback() { return m_out_wrdy_cb[1].bind(); }
	auto out_synca_callback() { return m_out_sync_cb[0].bind(); }
	auto out_syncb_callback() { return m_out_sync_cb[1].bind(); }
	auto out_int_callback() { return m_out_int_cb.bind(); }
	auto out_rxdrqa_callback() { return m_out_rxdrq_cb[0].bind(); }
	auto out_rxdrqb_callback() { return m_out_rxdrq_cb[1].bind(); }
	auto out_txdrqa_callback() { return m_out_txdrq_cb[0].bind(); }
	auto out_txdrqb_callback() { return m_out_txdrq_cb[1].bind(); }

	template <typename T> void set_cputag(T &&tag) { m_hostcpu.set_tag(std::forward<T>(tag)); }

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
	virtual int m1_r();

	void rxa_w(int state) { m_chanA->write_rx(state); }
	void rxb_w(int state) { m_chanB->write_rx(state); }
	void ctsa_w(int state) { m_chanA->cts_w(state); }
	void ctsb_w(int state) { m_chanB->cts_w(state); }
	void dcda_w(int state) { m_chanA->dcd_w(state); }
	void dcdb_w(int state) { m_chanB->dcd_w(state); }
	void rxca_w(int state) { m_chanA->rxc_w(state); }
	void rxcb_w(int state) { m_chanB->rxc_w(state); }
	void txca_w(int state) { m_chanA->txc_w(state); }
	void txcb_w(int state) { m_chanB->txc_w(state); }
	void rxtxcb_w(int state) { m_chanB->rxc_w(state); m_chanB->txc_w(state); }
	void synca_w(int state) { m_chanA->sync_w(state); }
	void syncb_w(int state) { m_chanB->sync_w(state); }

protected:
	z80sio_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_z80daisy_interface implementation
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;

	// internal interrupt management
	void check_interrupts();
	void reset_interrupts();
	void trigger_interrupt(int index, int type);
	void clear_interrupt(int index, int type);
	void return_from_interrupt();
	virtual uint8_t read_vector();
	virtual int const *interrupt_priorities() const;

	int get_channel_index(z80sio_channel const *ch) const { return (ch == m_chanA) ? 0 : 1; }

	enum
	{
		CHANNEL_A = 0,
		CHANNEL_B
	};

	required_device<z80sio_channel> m_chanA;
	required_device<z80sio_channel> m_chanB;
	optional_device<cpu_device> m_hostcpu;

	// internal state
	devcb_write_line::array<2> m_out_txd_cb;
	devcb_write_line::array<2> m_out_dtr_cb;
	devcb_write_line::array<2> m_out_rts_cb;
	devcb_write_line::array<2> m_out_wrdy_cb;
	devcb_write_line::array<2> m_out_sync_cb;

	devcb_write_line m_out_int_cb;
	devcb_write_line::array<2> m_out_rxdrq_cb;
	devcb_write_line::array<2> m_out_txdrq_cb;

	int m_int_state[8]; // interrupt state
	int m_int_source[8]; // interrupt source
};

class z80dart_device : public z80sio_device
{
public:
	z80dart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void ria_w(int state) { m_chanA->sync_w(state); }
	void rib_w(int state) { m_chanB->sync_w(state); }

protected:
	// device_t overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class i8274_device : public z80sio_device
{
public:
	i8274_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t inta_r() { return m1_r(); }

protected:
	i8274_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_z80daisy_interface implementation
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;

	virtual uint8_t read_vector() override;
	virtual int const *interrupt_priorities() const override;
};

class upd7201_device : public i8274_device
{
public:
	upd7201_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class mk68564_device : public i8274_device
{
public:
	mk68564_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_xtal(uint32_t clock);
	void set_xtal(const XTAL &clock) { set_xtal(clock.value()); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void vectrg_w(uint8_t data);
};

// device type declaration
DECLARE_DEVICE_TYPE(Z80SIO,         z80sio_device)
DECLARE_DEVICE_TYPE(Z80DART,        z80dart_device)
DECLARE_DEVICE_TYPE(I8274,          i8274_device)
DECLARE_DEVICE_TYPE(UPD7201,        upd7201_device)
DECLARE_DEVICE_TYPE(MK68564,        mk68564_device)

#endif // MAME_MACHINE_Z80SIO_H
