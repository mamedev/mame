/***************************************************************************

    atarigen.c

    General functions for Atari raster games.

***************************************************************************/


#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/2151intf.h"
#include "sound/2413intf.h"
#include "sound/tms5220.h"
#include "sound/okim6295.h"
#include "sound/pokey.h"
#include "includes/slapstic.h"
#include "atarigen.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define SOUND_TIMER_RATE			ATTOTIME_IN_USEC(5)
#define SOUND_TIMER_BOOST			ATTOTIME_IN_USEC(100)



/***************************************************************************
    STATIC FUNCTION DECLARATIONS
***************************************************************************/

static STATE_POSTLOAD( slapstic_postload );

static TIMER_CALLBACK( scanline_interrupt_callback );

static void decompress_eeprom_word(UINT16 *dest, const UINT16 *data);
static void decompress_eeprom_byte(UINT8 *dest, const UINT16 *data);

static void update_6502_irq(running_machine *machine);
static TIMER_CALLBACK( delayed_sound_reset );
static TIMER_CALLBACK( delayed_sound_w );
static TIMER_CALLBACK( delayed_6502_sound_w );

static void atarigen_set_vol(running_machine *machine, int volume, device_type type);

static TIMER_CALLBACK( scanline_timer_callback );

static void atarivc_common_w(screen_device &screen, offs_t offset, UINT16 newword);

static TIMER_CALLBACK( unhalt_cpu );

static TIMER_CALLBACK( atarivc_eof_update );



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE const atarigen_screen_timer *get_screen_timer(screen_device &screen)
{
	atarigen_state *state = (atarigen_state *)screen.machine->driver_data;
	int i;

	/* find the index of the timer that matches the screen */
	for (i = 0; i < ARRAY_LENGTH(state->screen_timer); i++)
		if (state->screen_timer[i].screen == &screen)
			return &state->screen_timer[i];

	fatalerror("Unexpected: no atarivc_eof_update_timer for screen '%s'\n", screen.tag());
	return NULL;
}



/***************************************************************************
    OVERALL INIT
***************************************************************************/

void atarigen_init(running_machine *machine)
{
	atarigen_state *state = (atarigen_state *)machine->driver_data;
	screen_device *screen;
	int i;

	/* allocate timers for all screens */
	assert(screen_count(*machine) <= ARRAY_LENGTH(state->screen_timer));
	for (i = 0, screen = screen_first(*machine); screen != NULL; i++, screen = screen_next(screen))
	{
		state->screen_timer[i].screen = screen;
		state->screen_timer[i].scanline_interrupt_timer = timer_alloc(machine, scanline_interrupt_callback, (void *)screen);
		state->screen_timer[i].scanline_timer = timer_alloc(machine, scanline_timer_callback, (void *)screen);
		state->screen_timer[i].atarivc_eof_update_timer = timer_alloc(machine, atarivc_eof_update, (void *)screen);
	}

	state_save_register_global(machine, state->scanline_int_state);
	state_save_register_global(machine, state->sound_int_state);
	state_save_register_global(machine, state->video_int_state);

	state_save_register_global(machine, state->cpu_to_sound_ready);
	state_save_register_global(machine, state->sound_to_cpu_ready);

	state_save_register_global(machine, state->atarivc_state.latch1);				/* latch #1 value (-1 means disabled) */
	state_save_register_global(machine, state->atarivc_state.latch2);				/* latch #2 value (-1 means disabled) */
	state_save_register_global(machine, state->atarivc_state.rowscroll_enable);		/* true if row-scrolling is enabled */
	state_save_register_global(machine, state->atarivc_state.palette_bank);			/* which palette bank is enabled */
	state_save_register_global(machine, state->atarivc_state.pf0_xscroll);			/* playfield 1 xscroll */
	state_save_register_global(machine, state->atarivc_state.pf0_xscroll_raw);		/* playfield 1 xscroll raw value */
	state_save_register_global(machine, state->atarivc_state.pf0_yscroll);			/* playfield 1 yscroll */
	state_save_register_global(machine, state->atarivc_state.pf1_xscroll);			/* playfield 2 xscroll */
	state_save_register_global(machine, state->atarivc_state.pf1_xscroll_raw);		/* playfield 2 xscroll raw value */
	state_save_register_global(machine, state->atarivc_state.pf1_yscroll);			/* playfield 2 yscroll */
	state_save_register_global(machine, state->atarivc_state.mo_xscroll);			/* sprite xscroll */
	state_save_register_global(machine, state->atarivc_state.mo_yscroll);			/* sprite xscroll */

	state_save_register_global(machine, state->eeprom_unlocked);

	state_save_register_global(machine, state->slapstic_num);
	state_save_register_global(machine, state->slapstic_bank);
	state_save_register_global(machine, state->slapstic_last_pc);
	state_save_register_global(machine, state->slapstic_last_address);

	state_save_register_global(machine, state->cpu_to_sound);
	state_save_register_global(machine, state->sound_to_cpu);
	state_save_register_global(machine, state->timed_int);
	state_save_register_global(machine, state->ym2151_int);

	state_save_register_global(machine, state->scanlines_per_callback);

	state_save_register_global(machine, state->actual_vc_latch0);
	state_save_register_global(machine, state->actual_vc_latch1);

	state_save_register_global(machine, state->playfield_latch);
	state_save_register_global(machine, state->playfield2_latch);

	/* need a postload to reset the state */
	state_save_register_postload(machine, slapstic_postload, NULL);
}



/***************************************************************************
    INTERRUPT HANDLING
***************************************************************************/

/*---------------------------------------------------------------
    atarigen_interrupt_reset: Initializes the state of all
    the interrupt sources.
---------------------------------------------------------------*/

void atarigen_interrupt_reset(atarigen_state *state, atarigen_int_func update_int)
{
	/* set the callback */
	state->update_int_callback = update_int;

	/* reset the interrupt states */
	state->video_int_state = state->sound_int_state = state->scanline_int_state = 0;
}


/*---------------------------------------------------------------
    atarigen_update_interrupts: Forces the interrupt callback
    to be called with the current VBLANK and sound interrupt
    states.
---------------------------------------------------------------*/

void atarigen_update_interrupts(running_machine *machine)
{
	atarigen_state *state = (atarigen_state *)machine->driver_data;
	(*state->update_int_callback)(machine);
}


/*---------------------------------------------------------------
    atarigen_scanline_int_set: Sets the scanline when the next
    scanline interrupt should be generated.
---------------------------------------------------------------*/

void atarigen_scanline_int_set(screen_device &screen, int scanline)
{
	emu_timer *timer = get_screen_timer(screen)->scanline_interrupt_timer;
	timer_adjust_oneshot(timer, screen.time_until_pos(scanline), 0);
}


/*---------------------------------------------------------------
    atarigen_scanline_int_gen: Standard interrupt routine
    which sets the scanline interrupt state.
---------------------------------------------------------------*/

INTERRUPT_GEN( atarigen_scanline_int_gen )
{
	atarigen_state *state = (atarigen_state *)device->machine->driver_data;
	state->scanline_int_state = 1;
	(*state->update_int_callback)(device->machine);
}


/*---------------------------------------------------------------
    atarigen_scanline_int_ack_w: Resets the state of the
    scanline interrupt.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_scanline_int_ack_w )
{
	atarigen_state *state = (atarigen_state *)space->machine->driver_data;
	state->scanline_int_state = 0;
	(*state->update_int_callback)(space->machine);
}

WRITE32_HANDLER( atarigen_scanline_int_ack32_w )
{
	atarigen_state *state = (atarigen_state *)space->machine->driver_data;
	state->scanline_int_state = 0;
	(*state->update_int_callback)(space->machine);
}


/*---------------------------------------------------------------
    atarigen_sound_int_gen: Standard interrupt routine which
    sets the sound interrupt state.
---------------------------------------------------------------*/

INTERRUPT_GEN( atarigen_sound_int_gen )
{
	atarigen_state *state = (atarigen_state *)device->machine->driver_data;
	state->sound_int_state = 1;
	(*state->update_int_callback)(device->machine);
}


/*---------------------------------------------------------------
    atarigen_sound_int_ack_w: Resets the state of the sound
    interrupt.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_sound_int_ack_w )
{
	atarigen_state *state = (atarigen_state *)space->machine->driver_data;
	state->sound_int_state = 0;
	(*state->update_int_callback)(space->machine);
}

WRITE32_HANDLER( atarigen_sound_int_ack32_w )
{
	atarigen_state *state = (atarigen_state *)space->machine->driver_data;
	state->sound_int_state = 0;
	(*state->update_int_callback)(space->machine);
}


/*---------------------------------------------------------------
    atarigen_video_int_gen: Standard interrupt routine which
    sets the video interrupt state.
---------------------------------------------------------------*/

INTERRUPT_GEN( atarigen_video_int_gen )
{
	atarigen_state *state = (atarigen_state *)device->machine->driver_data;
	state->video_int_state = 1;
	(*state->update_int_callback)(device->machine);
}


/*---------------------------------------------------------------
    atarigen_video_int_ack_w: Resets the state of the video
    interrupt.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_video_int_ack_w )
{
	atarigen_state *state = (atarigen_state *)space->machine->driver_data;
	state->video_int_state = 0;
	(*state->update_int_callback)(space->machine);
}

WRITE32_HANDLER( atarigen_video_int_ack32_w )
{
	atarigen_state *state = (atarigen_state *)space->machine->driver_data;
	state->video_int_state = 0;
	(*state->update_int_callback)(space->machine);
}


/*---------------------------------------------------------------
    scanline_interrupt_callback: Signals an interrupt.
---------------------------------------------------------------*/

static TIMER_CALLBACK( scanline_interrupt_callback )
{
	screen_device &screen = *reinterpret_cast<screen_device *>(ptr);
	emu_timer *timer = get_screen_timer(screen)->scanline_interrupt_timer;

	/* generate the interrupt */
	atarigen_scanline_int_gen(devtag_get_device(machine, "maincpu"));

	/* set a new timer to go off at the same scan line next frame */
	timer_adjust_oneshot(timer, screen.frame_period(), 0);
}



/***************************************************************************
    EEPROM HANDLING
***************************************************************************/

/*---------------------------------------------------------------
    atarigen_eeprom_reset: Makes sure that the unlocked state
    is cleared when we reset.
---------------------------------------------------------------*/

void atarigen_eeprom_reset(atarigen_state *state)
{
	state->eeprom_unlocked = 0;
}


/*---------------------------------------------------------------
    atarigen_eeprom_enable_w: Any write to this handler will
    allow one byte to be written to the EEPROM data area the
    next time.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_eeprom_enable_w )
{
	atarigen_state *state = (atarigen_state *)space->machine->driver_data;
	state->eeprom_unlocked = 1;
}

WRITE32_HANDLER( atarigen_eeprom_enable32_w )
{
	atarigen_state *state = (atarigen_state *)space->machine->driver_data;
	state->eeprom_unlocked = 1;
}


/*---------------------------------------------------------------
    atarigen_eeprom_w: Writes a "word" to the EEPROM, which is
    almost always accessed via the low byte of the word only.
    If the EEPROM hasn't been unlocked, the write attempt is
    ignored.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_eeprom_w )
{
	atarigen_state *state = (atarigen_state *)space->machine->driver_data;

	if (!state->eeprom_unlocked)
		return;

	COMBINE_DATA(&state->eeprom[offset]);
	state->eeprom_unlocked = 0;
}

WRITE32_HANDLER( atarigen_eeprom32_w )
{
	atarigen_state *state = (atarigen_state *)space->machine->driver_data;

	if (!state->eeprom_unlocked)
		return;

	COMBINE_DATA(&state->eeprom[offset * 2 + 1]);
	data >>= 16;
	mem_mask >>= 16;
	COMBINE_DATA(&state->eeprom[offset * 2]);
	state->eeprom_unlocked = 0;
}


/*---------------------------------------------------------------
    atarigen_eeprom_r: Reads a "word" from the EEPROM, which is
    almost always accessed via the low byte of the word only.
---------------------------------------------------------------*/

READ16_HANDLER( atarigen_eeprom_r )
{
	atarigen_state *state = (atarigen_state *)space->machine->driver_data;
	return state->eeprom[offset] | 0xff00;
}

READ16_HANDLER( atarigen_eeprom_upper_r )
{
	atarigen_state *state = (atarigen_state *)space->machine->driver_data;
	return state->eeprom[offset] | 0x00ff;
}

READ32_HANDLER( atarigen_eeprom_upper32_r )
{
	atarigen_state *state = (atarigen_state *)space->machine->driver_data;
	return (state->eeprom[offset * 2] << 16) | state->eeprom[offset * 2 + 1] | 0x00ff00ff;
}


/*---------------------------------------------------------------
    NVRAM_HANDLER( atarigen ): Loads the EEPROM data.
---------------------------------------------------------------*/

NVRAM_HANDLER( atarigen )
{
	atarigen_state *state = (atarigen_state *)machine->driver_data;
	if (read_or_write)
		mame_fwrite(file, state->eeprom, state->eeprom_size);
	else if (file)
		mame_fread(file, state->eeprom, state->eeprom_size);
	else
	{
		/* all 0xff's work for most games */
		memset(state->eeprom, 0xff, state->eeprom_size);

		/* anything else must be decompressed */
		if (state->eeprom_default)
		{
			if (state->eeprom_default[0] == 0)
				decompress_eeprom_byte((UINT8 *)state->eeprom, state->eeprom_default + 1);
			else
				decompress_eeprom_word(state->eeprom, state->eeprom_default + 1);
		}
	}
}


/*---------------------------------------------------------------
    decompress_eeprom_word: Used for decompressing EEPROM data
    that has every other byte invalid.
---------------------------------------------------------------*/

void decompress_eeprom_word(UINT16 *dest, const UINT16 *data)
{
	UINT16 value;

	while ((value = *data++) != 0)
	{
		int count = (value >> 8);
		value = (value << 8) | (value & 0xff);

		while (count--)
			*dest++ = value;
	}
}


/*---------------------------------------------------------------
    decompress_eeprom_byte: Used for decompressing EEPROM data
    that is byte-packed.
---------------------------------------------------------------*/

void decompress_eeprom_byte(UINT8 *dest, const UINT16 *data)
{
	UINT16 value;

	while ((value = *data++) != 0)
	{
		int count = (value >> 8);
		value = (value << 8) | (value & 0xff);

		while (count--)
			*dest++ = value;
	}
}



/***************************************************************************
    SLAPSTIC HANDLING
***************************************************************************/

INLINE void update_bank(atarigen_state *state, int bank)
{
	/* if the bank has changed, copy the memory; Pit Fighter needs this */
	if (bank != state->slapstic_bank)
	{
		/* bank 0 comes from the copy we made earlier */
		if (bank == 0)
			memcpy(state->slapstic, state->slapstic_bank0, 0x2000);
		else
			memcpy(state->slapstic, &state->slapstic[bank * 0x1000], 0x2000);

		/* remember the current bank */
		state->slapstic_bank = bank;
	}
}


static STATE_POSTLOAD( slapstic_postload )
{
	atarigen_state *state = (atarigen_state *)machine->driver_data;
	update_bank(state, slapstic_bank());
}


static DIRECT_UPDATE_HANDLER( atarigen_slapstic_setdirect )
{
	atarigen_state *state = (atarigen_state *)space->machine->driver_data;

	/* if we jump to an address in the slapstic region, tweak the slapstic
       at that address and return ~0; this will cause us to be called on
       subsequent fetches as well */
	address &= ~state->slapstic_mirror;
	if (address >= state->slapstic_base && address < state->slapstic_base + 0x8000)
	{
		offs_t pc = cpu_get_previouspc(space->cpu);
		if (pc != state->slapstic_last_pc || address != state->slapstic_last_address)
		{
			state->slapstic_last_pc = pc;
			state->slapstic_last_address = address;
			atarigen_slapstic_r(space, (address >> 1) & 0x3fff, 0xffff);
		}
		return ~0;
	}

	return address;
}



/*---------------------------------------------------------------
    atarigen_slapstic_init: Installs memory handlers for the
    slapstic and sets the chip number.
---------------------------------------------------------------*/

void atarigen_slapstic_init(running_device *device, offs_t base, offs_t mirror, int chipnum)
{
	atarigen_state *state = (atarigen_state *)device->machine->driver_data;

	/* reset in case we have no state */
	state->slapstic_num = chipnum;
	state->slapstic = NULL;

	/* if we have a chip, install it */
	if (chipnum != 0)
	{
		/* initialize the slapstic */
		slapstic_init(device->machine, chipnum);

		/* install the memory handlers */
		state->slapstic = memory_install_readwrite16_handler(cpu_get_address_space(device, ADDRESS_SPACE_PROGRAM), base, base + 0x7fff, 0, mirror, atarigen_slapstic_r, atarigen_slapstic_w);

		/* allocate memory for a copy of bank 0 */
		state->slapstic_bank0 = auto_alloc_array(device->machine, UINT8, 0x2000);
		memcpy(state->slapstic_bank0, state->slapstic, 0x2000);

		/* ensure we recopy memory for the bank */
		state->slapstic_bank = 0xff;

		/* install an opcode base handler if we are a 68000 or variant */
		state->slapstic_base = base;
		state->slapstic_mirror = mirror;
		memory_set_direct_update_handler(cpu_get_address_space(device, ADDRESS_SPACE_PROGRAM), atarigen_slapstic_setdirect);
	}
}


/*---------------------------------------------------------------
    atarigen_slapstic_reset: Makes the selected slapstic number
    active and resets its state.
---------------------------------------------------------------*/

void atarigen_slapstic_reset(atarigen_state *state)
{
	if (state->slapstic_num != 0)
	{
		slapstic_reset();
		update_bank(state, slapstic_bank());
	}
}


/*---------------------------------------------------------------
    atarigen_slapstic_w: Assuming that the slapstic sits in
    ROM memory space, we just simply tweak the slapstic at this
    address and do nothing more.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_slapstic_w )
{
	atarigen_state *state = (atarigen_state *)space->machine->driver_data;
	update_bank(state, slapstic_tweak(space, offset));
}


/*---------------------------------------------------------------
    atarigen_slapstic_r: Tweaks the slapstic at the appropriate
    address and then reads a word from the underlying memory.
---------------------------------------------------------------*/

READ16_HANDLER( atarigen_slapstic_r )
{
	/* fetch the result from the current bank first */
	atarigen_state *state = (atarigen_state *)space->machine->driver_data;
	int result = state->slapstic[offset & 0xfff];

	/* then determine the new one */
	update_bank(state, slapstic_tweak(space, offset));
	return result;
}



/***************************************************************************
    SOUND I/O
***************************************************************************/

/*---------------------------------------------------------------
    atarigen_sound_io_reset: Resets the state of the sound I/O.
---------------------------------------------------------------*/

void atarigen_sound_io_reset(running_device *device)
{
	atarigen_state *state = (atarigen_state *)device->machine->driver_data;

	/* remember which CPU is the sound CPU */
	state->sound_cpu = device;

	/* reset the internal interrupts states */
	state->timed_int = state->ym2151_int = 0;

	/* reset the sound I/O states */
	state->cpu_to_sound = state->sound_to_cpu = 0;
	state->cpu_to_sound_ready = state->sound_to_cpu_ready = 0;
}


/*---------------------------------------------------------------
    atarigen_6502_irq_gen: Generates an IRQ signal to the 6502
    sound processor.
---------------------------------------------------------------*/

INTERRUPT_GEN( atarigen_6502_irq_gen )
{
	atarigen_state *state = (atarigen_state *)device->machine->driver_data;
	state->timed_int = 1;
	update_6502_irq(device->machine);
}


/*---------------------------------------------------------------
    atarigen_6502_irq_ack_r: Resets the IRQ signal to the 6502
    sound processor. Both reads and writes can be used.
---------------------------------------------------------------*/

READ8_HANDLER( atarigen_6502_irq_ack_r )
{
	atarigen_state *state = (atarigen_state *)space->machine->driver_data;
	state->timed_int = 0;
	update_6502_irq(space->machine);
	return 0;
}

WRITE8_HANDLER( atarigen_6502_irq_ack_w )
{
	atarigen_state *state = (atarigen_state *)space->machine->driver_data;
	state->timed_int = 0;
	update_6502_irq(space->machine);
}


/*---------------------------------------------------------------
    atarigen_ym2151_irq_gen: Sets the state of the YM2151's
    IRQ line.
---------------------------------------------------------------*/

void atarigen_ym2151_irq_gen(running_device *device, int irq)
{
	atarigen_state *state = (atarigen_state *)device->machine->driver_data;
	state->ym2151_int = irq;
	update_6502_irq(device->machine);
}


/*---------------------------------------------------------------
    atarigen_sound_reset_w: Write handler which resets the
    sound CPU in response.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_sound_reset_w )
{
	timer_call_after_resynch(space->machine, NULL, 0, delayed_sound_reset);
}


/*---------------------------------------------------------------
    atarigen_sound_reset: Resets the state of the sound CPU
    manually.
---------------------------------------------------------------*/

void atarigen_sound_reset(running_machine *machine)
{
	timer_call_after_resynch(machine, NULL, 1, delayed_sound_reset);
}


/*---------------------------------------------------------------
    atarigen_sound_w: Handles communication from the main CPU
    to the sound CPU. Two versions are provided, one with the
    data byte in the low 8 bits, and one with the data byte in
    the upper 8 bits.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_sound_w )
{
	if (ACCESSING_BITS_0_7)
		timer_call_after_resynch(space->machine, NULL, data & 0xff, delayed_sound_w);
}

WRITE16_HANDLER( atarigen_sound_upper_w )
{
	if (ACCESSING_BITS_8_15)
		timer_call_after_resynch(space->machine, NULL, (data >> 8) & 0xff, delayed_sound_w);
}

WRITE32_HANDLER( atarigen_sound_upper32_w )
{
	if (ACCESSING_BITS_24_31)
		timer_call_after_resynch(space->machine, NULL, (data >> 24) & 0xff, delayed_sound_w);
}


/*---------------------------------------------------------------
    atarigen_sound_r: Handles reading data communicated from the
    sound CPU to the main CPU. Two versions are provided, one
    with the data byte in the low 8 bits, and one with the data
    byte in the upper 8 bits.
---------------------------------------------------------------*/

READ16_HANDLER( atarigen_sound_r )
{
	atarigen_state *state = (atarigen_state *)space->machine->driver_data;
	state->sound_to_cpu_ready = 0;
	atarigen_sound_int_ack_w(space, 0, 0, 0xffff);
	return state->sound_to_cpu | 0xff00;
}

READ16_HANDLER( atarigen_sound_upper_r )
{
	atarigen_state *state = (atarigen_state *)space->machine->driver_data;
	state->sound_to_cpu_ready = 0;
	atarigen_sound_int_ack_w(space, 0, 0, 0xffff);
	return (state->sound_to_cpu << 8) | 0x00ff;
}

READ32_HANDLER( atarigen_sound_upper32_r )
{
	atarigen_state *state = (atarigen_state *)space->machine->driver_data;
	state->sound_to_cpu_ready = 0;
	atarigen_sound_int_ack32_w(space, 0, 0, 0xffff);
	return (state->sound_to_cpu << 24) | 0x00ffffff;
}


/*---------------------------------------------------------------
    atarigen_6502_sound_w: Handles communication from the sound
    CPU to the main CPU.
---------------------------------------------------------------*/

WRITE8_HANDLER( atarigen_6502_sound_w )
{
	timer_call_after_resynch(space->machine, NULL, data, delayed_6502_sound_w);
}


/*---------------------------------------------------------------
    atarigen_6502_sound_r: Handles reading data communicated
    from the main CPU to the sound CPU.
---------------------------------------------------------------*/

READ8_HANDLER( atarigen_6502_sound_r )
{
	atarigen_state *state = (atarigen_state *)space->machine->driver_data;
	state->cpu_to_sound_ready = 0;
	cpu_set_input_line(state->sound_cpu, INPUT_LINE_NMI, CLEAR_LINE);
	return state->cpu_to_sound;
}


/*---------------------------------------------------------------
    update_6502_irq: Called whenever the IRQ state changes. An
    interrupt is generated if either atarigen_6502_irq_gen()
    was called, or if the YM2151 generated an interrupt via
    the atarigen_ym2151_irq_gen() callback.
---------------------------------------------------------------*/

static void update_6502_irq(running_machine *machine)
{
	atarigen_state *state = (atarigen_state *)machine->driver_data;
	if (state->timed_int || state->ym2151_int)
		cpu_set_input_line(state->sound_cpu, M6502_IRQ_LINE, ASSERT_LINE);
	else
		cpu_set_input_line(state->sound_cpu, M6502_IRQ_LINE, CLEAR_LINE);
}


/*---------------------------------------------------------------
    delayed_sound_reset: Synchronizes the sound reset command
    between the two CPUs.
---------------------------------------------------------------*/

static TIMER_CALLBACK( delayed_sound_reset )
{
	atarigen_state *state = (atarigen_state *)machine->driver_data;
	const address_space *space = cpu_get_address_space(state->sound_cpu, ADDRESS_SPACE_PROGRAM);

	/* unhalt and reset the sound CPU */
	if (param == 0)
	{
		cpu_set_input_line(state->sound_cpu, INPUT_LINE_HALT, CLEAR_LINE);
		cpu_set_input_line(state->sound_cpu, INPUT_LINE_RESET, PULSE_LINE);
	}

	/* reset the sound write state */
	state->sound_to_cpu_ready = 0;
	atarigen_sound_int_ack_w(space, 0, 0, 0xffff);

	/* allocate a high frequency timer until a response is generated */
	/* the main CPU is *very* sensistive to the timing of the response */
	cpuexec_boost_interleave(machine, SOUND_TIMER_RATE, SOUND_TIMER_BOOST);
}


/*---------------------------------------------------------------
    delayed_sound_w: Synchronizes a data write from the main
    CPU to the sound CPU.
---------------------------------------------------------------*/

static TIMER_CALLBACK( delayed_sound_w )
{
	atarigen_state *state = (atarigen_state *)machine->driver_data;

	/* warn if we missed something */
	if (state->cpu_to_sound_ready)
		logerror("Missed command from 68010\n");

	/* set up the states and signal an NMI to the sound CPU */
	state->cpu_to_sound = param;
	state->cpu_to_sound_ready = 1;
	cpu_set_input_line(state->sound_cpu, INPUT_LINE_NMI, ASSERT_LINE);

	/* allocate a high frequency timer until a response is generated */
	/* the main CPU is *very* sensistive to the timing of the response */
	cpuexec_boost_interleave(machine, SOUND_TIMER_RATE, SOUND_TIMER_BOOST);
}


/*---------------------------------------------------------------
    delayed_6502_sound_w: Synchronizes a data write from the
    sound CPU to the main CPU.
---------------------------------------------------------------*/

static TIMER_CALLBACK( delayed_6502_sound_w )
{
	atarigen_state *state = (atarigen_state *)machine->driver_data;

	/* warn if we missed something */
	if (state->sound_to_cpu_ready)
		logerror("Missed result from 6502\n");

	/* set up the states and signal the sound interrupt to the main CPU */
	state->sound_to_cpu = param;
	state->sound_to_cpu_ready = 1;
	atarigen_sound_int_gen(devtag_get_device(machine, "maincpu"));
}



/***************************************************************************
    SOUND HELPERS
***************************************************************************/

/*---------------------------------------------------------------
    atarigen_set_vol: Scans for a particular sound chip and
    changes the volume on all channels associated with it.
---------------------------------------------------------------*/

void atarigen_set_vol(running_machine *machine, int volume, device_type type)
{
	device_sound_interface *sound = NULL;
	for (bool gotone = machine->devicelist.first(sound); gotone; gotone = sound->next(sound))
		if (sound->device().type() == type)
			sound_set_output_gain(*sound, ALL_OUTPUTS, volume / 100.0);
}


/*---------------------------------------------------------------
    atarigen_set_XXXXX_vol: Sets the volume for a given type
    of chip.
---------------------------------------------------------------*/

void atarigen_set_ym2151_vol(running_machine *machine, int volume)
{
	atarigen_set_vol(machine, volume, SOUND_YM2151);
}

void atarigen_set_ym2413_vol(running_machine *machine, int volume)
{
	atarigen_set_vol(machine, volume, SOUND_YM2413);
}

void atarigen_set_pokey_vol(running_machine *machine, int volume)
{
	atarigen_set_vol(machine, volume, SOUND_POKEY);
}

void atarigen_set_tms5220_vol(running_machine *machine, int volume)
{
	atarigen_set_vol(machine, volume, SOUND_TMS5220);
}

void atarigen_set_oki6295_vol(running_machine *machine, int volume)
{
	atarigen_set_vol(machine, volume, SOUND_OKIM6295);
}



/***************************************************************************
    SCANLINE TIMING
***************************************************************************/

/*---------------------------------------------------------------
    atarigen_scanline_timer_reset: Sets up the scanline timer.
---------------------------------------------------------------*/

void atarigen_scanline_timer_reset(screen_device &screen, atarigen_scanline_func update_graphics, int frequency)
{
	atarigen_state *state = (atarigen_state *)screen.machine->driver_data;

	/* set the scanline callback */
	state->scanline_callback = update_graphics;
	state->scanlines_per_callback = frequency;

	/* set a timer to go off at scanline 0 */
	if (state->scanline_callback != NULL)
	{
		emu_timer *timer = get_screen_timer(screen)->scanline_timer;
		timer_adjust_oneshot(timer, screen.time_until_pos(0), 0);
	}
}


/*---------------------------------------------------------------
    scanline_timer_callback: Called once every n scanlines
    to generate the periodic callback to the main system.
---------------------------------------------------------------*/

static TIMER_CALLBACK( scanline_timer_callback )
{
	atarigen_state *state = (atarigen_state *)machine->driver_data;
	screen_device &screen = *reinterpret_cast<screen_device *>(ptr);
	int scanline = param;

	/* callback */
	if (state->scanline_callback != NULL)
	{
		(*state->scanline_callback)(screen, scanline);

		/* generate another */
		scanline += state->scanlines_per_callback;
		if (scanline >= screen.height())
			scanline = 0;
		timer_adjust_oneshot(get_screen_timer(screen)->scanline_timer, screen.time_until_pos(scanline), scanline);
	}
}



/***************************************************************************
    VIDEO CONTROLLER
***************************************************************************/

/*---------------------------------------------------------------
    atarivc_eof_update: Callback that slurps up data and feeds
    it into the video controller registers every refresh.
---------------------------------------------------------------*/

static TIMER_CALLBACK( atarivc_eof_update )
{
	atarigen_state *state = (atarigen_state *)machine->driver_data;
	screen_device &screen = *reinterpret_cast<screen_device *>(ptr);
	emu_timer *timer = get_screen_timer(screen)->atarivc_eof_update_timer;
	int i;

	/* echo all the commands to the video controller */
	for (i = 0; i < 0x1c; i++)
		if (state->atarivc_eof_data[i])
			atarivc_common_w(screen, i, state->atarivc_eof_data[i]);

	/* update the scroll positions */
	atarimo_set_xscroll(0, state->atarivc_state.mo_xscroll);
	atarimo_set_yscroll(0, state->atarivc_state.mo_yscroll);

	tilemap_set_scrollx(state->playfield_tilemap, 0, state->atarivc_state.pf0_xscroll);
	tilemap_set_scrolly(state->playfield_tilemap, 0, state->atarivc_state.pf0_yscroll);

	if (state->atarivc_playfields > 1)
	{
		tilemap_set_scrollx(state->playfield2_tilemap, 0, state->atarivc_state.pf1_xscroll);
		tilemap_set_scrolly(state->playfield2_tilemap, 0, state->atarivc_state.pf1_yscroll);
	}
	timer_adjust_oneshot(timer, screen.time_until_pos(0), 0);

	/* use this for debugging the video controller values */
#if 0
	if (input_code_pressed(machine, KEYCODE_8))
	{
		static FILE *out;
		if (!out) out = fopen("scroll.log", "w");
		if (out)
		{
			for (i = 0; i < 64; i++)
				fprintf(out, "%04X ", data[i]);
			fprintf(out, "\n");
		}
	}
#endif
}


/*---------------------------------------------------------------
    atarivc_reset: Initializes the video controller.
---------------------------------------------------------------*/

void atarivc_reset(screen_device &screen, UINT16 *eof_data, int playfields)
{
	atarigen_state *state = (atarigen_state *)screen.machine->driver_data;

	/* this allows us to manually reset eof_data to NULL if it's not used */
	state->atarivc_eof_data = eof_data;
	state->atarivc_playfields = playfields;

	/* clear the RAM we use */
	memset(state->atarivc_data, 0, 0x40);
	memset(&state->atarivc_state, 0, sizeof(state->atarivc_state));

	/* reset the latches */
	state->atarivc_state.latch1 = state->atarivc_state.latch2 = -1;
	state->actual_vc_latch0 = state->actual_vc_latch1 = -1;

	/* start a timer to go off a little before scanline 0 */
	if (state->atarivc_eof_data)
	{
		emu_timer *timer = get_screen_timer(screen)->atarivc_eof_update_timer;
		timer_adjust_oneshot(timer, screen.time_until_pos(0), 0);
	}
}



/*---------------------------------------------------------------
    atarivc_w: Handles an I/O write to the video controller.
---------------------------------------------------------------*/

void atarivc_w(screen_device &screen, offs_t offset, UINT16 data, UINT16 mem_mask)
{
	atarigen_state *state = (atarigen_state *)screen.machine->driver_data;
	int oldword = state->atarivc_data[offset];
	int newword = oldword;

	COMBINE_DATA(&newword);
	atarivc_common_w(screen, offset, newword);
}



/*---------------------------------------------------------------
    atarivc_common_w: Does the bulk of the word for an I/O
    write.
---------------------------------------------------------------*/

static void atarivc_common_w(screen_device &screen, offs_t offset, UINT16 newword)
{
	atarigen_state *state = (atarigen_state *)screen.machine->driver_data;
	int oldword = state->atarivc_data[offset];
	state->atarivc_data[offset] = newword;

	/* switch off the offset */
	switch (offset)
	{
		/*
            additional registers:

                01 = vertical start (for centering)
                04 = horizontal start (for centering)
        */

		/* set the scanline interrupt here */
		case 0x03:
			if (oldword != newword)
				atarigen_scanline_int_set(screen, newword & 0x1ff);
			break;

		/* latch enable */
		case 0x0a:

			/* reset the latches when disabled */
			atarigen_set_playfield_latch(state, (newword & 0x0080) ? state->actual_vc_latch0 : -1);
			atarigen_set_playfield2_latch(state, (newword & 0x0080) ? state->actual_vc_latch1 : -1);

			/* check for rowscroll enable */
			state->atarivc_state.rowscroll_enable = (newword & 0x2000) >> 13;

			/* check for palette banking */
			if (state->atarivc_state.palette_bank != (((newword & 0x0400) >> 10) ^ 1))
			{
				screen.update_partial(screen.vpos());
				state->atarivc_state.palette_bank = ((newword & 0x0400) >> 10) ^ 1;
			}
			break;

		/* indexed parameters */
		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b:
			switch (newword & 15)
			{
				case 9:
					state->atarivc_state.mo_xscroll = (newword >> 7) & 0x1ff;
					break;

				case 10:
					state->atarivc_state.pf1_xscroll_raw = (newword >> 7) & 0x1ff;
					atarivc_update_pf_xscrolls(state);
					break;

				case 11:
					state->atarivc_state.pf0_xscroll_raw = (newword >> 7) & 0x1ff;
					atarivc_update_pf_xscrolls(state);
					break;

				case 13:
					state->atarivc_state.mo_yscroll = (newword >> 7) & 0x1ff;
					break;

				case 14:
					state->atarivc_state.pf1_yscroll = (newword >> 7) & 0x1ff;
					break;

				case 15:
					state->atarivc_state.pf0_yscroll = (newword >> 7) & 0x1ff;
					break;
			}
			break;

		/* latch 1 value */
		case 0x1c:
			state->actual_vc_latch0 = -1;
			state->actual_vc_latch1 = newword;
			atarigen_set_playfield_latch(state, (state->atarivc_data[0x0a] & 0x80) ? state->actual_vc_latch0 : -1);
			atarigen_set_playfield2_latch(state, (state->atarivc_data[0x0a] & 0x80) ? state->actual_vc_latch1 : -1);
			break;

		/* latch 2 value */
		case 0x1d:
			state->actual_vc_latch0 = newword;
			state->actual_vc_latch1 = -1;
			atarigen_set_playfield_latch(state, (state->atarivc_data[0x0a] & 0x80) ? state->actual_vc_latch0 : -1);
			atarigen_set_playfield2_latch(state, (state->atarivc_data[0x0a] & 0x80) ? state->actual_vc_latch1 : -1);
			break;

		/* scanline IRQ ack here */
		case 0x1e:
			/* hack: this should be a device */
			atarigen_scanline_int_ack_w(cputag_get_address_space(screen.machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0, 0, 0xffff);
			break;

		/* log anything else */
		case 0x00:
		default:
			if (oldword != newword)
				logerror("vc_w(%02X, %04X) ** [prev=%04X]\n", offset, newword, oldword);
			break;
	}
}


/*---------------------------------------------------------------
    atarivc_r: Handles an I/O read from the video controller.
---------------------------------------------------------------*/

UINT16 atarivc_r(screen_device &screen, offs_t offset)
{
	atarigen_state *state = (atarigen_state *)screen.machine->driver_data;

	logerror("vc_r(%02X)\n", offset);

	/* a read from offset 0 returns the current scanline */
	/* also sets bit 0x4000 if we're in VBLANK */
	if (offset == 0)
	{
		int result = screen.vpos();

		if (result > 255)
			result = 255;
		if (result > screen.visible_area().max_y)
			result |= 0x4000;

		return result;
	}
	else
		return state->atarivc_data[offset];
}



/***************************************************************************
    PLAYFIELD/ALPHA MAP HELPERS
***************************************************************************/

/*---------------------------------------------------------------
    atarigen_alpha_w: Generic write handler for alpha RAM.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_alpha_w )
{
	atarigen_state *state = (atarigen_state *)space->machine->driver_data;
	COMBINE_DATA(&state->alpha[offset]);
	tilemap_mark_tile_dirty(state->alpha_tilemap, offset);
}

WRITE32_HANDLER( atarigen_alpha32_w )
{
	atarigen_state *state = (atarigen_state *)space->machine->driver_data;
	COMBINE_DATA(&state->alpha32[offset]);
	if (ACCESSING_BITS_16_31)
		tilemap_mark_tile_dirty(state->alpha_tilemap, offset * 2);
	if (ACCESSING_BITS_0_15)
		tilemap_mark_tile_dirty(state->alpha_tilemap, offset * 2 + 1);
}

WRITE16_HANDLER( atarigen_alpha2_w )
{
	atarigen_state *state = (atarigen_state *)space->machine->driver_data;
	COMBINE_DATA(&state->alpha2[offset]);
	tilemap_mark_tile_dirty(state->alpha2_tilemap, offset);
}



/*---------------------------------------------------------------
    atarigen_set_playfield_latch: Sets the latch for the latched
    playfield handlers below.
---------------------------------------------------------------*/

void atarigen_set_playfield_latch(atarigen_state *state, int data)
{
	state->playfield_latch = data;
}

void atarigen_set_playfield2_latch(atarigen_state *state, int data)
{
	state->playfield2_latch = data;
}



/*---------------------------------------------------------------
    atarigen_playfield_w: Generic write handler for PF RAM.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_playfield_w )
{
	atarigen_state *state = (atarigen_state *)space->machine->driver_data;
	COMBINE_DATA(&state->playfield[offset]);
	tilemap_mark_tile_dirty(state->playfield_tilemap, offset);
}

WRITE32_HANDLER( atarigen_playfield32_w )
{
	atarigen_state *state = (atarigen_state *)space->machine->driver_data;
	COMBINE_DATA(&state->playfield32[offset]);
	if (ACCESSING_BITS_16_31)
		tilemap_mark_tile_dirty(state->playfield_tilemap, offset * 2);
	if (ACCESSING_BITS_0_15)
		tilemap_mark_tile_dirty(state->playfield_tilemap, offset * 2 + 1);
}

WRITE16_HANDLER( atarigen_playfield2_w )
{
	atarigen_state *state = (atarigen_state *)space->machine->driver_data;
	COMBINE_DATA(&state->playfield2[offset]);
	tilemap_mark_tile_dirty(state->playfield2_tilemap, offset);
}



/*---------------------------------------------------------------
    atarigen_playfield_large_w: Generic write handler for
    large (2-word) playfield RAM.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_playfield_large_w )
{
	atarigen_state *state = (atarigen_state *)space->machine->driver_data;
	COMBINE_DATA(&state->playfield[offset]);
	tilemap_mark_tile_dirty(state->playfield_tilemap, offset / 2);
}



/*---------------------------------------------------------------
    atarigen_playfield_upper_w: Generic write handler for
    upper word of split playfield RAM.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_playfield_upper_w )
{
	atarigen_state *state = (atarigen_state *)space->machine->driver_data;
	COMBINE_DATA(&state->playfield_upper[offset]);
	tilemap_mark_tile_dirty(state->playfield_tilemap, offset);
}



/*---------------------------------------------------------------
    atarigen_playfield_dual_upper_w: Generic write handler for
    upper word of split dual playfield RAM.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_playfield_dual_upper_w )
{
	atarigen_state *state = (atarigen_state *)space->machine->driver_data;
	COMBINE_DATA(&state->playfield_upper[offset]);
	tilemap_mark_tile_dirty(state->playfield_tilemap, offset);
	tilemap_mark_tile_dirty(state->playfield2_tilemap, offset);
}



/*---------------------------------------------------------------
    atarigen_playfield_latched_lsb_w: Generic write handler for
    lower word of playfield RAM with a latch in the LSB of the
    upper word.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_playfield_latched_lsb_w )
{
	atarigen_state *state = (atarigen_state *)space->machine->driver_data;

	COMBINE_DATA(&state->playfield[offset]);
	tilemap_mark_tile_dirty(state->playfield_tilemap, offset);

	if (state->playfield_latch != -1)
		state->playfield_upper[offset] = (state->playfield_upper[offset] & ~0x00ff) | (state->playfield_latch & 0x00ff);
}



/*---------------------------------------------------------------
    atarigen_playfield_latched_lsb_w: Generic write handler for
    lower word of playfield RAM with a latch in the MSB of the
    upper word.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_playfield_latched_msb_w )
{
	atarigen_state *state = (atarigen_state *)space->machine->driver_data;

	COMBINE_DATA(&state->playfield[offset]);
	tilemap_mark_tile_dirty(state->playfield_tilemap, offset);

	if (state->playfield_latch != -1)
		state->playfield_upper[offset] = (state->playfield_upper[offset] & ~0xff00) | (state->playfield_latch & 0xff00);
}



/*---------------------------------------------------------------
    atarigen_playfield_latched_lsb_w: Generic write handler for
    lower word of second playfield RAM with a latch in the MSB
    of the upper word.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_playfield2_latched_msb_w )
{
	atarigen_state *state = (atarigen_state *)space->machine->driver_data;

	COMBINE_DATA(&state->playfield2[offset]);
	tilemap_mark_tile_dirty(state->playfield2_tilemap, offset);

	if (state->playfield2_latch != -1)
		state->playfield_upper[offset] = (state->playfield_upper[offset] & ~0xff00) | (state->playfield2_latch & 0xff00);
}




/***************************************************************************
    VIDEO HELPERS
***************************************************************************/

/*---------------------------------------------------------------
    atarigen_get_hblank: Returns a guesstimate about the current
    HBLANK state, based on the assumption that HBLANK represents
    10% of the scanline period.
---------------------------------------------------------------*/

int atarigen_get_hblank(screen_device &screen)
{
	return (screen.hpos() > (screen.width() * 9 / 10));
}


/*---------------------------------------------------------------
    atarigen_halt_until_hblank_0: Halts CPU 0 until the
    next HBLANK.
---------------------------------------------------------------*/

void atarigen_halt_until_hblank_0(screen_device &screen)
{
	running_device *cpu = devtag_get_device(screen.machine, "maincpu");

	/* halt the CPU until the next HBLANK */
	int hpos = screen.hpos();
	int width = screen.width();
	int hblank = width * 9 / 10;
	double fraction;

	/* if we're in hblank, set up for the next one */
	if (hpos >= hblank)
		hblank += width;

	/* halt and set a timer to wake up */
	fraction = (double)(hblank - hpos) / (double)width;
	timer_set(screen.machine, double_to_attotime(attotime_to_double(screen.scan_period()) * fraction), (void *)cpu, 0, unhalt_cpu);
	cpu_set_input_line(cpu, INPUT_LINE_HALT, ASSERT_LINE);
}


/*---------------------------------------------------------------
    atarigen_666_paletteram_w: 6-6-6 RGB palette RAM handler.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_666_paletteram_w )
{
	int newword, r, g, b;

	COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);
	newword = space->machine->generic.paletteram.u16[offset];

	r = ((newword >> 9) & 0x3e) | ((newword >> 15) & 1);
	g = ((newword >> 4) & 0x3e) | ((newword >> 15) & 1);
	b = ((newword << 1) & 0x3e) | ((newword >> 15) & 1);

	palette_set_color_rgb(space->machine, offset, pal6bit(r), pal6bit(g), pal6bit(b));
}


/*---------------------------------------------------------------
    atarigen_expanded_666_paletteram_w: 6-6-6 RGB expanded
    palette RAM handler.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_expanded_666_paletteram_w )
{
	COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);

	if (ACCESSING_BITS_8_15)
	{
		int palentry = offset / 2;
		int newword = (space->machine->generic.paletteram.u16[palentry * 2] & 0xff00) | (space->machine->generic.paletteram.u16[palentry * 2 + 1] >> 8);

		int r, g, b;

		r = ((newword >> 9) & 0x3e) | ((newword >> 15) & 1);
		g = ((newword >> 4) & 0x3e) | ((newword >> 15) & 1);
		b = ((newword << 1) & 0x3e) | ((newword >> 15) & 1);

		palette_set_color_rgb(space->machine, palentry & 0x1ff, pal6bit(r), pal6bit(g), pal6bit(b));
	}
}


/*---------------------------------------------------------------
    atarigen_666_paletteram32_w: 6-6-6 RGB palette RAM handler.
---------------------------------------------------------------*/

WRITE32_HANDLER( atarigen_666_paletteram32_w )
{
	int newword, r, g, b;

	COMBINE_DATA(&space->machine->generic.paletteram.u32[offset]);

	if (ACCESSING_BITS_16_31)
	{
		newword = space->machine->generic.paletteram.u32[offset] >> 16;

		r = ((newword >> 9) & 0x3e) | ((newword >> 15) & 1);
		g = ((newword >> 4) & 0x3e) | ((newword >> 15) & 1);
		b = ((newword << 1) & 0x3e) | ((newword >> 15) & 1);

		palette_set_color_rgb(space->machine, offset * 2, pal6bit(r), pal6bit(g), pal6bit(b));
	}

	if (ACCESSING_BITS_0_15)
	{
		newword = space->machine->generic.paletteram.u32[offset] & 0xffff;

		r = ((newword >> 9) & 0x3e) | ((newword >> 15) & 1);
		g = ((newword >> 4) & 0x3e) | ((newword >> 15) & 1);
		b = ((newword << 1) & 0x3e) | ((newword >> 15) & 1);

		palette_set_color_rgb(space->machine, offset * 2 + 1, pal6bit(r), pal6bit(g), pal6bit(b));
	}
}


/*---------------------------------------------------------------
    unhalt_cpu: Timer callback to release the CPU from a halted state.
---------------------------------------------------------------*/

static TIMER_CALLBACK( unhalt_cpu )
{
	running_device *cpu = (running_device *)ptr;
	cpu_set_input_line(cpu, INPUT_LINE_HALT, CLEAR_LINE);
}



/***************************************************************************
    MISC HELPERS
***************************************************************************/

/*---------------------------------------------------------------
    atarigen_swap_mem: Inverts the bits in a region.
---------------------------------------------------------------*/

void atarigen_swap_mem(void *ptr1, void *ptr2, int bytes)
{
	UINT8 *p1 = (UINT8 *)ptr1;
	UINT8 *p2 = (UINT8 *)ptr2;
	while (bytes--)
	{
		int temp = *p1;
		*p1++ = *p2;
		*p2++ = temp;
	}
}


/*---------------------------------------------------------------
    atarigen_blend_gfx: Takes two GFXElements and blends their
    data together to form one. Then frees the second.
---------------------------------------------------------------*/

void atarigen_blend_gfx(running_machine *machine, int gfx0, int gfx1, int mask0, int mask1)
{
	gfx_element *gx0 = machine->gfx[gfx0];
	gfx_element *gx1 = machine->gfx[gfx1];
	UINT8 *srcdata, *dest;
	int c, x, y;

	/* allocate memory for the assembled data */
	srcdata = auto_alloc_array(machine, UINT8, gx0->total_elements * gx0->width * gx0->height);

	/* loop over elements */
	dest = srcdata;
	for (c = 0; c < gx0->total_elements; c++)
	{
		const UINT8 *c0base = gfx_element_get_data(gx0, c);
		const UINT8 *c1base = gfx_element_get_data(gx1, c);

		/* loop over height */
		for (y = 0; y < gx0->height; y++)
		{
			const UINT8 *c0 = c0base;
			const UINT8 *c1 = c1base;

			for (x = 0; x < gx0->width; x++)
				*dest++ = (*c0++ & mask0) | (*c1++ & mask1);
			c0base += gx0->line_modulo;
			c1base += gx1->line_modulo;
		}
	}

	/* free the second graphics element */
	gfx_element_free(gx1);
	machine->gfx[gfx1] = NULL;

	/* create a simple target layout */
	gx0->layout.planes = 8;
	for (x = 0; x < 8; x++)
		gx0->layout.planeoffset[x] = x;
	for (x = 0; x < gx0->width; x++)
		gx0->layout.xoffset[x] = 8 * x;
	for (y = 0; y < gx0->height; y++)
		gx0->layout.yoffset[y] = 8 * y * gx0->width;
	gx0->layout.charincrement = 8 * gx0->width * gx0->height;

	/* make the assembled data our new source data */
	gfx_element_set_source(gx0, srcdata);
}
