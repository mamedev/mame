// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII parity task (PART)
 *
 *****************************************************************************/
#ifdef  ALTO2_DEFINE_CONSTANTS

#else   // ALTO2_DEFINE_CONSTANTS
#ifndef MAME_CPU_ALTO2_A2PART_H
#define MAME_CPU_ALTO2_A2PART_H
void activate_part();
void init_part(int task = task_part);           //!< initialize the parity task
void exit_part();                               //!< deinitialize the parity task
void reset_part();                              //!< reset the parity task
#endif // MAME_CPU_ALTO2_A2PART_H
#endif  // ALTO2_DEFINE_CONSTANTS
