// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*********************************************************************

    bookkeeping.h

    Bookkeeping simple machine functions.

*********************************************************************/

#pragma once

#ifndef __BOOKKEEPING_H__
#define __BOOKKEEPING_H__



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* total # of coin counters */
#define COIN_COUNTERS           8

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> bookkeeping_manager

enum class config_type;

class bookkeeping_manager
{
public:
	// construction/destruction
	bookkeeping_manager(running_machine &machine);

	// ----- tickets -----
	// return the number of tickets dispensed
	int get_dispensed_tickets() const;

	// increment the number of dispensed tickets
	void increment_dispensed_tickets(int delta);

	// ----- coin counters -----
	// write to a particular coin counter (clocks on active high edge)
	void coin_counter_w(int num, int on);

	// return the coin count for a given coin
	int coin_counter_get_count(int num);

	// enable/disable coin lockout for a particular coin
	void coin_lockout_w(int num, int on);

	// return current lockout state for a particular coin
	int coin_lockout_get_state(int num);

	// enable/disable global coin lockout
	void coin_lockout_global_w(int on);

	// getters
	running_machine &machine() const { return m_machine; }
private:
	void config_load(config_type cfg_type, xml_data_node *parentnode);
	void config_save(config_type cfg_type, xml_data_node *parentnode);

	// internal state
	running_machine &   m_machine;                  // reference to our machine

	UINT32      m_dispensed_tickets;
	UINT32      m_coin_count[COIN_COUNTERS];
	UINT32      m_coinlockedout[COIN_COUNTERS];
	UINT32      m_lastcoin[COIN_COUNTERS];
};

#endif  /* __BOOKKEEPING_H__ */
