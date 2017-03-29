// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#pragma once

#ifndef __KC_D004_H__
#define __KC_D004_H__

#include "kc.h"
#include "machine/z80ctc.h"
#include "cpu/z80/z80.h"
#include "machine/upd765.h"
#include "machine/ataintf.h"
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
	kc_d004_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	DECLARE_FLOPPY_FORMATS( floppy_formats );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// kcexp_interface overrides
	virtual uint8_t module_id_r() override { return 0xa7; }
	virtual void control_w(uint8_t data) override;
	virtual void read(offs_t offset, uint8_t &data) override;
	virtual void io_read(offs_t offset, uint8_t &data) override;
	virtual void io_write(offs_t offset, uint8_t data) override;

public:
	DECLARE_READ8_MEMBER(hw_input_gate_r);
	DECLARE_WRITE8_MEMBER(fdd_select_w);
	DECLARE_WRITE8_MEMBER(hw_terminal_count_w);

	DECLARE_WRITE_LINE_MEMBER( fdc_irq );

private:
	static const device_timer_id TIMER_RESET = 0;

	required_device<cpu_device> m_cpu;
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

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

protected:
	// device-level overrides
	virtual void device_reset() override;

public:
	DECLARE_READ8_MEMBER(gide_r);
	DECLARE_WRITE8_MEMBER(gide_w);

private:
	required_device<ata_interface_device> m_ata;

	uint16_t              m_ata_data;
	int                 m_lh;
};

// device type definition
extern const device_type KC_D004;
extern const device_type KC_D004_GIDE;

#endif  /* __KC_D004_H__ */
