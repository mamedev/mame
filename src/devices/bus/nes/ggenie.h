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

	// everything else on the cartridge connector passes through to the plugged cartridge too, once the Game Genie has
	// handed it control; the "wants" queries report what that cartridge wants
	virtual u8 read_ex(offs_t offset) override { return pass() ? m_ggslot->m_cart->read_ex(offset) : 0xff; }
	virtual void write_ex(offs_t offset, u8 data) override { if (pass()) m_ggslot->m_cart->write_ex(offset, data); }
	virtual void cpu_write_latch(u8 data) override { if (pass()) m_ggslot->m_cart->cpu_write_latch(data); }
	virtual bool wants_a12() override { return m_ggslot->m_cart && m_ggslot->m_cart->wants_a12(); }
	virtual void ppu_a12_rise() override { if (pass()) m_ggslot->m_cart->ppu_a12_rise(); }
	virtual bool wants_a12_filter() override { return m_ggslot->m_cart && m_ggslot->m_cart->wants_a12_filter(); }
	virtual bool a12_rise_accepted(const attotime &low_since, const attotime &when) override { return !pass() || m_ggslot->m_cart->a12_rise_accepted(low_since, when); }
	virtual bool wants_a12_level() override { return m_ggslot->m_cart && m_ggslot->m_cart->wants_a12_level(); }
	virtual void ppu_a12_level(int state) override { if (pass()) m_ggslot->m_cart->ppu_a12_level(state); }
	virtual bool wants_ppu_addr_bus() override { return m_ggslot->m_cart && m_ggslot->m_cart->wants_ppu_addr_bus(); }
	virtual void ppu_addr_bus(u16 addr) override { if (pass()) m_ggslot->m_cart->ppu_addr_bus(addr); }
	virtual bool wants_ppu_strobes() override { return m_ggslot->m_cart && m_ggslot->m_cart->wants_ppu_strobes(); }
	virtual void ppu_ale(offs_t addr) override { if (pass()) m_ggslot->m_cart->ppu_ale(addr); }
	virtual void ppu_rd(offs_t addr) override { if (pass()) m_ggslot->m_cart->ppu_rd(addr); }
	virtual void ppu_wr(offs_t addr, u8 data) override { if (pass()) m_ggslot->m_cart->ppu_wr(addr, data); }
	virtual bool wants_m2() override { return m_ggslot->m_cart && m_ggslot->m_cart->wants_m2(); }
	virtual void m2_w(int state, offs_t address, bool write) override { if (pass()) m_ggslot->m_cart->m2_w(state, address, write); }
	virtual bool wants_read_latch() override { return m_ggslot->m_cart && m_ggslot->m_cart->wants_read_latch(); }
	virtual void ppu_read_latch(offs_t offset) override { if (pass()) m_ggslot->m_cart->ppu_read_latch(offset); }
	virtual void set_a12_driven(bool driven) override { device_nes_cart_interface::set_a12_driven(driven); if (m_ggslot->m_cart) m_ggslot->m_cart->set_a12_driven(driven); }
	virtual int mmc3_irq_variant() const override { return m_ggslot->m_cart ? m_ggslot->m_cart->mmc3_irq_variant() : device_nes_cart_interface::mmc3_irq_variant(); }

	virtual void pcb_reset() override;
	virtual void pcb_start(running_machine &machine, u8 *ciram_ptr, bool cart_mounted) override;

	void gg_irq_w(int state) { set_irq_line(state); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	// game mode, with a cartridge plugged in to pass everything through to
	bool pass() const { return m_gg_bypass && m_ggslot->m_cart; }

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
