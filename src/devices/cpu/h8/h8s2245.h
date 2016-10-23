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
#include "h8_intc.h"
#include "h8_adc.h"
#include "h8_dtc.h"
#include "h8_port.h"
#include "h8_timer8.h"
#include "h8_timer16.h"
#include "h8_sci.h"
#include "h8_watchdog.h"

class h8s2245_device : public h8s2000_device {
public:
	h8s2245_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	h8s2245_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t syscr_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void syscr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t mstpcr_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void mstpcr_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

protected:
	required_device<h8s_intc_device> intc;
	required_device<h8_adc_device> adc;
	required_device<h8_dtc_device> dtc;
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
	required_device<h8_watchdog_device> watchdog;

	uint32_t ram_start;
	uint16_t mstpcr;
	uint8_t syscr;

	virtual bool exr_in_stack() const override;
	virtual void update_irq_filter() override;
	virtual void interrupt_taken() override;
	virtual int trapa_setup() override;
	virtual void irq_setup() override;
	virtual void internal_update(uint64_t current_time) override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	DECLARE_ADDRESS_MAP(map, 16);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void execute_set_input(int inputnum, int state) override;
};

class h8s2241_device : public h8s2245_device {
public:
	h8s2241_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class h8s2242_device : public h8s2245_device {
public:
	h8s2242_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class h8s2246_device : public h8s2245_device {
public:
	h8s2246_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

extern const device_type H8S2241;
extern const device_type H8S2242;
extern const device_type H8S2245;
extern const device_type H8S2246;

#endif
