// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************

    MPCC Multi-Protocol Communications Controller emulation

****************************************************************************
                   _____   _____
          UDS*   1|*    \_/     |48 IACK*
        DTACK*   2|             |47 LDS*
          RxD    3|             |46 DTC*
          D10    4|             |45 D9                _____   _____
          DTR*   5|             |44 CS*        A0   1|*    \_/     |40 IACK*
          DSR*   6|             |43 DACK*   DTACK*  2|             |39 DS*
          DCD*   7|             |42 GND       RxD*  3|             |38 DTC*
          D11    8|             |41 D0        DTR*  4|             |37 CS*
         RDSR*   9|             |40 D8        DSR*  5|             |36 DACK*
           A1   10|             |39 D1        DCD*  6|             |35 GND
          GND   11|             |38 D2       RDSR*  7|             |34 D0
           A4   12|             |37 D3         A1   8|             |33 D1
           A2   13|   R68561    |36 D4        GND   9|   R68560    |32 D2
           A3   14|   R68561A   |35 D5         A4  10|   R68560A   |31 D3
          RxC   15|             |34 D6         A2  11|             |30 D4
          D12   16|             |33 D15        A3  12|             |29 D5
          TxC   17|             |32 D7        RxC  13|             |28 D6
         BCLK   18|             |31 RESET*    TxC  14|             |27 D7
        EXTAL   19|             |30 CTS*     BCLK  15|             |26 RESET*
         XTAL   20|             |29 Vcc     EXTAL  16|             |25 CTS*
          D13   21|             |28 D14      XTAL  17|             |24 Vcc
          R/W*  22|             |27 DONE*     R/W* 18|             |23 DONE*
          IRQ*  23|             |26 TxD       IRQ* 19|             |22 TxD
          RTS*  24|_____________|25 TDSR*     RTS* 20|_____________|21 TDSR*
                  16 bit data bus                     8 bit data bus
                Also in 68 pin PLCE                Also in 44 pin PLCE

***************************************************************************/

#ifndef MAME_MACHINE_68561MPCC_H
#define MAME_MACHINE_68561MPCC_H

#include "diserial.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************
class mpcc_device : public device_t, public device_serial_interface
{
public:
	// construction/destruction
	mpcc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void configure_clocks(int rxc, int txc)
	{
		m_rxc = rxc;
		m_txc = txc;
	}

	auto out_txd_cb() { return m_out_txd_cb.bind(); }
	auto out_dtr_cb() { return m_out_dtr_cb.bind(); }
	auto out_rts_cb() { return m_out_rts_cb.bind(); }
	auto out_rtxc_cb() { return m_out_rtxc_cb.bind(); }
	auto out_trxc_cb() { return m_out_trxc_cb.bind(); }
	auto out_int_cb() { return m_out_int_cb.bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	// interrupt acknowledge
	uint8_t iack(offs_t offset); // declared but not defined?

	/* Callbacks to be called by others for signals driven by connected devices */
	void write_rx(int state);
	void cts_w(int state);
	void dsr_w(int state);
	void dcd_w(int state);
	void rxc_w(int state) {} // { m_chanA->rxc_w(state); }
	void txc_w(int state) {} // { m_chanA->txc_w(state); }

protected:
	mpcc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t variant);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_serial_interface overrides
	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_callback() override;
	virtual void rcv_complete() override;

	// serial device setup helpers
	uint32_t get_brg_rate();
	uint32_t get_tx_rate();
	uint32_t get_rx_rate();
	uint32_t get_clock_div();
	uint32_t get_word_length();
	stop_bits_t get_stop_bits();
	parity_t get_parity();
	void update_serial();

	/*
	 * Interrupts
	 */
	void check_interrupts();
	void reset_interrupts();
	void update_interrupts(int source);
	void trigger_interrupt(int source);
	enum
	{
		INT_TX,         // TX int category, used for update of SR
		INT_TX_TDRA,    // Tx char available
		INT_TX_TFC,     // Tx frame complete
		INT_TX_TUNRN,   // Tx underrun detected
		INT_TX_TFERR,   // Tx frame error detected
		INT_RX,         // RX int category, used for update of SR
		INT_RX_RDA,     // Rx interrupt on Receiver Data Available
		INT_RX_EOF,     // Rx interrupt on End of frame
		INT_RX_CPERR,   // Rx interrupt on CRC or Parity error
		INT_RX_FRERR,   // Rx interrupt on Frame error
		INT_RX_ROVRN,   // Rx interrupt on Receiver overrun
		INT_RX_RAB,     // Rx interrupt on Abort/Break
		INT_SR,         // SR int category, used for update of SR
		INT_SR_CTS,     // Serial interface interrupt on CTS asserted
		INT_SR_DSR,     // Serial interface interrupt on DSR asserted
		INT_SR_DCD,     // Serial interface interrupt on DCD asserted
	};

	enum
	{
		RX_INT_PRIO = 0x00, // Highest interrupt priority
		TX_INT_PRIO = 0x01,
		SR_INT_PRIO = 0x02  // Lowest interrupt priority
	};

	enum
	{
		INT_NONE = 0x00, // No interrupts
		INT_REQ  = 0x01, // Interrupt requested
		INT_ACK  = 0x02  // Interrupt acknowledged
	};

	// Variants in the MPCC family
	enum
	{
		TYPE_MPCC       = 0x001,
		TYPE_MPCC68560  = 0x002,
		TYPE_MPCC68560A = 0x004,
		TYPE_MPCC68561  = 0x008,
		TYPE_MPCC68561A = 0x010,
		SET_TYPE_A      = TYPE_MPCC68560A | TYPE_MPCC68561A
	};

	// State variables
	uint32_t m_irq;
	uint32_t m_variant;
	uint32_t m_rxc;
	uint32_t m_txc;
	uint32_t m_brg_rate;
	uint32_t m_rcv;
	uint32_t m_rxd;
	uint32_t m_tra;
	uint32_t m_int_state[3]; // Three priority levels Rx, Tx and Serial interface

	// Callbacks
	devcb_write_line    m_out_txd_cb;
	devcb_write_line    m_out_dtr_cb;
	devcb_write_line    m_out_rts_cb;
	devcb_write_line    m_out_rtxc_cb;
	devcb_write_line    m_out_trxc_cb;

	devcb_write_line    m_out_int_cb;

	/*
	 *  Register handling
	 */
	// RSR - Rx Status Register
	uint8_t m_rsr;
	uint8_t do_rsr();
	void do_rsr(uint8_t data);
	enum
	{
		REG_RSR_RDA     = 0x80, // Rx Data available
		REG_RSR_EOF     = 0x40, // End of frame detected (BOP and BSC modes)
		REG_RSR_RHW     = 0x20, // Odd number of frame data bytes reeived in 16 bit mode.
		REG_RSR_CPERR   = 0x10, // CRC or parity error detected
		REG_RSR_FRERR   = 0x08, // Frame error detected
		REG_RSR_ROVRN   = 0x04, // Rx overrun detected
		REG_RSR_RAB     = 0x02, // Rx Abort break detected
		REG_RSR_RIDLE   = 0x01, // Rx idle detcted (15+ high consecutive bits accounted for)
	};

	// RCR - Rx Control Register
	uint8_t m_rcr;
	uint8_t do_rcr();
	void do_rcr(uint8_t data);
	enum
	{
		REG_RCR_RDSREN = 0x40, // Rx Data Service Request Enable (DMA)
		REG_RCR_DONEEN = 0x20, // DONE output enable
		REG_RCR_RSYNEN = 0x10, // RSYNEN output enable
		REG_RCR_STRSYN = 0x08, // STRIP SYN character (COP mode)
		REG_RCR_RABTEN = 0x02, // Receiver Abort Enable (BOP mode)
		REG_RCR_RRES   = 0x01, // Receiver Reset command/Enable
	};

	uint8_t m_rdr;
	uint8_t do_rdr();
	// TODO: investigate if 4 x 16 bit wide FIFO is needed for 16 bit mode
	util::fifo<uint16_t, 8> m_rx_data_fifo;

	// RIVNR - Rx Interrupt Vector Number Register
	uint8_t m_rivnr;
	uint8_t do_rivnr();
	void do_rivnr(uint8_t data);

	// RIER - Rx Interrupt Enable Register
	uint8_t m_rier;
	uint8_t do_rier();
	void do_rier(uint8_t data);
	enum {
		REG_RIER_RDA    = 0x80, // Rx interrupt on Receiver Data Available
		REG_RIER_EOF    = 0x40, // Rx interrupt on End of frame
		REG_RIER_CPERR  = 0x10, // Rx interrupt on CRC or Parity error
		REG_RIER_FRERR  = 0x08, // Rx interrupt on Frame error
		REG_RIER_ROVRN  = 0x04, // Rx interrupt on Receiver overrun
		REG_RIER_RAB    = 0x02, // Rx interrupt on Abort/Break
	};

	// TSR - Tx Status Register
	uint8_t m_tsr;
	uint8_t do_tsr();
	void do_tsr(uint8_t data);
	enum
	{
		REG_TSR_TDRA  = 0x80, // Tx Fifo Full or not
		REG_TSR_TFC   = 0x40, // Tx Frame Complete
		REG_TSR_TUNRN = 0x04, // Tx underrun
		REG_TSR_TFERR = 0x02, // Tx Frame Error
	};

	// TCR - Tx Control Register
	uint8_t m_tcr;
	uint8_t do_tcr();
	void do_tcr(uint8_t data);
	enum
	{
		REG_TCR_TEN     = 0x80, // Tx enable
		REG_TCR_TDSREN  = 0x40, // DMA enable
		REG_TCR_TICS    = 0x20, // Tx Idle Char Select, 'A' variant differs
		REG_TCR_THW     = 0x10, // Indicates that last 16 bit word has only 8 bits, in 16 bits mode only
		REG_TCR_TLAST   = 0x08, // Indicates the last byte to be written in to TDR (BOP, BCS or COP)
		REG_TCR_TSYN    = 0x04, // SYN enable (BCS or COP)
		REG_TCR_TABT    = 0x02, // Abort command (BOP)
		REG_TCR_TRES    = 0x01, // Tx Reset command
	};

	// TDR - Tx Data Register (write only)
	uint8_t m_tdr;
	void do_tdr(uint8_t data);
	// TODO: investigate if 4 x 16 bit wide FIFO is needed for 16 bit mode
	util::fifo<uint8_t, 8> m_tx_data_fifo;

	// TIVNR - Tx Interrupt Vector Number Register
	uint8_t m_tivnr;
	uint8_t do_tivnr();
	void do_tivnr(uint8_t data);

	// TIER - Tx Interrupt Enable Register
	uint8_t m_tier;
	uint8_t do_tier();
	void do_tier(uint8_t data);
	enum
	{
		REG_TIER_TDRA   = 0x80, // TX Character available interrupt
		REG_TIER_TFC    = 0x40, // TX Frame complete interrupt
		REG_TIER_TUNRN  = 0x04, // TX Underrun interrupt
		REG_TIER_TFERR  = 0x02, // TX Frame error interrupt
	};

	// SISR - Serial Interface Status Register
	uint8_t m_sisr;
	uint8_t do_sisr();
	void do_sisr(uint8_t data);
	enum
	{
		REG_SISR_CTST   = 0x80, // Clear To Send Transition Status
		REG_SISR_DSRT   = 0x40, // Data Set Ready Transition Status
		REG_SISR_DCDT   = 0x20, // Data Carrier Detect Transition Status
		REG_SISR_CTSLVL = 0x10, // Clear To Send Level
		REG_SISR_DSRLVL = 0x08, // Data Set Ready Level
		REG_SISR_DCDLVL = 0x04, // Data Carrier Detect Level
	};

	// SICR - Serial Interface Control Register
	uint8_t m_sicr;
	uint8_t do_sicr();
	void do_sicr(uint8_t data);
	enum
	{
		REG_SICR_RTSLVL = 0x80, // RTS level
		REG_SICR_DTRLVL = 0x40, // DTR level
		REG_SICR_ECHO   = 0x04, // Echo Mode
		REG_SICR_TEST   = 0x02, // Test Mode
	};

	uint8_t m_sivnr;

	// SIER - Serial interface Interrupt Enable
	uint8_t m_sier;
	uint8_t do_sier();
	void do_sier(uint8_t data);
	enum
	{
		REG_SIER_CTS    = 0x80,
		REG_SIER_DSR    = 0x40,
		REG_SIER_DCD    = 0x20,
	};

	// PSR1 Protocol Selection Register 1
	uint8_t m_psr1;
	uint8_t do_psr1();
	void do_psr1(uint8_t data);
	enum
	{
		REG_PSR1_ADRZ       = 0x08, // Zero adress option (BOP) (A models only)
		REG_PSR1_IPARS      = 0x04, // IPARS option (COP)
		REG_PSR1_CTLEX      = 0x02, // Control field width 8/16 bit (BOP) (A models only)
		REG_PSR1_ADDEX      = 0x01, // Address extend option (BOP) (A models only)
	};

	// PSR2 Protocol Selection Register 2
	uint8_t m_psr2;
	uint8_t do_psr2();
	void do_psr2(uint8_t data);
	enum
	{
		REG_PSR2_WDBYT      = 0x80, // 8/16 bit data bus selector
		REG_PSR2_STP_MSK    = 0x60, // Stop bits selection field
		REG_PSR2_STP_1      = 0x00, // 1   Stop bits
		REG_PSR2_STP_1_5    = 0x20, // 1.5 Stop bits
		REG_PSR2_STP_2      = 0x40, // 2   Stop bits
		REG_PSR2_CHLN_MSK   = 0x18, // char len selection field
		REG_PSR2_CHLN_5     = 0x00, // 5 bit char len
		REG_PSR2_CHLN_6     = 0x08, // 6 bit char len
		REG_PSR2_CHLN_7     = 0x10, // 7 bit char len
		REG_PSR2_CHLN_8     = 0x18, // 8 bit char len
		REG_PSR2_PSEL_MSK   = 0x07, // Protocol selection field
		REG_PSR2_PSEL_BOPP  = 0x00, // Protocol selection BOP Primary
		REG_PSR2_PSEL_BOPS  = 0x01, // Protocol selection BOP Secondary
		REG_PSR2_PSEL_RSV   = 0x02, // Protocol selection Reserved
		REG_PSR2_PSEL_COP   = 0x03, // Protocol selection COP
		REG_PSR2_PSEL_BCSE  = 0x04, // Protocol selection BCS EBCDIC
		REG_PSR2_PSEL_BCSA  = 0x05, // Protocol selection BCS ASCII
		REG_PSR2_PSEL_ASCII = 0x06, // Protocol selection ASYNC
		REG_PSR2_PSEL_ISOC  = 0x07, // Protocol selection ISOC
	};

	uint8_t m_ar1;
	uint8_t m_ar2;


	// BRDR1 - Baud Rate Divisor Register 1 (Lo)
	uint8_t m_brdr1;
	uint8_t do_brdr1();
	void do_brdr1(uint8_t data);

	// BRDR2 - Baud Rate Divisor Register 2 (Hi)
	uint8_t m_brdr2;
	uint8_t do_brdr2();
	void do_brdr2(uint8_t data);

	// CCR - Clock Control Register
	uint8_t m_ccr;
	uint8_t do_ccr();
	void do_ccr(uint8_t data);
	enum
	{
		REG_CCR_PSCDIV      = 0x10, // Internal prescaler Divider x2 or x3
		REG_CCR_TCLO        = 0x08, // TxC input/output selection
		REG_CCR_RCLKIN      = 0x04, // RxC from internal/external source selection
		REG_CCR_CLKDIV_MSK  = 0x03, // External RxC prescaler Divider
		REG_CCR_CLKDIV_X1   = 0x00, // x1  - ISOC only
		REG_CCR_CLKDIV_X16  = 0x01, // x16 - ASYNC only
		REG_CCR_CLKDIV_X32  = 0x02, // x32 - ASYNC only
		REG_CCR_CLKDIV_X64  = 0x03, // x64 - ASYNC only
	};

	// ECR - Error Control Regsiter
	uint8_t m_ecr;
	uint8_t do_ecr();
	void do_ecr(uint8_t data);
	enum
	{
		REG_ECR_PAREN   = 0x80, // Parity Enable
		REG_ECR_ODDPAR  = 0x40, // Odd/Even Parity
		REG_ECR_CFCRC   = 0x08, // CRC Enable
		REG_ECR_CRCPRE  = 0x04, // CRC Preset 0 (BSC) or 1 (BOP)
		REG_ECR_CRCSEL_MSK  = 0x03, // CRC Polynominal Selection Mask
		REG_ECR_CRCSEL_V41  = 0x00, // CCITT V.41 (BOP) CRC Polynomial
		REG_ECR_CRCSEL_C16  = 0x01, // CRC-16 (BSC) CRC Polynomial
		REG_ECR_CRCSEL_VRC  = 0x02, // VRC/LRC (BSC, ASCII, non-transp) CRC Polynomial
	};

};

class mpcc68560_device  : public mpcc_device
{
public:
	mpcc68560_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, int rxc, int txc)
		: mpcc68560_device(mconfig, tag, owner, clock)
	{
		configure_clocks(rxc, txc);
	}

	mpcc68560_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class mpcc68560a_device  : public mpcc_device
{
public:
	mpcc68560a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, int rxc, int txc)
		: mpcc68560a_device(mconfig, tag, owner, clock)
	{
		configure_clocks(rxc, txc);
	}

	mpcc68560a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class mpcc68561_device  : public mpcc_device
{
public:
	mpcc68561_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, int rxc, int txc)
		: mpcc68561_device(mconfig, tag, owner, clock)
	{
		configure_clocks(rxc, txc);
	}

	mpcc68561_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class mpcc68561a_device  : public mpcc_device
{
public:
	mpcc68561a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, int rxc, int txc)
		: mpcc68561a_device(mconfig, tag, owner, clock)
	{
		configure_clocks(rxc, txc);
	}

	mpcc68561a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// device type definition
DECLARE_DEVICE_TYPE(MPCC,       mpcc_device)
DECLARE_DEVICE_TYPE(MPCC68560,  mpcc68560_device)
DECLARE_DEVICE_TYPE(MPCC68560A, mpcc68560a_device)
DECLARE_DEVICE_TYPE(MPCC68561,  mpcc68561_device)
DECLARE_DEVICE_TYPE(MPCC68561A, mpcc68561a_device)

#endif // MAME_MACHINE_68561MPCC_H
