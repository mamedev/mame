/***************************************************************************

    h8s2357.h

    H8S-2357 family emulation

    H8S/2000-based mcus.

    Variant           ROM        RAM
    H8S/2357         128K         8K
    H8S/2352         -            8K
    H8S/2398         256K         8K
    H8S/2394         -           32K
    H8S/2392         -            8K
    H8S/2390         -            4K


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

#ifndef __H8S2357_H__
#define __H8S2357_H__

#include "h8s2000.h"
#include "h8_adc.h"
#include "h8_port.h"
#include "h8_intc.h"
#include "h8_sci.h"
#include "h8_timer8.h"
#include "h8_timer16.h"

class h8s2357_device : public h8s2000_device {
public:
	h8s2357_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	h8s2357_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER(syscr_r);
	DECLARE_WRITE8_MEMBER(syscr_w);

protected:
	required_device<h8s_intc_device> intc;
	required_device<h8_adc_device> adc;
	required_device<h8_port_device> port1;
	required_device<h8_port_device> port2;
	required_device<h8_port_device> port3;
	required_device<h8_port_device> port4;
	required_device<h8_port_device> port5;
	required_device<h8_port_device> port6;
	required_device<h8_port_device> porta;
	required_device<h8_port_device> portb;
	required_device<h8_port_device> portc;
	required_device<h8_port_device> portd;
	required_device<h8_port_device> porte;
	required_device<h8_port_device> portf;
	required_device<h8_port_device> portg;
	required_device<h8h_timer8_channel_device> timer8_0;
	required_device<h8h_timer8_channel_device> timer8_1;
	required_device<h8_timer16_device> timer16;
	required_device<h8s_timer16_channel_device> timer16_0;
	required_device<h8s_timer16_channel_device> timer16_1;
	required_device<h8s_timer16_channel_device> timer16_2;
	required_device<h8s_timer16_channel_device> timer16_3;
	required_device<h8s_timer16_channel_device> timer16_4;
	required_device<h8s_timer16_channel_device> timer16_5;
	required_device<h8_sci_device> sci0;
	required_device<h8_sci_device> sci1;
	required_device<h8_sci_device> sci2;

	UINT32 ram_start;
	unsigned char syscr;

	virtual bool exr_in_stack() const;
	virtual void update_irq_filter();
	virtual void interrupt_taken();
	virtual int trace_setup();
	virtual int trapa_setup();
	virtual void irq_setup();
	virtual void internal_update(UINT64 current_time);
	virtual machine_config_constructor device_mconfig_additions() const;
	DECLARE_ADDRESS_MAP(map, 16);

	virtual void device_start();
	virtual void device_reset();
	virtual void execute_set_input(int inputnum, int state);
};

class h8s2352_device : public h8s2357_device {
public:
	h8s2352_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class h8s2398_device : public h8s2357_device {
public:
	h8s2398_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class h8s2394_device : public h8s2357_device {
public:
	h8s2394_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class h8s2392_device : public h8s2357_device {
public:
	h8s2392_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class h8s2390_device : public h8s2357_device {
public:
	h8s2390_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type H8S2357;
extern const device_type H8S2352;
extern const device_type H8S2398;
extern const device_type H8S2394;
extern const device_type H8S2392;
extern const device_type H8S2390;


#endif
