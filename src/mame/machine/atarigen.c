/***************************************************************************

    atarigen.c

    General functions for Atari games.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/


#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/2151intf.h"
#include "sound/2413intf.h"
#include "sound/tms5220.h"
#include "sound/okim6295.h"
#include "sound/pokey.h"
#include "video/atarimo.h"
#include "includes/slapstic.h"
#include "atarigen.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define SOUND_TIMER_RATE			attotime::from_usec(5)
#define SOUND_TIMER_BOOST			attotime::from_usec(100)



/***************************************************************************
    STATIC FUNCTION DECLARATIONS
***************************************************************************/

static void slapstic_postload(running_machine &machine);

static TIMER_CALLBACK( scanline_interrupt_callback );

static void update_6502_irq(running_machine &machine);
static TIMER_CALLBACK( delayed_sound_reset );
static TIMER_CALLBACK( delayed_sound_w );
static TIMER_CALLBACK( delayed_6502_sound_w );

static void atarigen_set_vol(running_machine &machine, int volume, device_type type);

static TIMER_CALLBACK( scanline_timer_callback );

static void atarivc_common_w(screen_device &screen, offs_t offset, UINT16 newword);

static TIMER_CALLBACK( unhalt_cpu );

static TIMER_CALLBACK( atarivc_eof_update );



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE const atarigen_screen_timer *get_screen_timer(screen_device &screen)
{
	atarigen_state *state = screen.machine().driver_data<atarigen_state>();
	int i;

	/* find the index of the timer that matches the screen */
	for (i = 0; i < ARRAY_LENGTH(state->m_screen_timer); i++)
		if (state->m_screen_timer[i].screen == &screen)
			return &state->m_screen_timer[i];

	fatalerror("Unexpected: no atarivc_eof_update_timer for screen '%s'\n", screen.tag());
	return NULL;
}



/***************************************************************************
    OVERALL INIT
***************************************************************************/

void atarigen_init(running_machine &machine)
{
	atarigen_state *state = machine.driver_data<atarigen_state>();
	screen_device *screen;
	int i;

	/* allocate timers for all screens */
	screen_device_iterator iter(machine.root_device());
	assert(iter.count() <= ARRAY_LENGTH(state->m_screen_timer));
	for (i = 0, screen = iter.first(); screen != NULL; i++, screen = iter.next())
	{
		state->m_screen_timer[i].screen = screen;
		state->m_screen_timer[i].scanline_interrupt_timer = machine.scheduler().timer_alloc(FUNC(scanline_interrupt_callback), (void *)screen);
		state->m_screen_timer[i].scanline_timer = machine.scheduler().timer_alloc(FUNC(scanline_timer_callback), (void *)screen);
		state->m_screen_timer[i].atarivc_eof_update_timer = machine.scheduler().timer_alloc(FUNC(atarivc_eof_update), (void *)screen);
	}

	state->save_item(NAME(state->m_scanline_int_state));
	state->save_item(NAME(state->m_sound_int_state));
	state->save_item(NAME(state->m_video_int_state));

	state->save_item(NAME(state->m_cpu_to_sound_ready));
	state->save_item(NAME(state->m_sound_to_cpu_ready));

	state->save_item(NAME(state->m_atarivc_state.latch1));				/* latch #1 value (-1 means disabled) */
	state->save_item(NAME(state->m_atarivc_state.latch2));				/* latch #2 value (-1 means disabled) */
	state->save_item(NAME(state->m_atarivc_state.rowscroll_enable));		/* true if row-scrolling is enabled */
	state->save_item(NAME(state->m_atarivc_state.palette_bank));			/* which palette bank is enabled */
	state->save_item(NAME(state->m_atarivc_state.pf0_xscroll));			/* playfield 1 xscroll */
	state->save_item(NAME(state->m_atarivc_state.pf0_xscroll_raw));		/* playfield 1 xscroll raw value */
	state->save_item(NAME(state->m_atarivc_state.pf0_yscroll));			/* playfield 1 yscroll */
	state->save_item(NAME(state->m_atarivc_state.pf1_xscroll));			/* playfield 2 xscroll */
	state->save_item(NAME(state->m_atarivc_state.pf1_xscroll_raw));		/* playfield 2 xscroll raw value */
	state->save_item(NAME(state->m_atarivc_state.pf1_yscroll));			/* playfield 2 yscroll */
	state->save_item(NAME(state->m_atarivc_state.mo_xscroll));			/* sprite xscroll */
	state->save_item(NAME(state->m_atarivc_state.mo_yscroll));			/* sprite xscroll */

	state->save_item(NAME(state->m_eeprom_unlocked));

	state->save_item(NAME(state->m_slapstic_num));
	state->save_item(NAME(state->m_slapstic_bank));
	state->save_item(NAME(state->m_slapstic_last_pc));
	state->save_item(NAME(state->m_slapstic_last_address));

	state->save_item(NAME(state->m_cpu_to_sound));
	state->save_item(NAME(state->m_sound_to_cpu));
	state->save_item(NAME(state->m_timed_int));
	state->save_item(NAME(state->m_ym2151_int));

	state->save_item(NAME(state->m_scanlines_per_callback));

	state->save_item(NAME(state->m_actual_vc_latch0));
	state->save_item(NAME(state->m_actual_vc_latch1));

	state->save_item(NAME(state->m_playfield_latch));
	state->save_item(NAME(state->m_playfield2_latch));

	/* need a postload to reset the state */
	machine.save().register_postload(save_prepost_delegate(FUNC(slapstic_postload), &machine));
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
	state->m_update_int_callback = update_int;

	/* reset the interrupt states */
	state->m_video_int_state = state->m_sound_int_state = state->m_scanline_int_state = 0;
}


/*---------------------------------------------------------------
    atarigen_update_interrupts: Forces the interrupt callback
    to be called with the current VBLANK and sound interrupt
    states.
---------------------------------------------------------------*/

void atarigen_update_interrupts(running_machine &machine)
{
	atarigen_state *state = machine.driver_data<atarigen_state>();
	(*state->m_update_int_callback)(machine);
}


/*---------------------------------------------------------------
    atarigen_scanline_int_set: Sets the scanline when the next
    scanline interrupt should be generated.
---------------------------------------------------------------*/

void atarigen_scanline_int_set(screen_device &screen, int scanline)
{
	emu_timer *timer = get_screen_timer(screen)->scanline_interrupt_timer;
	timer->adjust(screen.time_until_pos(scanline));
}


/*---------------------------------------------------------------
    atarigen_scanline_int_gen: Standard interrupt routine
    which sets the scanline interrupt state.
---------------------------------------------------------------*/

INTERRUPT_GEN( atarigen_scanline_int_gen )
{
	atarigen_state *state = device->machine().driver_data<atarigen_state>();
	state->m_scanline_int_state = 1;
	(*state->m_update_int_callback)(device->machine());
}


/*---------------------------------------------------------------
    atarigen_scanline_int_ack_w: Resets the state of the
    scanline interrupt.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_scanline_int_ack_w )
{
	atarigen_state *state = space->machine().driver_data<atarigen_state>();
	state->m_scanline_int_state = 0;
	(*state->m_update_int_callback)(space->machine());
}

WRITE32_HANDLER( atarigen_scanline_int_ack32_w )
{
	atarigen_state *state = space->machine().driver_data<atarigen_state>();
	state->m_scanline_int_state = 0;
	(*state->m_update_int_callback)(space->machine());
}


/*---------------------------------------------------------------
    atarigen_sound_int_gen: Standard interrupt routine which
    sets the sound interrupt state.
---------------------------------------------------------------*/

INTERRUPT_GEN( atarigen_sound_int_gen )
{
	atarigen_state *state = device->machine().driver_data<atarigen_state>();
	state->m_sound_int_state = 1;
	(*state->m_update_int_callback)(device->machine());
}


/*---------------------------------------------------------------
    atarigen_sound_int_ack_w: Resets the state of the sound
    interrupt.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_sound_int_ack_w )
{
	atarigen_state *state = space->machine().driver_data<atarigen_state>();
	state->m_sound_int_state = 0;
	(*state->m_update_int_callback)(space->machine());
}

WRITE32_HANDLER( atarigen_sound_int_ack32_w )
{
	atarigen_state *state = space->machine().driver_data<atarigen_state>();
	state->m_sound_int_state = 0;
	(*state->m_update_int_callback)(space->machine());
}


/*---------------------------------------------------------------
    atarigen_video_int_gen: Standard interrupt routine which
    sets the video interrupt state.
---------------------------------------------------------------*/

INTERRUPT_GEN( atarigen_video_int_gen )
{
	atarigen_state *state = device->machine().driver_data<atarigen_state>();
	state->m_video_int_state = 1;
	(*state->m_update_int_callback)(device->machine());
}


/*---------------------------------------------------------------
    atarigen_video_int_ack_w: Resets the state of the video
    interrupt.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_video_int_ack_w )
{
	atarigen_state *state = space->machine().driver_data<atarigen_state>();
	state->m_video_int_state = 0;
	(*state->m_update_int_callback)(space->machine());
}

WRITE32_HANDLER( atarigen_video_int_ack32_w )
{
	atarigen_state *state = space->machine().driver_data<atarigen_state>();
	state->m_video_int_state = 0;
	(*state->m_update_int_callback)(space->machine());
}


/*---------------------------------------------------------------
    scanline_interrupt_callback: Signals an interrupt.
---------------------------------------------------------------*/

static TIMER_CALLBACK( scanline_interrupt_callback )
{
	screen_device &screen = *reinterpret_cast<screen_device *>(ptr);
	emu_timer *timer = get_screen_timer(screen)->scanline_interrupt_timer;

	/* generate the interrupt */
	atarigen_scanline_int_gen(machine.device("maincpu"));

	/* set a new timer to go off at the same scan line next frame */
	timer->adjust(screen.frame_period());
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
	state->m_eeprom_unlocked = 0;
	if (state->m_eeprom == NULL && state->m_eeprom32 != NULL)
		state->m_eeprom.set_target(reinterpret_cast<UINT16 *>(state->m_eeprom32.target()), state->m_eeprom32.bytes());
}


/*---------------------------------------------------------------
    atarigen_eeprom_enable_w: Any write to this handler will
    allow one byte to be written to the EEPROM data area the
    next time.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_eeprom_enable_w )
{
	atarigen_state *state = space->machine().driver_data<atarigen_state>();
	state->m_eeprom_unlocked = 1;
}

WRITE32_HANDLER( atarigen_eeprom_enable32_w )
{
	atarigen_state *state = space->machine().driver_data<atarigen_state>();
	state->m_eeprom_unlocked = 1;
}


/*---------------------------------------------------------------
    atarigen_eeprom_w: Writes a "word" to the EEPROM, which is
    almost always accessed via the low byte of the word only.
    If the EEPROM hasn't been unlocked, the write attempt is
    ignored.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_eeprom_w )
{
	atarigen_state *state = space->machine().driver_data<atarigen_state>();

	if (!state->m_eeprom_unlocked)
		return;

	COMBINE_DATA(&state->m_eeprom[offset]);
	state->m_eeprom_unlocked = 0;
}

WRITE32_HANDLER( atarigen_eeprom32_w )
{
	atarigen_state *state = space->machine().driver_data<atarigen_state>();

	if (!state->m_eeprom_unlocked)
		return;

	COMBINE_DATA(&state->m_eeprom[offset * 2 + 1]);
	data >>= 16;
	mem_mask >>= 16;
	COMBINE_DATA(&state->m_eeprom[offset * 2]);
	state->m_eeprom_unlocked = 0;
}


/*---------------------------------------------------------------
    atarigen_eeprom_r: Reads a "word" from the EEPROM, which is
    almost always accessed via the low byte of the word only.
---------------------------------------------------------------*/

READ16_HANDLER( atarigen_eeprom_r )
{
	atarigen_state *state = space->machine().driver_data<atarigen_state>();
	return state->m_eeprom[offset] | 0xff00;
}

READ16_HANDLER( atarigen_eeprom_upper_r )
{
	atarigen_state *state = space->machine().driver_data<atarigen_state>();
	return state->m_eeprom[offset] | 0x00ff;
}

READ32_HANDLER( atarigen_eeprom_upper32_r )
{
	atarigen_state *state = space->machine().driver_data<atarigen_state>();
	return (state->m_eeprom[offset * 2] << 16) | state->m_eeprom[offset * 2 + 1] | 0x00ff00ff;
}



/***************************************************************************
    SLAPSTIC HANDLING
***************************************************************************/

INLINE void update_bank(atarigen_state *state, int bank)
{
	/* if the bank has changed, copy the memory; Pit Fighter needs this */
	if (bank != state->m_slapstic_bank)
	{
		/* bank 0 comes from the copy we made earlier */
		if (bank == 0)
			memcpy(state->m_slapstic, state->m_slapstic_bank0, 0x2000);
		else
			memcpy(state->m_slapstic, &state->m_slapstic[bank * 0x1000], 0x2000);

		/* remember the current bank */
		state->m_slapstic_bank = bank;
	}
}


static void slapstic_postload(running_machine &machine)
{
	atarigen_state *state = machine.driver_data<atarigen_state>();
	update_bank(state, slapstic_bank());
}


DIRECT_UPDATE_MEMBER(atarigen_state::atarigen_slapstic_setdirect)
{
	/* if we jump to an address in the slapstic region, tweak the slapstic
       at that address and return ~0; this will cause us to be called on
       subsequent fetches as well */
	address &= ~m_slapstic_mirror;
	if (address >= m_slapstic_base && address < m_slapstic_base + 0x8000)
	{
		offs_t pc = direct.space().device().safe_pcbase();
		if (pc != m_slapstic_last_pc || address != m_slapstic_last_address)
		{
			m_slapstic_last_pc = pc;
			m_slapstic_last_address = address;
			atarigen_slapstic_r(&direct.space(), (address >> 1) & 0x3fff, 0xffff);
		}
		return ~0;
	}

	return address;
}



/*---------------------------------------------------------------
    atarigen_slapstic_init: Installs memory handlers for the
    slapstic and sets the chip number.
---------------------------------------------------------------*/

void atarigen_slapstic_init(device_t *device, offs_t base, offs_t mirror, int chipnum)
{
	atarigen_state *state = device->machine().driver_data<atarigen_state>();

	/* reset in case we have no state */
	state->m_slapstic_num = chipnum;
	state->m_slapstic = NULL;

	/* if we have a chip, install it */
	if (chipnum != 0)
	{
		/* initialize the slapstic */
		slapstic_init(device->machine(), chipnum);

		/* install the memory handlers */
		state->m_slapstic = device->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(base, base + 0x7fff, 0, mirror, FUNC(atarigen_slapstic_r), FUNC(atarigen_slapstic_w));

		/* allocate memory for a copy of bank 0 */
		state->m_slapstic_bank0 = auto_alloc_array(device->machine(), UINT8, 0x2000);
		memcpy(state->m_slapstic_bank0, state->m_slapstic, 0x2000);

		/* ensure we recopy memory for the bank */
		state->m_slapstic_bank = 0xff;

		/* install an opcode base handler if we are a 68000 or variant */
		state->m_slapstic_base = base;
		state->m_slapstic_mirror = mirror;

		address_space *space = downcast<cpu_device *>(device)->space(AS_PROGRAM);
		space->set_direct_update_handler(direct_update_delegate(FUNC(atarigen_state::atarigen_slapstic_setdirect), state));
	}
}


/*---------------------------------------------------------------
    atarigen_slapstic_reset: Makes the selected slapstic number
    active and resets its state.
---------------------------------------------------------------*/

void atarigen_slapstic_reset(atarigen_state *state)
{
	if (state->m_slapstic_num != 0)
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
	atarigen_state *state = space->machine().driver_data<atarigen_state>();
	update_bank(state, slapstic_tweak(space, offset));
}


/*---------------------------------------------------------------
    atarigen_slapstic_r: Tweaks the slapstic at the appropriate
    address and then reads a word from the underlying memory.
---------------------------------------------------------------*/

READ16_HANDLER( atarigen_slapstic_r )
{
	/* fetch the result from the current bank first */
	atarigen_state *state = space->machine().driver_data<atarigen_state>();
	int result = state->m_slapstic[offset & 0xfff];

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

void atarigen_sound_io_reset(device_t *device)
{
	atarigen_state *state = device->machine().driver_data<atarigen_state>();

	/* remember which CPU is the sound CPU */
	state->m_sound_cpu = device;

	/* reset the internal interrupts states */
	state->m_timed_int = state->m_ym2151_int = 0;

	/* reset the sound I/O states */
	state->m_cpu_to_sound = state->m_sound_to_cpu = 0;
	state->m_cpu_to_sound_ready = state->m_sound_to_cpu_ready = 0;
}


/*---------------------------------------------------------------
    atarigen_6502_irq_gen: Generates an IRQ signal to the 6502
    sound processor.
---------------------------------------------------------------*/

INTERRUPT_GEN( atarigen_6502_irq_gen )
{
	atarigen_state *state = device->machine().driver_data<atarigen_state>();
	state->m_timed_int = 1;
	update_6502_irq(device->machine());
}


/*---------------------------------------------------------------
    atarigen_6502_irq_ack_r: Resets the IRQ signal to the 6502
    sound processor. Both reads and writes can be used.
---------------------------------------------------------------*/

READ8_HANDLER( atarigen_6502_irq_ack_r )
{
	atarigen_state *state = space->machine().driver_data<atarigen_state>();
	state->m_timed_int = 0;
	update_6502_irq(space->machine());
	return 0;
}

WRITE8_HANDLER( atarigen_6502_irq_ack_w )
{
	atarigen_state *state = space->machine().driver_data<atarigen_state>();
	state->m_timed_int = 0;
	update_6502_irq(space->machine());
}


/*---------------------------------------------------------------
    atarigen_ym2151_irq_gen: Sets the state of the YM2151's
    IRQ line.
---------------------------------------------------------------*/

void atarigen_ym2151_irq_gen(device_t *device, int irq)
{
	atarigen_state *state = device->machine().driver_data<atarigen_state>();
	state->m_ym2151_int = irq;
	update_6502_irq(device->machine());
}


/*---------------------------------------------------------------
    atarigen_sound_reset_w: Write handler which resets the
    sound CPU in response.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_sound_reset_w )
{
	space->machine().scheduler().synchronize(FUNC(delayed_sound_reset));
}


/*---------------------------------------------------------------
    atarigen_sound_reset: Resets the state of the sound CPU
    manually.
---------------------------------------------------------------*/

void atarigen_sound_reset(running_machine &machine)
{
	machine.scheduler().synchronize(FUNC(delayed_sound_reset), 1);
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
		space->machine().scheduler().synchronize(FUNC(delayed_sound_w), data & 0xff);
}

WRITE16_HANDLER( atarigen_sound_upper_w )
{
	if (ACCESSING_BITS_8_15)
		space->machine().scheduler().synchronize(FUNC(delayed_sound_w), (data >> 8) & 0xff);
}

WRITE32_HANDLER( atarigen_sound_upper32_w )
{
	if (ACCESSING_BITS_24_31)
		space->machine().scheduler().synchronize(FUNC(delayed_sound_w), (data >> 24) & 0xff);
}


/*---------------------------------------------------------------
    atarigen_sound_r: Handles reading data communicated from the
    sound CPU to the main CPU. Two versions are provided, one
    with the data byte in the low 8 bits, and one with the data
    byte in the upper 8 bits.
---------------------------------------------------------------*/

READ16_HANDLER( atarigen_sound_r )
{
	atarigen_state *state = space->machine().driver_data<atarigen_state>();
	state->m_sound_to_cpu_ready = 0;
	atarigen_sound_int_ack_w(space, 0, 0, 0xffff);
	return state->m_sound_to_cpu | 0xff00;
}

READ16_HANDLER( atarigen_sound_upper_r )
{
	atarigen_state *state = space->machine().driver_data<atarigen_state>();
	state->m_sound_to_cpu_ready = 0;
	atarigen_sound_int_ack_w(space, 0, 0, 0xffff);
	return (state->m_sound_to_cpu << 8) | 0x00ff;
}

READ32_HANDLER( atarigen_sound_upper32_r )
{
	atarigen_state *state = space->machine().driver_data<atarigen_state>();
	state->m_sound_to_cpu_ready = 0;
	atarigen_sound_int_ack32_w(space, 0, 0, 0xffff);
	return (state->m_sound_to_cpu << 24) | 0x00ffffff;
}


/*---------------------------------------------------------------
    atarigen_6502_sound_w: Handles communication from the sound
    CPU to the main CPU.
---------------------------------------------------------------*/

WRITE8_HANDLER( atarigen_6502_sound_w )
{
	space->machine().scheduler().synchronize(FUNC(delayed_6502_sound_w), data);
}


/*---------------------------------------------------------------
    atarigen_6502_sound_r: Handles reading data communicated
    from the main CPU to the sound CPU.
---------------------------------------------------------------*/

READ8_HANDLER( atarigen_6502_sound_r )
{
	atarigen_state *state = space->machine().driver_data<atarigen_state>();
	state->m_cpu_to_sound_ready = 0;
	device_set_input_line(state->m_sound_cpu, INPUT_LINE_NMI, CLEAR_LINE);
	return state->m_cpu_to_sound;
}


/*---------------------------------------------------------------
    update_6502_irq: Called whenever the IRQ state changes. An
    interrupt is generated if either atarigen_6502_irq_gen()
    was called, or if the YM2151 generated an interrupt via
    the atarigen_ym2151_irq_gen() callback.
---------------------------------------------------------------*/

static void update_6502_irq(running_machine &machine)
{
	atarigen_state *state = machine.driver_data<atarigen_state>();
	if (state->m_timed_int || state->m_ym2151_int)
		device_set_input_line(state->m_sound_cpu, M6502_IRQ_LINE, ASSERT_LINE);
	else
		device_set_input_line(state->m_sound_cpu, M6502_IRQ_LINE, CLEAR_LINE);
}


/*---------------------------------------------------------------
    delayed_sound_reset: Synchronizes the sound reset command
    between the two CPUs.
---------------------------------------------------------------*/

static TIMER_CALLBACK( delayed_sound_reset )
{
	atarigen_state *state = machine.driver_data<atarigen_state>();
	address_space *space = state->m_sound_cpu->memory().space(AS_PROGRAM);

	/* unhalt and reset the sound CPU */
	if (param == 0)
	{
		device_set_input_line(state->m_sound_cpu, INPUT_LINE_HALT, CLEAR_LINE);
		device_set_input_line(state->m_sound_cpu, INPUT_LINE_RESET, PULSE_LINE);
	}

	/* reset the sound write state */
	state->m_sound_to_cpu_ready = 0;
	atarigen_sound_int_ack_w(space, 0, 0, 0xffff);

	/* allocate a high frequency timer until a response is generated */
	/* the main CPU is *very* sensistive to the timing of the response */
	machine.scheduler().boost_interleave(SOUND_TIMER_RATE, SOUND_TIMER_BOOST);
}


/*---------------------------------------------------------------
    delayed_sound_w: Synchronizes a data write from the main
    CPU to the sound CPU.
---------------------------------------------------------------*/

static TIMER_CALLBACK( delayed_sound_w )
{
	atarigen_state *state = machine.driver_data<atarigen_state>();

	/* warn if we missed something */
	if (state->m_cpu_to_sound_ready)
		logerror("Missed command from 68010\n");

	/* set up the states and signal an NMI to the sound CPU */
	state->m_cpu_to_sound = param;
	state->m_cpu_to_sound_ready = 1;
	device_set_input_line(state->m_sound_cpu, INPUT_LINE_NMI, ASSERT_LINE);

	/* allocate a high frequency timer until a response is generated */
	/* the main CPU is *very* sensistive to the timing of the response */
	machine.scheduler().boost_interleave(SOUND_TIMER_RATE, SOUND_TIMER_BOOST);
}


/*---------------------------------------------------------------
    delayed_6502_sound_w: Synchronizes a data write from the
    sound CPU to the main CPU.
---------------------------------------------------------------*/

static TIMER_CALLBACK( delayed_6502_sound_w )
{
	atarigen_state *state = machine.driver_data<atarigen_state>();

	/* warn if we missed something */
	if (state->m_sound_to_cpu_ready)
		logerror("Missed result from 6502\n");

	/* set up the states and signal the sound interrupt to the main CPU */
	state->m_sound_to_cpu = param;
	state->m_sound_to_cpu_ready = 1;
	atarigen_sound_int_gen(machine.device("maincpu"));
}



/***************************************************************************
    SOUND HELPERS
***************************************************************************/

/*---------------------------------------------------------------
    atarigen_set_vol: Scans for a particular sound chip and
    changes the volume on all channels associated with it.
---------------------------------------------------------------*/

void atarigen_set_vol(running_machine &machine, int volume, device_type type)
{
	sound_interface_iterator iter(machine.root_device());
	for (device_sound_interface *sound = iter.first(); sound != NULL; sound = iter.next())
		if (sound->device().type() == type)
			sound->set_output_gain(ALL_OUTPUTS, volume / 100.0);
}


/*---------------------------------------------------------------
    atarigen_set_XXXXX_vol: Sets the volume for a given type
    of chip.
---------------------------------------------------------------*/

void atarigen_set_ym2151_vol(running_machine &machine, int volume)
{
	atarigen_set_vol(machine, volume, YM2151);
}

void atarigen_set_ym2413_vol(running_machine &machine, int volume)
{
	atarigen_set_vol(machine, volume, YM2413);
}

void atarigen_set_pokey_vol(running_machine &machine, int volume)
{
	atarigen_set_vol(machine, volume, POKEY);
}

void atarigen_set_tms5220_vol(running_machine &machine, int volume)
{
	atarigen_set_vol(machine, volume, TMS5220);
}

void atarigen_set_oki6295_vol(running_machine &machine, int volume)
{
	atarigen_set_vol(machine, volume, OKIM6295);
}



/***************************************************************************
    SCANLINE TIMING
***************************************************************************/

/*---------------------------------------------------------------
    atarigen_scanline_timer_reset: Sets up the scanline timer.
---------------------------------------------------------------*/

void atarigen_scanline_timer_reset(screen_device &screen, atarigen_scanline_func update_graphics, int frequency)
{
	atarigen_state *state = screen.machine().driver_data<atarigen_state>();

	/* set the scanline callback */
	state->m_scanline_callback = update_graphics;
	state->m_scanlines_per_callback = frequency;

	/* set a timer to go off at scanline 0 */
	if (state->m_scanline_callback != NULL)
	{
		emu_timer *timer = get_screen_timer(screen)->scanline_timer;
		timer->adjust(screen.time_until_pos(0));
	}
}


/*---------------------------------------------------------------
    scanline_timer_callback: Called once every n scanlines
    to generate the periodic callback to the main system.
---------------------------------------------------------------*/

static TIMER_CALLBACK( scanline_timer_callback )
{
	atarigen_state *state = machine.driver_data<atarigen_state>();
	screen_device &screen = *reinterpret_cast<screen_device *>(ptr);
	int scanline = param;

	/* callback */
	if (state->m_scanline_callback != NULL)
	{
		(*state->m_scanline_callback)(screen, scanline);

		/* generate another */
		scanline += state->m_scanlines_per_callback;
		if (scanline >= screen.height())
			scanline = 0;
		get_screen_timer(screen)->scanline_timer->adjust(screen.time_until_pos(scanline), scanline);
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
	atarigen_state *state = machine.driver_data<atarigen_state>();
	screen_device &screen = *reinterpret_cast<screen_device *>(ptr);
	emu_timer *timer = get_screen_timer(screen)->atarivc_eof_update_timer;
	int i;

	/* echo all the commands to the video controller */
	for (i = 0; i < 0x1c; i++)
		if (state->m_atarivc_eof_data[i])
			atarivc_common_w(screen, i, state->m_atarivc_eof_data[i]);

	/* update the scroll positions */
	atarimo_set_xscroll(0, state->m_atarivc_state.mo_xscroll);
	atarimo_set_yscroll(0, state->m_atarivc_state.mo_yscroll);

	state->m_playfield_tilemap->set_scrollx(0, state->m_atarivc_state.pf0_xscroll);
	state->m_playfield_tilemap->set_scrolly(0, state->m_atarivc_state.pf0_yscroll);

	if (state->m_atarivc_playfields > 1)
	{
		state->m_playfield2_tilemap->set_scrollx(0, state->m_atarivc_state.pf1_xscroll);
		state->m_playfield2_tilemap->set_scrolly(0, state->m_atarivc_state.pf1_yscroll);
	}
	timer->adjust(screen.time_until_pos(0));

	/* use this for debugging the video controller values */
#if 0
	if (machine.input().code_pressed(KEYCODE_8))
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
	atarigen_state *state = screen.machine().driver_data<atarigen_state>();

	/* this allows us to manually reset eof_data to NULL if it's not used */
	state->m_atarivc_eof_data.set_target(eof_data, 0x100);
	state->m_atarivc_playfields = playfields;

	/* clear the RAM we use */
	memset(state->m_atarivc_data, 0, 0x40);
	memset(&state->m_atarivc_state, 0, sizeof(state->m_atarivc_state));

	/* reset the latches */
	state->m_atarivc_state.latch1 = state->m_atarivc_state.latch2 = -1;
	state->m_actual_vc_latch0 = state->m_actual_vc_latch1 = -1;

	/* start a timer to go off a little before scanline 0 */
	if (state->m_atarivc_eof_data)
	{
		emu_timer *timer = get_screen_timer(screen)->atarivc_eof_update_timer;
		timer->adjust(screen.time_until_pos(0));
	}
}



/*---------------------------------------------------------------
    atarivc_w: Handles an I/O write to the video controller.
---------------------------------------------------------------*/

void atarivc_w(screen_device &screen, offs_t offset, UINT16 data, UINT16 mem_mask)
{
	atarigen_state *state = screen.machine().driver_data<atarigen_state>();
	int oldword = state->m_atarivc_data[offset];
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
	atarigen_state *state = screen.machine().driver_data<atarigen_state>();
	int oldword = state->m_atarivc_data[offset];
	state->m_atarivc_data[offset] = newword;

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
			atarigen_set_playfield_latch(state, (newword & 0x0080) ? state->m_actual_vc_latch0 : -1);
			atarigen_set_playfield2_latch(state, (newword & 0x0080) ? state->m_actual_vc_latch1 : -1);

			/* check for rowscroll enable */
			state->m_atarivc_state.rowscroll_enable = (newword & 0x2000) >> 13;

			/* check for palette banking */
			if (state->m_atarivc_state.palette_bank != (((newword & 0x0400) >> 10) ^ 1))
			{
				screen.update_partial(screen.vpos());
				state->m_atarivc_state.palette_bank = ((newword & 0x0400) >> 10) ^ 1;
			}
			break;

		/* indexed parameters */
		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b:
			switch (newword & 15)
			{
				case 9:
					state->m_atarivc_state.mo_xscroll = (newword >> 7) & 0x1ff;
					break;

				case 10:
					state->m_atarivc_state.pf1_xscroll_raw = (newword >> 7) & 0x1ff;
					atarivc_update_pf_xscrolls(state);
					break;

				case 11:
					state->m_atarivc_state.pf0_xscroll_raw = (newword >> 7) & 0x1ff;
					atarivc_update_pf_xscrolls(state);
					break;

				case 13:
					state->m_atarivc_state.mo_yscroll = (newword >> 7) & 0x1ff;
					break;

				case 14:
					state->m_atarivc_state.pf1_yscroll = (newword >> 7) & 0x1ff;
					break;

				case 15:
					state->m_atarivc_state.pf0_yscroll = (newword >> 7) & 0x1ff;
					break;
			}
			break;

		/* latch 1 value */
		case 0x1c:
			state->m_actual_vc_latch0 = -1;
			state->m_actual_vc_latch1 = newword;
			atarigen_set_playfield_latch(state, (state->m_atarivc_data[0x0a] & 0x80) ? state->m_actual_vc_latch0 : -1);
			atarigen_set_playfield2_latch(state, (state->m_atarivc_data[0x0a] & 0x80) ? state->m_actual_vc_latch1 : -1);
			break;

		/* latch 2 value */
		case 0x1d:
			state->m_actual_vc_latch0 = newword;
			state->m_actual_vc_latch1 = -1;
			atarigen_set_playfield_latch(state, (state->m_atarivc_data[0x0a] & 0x80) ? state->m_actual_vc_latch0 : -1);
			atarigen_set_playfield2_latch(state, (state->m_atarivc_data[0x0a] & 0x80) ? state->m_actual_vc_latch1 : -1);
			break;

		/* scanline IRQ ack here */
		case 0x1e:
			/* hack: this should be a device */
			atarigen_scanline_int_ack_w(screen.machine().device("maincpu")->memory().space(AS_PROGRAM), 0, 0, 0xffff);
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
	atarigen_state *state = screen.machine().driver_data<atarigen_state>();

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
		return state->m_atarivc_data[offset];
}



/***************************************************************************
    PLAYFIELD/ALPHA MAP HELPERS
***************************************************************************/

/*---------------------------------------------------------------
    atarigen_alpha_w: Generic write handler for alpha RAM.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_alpha_w )
{
	atarigen_state *state = space->machine().driver_data<atarigen_state>();
	COMBINE_DATA(&state->m_alpha[offset]);
	state->m_alpha_tilemap->mark_tile_dirty(offset);
}

WRITE32_HANDLER( atarigen_alpha32_w )
{
	atarigen_state *state = space->machine().driver_data<atarigen_state>();
	COMBINE_DATA(&state->m_alpha32[offset]);
	if (ACCESSING_BITS_16_31)
		state->m_alpha_tilemap->mark_tile_dirty(offset * 2);
	if (ACCESSING_BITS_0_15)
		state->m_alpha_tilemap->mark_tile_dirty(offset * 2 + 1);
}

WRITE16_HANDLER( atarigen_alpha2_w )
{
	atarigen_state *state = space->machine().driver_data<atarigen_state>();
	COMBINE_DATA(&state->m_alpha2[offset]);
	state->m_alpha2_tilemap->mark_tile_dirty(offset);
}



/*---------------------------------------------------------------
    atarigen_set_playfield_latch: Sets the latch for the latched
    playfield handlers below.
---------------------------------------------------------------*/

void atarigen_set_playfield_latch(atarigen_state *state, int data)
{
	state->m_playfield_latch = data;
}

void atarigen_set_playfield2_latch(atarigen_state *state, int data)
{
	state->m_playfield2_latch = data;
}



/*---------------------------------------------------------------
    atarigen_playfield_w: Generic write handler for PF RAM.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_playfield_w )
{
	atarigen_state *state = space->machine().driver_data<atarigen_state>();
	COMBINE_DATA(&state->m_playfield[offset]);
	state->m_playfield_tilemap->mark_tile_dirty(offset);
}

WRITE32_HANDLER( atarigen_playfield32_w )
{
	atarigen_state *state = space->machine().driver_data<atarigen_state>();
	COMBINE_DATA(&state->m_playfield32[offset]);
	if (ACCESSING_BITS_16_31)
		state->m_playfield_tilemap->mark_tile_dirty(offset * 2);
	if (ACCESSING_BITS_0_15)
		state->m_playfield_tilemap->mark_tile_dirty(offset * 2 + 1);
}

WRITE16_HANDLER( atarigen_playfield2_w )
{
	atarigen_state *state = space->machine().driver_data<atarigen_state>();
	COMBINE_DATA(&state->m_playfield2[offset]);
	state->m_playfield2_tilemap->mark_tile_dirty(offset);
}



/*---------------------------------------------------------------
    atarigen_playfield_large_w: Generic write handler for
    large (2-word) playfield RAM.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_playfield_large_w )
{
	atarigen_state *state = space->machine().driver_data<atarigen_state>();
	COMBINE_DATA(&state->m_playfield[offset]);
	state->m_playfield_tilemap->mark_tile_dirty(offset / 2);
}



/*---------------------------------------------------------------
    atarigen_playfield_upper_w: Generic write handler for
    upper word of split playfield RAM.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_playfield_upper_w )
{
	atarigen_state *state = space->machine().driver_data<atarigen_state>();
	COMBINE_DATA(&state->m_playfield_upper[offset]);
	state->m_playfield_tilemap->mark_tile_dirty(offset);
}



/*---------------------------------------------------------------
    atarigen_playfield_dual_upper_w: Generic write handler for
    upper word of split dual playfield RAM.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_playfield_dual_upper_w )
{
	atarigen_state *state = space->machine().driver_data<atarigen_state>();
	COMBINE_DATA(&state->m_playfield_upper[offset]);
	state->m_playfield_tilemap->mark_tile_dirty(offset);
	state->m_playfield2_tilemap->mark_tile_dirty(offset);
}



/*---------------------------------------------------------------
    atarigen_playfield_latched_lsb_w: Generic write handler for
    lower word of playfield RAM with a latch in the LSB of the
    upper word.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_playfield_latched_lsb_w )
{
	atarigen_state *state = space->machine().driver_data<atarigen_state>();

	COMBINE_DATA(&state->m_playfield[offset]);
	state->m_playfield_tilemap->mark_tile_dirty(offset);

	if (state->m_playfield_latch != -1)
		state->m_playfield_upper[offset] = (state->m_playfield_upper[offset] & ~0x00ff) | (state->m_playfield_latch & 0x00ff);
}



/*---------------------------------------------------------------
    atarigen_playfield_latched_lsb_w: Generic write handler for
    lower word of playfield RAM with a latch in the MSB of the
    upper word.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_playfield_latched_msb_w )
{
	atarigen_state *state = space->machine().driver_data<atarigen_state>();

	COMBINE_DATA(&state->m_playfield[offset]);
	state->m_playfield_tilemap->mark_tile_dirty(offset);

	if (state->m_playfield_latch != -1)
		state->m_playfield_upper[offset] = (state->m_playfield_upper[offset] & ~0xff00) | (state->m_playfield_latch & 0xff00);
}



/*---------------------------------------------------------------
    atarigen_playfield_latched_lsb_w: Generic write handler for
    lower word of second playfield RAM with a latch in the MSB
    of the upper word.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_playfield2_latched_msb_w )
{
	atarigen_state *state = space->machine().driver_data<atarigen_state>();

	COMBINE_DATA(&state->m_playfield2[offset]);
	state->m_playfield2_tilemap->mark_tile_dirty(offset);

	if (state->m_playfield2_latch != -1)
		state->m_playfield_upper[offset] = (state->m_playfield_upper[offset] & ~0xff00) | (state->m_playfield2_latch & 0xff00);
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
	device_t *cpu = screen.machine().device("maincpu");

	/* halt the CPU until the next HBLANK */
	int hpos = screen.hpos();
	int width = screen.width();
	int hblank = width * 9 / 10;

	/* if we're in hblank, set up for the next one */
	if (hpos >= hblank)
		hblank += width;

	/* halt and set a timer to wake up */
	screen.machine().scheduler().timer_set(screen.scan_period() * (hblank - hpos) / width, FUNC(unhalt_cpu), 0, (void *)cpu);
	device_set_input_line(cpu, INPUT_LINE_HALT, ASSERT_LINE);
}


/*---------------------------------------------------------------
    atarigen_666_paletteram_w: 6-6-6 RGB palette RAM handler.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_666_paletteram_w )
{
	atarigen_state *state = space->machine().driver_data<atarigen_state>();
	int newword, r, g, b;

	COMBINE_DATA(&state->m_generic_paletteram_16[offset]);
	newword = state->m_generic_paletteram_16[offset];

	r = ((newword >> 9) & 0x3e) | ((newword >> 15) & 1);
	g = ((newword >> 4) & 0x3e) | ((newword >> 15) & 1);
	b = ((newword << 1) & 0x3e) | ((newword >> 15) & 1);

	palette_set_color_rgb(space->machine(), offset, pal6bit(r), pal6bit(g), pal6bit(b));
}


/*---------------------------------------------------------------
    atarigen_expanded_666_paletteram_w: 6-6-6 RGB expanded
    palette RAM handler.
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_expanded_666_paletteram_w )
{
	atarigen_state *state = space->machine().driver_data<atarigen_state>();
	COMBINE_DATA(&state->m_generic_paletteram_16[offset]);

	if (ACCESSING_BITS_8_15)
	{
		int palentry = offset / 2;
		int newword = (state->m_generic_paletteram_16[palentry * 2] & 0xff00) | (state->m_generic_paletteram_16[palentry * 2 + 1] >> 8);

		int r, g, b;

		r = ((newword >> 9) & 0x3e) | ((newword >> 15) & 1);
		g = ((newword >> 4) & 0x3e) | ((newword >> 15) & 1);
		b = ((newword << 1) & 0x3e) | ((newword >> 15) & 1);

		palette_set_color_rgb(space->machine(), palentry & 0x1ff, pal6bit(r), pal6bit(g), pal6bit(b));
	}
}


/*---------------------------------------------------------------
    atarigen_666_paletteram32_w: 6-6-6 RGB palette RAM handler.
---------------------------------------------------------------*/

WRITE32_HANDLER( atarigen_666_paletteram32_w )
{
	atarigen_state *state = space->machine().driver_data<atarigen_state>();
	int newword, r, g, b;

	COMBINE_DATA(&state->m_generic_paletteram_32[offset]);

	if (ACCESSING_BITS_16_31)
	{
		newword = state->m_generic_paletteram_32[offset] >> 16;

		r = ((newword >> 9) & 0x3e) | ((newword >> 15) & 1);
		g = ((newword >> 4) & 0x3e) | ((newword >> 15) & 1);
		b = ((newword << 1) & 0x3e) | ((newword >> 15) & 1);

		palette_set_color_rgb(space->machine(), offset * 2, pal6bit(r), pal6bit(g), pal6bit(b));
	}

	if (ACCESSING_BITS_0_15)
	{
		newword = state->m_generic_paletteram_32[offset] & 0xffff;

		r = ((newword >> 9) & 0x3e) | ((newword >> 15) & 1);
		g = ((newword >> 4) & 0x3e) | ((newword >> 15) & 1);
		b = ((newword << 1) & 0x3e) | ((newword >> 15) & 1);

		palette_set_color_rgb(space->machine(), offset * 2 + 1, pal6bit(r), pal6bit(g), pal6bit(b));
	}
}


/*---------------------------------------------------------------
    unhalt_cpu: Timer callback to release the CPU from a halted state.
---------------------------------------------------------------*/

static TIMER_CALLBACK( unhalt_cpu )
{
	device_t *cpu = (device_t *)ptr;
	device_set_input_line(cpu, INPUT_LINE_HALT, CLEAR_LINE);
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

void atarigen_blend_gfx(running_machine &machine, int gfx0, int gfx1, int mask0, int mask1)
{
	gfx_element *gx0 = machine.gfx[gfx0];
	gfx_element *gx1 = machine.gfx[gfx1];
	UINT8 *srcdata, *dest;
	int c, x, y;

	/* allocate memory for the assembled data */
	srcdata = auto_alloc_array(machine, UINT8, gx0->elements() * gx0->width() * gx0->height());

	/* loop over elements */
	dest = srcdata;
	for (c = 0; c < gx0->elements(); c++)
	{
		const UINT8 *c0base = gx0->get_data(c);
		const UINT8 *c1base = gx1->get_data(c);

		/* loop over height */
		for (y = 0; y < gx0->height(); y++)
		{
			const UINT8 *c0 = c0base;
			const UINT8 *c1 = c1base;

			for (x = 0; x < gx0->width(); x++)
				*dest++ = (*c0++ & mask0) | (*c1++ & mask1);
			c0base += gx0->rowbytes();
			c1base += gx1->rowbytes();
		}
	}
	
//	int newdepth = gx0->depth() * gx1->depth();
	int granularity = gx0->granularity();
	gx0->set_raw_layout(srcdata, gx0->width(), gx0->height(), gx0->elements(), 8 * gx0->width(), 8 * gx0->width() * gx0->height());
	gx0->set_granularity(granularity);

	/* free the second graphics element */
	machine.gfx[gfx1] = NULL;
	auto_free(machine, gx1);
}



//**************************************************************************
//  VECTOR AND EARLY RASTER EAROM INTERFACE
//**************************************************************************

void atarigen_state::machine_start()
{
	// until everyone is converted to modern devices, call our parent
	driver_device::machine_start();

	save_item(NAME(m_earom_data));
	save_item(NAME(m_earom_control));
}


void atarigen_state::machine_reset()
{
	// until everyone is converted to modern devices, call our parent
	driver_device::machine_reset();

	// reset the control latch on the EAROM, if present
	if (m_earom != NULL)
		m_earom->set_control(0, 1, 1, 0, 0);
}



//**************************************************************************
//  VECTOR AND EARLY RASTER EAROM INTERFACE
//**************************************************************************

READ8_MEMBER( atarigen_state::earom_r )
{
	// return data latched from previous clock
	return m_earom->data();
}


WRITE8_MEMBER( atarigen_state::earom_w )
{
	// remember the value written
	m_earom_data = data;

	// output latch only enabled if control bit 2 is set
	if (m_earom_control & 4)
		m_earom->set_data(m_earom_data);

	// always latch the address
	m_earom->set_address(offset);
}


WRITE8_MEMBER( atarigen_state::earom_control_w )
{
	// remember the control state
	m_earom_control = data;

	// ensure ouput data is put on data lines prior to updating controls
	if (m_earom_control & 4)
		m_earom->set_data(m_earom_data);

	// set the control lines; /CS2 is always held low
	m_earom->set_control(data & 8, 1, ~data & 4, data & 2, data & 1);
}
