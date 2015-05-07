// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII display word task (DWT)
 *
 *****************************************************************************/
#ifdef  ALTO2_DEFINE_CONSTANTS

#else   // ALTO2_DEFINE_CONSTANTS
#ifndef _A2DWT_H_
#define _A2DWT_H_

//! F2 functions for display word task
enum {
	f2_dwt_load_ddr     = f2_task_10            //!< f2 10: load display data register
};

void f1_early_dwt_block();                      //!< F1 func: block the display word task
void f2_late_dwt_load_ddr();                    //!< F2 func: load the display data register
void init_dwt(int task = task_dwt);             //!< initialize the display word task
void exit_dwt();                                //!< deinitialize the display word task
void reset_dwt();                               //!< reset the display word task
#endif  // _A2DWT_H_
#endif  // ALTO2_DEFINE_CONSTANTS
