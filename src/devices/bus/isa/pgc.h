// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev

#pragma once

#ifndef __ISA_PGC_H__
#define __ISA_PGC_H__

#include "emu.h"

#include "cpu/i86/i86.h"
#include "isa.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa8_pgc_device

class isa8_pgc_device :
	public device_t,
	public device_isa8_card_interface
{
public:
	// construction/destruction
	isa8_pgc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	isa8_pgc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void scanline_callback(timer_device &timer, void *ptr, int32_t param);
	void vblank_irq(device_t &device);
	int irq_callback(device_t &device, int irqline);

	void stateparam_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t stateparam_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void lut_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t init_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void reset_common();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<i8088_cpu_device> m_cpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	uint8_t *m_commarea;
	std::unique_ptr<uint8_t[]> m_vram;
	std::unique_ptr<uint8_t[]> m_eram;
	uint8_t m_stateparam[16];
	uint8_t m_lut[256*3];
	std::unique_ptr<bitmap_ind16> m_bitmap;
};


// device type definition
extern const device_type ISA8_PGC;

#endif  /* __ISA_PGC_H__ */
