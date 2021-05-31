// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h83032.h

    H8-3032 family emulation

    H8-300H-based mcus.

    Variant         ROM        RAM
    H8/3030         16K         512b
    H8/3031         32K         1K
    H8/3032         64K         2K


***************************************************************************/

#ifndef MAME_CPU_H8_H83032_H
#define MAME_CPU_H8_H83032_H

#include "h8h.h"
#include "h8_adc.h"
#include "h8_port.h"
#include "h8_intc.h"
#include "h8_timer16.h"
#include "h8_sci.h"
#include "h8_watchdog.h"

class h83032_device : public h8h_device {
public:
	h83032_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t syscr_r();
	void syscr_w(uint8_t data);

protected:
	h83032_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t start);

	required_device<h8h_intc_device> intc;
	required_device<h8_adc_device> adc;
	required_device<h8_port_device> port1;
	required_device<h8_port_device> port2;
	required_device<h8_port_device> port3;
	required_device<h8_port_device> port5;
	required_device<h8_port_device> port6;
	required_device<h8_port_device> port7;
	required_device<h8_port_device> port8;
	required_device<h8_port_device> port9;
	required_device<h8_port_device> porta;
	required_device<h8_port_device> portb;
	required_device<h8_port_device> portc;
	required_device<h8_timer16_device> timer16;
	required_device<h8h_timer16_channel_device> timer16_0;
	required_device<h8h_timer16_channel_device> timer16_1;
	required_device<h8h_timer16_channel_device> timer16_2;
	required_device<h8h_timer16_channel_device> timer16_3;
	required_device<h8h_timer16_channel_device> timer16_4;
	required_device<h8_sci_device> sci0;
	required_device<h8_watchdog_device> watchdog;

	uint32_t ram_start;
	uint8_t syscr;

	virtual void update_irq_filter() override;
	virtual void interrupt_taken() override;
	virtual int trapa_setup() override;
	virtual void irq_setup() override;
	virtual void internal_update(uint64_t current_time) override;
	virtual void device_add_mconfig(machine_config &config) override;
	void map(address_map &map);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void execute_set_input(int inputnum, int state) override;
};

class h83031_device : public h83032_device {
public:
	h83031_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class h83030_device : public h83032_device {
public:
	h83030_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(H83032, h83032_device)
DECLARE_DEVICE_TYPE(H83031, h83031_device)
DECLARE_DEVICE_TYPE(H83030, h83030_device)

#endif // MAME_CPU_H8_H83032_H
