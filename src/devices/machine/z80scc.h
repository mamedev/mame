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

#ifndef __Z80SCC_H__
#define __Z80SCC_H__

#include "emu.h"
#include "z80sio.h"
#include "cpu/z80/z80daisy.h"

//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_Z80SCC_ADD(_tag, _clock, _rxa, _txa, _rxb, _txb) \
	MCFG_DEVICE_ADD(_tag, Z80SCC, _clock) \
	MCFG_Z80SCC_OFFSETS(_rxa, _txa, _rxb, _txb)

#define MCFG_SCC8530_ADD(_tag, _clock, _rxa, _txa, _rxb, _txb) \
	MCFG_DEVICE_ADD(_tag, SCC8530N, _clock) \
	MCFG_Z80SCC_OFFSETS(_rxa, _txa, _rxb, _txb)

#define MCFG_Z80SCC_OFFSETS(_rxa, _txa, _rxb, _txb) \
	z80scc_device::configure_channels(*device, _rxa, _txa, _rxb, _txb);

// Port A callbacks
#define MCFG_Z80SCC_OUT_TXDA_CB(_devcb) \
	devcb = &z80scc_device::set_out_txda_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80SCC_OUT_DTRA_CB(_devcb) \
	devcb = &z80scc_device::set_out_dtra_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80SCC_OUT_RTSA_CB(_devcb) \
	devcb = &z80scc_device::set_out_rtsa_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80SCC_OUT_WRDYA_CB(_devcb) \
	devcb = &z80scc_device::set_out_wrdya_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80SCC_OUT_SYNCA_CB(_devcb) \
	devcb = &z80scc_device::set_out_synca_callback(*device, DEVCB_##_devcb);

// Port B callbacks
#define MCFG_Z80SCC_OUT_TXDB_CB(_devcb) \
	devcb = &z80scc_device::set_out_txdb_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80SCC_OUT_DTRB_CB(_devcb) \
	devcb = &z80scc_device::set_out_dtrb_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80SCC_OUT_RTSB_CB(_devcb) \
	devcb = &z80scc_device::set_out_rtsb_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80SCC_OUT_WRDYB_CB(_devcb) \
	devcb = &z80scc_device::set_out_wrdyb_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80SCC_OUT_SYNCB_CB(_devcb) \
	devcb = &z80scc_device::set_out_syncb_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80SCC_OUT_INT_CB(_devcb) \
	devcb = &z80scc_device::set_out_int_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80SCC_OUT_RXDRQA_CB(_devcb) \
	devcb = &z80scc_device::set_out_rxdrqa_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80SCC_OUT_TXDRQA_CB(_devcb) \
	devcb = &z80scc_device::set_out_txdrqa_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80SCC_OUT_RXDRQB_CB(_devcb) \
	devcb = &z80scc_device::set_out_rxdrqb_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80SCC_OUT_TXDRQB_CB(_devcb) \
	devcb = &z80scc_device::set_out_txdrqb_callback(*device, DEVCB_##_devcb);


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
	z80scc_channel(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

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
	UINT8 do_sccreg_rr0();
	UINT8 do_sccreg_rr1();
	UINT8 do_sccreg_rr2();
	UINT8 do_sccreg_rr3();
	UINT8 do_sccreg_rr4();
	UINT8 do_sccreg_rr5();
	UINT8 do_sccreg_rr6();
	UINT8 do_sccreg_rr7();
	UINT8 do_sccreg_rr8();
	UINT8 do_sccreg_rr9();
	UINT8 do_sccreg_rr10();
	UINT8 do_sccreg_rr11();
	UINT8 do_sccreg_rr12();
	UINT8 do_sccreg_rr13();
	UINT8 do_sccreg_rr14();
	UINT8 do_sccreg_rr15();

	// write register handlers
	void do_sccreg_wr0(UINT8 data);
	void do_sccreg_wr0_resets(UINT8 data);
	void do_sccreg_wr1(UINT8 data);
	void do_sccreg_wr2(UINT8 data);
	void do_sccreg_wr3(UINT8 data);
	void do_sccreg_wr4(UINT8 data);
	void do_sccreg_wr5(UINT8 data);
	void do_sccreg_wr6(UINT8 data);
	void do_sccreg_wr7(UINT8 data);
	void do_sccreg_wr8(UINT8 data);
	void do_sccreg_wr9(UINT8 data);
	void do_sccreg_wr10(UINT8 data);
	void do_sccreg_wr11(UINT8 data);
	void do_sccreg_wr12(UINT8 data);
	void do_sccreg_wr13(UINT8 data);
	void do_sccreg_wr14(UINT8 data);
	void do_sccreg_wr15(UINT8 data);

	UINT8 control_read();
	void control_write(UINT8 data);

	UINT8 data_read();
	void data_write(UINT8 data);

	void receive_data(UINT8 data);
	void m_rx_fifo_rp_step();
	UINT8 m_rx_fifo_rp_data();

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
	// read registers     enum
	UINT8 m_rr0; // REG_RR0_STATUS
	UINT8 m_rr1; // REG_RR1_SPEC_RCV_COND
	UINT8 m_rr2; // REG_RR2_INTERRUPT_VECT
	UINT8 m_rr3; // REG_RR3_INTERUPPT_PEND
	UINT8 m_rr4; // REG_RR4_WR4_OR_RR0
	UINT8 m_rr5; // REG_RR5_WR5_OR_RR0
	UINT8 m_rr6; // REG_RR6_LSB_OR_RR2
	UINT8 m_rr7; // REG_RR7_MSB_OR_RR3
	UINT8 m_rr8; // REG_RR8_RECEIVE_DATA
	UINT8 m_rr9; //  REG_RR9_WR3_OR_RR13
	UINT8 m_rr10; // REG_RR10_MISC_STATUS
	UINT8 m_rr11; // REG_RR11_WR10_OR_RR15
	UINT8 m_rr12; // REG_RR12_LO_TIME_CONST
	UINT8 m_rr13; // REG_RR13_HI_TIME_CONST
	UINT8 m_rr14; // REG_RR14_WR7_OR_R10
	UINT8 m_rr15; // REG_RR15_WR15_EXT_STAT

		// write registers    enum
	UINT8 m_wr0; // REG_WR0_COMMAND_REGPT
	UINT8 m_wr1; // REG_WR1_INT_DMA_ENABLE
	UINT8 m_wr2; // REG_WR2_INT_VECTOR
	UINT8 m_wr3; // REG_WR3_RX_CONTROL
	UINT8 m_wr4; // REG_WR4_RX_TX_MODES
	UINT8 m_wr5; // REG_WR5_TX_CONTROL
	UINT8 m_wr6; // REG_WR6_SYNC_OR_SDLC_A
	UINT8 m_wr7; // REG_WR7_SYNC_OR_SDLC_F
	UINT8 m_wr8;  // REG_WR8_TRANSMIT_DATA
	UINT8 m_wr9;  // REG_WR9_MASTER_INT_CTRL
	UINT8 m_wr10; // REG_WR10_MSC_RX_TX_CTRL
	UINT8 m_wr11; // REG_WR11_CLOCK_MODES
	UINT8 m_wr12; // REG_WR12_LO_BAUD_GEN
	UINT8 m_wr13; // REG_WR13_HI_BAUD_GEN
	UINT8 m_wr14; // REG_WR14_MISC_CTRL
	UINT8 m_wr15; // REG_WR15_EXT_ST_INT_CTRL


protected:
	enum
	{
		INT_TRANSMIT = 0,
		INT_EXTERNAL = 1,
		INT_RECEIVE  = 2,
				INT_SPECIAL  = 3
	};

	// Read registers
	enum
	{
		REG_RR0_STATUS      = 0, // SIO
		REG_RR1_SPEC_RCV_COND   = 1, // SIO
		REG_RR2_INTERRUPT_VECT  = 2, // SIO
		REG_RR3_INTERUPPT_PEND  = 3,
		REG_RR4_WR4_OR_RR0  = 4,
		REG_RR5_WR5_OR_RR0  = 5,
		REG_RR6_LSB_OR_RR2  = 6,
		REG_RR7_MSB_OR_RR3  = 7,
		REG_RR8_RECEIVE_DATA    = 8,
		REG_RR9_WR3_OR_RR13 = 9,
		REG_RR10_MISC_STATUS    = 10,
		REG_RR11_WR10_OR_RR15   = 11,
		REG_RR12_LO_TIME_CONST  = 12,
		REG_RR13_HI_TIME_CONST  = 13,
		REG_RR14_WR7_OR_R10 = 14,
		REG_RR15_WR15_EXT_STAT  = 15
	};

	// Write registers
	enum
	{
		REG_WR0_COMMAND_REGPT   = 0, // SIO
		REG_WR1_INT_DMA_ENABLE  = 1, // SIO
		REG_WR2_INT_VECTOR  = 2, // SIO
		REG_WR3_RX_CONTROL  = 3, // SIO
		REG_WR4_RX_TX_MODES = 4, // SIO
		REG_WR5_TX_CONTROL  = 5, // SIO
		REG_WR6_SYNC_OR_SDLC_A  = 6, // SIO
		REG_WR7_SYNC_OR_SDLC_F  = 7, // SIO
		REG_WR8_TRANSMIT_DATA   = 8,
		REG_WR9_MASTER_INT_CTRL = 9,
		REG_WR10_MSC_RX_TX_CTRL = 10,
		REG_WR11_CLOCK_MODES    = 11,
		REG_WR12_LO_BAUD_GEN    = 12,
		REG_WR13_HI_BAUD_GEN    = 13,
		REG_WR14_MISC_CTRL  = 14,
		REG_WR15_EXT_ST_INT_CTRL= 15
	};

	enum
	{
		RR0_RX_CHAR_AVAILABLE     = 0x01, // SIO bit
		RR0_ZC            = 0x02, // SCC bit
		RR0_TX_BUFFER_EMPTY   = 0x04, // SIO
		RR0_DCD           = 0x08, // SIO
		RR0_RI            = 0x10, // DART bit?  TODO: investigate function and remove
		RR0_SYNC_HUNT         = 0x10, // SIO bit, not supported
		RR0_CTS           = 0x20, // SIO bit
		RR0_TX_UNDERRUN       = 0x40, // SIO bit, not supported
		RR0_BREAK_ABORT       = 0x80  // SIO bit, not supported
	};

	enum
	{
		RR1_ALL_SENT          = 0x01, // SIO/SCC bit
		RR1_RESIDUE_CODE_MASK     = 0x0e, // SIO/SCC bits, not supported
		RR1_PARITY_ERROR      = 0x10, // SIO/SCC bits
		RR1_RX_OVERRUN_ERROR      = 0x20, // SIO/SCC bits
		RR1_CRC_FRAMING_ERROR     = 0x40, // SIO/SCC bits
		RR1_END_OF_FRAME      = 0x80  // SIO/SCC bits, not supported
	};

	enum
	{                     // TODO: overload SIO functionality
		RR2_INT_VECTOR_MASK   = 0xff, // SCC channel A, SIO channel B (special case)
		RR2_INT_VECTOR_V1     = 0x02, // SIO (special case) /SCC Channel B
		RR2_INT_VECTOR_V2     = 0x04, // SIO (special case) /SCC Channel B
		RR2_INT_VECTOR_V3     = 0x08  // SIO (special case) /SCC Channel B
	};

	enum
	{
		RR3_CHANB_EXT_IP      = 0x01, // SCC IP pending registers
		RR3_CHANB_TX_IP       = 0x02, //  only read in Channel A (for both channels)
		RR3_CHANB_RX_IP       = 0x04, //  channel B return all zero
		RR3_CHANA_EXT_IP      = 0x08,
		RR3_CHANA_TX_IP       = 0x10,
		RR3_CHANA_RX_IP       = 0x20
	};

	enum // Universal Bus WR0 commands for 85X30
	{
		WR0_REGISTER_MASK     = 0x07,
		WR0_COMMAND_MASK      = 0x38, // COMMANDS
		WR0_NULL          = 0x00, // 0 0 0
		WR0_POINT_HIGH        = 0x08, // 0 0 1
		WR0_RESET_EXT_STATUS      = 0x10, // 0 1 0
		WR0_SEND_ABORT        = 0x18, // 0 1 1
		WR0_ENABLE_INT_NEXT_RX    = 0x20, // 1 0 0
		WR0_RESET_TX_INT      = 0x28, // 1 0 1
		WR0_ERROR_RESET       = 0x30, // 1 1 0
		WR0_RESET_HIGHEST_IUS     = 0x38, // 1 1 1
		WR0_CRC_RESET_CODE_MASK   = 0xc0, // RESET
		WR0_CRC_RESET_NULL    = 0x00, // 0 0
		WR0_CRC_RESET_RX      = 0x40, // 0 1
		WR0_CRC_RESET_TX      = 0x80, // 1 0
		WR0_CRC_RESET_TX_UNDERRUN = 0xc0  // 1 1
	};

	enum // ZBUS WR0 commands or 80X30
	{
		WR0_Z_COMMAND_MASK      = 0x38, // COMMANDS
		WR0_Z_NULL_1        = 0x00, // 0 0 0
		WR0_Z_NULL_2        = 0x08, // 0 0 1
				WR0_Z_RESET_EXT_STATUS  = 0x10, // 0 1 0
		WR0_Z_SEND_ABORT        = 0x18, // 0 1 1
		WR0_Z_ENABLE_INT_NEXT_RX    = 0x20, // 1 0 0
		WR0_Z_RESET_TX_INT      = 0x28, // 1 0 1
		WR0_Z_ERROR_RESET       = 0x30, // 1 1 0
		WR0_Z_RESET_HIGHEST_IUS = 0x38, // 1 1 1
		WR0_Z_SHIFT_MASK        = 0x03, // SHIFT mode SDLC chan B
		WR0_Z_SEL_SHFT_LEFT     = 0x02, // 1 0
		WR0_Z_SEL_SHFT_RIGHT    = 0x03  // 1 1
	};

	enum
	{
		WR1_EXT_INT_ENABLE    = 0x01,
		WR1_TX_INT_ENABLE     = 0x02,
		WR1_PARITY_IS_SPEC_COND   = 0x04,
		WR1_RX_INT_MODE_MASK      = 0x18,
		WR1_RX_INT_DISABLE    = 0x00,
		WR1_RX_INT_FIRST      = 0x08,
		WR1_RX_INT_ALL_PARITY     = 0x10, // not supported
		WR1_RX_INT_ALL        = 0x18,
		WR1_WRDY_ON_RX_TX     = 0x20, // not supported
		WR1_WRDY_FUNCTION     = 0x40, // not supported
		WR1_WRDY_ENABLE       = 0x80  // not supported
	};

	enum
	{
		WR3_RX_ENABLE         = 0x01,
		WR3_SYNC_CHAR_LOAD_INHIBIT= 0x02, // not supported
		WR3_ADDRESS_SEARCH_MODE   = 0x04, // not supported
		WR3_RX_CRC_ENABLE     = 0x08, // not supported
		WR3_ENTER_HUNT_PHASE      = 0x10, // not supported
		WR3_AUTO_ENABLES      = 0x20,
		WR3_RX_WORD_LENGTH_MASK   = 0xc0,
		WR3_RX_WORD_LENGTH_5      = 0x00,
		WR3_RX_WORD_LENGTH_7      = 0x40,
		WR3_RX_WORD_LENGTH_6      = 0x80,
		WR3_RX_WORD_LENGTH_8      = 0xc0
	};

	enum
	{
		WR4_PARITY_ENABLE     = 0x01,
		WR4_PARITY_EVEN       = 0x02,
		WR4_STOP_BITS_MASK    = 0x0c,
		WR4_STOP_BITS_1       = 0x04,
		WR4_STOP_BITS_1_5     = 0x08, // not supported
		WR4_STOP_BITS_2       = 0x0c,
		WR4_SYNC_MODE_MASK    = 0x30, // not supported
		WR4_SYNC_MODE_8_BIT   = 0x00, // not supported
		WR4_SYNC_MODE_16_BIT      = 0x10, // not supported
		WR4_SYNC_MODE_SDLC    = 0x20, // not supported
		WR4_SYNC_MODE_EXT     = 0x30, // not supported
		WR4_CLOCK_RATE_MASK   = 0xc0,
		WR4_CLOCK_RATE_X1     = 0x00,
		WR4_CLOCK_RATE_X16    = 0x40,
		WR4_CLOCK_RATE_X32    = 0x80,
		WR4_CLOCK_RATE_X64    = 0xc0
	};

	enum
	{
		WR5_TX_CRC_ENABLE     = 0x01, // not supported
		WR5_RTS           = 0x02,
		WR5_CRC16         = 0x04, // not supported
		WR5_TX_ENABLE         = 0x08,
		WR5_SEND_BREAK        = 0x10,
		WR5_TX_WORD_LENGTH_MASK   = 0x60,
		WR5_TX_WORD_LENGTH_5      = 0x00,
		WR5_TX_WORD_LENGTH_6      = 0x40,
		WR5_TX_WORD_LENGTH_7      = 0x20,
		WR5_TX_WORD_LENGTH_8      = 0x60,
		WR5_DTR           = 0x80
	};

	/* SCC specifics */
	enum
	{
		WR9_CMD_MASK      = 0xC0,
		WR9_CMD_NORESET   = 0x00,
		WR9_CMD_CHNB_RESET    = 0x40,
		WR9_CMD_CHNA_RESET    = 0x80,
		WR9_CMD_HW_RESET      = 0xC0,
		WR9_BIT_VIS       = 0x01,
		WR9_BIT_NV        = 0x02,
		WR9_BIT_DLC       = 0x04,
		WR9_BIT_MIE       = 0x08,
		WR9_BIT_SHSL      = 0x10,
		WR9_BIT_IACK      = 0x20
	};

	enum
	{
		WR11_RCVCLK_TYPE      = 0x80,
		WR11_RCVCLK_SRC_MASK  = 0x60, // RCV CLOCK
		WR11_RCVCLK_SRC_RTXC  = 0x00, //  0 0
		WR11_RCVCLK_SRC_TRXC  = 0x20, //  0 1
		WR11_RCVCLK_SRC_BR    = 0x40, //  1 0
		WR11_RCVCLK_SRC_DPLL  = 0x60, //  1 1
		WR11_TRACLK_SRC_MASK  = 0x18, // TRA CLOCK
		WR11_TRACLK_SRC_RTXC  = 0x00, //  0 0
		WR11_TRACLK_SRC_TRXC  = 0x08, //  0 1
		WR11_TRACLK_SRC_BR    = 0x10, //  1 0
		WR11_TRACLK_SRC_DPLL  = 0x18, //  1 1
		WR11_TRXC_DIRECTION   = 0x04,
		WR11_TRXSRC_SRC_MASK  = 0x03, // TRXX CLOCK
		WR11_TRXSRC_SRC_XTAL  = 0x00, //  0 0
		WR11_TRXSRC_SRC_TRA   = 0x01, //  0 1
		WR11_TRXSRC_SRC_BR    = 0x02, //  1 0
		WR11_TRXSRC_SRC_DPLL  = 0x03  //  1 1
	};

	enum
	{
		WR14_DPLL_CMD_MASK    = 0xe0, // Command
		WR14_CMD_NULL     = 0x00, // 0 0 0
		WR14_CMD_ESM      = 0x20, // 0 0 1
		WR14_CMD_RMC      = 0x40, // 0 1 0
		WR14_CMD_DISABLE_DPLL = 0x60, // 0 1 1
		WR14_CMD_SS_BGR   = 0x80, // 1 0 0
		WR14_CMD_SS_RTXC      = 0xa0, // 1 0 1
		WR14_CMD_SET_FM   = 0xc0, // 1 1 0
		WR14_CMD_SET_NRZI     = 0xe0  // 1 1 1
	};

	void update_serial();
	void set_dtr(int state);
	void set_rts(int state);

	int get_clock_mode();
	void update_rts();
	stop_bits_t get_stop_bits();
	int get_rx_word_length();
	int get_tx_word_length();

	// receiver state
	UINT8 m_rx_data_fifo[8];    // receive data FIFO
	UINT8 m_rx_error_fifo[8];   // receive error FIFO
	UINT8 m_rx_error;       // current receive error
	//int m_rx_fifo     // receive FIFO pointer
		int m_rx_fifo_rp;       // receive FIFO read pointer
	int m_rx_fifo_wp;       // receive FIFO write pointer
	int m_rx_fifo_sz;       // receive FIFO size

	int m_rx_clock;     // receive clock pulse count
	int m_rx_first;     // first character received
	int m_rx_break;     // receive break condition
	UINT8 m_rx_rr0_latch;   // read register 0 latched

	int m_rxd;
	int m_ri;           // ring indicator latch
	int m_cts;          // clear to send latch
	int m_dcd;          // data carrier detect latch

	// transmitter state
	UINT8 m_tx_data;        // transmit data register
	int m_tx_clock;     // transmit clock pulse count

	int m_dtr;          // data terminal ready
	int m_rts;          // request to send

	// synchronous state
	UINT16 m_sync;      // sync character

	int m_index;
	z80scc_device *m_uart;

	// SCC specifics
	int m_ph;           // Point high command to access regs 08-0f
	UINT8 m_zc;
};


// ======================> z80scc_device

class z80scc_device :  public device_t
		,public device_z80daisy_interface
{
	friend class z80scc_channel;

public:
	// construction/destruction
	z80scc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT32 variant, const char *shortname, const char *source);
	z80scc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_out_txda_callback(device_t &device, _Object object) { return downcast<z80scc_device &>(device).m_out_txda_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_dtra_callback(device_t &device, _Object object) { return downcast<z80scc_device &>(device).m_out_dtra_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_rtsa_callback(device_t &device, _Object object) { return downcast<z80scc_device &>(device).m_out_rtsa_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_wrdya_callback(device_t &device, _Object object) { return downcast<z80scc_device &>(device).m_out_wrdya_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_synca_callback(device_t &device, _Object object) { return downcast<z80scc_device &>(device).m_out_synca_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_txdb_callback(device_t &device, _Object object) { return downcast<z80scc_device &>(device).m_out_txdb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_dtrb_callback(device_t &device, _Object object) { return downcast<z80scc_device &>(device).m_out_dtrb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_rtsb_callback(device_t &device, _Object object) { return downcast<z80scc_device &>(device).m_out_rtsb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_wrdyb_callback(device_t &device, _Object object) { return downcast<z80scc_device &>(device).m_out_wrdyb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_syncb_callback(device_t &device, _Object object) { return downcast<z80scc_device &>(device).m_out_syncb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_int_callback(device_t &device, _Object object) { return downcast<z80scc_device &>(device).m_out_int_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_rxdrqa_callback(device_t &device, _Object object) { return downcast<z80scc_device &>(device).m_out_rxdrqa_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_txdrqa_callback(device_t &device, _Object object) { return downcast<z80scc_device &>(device).m_out_txdrqa_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_rxdrqb_callback(device_t &device, _Object object) { return downcast<z80scc_device &>(device).m_out_rxdrqb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_txdrqb_callback(device_t &device, _Object object) { return downcast<z80scc_device &>(device).m_out_txdrqb_cb.set_callback(object); }

	static void configure_channels(device_t &device, int rxa, int txa, int rxb, int txb)
	{
		z80scc_device &dev = downcast<z80scc_device &>(device);
		dev.m_rxca = rxa;
		dev.m_txca = txa;
		dev.m_rxcb = rxb;
		dev.m_txcb = txb;
	}

	DECLARE_READ8_MEMBER( cd_ba_r );
	DECLARE_WRITE8_MEMBER( cd_ba_w );
	DECLARE_READ8_MEMBER( ba_cd_r );
	DECLARE_WRITE8_MEMBER( ba_cd_w );

	/* Definitions moved to z80scc.c for enhencements */
	DECLARE_READ8_MEMBER( da_r );  // { return m_chanA->data_read(); }
	DECLARE_WRITE8_MEMBER( da_w ); // { m_chanA->data_write(data); }
	DECLARE_READ8_MEMBER( db_r );  // { return m_chanB->data_read(); }
	DECLARE_WRITE8_MEMBER( db_w ); // { m_chanB->data_write(data); }

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
	UINT8 modify_vector(UINT8 vect, int i, UINT8 src);
	void trigger_interrupt(int index, int state);
	int get_channel_index(z80scc_channel *ch) { return (ch == m_chanA) ? 0 : 1; }

		// Variants in the SCC family
	enum
	{
		TYPE_Z80SCC   = 0x001,
		TYPE_SCC8030  = 0x002,
		TYPE_SCC80C30 = 0x004,
		TYPE_SCC80230 = 0x008,
		TYPE_SCC8530  = 0x010,
		TYPE_SCC85C30 = 0x020,
		TYPE_SCC85230 = 0x040,
		TYPE_SCC85233 = 0x080,
		TYPE_SCC8523L = 0x100
	};

#define SET_NMOS   ( z80scc_device::TYPE_SCC8030  | z80scc_device::TYPE_SCC8530 )
#define SET_CMOS   ( z80scc_device::TYPE_SCC80C30 | z80scc_device::TYPE_SCC85C30 )
#define SET_ESCC   ( z80scc_device::TYPE_SCC80230 | z80scc_device::TYPE_SCC85230 | z80scc_device::TYPE_SCC8523L )
#define SET_EMSCC    z80scc_device::TYPE_SCC85233
#define SET_Z80X30 ( z80scc_device::TYPE_SCC8030  | z80scc_device::TYPE_SCC80C30 | z80scc_device::TYPE_SCC80230 )
#define SET_Z85X3X ( z80scc_device::TYPE_SCC8530  | z80scc_device::TYPE_SCC85C30 | z80scc_device::TYPE_SCC85230 \
				| z80scc_device::TYPE_SCC8523L | z80scc_device::TYPE_SCC85233 )

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

	int m_int_state[6]; // interrupt state

	int m_variant;
	UINT8 m_wr0_ptrbits;
};

class scc8030_device : public z80scc_device
{
public :
	scc8030_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class scc80C30_device : public z80scc_device
{
public :
	scc80C30_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class scc80230_device : public z80scc_device
{
public :
	scc80230_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class scc8530_device : public z80scc_device
{
public :
	scc8530_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class scc85C30_device : public z80scc_device
{
public :
	scc85C30_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class scc85230_device : public z80scc_device
{
public :
	scc85230_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class scc85233_device : public z80scc_device
{
public :
	scc85233_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class scc8523L_device : public z80scc_device
{
public :
	scc8523L_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

// device type definition
extern const device_type Z80SCC;
extern const device_type Z80SCC_CHANNEL;
extern const device_type SCC8030;
extern const device_type SCC80C30;
extern const device_type SCC80230;
extern const device_type SCC8530N; // remove trailing N when 8530scc.c is fully replaced and removed
extern const device_type SCC85C30;
extern const device_type SCC85230;
extern const device_type SCC85233;
extern const device_type SCC8523L;

#endif // __Z80SCC_H__
