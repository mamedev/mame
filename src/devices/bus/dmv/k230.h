// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#ifndef MAME_BUS_DMV_K230_H
#define MAME_BUS_DMV_K230_H

#pragma once

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
	dmv_k230_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	dmv_k230_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	// dmvcart_interface overrides
	virtual void hold_w(int state) override;
	virtual void switch16_w(int state) override;
	virtual bool av16bit() override;

	void k230_io(address_map &map);
	void k230_mem(address_map &map);

	required_device<cpu_device> m_maincpu;
	optional_memory_region      m_rom;
	required_device<dmvcart_slot_device> m_bus;
	int                         m_switch16;
	int                         m_hold;

	DECLARE_READ8_MEMBER(io_r);
	DECLARE_READ8_MEMBER(program_r);
	DECLARE_WRITE8_MEMBER(io_w);
	DECLARE_WRITE8_MEMBER(program_w);

private:
	DECLARE_READ8_MEMBER(rom_r);
};


// ======================> dmv_k231_device

class dmv_k231_device :
		public dmv_k230_device
{
public:
	// construction/destruction
	dmv_k231_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
};


// ======================> dmv_k234_device

class dmv_k234_device :
		public dmv_k230_device
{
public:
	// construction/destruction
	dmv_k234_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// dmvcart_interface overrides
	virtual void hold_w(int state) override;
	virtual void switch16_w(int state) override;

private:
	int                         m_snr;

	DECLARE_READ8_MEMBER(snr_r);
	DECLARE_WRITE8_MEMBER(snr_w);

	void k234_mem(address_map &map);
};


// ======================> dmv_k235_device

class dmv_k235_device :
		public dmv_k230_device
{
public:
	// construction/destruction
	dmv_k235_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
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

	void k235_io(address_map &map);
};

// device type definition
DECLARE_DEVICE_TYPE(DMV_K230, dmv_k230_device)
DECLARE_DEVICE_TYPE(DMV_K231, dmv_k231_device)
DECLARE_DEVICE_TYPE(DMV_K234, dmv_k234_device)
DECLARE_DEVICE_TYPE(DMV_K235, dmv_k235_device)

#endif // MAME_BUS_DMV_K230_H
