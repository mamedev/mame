// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII cursor task (CURT)
 *
 *****************************************************************************/
#ifdef  ALTO2_DEFINE_CONSTANTS

#else   // ALTO2_DEFINE_CONSTANTS
#ifndef _A2CURT_H_
#define _A2CURT_H_

//! F2 functions for cursor task
enum {
	f2_curt_load_xpreg  = f2_task_10,           //!< f2 10: load x position register
	f2_curt_load_csr    = f2_task_11            //!< f2 11: load cursor shift register
};

void f1_early_curt_block();                     //!< f1_curt_block early: disable the cursor task and set the curt_blocks flag
void f2_late_load_xpreg();                      //!< f2_load_xpreg late: load the x position register from BUS[6-15]
void f2_late_load_csr();                        //!< f2_load_csr late: load the cursor shift register from BUS[0-15]
void activate_curt();                           //!< curt_activate: called by the CPU when the cursor task becomes active
void init_curt(int task = task_curt);           //!< initialize cursor task
void exit_curt();                               //!< deinitialize cursor task
void reset_curt();                              //!< reset cursor task
#endif // _A2CURT_H_
#endif  // ALTO2_DEFINE_CONSTANTS
