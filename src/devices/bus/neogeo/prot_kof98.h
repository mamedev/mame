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
	kof98_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void decrypt_68k(u8* cpurom, u32 cpurom_size);
	DECLARE_WRITE16_MEMBER(protection_w);
	DECLARE_READ16_MEMBER(protection_r);
	int m_prot_state;
	u16 m_default_rom[2];

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
};

#endif // MAME_BUS_NEOGEO_PROT_KOF98_H
