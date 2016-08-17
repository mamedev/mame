// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli

/***************************************************************************

 Neo-Geo hardware encryption and protection used on bootleg cartridges

 Many of the NeoGeo bootlegs use their own form of encryption and
 protection, presumably to make them harder for other bootleggers to
 copy.  This encryption often involves non-trivial scrambling of the
 program roms and the games are protected using an Altera chip which
 provides some kind of rom overlay, patching parts of the code.
 The graphics roms are usually scrambled in a different way to the
 official SNK cartridges too.

 Here we collect functions to emulate some of the protection devices used.

 TODO: split different devices according to the chip responsible for them!

 ***************************************************************************/

#include "emu.h"
#include "prot_misc.h"


extern const device_type NEOBOOT_PROT = &device_creator<neoboot_prot_device>;


neoboot_prot_device::neoboot_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, NEOBOOT_PROT, "Neo Geo Bootleg Protection(s)", tag, owner, clock, "ngboot_prot", __FILE__)
{}


void neoboot_prot_device::device_start()
{
}

void neoboot_prot_device::device_reset()
{
}




/* General Bootleg Functions - used by more than 1 game */

void neoboot_prot_device::cx_decrypt(UINT8*sprrom, UINT32 sprrom_size)
{
	int cx_size = sprrom_size;
	UINT8 *rom = sprrom;
	dynamic_buffer buf(cx_size);

	memcpy(&buf[0], rom, cx_size);

	for (int i = 0; i < cx_size / 0x40; i++)
		memcpy(&rom[i * 0x40], &buf[(i ^ 1) * 0x40], 0x40);
}


void neoboot_prot_device::sx_decrypt(UINT8* fixed, UINT32 fixed_size, int value)
{
	int sx_size = fixed_size;
	UINT8 *rom = fixed;

	if (value == 1)
	{
		dynamic_buffer buf(sx_size);
		memcpy(&buf[0], rom, sx_size);

		for (int i = 0; i < sx_size; i += 0x10)
		{
			memcpy(&rom[i], &buf[i + 8], 8);
			memcpy(&rom[i + 8], &buf[i], 8);
		}
	}
	else if (value == 2)
	{
		for (int i = 0; i < sx_size; i++)
			rom[i] = BITSWAP8(rom[i], 7, 6, 0, 4, 3, 2, 1, 5);
	}
}



/* The King of Fighters '97 Oroshi Plus 2003 (bootleg) */

void neoboot_prot_device::kof97oro_px_decode(UINT8* cpurom, UINT32 cpurom_size)
{
	std::vector<UINT16> tmp(0x500000);
	UINT16 *src = (UINT16*)cpurom;

	for (int i = 0; i < 0x500000/2; i++)
		tmp[i] = src[i ^ 0x7ffef];

	memcpy(src, &tmp[0], 0x500000);
}



/* The King of Fighters 10th Anniversary Extra Plus (The King of Fighters 2002 bootleg) */

void neoboot_prot_device::kf10thep_px_decrypt(UINT8* cpurom, UINT32 cpurom_size)
{
	UINT16 *rom = (UINT16*)cpurom;
	std::vector<UINT16> buf(0x100000/2);

	memcpy(&buf[0x000000/2], &rom[0x060000/2], 0x20000);
	memcpy(&buf[0x020000/2], &rom[0x100000/2], 0x20000);
	memcpy(&buf[0x040000/2], &rom[0x0e0000/2], 0x20000);
	memcpy(&buf[0x060000/2], &rom[0x180000/2], 0x20000);
	memcpy(&buf[0x080000/2], &rom[0x020000/2], 0x20000);
	memcpy(&buf[0x0a0000/2], &rom[0x140000/2], 0x20000);
	memcpy(&buf[0x0c0000/2], &rom[0x0c0000/2], 0x20000);
	memcpy(&buf[0x0e0000/2], &rom[0x1a0000/2], 0x20000);
	memcpy(&buf[0x0002e0/2], &rom[0x0402e0/2], 0x6a);  // copy banked code to a new memory region
	memcpy(&buf[0x0f92bc/2], &rom[0x0492bc/2], 0xb9e); // copy banked code to a new memory region
	memcpy(rom, &buf[0], 0x100000);

	for (int i = 0xf92bc/2; i < 0xf9e58/2; i++)
	{
		if (rom[i+0] == 0x4eb9 && rom[i+1] == 0x0000) rom[i+1] = 0x000F; // correct JSR in moved code
		if (rom[i+0] == 0x4ef9 && rom[i+1] == 0x0000) rom[i+1] = 0x000F; // correct JMP in moved code
	}
	rom[0x00342/2] = 0x000f;

	memmove(&rom[0x100000/2], &rom[0x200000/2], 0x600000);
}


/* The King of Fighters 10th Anniversary 2005 Unique (The King of Fighters 2002 bootleg) */

void neoboot_prot_device::kf2k5uni_px_decrypt(UINT8* cpurom, UINT32 cpurom_size)
{
	UINT8 *src = cpurom;
	UINT8 dst[0x80];

	for (int i = 0; i < 0x800000; i += 0x80)
	{
		for (int j = 0; j < 0x80; j += 2)
		{
			int ofst = BITSWAP8(j, 0, 3, 4, 5, 6, 1, 2, 7);
			memcpy(&dst[j], src + i + ofst, 2);
		}
		memcpy(src + i, &dst[0], 0x80);
	}

	memcpy(src, src + 0x600000, 0x100000); // Seems to be the same as kof10th
}


void neoboot_prot_device::kf2k5uni_sx_decrypt(UINT8* fixedrom, UINT32 fixedrom_size)
{
	UINT8 *srom = fixedrom;

	for (int i = 0; i < 0x20000; i++)
		srom[i] = BITSWAP8(srom[i], 4, 5, 6, 7, 0, 1, 2, 3);
}

void neoboot_prot_device::kf2k5uni_mx_decrypt(UINT8* audiorom, UINT32 audiorom_size)
{
	UINT8 *mrom = audiorom;

	for (int i = 0; i < 0x30000; i++)
		mrom[i] = BITSWAP8(mrom[i], 4, 5, 6, 7, 0, 1, 2, 3);
}



/* King of Fighters Special Edition 2004 (bootleg of King of Fighters 2002) */

void neoboot_prot_device::decrypt_kof2k4se_68k(UINT8* cpurom, UINT32 cpurom_size)
{
	UINT8 *src = cpurom + 0x100000;
	dynamic_buffer dst(0x400000);
	static const int sec[] = {0x300000,0x200000,0x100000,0x000000};
	memcpy(&dst[0], src, 0x400000);

	for (int i = 0; i < 4; ++i)
		memcpy(src + i * 0x100000, &dst[sec[i]], 0x100000);
}


/* Lansquenet 2004 (Shock Troopers - 2nd Squad bootleg) */

void neoboot_prot_device::lans2004_vx_decrypt(UINT8* ymsndrom, UINT32 ymsndrom_size)
{
	UINT8 *rom = ymsndrom;
	for (int i = 0; i < 0xA00000; i++)
		rom[i] = BITSWAP8(rom[i], 0, 1, 5, 4, 3, 2, 6, 7);
}

void neoboot_prot_device::lans2004_decrypt_68k(UINT8* cpurom, UINT32 cpurom_size)
{
	// Descrambling P ROMs - Thanks to Razoola for the info
	UINT8 *src = cpurom;
	UINT16 *rom = (UINT16*)cpurom;

	static const int sec[] = { 0x3, 0x8, 0x7, 0xc, 0x1, 0xa, 0x6, 0xd };
	dynamic_buffer dst(0x600000);

	for (int i = 0; i < 8; i++)
		memcpy (&dst[i * 0x20000], src + sec[i] * 0x20000, 0x20000);

	memcpy (&dst[0x0bbb00], src + 0x045b00, 0x001710);
	memcpy (&dst[0x02fff0], src + 0x1a92be, 0x000010);
	memcpy (&dst[0x100000], src + 0x200000, 0x400000);
	memcpy (src, &dst[0], 0x600000);

	for (int i = 0xbbb00/2; i < 0xbe000/2; i++)
	{
		if ((((rom[i] & 0xffbf)==0x4eb9) || ((rom[i] & 0xffbf)==0x43b9)) && (rom[i+1]==0x0000))
		{
			rom[i + 1] = 0x000b;
			rom[i + 2] += 0x6000;
		}
	}

	/* Patched by protection chip (Altera) ? */
	rom[0x2d15c/2] = 0x000b;
	rom[0x2d15e/2] = 0xbb00;
	rom[0x2d1e4/2] = 0x6002;
	rom[0x2ea7e/2] = 0x6002;
	rom[0xbbcd0/2] = 0x6002;
	rom[0xbbdf2/2] = 0x6002;
	rom[0xbbe42/2] = 0x6002;
}


/* Samurai Shodown V / Samurai Spirits Zero (bootleg) */

void neoboot_prot_device::samsho5b_px_decrypt(UINT8* cpurom, UINT32 cpurom_size)
{
	int px_size = cpurom_size;
	UINT8 *rom = cpurom;
	dynamic_buffer buf(px_size);

	memcpy(&buf[0], rom, px_size);

	for (int i = 0; i < px_size / 2; i++)
	{
		int ofst = BITSWAP8((i & 0x000ff), 7, 6, 5, 4, 3, 0, 1, 2);
		ofst += (i & 0xfffff00);
		ofst ^= 0x060005;

		memcpy(&rom[i * 2], &buf[ofst * 2], 0x02);
	}

	memcpy(&buf[0], rom, px_size);

	memcpy(&rom[0x000000], &buf[0x700000], 0x100000);
	memcpy(&rom[0x100000], &buf[0x000000], 0x700000);
}


void neoboot_prot_device::samsho5b_vx_decrypt(UINT8* ymsndrom, UINT32 ymsndrom_size)
{
	int vx_size = ymsndrom_size;
	UINT8 *rom = ymsndrom;

	for (int i = 0; i < vx_size; i++)
		rom[i] = BITSWAP8(rom[i], 0, 1, 5, 4, 3, 2, 6, 7);
}




/* Metal Slug 5 Plus (bootleg) */

READ16_MEMBER( neoboot_prot_device::mslug5p_prot_r )
{
	logerror("PC %06x: access protected\n", space.device().safe_pc());
	return 0xa0;
}

// FIXME: temporarily moved to the driver, through mslug5p_bank_base() below
/*
WRITE16_MEMBER( neoboot_prot_device::ms5plus_bankswitch_w )
{
    int bankaddress;
    logerror("offset: %06x PC %06x: set banking %04x\n",offset,space.device().safe_pc(),data);
    if ((offset == 0) && (data == 0xa0))
    {
        bankaddress = 0xa0;
        m_bankdev->neogeo_set_main_cpu_bank_address(bankaddress);
        logerror("offset: %06x PC %06x: set banking %04x\n\n",offset,space.device().safe_pc(),bankaddress);
    }
    else if(offset == 2)
    {
        data = data >> 4;
        //data = data & 7;
        bankaddress = data * 0x100000;
        m_bankdev->neogeo_set_main_cpu_bank_address(bankaddress);
        logerror("offset: %06x PC %06x: set banking %04x\n\n",offset,space.device().safe_pc(),bankaddress);
    }
}
*/

UINT32 neoboot_prot_device::mslug5p_bank_base(UINT16 sel)
{
	sel = sel >> 4;
	//sel = sel & 7;
	return sel * 0x100000;
}


/* The King of Gladiator (The King of Fighters '97 bootleg) */

// The protection patching here may be incomplete - Thanks to Razoola for the info

void neoboot_prot_device::kog_px_decrypt(UINT8* cpurom, UINT32 cpurom_size)
{
	// the protection chip does some *very* strange things to the rom
	UINT8 *src = cpurom;
	dynamic_buffer dst(0x600000);
	UINT16 *rom = (UINT16 *)cpurom;
	static const int sec[] = { 0x3, 0x8, 0x7, 0xc, 0x1, 0xa, 0x6, 0xd };

	for (int i = 0; i < 8; i++)
		memcpy (&dst[i * 0x20000], src + sec[i] * 0x20000, 0x20000);

	memcpy (&dst[0x0007a6], src + 0x0407a6, 0x000006);
	memcpy (&dst[0x0007c6], src + 0x0407c6, 0x000006);
	memcpy (&dst[0x0007e6], src + 0x0407e6, 0x000006);
	memcpy (&dst[0x090000], src + 0x040000, 0x004000);
	memcpy (&dst[0x100000], src + 0x200000, 0x400000);
	memcpy (src, &dst[0], 0x600000);

	for (int i = 0x90000/2; i < 0x94000/2; i++)
	{
		if (((rom[i] & 0xffbf) == 0x4eb9 || rom[i] == 0x43f9) && !rom[i + 1])
			rom[i + 1] = 0x0009;

		if (rom[i] == 0x4eb8)
			rom[i] = 0x6100;
	}

	rom[0x007a8/2] = 0x0009;
	rom[0x007c8/2] = 0x0009;
	rom[0x007e8/2] = 0x0009;
	rom[0x93408/2] = 0xf168;
	rom[0x9340c/2] = 0xfb7a;
	rom[0x924ac/2] = 0x0009;
	rom[0x9251c/2] = 0x0009;
	rom[0x93966/2] = 0xffda;
	rom[0x93974/2] = 0xffcc;
	rom[0x93982/2] = 0xffbe;
	rom[0x93990/2] = 0xffb0;
	rom[0x9399e/2] = 0xffa2;
	rom[0x939ac/2] = 0xff94;
	rom[0x939ba/2] = 0xff86;
	rom[0x939c8/2] = 0xff78;
	rom[0x939d4/2] = 0xfa5c;
	rom[0x939e0/2] = 0xfa50;
	rom[0x939ec/2] = 0xfa44;
	rom[0x939f8/2] = 0xfa38;
	rom[0x93a04/2] = 0xfa2c;
	rom[0x93a10/2] = 0xfa20;
	rom[0x93a1c/2] = 0xfa14;
	rom[0x93a28/2] = 0xfa08;
	rom[0x93a34/2] = 0xf9fc;
	rom[0x93a40/2] = 0xf9f0;
	rom[0x93a4c/2] = 0xfd14;
	rom[0x93a58/2] = 0xfd08;
	rom[0x93a66/2] = 0xf9ca;
	rom[0x93a72/2] = 0xf9be;

}

/* SNK vs. CAPCOM SVC CHAOS (bootleg) */

void neoboot_prot_device::svcboot_px_decrypt(UINT8* cpurom, UINT32 cpurom_size)
{
	static const UINT8 sec[] = { 0x06, 0x07, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00 };
	int size = cpurom_size;
	UINT8 *src = cpurom;
	dynamic_buffer dst(size);

	for (int i = 0; i < size / 0x100000; i++)
		memcpy(&dst[i * 0x100000], &src[sec[i] * 0x100000], 0x100000);

	for (int i = 0; i < size / 2; i++)
	{
		int ofst = BITSWAP8((i & 0x0000ff), 7, 6, 1, 0, 3, 2, 5, 4);
		ofst += (i & 0xffff00);
		memcpy(&src[i * 2], &dst[ofst * 2], 0x02);
	}
}

void neoboot_prot_device::svcboot_cx_decrypt(UINT8* sprrom, UINT32 sprrom_size)
{
	static const UINT8 idx_tbl[ 0x10 ] = { 0, 1, 0, 1, 2, 3, 2, 3, 3, 4, 3, 4, 4, 5, 4, 5, };
	static const UINT8 bitswap4_tbl[ 6 ][ 4 ] = {
		{ 3, 0, 1, 2 },
		{ 2, 3, 0, 1 },
		{ 1, 2, 3, 0 },
		{ 0, 1, 2, 3 },
		{ 3, 2, 1, 0 },
		{ 3, 0, 2, 1 },
	};
	int size = sprrom_size;
	UINT8 *src = sprrom;
	dynamic_buffer dst(size);

	memcpy(&dst[0], src, size);
	for (int i = 0; i < size / 0x80; i++)
	{
		int idx = idx_tbl[(i & 0xf00) >> 8];
		int bit0 = bitswap4_tbl[idx][0];
		int bit1 = bitswap4_tbl[idx][1];
		int bit2 = bitswap4_tbl[idx][2];
		int bit3 = bitswap4_tbl[idx][3];
		int ofst = BITSWAP8((i & 0x0000ff), 7, 6, 5, 4, bit3, bit2, bit1, bit0);
		ofst += (i & 0xfffff00);
		memcpy(&src[i * 0x80], &dst[ofst * 0x80], 0x80);
	}
}


/* SNK vs. CAPCOM SVC CHAOS Plus (bootleg set 1) */

void neoboot_prot_device::svcplus_px_decrypt(UINT8* cpurom, UINT32 cpurom_size)
{
	static const int sec[] = { 0x00, 0x03, 0x02, 0x05, 0x04, 0x01 };
	int size = cpurom_size;
	UINT8 *src = cpurom;
	dynamic_buffer dst(size);

	memcpy(&dst[0], src, size);
	for (int i = 0; i < size / 2; i++)
	{
		int ofst = BITSWAP24((i & 0xfffff), 0x17, 0x16, 0x15, 0x14, 0x13, 0x00, 0x01, 0x02,
								0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08,
								0x07, 0x06, 0x05, 0x04, 0x03, 0x10, 0x11, 0x12);
		ofst ^= 0x0f0007;
		ofst += (i & 0xff00000);
		memcpy(&src[i * 0x02], &dst[ofst * 0x02], 0x02);
	}

	memcpy(&dst[0], src, size);
	for (int i = 0; i < 6; i++)
		memcpy(&src[i * 0x100000], &dst[sec[i] * 0x100000], 0x100000);
}


void neoboot_prot_device::svcplus_px_hack(UINT8* cpurom, UINT32 cpurom_size)
{
	/* patched by the protection chip? */
	UINT16 *mem16 = (UINT16 *)cpurom;
	mem16[0x0f8016/2] = 0x33c1;
}


/* SNK vs. CAPCOM SVC CHAOS Plus (bootleg set 2) */

void neoboot_prot_device::svcplusa_px_decrypt(UINT8* cpurom, UINT32 cpurom_size)
{
	static const int sec[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x00 };
	int size = cpurom_size;
	UINT8 *src = cpurom;
	dynamic_buffer dst(size);

	memcpy(&dst[0], src, size);
	for (int i = 0; i < 6; i++)
		memcpy(&src[i * 0x100000], &dst[sec[i] * 0x100000], 0x100000);
}


/* SNK vs. CAPCOM SVC CHAOS Super Plus (bootleg) */

void neoboot_prot_device::svcsplus_px_decrypt(UINT8* cpurom, UINT32 cpurom_size)
{
	static const int sec[] = { 0x06, 0x07, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00 };
	int size = cpurom_size;
	UINT8 *src = cpurom;
	dynamic_buffer dst(size);

	memcpy(&dst[0], src, size);
	for (int i = 0; i < size / 2; i++)
	{
		int ofst = BITSWAP16((i & 0x007fff), 0x0f, 0x00, 0x08, 0x09, 0x0b, 0x0a, 0x0c, 0x0d,
								0x04, 0x03, 0x01, 0x07, 0x06, 0x02, 0x05, 0x0e);

		ofst += (i & 0x078000);
		ofst += sec[(i & 0xf80000) >> 19] << 19;
		memcpy(&src[i * 2], &dst[ofst * 2], 0x02);
	}
}

void neoboot_prot_device::svcsplus_px_hack(UINT8* cpurom, UINT32 cpurom_size)
{
	/* patched by the protection chip? */
	UINT16 *mem16 = (UINT16 *)cpurom;
	mem16[0x9e90/2] = 0x000f;
	mem16[0x9e92/2] = 0xc9c0;
	mem16[0xa10c/2] = 0x4eb9;
	mem16[0xa10e/2] = 0x000e;
	mem16[0xa110/2] = 0x9750;
}


/* The King of Fighters 2002 (bootleg) */

void neoboot_prot_device::kof2002b_gfx_decrypt(UINT8 *src, int size)
{
	static const UINT8 t[8][6] =
	{
		{ 0, 8, 7, 6, 2, 1 },
		{ 1, 0, 8, 7, 6, 2 },
		{ 2, 1, 0, 8, 7, 6 },
		{ 6, 2, 1, 0, 8, 7 },
		{ 7, 6, 2, 1, 0, 8 },
		{ 0, 1, 2, 6, 7, 8 },
		{ 2, 1, 0, 6, 7, 8 },
		{ 8, 0, 7, 6, 2, 1 },
	};

	dynamic_buffer dst(0x10000);

	for (int i = 0; i < size; i += 0x10000)
	{
		memcpy(&dst[0], src + i, 0x10000);

		for (int j = 0; j < 0x200; j++)
		{
			int n = (j & 0x38) >> 3;
			int ofst = BITSWAP16(j, 15, 14, 13, 12, 11, 10, 9, t[n][0], t[n][1], t[n][2], 5, 4, 3, t[n][3], t[n][4], t[n][5]);
			memcpy(src + i + ofst * 128, &dst[j * 128], 128);
		}
	}
}


/* The King of Fighters 2002 Magic Plus (bootleg) */

void neoboot_prot_device::kf2k2mp_decrypt(UINT8* cpurom, UINT32 cpurom_size)
{
	UINT8 *src = cpurom;
	UINT8 dst[0x80];

	memmove(src, src + 0x300000, 0x500000);

	for (int i = 0; i < 0x800000; i+=0x80)
	{
		for (int j = 0; j < 0x80 / 2; j++)
		{
			int ofst = BITSWAP8( j, 6, 7, 2, 3, 4, 5, 0, 1 );
			memcpy(dst + j * 2, src + i + ofst * 2, 2);
		}
		memcpy(src + i, dst, 0x80);
	}
}


/* The King of Fighters 2002 Magic Plus II (bootleg) */

void neoboot_prot_device::kf2k2mp2_px_decrypt(UINT8* cpurom, UINT32 cpurom_size)
{
	UINT8 *src = cpurom;
	dynamic_buffer dst(0x600000);

	memcpy(&dst[0x000000], &src[0x1C0000], 0x040000);
	memcpy(&dst[0x040000], &src[0x140000], 0x080000);
	memcpy(&dst[0x0C0000], &src[0x100000], 0x040000);
	memcpy(&dst[0x100000], &src[0x200000], 0x400000);
	memcpy(&src[0x000000], &dst[0x000000], 0x600000);
}



/* The King of Fighters 10th Anniversary (The King of Fighters 2002 bootleg) */

void neoboot_prot_device::kof10th_decrypt(UINT8* cpurom, UINT32 cpurom_size)
{
	dynamic_buffer dst(0x900000);
	UINT8 *src = cpurom;

	memcpy(&dst[0x000000], src + 0x700000, 0x100000); // Correct (Verified in Uni-bios)
	memcpy(&dst[0x100000], src + 0x000000, 0x800000);

	for (int i = 0; i < 0x900000; i++)
	{
		int j = BITSWAP24(i,23,22,21,20,19,18,17,16,15,14,13,12,11,2,9,8,7,1,5,4,3,10,6,0);
		src[j] = dst[i];
	}

	// Altera protection chip patches these over P ROM
	((UINT16*)src)[0x0124/2] = 0x000d; // Enables XOR for RAM moves, forces SoftDIPs, and USA region
	((UINT16*)src)[0x0126/2] = 0xf7a8;

	((UINT16*)src)[0x8bf4/2] = 0x4ef9; // Run code to change "S" data
	((UINT16*)src)[0x8bf6/2] = 0x000d;
	((UINT16*)src)[0x8bf8/2] = 0xf980;
}
