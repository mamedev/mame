/***************************************************************************

    h8_port.h

    H8 8 bits digital port

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
    DISCLAIMED. IN NO EVENT SHALL OLIVIER GALIBERT BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#ifndef __H8_PORT_H__
#define __H8_PORT_H__

#include "h8.h"

#define MCFG_H8_PORT_ADD( _tag, address, ddr, mask )    \
	MCFG_DEVICE_ADD( _tag, H8_PORT, 0 ) \
	downcast<h8_port_device *>(device)->set_info(address, ddr, mask);

class h8_port_device : public device_t {
public:
	h8_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void set_info(int address, UINT8 default_ddr, UINT8 mask);

	DECLARE_WRITE8_MEMBER(ddr_w);
	DECLARE_WRITE8_MEMBER(dr_w);
	DECLARE_READ8_MEMBER(dr_r);
	DECLARE_READ8_MEMBER(port_r);
	DECLARE_WRITE8_MEMBER(pcr_w);
	DECLARE_READ8_MEMBER(pcr_r);
	DECLARE_WRITE8_MEMBER(odr_w);
	DECLARE_READ8_MEMBER(odr_r);

protected:
	required_device<h8_device> cpu;
	address_space *io;

	int address;
	UINT8 default_ddr, ddr, pcr, odr;
	UINT8 mask;
	UINT8 dr;
	UINT8 last_output;

	virtual void device_start();
	virtual void device_reset();
	void update_output();
};

extern const device_type H8_PORT;

#endif
