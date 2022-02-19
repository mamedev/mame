// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************

    Philips DUSCC - Dual Serial Communications Controller emulation

****************************************************************************

         Chan B                     Chan A  Chan B                     Chan A
         =======   _____   _____  ========  =======   _____   _____  ========
          IACKN  1|*    \_/     |48 VCC      IACKN  1|*    \_/     |48 VDD
             A3  2|             |47 A4          A3  2|             |47 A4
             A2  3|             |46 A5          A2  3|             |46 A5
             A1  4|             |45 A6          A1  4|             |45 A6
    RTxDAK/GPI1  5|             |44   RTxDAK/GPI1   5|             |44 RTxDAK/GP1
           IRQN  6|             |43 X1/CLK    IRQN  6|             |43 X1/CLK
           RDYN  7|             |42 X2      RESETN  7|             |42 X2
     RTS/SYNOUT  8|             |41    RTS/SYNOUT   8|             |41 RTS/SYNOUT
           TRxC  9|             |40 TRxC      TRxC  9|             |40 TRxC
           RTxC 10|             |39 RTxC      RTxC 10|             |39 RTxC
       DCD/SYNI 11|             |38     DCD/SYNI   11|             |38 DCD/SYNI
            RxD 12|             |37 RxD        RxD 12|             |37 RxD
            TxD 13|  SCN26562   |36 TxD        TxD 13|  SCN68562   |36 TxD
     TxDAK/GPI2 14|  SCN26C562  |35   TxDAK/GPI2   14|  SCN68C562  |35 TxDAK/GPI2
    RTxDRQ/GPO1 15|             |34   RTxDRQ/GPO1  15|             |34 RTxDRQ/GPO1
 TxDRQ/RTS/GPO2 16|             |33 TxDRQ/RTS/GPO2 16|             |33 TxDRQ/RTS/GPO2
         CTS/LC 17|             |32 CTS/LC  CTS/LC 17|             |32 CTS/LC
             D7 18|             |31 D0          D7 18|             |31 D0
             D6 19|             |30 D1          D6 19|             |30 D1
             D5 20|             |29 D2          D5 20|             |29 D2
             D4 21|             |28 D3          D4 21|             |28 D3
            RDN 22|             |27 EOPN    DTACKN 22|             |27 DONEN
         RESETN 23|             |26 WRN       DTCN 23|             |26 R/WN
            GND 24|_____________|25 CEN        CND 24|_____________|25 CSN
                    Intel Bus                          Motorola Bus

***************************************************************************/

#ifndef MAME_MACHINE_SCNXX562_H
#define MAME_MACHINE_SCNXX562_H

#pragma once

#include "machine/z80daisy.h"
#include "diserial.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> duscc_channel

class duscc_device;

class duscc_channel : public device_t, public device_serial_interface
{
	friend class duscc_device;

public:
	duscc_channel(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// read register handlers
	uint8_t do_dusccreg_cmr1_r();
	uint8_t do_dusccreg_cmr2_r();
	uint8_t do_dusccreg_s1r_r();
	uint8_t do_dusccreg_s2r_r();
	uint8_t do_dusccreg_tpr_r();
	uint8_t do_dusccreg_ttr_r();
	uint8_t do_dusccreg_rpr_r();
	uint8_t do_dusccreg_rtr_r();
	uint8_t do_dusccreg_ctprh_r();
	uint8_t do_dusccreg_ctprl_r();
	uint8_t do_dusccreg_ctcr_r();
	uint8_t do_dusccreg_omr_r();
	uint8_t do_dusccreg_cth_r();
	uint8_t do_dusccreg_ctl_r();
	uint8_t do_dusccreg_pcr_r();
	uint8_t do_dusccreg_ccr_r();
	uint8_t do_dusccreg_rxfifo_r();
	uint8_t do_dusccreg_rsr_r();
	uint8_t do_dusccreg_trsr_r();
	uint8_t do_dusccreg_ictsr_r();
	uint8_t do_dusccreg_gsr_r();
	uint8_t do_dusccreg_ier_r();
	uint8_t do_dusccreg_cid_r();
	uint8_t do_dusccreg_ivr_ivrm_r();
	uint8_t do_dusccreg_icr_r();
	uint8_t do_dusccreg_mrr_r();
	uint8_t do_dusccreg_ier1_r();
	uint8_t do_dusccreg_ier2_r();
	uint8_t do_dusccreg_ier3_r();
	uint8_t do_dusccreg_trcr_r();
	uint8_t do_dusccreg_rflr_r();
	uint8_t do_dusccreg_ftlr_r();
	uint8_t do_dusccreg_trmsr_r();
	uint8_t do_dusccreg_telr_r();

	// write register handlers
	void do_dusccreg_cmr1_w(uint8_t data);
	void do_dusccreg_cmr2_w(uint8_t data);
	void do_dusccreg_s1r_w(uint8_t data);
	void do_dusccreg_s2r_w(uint8_t data);
	void do_dusccreg_tpr_w(uint8_t data);
	void do_dusccreg_ttr_w(uint8_t data);
	void do_dusccreg_rpr_w(uint8_t data);
	void do_dusccreg_rtr_w(uint8_t data);
	void do_dusccreg_ctprh_w(uint8_t data);
	void do_dusccreg_ctprl_w(uint8_t data);
	void do_dusccreg_ctcr_w(uint8_t data);
	void do_dusccreg_omr_w(uint8_t data);
	void do_dusccreg_pcr_w(uint8_t data);
	void do_dusccreg_ccr_w(uint8_t data);
	void do_dusccreg_txfifo_w(uint8_t data);
	void do_dusccreg_rsr_w(uint8_t data);
	void do_dusccreg_trsr_w(uint8_t data);
	void do_dusccreg_ictsr_w(uint8_t data);
	void do_dusccreg_gsr_w(uint8_t data);
	void do_dusccreg_ier_w(uint8_t data);
	//  void do_dusccreg_rea_w(uint8_t data); // Short cutted non complex feature
	void do_dusccreg_ivr_w(uint8_t data);
	void do_dusccreg_icr_w(uint8_t data);
	void do_dusccreg_sea_rea_w(uint8_t data); // Short cutted non complex feature
	void do_dusccreg_mrr_w(uint8_t data);
	void do_dusccreg_ier1_w(uint8_t data);
	void do_dusccreg_ier2_w(uint8_t data);
	void do_dusccreg_ier3_w(uint8_t data);
	void do_dusccreg_trcr_w(uint8_t data);
	void do_dusccreg_ftlr_w(uint8_t data);
	void do_dusccreg_trmsr_w(uint8_t data);

	uint8_t read(offs_t &offset);
	void write(uint8_t data, offs_t &offset);

	//  uint8_t data_read();
	//  void data_write(uint8_t data);

	void receive_data(uint8_t data);
	void m_tx_fifo_rp_step();
	void m_rx_fifo_rp_step();
	uint8_t m_rx_fifo_rp_data();

	void write_rx(int state);
	void cts_w(int state);
	void dcd_w(int state);
	void ri_w(int state);
	void rxc_w(int state);
	void txc_w(int state);
	void sync_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

	// device_serial_interface overrides
	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_callback() override;
	virtual void rcv_complete() override;

	int m_rxc;
	int m_txc;
	int m_tra;
	int m_rcv;

	// Register state
	uint8_t m_cmr1;
	uint8_t m_cmr2;
	uint8_t m_s1r;
	uint8_t m_s2r;
	uint8_t m_tpr;
	uint8_t m_ttr;
	uint8_t m_rpr;
	uint8_t m_rtr;
	//  uint8_t m_ctprh;
	//  uint8_t m_ctprl;
	unsigned int m_ctpr;
	uint8_t m_ctcr;
	uint8_t m_omr;
	//  uint8_t m_cth;
	//  uint8_t m_ctl;
	unsigned int m_ct;
	uint8_t m_pcr;
	uint8_t m_ccr;
	uint8_t m_txfifo[4];
	uint8_t m_rxfifo[4];
	uint8_t m_rsr;
	uint8_t m_trsr;
	uint8_t m_ictsr;
	//  uint8_t m_gsr; // moved to the device since it is global
	uint8_t m_ier;
	//  uint8_t m_rea;
	uint8_t m_cid;
	//uint8_t m_ivr;
	//uint8_t m_icr;
	//  uint8_t m_sea;
	//uint8_t m_ivrm;
	uint8_t m_mrr;
	uint8_t m_ier1;
	uint8_t m_ier2;
	uint8_t m_ier3;
	uint8_t m_trcr;
	uint8_t m_rflr;
	uint8_t m_ftlr;
	uint8_t m_trmsr;
	uint8_t m_telr;

	enum // Needs to be 0-3 in unmodified prio level
	{
		INT_RXREADY     = 0,
		INT_TXREADY     = 1,
		INT_RXTXSTAT    = 2,
		INT_EXTCTSTAT   = 3
	};

	enum
	{
		REG_CCR_RESET_TX    = 0x00,
		REG_CCR_ENABLE_TX   = 0x02,
		REG_CCR_DISABLE_TX  = 0x03,
		REG_CCR_RESET_RX    = 0x40,
		REG_CCR_ENABLE_RX   = 0x42,
		REG_CCR_DISABLE_RX  = 0x43,
		REG_CCR_START_TIMER = 0x80,
		REG_CCR_STOP_TIMER  = 0x81,
		REG_CCR_PRST_FFFF   = 0x82,
		REG_CCR_PRST_CTPR   = 0x83,
	};

	enum
	{
		REG_CMR1_PARITY         = 0x20,
		REG_CMR1_PMMODE_MASK    = 0x18,
		REG_CMR1_PMMODE_NONE    = 0x00,
		REG_CMR1_PMMODE_RES     = 0x01,
		REG_CMR1_PMMODE_PARITY  = 0x10,
		REG_CMR1_PMMODE_FORCED  = 0x11,
		REG_CMR1_CPMODE_MASK    = 0x07,
		REG_CMR1_CPMODE_ASYNC   = 0x07
	};

	enum
	{
		REG_CMR2_DTI_MASK = 0x38,
		REG_CMR2_DTI_NODMA = 0x38
	};

	enum
	{
		REG_RPR_DATA_BITS_MASK  = 0x03,
		REG_RPR_DATA_BITS_5BIT  = 0x00,
		REG_RPR_DATA_BITS_6BIT  = 0x01,
		REG_RPR_DATA_BITS_7BIT  = 0x02,
		REG_RPR_DATA_BITS_8BIT  = 0x03,
		REG_RPR_DCD             = 0x04,
		REG_RPR_STRIP_PARITY    = 0x08,
		REG_RPR_RTS             = 0x10
	};

	enum
	{
		REG_TPR_DATA_BITS_MASK  = 0x03,
		REG_TPR_DATA_BITS_5BIT  = 0x00,
		REG_TPR_DATA_BITS_6BIT  = 0x01,
		REG_TPR_DATA_BITS_7BIT  = 0x02,
		REG_TPR_DATA_BITS_8BIT  = 0x03,
		REG_TPR_CTS             = 0x04,
		REG_TPR_RTS             = 0x08,
		REG_TPR_STOP_BITS_MASK  = 0xf0
	};

	enum
	{
		REG_TTR_EXT             = 0x80,
		REG_TTR_TXCLK_MASK      = 0x70,
		REG_TTR_TXCLK_1XEXT     = 0x00,
		REG_TTR_TXCLK_16XEXT    = 0x10,
		REG_TTR_TXCLK_DPLL      = 0x20,
		REG_TTR_TXCLK_BRG       = 0x30,
		REG_TTR_TXCLK_2X_OTHER  = 0x40,
		REG_TTR_TXCLK_32X_OTHER = 0x50,
		REG_TTR_TXCLK_2X_OWN    = 0x60,
		REG_TTR_TXCLK_32X_OWN   = 0x70,
		REG_TTR_BRG_RATE_MASK   = 0x0f,
	};

	enum
	{
		REG_RTR_EXT             = 0x80,
		REG_RTR_RXCLK_MASK      = 0x70,
		REG_RTR_RXCLK_1XEXT     = 0x00,
		REG_RTR_RXCLK_16XEXT    = 0x10,
		REG_RTR_RXCLK_BRG       = 0x20,
		REG_RTR_RXCLK_CT        = 0x30,
		REG_RTR_RXCLK_DPLL_64X_X1   = 0x40,
		REG_RTR_RXCLK_DPLL_32X_EXT  = 0x50,
		REG_RTR_RXCLK_DPLL_32X_BRG  = 0x60,
		REG_RTR_RXCLK_DPLL_32X_CT   = 0x70,
		REG_RTR_BRG_RATE_MASK       = 0x0f,
	};

	enum
	{
		REG_PCR_X2_IDC              = 0x80,
		REG_PCR_GP02_RTS            = 0x40,
		REG_PCR_SYNOUT_RTS          = 0x20,
		REG_PCR_RTXC_MASK           = 0x18,
		REG_PCR_RTXC_INPUT          = 0x00,
		REG_PCR_RTXC_CNTR_OUT       = 0x08,
		REG_PCR_RTXC_TXCLK_OUT      = 0x10,
		REG_PCR_RTXC_RXCLK_OUT      = 0x18,
		REG_PCR_TRXC_MASK           = 0x07,
		REG_PCR_TRXC_INPUT          = 0x00,
		REG_PCR_TRXC_CRYST_OUT      = 0x01,
		REG_PCR_TRXC_DPLL_OUT       = 0x02,
		REG_PCR_TRXC_CNTR_OUT       = 0x03,
		REG_PCR_TRXC_TXBRG_OUT      = 0x04,
		REG_PCR_TRXC_RXBRG_OUT      = 0x05,
		REG_PCR_TRXC_TXCLK_OUT      = 0x06,
		REG_PCR_TRXC_RXCLK_OUT      = 0x07,
	};

	enum
	{
		REG_OMR_TXRCL_MASK          = 0xe0,
		REG_OMR_TXRCL_8BIT          = 0xe0,
		REG_OMR_TXRDY_ACTIVATED     = 0x10,
		REG_OMR_RXRDY_ACTIVATED     = 0x08,
		REG_OMR_GP02                = 0x04,
		REG_OMR_GP01                = 0x02,
		REG_OMR_RTS                 = 0x01,
	};

	enum
	{
		REG_RSR_CHAR_COMPARE        = 0x80,
		REG_RSR_OVERRUN_ERROR       = 0x20,
		REG_RSR_FRAMING_ERROR       = 0x02,
		REG_RSR_PARITY_ERROR        = 0x01,
	};

	enum
	{
		REG_GSR_CHAN_A_RXREADY      = 0x01,
		REG_GSR_CHAN_B_RXREADY      = 0x10,
		REG_GSR_CHAN_A_TXREADY      = 0x02,
		REG_GSR_CHAN_B_TXREADY      = 0x20,
		REG_GSR_XXREADY_MASK        = 0x33
	};

	enum
	{
		REG_ICTSR_ZERO_DET          = 0x40,
		REG_ICTSR_DELTA_CTS         = 0x10,
		REG_ICTSR_DCD               = 0x08,
		REG_ICTSR_CTS               = 0x04,
	};

	enum
	{
		REG_IER_DCD_CTS             = 0x80,
		REG_IER_TXRDY               = 0x40,
		REG_IER_TRSR73              = 0x20,
		REG_IER_RXRDY               = 0x10,
		REG_IER_RSR76               = 0x08,
		REG_IER_RSR54               = 0x04,
		REG_IER_RSR32               = 0x02,
		REG_IER_RSR10               = 0x01,
	};

	// Register offsets, stripped from channel bit 0x20 but including A7 bit
	enum
	{
		REG_CMR1    = 0x00,
		REG_CMR2    = 0x01,
		REG_S1R     = 0x02,
		REG_S2R     = 0x03,
		REG_TPR     = 0x04,
		REG_TTR     = 0x05,
		REG_RPR     = 0x06,
		REG_RTR     = 0x07,
		REG_CTPRH   = 0x08,
		REG_CTPRL   = 0x09,
		REG_CTCR    = 0x0a,
		REG_OMR     = 0x0b,
		REG_CTH     = 0x0c,
		REG_CTL     = 0x0d,
		REG_PCR     = 0x0e,
		REG_CCR     = 0x0f,
		REG_TXFIFO_0= 0x10,
		REG_TXFIFO_1= 0x11,
		REG_TXFIFO_2= 0x12,
		REG_TXFIFO_3= 0x13,
		REG_RXFIFO_0= 0x14,
		REG_RXFIFO_1= 0x15,
		REG_RXFIFO_2= 0x16,
		REG_RXFIFO_3= 0x17,
		REG_RSR     = 0x18,
		REG_TRSR    = 0x19,
		REG_ICTSR   = 0x1a,
		REG_GSR     = 0x1b,
		REG_IER     = 0x1c,
		REG_REA     = 0x1d,
		REG_CID     = 0x1d,
		REG_IVR     = 0x1e,
		REG_ICR     = 0x1f,
		REG_SEA     = 0x1d,
		REG_IVRM    = 0x1e,
		REG_MRR     = 0x1f,
		REG_IER1    = 0x42,
		REG_IER2    = 0x43,
		REG_IER3    = 0x45,
		REG_TRCR    = 0x47,
		REG_RFLR    = 0x4e,
		REG_FTLR    = 0x5c,
		REG_TRMSR   = 0x5e,
		REG_TELR    = 0x5f,
	};

	// Timers
	emu_timer *duscc_timer;
	emu_timer *rtxc_timer;
	emu_timer *trxc_timer;

	uint8_t m_rtxc;
	uint8_t m_trxc;


	enum
	{
		REG_CTCR_ZERO_DET_INT   = 0x80,
		REG_CTCR_ZERO_DET_CTL   = 0x40,
		REG_CTCR_TIM_OC         = 0x20,
	};

	enum
	{
		TIMER_ID,
		TIMER_ID_RTXC,
		TIMER_ID_TRXC
	};

	uint16_t m_brg_rx_rate;
	uint16_t m_brg_tx_rate;
	uint16_t m_brg_const;

	// TODO: Implement the 14.4K, 56K and 64K bauds available on the CDUSCC
	static unsigned int get_baudrate(unsigned int br)
	{
		switch (br)
		{
		case 0x00: return   50; break;
		case 0x01: return   75; break;
		case 0x02: return   110; break;
		case 0x03: return   134; break;
		case 0x04: return   150; break;
		case 0x05: return   200; break;
		case 0x06: return   300; break;
		case 0x07: return   600; break;
		case 0x08: return   1050; break;
		case 0x09: return   1200; break;
		case 0x0a: return   2000; break;
		case 0x0b: return   2400; break;
		case 0x0c: return   4800; break;
		case 0x0d: return   9600; break;
		case 0x0e: return   19200; break;
		case 0x0f: return   38400; break;
		};
		return 0;
	}

	void update_serial();
	void set_dtr(int state);
	void set_rts(int state);

	int get_tx_clock_mode();
	int get_rx_clock_mode();
	stop_bits_t get_stop_bits();
	int get_rx_word_length();
	int get_tx_word_length();

	/* FIFOs and rx/tx status */
	/* Receiver */
	uint8_t m_rx_data_fifo[16];   // data FIFO
	uint8_t m_rx_error_fifo[16];  // error FIFO
	int m_rx_fifo_rp;           // FIFO read pointer
	int m_rx_fifo_wp;           // FIFO write pointer
	int m_rx_fifo_sz;           // FIFO size
	uint8_t m_rx_error;           // current error

	/* Transmitter */
	uint8_t m_tx_data_fifo[16];   // data FIFO
	uint8_t m_tx_error_fifo[16];  // error FIFO
	int m_tx_fifo_rp;           // FIFO read pointer
	int m_tx_fifo_wp;           // FIFO write pointer
	int m_tx_fifo_sz;           // FIFO size
	uint8_t m_tx_error;           // current error

	int m_rx_clock;     // receive clock pulse count
	int m_rx_first;     // first character received
	int m_rx_break;     // receive break condition

	int m_rxd;
	int m_ri;       // ring indicator latch
	int m_cts;      // clear to send latch
	int m_dcd;      // data carrier detect latch

	// transmitter state
	uint8_t m_tx_data;    // transmit data register
	int m_tx_clock;     // transmit clock pulse count

	int m_dtr;      // data terminal ready
	int m_rts;      // request to send

	// synchronous state
	uint16_t m_sync;      // sync character

	int m_rcv_mode;
	int m_index;
	duscc_device *m_uart;

	// CDUSCC specifics
	int m_a7;       // Access additional registers
};


// ======================> duscc_device

class duscc_device : public device_t, public device_z80daisy_interface
{
	friend class duscc_channel;

public:
	// construction/destruction
	duscc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto out_txda_callback() { return m_out_txda_cb.bind(); }
	auto out_dtra_callback() { return m_out_dtra_cb.bind(); }
	auto out_rtsa_callback() { return m_out_rtsa_cb.bind(); }
	auto out_synca_callback() { return m_out_synca_cb.bind(); }
	auto out_rtxca_callback() { return m_out_rtxca_cb.bind(); }
	auto out_trxca_callback() { return m_out_trxca_cb.bind(); }

	auto out_txdb_callback() { return m_out_txdb_cb.bind(); }
	auto out_dtrb_callback() { return m_out_dtrb_cb.bind(); }
	auto out_rtsb_callback() { return m_out_rtsb_cb.bind(); }
	auto out_syncb_callback() { return m_out_syncb_cb.bind(); }
	auto out_rtxcb_callback() { return m_out_rtxcb_cb.bind(); }
	auto out_trxcb_callback() { return m_out_trxcb_cb.bind(); }
	auto out_int_callback() { return m_out_int_cb.bind(); }

	void configure_channels(int rxa, int txa, int rxb, int txb)
	{
#if 0 // TODO: Fix this, need a way to set external rx/tx clocks for the channels
		m_chanA->m_rxc = rxa;
		m_chanA->m_txc = txa;
		m_chanB->m_rxc = rxb;
		m_chanB->m_txc = txb;
#endif
	}

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	// interrupt acknowledge
	uint8_t iack();

	// device_z80daisy_interface overrides
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;

	void rxa_w(int state) { m_chanA->write_rx(state); }
	void rxb_w(int state) { m_chanB->write_rx(state); }
	void ctsa_w(int state) { m_chanA->cts_w(state); }
	void ctsb_w(int state) { m_chanB->cts_w(state); }
	void dcda_w(int state) { m_chanA->dcd_w(state); }
	void dcdb_w(int state) { m_chanB->dcd_w(state); }
	void ria_w(int state) { m_chanA->ri_w(state); }
	void rib_w(int state) { m_chanB->ri_w(state); }
#if 0
	void rxca_w(int state) { m_chanA->rxc_w(state); }
	void rxcb_w(int state) { m_chanB->rxc_w(state); }
	void txca_w(int state) { m_chanA->txc_w(state); }
	void txcb_w(int state) { m_chanB->txc_w(state); }
	void rxtxcb_w(int state) { m_chanB->rxc_w(state); m_chanB->txc_w(state); }
#endif
	void synca_w(int state) { m_chanA->sync_w(state); }
	void syncb_w(int state) { m_chanB->sync_w(state); }

protected:
	duscc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t variant);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	// internal interrupt management
	void check_interrupts();
	void reset_interrupts();
	uint8_t modify_vector(uint8_t vect, int i, uint8_t src);
	int interrupt_priority(int index, int state);
	void trigger_interrupt(int index, int state);
	void clear_interrupt(int index, int state);
	int get_channel_index(duscc_channel *ch) const { return (ch == m_chanA) ? 0 : 1; }

	// Variants in the DUSCC family
	enum
	{
		TYPE_DUSCC       = 0x001,
		TYPE_DUSCC26562  = 0x002,
		TYPE_DUSCC26C562 = 0x004,
		TYPE_DUSCC68562  = 0x008,
		TYPE_DUSCC68C562 = 0x010,

		SET_NMOS   = TYPE_DUSCC26562 | TYPE_DUSCC68562,
		SET_CMOS   = TYPE_DUSCC26C562 | TYPE_DUSCC68C562
	};

	enum
	{
		CHANNEL_A = 0,
		CHANNEL_B
	};

	required_device<duscc_channel> m_chanA;
	required_device<duscc_channel> m_chanB;

	// internal state
#if 0
	int m_rxca;
	int m_txca;
	int m_rxcb;
	int m_txcb;
#endif

	devcb_write_line    m_out_txda_cb;
	devcb_write_line    m_out_dtra_cb;
	devcb_write_line    m_out_rtsa_cb;
	devcb_write_line    m_out_synca_cb;
	devcb_write_line    m_out_rtxca_cb;
	devcb_write_line    m_out_trxca_cb;

	devcb_write_line    m_out_txdb_cb;
	devcb_write_line    m_out_dtrb_cb;
	devcb_write_line    m_out_rtsb_cb;
	devcb_write_line    m_out_syncb_cb;
	devcb_write_line    m_out_rtxcb_cb;
	devcb_write_line    m_out_trxcb_cb;

	devcb_write_line    m_out_int_cb;

	int m_int_state[8]; // interrupt state

	int m_variant;
	uint8_t m_gsr;
	uint8_t m_ivr;
	uint8_t m_ivrm;
	uint8_t m_icr;

	enum
	{
		REG_ICR_CHB             = 0x01,
		REG_ICR_CHA             = 0x02,
		REG_ICR_VEC_MOD         = 0x04,
		REG_ICR_V2V4_MOD        = 0x08,
		REG_ICR_VEC_MODE_MASK   = 0x30,
		REG_ICR_VEC_MODE_NONE   = 0x30,
		REG_ICR_PRIO_MASK       = 0xC0,
		REG_ICR_PRIO_AHI        = 0x00,
		REG_ICR_PRIO_BHI        = 0x40,
		REG_ICR_PRIO_AINT       = 0x80,
		REG_ICR_PRIO_BINT       = 0xC0,
	};
};

// device type definition
DECLARE_DEVICE_TYPE(DUSCC,         duscc_device)
DECLARE_DEVICE_TYPE(DUSCC_CHANNEL, duscc_channel)
DECLARE_DEVICE_TYPE(DUSCC26562,    duscc26562_device)
DECLARE_DEVICE_TYPE(DUSCC26C562,   duscc26c562_device)
DECLARE_DEVICE_TYPE(DUSCC68562,    duscc68562_device)
DECLARE_DEVICE_TYPE(DUSCC68C562,   duscc68c562_device)

class duscc26562_device : public duscc_device
{
public :
	duscc26562_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class duscc26c562_device : public duscc_device
{
public :
	duscc26c562_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class duscc68562_device : public duscc_device
{
public :
	duscc68562_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class duscc68c562_device : public duscc_device
{
public :
	duscc68c562_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

#endif // MAME_MACHINE_SCNXX562_H
