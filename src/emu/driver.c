/***************************************************************************

    driver.c

    Core driver device base class.

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
#include "drivenum.h"


//**************************************************************************
//  ADDRESS_MAPS
//**************************************************************************

// default address map
static ADDRESS_MAP_START( generic, AS_0, 8, driver_device )
	AM_RANGE(0x00000000, 0xffffffff) AM_DEVREADWRITE(":", driver_device, fatal_generic_read, fatal_generic_write)
ADDRESS_MAP_END



//**************************************************************************
//  DRIVER DEVICE
//**************************************************************************

//-------------------------------------------------
//  driver_device - constructor
//-------------------------------------------------

driver_device::driver_device(const machine_config &mconfig, device_type type, const char *tag)
	: device_t(mconfig, type, "Driver Device", tag, NULL, 0),
	  device_memory_interface(mconfig, *this),
	  m_generic_paletteram_8(*this, "paletteram"),
	  m_generic_paletteram2_8(*this, "paletteram2"),
	  m_generic_paletteram_16(*this, "paletteram"),
	  m_generic_paletteram2_16(*this, "paletteram2"),
	  m_generic_paletteram_32(*this, "paletteram"),
	  m_generic_paletteram2_32(*this, "paletteram2"),
	  m_space_config("generic", ENDIANNESS_LITTLE, 8, 32, 0, NULL, *ADDRESS_MAP_NAME(generic)),
	  m_system(NULL),
	  m_latch_clear_value(0),
	  m_flip_screen_x(0),
	  m_flip_screen_y(0)
{
	memset(m_legacy_callbacks, 0, sizeof(m_legacy_callbacks));
	memset(m_latched_value, 0, sizeof(m_latched_value));
	memset(m_latch_read, 0, sizeof(m_latch_read));
}


//-------------------------------------------------
//  driver_device - destructor
//-------------------------------------------------

driver_device::~driver_device()
{
}


//-------------------------------------------------
//  static_set_game - set the game in the device
//  configuration
//-------------------------------------------------

void driver_device::static_set_game(device_t &device, const game_driver &game)
{
	driver_device &driver = downcast<driver_device &>(device);

	// set the system
	driver.m_system = &game;

	// set the short name to the game's name
	driver.m_shortname = game.name;

	// set the full name to the game's description
	driver.m_name = game.description;

	// and set the search path to include all parents
	driver.m_searchpath = game.name;
	for (int parent = driver_list::clone(game); parent != -1; parent = driver_list::clone(parent))
		driver.m_searchpath.cat(";").cat(driver_list::driver(parent).name);
}


//-------------------------------------------------
//  static_set_callback - set the a callback in
//  the device configuration
//-------------------------------------------------

void driver_device::static_set_callback(device_t &device, callback_type type, legacy_callback_func callback)
{
	downcast<driver_device &>(device).m_legacy_callbacks[type] = callback;
}

void driver_device::static_set_callback(device_t &device, callback_type type, driver_callback_delegate callback)
{
	downcast<driver_device &>(device).m_callbacks[type] = callback;
}


//-------------------------------------------------
//  driver_start - default implementation which
//  does nothing
//-------------------------------------------------

void driver_device::driver_start()
{
}


//-------------------------------------------------
//  machine_start - default implementation which
//  calls to the legacy machine_start function
//-------------------------------------------------

void driver_device::machine_start()
{
}


//-------------------------------------------------
//  sound_start - default implementation which
//  calls to the legacy sound_start function
//-------------------------------------------------

void driver_device::sound_start()
{
}


//-------------------------------------------------
//  palette_init - default implementation which
//  does nothing
//-------------------------------------------------

void driver_device::palette_init()
{
}


//-------------------------------------------------
//  video_start - default implementation which
//  calls to the legacy video_start function
//-------------------------------------------------

void driver_device::video_start()
{
}


//-------------------------------------------------
//  driver_reset - default implementation which
//  does nothing
//-------------------------------------------------

void driver_device::driver_reset()
{
}


//-------------------------------------------------
//  machine_reset - default implementation which
//  calls to the legacy machine_reset function
//-------------------------------------------------

void driver_device::machine_reset()
{
}


//-------------------------------------------------
//  sound_reset - default implementation which
//  calls to the legacy sound_reset function
//-------------------------------------------------

void driver_device::sound_reset()
{
}


//-------------------------------------------------
//  video_reset - default implementation which
//  calls to the legacy video_reset function
//-------------------------------------------------

void driver_device::video_reset()
{
}


//-------------------------------------------------
//  device_rom_region - return a pointer to the
//  game's ROMs
//-------------------------------------------------

const rom_entry *driver_device::device_rom_region() const
{
	return m_system->rom;
}


//-------------------------------------------------
//  device_input_ports - return a pointer to the
//  game's input ports
//-------------------------------------------------

ioport_constructor driver_device::device_input_ports() const
{
	return m_system->ipt;
}


//-------------------------------------------------
//  device_start - device override which calls
//  the various helpers
//-------------------------------------------------

void driver_device::device_start()
{
	// bind our legacy callbacks
	for (int index = 0; index < ARRAY_LENGTH(m_legacy_callbacks); index++)
		if (m_legacy_callbacks[index] != NULL)
			m_callbacks[index] = driver_callback_delegate(m_legacy_callbacks[index], "legacy_callback", &machine());

	// reschedule ourselves to be last
	device_iterator iter(*this);
	for (device_t *test = iter.first(); test != NULL; test = iter.next())
		if (test != this && !test->started())
			throw device_missing_dependencies();

	// call the game-specific init
	if (m_system->driver_init != NULL)
		(*m_system->driver_init)(machine());

	// finish image devices init process
	image_postdevice_init(machine());

	// call palette_init if present
	if (!m_callbacks[CB_PALETTE_INIT].isnull())
		m_callbacks[CB_PALETTE_INIT]();
	else
		palette_init();

	// start the various pieces
	driver_start();

	if (!m_callbacks[CB_MACHINE_START].isnull())
		m_callbacks[CB_MACHINE_START]();
	else
		machine_start();

	if (!m_callbacks[CB_SOUND_START].isnull())
		m_callbacks[CB_SOUND_START]();
	else
		sound_start();

	if (!m_callbacks[CB_VIDEO_START].isnull())
		m_callbacks[CB_VIDEO_START]();
	else
		video_start();

	// save generic states
	save_item(NAME(m_flip_screen_x));
	save_item(NAME(m_flip_screen_y));
}


//-------------------------------------------------
//  device_reset_after_children - device override
//  which calls the various helpers; must happen
//  after all child devices are reset
//-------------------------------------------------

void driver_device::device_reset_after_children()
{
	// reset each piece
	driver_reset();

	if (!m_callbacks[CB_MACHINE_RESET].isnull())
		m_callbacks[CB_MACHINE_RESET]();
	else
		machine_reset();

	if (!m_callbacks[CB_SOUND_RESET].isnull())
		m_callbacks[CB_SOUND_RESET]();
	else
		sound_reset();

	if (!m_callbacks[CB_VIDEO_RESET].isnull())
		m_callbacks[CB_VIDEO_RESET]();
	else
		video_reset();
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *driver_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == 0) ? &m_space_config : NULL;
}



//**************************************************************************
//  INTERRUPT ENABLE AND VECTOR HELPERS
//**************************************************************************

//-------------------------------------------------
//  irq_pulse_clear - clear a "pulsed" IRQ line
//-------------------------------------------------

void driver_device::irq_pulse_clear(void *ptr, INT32 param)
{
	device_execute_interface *exec = reinterpret_cast<device_execute_interface *>(ptr);
	int irqline = param;
	exec->set_input_line(irqline, CLEAR_LINE);
}


//-------------------------------------------------
//  generic_pulse_irq_line - "pulse" an IRQ line by
//  asserting it and then clearing it x cycle(s)
//  later
//-------------------------------------------------

void driver_device::generic_pulse_irq_line(device_execute_interface &exec, int irqline, int cycles)
{
	assert(irqline != INPUT_LINE_NMI && irqline != INPUT_LINE_RESET && cycles > 0);
	exec.set_input_line(irqline, ASSERT_LINE);

	attotime target_time = exec.local_time() + exec.cycles_to_attotime(cycles * exec.min_cycles());
	machine().scheduler().timer_set(target_time - machine().time(), timer_expired_delegate(FUNC(driver_device::irq_pulse_clear), this), irqline, (void *)&exec);
}


//-------------------------------------------------
//  generic_pulse_irq_line_and_vector - "pulse" an
//  IRQ line by asserting it and then clearing it
//  x cycle(s) later, specifying a vector
//-------------------------------------------------

void driver_device::generic_pulse_irq_line_and_vector(device_execute_interface &exec, int irqline, int vector, int cycles)
{
	assert(irqline != INPUT_LINE_NMI && irqline != INPUT_LINE_RESET && cycles > 0);
	exec.set_input_line_and_vector(irqline, ASSERT_LINE, vector);

	attotime target_time = exec.local_time() + exec.cycles_to_attotime(cycles * exec.min_cycles());
	machine().scheduler().timer_set(target_time - machine().time(), timer_expired_delegate(FUNC(driver_device::irq_pulse_clear), this), irqline, (void *)&exec);
}



//**************************************************************************
//  INTERRUPT GENERATION CALLBACK HELPERS
//**************************************************************************

//-------------------------------------------------
//  NMI callbacks
//-------------------------------------------------

INTERRUPT_GEN_MEMBER( driver_device::nmi_line_pulse )	{ device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE); }
INTERRUPT_GEN_MEMBER( driver_device::nmi_line_assert )	{ device.execute().set_input_line(INPUT_LINE_NMI, ASSERT_LINE); }


//-------------------------------------------------
//  IRQn callbacks
//-------------------------------------------------

INTERRUPT_GEN_MEMBER( driver_device::irq0_line_hold )	{ device.execute().set_input_line(0, HOLD_LINE); }
INTERRUPT_GEN_MEMBER( driver_device::irq0_line_pulse )	{ generic_pulse_irq_line(device.execute(), 0, 1); }
INTERRUPT_GEN_MEMBER( driver_device::irq0_line_assert )	{ device.execute().set_input_line(0, ASSERT_LINE); }

INTERRUPT_GEN_MEMBER( driver_device::irq1_line_hold )	{ device.execute().set_input_line(1, HOLD_LINE); }
INTERRUPT_GEN_MEMBER( driver_device::irq1_line_pulse )	{ generic_pulse_irq_line(device.execute(), 1, 1); }
INTERRUPT_GEN_MEMBER( driver_device::irq1_line_assert )	{ device.execute().set_input_line(1, ASSERT_LINE); }

INTERRUPT_GEN_MEMBER( driver_device::irq2_line_hold )	{ device.execute().set_input_line(2, HOLD_LINE); }
INTERRUPT_GEN_MEMBER( driver_device::irq2_line_pulse )	{ generic_pulse_irq_line(device.execute(), 2, 1); }
INTERRUPT_GEN_MEMBER( driver_device::irq2_line_assert )	{ device.execute().set_input_line(2, ASSERT_LINE); }

INTERRUPT_GEN_MEMBER( driver_device::irq3_line_hold )	{ device.execute().set_input_line(3, HOLD_LINE); }
INTERRUPT_GEN_MEMBER( driver_device::irq3_line_pulse )	{ generic_pulse_irq_line(device.execute(), 3, 1); }
INTERRUPT_GEN_MEMBER( driver_device::irq3_line_assert )	{ device.execute().set_input_line(3, ASSERT_LINE); }

INTERRUPT_GEN_MEMBER( driver_device::irq4_line_hold )	{ device.execute().set_input_line(4, HOLD_LINE); }
INTERRUPT_GEN_MEMBER( driver_device::irq4_line_pulse )	{ generic_pulse_irq_line(device.execute(), 4, 1); }
INTERRUPT_GEN_MEMBER( driver_device::irq4_line_assert )	{ device.execute().set_input_line(4, ASSERT_LINE); }

INTERRUPT_GEN_MEMBER( driver_device::irq5_line_hold )	{ device.execute().set_input_line(5, HOLD_LINE); }
INTERRUPT_GEN_MEMBER( driver_device::irq5_line_pulse )	{ generic_pulse_irq_line(device.execute(), 5, 1); }
INTERRUPT_GEN_MEMBER( driver_device::irq5_line_assert )	{ device.execute().set_input_line(5, ASSERT_LINE); }

INTERRUPT_GEN_MEMBER( driver_device::irq6_line_hold )	{ device.execute().set_input_line(6, HOLD_LINE); }
INTERRUPT_GEN_MEMBER( driver_device::irq6_line_pulse )	{ generic_pulse_irq_line(device.execute(), 6, 1); }
INTERRUPT_GEN_MEMBER( driver_device::irq6_line_assert )	{ device.execute().set_input_line(6, ASSERT_LINE); }

INTERRUPT_GEN_MEMBER( driver_device::irq7_line_hold )	{ device.execute().set_input_line(7, HOLD_LINE); }
INTERRUPT_GEN_MEMBER( driver_device::irq7_line_pulse )	{ generic_pulse_irq_line(device.execute(), 7, 1); }
INTERRUPT_GEN_MEMBER( driver_device::irq7_line_assert )	{ device.execute().set_input_line(7, ASSERT_LINE); }



//**************************************************************************
//  WATCHDOG READ/WRITE HELPERS
//**************************************************************************

//-------------------------------------------------
//  8-bit reset read/write handlers
//-------------------------------------------------

WRITE8_MEMBER( driver_device::watchdog_reset_w ) { machine().watchdog_reset(); }
READ8_MEMBER( driver_device::watchdog_reset_r ) { machine().watchdog_reset(); return space.unmap(); }


//-------------------------------------------------
//  16-bit reset read/write handlers
//-------------------------------------------------

WRITE16_MEMBER( driver_device::watchdog_reset16_w ) { machine().watchdog_reset(); }
READ16_MEMBER( driver_device::watchdog_reset16_r ) { machine().watchdog_reset(); return space.unmap(); }


//-------------------------------------------------
//  32-bit reset read/write handlers
//-------------------------------------------------

WRITE32_MEMBER( driver_device::watchdog_reset32_w ) { machine().watchdog_reset(); }
READ32_MEMBER( driver_device::watchdog_reset32_r ) { machine().watchdog_reset(); return space.unmap(); }



//**************************************************************************
//  GENERIC SOUND COMMAND LATCHING
//**************************************************************************

//-------------------------------------------------
//  soundlatch_sync_callback - time-delayed
//  callback to set a latch value
//-------------------------------------------------

void driver_device::soundlatch_sync_callback(void *ptr, INT32 param)
{
	UINT16 value = param >> 8;
	int which = param & 0xff;

	// if the latch hasn't been read and the value is changed, log a warning
	if (!m_latch_read[which] && m_latched_value[which] != value)
		logerror("Warning: sound latch %d written before being read. Previous: %02x, new: %02x\n", which, m_latched_value[which], value);

	// store the new value and mark it not read
	m_latched_value[which] = value;
	m_latch_read[which] = 0;
}


//-------------------------------------------------
//  soundlatch_byte_w - global write handlers for
//  writing to sound latches
//-------------------------------------------------

void driver_device::soundlatch_write(UINT8 index, UINT32 data) { machine().scheduler().synchronize(timer_expired_delegate(FUNC(driver_device::soundlatch_sync_callback), this), index | (data << 8)); }
WRITE8_MEMBER( driver_device::soundlatch_byte_w )	{ soundlatch_write(0, data); }
WRITE16_MEMBER( driver_device::soundlatch_word_w )	{ soundlatch_write(0, data); }
WRITE8_MEMBER( driver_device::soundlatch2_byte_w )	{ soundlatch_write(1, data); }
WRITE16_MEMBER( driver_device::soundlatch2_word_w )	{ soundlatch_write(1, data); }
WRITE8_MEMBER( driver_device::soundlatch3_byte_w )	{ soundlatch_write(2, data); }
WRITE16_MEMBER( driver_device::soundlatch3_word_w )	{ soundlatch_write(2, data); }
WRITE8_MEMBER( driver_device::soundlatch4_byte_w )	{ soundlatch_write(3, data); }
WRITE16_MEMBER( driver_device::soundlatch4_word_w )	{ soundlatch_write(3, data); }


//-------------------------------------------------
//  soundlatch_byte_r - global read handlers for
//  reading from sound latches
//-------------------------------------------------

UINT32 driver_device::soundlatch_read(UINT8 index) { m_latch_read[index] = 1; return m_latched_value[index]; }
READ8_MEMBER( driver_device::soundlatch_byte_r )	{ return soundlatch_read(0); }
READ16_MEMBER( driver_device::soundlatch_word_r )	{ return soundlatch_read(0); }
READ8_MEMBER( driver_device::soundlatch2_byte_r )	{ return soundlatch_read(1); }
READ16_MEMBER( driver_device::soundlatch2_word_r )	{ return soundlatch_read(1); }
READ8_MEMBER( driver_device::soundlatch3_byte_r )	{ return soundlatch_read(2); }
READ16_MEMBER( driver_device::soundlatch3_word_r )	{ return soundlatch_read(2); }
READ8_MEMBER( driver_device::soundlatch4_byte_r )	{ return soundlatch_read(3); }
READ16_MEMBER( driver_device::soundlatch4_word_r )	{ return soundlatch_read(3); }


//-------------------------------------------------
//  soundlatch_clear_byte_w - global write handlers
//  for clearing sound latches
//-------------------------------------------------

void driver_device::soundlatch_clear(UINT8 index) { m_latched_value[index] = m_latch_clear_value; }
WRITE8_MEMBER( driver_device::soundlatch_clear_byte_w )  { soundlatch_clear(0); }
WRITE8_MEMBER( driver_device::soundlatch2_clear_byte_w ) { soundlatch_clear(1); }
WRITE8_MEMBER( driver_device::soundlatch3_clear_byte_w ) { soundlatch_clear(2); }
WRITE8_MEMBER( driver_device::soundlatch4_clear_byte_w ) { soundlatch_clear(3); }



//**************************************************************************
//  GENERIC FLIP SCREEN HANDLING
//**************************************************************************

//-------------------------------------------------
//  updateflip - handle global flipping
//-------------------------------------------------

void driver_device::updateflip()
{
	// push the flip state to all tilemaps
	machine().tilemap().set_flip_all((TILEMAP_FLIPX & m_flip_screen_x) | (TILEMAP_FLIPY & m_flip_screen_y));

	// flip the visible area within the screen width/height
	int width = machine().primary_screen->width();
	int height = machine().primary_screen->height();
	rectangle visarea = machine().primary_screen->visible_area();
	if (m_flip_screen_x)
	{
		int temp = width - visarea.min_x - 1;
		visarea.min_x = width - visarea.max_x - 1;
		visarea.max_x = temp;
	}
	if (m_flip_screen_y)
	{
		int temp = height - visarea.min_y - 1;
		visarea.min_y = height - visarea.max_y - 1;
		visarea.max_y = temp;
	}

	// reconfigure the screen with the new visible area
	attoseconds_t period = machine().primary_screen->frame_period().attoseconds;
	machine().primary_screen->configure(width, height, visarea, period);
}


//-------------------------------------------------
//  flip_screen_set - set global flip
//-------------------------------------------------

void driver_device::flip_screen_set(UINT32 on)
{
	// normalize to all 1
	if (on)
		on = ~0;

	// if something's changed, handle it
	if (m_flip_screen_x != on || m_flip_screen_y != on)
	{
		if (!on)
			updateflip(); // flip visarea back
		m_flip_screen_x = m_flip_screen_y = on;
		updateflip();
	}
}


//-------------------------------------------------
//  flip_screen_set_no_update - set global flip
//  do not call update_flip.
//-------------------------------------------------

void driver_device::flip_screen_set_no_update(UINT32 on)
{
	// flip_screen_y is not updated on purpose
    // this function is for drivers which
    // where writing to flip_screen_x to
    // bypass update_flip
	if (on)
		on = ~0;
	m_flip_screen_x = on;
}


//-------------------------------------------------
//  flip_screen_x_set - set global horizontal flip
//-------------------------------------------------

void driver_device::flip_screen_x_set(UINT32 on)
{
	// normalize to all 1
	if (on)
		on = ~0;

	// if something's changed, handle it
	if (m_flip_screen_x != on)
	{
		m_flip_screen_x = on;
		updateflip();
	}
}


//-------------------------------------------------
//  flip_screen_y_set - set global vertical flip
//-------------------------------------------------

void driver_device::flip_screen_y_set(UINT32 on)
{
	// normalize to all 1
	if (on)
		on = ~0;

	// if something's changed, handle it
	if (m_flip_screen_y != on)
	{
		m_flip_screen_y = on;
		updateflip();
	}
}



//**************************************************************************
//  8-BIT PALETTE WRITE HANDLERS
//**************************************************************************

// 3-3-2 RGB palette write handlers
WRITE8_MEMBER( driver_device::paletteram_BBGGGRRR_byte_w ) { palette_8bit_byte_w<3,3,2, 0,3,6>(space, offset, data, mem_mask); }
WRITE8_MEMBER( driver_device::paletteram_RRRGGGBB_byte_w ) { palette_8bit_byte_w<3,3,2, 5,2,0>(space, offset, data, mem_mask); }
WRITE8_MEMBER( driver_device::paletteram_BBGGRRII_byte_w )
{
	m_generic_paletteram_8[offset] = data;
	int i = (data >> 0) & 3;
	palette_set_color_rgb(machine(), offset, pal4bit(((data >> 0) & 0x0c) | i),
	                                   pal4bit(((data >> 2) & 0x0c) | i),
	                                   pal4bit(((data >> 4) & 0x0c) | i));
}



//**************************************************************************
//  16-BIT PALETTE WRITE HANDLERS
//**************************************************************************

// 4-4-4 RGB palette write handlers
WRITE8_MEMBER( driver_device::paletteram_xxxxBBBBGGGGRRRR_byte_le_w ) { palette_16bit_byte_le_w<4,4,4, 0,4,8>(space, offset, data, mem_mask); }
WRITE8_MEMBER( driver_device::paletteram_xxxxBBBBGGGGRRRR_byte_be_w ) { palette_16bit_byte_be_w<4,4,4, 0,4,8>(space, offset, data, mem_mask); }
WRITE8_MEMBER( driver_device::paletteram_xxxxBBBBGGGGRRRR_byte_split_lo_w ) { palette_16bit_byte_split_lo_w<4,4,4, 0,4,8>(space, offset, data, mem_mask); }
WRITE8_MEMBER( driver_device::paletteram_xxxxBBBBGGGGRRRR_byte_split_hi_w ) { palette_16bit_byte_split_hi_w<4,4,4, 0,4,8>(space, offset, data, mem_mask); }
WRITE16_MEMBER( driver_device::paletteram_xxxxBBBBGGGGRRRR_word_w ) { palette_16bit_word_w<4,4,4, 0,4,8>(space, offset, data, mem_mask); }

WRITE8_MEMBER( driver_device::paletteram_xxxxBBBBRRRRGGGG_byte_le_w ) { palette_16bit_byte_le_w<4,4,4, 4,0,8>(space, offset, data, mem_mask); }
WRITE8_MEMBER( driver_device::paletteram_xxxxBBBBRRRRGGGG_byte_be_w ) { palette_16bit_byte_be_w<4,4,4, 4,0,8>(space, offset, data, mem_mask); }
WRITE8_MEMBER( driver_device::paletteram_xxxxBBBBRRRRGGGG_byte_split_lo_w ) { palette_16bit_byte_split_lo_w<4,4,4, 4,0,8>(space, offset, data, mem_mask); }
WRITE8_MEMBER( driver_device::paletteram_xxxxBBBBRRRRGGGG_byte_split_hi_w ) { palette_16bit_byte_split_hi_w<4,4,4, 4,0,8>(space, offset, data, mem_mask); }
WRITE16_MEMBER( driver_device::paletteram_xxxxBBBBRRRRGGGG_word_w ) { palette_16bit_word_w<4,4,4, 4,0,8>(space, offset, data, mem_mask); }

WRITE8_MEMBER( driver_device::paletteram_xxxxRRRRBBBBGGGG_byte_split_lo_w ) { palette_16bit_byte_split_lo_w<4,4,4, 8,0,4>(space, offset, data, mem_mask); }
WRITE8_MEMBER( driver_device::paletteram_xxxxRRRRBBBBGGGG_byte_split_hi_w ) { palette_16bit_byte_split_hi_w<4,4,4, 8,0,4>(space, offset, data, mem_mask); }

WRITE8_MEMBER( driver_device::paletteram_xxxxRRRRGGGGBBBB_byte_le_w ) { palette_16bit_byte_le_w<4,4,4, 8,4,0>(space, offset, data, mem_mask); }
WRITE8_MEMBER( driver_device::paletteram_xxxxRRRRGGGGBBBB_byte_be_w ) { palette_16bit_byte_be_w<4,4,4, 8,4,0>(space, offset, data, mem_mask); }
WRITE8_MEMBER( driver_device::paletteram_xxxxRRRRGGGGBBBB_byte_split_lo_w ) { palette_16bit_byte_split_lo_w<4,4,4, 8,4,0>(space, offset, data, mem_mask); }
WRITE8_MEMBER( driver_device::paletteram_xxxxRRRRGGGGBBBB_byte_split_hi_w ) { palette_16bit_byte_split_hi_w<4,4,4, 8,4,0>(space, offset, data, mem_mask); }
WRITE16_MEMBER( driver_device::paletteram_xxxxRRRRGGGGBBBB_word_w ) { palette_16bit_word_w<4,4,4, 8,4,0>(space, offset, data, mem_mask); }

WRITE8_MEMBER( driver_device::paletteram_RRRRGGGGBBBBxxxx_byte_be_w ) { palette_16bit_byte_be_w<4,4,4, 12,8,4>(space, offset, data, mem_mask); }
WRITE8_MEMBER( driver_device::paletteram_RRRRGGGGBBBBxxxx_byte_split_lo_w ) { palette_16bit_byte_split_lo_w<4,4,4, 12,8,4>(space, offset, data, mem_mask); }
WRITE8_MEMBER( driver_device::paletteram_RRRRGGGGBBBBxxxx_byte_split_hi_w ) { palette_16bit_byte_split_hi_w<4,4,4, 12,8,4>(space, offset, data, mem_mask); }
WRITE16_MEMBER( driver_device::paletteram_RRRRGGGGBBBBxxxx_word_w ) { palette_16bit_word_w<4,4,4, 12,8,4>(space, offset, data, mem_mask); }

// 4-4-4-4 IRGB palette write handlers
template<int _IShift, int _RShift, int _GShift, int _BShift>
inline void set_color_irgb(running_machine &machine, pen_t color, UINT16 data)
{
	static const UINT8 ztable[16] = { 0x0, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x10, 0x11 };
	UINT8 i = ztable[(data >> _IShift) & 15];
	UINT8 r = ((data >> _RShift) & 15) * i;
	UINT8 g = ((data >> _GShift) & 15) * i;
	UINT8 b = ((data >> _BShift) & 15) * i;
	palette_set_color_rgb(machine, color, r, g, b);
}

WRITE16_MEMBER( driver_device::paletteram_IIIIRRRRGGGGBBBB_word_w )
{
	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	set_color_irgb<12,8,4,0>(machine(), offset, m_generic_paletteram_16[offset]);
}

WRITE16_MEMBER( driver_device::paletteram_RRRRGGGGBBBBIIII_word_w )
{
	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	set_color_irgb<0,12,8,4>(machine(), offset, m_generic_paletteram_16[offset]);
}

// 5-5-5 RGB palette write handlers
WRITE8_MEMBER( driver_device::paletteram_xBBBBBGGGGGRRRRR_byte_le_w ) { palette_16bit_byte_le_w<5,5,5, 0,5,10>(space, offset, data, mem_mask); }
WRITE8_MEMBER( driver_device::paletteram_xBBBBBGGGGGRRRRR_byte_be_w ) { palette_16bit_byte_be_w<5,5,5, 0,5,10>(space, offset, data, mem_mask); }
WRITE8_MEMBER( driver_device::paletteram_xBBBBBGGGGGRRRRR_byte_split_lo_w ) { palette_16bit_byte_split_lo_w<5,5,5, 0,5,10>(space, offset, data, mem_mask); }
WRITE8_MEMBER( driver_device::paletteram_xBBBBBGGGGGRRRRR_byte_split_hi_w ) { palette_16bit_byte_split_hi_w<5,5,5, 0,5,10>(space, offset, data, mem_mask); }
WRITE16_MEMBER( driver_device::paletteram_xBBBBBGGGGGRRRRR_word_w ) { palette_16bit_word_w<5,5,5, 0,5,10>(space, offset, data, mem_mask); }

WRITE8_MEMBER( driver_device::paletteram_xBBBBBRRRRRGGGGG_byte_split_lo_w ) { palette_16bit_byte_split_lo_w<5,5,5, 5,0,10>(space, offset, data, mem_mask); }
WRITE8_MEMBER( driver_device::paletteram_xBBBBBRRRRRGGGGG_byte_split_hi_w ) { palette_16bit_byte_split_hi_w<5,5,5, 5,0,10>(space, offset, data, mem_mask); }

WRITE8_MEMBER( driver_device::paletteram_xRRRRRGGGGGBBBBB_byte_le_w ) { palette_16bit_byte_le_w<5,5,5, 10,5,0>(space, offset, data, mem_mask); }
WRITE8_MEMBER( driver_device::paletteram_xRRRRRGGGGGBBBBB_byte_be_w ) { palette_16bit_byte_be_w<5,5,5, 10,5,0>(space, offset, data, mem_mask); }
WRITE8_MEMBER( driver_device::paletteram_xRRRRRGGGGGBBBBB_byte_split_lo_w ) { palette_16bit_byte_split_lo_w<5,5,5, 10,5,0>(space, offset, data, mem_mask); }
WRITE8_MEMBER( driver_device::paletteram_xRRRRRGGGGGBBBBB_byte_split_hi_w ) { palette_16bit_byte_split_hi_w<5,5,5, 10,5,0>(space, offset, data, mem_mask); }
WRITE16_MEMBER( driver_device::paletteram_xRRRRRGGGGGBBBBB_word_w ) { palette_16bit_word_w<5,5,5, 10,5,0>(space, offset, data, mem_mask); }
WRITE32_MEMBER( driver_device::paletteram_xRRRRRGGGGGBBBBB_dword_be_w ) { palette_16bit_dword_be_w<5,5,5, 10,5,0>(space, offset, data, mem_mask); }
WRITE32_MEMBER( driver_device::paletteram_xRRRRRGGGGGBBBBB_dword_le_w ) { palette_16bit_dword_le_w<5,5,5, 10,5,0>(space, offset, data, mem_mask); }

WRITE8_MEMBER( driver_device::paletteram_xGGGGGRRRRRBBBBB_byte_le_w ) { palette_16bit_byte_le_w<5,5,5, 5,10,0>(space, offset, data, mem_mask); }

WRITE16_MEMBER( driver_device::paletteram_xGGGGGRRRRRBBBBB_word_w ) { palette_16bit_word_w<5,5,5, 5,10,0>(space, offset, data, mem_mask); }
WRITE16_MEMBER( driver_device::paletteram_xGGGGGBBBBBRRRRR_word_w ) { palette_16bit_word_w<5,5,5, 0,10,5>(space, offset, data, mem_mask); }
WRITE16_MEMBER( driver_device::paletteram_RRRRRGGGGGBBBBBx_word_w ) { palette_16bit_word_w<5,5,5, 11,6,1>(space, offset, data, mem_mask); }
WRITE16_MEMBER( driver_device::paletteram_GGGGGRRRRRBBBBBx_word_w ) { palette_16bit_word_w<5,5,5, 6,11,1>(space, offset, data, mem_mask); }
WRITE16_MEMBER( driver_device::paletteram_RRRRGGGGBBBBRGBx_word_w )
{
	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	data = m_generic_paletteram_16[offset];
	palette_set_color_rgb(machine(), offset, pal5bit(((data >> 11) & 0x1e) | ((data >> 3) & 0x01)),
	                                    	pal5bit(((data >>  7) & 0x1e) | ((data >> 2) & 0x01)),
	                                    	pal5bit(((data >>  3) & 0x1e) | ((data >> 1) & 0x01)));
}


//**************************************************************************
//  32-BIT PALETTE WRITE HANDLERS
//**************************************************************************

// 8-8-8 RGB palette write handlers
WRITE16_MEMBER( driver_device::paletteram_xrgb_word_be_w ) { palette_32bit_word_be_w<8,8,8, 16,8,0>(space, offset, data, mem_mask); }
WRITE16_MEMBER( driver_device::paletteram_xbgr_word_be_w ) { palette_32bit_word_be_w<8,8,8, 0,8,16>(space, offset, data, mem_mask); }



//**************************************************************************
//  MISC READ/WRITE HANDLERS
//**************************************************************************

//-------------------------------------------------
//  generic space fatal error handlers
//-------------------------------------------------

READ8_MEMBER( driver_device::fatal_generic_read )
{
	throw emu_fatalerror("Attempted to read from generic address space (offs %X)\n", offset);
}

WRITE8_MEMBER( driver_device::fatal_generic_write )
{
	throw emu_fatalerror("Attempted to write to generic address space (offs %X = %02X)\n", offset, data);
}


