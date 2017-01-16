// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************

    MPCC Multi-Protocol Communications Controller emulation

****************************************************************************
                   _____   _____
                 1|*    \_/     |48
                 2|             |47
                 3|             |46
                 4|             |45                   _____   _____
                 5|             |44                 1|*    \_/     |40
                 6|             |43                 2|             |39
                 7|             |42                 3|             |38
                 8|             |41                 4|             |37
                 9|             |40                 5|             |36
                10|             |39                 6|             |35
                11|             |38                 7|             |34
                12|             |37                 8|             |33
                13|  SCN26562   |36                 9|             |32
                14|  SCN26C562  |35                10|             |31
                15|             |34                11|   Z8530     |30
                16|             |33                12|   Z85C30    |29
                17|             |32                13|   Z85230    |28
                18|             |31                14|             |27
                19|             |30                15|             |26
                20|             |29                16|             |25
                21|             |28                17|             |24
                22|             |27                18|             |23
                23|             |26                19|             |22
                24|_____________|25                20|_____________|21

***************************************************************************/

#ifndef MPCC68561_H
#define MPCC68561_H

#include "emu.h"

/* Variant ADD macros - use the right one to enable the right feature set! */
#define MCFG_MPCC68560_ADD(_tag, _clock, _rx, _tx) \
	MCFG_DEVICE_ADD(_tag, MPCC68560, _clock) \
	MCFG_MPCC_CLOCK(_rx, _tx)

#define MCFG_MPCC68560A_ADD(_tag, _clock, _rx, _tx) \
	MCFG_DEVICE_ADD(_tag, MPCC68560A, _clock) \
	MCFG_MPCC_CLOCK(_rx, _tx)

#define MCFG_MPCC68561_ADD(_tag, _clock, _rx, _tx) \
	MCFG_DEVICE_ADD(_tag, MPCC68561, _clock) \
	MCFG_MPCC_CLOCK(_rx, _tx)

#define MCFG_MPCC68561A_ADD(_tag, _clock, _rx, _tx) \
	MCFG_DEVICE_ADD(_tag, MPCC68561A, _clock) \
	MCFG_MPCC_CLOCK(_rx, _tx)

/* Generic ADD macro - Avoid using it directly, see above for correct variant instead */
#define MCFG_MPCC_ADD(_tag, _clock, _rxa, _txa, _rxb, _txb) \
	MCFG_DEVICE_ADD(_tag, MPCC, _clock) \
	MCFG_MPCC_CLOCK(_rx, _tx)

/* Generic macros */
#define MCFG_MPCC_CLOCK(_rx, _tx) \
	mpcc_device::configure_clocks(*device, _rx, _tx);

/* Callbacks to be called by us for signals driven by the MPCC */
#define MCFG_MPCC_OUT_TXD_CB(_devcb) \
	devcb = &mpcc_device::set_out_txd_callback(*device, DEVCB_##_devcb);

#define MCFG_MPCC_OUT_DTR_CB(_devcb) \
	devcb = &mpcc_device::set_out_dtr_callback(*device, DEVCB_##_devcb);

#define MCFG_MPCC_OUT_RTS_CB(_devcb) \
	devcb = &mpcc_device::set_out_rts_callback(*device, DEVCB_##_devcb);

#define MCFG_MPCC_OUT_TRXC_CB(_devcb) \
	devcb = &mpcc_device::set_out_trxc_callback(*device, DEVCB_##_devcb);

#define MCFG_MPCC_OUT_RTXC_CB(_devcb) \
	devcb = &mpcc_device::set_out_rtxc_callback(*device, DEVCB_##_devcb);

#define MCFG_MPCC_OUT_INT_CB(_devcb) \
	devcb = &mpcc_device::set_out_int_callback(*device, DEVCB_##_devcb);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************
class mpcc_device :  public device_t,
		public device_serial_interface
{
public:
	// construction/destruction
	mpcc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, uint32_t variant, const char *shortname, const char *source);
	mpcc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class _Object> static devcb_base &set_out_txd_callback(device_t &device, _Object object) { return downcast<mpcc_device &>(device).m_out_txd_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_dtr_callback(device_t &device, _Object object) { return downcast<mpcc_device &>(device).m_out_dtr_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_rts_callback(device_t &device, _Object object) { return downcast<mpcc_device &>(device).m_out_rts_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_rtxc_callback(device_t &device, _Object object) { return downcast<mpcc_device &>(device).m_out_rtxc_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_trxc_callback(device_t &device, _Object object) { return downcast<mpcc_device &>(device).m_out_trxc_cb.set_callback(object); }

	template<class _Object> static devcb_base &set_out_int_callback(device_t &device, _Object object) { return downcast<mpcc_device &>(device).m_out_int_cb.set_callback(object); }

	static void configure_clocks(device_t &device, int rxc, int txc)
	{
		mpcc_device &dev = downcast<mpcc_device &>(device);
		dev.m_rxc = rxc;
		dev.m_txc = txc;
	}

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	// interrupt acknowledge
	DECLARE_READ8_MEMBER( iack );

	/* Callbacks to be called by others for signals driven by connected devices */
	DECLARE_WRITE_LINE_MEMBER( write_rx ); // bit transitions from serial device
	DECLARE_WRITE_LINE_MEMBER( cts_w ) {} // { m_chanA->cts_w(state); }
	DECLARE_WRITE_LINE_MEMBER( dcd_w ) {} // { m_chanA->dcd_w(state); }
	DECLARE_WRITE_LINE_MEMBER( rxc_w ) {} // { m_chanA->rxc_w(state); }
	DECLARE_WRITE_LINE_MEMBER( txc_w ) {} // { m_chanA->txc_w(state); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_serial_interface overrides
	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_callback() override;
	virtual void rcv_complete() override;

	/*
	 * Interrupts
	 */
	void check_interrupts();
	void reset_interrupts();
	void trigger_interrupt(int source);
	enum
	{
		INT_TX_TDRA,	// Tx char available
		INT_TX_TFC,		// Tx frame complete
		INT_TX_TUNRN,	// Tx underrun detected
		INT_TX_TFERR,	// Tx frame error detected
		INT_RX_RDA,		// Rx interrupt on Receiver Data Available
		INT_RX_EOF,		// Rx interrupt on End of frame
		INT_RX_CPERR,	// Rx interrupt on CRC or Parity error
		INT_RX_FRERR,	// Rx interrupt on Frame error
		INT_RX_ROVRN,	// Rx interrupt on Receiver overrun
		INT_RX_RAB,		// Rx interrupt on Abort/Break
		INT_SR_CTS, 	// Serial interface interrupt on CTS asserted
		INT_SR_DSR, 	// Serial interface interrupt on DSR asserted
		INT_SR_DCD, 	// Serial interface interrupt on DCD asserted
	};

	enum
	{
		RX_INT_PRIO = 0x00,	// Highest interrupt priority
		TX_INT_PRIO = 0x01,
		SR_INT_PRIO = 0x02	// Lowest interrupt priority
	};

	enum
	{
		INT_REQ = 0x01,	// Interrupt requested
		INT_ACK = 0x02	// Interrupt acknowledged
	};

	// Variants in the MPCC family
	enum
	{
		TYPE_MPCC       = 0x001,
		TYPE_MPCC68560  = 0x002,
		TYPE_MPCC68560A = 0x004,
		TYPE_MPCC68561  = 0x008,
		TYPE_MPCC68561A = 0x010,
	};

#define SET_TYPE_A ( mpcc_device::TYPE_MPCC68560A | mpcc_device::TYPE_MPCC68561A )

	// State variables
	int m_variant;
	int m_rxc;
	int m_txc;
	int m_brg_rate;
	int m_rcv;
	int m_rxd;
	int m_tra;
	int m_int_state[3]; // Three priority levels Rx, Tx and Serial interface

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
	// RSR register
	uint8_t m_rsr;
	enum
	{
		REG_RSR_RDA		= 0x80, // Rx Data available
		REG_RSR_EOF		= 0x40, // End of frame detected (BOP and BSC modes)
		REG_RSR_RHW 	= 0x20, // Odd number of frame data bytes reeived in 16 bit mode.
		REG_RSR_CPERR	= 0x10, // CRC or parity error detected
		REG_RSR_FRERR	= 0x08, // Frame error detected
		REG_RSR_ROVRN	= 0x04, // Rx overrun detected
		REG_RSR_RAB		= 0x02, // Rx Abort break detected
		REG_RSR_RIDLE	= 0x01, // Rx idle detcted (15+ high consecutive bits accounted for)
	};

	// RCR register
	uint8_t m_rcr;
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
	// TODO: investigate if 4 x 16 bit wide FIFO is needed for 16 bit mode
	util::fifo<uint8_t, 8> m_rx_data_fifo;

	uint8_t m_rivnr;

	// RIER register
	uint8_t m_rier;
	enum {
		REG_RIER_RDA	= 0x80, // Rx interrupt on Receiver Data Available
		REG_RIER_EOF	= 0x40, // Rx interrupt on End of frame
		REG_RIER_CPERR	= 0x10, // Rx interrupt on CRC or Parity error
		REG_RIER_FRERR	= 0x08, // Rx interrupt on Frame error
		REG_RIER_ROVRN	= 0x04, // Rx interrupt on Receiver overrun
		REG_RIER_RAB	= 0x02, // Rx interrupt on Abort/Break
	};

	// TSR register
	uint8_t m_tsr;
	enum
	{
		REG_TSR_TDRA  = 0x80,
		REG_TSR_TFC   = 0x40,
		REG_TSR_TUNRN = 0x04,
		REG_TSR_TFERR = 0x02,
	};

	// TCR register
	uint8_t m_tcr;
	enum
	{
		REG_TCR_TEN		= 0x80, // Tx enabled
		REG_TCR_TDSREN	= 0x40,
		REG_TCR_TICS	= 0x20, // Tx Idle Char Select, 'A' variant differs
		REG_TCR_THW		= 0x10,
		REG_TCR_TLAST	= 0x08,
		REG_TCR_TSYN	= 0x04,
		REG_TCR_TABT	= 0x02,
		REG_TCR_TRES	= 0x01,
	};

	// TDR register
	uint8_t m_tdr;
	void do_tdr_w(uint8_t data);
	// TODO: investigate if 4 x 16 bit wide FIFO is needed for 16 bit mode
	util::fifo<uint8_t, 8> m_tx_data_fifo;

	uint8_t m_tivnr;

	// TIER register
	uint8_t m_tier;
	enum
	{
		REG_TIER_TDRA	= 0x80, // TX Character available interrupt
		REG_TIER_TFC	= 0x40, // TX Frame complete interrupt
		REG_TIER_TUNRN	= 0x04, // TX Underrun interrupt
		REG_TIER_TFERR	= 0x02, // TX Frame error interrupt
	};

	uint8_t m_sisr;
	uint8_t m_sicr;
	uint8_t m_sivnr;
	uint8_t m_sier;
	uint8_t m_psr1;
	uint8_t m_psr2;
	uint8_t m_ar1;
	uint8_t m_ar2;
	uint8_t m_brdr1;
	uint8_t m_brdr2;
	uint8_t m_ccr;
	uint8_t m_ecr;
};

// device type definition
extern const device_type MPCC;
extern const device_type MPCC68560;
extern const device_type MPCC68560A;
extern const device_type MPCC68561;
extern const device_type MPCC68561A;

class mpcc68560_device  : public mpcc_device { public : mpcc68560_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock); };
class mpcc68560A_device : public mpcc_device { public : mpcc68560A_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock); };
class mpcc68561_device  : public mpcc_device { public : mpcc68561_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock); };
class mpcc68561A_device : public mpcc_device { public : mpcc68561A_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock); };

#endif // MPCC68561_H
