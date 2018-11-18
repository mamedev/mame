// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 2040/3040/4040 Disk Drive emulation

**********************************************************************/

#ifndef MAME_BUS_IEEE488_C2040_H
#define MAME_BUS_IEEE488_C2040_H

#pragma once

#include "ieee488.h"
#include "c2040fdc.h"
#include "cpu/m6502/m6502.h"
#include "cpu/m6502/m6504.h"
#include "machine/6522via.h"
#include "machine/mos6530n.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c2040_device

class c2040_device : public device_t, public device_ieee488_interface
{
public:
	// construction/destruction
	c2040_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER( dio_r );
	DECLARE_WRITE8_MEMBER( dio_w );
	DECLARE_READ8_MEMBER( riot1_pa_r );
	DECLARE_WRITE8_MEMBER( riot1_pa_w );
	DECLARE_READ8_MEMBER( riot1_pb_r );
	DECLARE_WRITE8_MEMBER( riot1_pb_w );
	DECLARE_WRITE8_MEMBER( via_pb_w );

	void c2040_fdc_mem(address_map &map);
	void c2040_main_mem(address_map &map);
protected:
	c2040_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

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

	enum
	{
		LED_POWER = 0,
		LED_ACT0,
		LED_ACT1,
		LED_ERR
	};

	void add_common_devices(machine_config &config);
	inline void update_ieee_signals();

	DECLARE_FLOPPY_FORMATS( floppy_formats );

	required_device<m6502_device> m_maincpu;
	required_device<m6504_device> m_fdccpu;
	required_device<mos6532_new_device> m_riot0;
	required_device<mos6532_new_device> m_riot1;
	required_device<mos6530_new_device> m_miot;
	required_device<via6522_device> m_via;
	required_device<floppy_image_device> m_floppy0;
	optional_device<floppy_image_device> m_floppy1;
	required_device<c2040_fdc_device> m_fdc;
	required_memory_region m_gcr;
	required_ioport m_address;
	output_finder<4> m_leds;

	// IEEE-488 bus
	int m_rfdo;                         // not ready for data output
	int m_daco;                         // not data accepted output
	int m_atna;                         // attention acknowledge
	int m_ifc;
};


// ======================> c3040_device

class c3040_device :  public c2040_device
{
public:
	// construction/destruction
	c3040_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	DECLARE_FLOPPY_FORMATS( floppy_formats );
};


// ======================> c4040_device

class c4040_device :  public c2040_device
{
public:
	// construction/destruction
	c4040_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	DECLARE_FLOPPY_FORMATS( floppy_formats );
};


// device type definition
DECLARE_DEVICE_TYPE(C2040, c2040_device)
DECLARE_DEVICE_TYPE(C3040, c3040_device)
DECLARE_DEVICE_TYPE(C4040, c4040_device)


#endif // MAME_BUS_IEEE488_C2040_H
