// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII disk controller block
 *
 *****************************************************************************/
#ifdef  ALTO2_DEFINE_CONSTANTS

#else   // ALTO2_DEFINE_CONSTANTS
#ifndef _A2DISK_H_
#define _A2DISK_H_

diablo_hd_device* m_drive[2];       //!< two diablo_hd_device drives

//! disk controller context
struct {
	UINT8 drive;                    //!< selected drive from KADDR[14] (written to data out with SENDADR)
	UINT16 kaddr;                   //!< A[0-15] disk hardware address (sector, cylinder, head, drive, restore)
	UINT16 kadr;                    //!< C[0-15] with read/write/check modes for header, label and data
	UINT16 kstat;                   //!< S[0-15] disk status
	UINT16 kcom;                    //!< disk command (5 bits kcom[1-5])
	UINT8 krecno;                   //!< record number (2 bits indexing header, label, data, -/-)
	UINT8 egate;                    //!< current erase gate signal to the DIABLO hd
	UINT8 wrgate;                   //!< current write gate signal to the DIABLO hd
	UINT8 rdgate;                   //!< current read gate signal to the DIABLO hd
	UINT32 shiftin;                 //!< input shift register
	UINT32 shiftout;                //!< output shift register
	UINT32 datain;                  //!< disk data in latch
	UINT32 dataout;                 //!< disk data out latch
	UINT8 krwc;                     //!< read/write/check for current record
	UINT8 kfer;                     //!< disk fatal error signal state
	UINT8 wdtskena;                 //!< disk word task enable (active low)
	UINT8 wddone;                   //!< previous state of WDDONE
	UINT8 wdinit0;                  //!< disk word task init at the early microcycle
	UINT8 wdinit;                   //!< disk word task init at the late microcycle
	UINT8 strobe;                   //!< strobe (still) active
	emu_timer* strobon_timer;       //!< set strobe on timer
	UINT8 bitclk;                   //!< current bitclk state (either crystal clock, or rdclk from the drive)
#if USE_BITCLK_TIMER
	emu_timer* bitclk_timer;        //!< bit clock timer
#else
	int bitclk_time[2];             //!< per drive time in clocks per bit
#endif
	UINT8 datin;                    //!< current datin from the drive
	UINT8 bitcount;                 //!< bit counter
	UINT8 carry;                    //!< carry output of the bitcounter
	UINT8 seclate;                  //!< sector late (monoflop output)
	emu_timer* seclate_timer;       //!< sector late timer
	UINT8 seekok;                   //!< seekok state (SKINC' & LAI' & ff_44a.Q')
	UINT8 ok_to_run;                //!< ok to run signal (set to 1 some time after reset)
	emu_timer* ok_to_run_timer;     //!< ok to run timer
	UINT8 ready_mf31a;              //!< ready monoflop 31a
	emu_timer* ready_timer;         //!< ready timer
	UINT8 seclate_mf31b;            //!< seclate monoflop 31b
	jkff_t ff_21a;                  //!< JK flip-flop 21a (sector task)
	jkff_t ff_21a_old;              //!< -"- previous state
	jkff_t ff_21b;                  //!< JK flip-flop 21b (sector task)
	jkff_t ff_22a;                  //!< JK flip-flop 22a (sector task)
	jkff_t ff_22b;                  //!< JK flip-flop 22b (sector task)
	jkff_t ff_43b;                  //!< JK flip-flop 43b (word task)
	jkff_t ff_53a;                  //!< JK flip-flop 53a (word task)
	jkff_t ff_43a;                  //!< JK flip-flop 43a (word task)
	jkff_t ff_53b;                  //!< brief JK flip-flop 53b (word task)
	jkff_t ff_44a;                  //!< JK flip-flop 44a (LAI' clocked)
	jkff_t ff_44b;                  //!< JK flip-flop 44b (CKSUM)
	jkff_t ff_45a;                  //!< JK flip-flop 45a (ready latch)
	jkff_t ff_45b;                  //!< JK flip-flop 45b (seqerr latch)
}   m_dsk;

jkff_t m_sysclka0[4];               //!< simulate previous sysclka
jkff_t m_sysclka1[4];               //!< simulate current sysclka
jkff_t m_sysclkb0[4];               //!< simulate previous sysclkb
jkff_t m_sysclkb1[4];               //!< simulate current sysclkb

void kwd_timing(int bitclk, int datin, int block);  //!< disk word timing
TIMER_CALLBACK_MEMBER( disk_seclate );          //!< timer callback to take away the SECLATE pulse (monoflop)
TIMER_CALLBACK_MEMBER( disk_ok_to_run );        //!< timer callback to take away the OK TO RUN pulse (reset)
TIMER_CALLBACK_MEMBER( disk_strobon );          //!< timer callback to pulse the STROBE' signal to the drive
TIMER_CALLBACK_MEMBER( disk_ready_mf31a );      //!< timer callback to change the READY monoflop 31a
#if USE_BITCLK_TIMER
TIMER_CALLBACK_MEMBER( disk_bitclk );           //!< callback to update the disk controller with a new bitclk
#else
void disk_bitclk(void *ptr, int arg);           //!< function to update the disk controller with a new bitclk
#endif
void disk_block(int task);                      //!< called if one of the disk tasks (task_kwd or task_ksec) blocks
void bs_early_read_kstat();                     //!< bus source: bus driven by disk status register KSTAT
void bs_early_read_kdata();                     //!< bus source: bus driven by disk data register KDATA input
void f1_late_strobe();                          //!< F1 func: initiates a disk seek
void f1_late_load_kstat();                      //!< F1 func: load disk status register
void f1_late_load_kdata();                      //!< F1 func: load data out register, or the disk address register
void f1_late_increcno();                        //!< F1 func: advances shift registers holding KADR
void f1_late_clrstat();                         //!< F1 func: reset all error latches
void f1_late_load_kcom();                       //!< F1 func: load the KCOM register from bus
void f1_late_load_kadr();                       //!< F1 func: load the KADR register from bus
void f2_late_init();                            //!< F2 func: branch on disk word task active and init
void f2_late_rwc();                             //!< F2 func: branch on read/write/check state of the current record
void f2_late_recno();                           //!< F2 func: branch on the current record number by a lookup table
void f2_late_xfrdat();                          //!< F2 func: branch on the data transfer state
void f2_late_swrnrdy();                         //!< F2 func: branch on the disk ready signal
void f2_late_nfer();                            //!< f2_nfer late: branch on the disk fatal error condition
void f2_late_strobon();                         //!< f2_strobon late: branch on the seek busy status
void init_disk();                               //!< initialize the disk controller
void exit_disk();                               //!< deinitialize the disk controller
void reset_disk();                              //!< reset the disk controller
#endif  // _A2DISK_H_
#endif  // ALTO2_DEFINE_CONSTANTS
