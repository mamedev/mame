// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli


#ifndef MAME_BUS_NEOGEO_PROT_KOF98_H
#define MAME_BUS_NEOGEO_PROT_KOF98_H

#pragma once

DECLARE_DEVICE_TYPE(NG_KOF98_PROT, kof98_prot_device)


class kof98_prot_device :  public device_t
{
public:
	// construction/destruction
	kof98_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void decrypt_68k(uint8_t* cpurom, uint32_t cpurom_size);
	void protection_w(uint16_t data);
	uint16_t protection_r(offs_t offset);
	int m_prot_state;
	uint16_t m_default_rom[2];

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};

#endif // MAME_BUS_NEOGEO_PROT_KOF98_H
