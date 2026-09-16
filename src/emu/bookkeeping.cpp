// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*********************************************************************

    bookkeeping.cpp

    Bookkeeping simple machine functions.

*********************************************************************/

#include "emu.h"
#include "config.h"

#include "xmlfile.h"


//**************************************************************************
//  BOOKKEEPING MANAGER
//**************************************************************************

//-------------------------------------------------
//  bookkeeping_manager - constructor
//-------------------------------------------------

bookkeeping_manager::bookkeeping_manager(running_machine &machine) :
	m_machine(machine),
	m_dispensed_tickets(0)
{
	// reset coin counters
	for (int counternum = 0; counternum < COIN_COUNTERS; counternum++)
	{
		m_lastcoin[counternum] = 0;
		m_coinlockedout[counternum] = 0;
		m_coin_count[counternum] = 0;
	}

	// register coin save state
	machine.save().save_item(NAME(m_coin_count));
	machine.save().save_item(NAME(m_coinlockedout));
	machine.save().save_item(NAME(m_lastcoin));
	machine.save().save_item(NAME(m_dispensed_tickets));

	// register for configuration
	machine.configuration().config_register("counters",
			configuration_manager::load_delegate(&bookkeeping_manager::config_load, this),
			configuration_manager::save_delegate(&bookkeeping_manager::config_save, this));
}



/***************************************************************************
    TICKETS
***************************************************************************/

/*-------------------------------------------------
    increment_dispensed_tickets - increment the
    number of dispensed tickets
-------------------------------------------------*/

void bookkeeping_manager::increment_dispensed_tickets(int delta)
{
	m_dispensed_tickets += delta;
}

/*-------------------------------------------------
    get_dispensed_tickets - return the number of
    tickets dispensed
-------------------------------------------------*/

int bookkeeping_manager::get_dispensed_tickets() const
{
	return m_dispensed_tickets;
}


/*-------------------------------------------------
    reset_dispensed_tickets - reset the number of
    tickets dispensed
-------------------------------------------------*/

void bookkeeping_manager::reset_dispensed_tickets()
{
	m_dispensed_tickets = 0;
}



/***************************************************************************
    COIN COUNTERS
***************************************************************************/

/*-------------------------------------------------
    config_load - load the state of the counters
    and tickets
-------------------------------------------------*/

void bookkeeping_manager::config_load(config_type cfg_type, config_level cfg_level, util::xml::data_node const *parentnode)
{
	// on init, reset the counters
	if (cfg_type == config_type::INIT)
	{
		memset(m_coin_count, 0, sizeof(m_coin_count));
		m_dispensed_tickets = 0;
	}

	// only care about system-specific data
	if ((cfg_type != config_type::SYSTEM) || !parentnode)
		return;

	// iterate over coins nodes
	for (util::xml::data_node const *coinnode = parentnode->get_child("coins"); coinnode; coinnode = coinnode->get_next_sibling("coins"))
	{
		int index = coinnode->get_attribute_int("index", -1);
		if (index >= 0 && index < COIN_COUNTERS)
			m_coin_count[index] = coinnode->get_attribute_int("number", 0);
	}

	// get the single tickets node
	util::xml::data_node const *const ticketnode = parentnode->get_child("tickets");
	if (ticketnode)
		m_dispensed_tickets = ticketnode->get_attribute_int("number", 0);
}


/*-------------------------------------------------
    config_save - save the state of the counters
    and tickets
-------------------------------------------------*/

void bookkeeping_manager::config_save(config_type cfg_type, util::xml::data_node *parentnode)
{
	// only save system-specific data
	if (cfg_type != config_type::SYSTEM)
		return;

	// iterate over coin counters
	for (int i = 0; i < COIN_COUNTERS; i++)
	{
		if (m_coin_count[i] != 0)
		{
			util::xml::data_node *const coinnode = parentnode->add_child("coins", nullptr);
			if (coinnode)
			{
				coinnode->set_attribute_int("index", i);
				coinnode->set_attribute_int("number", m_coin_count[i]);
			}
		}
	}

	// output tickets
	if (m_dispensed_tickets != 0)
	{
		util::xml::data_node *const tickets = parentnode->add_child("tickets", nullptr);
		if (tickets)
			tickets->set_attribute_int("number", m_dispensed_tickets);
	}
}


/*-------------------------------------------------
    coin_counter_w - sets input for coin counter
-------------------------------------------------*/

void bookkeeping_manager::coin_counter_w(int num, int on)
{
	if (num >= std::size(m_coin_count))
		return;

	// count it only if the data has changed from 0 to non-zero
	if (machine().time() > attotime::zero && on && (m_lastcoin[num] == 0))
		m_coin_count[num]++;
	m_lastcoin[num] = on;
}


/*-------------------------------------------------
    coin_counter_get_count - return the coin count
    for a given coin
-------------------------------------------------*/

int bookkeeping_manager::coin_counter_get_count(int num)
{
	if (num >= std::size(m_coin_count))
		return 0;
	return m_coin_count[num];
}


/*-------------------------------------------------
    coin_counter_reset_count - reset the coin count
    for a given coin
-------------------------------------------------*/

void bookkeeping_manager::coin_counter_reset_count(int num)
{
	if (num >= std::size(m_coin_count))
		return;
	m_coin_count[num] = 0;
}


/*-------------------------------------------------
    coin_lockout_w - locks out one coin input
-------------------------------------------------*/

void bookkeeping_manager::coin_lockout_w(int num, int on)
{
	if (num >= std::size(m_coinlockedout))
		return;
	m_coinlockedout[num] = on;
}


/*-------------------------------------------------
    coin_lockout_get_state - return current lockout
    state for a particular coin
-------------------------------------------------*/

int bookkeeping_manager::coin_lockout_get_state(int num)
{
	if (num >= std::size(m_coinlockedout))
		return false;
	return m_coinlockedout[num];
}


/*-------------------------------------------------
    coin_lockout_global_w - locks out all the coin
    inputs
-------------------------------------------------*/

void bookkeeping_manager::coin_lockout_global_w(int on)
{
	for (int i = 0; i < std::size(m_coinlockedout); i++)
		coin_lockout_w(i, on);
}
