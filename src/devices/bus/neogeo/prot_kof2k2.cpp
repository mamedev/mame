// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli

#include "emu.h"
#include "prot_kof2k2.h"

DEFINE_DEVICE_TYPE(NG_KOF2002_PROT, kof2002_prot_device, "ng_kof2002_prot", "Neo Geo KoF 2002 Protection")


kof2002_prot_device::kof2002_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NG_KOF2002_PROT, tag, owner, clock)
{
}


void kof2002_prot_device::device_start()
{
}

void kof2002_prot_device::device_reset()
{
}


/* kof2002, matrim, samsho5, samsh5sp have some simple block swapping */
void kof2002_prot_device::kof2002_decrypt_68k(u8* cpurom, u32 cpurom_size)
{
	static const int sec[]={0x100000,0x280000,0x300000,0x180000,0x000000,0x380000,0x200000,0x080000};
	u8 *src = cpurom + 0x100000;
	std::vector<u8> dst(0x400000);
	memcpy(&dst[0], src, 0x400000);

	for (int i = 0; i < 8; ++i)
		memcpy(src + i * 0x80000, &dst[sec[i]], 0x80000);
}


void kof2002_prot_device::matrim_decrypt_68k(u8* cpurom, u32 cpurom_size)
{
	static const int sec[]={0x100000,0x280000,0x300000,0x180000,0x000000,0x380000,0x200000,0x080000};
	u8 *src = cpurom + 0x100000;
	std::vector<u8> dst(0x400000);
	memcpy(&dst[0], src, 0x400000);

	for (int i = 0; i < 8; ++i)
		memcpy(src + i * 0x80000, &dst[sec[i]], 0x80000);
}


void kof2002_prot_device::samsho5_decrypt_68k(u8* cpurom, u32 cpurom_size)
{
	static const int sec[]={0x000000,0x080000,0x700000,0x680000,0x500000,0x180000,0x200000,0x480000,0x300000,0x780000,0x600000,0x280000,0x100000,0x580000,0x400000,0x380000};
	u8 *src = cpurom;
	std::vector<u8> dst(0x800000);
	memcpy(&dst[0], src, 0x800000);
	for (int i = 0; i < 16; ++i)
		memcpy(src + i * 0x80000, &dst[sec[i]], 0x80000);
}


void kof2002_prot_device::samsh5sp_decrypt_68k(u8* cpurom, u32 cpurom_size)
{
	static const int sec[]={0x000000,0x080000,0x500000,0x480000,0x600000,0x580000,0x700000,0x280000,0x100000,0x680000,0x400000,0x780000,0x200000,0x380000,0x300000,0x180000};
	u8 *src = cpurom;
	std::vector<u8> dst(0x800000);

	memcpy(&dst[0], src, 0x800000);
	for (int i = 0; i < 16; ++i)
		memcpy(src + i * 0x80000, &dst[sec[i]], 0x80000);
}
