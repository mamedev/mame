// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII ethernet task (ETHER)
 *
 *****************************************************************************/
#ifdef  ALTO2_DEFINE_CONSTANTS

#define ALTO2_ETHER_FIFO_SIZE   16              //!< number of words in the ethernet FIFO
#define ALTO2_ETHER_PACKET_SIZE 0400            //!< size of a packet in words

#else   // ALTO2_DEFINE_CONSTANTS
#ifndef MAME_CPU_ALTO2_A2ETHER_H
#define MAME_CPU_ALTO2_A2ETHER_H
//! BUS source for ethernet task
enum {
	bs_ether_eidfct     = bs_task_3             //!< ethernet task: Ethernet input data function
};

//! F1 functions for ethernet task
enum {
	f1_ether_eilfct     = f1_task_13,           //!< f1 (1011): ethernet input look function
	f1_ether_epfct      = f1_task_14,           //!< f1 (1100): ethernet post function
	f1_ether_ewfct      = f1_task_15            //!< f1 (1101): ethernet countdown wakeup function
};

//! F2 functions for ethernet task
enum {
	f2_ether_eodfct     = f2_task_10,           //!< f2 (1000): ethernet output data function
	f2_ether_eosfct     = f2_task_11,           //!< f2 (1001): ethernet output start function
	f2_ether_erbfct     = f2_task_12,           //!< f2 (1010): ethernet reset branch function
	f2_ether_eefct      = f2_task_13,           //!< f2 (1011): ethernet end of transmission function
	f2_ether_ebfct      = f2_task_14,           //!< f2 (1100): ethernet branch function
	f2_ether_ecbfct     = f2_task_15,           //!< f2 (1101): ethernet countdown branch function
	f2_ether_eisfct     = f2_task_16            //!< f2 (1110): ethernet input start function
												//!< f2 (1111): undefined
};

std::unique_ptr<uint8_t[]> m_ether_a41;                             //!< BPROM; P3601-1; 256x4; enet.a41 "PE1"
std::unique_ptr<uint8_t[]> m_ether_a42;                             //!< BPROM; P3601-1; 256x4; enet.a42 "PE2"
std::unique_ptr<uint8_t[]> m_ether_a49;                             //!< BPROM; P3601-1; 265x4 enet.a49 "AFIFO"
enum {
	ether_a49_BE    = (1 << 0),                 //!< buffer empty
	ether_a49_BNE   = (1 << 1),                 //!< buffer next empty
	ether_a49_BNNE  = (1 << 2),                 //!< buffer next next empty
	ether_a49_BF    = (1 << 3)                  //!< buffer full
};

struct {
	uint32_t serin = 0;                         //!< serial input shift registers 74164 #37 and #33
	uint16_t fifo[ALTO2_ETHER_FIFO_SIZE] = { }; //!< FIFO buffer
	uint16_t fifo_rd = 0;                       //!< FIFO input pointer
	uint16_t fifo_wr = 0;                       //!< FIFO output pointer
	uint16_t status = 0;                        //!< status word
	uint16_t rx_crc = 0;                        //!< receiver CRC
	uint16_t tx_crc = 0;                        //!< transmitter CRC
	uint32_t rx_count = 0;                      //!< received words count
	uint32_t tx_count = 0;                      //!< transmitted words count
	std::unique_ptr<uint16_t []> rx_packet;     //!< buffer to collect received words
	std::unique_ptr<uint16_t []> tx_packet;     //!< buffer to collect transmitted words
	emu_timer *rx_timer = nullptr;              //!< receiver timer
	emu_timer *tx_timer = nullptr;              //!< transmitter timer
	jkff_t ff_10a = JKFF_0;                     //!< JK flip-flop 10a IBUSY (Sheet 13)
	jkff_t ff_10b = JKFF_0;                     //!< JK flip-flop 10b OBUSY (Sheet 13)
	jkff_t ff_21a = JKFF_0;                     //!< JK flip-flop 21a OUTON (Sheet 19)
	jkff_t ff_21b = JKFF_0;                     //!< JK flip-flop 21b COLL (Sheet 19)
	jkff_t ff_31a = JKFF_0;                     //!< JK flip-flop 31a OUTGONE (Sheet 19)
	jkff_t ff_31b = JKFF_0;                     //!< JK flip-flop 31b OEOT (Sheet 19)
	jkff_t ff_35a = JKFF_0;                     //!< JK flip-flop 35a OCMD (Sheet 7)
	jkff_t ff_35b = JKFF_0;                     //!< JK flip-flop 35b ICMD (Sheet 7)
	jkff_t ff_47a = JKFF_0;                     //!< JK flip-flop 47a ISR14 (Sheet 15)
	jkff_t ff_47b = JKFF_0;                     //!< JK flip-flop 47b ISR15 (Sheet 15)
	jkff_t ff_51a = JKFF_0;                     //!< JK flip-flop 51a EWFCT latch (Sheet 19)
	jkff_t ff_51b = JKFF_0;                     //!< JK flip-flop 51b OCDW (Sheet 19)
	jkff_t ff_52b = JKFF_0;                     //!< JK flip-flop 52b OSLOAD (Sheet 17)
	jkff_t ff_61a = JKFF_0;                     //!< JK flip-flop 61a CRCGO (Sheet 21)
	jkff_t ff_61b = JKFF_0;                     //!< JK flip-flop 61b OUTRGO (Sheet 21)
	jkff_t ff_62a = JKFF_0;                     //!< JK flip-flop 62a OUTON (Sheet 21)
	jkff_t ff_62b = JKFF_0;                     //!< JK flip-flop 62b OUTGO (Sheet 21)
	jkff_t ff_65a = JKFF_0;                     //!< JK flip-flop 65a IDL (Sheet 10)
	jkff_t ff_65b = JKFF_0;                     //!< JK flip-flop 65b IO (Sheet 10)
	jkff_t ff_69a = JKFF_0;                     //!< JK flip-flop 69a IT (Sheet 14)
	jkff_t ff_69b = JKFF_0;                     //!< JK flip-flop 69b INON (Sheet 14)
	jkff_t ff_70a = JKFF_0;                     //!< JK flip-flop 70a IMID (Sheet 14)
	jkff_t ff_70b = JKFF_0;                     //!< JK flip-flop 70b ILOC (Sheet 14)
	jkff_t ff_77a = JKFF_0;                     //!< JK flip-flop 77a WR (Sheet 10)
	jkff_t ff_77b = JKFF_0;                     //!< JK flip-flop 77b WLF (Sheet 10)
	int breath_of_life = 0;                     //!< if non-zero, interval in seconds at which to broadcast the breath-of-life
}   m_eth;

TIMER_CALLBACK_MEMBER( rx_breath_of_life );     //!< HACK: pull the next word from the breath-of-life in the fifo
TIMER_CALLBACK_MEMBER( tx_packet );             //!< transmit data from the FIFO to <nirvana for now>
void eth_wakeup();                              //!< check for the various reasons to wakeup the Ethernet task
void eth_startf();                              //!< start input or output depending on m_bus
void bs_early_eidfct();                         //!< bus source: Ethernet input data function
void f1_early_eth_block();                      //!< F1 func: block the Ether task
void f1_early_eilfct();                         //!< F1 func: Ethernet input look function
void f1_early_epfct();                          //!< F1 func: Ethernet post function
void f1_late_ewfct();                           //!< F1 func: Ethernet countdown wakeup function
void f2_late_eodfct();                          //!< F2 func: Ethernet output data function
void f2_late_eosfct();                          //!< F2 func: Ethernet output start function
void f2_late_erbfct();                          //!< F2 func: Ethernet reset branch function
void f2_late_eefct();                           //!< F2 func: Ethernet end of transmission function
void f2_late_ebfct();                           //!< F2 func: Ethernet branch function
void f2_late_ecbfct();                          //!< F2 func: Ethernet countdown branch function
void f2_late_eisfct();                          //!< F2 func: Ethernet input start function
void activate_eth();                            //!< called by the CPU when the Ethernet task becomes active
void update_sysclk(int sysclk);                 //!< update all JK flip-flops for one cycle of SYSCLK
void update_rclk(int rclk);                     //!< update all JK flip-flops for one cycle of RCLK
void update_tclk(int tclk);                     //!< update all JK flip-flops for one cycle of TCLK
void init_ether(int task = task_ether);         //!< initialize the ethernet task
void exit_ether();                              //!< deinitialize the ethernet task
void reset_ether();                             //!< reset the ethernet task
#endif // MAME_CPU_ALTO2_A2ETHER_H
#endif  // ALTO2_DEFINE_CONSTANTS
