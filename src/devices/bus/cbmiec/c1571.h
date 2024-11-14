// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 1570/1571/1571CR Single Disk Drive emulation

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_C1571_H
#define MAME_BUS_CBMIEC_C1571_H

#pragma once

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

// ======================> c1571_device

class c1571_device : public device_t, public device_cbm_iec_interface, public device_c64_floppy_parallel_interface
{
public:
	// construction/destruction
	c1571_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void via0_irq_w(int state);
	uint8_t via0_pa_r();
	void via0_pa_w(uint8_t data);
	uint8_t via0_pb_r();
	void via0_pb_w(uint8_t data);

	uint8_t via1_r(offs_t offset);
	void via1_w(offs_t offset, uint8_t data);
	void via1_irq_w(int state);
	uint8_t via1_pb_r();
	void via1_pb_w(uint8_t data);

	void cia_irq_w(int state);
	void cia_pc_w(int state);
	void cia_cnt_w(int state);
	void cia_sp_w(int state);
	uint8_t cia_pb_r();
	void cia_pb_w(uint8_t data);

	void byte_w(int state);

	static void floppy_formats(format_registration &fr);

	void wpt_callback(floppy_image_device *floppy, int state);

	void c1571_mem(address_map &map) ATTR_COLD;
protected:
	c1571_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

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

	// device_c64_floppy_parallel_interface overrides
	virtual void parallel_data_w(uint8_t data) override;
	virtual void parallel_strobe_w(int state) override;

	void add_base_mconfig(machine_config &config);
	void add_cia_mconfig(machine_config &config);

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
	required_device<wd1770_device> m_fdc;
	required_device<c64h156_device> m_ga;
	required_device<floppy_image_device> m_floppy;
	required_ioport m_address;
	output_finder<2> m_leds;

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


// ======================> c1570_device

class c1570_device :  public c1571_device
{
public:
	// construction/destruction
	c1570_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// ======================> c1571cr_device

class c1571cr_device :  public c1571_device
{
public:
	// construction/destruction
	c1571cr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void via0_pa_w(uint8_t data);
	void via0_pb_w(uint8_t data);
};


// ======================> mini_chief_device

class mini_chief_device :  public c1571_device
{
public:
	// construction/destruction
	mini_chief_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	uint8_t cia_pa_r();
	void cia_pa_w(uint8_t data);
	void cia_pb_w(uint8_t data);

	void mini_chief_mem(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(C1570,      c1570_device)
DECLARE_DEVICE_TYPE(C1571,      c1571_device)
DECLARE_DEVICE_TYPE(C1571CR,    c1571cr_device)
DECLARE_DEVICE_TYPE(MINI_CHIEF, mini_chief_device)

#endif // MAME_BUS_CBMIEC_C1571_H
