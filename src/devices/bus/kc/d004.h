// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#ifndef MAME_BUS_KC_D004_H
#define MAME_BUS_KC_D004_H

#pragma once

#include "kc.h"
#include "machine/z80ctc.h"
#include "cpu/z80/z80.h"
#include "machine/upd765.h"
#include "bus/ata/ataintf.h"
#include "imagedev/floppy.h"
#include "imagedev/harddriv.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> kc_d004_device

class kc_d004_device :
		public device_t,
		public device_kcexp_interface
{
public:
	// construction/destruction
	kc_d004_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	kc_d004_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// kcexp_interface overrides
	virtual uint8_t module_id_r() override { return 0xa7; }
	virtual void control_w(uint8_t data) override;
	virtual void read(offs_t offset, uint8_t &data) override;
	virtual void io_read(offs_t offset, uint8_t &data) override;
	virtual void io_write(offs_t offset, uint8_t data) override;

	TIMER_CALLBACK_MEMBER(reset_tick);

	uint8_t hw_input_gate_r();
	void fdd_select_w(uint8_t data);
	void hw_terminal_count_w(uint8_t data);

	required_device<z80_device> m_cpu;

private:
	static void floppy_formats(format_registration &fr);

	void fdc_irq(int state);

	void kc_d004_io(address_map &map) ATTR_COLD;
	void kc_d004_mem(address_map &map) ATTR_COLD;

	required_device<upd765a_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;
	required_shared_ptr<uint8_t>  m_koppel_ram;

	// internal state
	emu_timer *         m_reset_timer;

	uint8_t *             m_rom;
	//uint8_t               m_hw_input_gate;
	uint16_t              m_rom_base;
	uint8_t               m_enabled;
	uint8_t               m_connected;

	floppy_image_device *m_floppy;
};


// ======================> kc_d004_gide_device

class kc_d004_gide_device :
		public kc_d004_device
{
public:
	// construction/destruction
	kc_d004_gide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	required_device<ata_interface_device> m_ata;

	uint16_t              m_ata_data;
	int                 m_lh;

	uint8_t gide_r(offs_t offset);
	void gide_w(offs_t offset, uint8_t data);

	void kc_d004_gide_io(address_map &map) ATTR_COLD;
};

// device type definition
DECLARE_DEVICE_TYPE(KC_D004,      kc_d004_device)
DECLARE_DEVICE_TYPE(KC_D004_GIDE, kc_d004_gide_device)

#endif  // MAME_BUS_KC_D004_H
