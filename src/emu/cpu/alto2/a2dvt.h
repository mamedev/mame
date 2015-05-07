// license:???
// copyright-holders:???
/*****************************************************************************
 *
 *   Xerox AltoII display vertical task (DVT)
 *
 *   Copyright Juergen Buchmueller <pullmoll@t-online.de>
 *
 *   Licenses: MAME, GPLv2
 *
 *****************************************************************************/
#ifdef  ALTO2_DEFINE_CONSTANTS

#else   // ALTO2_DEFINE_CONSTANTS
#ifndef _A2DVT_H_
#define _A2DVT_H_

//! F2 functions for display vertical task
enum {
	f2_dvt_evenfield    = f2_task_10            //!< f2 10: load even field
};

void f1_early_dvt_block();                      //!< F1 func: disable the display word task
void activate_dvt();                            //!< called by the CPU when the display vertical task becomes active
void init_dvt(int task = task_dvt);             //!< initialize the display vertical task
void exit_dvt();                                //!< deinitialize the display vertical task
void reset_dvt();                               //!< reset the display vertical task
#endif  // _A2DVT_H_
#endif  // ALTO2_DEFINE_CONSTANTS
