// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h83048.h

    H8-3048 family emulation

    H8-300H-based mcus.

    Variant         ROM        RAM
    H8/3044         32K         2K
    H8/3045         64K         2K
    H8/3047         96K         4K
    H8/3048        192K         4K

    The 3394, 3396, and 3997 variants are the mask-rom versions.


***************************************************************************/

#ifndef MAME_CPU_H8_H83048_H
#define MAME_CPU_H8_H83048_H

#include "h8h.h"
#include "h8_adc.h"
#include "h8_port.h"
#include "h8_intc.h"
#include "h8_timer16.h"
#include "h8_sci.h"
#include "h8_watchdog.h"

class h83048_device : public h8h_device {
public:
	h83048_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_mode_a20() { mode_a20 = true; }
	void set_mode_a24() { mode_a20 = false; }

	DECLARE_READ8_MEMBER(syscr_r);
	DECLARE_WRITE8_MEMBER(syscr_w);

protected:
	h83048_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t start);

	required_device<h8h_intc_device> intc;
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
	required_device<h8_port_device> porta;
	required_device<h8_port_device> portb;
	required_device<h8_timer16_device> timer16;
	required_device<h8h_timer16_channel_device> timer16_0;
	required_device<h8h_timer16_channel_device> timer16_1;
	required_device<h8h_timer16_channel_device> timer16_2;
	required_device<h8h_timer16_channel_device> timer16_3;
	required_device<h8h_timer16_channel_device> timer16_4;
	required_device<h8_sci_device> sci0;
	required_device<h8_sci_device> sci1;
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

class h83044_device : public h83048_device {
public:
	h83044_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class h83045_device : public h83048_device {
public:
	h83045_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class h83047_device : public h83048_device {
public:
	h83047_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(H83044, h83044_device)
DECLARE_DEVICE_TYPE(H83045, h83045_device)
DECLARE_DEVICE_TYPE(H83047, h83047_device)
DECLARE_DEVICE_TYPE(H83048, h83048_device)

#endif // MAME_CPU_H8_H83048_H
