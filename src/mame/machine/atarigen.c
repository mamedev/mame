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

#define SOUND_TIMER_RATE            attotime::from_usec(5)
#define SOUND_TIMER_BOOST           attotime::from_usec(100)



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

inline const atarigen_screen_timer *get_screen_timer(screen_device &screen)
{
	atarigen_state *state = screen.machine().driver_data<atarigen_state>();
	int i;

	// find the index of the timer that matches the screen
	for (i = 0; i < ARRAY_LENGTH(state->m_screen_timer); i++)
		if (state->m_screen_timer[i].screen == &screen)
			return &state->m_screen_timer[i];

	fatalerror("Unexpected: no atarivc_eof_update_timer for screen '%s'\n", screen.tag());
	return NULL;
}



//**************************************************************************
//  SOUND COMMUNICATIONS DEVICE
//**************************************************************************

// device type definition
const device_type ATARI_SOUND_COMM = &device_creator<atari_sound_comm_device>;

//-------------------------------------------------
//  atari_sound_comm_device - constructor
//-------------------------------------------------

atari_sound_comm_device::atari_sound_comm_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ATARI_SOUND_COMM, "Atari Sound Communications", tag, owner, clock, "atarscom", __FILE__),
		m_sound_cpu_tag(NULL),
		m_main_int_cb(*this),
		m_sound_cpu(NULL),
		m_main_to_sound_ready(false),
		m_sound_to_main_ready(false),
		m_main_to_sound_data(0),
		m_sound_to_main_data(0),
		m_timed_int(0),
		m_ym2151_int(0)
{
}


//-------------------------------------------------
//  static_set_sound_cpu: Set the tag of the
//	sound CPU
//-------------------------------------------------

void atari_sound_comm_device::static_set_sound_cpu(device_t &device, const char *cputag)
{
	downcast<atari_sound_comm_device &>(device).m_sound_cpu_tag = cputag;
}


//-------------------------------------------------
//  device_start: Start up the device
//-------------------------------------------------

void atari_sound_comm_device::device_start()
{
	// find the sound CPU
	if (m_sound_cpu_tag == NULL)
		throw emu_fatalerror("No sound CPU specified!");
	m_sound_cpu = siblingdevice<m6502_device>(m_sound_cpu_tag);
	if (m_sound_cpu == NULL)
		throw emu_fatalerror("Sound CPU '%s' not found!", m_sound_cpu_tag);

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
//	clearing the interrupt lines and states
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
//	calbacks
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
	m_timed_int = 1;
	update_sound_irq();
}


//-------------------------------------------------
//  sound_irq_ack_r: Resets the IRQ signal to the 
//  6502 sound processor. Both reads and writes 
//  can be used.
//-------------------------------------------------

READ8_MEMBER(atari_sound_comm_device::sound_irq_ack_r)
{
	m_timed_int = 0;
	update_sound_irq();
	return 0;
}

WRITE8_MEMBER(atari_sound_comm_device::sound_irq_ack_w)
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

WRITE16_MEMBER(atari_sound_comm_device::sound_reset_w)
{
	synchronize(TID_SOUND_RESET);
}


//-------------------------------------------------
//  main_command_w: Handles communication from the main CPU
//  to the sound CPU. Two versions are provided, one with the
//  data byte in the low 8 bits, and one with the data byte in
//  the upper 8 bits.
//-------------------------------------------------

WRITE8_MEMBER(atari_sound_comm_device::main_command_w)
{
	synchronize(TID_SOUND_WRITE, data);
}


//-------------------------------------------------
//  main_response_r: Handles reading data communicated from the
//  sound CPU to the main CPU. Two versions are provided, one
//  with the data byte in the low 8 bits, and one with the data
//  byte in the upper 8 bits.
//-------------------------------------------------

READ8_MEMBER(atari_sound_comm_device::main_response_r)
{
	m_sound_to_main_ready = false;
	m_main_int_cb(CLEAR_LINE);
	return m_sound_to_main_data;
}


//-------------------------------------------------
//  sound_response_w: Handles communication from the
//  sound CPU to the main CPU.
//-------------------------------------------------

WRITE8_MEMBER(atari_sound_comm_device::sound_response_w)
{
	synchronize(TID_6502_WRITE, data);
}


//-------------------------------------------------
//  sound_command_r: Handles reading data
//  communicated from the main CPU to the sound
//  CPU.
//-------------------------------------------------

READ8_MEMBER(atari_sound_comm_device::sound_command_r)
{
	m_main_to_sound_ready = false;
	m_sound_cpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	return m_main_to_sound_data;
}


//-------------------------------------------------
//  update_sound_irq: Called whenever the IRQ state 
//  changes. An interrupt is generated if either 
//	sound_irq_gen() was called, or if the YM2151 
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
//	reset command between the two CPUs.
//-------------------------------------------------

void atari_sound_comm_device::delayed_sound_reset(int param)
{
	// unhalt and reset the sound CPU
	if (param == 0)
	{
		m_sound_cpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		m_sound_cpu->set_input_line(INPUT_LINE_RESET, PULSE_LINE);
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
		logerror("Missed command from 68010\n");

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
	: driver_device(mconfig, type, tag),
		m_earom(*this, "earom"),
		m_earom_data(0),
		m_earom_control(0),
		m_eeprom(*this, "eeprom"),
		m_eeprom32(*this, "eeprom"),
		m_scanline_int_state(0),
		m_sound_int_state(0),
		m_video_int_state(0),
		m_eeprom_default(NULL),
		m_xscroll(*this, "xscroll"),
		m_yscroll(*this, "yscroll"),
			m_atarivc_playfield_tilemap(*this, "playfield"),
			m_atarivc_playfield2_tilemap(*this, "playfield2"),
			m_atarivc_data(*this, "atarivc_data"),
			m_atarivc_eof_data(*this, "atarivc_eof"),
			m_actual_vc_latch0(0),
			m_actual_vc_latch1(0),
			m_atarivc_playfields(0),
			m_atarivc_playfield_latch(0),
			m_atarivc_playfield2_latch(0),
		m_eeprom_unlocked(false),
		m_slapstic_num(0),
		m_slapstic(NULL),
		m_slapstic_bank(0),
		m_slapstic_last_pc(0),
		m_slapstic_last_address(0),
		m_slapstic_base(0),
		m_slapstic_mirror(0),
		m_scanlines_per_callback(0),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_soundcomm(*this, "soundcomm")
{
}

void atarigen_state::machine_start()
{
	screen_device *screen;
	int i;

	// allocate timers for all screens
	screen_device_iterator iter(*this);
	assert(iter.count() <= ARRAY_LENGTH(m_screen_timer));
	for (i = 0, screen = iter.first(); screen != NULL; i++, screen = iter.next())
	{
		m_screen_timer[i].screen = screen;
		m_screen_timer[i].scanline_interrupt_timer = timer_alloc(TID_SCANLINE_INTERRUPT, (void *)screen);
		m_screen_timer[i].scanline_timer = timer_alloc(TID_SCANLINE_TIMER, (void *)screen);
		m_screen_timer[i].atarivc_eof_update_timer = timer_alloc(TID_ATARIVC_EOF, (void *)screen);
	}

	save_item(NAME(m_scanline_int_state));
	save_item(NAME(m_sound_int_state));
	save_item(NAME(m_video_int_state));

	save_item(NAME(m_atarivc_state.latch1));                // latch #1 value (-1 means disabled)
	save_item(NAME(m_atarivc_state.latch2));                // latch #2 value (-1 means disabled)
	save_item(NAME(m_atarivc_state.rowscroll_enable));      // true if row-scrolling is enabled
	save_item(NAME(m_atarivc_state.palette_bank));          // which palette bank is enabled
	save_item(NAME(m_atarivc_state.pf0_xscroll));           // playfield 1 xscroll
	save_item(NAME(m_atarivc_state.pf0_xscroll_raw));       // playfield 1 xscroll raw value
	save_item(NAME(m_atarivc_state.pf0_yscroll));           // playfield 1 yscroll
	save_item(NAME(m_atarivc_state.pf1_xscroll));           // playfield 2 xscroll
	save_item(NAME(m_atarivc_state.pf1_xscroll_raw));       // playfield 2 xscroll raw value
	save_item(NAME(m_atarivc_state.pf1_yscroll));           // playfield 2 yscroll
	save_item(NAME(m_atarivc_state.mo_xscroll));            // sprite xscroll
	save_item(NAME(m_atarivc_state.mo_yscroll));            // sprite xscroll
	save_item(NAME(m_actual_vc_latch0));
	save_item(NAME(m_actual_vc_latch1));
	save_item(NAME(m_atarivc_playfield_latch));
	save_item(NAME(m_atarivc_playfield2_latch));

	save_item(NAME(m_eeprom_unlocked));

	save_item(NAME(m_slapstic_num));
	save_item(NAME(m_slapstic_bank));
	save_item(NAME(m_slapstic_last_pc));
	save_item(NAME(m_slapstic_last_address));

	save_item(NAME(m_scanlines_per_callback));

	save_item(NAME(m_earom_data));
	save_item(NAME(m_earom_control));
}


void atarigen_state::machine_reset()
{
	// reset the interrupt states
	m_video_int_state = m_sound_int_state = m_scanline_int_state = 0;

	// reset the control latch on the EAROM, if present
	if (m_earom != NULL)
		m_earom->set_control(0, 1, 1, 0, 0);

	// reset the EEPROM
	m_eeprom_unlocked = false;
	if (m_eeprom == NULL && m_eeprom32 != NULL)
		m_eeprom.set_target(reinterpret_cast<UINT16 *>(m_eeprom32.target()), m_eeprom32.bytes());

	// reset the slapstic
	if (m_slapstic_num != 0)
	{
		slapstic_reset();
		slapstic_update_bank(slapstic_bank());
	}
}


void atarigen_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TID_SCANLINE_INTERRUPT:
		{
			scanline_int_gen(*m_maincpu);
			screen_device *screen = reinterpret_cast<screen_device *>(ptr);
			timer.adjust(screen->frame_period());
			break;
		}

		case TID_SCANLINE_TIMER:
			scanline_timer(timer, *reinterpret_cast<screen_device *>(ptr), param);
			break;

		case TID_ATARIVC_EOF:
			atarivc_eof_update(timer, *reinterpret_cast<screen_device *>(ptr));
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
//  scanline_int_gen: Standard interrupt routine
//  which sets the scanline interrupt state.
//-------------------------------------------------

INTERRUPT_GEN_MEMBER(atarigen_state::scanline_int_gen)
{
	m_scanline_int_state = 1;
	update_interrupts();
}


//-------------------------------------------------
//  scanline_int_ack_w: Resets the state of the
//  scanline interrupt.
//-------------------------------------------------

WRITE16_MEMBER(atarigen_state::scanline_int_ack_w)
{
	m_scanline_int_state = 0;
	update_interrupts();
}


//-------------------------------------------------
//  sound_int_write_line: Standard write line
//  callback for the sound interrupt
//-------------------------------------------------

WRITE_LINE_MEMBER(atarigen_state::sound_int_write_line)
{
	m_sound_int_state = state;
	update_interrupts();
}


//-------------------------------------------------
//  sound_int_gen: Standard interrupt routine which
//  sets the sound interrupt state.
//-------------------------------------------------

INTERRUPT_GEN_MEMBER(atarigen_state::sound_int_gen)
{
	m_sound_int_state = 1;
	update_interrupts();
}


//-------------------------------------------------
//  sound_int_ack_w: Resets the state of the sound
//  interrupt.
//-------------------------------------------------

WRITE16_MEMBER(atarigen_state::sound_int_ack_w)
{
	m_sound_int_state = 0;
	update_interrupts();
}


//-------------------------------------------------
//  video_int_gen: Standard interrupt routine which
//  sets the video interrupt state.
//-------------------------------------------------

INTERRUPT_GEN_MEMBER(atarigen_state::video_int_gen)
{
	m_video_int_state = 1;
	update_interrupts();
}


//-------------------------------------------------
//  video_int_ack_w: Resets the state of the video
//  interrupt.
//-------------------------------------------------

WRITE16_MEMBER(atarigen_state::video_int_ack_w)
{
	m_video_int_state = 0;
	update_interrupts();
}



/***************************************************************************
    EEPROM HANDLING
***************************************************************************/

//-------------------------------------------------
//  eeprom_enable_w: Any write to this handler will
//  allow one byte to be written to the EEPROM data area the
//  next time.
//-------------------------------------------------

WRITE16_MEMBER(atarigen_state::eeprom_enable_w)
{
	m_eeprom_unlocked = true;
}


//-------------------------------------------------
//  eeprom_w: Writes a "word" to the EEPROM, which is
//  almost always accessed via the low byte of the word only.
//  If the EEPROM hasn't been unlocked, the write attempt is
//  ignored.
//-------------------------------------------------

WRITE16_MEMBER(atarigen_state::eeprom_w)
{
	if (!m_eeprom_unlocked)
		return;

	COMBINE_DATA(&m_eeprom[offset]);
	m_eeprom_unlocked = false;
}

WRITE32_MEMBER(atarigen_state::eeprom32_w)
{
	if (!m_eeprom_unlocked)
		return;

	COMBINE_DATA(&m_eeprom[offset * 2 + 1]);
	data >>= 16;
	mem_mask >>= 16;
	COMBINE_DATA(&m_eeprom[offset * 2]);
	m_eeprom_unlocked = false;
}


//-------------------------------------------------
//  eeprom_r: Reads a "word" from the EEPROM, which is
//  almost always accessed via the low byte of the word only.
//-------------------------------------------------

READ16_MEMBER(atarigen_state::eeprom_r)
{
	return m_eeprom[offset] | 0xff00;
}

READ32_MEMBER(atarigen_state::eeprom_upper32_r)
{
	return (m_eeprom[offset * 2] << 16) | m_eeprom[offset * 2 + 1] | 0x00ff00ff;
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
			memcpy(m_slapstic, m_slapstic_bank0, 0x2000);
		else
			memcpy(m_slapstic, &m_slapstic[bank * 0x1000], 0x2000);

		// remember the current bank
		m_slapstic_bank = bank;
	}
}


void atarigen_state::device_post_load()
{
	slapstic_update_bank(slapstic_bank());
}


DIRECT_UPDATE_MEMBER(atarigen_state::slapstic_setdirect)
{
	// if we jump to an address in the slapstic region, tweak the slapstic
	// at that address and return ~0; this will cause us to be called on
	// subsequent fetches as well
	address &= ~m_slapstic_mirror;
	if (address >= m_slapstic_base && address < m_slapstic_base + 0x8000)
	{
		offs_t pc = direct.space().device().safe_pcbase();
		if (pc != m_slapstic_last_pc || address != m_slapstic_last_address)
		{
			m_slapstic_last_pc = pc;
			m_slapstic_last_address = address;
			slapstic_r(direct.space(), (address >> 1) & 0x3fff, 0xffff);
		}
		return ~0;
	}
	return address;
}



//-------------------------------------------------
//  slapstic_configure: Installs memory handlers for the
//  slapstic and sets the chip number.
//-------------------------------------------------

void atarigen_state::slapstic_configure(cpu_device &device, offs_t base, offs_t mirror, int chipnum)
{
	// reset in case we have no state
	m_slapstic_num = chipnum;
	m_slapstic = NULL;

	// if we have a chip, install it
	if (chipnum != 0)
	{
		// initialize the slapstic
		slapstic_init(machine(), chipnum);

		// install the memory handlers
		address_space &program = device.space(AS_PROGRAM);
		m_slapstic = program.install_readwrite_handler(base, base + 0x7fff, 0, mirror, read16_delegate(FUNC(atarigen_state::slapstic_r), this), write16_delegate(FUNC(atarigen_state::slapstic_w), this));
		program.set_direct_update_handler(direct_update_delegate(FUNC(atarigen_state::slapstic_setdirect), this));

		// allocate memory for a copy of bank 0
		m_slapstic_bank0.resize(0x2000);
		memcpy(m_slapstic_bank0, m_slapstic, 0x2000);

		// ensure we recopy memory for the bank
		m_slapstic_bank = 0xff;

		// install an opcode base handler if we are a 68000 or variant
		m_slapstic_base = base;
		m_slapstic_mirror = mirror;
	}
}


//-------------------------------------------------
//  slapstic_w: Assuming that the slapstic sits in
//  ROM memory space, we just simply tweak the slapstic at this
//  address and do nothing more.
//-------------------------------------------------

WRITE16_MEMBER(atarigen_state::slapstic_w)
{
	slapstic_update_bank(slapstic_tweak(space, offset));
}


//-------------------------------------------------
//  slapstic_r: Tweaks the slapstic at the appropriate
//  address and then reads a word from the underlying memory.
//-------------------------------------------------

READ16_MEMBER(atarigen_state::slapstic_r)
{
	// fetch the result from the current bank first
	int result = m_slapstic[offset & 0xfff];

	// then determine the new one
	slapstic_update_bank(slapstic_tweak(space, offset));
	return result;
}



/***************************************************************************
    SOUND HELPERS
***************************************************************************/

//-------------------------------------------------
//  set_volume_by_type: Scans for a particular
//  sound chip and changes the volume on all
//  channels associated with it.
//-------------------------------------------------

void atarigen_state::set_volume_by_type(int volume, device_type type)
{
	sound_interface_iterator iter(*this);
	for (device_sound_interface *sound = iter.first(); sound != NULL; sound = iter.next())
		if (sound->device().type() == type)
			sound->set_output_gain(ALL_OUTPUTS, volume / 100.0);
}


//-------------------------------------------------
//  set_XXXXX_volume: Sets the volume for a given
//  type of chip.
//-------------------------------------------------

void atarigen_state::set_ym2151_volume(int volume)
{
	set_volume_by_type(volume, YM2151);
}

void atarigen_state::set_ym2413_volume(int volume)
{
	set_volume_by_type(volume, YM2413);
}

void atarigen_state::set_pokey_volume(int volume)
{
	set_volume_by_type(volume, POKEY);
}

void atarigen_state::set_tms5220_volume(int volume)
{
	set_volume_by_type(volume, TMS5220);
}

void atarigen_state::set_oki6295_volume(int volume)
{
	set_volume_by_type(volume, OKIM6295);
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
    VIDEO CONTROLLER
***************************************************************************/

//-------------------------------------------------
//  atarivc_eof_update: Callback that slurps up data and feeds
//  it into the video controller registers every refresh.
//-------------------------------------------------

void atarigen_state::atarivc_eof_update(emu_timer &timer, screen_device &screen)
{
	// echo all the commands to the video controller
	for (int i = 0; i < 0x1c; i++)
		if (m_atarivc_eof_data[i])
			atarivc_common_w(screen, i, m_atarivc_eof_data[i]);

	// update the scroll positions
	atarimo_set_xscroll(0, m_atarivc_state.mo_xscroll);
	atarimo_set_yscroll(0, m_atarivc_state.mo_yscroll);

	m_atarivc_playfield_tilemap->set_scrollx(0, m_atarivc_state.pf0_xscroll);
	m_atarivc_playfield_tilemap->set_scrolly(0, m_atarivc_state.pf0_yscroll);

	if (m_atarivc_playfields > 1)
	{
		m_atarivc_playfield2_tilemap->set_scrollx(0, m_atarivc_state.pf1_xscroll);
		m_atarivc_playfield2_tilemap->set_scrolly(0, m_atarivc_state.pf1_yscroll);
	}
	timer.adjust(screen.time_until_pos(0));

	// use this for debugging the video controller values
#if 0
	if (machine().input().code_pressed(KEYCODE_8))
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


//-------------------------------------------------
//  atarivc_reset: Initializes the video controller.
//-------------------------------------------------

void atarigen_state::atarivc_reset(screen_device &screen, UINT16 *eof_data, int playfields)
{
	// this allows us to manually reset eof_data to NULL if it's not used
	m_atarivc_eof_data.set_target(eof_data, 0x100);
	m_atarivc_playfields = playfields;
	
	if (playfields == 2 && m_atarivc_playfield2_tilemap != NULL)
		m_atarivc_playfield2_tilemap->extmem().set(m_atarivc_playfield_tilemap->extmem());

	// clear the RAM we use
	memset(m_atarivc_data, 0, 0x40);
	memset(&m_atarivc_state, 0, sizeof(m_atarivc_state));

	// reset the latches
	m_atarivc_state.latch1 = m_atarivc_state.latch2 = -1;
	m_actual_vc_latch0 = m_actual_vc_latch1 = -1;

	// start a timer to go off a little before scanline 0
	if (m_atarivc_eof_data != NULL)
		get_screen_timer(screen)->atarivc_eof_update_timer->adjust(screen.time_until_pos(0));
}


//-------------------------------------------------
//  atarivc_w: Handles an I/O write to the video controller.
//-------------------------------------------------

void atarigen_state::atarivc_w(screen_device &screen, offs_t offset, UINT16 data, UINT16 mem_mask)
{
	int oldword = m_atarivc_data[offset];
	int newword = oldword;

	COMBINE_DATA(&newword);
	atarivc_common_w(screen, offset, newword);
}



//-------------------------------------------------
//  atarivc_common_w: Does the bulk of the word for an I/O
//  write.
//-------------------------------------------------

void atarigen_state::atarivc_common_w(screen_device &screen, offs_t offset, UINT16 newword)
{
	int oldword = m_atarivc_data[offset];
	m_atarivc_data[offset] = newword;

	// switch off the offset
	switch (offset)
	{
		//
		//  additional registers:
		//
		//      01 = vertical start (for centering)
		//      04 = horizontal start (for centering)
		//

		// set the scanline interrupt here
		case 0x03:
			if (oldword != newword)
				scanline_int_set(screen, newword & 0x1ff);
			break;

		// latch enable
		case 0x0a:

			// reset the latches when disabled
			m_atarivc_playfield_latch = (newword & 0x0080) ? m_actual_vc_latch0 : -1;
			m_atarivc_playfield2_latch = (newword & 0x0080) ? m_actual_vc_latch1 : -1;

			// check for rowscroll enable
			m_atarivc_state.rowscroll_enable = (newword & 0x2000) >> 13;

			// check for palette banking
			if (m_atarivc_state.palette_bank != (((newword & 0x0400) >> 10) ^ 1))
			{
				screen.update_partial(screen.vpos());
				m_atarivc_state.palette_bank = ((newword & 0x0400) >> 10) ^ 1;
			}
			break;

		// indexed parameters
		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b:
			switch (newword & 15)
			{
				case 9:
					m_atarivc_state.mo_xscroll = (newword >> 7) & 0x1ff;
					break;

				case 10:
					m_atarivc_state.pf1_xscroll_raw = (newword >> 7) & 0x1ff;
					atarivc_update_pf_xscrolls();
					break;

				case 11:
					m_atarivc_state.pf0_xscroll_raw = (newword >> 7) & 0x1ff;
					atarivc_update_pf_xscrolls();
					break;

				case 13:
					m_atarivc_state.mo_yscroll = (newword >> 7) & 0x1ff;
					break;

				case 14:
					m_atarivc_state.pf1_yscroll = (newword >> 7) & 0x1ff;
					break;

				case 15:
					m_atarivc_state.pf0_yscroll = (newword >> 7) & 0x1ff;
					break;
			}
			break;

		// latch 1 value
		case 0x1c:
			m_actual_vc_latch0 = -1;
			m_actual_vc_latch1 = newword;
			m_atarivc_playfield_latch = (m_atarivc_data[0x0a] & 0x80) ? m_actual_vc_latch0 : -1;
			m_atarivc_playfield2_latch = (m_atarivc_data[0x0a] & 0x80) ? m_actual_vc_latch1 : -1;
			break;

		// latch 2 value
		case 0x1d:
			m_actual_vc_latch0 = newword;
			m_actual_vc_latch1 = -1;
			m_atarivc_playfield_latch = (m_atarivc_data[0x0a] & 0x80) ? m_actual_vc_latch0 : -1;
			m_atarivc_playfield2_latch = (m_atarivc_data[0x0a] & 0x80) ? m_actual_vc_latch1 : -1;
			break;

		// scanline IRQ ack here
		case 0x1e:
			// hack: this should be a device
			scanline_int_ack_w(m_maincpu->space(AS_PROGRAM), 0, 0, 0xffff);
			break;

		// log anything else
		case 0x00:
		default:
			if (oldword != newword)
				logerror("vc_w(%02X, %04X) ** [prev=%04X]\n", offset, newword, oldword);
			break;
	}
}


//-------------------------------------------------
//  atarivc_r: Handles an I/O read from the video controller.
//-------------------------------------------------

UINT16 atarigen_state::atarivc_r(screen_device &screen, offs_t offset)
{
	logerror("vc_r(%02X)\n", offset);

	// a read from offset 0 returns the current scanline
	// also sets bit 0x4000 if we're in VBLANK
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
		return m_atarivc_data[offset];
}



/***************************************************************************
    PLAYFIELD/ALPHA MAP HELPERS
***************************************************************************/

//-------------------------------------------------
//  playfield_upper_w: Generic write handler for
//  upper word of split playfield RAM.
//-------------------------------------------------

WRITE16_MEMBER(atarigen_state::atarivc_playfield_upper_w)
{
	m_atarivc_playfield_tilemap->write_ext(space, offset, data, mem_mask);
}



//-------------------------------------------------
//  playfield_dual_upper_w: Generic write handler for
//  upper word of split dual playfield RAM.
//-------------------------------------------------

WRITE16_MEMBER(atarigen_state::atarivc_playfield_dual_upper_w)
{
	m_atarivc_playfield_tilemap->write_ext(space, offset, data, mem_mask);
	m_atarivc_playfield2_tilemap->write_ext(space, offset, data, mem_mask);
}



//-------------------------------------------------
//  playfield_latched_lsb_w: Generic write handler for
//  lower word of playfield RAM with a latch in the LSB of the
//  upper word.
//-------------------------------------------------

WRITE16_MEMBER(atarigen_state::atarivc_playfield_latched_lsb_w)
{
	m_atarivc_playfield_tilemap->write(space, offset, data, mem_mask);
	if (m_atarivc_playfield_latch != -1)
		m_atarivc_playfield_tilemap->write_ext(space, offset, UINT16(m_atarivc_playfield_latch), UINT16(0x00ff));
}



//-------------------------------------------------
//  playfield_latched_msb_w: Generic write handler for
//  lower word of playfield RAM with a latch in the MSB of the
//  upper word.
//-------------------------------------------------

WRITE16_MEMBER(atarigen_state::atarivc_playfield_latched_msb_w)
{
	m_atarivc_playfield_tilemap->write(space, offset, data, mem_mask);
	if (m_atarivc_playfield_latch != -1)
		m_atarivc_playfield_tilemap->write_ext(space, offset, UINT16(m_atarivc_playfield_latch), UINT16(0xff00));
}



//-------------------------------------------------
//  playfield2_latched_msb_w: Generic write handler for
//  lower word of second playfield RAM with a latch in the MSB
//  of the upper word.
//-------------------------------------------------

WRITE16_MEMBER(atarigen_state::atarivc_playfield2_latched_msb_w)
{
	m_atarivc_playfield2_tilemap->write(space, offset, data, mem_mask);
	if (m_atarivc_playfield2_latch != -1)
		m_atarivc_playfield2_tilemap->write_ext(space, offset, UINT16(m_atarivc_playfield2_latch), UINT16(0xff00));
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


//-------------------------------------------------
//  paletteram_666_w: 6-6-6 RGB palette RAM handler.
//-------------------------------------------------

WRITE16_MEMBER(atarigen_state::paletteram_666_w)
{
	int newword, r, g, b;

	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	newword = m_generic_paletteram_16[offset];

	r = ((newword >> 9) & 0x3e) | ((newword >> 15) & 1);
	g = ((newword >> 4) & 0x3e) | ((newword >> 15) & 1);
	b = ((newword << 1) & 0x3e) | ((newword >> 15) & 1);

	palette_set_color_rgb(space.machine(), offset, pal6bit(r), pal6bit(g), pal6bit(b));
}


//-------------------------------------------------
//  expanded_paletteram_666_w: 6-6-6 RGB expanded
//  palette RAM handler.
//-------------------------------------------------

WRITE16_MEMBER(atarigen_state::expanded_paletteram_666_w)
{
	COMBINE_DATA(&m_generic_paletteram_16[offset]);

	if (ACCESSING_BITS_8_15)
	{
		int palentry = offset / 2;
		int newword = (m_generic_paletteram_16[palentry * 2] & 0xff00) | (m_generic_paletteram_16[palentry * 2 + 1] >> 8);

		int r, g, b;

		r = ((newword >> 9) & 0x3e) | ((newword >> 15) & 1);
		g = ((newword >> 4) & 0x3e) | ((newword >> 15) & 1);
		b = ((newword << 1) & 0x3e) | ((newword >> 15) & 1);

		palette_set_color_rgb(space.machine(), palentry & 0x1ff, pal6bit(r), pal6bit(g), pal6bit(b));
	}
}


//-------------------------------------------------
//  paletteram32_666_w: 6-6-6 RGB palette RAM handler.
//-------------------------------------------------

WRITE32_MEMBER(atarigen_state::paletteram32_666_w )
{
	int newword, r, g, b;

	COMBINE_DATA(&m_generic_paletteram_32[offset]);

	if (ACCESSING_BITS_16_31)
	{
		newword = m_generic_paletteram_32[offset] >> 16;

		r = ((newword >> 9) & 0x3e) | ((newword >> 15) & 1);
		g = ((newword >> 4) & 0x3e) | ((newword >> 15) & 1);
		b = ((newword << 1) & 0x3e) | ((newword >> 15) & 1);

		palette_set_color_rgb(space.machine(), offset * 2, pal6bit(r), pal6bit(g), pal6bit(b));
	}

	if (ACCESSING_BITS_0_15)
	{
		newword = m_generic_paletteram_32[offset] & 0xffff;

		r = ((newword >> 9) & 0x3e) | ((newword >> 15) & 1);
		g = ((newword >> 4) & 0x3e) | ((newword >> 15) & 1);
		b = ((newword << 1) & 0x3e) | ((newword >> 15) & 1);

		palette_set_color_rgb(space.machine(), offset * 2 + 1, pal6bit(r), pal6bit(g), pal6bit(b));
	}
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
	gfx_element *gx0 = machine().gfx[gfx0];
	gfx_element *gx1 = machine().gfx[gfx1];
	UINT8 *srcdata, *dest;
	int c, x, y;

	// allocate memory for the assembled data
	srcdata = auto_alloc_array(machine(), UINT8, gx0->elements() * gx0->width() * gx0->height());

	// loop over elements
	dest = srcdata;
	for (c = 0; c < gx0->elements(); c++)
	{
		const UINT8 *c0base = gx0->get_data(c);
		const UINT8 *c1base = gx1->get_data(c);

		// loop over height
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

//  int newdepth = gx0->depth() * gx1->depth();
	int granularity = gx0->granularity();
	gx0->set_raw_layout(srcdata, gx0->width(), gx0->height(), gx0->elements(), 8 * gx0->width(), 8 * gx0->width() * gx0->height());
	gx0->set_granularity(granularity);

	// free the second graphics element
	machine().gfx[gfx1] = NULL;
	auto_free(machine(), gx1);
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
