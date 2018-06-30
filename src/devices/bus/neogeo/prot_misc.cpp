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


DEFINE_DEVICE_TYPE(NEOBOOT_PROT, neoboot_prot_device, "ngboot_prot", "Neo Geo Bootleg Protection(s)")


neoboot_prot_device::neoboot_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NEOBOOT_PROT, tag, owner, clock)
{
}


void neoboot_prot_device::device_start()
{
}

void neoboot_prot_device::device_reset()
{
}




/* General Bootleg Functions - used by more than 1 game */

void neoboot_prot_device::cx_decrypt(uint8_t*sprrom, uint32_t sprrom_size)
{
	int cx_size = sprrom_size;
	uint8_t *rom = sprrom;
	std::vector<uint8_t> buf(cx_size);

	memcpy(&buf[0], rom, cx_size);

	for (int i = 0; i < cx_size / 0x40; i++)
		memcpy(&rom[i * 0x40], &buf[(i ^ 1) * 0x40], 0x40);
}


void neoboot_prot_device::sx_decrypt(uint8_t* fixed, uint32_t fixed_size, int value)
{
	int sx_size = fixed_size;
	uint8_t *rom = fixed;

	if (value == 1)
	{
		std::vector<uint8_t> buf(sx_size);
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
			rom[i] = bitswap<8>(rom[i], 7, 6, 0, 4, 3, 2, 1, 5);
	}
}



/* The King of Fighters '97 Oroshi Plus 2003 (bootleg) */

void neoboot_prot_device::kof97oro_px_decode(uint8_t* cpurom, uint32_t cpurom_size)
{
	std::vector<uint16_t> tmp(0x500000);
	uint16_t *src = (uint16_t*)cpurom;

	for (int i = 0; i < 0x500000/2; i++)
		tmp[i] = src[i ^ 0x7ffef];

	memcpy(src, &tmp[0], 0x500000);
}



/* The King of Fighters 10th Anniversary Extra Plus (The King of Fighters 2002 bootleg) */

void neoboot_prot_device::kf10thep_px_decrypt(uint8_t* cpurom, uint32_t cpurom_size)
{
	uint16_t *rom = (uint16_t*)cpurom;
	std::vector<uint16_t> buf(0x100000/2);

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

void neoboot_prot_device::kf2k5uni_px_decrypt(uint8_t* cpurom, uint32_t cpurom_size)
{
	uint8_t *src = cpurom;
	uint8_t dst[0x80];

	for (int i = 0; i < 0x800000; i += 0x80)
	{
		for (int j = 0; j < 0x80; j += 2)
		{
			int ofst = bitswap<8>(j, 0, 3, 4, 5, 6, 1, 2, 7);
			memcpy(&dst[j], src + i + ofst, 2);
		}
		memcpy(src + i, &dst[0], 0x80);
	}

	memcpy(src, src + 0x600000, 0x100000); // Seems to be the same as kof10th
}


void neoboot_prot_device::kf2k5uni_sx_decrypt(uint8_t* fixedrom, uint32_t fixedrom_size)
{
	uint8_t *srom = fixedrom;

	for (int i = 0; i < 0x20000; i++)
		srom[i] = bitswap<8>(srom[i], 4, 5, 6, 7, 0, 1, 2, 3);
}

void neoboot_prot_device::kf2k5uni_mx_decrypt(uint8_t* audiorom, uint32_t audiorom_size)
{
	uint8_t *mrom = audiorom;

	for (int i = 0; i < 0x30000; i++)
		mrom[i] = bitswap<8>(mrom[i], 4, 5, 6, 7, 0, 1, 2, 3);
}



/* King of Fighters Special Edition 2004 (bootleg of King of Fighters 2002) */

void neoboot_prot_device::decrypt_kof2k4se_68k(uint8_t* cpurom, uint32_t cpurom_size)
{
	uint8_t *src = cpurom + 0x100000;
	std::vector<uint8_t> dst(0x400000);
	static const int sec[] = {0x300000,0x200000,0x100000,0x000000};
	memcpy(&dst[0], src, 0x400000);

	for (int i = 0; i < 4; ++i)
		memcpy(src + i * 0x100000, &dst[sec[i]], 0x100000);
}


/* Lansquenet 2004 (Shock Troopers - 2nd Squad bootleg) */

void neoboot_prot_device::lans2004_vx_decrypt(uint8_t* ymsndrom, uint32_t ymsndrom_size)
{
	uint8_t *rom = ymsndrom;
	for (int i = 0; i < 0xA00000; i++)
		rom[i] = bitswap<8>(rom[i], 0, 1, 5, 4, 3, 2, 6, 7);
}

void neoboot_prot_device::lans2004_decrypt_68k(uint8_t* cpurom, uint32_t cpurom_size)
{
	// Descrambling P ROMs - Thanks to Razoola for the info
	uint8_t *src = cpurom;
	uint16_t *rom = (uint16_t*)cpurom;

	static const int sec[] = { 0x3, 0x8, 0x7, 0xc, 0x1, 0xa, 0x6, 0xd };
	std::vector<uint8_t> dst(0x600000);

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

void neoboot_prot_device::samsho5b_px_decrypt(uint8_t* cpurom, uint32_t cpurom_size)
{
	int px_size = cpurom_size;
	uint8_t *rom = cpurom;
	std::vector<uint8_t> buf(px_size);

	memcpy(&buf[0], rom, px_size);

	for (int i = 0; i < px_size / 2; i++)
	{
		int ofst = bitswap<8>((i & 0x000ff), 7, 6, 5, 4, 3, 0, 1, 2);
		ofst += (i & 0xfffff00);
		ofst ^= 0x060005;

		memcpy(&rom[i * 2], &buf[ofst * 2], 0x02);
	}

	memcpy(&buf[0], rom, px_size);

	memcpy(&rom[0x000000], &buf[0x700000], 0x100000);
	memcpy(&rom[0x100000], &buf[0x000000], 0x700000);
}


void neoboot_prot_device::samsho5b_vx_decrypt(uint8_t* ymsndrom, uint32_t ymsndrom_size)
{
	int vx_size = ymsndrom_size;
	uint8_t *rom = ymsndrom;

	for (int i = 0; i < vx_size; i++)
		rom[i] = bitswap<8>(rom[i], 0, 1, 5, 4, 3, 2, 6, 7);
}




/* Metal Slug 5 Plus (bootleg) */

READ16_MEMBER( neoboot_prot_device::mslug5p_prot_r )
{
	logerror("%s access protected\n", machine().describe_context());
	return 0xa0;
}

// FIXME: temporarily moved to the driver, through mslug5p_bank_base() below
/*
WRITE16_MEMBER( neoboot_prot_device::ms5plus_bankswitch_w )
{
    int bankaddress;
    logerror("offset: %06x %s set banking %04x\n",offset,machine().describe_context(),data);
    if ((offset == 0) && (data == 0xa0))
    {
        bankaddress = 0xa0;
        m_bankdev->neogeo_set_main_cpu_bank_address(bankaddress);
        logerror("offset: %06x %s set banking %04x\n\n",offset,machine().describe_context(),bankaddress);
    }
    else if(offset == 2)
    {
        data = data >> 4;
        //data = data & 7;
        bankaddress = data * 0x100000;
        m_bankdev->neogeo_set_main_cpu_bank_address(bankaddress);
        logerror("offset: %06x %s set banking %04x\n\n",offset,machine().describe_context(),bankaddress);
    }
}
*/

uint32_t neoboot_prot_device::mslug5p_bank_base(uint16_t sel)
{
	sel = sel >> 4;
	//sel = sel & 7;
	return sel * 0x100000;
}


/* The King of Gladiator (The King of Fighters '97 bootleg) */

// The protection patching here may be incomplete - Thanks to Razoola for the info

void neoboot_prot_device::kog_px_decrypt(uint8_t* cpurom, uint32_t cpurom_size)
{
	// the protection chip does some *very* strange things to the rom
	uint8_t *src = cpurom;
	std::vector<uint8_t> dst(0x600000);
	uint16_t *rom = (uint16_t *)cpurom;
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

void neoboot_prot_device::svcboot_px_decrypt(uint8_t* cpurom, uint32_t cpurom_size)
{
	static const uint8_t sec[] = { 0x06, 0x07, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00 };
	int size = cpurom_size;
	uint8_t *src = cpurom;
	std::vector<uint8_t> dst(size);

	for (int i = 0; i < size / 0x100000; i++)
		memcpy(&dst[i * 0x100000], &src[sec[i] * 0x100000], 0x100000);

	for (int i = 0; i < size / 2; i++)
	{
		int ofst = bitswap<8>((i & 0x0000ff), 7, 6, 1, 0, 3, 2, 5, 4);
		ofst += (i & 0xffff00);
		memcpy(&src[i * 2], &dst[ofst * 2], 0x02);
	}
}

void neoboot_prot_device::svcboot_cx_decrypt(uint8_t* sprrom, uint32_t sprrom_size)
{
	static const uint8_t idx_tbl[ 0x10 ] = { 0, 1, 0, 1, 2, 3, 2, 3, 3, 4, 3, 4, 4, 5, 4, 5, };
	static const uint8_t bitswap4_tbl[ 6 ][ 4 ] = {
		{ 3, 0, 1, 2 },
		{ 2, 3, 0, 1 },
		{ 1, 2, 3, 0 },
		{ 0, 1, 2, 3 },
		{ 3, 2, 1, 0 },
		{ 3, 0, 2, 1 },
	};
	int size = sprrom_size;
	uint8_t *src = sprrom;
	std::vector<uint8_t> dst(size);

	memcpy(&dst[0], src, size);
	for (int i = 0; i < size / 0x80; i++)
	{
		int idx = idx_tbl[(i & 0xf00) >> 8];
		int bit0 = bitswap4_tbl[idx][0];
		int bit1 = bitswap4_tbl[idx][1];
		int bit2 = bitswap4_tbl[idx][2];
		int bit3 = bitswap4_tbl[idx][3];
		int ofst = bitswap<8>((i & 0x0000ff), 7, 6, 5, 4, bit3, bit2, bit1, bit0);
		ofst += (i & 0xfffff00);
		memcpy(&src[i * 0x80], &dst[ofst * 0x80], 0x80);
	}
}


/* SNK vs. CAPCOM SVC CHAOS Plus (bootleg set 1) */

void neoboot_prot_device::svcplus_px_decrypt(uint8_t* cpurom, uint32_t cpurom_size)
{
	static const int sec[] = { 0x00, 0x03, 0x02, 0x05, 0x04, 0x01 };
	int size = cpurom_size;
	uint8_t *src = cpurom;
	std::vector<uint8_t> dst(size);

	memcpy(&dst[0], src, size);
	for (int i = 0; i < size / 2; i++)
	{
		int ofst = bitswap<24>((i & 0xfffff), 0x17, 0x16, 0x15, 0x14, 0x13, 0x00, 0x01, 0x02,
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


void neoboot_prot_device::svcplus_px_hack(uint8_t* cpurom, uint32_t cpurom_size)
{
	/* patched by the protection chip? */
	uint16_t *mem16 = (uint16_t *)cpurom;
	mem16[0x0f8016/2] = 0x33c1;
}


/* SNK vs. CAPCOM SVC CHAOS Plus (bootleg set 2) */

void neoboot_prot_device::svcplusa_px_decrypt(uint8_t* cpurom, uint32_t cpurom_size)
{
	static const int sec[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x00 };
	int size = cpurom_size;
	uint8_t *src = cpurom;
	std::vector<uint8_t> dst(size);

	memcpy(&dst[0], src, size);
	for (int i = 0; i < 6; i++)
		memcpy(&src[i * 0x100000], &dst[sec[i] * 0x100000], 0x100000);
}


/* SNK vs. CAPCOM SVC CHAOS Super Plus (bootleg) */

void neoboot_prot_device::svcsplus_px_decrypt(uint8_t* cpurom, uint32_t cpurom_size)
{
	static const int sec[] = { 0x06, 0x07, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00 };
	int size = cpurom_size;
	uint8_t *src = cpurom;
	std::vector<uint8_t> dst(size);

	memcpy(&dst[0], src, size);
	for (int i = 0; i < size / 2; i++)
	{
		int ofst = bitswap<16>((i & 0x007fff), 0x0f, 0x00, 0x08, 0x09, 0x0b, 0x0a, 0x0c, 0x0d,
								0x04, 0x03, 0x01, 0x07, 0x06, 0x02, 0x05, 0x0e);

		ofst += (i & 0x078000);
		ofst += sec[(i & 0xf80000) >> 19] << 19;
		memcpy(&src[i * 2], &dst[ofst * 2], 0x02);
	}
}

void neoboot_prot_device::svcsplus_px_hack(uint8_t* cpurom, uint32_t cpurom_size)
{
	/* patched by the protection chip? */
	uint16_t *mem16 = (uint16_t *)cpurom;
	mem16[0x9e90/2] = 0x000f;
	mem16[0x9e92/2] = 0xc9c0;
	mem16[0xa10c/2] = 0x4eb9;
	mem16[0xa10e/2] = 0x000e;
	mem16[0xa110/2] = 0x9750;
}


/* The King of Fighters 2002 (bootleg) */

void neoboot_prot_device::kof2002b_gfx_decrypt(uint8_t *src, int size)
{
	static const uint8_t t[8][6] =
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

	std::vector<uint8_t> dst(0x10000);

	for (int i = 0; i < size; i += 0x10000)
	{
		memcpy(&dst[0], src + i, 0x10000);

		for (int j = 0; j < 0x200; j++)
		{
			int n = (j & 0x38) >> 3;
			int ofst = bitswap<16>(j, 15, 14, 13, 12, 11, 10, 9, t[n][0], t[n][1], t[n][2], 5, 4, 3, t[n][3], t[n][4], t[n][5]);
			memcpy(src + i + ofst * 128, &dst[j * 128], 128);
		}
	}
}


/* The King of Fighters 2002 Magic Plus (bootleg) */

void neoboot_prot_device::kf2k2mp_decrypt(uint8_t* cpurom, uint32_t cpurom_size)
{
	uint8_t *src = cpurom;
	uint8_t dst[0x80];

	memmove(src, src + 0x300000, 0x500000);

	for (int i = 0; i < 0x800000; i+=0x80)
	{
		for (int j = 0; j < 0x80 / 2; j++)
		{
			int ofst = bitswap<8>( j, 6, 7, 2, 3, 4, 5, 0, 1 );
			memcpy(dst + j * 2, src + i + ofst * 2, 2);
		}
		memcpy(src + i, dst, 0x80);
	}
}


/* The King of Fighters 2002 Magic Plus II (bootleg) */

void neoboot_prot_device::kf2k2mp2_px_decrypt(uint8_t* cpurom, uint32_t cpurom_size)
{
	uint8_t *src = cpurom;
	std::vector<uint8_t> dst(0x600000);

	memcpy(&dst[0x000000], &src[0x1C0000], 0x040000);
	memcpy(&dst[0x040000], &src[0x140000], 0x080000);
	memcpy(&dst[0x0C0000], &src[0x100000], 0x040000);
	memcpy(&dst[0x100000], &src[0x200000], 0x400000);
	memcpy(&src[0x000000], &dst[0x000000], 0x600000);
}



/* The King of Fighters 10th Anniversary (The King of Fighters 2002 bootleg) */

void neoboot_prot_device::kof10th_decrypt(uint8_t* cpurom, uint32_t cpurom_size)
{
	std::vector<uint8_t> dst(0x900000);
	uint8_t *src = cpurom;

	memcpy(&dst[0x000000], src + 0x700000, 0x100000); // Correct (Verified in Uni-bios)
	memcpy(&dst[0x100000], src + 0x000000, 0x800000);

	for (int i = 0; i < 0x900000; i++)
	{
		int j = bitswap<24>(i,23,22,21,20,19,18,17,16,15,14,13,12,11,2,9,8,7,1,5,4,3,10,6,0);
		src[j] = dst[i];
	}

	// Altera protection chip patches these over P ROM
	((uint16_t*)src)[0x0124/2] = 0x000d; // Enables XOR for RAM moves, forces SoftDIPs, and USA region
	((uint16_t*)src)[0x0126/2] = 0xf7a8;

	((uint16_t*)src)[0x8bf4/2] = 0x4ef9; // Run code to change "S" data
	((uint16_t*)src)[0x8bf6/2] = 0x000d;
	((uint16_t*)src)[0x8bf8/2] = 0xf980;
}
