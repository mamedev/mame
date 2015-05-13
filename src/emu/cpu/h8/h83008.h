// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h83008.h

    H8/3008

    H8/300H-based mcu.


***************************************************************************/

#ifndef __H83008_H__
#define __H83008_H__

#include "h8h.h"
#include "h8_adc.h"
#include "h8_port.h"
#include "h8_intc.h"
#include "h8_sci.h"
#include "h8_timer8.h"
#include "h8_timer16.h"

class h83008_device : public h8h_device {
public:
	h83008_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	h83008_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER(syscr_r);
	DECLARE_WRITE8_MEMBER(syscr_w);

protected:
	required_device<h8h_intc_device> intc;
	required_device<h8_adc_device> adc;
	required_device<h8_port_device> port4;
	required_device<h8_port_device> port6;
	required_device<h8_port_device> port7;
	required_device<h8_port_device> port8;
	required_device<h8_port_device> port9;
	required_device<h8_port_device> porta;
	required_device<h8_port_device> portb;
	required_device<h8h_timer8_channel_device> timer8_0;
	required_device<h8h_timer8_channel_device> timer8_1;
	required_device<h8h_timer8_channel_device> timer8_2;
	required_device<h8h_timer8_channel_device> timer8_3;
	required_device<h8_timer16_device> timer16;
	required_device<h8h_timer16_channel_device> timer16_0;
	required_device<h8h_timer16_channel_device> timer16_1;
	required_device<h8h_timer16_channel_device> timer16_2;
	required_device<h8_sci_device> sci0;
	required_device<h8_sci_device> sci1;

	UINT8 syscr;

	virtual void update_irq_filter();
	virtual void interrupt_taken();
	virtual int trapa_setup();
	virtual void irq_setup();
	virtual void internal_update(UINT64 current_time);
	virtual machine_config_constructor device_mconfig_additions() const;
	DECLARE_ADDRESS_MAP(map, 16);

	virtual void device_start();
	virtual void device_reset();
	virtual void execute_set_input(int inputnum, int state);
};

extern const device_type H83008;

#endif
