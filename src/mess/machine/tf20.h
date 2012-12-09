/**********************************************************************

    EPSON TF-20

    Dual 5.25" floppy drive with HX-20 factory option

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __TF20_H__
#define __TF20_H__

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/ram.h"
#include "machine/upd765.h"
#include "machine/upd7201.h"
#include "machine/epson_sio.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class epson_tf20_device : public device_t,
                          public device_epson_sio_interface
{
public:
	// construction/destruction
	epson_tf20_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	// not really public
	DECLARE_READ8_MEMBER( rom_disable_r );
	DECLARE_READ8_MEMBER( upd765_tc_r );
	DECLARE_WRITE8_MEMBER( fdc_control_w );
	static IRQ_CALLBACK( irq_callback );

	void fdc_irq(bool state);

protected:
	// device-level overrides
	virtual void device_config_complete() { m_shortname = "epson_tf20"; }
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// device_epson_sio_interface overrides
	virtual int rx_r();
	virtual int pin_r();
	virtual void tx_w(int level);
	virtual void pout_w(int level);

private:
	required_device<cpu_device> m_cpu;
	required_device<ram_device> m_ram;
	required_device<upd765a_device> m_fdc;
	required_device<upd7201_device> m_mpsc;
	required_device<epson_sio_device> m_sio;

	floppy_image_device *m_fd0;
	floppy_image_device *m_fd1;

	emu_timer *m_timer_serial;
	emu_timer *m_timer_tc;

	static const int XTAL_CR1 = XTAL_8MHz;
	static const int XTAL_CR2 = XTAL_4_9152MHz;
};


// device type definition
extern const device_type EPSON_TF20;


#endif // __TF20_H__
