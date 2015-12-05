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
	nes_ggenie_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_READ8_MEMBER(read_l);
	virtual DECLARE_READ8_MEMBER(read_m);
	virtual DECLARE_READ8_MEMBER(read_h);
	virtual DECLARE_WRITE8_MEMBER(write_l);
	virtual DECLARE_WRITE8_MEMBER(write_m);
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual DECLARE_READ8_MEMBER(chr_r);
	virtual DECLARE_WRITE8_MEMBER(chr_w);
	virtual DECLARE_READ8_MEMBER(nt_r);
	virtual DECLARE_WRITE8_MEMBER(nt_w);

	void hblank_irq(int scanline, int vblank, int blanked) override { if (m_gg_bypass && m_ggslot->m_cart) m_ggslot->m_cart->hblank_irq(scanline, vblank, blanked); }
	void scanline_irq(int scanline, int vblank, int blanked) override { if (m_gg_bypass && m_ggslot->m_cart) m_ggslot->m_cart->scanline_irq(scanline, vblank, blanked); }
	void ppu_latch(offs_t offset) override { if (m_gg_bypass && m_ggslot->m_cart) m_ggslot->m_cart->ppu_latch(offset); }

	virtual machine_config_constructor device_mconfig_additions() const override;

	virtual void pcb_reset() override;
	virtual void pcb_start(running_machine &machine, UINT8 *ciram_ptr, bool cart_mounted) override;

private:
	// emulate the Game Genie!
	required_device<nes_cart_slot_device> m_ggslot;

	int m_gg_bypass;
	// GG codes
	UINT16 m_gg_addr[3];
	UINT8  m_gg_repl[3];
	UINT8  m_gg_comp[3];
	int    m_gg_disable[3];
	int    m_gg_is_comp[3];
};



// device type definition
extern const device_type NES_GGENIE;

#endif
