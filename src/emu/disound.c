/***************************************************************************

    disound.c

    Device sound interfaces.

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
#include "streams.h"



//**************************************************************************
//  DEVICE CONFIG SOUND INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_config_sound_interface - constructor
//-------------------------------------------------

device_config_sound_interface::device_config_sound_interface(const machine_config &mconfig, device_config &devconfig)
	: device_config_interface(mconfig, devconfig),
	  m_route_list(NULL)
{
}


//-------------------------------------------------
//  ~device_config_sound_interface - destructor
//-------------------------------------------------

device_config_sound_interface::~device_config_sound_interface()
{
	reset_routes();
}


//-------------------------------------------------
//  static_add_route - configuration helper to add
//  a new route to the device
//-------------------------------------------------

void device_config_sound_interface::static_add_route(device_config *device, UINT32 output, const char *target, double gain, UINT32 input)
{
	device_config_sound_interface *sound = dynamic_cast<device_config_sound_interface *>(device);
	if (sound == NULL)
		throw emu_fatalerror("MDRV_SOUND_ROUTE called on device '%s' with no sound interface", device->tag());

	sound_route **routeptr;
	for (routeptr = &sound->m_route_list; *routeptr != NULL; routeptr = &(*routeptr)->m_next) ;
	*routeptr = global_alloc(sound_route(output, input, gain, target));
}


//-------------------------------------------------
//  static_reset_routes - configuration helper to
//  reset all existing routes to the device
//-------------------------------------------------

void device_config_sound_interface::static_reset_routes(device_config *device)
{
	device_config_sound_interface *sound = dynamic_cast<device_config_sound_interface *>(device);
	if (sound == NULL)
		throw emu_fatalerror("MDRV_SOUND_ROUTES_RESET called on device '%s' with no sound interface", device->tag());

	sound->reset_routes();
}


//-------------------------------------------------
//  interface_validity_check - validation for a
//  device after the configuration has been
//  constructed
//-------------------------------------------------

bool device_config_sound_interface::interface_validity_check(const game_driver &driver) const
{
	bool error = false;

	// loop over all the routes
	for (const sound_route *route = m_route_list; route != NULL; route = route->m_next)
	{
		// find a device with the requested tag
		const device_config *target = m_machine_config.m_devicelist.find(route->m_target);
		if (target == NULL)
		{
			mame_printf_error("%s: %s attempting to route sound to non-existant device '%s'\n", driver.source_file, driver.name, route->m_target);
			error = true;
		}

		// if it's not a speaker or a sound device, error
		const device_config_sound_interface *sound;
		if (target->type() != SPEAKER && !target->interface(sound))
		{
			mame_printf_error("%s: %s attempting to route sound to a non-sound device '%s' (%s)\n", driver.source_file, driver.name, route->m_target, target->name());
			error = true;
		}
	}
	return error;
}


//-------------------------------------------------
//  reset_routes - free up all allocated routes
//  in this configuration
//-------------------------------------------------

void device_config_sound_interface::reset_routes()
{
	// loop until all are gone
	while (m_route_list != NULL)
	{
		sound_route *temp = m_route_list;
		m_route_list = temp->m_next;
		global_free(temp);
	}
}


//-------------------------------------------------
//  sound_route - constructor
//-------------------------------------------------

device_config_sound_interface::sound_route::sound_route(int output, int input, float gain, const char *target)
	: m_next(NULL),
	  m_output(output),
	  m_input(input),
	  m_gain(gain),
	  m_target(target)
{
}



//**************************************************************************
//  DEVICE CONFIG SOUND INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_sound_interface - constructor
//-------------------------------------------------

device_sound_interface::device_sound_interface(running_machine &machine, const device_config &config, device_t &device)
	: device_interface(machine, config, device),
	  m_outputs(0),
	  m_sound_config(dynamic_cast<const device_config_sound_interface &>(config))
{
}


//-------------------------------------------------
//  ~device_sound_interface - destructor
//-------------------------------------------------

device_sound_interface::~device_sound_interface()
{
}


//-------------------------------------------------
//  interface_post_start - verify that state was
//  properly set up
//-------------------------------------------------

void device_sound_interface::interface_post_start()
{
	// count the outputs
	for (int outputnum = 0; outputnum < MAX_OUTPUTS; outputnum++)
	{
		// stop when we run out of streams
		sound_stream *stream = stream_find_by_device(&m_device, outputnum);
		if (stream == NULL)
			break;

		// accumulate the number of outputs from this stream
		int numoutputs = stream_get_outputs(stream);
		assert(m_outputs + numoutputs < MAX_OUTPUTS);

		// fill in the array
		for (int curoutput = 0; curoutput < numoutputs; curoutput++)
		{
			sound_output *output = &m_output[m_outputs++];
			output->stream = stream;
			output->output = curoutput;
		}
	}
}
