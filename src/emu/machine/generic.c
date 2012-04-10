/*********************************************************************

    generic.c

    Generic simple machine functions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "config.h"



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void counters_load(running_machine &machine, int config_type, xml_data_node *parentnode);
static void counters_save(running_machine &machine, int config_type, xml_data_node *parentnode);



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct _generic_machine_private
{
	/* tickets and coin counters */
	UINT32		dispensed_tickets;
	UINT32		coin_count[COIN_COUNTERS];
	UINT32		coinlockedout[COIN_COUNTERS];
	UINT32		lastcoin[COIN_COUNTERS];

	/* memory card status */
	int 		memcard_inserted;
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
	machine.generic_machine_data = auto_alloc_clear(machine, generic_machine_private);
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

	/* reset memory card info */
	state->memcard_inserted = -1;

	/* register for configuration */
	config_register(machine, "counters", config_saveload_delegate(FUNC(counters_load), &machine), config_saveload_delegate(FUNC(counters_save), &machine));

	/* for memory cards, request save state and an exit callback */
	if (machine.config().m_memcard_handler != NULL)
	{
		state_save_register_global(machine, state->memcard_inserted);
		machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(memcard_eject), &machine));
	}
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
	if (parentnode == NULL)
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
	if (ticketnode != NULL)
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
			xml_data_node *coinnode = xml_add_child(parentnode, "coins", NULL);
			if (coinnode != NULL)
			{
				xml_set_attribute_int(coinnode, "index", i);
				xml_set_attribute_int(coinnode, "number", state->coin_count[i]);
			}
		}

	/* output tickets */
	if (state->dispensed_tickets != 0)
	{
		xml_data_node *tickets = xml_add_child(parentnode, "tickets", NULL);
		if (tickets != NULL)
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
    NVRAM MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    nvram_filename - returns filename of system's
    NVRAM depending of selected BIOS
-------------------------------------------------*/

static astring &nvram_filename(astring &result, device_t &device)
{
	running_machine &machine = device.machine();

	// start with either basename or basename_biosnum
	result.cpy(machine.basename());
	if (rom_system_bios(machine) != 0 && rom_default_bios(machine) != rom_system_bios(machine))
		result.catprintf("_%d", rom_system_bios(machine) - 1);

	// device-based NVRAM gets its own name in a subdirectory
	if (&device != &device.machine().root_device())
	{
		astring tag(device.tag());
		tag.del(0, 1).replacechr(':', '_');
		result.cat('\\').cat(tag);
	}
	return result;
}

/*-------------------------------------------------
    nvram_load - load a system's NVRAM
-------------------------------------------------*/

void nvram_load(running_machine &machine)
{
	if (machine.config().m_nvram_handler != NULL)
	{
		astring filename;
		emu_file file(machine.options().nvram_directory(), OPEN_FLAG_READ);
		if (file.open(nvram_filename(filename, machine.root_device()), ".nv") == FILERR_NONE)
		{
			(*machine.config().m_nvram_handler)(machine, &file, FALSE);
			file.close();
		}
		else
		{
			(*machine.config().m_nvram_handler)(machine, NULL, FALSE);
		}
	}

	nvram_interface_iterator iter(machine.root_device());
	for (device_nvram_interface *nvram = iter.first(); nvram != NULL; nvram = iter.next())
	{
		astring filename;
		emu_file file(machine.options().nvram_directory(), OPEN_FLAG_READ);
		if (file.open(nvram_filename(filename, nvram->device())) == FILERR_NONE)
		{
			nvram->nvram_load(file);
			file.close();
		}
		else
			nvram->nvram_reset();
	}
}


/*-------------------------------------------------
    nvram_save - save a system's NVRAM
-------------------------------------------------*/

void nvram_save(running_machine &machine)
{
	if (machine.config().m_nvram_handler != NULL)
	{
		astring filename;
		emu_file file(machine.options().nvram_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
		if (file.open(nvram_filename(filename, machine.root_device()), ".nv") == FILERR_NONE)
		{
			(*machine.config().m_nvram_handler)(machine, &file, TRUE);
			file.close();
		}
	}

	nvram_interface_iterator iter(machine.root_device());
	for (device_nvram_interface *nvram = iter.first(); nvram != NULL; nvram = iter.next())
	{
		astring filename;
		emu_file file(machine.options().nvram_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
		if (file.open(nvram_filename(filename, nvram->device())) == FILERR_NONE)
		{
			nvram->nvram_save(file);
			file.close();
		}
	}
}



/***************************************************************************
    MEMORY CARD MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    memcard_name - determine the name of a memcard
    file
-------------------------------------------------*/

INLINE void memcard_name(int index, char *buffer)
{
	sprintf(buffer, "memcard.%03d", index);
}


/*-------------------------------------------------
    memcard_create - create a new memory card with
    the given index
-------------------------------------------------*/

int memcard_create(running_machine &machine, int index, int overwrite)
{
	char name[16];

	/* create a name */
	memcard_name(index, name);

	/* if we can't overwrite, fail if the file already exists */
	astring fname(machine.basename(), PATH_SEPARATOR, name);
	if (!overwrite)
	{
		emu_file testfile(machine.options().memcard_directory(), OPEN_FLAG_READ);
		if (testfile.open(fname) == FILERR_NONE)
			return 1;
	}

	/* create a new file */
	emu_file file(machine.options().memcard_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	file_error filerr = file.open(fname);
	if (filerr != FILERR_NONE)
		return 1;

	/* initialize and then save the card */
	if (machine.config().m_memcard_handler)
		(*machine.config().m_memcard_handler)(machine, file, MEMCARD_CREATE);

	/* close the file */
	return 0;
}


/*-------------------------------------------------
    memcard_insert - insert an existing memory card
    with the given index
-------------------------------------------------*/

int memcard_insert(running_machine &machine, int index)
{
	generic_machine_private *state = machine.generic_machine_data;
	char name[16];

	/* if a card is already inserted, eject it first */
	if (state->memcard_inserted != -1)
		memcard_eject(machine);
	assert(state->memcard_inserted == -1);

	/* create a name */
	memcard_name(index, name);

	/* open the file; if we can't, it's an error */
	emu_file file(machine.options().memcard_directory(), OPEN_FLAG_READ);
	file_error filerr = file.open(machine.basename(), PATH_SEPARATOR, name);
	if (filerr != FILERR_NONE)
		return 1;

	/* initialize and then load the card */
	if (machine.config().m_memcard_handler)
		(*machine.config().m_memcard_handler)(machine, file, MEMCARD_INSERT);

	/* close the file */
	state->memcard_inserted = index;
	return 0;
}


/*-------------------------------------------------
    memcard_eject - eject a memory card, saving
    its contents along the way
-------------------------------------------------*/

void memcard_eject(running_machine &machine)
{
	generic_machine_private *state = machine.generic_machine_data;
	char name[16];

	/* if no card is preset, just ignore */
	if (state->memcard_inserted == -1)
		return;

	/* create a name */
	memcard_name(state->memcard_inserted, name);

	/* open the file; if we can't, it's an error */
	emu_file file(machine.options().memcard_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	file_error filerr = file.open(machine.basename(), PATH_SEPARATOR, name);
	if (filerr != FILERR_NONE)
		return;

	/* initialize and then load the card */
	if (machine.config().m_memcard_handler)
		(*machine.config().m_memcard_handler)(machine, file, MEMCARD_EJECT);

	/* close the file */
	state->memcard_inserted = -1;
}


/*-------------------------------------------------
    memcard_present - return the currently loaded
    card index, or -1 if none
-------------------------------------------------*/

int memcard_present(running_machine &machine)
{
	generic_machine_private *state = machine.generic_machine_data;
	return state->memcard_inserted;
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
	return input_port_read(machine(), tag);
}


void generic_pulse_irq_line(device_t *device, int irqline, int cycles) { device->machine().driver_data()->generic_pulse_irq_line(device->execute(), irqline, cycles); }
void generic_pulse_irq_line_and_vector(device_t *device, int irqline, int vector, int cycles) { device->machine().driver_data()->generic_pulse_irq_line_and_vector(device->execute(), irqline, vector, cycles); }

INTERRUPT_GEN( nmi_line_pulse ) { device->machine().driver_data()->nmi_line_pulse(*device); }
INTERRUPT_GEN( nmi_line_assert ) { device->machine().driver_data()->nmi_line_assert(*device); }

INTERRUPT_GEN( irq0_line_hold ) { device->machine().driver_data()->irq0_line_hold(*device); }
INTERRUPT_GEN( irq0_line_pulse ) { device->machine().driver_data()->irq0_line_pulse(*device); }
INTERRUPT_GEN( irq0_line_assert ) { device->machine().driver_data()->irq0_line_assert(*device); }

INTERRUPT_GEN( irq1_line_hold ) { device->machine().driver_data()->irq1_line_hold(*device); }
INTERRUPT_GEN( irq1_line_pulse ) { device->machine().driver_data()->irq1_line_pulse(*device); }
INTERRUPT_GEN( irq1_line_assert ) { device->machine().driver_data()->irq1_line_assert(*device); }

INTERRUPT_GEN( irq2_line_hold ) { device->machine().driver_data()->irq2_line_hold(*device); }
INTERRUPT_GEN( irq2_line_pulse ) { device->machine().driver_data()->irq2_line_pulse(*device); }
INTERRUPT_GEN( irq2_line_assert ) { device->machine().driver_data()->irq2_line_assert(*device); }

INTERRUPT_GEN( irq3_line_hold ) { device->machine().driver_data()->irq3_line_hold(*device); }
INTERRUPT_GEN( irq3_line_pulse ) { device->machine().driver_data()->irq3_line_pulse(*device); }
INTERRUPT_GEN( irq3_line_assert ) { device->machine().driver_data()->irq3_line_assert(*device); }

INTERRUPT_GEN( irq4_line_hold ) { device->machine().driver_data()->irq4_line_hold(*device); }
INTERRUPT_GEN( irq4_line_pulse ) { device->machine().driver_data()->irq4_line_pulse(*device); }
INTERRUPT_GEN( irq4_line_assert ) { device->machine().driver_data()->irq4_line_assert(*device); }

INTERRUPT_GEN( irq5_line_hold ) { device->machine().driver_data()->irq5_line_hold(*device); }
INTERRUPT_GEN( irq5_line_pulse ) { device->machine().driver_data()->irq5_line_pulse(*device); }
INTERRUPT_GEN( irq5_line_assert ) { device->machine().driver_data()->irq5_line_assert(*device); }

INTERRUPT_GEN( irq6_line_hold ) { device->machine().driver_data()->irq6_line_hold(*device); }
INTERRUPT_GEN( irq6_line_pulse ) { device->machine().driver_data()->irq6_line_pulse(*device); }
INTERRUPT_GEN( irq6_line_assert ) { device->machine().driver_data()->irq6_line_assert(*device); }

INTERRUPT_GEN( irq7_line_hold ) { device->machine().driver_data()->irq7_line_hold(*device); }
INTERRUPT_GEN( irq7_line_pulse ) { device->machine().driver_data()->irq7_line_pulse(*device); }
INTERRUPT_GEN( irq7_line_assert ) { device->machine().driver_data()->irq7_line_assert(*device); }
