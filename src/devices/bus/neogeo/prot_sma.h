// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli


#pragma once

#ifndef __SMA_PROT__
#define __SMA_PROT__

extern const device_type SMA_PROT;

#define MCFG_SMA_PROT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SMA_PROT, 0)


class sma_prot_device :  public device_t
{
public:
	// construction/destruction
	sma_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	//DECLARE_WRITE16_MEMBER( kof99_bankswitch_w );
	//DECLARE_WRITE16_MEMBER( garou_bankswitch_w );
	//DECLARE_WRITE16_MEMBER( garouh_bankswitch_w );
	//DECLARE_WRITE16_MEMBER( mslug3_bankswitch_w );
	//DECLARE_WRITE16_MEMBER( kof2000_bankswitch_w );
	DECLARE_READ16_MEMBER( prot_9a37_r );
	DECLARE_READ16_MEMBER( random_r );
	UINT32 kof99_bank_base(UINT16 sel);
	UINT32 garou_bank_base(UINT16 sel);
	UINT32 garouh_bank_base(UINT16 sel);
	UINT32 mslug3_bank_base(UINT16 sel);
	UINT32 kof2000_bank_base(UINT16 sel);
	void kof99_decrypt_68k(UINT8* base);
	void garou_decrypt_68k(UINT8* base);
	void garouh_decrypt_68k(UINT8* base);
	void mslug3_decrypt_68k(UINT8* base);
	void kof2000_decrypt_68k(UINT8* base);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	UINT16     m_sma_rng;
};

#endif
