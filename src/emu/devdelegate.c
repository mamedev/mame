/***************************************************************************

    devdelegate.c

    Delegates that are late-bound to MAME devices.

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


//-------------------------------------------------
//  bound_object - use the device name to locate
//	a device relative to the given search root;
//	fatal error if not found
//-------------------------------------------------

delegate_late_bind &device_delegate_helper::bound_object(device_t &search_root)
{
	device_t *device = search_root.subdevice(m_device_name);
	if (device == NULL)
		throw emu_fatalerror("Unable to locate device '%s' relative to '%s'\n", m_device_name, search_root.tag());
	return *device;
}


//-------------------------------------------------
//  safe_tag - return a tag string or (unknown) if
//	the object is not valid
//-------------------------------------------------

const char *device_delegate_helper::safe_tag(device_t *object)
{
	return (object != NULL) ? object->tag() : "(unknown)";
}
