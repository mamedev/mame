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
	isa8_pgc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	isa8_pgc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER( scanline_callback );
	INTERRUPT_GEN_MEMBER(vblank_irq);
	IRQ_CALLBACK_MEMBER(irq_callback);

	DECLARE_WRITE8_MEMBER( stateparam_w );
	DECLARE_READ8_MEMBER( stateparam_r );
	DECLARE_WRITE8_MEMBER( lut_w );
	DECLARE_READ8_MEMBER( init_r );

	void reset_common();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<i8088_cpu_device> m_cpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	UINT8 *m_commarea;
	std::unique_ptr<UINT8[]> m_vram;
	std::unique_ptr<UINT8[]> m_eram;
	UINT8 m_stateparam[16];
	UINT8 m_lut[256*3];
	std::unique_ptr<bitmap_ind16> m_bitmap;
};


// device type definition
extern const device_type ISA8_PGC;

#endif  /* __ISA_PGC_H__ */
