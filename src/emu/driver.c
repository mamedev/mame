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
//  DRIVER DEVICE
//**************************************************************************

//-------------------------------------------------
//  driver_device - constructor
//-------------------------------------------------

driver_device::driver_device(const machine_config &mconfig, device_type type, const char *tag)
	: device_t(mconfig, type, "Driver Device", tag, NULL, 0),
	  m_system(NULL),
	  m_palette_init(NULL),
	  m_generic_paletteram_8(*this, "paletteram"),
	  m_generic_paletteram2_8(*this, "paletteram2"),
	  m_generic_paletteram_16(*this, "paletteram"),
	  m_generic_paletteram2_16(*this, "paletteram2"),
	  m_generic_paletteram_32(*this, "paletteram"),
	  m_generic_paletteram2_32(*this, "paletteram2")
{
	memset(m_callbacks, 0, sizeof(m_callbacks));
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
//  static_set_machine_start - set the legacy
//  machine start callback in the device
//  configuration
//-------------------------------------------------

void driver_device::static_set_callback(device_t &device, callback_type type, legacy_callback_func callback)
{
	downcast<driver_device &>(device).m_callbacks[type] = callback;
}


//-------------------------------------------------
//  static_set_palette_init - set the legacy
//  palette init callback in the device
//  configuration
//-------------------------------------------------

void driver_device::static_set_palette_init(device_t &device, palette_init_func callback)
{
	downcast<driver_device &>(device).m_palette_init = callback;
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
	if (m_callbacks[CB_MACHINE_START] != NULL)
		(*m_callbacks[CB_MACHINE_START])(machine());
}


//-------------------------------------------------
//  sound_start - default implementation which
//  calls to the legacy sound_start function
//-------------------------------------------------

void driver_device::sound_start()
{
	if (m_callbacks[CB_SOUND_START] != NULL)
		(*m_callbacks[CB_SOUND_START])(machine());
}


//-------------------------------------------------
//  video_start - default implementation which
//  calls to the legacy video_start function
//-------------------------------------------------

void driver_device::video_start()
{
	if (m_callbacks[CB_VIDEO_START] != NULL)
		(*m_callbacks[CB_VIDEO_START])(machine());
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
	if (m_callbacks[CB_MACHINE_RESET] != NULL)
		(*m_callbacks[CB_MACHINE_RESET])(machine());
}


//-------------------------------------------------
//  sound_reset - default implementation which
//  calls to the legacy sound_reset function
//-------------------------------------------------

void driver_device::sound_reset()
{
	if (m_callbacks[CB_SOUND_RESET] != NULL)
		(*m_callbacks[CB_SOUND_RESET])(machine());
}


//-------------------------------------------------
//  video_reset - default implementation which
//  calls to the legacy video_reset function
//-------------------------------------------------

void driver_device::video_reset()
{
	if (m_callbacks[CB_VIDEO_RESET] != NULL)
		(*m_callbacks[CB_VIDEO_RESET])(machine());
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
	for (device_t *test = iter.first(); test != NULL; test = iter.next())
		if (test != this && !test->started())
			throw device_missing_dependencies();

	// call the game-specific init
	if (m_system->driver_init != NULL)
		(*m_system->driver_init)(machine());

	// finish image devices init process
	image_postdevice_init(machine());

	// call palette_init if present
	if (m_palette_init != NULL)
		(*m_palette_init)(machine(), machine().region("proms")->base());

	// start the various pieces
	driver_start();
	machine_start();
	sound_start();
	video_start();
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
	machine_reset();
	sound_reset();
	video_reset();
}
