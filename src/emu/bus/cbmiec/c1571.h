// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 1570/1571/1571CR Single Disk Drive emulation

**********************************************************************/

#pragma once

#ifndef __C1571__
#define __C1571__

#include "emu.h"
#include "cbmiec.h"
#include "bus/c64/bn1541.h"
#include "cpu/m6502/m6502.h"
#include "machine/64h156.h"
#include "machine/6522via.h"
#include "bus/isa/isa.h"
#include "bus/isa/wd1002a_wx1.h"
#include "machine/mos6526.h"
#include "machine/wd_fdc.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define C1571_TAG           "c1571"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c1571_t

class c1571_t :  public device_t,
					public device_cbm_iec_interface,
					public device_c64_floppy_parallel_interface
{
public:
	// construction/destruction
	c1571_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	c1571_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	DECLARE_WRITE_LINE_MEMBER( via0_irq_w );
	DECLARE_READ8_MEMBER( via0_pa_r );
	DECLARE_WRITE8_MEMBER( via0_pa_w );
	DECLARE_READ8_MEMBER( via0_pb_r );
	DECLARE_WRITE8_MEMBER( via0_pb_w );

	DECLARE_READ8_MEMBER( via1_r );
	DECLARE_WRITE8_MEMBER( via1_w );
	DECLARE_WRITE_LINE_MEMBER( via1_irq_w );
	DECLARE_READ8_MEMBER( via1_pb_r );
	DECLARE_WRITE8_MEMBER( via1_pb_w );

	DECLARE_WRITE_LINE_MEMBER( cia_irq_w );
	DECLARE_WRITE_LINE_MEMBER( cia_pc_w );
	DECLARE_WRITE_LINE_MEMBER( cia_cnt_w );
	DECLARE_WRITE_LINE_MEMBER( cia_sp_w );
	DECLARE_READ8_MEMBER( cia_pb_r );
	DECLARE_WRITE8_MEMBER( cia_pb_w );

	DECLARE_WRITE_LINE_MEMBER( byte_w );

	DECLARE_WRITE_LINE_MEMBER( wpt_w );

	DECLARE_FLOPPY_FORMATS( floppy_formats );

	void wpt_callback(floppy_image_device *floppy, int state);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_cbm_iec_interface overrides
	virtual void cbm_iec_srq(int state);
	virtual void cbm_iec_atn(int state);
	virtual void cbm_iec_data(int state);
	virtual void cbm_iec_reset(int state);

	// device_c64_floppy_parallel_interface overrides
	virtual void parallel_data_w(UINT8 data);
	virtual void parallel_strobe_w(int state);

	enum
	{
		LED_POWER = 0,
		LED_ACT
	};

	void update_iec();

	required_device<cpu_device> m_maincpu;
	required_device<via6522_device> m_via0;
	required_device<via6522_device> m_via1;
	required_device<mos6526_device> m_cia;
	required_device<wd1770_t> m_fdc;
	required_device<c64h156_device> m_ga;
	required_device<floppy_image_device> m_floppy;
	required_ioport m_address;

	// signals
	int m_1_2mhz;                           // clock speed

	// IEC bus
	int m_data_out;                         // serial data out
	int m_ser_dir;                          // fast serial direction
	int m_sp_out;                           // fast serial data out
	int m_cnt_out;                          // fast serial clock out

	// interrupts
	int m_via0_irq;                         // VIA #0 interrupt request
	int m_via1_irq;                         // VIA #1 interrupt request
	int m_cia_irq;                          // CIA interrupt request
};


// ======================> c1570_t

class c1570_t :  public c1571_t
{
public:
	// construction/destruction
	c1570_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
};


// ======================> c1571cr_t

class c1571cr_t :  public c1571_t
{
public:
	// construction/destruction
	c1571cr_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

	DECLARE_WRITE8_MEMBER( via0_pa_w );
	DECLARE_WRITE8_MEMBER( via0_pb_w );
};


// ======================> mini_chief_t

class mini_chief_t :  public c1571_t
{
public:
	// construction/destruction
	mini_chief_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

	DECLARE_READ8_MEMBER( cia_pa_r );
	DECLARE_WRITE8_MEMBER( cia_pa_w );
	DECLARE_WRITE8_MEMBER( cia_pb_w );
};


// device type definition
extern const device_type C1570;
extern const device_type C1571;
extern const device_type C1571CR;
extern const device_type MINI_CHIEF;



#endif
