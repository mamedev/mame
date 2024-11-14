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
#include "machine/mos6530.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c2040_device

class c2040_device : public device_t, public device_ieee488_interface
{
public:
	// construction/destruction
	c2040_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t dio_r();
	void dio_w(uint8_t data);
	uint8_t riot1_pa_r();
	void riot1_pa_w(uint8_t data);
	uint8_t riot1_pb_r();
	void riot1_pb_w(uint8_t data);
	void via_pb_w(uint8_t data);

	void c2040_fdc_mem(address_map &map) ATTR_COLD;
	void c2040_main_mem(address_map &map) ATTR_COLD;
protected:
	c2040_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

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

	static void floppy_formats(format_registration &fr);

	required_device<m6502_device> m_maincpu;
	required_device<m6504_device> m_fdccpu;
	required_device<mos6532_device> m_riot0;
	required_device<mos6532_device> m_riot1;
	required_device<mos6530_device> m_miot;
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
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	static void floppy_formats(format_registration &fr);
};


// ======================> c4040_device

class c4040_device :  public c2040_device
{
public:
	// construction/destruction
	c4040_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	static void floppy_formats(format_registration &fr);
};


// device type definition
DECLARE_DEVICE_TYPE(C2040, c2040_device)
DECLARE_DEVICE_TYPE(C3040, c3040_device)
DECLARE_DEVICE_TYPE(C4040, c4040_device)


#endif // MAME_BUS_IEEE488_C2040_H
