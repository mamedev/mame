// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII display horizontal task (DHT)
 *
 *****************************************************************************/
#ifdef  ALTO2_DEFINE_CONSTANTS

#else   // ALTO2_DEFINE_CONSTANTS
#ifndef _A2DHT_H_
#define _A2DHT_H_

//! F2 functions for display horizontal task
enum {
	f2_dht_evenfield    = f2_task_10,           //!< f2 10: load even field
	f2_dht_setmode      = f2_task_11            //!< f2 11: set mode
};

void f1_early_dht_block();                      //!< F1 func: disable the display word task
void f2_late_dht_setmode();                     //!< F2 func: set the next scanline's mode inverse and half clock and branch
void activate_dht();                            //!< called by the CPU when the display horizontal task becomes active
void init_dht(int task = task_dht);             //!< initialize display horizontal task
void exit_dht();                                //!< deinitialize display horizontal task
void reset_dht();                               //!< reset the display horizontal task
#endif // _A2DHT_H_
#endif  // ALTO2_DEFINE_CONSTANTS
