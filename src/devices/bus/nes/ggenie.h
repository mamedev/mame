// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_GGENIE_H
#define __NES_GGENIE_H

#include "nxrom.h"


// ======================> nes_ggenie_device

class nes_ggenie_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ggenie_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual uint8_t read_l(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_m(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_h(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_l(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual void write_m(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual uint8_t chr_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void chr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual uint8_t nt_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void nt_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	void hblank_irq(int scanline, int vblank, int blanked) override { if (m_gg_bypass && m_ggslot->m_cart) m_ggslot->m_cart->hblank_irq(scanline, vblank, blanked); }
	void scanline_irq(int scanline, int vblank, int blanked) override { if (m_gg_bypass && m_ggslot->m_cart) m_ggslot->m_cart->scanline_irq(scanline, vblank, blanked); }
	void ppu_latch(offs_t offset) override { if (m_gg_bypass && m_ggslot->m_cart) m_ggslot->m_cart->ppu_latch(offset); }

	virtual machine_config_constructor device_mconfig_additions() const override;

	virtual void pcb_reset() override;
	virtual void pcb_start(running_machine &machine, uint8_t *ciram_ptr, bool cart_mounted) override;

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
extern const device_type NES_GGENIE;

#endif
