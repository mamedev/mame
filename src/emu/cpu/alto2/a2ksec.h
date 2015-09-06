// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII disk sector task (KSEC)
 *
 *****************************************************************************/
#ifdef  ALTO2_DEFINE_CONSTANTS

#else   // ALTO2_DEFINE_CONSTANTS
#ifndef _A2KSEC_H_
#define _A2KSEC_H_
//! BUS source for disk sector task
enum {
	bs_ksec_read_kstat  = bs_task_3,            //!< bus source: read disk status register
	bs_ksec_read_kdata  = bs_task_4             //!< bus source: read disk data register
};

//! F1 functions for disk sector task
enum {
												//!< f1 10: undefined
	f1_ksec_strobe      = f1_task_11,           //!< f1 11: strobe
	f1_ksec_load_kstat  = f1_task_12,           //!< f1 12: load kstat register
	f1_ksec_increcno    = f1_task_13,           //!< f1 13: increment record number
	f1_ksec_clrstat     = f1_task_14,           //!< f1 14: clear status register
	f1_ksec_load_kcom   = f1_task_15,           //!< f1 15: load kcom register
	f1_ksec_load_kadr   = f1_task_16,           //!< f1 16: load kadr register
	f1_ksec_load_kdata  = f1_task_17            //!< f1 17: load kdata register
};

//! F2 functions for disk sector task
enum {
	f2_ksec_init        = f2_task_10,           //!< f2 10: branches NEXT[5-9] on WDTASKACT && WDINIT
	f2_ksec_rwc         = f2_task_11,           //!< f2 11: branches NEXT[8-9] on READ/WRITE/CHECK for record
	f2_ksec_recno       = f2_task_12,           //!< f2 12: branches NEXT[8-9] on RECNO[0-1]
	f2_ksec_xfrdat      = f2_task_13,           //!< f2 13: branches NEXT[9] on !SEEKONLY
	f2_ksec_swrnrdy     = f2_task_14,           //!< f2 14: branches NEXT[9] on !SWRDY
	f2_ksec_nfer        = f2_task_15,           //!< f2 15: branches NEXT[9] on !KFER
	f2_ksec_strobon     = f2_task_16            //!< f2 16: branches NEXT[9] on STROBE
												//!< f2 17: undefined
};

void f1_early_ksec_block(void);                 //!< block ksec task
void init_ksec(int task = task_ksec);           //!< initialize the disk sector task
void exit_ksec();                               //!< deinitialize the disk sector task
void reset_ksec();                              //!< reset the disk sector task
#endif // _A2KSEC_H_
#endif  // ALTO2_DEFINE_CONSTANTS
