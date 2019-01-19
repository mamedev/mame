// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli

#ifndef MAME_BUS_NEOGEO_PROT_SMA_H
#define MAME_BUS_NEOGEO_PROT_SMA_H

#pragma once


DECLARE_DEVICE_TYPE(NG_SMA_PROT, sma_prot_device)


class sma_prot_device : public device_t
{
public:
	// construction/destruction
	sma_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	//DECLARE_WRITE16_MEMBER( kof99_bankswitch_w );
	//DECLARE_WRITE16_MEMBER( garou_bankswitch_w );
	//DECLARE_WRITE16_MEMBER( garouh_bankswitch_w );
	//DECLARE_WRITE16_MEMBER( mslug3_bankswitch_w );
	//DECLARE_WRITE16_MEMBER( kof2000_bankswitch_w );
	DECLARE_READ16_MEMBER( prot_9a37_r );
	DECLARE_READ16_MEMBER( random_r );
	u32 kof99_bank_base(u16 sel);
	u32 garou_bank_base(u16 sel);
	u32 garouh_bank_base(u16 sel);
	u32 mslug3_bank_base(u16 sel);
	u32 mslug3a_bank_base(u16 sel);
	u32 kof2000_bank_base(u16 sel);
	void kof99_decrypt_68k(u8* base);
	void garou_decrypt_68k(u8* base);
	void garouh_decrypt_68k(u8* base);
	void mslug3_decrypt_68k(u8* base);
	void mslug3a_decrypt_68k(u8* base);
	void kof2000_decrypt_68k(u8* base);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	u16     m_sma_rng;
};

#endif // MAME_BUS_NEOGEO_PROT_SMA_H
