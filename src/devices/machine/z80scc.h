// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************

    Z80-SCC Serial Communications Controller emulation

****************************************************************************
               _____   _____                          _____   _____
        AD1  1|*    \_/     |40 AD0             D1  1|*    \_/     |40 D0
        AD3  2|             |39 AD2             D3  2|             |39 D2
        AD5  3|             |38 AD4             D5  3|             |38 D4
        AD7  4|             |37 AD6             D7  4|             |37 D6
       _INT  5|             |36 _DS           _INT  5|             |36 _RD
        IEO  6|             |35 _AS            IEO  6|             |35 _WR
        IEI  7|             |34 R/_W           IEI  7|             |34 B/_A
    _INTACK  8|             |33 _CS0       _INTACK  8|             |33 _CE
        VCC  9|             |32 CS1            VCC  9|             |32 C/_D
   _W//REQA 10|             |31 GND       _W//REQA 10|             |31 GND
     _SYNCA 11|   Z8030     |30 _W/_REQ     _SYNCA 11|   Z8530     |30 _W/_REQB
     _RTxCA 12|   Z80C30    |29 _SYNCB      _RTxCA 12|   Z85C30    |29 _SYNCB
       RxDA 13|   Z80230    |28 _RTxCB        RxDA 13|   Z85230    |28 _RTxCB
     _TRxCA 14|             |27 RxDB        _TRxCA 14|             |27 RxDB
       TxDA 15|             |26 _TRxCB        TxDA 15|             |26 _TRxCB
 _DTR//REQA 16|             |25 TxDB    _DTR//REQA 16|             |25 TxDB
      _RTSA 17|             |24 _DTR/_REQB   _RTSA 17|             |24 _DTR/_REQB
      _CTSA 18|             |23 _RTSB        _CTSA 18|             |23 _RTSB
      _DCDA 19|             |22 _CTSB        _DCDA 19|             |22 _CTSB
       PCLK 20|_____________|21 _DCDB         PCLK 20|_____________|21 _DCDB
                  ZBUS                                Universal Bus

***************************************************************************/

#ifndef MAME_MACHINE_Z80SCC_H
#define MAME_MACHINE_Z80SCC_H

#pragma once

#include "machine/z80daisy.h"
#include "diserial.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> z80scc_channel

class z80scc_device;

//class z80scc_channel : public z80sio_channel
class z80scc_channel : public device_t,
		public device_serial_interface
{
	friend class z80scc_device;

public:
	z80scc_channel(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_serial_interface implementation
	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_callback() override;
	virtual void rcv_complete() override;

	// read register handlers
	uint8_t do_sccreg_rr0();
	uint8_t do_sccreg_rr1();
	uint8_t do_sccreg_rr2();
	uint8_t do_sccreg_rr3();
	uint8_t do_sccreg_rr4();
	uint8_t do_sccreg_rr5();
	uint8_t do_sccreg_rr6();
	uint8_t do_sccreg_rr7();
	uint8_t do_sccreg_rr7p(){ return 0; } // Needs to be implemented for Synchronous mode
	// uint8_t do_sccreg_rr8(); Short cutted due to frequent use
	uint8_t do_sccreg_rr9();
	uint8_t do_sccreg_rr10();
	uint8_t do_sccreg_rr11();
	uint8_t do_sccreg_rr12();
	uint8_t do_sccreg_rr13();
	uint8_t do_sccreg_rr14();
	uint8_t do_sccreg_rr15();

	// write register handlers
	void do_sccreg_wr0(uint8_t data);
	void do_sccreg_wr0_resets(uint8_t data);
	void do_sccreg_wr1(uint8_t data);
	void do_sccreg_wr2(uint8_t data);
	void do_sccreg_wr3(uint8_t data);
	void do_sccreg_wr4(uint8_t data);
	void do_sccreg_wr5(uint8_t data);
	void do_sccreg_wr6(uint8_t data);
	void do_sccreg_wr7(uint8_t data);
	void do_sccreg_wr7p(uint8_t data){}; // Needs to be implemented for Synchronous mode
	void do_sccreg_wr8(uint8_t data);
	void do_sccreg_wr9(uint8_t data);
	void do_sccreg_wr10(uint8_t data);
	void do_sccreg_wr11(uint8_t data);
	void do_sccreg_wr12(uint8_t data);
	void do_sccreg_wr13(uint8_t data);
	void do_sccreg_wr14(uint8_t data);
	void do_sccreg_wr15(uint8_t data);

	uint8_t control_read();
	void control_write(uint8_t data);

	uint8_t data_read();
	void data_write(uint8_t data);

	void receive_data(uint8_t data);
	void m_tx_fifo_rp_step();
	void m_rx_fifo_rp_step();
	uint8_t m_rx_fifo_rp_data();

	void write_rx(int state);
	void cts_w(int state);
	void dcd_w(int state);
	void rxc_w(int state);
	void txc_w(int state);
	void sync_w(int state);

	TIMER_CALLBACK_MEMBER(brg_tick);

	int m_rxc;
	int m_txc;

	// Register state
	// read registers     enum
	uint8_t m_rr0; // REG_RR0_STATUS
	uint8_t m_rr1; // REG_RR1_SPEC_RCV_COND
	uint8_t m_rr2; // REG_RR2_INTERRUPT_VECT
	uint8_t m_rr3; // REG_RR3_INTERUPPT_PEND
	uint8_t m_rr4; // REG_RR4_WR4_OR_RR0
	uint8_t m_rr5; // REG_RR5_WR5_OR_RR0
	uint8_t m_rr6; // REG_RR6_LSB_OR_RR2
	uint8_t m_rr7; // REG_RR7_MSB_OR_RR3
	uint8_t m_rr7p;
	uint8_t m_rr8; // REG_RR8_RECEIVE_DATA
	uint8_t m_rr9; //  REG_RR9_WR3_OR_RR13
	uint8_t m_rr10; // REG_RR10_MISC_STATUS
	uint8_t m_rr11; // REG_RR11_WR10_OR_RR15
	uint8_t m_rr12; // REG_RR12_LO_TIME_CONST
	uint8_t m_rr13; // REG_RR13_HI_TIME_CONST
	uint8_t m_rr14; // REG_RR14_WR7_OR_R10
	uint8_t m_rr15; // REG_RR15_WR15_EXT_STAT

	// write registers    enum
	uint8_t m_wr0; // REG_WR0_COMMAND_REGPT
	uint8_t m_wr1; // REG_WR1_INT_DMA_ENABLE
	uint8_t m_wr2; // REG_WR2_INT_VECTOR
	uint8_t m_wr3; // REG_WR3_RX_CONTROL
	uint8_t m_wr4; // REG_WR4_RX_TX_MODES
	uint8_t m_wr5; // REG_WR5_TX_CONTROL
	uint8_t m_wr6; // REG_WR6_SYNC_OR_SDLC_A
	uint8_t m_wr7; // REG_WR7_SYNC_OR_SDLC_F
	uint8_t m_wr7p; //
	uint8_t m_wr8;  // REG_WR8_TRANSMIT_DATA
	//  uint8_t m_wr9;  // REG_WR9_MASTER_INT_CTRL
	uint8_t m_wr10; // REG_WR10_MSC_RX_TX_CTRL
	uint8_t m_wr11; // REG_WR11_CLOCK_MODES
	uint8_t m_wr12; // REG_WR12_LO_BAUD_GEN
	uint8_t m_wr13; // REG_WR13_HI_BAUD_GEN
	uint8_t m_wr14; // REG_WR14_MISC_CTRL
	uint8_t m_wr15; // REG_WR15_EXT_ST_INT_CTRL

protected:
	enum
	{
		RCV_IDLE                = 0,
		RCV_SEEKING             = 1,
		RCV_SAMPLING            = 2
	};

	enum
	{
		INT_TRANSMIT            = 0,
		INT_EXTERNAL            = 1,
		INT_RECEIVE             = 2,
		INT_SPECIAL             = 3,
	};

	enum
	{
		INT_TRANSMIT_PRIO       = 1,
		INT_EXTERNAL_PRIO       = 0,
		INT_RECEIVE_PRIO        = 2,
		INT_SPECIAL_PRIO        = 0,
	};

	// Read registers
	enum
	{
		REG_RR0_STATUS          = 0,
		REG_RR1_SPEC_RCV_COND   = 1,
		REG_RR2_INTERRUPT_VECT  = 2,
		REG_RR3_INTERUPPT_PEND  = 3,
		REG_RR4_WR4_OR_RR0      = 4,
		REG_RR5_WR5_OR_RR0      = 5,
		REG_RR6_LSB_OR_RR2      = 6,
		REG_RR7_MSB_OR_RR3      = 7,
		REG_RR8_RECEIVE_DATA    = 8,
		REG_RR9_WR3_OR_RR13     = 9,
		REG_RR10_MISC_STATUS    = 10,
		REG_RR11_WR10_OR_RR15   = 11,
		REG_RR12_LO_TIME_CONST  = 12,
		REG_RR13_HI_TIME_CONST  = 13,
		REG_RR14_WR7_OR_R10     = 14,
		REG_RR15_WR15_EXT_STAT  = 15
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
		REG_WR7_SYNC_OR_SDLC_F  = 7,
		REG_WR8_TRANSMIT_DATA   = 8,
		REG_WR9_MASTER_INT_CTRL = 9,
		REG_WR10_MSC_RX_TX_CTRL = 10,
		REG_WR11_CLOCK_MODES    = 11,
		REG_WR12_LO_BAUD_GEN    = 12,
		REG_WR13_HI_BAUD_GEN    = 13,
		REG_WR14_MISC_CTRL      = 14,
		REG_WR15_EXT_ST_INT_CTRL= 15
	};

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	unsigned int get_brg_rate();
	unsigned int get_rtxc_rate();
	void update_baudtimer();

	void scc_register_write(uint8_t reg, uint8_t data);
	uint8_t scc_register_read(uint8_t reg);
	void update_serial();
	void set_dtr(int state);
	void set_rts(int state);

	int get_clock_mode();
	void update_rts();
	stop_bits_t get_stop_bits();
	int get_rx_word_length();
	int get_tx_word_length();
	void safe_transmit_register_reset();
	void check_dma_request();

	emu_timer *m_baudtimer;
	uint16_t m_brg_counter;
	unsigned int m_brg_rate;
	unsigned int m_delayed_tx_brg_change;

	// receiver state
	uint8_t m_rx_data_fifo[8];    // receive data FIFO
	uint8_t m_rx_error_fifo[8];   // receive error FIFO
	uint8_t m_rx_error;       // current receive error
	//int m_rx_fifo         // receive FIFO pointer
	int m_rx_fifo_rp;       // receive FIFO read pointer
	int m_rx_fifo_wp;       // receive FIFO write pointer
	int m_rx_fifo_sz;       // receive FIFO size

	int m_rx_clock;         // receive clock pulse count
	int m_rx_first;         // first character received
	int m_rx_break;         // receive break condition

	uint8_t m_extint_latch;    // external/status Int latch enable
	uint8_t m_extint_states;   // external/status Int latches state

	int m_rxd;
	int m_ri;       // ring indicator latch

	// transmitter state
	uint8_t m_tx_data_fifo[4];  // data FIFO
	uint8_t m_tx_error_fifo[4]; // error FIFO
	int m_tx_fifo_rp;           // FIFO read pointer
	int m_tx_fifo_wp;           // FIFO write pointer
	int m_tx_fifo_sz;           // FIFO size
	uint8_t m_tx_error;         // current error
	int m_tx_clock;             // transmit clock pulse count
	int m_tx_int_disarm;        // temp Tx int disarm until next byte written


	int m_dtr;      // data terminal ready
	int m_rts;      // request to send

	// synchronous state
	uint16_t m_sync_pattern;      // sync character

	int m_rcv_mode;
	int m_index;
	z80scc_device *m_uart;

	// SCC specifics
	int m_ph;       // Point high command to access regs 08-0f
	uint8_t m_zc;

private:
	// helpers
	void out_txd_cb(int state);
	void out_rts_cb(int state);
	void out_dtr_cb(int state);

};


// ======================> z80scc_device

class z80scc_device : public device_t, public device_z80daisy_interface
{
	friend class z80scc_channel;

public:
	auto out_txda_callback() { return m_out_txd_cb[0].bind(); }
	auto out_dtra_callback() { return m_out_dtr_cb[0].bind(); }
	auto out_rtsa_callback() { return m_out_rts_cb[0].bind(); }
	auto out_wreqa_callback() { return m_out_wreq_cb[0].bind(); }
	auto out_synca_callback() { return m_out_sync_cb[0].bind(); }
	auto out_rxdrqa_callback() { return m_out_rxdrq_cb[0].bind(); }
	auto out_txdrqa_callback() { return m_out_txdrq_cb[0].bind(); }
	auto out_txdb_callback() { return m_out_txd_cb[1].bind(); }
	auto out_dtrb_callback() { return m_out_dtr_cb[1].bind(); }
	auto out_rtsb_callback() { return m_out_rts_cb[1].bind(); }
	auto out_wreqb_callback() { return m_out_wreq_cb[1].bind(); }
	auto out_syncb_callback() { return m_out_sync_cb[1].bind(); }
	auto out_rxdrqb_callback() { return m_out_rxdrq_cb[1].bind(); }
	auto out_txdrqb_callback() { return m_out_txdrq_cb[1].bind(); }
	auto out_int_callback() { return m_out_int_cb.bind(); }

	void set_cputag(const char *tag) { m_cputag = tag; }

	void configure_channels(int rxa, int txa, int rxb, int txb)
	{
		m_rxca = rxa;
		m_txca = txa;
		m_rxcb = rxb;
		m_txcb = txb;
	}

	uint8_t dc_ab_r(offs_t offset);
	void dc_ab_w(offs_t offset, uint8_t data);
	uint8_t ab_dc_r(offs_t offset);
	void ab_dc_w(offs_t offset, uint8_t data);

	/* Definitions moved to z80scc.c for enhancements */
	uint8_t da_r(offs_t offset);
	void da_w(offs_t offset, uint8_t data);
	uint8_t db_r(offs_t offset);
	void db_w(offs_t offset, uint8_t data);

	uint8_t ca_r(offs_t offset)            { return m_chanA->control_read(); }
	void ca_w(offs_t offset, uint8_t data) { m_chanA->control_write(data); }
	uint8_t cb_r(offs_t offset)            { return m_chanB->control_read(); }
	void cb_w(offs_t offset, uint8_t data) { m_chanB->control_write(data); }

	uint8_t zbus_r(offs_t offset);
	void zbus_w(offs_t offset, uint8_t data);

	// interrupt acknowledge
	int m1_r();

	// Single registers instances accessed from both channels
	uint8_t m_wr9;  // REG_WR9_MASTER_INT_CTRL

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
	int update_extint(int i);
	int get_extint_priority(int type);

protected:
	z80scc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t variant);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset_after_children() override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_z80daisy_interface implementation
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;

	// internal interrupt management
	void check_interrupts();
	void reset_interrupts();
	uint8_t modify_vector(uint8_t vect, int i, uint8_t src);
	void trigger_interrupt(int index, int state);


	int get_channel_index(z80scc_channel *ch) { return (ch == m_chanA) ? 0 : 1; }

	// Variants in the SCC family
	enum
	{
		TYPE_SCC8030  = 0x002,
		TYPE_SCC80C30 = 0x004,
		TYPE_SCC80230 = 0x008,
		TYPE_SCC8530  = 0x010,
		TYPE_SCC85C30 = 0x020,
		TYPE_SCC85230 = 0x040,
		TYPE_SCC85233 = 0x080,
		TYPE_SCC8523L = 0x100,

		SET_NMOS   = TYPE_SCC8030  | TYPE_SCC8530,
		SET_CMOS   = TYPE_SCC80C30 | TYPE_SCC85C30,
		SET_ESCC   = TYPE_SCC80230 | TYPE_SCC85230 | TYPE_SCC8523L,
		SET_EMSCC  = TYPE_SCC85233,
		SET_Z80X30 = TYPE_SCC8030  | TYPE_SCC80C30 | TYPE_SCC80230,
		SET_Z85X3X = TYPE_SCC8530  | TYPE_SCC85C30 | TYPE_SCC85230 | TYPE_SCC8523L | TYPE_SCC85233
	};

	enum
	{
		CHANNEL_A = 0,
		CHANNEL_B
	};

	required_device<z80scc_channel> m_chanA;
	required_device<z80scc_channel> m_chanB;

	// internal state
	int m_rxca;
	int m_txca;
	int m_rxcb;
	int m_txcb;

	// internal state
	devcb_write_line::array<2> m_out_txd_cb;
	devcb_write_line::array<2> m_out_dtr_cb;
	devcb_write_line::array<2> m_out_rts_cb;
	devcb_write_line::array<2> m_out_wreq_cb;
	devcb_write_line::array<2> m_out_sync_cb;
	devcb_write_line::array<2> m_out_rxdrq_cb;
	devcb_write_line::array<2> m_out_txdrq_cb;

	devcb_write_line    m_out_int_cb;

	int m_out_int_state;
	int m_int_state[6]; // interrupt state
	int m_int_source[6]; // interrupt source

	int const m_variant;
	uint8_t m_wr0_ptrbits;
	const char *m_cputag;
};

class scc8030_device : public z80scc_device
{
public:
	scc8030_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class scc80c30_device : public z80scc_device
{
public:
	scc80c30_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class scc80230_device : public z80scc_device
{
public:
	scc80230_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class scc8530_device : public z80scc_device
{
public:
	scc8530_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class scc85c30_device : public z80scc_device
{
public:
	scc85c30_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class scc85230_device : public z80scc_device
{
public:
	scc85230_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class scc85233_device : public z80scc_device
{
public:
	scc85233_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class scc8523l_device : public z80scc_device
{
public:
	scc8523l_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// device type definition
DECLARE_DEVICE_TYPE(Z80SCC_CHANNEL, z80scc_channel)
DECLARE_DEVICE_TYPE(SCC8030,        scc8030_device)
DECLARE_DEVICE_TYPE(SCC80C30,       scc80c30_device)
DECLARE_DEVICE_TYPE(SCC80230,       scc80230_device)
DECLARE_DEVICE_TYPE(SCC8530N,       scc8530_device) // remove trailing N when 8530scc.c is fully replaced and removed
DECLARE_DEVICE_TYPE(SCC85C30,       scc85c30_device)
DECLARE_DEVICE_TYPE(SCC85230,       scc85230_device)
DECLARE_DEVICE_TYPE(SCC85233,       scc85233_device)
DECLARE_DEVICE_TYPE(SCC8523L,       scc8523l_device)

#endif // MAME_MACHINE_Z80SCC_H
