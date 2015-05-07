// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII emulator task (EMU)
 *
 *****************************************************************************/
#ifdef  ALTO2_DEFINE_CONSTANTS

#else   // ALTO2_DEFINE_CONSTANTS
#ifndef _A2EMU_H_
#define _A2EMU_H_

//! BUS source for emulator task
enum {
	bs_emu_read_sreg    = bs_task_3,            //!< bus source: read S register
	bs_emu_load_sreg    = bs_task_4             //!< bus source: load S register from BUS
};

//! F1 functions for emulator task
enum {
	f1_emu_swmode       = f1_task_10,           //!< f1 (1000): switch mode; branch to ROM/RAM microcode
	f1_emu_wrtram       = f1_task_11,           //!< f1 (1001): write microcode RAM cycle
	f1_emu_rdram        = f1_task_12,           //!< f1 (1010): read microcode RAM cycle
	f1_emu_load_rmr     = f1_task_13,           //!< f1 (1011): load reset mode register
												//!< f1 (1100): undefined
	f1_emu_load_esrb    = f1_task_15,           //!< f1 (1101): load extended S register bank
	f1_emu_rsnf         = f1_task_16,           //!< f1 (1110): read serial number (Ethernet ID)
	f1_emu_startf       = f1_task_17            //!< f1 (1111): start I/O hardware (Ethernet)
};

//! F2 functions for emulator task
enum {
	f2_emu_busodd       = f2_task_10,           //!< f2 (1000): branch on bus odd
	f2_emu_magic        = f2_task_11,           //!< f2 (1001): magic shifter (MRSH 1: shifter[15]=T[0], MLSH 1: shifter[015])
	f2_emu_load_dns     = f2_task_12,           //!< f2 (1010): do novel shift (RSH 1: shifter[15]=XC, LSH 1: shifer[0]=XC)
	f2_emu_acdest       = f2_task_13,           //!< f2 (1011): destination accu
	f2_emu_load_ir      = f2_task_14,           //!< f2 (1100): load instruction register and branch
	f2_emu_idisp        = f2_task_15,           //!< f2 (1101): load instruction displacement and branch
	f2_emu_acsource     = f2_task_16            //!< f2 (1110): source accu
												//!< f2 (1111): undefined
};

struct {
	UINT16 ir;                                  //!< emulator instruction register
	UINT8 skip;                                 //!< emulator skip
	UINT8 cy;                                   //!< emulator carry
}   m_emu;
void bs_early_emu_disp();                       //!< bus source: drive bus by IR[8-15], possibly sign extended
void f1_early_emu_block();                      //!< F1 func: block task
void f1_late_emu_load_rmr();                    //!< F1 func: load the reset mode register
void f1_late_emu_load_esrb();                   //!< F1 func: load the extended S register bank from BUS[12-14]
void f1_early_rsnf();                           //!< F1 func: drive the bus from the Ethernet node ID
void f1_early_startf();                         //!< F1 func: defines commands for for I/O hardware, including Ethernet
void f2_late_busodd();                          //!< F2 func: branch on odd bus
void f2_late_magic();                           //!< F2 func: shift and use T
void f2_early_load_dns();                       //!< F2 func: modify RESELECT with DstAC = (3 - IR[3-4])
void f2_late_load_dns();                        //!< F2 func: do novel shifts
void f2_early_acdest();                         //!< F2 func: modify RSELECT with DstAC = (3 - IR[3-4])
void bitblt_info();                             //!< debug bitblt opcode
void f2_late_load_ir();                         //!< F2 func: load instruction register IR and branch on IR[0,5-7]
void f2_late_idisp();                           //!< F2 func: branch on: arithmetic IR_SH, others PROM ctl2k_u3[IR[1-7]]
void f2_early_acsource();                       //!< F2 func: modify RSELECT with SrcAC = (3 - IR[1-2])
void f2_late_acsource();                        //!< F2 func: branch on arithmetic IR_SH, others PROM ctl2k_u3[IR[1-7]]
void init_emu(int task = task_emu);             //!< initialize the emulator task
void exit_emu();                                //!< deinitialize the emulator task
void reset_emu();                               //!< reset the emulator task
#endif // _A2EMU_H_
#endif  // ALTO2_DEFINE_CONSTANTS
