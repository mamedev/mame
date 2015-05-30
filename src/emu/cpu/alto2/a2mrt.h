// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII memory refresh task (MRT)
 *
 *****************************************************************************/
#ifdef  ALTO2_DEFINE_CONSTANTS

#else   // ALTO2_DEFINE_CONSTANTS
#ifndef _A2MRT_H_
#define _A2MRT_H_
void f1_early_mrt_block();                      //!< F1 func: block the display word task
void activate_mrt();                            //!< called by the CPU when MRT becomes active
void init_mrt(int task = task_mrt);             //!< initialize the memory refresh task
void exit_mrt();                                //!< deinitialize the memory refresh task
void reset_mrt();                               //!< reset the memory refresh task
#endif // _A2MRT_H_
#endif  // ALTO2_DEFINE_CONSTANTS
