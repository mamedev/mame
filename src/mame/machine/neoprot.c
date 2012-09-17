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

static READ16_HANDLER( fatfury2_protection_16_r )
{
	neogeo_state *state = space.machine().driver_data<neogeo_state>();
	UINT16 res = state->m_fatfury2_prot_data >> 24;

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


static WRITE16_HANDLER( fatfury2_protection_16_w )
{
	neogeo_state *state = space.machine().driver_data<neogeo_state>();

	switch (offset)
	{
		case 0x11112/2: /* data == 0x1111; expects 0xff000000 back */
			state->m_fatfury2_prot_data = 0xff000000;
			break;

		case 0x33332/2: /* data == 0x3333; expects 0x0000ffff back */
			state->m_fatfury2_prot_data = 0x0000ffff;
			break;

		case 0x44442/2: /* data == 0x4444; expects 0x00ff0000 back */
			state->m_fatfury2_prot_data = 0x00ff0000;
			break;

		case 0x55552/2: /* data == 0x5555; read back from 55550, ffff0, 00000, ff000 */
			state->m_fatfury2_prot_data = 0xff00ff00;
			break;

		case 0x56782/2: /* data == 0x1234; read back from 36000 *or* 36004 */
			state->m_fatfury2_prot_data = 0xf05a3601;
			break;

		case 0x42812/2: /* data == 0x1824; read back from 36008 *or* 3600c */
			state->m_fatfury2_prot_data = 0x81422418;
			break;

		case 0x55550/2:
		case 0xffff0/2:
		case 0xff000/2:
		case 0x36000/2:
		case 0x36004/2:
		case 0x36008/2:
		case 0x3600c/2:
			state->m_fatfury2_prot_data <<= 8;
			break;

		default:
			logerror("unknown protection write at pc %06x, offset %08x, data %02x\n", space.device().safe_pc(), offset, data);
			break;
	}
}


void fatfury2_install_protection( running_machine &machine )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();

	/* the protection involves reading and writing addresses in the */
	/* 0x2xxxxx range. There are several checks all around the code. */
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x200000, 0x2fffff, FUNC(fatfury2_protection_16_r), FUNC(fatfury2_protection_16_w));

	state->m_fatfury2_prot_data = 0;

	state->save_item(NAME(state->m_fatfury2_prot_data));
}


/************************ King of Fighters 98*******************
  The encrypted set has a rom overlay feature, checked at
  various points in the game.
  Special board is used: NEO-MVS PROGSF1 (1998.6.17)
  The board has a ALTERA (EPM7128SQC100-15) chip which is tied to 242-P1
***************************************************************/

static WRITE16_HANDLER ( kof98_prot_w )
{
	/* info from razoola */
	UINT16* mem16 = (UINT16*)space.machine().root_device().memregion("maincpu")->base();

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


void install_kof98_protection( running_machine &machine )
{
	/* when 0x20aaaa contains 0x0090 (word) then 0x100 (normally the neogeo header) should return 0x00c200fd worked out using real hw */

	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x20aaaa, 0x20aaab, FUNC(kof98_prot_w));
}


/************************ Metal Slug X *************************
  todo: emulate, not patch!
  Special board is used: NEO-MVS PROGEOP (1999.2.2)
  The board has a ALTERA (EPM7128SQC100-15) chip which is tied to 250-P1
  Also found on this special board is a QFP144 labeled with 0103
***************************************************************/

void mslugx_install_protection( running_machine &machine )
{
	int i;
	UINT16 *mem16 = (UINT16 *)machine.root_device().memregion("maincpu")->base();

	for (i = 0;i < (0x100000/2) - 4;i++)
	{
		if (mem16[i + 0] == 0x0243 &&
			mem16[i + 1] == 0x0001 &&	/* andi.w  #$1, D3 */
			mem16[i + 2] == 0x6600)		/* bne xxxx */
		{
			mem16[i + 2] = 0x4e71;
			mem16[i + 3] = 0x4e71;
		}
	}
	mem16[0x3bdc/2] = 0x4e71;
	mem16[0x3bde/2] = 0x4e71;
	mem16[0x3be0/2] = 0x4e71;
	mem16[0x3c0c/2] = 0x4e71;
	mem16[0x3c0e/2] = 0x4e71;
	mem16[0x3c10/2] = 0x4e71;

	mem16[0x3c36/2] = 0x4e71;
	mem16[0x3c38/2] = 0x4e71;
}



/************************ SMA Protection************************
  thanks to Razoola
***************************************************************/

static WRITE16_HANDLER( kof99_bankswitch_w )
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
		0x598000,	/* rest not used? */
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

	neogeo_set_main_cpu_bank_address(space, bankaddress);
}


static WRITE16_HANDLER( garou_bankswitch_w )
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

	neogeo_set_main_cpu_bank_address(space, bankaddress);
}


static WRITE16_HANDLER( garouh_bankswitch_w )
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

	neogeo_set_main_cpu_bank_address(space, bankaddress);
}


static WRITE16_HANDLER( mslug3_bankswitch_w )
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

	neogeo_set_main_cpu_bank_address(space, bankaddress);
}


static WRITE16_HANDLER( kof2000_bankswitch_w )
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

	neogeo_set_main_cpu_bank_address(space, bankaddress);
}


static READ16_HANDLER( prot_9a37_r )
{
	return 0x9a37;
}


/* information about the sma random number generator provided by razoola */
/* this RNG is correct for KOF99, other games might be different */

static READ16_HANDLER( sma_random_r )
{
	neogeo_state *state = space.machine().driver_data<neogeo_state>();
	UINT16 old = state->m_neogeo_rng;

	UINT16 newbit = ((state->m_neogeo_rng >> 2) ^
					 (state->m_neogeo_rng >> 3) ^
					 (state->m_neogeo_rng >> 5) ^
					 (state->m_neogeo_rng >> 6) ^
					 (state->m_neogeo_rng >> 7) ^
					 (state->m_neogeo_rng >>11) ^
					 (state->m_neogeo_rng >>12) ^
					 (state->m_neogeo_rng >>15)) & 1;

	state->m_neogeo_rng = (state->m_neogeo_rng << 1) | newbit;

	return old;
}


void neogeo_reset_rng( running_machine &machine )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	state->m_neogeo_rng = 0x2345;
}


static void sma_install_random_read_handler( running_machine &machine, int addr1, int addr2 )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	state->save_item(NAME(state->m_neogeo_rng));

	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(addr1, addr1 + 1, FUNC(sma_random_r));
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(addr2, addr2 + 1, FUNC(sma_random_r));
}


void kof99_install_protection( running_machine &machine )
{
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x2ffff0, 0x2ffff1, FUNC(kof99_bankswitch_w));
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x2fe446, 0x2fe447, FUNC(prot_9a37_r));

	sma_install_random_read_handler(machine, 0x2ffff8, 0x2ffffa);
}


void garou_install_protection( running_machine &machine )
{
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x2fffc0, 0x2fffc1, FUNC(garou_bankswitch_w));
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x2fe446, 0x2fe447, FUNC(prot_9a37_r));

	sma_install_random_read_handler(machine, 0x2fffcc, 0x2ffff0);
}


void garouh_install_protection( running_machine &machine )
{
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x2fffc0, 0x2fffc1, FUNC(garouh_bankswitch_w));
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x2fe446, 0x2fe447, FUNC(prot_9a37_r));

	sma_install_random_read_handler(machine, 0x2fffcc, 0x2ffff0);
}


void mslug3_install_protection( running_machine &machine )
{
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x2fffe4, 0x2fffe5, FUNC(mslug3_bankswitch_w));
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x2fe446, 0x2fe447, FUNC(prot_9a37_r));

//  sma_install_random_read_handler(machine, 0x2ffff8, 0x2ffffa);
}


void kof2000_install_protection( running_machine &machine )
{
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x2fffec, 0x2fffed, FUNC(kof2000_bankswitch_w));
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x2fe446, 0x2fe447, FUNC(prot_9a37_r));

	sma_install_random_read_handler(machine, 0x2fffd8, 0x2fffda);
}



/************************ PVC Protection ***********************
  mslug5, svcchaos, kof2003
***************************************************************/

static void pvc_w8( running_machine &machine, offs_t offset, UINT8 data )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	*(((UINT8*)state->m_pvc_cartridge_ram) + BYTE_XOR_LE(offset)) = data;
}


static UINT8 pvc_r8( running_machine &machine, offs_t offset )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	return *(((UINT8*)state->m_pvc_cartridge_ram) + BYTE_XOR_LE(offset));
}


static void pvc_prot1( running_machine &machine )
{
	UINT8 b1, b2;

	b1 = pvc_r8(machine, 0x1fe1);
	b2 = pvc_r8(machine, 0x1fe0);
	pvc_w8(machine, 0x1fe2, (((b2 >> 0) & 0xf) << 1) | ((b1 >> 4) & 1));
	pvc_w8(machine, 0x1fe3, (((b2 >> 4) & 0xf) << 1) | ((b1 >> 5) & 1));
	pvc_w8(machine, 0x1fe4, (((b1 >> 0) & 0xf) << 1) | ((b1 >> 6) & 1));
	pvc_w8(machine, 0x1fe5, (b1 >> 7));
}


static void pvc_prot2( running_machine &machine ) // on writes to e8/e9/ea/eb
{
	UINT8 b1, b2, b3, b4;

	b1 = pvc_r8(machine, 0x1fe9);
	b2 = pvc_r8(machine, 0x1fe8);
	b3 = pvc_r8(machine, 0x1feb);
	b4 = pvc_r8(machine, 0x1fea);
	pvc_w8(machine, 0x1fec, (b2 >> 1) | ((b1 >> 1) << 4));
	pvc_w8(machine, 0x1fed, (b4 >> 1) | ((b2 & 1) << 4) | ((b1 & 1) << 5) | ((b4 & 1) << 6) | ((b3 & 1) << 7));
}


static void pvc_write_bankswitch( address_space &space )
{
	neogeo_state *state = space.machine().driver_data<neogeo_state>();
	UINT32 bankaddress;

	bankaddress = ((state->m_pvc_cartridge_ram[0xff8] >> 8)|(state->m_pvc_cartridge_ram[0xff9] << 8));
	*(((UINT8 *)state->m_pvc_cartridge_ram) + BYTE_XOR_LE(0x1ff0)) = 0xa0;
	*(((UINT8 *)state->m_pvc_cartridge_ram) + BYTE_XOR_LE(0x1ff1)) &= 0xfe;
	*(((UINT8 *)state->m_pvc_cartridge_ram) + BYTE_XOR_LE(0x1ff3)) &= 0x7f;
	neogeo_set_main_cpu_bank_address(space, bankaddress + 0x100000);
}


static READ16_HANDLER( pvc_prot_r )
{
	neogeo_state *state = space.machine().driver_data<neogeo_state>();
	return state->m_pvc_cartridge_ram[offset];
}


static WRITE16_HANDLER( pvc_prot_w )
{
	neogeo_state *state = space.machine().driver_data<neogeo_state>();

	COMBINE_DATA(&state->m_pvc_cartridge_ram[offset] );
	if (offset == 0xff0)
		pvc_prot1(space.machine());
	else if(offset >= 0xff4 && offset <= 0xff5)
		pvc_prot2(space.machine());
	else if(offset >= 0xff8)
		pvc_write_bankswitch(space);
}


void install_pvc_protection( running_machine &machine )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	state->m_pvc_cartridge_ram = auto_alloc_array(machine, UINT16, 0x2000 / 2);
	state->save_pointer(NAME(state->m_pvc_cartridge_ram), 0x2000 / 2);

	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x2fe000, 0x2fffff, FUNC(pvc_prot_r), FUNC(pvc_prot_w));
}
