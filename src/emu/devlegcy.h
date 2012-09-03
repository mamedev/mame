/***************************************************************************

    devlegcy.h

    Legacy device helpers.

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

#ifndef __DEVLEGCY_H__
#define __DEVLEGCY_H__

#include <stddef.h>

//**************************************************************************
//  MACROS
//**************************************************************************

#define DEVICE_START_NAME(name)		device_start_##name
#define DEVICE_START(name)			void DEVICE_START_NAME(name)(device_t *device)
#define DEVICE_START_CALL(name)		DEVICE_START_NAME(name)(device)

#define DEVICE_STOP_NAME(name)		device_stop_##name
#define DEVICE_STOP(name)			void DEVICE_STOP_NAME(name)(device_t *device)
#define DEVICE_STOP_CALL(name)		DEVICE_STOP_NAME(name)(device)

#define DEVICE_RESET_NAME(name)		device_reset_##name
#define DEVICE_RESET(name)			void DEVICE_RESET_NAME(name)(device_t *device)
#define DEVICE_RESET_CALL(name)		DEVICE_RESET_NAME(name)(device)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_t;

typedef void (*device_start_func)(device_t *device);

#endif	/* __DEVLEGCY_H__ */
