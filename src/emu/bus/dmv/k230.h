// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#pragma once

#ifndef __DMV_K230_H__
#define __DMV_K230_H__

#include "emu.h"
#include "dmvbus.h"
#include "cpu/i86/i86.h"

// K235
#include "cpu/nec/nec.h"
#include "machine/pic8259.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dmv_k230_device

class dmv_k230_device :
		public device_t,
		public device_dmvslot_interface
{
public:
	// construction/destruction
	dmv_k230_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	dmv_k230_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

	DECLARE_READ8_MEMBER(io_r);
	DECLARE_READ8_MEMBER(program_r);
	DECLARE_WRITE8_MEMBER(io_w);
	DECLARE_WRITE8_MEMBER(program_w);
	DECLARE_READ8_MEMBER(rom_r);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// dmvcart_interface overrides
	virtual void hold_w(int state);
	virtual bool av16bit();

private:
	required_device<cpu_device> m_maincpu;
	required_memory_region      m_rom;
	dmvcart_slot_device *       m_bus;
	address_space *             m_io;
};


// ======================> dmv_k231_device

class dmv_k231_device :
		public dmv_k230_device
{
public:
	// construction/destruction
	dmv_k231_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
};


// ======================> dmv_k235_device

class dmv_k235_device :
		public dmv_k230_device
{
public:
	// construction/destruction
	dmv_k235_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

	void irq0_w(int state) { m_pic->ir0_w(state); }
	void irq1_w(int state) { m_pic->ir1_w(state); }
	void irq2_w(int state) { m_pic->ir2_w(state); }
	void irq3_w(int state) { m_pic->ir3_w(state); }
	void irq4_w(int state) { m_pic->ir4_w(state); }
	void irq5_w(int state) { m_pic->ir5_w(state); }
	void irq6_w(int state) { m_pic->ir6_w(state); }
	void irq7_w(int state) { m_pic->ir7_w(state); }

private:
	required_device<pic8259_device> m_pic;

};

// device type definition
extern const device_type DMV_K230;
extern const device_type DMV_K231;
extern const device_type DMV_K235;

#endif  /* __DMV_K230_H__ */
