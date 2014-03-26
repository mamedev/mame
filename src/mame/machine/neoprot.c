/***************************************************************************

    Neo-Geo hardware protection devices

    unknown devices
        ssideki, fatfury2, kof98 (some versions), mslugx

    SMA chip
        kof99, garou, garouh, mslug3, kof2002

        custom banking, random number generator
        encryption (see machine/neocrypt.c)
        internal rom data

    PVC chip
        mslug5, kof2003, svcchaos

***************************************************************************/

#include "emu.h"
#include "includes/neogeo.h"

/************************ Fatal Fury 2 *************************/

READ16_MEMBER( neogeo_state::fatfury2_protection_16_r )
{
	UINT16 res = m_fatfury2_prot_data >> 24;

	switch (offset)
	{
		case 0x55550/2:
		case 0xffff0/2:
		case 0x00000/2:
		case 0xff000/2:
		case 0x36000/2:
		case 0x36008/2:
			return res;

		case 0x36004/2:
		case 0x3600c/2:
			return ((res & 0xf0) >> 4) | ((res & 0x0f) << 4);

		default:
			logerror("unknown protection read at pc %06x, offset %08x\n", space.device().safe_pc(), offset << 1);
			return 0;
	}
}


WRITE16_MEMBER( neogeo_state::fatfury2_protection_16_w )
{
	switch (offset)
	{
		case 0x11112/2: /* data == 0x1111; expects 0xff000000 back */
			m_fatfury2_prot_data = 0xff000000;
			break;

		case 0x33332/2: /* data == 0x3333; expects 0x0000ffff back */
			m_fatfury2_prot_data = 0x0000ffff;
			break;

		case 0x44442/2: /* data == 0x4444; expects 0x00ff0000 back */
			m_fatfury2_prot_data = 0x00ff0000;
			break;

		case 0x55552/2: /* data == 0x5555; read back from 55550, ffff0, 00000, ff000 */
			m_fatfury2_prot_data = 0xff00ff00;
			break;

		case 0x56782/2: /* data == 0x1234; read back from 36000 *or* 36004 */
			m_fatfury2_prot_data = 0xf05a3601;
			break;

		case 0x42812/2: /* data == 0x1824; read back from 36008 *or* 3600c */
			m_fatfury2_prot_data = 0x81422418;
			break;

		case 0x55550/2:
		case 0xffff0/2:
		case 0xff000/2:
		case 0x36000/2:
		case 0x36004/2:
		case 0x36008/2:
		case 0x3600c/2:
			m_fatfury2_prot_data <<= 8;
			break;

		default:
			logerror("unknown protection write at pc %06x, offset %08x, data %02x\n", space.device().safe_pc(), offset, data);
			break;
	}
}


void neogeo_state::fatfury2_install_protection()
{
	/* the protection involves reading and writing addresses in the */
	/* 0x2xxxxx range. There are several checks all around the code. */
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x200000, 0x2fffff, read16_delegate(FUNC(neogeo_state::fatfury2_protection_16_r),this), write16_delegate(FUNC(neogeo_state::fatfury2_protection_16_w),this));

	m_fatfury2_prot_data = 0;

	save_item(NAME(m_fatfury2_prot_data));
}


/************************ King of Fighters 98*******************
  The encrypted set has a rom overlay feature, checked at
  various points in the game.
  Boards used: NEO-MVS PROGSF1 (1998.6.17) / NEO-MVS PROGSF1E (1998.6.18)
  The boards have an ALTERA chip (EPM7128SQC100-15) which is tied to 242-P1
***************************************************************/

WRITE16_MEMBER( neogeo_state::kof98_prot_w )
{
	/* info from razoola */
	UINT16* mem16 = (UINT16*)memregion("maincpu")->base();

	switch (data)
	{
	case 0x0090:
		logerror ("%06x kof98 - protection 0x0090 old %04x %04x\n", space.device().safe_pc(), mem16[0x100/2], mem16[0x102/2]);
		mem16[0x100/2] = 0x00c2;
		mem16[0x102/2] = 0x00fd;
		break;

	case 0x00f0:
		logerror ("%06x kof98 - protection 0x00f0 old %04x %04x\n", space.device().safe_pc(), mem16[0x100/2], mem16[0x102/2]);
		mem16[0x100/2] = 0x4e45;
		mem16[0x102/2] = 0x4f2d;
		break;

	default: // 00aa is written, but not needed?
		logerror ("%06x kof98 - unknown protection write %04x\n", space.device().safe_pc(), data);
		break;
	}
}


void neogeo_state::install_kof98_protection()
{
	/* when 0x20aaaa contains 0x0090 (word) then 0x100 (normally the neogeo header) should return 0x00c200fd worked out using real hw */

	m_maincpu->space(AS_PROGRAM).install_write_handler(0x20aaaa, 0x20aaab, write16_delegate(FUNC(neogeo_state::kof98_prot_w),this));
}


/************************ Metal Slug X *************************
  Board used: NEO-MVS PROGEOP (1999.2.2)
  The board has an ALTERA chip (EPM7128SQC100-15) which is tied to 250-P1
  Also found is a QFP144 chip labeled with 0103 - function unknown
***************************************************************/

WRITE16_MEMBER( neogeo_state::mslugx_protection_16_w )
{
	switch (offset)
	{
		case 0x0/2: // start new read?
			m_mslugx_command = 0;
		break;

		case 0x2/2: // command? These are pulsed with data and then 0
		case 0x4/2:
			m_mslugx_command |= data;
		break;

		case 0x6/2: // finished?
		break;

		case 0xa/2: // init?
			m_mslugx_counter = 0;
			m_mslugx_command = 0;
		break;

		default:
			logerror("unknown protection write at pc %06x, offset %08x, data %02x\n", space.device().safe_pc(), offset << 1, data);
		break;
	}
}


READ16_MEMBER( neogeo_state::mslugx_protection_16_r )
{
	UINT16 res = 0;

	switch (m_mslugx_command)
	{
		case 0x0001: { // $3bdc(?) and $3c30 (Register D7)
			res = (space.read_byte(0xdedd2 + ((m_mslugx_counter >> 3) & 0xfff)) >> (~m_mslugx_counter & 0x07)) & 1;
			m_mslugx_counter++;
		}
		break;

		case 0x0fff: { // All other accesses (Register D2)
			INT32 select = space.read_word(0x10f00a) - 1; // How should this be calculated?
			res = (space.read_byte(0xdedd2 + ((select >> 3) & 0x0fff)) >> (~select & 0x07)) & 1;
		}
		break;

		default:
			logerror("unknown protection read at pc %06x, offset %08x\n", space.device().safe_pc(), offset << 1);
		break;
	}

	return res;
}


void neogeo_state::mslugx_install_protection()
{
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x2fffe0, 0x2fffef, read16_delegate(FUNC(neogeo_state::mslugx_protection_16_r),this), write16_delegate(FUNC(neogeo_state::mslugx_protection_16_w),this));

	save_item(NAME(m_mslugx_command));
	save_item(NAME(m_mslugx_counter));
}


/************************ SMA Protection************************
  thanks to Razoola
***************************************************************/

WRITE16_MEMBER( neogeo_state::kof99_bankswitch_w )
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

	neogeo_set_main_cpu_bank_address(bankaddress);
}


WRITE16_MEMBER( neogeo_state::garou_bankswitch_w )
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

	neogeo_set_main_cpu_bank_address(bankaddress);
}


WRITE16_MEMBER( neogeo_state::garouh_bankswitch_w )
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

	neogeo_set_main_cpu_bank_address(bankaddress);
}


WRITE16_MEMBER( neogeo_state::mslug3_bankswitch_w )
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

	neogeo_set_main_cpu_bank_address(bankaddress);
}


WRITE16_MEMBER( neogeo_state::kof2000_bankswitch_w )
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

	neogeo_set_main_cpu_bank_address(bankaddress);
}


READ16_MEMBER( neogeo_state::prot_9a37_r )
{
	return 0x9a37;
}


/* information about the sma random number generator provided by razoola */
/* this RNG is correct for KOF99, other games might be different */

READ16_MEMBER( neogeo_state::sma_random_r )
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


void neogeo_state::reset_sma_rng()
{
	m_sma_rng = 0x2345;
}


void neogeo_state::sma_install_random_read_handler(int addr1, int addr2 )
{
	save_item(NAME(m_sma_rng));

	m_maincpu->space(AS_PROGRAM).install_read_handler(addr1, addr1 + 1, read16_delegate(FUNC(neogeo_state::sma_random_r),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(addr2, addr2 + 1, read16_delegate(FUNC(neogeo_state::sma_random_r),this));
}


void neogeo_state::kof99_install_protection()
{
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x2ffff0, 0x2ffff1, write16_delegate(FUNC(neogeo_state::kof99_bankswitch_w),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x2fe446, 0x2fe447, read16_delegate(FUNC(neogeo_state::prot_9a37_r),this));

	sma_install_random_read_handler(0x2ffff8, 0x2ffffa);
}


void neogeo_state::garou_install_protection()
{
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x2fffc0, 0x2fffc1, write16_delegate(FUNC(neogeo_state::garou_bankswitch_w),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x2fe446, 0x2fe447, read16_delegate(FUNC(neogeo_state::prot_9a37_r),this));

	sma_install_random_read_handler(0x2fffcc, 0x2ffff0);
}


void neogeo_state::garouh_install_protection()
{
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x2fffc0, 0x2fffc1, write16_delegate(FUNC(neogeo_state::garouh_bankswitch_w),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x2fe446, 0x2fe447, read16_delegate(FUNC(neogeo_state::prot_9a37_r),this));

	sma_install_random_read_handler(0x2fffcc, 0x2ffff0);
}


void neogeo_state::mslug3_install_protection()
{
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x2fffe4, 0x2fffe5, write16_delegate(FUNC(neogeo_state::mslug3_bankswitch_w),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x2fe446, 0x2fe447, read16_delegate(FUNC(neogeo_state::prot_9a37_r),this));

//  sma_install_random_read_handler(0x2ffff8, 0x2ffffa);
}


void neogeo_state::kof2000_install_protection()
{
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x2fffec, 0x2fffed, write16_delegate(FUNC(neogeo_state::kof2000_bankswitch_w),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x2fe446, 0x2fe447, read16_delegate(FUNC(neogeo_state::prot_9a37_r),this));

	sma_install_random_read_handler(0x2fffd8, 0x2fffda);
}



/************************ PVC Protection ***********************
  mslug5, svcchaos, kof2003
***************************************************************/

void neogeo_state::pvc_write_unpack_color()
{
	UINT16 pen = m_cartridge_ram[0xff0];

	UINT8 b = ((pen & 0x000f) << 1) | ((pen & 0x1000) >> 12);
	UINT8 g = ((pen & 0x00f0) >> 3) | ((pen & 0x2000) >> 13);
	UINT8 r = ((pen & 0x0f00) >> 7) | ((pen & 0x4000) >> 14);
	UINT8 s = (pen & 0x8000) >> 15;

	m_cartridge_ram[0xff1] = (g << 8) | b;
	m_cartridge_ram[0xff2] = (s << 8) | r;
}


void neogeo_state::pvc_write_pack_color()
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


void neogeo_state::pvc_write_bankswitch( address_space &space )
{
	UINT32 bankaddress;

	bankaddress = ((m_cartridge_ram[0xff8] >> 8)|(m_cartridge_ram[0xff9] << 8));
	m_cartridge_ram[0xff8] = (m_cartridge_ram[0xff8] & 0xfe00) | 0x00a0;
	m_cartridge_ram[0xff9] &= 0x7fff;
	neogeo_set_main_cpu_bank_address(bankaddress + 0x100000);
}


READ16_MEMBER( neogeo_state::pvc_prot_r )
{
	return m_cartridge_ram[offset];
}


WRITE16_MEMBER( neogeo_state::pvc_prot_w )
{
	COMBINE_DATA(&m_cartridge_ram[offset] );
	if (offset == 0xff0)
		pvc_write_unpack_color();
	else if(offset >= 0xff4 && offset <= 0xff5)
		pvc_write_pack_color();
	else if(offset >= 0xff8)
		pvc_write_bankswitch(space);
}


void neogeo_state::install_pvc_protection()
{
	save_item(NAME(m_cartridge_ram));

	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x2fe000, 0x2fffff, read16_delegate(FUNC(neogeo_state::pvc_prot_r),this), write16_delegate(FUNC(neogeo_state::pvc_prot_w),this));
}
