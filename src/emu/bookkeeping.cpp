// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*********************************************************************

    bookkeeping.cpp

    Bookkeeping simple machine functions.

*********************************************************************/

#include "emu.h"
#include "config.h"


//**************************************************************************
//  BOOKKEEPING MANAGER
//**************************************************************************

//-------------------------------------------------
//  bookkeeping_manager - constructor
//-------------------------------------------------

bookkeeping_manager::bookkeeping_manager(running_machine &machine)
	: m_machine(machine),
		m_dispensed_tickets(0)
{
	/* reset coin counters */
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
	machine.configuration().config_register("counters", config_load_delegate(&bookkeeping_manager::config_load, this), config_save_delegate(&bookkeeping_manager::config_save, this));
}



/***************************************************************************
    TICKETS
***************************************************************************/

/*-------------------------------------------------
    get_dispensed_tickets - return the number of
    tickets dispensed
-------------------------------------------------*/

int bookkeeping_manager::get_dispensed_tickets() const
{
	return m_dispensed_tickets;
}


/*-------------------------------------------------
    increment_dispensed_tickets - increment the
    number of dispensed tickets
-------------------------------------------------*/

void bookkeeping_manager::increment_dispensed_tickets(int delta)
{
	m_dispensed_tickets += delta;
}



/***************************************************************************
    COIN COUNTERS
***************************************************************************/

/*-------------------------------------------------
    config_load - load the state of the counters
    and tickets
-------------------------------------------------*/

void bookkeeping_manager::config_load(config_type cfg_type, util::xml::data_node const *parentnode)
{
	util::xml::data_node const *coinnode, *ticketnode;

	/* on init, reset the counters */
	if (cfg_type == config_type::INIT)
	{
		memset(m_coin_count, 0, sizeof(m_coin_count));
		m_dispensed_tickets = 0;
	}

	/* only care about game-specific data */
	if (cfg_type != config_type::GAME)
		return;

	/* might not have any data */
	if (parentnode == nullptr)
		return;

	/* iterate over coins nodes */
	for (coinnode = parentnode->get_child("coins"); coinnode; coinnode = coinnode->get_next_sibling("coins"))
	{
		int index = coinnode->get_attribute_int("index", -1);
		if (index >= 0 && index < COIN_COUNTERS)
			m_coin_count[index] = coinnode->get_attribute_int("number", 0);
	}

	/* get the single tickets node */
	ticketnode = parentnode->get_child("tickets");
	if (ticketnode != nullptr)
		m_dispensed_tickets = ticketnode->get_attribute_int("number", 0);
}


/*-------------------------------------------------
    config_save - save the state of the counters
    and tickets
-------------------------------------------------*/

void bookkeeping_manager::config_save(config_type cfg_type, util::xml::data_node *parentnode)
{
	int i;

	/* only care about game-specific data */
	if (cfg_type != config_type::GAME)
		return;

	/* iterate over coin counters */
	for (i = 0; i < COIN_COUNTERS; i++)
	{
		if (m_coin_count[i] != 0)
		{
			util::xml::data_node *coinnode = parentnode->add_child("coins", nullptr);
			if (coinnode != nullptr)
			{
				coinnode->set_attribute_int("index", i);
				coinnode->set_attribute_int("number", m_coin_count[i]);
			}
		}
	}

	/* output tickets */
	if (m_dispensed_tickets != 0)
	{
		util::xml::data_node *tickets = parentnode->add_child("tickets", nullptr);
		if (tickets != nullptr)
			tickets->set_attribute_int("number", m_dispensed_tickets);
	}
}


/*-------------------------------------------------
    coin_counter_w - sets input for coin counter
-------------------------------------------------*/

void bookkeeping_manager::coin_counter_w(int num, int on)
{
	if (num >= ARRAY_LENGTH(m_coin_count))
		return;

	/* Count it only if the data has changed from 0 to non-zero */
	if (on && (m_lastcoin[num] == 0))
		m_coin_count[num]++;
	m_lastcoin[num] = on;
}


/*-------------------------------------------------
    coin_counter_get_count - return the coin count
    for a given coin
-------------------------------------------------*/

int bookkeeping_manager::coin_counter_get_count(int num)
{
	if (num >= ARRAY_LENGTH(m_coin_count))
		return 0;
	return m_coin_count[num];
}


/*-------------------------------------------------
    coin_lockout_w - locks out one coin input
-------------------------------------------------*/

void bookkeeping_manager::coin_lockout_w(int num,int on)
{
	if (num >= ARRAY_LENGTH(m_coinlockedout))
		return;
	m_coinlockedout[num] = on;
}


/*-------------------------------------------------
    coin_lockout_get_state - return current lockout
    state for a particular coin
-------------------------------------------------*/

int bookkeeping_manager::coin_lockout_get_state(int num)
{
	if (num >= ARRAY_LENGTH(m_coinlockedout))
		return false;
	return m_coinlockedout[num];
}


/*-------------------------------------------------
    coin_lockout_global_w - locks out all the coin
    inputs
-------------------------------------------------*/

void bookkeeping_manager::coin_lockout_global_w(int on)
{
	for (int i = 0; i < ARRAY_LENGTH(m_coinlockedout); i++)
		coin_lockout_w(i, on);
}
