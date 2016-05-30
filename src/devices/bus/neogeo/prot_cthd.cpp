// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli

#include "emu.h"
#include "prot_cthd.h"

extern const device_type CTHD_PROT = &device_creator<cthd_prot_device>;


cthd_prot_device::cthd_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, CTHD_PROT, "Neo Geo CTHD Protection (Bootleg)", tag, owner, clock, "cthd_prot", __FILE__)
{}


void cthd_prot_device::device_start()
{
}

void cthd_prot_device::device_reset()
{
}


/* Crouching Tiger Hidden Dragon 2003 (bootleg of King of Fighters 2001) */


/**************************

 decryption helpers

**************************/

// descrambling information from razoola
void cthd_prot_device::fix_do(UINT8* sprrom, UINT32 sprrom_size, int start, int end, int bit3shift, int bit2shift, int bit1shift, int bit0shift)
{
	int tilesize = 128;

	dynamic_buffer rom(16 * tilesize); // 16 tiles buffer
	UINT8* realrom = sprrom + start * tilesize;

	for (int i = 0; i < (end-start)/16; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			int offset = (BIT(j, 0) << bit0shift) +
						(BIT(j, 1) << bit1shift) +
						(BIT(j, 2) << bit2shift) +
						(BIT(j, 3) << bit3shift);
			memcpy(&rom[j * tilesize], realrom + offset * tilesize, tilesize);
		}
		memcpy(realrom, &rom[0], tilesize * 16);
		realrom += 16 * tilesize;
	}
}

void cthd_prot_device::gfx_address_fix(UINT8* sprrom, UINT32 sprrom_size, int start, int end)
{
	fix_do(sprrom, sprrom_size, start + 512 * 0, end + 512 * 0, 0,3,2,1);
	fix_do(sprrom, sprrom_size, start + 512 * 1, end + 512 * 1, 1,0,3,2);
	fix_do(sprrom, sprrom_size, start + 512 * 2, end + 512 * 2, 2,1,0,3);
	// skip 3 & 4
	fix_do(sprrom, sprrom_size, start + 512 * 5, end + 512 * 5, 0,1,2,3);
	fix_do(sprrom, sprrom_size, start + 512 * 6, end + 512 * 6, 0,1,2,3);
	fix_do(sprrom, sprrom_size, start + 512 * 7, end + 512 * 7, 0,2,3,1);
}

void cthd_prot_device::cthd2003_c(UINT8* sprrom, UINT32 sprrom_size, int pow)
{
	for (int i = 0; i <= 192; i += 8)
		gfx_address_fix(sprrom, sprrom_size, i * 512, i * 512 + 512);

	for (int i = 200; i <= 392; i += 8)
		gfx_address_fix(sprrom, sprrom_size, i * 512, i * 512 + 512);

	for (int i = 400; i <= 592; i += 8)
		gfx_address_fix(sprrom, sprrom_size, i * 512, i * 512 + 512);

	for (int i = 600; i <= 792; i += 8)
		gfx_address_fix(sprrom, sprrom_size, i * 512, i * 512 + 512);

	for (int i = 800; i <= 992; i += 8)
		gfx_address_fix(sprrom, sprrom_size, i * 512, i * 512 + 512);

	for (int i = 1000; i <= 1016; i += 8)
		gfx_address_fix(sprrom, sprrom_size, i * 512, i * 512 + 512);
}

/**************************

 protection / encryption

 **************************/

void cthd_prot_device::decrypt_cthd2003(UINT8* sprrom, UINT32 sprrom_size, UINT8* audiorom, UINT32 audiorom_size, UINT8* fixedrom, UINT32 fixedrom_size)
{
	UINT8 *romdata = fixedrom;
	dynamic_buffer tmp(8 * 128 * 128);

	memcpy(&tmp[8 *  0 * 128], romdata + 8 *  0 * 128, 8 * 32 * 128);
	memcpy(&tmp[8 * 32 * 128], romdata + 8 * 64 * 128, 8 * 32 * 128);
	memcpy(&tmp[8 * 64 * 128], romdata + 8 * 32 * 128, 8 * 32 * 128);
	memcpy(&tmp[8 * 96 * 128], romdata + 8 * 96 * 128, 8 * 32 * 128);
	memcpy(romdata, &tmp[0], 8 * 128 * 128);

	romdata = audiorom + 0x10000;
	memcpy(&tmp[8 *  0 * 128], romdata + 8 *  0 * 128, 8 * 32 * 128);
	memcpy(&tmp[8 * 32 * 128], romdata + 8 * 64 * 128, 8 * 32 * 128);
	memcpy(&tmp[8 * 64 * 128], romdata + 8 * 32 * 128, 8 * 32 * 128);
	memcpy(&tmp[8 * 96 * 128], romdata + 8 * 96 * 128, 8 * 32 * 128);
	memcpy(romdata, &tmp[0], 8 * 128 * 128);

	memcpy(romdata - 0x10000, romdata, 0x10000);

	cthd2003_c(sprrom, sprrom_size, 0);
}


// temporarily replaced by the get_bank_base functions below, until we clean up bankswitch implementation
/*
WRITE16_MEMBER( ngbootleg_prot_device::cthd2003_bankswitch_w )
{
    int bankaddress;
    static const int cthd2003_banks[8] =
    {
        1,0,1,0,1,0,3,2,
    };
    if (offset == 0)
    {
        bankaddress = 0x100000 + cthd2003_banks[data&7]*0x100000;
        m_bankdev->neogeo_set_main_cpu_bank_address(bankaddress);
    }
}
*/

UINT32 cthd_prot_device::get_bank_base(UINT16 sel)
{
	static const int cthd2003_banks[8] = { 1, 0, 1, 0, 1, 0, 3, 2 };
	return (cthd2003_banks[sel & 7] + 1) * 0x100000;
}


void cthd_prot_device::patch_cthd2003(UINT8* cpurom, UINT32 cpurom_size)
{
	// patches thanks to razoola
	UINT16 *mem16 = (UINT16 *)cpurom;

	// theres still a problem on the character select screen but it seems to be related to cpu core timing issues,
	// overclocking the 68k prevents it.

	// fix garbage on s1 layer over everything
	mem16[0xf415a/2] = 0x4ef9;
	mem16[0xf415c/2] = 0x000f;
	mem16[0xf415e/2] = 0x4cf2;

	// Fix corruption in attract mode before title screen
	for (int i = 0x1ae290/2; i < 0x1ae8d0/2; i++)
		mem16[i] = 0x0000;

	// Fix for title page
	for (int i = 0x1f8ef0/2; i < 0x1fa1f0/2; i += 2)
	{
		mem16[i]   -= 0x7000;
		mem16[i+1] -= 0x0010;
	}

	// Fix for green dots on title page
	for (int i = 0xac500/2; i < 0xac520/2; i++)
		mem16[i] = 0xffff;

	// Fix for blanks as screen change level end clear
	mem16[0x991d0/2] = 0xdd03;
	mem16[0x99306/2] = 0xdd03;
	mem16[0x99354/2] = 0xdd03;
	mem16[0x9943e/2] = 0xdd03;
}


/* Crouching Tiger Hidden Dragon 2003 Super Plus (bootleg of King of Fighters 2001) */


void cthd_prot_device::ct2k3sp_sx_decrypt( UINT8* fixedrom, UINT32 fixedrom_size )
{
	int rom_size = fixedrom_size;
	UINT8 *rom = fixedrom;
	dynamic_buffer buf(rom_size);

	memcpy(&buf[0], rom, rom_size);

	for (int i = 0; i < rom_size; i++)
	{
		int ofst = BITSWAP24((i & 0x1ffff), 23, 22, 21, 20, 19, 18, 17,  3,
											0,  1,  4,  2, 13, 14, 16, 15,
											5,  6, 11, 10,  9,  8,  7, 12 );
		ofst += (i >> 17) << 17;
		rom[i] = buf[ofst];
	}

	memcpy(&buf[0], rom, rom_size);

	memcpy(&rom[0x08000], &buf[0x10000], 0x8000);
	memcpy(&rom[0x10000], &buf[0x08000], 0x8000);
	memcpy(&rom[0x28000], &buf[0x30000], 0x8000);
	memcpy(&rom[0x30000], &buf[0x28000], 0x8000);
}

void cthd_prot_device::decrypt_ct2k3sp(UINT8* sprrom, UINT32 sprrom_size, UINT8* audiorom, UINT32 audiorom_size, UINT8* fixedrom, UINT32 fixedrom_size)
{
	UINT8 *romdata = audiorom + 0x10000;
	dynamic_buffer tmp(8 * 128 * 128);
	memcpy(&tmp[8 *  0 * 128], romdata + 8 *  0 * 128, 8 * 32 * 128);
	memcpy(&tmp[8 * 32 * 128], romdata + 8 * 64 * 128, 8 * 32 * 128);
	memcpy(&tmp[8 * 64 * 128], romdata + 8 * 32 * 128, 8 * 32 * 128);
	memcpy(&tmp[8 * 96 * 128], romdata + 8 * 96 * 128, 8 * 32 * 128);
	memcpy(romdata, &tmp[0], 8 * 128 * 128);

	memcpy(romdata - 0x10000, romdata, 0x10000);
	ct2k3sp_sx_decrypt(fixedrom, fixedrom_size);
	cthd2003_c(sprrom, sprrom_size, 0);
}


/* Crouching Tiger Hidden Dragon 2003 Super Plus alternate (bootleg of King of Fighters 2001) */


void cthd_prot_device::decrypt_ct2k3sa(UINT8* sprrom, UINT32 sprrom_size, UINT8* audiorom, UINT32 audiorom_size )
{
	UINT8 *romdata = audiorom + 0x10000;
	dynamic_buffer tmp(8 * 128 * 128);
	memcpy(&tmp[8 *  0 * 128], romdata + 8 *  0 * 128, 8 * 32 * 128);
	memcpy(&tmp[8 * 32 * 128], romdata + 8 * 64 * 128, 8 * 32 * 128);
	memcpy(&tmp[8 * 64 * 128], romdata + 8 * 32 * 128, 8 * 32 * 128);
	memcpy(&tmp[8 * 96 * 128], romdata + 8 * 96 * 128, 8 * 32 * 128);
	memcpy(romdata, &tmp[0], 8 * 128 * 128);

	memcpy(romdata - 0x10000, romdata, 0x10000);
	cthd2003_c(sprrom,sprrom_size, 0);
}


void cthd_prot_device::patch_ct2k3sa(UINT8* cpurom, UINT32 cpurom_size)
{
	/* patches thanks to razoola - same as for cthd2003*/
	UINT16 *mem16 = (UINT16 *)cpurom;

	// theres still a problem on the character select screen but it seems to be related to cpu core timing issues,
	// overclocking the 68k prevents it.

	// fix garbage on s1 layer over everything
	mem16[0xf415a/2] = 0x4ef9;
	mem16[0xf415c/2] = 0x000f;
	mem16[0xf415e/2] = 0x4cf2;

	// Fix corruption in attract mode before title screen
	for (int i = 0x1ae290/2; i < 0x1ae8d0/2; i += 1)
		mem16[i] = 0x0000;

	// Fix for title page
	for (int i = 0x1f8ef0/2; i < 0x1fa1f0/2; i += 2)
	{
		mem16[i]   -= 0x7000;
		mem16[i+1] -= 0x0010;
	}

	// Fix for green dots on title page
	for (int i = 0xac500/2; i < 0xac520/2; i += 1)
		mem16[i] = 0xffff;

	// Fix for blanks as screen change level end clear
	mem16[0x991d0/2] = 0xdd03;
	mem16[0x99306/2] = 0xdd03;
	mem16[0x99354/2] = 0xdd03;
	mem16[0x9943e/2] = 0xdd03;
}


/* Matrimelee / Shin Gouketsuji Ichizoku Toukon (bootleg) */
#define MATRIMBLZ80(i) (i ^ (BITSWAP8(i & 0x3,4,3,1,2,0,7,6,5) << 8))

void cthd_prot_device::matrimbl_decrypt(UINT8* sprrom, UINT32 sprrom_size, UINT8* audiorom, UINT32 audiorom_size)
{
	// decrypt Z80
	UINT8 *rom = audiorom + 0x10000;
	dynamic_buffer buf(0x20000);
	memcpy(&buf[0], rom, 0x20000);

	int j;
	for (int i = 0x00000; i < 0x20000; i++)
	{
		if (i & 0x10000)
		{
			if (i & 0x800)
			{
				j = MATRIMBLZ80(i);
				j ^= 0x10000;
			}
			else
			{
				j = MATRIMBLZ80((i ^ 0x01));
			}
		}
		else
		{
			if (i & 0x800)
			{
				j = MATRIMBLZ80((i ^ 0x01));
				j ^= 0x10000;
			}
			else
			{
				j = MATRIMBLZ80(i);
			}
		}
		rom[j] = buf[i];
	}
	memcpy(rom - 0x10000, rom, 0x10000);

	// decrypt gfx
	cthd2003_c(sprrom,sprrom_size, 0 );
}
