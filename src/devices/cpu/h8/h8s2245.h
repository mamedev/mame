// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8s2245.h

    H8S-2245 family emulation

    H8S/2000-based mcus.

    Variant           ROM        RAM
    H8S/2241          32K         4K
    H8S/2242          32K         8K
    H8S/2245         128K         4K
    H8S/2246         128K         8K



***************************************************************************/

#ifndef __H8S2245_H__
#define __H8S2245_H__

#include "h8s2000.h"
#include "h8_adc.h"
#include "h8_port.h"
#include "h8_intc.h"
#include "h8_sci.h"
#include "h8_timer8.h"
#include "h8_timer16.h"

class h8s2245_device : public h8s2000_device {
public:
	h8s2245_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);
	h8s2245_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

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
	required_device<h8_sci_device> sci0;
	required_device<h8_sci_device> sci1;
	required_device<h8_sci_device> sci2;

	UINT32 ram_start;
	UINT8 syscr;

	virtual bool exr_in_stack() const override;
	virtual void update_irq_filter() override;
	virtual void interrupt_taken() override;
	virtual int trapa_setup() override;
	virtual void irq_setup() override;
	virtual void internal_update(UINT64 current_time) override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	DECLARE_ADDRESS_MAP(map, 16);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void execute_set_input(int inputnum, int state) override;
};

class h8s2241_device : public h8s2245_device {
public:
	h8s2241_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

class h8s2242_device : public h8s2245_device {
public:
	h8s2242_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

class h8s2246_device : public h8s2245_device {
public:
	h8s2246_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

extern const device_type H8S2241;
extern const device_type H8S2242;
extern const device_type H8S2245;
extern const device_type H8S2246;

#endif
