// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII display vertical task (DVT)
 *
 *****************************************************************************/
#ifdef  ALTO2_DEFINE_CONSTANTS

#else   // ALTO2_DEFINE_CONSTANTS
#ifndef MAME_CPU_ALTO2_A2DVT_H
#define MAME_CPU_ALTO2_A2DVT_H

//! F2 functions for display vertical task
enum {
	f2_dvt_evenfield    = f2_task_10            //!< f2 10: load even field
};

//! F1 func: Disable the display word task.
void f1_early_dvt_block();

//! Called by the CPU when the display vertical task becomes active.
void activate_dvt();

//! Initialize the display vertical task.
void init_dvt(int task = task_dvt);

//! Deinitialize the display vertical task.
void exit_dvt();

//! Reset the display vertical task.
void reset_dvt();
#endif  // MAME_CPU_ALTO2_A2DVT_H
#endif  // ALTO2_DEFINE_CONSTANTS
