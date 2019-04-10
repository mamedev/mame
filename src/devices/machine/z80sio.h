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
	friend class i8274_new_device;
	friend class upd7201_new_device;

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

	DECLARE_WRITE_LINE_MEMBER( write_rx ) { m_rxd = state; }
	DECLARE_WRITE_LINE_MEMBER( cts_w );
	DECLARE_WRITE_LINE_MEMBER( dcd_w );
	DECLARE_WRITE_LINE_MEMBER( rxc_w );
	DECLARE_WRITE_LINE_MEMBER( txc_w );
	DECLARE_WRITE_LINE_MEMBER( sync_w );

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
		TX_FLAG_FRAMING = 1U << 1,  // tranmitting framing bits
		TX_FLAG_SPECIAL = 1U << 2   // transmitting checksum or abort sequence
	};

	z80sio_channel(
			const machine_config &mconfig,
			device_type type,
			const char *tag,
			device_t *owner,
			uint32_t clock,
			uint8_t rr1_auto_reset);

	// device-level overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	void update_dtr_rts_break();
	void set_dtr(int state);
	void set_rts(int state);

	int get_clock_mode();
	int get_rx_word_length();
	int get_tx_word_length() const;
	int get_tx_word_length(uint8_t data) const;

	// receiver state
	int m_rx_fifo_depth;
	uint32_t m_rx_data_fifo;
	uint32_t m_rx_error_fifo;

	int m_rx_clock;     // receive clock line state
	int m_rx_count;     // clocks until next sample
	int m_rx_bit;       // receive data bit (0 = start bit, 1 = LSB, etc.)
	uint16_t m_rx_sr;   // receive shift register

	int m_rx_first;     // first character received
	int m_rx_break;     // receive break condition

	int m_rxd;
	int m_sh;           // sync hunt

	// transmitter state
	uint8_t m_tx_data;

	int m_tx_clock;     // transmit clock line state
	int m_tx_count;     // clocks until next bit transition
	int m_tx_bits;      // remaining bits in shift register
	int m_tx_parity;    // parity bit position or zero if disabled
	uint16_t m_tx_sr;   // transmit shift register
	uint16_t m_tx_crc;  // calculated transmit checksum
	uint8_t m_tx_hist;  // transmit history (for bitstuffing)
	uint8_t m_tx_flags; // internal transmit control flags

	int m_txd;
	int m_dtr;          // data terminal ready
	int m_rts;          // request to send

	// external/status monitoring
	int m_ext_latched;  // changed data lines
	int m_brk_latched;  // break status latched
	int m_cts;          // clear to send line state
	int m_dcd;          // data carrier detect line state
	int m_sync;         // sync line state

	// synchronous state

	int m_index;
	z80sio_device *m_uart;

private:
	// helpers
	void out_txd_cb(int state);
	void out_rts_cb(int state);
	void out_dtr_cb(int state);
	void set_ready(bool ready);
	bool receive_allowed() const;
	bool transmit_allowed() const;

	void receive_enabled();
	void sync_receive();
	void receive_data();
	void queue_received(uint16_t data, uint32_t error);
	void advance_rx_fifo();

	void transmit_enable();
	void transmit_complete();
	void async_tx_setup();
	void sync_tx_sr_empty();
	void tx_setup(uint16_t data, int bits, int parity, bool framing, bool special);
	void tx_setup_idle();

	void reset_ext_status();
	void read_ext();
	void trigger_ext_int();

	uint8_t const m_rr1_auto_reset;
};


// ======================> i8274_channel

class i8274_channel : public z80sio_channel
{
public:
	i8274_channel(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
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

	DECLARE_WRITE_LINE_MEMBER( rxa_w ) { m_chanA->write_rx(state); }
	DECLARE_WRITE_LINE_MEMBER( rxb_w ) { m_chanB->write_rx(state); }
	DECLARE_WRITE_LINE_MEMBER( ctsa_w ) { m_chanA->cts_w(state); }
	DECLARE_WRITE_LINE_MEMBER( ctsb_w ) { m_chanB->cts_w(state); }
	DECLARE_WRITE_LINE_MEMBER( dcda_w ) { m_chanA->dcd_w(state); }
	DECLARE_WRITE_LINE_MEMBER( dcdb_w ) { m_chanB->dcd_w(state); }
	DECLARE_WRITE_LINE_MEMBER( rxca_w ) { m_chanA->rxc_w(state); }
	DECLARE_WRITE_LINE_MEMBER( rxcb_w ) { m_chanB->rxc_w(state); }
	DECLARE_WRITE_LINE_MEMBER( txca_w ) { m_chanA->txc_w(state); }
	DECLARE_WRITE_LINE_MEMBER( txcb_w ) { m_chanB->txc_w(state); }
	DECLARE_WRITE_LINE_MEMBER( rxtxcb_w ) { m_chanB->rxc_w(state); m_chanB->txc_w(state); }
	DECLARE_WRITE_LINE_MEMBER( synca_w ) { m_chanA->sync_w(state); }
	DECLARE_WRITE_LINE_MEMBER( syncb_w ) { m_chanB->sync_w(state); }

protected:
	z80sio_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_resolve_objects() override;
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
	devcb_write_line    m_out_txd_cb[2];
	devcb_write_line    m_out_dtr_cb[2];
	devcb_write_line    m_out_rts_cb[2];
	devcb_write_line    m_out_wrdy_cb[2];
	devcb_write_line    m_out_sync_cb[2];

	devcb_write_line    m_out_int_cb;
	devcb_write_line    m_out_rxdrq_cb[2];
	devcb_write_line    m_out_txdrq_cb[2];

	int m_int_state[8]; // interrupt state
	int m_int_source[8]; // interrupt source
};

class i8274_new_device : public z80sio_device
{
public:
	i8274_new_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual int m1_r() override;

protected:
	i8274_new_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t overrides
	virtual void device_add_mconfig(machine_config &config) override;

	// device_z80daisy_interface overrides
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;

	virtual uint8_t read_vector() override;
	virtual int const *interrupt_priorities() const override;
};

class upd7201_new_device : public i8274_new_device
{
public:
	upd7201_new_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// device type declaration
DECLARE_DEVICE_TYPE(Z80SIO,         z80sio_device)
DECLARE_DEVICE_TYPE(I8274_NEW,      i8274_new_device)
DECLARE_DEVICE_TYPE(UPD7201_NEW,    upd7201_new_device)

#endif // MAME_MACHINE_Z80SIO_H
