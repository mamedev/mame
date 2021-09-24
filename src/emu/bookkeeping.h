// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*********************************************************************

    bookkeeping.h

    Bookkeeping simple machine functions.

*********************************************************************/

#ifndef MAME_EMU_BOOKKEEPING_H
#define MAME_EMU_BOOKKEEPING_H

#pragma once



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> bookkeeping_manager

class bookkeeping_manager
{
public:
	// total # of coin counters
	static constexpr size_t COIN_COUNTERS = 8;

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
	void config_load(config_type cfg_type, config_level cfg_level, util::xml::data_node const *parentnode);
	void config_save(config_type cfg_type, util::xml::data_node *parentnode);

	// internal state
	running_machine &   m_machine;                  // reference to our machine

	u32                 m_dispensed_tickets;
	u32                 m_coin_count[COIN_COUNTERS];
	u32                 m_coinlockedout[COIN_COUNTERS];
	u32                 m_lastcoin[COIN_COUNTERS];
};

#endif  /* MAME_EMU_BOOKKEEPING_H */
