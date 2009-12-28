/*********************************************************************

    generic.c

    Generic simple machine functions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "driver.h"
#include "config.h"
#include "generic.h"



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void counters_load(running_machine *machine, int config_type, xml_data_node *parentnode);
static void counters_save(running_machine *machine, int config_type, xml_data_node *parentnode);
static void interrupt_reset(running_machine *machine);



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

	/* interrupt status for up to 8 CPUs */
	UINT8		interrupt_enable[8];
};



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    interrupt_enabled - return true if interrupts
    are enabled for the given CPU
-------------------------------------------------*/

INLINE int interrupt_enabled(const device_config *device)
{
	generic_machine_private *state = device->machine->generic_machine_data;
	int cpunum = cpu_get_index(device);
	return (cpunum >= ARRAY_LENGTH(state->interrupt_enable) || state->interrupt_enable[cpunum]);
}



/***************************************************************************
    INITIALIZATION
***************************************************************************/

/*-------------------------------------------------
    generic_machine_init - initialize globals and
    register for save states
-------------------------------------------------*/

void generic_machine_init(running_machine *machine)
{
	generic_machine_private *state;
	int counternum;

	/* allocate our state */
	machine->generic_machine_data = auto_alloc_clear(machine, generic_machine_private);
	state = machine->generic_machine_data;

	/* reset coin counters */
	for (counternum = 0; counternum < COIN_COUNTERS; counternum++)
	{
		state->lastcoin[counternum] = 0;
		state->coinlockedout[counternum] = 0;
	}

	/* register coin save state */
	state_save_register_item_array(machine, "coin", NULL, 0, state->coin_count);
	state_save_register_item_array(machine, "coin", NULL, 0, state->coinlockedout);
	state_save_register_item_array(machine, "coin", NULL, 0, state->lastcoin);

	/* reset NVRAM size and pointers */
	machine->generic.nvram.v = NULL;
	machine->generic.nvram_size = 0;

	/* reset memory card info */
	state->memcard_inserted = -1;

	/* register a reset callback and save state for interrupt enable */
	add_reset_callback(machine, interrupt_reset);
	state_save_register_item_array(machine, "cpu", NULL, 0, state->interrupt_enable);

	/* register for configuration */
	config_register(machine, "counters", counters_load, counters_save);

	/* for memory cards, request save state and an exit callback */
	if (machine->config->memcard_handler != NULL)
	{
		state_save_register_global(machine, state->memcard_inserted);
		add_exit_callback(machine, memcard_eject);
	}
}



/***************************************************************************
    TICKETS
***************************************************************************/

/*-------------------------------------------------
    get_dispensed_tickets - return the number of
    tickets dispensed
-------------------------------------------------*/

int get_dispensed_tickets(running_machine *machine)
{
	generic_machine_private *state = machine->generic_machine_data;
	return state->dispensed_tickets;
}


/*-------------------------------------------------
    increment_dispensed_tickets - increment the
    number of dispensed tickets
-------------------------------------------------*/

void increment_dispensed_tickets(running_machine *machine, int delta)
{
	generic_machine_private *state = machine->generic_machine_data;
	state->dispensed_tickets += delta;
}



/***************************************************************************
    COIN COUNTERS
***************************************************************************/

/*-------------------------------------------------
    counters_load - load the state of the counters
    and tickets
-------------------------------------------------*/

static void counters_load(running_machine *machine, int config_type, xml_data_node *parentnode)
{
	generic_machine_private *state = machine->generic_machine_data;
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

static void counters_save(running_machine *machine, int config_type, xml_data_node *parentnode)
{
	generic_machine_private *state = machine->generic_machine_data;
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

void coin_counter_w(running_machine *machine, int num, int on)
{
	generic_machine_private *state = machine->generic_machine_data;
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

int coin_counter_get_count(running_machine *machine, int num)
{
	generic_machine_private *state = machine->generic_machine_data;
	if (num >= ARRAY_LENGTH(state->coin_count))
		return 0;
	return state->coin_count[num];
}


/*-------------------------------------------------
    coin_lockout_w - locks out one coin input
-------------------------------------------------*/

void coin_lockout_w(running_machine *machine, int num,int on)
{
	generic_machine_private *state = machine->generic_machine_data;
	if (num >= ARRAY_LENGTH(state->coinlockedout))
		return;
	state->coinlockedout[num] = on;
}


/*-------------------------------------------------
    coin_lockout_get_state - return current lockout
    state for a particular coin
-------------------------------------------------*/

int coin_lockout_get_state(running_machine *machine, int num)
{
	generic_machine_private *state = machine->generic_machine_data;
	if (num >= ARRAY_LENGTH(state->coinlockedout))
		return FALSE;
	return state->coinlockedout[num];
}


/*-------------------------------------------------
    coin_lockout_global_w - locks out all the coin
    inputs
-------------------------------------------------*/

void coin_lockout_global_w(running_machine *machine, int on)
{
	generic_machine_private *state = machine->generic_machine_data;
	int i;

	for (i = 0; i < ARRAY_LENGTH(state->coinlockedout); i++)
		coin_lockout_w(machine, i, on);
}



/***************************************************************************
    NVRAM MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    nvram_fopen - open an NVRAM file directly
-------------------------------------------------*/

mame_file *nvram_fopen(running_machine *machine, UINT32 openflags)
{
	file_error filerr;
	mame_file *file;
	astring *fname;

	fname = astring_assemble_2(astring_alloc(), machine->basename, ".nv");
	filerr = mame_fopen(SEARCHPATH_NVRAM, astring_c(fname), openflags, &file);
	astring_free(fname);

	return (filerr == FILERR_NONE) ? file : NULL;
}


/*-------------------------------------------------
    nvram_load - load a system's NVRAM
-------------------------------------------------*/

void nvram_load(running_machine *machine)
{
	mame_file *nvram_file = NULL;
	const device_config *device;

	/* read data from general NVRAM handler first */
	if (machine->config->nvram_handler != NULL)
	{
		nvram_file = nvram_fopen(machine, OPEN_FLAG_READ);
		(*machine->config->nvram_handler)(machine, nvram_file, 0);
	}

	/* find all devices with NVRAM handlers, and read from them next */
	for (device = machine->config->devicelist.head; device != NULL; device = device->next)
	{
		device_nvram_func nvram = (device_nvram_func)device_get_info_fct(device, DEVINFO_FCT_NVRAM);
		if (nvram != NULL)
		{
			if (nvram_file == NULL)
				nvram_file = nvram_fopen(machine, OPEN_FLAG_READ);
			(*nvram)(device, nvram_file, 0);
		}
	}

	if (nvram_file != NULL)
		mame_fclose(nvram_file);
}


/*-------------------------------------------------
    nvram_save - save a system's NVRAM
-------------------------------------------------*/

void nvram_save(running_machine *machine)
{
	mame_file *nvram_file = NULL;
	const device_config *device;

	/* write data from general NVRAM handler first */
	if (machine->config->nvram_handler != NULL)
	{
		nvram_file = nvram_fopen(machine, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
		(*machine->config->nvram_handler)(machine, nvram_file, 1);
	}

	/* find all devices with NVRAM handlers, and write them next */
	for (device = machine->config->devicelist.head; device != NULL; device = device->next)
	{
		device_nvram_func nvram = (device_nvram_func)device_get_info_fct(device, DEVINFO_FCT_NVRAM);
		if (nvram != NULL)
		{
			if (nvram_file == NULL)
				nvram_file = nvram_fopen(machine, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
			(*nvram)(device, nvram_file, 1);
		}
	}

	if (nvram_file != NULL)
		mame_fclose(nvram_file);
}


/*-------------------------------------------------
    NVRAM_HANDLER( generic_0fill ) - generic NVRAM
    with a 0 fill
-------------------------------------------------*/

NVRAM_HANDLER( generic_0fill )
{
	if (read_or_write)
		mame_fwrite(file, machine->generic.nvram.v, machine->generic.nvram_size);
	else if (file != NULL)
		mame_fread(file, machine->generic.nvram.v, machine->generic.nvram_size);
	else if (memory_region_length(machine, "nvram") == machine->generic.nvram_size)
		memcpy(machine->generic.nvram.v, memory_region(machine, "nvram"), machine->generic.nvram_size);
	else
		memset(machine->generic.nvram.v, 0, machine->generic.nvram_size);
}


/*-------------------------------------------------
    NVRAM_HANDLER( generic_1fill ) - generic NVRAM
    with a 1 fill
-------------------------------------------------*/

NVRAM_HANDLER( generic_1fill )
{
	if (read_or_write)
		mame_fwrite(file, machine->generic.nvram.v, machine->generic.nvram_size);
	else if (file != NULL)
		mame_fread(file, machine->generic.nvram.v, machine->generic.nvram_size);
	else if (memory_region_length(machine, "nvram") == machine->generic.nvram_size)
		memcpy(machine->generic.nvram.v, memory_region(machine, "nvram"), machine->generic.nvram_size);
	else
		memset(machine->generic.nvram.v, 0xff, machine->generic.nvram_size);
}


/*-------------------------------------------------
    NVRAM_HANDLER( generic_randfill ) - generic NVRAM
    with a random fill
-------------------------------------------------*/

NVRAM_HANDLER( generic_randfill )
{
	if (read_or_write)
		mame_fwrite(file, machine->generic.nvram.v, machine->generic.nvram_size);
	else if (file != NULL)
		mame_fread(file, machine->generic.nvram.v, machine->generic.nvram_size);
	else if (memory_region_length(machine, "nvram") == machine->generic.nvram_size)
		memcpy(machine->generic.nvram.v, memory_region(machine, "nvram"), machine->generic.nvram_size);
	else
	{
		UINT8 *nvram = (UINT8 *)machine->generic.nvram.v;
		int i;
		for (i = 0; i < machine->generic.nvram_size; i++)
			nvram[i] = mame_rand(machine);
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

int memcard_create(running_machine *machine, int index, int overwrite)
{
	file_error filerr;
	mame_file *file;
	astring *fname;
	char name[16];

	/* create a name */
	memcard_name(index, name);

	/* if we can't overwrite, fail if the file already exists */
	fname = astring_assemble_3(astring_alloc(), machine->basename, PATH_SEPARATOR, name);
	if (!overwrite)
	{
		filerr = mame_fopen(SEARCHPATH_MEMCARD, astring_c(fname), OPEN_FLAG_READ, &file);
		if (filerr == FILERR_NONE)
		{
			mame_fclose(file);
			astring_free(fname);
			return 1;
		}
	}

	/* create a new file */
	filerr = mame_fopen(SEARCHPATH_MEMCARD, astring_c(fname), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS, &file);
	astring_free(fname);
	if (filerr != FILERR_NONE)
		return 1;

	/* initialize and then save the card */
	if (machine->config->memcard_handler)
		(*machine->config->memcard_handler)(machine, file, MEMCARD_CREATE);

	/* close the file */
	mame_fclose(file);
	return 0;
}


/*-------------------------------------------------
    memcard_insert - insert an existing memory card
    with the given index
-------------------------------------------------*/

int memcard_insert(running_machine *machine, int index)
{
	generic_machine_private *state = machine->generic_machine_data;
	file_error filerr;
	mame_file *file;
	char name[16];
	astring *fname;

	/* if a card is already inserted, eject it first */
	if (state->memcard_inserted != -1)
		memcard_eject(machine);
	assert(state->memcard_inserted == -1);

	/* create a name */
	memcard_name(index, name);
	fname = astring_assemble_3(astring_alloc(), machine->basename, PATH_SEPARATOR, name);

	/* open the file; if we can't, it's an error */
	filerr = mame_fopen(SEARCHPATH_MEMCARD, astring_c(fname), OPEN_FLAG_READ, &file);
	astring_free(fname);
	if (filerr != FILERR_NONE)
		return 1;

	/* initialize and then load the card */
	if (machine->config->memcard_handler)
		(*machine->config->memcard_handler)(machine, file, MEMCARD_INSERT);

	/* close the file */
	mame_fclose(file);
	state->memcard_inserted = index;
	return 0;
}


/*-------------------------------------------------
    memcard_eject - eject a memory card, saving
    its contents along the way
-------------------------------------------------*/

void memcard_eject(running_machine *machine)
{
	generic_machine_private *state = machine->generic_machine_data;
	file_error filerr;
	mame_file *file;
	char name[16];
	astring *fname;

	/* if no card is preset, just ignore */
	if (state->memcard_inserted == -1)
		return;

	/* create a name */
	memcard_name(state->memcard_inserted, name);
	fname = astring_assemble_3(astring_alloc(), machine->basename, PATH_SEPARATOR, name);

	/* open the file; if we can't, it's an error */
	filerr = mame_fopen(SEARCHPATH_MEMCARD, astring_c(fname), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS, &file);
	astring_free(fname);
	if (filerr != FILERR_NONE)
	{
		mame_fclose(file);
		return;
	}

	/* initialize and then load the card */
	if (machine->config->memcard_handler)
		(*machine->config->memcard_handler)(machine, file, MEMCARD_EJECT);

	/* close the file */
	mame_fclose(file);
	state->memcard_inserted = -1;
}


/*-------------------------------------------------
    memcard_present - return the currently loaded
    card index, or -1 if none
-------------------------------------------------*/

int memcard_present(running_machine *machine)
{
	generic_machine_private *state = machine->generic_machine_data;
	return state->memcard_inserted;
}



/***************************************************************************
    LED CODE
***************************************************************************/

/*-------------------------------------------------
    set_led_status - set the state of a given LED
-------------------------------------------------*/

void set_led_status(running_machine *machine, int num, int on)
{
	output_set_led_value(num, on);
}



/***************************************************************************
    INTERRUPT ENABLE AND VECTOR HELPERS
***************************************************************************/

/*-------------------------------------------------
    interrupt_reset - reset the interrupt enable
    states on a reset
-------------------------------------------------*/

static void interrupt_reset(running_machine *machine)
{
	generic_machine_private *state = machine->generic_machine_data;
	int cpunum;

	/* on a reset, enable all interrupts */
	for (cpunum = 0; cpunum < ARRAY_LENGTH(state->interrupt_enable); cpunum++)
		state->interrupt_enable[cpunum] = 1;
}


/*-------------------------------------------------
    clear_all_lines - sets the state of all input
    lines and the NMI line to clear
-------------------------------------------------*/

static TIMER_CALLBACK( clear_all_lines )
{
	const device_config *device = (const device_config *)ptr;
	int inputcount = cpu_get_input_lines(device);
	int line;

	/* clear NMI and all inputs */
	cpu_set_input_line(device, INPUT_LINE_NMI, CLEAR_LINE);
	for (line = 0; line < inputcount; line++)
		cpu_set_input_line(device, line, CLEAR_LINE);
}


/*-------------------------------------------------
    irq_pulse_clear - clear a "pulsed" IRQ line
-------------------------------------------------*/

static TIMER_CALLBACK( irq_pulse_clear )
{
	const device_config *device = (const device_config *)ptr;
	int irqline = param;
	cpu_set_input_line(device, irqline, CLEAR_LINE);
}


/*-------------------------------------------------
    generic_pulse_irq_line - "pulse" an IRQ line by
    asserting it and then clearing it 1 cycle
    later
-------------------------------------------------*/

void generic_pulse_irq_line(const device_config *device, int irqline)
{
	int multiplier = cpu_get_clock_multiplier(device);
	int clocks = (cpu_get_min_cycles(device) * cpu_get_clock_divider(device) + multiplier - 1) / multiplier;
	assert(irqline != INPUT_LINE_NMI && irqline != INPUT_LINE_RESET);
	cpu_set_input_line(device, irqline, ASSERT_LINE);
	timer_set(device->machine, cpu_clocks_to_attotime(device, MAX(clocks, 1)), (void *)device, irqline, irq_pulse_clear);
}


/*-------------------------------------------------
    generic_pulse_irq_line_and_vector - "pulse" an
    IRQ line by asserting it and then clearing it
    1 cycle later, specifying a vector
-------------------------------------------------*/

void generic_pulse_irq_line_and_vector(const device_config *device, int irqline, int vector)
{
	assert(irqline != INPUT_LINE_NMI && irqline != INPUT_LINE_RESET);
	cpu_set_input_line_and_vector(device, irqline, ASSERT_LINE, vector);
	timer_set(device->machine, cpu_clocks_to_attotime(device, 1), (void *)device, irqline, irq_pulse_clear);
}


/*-------------------------------------------------
    cpu_interrupt_enable - controls the enable/
    disable value for global interrupts
-------------------------------------------------*/

void cpu_interrupt_enable(const device_config *device, int enabled)
{
	generic_machine_private *state = device->machine->generic_machine_data;
	int cpunum = cpu_get_index(device);

	assert_always(device != NULL, "cpu_interrupt_enable() called for invalid cpu!");
	assert_always(cpunum < ARRAY_LENGTH(state->interrupt_enable), "cpu_interrupt_enable() called for a CPU > position 7!");

	/* set the new state */
	if (cpunum < ARRAY_LENGTH(state->interrupt_enable))
		state->interrupt_enable[cpunum] = enabled;

	/* make sure there are no queued interrupts */
	if (enabled == 0)
		timer_call_after_resynch(device->machine, (void *)device, 0, clear_all_lines);
}


/*-------------------------------------------------
    interrupt_enable_w - set the global interrupt
    enable
-------------------------------------------------*/

WRITE8_HANDLER( interrupt_enable_w )
{
	cpu_interrupt_enable(space->cpu, data);
}


/*-------------------------------------------------
    interrupt_enable_r - read the global interrupt
    enable
-------------------------------------------------*/

READ8_HANDLER( interrupt_enable_r )
{
	return interrupt_enabled(space->cpu);
}



/***************************************************************************
    INTERRUPT GENERATION CALLBACK HELPERS
***************************************************************************/

/*-------------------------------------------------
    NMI callbacks
-------------------------------------------------*/

INTERRUPT_GEN( nmi_line_pulse )		{ if (interrupt_enabled(device)) cpu_set_input_line(device, INPUT_LINE_NMI, PULSE_LINE); }
INTERRUPT_GEN( nmi_line_assert )	{ if (interrupt_enabled(device)) cpu_set_input_line(device, INPUT_LINE_NMI, ASSERT_LINE); }


/*-------------------------------------------------
    IRQn callbacks
-------------------------------------------------*/

INTERRUPT_GEN( irq0_line_hold )		{ if (interrupt_enabled(device)) cpu_set_input_line(device, 0, HOLD_LINE); }
INTERRUPT_GEN( irq0_line_pulse )	{ if (interrupt_enabled(device)) generic_pulse_irq_line(device, 0); }
INTERRUPT_GEN( irq0_line_assert )	{ if (interrupt_enabled(device)) cpu_set_input_line(device, 0, ASSERT_LINE); }

INTERRUPT_GEN( irq1_line_hold )		{ if (interrupt_enabled(device)) cpu_set_input_line(device, 1, HOLD_LINE); }
INTERRUPT_GEN( irq1_line_pulse )	{ if (interrupt_enabled(device)) generic_pulse_irq_line(device, 1); }
INTERRUPT_GEN( irq1_line_assert )	{ if (interrupt_enabled(device)) cpu_set_input_line(device, 1, ASSERT_LINE); }

INTERRUPT_GEN( irq2_line_hold )		{ if (interrupt_enabled(device)) cpu_set_input_line(device, 2, HOLD_LINE); }
INTERRUPT_GEN( irq2_line_pulse )	{ if (interrupt_enabled(device)) generic_pulse_irq_line(device, 2); }
INTERRUPT_GEN( irq2_line_assert )	{ if (interrupt_enabled(device)) cpu_set_input_line(device, 2, ASSERT_LINE); }

INTERRUPT_GEN( irq3_line_hold )		{ if (interrupt_enabled(device)) cpu_set_input_line(device, 3, HOLD_LINE); }
INTERRUPT_GEN( irq3_line_pulse )	{ if (interrupt_enabled(device)) generic_pulse_irq_line(device, 3); }
INTERRUPT_GEN( irq3_line_assert )	{ if (interrupt_enabled(device)) cpu_set_input_line(device, 3, ASSERT_LINE); }

INTERRUPT_GEN( irq4_line_hold )		{ if (interrupt_enabled(device)) cpu_set_input_line(device, 4, HOLD_LINE); }
INTERRUPT_GEN( irq4_line_pulse )	{ if (interrupt_enabled(device)) generic_pulse_irq_line(device, 4); }
INTERRUPT_GEN( irq4_line_assert )	{ if (interrupt_enabled(device)) cpu_set_input_line(device, 4, ASSERT_LINE); }

INTERRUPT_GEN( irq5_line_hold )		{ if (interrupt_enabled(device)) cpu_set_input_line(device, 5, HOLD_LINE); }
INTERRUPT_GEN( irq5_line_pulse )	{ if (interrupt_enabled(device)) generic_pulse_irq_line(device, 5); }
INTERRUPT_GEN( irq5_line_assert )	{ if (interrupt_enabled(device)) cpu_set_input_line(device, 5, ASSERT_LINE); }

INTERRUPT_GEN( irq6_line_hold )		{ if (interrupt_enabled(device)) cpu_set_input_line(device, 6, HOLD_LINE); }
INTERRUPT_GEN( irq6_line_pulse )	{ if (interrupt_enabled(device)) generic_pulse_irq_line(device, 6); }
INTERRUPT_GEN( irq6_line_assert )	{ if (interrupt_enabled(device)) cpu_set_input_line(device, 6, ASSERT_LINE); }

INTERRUPT_GEN( irq7_line_hold )		{ if (interrupt_enabled(device)) cpu_set_input_line(device, 7, HOLD_LINE); }
INTERRUPT_GEN( irq7_line_pulse )	{ if (interrupt_enabled(device)) generic_pulse_irq_line(device, 7); }
INTERRUPT_GEN( irq7_line_assert )	{ if (interrupt_enabled(device)) cpu_set_input_line(device, 7, ASSERT_LINE); }



/***************************************************************************
    WATCHDOG READ/WRITE HELPERS
***************************************************************************/

/*-------------------------------------------------
    8-bit reset read/write handlers
-------------------------------------------------*/

WRITE8_HANDLER( watchdog_reset_w ) { watchdog_reset(space->machine); }
READ8_HANDLER( watchdog_reset_r ) { watchdog_reset(space->machine); return space->unmap; }


/*-------------------------------------------------
    16-bit reset read/write handlers
-------------------------------------------------*/

WRITE16_HANDLER( watchdog_reset16_w ) {	watchdog_reset(space->machine); }
READ16_HANDLER( watchdog_reset16_r ) { watchdog_reset(space->machine); return space->unmap; }


/*-------------------------------------------------
    32-bit reset read/write handlers
-------------------------------------------------*/

WRITE32_HANDLER( watchdog_reset32_w ) {	watchdog_reset(space->machine); }
READ32_HANDLER( watchdog_reset32_r ) { watchdog_reset(space->machine); return space->unmap; }



/***************************************************************************
    PORT READING HELPERS
***************************************************************************/

/*-------------------------------------------------
    custom_port_read - act like input_port_read
    but it is a custom port, it is useful for
    e.g. input ports which expect the same port
    repeated both in the upper and lower half
-------------------------------------------------*/

CUSTOM_INPUT( custom_port_read )
{
	const char *tag = (const char *)param;
	return input_port_read(field->port->machine, tag);
}
