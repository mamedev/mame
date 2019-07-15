// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    atarigen.cpp

    General functions for Atari games.

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "atarigen.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define SOUND_TIMER_RATE            attotime::from_usec(5)
#define SOUND_TIMER_BOOST           attotime::from_usec(1000)



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

inline const atarigen_screen_timer *atarigen_state::get_screen_timer(screen_device &screen)
{
	atarigen_state *state = screen.machine().driver_data<atarigen_state>();
	int i;

	// find the index of the timer that matches the screen
	for (i = 0; i < ARRAY_LENGTH(state->m_screen_timer); i++)
		if (state->m_screen_timer[i].screen == &screen)
			return &state->m_screen_timer[i];

	fatalerror("Unexpected: no atarivc_eof_update_timer for screen '%s'\n", screen.tag());
	return nullptr;
}



//**************************************************************************
//  SOUND COMMUNICATIONS DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(ATARI_SOUND_COMM, atari_sound_comm_device, "atarscom", "Atari Sound Communications")

//-------------------------------------------------
//  atari_sound_comm_device - constructor
//-------------------------------------------------

atari_sound_comm_device::atari_sound_comm_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ATARI_SOUND_COMM, tag, owner, clock)
	, m_main_int_cb(*this)
	, m_sound_cpu(*this, finder_base::DUMMY_TAG)
	, m_main_to_sound_ready(false)
	, m_sound_to_main_ready(false)
	, m_main_to_sound_data(0)
	, m_sound_to_main_data(0)
	, m_timed_int(0)
	, m_ym2151_int(0)
{
}


//-------------------------------------------------
//  device_start: Start up the device
//-------------------------------------------------

void atari_sound_comm_device::device_start()
{
	// resolve callbacks
	m_main_int_cb.resolve_safe();

	// register for save states
	save_item(NAME(m_main_to_sound_ready));
	save_item(NAME(m_sound_to_main_ready));
	save_item(NAME(m_main_to_sound_data));
	save_item(NAME(m_sound_to_main_data));
	save_item(NAME(m_timed_int));
	save_item(NAME(m_ym2151_int));
}


//-------------------------------------------------
//  device_reset: Handle a device reset by
//  clearing the interrupt lines and states
//-------------------------------------------------

void atari_sound_comm_device::device_reset()
{
	// reset the internal interrupts states
	m_timed_int = m_ym2151_int = 0;

	// reset the sound I/O states
	m_main_to_sound_data = m_sound_to_main_data = 0;
	m_main_to_sound_ready = m_sound_to_main_ready = false;
}


//-------------------------------------------------
//  device_timer: Handle device-specific timer
//  calbacks
//-------------------------------------------------

void atari_sound_comm_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TID_SOUND_RESET:
			delayed_sound_reset(param);
			break;

		case TID_SOUND_WRITE:
			delayed_sound_write(param);
			break;

		case TID_6502_WRITE:
			delayed_6502_write(param);
			break;
	}
}


//-------------------------------------------------
//  sound_irq_gen: Generates an IRQ signal to the
//  6502 sound processor.
//-------------------------------------------------

INTERRUPT_GEN_MEMBER(atari_sound_comm_device::sound_irq_gen)
{
	sound_irq();
}

void atari_sound_comm_device::sound_irq()
{
	m_timed_int = 1;
	update_sound_irq();
}

//-------------------------------------------------
//  sound_irq_ack_r: Resets the IRQ signal to the
//  6502 sound processor. Both reads and writes
//  can be used.
//-------------------------------------------------

u8 atari_sound_comm_device::sound_irq_ack_r()
{
	if (!machine().side_effects_disabled())
	{
		m_timed_int = 0;
		update_sound_irq();
	}
	return 0;
}

void atari_sound_comm_device::sound_irq_ack_w(u8 data)
{
	m_timed_int = 0;
	update_sound_irq();
}


//-------------------------------------------------
//  atarigen_ym2151_irq_gen: Sets the state of the
//  YM2151's IRQ line.
//-------------------------------------------------

WRITE_LINE_MEMBER(atari_sound_comm_device::ym2151_irq_gen)
{
	m_ym2151_int = state;
	update_sound_irq();
}


//-------------------------------------------------
//  sound_reset_w: Write handler which resets the
//  sound CPU in response.
//-------------------------------------------------

void atari_sound_comm_device::sound_reset_w(u16 data)
{
	synchronize(TID_SOUND_RESET);
}


//-------------------------------------------------
//  main_command_w: Handles communication from the main CPU
//  to the sound CPU. Two versions are provided, one with the
//  data byte in the low 8 bits, and one with the data byte in
//  the upper 8 bits.
//-------------------------------------------------

void atari_sound_comm_device::main_command_w(u8 data)
{
	synchronize(TID_SOUND_WRITE, data);
}


//-------------------------------------------------
//  main_response_r: Handles reading data communicated from the
//  sound CPU to the main CPU. Two versions are provided, one
//  with the data byte in the low 8 bits, and one with the data
//  byte in the upper 8 bits.
//-------------------------------------------------

u8 atari_sound_comm_device::main_response_r()
{
	if (!machine().side_effects_disabled())
	{
		m_sound_to_main_ready = false;
		m_main_int_cb(CLEAR_LINE);
	}
	return m_sound_to_main_data;
}


//-------------------------------------------------
//  sound_response_w: Handles communication from the
//  sound CPU to the main CPU.
//-------------------------------------------------

void atari_sound_comm_device::sound_response_w(u8 data)
{
	synchronize(TID_6502_WRITE, data);
}


//-------------------------------------------------
//  sound_command_r: Handles reading data
//  communicated from the main CPU to the sound
//  CPU.
//-------------------------------------------------

u8 atari_sound_comm_device::sound_command_r()
{
	if (!machine().side_effects_disabled())
	{
		m_main_to_sound_ready = false;
		m_sound_cpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	}
	return m_main_to_sound_data;
}


//-------------------------------------------------
//  update_sound_irq: Called whenever the IRQ state
//  changes. An interrupt is generated if either
//  sound_irq_gen() was called, or if the YM2151
//  generated an interrupt via the
//  ym2151_irq_gen() callback.
//-------------------------------------------------

void atari_sound_comm_device::update_sound_irq()
{
	if (m_timed_int || m_ym2151_int)
		m_sound_cpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
	else
		m_sound_cpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
}


//-------------------------------------------------
//  delayed_sound_reset: Synchronizes the sound
//  reset command between the two CPUs.
//-------------------------------------------------

void atari_sound_comm_device::delayed_sound_reset(int param)
{
	// unhalt and reset the sound CPU
	if (param == 0)
	{
		m_sound_cpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		m_sound_cpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
	}

	// reset the sound write state
	m_sound_to_main_ready = false;
	m_main_int_cb(CLEAR_LINE);

	// allocate a high frequency timer until a response is generated
	// the main CPU is *very* sensistive to the timing of the response
	machine().scheduler().boost_interleave(SOUND_TIMER_RATE, SOUND_TIMER_BOOST);
}


//-------------------------------------------------
//  delayed_sound_write: Synchronizes a data write
//  from the main CPU to the sound CPU.
//-------------------------------------------------

void atari_sound_comm_device::delayed_sound_write(int data)
{
	// warn if we missed something
	if (m_main_to_sound_ready)
		logerror("Missed command from 680x0\n");

	// set up the states and signal an NMI to the sound CPU
	m_main_to_sound_data = data;
	m_main_to_sound_ready = true;
	m_sound_cpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);

	// allocate a high frequency timer until a response is generated
	// the main CPU is *very* sensistive to the timing of the response
	machine().scheduler().boost_interleave(SOUND_TIMER_RATE, SOUND_TIMER_BOOST);
}


//-------------------------------------------------
//  delayed_6502_write: Synchronizes a data write
//  from the sound CPU to the main CPU.
//-------------------------------------------------

void atari_sound_comm_device::delayed_6502_write(int data)
{
	// warn if we missed something
	if (m_sound_to_main_ready)
		logerror("Missed result from 6502\n");

	// set up the states and signal the sound interrupt to the main CPU
	m_sound_to_main_data = data;
	m_sound_to_main_ready = true;
	m_main_int_cb(ASSERT_LINE);
}



/***************************************************************************
    OVERALL INIT
***************************************************************************/

atarigen_state::atarigen_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag)
	, m_scanline_int_state(0)
	, m_video_int_state(0)
	, m_xscroll(*this, "xscroll")
	, m_yscroll(*this, "yscroll")
	, m_slapstic_num(0)
	, m_slapstic(nullptr)
	, m_slapstic_bank(0)
	, m_slapstic_last_pc(0)
	, m_slapstic_last_address(0)
	, m_slapstic_base(0)
	, m_slapstic_mirror(0)
	, m_scanlines_per_callback(0)
	, m_maincpu(*this, "maincpu")
	, m_gfxdecode(*this, "gfxdecode")
	, m_screen(*this, "screen")
	, m_slapstic_device(*this, ":slapstic")
{
}

void atarigen_state::machine_start()
{
	// allocate timers for all screens
	int i = 0;
	for (screen_device &screen : screen_device_iterator(*this))
	{
		assert(i <= ARRAY_LENGTH(m_screen_timer));
		m_screen_timer[i].screen = &screen;
		m_screen_timer[i].scanline_interrupt_timer = timer_alloc(TID_SCANLINE_INTERRUPT, (void *)&screen);
		m_screen_timer[i].scanline_timer = timer_alloc(TID_SCANLINE_TIMER, (void *)&screen);
		i++;
	}

	save_item(NAME(m_scanline_int_state));
	save_item(NAME(m_video_int_state));

	save_item(NAME(m_slapstic_num));
	save_item(NAME(m_slapstic_bank));
	save_item(NAME(m_slapstic_last_pc));
	save_item(NAME(m_slapstic_last_address));

	save_item(NAME(m_scanlines_per_callback));
}


void atarigen_state::machine_reset()
{
	// reset the interrupt states
	m_video_int_state = m_scanline_int_state = 0;

	// reset the slapstic
	if (m_slapstic_num != 0)
	{
		if (!m_slapstic_device.found())
			fatalerror("Slapstic device is missing?\n");

		m_slapstic_device->slapstic_reset();
		slapstic_update_bank(m_slapstic_device->slapstic_bank());
	}
}


void atarigen_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TID_SCANLINE_INTERRUPT:
		{
			scanline_int_write_line(1);
			screen_device *screen = reinterpret_cast<screen_device *>(ptr);
			timer.adjust(screen->frame_period());
			break;
		}

		case TID_SCANLINE_TIMER:
			scanline_timer(timer, *reinterpret_cast<screen_device *>(ptr), param);
			break;

		// unhalt the CPU that was passed as a pointer
		case TID_UNHALT_CPU:
			reinterpret_cast<device_t *>(ptr)->execute().set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
			break;
	}
}


void atarigen_state::scanline_update(screen_device &screen, int scanline)
{
}


/***************************************************************************
    INTERRUPT HANDLING
***************************************************************************/

//-------------------------------------------------
//  scanline_int_set: Sets the scanline when the next
//  scanline interrupt should be generated.
//-------------------------------------------------

void atarigen_state::scanline_int_set(screen_device &screen, int scanline)
{
	get_screen_timer(screen)->scanline_interrupt_timer->adjust(screen.time_until_pos(scanline));
}


//-------------------------------------------------
//  scanline_int_write_line: Standard write line
//  callback for the scanline interrupt
//-------------------------------------------------

WRITE_LINE_MEMBER(atarigen_state::scanline_int_write_line)
{
	m_scanline_int_state = state;
	update_interrupts();
}


//-------------------------------------------------
//  scanline_int_ack_w: Resets the state of the
//  scanline interrupt.
//-------------------------------------------------

void atarigen_state::scanline_int_ack_w(u16 data)
{
	m_scanline_int_state = 0;
	update_interrupts();
}


//-------------------------------------------------
//  video_int_write_line: Standard write line
//  callback for the video interrupt.
//-------------------------------------------------

WRITE_LINE_MEMBER(atarigen_state::video_int_write_line)
{
	if (state)
	{
		m_video_int_state = 1;
		update_interrupts();
	}
}


//-------------------------------------------------
//  video_int_ack_w: Resets the state of the video
//  interrupt.
//-------------------------------------------------

void atarigen_state::video_int_ack_w(u16 data)
{
	m_video_int_state = 0;
	update_interrupts();
}



/***************************************************************************
    SLAPSTIC HANDLING
***************************************************************************/

inline void atarigen_state::slapstic_update_bank(int bank)
{
	// if the bank has changed, copy the memory; Pit Fighter needs this
	if (bank != m_slapstic_bank)
	{
		// bank 0 comes from the copy we made earlier
		if (bank == 0)
			memcpy(m_slapstic, &m_slapstic_bank0[0], 0x2000);
		else
			memcpy(m_slapstic, &m_slapstic[bank * 0x1000], 0x2000);

		// remember the current bank
		m_slapstic_bank = bank;
	}
}


void atarigen_state::device_post_load()
{
	if (m_slapstic_num != 0)
	{
		if (!m_slapstic_device.found())
			fatalerror("Slapstic device is missing?\n");

		slapstic_update_bank(m_slapstic_device->slapstic_bank());
	}
}


//-------------------------------------------------
//  slapstic_configure: Installs memory handlers for the
//  slapstic and sets the chip number.
//-------------------------------------------------

void atarigen_state::slapstic_configure(cpu_device &device, offs_t base, offs_t mirror, u8 *mem)
{
	if (!m_slapstic_device.found())
		fatalerror("Slapstic device is missing\n");

	// initialize the slapstic
	m_slapstic_num = m_slapstic_device->m_chipnum;
	m_slapstic_device->slapstic_init();

	// install the memory handlers
	address_space &program = device.space(AS_PROGRAM);
	program.install_readwrite_handler(base, base + 0x7fff, 0, mirror, 0, read16_delegate(FUNC(atarigen_state::slapstic_r), this), write16_delegate(FUNC(atarigen_state::slapstic_w), this));
	m_slapstic = (u16 *)mem;

	// allocate memory for a copy of bank 0
	m_slapstic_bank0.resize(0x2000);
	memcpy(&m_slapstic_bank0[0], m_slapstic, 0x2000);

	// ensure we recopy memory for the bank
	m_slapstic_bank = 0xff;

	// install an opcode base handler if we are a 68000 or variant
	m_slapstic_base = base;
	m_slapstic_mirror = mirror;
}


//-------------------------------------------------
//  slapstic_w: Assuming that the slapstic sits in
//  ROM memory space, we just simply tweak the slapstic at this
//  address and do nothing more.
//-------------------------------------------------

WRITE16_MEMBER(atarigen_state::slapstic_w)
{
	if (!m_slapstic_device.found())
		fatalerror("Slapstic device is missing?\n");

	slapstic_update_bank(m_slapstic_device->slapstic_tweak(space, offset));
}


//-------------------------------------------------
//  slapstic_r: Tweaks the slapstic at the appropriate
//  address and then reads a word from the underlying memory.
//-------------------------------------------------

READ16_MEMBER(atarigen_state::slapstic_r)
{
	if (!m_slapstic_device.found())
		fatalerror("Slapstic device is missing?\n");

	// fetch the result from the current bank first
	int result = m_slapstic[offset & 0xfff];

	if (!machine().side_effects_disabled())
	{
		// then determine the new one
		slapstic_update_bank(m_slapstic_device->slapstic_tweak(space, offset));
	}
	return result;
}



/***************************************************************************
    SCANLINE TIMING
***************************************************************************/

//-------------------------------------------------
//  scanline_timer_reset: Sets up the scanline timer.
//-------------------------------------------------

void atarigen_state::scanline_timer_reset(screen_device &screen, int frequency)
{
	// set the scanline callback
	m_scanlines_per_callback = frequency;

	// set a timer to go off at scanline 0
	if (frequency != 0)
		get_screen_timer(screen)->scanline_timer->adjust(screen.time_until_pos(0));
}


//-------------------------------------------------
//  scanline_timer: Called once every n scanlines
//  to generate the periodic callback to the main
//  system.
//-------------------------------------------------

void atarigen_state::scanline_timer(emu_timer &timer, screen_device &screen, int scanline)
{
	// callback
	scanline_update(screen, scanline);

	// generate another
	scanline += m_scanlines_per_callback;
	if (scanline >= screen.height())
		scanline = 0;
	timer.adjust(screen.time_until_pos(scanline), scanline);
}




/***************************************************************************
    VIDEO HELPERS
***************************************************************************/

//-------------------------------------------------
//  halt_until_hblank_0: Halts CPU 0 until the
//  next HBLANK.
//-------------------------------------------------

void atarigen_state::halt_until_hblank_0(device_t &device, screen_device &screen)
{
	// halt the CPU until the next HBLANK
	int hpos = screen.hpos();
	int width = screen.width();
	int hblank = width * 9 / 10;

	// if we're in hblank, set up for the next one
	if (hpos >= hblank)
		hblank += width;

	// halt and set a timer to wake up
	device.execute().set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	timer_set(screen.scan_period() * (hblank - hpos) / width, TID_UNHALT_CPU, 0, (void *)&device);
}


/***************************************************************************
    MISC HELPERS
***************************************************************************/

//-------------------------------------------------
//  blend_gfx: Takes two GFXElements and blends their
//  data together to form one. Then frees the second.
//-------------------------------------------------

void atarigen_state::blend_gfx(int gfx0, int gfx1, int mask0, int mask1)
{
	gfx_element *gx0 = m_gfxdecode->gfx(gfx0);
	gfx_element *gx1 = m_gfxdecode->gfx(gfx1);

	// allocate memory for the assembled data
	u8 *srcdata = auto_alloc_array(machine(), u8, gx0->elements() * gx0->width() * gx0->height());

	// loop over elements
	u8 *dest = srcdata;
	for (int c = 0; c < gx0->elements(); c++)
	{
		const u8 *c0base = gx0->get_data(c);
		const u8 *c1base = gx1->get_data(c);

		// loop over height
		for (int y = 0; y < gx0->height(); y++)
		{
			const u8 *c0 = c0base;
			const u8 *c1 = c1base;

			for (int x = 0; x < gx0->width(); x++)
				*dest++ = (*c0++ & mask0) | (*c1++ & mask1);
			c0base += gx0->rowbytes();
			c1base += gx1->rowbytes();
		}
	}

//  int newdepth = gx0->depth() * gx1->depth();
	int granularity = gx0->granularity();
	gx0->set_raw_layout(srcdata, gx0->width(), gx0->height(), gx0->elements(), 8 * gx0->width(), 8 * gx0->width() * gx0->height());
	gx0->set_granularity(granularity);

	// free the second graphics element
	m_gfxdecode->set_gfx(gfx1, nullptr);
}
