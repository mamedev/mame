// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood

#include "emu.h"
#include "kof2002_prot.h"



extern const device_type KOF2002_PROT = &device_creator<kof2002_prot_device>;


kof2002_prot_device::kof2002_prot_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, KOF2002_PROT, "NeoGeo Protection (KOF2002)", tag, owner, clock, "kof2002_prot", __FILE__)
{
}


void kof2002_prot_device::device_start()
{
}

void kof2002_prot_device::device_reset()
{
}


/* kof2002, matrim, samsho5, samsh5sp have some simple block swapping */
void kof2002_prot_device::kof2002_decrypt_68k(UINT8* cpurom, UINT32 cpurom_size)
{
	int i;
	static const int sec[]={0x100000,0x280000,0x300000,0x180000,0x000000,0x380000,0x200000,0x080000};
	UINT8 *src = cpurom+0x100000;
	dynamic_buffer dst(0x400000);
	memcpy( &dst[0], src, 0x400000 );
	for( i=0; i<8; ++i )
	{
		memcpy( src+i*0x80000, &dst[sec[i]], 0x80000 );
	}
}


void kof2002_prot_device::matrim_decrypt_68k(UINT8* cpurom, UINT32 cpurom_size)
{
	int i;
	static const int sec[]={0x100000,0x280000,0x300000,0x180000,0x000000,0x380000,0x200000,0x080000};
	UINT8 *src = cpurom+0x100000;
	dynamic_buffer dst(0x400000);
	memcpy( &dst[0], src, 0x400000);
	for( i=0; i<8; ++i )
	{
		memcpy( src+i*0x80000, &dst[sec[i]], 0x80000 );
	}
}


void kof2002_prot_device::samsho5_decrypt_68k(UINT8* cpurom, UINT32 cpurom_size)
{
	int i;
	static const int sec[]={0x000000,0x080000,0x700000,0x680000,0x500000,0x180000,0x200000,0x480000,0x300000,0x780000,0x600000,0x280000,0x100000,0x580000,0x400000,0x380000};
	UINT8 *src = cpurom;
	dynamic_buffer dst(0x800000);

	memcpy( &dst[0], src, 0x800000 );
	for( i=0; i<16; ++i )
	{
		memcpy( src+i*0x80000, &dst[sec[i]], 0x80000 );
	}
}


void kof2002_prot_device::samsh5sp_decrypt_68k(UINT8* cpurom, UINT32 cpurom_size)
{
	int i;
	static const int sec[]={0x000000,0x080000,0x500000,0x480000,0x600000,0x580000,0x700000,0x280000,0x100000,0x680000,0x400000,0x780000,0x200000,0x380000,0x300000,0x180000};
	UINT8 *src = cpurom;
	dynamic_buffer dst(0x800000);

	memcpy( &dst[0], src, 0x800000 );
	for( i=0; i<16; ++i )
	{
		memcpy( src+i*0x80000, &dst[sec[i]], 0x80000 );
	}
}
