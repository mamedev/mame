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
	nes_ggenie_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_READ8_MEMBER(read_l) override;
	virtual DECLARE_READ8_MEMBER(read_m) override;
	virtual DECLARE_READ8_MEMBER(read_h) override;
	virtual DECLARE_WRITE8_MEMBER(write_l) override;
	virtual DECLARE_WRITE8_MEMBER(write_m) override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual DECLARE_READ8_MEMBER(chr_r) override;
	virtual DECLARE_WRITE8_MEMBER(chr_w) override;
	virtual DECLARE_READ8_MEMBER(nt_r) override;
	virtual DECLARE_WRITE8_MEMBER(nt_w) override;

	void hblank_irq(int scanline, int vblank, int blanked) override { if (m_gg_bypass && m_ggslot->m_cart) m_ggslot->m_cart->hblank_irq(scanline, vblank, blanked); }
	void scanline_irq(int scanline, int vblank, int blanked) override { if (m_gg_bypass && m_ggslot->m_cart) m_ggslot->m_cart->scanline_irq(scanline, vblank, blanked); }
	void ppu_latch(offs_t offset) override { if (m_gg_bypass && m_ggslot->m_cart) m_ggslot->m_cart->ppu_latch(offset); }

	virtual void pcb_reset() override;
	virtual void pcb_start(running_machine &machine, uint8_t *ciram_ptr, bool cart_mounted) override;

protected:
	// device-level overrides
	virtual void device_start() override;

	virtual void device_add_mconfig(machine_config &config) override;

private:
	// emulate the Game Genie!
	required_device<nes_cart_slot_device> m_ggslot;

	int m_gg_bypass;
	// GG codes
	uint16_t m_gg_addr[3];
	uint8_t  m_gg_repl[3];
	uint8_t  m_gg_comp[3];
	int    m_gg_disable[3];
	int    m_gg_is_comp[3];
};


// device type definition
DECLARE_DEVICE_TYPE(NES_GGENIE, nes_ggenie_device)

#endif // MAME_BUS_NES_GGENIE_H
