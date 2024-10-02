// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli

#ifndef MAME_BUS_NEOGEO_PROT_KOF2K3BL_H
#define MAME_BUS_NEOGEO_PROT_KOF2K3BL_H

#pragma once

DECLARE_DEVICE_TYPE(NG_KOF2K3BL_PROT, kof2k3bl_prot_device)


class kof2k3bl_prot_device :  public device_t
{
public:
	// construction/destruction
	kof2k3bl_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	uint16_t protection_r(offs_t offset);
	void kof2003_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void kof2003p_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t overlay_r();
	void bl_px_decrypt(uint8_t* cpurom, uint32_t cpurom_size);
	void pl_px_decrypt(uint8_t* cpurom, uint32_t cpurom_size);
	void upl_px_decrypt(uint8_t* cpurom, uint32_t cpurom_size);
	uint32_t get_bank_base() {return m_bank_base; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint16_t m_overlay;
	uint32_t m_bank_base;
	uint16_t m_cartridge_ram[0x1000]; // bootlegs
};

#endif // MAME_BUS_NEOGEO_PROT_KOF2K3BL_H
