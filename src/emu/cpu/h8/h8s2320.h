// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8s2320.h

    H8S-2320 family emulation

    H8S/2600-based mcus.

    Variant           ROM        RAM
    H8S/2320           -          4K
    H8S/2321           -          4K
    H8S/2322           -          8K
    H8S/2323          32K         8K
    H8S/2324           -         32K
    H8S/2326         512K         8K
    H8S/2327         128K         8K
    H8S/2328         256K         8K
    H8S/2329         384K        32K



***************************************************************************/

#ifndef __H8S2320_H__
#define __H8S2320_H__

#include "h8s2000.h"
#include "h8_adc.h"
#include "h8_port.h"
#include "h8_intc.h"
#include "h8_sci.h"
#include "h8_timer8.h"
#include "h8_timer16.h"

class h8s2320_device : public h8s2000_device {
public:
	h8s2320_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	h8s2320_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

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
	UINT8 syscr;

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

class h8s2321_device : public h8s2320_device {
public:
	h8s2321_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class h8s2322_device : public h8s2320_device {
public:
	h8s2322_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class h8s2323_device : public h8s2320_device {
public:
	h8s2323_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class h8s2324_device : public h8s2320_device {
public:
	h8s2324_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class h8s2326_device : public h8s2320_device {
public:
	h8s2326_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class h8s2327_device : public h8s2320_device {
public:
	h8s2327_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class h8s2328_device : public h8s2320_device {
public:
	h8s2328_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class h8s2329_device : public h8s2320_device {
public:
	h8s2329_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type H8S2320;
extern const device_type H8S2321;
extern const device_type H8S2322;
extern const device_type H8S2323;
extern const device_type H8S2324;
extern const device_type H8S2326;
extern const device_type H8S2327;
extern const device_type H8S2328;
extern const device_type H8S2329;


#endif
