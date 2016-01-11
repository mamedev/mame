// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    driver.c

    Core driver device base class.

***************************************************************************/

#include "emu.h"
#include "image.h"
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
	: device_t(mconfig, type, "Driver Device", tag, nullptr, 0, "", __FILE__),
		device_memory_interface(mconfig, *this),
		m_space_config("generic", ENDIANNESS_LITTLE, 8, 32, 0, nullptr, *ADDRESS_MAP_NAME(generic)),
		m_system(nullptr),
		m_latch_clear_value(0),
		m_flip_screen_x(0),
		m_flip_screen_y(0)
{
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
		driver.m_searchpath.append(";").append(driver_list::driver(parent).name);
}


//-------------------------------------------------
//  static_set_callback - set the a callback in
//  the device configuration
//-------------------------------------------------

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
	// reschedule ourselves to be last
	device_iterator iter(*this);
	for (device_t *test = iter.first(); test != nullptr; test = iter.next())
		if (test != this && !test->started())
			throw device_missing_dependencies();

	// call the game-specific init
	if (m_system->driver_init != nullptr)
		(*m_system->driver_init)(machine());

	// finish image devices init process
	machine().image().postdevice_init();

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
	save_item(NAME(m_latch_clear_value));
	save_item(NAME(m_latched_value));
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
	return (spacenum == 0) ? &m_space_config : nullptr;
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

INTERRUPT_GEN_MEMBER( driver_device::nmi_line_pulse )   { device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE); }
INTERRUPT_GEN_MEMBER( driver_device::nmi_line_assert )  { device.execute().set_input_line(INPUT_LINE_NMI, ASSERT_LINE); }


//-------------------------------------------------
//  IRQn callbacks
//-------------------------------------------------

INTERRUPT_GEN_MEMBER( driver_device::irq0_line_hold )   { device.execute().set_input_line(0, HOLD_LINE); }
INTERRUPT_GEN_MEMBER( driver_device::irq0_line_pulse )  { generic_pulse_irq_line(device.execute(), 0, 1); }
INTERRUPT_GEN_MEMBER( driver_device::irq0_line_assert ) { device.execute().set_input_line(0, ASSERT_LINE); }

INTERRUPT_GEN_MEMBER( driver_device::irq1_line_hold )   { device.execute().set_input_line(1, HOLD_LINE); }
INTERRUPT_GEN_MEMBER( driver_device::irq1_line_pulse )  { generic_pulse_irq_line(device.execute(), 1, 1); }
INTERRUPT_GEN_MEMBER( driver_device::irq1_line_assert ) { device.execute().set_input_line(1, ASSERT_LINE); }

INTERRUPT_GEN_MEMBER( driver_device::irq2_line_hold )   { device.execute().set_input_line(2, HOLD_LINE); }
INTERRUPT_GEN_MEMBER( driver_device::irq2_line_pulse )  { generic_pulse_irq_line(device.execute(), 2, 1); }
INTERRUPT_GEN_MEMBER( driver_device::irq2_line_assert ) { device.execute().set_input_line(2, ASSERT_LINE); }

INTERRUPT_GEN_MEMBER( driver_device::irq3_line_hold )   { device.execute().set_input_line(3, HOLD_LINE); }
INTERRUPT_GEN_MEMBER( driver_device::irq3_line_pulse )  { generic_pulse_irq_line(device.execute(), 3, 1); }
INTERRUPT_GEN_MEMBER( driver_device::irq3_line_assert ) { device.execute().set_input_line(3, ASSERT_LINE); }

INTERRUPT_GEN_MEMBER( driver_device::irq4_line_hold )   { device.execute().set_input_line(4, HOLD_LINE); }
INTERRUPT_GEN_MEMBER( driver_device::irq4_line_pulse )  { generic_pulse_irq_line(device.execute(), 4, 1); }
INTERRUPT_GEN_MEMBER( driver_device::irq4_line_assert ) { device.execute().set_input_line(4, ASSERT_LINE); }

INTERRUPT_GEN_MEMBER( driver_device::irq5_line_hold )   { device.execute().set_input_line(5, HOLD_LINE); }
INTERRUPT_GEN_MEMBER( driver_device::irq5_line_pulse )  { generic_pulse_irq_line(device.execute(), 5, 1); }
INTERRUPT_GEN_MEMBER( driver_device::irq5_line_assert ) { device.execute().set_input_line(5, ASSERT_LINE); }

INTERRUPT_GEN_MEMBER( driver_device::irq6_line_hold )   { device.execute().set_input_line(6, HOLD_LINE); }
INTERRUPT_GEN_MEMBER( driver_device::irq6_line_pulse )  { generic_pulse_irq_line(device.execute(), 6, 1); }
INTERRUPT_GEN_MEMBER( driver_device::irq6_line_assert ) { device.execute().set_input_line(6, ASSERT_LINE); }

INTERRUPT_GEN_MEMBER( driver_device::irq7_line_hold )   { device.execute().set_input_line(7, HOLD_LINE); }
INTERRUPT_GEN_MEMBER( driver_device::irq7_line_pulse )  { generic_pulse_irq_line(device.execute(), 7, 1); }
INTERRUPT_GEN_MEMBER( driver_device::irq7_line_assert ) { device.execute().set_input_line(7, ASSERT_LINE); }



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
WRITE8_MEMBER( driver_device::soundlatch_byte_w )   { soundlatch_write(0, data); }
WRITE16_MEMBER( driver_device::soundlatch_word_w )  { soundlatch_write(0, data); }
WRITE8_MEMBER( driver_device::soundlatch2_byte_w )  { soundlatch_write(1, data); }
WRITE16_MEMBER( driver_device::soundlatch2_word_w ) { soundlatch_write(1, data); }
WRITE8_MEMBER( driver_device::soundlatch3_byte_w )  { soundlatch_write(2, data); }
WRITE16_MEMBER( driver_device::soundlatch3_word_w ) { soundlatch_write(2, data); }
WRITE8_MEMBER( driver_device::soundlatch4_byte_w )  { soundlatch_write(3, data); }
WRITE16_MEMBER( driver_device::soundlatch4_word_w ) { soundlatch_write(3, data); }


//-------------------------------------------------
//  soundlatch_byte_r - global read handlers for
//  reading from sound latches
//-------------------------------------------------

UINT32 driver_device::soundlatch_read(UINT8 index) { m_latch_read[index] = 1; return m_latched_value[index]; }
READ8_MEMBER( driver_device::soundlatch_byte_r )    { return soundlatch_read(0); }
READ16_MEMBER( driver_device::soundlatch_word_r )   { return soundlatch_read(0); }
READ8_MEMBER( driver_device::soundlatch2_byte_r )   { return soundlatch_read(1); }
READ16_MEMBER( driver_device::soundlatch2_word_r )  { return soundlatch_read(1); }
READ8_MEMBER( driver_device::soundlatch3_byte_r )   { return soundlatch_read(2); }
READ16_MEMBER( driver_device::soundlatch3_word_r )  { return soundlatch_read(2); }
READ8_MEMBER( driver_device::soundlatch4_byte_r )   { return soundlatch_read(3); }
READ16_MEMBER( driver_device::soundlatch4_word_r )  { return soundlatch_read(3); }


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
		m_flip_screen_x = m_flip_screen_y = on;
		updateflip();
	}
}


//-------------------------------------------------
//  flip_screen_set_no_update - set global flip
//  do not call updateflip.
//-------------------------------------------------

void driver_device::flip_screen_set_no_update(UINT32 on)
{
	// flip_screen_y is not updated on purpose
	// this function is for drivers which
	// were writing to flip_screen_x to
	// bypass updateflip
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


/***************************************************************************
PORT READING HELPERS
***************************************************************************/

/*-------------------------------------------------
custom_port_read - act like input_port_read
but it is a custom port, it is useful for
e.g. input ports which expect the same port
repeated both in the upper and lower half
-------------------------------------------------*/

CUSTOM_INPUT_MEMBER(driver_device::custom_port_read)
{
	const char *tag = (const char *)param;
	return ioport(tag)->read();
}


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
