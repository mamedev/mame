/*********************************************************************

    generic.c

    Generic simple machine functions.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "driver.h"
#include "config.h"
#include "generic.h"
#include <stdarg.h>
#include <ctype.h>



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* These globals are only kept on a machine basis - LBO 042898 */
UINT32 dispensed_tickets;
UINT32 coin_count[COIN_COUNTERS];
UINT32 coinlockedout[COIN_COUNTERS];
UINT32 servicecoinlockedout[COIN_COUNTERS];
static UINT32 lastcoin[COIN_COUNTERS];

/* generic NVRAM */
size_t generic_nvram_size;
UINT8 *generic_nvram;
UINT16 *generic_nvram16;
UINT32 *generic_nvram32;

/* memory card status */
static int memcard_inserted;

/* interrupt status */
static UINT8 interrupt_enable[MAX_CPU];




/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void counters_load(int config_type, xml_data_node *parentnode);
static void counters_save(int config_type, xml_data_node *parentnode);
static void interrupt_reset(running_machine *machine);



/***************************************************************************
    INITIALIZATION
***************************************************************************/

/*-------------------------------------------------
    generic_machine_init - initialize globals and
    register for save states
-------------------------------------------------*/

void generic_machine_init(running_machine *machine)
{
	int counternum;

	/* reset coin counters */
	for (counternum = 0; counternum < COIN_COUNTERS; counternum++)
	{
		lastcoin[counternum] = 0;
		coinlockedout[counternum] = 0;
		servicecoinlockedout[counternum] = 0;
	}

	/* reset NVRAM size and pointers */
	generic_nvram_size = 0;
	generic_nvram = NULL;
	generic_nvram16 = NULL;
	generic_nvram32 = NULL;

	/* reset memory card info */
	memcard_inserted = -1;

	/* register a reset callback and save state for interrupt enable */
	add_reset_callback(machine, interrupt_reset);
	state_save_register_item_array("cpu", 0, interrupt_enable);

	/* register for configuration */
	config_register("counters", counters_load, counters_save);

	/* for memory cards, request save state and an exit callback */
	if (machine->drv->memcard_handler != NULL)
	{
		state_save_register_global(memcard_inserted);
		add_exit_callback(machine, memcard_eject);
	}
}



/***************************************************************************
    COIN COUNTERS
***************************************************************************/

/*-------------------------------------------------
    counters_load - load the state of the counters
    and tickets
-------------------------------------------------*/

static void counters_load(int config_type, xml_data_node *parentnode)
{
	xml_data_node *coinnode, *ticketnode;

	/* on init, reset the counters */
	if (config_type == CONFIG_TYPE_INIT)
	{
		memset(coin_count, 0, sizeof(coin_count));
		dispensed_tickets = 0;
	}

	/* only care about game-specific data */
	if (config_type != CONFIG_TYPE_GAME)
		return;

	/* might not have any data */
	if (!parentnode)
		return;

	/* iterate over coins nodes */
	for (coinnode = xml_get_sibling(parentnode->child, "coins"); coinnode; coinnode = xml_get_sibling(coinnode->next, "coins"))
	{
		int index = xml_get_attribute_int(coinnode, "index", -1);
		if (index >= 0 && index < COIN_COUNTERS)
			coin_count[index] = xml_get_attribute_int(coinnode, "number", 0);
	}

	/* get the single tickets node */
	ticketnode = xml_get_sibling(parentnode->child, "tickets");
	if (ticketnode)
		dispensed_tickets = xml_get_attribute_int(ticketnode, "number", 0);
}


/*-------------------------------------------------
    counters_save - save the state of the counters
    and tickets
-------------------------------------------------*/

static void counters_save(int config_type, xml_data_node *parentnode)
{
	int i;

	/* only care about game-specific data */
	if (config_type != CONFIG_TYPE_GAME)
		return;

	/* iterate over coin counters */
	for (i = 0; i < COIN_COUNTERS; i++)
		if (coin_count[i] != 0)
		{
			xml_data_node *coinnode = xml_add_child(parentnode, "coins", NULL);
			if (coinnode)
			{
				xml_set_attribute_int(coinnode, "index", i);
				xml_set_attribute_int(coinnode, "number", coin_count[i]);
			}
		}

	/* output tickets */
	if (dispensed_tickets != 0)
	{
		xml_data_node *tickets = xml_add_child(parentnode, "tickets", NULL);
		if (tickets)
			xml_set_attribute_int(tickets, "number", dispensed_tickets);
	}
}


/*-------------------------------------------------
    coin_counter_w - sets input for coin counter
-------------------------------------------------*/

void coin_counter_w(int num,int on)
{
	if (num >= COIN_COUNTERS) return;
	/* Count it only if the data has changed from 0 to non-zero */
	if (on && (lastcoin[num] == 0))
	{
		coin_count[num]++;
	}
	lastcoin[num] = on;
}


/*-------------------------------------------------
    coin_lockout_w - locks out one coin input
-------------------------------------------------*/

void coin_lockout_w(int num,int on)
{
	if (num >= COIN_COUNTERS) return;

	coinlockedout[num] = on;
}


/*-------------------------------------------------
    service_coin_lockout_w - locks out one coin input
-------------------------------------------------*/

void service_coin_lockout_w(int num,int on)
{
	if (num >= COIN_COUNTERS) return;

	servicecoinlockedout[num] = on;
}


/*-------------------------------------------------
    coin_lockout_global_w - locks out all the coin
    inputs
-------------------------------------------------*/

void coin_lockout_global_w(int on)
{
	int i;

	for (i = 0; i < COIN_COUNTERS; i++)
	{
		coin_lockout_w(i,on);
	}
}



/***************************************************************************
    NVRAM MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    nvram_select - select the right pointer based
    on which ones are non-NULL
-------------------------------------------------*/

INLINE void *nvram_select(void)
{
	if (generic_nvram)
		return generic_nvram;
	if (generic_nvram16)
		return generic_nvram16;
	if (generic_nvram32)
		return generic_nvram32;
	fatalerror("generic nvram handler called without nvram in the memory map");
	return 0;
}


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

void nvram_load(void)
{
	if (Machine->drv->nvram_handler != NULL)
	{
		mame_file *nvram_file = nvram_fopen(Machine, OPEN_FLAG_READ);
		(*Machine->drv->nvram_handler)(Machine, nvram_file, 0);
		if (nvram_file != NULL)
			mame_fclose(nvram_file);
	}
}


/*-------------------------------------------------
    nvram_save - save a system's NVRAM
-------------------------------------------------*/

void nvram_save(void)
{
	if (Machine->drv->nvram_handler != NULL)
	{
		mame_file *nvram_file = nvram_fopen(Machine, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
		if (nvram_file != NULL)
		{
			(*Machine->drv->nvram_handler)(Machine, nvram_file, 1);
			mame_fclose(nvram_file);
		}
	}
}


/*-------------------------------------------------
    nvram_handler_generic_0fill - generic NVRAM
    with a 0 fill
-------------------------------------------------*/

NVRAM_HANDLER( generic_0fill )
{
	if (read_or_write)
		mame_fwrite(file, nvram_select(), generic_nvram_size);
	else if (file)
		mame_fread(file, nvram_select(), generic_nvram_size);
	else
		memset(nvram_select(), 0, generic_nvram_size);
}


/*-------------------------------------------------
    nvram_handler_generic_1fill - generic NVRAM
    with a 1 fill
-------------------------------------------------*/

NVRAM_HANDLER( generic_1fill )
{
	if (read_or_write)
		mame_fwrite(file, nvram_select(), generic_nvram_size);
	else if (file)
		mame_fread(file, nvram_select(), generic_nvram_size);
	else
		memset(nvram_select(), 0xff, generic_nvram_size);
}


/*-------------------------------------------------
    nvram_handler_generic_randfill - generic NVRAM
    with a random fill
-------------------------------------------------*/

NVRAM_HANDLER( generic_randfill )
{
	int i;

	if (read_or_write)
		mame_fwrite(file, nvram_select(), generic_nvram_size);
	else if (file)
		mame_fread(file, nvram_select(), generic_nvram_size);
	else
	{
		UINT8 *nvram = nvram_select();
		for (i = 0; i < generic_nvram_size; i++)
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

int memcard_create(int index, int overwrite)
{
	file_error filerr;
	mame_file *file;
	astring *fname;
	char name[16];

	/* create a name */
	memcard_name(index, name);

	/* if we can't overwrite, fail if the file already exists */
	fname = astring_assemble_3(astring_alloc(), Machine->basename, PATH_SEPARATOR, name);
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
	if (Machine->drv->memcard_handler)
		(*Machine->drv->memcard_handler)(Machine, file, MEMCARD_CREATE);

	/* close the file */
	mame_fclose(file);
	return 0;
}


/*-------------------------------------------------
    memcard_insert - insert an existing memory card
    with the given index
-------------------------------------------------*/

int memcard_insert(int index)
{
	file_error filerr;
	mame_file *file;
	char name[16];
	astring *fname;

	/* if a card is already inserted, eject it first */
	if (memcard_inserted != -1)
		memcard_eject(Machine);
	assert(memcard_inserted == -1);

	/* create a name */
	memcard_name(index, name);
	fname = astring_assemble_3(astring_alloc(), Machine->basename, PATH_SEPARATOR, name);

	/* open the file; if we can't, it's an error */
	filerr = mame_fopen(SEARCHPATH_MEMCARD, astring_c(fname), OPEN_FLAG_READ, &file);
	astring_free(fname);
	if (filerr != FILERR_NONE)
		return 1;

	/* initialize and then load the card */
	if (Machine->drv->memcard_handler)
		(*Machine->drv->memcard_handler)(Machine, file, MEMCARD_INSERT);

	/* close the file */
	mame_fclose(file);
	memcard_inserted = index;
	return 0;
}


/*-------------------------------------------------
    memcard_eject - eject a memory card, saving
    its contents along the way
-------------------------------------------------*/

void memcard_eject(running_machine *machine)
{
	file_error filerr;
	mame_file *file;
	char name[16];
	astring *fname;

	/* if no card is preset, just ignore */
	if (memcard_inserted == -1)
		return;

	/* create a name */
	memcard_name(memcard_inserted, name);
	fname = astring_assemble_3(astring_alloc(), Machine->basename, PATH_SEPARATOR, name);

	/* open the file; if we can't, it's an error */
	filerr = mame_fopen(SEARCHPATH_MEMCARD, astring_c(fname), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS, &file);
	astring_free(fname);
	if (filerr != FILERR_NONE)
	{
		mame_fclose(file);
		return;
	}

	/* initialize and then load the card */
	if (machine->drv->memcard_handler)
		(*machine->drv->memcard_handler)(machine, file, MEMCARD_EJECT);

	/* close the file */
	mame_fclose(file);
	memcard_inserted = -1;
}


/*-------------------------------------------------
    memcard_present - return the currently loaded
    card index, or -1 if none
-------------------------------------------------*/

int memcard_present(void)
{
	return memcard_inserted;
}



/***************************************************************************
    LED CODE
***************************************************************************/

/*-------------------------------------------------
    set_led_status - set the state of a given LED
-------------------------------------------------*/

void set_led_status(int num, int on)
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
	int cpunum;

	/* on a reset, enable all interrupts */
	for (cpunum = 0; cpunum < cpu_gettotalcpu(); cpunum++)
		interrupt_enable[cpunum] = 1;
}


/*-------------------------------------------------
    clear_all_lines - sets the state of all input
    lines and the NMI line to clear
-------------------------------------------------*/

static TIMER_CALLBACK( clear_all_lines )
{
	int cpunum = param;
	int inputcount = cpunum_input_lines(cpunum);
	int line;

	/* clear NMI and all inputs */
	cpunum_set_input_line(cpunum, INPUT_LINE_NMI, CLEAR_LINE);
	for (line = 0; line < inputcount; line++)
		cpunum_set_input_line(cpunum, line, CLEAR_LINE);
}


/*-------------------------------------------------
    cpu_interrupt_enable - controls the enable/
    disable value for global interrupts
-------------------------------------------------*/

void cpu_interrupt_enable(int cpunum, int enabled)
{
	assert_always(cpunum >= 0 && cpunum < cpu_gettotalcpu(), "cpu_interrupt_enable() called for invalid cpu num!");

	/* set the new state */
	interrupt_enable[cpunum] = enabled;

	/* make sure there are no queued interrupts */
	if (enabled == 0)
		timer_call_after_resynch(cpunum, clear_all_lines);
}


/*-------------------------------------------------
    interrupt_enable_w - set the global interrupt
    enable
-------------------------------------------------*/

WRITE8_HANDLER( interrupt_enable_w )
{
	int activecpu = cpu_getactivecpu();
	assert_always(activecpu >= 0, "interrupt_enable_w() called with no active cpu!");
	cpu_interrupt_enable(activecpu, data);
}


/*-------------------------------------------------
    interrupt_enable_r - read the global interrupt
    enable
-------------------------------------------------*/

READ8_HANDLER( interrupt_enable_r )
{
	int activecpu = cpu_getactivecpu();
	assert_always(activecpu >= 0, "interrupt_enable_r() called with no active cpu!");
	return interrupt_enable[activecpu];
}



/***************************************************************************
    INTERRUPT GENERATION CALLBACK HELPERS
***************************************************************************/

/*-------------------------------------------------
    irqn_line_set - set the given IRQ line to the
    specified state on the active CPU
-------------------------------------------------*/

INLINE void irqn_line_set(int line, int state)
{
	int cpunum = cpu_getactivecpu();
	if (interrupt_enable[cpunum])
		cpunum_set_input_line(cpunum, line, state);
}


/*-------------------------------------------------
    NMI callbacks
-------------------------------------------------*/

INTERRUPT_GEN( nmi_line_pulse )		{ irqn_line_set(INPUT_LINE_NMI, PULSE_LINE); }
INTERRUPT_GEN( nmi_line_assert )	{ irqn_line_set(INPUT_LINE_NMI, ASSERT_LINE); }


/*-------------------------------------------------
    IRQn callbacks
-------------------------------------------------*/

INTERRUPT_GEN( irq0_line_hold )		{ irqn_line_set(0, HOLD_LINE); }
INTERRUPT_GEN( irq0_line_pulse )	{ irqn_line_set(0, PULSE_LINE); }
INTERRUPT_GEN( irq0_line_assert )	{ irqn_line_set(0, ASSERT_LINE); }

INTERRUPT_GEN( irq1_line_hold )		{ irqn_line_set(1, HOLD_LINE); }
INTERRUPT_GEN( irq1_line_pulse )	{ irqn_line_set(1, PULSE_LINE); }
INTERRUPT_GEN( irq1_line_assert )	{ irqn_line_set(1, ASSERT_LINE); }

INTERRUPT_GEN( irq2_line_hold )		{ irqn_line_set(2, HOLD_LINE); }
INTERRUPT_GEN( irq2_line_pulse )	{ irqn_line_set(2, PULSE_LINE); }
INTERRUPT_GEN( irq2_line_assert )	{ irqn_line_set(2, ASSERT_LINE); }

INTERRUPT_GEN( irq3_line_hold )		{ irqn_line_set(3, HOLD_LINE); }
INTERRUPT_GEN( irq3_line_pulse )	{ irqn_line_set(3, PULSE_LINE); }
INTERRUPT_GEN( irq3_line_assert )	{ irqn_line_set(3, ASSERT_LINE); }

INTERRUPT_GEN( irq4_line_hold )		{ irqn_line_set(4, HOLD_LINE); }
INTERRUPT_GEN( irq4_line_pulse )	{ irqn_line_set(4, PULSE_LINE); }
INTERRUPT_GEN( irq4_line_assert )	{ irqn_line_set(4, ASSERT_LINE); }

INTERRUPT_GEN( irq5_line_hold )		{ irqn_line_set(5, HOLD_LINE); }
INTERRUPT_GEN( irq5_line_pulse )	{ irqn_line_set(5, PULSE_LINE); }
INTERRUPT_GEN( irq5_line_assert )	{ irqn_line_set(5, ASSERT_LINE); }

INTERRUPT_GEN( irq6_line_hold )		{ irqn_line_set(6, HOLD_LINE); }
INTERRUPT_GEN( irq6_line_pulse )	{ irqn_line_set(6, PULSE_LINE); }
INTERRUPT_GEN( irq6_line_assert )	{ irqn_line_set(6, ASSERT_LINE); }

INTERRUPT_GEN( irq7_line_hold )		{ irqn_line_set(7, HOLD_LINE); }
INTERRUPT_GEN( irq7_line_pulse )	{ irqn_line_set(7, PULSE_LINE); }
INTERRUPT_GEN( irq7_line_assert )	{ irqn_line_set(7, ASSERT_LINE); }



/***************************************************************************
    WATCHDOG READ/WRITE HELPERS
***************************************************************************/

/*-------------------------------------------------
    8-bit reset read/write handlers
-------------------------------------------------*/

WRITE8_HANDLER( watchdog_reset_w ) { watchdog_reset(); }
READ8_HANDLER( watchdog_reset_r ) { watchdog_reset(); return 0xff; }


/*-------------------------------------------------
    16-bit reset read/write handlers
-------------------------------------------------*/

WRITE16_HANDLER( watchdog_reset16_w ) {	watchdog_reset(); }
READ16_HANDLER( watchdog_reset16_r ) { watchdog_reset(); return 0xffff; }


/*-------------------------------------------------
    32-bit reset read/write handlers
-------------------------------------------------*/

WRITE32_HANDLER( watchdog_reset32_w ) {	watchdog_reset(); }
READ32_HANDLER( watchdog_reset32_r ) { watchdog_reset(); return 0xffffffff; }



/***************************************************************************
    PORT READING HELPERS
***************************************************************************/

/*-------------------------------------------------
    8-bit read handlers
-------------------------------------------------*/

READ8_HANDLER( input_port_0_r ) { return readinputport(0); }
READ8_HANDLER( input_port_1_r ) { return readinputport(1); }
READ8_HANDLER( input_port_2_r ) { return readinputport(2); }
READ8_HANDLER( input_port_3_r ) { return readinputport(3); }
READ8_HANDLER( input_port_4_r ) { return readinputport(4); }
READ8_HANDLER( input_port_5_r ) { return readinputport(5); }
READ8_HANDLER( input_port_6_r ) { return readinputport(6); }
READ8_HANDLER( input_port_7_r ) { return readinputport(7); }
READ8_HANDLER( input_port_8_r ) { return readinputport(8); }
READ8_HANDLER( input_port_9_r ) { return readinputport(9); }
READ8_HANDLER( input_port_10_r ) { return readinputport(10); }
READ8_HANDLER( input_port_11_r ) { return readinputport(11); }
READ8_HANDLER( input_port_12_r ) { return readinputport(12); }
READ8_HANDLER( input_port_13_r ) { return readinputport(13); }
READ8_HANDLER( input_port_14_r ) { return readinputport(14); }
READ8_HANDLER( input_port_15_r ) { return readinputport(15); }
READ8_HANDLER( input_port_16_r ) { return readinputport(16); }
READ8_HANDLER( input_port_17_r ) { return readinputport(17); }
READ8_HANDLER( input_port_18_r ) { return readinputport(18); }
READ8_HANDLER( input_port_19_r ) { return readinputport(19); }
READ8_HANDLER( input_port_20_r ) { return readinputport(20); }
READ8_HANDLER( input_port_21_r ) { return readinputport(21); }
READ8_HANDLER( input_port_22_r ) { return readinputport(22); }
READ8_HANDLER( input_port_23_r ) { return readinputport(23); }
READ8_HANDLER( input_port_24_r ) { return readinputport(24); }
READ8_HANDLER( input_port_25_r ) { return readinputport(25); }
READ8_HANDLER( input_port_26_r ) { return readinputport(26); }
READ8_HANDLER( input_port_27_r ) { return readinputport(27); }
READ8_HANDLER( input_port_28_r ) { return readinputport(28); }
READ8_HANDLER( input_port_29_r ) { return readinputport(29); }
READ8_HANDLER( input_port_30_r ) { return readinputport(30); }
READ8_HANDLER( input_port_31_r ) { return readinputport(31); }


/*-------------------------------------------------
    16-bit read handlers
-------------------------------------------------*/

READ16_HANDLER( input_port_0_word_r ) { return readinputport(0); }
READ16_HANDLER( input_port_1_word_r ) { return readinputport(1); }
READ16_HANDLER( input_port_2_word_r ) { return readinputport(2); }
READ16_HANDLER( input_port_3_word_r ) { return readinputport(3); }
READ16_HANDLER( input_port_4_word_r ) { return readinputport(4); }
READ16_HANDLER( input_port_5_word_r ) { return readinputport(5); }
READ16_HANDLER( input_port_6_word_r ) { return readinputport(6); }
READ16_HANDLER( input_port_7_word_r ) { return readinputport(7); }
READ16_HANDLER( input_port_8_word_r ) { return readinputport(8); }
READ16_HANDLER( input_port_9_word_r ) { return readinputport(9); }
READ16_HANDLER( input_port_10_word_r ) { return readinputport(10); }
READ16_HANDLER( input_port_11_word_r ) { return readinputport(11); }
READ16_HANDLER( input_port_12_word_r ) { return readinputport(12); }
READ16_HANDLER( input_port_13_word_r ) { return readinputport(13); }
READ16_HANDLER( input_port_14_word_r ) { return readinputport(14); }
READ16_HANDLER( input_port_15_word_r ) { return readinputport(15); }
READ16_HANDLER( input_port_16_word_r ) { return readinputport(16); }
READ16_HANDLER( input_port_17_word_r ) { return readinputport(17); }
READ16_HANDLER( input_port_18_word_r ) { return readinputport(18); }
READ16_HANDLER( input_port_19_word_r ) { return readinputport(19); }
READ16_HANDLER( input_port_20_word_r ) { return readinputport(20); }
READ16_HANDLER( input_port_21_word_r ) { return readinputport(21); }
READ16_HANDLER( input_port_22_word_r ) { return readinputport(22); }
READ16_HANDLER( input_port_23_word_r ) { return readinputport(23); }
READ16_HANDLER( input_port_24_word_r ) { return readinputport(24); }
READ16_HANDLER( input_port_25_word_r ) { return readinputport(25); }
READ16_HANDLER( input_port_26_word_r ) { return readinputport(26); }
READ16_HANDLER( input_port_27_word_r ) { return readinputport(27); }
READ16_HANDLER( input_port_28_word_r ) { return readinputport(28); }
READ16_HANDLER( input_port_29_word_r ) { return readinputport(29); }
READ16_HANDLER( input_port_30_word_r ) { return readinputport(30); }
READ16_HANDLER( input_port_31_word_r ) { return readinputport(31); }


/*-------------------------------------------------
    32-bit read handlers
-------------------------------------------------*/

READ32_HANDLER( input_port_0_dword_r ) { return readinputport(0); }
READ32_HANDLER( input_port_1_dword_r ) { return readinputport(1); }
READ32_HANDLER( input_port_2_dword_r ) { return readinputport(2); }
READ32_HANDLER( input_port_3_dword_r ) { return readinputport(3); }
READ32_HANDLER( input_port_4_dword_r ) { return readinputport(4); }
READ32_HANDLER( input_port_5_dword_r ) { return readinputport(5); }
READ32_HANDLER( input_port_6_dword_r ) { return readinputport(6); }
READ32_HANDLER( input_port_7_dword_r ) { return readinputport(7); }
READ32_HANDLER( input_port_8_dword_r ) { return readinputport(8); }
READ32_HANDLER( input_port_9_dword_r ) { return readinputport(9); }
READ32_HANDLER( input_port_10_dword_r ) { return readinputport(10); }
READ32_HANDLER( input_port_11_dword_r ) { return readinputport(11); }
READ32_HANDLER( input_port_12_dword_r ) { return readinputport(12); }
READ32_HANDLER( input_port_13_dword_r ) { return readinputport(13); }
READ32_HANDLER( input_port_14_dword_r ) { return readinputport(14); }
READ32_HANDLER( input_port_15_dword_r ) { return readinputport(15); }
READ32_HANDLER( input_port_16_dword_r ) { return readinputport(16); }
READ32_HANDLER( input_port_17_dword_r ) { return readinputport(17); }
READ32_HANDLER( input_port_18_dword_r ) { return readinputport(18); }
READ32_HANDLER( input_port_19_dword_r ) { return readinputport(19); }
READ32_HANDLER( input_port_20_dword_r ) { return readinputport(20); }
READ32_HANDLER( input_port_21_dword_r ) { return readinputport(21); }
READ32_HANDLER( input_port_22_dword_r ) { return readinputport(22); }
READ32_HANDLER( input_port_23_dword_r ) { return readinputport(23); }
READ32_HANDLER( input_port_24_dword_r ) { return readinputport(24); }
READ32_HANDLER( input_port_25_dword_r ) { return readinputport(25); }
READ32_HANDLER( input_port_26_dword_r ) { return readinputport(26); }
READ32_HANDLER( input_port_27_dword_r ) { return readinputport(27); }
READ32_HANDLER( input_port_28_dword_r ) { return readinputport(28); }
READ32_HANDLER( input_port_29_dword_r ) { return readinputport(29); }
READ32_HANDLER( input_port_30_dword_r ) { return readinputport(30); }
READ32_HANDLER( input_port_31_dword_r ) { return readinputport(31); }

