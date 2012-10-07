/**********************************************************************

    Acorn FileStore E01/E01S network hard disk emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __E01__
#define __E01__

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "imagedev/flopdrv.h"
#include "machine/6522via.h"
#include "machine/ctronics.h"
#include "machine/econet.h"
#include "machine/mc146818.h"
#include "machine/mc6854.h"
#include "machine/ram.h"
#include "machine/scsicb.h"
#include "machine/wd17xx.h"

class e01_device : public device_t,
				   public device_econet_interface
{
public:
    // construction/destruction
	e01_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
    e01_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	enum
	{
		TYPE_E01 = 0,
		TYPE_E01S
	};

	DECLARE_READ8_MEMBER( ram_select_r );
	DECLARE_WRITE8_MEMBER( floppy_w );
	DECLARE_READ8_MEMBER( network_irq_disable_r );
	DECLARE_WRITE8_MEMBER( network_irq_disable_w );
	DECLARE_READ8_MEMBER( network_irq_enable_r );
	DECLARE_WRITE8_MEMBER( network_irq_enable_w );
	DECLARE_READ8_MEMBER( hdc_data_r );
	DECLARE_WRITE8_MEMBER( hdc_data_w );
	DECLARE_READ8_MEMBER( hdc_status_r );
	DECLARE_WRITE8_MEMBER( hdc_select_w );
	DECLARE_WRITE8_MEMBER( hdc_irq_enable_w );
	DECLARE_READ8_MEMBER( rtc_address_r );
	DECLARE_WRITE8_MEMBER( rtc_address_w );
	DECLARE_READ8_MEMBER( rtc_data_r );
	DECLARE_WRITE8_MEMBER( rtc_data_w );
	DECLARE_WRITE_LINE_MEMBER( rtc_irq_w );
	DECLARE_WRITE_LINE_MEMBER( adlc_irq_w );
	DECLARE_READ_LINE_MEMBER( econet_data_r );
	DECLARE_WRITE_LINE_MEMBER( econet_data_w );
	DECLARE_WRITE_LINE_MEMBER( via_irq_w );
	DECLARE_WRITE_LINE_MEMBER( clk_en_w );
	DECLARE_WRITE_LINE_MEMBER( fdc_irq_w );
	DECLARE_WRITE_LINE_MEMBER( fdc_drq_w );
	DECLARE_WRITE_LINE_MEMBER( scsi_bsy_w );
	DECLARE_WRITE_LINE_MEMBER( scsi_req_w );

protected:
    // device-level overrides
    virtual void device_config_complete();
    virtual void device_start();
    virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	// device_econet_interface overrides
	virtual void econet_clk(int state);

	required_device<cpu_device> m_maincpu;
	required_device<device_t> m_fdc;
	required_device<device_t> m_adlc;
	required_device<mc146818_device> m_rtc;
	required_device<ram_device> m_ram;
	required_device<scsicb_device> m_scsibus;

	inline void update_interrupts();
	inline void network_irq_enable(int enabled);
	inline void hdc_irq_enable(int enabled);

	// interrupt state
	int m_adlc_ie;
	int m_hdc_ie;
	int m_rtc_irq;
	int m_via_irq;
	int m_hdc_irq;
	int m_fdc_irq;
	int m_fdc_drq;
	int m_adlc_irq;

	int m_clk_en;

	int m_variant;

	// timers
	emu_timer *m_clk_timer;
};


// ======================> e01s_device

class e01s_device :  public e01_device
{
public:
    // construction/destruction
    e01s_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// device type definition
extern const device_type E01;
extern const device_type E01S;



#endif
