// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_NANJING_H
#define MAME_BUS_NES_NANJING_H

#pragma once

#include "nxrom.h"


class ppu2c0x_device;

// ======================> nes_nanjing_device

class nes_nanjing_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_nanjing_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_l(offs_t offset) override;
	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual uint8_t chr_r(offs_t offset) override;
	virtual void chr_w(offs_t offset, uint8_t data) override;
	virtual uint8_t nt_r(offs_t offset) override;
	virtual void nt_w(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	void update_prg();

	uint8_t m_reg5000, m_reg5200, m_mode;   // NESdev mapper 163 registers ($5000, $5200, $5300)
	uint8_t m_feedback;                     // latched F (bit 2) and E (bit 0) from $5100
	bool m_last_nt;                         // last PPU access was a nametable access (PPU A13=1)
	uint8_t m_chr_a12;                      // PPU A9 latched on the last rise of PPU A13
};


// device type definition
DECLARE_DEVICE_TYPE(NES_NANJING, nes_nanjing_device)

#endif // MAME_BUS_NES_NANJING_H
