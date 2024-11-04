// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 1581/1563 Single Disk Drive emulation

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_C1581_H
#define MAME_BUS_CBMIEC_C1581_H

#pragma once

#include "cbmiec.h"
#include "imagedev/floppy.h"
#include "machine/mos6526.h"
#include "machine/wd_fdc.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define C1581_TAG           "c1581"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c1581_device

class c1581_device : public device_t, public device_cbm_iec_interface
{
public:
	// construction/destruction
	c1581_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	c1581_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_cbm_iec_interface overrides
	virtual void cbm_iec_srq(int state) override;
	virtual void cbm_iec_atn(int state) override;
	virtual void cbm_iec_data(int state) override;
	virtual void cbm_iec_reset(int state) override;

private:
	enum
	{
		LED_POWER = 0,
		LED_ACT
	};

	void update_iec();

	void cnt_w(int state);
	void sp_w(int state);
	uint8_t cia_pa_r();
	void cia_pa_w(uint8_t data);
	uint8_t cia_pb_r();
	void cia_pb_w(uint8_t data);

	void c1581_mem(address_map &map) ATTR_COLD;

	static void floppy_formats(format_registration &fr);

	required_device<cpu_device> m_maincpu;
	required_device<mos6526_device> m_cia;
	required_device<wd1772_device> m_fdc;
	required_device<floppy_image_device> m_floppy;
	required_ioport m_address;
	output_finder<2> m_leds;

	int m_data_out;             // serial data out
	int m_atn_ack;              // attention acknowledge
	int m_fast_ser_dir;         // fast serial direction
	int m_sp_out;               // fast serial data out
	int m_cnt_out;              // fast serial clock out
};


// ======================> c1563_device

class c1563_device : public c1581_device
{
public:
	// construction/destruction
	c1563_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(C1563, c1563_device)
DECLARE_DEVICE_TYPE(C1581, c1581_device)


#endif // MAME_BUS_CBMIEC_C1581_H
