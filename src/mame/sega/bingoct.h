// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Bingo Circus Terminal

    © 1989 Sega

***************************************************************************/

#ifndef MAME_SEGA_BINGOCT_H
#define MAME_SEGA_BINGOCT_H

#pragma once

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "video/315_5124.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bingoct_device

class bingoct_device :  public device_t
{
public:
	// construction/destruction
	bingoct_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	// device-level overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device<z80_device> m_maincpu;
	required_device<z80_device> m_soundcpu;
	required_device<sega315_5246_device> m_vdp;
	required_memory_bank m_gfxbank;
	required_memory_bank m_soundbank;

	void main_mem_map(address_map &map) ATTR_COLD;
	void main_io_map(address_map &map) ATTR_COLD;
	void sound_mem_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
	void pcm_map(address_map &map) ATTR_COLD;

	IRQ_CALLBACK_MEMBER(soundcpu_irq_ack);
	void output_w(offs_t offset, uint8_t data);
};

// device type definition
DECLARE_DEVICE_TYPE(BINGOCT, bingoct_device)

#endif // MAME_SEGA_BINGOCT_H
