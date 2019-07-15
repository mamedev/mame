// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev

#ifndef MAME_BUS_ISA_PGC_H
#define MAME_BUS_ISA_PGC_H

#pragma once


#include "cpu/i86/i86.h"
#include "machine/timer.h"
#include "isa.h"
#include "emupal.h"

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

protected:
	isa8_pgc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	IRQ_CALLBACK_MEMBER(irq_callback);

	DECLARE_WRITE8_MEMBER(vram_w);
	DECLARE_READ8_MEMBER(vram_r);
	DECLARE_WRITE8_MEMBER(stateparam_w);
	DECLARE_READ8_MEMBER(stateparam_r);
	DECLARE_WRITE8_MEMBER(lut_w);
	DECLARE_READ8_MEMBER(init_r);
	DECLARE_WRITE8_MEMBER(accel_w);

	void reset_common();

	void pgc_io(address_map &map);
	void pgc_map(address_map &map);

	required_device<i8088_cpu_device> m_cpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	uint8_t *m_commarea;
	std::unique_ptr<uint8_t[]> m_vram;
	std::unique_ptr<uint8_t[]> m_eram;
	uint8_t m_stateparam[16];
	uint8_t m_lut[256 * 3];
	int m_accel;
};


// device type definition
DECLARE_DEVICE_TYPE(ISA8_PGC, isa8_pgc_device)

#endif // MAME_BUS_ISA_PGC_H
