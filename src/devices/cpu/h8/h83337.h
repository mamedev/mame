// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h83337.h

    H8-3337 family emulation

    H8-300-based mcus.

    Variant         ROM        RAM
    H8/3334         32K         1K
    H8/3336         48K         2K
    H8/3337         60K         2K

    The 3394, 3396, and 3397 variants are the mask-rom versions.


***************************************************************************/

#ifndef __H83337_H__
#define __H83337_H__

#include "h8.h"
#include "h8_intc.h"
#include "h8_adc.h"
#include "h8_port.h"
#include "h8_timer8.h"
#include "h8_timer16.h"
#include "h8_sci.h"
#include "h8_watchdog.h"

class h83337_device : public h8_device {
public:
	h83337_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	h83337_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t wscr_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void wscr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t stcr_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void stcr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t syscr_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void syscr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mdcr_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mdcr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

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
	required_device<h8_watchdog_device> watchdog;

	uint8_t syscr;
	uint32_t ram_start;

	virtual void update_irq_filter() override;
	virtual void interrupt_taken() override;
	virtual void irq_setup() override;
	virtual void internal_update(uint64_t current_time) override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	DECLARE_ADDRESS_MAP(map, 16);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void execute_set_input(int inputnum, int state) override;
};

class h83334_device : public h83337_device {
public:
	h83334_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class h83336_device : public h83337_device {
public:
	h83336_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

extern const device_type H83334;
extern const device_type H83336;
extern const device_type H83337;

#endif
