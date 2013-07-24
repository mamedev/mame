/***************************************************************************

    divideo.h

    Device video interfaces.

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

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __DIVIDEO_H__
#define __DIVIDEO_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_VIDEO_SET_SCREEN(_tag) \
	device_video_interface::static_set_screen(*device, _tag);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> device_video_interface

class device_video_interface : public device_interface
{
public:
	// construction/destruction
	device_video_interface(const machine_config &mconfig, device_t &device, bool screen_required = true);
	virtual ~device_video_interface();

	// static configuration
	static void static_set_screen(device_t &device, const char *tag);

	// getters
	screen_device &screen() const { return *m_screen; }

protected:
	// optional operation overrides
	virtual void interface_validity_check(validity_checker &valid) const;
	virtual void interface_pre_start();
	
	// configuration state
	bool 			m_screen_required;			// is a screen required?
	const char *	m_screen_tag;				// configured tag for the target screen

	// internal state
	screen_device *	m_screen;					// pointer to the screen device
};

// iterator
typedef device_interface_iterator<device_video_interface> video_interface_iterator;


#endif  /* __DIVIDEO_H__ */
