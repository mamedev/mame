/***************************************************************************

    h83337.h

    H8-3337 family emulation

    H8-300-based mcus.

    Variant         ROM        RAM
    H8/3334         32K         1K
    H8/3336         48K         2K
    H8/3337         60K         2K

    The 3394, 3396, and 3397 variants are the mask-rom versions.

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

#ifndef __H83337_H__
#define __H83337_H__

#include "h8.h"
#include "h8_adc.h"
#include "h8_port.h"
#include "h8_intc.h"
#include "h8_timer8.h"
#include "h8_timer16.h"
#include "h8_sci.h"

class h83337_device : public h8_device {
public:
	h83337_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	h83337_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER(wscr_r);
	DECLARE_WRITE8_MEMBER(wscr_w);
	DECLARE_READ8_MEMBER(stcr_r);
	DECLARE_WRITE8_MEMBER(stcr_w);
	DECLARE_READ8_MEMBER(syscr_r);
	DECLARE_WRITE8_MEMBER(syscr_w);
	DECLARE_READ8_MEMBER(mdcr_r);
	DECLARE_WRITE8_MEMBER(mdcr_w);

protected:
	required_device<h8_intc_device> intc;
	required_device<h8_adc_device> adc;
	required_device<h8_port_device> port1;
	required_device<h8_port_device> port2;
	required_device<h8_port_device> port3;
	required_device<h8_port_device> port4;
	required_device<h8_port_device> port5;
	required_device<h8_port_device> port6;
	required_device<h8_port_device> port7;
	required_device<h8_port_device> port8;
	required_device<h8_port_device> port9;
	required_device<h8_timer8_channel_device> timer8_0;
	required_device<h8_timer8_channel_device> timer8_1;
	required_device<h8_timer16_device> timer16;
	required_device<h8_timer16_channel_device> timer16_0;
	required_device<h8_sci_device> sci0;
	required_device<h8_sci_device> sci1;

	UINT8 syscr;
	UINT32 ram_start;

	virtual void update_irq_filter();
	virtual void interrupt_taken();
	virtual void irq_setup();
	virtual void internal_update(UINT64 current_time);
	virtual machine_config_constructor device_mconfig_additions() const;
	DECLARE_ADDRESS_MAP(map, 16);

	virtual void device_start();
	virtual void device_reset();
	virtual void execute_set_input(int inputnum, int state);
};

class h83334_device : public h83337_device {
public:
	h83334_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class h83336_device : public h83337_device {
public:
	h83336_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type H83334;
extern const device_type H83336;
extern const device_type H83337;

#endif
