/*****************************************************************************
 *
 *   Xerox AltoII ethernet task (ETHER)
 *
 *   Copyright © Jürgen Buchmüller <pullmoll@t-online.de>
 *
 *   Licenses: MAME, GPLv2
 *
 *****************************************************************************/
#ifdef  ALTO2_DEFINE_CONSTANTS

#define ALTO2_ETHER_FIFO_SIZE	16              //!< number of words in the ethernet FIFO

#else   // ALTO2_DEFINE_CONSTANTS
#ifndef _A2ETHER_H_
#define _A2ETHER_H_
//! BUS source for ethernet task
enum {
	bs_ether_eidfct		= bs_task_3				//!< ethernet task: Ethernet input data function
};

//! F1 functions for ethernet task
enum {
	f1_ether_eilfct		= f1_task_13,			//!< f1 (1011): ethernet input look function
	f1_ether_epfct		= f1_task_14,			//!< f1 (1100): ethernet post function
	f1_ether_ewfct		= f1_task_15			//!< f1 (1101): ethernet countdown wakeup function
};

//! F2 functions for ethernet task
enum {
	f2_ether_eodfct		= f2_task_10,			//!< f2 (1000): ethernet output data function
	f2_ether_eosfct		= f2_task_11,			//!< f2 (1001): ethernet output start function
	f2_ether_erbfct		= f2_task_12,			//!< f2 (1010): ethernet reset branch function
	f2_ether_eefct		= f2_task_13,			//!< f2 (1011): ethernet end of transmission function
	f2_ether_ebfct		= f2_task_14,			//!< f2 (1100): ethernet branch function
	f2_ether_ecbfct		= f2_task_15,			//!< f2 (1101): ethernet countdown branch function
	f2_ether_eisfct		= f2_task_16			//!< f2 (1110): ethernet input start function
												//!< f2 (1111): undefined
};

/**
 * @brief BPROMs P3601-1; 256x4; enet.a41 "PE1" and enet.a42 "PE2"
 *
 * Phase encoder
 *
 * a41: P3601-1; 256x4; "PE1"
 * a42: P3601-1; 256x4; "PE2"
 *
 * PE1/PE2 inputs
 * ----------------
 * A0  (5) OUTGO
 * A1  (6) XDATA
 * A2  (7) OSDATAG
 * A3  (4) XCLOCK
 * A4  (3) OCNTR0
 * A5  (2) OCNTR1
 * A6  (1) OCNTR2
 * A7 (15) OCNTR3
 *
 * PE1 outputs
 * ----------------
 * D0 (12) OCNTR0
 * D1 (11) OCNTR1
 * D2 (10) OCNTR2
 * D3  (9) OCNTR3
 *
 * PE2 outputs
 * ----------------
 * D0 (12) n.c.
 * D1 (11) to OSLOAD flip flop J and K'
 * D2 (10) XDATA
 * D3  (9) XCLOCK
 */
UINT8* m_ether_a41;
UINT8* m_ether_a42;

/**
 * @brief BPROM; P3601-1; 265x4 enet.a49 "AFIFO"
 *
 * Perhaps try with the contents of the display FIFO, as it is
 * the same type and the display FIFO has the same size.
 *
 * FIFO control
 *
 * a49: P3601-1; 256x4; "AFIFO"
 *
 * inputs
 * ----------------
 * A0  (5) fifo_wr[0]
 * A1  (6) fifo_wr[1]
 * A2  (7) fifo_wr[2]
 * A3  (4) fifo_wr[3]
 * A4  (3) fifo_rd[0]
 * A5  (2) fifo_rd[1]
 * A6  (1) fifo_rd[2]
 * A7 (15) fifo_rd[3]
 *
 * outputs active low
 * ----------------------------
 * D0 (12) BE'    (buffer empty)
 * D1 (11) BNE'   (buffer next empty ?)
 * D2 (10) BNNE'  (buffer next next empty ?)
 * D3  (9) BF'    (buffer full)
 */
UINT8* m_ether_a49;

static const int m_duckbreath_sec = 15;			//!< send duckbreath every 15 seconds

struct {
	UINT16 fifo[ALTO2_ETHER_FIFO_SIZE];			//!< FIFO buffer
	UINT16 fifo_rd;								//!< FIFO input pointer
	UINT16 fifo_wr;								//!< FIFO output pointer
	UINT16 status;								//!< status word
	UINT32 rx_crc;								//!< receiver CRC
	UINT32 tx_crc;								//!< transmitter CRC
	UINT32 rx_count;							//!< received words count
	UINT32 tx_count;							//!< transmitted words count
	emu_timer* tx_timer;						//!< transmitter timer
	int duckbreath;								//!< if non-zero, interval in seconds at which to broadcast the duckbreath
}	m_eth;

TIMER_CALLBACK_MEMBER( rx_duckbreath );			//!< HACK: pull the next word from the duckbreath in the fifo
TIMER_CALLBACK_MEMBER( tx_packet );				//!< transmit data from the FIFO to <nirvana for now>
void eth_wakeup();								//!< check for the various reasons to wakeup the Ethernet task
void eth_startf();								//!< start input or output depending on m_bus
void bs_early_eidfct();							//!< bus source: Ethernet input data function
void f1_early_eth_block();						//!< F1 func: block the Ether task
void f1_early_eilfct();							//!< F1 func: Ethernet input look function
void f1_early_epfct();							//!< F1 func: Ethernet post function
void f1_late_ewfct();							//!< F1 func: Ethernet countdown wakeup function
void f2_late_eodfct();							//!< F2 func: Ethernet output data function
void f2_late_eosfct();							//!< F2 func: Ethernet output start function
void f2_late_erbfct();							//!< F2 func: Ethernet reset branch function
void f2_late_eefct();							//!< F2 func: Ethernet end of transmission function
void f2_late_ebfct();							//!< F2 func: Ethernet branch function
void f2_late_ecbfct();							//!< F2 func: Ethernet countdown branch function
void f2_late_eisfct();							//!< F2 func: Ethernet input start function
void activate_eth();							//!< called by the CPU when the Ethernet task becomes active
void init_ether(int task);						//!< 007 initialize ethernet task
void exit_ether();								//!< deinitialize ethernet task
#endif // _A2ETHER_H_
#endif	// ALTO2_DEFINE_CONSTANTS
