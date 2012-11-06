/***************************************************************************

    m8502.h

    6510 derivative, capable of running at 2MHz.

****************************************************************************

    Copyright Olivier Galibert
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

    THIS SOFTWARE IS PROVIDED BY OLIVIER GALIBERT ''AS IS'' AND ANY EXPRESS OR
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

#ifndef __M8502_H__
#define __M8502_H__

#include "m6510.h"

#define MCFG_M8502_PORT_CALLBACKS(_read, _write) \
	downcast<m8502_device *>(device)->set_callbacks(DEVCB2_##_read, DEVCB2_##_write);

#define MCFG_M8502_PORT_PULLS(_up, _down) \
	downcast<m8502_device *>(device)->set_pulls(_up, _down);

class m8502_device : public m6510_device {
public:
	m8502_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

enum {
	M8502_IRQ_LINE = m6502_device::IRQ_LINE,
	M8502_NMI_LINE = m6502_device::NMI_LINE,
};

extern const device_type M8502;

#endif
