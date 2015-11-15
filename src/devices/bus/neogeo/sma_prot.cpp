// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood
#include "emu.h"
#include "sma_prot.h"



extern const device_type SMA_PROT = &device_creator<sma_prot_device>;


sma_prot_device::sma_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SMA_PROT, "NeoGeo SMA Cartridge", tag, owner, clock, "sma_prot", __FILE__), 
	m_bankdev(NULL), 
	m_sma_rng(0)
{
}


void sma_prot_device::device_start()
{
	save_item(NAME(m_sma_rng));
}

void sma_prot_device::device_reset()
{
	reset_sma_rng();
}





/************************ SMA Protection************************
  thanks to Razoola
***************************************************************/

WRITE16_MEMBER( sma_prot_device::kof99_bankswitch_w )
{
	int bankaddress;
	static const int bankoffset[64] =
	{
		0x000000, 0x100000, 0x200000, 0x300000,
		0x3cc000, 0x4cc000, 0x3f2000, 0x4f2000,
		0x407800, 0x507800, 0x40d000, 0x50d000,
		0x417800, 0x517800, 0x420800, 0x520800,
		0x424800, 0x524800, 0x429000, 0x529000,
		0x42e800, 0x52e800, 0x431800, 0x531800,
		0x54d000, 0x551000, 0x567000, 0x592800,
		0x588800, 0x581800, 0x599800, 0x594800,
		0x598000,   /* rest not used? */
	};

	/* unscramble bank number */
	data =
		(((data>>14)&1)<<0)+
		(((data>> 6)&1)<<1)+
		(((data>> 8)&1)<<2)+
		(((data>>10)&1)<<3)+
		(((data>>12)&1)<<4)+
		(((data>> 5)&1)<<5);

	bankaddress = 0x100000 + bankoffset[data];
	m_bankdev->neogeo_set_main_cpu_bank_address(bankaddress);
}


WRITE16_MEMBER( sma_prot_device::garou_bankswitch_w )
{
	/* thanks to Razoola and Mr K for the info */
	int bankaddress;
	static const int bankoffset[64] =
	{
		0x000000, 0x100000, 0x200000, 0x300000, // 00
		0x280000, 0x380000, 0x2d0000, 0x3d0000, // 04
		0x2f0000, 0x3f0000, 0x400000, 0x500000, // 08
		0x420000, 0x520000, 0x440000, 0x540000, // 12
		0x498000, 0x598000, 0x4a0000, 0x5a0000, // 16
		0x4a8000, 0x5a8000, 0x4b0000, 0x5b0000, // 20
		0x4b8000, 0x5b8000, 0x4c0000, 0x5c0000, // 24
		0x4c8000, 0x5c8000, 0x4d0000, 0x5d0000, // 28
		0x458000, 0x558000, 0x460000, 0x560000, // 32
		0x468000, 0x568000, 0x470000, 0x570000, // 36
		0x478000, 0x578000, 0x480000, 0x580000, // 40
		0x488000, 0x588000, 0x490000, 0x590000, // 44
		0x5d0000, 0x5d8000, 0x5e0000, 0x5e8000, // 48
		0x5f0000, 0x5f8000, 0x600000, /* rest not used? */
	};

	/* unscramble bank number */
	data =
		(((data>> 5)&1)<<0)+
		(((data>> 9)&1)<<1)+
		(((data>> 7)&1)<<2)+
		(((data>> 6)&1)<<3)+
		(((data>>14)&1)<<4)+
		(((data>>12)&1)<<5);

	bankaddress = 0x100000 + bankoffset[data];
	m_bankdev->neogeo_set_main_cpu_bank_address(bankaddress);
}


WRITE16_MEMBER( sma_prot_device::garouh_bankswitch_w )
{
	/* thanks to Razoola and Mr K for the info */
	int bankaddress;
	static const int bankoffset[64] =
	{
		0x000000, 0x100000, 0x200000, 0x300000, // 00
		0x280000, 0x380000, 0x2d0000, 0x3d0000, // 04
		0x2c8000, 0x3c8000, 0x400000, 0x500000, // 08
		0x420000, 0x520000, 0x440000, 0x540000, // 12
		0x598000, 0x698000, 0x5a0000, 0x6a0000, // 16
		0x5a8000, 0x6a8000, 0x5b0000, 0x6b0000, // 20
		0x5b8000, 0x6b8000, 0x5c0000, 0x6c0000, // 24
		0x5c8000, 0x6c8000, 0x5d0000, 0x6d0000, // 28
		0x458000, 0x558000, 0x460000, 0x560000, // 32
		0x468000, 0x568000, 0x470000, 0x570000, // 36
		0x478000, 0x578000, 0x480000, 0x580000, // 40
		0x488000, 0x588000, 0x490000, 0x590000, // 44
		0x5d8000, 0x6d8000, 0x5e0000, 0x6e0000, // 48
		0x5e8000, 0x6e8000, 0x6e8000, 0x000000, // 52
		0x000000, 0x000000, 0x000000, 0x000000, // 56
		0x000000, 0x000000, 0x000000, 0x000000, // 60
	};

	/* unscramble bank number */
	data =
		(((data>> 4)&1)<<0)+
		(((data>> 8)&1)<<1)+
		(((data>>14)&1)<<2)+
		(((data>> 2)&1)<<3)+
		(((data>>11)&1)<<4)+
		(((data>>13)&1)<<5);

	bankaddress = 0x100000 + bankoffset[data];
	m_bankdev->neogeo_set_main_cpu_bank_address(bankaddress);
}


WRITE16_MEMBER( sma_prot_device::mslug3_bankswitch_w )
{
	/* thanks to Razoola and Mr K for the info */
	int bankaddress;
	static const int bankoffset[64] =
	{
		0x000000, 0x020000, 0x040000, 0x060000, // 00
		0x070000, 0x090000, 0x0b0000, 0x0d0000, // 04
		0x0e0000, 0x0f0000, 0x120000, 0x130000, // 08
		0x140000, 0x150000, 0x180000, 0x190000, // 12
		0x1a0000, 0x1b0000, 0x1e0000, 0x1f0000, // 16
		0x200000, 0x210000, 0x240000, 0x250000, // 20
		0x260000, 0x270000, 0x2a0000, 0x2b0000, // 24
		0x2c0000, 0x2d0000, 0x300000, 0x310000, // 28
		0x320000, 0x330000, 0x360000, 0x370000, // 32
		0x380000, 0x390000, 0x3c0000, 0x3d0000, // 36
		0x400000, 0x410000, 0x440000, 0x450000, // 40
		0x460000, 0x470000, 0x4a0000, 0x4b0000, // 44
		0x4c0000, /* rest not used? */
	};

	/* unscramble bank number */
	data =
		(((data>>14)&1)<<0)+
		(((data>>12)&1)<<1)+
		(((data>>15)&1)<<2)+
		(((data>> 6)&1)<<3)+
		(((data>> 3)&1)<<4)+
		(((data>> 9)&1)<<5);

	bankaddress = 0x100000 + bankoffset[data];
	m_bankdev->neogeo_set_main_cpu_bank_address(bankaddress);
}


WRITE16_MEMBER( sma_prot_device::kof2000_bankswitch_w )
{
	/* thanks to Razoola and Mr K for the info */
	int bankaddress;
	static const int bankoffset[64] =
	{
		0x000000, 0x100000, 0x200000, 0x300000, // 00
		0x3f7800, 0x4f7800, 0x3ff800, 0x4ff800, // 04
		0x407800, 0x507800, 0x40f800, 0x50f800, // 08
		0x416800, 0x516800, 0x41d800, 0x51d800, // 12
		0x424000, 0x524000, 0x523800, 0x623800, // 16
		0x526000, 0x626000, 0x528000, 0x628000, // 20
		0x52a000, 0x62a000, 0x52b800, 0x62b800, // 24
		0x52d000, 0x62d000, 0x52e800, 0x62e800, // 28
		0x618000, 0x619000, 0x61a000, 0x61a800, // 32
	};

	/* unscramble bank number */
	data =
		(((data>>15)&1)<<0)+
		(((data>>14)&1)<<1)+
		(((data>> 7)&1)<<2)+
		(((data>> 3)&1)<<3)+
		(((data>>10)&1)<<4)+
		(((data>> 5)&1)<<5);

	bankaddress = 0x100000 + bankoffset[data];
	m_bankdev->neogeo_set_main_cpu_bank_address(bankaddress);
}


READ16_MEMBER( sma_prot_device::prot_9a37_r )
{
	return 0x9a37;
}


/* information about the sma random number generator provided by razoola */
/* this RNG is correct for KOF99, other games might be different */

READ16_MEMBER( sma_prot_device::sma_random_r )
{
	UINT16 old = m_sma_rng;

	UINT16 newbit = ((m_sma_rng >> 2) ^
						(m_sma_rng >> 3) ^
						(m_sma_rng >> 5) ^
						(m_sma_rng >> 6) ^
						(m_sma_rng >> 7) ^
						(m_sma_rng >>11) ^
						(m_sma_rng >>12) ^
						(m_sma_rng >>15)) & 1;

	m_sma_rng = (m_sma_rng << 1) | newbit;

	return old;
}


void sma_prot_device::reset_sma_rng()
{
	m_sma_rng = 0x2345;
}


void sma_prot_device::sma_install_random_read_handler(cpu_device* maincpu, int addr1, int addr2 )
{
	maincpu->space(AS_PROGRAM).install_read_handler(addr1, addr1 + 1, read16_delegate(FUNC(sma_prot_device::sma_random_r),this));
	maincpu->space(AS_PROGRAM).install_read_handler(addr2, addr2 + 1, read16_delegate(FUNC(sma_prot_device::sma_random_r),this));
}


void sma_prot_device::kof99_install_protection(cpu_device* maincpu, neogeo_banked_cart_device* bankdev)
{
	maincpu->space(AS_PROGRAM).install_write_handler(0x2ffff0, 0x2ffff1, write16_delegate(FUNC(sma_prot_device::kof99_bankswitch_w),this));
	maincpu->space(AS_PROGRAM).install_read_handler(0x2fe446, 0x2fe447, read16_delegate(FUNC(sma_prot_device::prot_9a37_r),this));
	m_bankdev = bankdev;

	sma_install_random_read_handler(maincpu, 0x2ffff8, 0x2ffffa);
}


void sma_prot_device::garou_install_protection(cpu_device* maincpu, neogeo_banked_cart_device* bankdev)
{
	maincpu->space(AS_PROGRAM).install_write_handler(0x2fffc0, 0x2fffc1, write16_delegate(FUNC(sma_prot_device::garou_bankswitch_w),this));
	maincpu->space(AS_PROGRAM).install_read_handler(0x2fe446, 0x2fe447, read16_delegate(FUNC(sma_prot_device::prot_9a37_r),this));
	m_bankdev = bankdev;

	sma_install_random_read_handler(maincpu, 0x2fffcc, 0x2ffff0);
}


void sma_prot_device::garouh_install_protection(cpu_device* maincpu, neogeo_banked_cart_device* bankdev)
{
	maincpu->space(AS_PROGRAM).install_write_handler(0x2fffc0, 0x2fffc1, write16_delegate(FUNC(sma_prot_device::garouh_bankswitch_w),this));
	maincpu->space(AS_PROGRAM).install_read_handler(0x2fe446, 0x2fe447, read16_delegate(FUNC(sma_prot_device::prot_9a37_r),this));
	m_bankdev = bankdev;

	sma_install_random_read_handler(maincpu, 0x2fffcc, 0x2ffff0);
}


void sma_prot_device::mslug3_install_protection(cpu_device* maincpu, neogeo_banked_cart_device* bankdev)
{
	maincpu->space(AS_PROGRAM).install_write_handler(0x2fffe4, 0x2fffe5, write16_delegate(FUNC(sma_prot_device::mslug3_bankswitch_w),this));
	maincpu->space(AS_PROGRAM).install_read_handler(0x2fe446, 0x2fe447, read16_delegate(FUNC(sma_prot_device::prot_9a37_r),this));
	m_bankdev = bankdev;

//  sma_install_random_read_handler(maincpu, 0x2ffff8, 0x2ffffa);
}


void sma_prot_device::kof2000_install_protection(cpu_device* maincpu, neogeo_banked_cart_device* bankdev)
{
	maincpu->space(AS_PROGRAM).install_write_handler(0x2fffec, 0x2fffed, write16_delegate(FUNC(sma_prot_device::kof2000_bankswitch_w),this));
	maincpu->space(AS_PROGRAM).install_read_handler(0x2fe446, 0x2fe447, read16_delegate(FUNC(sma_prot_device::prot_9a37_r),this));
	m_bankdev = bankdev;

	sma_install_random_read_handler(maincpu, 0x2fffd8, 0x2fffda);
}



/* kof99, garou, garouh, mslug3 and kof2000 have and SMA chip which contains program code and decrypts the 68k roms */
void sma_prot_device::kof99_decrypt_68k(UINT8* base)
{
	UINT16 *rom;
	int i,j;

	rom = (UINT16 *)(base + 0x100000);
	/* swap data lines on the whole ROMs */
	for (i = 0;i < 0x800000/2;i++)
	{
		rom[i] = BITSWAP16(rom[i],13,7,3,0,9,4,5,6,1,12,8,14,10,11,2,15);
	}

	/* swap address lines for the banked part */
	for (i = 0;i < 0x600000/2;i+=0x800/2)
	{
		UINT16 buffer[0x800/2];
		memcpy(buffer,&rom[i],0x800);
		for (j = 0;j < 0x800/2;j++)
		{
			rom[i+j] = buffer[BITSWAP24(j,23,22,21,20,19,18,17,16,15,14,13,12,11,10,6,2,4,9,8,3,1,7,0,5)];
		}
	}

	/* swap address lines & relocate fixed part */
	rom = (UINT16 *)base;
	for (i = 0;i < 0x0c0000/2;i++)
	{
		rom[i] = rom[0x700000/2 + BITSWAP24(i,23,22,21,20,19,18,11,6,14,17,16,5,8,10,12,0,4,3,2,7,9,15,13,1)];
	}
}


void sma_prot_device::garou_decrypt_68k(UINT8* base)
{
	UINT16 *rom;
	int i,j;

	/* thanks to Razoola and Mr K for the info */
	rom = (UINT16 *)(base + 0x100000);
	/* swap data lines on the whole ROMs */
	for (i = 0;i < 0x800000/2;i++)
	{
		rom[i] = BITSWAP16(rom[i],13,12,14,10,8,2,3,1,5,9,11,4,15,0,6,7);
	}

	/* swap address lines & relocate fixed part */
	rom = (UINT16 *)base;
	for (i = 0;i < 0x0c0000/2;i++)
	{
		rom[i] = rom[0x710000/2 + BITSWAP24(i,23,22,21,20,19,18,4,5,16,14,7,9,6,13,17,15,3,1,2,12,11,8,10,0)];
	}

	/* swap address lines for the banked part */
	rom = (UINT16 *)(base + 0x100000);
	for (i = 0;i < 0x800000/2;i+=0x8000/2)
	{
		UINT16 buffer[0x8000/2];
		memcpy(buffer,&rom[i],0x8000);
		for (j = 0;j < 0x8000/2;j++)
		{
			rom[i+j] = buffer[BITSWAP24(j,23,22,21,20,19,18,17,16,15,14,9,4,8,3,13,6,2,7,0,12,1,11,10,5)];
		}
	}
}


void sma_prot_device::garouh_decrypt_68k(UINT8* base)
{
	UINT16 *rom;
	int i,j;

	/* thanks to Razoola and Mr K for the info */
	rom = (UINT16 *)(base + 0x100000);
	/* swap data lines on the whole ROMs */
	for (i = 0;i < 0x800000/2;i++)
	{
		rom[i] = BITSWAP16(rom[i],14,5,1,11,7,4,10,15,3,12,8,13,0,2,9,6);
	}

	/* swap address lines & relocate fixed part */
	rom = (UINT16 *)base;
	for (i = 0;i < 0x0c0000/2;i++)
	{
		rom[i] = rom[0x7f8000/2 + BITSWAP24(i,23,22,21,20,19,18,5,16,11,2,6,7,17,3,12,8,14,4,0,9,1,10,15,13)];
	}

	/* swap address lines for the banked part */
	rom = (UINT16 *)(base + 0x100000);
	for (i = 0;i < 0x800000/2;i+=0x8000/2)
	{
		UINT16 buffer[0x8000/2];
		memcpy(buffer,&rom[i],0x8000);
		for (j = 0;j < 0x8000/2;j++)
		{
			rom[i+j] = buffer[BITSWAP24(j,23,22,21,20,19,18,17,16,15,14,12,8,1,7,11,3,13,10,6,9,5,4,0,2)];
		}
	}
}


void sma_prot_device::mslug3_decrypt_68k(UINT8* base)
{
	UINT16 *rom;
	int i,j;

	/* thanks to Razoola and Mr K for the info */
	rom = (UINT16 *)(base + 0x100000);
	/* swap data lines on the whole ROMs */
	for (i = 0;i < 0x800000/2;i++)
	{
		rom[i] = BITSWAP16(rom[i],4,11,14,3,1,13,0,7,2,8,12,15,10,9,5,6);
	}

	/* swap address lines & relocate fixed part */
	rom = (UINT16 *)base;
	for (i = 0;i < 0x0c0000/2;i++)
	{
		rom[i] = rom[0x5d0000/2 + BITSWAP24(i,23,22,21,20,19,18,15,2,1,13,3,0,9,6,16,4,11,5,7,12,17,14,10,8)];
	}

	/* swap address lines for the banked part */
	rom = (UINT16 *)(base + 0x100000);
	for (i = 0;i < 0x800000/2;i+=0x10000/2)
	{
		UINT16 buffer[0x10000/2];
		memcpy(buffer,&rom[i],0x10000);
		for (j = 0;j < 0x10000/2;j++)
		{
			rom[i+j] = buffer[BITSWAP24(j,23,22,21,20,19,18,17,16,15,2,11,0,14,6,4,13,8,9,3,10,7,5,12,1)];
		}
	}
}


void sma_prot_device::kof2000_decrypt_68k(UINT8* base)
{
	UINT16 *rom;
	int i,j;

	/* thanks to Razoola and Mr K for the info */
	rom = (UINT16 *)(base + 0x100000);
	/* swap data lines on the whole ROMs */
	for (i = 0;i < 0x800000/2;i++)
	{
		rom[i] = BITSWAP16(rom[i],12,8,11,3,15,14,7,0,10,13,6,5,9,2,1,4);
	}

	/* swap address lines for the banked part */
	for (i = 0;i < 0x63a000/2;i+=0x800/2)
	{
		UINT16 buffer[0x800/2];
		memcpy(buffer,&rom[i],0x800);
		for (j = 0;j < 0x800/2;j++)
		{
			rom[i+j] = buffer[BITSWAP24(j,23,22,21,20,19,18,17,16,15,14,13,12,11,10,4,1,3,8,6,2,7,0,9,5)];
		}
	}

	/* swap address lines & relocate fixed part */
	rom = (UINT16 *)base;
	for (i = 0;i < 0x0c0000/2;i++)
	{
		rom[i] = rom[0x73a000/2 + BITSWAP24(i,23,22,21,20,19,18,8,4,15,13,3,14,16,2,6,17,7,12,10,0,5,11,1,9)];
	}
}
