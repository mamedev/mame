// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_GGENIE_H
#define MAME_BUS_NES_GGENIE_H

#pragma once

#include "nxrom.h"


// ======================> nes_ggenie_device

class nes_ggenie_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ggenie_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_l(offs_t offset) override;
	virtual u8 read_m(offs_t offset) override;
	virtual u8 read_h(offs_t offset) override;
	virtual void write_l(offs_t offset, u8 data) override;
	virtual void write_m(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual u8 chr_r(offs_t offset) override;
	virtual void chr_w(offs_t offset, u8 data) override;
	virtual u8 nt_r(offs_t offset) override;
	virtual void nt_w(offs_t offset, u8 data) override;

	void hblank_irq(int scanline, bool vblank, bool blanked) override { if (m_gg_bypass && m_ggslot->m_cart) m_ggslot->m_cart->hblank_irq(scanline, vblank, blanked); }
	void scanline_irq(int scanline, bool vblank, bool blanked) override { if (m_gg_bypass && m_ggslot->m_cart) m_ggslot->m_cart->scanline_irq(scanline, vblank, blanked); }
	void ppu_latch(offs_t offset) override { if (m_gg_bypass && m_ggslot->m_cart) m_ggslot->m_cart->ppu_latch(offset); }

	virtual void pcb_reset() override;
	virtual void pcb_start(running_machine &machine, u8 *ciram_ptr, bool cart_mounted) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	// passthrough cart slot
	required_device<nes_cart_slot_device> m_ggslot;

	bool m_gg_bypass;
	// GG codes
	u16  m_gg_addr[3];
	u8   m_gg_repl[3];
	u8   m_gg_comp[3];
	bool m_gg_enable[3];
	bool m_gg_is_comp[3];
};


// device type definition
DECLARE_DEVICE_TYPE(NES_GGENIE, nes_ggenie_device)

#endif // MAME_BUS_NES_GGENIE_H
