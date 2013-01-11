/***************************************************************************

    osdepend.c

    OS-dependent code interface.

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

*******************************************************************c********/

#include "osdepend.h"


//-------------------------------------------------
//  osd_interface - constructor
//-------------------------------------------------

osd_interface::osd_interface()
	: m_machine(NULL)
{
}


//-------------------------------------------------
//  osd_interface - destructor
//-------------------------------------------------

osd_interface::~osd_interface()
{
}


//-------------------------------------------------
//  init - initialize the OSD system.
//-------------------------------------------------

void osd_interface::init(running_machine &machine)
{
	//
	// This function is responsible for initializing the OSD-specific
	// video and input functionality, and registering that functionality
	// with the MAME core.
	//
	// In terms of video, this function is expected to create one or more
	// render_targets that will be used by the MAME core to provide graphics
	// data to the system. Although it is possible to do this later, the
	// assumption in the MAME core is that the user interface will be
	// visible starting at init() time, so you will have some work to
	// do to avoid these assumptions.
	//
	// In terms of input, this function is expected to enumerate all input
	// devices available and describe them to the MAME core by adding
	// input devices and their attached items (buttons/axes) via the input
	// system.
	//
	// Beyond these core responsibilities, init() should also initialize
	// any other OSD systems that require information about the current
	// running_machine.
	//
	// This callback is also the last opportunity to adjust the options
	// before they are consumed by the rest of the core.
	//
	// Future work/changes:
	//
	// Audio initialization may eventually move into here as well,
	// instead of relying on independent callbacks from each system.
	//

	m_machine = &machine;
}


//-------------------------------------------------
//  update - periodic system update
//-------------------------------------------------

void osd_interface::update(bool skip_redraw)
{
	//
	// This method is called periodically to flush video updates to the
	// screen, and also to allow the OSD a chance to update other systems
	// on a regular basis. In general this will be called at the frame
	// rate of the system being run; however, it may be called at more
	// irregular intervals in some circumstances (e.g., multi-screen games
	// or games with asynchronous updates).
	//
}


//-------------------------------------------------
//  init_debugger - perform debugger-specific
//  initialization
//-------------------------------------------------

void osd_interface::init_debugger()
{
	//
	// Unlike init() above, this method is only called if the debugger
	// is active. This gives any OSD debugger interface a chance to
	// create all of its structures.
	//
}


//-------------------------------------------------
//  wait_for_debugger - wait for a debugger
//  command to be processed
//-------------------------------------------------

void osd_interface::wait_for_debugger(device_t &device, bool firststop)
{
	//
	// When implementing an OSD-driver debugger, this method should be
	// overridden to wait for input, process it, and return. It will be
	// called repeatedly until a command is issued that resumes
	// execution.
	//
}


//-------------------------------------------------
//  update_audio_stream - update the stereo audio
//  stream
//-------------------------------------------------

void osd_interface::update_audio_stream(const INT16 *buffer, int samples_this_frame)
{
	//
	// This method is called whenever the system has new audio data to stream.
	// It provides an array of stereo samples in L-R order which should be
	// output at the configured sample_rate.
	//
}


//-------------------------------------------------
//  set_mastervolume - set the system volume
//-------------------------------------------------

void osd_interface::set_mastervolume(int attenuation)
{
	//
	// Attenuation is the attenuation in dB (a negative number).
	// To convert from dB to a linear volume scale do the following:
	//    volume = MAX_VOLUME;
	//    while (attenuation++ < 0)
	//       volume /= 1.122018454;      //  = (10 ^ (1/20)) = 1dB
	//
}


//-------------------------------------------------
//  customize_input_type_list - provide OSD
//  additions/modifications to the input list
//-------------------------------------------------

void osd_interface::customize_input_type_list(simple_list<input_type_entry> &typelist)
{
	//
	// inptport.c defines some general purpose defaults for key and joystick bindings.
	// They may be further adjusted by the OS dependent code to better match the
	// available keyboard, e.g. one could map pause to the Pause key instead of P, or
	// snapshot to PrtScr instead of F12. Of course the user can further change the
	// settings to anything he/she likes.
	//
	// This function is called on startup, before reading the configuration from disk.
	// Scan the list, and change the keys/joysticks you want.
	//
}


//-------------------------------------------------
//  font_open - attempt to "open" a handle to the
//  font with the given name
//-------------------------------------------------

osd_font osd_interface::font_open(const char *name, int &height)
{
	return NULL;
}


//-------------------------------------------------
//  font_close - release resources associated with
//  a given OSD font
//-------------------------------------------------

void osd_interface::font_close(osd_font font)
{
}


//-------------------------------------------------
//  font_get_bitmap - allocate and populate a
//  BITMAP_FORMAT_ARGB32 bitmap containing the
//  pixel values MAKE_ARGB(0xff,0xff,0xff,0xff)
//  or MAKE_ARGB(0x00,0xff,0xff,0xff) for each
//  pixel of a black & white font
//-------------------------------------------------

bool osd_interface::font_get_bitmap(osd_font font, unicode_char chnum, bitmap_argb32 &bitmap, INT32 &width, INT32 &xoffs, INT32 &yoffs)
{
	return false;
}

//-------------------------------------------------
//  get_slider_list - allocate and populate a
//  list of OS-dependent slider values.
//-------------------------------------------------
void *osd_interface::get_slider_list()
{
	return NULL;
}
