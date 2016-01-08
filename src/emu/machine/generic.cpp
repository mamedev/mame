// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*********************************************************************

    generic.c

    Generic simple machine functions.

*********************************************************************/

#include "emu.h"
#include "config.h"



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void counters_load(running_machine &machine, int config_type, xml_data_node *parentnode);
static void counters_save(running_machine &machine, int config_type, xml_data_node *parentnode);



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct generic_machine_private
{
	/* tickets and coin counters */
	UINT32      dispensed_tickets;
	UINT32      coin_count[COIN_COUNTERS];
	UINT32      coinlockedout[COIN_COUNTERS];
	UINT32      lastcoin[COIN_COUNTERS];
};



/***************************************************************************
    INITIALIZATION
***************************************************************************/

/*-------------------------------------------------
    generic_machine_init - initialize globals and
    register for save states
-------------------------------------------------*/

void generic_machine_init(running_machine &machine)
{
	generic_machine_private *state;
	int counternum;

	/* allocate our state */
	machine.generic_machine_data = auto_alloc_clear(machine, <generic_machine_private>());
	state = machine.generic_machine_data;

	/* reset coin counters */
	for (counternum = 0; counternum < COIN_COUNTERS; counternum++)
	{
		state->lastcoin[counternum] = 0;
		state->coinlockedout[counternum] = 0;
	}

	/* register coin save state */
	machine.save().save_item(NAME(state->coin_count));
	machine.save().save_item(NAME(state->coinlockedout));
	machine.save().save_item(NAME(state->lastcoin));

	/* register for configuration */
	config_register(machine, "counters", config_saveload_delegate(FUNC(counters_load), &machine), config_saveload_delegate(FUNC(counters_save), &machine));
}



/***************************************************************************
    TICKETS
***************************************************************************/

/*-------------------------------------------------
    get_dispensed_tickets - return the number of
    tickets dispensed
-------------------------------------------------*/

int get_dispensed_tickets(running_machine &machine)
{
	generic_machine_private *state = machine.generic_machine_data;
	return state->dispensed_tickets;
}


/*-------------------------------------------------
    increment_dispensed_tickets - increment the
    number of dispensed tickets
-------------------------------------------------*/

void increment_dispensed_tickets(running_machine &machine, int delta)
{
	generic_machine_private *state = machine.generic_machine_data;
	state->dispensed_tickets += delta;
}



/***************************************************************************
    COIN COUNTERS
***************************************************************************/

/*-------------------------------------------------
    counters_load - load the state of the counters
    and tickets
-------------------------------------------------*/

static void counters_load(running_machine &machine, int config_type, xml_data_node *parentnode)
{
	generic_machine_private *state = machine.generic_machine_data;
	xml_data_node *coinnode, *ticketnode;

	/* on init, reset the counters */
	if (config_type == CONFIG_TYPE_INIT)
	{
		memset(state->coin_count, 0, sizeof(state->coin_count));
		state->dispensed_tickets = 0;
	}

	/* only care about game-specific data */
	if (config_type != CONFIG_TYPE_GAME)
		return;

	/* might not have any data */
	if (parentnode == nullptr)
		return;

	/* iterate over coins nodes */
	for (coinnode = xml_get_sibling(parentnode->child, "coins"); coinnode; coinnode = xml_get_sibling(coinnode->next, "coins"))
	{
		int index = xml_get_attribute_int(coinnode, "index", -1);
		if (index >= 0 && index < COIN_COUNTERS)
			state->coin_count[index] = xml_get_attribute_int(coinnode, "number", 0);
	}

	/* get the single tickets node */
	ticketnode = xml_get_sibling(parentnode->child, "tickets");
	if (ticketnode != nullptr)
		state->dispensed_tickets = xml_get_attribute_int(ticketnode, "number", 0);
}


/*-------------------------------------------------
    counters_save - save the state of the counters
    and tickets
-------------------------------------------------*/

static void counters_save(running_machine &machine, int config_type, xml_data_node *parentnode)
{
	generic_machine_private *state = machine.generic_machine_data;
	int i;

	/* only care about game-specific data */
	if (config_type != CONFIG_TYPE_GAME)
		return;

	/* iterate over coin counters */
	for (i = 0; i < COIN_COUNTERS; i++)
		if (state->coin_count[i] != 0)
		{
			xml_data_node *coinnode = xml_add_child(parentnode, "coins", nullptr);
			if (coinnode != nullptr)
			{
				xml_set_attribute_int(coinnode, "index", i);
				xml_set_attribute_int(coinnode, "number", state->coin_count[i]);
			}
		}

	/* output tickets */
	if (state->dispensed_tickets != 0)
	{
		xml_data_node *tickets = xml_add_child(parentnode, "tickets", nullptr);
		if (tickets != nullptr)
			xml_set_attribute_int(tickets, "number", state->dispensed_tickets);
	}
}


/*-------------------------------------------------
    coin_counter_w - sets input for coin counter
-------------------------------------------------*/

void coin_counter_w(running_machine &machine, int num, int on)
{
	generic_machine_private *state = machine.generic_machine_data;
	if (num >= ARRAY_LENGTH(state->coin_count))
		return;

	/* Count it only if the data has changed from 0 to non-zero */
	if (on && (state->lastcoin[num] == 0))
		state->coin_count[num]++;
	state->lastcoin[num] = on;
}


/*-------------------------------------------------
    coin_counter_get_count - return the coin count
    for a given coin
-------------------------------------------------*/

int coin_counter_get_count(running_machine &machine, int num)
{
	generic_machine_private *state = machine.generic_machine_data;
	if (num >= ARRAY_LENGTH(state->coin_count))
		return 0;
	return state->coin_count[num];
}


/*-------------------------------------------------
    coin_lockout_w - locks out one coin input
-------------------------------------------------*/

void coin_lockout_w(running_machine &machine, int num,int on)
{
	generic_machine_private *state = machine.generic_machine_data;
	if (num >= ARRAY_LENGTH(state->coinlockedout))
		return;
	state->coinlockedout[num] = on;
}


/*-------------------------------------------------
    coin_lockout_get_state - return current lockout
    state for a particular coin
-------------------------------------------------*/

int coin_lockout_get_state(running_machine &machine, int num)
{
	generic_machine_private *state = machine.generic_machine_data;
	if (num >= ARRAY_LENGTH(state->coinlockedout))
		return FALSE;
	return state->coinlockedout[num];
}


/*-------------------------------------------------
    coin_lockout_global_w - locks out all the coin
    inputs
-------------------------------------------------*/

void coin_lockout_global_w(running_machine &machine, int on)
{
	generic_machine_private *state = machine.generic_machine_data;
	int i;

	for (i = 0; i < ARRAY_LENGTH(state->coinlockedout); i++)
		coin_lockout_w(machine, i, on);
}


/***************************************************************************
    LED CODE
***************************************************************************/

/*-------------------------------------------------
    set_led_status - set the state of a given LED
-------------------------------------------------*/

void set_led_status(running_machine &machine, int num, int on)
{
	output_set_led_value(num, on);
}




/***************************************************************************
    PORT READING HELPERS
***************************************************************************/

/*-------------------------------------------------
    custom_port_read - act like input_port_read
    but it is a custom port, it is useful for
    e.g. input ports which expect the same port
    repeated both in the upper and lower half
-------------------------------------------------*/

CUSTOM_INPUT_MEMBER( driver_device::custom_port_read )
{
	const char *tag = (const char *)param;
	return ioport(tag)->read();
}
