/***************************************************************************

    Neo-Geo hardware

    Many of the NeoGeo bootlegs use their own form of encryption and
    protection, presumably to make them harder for other bootleggser to
    copy.  This encryption often involves non-trivial scrambling of the
    program roms and the games are protected using an Altera chip which
    provides some kind of rom overlay, patching parts of the code.
    The graphics roms are usually scrambled in a different way to the
    official SNK cartridges too.

***************************************************************************/

#include "emu.h"
#include "includes/neogeo.h"


/* General Bootleg Functions - used by more than 1 game */


void neogeo_bootleg_cx_decrypt( running_machine &machine )
{
	int i;
	int cx_size = machine.root_device().memregion( "sprites" )->bytes();
	UINT8 *rom = machine.root_device().memregion( "sprites" )->base();
	UINT8 *buf = auto_alloc_array(machine, UINT8, cx_size );

	memcpy( buf, rom, cx_size );

	for( i = 0; i < cx_size / 0x40; i++ ){
		memcpy( &rom[ i * 0x40 ], &buf[ (i ^ 1) * 0x40 ], 0x40 );
	}

	auto_free( machine, buf );
}


void neogeo_bootleg_sx_decrypt( running_machine &machine, int value )
{
	int sx_size = machine.root_device().memregion( "fixed" )->bytes();
	UINT8 *rom = machine.root_device().memregion( "fixed" )->base();
	int i;

	if (value == 1)
	{
		UINT8 *buf = auto_alloc_array(machine, UINT8, sx_size );
		memcpy( buf, rom, sx_size );

		for( i = 0; i < sx_size; i += 0x10 )
		{
			memcpy( &rom[ i ], &buf[ i + 8 ], 8 );
			memcpy( &rom[ i + 8 ], &buf[ i ], 8 );
		}
		auto_free( machine, buf );
	}
	else if (value == 2)
	{
		for( i = 0; i < sx_size; i++ )
			rom[ i ] = BITSWAP8( rom[ i ], 7, 6, 0, 4, 3, 2, 1, 5 );
	}
}


/* The King of Gladiator (The King of Fighters '97 bootleg) */


/* The protection patching here may be incomplete
   Thanks to Razoola for the info */

void kog_px_decrypt( running_machine &machine )
{
	/* the protection chip does some *very* strange things to the rom */
	UINT8 *src = machine.root_device().memregion("maincpu")->base();
	UINT8 *dst = auto_alloc_array(machine, UINT8, 0x600000 );
	UINT16 *rom = (UINT16 *)machine.root_device().memregion("maincpu")->base();
	int i;
	static const int sec[] = { 0x3, 0x8, 0x7, 0xC, 0x1, 0xA, 0x6, 0xD };

	for (i = 0; i < 8; i++){
		memcpy (dst + i * 0x20000, src + sec[i] * 0x20000, 0x20000);
	}

	memcpy (dst + 0x0007A6, src + 0x0407A6, 0x000006);
	memcpy (dst + 0x0007C6, src + 0x0407C6, 0x000006);
	memcpy (dst + 0x0007E6, src + 0x0407E6, 0x000006);
	memcpy (dst + 0x090000, src + 0x040000, 0x004000);
	memcpy (dst + 0x100000, src + 0x200000, 0x400000);
	memcpy (src, dst, 0x600000);
	auto_free (machine, dst);

	for (i = 0x90000/2; i < 0x94000/2; i++){
		if (((rom[i]&0xFFBF) == 0x4EB9 || rom[i] == 0x43F9) && !rom[i + 1])
			rom[i + 1] = 0x0009;

		if (rom[i] == 0x4EB8)
			rom[i] = 0x6100;
	}

	rom[0x007A8/2] = 0x0009;
	rom[0x007C8/2] = 0x0009;
	rom[0x007E8/2] = 0x0009;
	rom[0x93408/2] = 0xF168;
	rom[0x9340C/2] = 0xFB7A;
	rom[0x924AC/2] = 0x0009;
	rom[0x9251C/2] = 0x0009;
	rom[0x93966/2] = 0xFFDA;
	rom[0x93974/2] = 0xFFCC;
	rom[0x93982/2] = 0xFFBE;
	rom[0x93990/2] = 0xFFB0;
	rom[0x9399E/2] = 0xFFA2;
	rom[0x939AC/2] = 0xFF94;
	rom[0x939BA/2] = 0xFF86;
	rom[0x939C8/2] = 0xFF78;
	rom[0x939D4/2] = 0xFA5C;
	rom[0x939E0/2] = 0xFA50;
	rom[0x939EC/2] = 0xFA44;
	rom[0x939F8/2] = 0xFA38;
	rom[0x93A04/2] = 0xFA2C;
	rom[0x93A10/2] = 0xFA20;
	rom[0x93A1C/2] = 0xFA14;
	rom[0x93A28/2] = 0xFA08;
	rom[0x93A34/2] = 0xF9FC;
	rom[0x93A40/2] = 0xF9F0;
	rom[0x93A4C/2] = 0xFD14;
	rom[0x93A58/2] = 0xFD08;
	rom[0x93A66/2] = 0xF9CA;
	rom[0x93A72/2] = 0xF9BE;

}


/* The King of Fighters 10th Anniversary (The King of Fighters 2002 bootleg) */


/* this uses RAM based tiles for the text layer, however the implementation
  is incomplete, at the moment the S data is copied from the program rom on
  start-up instead */

static UINT16 kof10thExtraRAMB[0x01000];

static void kof10thBankswitch(address_space *space, UINT16 nBank)
{
	UINT32 bank = 0x100000 + ((nBank & 7) << 20);
	if (bank >= 0x700000)
		bank = 0x100000;
	neogeo_set_main_cpu_bank_address(space, bank);
}

static READ16_HANDLER( kof10th_RAMB_r )
{
	return kof10thExtraRAMB[offset];
}

static WRITE16_HANDLER( kof10th_custom_w )
{
	if (!kof10thExtraRAMB[0xFFE]) { // Write to RAM bank A
		UINT16 *prom = (UINT16*)space->machine().root_device().memregion( "maincpu" )->base();
		COMBINE_DATA(&prom[(0xE0000/2) + (offset & 0xFFFF)]);
	} else { // Write S data on-the-fly
		UINT8 *srom = space->machine().root_device().memregion( "fixed" )->base();
		srom[offset] = BITSWAP8(data,7,6,0,4,3,2,1,5);
	}
}

static WRITE16_HANDLER( kof10th_bankswitch_w )
{
	if (offset >= 0x5F000) {
		if (offset == 0x5FFF8) { // Standard bankswitch
			kof10thBankswitch(space, data);
		} else if (offset == 0x5FFFC && kof10thExtraRAMB[0xFFC] != data) { // Special bankswitch
			UINT8 *src = space->machine().root_device().memregion( "maincpu" )->base();
			memcpy (src + 0x10000,  src + ((data & 1) ? 0x810000 : 0x710000), 0xcffff);
		}
		COMBINE_DATA(&kof10thExtraRAMB[offset & 0xFFF]);
	}
}

void install_kof10th_protection ( running_machine &machine )
{
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x2fe000, 0x2fffff, FUNC(kof10th_RAMB_r));
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x200000, 0x23ffff, FUNC(kof10th_custom_w));
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x240000, 0x2fffff, FUNC(kof10th_bankswitch_w));
}

void decrypt_kof10th(running_machine &machine)
{
	int i, j;
	UINT8 *dst = auto_alloc_array(machine, UINT8, 0x900000);
	UINT8 *src = machine.root_device().memregion( "maincpu" )->base();

	memcpy(dst + 0x000000, src + 0x700000, 0x100000); // Correct (Verified in Uni-bios)
	memcpy(dst + 0x100000, src + 0x000000, 0x800000);

	for (i = 0; i < 0x900000; i++) {
		j = BITSWAP24(i,23,22,21,20,19,18,17,16,15,14,13,12,11,2,9,8,7,1,5,4,3,10,6,0);
		src[j] = dst[i];
	}

	auto_free(machine, dst);

	// Altera protection chip patches these over P ROM
	((UINT16*)src)[0x0124/2] = 0x000d; // Enables XOR for RAM moves, forces SoftDIPs, and USA region
	((UINT16*)src)[0x0126/2] = 0xf7a8;

	((UINT16*)src)[0x8bf4/2] = 0x4ef9; // Run code to change "S" data
	((UINT16*)src)[0x8bf6/2] = 0x000d;
	((UINT16*)src)[0x8bf8/2] = 0xf980;
}


/* The King of Fighters 10th Anniversary Extra Plus (The King of Fighters 2002 bootleg) */


void decrypt_kf10thep(running_machine &machine)
{
	int i;
	UINT16 *rom = (UINT16*)machine.root_device().memregion("maincpu")->base();
	UINT8  *src = machine.root_device().memregion("maincpu")->base();
	UINT16 *buf = (UINT16*)machine.root_device().memregion("audiocrypt")->base();
	UINT8 *srom = (UINT8*)machine.root_device().memregion("fixed")->base();
	UINT8 *sbuf = auto_alloc_array(machine, UINT8, 0x20000);

	UINT8 *dst = auto_alloc_array(machine, UINT8, 0x200000);

	memcpy(dst,buf,0x200000);
	memcpy(src+0x000000,dst+0x060000,0x20000);
	memcpy(src+0x020000,dst+0x100000,0x20000);
	memcpy(src+0x040000,dst+0x0e0000,0x20000);
	memcpy(src+0x060000,dst+0x180000,0x20000);
	memcpy(src+0x080000,dst+0x020000,0x20000);
	memcpy(src+0x0a0000,dst+0x140000,0x20000);
	memcpy(src+0x0c0000,dst+0x0c0000,0x20000);
	memcpy(src+0x0e0000,dst+0x1a0000,0x20000);
	memcpy(src+0x0002e0,dst+0x0402e0,0x6a); // copy banked code to a new memory region
	memcpy(src+0x0f92bc,dst+0x0492bc,0xb9e); // copy banked code to a new memory region
	for (i=0xf92bc/2;i < 0xf9e58/2 ;i++)
	{
		if (rom[i+0] == 0x4eb9 && rom[i+1] == 0x0000) rom[i+1] = 0x000F; // correct JSR in moved code
		if (rom[i+0] == 0x4ef9 && rom[i+1] == 0x0000) rom[i+1] = 0x000F; // correct JMP in moved code
	}
	rom[0x00342/2] = 0x000f;
	auto_free(machine, dst);

	for (i=0;i<0x20000;i++)
		sbuf[i]=srom[i^0x8];

	memcpy(srom,sbuf,0x20000);
	auto_free(machine, sbuf);
}


/* The King of Fighters 10th Anniversary 2005 Unique (The King of Fighters 2002 bootleg) */


static void kf2k5uni_px_decrypt( running_machine &machine )
{
	int i, j, ofst;
	UINT8 *src = machine.root_device().memregion( "maincpu" )->base();
	UINT8 *dst = auto_alloc_array(machine, UINT8, 0x80);

	for (i = 0; i < 0x800000; i+=0x80)
	{
		for (j = 0; j < 0x80; j+=2)
		{
			ofst = BITSWAP8(j, 0, 3, 4, 5, 6, 1, 2, 7);
			memcpy(dst + j, src + i + ofst, 2);
		}
		memcpy(src + i, dst, 0x80);
	}
	auto_free(machine, dst);

	memcpy(src, src + 0x600000, 0x100000); // Seems to be the same as kof10th
}

static void kf2k5uni_sx_decrypt( running_machine &machine )
{
	int i;
	UINT8 *srom = machine.root_device().memregion( "fixed" )->base();

	for (i = 0; i < 0x20000; i++)
		srom[i] = BITSWAP8(srom[i], 4, 5, 6, 7, 0, 1, 2, 3);
}

static void kf2k5uni_mx_decrypt( running_machine &machine )
{
	int i;
	UINT8 *mrom = machine.root_device().memregion( "audiocpu" )->base();

	for (i = 0; i < 0x30000; i++)
		mrom[i] = BITSWAP8(mrom[i], 4, 5, 6, 7, 0, 1, 2, 3);
}

void decrypt_kf2k5uni( running_machine &machine )
{
	kf2k5uni_px_decrypt(machine);
	kf2k5uni_sx_decrypt(machine);
	kf2k5uni_mx_decrypt(machine);
}


/* The King of Fighters 2002 (bootleg) */


// Thanks to IQ_132 for the info
void kof2002b_gfx_decrypt(running_machine &machine, UINT8 *src, int size)
{
	int i, j;
	int t[ 8 ][ 10 ] =
	{
		{ 0, 8, 7, 3, 4, 5, 6, 2, 1 },
		{ 1, 0, 8, 4, 5, 3, 7, 6, 2 },
		{ 2, 1, 0, 3, 4, 5, 8, 7, 6 },
		{ 6, 2, 1, 5, 3, 4, 0, 8, 7 },
		{ 7, 6, 2, 5, 3, 4, 1, 0, 8 },
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8 },
		{ 2, 1, 0, 4, 5, 3, 6, 7, 8 },
		{ 8, 0, 7, 3, 4, 5, 6, 2, 1 },
	};

	UINT8 *dst = auto_alloc_array(machine, UINT8,  0x10000 );

	for ( i = 0; i < size; i+=0x10000 )
	{
		memcpy( dst, src+i, 0x10000 );

		for ( j = 0; j < 0x200; j++ )
		{
			int n = (( j % 0x40) / 8 );
			int ofst = BITSWAP16(j, 15, 14, 13, 12, 11, 10, 9, t[n][0], t[n][1], t[n][2],
								 t[n][3], t[n][4], t[n][5], t[n][6], t[n][7], t[n][8]);
			memcpy( src+i+ofst*128, dst+j*128, 128 );
		}
	}
	auto_free( machine, dst );
}


/* The King of Fighters 2002 Magic Plus (bootleg) */


void kf2k2mp_decrypt( running_machine &machine )
{
	int i,j;

	UINT8 *src = machine.root_device().memregion("maincpu")->base();
	UINT8 *dst = auto_alloc_array(machine, UINT8, 0x80);

	memmove(src, src + 0x300000, 0x500000);

	for (i = 0; i < 0x800000; i+=0x80)
	{
		for (j = 0; j < 0x80 / 2; j++)
		{
			int ofst = BITSWAP8( j, 6, 7, 2, 3, 4, 5, 0, 1 );
			memcpy(dst + j * 2, src + i + ofst * 2, 2);
		}
		memcpy(src + i, dst, 0x80);
	}
	auto_free(machine, dst);
}


/* The King of Fighters 2002 Magic Plus II (bootleg) */


void kf2k2mp2_px_decrypt( running_machine &machine )
{
	UINT8 *src = machine.root_device().memregion("maincpu")->base();
	UINT8 *dst = auto_alloc_array(machine, UINT8, 0x600000);

	memcpy (dst + 0x000000, src + 0x1C0000, 0x040000);
	memcpy (dst + 0x040000, src + 0x140000, 0x080000);
	memcpy (dst + 0x0C0000, src + 0x100000, 0x040000);
	memcpy (dst + 0x100000, src + 0x200000, 0x400000);
	memcpy (src + 0x000000, dst + 0x000000, 0x600000);
	auto_free (machine, dst);
}


/* Crouching Tiger Hidden Dragon 2003 (bootleg of King of Fighters 2001) */


/* descrambling information from razoola */
static void cthd2003_neogeo_gfx_address_fix_do(running_machine &machine, int start, int end, int bit3shift, int bit2shift, int bit1shift, int bit0shift)
{
	int i,j;
	int tilesize=128;

	UINT8* rom = auto_alloc_array(machine, UINT8, 16*tilesize);	// 16 tiles buffer
	UINT8* realrom = machine.root_device().memregion("sprites")->base() + start*tilesize;

	for (i = 0; i < (end-start)/16; i++) {
		for (j = 0; j < 16; j++) {
			int offset = (((j&1)>>0)<<bit0shift)
					+(((j&2)>>1)<<bit1shift)
					+(((j&4)>>2)<<bit2shift)
					+(((j&8)>>3)<<bit3shift);

			memcpy(rom+j*tilesize, realrom+offset*tilesize, tilesize);
		}
		memcpy(realrom,rom,tilesize*16);
		realrom+=16*tilesize;
	}
	auto_free(machine, rom);
}

static void cthd2003_neogeo_gfx_address_fix(running_machine &machine, int start, int end)
{
	cthd2003_neogeo_gfx_address_fix_do(machine, start+512*0, end+512*0, 0,3,2,1);
	cthd2003_neogeo_gfx_address_fix_do(machine, start+512*1, end+512*1, 1,0,3,2);
	cthd2003_neogeo_gfx_address_fix_do(machine, start+512*2, end+512*2, 2,1,0,3);
	// skip 3 & 4
	cthd2003_neogeo_gfx_address_fix_do(machine, start+512*5, end+512*5, 0,1,2,3);
	cthd2003_neogeo_gfx_address_fix_do(machine, start+512*6, end+512*6, 0,1,2,3);
	cthd2003_neogeo_gfx_address_fix_do(machine, start+512*7, end+512*7, 0,2,3,1);
}

static void cthd2003_c(running_machine &machine, int pow)
{
	int i;

	for (i=0; i<=192; i+=8)
		cthd2003_neogeo_gfx_address_fix(machine, i*512,i*512+512);

	for (i=200; i<=392; i+=8)
		cthd2003_neogeo_gfx_address_fix(machine, i*512,i*512+512);

	for (i=400; i<=592; i+=8)
		cthd2003_neogeo_gfx_address_fix(machine, i*512,i*512+512);

	for (i=600; i<=792; i+=8)
		cthd2003_neogeo_gfx_address_fix(machine, i*512,i*512+512);

	for (i=800; i<=992; i+=8)
		cthd2003_neogeo_gfx_address_fix(machine, i*512,i*512+512);

	for (i=1000; i<=1016; i+=8)
		cthd2003_neogeo_gfx_address_fix(machine, i*512,i*512+512);
}

void decrypt_cthd2003( running_machine &machine )
{
	UINT8 *romdata = machine.root_device().memregion("fixed")->base();
	UINT8 *tmp = auto_alloc_array(machine, UINT8, 8*128*128);

	memcpy(tmp+8*0*128, romdata+8*0*128, 8*32*128);
	memcpy(tmp+8*32*128, romdata+8*64*128, 8*32*128);
	memcpy(tmp+8*64*128, romdata+8*32*128, 8*32*128);
	memcpy(tmp+8*96*128, romdata+8*96*128, 8*32*128);
	memcpy(romdata, tmp, 8*128*128);

	romdata = machine.root_device().memregion("audiocpu")->base()+0x10000;
	memcpy(tmp+8*0*128, romdata+8*0*128, 8*32*128);
	memcpy(tmp+8*32*128, romdata+8*64*128, 8*32*128);
	memcpy(tmp+8*64*128, romdata+8*32*128, 8*32*128);
	memcpy(tmp+8*96*128, romdata+8*96*128, 8*32*128);
	memcpy(romdata, tmp, 8*128*128);

	auto_free(machine, tmp);

	memcpy(romdata-0x10000,romdata,0x10000);

	cthd2003_c(machine, 0);
}

static WRITE16_HANDLER ( cthd2003_bankswitch_w )
{
	int bankaddress;
	static const int cthd2003_banks[8] =
	{
		1,0,1,0,1,0,3,2,
	};
	if (offset == 0)
	{
		bankaddress = 0x100000 + cthd2003_banks[data&7]*0x100000;
		neogeo_set_main_cpu_bank_address(space, bankaddress);
	}
}

void patch_cthd2003( running_machine &machine )
{
	/* patches thanks to razoola */
	int i;
	UINT16 *mem16 = (UINT16 *)machine.root_device().memregion("maincpu")->base();

	/* special ROM banking handler */
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x2ffff0, 0x2fffff, FUNC(cthd2003_bankswitch_w));

	// theres still a problem on the character select screen but it seems to be related to cpu core timing issues,
	// overclocking the 68k prevents it.

	// fix garbage on s1 layer over everything
	mem16[0xf415a/2] = 0x4ef9;
	mem16[0xf415c/2] = 0x000f;
	mem16[0xf415e/2] = 0x4cf2;
	// Fix corruption in attract mode before title screen
	for (i=0x1ae290/2;i < 0x1ae8d0/2; i=i+1)
	{
		mem16[i] = 0x0000;
	}

	// Fix for title page
	for (i=0x1f8ef0/2;i < 0x1fa1f0/2; i=i+2)
	{
		mem16[i] -= 0x7000;
		mem16[i+1] -= 0x0010;
	}

	// Fix for green dots on title page
	for (i=0xac500/2;i < 0xac520/2; i=i+1)
	{
		mem16[i] = 0xFFFF;
	}
	// Fix for blanks as screen change level end clear
	mem16[0x991d0/2] = 0xdd03;
	mem16[0x99306/2] = 0xdd03;
	mem16[0x99354/2] = 0xdd03;
	mem16[0x9943e/2] = 0xdd03;
}


/* Crouching Tiger Hidden Dragon 2003 Super Plus (bootleg of King of Fighters 2001) */


static void ct2k3sp_sx_decrypt( running_machine &machine )
{
	int rom_size = machine.root_device().memregion( "fixed" )->bytes();
	UINT8 *rom = machine.root_device().memregion( "fixed" )->base();
	UINT8 *buf = auto_alloc_array(machine, UINT8,  rom_size );
	int i;
	int ofst;

	memcpy( buf, rom, rom_size );

	for( i = 0; i < rom_size; i++ ){
		ofst = BITSWAP24( (i & 0x1ffff), 23, 22, 21, 20, 19, 18, 17,  3,
									      0,  1,  4,  2, 13, 14, 16, 15,
										  5,  6, 11, 10,  9,  8,  7, 12 );

		ofst += (i >> 17) << 17;

		rom[ i ] = buf[ ofst ];
	}

	memcpy( buf, rom, rom_size );

	memcpy( &rom[ 0x08000 ], &buf[ 0x10000 ], 0x8000 );
	memcpy( &rom[ 0x10000 ], &buf[ 0x08000 ], 0x8000 );
	memcpy( &rom[ 0x28000 ], &buf[ 0x30000 ], 0x8000 );
	memcpy( &rom[ 0x30000 ], &buf[ 0x28000 ], 0x8000 );

	auto_free( machine, buf );
}

void decrypt_ct2k3sp( running_machine &machine )
{
	UINT8 *romdata = machine.root_device().memregion("audiocpu")->base()+0x10000;
	UINT8*tmp = auto_alloc_array(machine, UINT8, 8*128*128);
	memcpy(tmp+8*0*128, romdata+8*0*128, 8*32*128);
	memcpy(tmp+8*32*128, romdata+8*64*128, 8*32*128);
	memcpy(tmp+8*64*128, romdata+8*32*128, 8*32*128);
	memcpy(tmp+8*96*128, romdata+8*96*128, 8*32*128);
	memcpy(romdata, tmp, 8*128*128);

	auto_free(machine, tmp);
	memcpy(romdata-0x10000,romdata,0x10000);
	ct2k3sp_sx_decrypt(machine);
	cthd2003_c(machine, 0);
}


/* Crouching Tiger Hidden Dragon 2003 Super Plus alternate (bootleg of King of Fighters 2001) */


void decrypt_ct2k3sa( running_machine &machine )
{
	UINT8 *romdata = machine.root_device().memregion("audiocpu")->base()+0x10000;
	UINT8*tmp = auto_alloc_array(machine, UINT8, 8*128*128);
	memcpy(tmp+8*0*128, romdata+8*0*128, 8*32*128);
	memcpy(tmp+8*32*128, romdata+8*64*128, 8*32*128);
	memcpy(tmp+8*64*128, romdata+8*32*128, 8*32*128);
	memcpy(tmp+8*96*128, romdata+8*96*128, 8*32*128);
	memcpy(romdata, tmp, 8*128*128);

	auto_free(machine, tmp);
	memcpy(romdata-0x10000,romdata,0x10000);
	cthd2003_c(machine, 0);
}

void patch_ct2k3sa( running_machine &machine )
{
	/* patches thanks to razoola - same as for cthd2003*/
	int i;
	UINT16 *mem16 = (UINT16 *)machine.root_device().memregion("maincpu")->base();

	// theres still a problem on the character select screen but it seems to be related to cpu core timing issues,
	// overclocking the 68k prevents it.

	// fix garbage on s1 layer over everything
	mem16[0xf415a/2] = 0x4ef9;
	mem16[0xf415c/2] = 0x000f;
	mem16[0xf415e/2] = 0x4cf2;

	// Fix corruption in attract mode before title screen
	for (i=0x1ae290/2;i < 0x1ae8d0/2; i=i+1)
	{
		mem16[i] = 0x0000;
	}

	// Fix for title page
	for (i=0x1f8ef0/2;i < 0x1fa1f0/2; i=i+2)
	{
		mem16[i] -= 0x7000;
		mem16[i+1] -= 0x0010;
	}

	// Fix for green dots on title page
	for (i=0xac500/2;i < 0xac520/2; i=i+1)
	{
		mem16[i] = 0xFFFF;
	}
	// Fix for blanks as screen change level end clear
	mem16[0x991d0/2] = 0xdd03;
	mem16[0x99306/2] = 0xdd03;
	mem16[0x99354/2] = 0xdd03;
	mem16[0x9943e/2] = 0xdd03;
}


/* King of Fighters Special Edition 2004 (bootleg of King of Fighters 2002) */


void decrypt_kof2k4se_68k( running_machine &machine )
{
	UINT8 *src = machine.root_device().memregion("maincpu")->base()+0x100000;
	UINT8 *dst = auto_alloc_array(machine, UINT8, 0x400000);
	int i;
	static const int sec[] = {0x300000,0x200000,0x100000,0x000000};
	memcpy(dst,src,0x400000);

	for(i = 0; i < 4; ++i)
	{
		memcpy(src+i*0x100000,dst+sec[i],0x100000);
	}
	auto_free(machine, dst);
}


/* Lansquenet 2004 (Shock Troopers - 2nd Squad bootleg) */


void lans2004_vx_decrypt( running_machine &machine )
{
	int i;
	UINT8 *rom = machine.root_device().memregion( "ymsnd" )->base();
	for (i = 0; i < 0xA00000; i++)
		rom[i] = BITSWAP8(rom[i], 0, 1, 5, 4, 3, 2, 6, 7);
}

void lans2004_decrypt_68k( running_machine &machine )
{
	/* Descrambling P ROMs - Thanks to Razoola for the info */
	int i;
	UINT8 *src = machine.root_device().memregion( "maincpu" )->base();
	UINT16 *rom = (UINT16*)machine.root_device().memregion( "maincpu" )->base();
	UINT8 *dst = auto_alloc_array(machine, UINT8, 0x600000);

	{
		static const int sec[] = { 0x3, 0x8, 0x7, 0xC, 0x1, 0xA, 0x6, 0xD };

		for (i = 0; i < 8; i++)
			memcpy (dst + i * 0x20000, src + sec[i] * 0x20000, 0x20000);

		memcpy (dst + 0x0BBB00, src + 0x045B00, 0x001710);
		memcpy (dst + 0x02FFF0, src + 0x1A92BE, 0x000010);
		memcpy (dst + 0x100000, src + 0x200000, 0x400000);
		memcpy (src, dst, 0x600000);
		auto_free (machine, dst);
	}

	for (i = 0xBBB00/2; i < 0xBE000/2; i++) {
		if ((((rom[i]&0xFFBF)==0x4EB9) || ((rom[i]&0xFFBF)==0x43B9)) && (rom[i+1]==0x0000)) {
			rom[i + 1] = 0x000B;
			rom[i + 2] += 0x6000;
		}
	}

	/* Patched by protection chip (Altera) ? */
	rom[0x2D15C/2] = 0x000B;
	rom[0x2D15E/2] = 0xBB00;
	rom[0x2D1E4/2] = 0x6002;
	rom[0x2EA7E/2] = 0x6002;
	rom[0xBBCD0/2] = 0x6002;
	rom[0xBBDF2/2] = 0x6002;
	rom[0xBBE42/2] = 0x6002;
}


/* Metal Slug 5 Plus (bootleg) */


static READ16_HANDLER( mslug5_prot_r )
{
	logerror("PC %06x: access protected\n",cpu_get_pc(&space->device()));
	return 0xa0;
}

static WRITE16_HANDLER ( ms5plus_bankswitch_w )
{
	int bankaddress;
	logerror("offset: %06x PC %06x: set banking %04x\n",offset,cpu_get_pc(&space->device()),data);
	if ((offset == 0)&&(data == 0xa0))
	{
		bankaddress=0xa0;
		neogeo_set_main_cpu_bank_address(space, bankaddress);
		logerror("offset: %06x PC %06x: set banking %04x\n\n",offset,cpu_get_pc(&space->device()),bankaddress);
	}
	else if(offset == 2)
	{
		data=data>>4;
		//data=data&7;
		bankaddress=data*0x100000;
		neogeo_set_main_cpu_bank_address(space, bankaddress);
		logerror("offset: %06x PC %06x: set banking %04x\n\n",offset,cpu_get_pc(&space->device()),bankaddress);
	}
}

void install_ms5plus_protection(running_machine &machine)
{
	// special ROM banking handler / additional protection
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x2ffff0, 0x2fffff,FUNC(mslug5_prot_r), FUNC(ms5plus_bankswitch_w));
}


/* SNK vs. CAPCOM SVC CHAOS (bootleg) */


void svcboot_px_decrypt( running_machine &machine )
{
	static const UINT8 sec[] = {
		0x06, 0x07, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00
	};
	int i;
	int size = machine.root_device().memregion( "maincpu" )->bytes();
	UINT8 *src = machine.root_device().memregion( "maincpu" )->base();
	UINT8 *dst = auto_alloc_array(machine, UINT8,  size );
	int ofst;
	for( i = 0; i < size / 0x100000; i++ ){
		memcpy( &dst[ i * 0x100000 ], &src[ sec[ i ] * 0x100000 ], 0x100000 );
	}
	for( i = 0; i < size / 2; i++ ){
		ofst = BITSWAP8( (i & 0x0000ff), 7, 6, 1, 0, 3, 2, 5, 4 );
		ofst += (i & 0xffff00);
		memcpy( &src[ i * 2 ], &dst[ ofst * 2 ], 0x02 );
	}
	auto_free( machine, dst );
}

void svcboot_cx_decrypt( running_machine &machine )
{
	static const UINT8 idx_tbl[ 0x10 ] = {
		0, 1, 0, 1, 2, 3, 2, 3, 3, 4, 3, 4, 4, 5, 4, 5,
	};
	static const UINT8 bitswap4_tbl[ 6 ][ 4 ] = {
		{ 3, 0, 1, 2 },
		{ 2, 3, 0, 1 },
		{ 1, 2, 3, 0 },
		{ 0, 1, 2, 3 },
		{ 3, 2, 1, 0 },
		{ 3, 0, 2, 1 },
	};
	int i;
	int size = machine.root_device().memregion( "sprites" )->bytes();
	UINT8 *src = machine.root_device().memregion( "sprites" )->base();
	UINT8 *dst = auto_alloc_array(machine, UINT8,  size );
	int ofst;
	memcpy( dst, src, size );
	for( i = 0; i < size / 0x80; i++ ){
		int idx = idx_tbl[ (i & 0xf00) >> 8 ];
		int bit0 = bitswap4_tbl[ idx ][ 0 ];
		int bit1 = bitswap4_tbl[ idx ][ 1 ];
		int bit2 = bitswap4_tbl[ idx ][ 2 ];
		int bit3 = bitswap4_tbl[ idx ][ 3 ];
		ofst = BITSWAP8( (i & 0x0000ff), 7, 6, 5, 4, bit3, bit2, bit1, bit0 );
		ofst += (i & 0xfffff00);
		memcpy( &src[ i * 0x80 ], &dst[ ofst * 0x80 ], 0x80 );
	}
	auto_free( machine, dst );
}


/* SNK vs. CAPCOM SVC CHAOS Plus (bootleg set 1) */


void svcplus_px_decrypt( running_machine &machine )
{
	static const int sec[] = {
		0x00, 0x03, 0x02, 0x05, 0x04, 0x01
	};
	int size = machine.root_device().memregion( "maincpu" )->bytes();
	UINT8 *src = machine.root_device().memregion( "maincpu" )->base();
	UINT8 *dst = auto_alloc_array(machine, UINT8,  size );
	int i;
	int ofst;
	memcpy( dst, src, size );
	for( i = 0; i < size / 2; i++ ){
		ofst = BITSWAP24( (i & 0xfffff), 0x17, 0x16, 0x15, 0x14, 0x13, 0x00, 0x01, 0x02,
										 0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08,
										 0x07, 0x06, 0x05, 0x04, 0x03, 0x10, 0x11, 0x12 );
		ofst ^= 0x0f0007;
		ofst += (i & 0xff00000);
		memcpy( &src[ i * 0x02 ], &dst[ ofst * 0x02 ], 0x02 );
	}
	memcpy( dst, src, size );
	for( i = 0; i < 6; i++ ){
		memcpy( &src[ i * 0x100000 ], &dst[ sec[ i ] * 0x100000 ], 0x100000 );
	}
	auto_free( machine, dst );
}

void svcplus_px_hack( running_machine &machine )
{
	/* patched by the protection chip? */
	UINT8 *src = machine.root_device().memregion( "maincpu" )->base();
	src[ 0x0f8010 ] = 0x40;
	src[ 0x0f8011 ] = 0x04;
	src[ 0x0f8012 ] = 0x00;
	src[ 0x0f8013 ] = 0x10;
	src[ 0x0f8014 ] = 0x40;
	src[ 0x0f8015 ] = 0x46;
	src[ 0x0f8016 ] = 0xc1;
	src[ 0x0f802c ] = 0x16;
}


/* SNK vs. CAPCOM SVC CHAOS Plus (bootleg set 2) */


void svcplusa_px_decrypt( running_machine &machine )
{
	int i;
	static const int sec[] = {
		0x01, 0x02, 0x03, 0x04, 0x05, 0x00
	};
	int size = machine.root_device().memregion( "maincpu" )->bytes();
	UINT8 *src = machine.root_device().memregion( "maincpu" )->base();
	UINT8 *dst = auto_alloc_array(machine, UINT8,  size );
	memcpy( dst, src, size );
	for( i = 0; i < 6; i++ ){
		memcpy( &src[ i * 0x100000 ], &dst[ sec[ i ] * 0x100000 ], 0x100000 );
	}
	auto_free( machine, dst );
}


/* SNK vs. CAPCOM SVC CHAOS Super Plus (bootleg) */


void svcsplus_px_decrypt( running_machine &machine )
{
	static const int sec[] = {
		0x06, 0x07, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00
	};
	int size = machine.root_device().memregion( "maincpu" )->bytes();
	UINT8 *src = machine.root_device().memregion( "maincpu" )->base();
	UINT8 *dst = auto_alloc_array(machine, UINT8,  size );
	int i;
	int ofst;
	memcpy( dst, src, size );
	for( i = 0; i < size / 2; i++ ){
		ofst = BITSWAP16( (i & 0x007fff), 0x0f, 0x00, 0x08, 0x09, 0x0b, 0x0a, 0x0c, 0x0d,
										  0x04, 0x03, 0x01, 0x07, 0x06, 0x02, 0x05, 0x0e );

		ofst += (i & 0x078000);
		ofst += sec[ (i & 0xf80000) >> 19 ] << 19;
		memcpy( &src[ i * 2 ], &dst[ ofst * 2 ], 0x02 );
	}
	auto_free( machine, dst );
}

void svcsplus_px_hack( running_machine &machine )
{
	/* patched by the protection chip? */
	UINT16 *mem16 = (UINT16 *)machine.root_device().memregion("maincpu")->base();
	mem16[0x9e90/2] = 0x000f;
	mem16[0x9e92/2] = 0xc9c0;
	mem16[0xa10c/2] = 0x4eb9;
	mem16[0xa10e/2] = 0x000e;
	mem16[0xa110/2] = 0x9750;
}

#ifdef UNUSED_FUNCTION
static UINT16 mv0_bank_ram[ 0x10/2 ];

static READ16_HANDLER( mv0_bankswitch_r )
{
    return mv0_bank_ram[ offset ];
}

static WRITE16_HANDLER( mv0_bankswitch_w )
{
    UINT32 bankaddress = (mv0_bank_ram[ 0 ] >> 8) + (mv0_bank_ram[ 1 ] << 8) + 0x100000;
	COMBINE_DATA( &mv0_bank_ram[ offset ] );
    neogeo_set_main_cpu_bank_address( space, bankaddress );
}
#endif


/* The King of Fighters 2003 (bootleg set 1) */


static UINT16 kof2003_tbl[4096];

static READ16_HANDLER( kof2003_r)
{
	return kof2003_tbl[offset];
}

static WRITE16_HANDLER( kof2003_w )
{
	data = COMBINE_DATA(&kof2003_tbl[offset]);
	if (offset == 0x1ff0/2 || offset == 0x1ff2/2) {
		UINT8* cr = (UINT8 *)kof2003_tbl;
		UINT32 address = (cr[BYTE_XOR_LE(0x1ff3)]<<16)|(cr[BYTE_XOR_LE(0x1ff2)]<<8)|cr[BYTE_XOR_LE(0x1ff1)];
		UINT8 prt = cr[BYTE_XOR_LE(0x1ff2)];
		UINT8* mem = (UINT8 *)space->machine().root_device().memregion("maincpu")->base();

		cr[BYTE_XOR_LE(0x1ff0)] =  0xa0;
		cr[BYTE_XOR_LE(0x1ff1)] &= 0xfe;
		cr[BYTE_XOR_LE(0x1ff3)] &= 0x7f;
		neogeo_set_main_cpu_bank_address(space, address+0x100000);

		mem[BYTE_XOR_LE(0x58196)] = prt;
	}
}

static WRITE16_HANDLER( kof2003p_w )
{
	data = COMBINE_DATA(&kof2003_tbl[offset]);
	if (offset == 0x1ff0/2 || offset == 0x1ff2/2) {
		UINT8* cr = (UINT8 *)kof2003_tbl;
		UINT32 address = (cr[BYTE_XOR_LE(0x1ff3)]<<16)|(cr[BYTE_XOR_LE(0x1ff2)]<<8)|cr[BYTE_XOR_LE(0x1ff0)];
		UINT8 prt = cr[BYTE_XOR_LE(0x1ff2)];
		UINT8* mem = (UINT8 *)space->machine().root_device().memregion("maincpu")->base();

		cr[BYTE_XOR_LE(0x1ff0)] &= 0xfe;
		cr[BYTE_XOR_LE(0x1ff3)] &= 0x7f;
		neogeo_set_main_cpu_bank_address(space, address+0x100000);

		mem[BYTE_XOR_LE(0x58196)] = prt;
	}
}

void kf2k3bl_px_decrypt( running_machine &machine )
{
	int i;
	static const UINT8 sec[] = {
		0x07, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06
	};

    int rom_size = 0x800000;
    UINT8 *rom = machine.root_device().memregion( "maincpu" )->base();
    UINT8 *buf = auto_alloc_array(machine, UINT8,  rom_size );
    memcpy( buf, rom, rom_size );

    for( i = 0; i < rom_size / 0x100000; i++ ){
        memcpy( &rom[ i * 0x100000 ], &buf[ sec[ i ] * 0x100000 ], 0x100000 );
    }
    auto_free( machine, buf );
}

void kf2k3bl_install_protection(running_machine &machine)
{
    machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x2fe000, 0x2fffff, FUNC(kof2003_r), FUNC(kof2003_w) );
}


/* The King of Fighters 2004 Plus / Hero (The King of Fighters 2003 bootleg) */


void kf2k3pl_px_decrypt( running_machine &machine )
{
	UINT16*tmp = auto_alloc_array(machine, UINT16, 0x100000/2);
	UINT16*rom = (UINT16*)machine.root_device().memregion( "maincpu" )->base();
	int j;
	int i;

	for (i = 0;i < 0x700000/2;i+=0x100000/2)
	{
		memcpy(tmp,&rom[i],0x100000);
		for (j = 0;j < 0x100000/2;j++)
			rom[i+j] = tmp[BITSWAP24(j,23,22,21,20,19,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18)];
	}
	auto_free(machine, tmp);

	/* patched by Altera protection chip on PCB */
	rom[0xf38ac/2] = 0x4e75;
}

void kf2k3pl_install_protection(running_machine &machine)
{
    machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x2fe000, 0x2fffff, FUNC(kof2003_r), FUNC(kof2003p_w) );
}


/* The King of Fighters 2004 Ultra Plus (The King of Fighters 2003 bootleg) */


void kf2k3upl_px_decrypt( running_machine &machine )
{
	{
		UINT8 *src = machine.root_device().memregion("maincpu")->base();
		memmove(src+0x100000, src, 0x600000);
		memmove(src, src+0x700000, 0x100000);
	}

	{

		int ofst;
		int i;
		UINT8 *rom = machine.root_device().memregion( "maincpu" )->base() + 0xfe000;
		UINT8 *buf = machine.root_device().memregion( "maincpu" )->base() + 0xd0610;

		for( i = 0; i < 0x2000 / 2; i++ ){
			ofst = (i & 0xff00) + BITSWAP8( (i & 0x00ff), 7, 6, 0, 4, 3, 2, 1, 5 );
			memcpy( &rom[ i * 2 ], &buf[ ofst * 2 ], 2 );
		}
	}
}

void kf2k3upl_install_protection(running_machine &machine)
{
    machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x2fe000, 0x2fffff, FUNC(kof2003_r), FUNC(kof2003_w) );
}


/* Samurai Shodown V / Samurai Spirits Zero (bootleg) */


void samsho5b_px_decrypt( running_machine &machine )
{
	int px_size = machine.root_device().memregion( "maincpu" )->bytes();
	UINT8 *rom = machine.root_device().memregion( "maincpu" )->base();
	UINT8 *buf = auto_alloc_array(machine, UINT8,  px_size );
	int ofst;
	int i;

	memcpy( buf, rom, px_size );

	for( i = 0; i < px_size / 2; i++ ){
		ofst = BITSWAP8( (i & 0x000ff), 7, 6, 5, 4, 3, 0, 1, 2 );
		ofst += (i & 0xfffff00);
		ofst ^= 0x060005;

		memcpy( &rom[ i * 2 ], &buf[ ofst * 2 ], 0x02 );
	}

	memcpy( buf, rom, px_size );

	memcpy( &rom[ 0x000000 ], &buf[ 0x700000 ], 0x100000 );
	memcpy( &rom[ 0x100000 ], &buf[ 0x000000 ], 0x700000 );

	auto_free( machine, buf );
}


void samsho5b_vx_decrypt( running_machine &machine )
{
	int vx_size = machine.root_device().memregion( "ymsnd" )->bytes();
	UINT8 *rom = machine.root_device().memregion( "ymsnd" )->base();
	int i;

	for( i = 0; i < vx_size; i++ )
		rom[ i ] = BITSWAP8( rom[ i ], 0, 1, 5, 4, 3, 2, 6, 7 );
}


/* Matrimelee / Shin Gouketsuji Ichizoku Toukon (bootleg) */


#define MATRIMBLZ80( i ) ( i^(BITSWAP8(i&0x3,4,3,1,2,0,7,6,5)<<8) )

void matrimbl_decrypt( running_machine &machine )
{
	/* decrypt Z80 */
	UINT8 *rom = machine.root_device().memregion( "audiocpu" )->base()+0x10000;
	UINT8 *buf = auto_alloc_array(machine, UINT8,  0x20000 );
	int i, j=0;
	memcpy( buf, rom, 0x20000 );
	for( i=0x00000; i<0x20000; i++ )
	{
		if ( i&0x10000 )
		{
			if ( i&0x800 )
			{
				j=MATRIMBLZ80( i );
				j=j^0x10000;
			}
			else
			{
				j=MATRIMBLZ80(( i^0x01 ));
			}
		}
		else
		{
			if ( i&0x800 )
			{
				j=MATRIMBLZ80(( i^0x01 ));
				j=j^0x10000;
			}
			else
			{
				j=MATRIMBLZ80( i );
			}
		}
		rom[ j ]=buf[ i ];
	}
	auto_free( machine, buf );
	memcpy( rom-0x10000, rom, 0x10000 );

	/* decrypt gfx */
	cthd2003_c( machine, 0 );
}

