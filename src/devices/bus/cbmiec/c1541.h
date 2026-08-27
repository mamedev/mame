// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 1540/1541/1541C/1541-II Single Disk Drive emulation

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_C1541_H
#define MAME_BUS_CBMIEC_C1541_H

#pragma once

#include "cbmiec.h"
#include "bus/c64/bn1541.h"
#include "cpu/m6502/m6502.h"
#include "machine/64h156.h"
#include "machine/6522via.h"
#include "machine/input_merger.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c1541_device_base

class c1541_device_base :  public device_t,
					       public device_cbm_iec_interface,
					       public device_c64_floppy_parallel_interface
{
protected:
	// construction/destruction
	c1541_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_cbm_iec_interface overrides
	virtual void cbm_iec_atn(int state) override;
	virtual void cbm_iec_reset(int state) override;

	// device_c64_floppy_parallel_interface overrides
	virtual void parallel_data_w(u8 data) override;
	virtual void parallel_strobe_w(int state) override;

	required_device<floppy_image_device> m_floppy;

	void c1541_mem(address_map &map) ATTR_COLD;

	required_device<m6502_device> m_maincpu;

private:
	enum
	{
		LED_POWER = 0,
		LED_ACT
	};

	void via0_irq_w(int state);
	virtual uint8_t via0_pa_r();
	void via0_pa_w(uint8_t data);
	uint8_t via0_pb_r();
	void via0_pb_w(uint8_t data);
	void via0_ca2_w(int state);
	void via1_irq_w(int state);
	uint8_t via1_pb_r();
	void via1_pb_w(uint8_t data);
	void atn_w(int state);
	void byte_w(int state);

	static void floppy_formats(format_registration &fr);

	required_device<via6522_device> m_via0;
	required_device<via6522_device> m_via1;
	required_device<c64h156_device> m_ga;
	required_ioport m_address;
	output_finder<2> m_leds;

	bool m_iec_clk;
	bool m_iec_data;

	emu_timer *m_iec_sync_timer;
	TIMER_CALLBACK_MEMBER(iec_sync_tick);
};


// ======================> c1540_device

class c1540_device :  public c1541_device_base
{
public:
	// construction/destruction
	c1540_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// ======================> c1541_device

class c1541_device :  public c1541_device_base
{
public:
	// construction/destruction
	c1541_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// ======================> c1541c_device

class c1541c_device :  public c1541_device_base
{
public:
	// construction/destruction
	c1541c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	virtual uint8_t via0_pa_r() override;
};


// ======================> c1541ii_device

class c1541ii_device :  public c1541_device_base
{
public:
	// construction/destruction
	c1541ii_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// ======================> sx1541_device

class sx1541_device :  public c1541_device_base
{
public:
	// construction/destruction
	sx1541_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(C1540, c1540_device)
DECLARE_DEVICE_TYPE(C1541, c1541_device)
DECLARE_DEVICE_TYPE(C1541C, c1541c_device)
DECLARE_DEVICE_TYPE(C1541II, c1541ii_device)
DECLARE_DEVICE_TYPE(SX1541, sx1541_device)


#endif // MAME_BUS_CBMIEC_C1541_H
