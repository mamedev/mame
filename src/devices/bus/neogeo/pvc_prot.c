// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood


#include "emu.h"
#include "pvc_prot.h"



extern const device_type PVC_PROT = &device_creator<pvc_prot_device>;


pvc_prot_device::pvc_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, PVC_PROT, "NeoGeo Protection (PVC)", tag, owner, clock, "pvc_prot", __FILE__)
{
}


void pvc_prot_device::device_start()
{
	save_item(NAME(m_cartridge_ram));
}

void pvc_prot_device::device_reset()
{
}




/************************ PVC Protection ***********************
  mslug5, svcchaos, kof2003
***************************************************************/

void pvc_prot_device::pvc_write_unpack_color()
{
	UINT16 pen = m_cartridge_ram[0xff0];

	UINT8 b = ((pen & 0x000f) << 1) | ((pen & 0x1000) >> 12);
	UINT8 g = ((pen & 0x00f0) >> 3) | ((pen & 0x2000) >> 13);
	UINT8 r = ((pen & 0x0f00) >> 7) | ((pen & 0x4000) >> 14);
	UINT8 s = (pen & 0x8000) >> 15;

	m_cartridge_ram[0xff1] = (g << 8) | b;
	m_cartridge_ram[0xff2] = (s << 8) | r;
}


void pvc_prot_device::pvc_write_pack_color()
{
	UINT16 gb = m_cartridge_ram[0xff4];
	UINT16 sr = m_cartridge_ram[0xff5];

	m_cartridge_ram[0xff6] = ((gb & 0x001e) >> 1) |
									((gb & 0x1e00) >> 5) |
									((sr & 0x001e) << 7) |
									((gb & 0x0001) << 12) |
									((gb & 0x0100) << 5) |
									((sr & 0x0001) << 14) |
									((sr & 0x0100) << 7);
}


void pvc_prot_device::pvc_write_bankswitch( address_space &space )
{
	UINT32 bankaddress;

	bankaddress = ((m_cartridge_ram[0xff8] >> 8)|(m_cartridge_ram[0xff9] << 8));
	m_cartridge_ram[0xff8] = (m_cartridge_ram[0xff8] & 0xfe00) | 0x00a0;
	m_cartridge_ram[0xff9] &= 0x7fff;
	m_bankdev->neogeo_set_main_cpu_bank_address(bankaddress + 0x100000);
}


READ16_MEMBER( pvc_prot_device::pvc_prot_r )
{
	return m_cartridge_ram[offset];
}


WRITE16_MEMBER( pvc_prot_device::pvc_prot_w )
{
	COMBINE_DATA(&m_cartridge_ram[offset] );
	if (offset == 0xff0)
		pvc_write_unpack_color();
	else if(offset >= 0xff4 && offset <= 0xff5)
		pvc_write_pack_color();
	else if(offset >= 0xff8)
		pvc_write_bankswitch(space);
}


void pvc_prot_device::install_pvc_protection(cpu_device* maincpu, neogeo_banked_cart_device* bankdev)
{
	m_bankdev = bankdev;
	maincpu->space(AS_PROGRAM).install_readwrite_handler(0x2fe000, 0x2fffff, read16_delegate(FUNC(pvc_prot_device::pvc_prot_r),this), write16_delegate(FUNC(pvc_prot_device::pvc_prot_w),this));
}




/* kf2k3pcb, kof2003, kof2003h, mslug5 and svc have updated P rom scramble */
void pvc_prot_device::mslug5_decrypt_68k(UINT8* rom, UINT32 size)
{
	static const UINT8 xor1[ 0x20 ] = { 0xc2, 0x4b, 0x74, 0xfd, 0x0b, 0x34, 0xeb, 0xd7, 0x10, 0x6d, 0xf9, 0xce, 0x5d, 0xd5, 0x61, 0x29, 0xf5, 0xbe, 0x0d, 0x82, 0x72, 0x45, 0x0f, 0x24, 0xb3, 0x34, 0x1b, 0x99, 0xea, 0x09, 0xf3, 0x03 };
	static const UINT8 xor2[ 0x20 ] = { 0x36, 0x09, 0xb0, 0x64, 0x95, 0x0f, 0x90, 0x42, 0x6e, 0x0f, 0x30, 0xf6, 0xe5, 0x08, 0x30, 0x64, 0x08, 0x04, 0x00, 0x2f, 0x72, 0x09, 0xa0, 0x13, 0xc9, 0x0b, 0xa0, 0x3e, 0xc2, 0x00, 0x40, 0x2b };
	int i;
	int ofst;
	int rom_size = 0x800000;
	dynamic_buffer buf( rom_size );

	for( i = 0; i < 0x100000; i++ )
	{
		rom[ i ] ^= xor1[ (BYTE_XOR_LE(i) % 0x20) ];
	}
	for( i = 0x100000; i < 0x800000; i++ )
	{
		rom[ i ] ^= xor2[ (BYTE_XOR_LE(i) % 0x20) ];
	}

	for( i = 0x100000; i < 0x0800000; i += 4 )
	{
		UINT16 rom16;
		rom16 = rom[BYTE_XOR_LE(i+1)] | rom[BYTE_XOR_LE(i+2)]<<8;
		rom16 = BITSWAP16( rom16, 15, 14, 13, 12, 10, 11, 8, 9, 6, 7, 4, 5, 3, 2, 1, 0 );
		rom[BYTE_XOR_LE(i+1)] = rom16&0xff;
		rom[BYTE_XOR_LE(i+2)] = rom16>>8;
	}
	memcpy( &buf[0], rom, rom_size );
	for( i = 0; i < 0x0100000 / 0x10000; i++ )
	{
		ofst = (i & 0xf0) + BITSWAP8( (i & 0x0f), 7, 6, 5, 4, 1, 0, 3, 2 );
		memcpy( &rom[ i * 0x10000 ], &buf[ ofst * 0x10000 ], 0x10000 );
	}
	for( i = 0x100000; i < 0x800000; i += 0x100 )
	{
		ofst = (i & 0xf000ff) + ((i & 0x000f00) ^ 0x00700) + (BITSWAP8( ((i & 0x0ff000) >> 12), 5, 4, 7, 6, 1, 0, 3, 2 ) << 12);
		memcpy( &rom[ i ], &buf[ ofst ], 0x100 );
	}
	memcpy( &buf[0], rom, rom_size );
	memcpy( &rom[ 0x100000 ], &buf[ 0x700000 ], 0x100000 );
	memcpy( &rom[ 0x200000 ], &buf[ 0x100000 ], 0x600000 );
}


void pvc_prot_device::svc_px_decrypt(UINT8* rom, UINT32 size)
{
	static const UINT8 xor1[ 0x20 ] = { 0x3b, 0x6a, 0xf7, 0xb7, 0xe8, 0xa9, 0x20, 0x99, 0x9f, 0x39, 0x34, 0x0c, 0xc3, 0x9a, 0xa5, 0xc8, 0xb8, 0x18, 0xce, 0x56, 0x94, 0x44, 0xe3, 0x7a, 0xf7, 0xdd, 0x42, 0xf0, 0x18, 0x60, 0x92, 0x9f };
	static const UINT8 xor2[ 0x20 ] = { 0x69, 0x0b, 0x60, 0xd6, 0x4f, 0x01, 0x40, 0x1a, 0x9f, 0x0b, 0xf0, 0x75, 0x58, 0x0e, 0x60, 0xb4, 0x14, 0x04, 0x20, 0xe4, 0xb9, 0x0d, 0x10, 0x89, 0xeb, 0x07, 0x30, 0x90, 0x50, 0x0e, 0x20, 0x26 };
	int i;
	int ofst;
	int rom_size = 0x800000;
	dynamic_buffer buf( rom_size );

	for( i = 0; i < 0x100000; i++ )
	{
		rom[ i ] ^= xor1[ (BYTE_XOR_LE(i) % 0x20) ];
	}
	for( i = 0x100000; i < 0x800000; i++ )
	{
		rom[ i ] ^= xor2[ (BYTE_XOR_LE(i) % 0x20) ];
	}

	for( i = 0x100000; i < 0x0800000; i += 4 )
	{
		UINT16 rom16;
		rom16 = rom[BYTE_XOR_LE(i+1)] | rom[BYTE_XOR_LE(i+2)]<<8;
		rom16 = BITSWAP16( rom16, 15, 14, 13, 12, 10, 11, 8, 9, 6, 7, 4, 5, 3, 2, 1, 0 );
		rom[BYTE_XOR_LE(i+1)] = rom16&0xff;
		rom[BYTE_XOR_LE(i+2)] = rom16>>8;
	}
	memcpy( &buf[0], rom, rom_size );
	for( i = 0; i < 0x0100000 / 0x10000; i++ )
	{
		ofst = (i & 0xf0) + BITSWAP8( (i & 0x0f), 7, 6, 5, 4, 2, 3, 0, 1 );
		memcpy( &rom[ i * 0x10000 ], &buf[ ofst * 0x10000 ], 0x10000 );
	}
	for( i = 0x100000; i < 0x800000; i += 0x100 )
	{
		ofst = (i & 0xf000ff) + ((i & 0x000f00) ^ 0x00a00) + (BITSWAP8( ((i & 0x0ff000) >> 12), 4, 5, 6, 7, 1, 0, 3, 2 ) << 12);
		memcpy( &rom[ i ], &buf[ ofst ], 0x100 );
	}
	memcpy( &buf[0], rom, rom_size );
	memcpy( &rom[ 0x100000 ], &buf[ 0x700000 ], 0x100000 );
	memcpy( &rom[ 0x200000 ], &buf[ 0x100000 ], 0x600000 );
}


void pvc_prot_device::kf2k3pcb_decrypt_68k(UINT8* rom, UINT32 size)
{
	static const UINT8 xor2[ 0x20 ] = { 0xb4, 0x0f, 0x40, 0x6c, 0x38, 0x07, 0xd0, 0x3f, 0x53, 0x08, 0x80, 0xaa, 0xbe, 0x07, 0xc0, 0xfa, 0xd0, 0x08, 0x10, 0xd2, 0xf1, 0x03, 0x70, 0x7e, 0x87, 0x0b, 0x40, 0xf6, 0x2a, 0x0a, 0xe0, 0xf9 };
	int i;
	int ofst;
	int rom_size = 0x900000;
	dynamic_buffer buf( rom_size );

	for (i = 0; i < 0x100000; i++)
	{
		rom[ 0x800000 + i ] ^= rom[ 0x100002 | i ];
	}
	for( i = 0x100000; i < 0x800000; i++ )
	{
		rom[ i ] ^= xor2[ (BYTE_XOR_LE(i) % 0x20) ];
	}
	for( i = 0x100000; i < 0x800000; i += 4 )
	{
		UINT16 rom16;
		rom16 = rom[BYTE_XOR_LE(i+1)] | rom[BYTE_XOR_LE(i+2)]<<8;
		rom16 = BITSWAP16( rom16, 15, 14, 13, 12, 4, 5, 6, 7, 8, 9, 10, 11, 3, 2, 1, 0 );
		rom[BYTE_XOR_LE(i+1)] = rom16&0xff;
		rom[BYTE_XOR_LE(i+2)] = rom16>>8;
	}
	for( i = 0; i < 0x0100000 / 0x10000; i++ )
	{
		ofst = (i & 0xf0) + BITSWAP8( (i & 0x0f), 7, 6, 5, 4, 1, 0, 3, 2 );
		memcpy( &buf[ i * 0x10000 ], &rom[ ofst * 0x10000 ], 0x10000 );
	}
	for( i = 0x100000; i < 0x900000; i += 0x100 )
	{
		ofst = (i & 0xf000ff) + ((i & 0x000f00) ^ 0x00300) + (BITSWAP8( ((i & 0x0ff000) >> 12), 4, 5, 6, 7, 1, 0, 3, 2 ) << 12);
		memcpy( &buf[ i ], &rom[ ofst ], 0x100 );
	}
	memcpy (&rom[0x000000], &buf[0x000000], 0x100000);
	memcpy (&rom[0x100000], &buf[0x800000], 0x100000);
	memcpy (&rom[0x200000], &buf[0x100000], 0x700000);
}


void pvc_prot_device::kof2003_decrypt_68k(UINT8* rom, UINT32 size)
{
	static const UINT8 xor1[0x20] = { 0x3b, 0x6a, 0xf7, 0xb7, 0xe8, 0xa9, 0x20, 0x99, 0x9f, 0x39, 0x34, 0x0c, 0xc3, 0x9a, 0xa5, 0xc8, 0xb8, 0x18, 0xce, 0x56, 0x94, 0x44, 0xe3, 0x7a, 0xf7, 0xdd, 0x42, 0xf0, 0x18, 0x60, 0x92, 0x9f };
	static const UINT8 xor2[0x20] = { 0x2f, 0x02, 0x60, 0xbb, 0x77, 0x01, 0x30, 0x08, 0xd8, 0x01, 0xa0, 0xdf, 0x37, 0x0a, 0xf0, 0x65, 0x28, 0x03, 0xd0, 0x23, 0xd3, 0x03, 0x70, 0x42, 0xbb, 0x06, 0xf0, 0x28, 0xba, 0x0f, 0xf0, 0x7a };
	int i;
	int ofst;
	int rom_size = 0x900000;
	dynamic_buffer buf( rom_size );

	for (i = 0; i < 0x100000; i++)
	{
		rom[ 0x800000 + i ] ^= rom[ 0x100002 | i ];
	}
	for( i = 0; i < 0x100000; i++)
	{
		rom[ i ] ^= xor1[ (BYTE_XOR_LE(i) % 0x20) ];
	}
	for( i = 0x100000; i < 0x800000; i++)
	{
		rom[ i ] ^= xor2[ (BYTE_XOR_LE(i) % 0x20) ];
	}
	for( i = 0x100000; i < 0x800000; i += 4)
	{
		UINT16 rom16;
		rom16 = rom[BYTE_XOR_LE(i+1)] | rom[BYTE_XOR_LE(i+2)]<<8;
		rom16 = BITSWAP16( rom16, 15, 14, 13, 12, 5, 4, 7, 6, 9, 8, 11, 10, 3, 2, 1, 0 );
		rom[BYTE_XOR_LE(i+1)] = rom16&0xff;
		rom[BYTE_XOR_LE(i+2)] = rom16>>8;
	}
	for( i = 0; i < 0x0100000 / 0x10000; i++ )
	{
		ofst = (i & 0xf0) + BITSWAP8((i & 0x0f), 7, 6, 5, 4, 0, 1, 2, 3);
		memcpy( &buf[ i * 0x10000 ], &rom[ ofst * 0x10000 ], 0x10000 );
	}
	for( i = 0x100000; i < 0x900000; i += 0x100)
	{
		ofst = (i & 0xf000ff) + ((i & 0x000f00) ^ 0x00800) + (BITSWAP8( ((i & 0x0ff000) >> 12), 4, 5, 6, 7, 1, 0, 3, 2 ) << 12);
		memcpy( &buf[ i ], &rom[ ofst ], 0x100 );
	}
	memcpy (&rom[0x000000], &buf[0x000000], 0x100000);
	memcpy (&rom[0x100000], &buf[0x800000], 0x100000);
	memcpy (&rom[0x200000], &buf[0x100000], 0x700000);
}


void pvc_prot_device::kof2003h_decrypt_68k(UINT8* rom, UINT32 size)
{
	static const UINT8 xor1[0x20] = { 0xc2, 0x4b, 0x74, 0xfd, 0x0b, 0x34, 0xeb, 0xd7, 0x10, 0x6d, 0xf9, 0xce, 0x5d, 0xd5, 0x61, 0x29, 0xf5, 0xbe, 0x0d, 0x82, 0x72, 0x45, 0x0f, 0x24, 0xb3, 0x34, 0x1b, 0x99, 0xea, 0x09, 0xf3, 0x03 };
	static const UINT8 xor2[0x20] = { 0x2b, 0x09, 0xd0, 0x7f, 0x51, 0x0b, 0x10, 0x4c, 0x5b, 0x07, 0x70, 0x9d, 0x3e, 0x0b, 0xb0, 0xb6, 0x54, 0x09, 0xe0, 0xcc, 0x3d, 0x0d, 0x80, 0x99, 0x87, 0x03, 0x90, 0x82, 0xfe, 0x04, 0x20, 0x18 };
	int i;
	int ofst;
	int rom_size = 0x900000;
	dynamic_buffer buf( rom_size );

	for (i = 0; i < 0x100000; i++)
	{
		rom[ 0x800000 + i ] ^= rom[ 0x100002 | i ];
	}
	for( i = 0; i < 0x100000; i++)
	{
		rom[ i ] ^= xor1[ (BYTE_XOR_LE(i) % 0x20) ];
	}
	for( i = 0x100000; i < 0x800000; i++)
	{
		rom[ i ] ^= xor2[ (BYTE_XOR_LE(i) % 0x20) ];
	}
	for( i = 0x100000; i < 0x800000; i += 4)
	{
		UINT16 rom16;
		rom16 = rom[BYTE_XOR_LE(i+1)] | rom[BYTE_XOR_LE(i+2)]<<8;
		rom16 = BITSWAP16( rom16, 15, 14, 13, 12, 10, 11, 8, 9, 6, 7, 4, 5, 3, 2, 1, 0 );
		rom[BYTE_XOR_LE(i+1)] = rom16&0xff;
		rom[BYTE_XOR_LE(i+2)] = rom16>>8;
	}
	for( i = 0; i < 0x0100000 / 0x10000; i++ )
	{
		ofst = (i & 0xf0) + BITSWAP8((i & 0x0f), 7, 6, 5, 4, 1, 0, 3, 2);
		memcpy( &buf[ i * 0x10000 ], &rom[ ofst * 0x10000 ], 0x10000 );
	}
	for( i = 0x100000; i < 0x900000; i += 0x100)
	{
		ofst = (i & 0xf000ff) + ((i & 0x000f00) ^ 0x00400) + (BITSWAP8( ((i & 0x0ff000) >> 12), 6, 7, 4, 5, 0, 1, 2, 3 ) << 12);
		memcpy( &buf[ i ], &rom[ ofst ], 0x100 );
	}
	memcpy (&rom[0x000000], &buf[0x000000], 0x100000);
	memcpy (&rom[0x100000], &buf[0x800000], 0x100000);
	memcpy (&rom[0x200000], &buf[0x100000], 0x700000);
}
