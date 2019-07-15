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
//  DRIVER DEVICE
//**************************************************************************

//-------------------------------------------------
//  driver_device - constructor
//-------------------------------------------------

driver_device::driver_device(const machine_config &mconfig, device_type type, const char *tag)
	: device_t(mconfig, type, tag, nullptr, 0)
	, m_system(nullptr)
	, m_flip_screen_x(0)
	, m_flip_screen_y(0)
{
}


//-------------------------------------------------
//  driver_device - destructor
//-------------------------------------------------

driver_device::~driver_device()
{
}


//-------------------------------------------------
//  set_game_driver - set the game in the device
//  configuration
//-------------------------------------------------

void driver_device::set_game_driver(const game_driver &game)
{
	assert(!m_system);

	// set the system
	m_system = &game;

	// and set the search path to include all parents
	m_searchpath = game.name;
	std::set<game_driver const *> seen;
	for (int parent = driver_list::clone(game); parent != -1; parent = driver_list::clone(parent))
	{
		if (!seen.insert(&driver_list::driver(parent)).second)
			throw emu_fatalerror("driver_device::set_game_driver(%s): parent/clone relationships form a loop", game.name);
		m_searchpath.append(";").append(driver_list::driver(parent).name);
	}
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
//  empty_init - default implementation which
//  calls driver init
//-------------------------------------------------

void driver_device::empty_init()
{
	driver_init();
}


//-------------------------------------------------
//  driver_init - default implementation which
//  does nothing
//-------------------------------------------------

void driver_device::driver_init()
{
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

const tiny_rom_entry *driver_device::device_rom_region() const
{
	assert(m_system);
	return m_system->rom;
}


//-------------------------------------------------
//  device_add_mconfig - add machine configuration
//-------------------------------------------------

void driver_device::device_add_mconfig(machine_config &config)
{
	assert(m_system);
	m_system->machine_creator(config, *this);
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
	for (device_t &test : device_iterator(*this))
		if (&test != this && !test.started())
			throw device_missing_dependencies();

	// call the game-specific init
	m_system->driver_init(*this);

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



//**************************************************************************
//  INTERRUPT GENERATION CALLBACK HELPERS
//**************************************************************************

//-------------------------------------------------
//  NMI callbacks
//-------------------------------------------------

INTERRUPT_GEN_MEMBER( driver_device::nmi_line_pulse )   { device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero); }
INTERRUPT_GEN_MEMBER( driver_device::nmi_line_assert )  { device.execute().set_input_line(INPUT_LINE_NMI, ASSERT_LINE); }


//-------------------------------------------------
//  IRQn callbacks
//-------------------------------------------------

INTERRUPT_GEN_MEMBER( driver_device::irq0_line_hold )   { device.execute().set_input_line(0, HOLD_LINE); }
INTERRUPT_GEN_MEMBER( driver_device::irq0_line_assert ) { device.execute().set_input_line(0, ASSERT_LINE); }

INTERRUPT_GEN_MEMBER( driver_device::irq1_line_hold )   { device.execute().set_input_line(1, HOLD_LINE); }
INTERRUPT_GEN_MEMBER( driver_device::irq1_line_assert ) { device.execute().set_input_line(1, ASSERT_LINE); }

INTERRUPT_GEN_MEMBER( driver_device::irq2_line_hold )   { device.execute().set_input_line(2, HOLD_LINE); }
INTERRUPT_GEN_MEMBER( driver_device::irq2_line_assert ) { device.execute().set_input_line(2, ASSERT_LINE); }

INTERRUPT_GEN_MEMBER( driver_device::irq3_line_hold )   { device.execute().set_input_line(3, HOLD_LINE); }
INTERRUPT_GEN_MEMBER( driver_device::irq3_line_assert ) { device.execute().set_input_line(3, ASSERT_LINE); }

INTERRUPT_GEN_MEMBER( driver_device::irq4_line_hold )   { device.execute().set_input_line(4, HOLD_LINE); }
INTERRUPT_GEN_MEMBER( driver_device::irq4_line_assert ) { device.execute().set_input_line(4, ASSERT_LINE); }

INTERRUPT_GEN_MEMBER( driver_device::irq5_line_hold )   { device.execute().set_input_line(5, HOLD_LINE); }
INTERRUPT_GEN_MEMBER( driver_device::irq5_line_assert ) { device.execute().set_input_line(5, ASSERT_LINE); }

INTERRUPT_GEN_MEMBER( driver_device::irq6_line_hold )   { device.execute().set_input_line(6, HOLD_LINE); }
INTERRUPT_GEN_MEMBER( driver_device::irq6_line_assert ) { device.execute().set_input_line(6, ASSERT_LINE); }

INTERRUPT_GEN_MEMBER( driver_device::irq7_line_hold )   { device.execute().set_input_line(7, HOLD_LINE); }
INTERRUPT_GEN_MEMBER( driver_device::irq7_line_assert ) { device.execute().set_input_line(7, ASSERT_LINE); }


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

void driver_device::flip_screen_set(u32 on)
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
//  flip_screen_x_set - set global horizontal flip
//-------------------------------------------------

void driver_device::flip_screen_x_set(u32 on)
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

void driver_device::flip_screen_y_set(u32 on)
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
