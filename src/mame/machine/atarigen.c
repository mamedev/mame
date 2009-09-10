/***************************************************************************

    atarigen.c

    General functions for Atari raster games.

***************************************************************************/


#include "driver.h"
#include "cpu/m6502/m6502.h"
#include "sound/2151intf.h"
#include "sound/2413intf.h"
#include "sound/tms5220.h"
#include "sound/okim6295.h"
#include "sound/pokey.h"
#include "slapstic.h"
#include "atarigen.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define SOUND_TIMER_RATE			ATTOTIME_IN_USEC(5)
#define SOUND_TIMER_BOOST			ATTOTIME_IN_USEC(100)



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

UINT8 				atarigen_scanline_int_state;
UINT8 				atarigen_sound_int_state;
UINT8 				atarigen_video_int_state;

const UINT16 *		atarigen_eeprom_default;
UINT16 *			atarigen_eeprom;
size_t 				atarigen_eeprom_size;

UINT8 				atarigen_cpu_to_sound_ready;
UINT8 				atarigen_sound_to_cpu_ready;

UINT16 *			atarigen_playfield;
UINT16 *			atarigen_playfield2;
UINT16 *			atarigen_playfield_upper;
UINT16 *			atarigen_alpha;
UINT16 *			atarigen_alpha2;
UINT16 *			atarigen_xscroll;
UINT16 *			atarigen_yscroll;

UINT32 *			atarigen_playfield32;
UINT32 *			atarigen_alpha32;

tilemap *			atarigen_playfield_tilemap;
tilemap *			atarigen_playfield2_tilemap;
tilemap *			atarigen_alpha_tilemap;
tilemap *			atarigen_alpha2_tilemap;

UINT16 *			atarivc_data;
UINT16 *			atarivc_eof_data;
struct atarivc_state_desc atarivc_state;



/***************************************************************************
    STATIC VARIABLES
***************************************************************************/

static atarigen_int_func update_int_callback;

static UINT8 					eeprom_unlocked;

static UINT8					atarigen_slapstic_num;
static UINT16 *					atarigen_slapstic;
static UINT8					atarigen_slapstic_bank;
static void *					atarigen_slapstic_bank0;
static offs_t					atarigen_slapstic_last_pc;
static offs_t					atarigen_slapstic_last_address;
static offs_t					atarigen_slapstic_base;
static offs_t					atarigen_slapstic_mirror;

static const device_config *	sound_cpu;
static UINT8 					atarigen_cpu_to_sound;
static UINT8 					atarigen_sound_to_cpu;
static UINT8 					timed_int;
static UINT8 					ym2151_int;

static atarigen_scanline_func 	scanline_callback;
static UINT32 					scanlines_per_callback;

static UINT32 					actual_vc_latch0;
static UINT32 					actual_vc_latch1;
static UINT8					atarivc_playfields;

static UINT32					playfield_latch;
static UINT32					playfield2_latch;

static const device_config *	scanline_interrupt_timer_screens[ATARIMO_MAX];
static emu_timer *				scanline_interrupt_timers[ATARIMO_MAX];

static const device_config *	scanline_timer_screens[ATARIMO_MAX];
static emu_timer *				scanline_timers[ATARIMO_MAX];

static const device_config *	atarivc_eof_update_timer_screens[ATARIMO_MAX];
static emu_timer *				atarivc_eof_update_timers[ATARIMO_MAX];



/***************************************************************************
    STATIC FUNCTION DECLARATIONS
***************************************************************************/

static TIMER_CALLBACK( scanline_interrupt_callback );

static void decompress_eeprom_word(const UINT16 *data);
static void decompress_eeprom_byte(const UINT16 *data);

static void update_6502_irq(running_machine *machine);
static TIMER_CALLBACK( delayed_sound_reset );
static TIMER_CALLBACK( delayed_sound_w );
static TIMER_CALLBACK( delayed_6502_sound_w );

static void atarigen_set_vol(running_machine *machine, int volume, sound_type type);

static TIMER_CALLBACK( scanline_timer_callback );

static void atarivc_common_w(const device_config *screen, offs_t offset, UINT16 newword);

static TIMER_CALLBACK( unhalt_cpu );

static TIMER_CALLBACK( atarivc_eof_update );



/***************************************************************************
    INTERRUPT HANDLING
***************************************************************************/

/*---------------------------------------------------------------
    atarigen_interrupt_reset: Initializes the state of all
    the interrupt sources.
---------------------------------------------------------------*/

void atarigen_interrupt_reset(atarigen_int_func update_int)
{
	int i;

	/* set the callback */
	update_int_callback = update_int;

	/* reset the interrupt states */
	atarigen_video_int_state = atarigen_sound_int_state = atarigen_scanline_int_state = 0;

	/* clear the timers */
	for (i = 0; i < ATARIMO_MAX; i++)
	{
		scanline_interrupt_timer_screens[i] = NULL;
		scanline_interrupt_timers[i] = NULL;
		scanline_timer_screens[i] = NULL;
		scanline_timers[i] = NULL;
	}
}


/*---------------------------------------------------------------
    atarigen_update_interrupts: Forces the interrupt callback
    to be called with the current VBLANK and sound interrupt
    states.
---------------------------------------------------------------*/

void atarigen_update_interrupts(running_machine *machine)
{
	(*update_int_callback)(machine);
}


/*---------------------------------------------------------------
    get_scanline_interrupt_timer_for_screen: Retrieves or
    creates a scanline interrupt timer.
---------------------------------------------------------------*/

static emu_timer *get_scanline_interrupt_timer_for_screen(const device_config *screen)
{
	int i;

	/* find the index of the timer that matches the screen */
	for (i = 0; i < ATARIMO_MAX; i++)
	{
		/* matching */
		if (scanline_interrupt_timer_screens[i] == screen)
			break;

		/* no more */
		if (scanline_interrupt_timer_screens[i] == NULL)
			break;
	}

	/* check that we still have room */
	assert(i != ATARIMO_MAX);

	/* need to create? */
	if (scanline_interrupt_timer_screens[i] == NULL)
	{
		scanline_interrupt_timer_screens[i] = screen;
		scanline_interrupt_timers[i] = timer_alloc(screen->machine, scanline_interrupt_callback, (void *)screen);
	}

	/* found it */
	return scanline_interrupt_timers[i];
}


/*---------------------------------------------------------------
    atarigen_scanline_int_set: Sets the scanline when the next
    scanline interrupt should be generated.
---------------------------------------------------------------*/

void atarigen_scanline_int_set(const device_config *screen, int scanline)
{
	emu_timer *timer = get_scanline_interrupt_timer_for_screen(screen);
	timer_adjust_oneshot(timer, video_screen_get_time_until_pos(screen, scanline, 0), 0);
}


/*---------------------------------------------------------------
    atarigen_scanline_int_gen: Standard interrupt routine
    which sets the scanline interrupt state.
---------------------------------------------------------------*/

INTERRUPT_GEN( atarigen_scanline_int_gen )
{
	atarigen_scanline_int_state = 1;
	(*update_int_callback)(device->machine);
}


/*---------------------------------------------------------------
    atarigen_scanline_int_ack_w: Resets the state of the
    scanline interrupt.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_scanline_int_ack_w )
{
	atarigen_scanline_int_state = 0;
	(*update_int_callback)(space->machine);
}

WRITE32_HANDLER( atarigen_scanline_int_ack32_w )
{
	atarigen_scanline_int_state = 0;
	(*update_int_callback)(space->machine);
}


/*---------------------------------------------------------------
    atarigen_sound_int_gen: Standard interrupt routine which
    sets the sound interrupt state.
---------------------------------------------------------------*/

INTERRUPT_GEN( atarigen_sound_int_gen )
{
	atarigen_sound_int_state = 1;
	(*update_int_callback)(device->machine);
}


/*---------------------------------------------------------------
    atarigen_sound_int_ack_w: Resets the state of the sound
    interrupt.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_sound_int_ack_w )
{
	atarigen_sound_int_state = 0;
	(*update_int_callback)(space->machine);
}

WRITE32_HANDLER( atarigen_sound_int_ack32_w )
{
	atarigen_sound_int_state = 0;
	(*update_int_callback)(space->machine);
}


/*---------------------------------------------------------------
    atarigen_video_int_gen: Standard interrupt routine which
    sets the video interrupt state.
---------------------------------------------------------------*/

INTERRUPT_GEN( atarigen_video_int_gen )
{
	atarigen_video_int_state = 1;
	(*update_int_callback)(device->machine);
}


/*---------------------------------------------------------------
    atarigen_video_int_ack_w: Resets the state of the video
    interrupt.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_video_int_ack_w )
{
	atarigen_video_int_state = 0;
	(*update_int_callback)(space->machine);
}

WRITE32_HANDLER( atarigen_video_int_ack32_w )
{
	atarigen_video_int_state = 0;
	(*update_int_callback)(space->machine);
}


/*---------------------------------------------------------------
    scanline_interrupt_callback: Signals an interrupt.
---------------------------------------------------------------*/

static TIMER_CALLBACK( scanline_interrupt_callback )
{
	const device_config *screen = (const device_config *)ptr;
	emu_timer *timer = get_scanline_interrupt_timer_for_screen(screen);

	/* generate the interrupt */
	atarigen_scanline_int_gen(cputag_get_cpu(machine, "maincpu"));

	/* set a new timer to go off at the same scan line next frame */
	timer_adjust_oneshot(timer, video_screen_get_frame_period(screen), 0);
}



/***************************************************************************
    EEPROM HANDLING
***************************************************************************/

/*---------------------------------------------------------------
    atarigen_eeprom_reset: Makes sure that the unlocked state
    is cleared when we reset.
---------------------------------------------------------------*/

void atarigen_eeprom_reset(void)
{
	eeprom_unlocked = 0;
}


/*---------------------------------------------------------------
    atarigen_eeprom_enable_w: Any write to this handler will
    allow one byte to be written to the EEPROM data area the
    next time.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_eeprom_enable_w )
{
	eeprom_unlocked = 1;
}

WRITE32_HANDLER( atarigen_eeprom_enable32_w )
{
	eeprom_unlocked = 1;
}


/*---------------------------------------------------------------
    atarigen_eeprom_w: Writes a "word" to the EEPROM, which is
    almost always accessed via the low byte of the word only.
    If the EEPROM hasn't been unlocked, the write attempt is
    ignored.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_eeprom_w )
{
	if (!eeprom_unlocked)
		return;

	COMBINE_DATA(&atarigen_eeprom[offset]);
	eeprom_unlocked = 0;
}

WRITE32_HANDLER( atarigen_eeprom32_w )
{
	if (!eeprom_unlocked)
		return;

	COMBINE_DATA(&atarigen_eeprom[offset * 2 + 1]);
	data >>= 16;
	mem_mask >>= 16;
	COMBINE_DATA(&atarigen_eeprom[offset * 2]);
	eeprom_unlocked = 0;
}


/*---------------------------------------------------------------
    atarigen_eeprom_r: Reads a "word" from the EEPROM, which is
    almost always accessed via the low byte of the word only.
---------------------------------------------------------------*/

READ16_HANDLER( atarigen_eeprom_r )
{
	return atarigen_eeprom[offset] | 0xff00;
}

READ16_HANDLER( atarigen_eeprom_upper_r )
{
	return atarigen_eeprom[offset] | 0x00ff;
}

READ32_HANDLER( atarigen_eeprom_upper32_r )
{
	return (atarigen_eeprom[offset * 2] << 16) | atarigen_eeprom[offset * 2 + 1] | 0x00ff00ff;
}


/*---------------------------------------------------------------
    NVRAM_HANDLER( atarigen ): Loads the EEPROM data.
---------------------------------------------------------------*/

NVRAM_HANDLER( atarigen )
{
	if (read_or_write)
		mame_fwrite(file, atarigen_eeprom, atarigen_eeprom_size);
	else if (file)
		mame_fread(file, atarigen_eeprom, atarigen_eeprom_size);
	else
	{
		/* all 0xff's work for most games */
		memset(atarigen_eeprom, 0xff, atarigen_eeprom_size);

		/* anything else must be decompressed */
		if (atarigen_eeprom_default)
		{
			if (atarigen_eeprom_default[0] == 0)
				decompress_eeprom_byte(atarigen_eeprom_default + 1);
			else
				decompress_eeprom_word(atarigen_eeprom_default + 1);
		}
	}
}


/*---------------------------------------------------------------
    decompress_eeprom_word: Used for decompressing EEPROM data
    that has every other byte invalid.
---------------------------------------------------------------*/

void decompress_eeprom_word(const UINT16 *data)
{
	UINT16 *dest = (UINT16 *)atarigen_eeprom;
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

void decompress_eeprom_byte(const UINT16 *data)
{
	UINT8 *dest = (UINT8 *)atarigen_eeprom;
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

INLINE void update_bank(int bank)
{
	/* if the bank has changed, copy the memory; Pit Fighter needs this */
	if (bank != atarigen_slapstic_bank)
	{
		/* bank 0 comes from the copy we made earlier */
		if (bank == 0)
			memcpy(atarigen_slapstic, atarigen_slapstic_bank0, 0x2000);
		else
			memcpy(atarigen_slapstic, &atarigen_slapstic[bank * 0x1000], 0x2000);

		/* remember the current bank */
		atarigen_slapstic_bank = bank;
	}
}


static STATE_POSTLOAD( slapstic_postload )
{
	update_bank(slapstic_bank());
}


static DIRECT_UPDATE_HANDLER( atarigen_slapstic_setdirect )
{
	/* if we jump to an address in the slapstic region, tweak the slapstic
       at that address and return ~0; this will cause us to be called on
       subsequent fetches as well */
	address &= ~atarigen_slapstic_mirror;
	if (address >= atarigen_slapstic_base && address < atarigen_slapstic_base + 0x8000)
	{
		offs_t pc = cpu_get_previouspc(space->cpu);
		if (pc != atarigen_slapstic_last_pc || address != atarigen_slapstic_last_address)
		{
			atarigen_slapstic_last_pc = pc;
			atarigen_slapstic_last_address = address;
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

void atarigen_slapstic_init(const device_config *device, offs_t base, offs_t mirror, int chipnum)
{
	/* reset in case we have no state */
	atarigen_slapstic_num = chipnum;
	atarigen_slapstic = NULL;

	/* if we have a chip, install it */
	if (chipnum != 0)
	{
		/* initialize the slapstic */
		slapstic_init(device->machine, chipnum);

		/* install the memory handlers */
		atarigen_slapstic = memory_install_readwrite16_handler(cpu_get_address_space(device, ADDRESS_SPACE_PROGRAM), base, base + 0x7fff, 0, mirror, atarigen_slapstic_r, atarigen_slapstic_w);

		/* allocate memory for a copy of bank 0 */
		atarigen_slapstic_bank0 = auto_alloc_array(device->machine, UINT8, 0x2000);
		memcpy(atarigen_slapstic_bank0, atarigen_slapstic, 0x2000);

		/* ensure we recopy memory for the bank */
		atarigen_slapstic_bank = 0xff;

		/* install an opcode base handler if we are a 68000 or variant */
		atarigen_slapstic_base = base;
		atarigen_slapstic_mirror = mirror;
		memory_set_direct_update_handler(cpu_get_address_space(device, ADDRESS_SPACE_PROGRAM), atarigen_slapstic_setdirect);
	}
}


/*---------------------------------------------------------------
    atarigen_slapstic_reset: Makes the selected slapstic number
    active and resets its state.
---------------------------------------------------------------*/

void atarigen_slapstic_reset(void)
{
	if (atarigen_slapstic_num != 0)
	{
		slapstic_reset();
		update_bank(slapstic_bank());
	}
}


/*---------------------------------------------------------------
    atarigen_slapstic_w: Assuming that the slapstic sits in
    ROM memory space, we just simply tweak the slapstic at this
    address and do nothing more.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_slapstic_w )
{
	update_bank(slapstic_tweak(space, offset));
}


/*---------------------------------------------------------------
    atarigen_slapstic_r: Tweaks the slapstic at the appropriate
    address and then reads a word from the underlying memory.
---------------------------------------------------------------*/

READ16_HANDLER( atarigen_slapstic_r )
{
	/* fetch the result from the current bank first */
	int result = atarigen_slapstic[offset & 0xfff];

	/* then determine the new one */
	update_bank(slapstic_tweak(space, offset));
	return result;
}



/***************************************************************************
    SOUND I/O
***************************************************************************/

/*---------------------------------------------------------------
    atarigen_sound_io_reset: Resets the state of the sound I/O.
---------------------------------------------------------------*/

void atarigen_sound_io_reset(const device_config *device)
{
	/* remember which CPU is the sound CPU */
	sound_cpu = device;

	/* reset the internal interrupts states */
	timed_int = ym2151_int = 0;

	/* reset the sound I/O states */
	atarigen_cpu_to_sound = atarigen_sound_to_cpu = 0;
	atarigen_cpu_to_sound_ready = atarigen_sound_to_cpu_ready = 0;
}


/*---------------------------------------------------------------
    atarigen_6502_irq_gen: Generates an IRQ signal to the 6502
    sound processor.
---------------------------------------------------------------*/

INTERRUPT_GEN( atarigen_6502_irq_gen )
{
	timed_int = 1;
	update_6502_irq(device->machine);
}


/*---------------------------------------------------------------
    atarigen_6502_irq_ack_r: Resets the IRQ signal to the 6502
    sound processor. Both reads and writes can be used.
---------------------------------------------------------------*/

READ8_HANDLER( atarigen_6502_irq_ack_r )
{
	timed_int = 0;
	update_6502_irq(space->machine);
	return 0;
}

WRITE8_HANDLER( atarigen_6502_irq_ack_w )
{
	timed_int = 0;
	update_6502_irq(space->machine);
}


/*---------------------------------------------------------------
    atarigen_ym2151_irq_gen: Sets the state of the YM2151's
    IRQ line.
---------------------------------------------------------------*/

void atarigen_ym2151_irq_gen(const device_config *device, int irq)
{
	ym2151_int = irq;
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
	atarigen_sound_to_cpu_ready = 0;
	atarigen_sound_int_ack_w(space, 0, 0, 0xffff);
	return atarigen_sound_to_cpu | 0xff00;
}

READ16_HANDLER( atarigen_sound_upper_r )
{
	atarigen_sound_to_cpu_ready = 0;
	atarigen_sound_int_ack_w(space, 0, 0, 0xffff);
	return (atarigen_sound_to_cpu << 8) | 0x00ff;
}

READ32_HANDLER( atarigen_sound_upper32_r )
{
	atarigen_sound_to_cpu_ready = 0;
	atarigen_sound_int_ack32_w(space, 0, 0, 0xffff);
	return (atarigen_sound_to_cpu << 24) | 0x00ffffff;
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
	atarigen_cpu_to_sound_ready = 0;
	cpu_set_input_line(sound_cpu, INPUT_LINE_NMI, CLEAR_LINE);
	return atarigen_cpu_to_sound;
}


/*---------------------------------------------------------------
    update_6502_irq: Called whenever the IRQ state changes. An
    interrupt is generated if either atarigen_6502_irq_gen()
    was called, or if the YM2151 generated an interrupt via
    the atarigen_ym2151_irq_gen() callback.
---------------------------------------------------------------*/

static void update_6502_irq(running_machine *machine)
{
	if (timed_int || ym2151_int)
		cpu_set_input_line(sound_cpu, M6502_IRQ_LINE, ASSERT_LINE);
	else
		cpu_set_input_line(sound_cpu, M6502_IRQ_LINE, CLEAR_LINE);
}


/*---------------------------------------------------------------
    delayed_sound_reset: Synchronizes the sound reset command
    between the two CPUs.
---------------------------------------------------------------*/

static TIMER_CALLBACK( delayed_sound_reset )
{
	const address_space *space = cpu_get_address_space(sound_cpu, ADDRESS_SPACE_PROGRAM);

	/* unhalt and reset the sound CPU */
	if (param == 0)
	{
		cpu_set_input_line(sound_cpu, INPUT_LINE_HALT, CLEAR_LINE);
		cpu_set_input_line(sound_cpu, INPUT_LINE_RESET, PULSE_LINE);
	}

	/* reset the sound write state */
	atarigen_sound_to_cpu_ready = 0;
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
	/* warn if we missed something */
	if (atarigen_cpu_to_sound_ready)
		logerror("Missed command from 68010\n");

	/* set up the states and signal an NMI to the sound CPU */
	atarigen_cpu_to_sound = param;
	atarigen_cpu_to_sound_ready = 1;
	cpu_set_input_line(sound_cpu, INPUT_LINE_NMI, ASSERT_LINE);

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
	/* warn if we missed something */
	if (atarigen_sound_to_cpu_ready)
		logerror("Missed result from 6502\n");

	/* set up the states and signal the sound interrupt to the main CPU */
	atarigen_sound_to_cpu = param;
	atarigen_sound_to_cpu_ready = 1;
	atarigen_sound_int_gen(cputag_get_cpu(machine, "maincpu"));
}



/***************************************************************************
    SOUND HELPERS
***************************************************************************/

/*---------------------------------------------------------------
    atarigen_set_vol: Scans for a particular sound chip and
    changes the volume on all channels associated with it.
---------------------------------------------------------------*/

void atarigen_set_vol(running_machine *machine, int volume, sound_type type)
{
	const device_config *device;

	for (device = sound_first(machine->config); device != NULL; device = sound_next(device))
		if (sound_get_type(device) == type)
			sound_set_output_gain(device, ALL_OUTPUTS, volume / 100.0);
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
    get_scanline_timer_for_screen: Retrieves or
    creates the the scanline timer.
---------------------------------------------------------------*/

static emu_timer *get_scanline_timer_for_screen(const device_config *screen)
{
	int i;

	/* find the index of the timer that matches the screen */
	for (i = 0; i < ATARIMO_MAX; i++)
	{
		/* matching */
		if (scanline_timer_screens[i] == screen)
			break;

		/* no more */
		if (scanline_timer_screens[i] == NULL)
			break;
	}

	/* check that we still have room */
	assert(i != ATARIMO_MAX);

	/* need to create? */
	if (scanline_timer_screens[i] == NULL)
	{
		scanline_timer_screens[i] = screen;
		scanline_timers[i] = timer_alloc(screen->machine, scanline_timer_callback, (void *)screen);
	}

	/* found it */
	return scanline_timers[i];
}


/*---------------------------------------------------------------
    atarigen_scanline_timer_reset: Sets up the scanline timer.
---------------------------------------------------------------*/

void atarigen_scanline_timer_reset(const device_config *screen, atarigen_scanline_func update_graphics, int frequency)
{
	/* set the scanline callback */
	scanline_callback = update_graphics;
	scanlines_per_callback = frequency;

	/* set a timer to go off at scanline 0 */
	if (scanline_callback != NULL)
	{
		emu_timer *timer = get_scanline_timer_for_screen(screen);
		timer_adjust_oneshot(timer, video_screen_get_time_until_pos(screen, 0, 0), 0);
	}

	/* implicitly allocate a scanline interrupt timer */
	get_scanline_interrupt_timer_for_screen(screen);

}


/*---------------------------------------------------------------
    scanline_timer_callback: Called once every n scanlines
    to generate the periodic callback to the main system.
---------------------------------------------------------------*/

static TIMER_CALLBACK( scanline_timer_callback )
{
	const device_config *screen = (const device_config *)ptr;
	int scanline = param;

	/* callback */
	if (scanline_callback != NULL)
	{
		(*scanline_callback)(screen, scanline);

		/* generate another */
		scanline += scanlines_per_callback;
		if (scanline >= video_screen_get_height(screen))
			scanline = 0;
		timer_adjust_oneshot(get_scanline_timer_for_screen(screen), video_screen_get_time_until_pos(screen, scanline, 0), scanline);
	}
}



/***************************************************************************
    VIDEO CONTROLLER
***************************************************************************/

/*---------------------------------------------------------------
    get_scanline_timer_for_screen: Retrieves or
    creates the the scanline timer.
---------------------------------------------------------------*/

static emu_timer *get_atarivc_eof_update_timer_for_screen(const device_config *screen)
{
	int i;

	/* find the index of the timer that matches the screen */
	for (i = 0; i < ATARIMO_MAX; i++)
	{
		/* matching */
		if (atarivc_eof_update_timer_screens[i] == screen)
			break;

		/* no more */
		if (atarivc_eof_update_timer_screens[i] == NULL)
			break;
	}

	/* check that we still have room */
	assert(i != ATARIMO_MAX);

	/* need to create? */
	if (atarivc_eof_update_timer_screens[i] == NULL)
	{
		atarivc_eof_update_timer_screens[i] = screen;
		atarivc_eof_update_timers[i] = timer_alloc(screen->machine, atarivc_eof_update, (void *)screen);
	}

	/* found it */
	return atarivc_eof_update_timers[i];
}


/*---------------------------------------------------------------
    atarivc_eof_update: Callback that slurps up data and feeds
    it into the video controller registers every refresh.
---------------------------------------------------------------*/

static TIMER_CALLBACK( atarivc_eof_update )
{
	const device_config *screen = (const device_config *)ptr;
	emu_timer *timer = get_atarivc_eof_update_timer_for_screen(screen);
	int i;

	/* echo all the commands to the video controller */
	for (i = 0; i < 0x1c; i++)
		if (atarivc_eof_data[i])
			atarivc_common_w(screen, i, atarivc_eof_data[i]);

	/* update the scroll positions */
	atarimo_set_xscroll(0, atarivc_state.mo_xscroll);
	atarimo_set_yscroll(0, atarivc_state.mo_yscroll);

	tilemap_set_scrollx(atarigen_playfield_tilemap, 0, atarivc_state.pf0_xscroll);
	tilemap_set_scrolly(atarigen_playfield_tilemap, 0, atarivc_state.pf0_yscroll);

	if (atarivc_playfields > 1)
	{
		tilemap_set_scrollx(atarigen_playfield2_tilemap, 0, atarivc_state.pf1_xscroll);
		tilemap_set_scrolly(atarigen_playfield2_tilemap, 0, atarivc_state.pf1_yscroll);
	}
	timer_adjust_oneshot(timer, video_screen_get_time_until_pos(screen, 0, 0), 0);

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

void atarivc_reset(const device_config *screen, UINT16 *eof_data, int playfields)
{
	int i;

	/* this allows us to manually reset eof_data to NULL if it's not used */
	atarivc_eof_data = eof_data;
	atarivc_playfields = playfields;

	/* clear the RAM we use */
	memset(atarivc_data, 0, 0x40);
	memset(&atarivc_state, 0, sizeof(atarivc_state));

	/* reset the latches */
	atarivc_state.latch1 = atarivc_state.latch2 = -1;
	actual_vc_latch0 = actual_vc_latch1 = -1;

	/* clear the timers */
	for (i = 0; i < ATARIMO_MAX; i++)
	{
		atarivc_eof_update_timer_screens[i] = NULL;
		atarivc_eof_update_timers[i] = NULL;
	}

	/* start a timer to go off a little before scanline 0 */
	if (atarivc_eof_data)
	{
		emu_timer *timer = get_atarivc_eof_update_timer_for_screen(screen);
		timer_adjust_oneshot(timer, video_screen_get_time_until_pos(screen, 0, 0), 0);
	}
}



/*---------------------------------------------------------------
    atarivc_w: Handles an I/O write to the video controller.
---------------------------------------------------------------*/

void atarivc_w(const device_config *screen, offs_t offset, UINT16 data, UINT16 mem_mask)
{
	int oldword = atarivc_data[offset];
	int newword = oldword;

	COMBINE_DATA(&newword);
	atarivc_common_w(screen, offset, newword);
}



/*---------------------------------------------------------------
    atarivc_common_w: Does the bulk of the word for an I/O
    write.
---------------------------------------------------------------*/

static void atarivc_common_w(const device_config *screen, offs_t offset, UINT16 newword)
{
	int oldword = atarivc_data[offset];
	atarivc_data[offset] = newword;

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
			atarigen_set_playfield_latch((newword & 0x0080) ? actual_vc_latch0 : -1);
			atarigen_set_playfield2_latch((newword & 0x0080) ? actual_vc_latch1 : -1);

			/* check for rowscroll enable */
			atarivc_state.rowscroll_enable = (newword & 0x2000) >> 13;

			/* check for palette banking */
			if (atarivc_state.palette_bank != (((newword & 0x0400) >> 10) ^ 1))
			{
				video_screen_update_partial(screen, video_screen_get_vpos(screen));
				atarivc_state.palette_bank = ((newword & 0x0400) >> 10) ^ 1;
			}
			break;

		/* indexed parameters */
		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b:
			switch (newword & 15)
			{
				case 9:
					atarivc_state.mo_xscroll = (newword >> 7) & 0x1ff;
					break;

				case 10:
					atarivc_state.pf1_xscroll_raw = (newword >> 7) & 0x1ff;
					atarivc_update_pf_xscrolls();
					break;

				case 11:
					atarivc_state.pf0_xscroll_raw = (newword >> 7) & 0x1ff;
					atarivc_update_pf_xscrolls();
					break;

				case 13:
					atarivc_state.mo_yscroll = (newword >> 7) & 0x1ff;
					break;

				case 14:
					atarivc_state.pf1_yscroll = (newword >> 7) & 0x1ff;
					break;

				case 15:
					atarivc_state.pf0_yscroll = (newword >> 7) & 0x1ff;
					break;
			}
			break;

		/* latch 1 value */
		case 0x1c:
			actual_vc_latch0 = -1;
			actual_vc_latch1 = newword;
			atarigen_set_playfield_latch((atarivc_data[0x0a] & 0x80) ? actual_vc_latch0 : -1);
			atarigen_set_playfield2_latch((atarivc_data[0x0a] & 0x80) ? actual_vc_latch1 : -1);
			break;

		/* latch 2 value */
		case 0x1d:
			actual_vc_latch0 = newword;
			actual_vc_latch1 = -1;
			atarigen_set_playfield_latch((atarivc_data[0x0a] & 0x80) ? actual_vc_latch0 : -1);
			atarigen_set_playfield2_latch((atarivc_data[0x0a] & 0x80) ? actual_vc_latch1 : -1);
			break;

		/* scanline IRQ ack here */
		case 0x1e:
			/* hack: this should be a device */
			atarigen_scanline_int_ack_w(cputag_get_address_space(screen->machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0, 0, 0xffff);
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

UINT16 atarivc_r(const device_config *screen, offs_t offset)
{
	logerror("vc_r(%02X)\n", offset);

	/* a read from offset 0 returns the current scanline */
	/* also sets bit 0x4000 if we're in VBLANK */
	if (offset == 0)
	{
		int result = video_screen_get_vpos(screen);

		if (result > 255)
			result = 255;
		if (result > video_screen_get_visible_area(screen)->max_y)
			result |= 0x4000;

		return result;
	}
	else
		return atarivc_data[offset];
}



/***************************************************************************
    PLAYFIELD/ALPHA MAP HELPERS
***************************************************************************/

/*---------------------------------------------------------------
    atarigen_alpha_w: Generic write handler for alpha RAM.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_alpha_w )
{
	COMBINE_DATA(&atarigen_alpha[offset]);
	tilemap_mark_tile_dirty(atarigen_alpha_tilemap, offset);
}

WRITE32_HANDLER( atarigen_alpha32_w )
{
	COMBINE_DATA(&atarigen_alpha32[offset]);
	if (ACCESSING_BITS_16_31)
		tilemap_mark_tile_dirty(atarigen_alpha_tilemap, offset * 2);
	if (ACCESSING_BITS_0_15)
		tilemap_mark_tile_dirty(atarigen_alpha_tilemap, offset * 2 + 1);
}

WRITE16_HANDLER( atarigen_alpha2_w )
{
	COMBINE_DATA(&atarigen_alpha2[offset]);
	tilemap_mark_tile_dirty(atarigen_alpha2_tilemap, offset);
}



/*---------------------------------------------------------------
    atarigen_set_playfield_latch: Sets the latch for the latched
    playfield handlers below.
---------------------------------------------------------------*/

void atarigen_set_playfield_latch(int data)
{
	playfield_latch = data;
}

void atarigen_set_playfield2_latch(int data)
{
	playfield2_latch = data;
}



/*---------------------------------------------------------------
    atarigen_playfield_w: Generic write handler for PF RAM.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_playfield_w )
{
	COMBINE_DATA(&atarigen_playfield[offset]);
	tilemap_mark_tile_dirty(atarigen_playfield_tilemap, offset);
}

WRITE32_HANDLER( atarigen_playfield32_w )
{
	COMBINE_DATA(&atarigen_playfield32[offset]);
	if (ACCESSING_BITS_16_31)
		tilemap_mark_tile_dirty(atarigen_playfield_tilemap, offset * 2);
	if (ACCESSING_BITS_0_15)
		tilemap_mark_tile_dirty(atarigen_playfield_tilemap, offset * 2 + 1);
}

WRITE16_HANDLER( atarigen_playfield2_w )
{
	COMBINE_DATA(&atarigen_playfield2[offset]);
	tilemap_mark_tile_dirty(atarigen_playfield2_tilemap, offset);
}



/*---------------------------------------------------------------
    atarigen_playfield_large_w: Generic write handler for
    large (2-word) playfield RAM.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_playfield_large_w )
{
	COMBINE_DATA(&atarigen_playfield[offset]);
	tilemap_mark_tile_dirty(atarigen_playfield_tilemap, offset / 2);
}



/*---------------------------------------------------------------
    atarigen_playfield_upper_w: Generic write handler for
    upper word of split playfield RAM.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_playfield_upper_w )
{
	COMBINE_DATA(&atarigen_playfield_upper[offset]);
	tilemap_mark_tile_dirty(atarigen_playfield_tilemap, offset);
}



/*---------------------------------------------------------------
    atarigen_playfield_dual_upper_w: Generic write handler for
    upper word of split dual playfield RAM.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_playfield_dual_upper_w )
{
	COMBINE_DATA(&atarigen_playfield_upper[offset]);
	tilemap_mark_tile_dirty(atarigen_playfield_tilemap, offset);
	tilemap_mark_tile_dirty(atarigen_playfield2_tilemap, offset);
}



/*---------------------------------------------------------------
    atarigen_playfield_latched_lsb_w: Generic write handler for
    lower word of playfield RAM with a latch in the LSB of the
    upper word.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_playfield_latched_lsb_w )
{
	COMBINE_DATA(&atarigen_playfield[offset]);
	tilemap_mark_tile_dirty(atarigen_playfield_tilemap, offset);

	if (playfield_latch != -1)
		atarigen_playfield_upper[offset] = (atarigen_playfield_upper[offset] & ~0x00ff) | (playfield_latch & 0x00ff);
}



/*---------------------------------------------------------------
    atarigen_playfield_latched_lsb_w: Generic write handler for
    lower word of playfield RAM with a latch in the MSB of the
    upper word.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_playfield_latched_msb_w )
{
	COMBINE_DATA(&atarigen_playfield[offset]);
	tilemap_mark_tile_dirty(atarigen_playfield_tilemap, offset);

	if (playfield_latch != -1)
		atarigen_playfield_upper[offset] = (atarigen_playfield_upper[offset] & ~0xff00) | (playfield_latch & 0xff00);
}



/*---------------------------------------------------------------
    atarigen_playfield_latched_lsb_w: Generic write handler for
    lower word of second playfield RAM with a latch in the MSB
    of the upper word.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_playfield2_latched_msb_w )
{
	COMBINE_DATA(&atarigen_playfield2[offset]);
	tilemap_mark_tile_dirty(atarigen_playfield2_tilemap, offset);

	if (playfield2_latch != -1)
		atarigen_playfield_upper[offset] = (atarigen_playfield_upper[offset] & ~0xff00) | (playfield2_latch & 0xff00);
}




/***************************************************************************
    VIDEO HELPERS
***************************************************************************/

/*---------------------------------------------------------------
    atarigen_get_hblank: Returns a guesstimate about the current
    HBLANK state, based on the assumption that HBLANK represents
    10% of the scanline period.
---------------------------------------------------------------*/

int atarigen_get_hblank(const device_config *screen)
{
	return (video_screen_get_hpos(screen) > (video_screen_get_width(screen) * 9 / 10));
}


/*---------------------------------------------------------------
    atarigen_halt_until_hblank_0: Halts CPU 0 until the
    next HBLANK.
---------------------------------------------------------------*/

void atarigen_halt_until_hblank_0(const device_config *screen)
{
	const device_config *cpu = cputag_get_cpu(screen->machine, "maincpu");

	/* halt the CPU until the next HBLANK */
	int hpos = video_screen_get_hpos(screen);
	int width = video_screen_get_width(screen);
	int hblank = width * 9 / 10;
	double fraction;

	/* if we're in hblank, set up for the next one */
	if (hpos >= hblank)
		hblank += width;

	/* halt and set a timer to wake up */
	fraction = (double)(hblank - hpos) / (double)width;
	timer_set(screen->machine, double_to_attotime(attotime_to_double(video_screen_get_scan_period(screen)) * fraction), (void *)cpu, 0, unhalt_cpu);
	cpu_set_input_line(cpu, INPUT_LINE_HALT, ASSERT_LINE);
}


/*---------------------------------------------------------------
    atarigen_666_paletteram_w: 6-6-6 RGB palette RAM handler.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_666_paletteram_w )
{
	int newword, r, g, b;

	COMBINE_DATA(&paletteram16[offset]);
	newword = paletteram16[offset];

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
	COMBINE_DATA(&paletteram16[offset]);

	if (ACCESSING_BITS_8_15)
	{
		int palentry = offset / 2;
		int newword = (paletteram16[palentry * 2] & 0xff00) | (paletteram16[palentry * 2 + 1] >> 8);

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

	COMBINE_DATA(&paletteram32[offset]);

	if (ACCESSING_BITS_16_31)
	{
		newword = paletteram32[offset] >> 16;

		r = ((newword >> 9) & 0x3e) | ((newword >> 15) & 1);
		g = ((newword >> 4) & 0x3e) | ((newword >> 15) & 1);
		b = ((newword << 1) & 0x3e) | ((newword >> 15) & 1);

		palette_set_color_rgb(space->machine, offset * 2, pal6bit(r), pal6bit(g), pal6bit(b));
	}

	if (ACCESSING_BITS_0_15)
	{
		newword = paletteram32[offset] & 0xffff;

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
	const device_config *cpu = (const device_config *)ptr;
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


/***************************************************************************
    SAVE STATE
***************************************************************************/

void atarigen_init_save_state(running_machine *machine)
{
	state_save_register_global(machine, atarigen_scanline_int_state);
	state_save_register_global(machine, atarigen_sound_int_state);
	state_save_register_global(machine, atarigen_video_int_state);

	state_save_register_global(machine, atarigen_cpu_to_sound_ready);
	state_save_register_global(machine, atarigen_sound_to_cpu_ready);

	state_save_register_global(machine, atarivc_state.latch1);				/* latch #1 value (-1 means disabled) */
	state_save_register_global(machine, atarivc_state.latch2);				/* latch #2 value (-1 means disabled) */
	state_save_register_global(machine, atarivc_state.rowscroll_enable);		/* true if row-scrolling is enabled */
	state_save_register_global(machine, atarivc_state.palette_bank);			/* which palette bank is enabled */
	state_save_register_global(machine, atarivc_state.pf0_xscroll);			/* playfield 1 xscroll */
	state_save_register_global(machine, atarivc_state.pf0_xscroll_raw);		/* playfield 1 xscroll raw value */
	state_save_register_global(machine, atarivc_state.pf0_yscroll);			/* playfield 1 yscroll */
	state_save_register_global(machine, atarivc_state.pf1_xscroll);			/* playfield 2 xscroll */
	state_save_register_global(machine, atarivc_state.pf1_xscroll_raw);		/* playfield 2 xscroll raw value */
	state_save_register_global(machine, atarivc_state.pf1_yscroll);			/* playfield 2 yscroll */
	state_save_register_global(machine, atarivc_state.mo_xscroll);			/* sprite xscroll */
	state_save_register_global(machine, atarivc_state.mo_yscroll);			/* sprite xscroll */

	state_save_register_global(machine, eeprom_unlocked);

	state_save_register_global(machine, atarigen_slapstic_num);
	state_save_register_global(machine, atarigen_slapstic_bank);
	state_save_register_global(machine, atarigen_slapstic_last_pc);
	state_save_register_global(machine, atarigen_slapstic_last_address);

	state_save_register_global(machine, atarigen_cpu_to_sound);
	state_save_register_global(machine, atarigen_sound_to_cpu);
	state_save_register_global(machine, timed_int);
	state_save_register_global(machine, ym2151_int);

	state_save_register_global(machine, scanlines_per_callback);

	state_save_register_global(machine, actual_vc_latch0);
	state_save_register_global(machine, actual_vc_latch1);

	state_save_register_global(machine, playfield_latch);
	state_save_register_global(machine, playfield2_latch);

	/* need a postload to reset the state */
	state_save_register_postload(machine, slapstic_postload, NULL);
}
