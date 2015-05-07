// license:???
// copyright-holders:???
/*****************************************************************************
 *
 *   Xerox AltoII parity task (PART)
 *
 *   Copyright Juergen Buchmueller <pullmoll@t-online.de>
 *
 *   Licenses: MAME, GPLv2
 *
 *****************************************************************************/
#ifdef  ALTO2_DEFINE_CONSTANTS

#else   // ALTO2_DEFINE_CONSTANTS
#ifndef _A2PART_H_
#define _A2PART_H_
void activate_part();
void init_part(int task = task_part);           //!< initialize the parity task
void exit_part();                               //!< deinitialize the parity task
void reset_part();                              //!< reset the parity task
#endif // _A2PART_H_
#endif  // ALTO2_DEFINE_CONSTANTS
