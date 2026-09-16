// license:BSD-3-Clause
// copyright-holders:TwistedTom
/**********************************************************************

    DISCiPLE Multi-purpose Interface
    +D Disk and Printer Interface

    (Miles Gordon Technology)

**********************************************************************/

#ifndef MAME_BUS_SPECTRUM_MGT_H
#define MAME_BUS_SPECTRUM_MGT_H

#pragma once

#include "exp.h"
#include "softlist.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"
#include "bus/centronics/ctronics.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class spectrum_plusd_device: public device_t, public device_spectrum_expansion_interface
{
public:
	// construction/destruction
	spectrum_plusd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static void floppy_formats(format_registration &fr);
	DECLARE_INPUT_CHANGED_MEMBER(snapshot_button);

protected:
	spectrum_plusd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void pre_opcode_fetch(offs_t offset) override;
	virtual uint8_t mreq_r(offs_t offset) override;
	virtual void mreq_w(offs_t offset, uint8_t data) override;
	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;
	virtual bool romcs() override;

	void busy_w(int state);

	required_memory_region m_rom;
	required_device<wd_fdc_device_base> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<centronics_device> m_centronics;

	bool m_romcs;
	uint8_t m_ram[8 * 1024];
	bool m_centronics_busy;
};

class spectrum_disciple_device: public spectrum_plusd_device
{
public:
	// construction/destruction
	spectrum_disciple_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_INPUT_CHANGED_MEMBER(inhibit_button) { if (!newval) m_romcs = 0; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void pre_opcode_fetch(offs_t offset) override;
	virtual uint8_t mreq_r(offs_t offset) override;
	virtual void mreq_w(offs_t offset, uint8_t data) override;
	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;
	virtual bool romcs() override;

	TIMER_CALLBACK_MEMBER(reset_tick);

private:
	required_device<spectrum_expansion_slot_device> m_exp;
	required_ioport m_joy1;
	required_ioport m_joy2;
	required_ioport m_inhibit;

	bool m_map;
	u8 m_control;
	bool m_reset_delay;
	emu_timer *m_reset_timer;
};


// device type definition
DECLARE_DEVICE_TYPE(SPECTRUM_PLUSD, spectrum_plusd_device)
DECLARE_DEVICE_TYPE(SPECTRUM_DISCIPLE, spectrum_disciple_device)

#endif // MAME_BUS_SPECTRUM_MGT_H
