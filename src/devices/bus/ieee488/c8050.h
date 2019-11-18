// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 8050/8250/SFD-1001 Disk Drive emulation

**********************************************************************/

#ifndef MAME_BUS_IEE488_C8050_H
#define MAME_BUS_IEE488_C8050_H

#pragma once

#include "ieee488.h"
#include "c8050fdc.h"
#include "cpu/m6502/m6502.h"
#include "cpu/m6502/m6504.h"
#include "machine/6522via.h"
#include "machine/mos6530n.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c8050_device

class c8050_device : public device_t, public device_ieee488_interface
{
public:
	// construction/destruction
	c8050_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER( dio_r );
	DECLARE_WRITE8_MEMBER( dio_w );
	DECLARE_READ8_MEMBER( riot1_pa_r );
	DECLARE_WRITE8_MEMBER( riot1_pa_w );
	DECLARE_READ8_MEMBER( riot1_pb_r );
	DECLARE_WRITE8_MEMBER( riot1_pb_w );
	DECLARE_WRITE8_MEMBER( via_pb_w );

	void c8050_fdc_mem(address_map &map);
	void c8050_main_mem(address_map &map);
	void c8250lp_fdc_mem(address_map &map);
	void sfd1001_fdc_mem(address_map &map);
protected:
	c8050_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

	// device_ieee488_interface overrides
	virtual void ieee488_atn(int state) override;
	virtual void ieee488_ifc(int state) override;

	void add_common_devices(machine_config &config);
	inline void update_ieee_signals();

	required_device<m6502_device> m_maincpu;
	required_device<m6504_device> m_fdccpu;
	required_device<mos6532_new_device> m_riot0;
	required_device<mos6532_new_device> m_riot1;
	required_device<mos6530_new_device> m_miot;
	required_device<via6522_device> m_via;
	required_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;
	required_device<c8050_fdc_device> m_fdc;
	required_ioport m_address;
	output_finder<4> m_leds;

	// IEEE-488 bus
	int m_rfdo;                         // not ready for data output
	int m_daco;                         // not data accepted output
	int m_atna;                         // attention acknowledge
	int m_ifc;

private:
	DECLARE_FLOPPY_FORMATS( floppy_formats );
};


// ======================> c8250_device

class c8250_device : public c8050_device
{
public:
	// construction/destruction
	c8250_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

private:
	DECLARE_FLOPPY_FORMATS( floppy_formats );
};


// ======================> c8250lp_device

class c8250lp_device : public c8050_device
{
public:
	// construction/destruction
	c8250lp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	DECLARE_FLOPPY_FORMATS( floppy_formats );
};


// ======================> sfd1001_device

class sfd1001_device : public c8050_device
{
public:
	// construction/destruction
	sfd1001_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	DECLARE_FLOPPY_FORMATS( floppy_formats );
};


// device type definition
DECLARE_DEVICE_TYPE(C8050,   c8050_device)
DECLARE_DEVICE_TYPE(C8250,   c8250_device)
DECLARE_DEVICE_TYPE(C8250LP, c8250lp_device)
DECLARE_DEVICE_TYPE(SFD1001, sfd1001_device)


#endif // MAME_BUS_IEE488_C8050_H
