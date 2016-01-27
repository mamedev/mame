// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#pragma once

#ifndef __DMV_K230_H__
#define __DMV_K230_H__

#include "emu.h"
#include "dmvbus.h"
#include "cpu/i86/i86.h"

// K234
#include "cpu/m68000/m68000.h"

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
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	DECLARE_READ8_MEMBER(io_r);
	DECLARE_READ8_MEMBER(program_r);
	DECLARE_WRITE8_MEMBER(io_w);
	DECLARE_WRITE8_MEMBER(program_w);
	DECLARE_READ8_MEMBER(rom_r);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// dmvcart_interface overrides
	virtual void hold_w(int state) override;
	virtual void switch16_w(int state) override;
	virtual bool av16bit() override;

protected:
	required_device<cpu_device> m_maincpu;
	optional_memory_region      m_rom;
	dmvcart_slot_device *       m_bus;
	address_space *             m_io;
	int                         m_switch16;
	int                         m_hold;
};


// ======================> dmv_k231_device

class dmv_k231_device :
		public dmv_k230_device
{
public:
	// construction/destruction
	dmv_k231_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
};


// ======================> dmv_k234_device

class dmv_k234_device :
		public dmv_k230_device
{
public:
	// construction/destruction
	dmv_k234_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER(snr_r);
	DECLARE_WRITE8_MEMBER(snr_w);

protected:
	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// dmvcart_interface overrides
	virtual void hold_w(int state) override;
	virtual void switch16_w(int state) override;

private:
	int                         m_snr;
};


// ======================> dmv_k235_device

class dmv_k235_device :
		public dmv_k230_device
{
public:
	// construction/destruction
	dmv_k235_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	void timint_w(int state) override  { m_pic->ir0_w(state); }
	void keyint_w(int state) override  { m_pic->ir1_w(state); }
	void busint_w(int state) override  { m_pic->ir2_w(state); }
	void flexint_w(int state) override { m_pic->ir6_w(state); }
	void irq2a_w(int state) override   { if (!(m_dsw->read() & 0x02))  m_pic->ir5_w(state); }
	void irq2_w(int state) override    { if ( (m_dsw->read() & 0x02))  m_pic->ir5_w(state); }
	void irq3_w(int state) override    { m_pic->ir3_w(state); }
	void irq4_w(int state) override    { m_pic->ir4_w(state); }
	void irq5_w(int state) override    { if (!(m_dsw->read() & 0x01))  m_pic->ir7_w(state); }
	void irq6_w(int state) override    { if ( (m_dsw->read() & 0x01))  m_pic->ir7_w(state); }

private:
	required_device<pic8259_device> m_pic;
	required_ioport m_dsw;
};

// device type definition
extern const device_type DMV_K230;
extern const device_type DMV_K231;
extern const device_type DMV_K234;
extern const device_type DMV_K235;

#endif  /* __DMV_K230_H__ */
