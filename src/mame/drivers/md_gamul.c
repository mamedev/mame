/*--------------------------------------- NOTICE ------------------------------------

 this file is not a key component of MAME and the sets included here are not
 available in a normal build.

 The sets included in this driver file are intended solely to aid in the deveopment
 of the Genesis emulation in MAME.  The Megadrive / Genesis form the basis of a number
 of official Sega Arcade PCBs as well as a selection of bootlegs; the arcade titles
 however do not provide a sufficient test-bed to ensure high quality emulation.

 If changes are made to the Genesis emulation they should be regression tested against
 a full range of titles.  By including accsesible support for a range of sets such
 bug-fixing and testing becomes easier.

----------------------------------------- NOTICE ----------------------------------*/



/* Unlicensed Games
 -- Mostly Asian titles
 -- 'Unlicensed' but more official games are in the main driver
     (accolade / electronic arts stuff)

 -- simple clones of licensed games in main driver


  Many unlicensed games have additional copy protection
  which requires per-game handling

*/

#include "driver.h"
#include "megadriv.h"
#include "cpu/m68000/m68000.h"


int g_l3alt_pdat;
int g_l3alt_pcmd;


WRITE16_HANDLER( g_l3alt_pdat_w )
{
	g_l3alt_pdat = data;
}

WRITE16_HANDLER( g_l3alt_pcmd_w )
{
	g_l3alt_pcmd = data;
}

READ16_HANDLER( g_l3alt_prot_r )
{
	int retdata = 0;

	offset &=0x7;

	switch (offset)
	{

		case 2:

			switch (g_l3alt_pcmd)
			{
				case 1:
					retdata = g_l3alt_pdat>>1;
					break;

				case 2:
					retdata = g_l3alt_pdat>>4;
					retdata |= (g_l3alt_pdat&0x0f)<<4;
					break;

				default:
					//printf("unk prot case %d\n",g_l3alt_pcmd);
					retdata = (((g_l3alt_pdat>>7)&1)<<0);
					retdata |=(((g_l3alt_pdat>>6)&1)<<1);
					retdata |=(((g_l3alt_pdat>>5)&1)<<2);
					retdata |=(((g_l3alt_pdat>>4)&1)<<3);
					retdata |=(((g_l3alt_pdat>>3)&1)<<4);
					retdata |=(((g_l3alt_pdat>>2)&1)<<5);
					retdata |=(((g_l3alt_pdat>>1)&1)<<6);
					retdata |=(((g_l3alt_pdat>>0)&1)<<7);
					break;
			}
			break;

		default:

			printf("protection read, unknown offset\n");
			break;
	}

//	printf("%06x: g_l3alt_pdat_w %04x g_l3alt_pcmd_w %04x return %04x\n",activecpu_get_pc(), g_l3alt_pdat,g_l3alt_pcmd,retdata);

	return retdata;

}

WRITE16_HANDLER( g_l3alt_protw )
{
	offset &=0x7;

	switch (offset)
	{
		case 0x0:
			g_l3alt_pdat = data;
			break;
		case 0x1:
			g_l3alt_pcmd = data;
			break;
		default:
			printf("protection write, unknown offst\n");
			break;
	}
}

WRITE16_HANDLER( g_l3alt_bank_w )
{
	offset &=0x7;

	switch (offset)
	{
		case 0:
		{
		UINT8 *ROM = memory_region(Machine, "main");
		printf("%06x data %04x\n",activecpu_get_pc(), data);
		memcpy(ROM + 0x000000, ROM + 0x400000+(data&0xffff)*0x8000, 0x8000);
		}
		break;

		default:
		printf("unk bank w\n");
		break;

	}

}


DRIVER_INIT( g_l3alt )
{
	UINT8 *ROM = memory_region(Machine, "main");
	memcpy(ROM + 0x00000, ROM + 0x400000, 0x400000); /* default rom */

	memory_install_write16_handler(Machine,0, ADDRESS_SPACE_PROGRAM, 0x600000, 0x6fffff, 0, 0, g_l3alt_protw );
	memory_install_write16_handler(Machine,0, ADDRESS_SPACE_PROGRAM, 0x700000, 0x7fffff, 0, 0, g_l3alt_bank_w );
	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0x600000, 0x6fffff, 0, 0, g_l3alt_prot_r );


	DRIVER_INIT_CALL(megadriv);

}

WRITE16_HANDLER( g_kaiju_bank_w )
{
	UINT8 *ROM = memory_region(Machine, "main");
	printf("%06x data %04x\n",activecpu_get_pc(), data);
	memcpy(ROM + 0x000000, ROM + 0x400000+(data&0x7f)*0x8000, 0x8000);
}

DRIVER_INIT( g_kaiju )
{
	UINT8 *ROM = memory_region(Machine, "main");
	memcpy(ROM + 0x00000, ROM + 0x400000, 0x400000); /* default rom */

	// 00770076
	memory_install_write16_handler(Machine,0, ADDRESS_SPACE_PROGRAM, 0x700000, 0x7fffff, 0, 0, g_kaiju_bank_w );

	DRIVER_INIT_CALL(megadriv);
}

/* Top Fighter - there is more to this.. No display at startup etc.. */

READ16_HANDLER( g_topfig_0x6BD294_r ) /* colours on title screen */
{
	static int x = -1;

/*
cpu #0 (PC=00177192): unmapped program memory word write to 006BD240 = 00A8 & 00FF
cpu #0 (PC=0017719A): unmapped program memory word write to 006BD2D2 = 0098 & 00FF
cpu #0 (PC=001771A2): unmapped program memory word read from 006BD294 & 00FF
*/

	if (activecpu_get_pc()==0x1771a2) return 0x50;
	else
	{
		x++;
		printf("%06x g_topfig_0x6BD294_r %04x\n",activecpu_get_pc(), x);
		return x;
	}
}

READ16_HANDLER( g_topfig_0x6F5344_r ) // after char select
{
	static int x = -1;

	if (activecpu_get_pc()==0x4C94E)
	{
		return activecpu_get_reg(M68K_D0)&0xff;
	}
	else
	{
		x++;
		printf("%06x g_topfig_0x6F5344_r %04x\n",activecpu_get_pc(), x);
		return x;
	}
}



WRITE16_HANDLER( g_topfig_bank_w )
{
	UINT8 *ROM = memory_region(Machine, "main");
	if (data==0x002A)
	{
		memcpy(ROM + 0x060000, ROM + 0x570000, 0x8000); // == 0x2e*0x8000?!
	//	printf("%06x offset %06x, data %04x\n",activecpu_get_pc(), offset, data);

	}
	else if (data==0x0035) // characters ingame
	{
		memcpy(ROM + 0x020000, ROM + 0x5a8000, 0x8000); // == 0x35*0x8000
	}
	else if (data==0x000f) // special moves
	{
		memcpy(ROM + 0x058000, ROM + 0x478000, 0x8000); // == 0xf*0x8000
	}
	else if (data==0x0000)
	{
		memcpy(ROM + 0x060000, ROM + 0x460000, 0x8000);
		memcpy(ROM + 0x020000, ROM + 0x420000, 0x8000);
		memcpy(ROM + 0x058000, ROM + 0x458000, 0x8000);
	//	printf("%06x offset %06x, data %04x\n",activecpu_get_pc(), offset, data);
	}
	else
	{
		printf("%06x offset %06x, data %04x\n",activecpu_get_pc(), offset, data);
	}

}

READ16_HANDLER( g_topfig_0x645B44_r )
{
//cpu #0 (PC=0004DE00): unmapped program memory word write to 00689B80 = 004A & 00FF
//cpu #0 (PC=0004DE08): unmapped program memory word write to 00 = 00B5 & 00FF
//cpu #0 (PC=0004DE0C): unmapped program memory word read from 00645B44 & 00FF

	return 0x9f;//0x25;
}

//cpu #0 (PC=0004CBAE): unmapped program memory word read from 006A35D4 & 00FF -- wants regD7

DRIVER_INIT( g_topfig )
{
	UINT8 *ROM = memory_region(Machine, "main");
	memcpy(ROM + 0x00000, ROM + 0x400000, 0x400000); /* default rom */



	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0x6F5344, 0x6F5345, 0, 0, g_topfig_0x6F5344_r );
	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0x6BD294, 0x6BD295, 0, 0, g_topfig_0x6BD294_r );
	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0x645B44, 0x645B45, 0, 0, g_topfig_0x645B44_r );

	memory_install_write16_handler(Machine,0, ADDRESS_SPACE_PROGRAM, 0x689B80, 0x689B81, 0, 0, SMH_NOP );
	memory_install_write16_handler(Machine,0, ADDRESS_SPACE_PROGRAM, 0x6D8B02, 0x6D8B03, 0, 0, SMH_NOP );


	memory_install_write16_handler(Machine,0, ADDRESS_SPACE_PROGRAM, 0x700000, 0x7fffff, 0, 0, g_topfig_bank_w );

	DRIVER_INIT_CALL(megadriv);


}

WRITE16_HANDLER( g_chifi3_bank_w )
{
	UINT8 *ROM = memory_region(Machine, "main");

	if (data==0xf100) // *hit player
	{
		int x;
		for (x=0;x<0x100000;x+=0x10000)
		{
			memcpy(ROM + x, ROM + 0x410000, 0x10000);
		}
	}
	else if (data==0xd700) // title screen..
	{
		int x;
		for (x=0;x<0x100000;x+=0x10000)
		{
			memcpy(ROM + x, ROM + 0x470000, 0x10000);
		}
	}
	else if (data==0xd300) // character hits floor
	{
		int x;
		for (x=0;x<0x100000;x+=0x10000)
		{
			memcpy(ROM + x, ROM + 0x430000, 0x10000);
		}
	}
	else if (data==0x0000)
	{
		int x;
		for (x=0;x<0x100000;x+=0x10000)
		{
			memcpy(ROM + x, ROM + 0x400000+x, 0x10000);
		}
	}
	else
	{
		printf("%06x chifi3, bankw? %04x %04x\n",activecpu_get_pc(), offset,data);
	}

}

READ16_HANDLER( g_chifi3_prot_r )
{
	UINT32 retdat;
	/* not 100% correct, there may be some relationship between the reads here
	and the writes made at the start of the game.. */

	/*
	04dc10 chifi3, prot_r? 2800
	04cefa chifi3, prot_r? 65262
	*/

	if (activecpu_get_pc()==0x01782) // makes 'VS' screen appear
	{
		retdat = activecpu_get_reg(M68K_D3)&0xff;
		retdat<<=8;
		return retdat;
	}
	else if (activecpu_get_pc()==0x1c24) // background gfx etc.
	{
		retdat = activecpu_get_reg(M68K_D3)&0xff;
		retdat<<=8;
		return retdat;
	}
	else if (activecpu_get_pc()==0x10c4a) // unknown
	{
		return mame_rand(Machine);
	}
	else if (activecpu_get_pc()==0x10c50) // unknown
	{
		return mame_rand(Machine);
	}
	else if (activecpu_get_pc()==0x10c52) // relates to the game speed..
	{
		retdat = activecpu_get_reg(M68K_D4)&0xff;
		retdat<<=8;
		return retdat;
	}
	else if (activecpu_get_pc()==0x061ae)
	{
		retdat = activecpu_get_reg(M68K_D3)&0xff;
		retdat<<=8;
		return retdat;
	}
	else if (activecpu_get_pc()==0x061b0)
	{
		retdat = activecpu_get_reg(M68K_D3)&0xff;
		retdat<<=8;
		return retdat;
	}
	else
	{
		printf("%06x chifi3, prot_r? %04x\n",activecpu_get_pc(), offset);
	}

	return 0;
}

DRIVER_INIT( g_chifi3 )
{
	UINT8 *ROM = memory_region(Machine, "main");
	memcpy(ROM + 0x00000, ROM + 0x400000, 0x400000); /* default rom */

	DRIVER_INIT_CALL(megadriv);

	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0x400000, 0x4fffff, 0, 0, g_chifi3_prot_r );
	memory_install_write16_handler(Machine,0, ADDRESS_SPACE_PROGRAM, 0x600000, 0x6fffff, 0, 0, g_chifi3_bank_w );

}



static int squirrel_king_extra;

READ16_HANDLER( squirrel_king_extra_r )
{
	return squirrel_king_extra;
}
WRITE16_HANDLER( squirrel_king_extra_w )
{
	squirrel_king_extra = data;
}

DRIVER_INIT( g_squi )
{
	DRIVER_INIT_CALL(megadriv);

	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0x400000, 0x400007, 0, 0, squirrel_king_extra_r);
	memory_install_write16_handler(Machine,0, ADDRESS_SPACE_PROGRAM, 0x400000, 0x400007, 0, 0, squirrel_king_extra_w);
}

READ16_HANDLER( sbub_extra1_r )
{
	return 0x5500;
}

READ16_HANDLER( sbub_extra2_r )
{
	return 0x0f00;
}

READ16_HANDLER( bugl_extra_r )
{
	return 0x28;
}

DRIVER_INIT( g_sbub )
{
	DRIVER_INIT_CALL(megadriv);

	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0x400000, 0x400001, 0, 0, sbub_extra1_r);
	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0x400002, 0x400003, 0, 0, sbub_extra2_r);

}

READ16_HANDLER( smb2_extra_r )
{
	return 0xa;
}

READ16_HANDLER( rx3_extra_r )
{
	return 0xc;
}

DRIVER_INIT( g_smb2 )
{
	DRIVER_INIT_CALL(megadriv);

	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0xA13000, 0xA13001, 0, 0, smb2_extra_r);

}

DRIVER_INIT( g_bugl )
{
	DRIVER_INIT_CALL(megadriv);
	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0xA13000, 0xA13001, 0, 0, bugl_extra_r);
}

DRIVER_INIT( g_rx3 )
{
	DRIVER_INIT_CALL(megadriv);
	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0xA13000, 0xA13001, 0, 0, rx3_extra_r);
}


WRITE16_HANDLER( g_12i1_bank_w )
{
	UINT8 *ROM = memory_region(Machine, "main");
	printf("offset %06x",offset<<17);
	memcpy(ROM + 0x000000, ROM + 0x400000+((offset&0x3f)<<17), 0x100000);
}


DRIVER_INIT( g_12i1 )
{
	UINT8 *ROM = memory_region(Machine, "main");
	memcpy(ROM + 0x00000, ROM + 0x400000, 0x400000); /* default rom */
	memory_install_write16_handler(Machine,0, ADDRESS_SPACE_PROGRAM, 0xA13000, 0xA1303f, 0, 0, g_12i1_bank_w);

	DRIVER_INIT_CALL(megadriv);
}




READ16_HANDLER( dte_extra_r )
{
	//printf("extra r\n");
	return mame_rand(Machine);
}

int realtek_bank_addr=0;
int realtek_bank_size=0;
int realtek_old_bank_addr;


WRITE16_HANDLER( realtec_402000_w )
{
	realtek_bank_addr = 0;
	realtek_bank_size = (data>>8)&0x1f;
}

WRITE16_HANDLER( realtec_400000_w )
{
	int bankdata = (data >> 9) & 0x7;

	UINT8 *ROM = memory_region(Machine, "main");

	realtek_old_bank_addr = realtek_bank_addr;
	realtek_bank_addr = (realtek_bank_addr & 0x7) | bankdata<<3;

	//if (realtek_old_bank_addr!=realtek_bank_addr)
	{
		memcpy(ROM, ROM +  (realtek_bank_addr*0x20000)+0x400000, realtek_bank_size*0x20000);
		memcpy(ROM+ realtek_bank_size*0x20000, ROM +  (realtek_bank_addr*0x20000)+0x400000, realtek_bank_size*0x20000);
	}
}

WRITE16_HANDLER( realtec_404000_w )
{
	int bankdata = (data >> 8) & 0x3;
	UINT8 *ROM = memory_region(Machine, "main");

	realtek_old_bank_addr = realtek_bank_addr;
	realtek_bank_addr = (realtek_bank_addr & 0xf8)|bankdata;

	if (realtek_old_bank_addr != realtek_bank_addr)
	{
		memcpy(ROM, ROM +  (realtek_bank_addr*0x20000)+0x400000, realtek_bank_size*0x20000);
		memcpy(ROM+ realtek_bank_size*0x20000, ROM +  (realtek_bank_addr*0x20000)+0x400000, realtek_bank_size*0x20000);
	}
}


DRIVER_INIT( g_dte )
{
	UINT32 mirroraddr;

	/* Realtec mapper!*/
	UINT8 *ROM = memory_region(Machine, "main");
	for (mirroraddr = 0; mirroraddr < 0x400000;mirroraddr+=0x2000)
		memcpy(ROM+mirroraddr, ROM +  0x47e000, 0x002000); /* copy last 8kb across the whole rom region */

	realtek_old_bank_addr = -1;

	memory_install_write16_handler(Machine,0, ADDRESS_SPACE_PROGRAM, 0x400000, 0x400001, 0, 0, realtec_400000_w);
	memory_install_write16_handler(Machine,0, ADDRESS_SPACE_PROGRAM, 0x402000, 0x402001, 0, 0, realtec_402000_w);
	memory_install_write16_handler(Machine,0, ADDRESS_SPACE_PROGRAM, 0x404000, 0x404001, 0, 0, realtec_404000_w);

	DRIVER_INIT_CALL(megadriv);
}

UINT16 lion2_prot1_data, lion2_prot2_data;

READ16_HANDLER( lion2_prot1_r )
{
	return lion2_prot1_data;
}

READ16_HANDLER( lion2_prot2_r )
{
	return lion2_prot2_data;
}

WRITE16_HANDLER ( lion2_prot1_w )
{
	lion2_prot1_data = data;
}

WRITE16_HANDLER ( lion2_prot2_w )
{
	lion2_prot2_data = data;
}

DRIVER_INIT( g_lionk2 )
{
	memory_install_write16_handler(Machine,0,ADDRESS_SPACE_PROGRAM, 0x400000, 0x400001, 0, 0, lion2_prot1_w );
	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0x400002, 0x400003, 0, 0, lion2_prot1_r );
	memory_install_write16_handler(Machine,0,ADDRESS_SPACE_PROGRAM, 0x400004, 0x400005, 0, 0, lion2_prot2_w );
	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0x400006, 0x400007, 0, 0, lion2_prot2_r );

	DRIVER_INIT_CALL(megadriv);

}

READ16_HANDLER( elfwor_0x400000_r )
{
	return 0x5500;
}

READ16_HANDLER( elfwor_0x400002_r )
{
	return 0x0f00;
}

READ16_HANDLER( elfwor_0x400004_r )
{
	return 0xc900;
}

READ16_HANDLER( elfwor_0x400006_r )
{
	return 0x1800;
}

DRIVER_INIT( g_elfwor )
{
	/* is there more to this, i can't seem to get off the first level? */
	/*
	Elf Wor (Unl) - return (0×55@0×400000 OR 0xc9@0×400004) AND (0×0f@0×400002 OR 0×18@0×400006). It is probably best to add handlers for all 4 addresses.
	*/
	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0x400000, 0x400001, 0, 0, elfwor_0x400000_r );
	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0x400004, 0x400005, 0, 0, elfwor_0x400004_r );
	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0x400002, 0x400003, 0, 0, elfwor_0x400002_r );
	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0x400006, 0x400007, 0, 0, elfwor_0x400006_r );

	DRIVER_INIT_CALL(megadriv);
}


READ16_HANDLER( mjlovr_prot_1_r )
{
	return 0x9000;
}

READ16_HANDLER( mjlovr_prot_2_r )
{
	return 0xd300;
}

DRIVER_INIT( g_mjlovr )
{
	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0x400000, 0x400001, 0, 0, mjlovr_prot_1_r );
	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0x401000, 0x401001, 0, 0, mjlovr_prot_2_r );

	DRIVER_INIT_CALL(megadriv);
}

READ16_HANDLER( smbro_prot_r )
{
	return 0xc;
}

DRIVER_INIT( g_smbro )
{
	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0xa13000, 0xa13001, 0, 0, smbro_prot_r );

	DRIVER_INIT_CALL(megadriv);

}


DRIVER_INIT( g_pockm )
{
	UINT16 *ROM = (UINT16*)memory_region(Machine, "main");
	/*todo: emulate protection instead
	0DD19E:47F8
	0DD1A0:FFF0
	0DD1A2:4E63
	0DD46E:4EF8
	0DD470:0300
	0DD49C:6002
	*/

	/*

	you need to return 1 @ 0xa13002 and 0×1f @ 0xa1303E (it does word reads).

	*/

	ROM[0x0dd19e/2] = 0x47F8;
	ROM[0x0dd1a0/2] = 0xFFF0;
	ROM[0x0dd1a2/2] = 0x4E63;
	ROM[0x0dd46e/2] = 0x4EF8;
	ROM[0x0dd470/2] = 0x0300;
	ROM[0x0dd49c/2] = 0x6002;


	DRIVER_INIT_CALL(megadriv);

}

DRIVER_INIT( g_pockm2 )
{
	UINT16 *ROM = (UINT16*)memory_region(Machine, "main");
	/*todo: emulate protection instead
	006036:E000
	002540:6026
	001ED0:6026
	002476:6022

	*/
	ROM[0x06036/2] = 0xE000;
	ROM[0x02540/2] = 0x6026;
	ROM[0x01ED0/2] = 0x6026;
	ROM[0x02476/2] = 0x6022;

	ROM[0x7E300/2] = 0x60FE;


	DRIVER_INIT_CALL(megadriv);

}

DRIVER_INIT( g_mulan )
{
	UINT16 *ROM = (UINT16*)memory_region(Machine, "main");
	/*todo: emulate protection instead
	006036:E000
	+more?

	*/
//	ROM[0x01ED0/2] = 0xE000;
//	ROM[0x02540/2] = 0xE000;


	ROM[0x06036/2] = 0xE000;

	DRIVER_INIT_CALL(megadriv);
}


READ16_HANDLER( g_kof98_aa_r )
{
//	printf("kof98 aa00 r\n");
	return 0xaa00;
}

READ16_HANDLER( g_kof98_0a_r )
{
//	printf("kof98 0a r\n");
	return 0x0a00;
}

READ16_HANDLER( g_kof98_00_r )
{
//	printf("kof98 00 r\n");
	return 0x0000;
}

DRIVER_INIT( g_kof98 )
{
	/* you still get a black bar on the char select screen? */

//cpu #0 (PC=00052974): unmapped program memory word read from 004865A0 & FF00
//cpu #0 (PC=00056AEA): unmapped program memory word read from 004C82C0 & FF00

	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0x480000, 0x480001, 0, 0, g_kof98_aa_r );
	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0x4800e0, 0x4800e1, 0, 0, g_kof98_aa_r );
	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0x4824a0, 0x4824a1, 0, 0, g_kof98_aa_r );
	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0x488880, 0x488881, 0, 0, g_kof98_aa_r );

	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0x4a8820, 0x4a8821, 0, 0, g_kof98_0a_r );
	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0x4f8820, 0x4f8821, 0, 0, g_kof98_00_r );

	DRIVER_INIT_CALL(megadriv);
}


READ16_HANDLER( smous_prot_r )
{
	switch (offset)
	{
		case 0: return 0x5500;
		case 1: return 0x0f00;
		case 2: return 0xaa00;
		case 3: return 0xf000;
	}

	return -1;
}


DRIVER_INIT( g_smous )
{
	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0x400000, 0x400007, 0, 0, smous_prot_r );
	DRIVER_INIT_CALL(megadriv);
}


READ16_HANDLER( kof99_0xA13002_r )
{
	// write 02 to a13002.. shift right 1?
	return 0x01;
}

READ16_HANDLER( kof99_00A1303E_r )
{
	// write 3e to a1303e.. shift right 1?
	return 0x1f;
}

READ16_HANDLER( kof99_0xA13000_r )
{
	// no write, startup check, chinese message if != 0
	return 0x0;
}

DRIVER_INIT( g_kof99 )
{
	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0xA13000, 0xA13001, 0, 0, kof99_0xA13000_r );
	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0xA13002, 0xA13003, 0, 0, kof99_0xA13002_r );
	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0xA1303e, 0xA1303f, 0, 0, kof99_00A1303E_r );


	DRIVER_INIT_CALL(megadriv);

}

READ16_HANDLER( soulb_0x400006_r )
{
	printf("%06x soulb_0x400006_r\n",activecpu_get_pc());

	return 0xf000;
}

READ16_HANDLER( soulb_0x400002_r )
{
	printf("%06x soulb_0x400002_r\n",activecpu_get_pc());
	return 0x9800;
}

READ16_HANDLER( soulb_0x400004_r )
{
//	return 0x9800;
	printf("%06x soulb_0x400004_r\n",activecpu_get_pc());
//
	return 0xc900;
//aa
//c9

}


DRIVER_INIT( g_soulb )
{
	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0x400006, 0x400007, 0, 0, soulb_0x400006_r );
	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0x400002, 0x400003, 0, 0, soulb_0x400002_r );
	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0x400004, 0x400005, 0, 0, soulb_0x400004_r );


	DRIVER_INIT_CALL(megadriv);

}


ROM_START( g_bible )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bible.bin", 0x000000, 0x080000,  CRC(64446b77) SHA1(0163b6cd6397ab4c1016f9fcf4f2e6d2bca8454f) )
ROM_END
ROM_START( g_joshua )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_joshua.bin", 0x000000, 0x040000,  CRC(da9e25aa) SHA1(a6c47babc7d84f8f411e77b9acdf01753d3a5951) )
ROM_END
ROM_START( g_exodus )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_exodus.bin", 0x000000, 0x080000,  CRC(22e6fc04) SHA1(a85fb15d29dc43d3bf4a06de83506c77aba8a7d5) )
ROM_END
ROM_START( g_spirit )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_spirit.bin", 0x000000, 0x080000,  CRC(d9a364ff) SHA1(efd7f8c1d7daf7a0b6cac974de093f224f6e1c32) )
ROM_END
ROM_START( g_divine )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_divine.bin", 0x000000, 0x100000,  CRC(ca72973c) SHA1(29cc95622a1c9602e7981bdc5a66164c47939028) )
ROM_END

ROM_START( g_act52 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_act52.bin", 0x000000, 0x200000, CRC(29ff58ae) SHA1(1d5b26a5598eea268d15fa16d43816f8c3e4f8c6) )
ROM_END
ROM_START( g_act52a )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_act52a.bin", 0x000000, 0x200000, CRC(8809d666) SHA1(fe9936517f45bd0262613ce4422ace873112210a) )
ROM_END
ROM_START( g_chaoji )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_chaoji.bin", 0x000000, 0x100000, CRC(2e2ea687) SHA1(be1a66bd1f75f5b1b37b8ae18e6334c36291e63d) )
ROM_END
ROM_START( g_chess )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_chess.bin", 0x000000, 0x080000,  CRC(47380edd) SHA1(42e56fc5543dcb40da73447b582c84c4ff50a825) )
ROM_END
ROM_START( g_maggrl )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_maggrl.bin", 0x000000, 0x080000,  CRC(8e49a92e) SHA1(22a48404e77cab473ee65e5d5167d17c28884a7a) )
ROM_END
ROM_START( g_mjlovr )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mjlovr.bin", 0x000000, 0x100000, CRC(ddd02ba4) SHA1(fe9ec21bd206ad1a178c54a2fee80b553c478fc4) )
ROM_END
ROM_START( g_sj6 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sj6.bin", 0x000000, 0x200000,  CRC(bf39d897) SHA1(3a6fe5a92dc2ada7e9ab17ac120c7e50d1f9a1ed) )
ROM_END
ROM_START( g_sj6p )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sj6p.bin", 0x000000, 0x200000,  CRC(04f0c93e) SHA1(a9e316ccde5b71f6aa85485b6897c1cfc780742d) )
ROM_END
ROM_START( g_smbro )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_smbro.bin", 0x000000, 0x200000, CRC(9cfa2bd8) SHA1(5011e16f0ab3a6487a1406b85c6090ad2d1fe345) )
ROM_END
ROM_START( g_alad2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_alad2.bin", 0x000000, 0x200000,  CRC(be5f9397) SHA1(9980997458dff7897009301a873cf84441f8a01f) )
ROM_END
ROM_START( g_barver )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_barver.bin", 0x000000, 0x200000,  CRC(d37a37c6) SHA1(6d49a0db7687ccad3441f47fbc483c87cd6eab53) )
ROM_END
ROM_START( g_rtk5c )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rtk5c.bin", 0x000000, 0x200000,  CRC(cd7e53d0) SHA1(1a3333983f40dd242ad86187e50d1abed68d5ae9))
ROM_END

ROM_START( g_tighun )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tighun.bin", 0x000000, 0x200000,  CRC(61e458c3) SHA1(f58c522bd0629833d3943ef4091c3c349c134879) )
ROM_END

ROM_START( g_topfig )
	ROM_REGION( 0x1400000, "main", 0 ) /* 68000 Code */
	/* Special Banking */
	ROM_LOAD( "g_topfig.bin", 0x400000, 0x200000, CRC(f75519dc) SHA1(617be8de1444ae0c6610d73967f3f0e67541b05a) )
ROM_END




ROM_START( g_pockm )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pockm.bin", 0x000000, 0x200000,  CRC(f68f6367) SHA1(d10de935e099c520384c986b1f00fd5e72d64e03) )
ROM_END

ROM_START( g_pockma )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pockma.bin", 0x000000, 0x200000,  CRC(fb176667) SHA1(3d667aea1b6fa980dddcc10e65845a6831491792) )
ROM_END

ROM_START( g_pockm2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pockm2.bin", 0x000000, 0x200000,  CRC(30f7031f) SHA1(dae100dfaee1b5b7816731cb2f43bcda3da273b7) )
ROM_END

ROM_START( g_mulan )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mulan.bin", 0x000000, 0x200000, CRC(796882b8) SHA1(d8936c1023db646e1e20f9208b68271afbd6dbf4) )
ROM_END



ROM_START( g_pcdrum )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pcdrum.bin", 0x000000, 0x200000,   CRC(8838a25d) SHA1(780a81fe6dd2fb9575ccdc506e7dbee13213d01d) )
ROM_END
ROM_START( g_tek3s )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tek3s.bin", 0x000000, 0x200000,  CRC(7fcae658) SHA1(99349bfe7966d65a4e782615aad9aa688905ad41) )
ROM_END
ROM_START( g_kof98p )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_kof98p.bin", 0x000000, 0x200000,  CRC(c79e1074) SHA1(6eb3a12e082ce4074e88ad3cb2b3c51f9a72225c) )
ROM_END
ROM_START( g_lion2p )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_lion2p.bin", 0x000000, 0x200000,  CRC(721b4981) SHA1(70eb5b423948e5a124de4d5d24c14b2c64bfb282) )
ROM_END
ROM_START( g_sdk99 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sdk99.bin", 0x000000, 0x200000,  CRC(8e7d9177) SHA1(91f6b10ada917e6dfdafdd5ad9d476723498a7a4) )
ROM_END

ROM_START( g_skk99 )
	ROM_REGION( 0x1400000, "main", 0 ) /* 68000 Code */
	/* Special Banking */
	ROM_LOAD( "g_skk99.bin", 0x400000, 0x200000,  CRC(413dfee2) SHA1(6973598d77a755beafff31ce85595f9610f8afa5) )
ROM_END

ROM_START( g_smw )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_smw.bin", 0x000000, 0x200000,  CRC(cf540ba6) SHA1(517c3a6b091c2c4e8505112a84bae2871243e92c) )
ROM_END
ROM_START( g_smwp )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_smwp.bin", 0x000000, 0x200000,  CRC(97c2695e) SHA1(ff6661d39b2bad26f460e16106ca369421388596) )
ROM_END
ROM_START( g_mk5p )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mk5p.bin", 0x000000, 0x200000,  CRC(41203006) SHA1(a558ad8de61c4d21c35d4dbaaede85d771e84f33) )
ROM_END
ROM_START( g_3in1a )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */ // no menu??
	ROM_LOAD( "g_3in1a.bin", 0x000000, 0x200000,  CRC(a8fd28d7) SHA1(a0dd99783667af20589d473a2054d4bbd62d943e) )
ROM_END
ROM_START( g_3in1b )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */ // no menu??
	ROM_LOAD( "g_3in1b.bin", 0x000000, 0x100000,  CRC(13c96154) SHA1(fd7255c2abdf90698f779a039ea1e560ca19639a) )
ROM_END
ROM_START( g_cches )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_cches.bin", 0x000000, 0x080000,  CRC(475215a0) SHA1(3907bf058493e7b9db9720493030f0284797908c) )
ROM_END
ROM_START( g_conqs )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_conqs.bin", 0x000000, 0x200000,  CRC(ea57b668) SHA1(7040e96c053f29c75cf0524ddb168a83d0fb526f) )
ROM_END
ROM_START( g_conqt )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_conqt.bin", 0x000000, 0x200000,  CRC(b23c4166) SHA1(534bf8f951ee30d47df18202246245b998c0eced) )
ROM_END


ROM_START( g_mk5sz )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mk5sz.bin", 0x000000, 0x200000,  CRC(11e367a1) SHA1(8f92ce78be753748daeae6e16e1eed785f99d287) )
ROM_END
ROM_START( g_kof98 )
// protection
//cpu #0 (PC=0000040A): unmapped program memory word read from 00488880 & FF00
//cpu #0 (PC=00000492): unmapped program memory word read from 00480000 & FF00
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_kof98.bin", 0x000000, 0x200000,  CRC(cbc38eea) SHA1(aeee33bfc2c440b6b861ac0d1b9bc9bface24861) )
ROM_END

ROM_START( g_lionk2 ) // protected
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_lionk2.bin", 0x000000, 0x200000,  CRC(aff46765) SHA1(5649fa1fbfb28d58b0608e8ebc5dc7bd5c4c9678) )
ROM_END

ROM_START( g_soulb ) // protected
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_soulb.bin", 0x000000, 0x400000,  CRC(f26f88d1) SHA1(7714e01819ab4a0f424d7e306e9f891031d053a8) )
ROM_END
ROM_START( g_xinqi )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_xinqi.bin", 0x000000, 0x400000,   CRC(dd2f38b5) SHA1(4a7494d8601149f43ba7e3595a0b2340cde2e9ba) )
ROM_END
ROM_START( g_xinqia )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_xinqia.bin", 0x000000, 0x400000,  CRC(da5a4bfe) SHA1(75f8003a6388814c1880347882b244549da62158) )
ROM_END
ROM_START( g_yang )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_yang.bin", 0x000000, 0x200000,  CRC(6604a79e) SHA1(6fcc3102fc22b42049e6eae9a1c30c8a7f022d14) )
ROM_END

ROM_START( g_yasec )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_yasec.bin", 0x000000, 0x200000,  CRC(095b9a15) SHA1(8fe0806427e123717ba20478ab1410c25fa942e6) )
ROM_END

ROM_START( g_sbub )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sbub.bin", 0x000000, 0x100000,  CRC(4820a161) SHA1(03f40c14624f1522d6e3105997d14e8eaba12257) )
ROM_END

ROM_START( g_smb2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_smb2.bin", 0x000000, 0x200000,  CRC(f7e1b3e1) SHA1(de10115ce6a7eb416de9cd167df9cf24e34687b1) )
ROM_END

ROM_START( g_bugl )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bugl.bin", 0x000000, 0x100000,  CRC(10458e09) SHA1(b620c2bebd5bab39bc9258a925169b4c93614599) )
ROM_END

ROM_START( g_dte )
	ROM_REGION( 0x1400000, "main", ROMREGION_ERASE00 ) /* 68000 Code */
	/* Has custom banking */
	ROM_LOAD( "g_dte.bin", 0x400000, 0x080000,  CRC(3519c422) SHA1(9bf4cda850495d7811df578592289018862df575) )
ROM_END

ROM_START( g_whac )
	ROM_REGION( 0x1400000, "main", ROMREGION_ERASE00 ) /* 68000 Code */
	/* Has custom banking */
	ROM_LOAD( "g_whac.bin", 0x400000, 0x080000,  CRC(1bdd02b8) SHA1(4b45801b112a641fee936e41a31728ee7aa2f834) )
ROM_END

ROM_START( g_fwbb )
	ROM_REGION( 0x1400000, "main", ROMREGION_ERASE00 ) /* 68000 Code */
	/* Has custom banking */
	ROM_LOAD( "g_fwbb.bin", 0x400000, 0x080000,  CRC(a82f05f0) SHA1(17481c8327433bfce8f7bae493fc044194e400a4) )
ROM_END


ROM_START( g_rx3 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rx3.bin", 0x000000, 0x200000,  CRC(3ee639f0) SHA1(37024d0088fab1d76c082014663c58921cdf33df) )
ROM_END

ROM_START( g_lio3 )
	ROM_REGION( 0x1400000, "main", 0 ) /* 68000 Code */
	/* Special Banking */
	ROM_LOAD( "g_lio3.bin", 0x400000, 0x200000,  CRC(c004219d) SHA1(54ffd355b0805741f58329fa38ed3d9f8f7c80ca) )
ROM_END

ROM_START( g_squi )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_squi.bin", 0x000000, 0x100000,  CRC(b8261ff5) SHA1(2a561b6e47c93272fe5947084837d9f6f514ed38) )
ROM_END

ROM_START( g_taiwan )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_taiwan.bin", 0x000000, 0x100000, CRC(baf20f81) SHA1(88726c11e5ed7927830bf5ae0b83d85dfff4a2a4) )
ROM_END

ROM_START( g_vf2t2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_vf2t2.bin", 0x000000, 0x200000, CRC(2cdb499d) SHA1(0a5be6d37db5579b9de991b71442a960afcfe902) )
ROM_END


ROM_START( g_12i1 )
	ROM_REGION( 0x1400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_12i1.bin", 0x400000, 0x200000, BAD_DUMP CRC(a98bf454) SHA1(7313c20071de0ab1cd84ac1352cb0ed1c4a4afa8) ) // incomplete
ROM_END

ROM_START( g_4in1 )
	ROM_REGION( 0x1400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_4in1.bin", 0x400000, 0x200000, CRC(be72857b) SHA1(2aceca16f13d6a7a6a1bff8543d31bded179df3b) )
ROM_END

ROM_START( g_19in1 )
	ROM_REGION( 0x1400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_19in1.bin", 0x400000, 0x400000, CRC(0ad2b342) SHA1(e1c041ba69da087c428dcda16850159f3caebd4b) )
ROM_END

ROM_START( g_15in1 )
	ROM_REGION( 0x1400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_15in1.bin", 0x400000, 0x200000, CRC(6d17dfff) SHA1(6b2a6de2622735f6d56c6c9c01f74daa90e355cb) )
ROM_END


ROM_START( g_10in1 )
	ROM_REGION( 0x1400000, "main", 0 ) /* 68000 Code */
	/* this should be 0x400000 in size! */
	ROM_LOAD( "g_10in1.bin", 0x400000, 0x100000, BAD_DUMP CRC(4fa3f82b) SHA1(04edbd35fe4916f61b516016b492352d96a8de7f) )
	/* these should be part of the rom, but are missing
	  -- which versions of these games is unknown, probably pirate ones with SEGA logos and strings removed-- */
	ROM_LOAD( "g_cill.bin", 0x000000, 0x080000, CRC(ba4e9fd0) SHA1(4ac3687634a5acc55ac7f156c6de9749158713e4) ) // doesn't boot until you reset, probably wrong verison
	ROM_LOAD( "g_flin.bin", 0x580000, 0x080000, CRC(7c982c59) SHA1(5541579ffaee1570da8bdd6b2c20da2e395065b0) )
	ROM_LOAD( "g_sor.bin",  0x600000, 0x080000, CRC(4052e845) SHA1(731cdf182fe647e4977477ba4dd2e2b46b9b878a) )
	ROM_LOAD( "g_chq2.bin", 0x680000, 0x080000, CRC(f39e4bf2) SHA1(47c8c173980749aca075b9b3278c0df89a21303f) )
	ROM_LOAD( "g_turt.bin", 0x700000, 0x100000, CRC(679c41de) SHA1(f440dfa689f65e782a150c1686ab90d7e5cc6355) ) // or the other turtles game?
 ROM_END


ROM_START( g_16tile )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_16tile.bin", 0x000000, 0x100000, CRC(36407c82) SHA1(7857c797245b52641a3d1d4512089bccb0ed5359) )
ROM_END
ROM_START( g_777cas )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_777cas.bin", 0x000000, 0x100000, CRC(42dc03e4) SHA1(df20a28d03a2cd481af134ef7602062636c3cc79) )
ROM_END

ROM_START( g_dmahtw )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dmahtw.bin", 0x000000, 0x100000, CRC(12e35994) SHA1(84e8bf546283c73396e40c4cfa05986ebeb123bb) )
ROM_END
ROM_START( g_dialq )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dialq.bin", 0x000000, 0x100000,  CRC(c632e5af) SHA1(08967e04d992264f193ecdfd0e0457baaf25f4f2) )
ROM_END
ROM_START( g_domino )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_domino.bin", 0x000000, 0x100000,  CRC(a64409be) SHA1(7000ea86d91bbb5642425b6a6f577fab9e2b3a51) )
ROM_END

ROM_START( g_elfwor )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_elfwor.bin", 0x000000, 0x100000, CRC(e24ac6b2) SHA1(5fc4591fbb1acc64e184466c7b6287c7f64e0b7a) )
ROM_END

ROM_START( g_ghunt )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ghunt.bin", 0x000000, 0x080000, CRC(76c62a8b) SHA1(3424892e913c20754d2e340c6e79476a9eb6761b) )
ROM_END

ROM_START( g_herc2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_herc2.bin", 0x000000, 0x200000, CRC(292623db) SHA1(7104a37f588f291b85eb8f62685cb1111373572c) )
ROM_END

ROM_START( g_iraq03 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_iraq03.bin", 0x000000, 0x100000, CRC(49dd6f52) SHA1(cc8b69debd68ba7c6d72d47d4c33530a1e7ef07c) )
ROM_END


ROM_START( g_linkdr )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_linkdr.bin", 0x000000, 0x040000, CRC(1b86e623) SHA1(09e4b59da3344f16ce6173c432c88ee9a12a3561) ) // sound?
ROM_END

ROM_START( g_smous )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_smous.bin", 0x000000, 0x080000, CRC(decdf740) SHA1(df7a2527875317406b466175f0614d343dd32117) )
ROM_END

ROM_START( g_fengsh )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fengsh.bin", 0x000000, 0x200000,  CRC(6a382b60) SHA1(7a6e06846a94df2df2417d6509e398c29354dc68) )
ROM_END

ROM_START( g_unkch )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_unkch.bin", 0x000000, 0x200000, CRC(dfacb9ff) SHA1(4283bb9aec05098b9f6b1739e1b02c1bb1f8242f) )
ROM_END

ROM_START( g_pgmah )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pgmah.bin", 0x000000, 0x100000, CRC(69f24500) SHA1(0b4f63b8de2dcc4359a26c3ff21f910f206b6110) )
ROM_END


ROM_START( g_chifi3 )
	ROM_REGION( 0x1400000, "main", 0 ) /* 68000 Code */
	/* Special Banking */
	ROM_LOAD( "g_chifi3.bin", 0x400000, 0x200000,  CRC(e833bc6e) SHA1(ecca9d2d21c8e27fc7584d53f557fdd8b4cbffa7) )
ROM_END

ROM_START( g_cf3p )
	ROM_REGION( 0x1400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_cf3p.bin", 0x000000, 0x200000,  CRC(6f98247d) SHA1(cc212b1564dc7c73ffdc55f9fde3269a83fee399) )
ROM_END

ROM_START( g_kaiju )
	ROM_REGION( 0x1400000, "main", 0 ) /* 68000 Code */
	/* Special Banking */
	ROM_LOAD( "g_kaiju.bin", 0x400000, 0x200000,  CRC(fe187c5d) SHA1(f65af5d86aba33bd3f4f91a0cd7428778bcceedf) )
ROM_END

ROM_START( g_kof99 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_kof99.bin", 0x000000, 0x300000,  CRC(54638c11) SHA1(cdef3008dec2ce1a214af8b9cb000053671a3c36) )
ROM_END


ROM_START( g_sdk993 )
	ROM_REGION( 0x1400000, "main", 0 ) /* 68000 Code */
	/* Special Banking */
	ROM_LOAD( "g_sdk993.bin", 0x400000, 0x300000,  CRC(43be4dd5) SHA1(5d3c84bd18f821b20212941a6f7a1a272eb0d7e3) )
ROM_END


ROM_START( g_redclf )
	ROM_REGION( 0x1400000, "main", 0 ) /* 68000 Code */
	/* what is an 'mdx' file?? */
	ROM_LOAD( "g_redclf.mdx", 0x400000, 0x200005,  CRC(44463492) SHA1(244334583fde808a56059c0b0eef77742c18274d) )
ROM_END

READ16_HANDLER( redclif_prot_r )
{
	return -0x56 << 8;
}

READ16_HANDLER( redclif_prot2_r )
{
	return 0x55 << 8;
}

DRIVER_INIT( g_redclf )
{
	UINT8 *ROM = memory_region(Machine, "main");
	int x;

	for (x=0x400000;x<0x400000+0x200005;x++)
	{
		ROM[x] ^= 0x40;
	}
	memcpy(ROM + 0x00000, ROM + 0x400004, 0x200000); /* default rom */

	DRIVER_INIT_CALL(megadriv);

	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0x400000, 0x400001, 0, 0, redclif_prot2_r );
	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0x400004, 0x400005, 0, 0, redclif_prot_r );

}


GAME( 1995, g_redclf,  0,        megadriv,    megadriv,    g_redclf, ROT0,   "Unknown (Unlicensed)", "Romance of the Three Kingdoms, The Battle of Red Cliffs (Unl)", 0 )

GAME( 1995, g_bible,  0,        megadriv,    megadriv,    megadriv, ROT0,   "Wisdom Tree", "Bible Adventures (Unl) [!]", 0 )
GAME( 1994, g_joshua, 0,        megadriv,    megadriv,    megadriv, ROT0,   "Wisdom Tree", "Joshua & the Battle of Jericho (Unl) [!]", 0 )
GAME( 1993, g_exodus, 0,        megadriv,    megadriv,    megadriv, ROT0,   "Wisdom Tree", "Exodus (Unl) [!]", 0 )
GAME( 1994, g_spirit, 0,        megadriv,    megadriv,    megadriv, ROT0,   "Wisdom Tree", "Spiritual Warfare (Unl) [!]", 0 )


GAME( 199?, g_act52,  0,        megadriv,    megadriv,    megadriv, ROT0,   "unknown", "Action 52 (Unl) [!]", 0 )
GAME( 199?, g_act52a, g_act52,  megadriv,    megadriv,    megadriv, ROT0,   "unknown", "Action 52 (Unl) [a1][!]", 0 )

GAME( 199?, g_sbub,   0,        megadriv,    megadriv,    g_sbub,   ROT0,   "Sun Mixing", "Super Bubble Bobble (Unl) [!]", 0 )

// Realtec's games have custom banking
GAME( 199?, g_dte,    0,        megadriv,    megadriv,    g_dte,    ROT0,   "Realtec", "Earth Defend, The (Unl) [!]", 0 )
GAME( 1993, g_whac,   0,        megadriv,    megadriv,    g_dte,    ROT0,   "Realtec", "Whac-A-Critter / Mallet Legend (Unl) [!]", 0 )
GAME( 1993, g_fwbb,   0,        megadriv,    megadriv,    g_dte,    ROT0,   "Realtec", "Funny World & Balloon Boy (Unl) [!]", 0 )

// Graphic Corruption on player select screen (more protection?)
GAME( 199?, g_squi,   0,        megadriv,    megadriv,    g_squi,   ROT0,   "unknown", "Squirrel King (R) [!]", 0 )

// Protected with a simple startup check
GAME( 1998, g_smb2,   0,        megadriv,    megadriv,    g_smb2,   ROT0,   "unknown", "Super Mario 2 1998 (Unl) [!]", 0 )

GAME( 1994, g_bugl,   0,        megadriv,    megadriv,    g_bugl,   ROT0,   "unknown", "Bug's Life, A (Unl) [!]", 0 )

GAME( 1994, g_rx3,    0,        megadriv,    megadriv,    g_rx3,    ROT0,   "unknown", "Rockman X3 (Unl) [!]", 0 )

GAME( 1994, g_lio3,   0,        megadriv,    megadriv,    g_l3alt,  ROT0,   "unknown", "Lion King 3 (Unl)", 0 )

GAME( 199?, g_divine, 0,        megadriv,    megadriv,    megadriv, ROT0,   "Unknown", "Divine Sealing (Unl) [!]", 0 )

GAME( 1900, g_chaoji,        0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Chaoji Dafuweng (Unl) [!]", GAME_NOT_WORKING )
GAME( 1900, g_chess,         0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Chess (Unl) [!]", 0 )
GAME( 1900, g_maggrl,        0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Magic Girl (Unl) [!]", 0 )
GAME( 1900, g_mjlovr,        0,        megadriv,    megadriv,    g_mjlovr, ROT0,   "Unsorted", "Mahjong Lover (Unl) [!]", 0 )
GAME( 1900, g_sj6,           0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Sonic Jam 6 (Unl) [!]", GAME_NOT_WORKING )
GAME( 1900, g_sj6p,          0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Sonic Jam 6 (Unl) [p1][!]", GAME_NOT_WORKING )
GAME( 1900, g_alad2,         0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Aladdin II (Unl)", 0 )
GAME( 1900, g_barver,        0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Barver Battle Saga - The Space Fighter (Ch)", 0 )
GAME( 1900, g_rtk5c,         0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Romance of the Three Kingdoms Part 5 (Ch)", 0 )
GAME( 1900, g_tighun,        0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Tiger Hunter Hero Novel (Ch)", 0 )


GAME( 1900, g_taiwan,        0,        megadriv,    megadriv,    megadriv, ROT0,   "Unknown (Unlicensed)", "Taiwan Tycoon (Unl)", 0 )

GAME( 1900, g_vf2t2,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unknown (Unlicensed)", "Virtua Fighter 2 vs Tekken 2 (Unl)", 0 )

GAME( 1900, g_topfig,  0,        megadriv,    megadriv,    g_topfig, ROT0,   "Unknown (Unlicensed)", "Top Fighter 2000 MK VIII (Unl) [!]", GAME_NOT_WORKING ) // protection

GAME( 1900, g_mk5sz,  0,        megadriv,    megadriv,    megadriv, ROT0,   "Unknown (Unlicensed)", "MK 5 - Mortal Combat - SubZero (Unl) [!]", 0 )
GAME( 1900, g_mk5p,   g_mk5sz,  megadriv,    megadriv,    megadriv, ROT0,   "Unknown (Unlicensed)", "MK 5 - Mortal Combat - SubZero (Unl) [p1][!]", 0 )


GAME( 1900, g_kof98,   0,        megadriv,    megadriv,    g_kof98,  ROT0,   "Unknown (Unlicensed)", "King of Fighters '98, The (Unl) [!]", GAME_NOT_WORKING ) // protection
GAME( 1900, g_kof98p,  g_kof98,  megadriv,    megadriv,    megadriv, ROT0,   "Unknown (Unlicensed)", "King of Fighters '98, The (Unl) [p1][!]", 0 )

GAME( 1900, g_lionk2,  0,        megadriv,    megadriv,    g_lionk2, ROT0,   "Unknown (Unlicensed)", "Lion King II, The (Unl) [!]", 0 )
GAME( 1900, g_lion2p,  g_lionk2, megadriv,    megadriv,    megadriv, ROT0,   "Unknown (Unlicensed)", "Lion King II, The (Unl) [p1][!]", 0 )

GAME( 1900, g_soulb,   0,        megadriv,    megadriv,    g_soulb, ROT0,   "Unknown (Unlicensed)", "Soul Blade (Unl) [!]", 0 ) // protection


GAME( 1900, g_xinqi,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unknown (Unlicensed)", "Xin Qi Gai Wang Zi (Ch)", 0 )
GAME( 1900, g_xinqia,  g_xinqi,  megadriv,    megadriv,    megadriv, ROT0,   "Unknown (Unlicensed)", "Xin Qi Gai Wang Zi (Ch) [a1]", 0 )

GAME( 1900, g_yang,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unknown (Unlicensed)", "Yang Warrior Family, The (Ch)", 0 )
GAME( 1900, g_yasec,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unknown (Unlicensed)", "Ya-Se Chuan Shuo (Ch)", GAME_NOT_WORKING ) // messy..

GAME( 1900, g_pockm,   0,        megadriv,    megadriv,    g_pockm,  ROT0,   "Unknown (Unlicensed)", "Pocket Monsters (Unl) [!]", 0 )
GAME( 1900, g_pockma,  g_pockm,  megadriv,    megadriv,    megadriv, ROT0,   "Unknown (Unlicensed)", "Pocket Monsters (Unl) [a1][!]", 0 )

GAME( 1900, g_pockm2,  0,        megadriv,    megadriv,    g_pockm2, ROT0,   "Unknown (Unlicensed)", "Pocket Monsters 2 (Unl) [!]", 0 )

GAME( 1900, g_mulan,   0,        megadriv,    megadriv,    g_mulan,  ROT0,   "Unknown (Unlicensed)", "Mulan (Unl) [!]", 0 )

GAME( 1900, g_pcdrum,  0,        megadriv,    megadriv,    megadriv,  ROT0,   "Unknown (Unlicensed)", "Pokemon Crazy Drummer (Unl)", 0 )

GAME( 1900, g_tek3s,   0,        megadriv,    megadriv,    megadriv,  ROT0,   "Unknown (Unlicensed)", "Tekken 3 Special (Unl)", 0 )

GAME( 1900, g_sdk99,   0,        megadriv,    megadriv,    megadriv,  ROT0,   "Unknown (Unlicensed)", "Super Donkey Kong 99 (Unl) [!]", 0 )
GAME( 1900, g_skk99,   g_sdk99,  megadriv,    megadriv,    g_l3alt,   ROT0,   "Unknown (Unlicensed)", "Super King Kong 99 (Unl) [!]", 0 ) // protected version
GAME( 1900, g_sdk993,  g_sdk99,  megadriv,    megadriv,    g_l3alt,  ROT0,    "Unknown (Unlicensed)", "Super Donkey Kong 99 (Protected)", 0 )

GAME( 1900, g_smw,     0,        megadriv,    megadriv,    megadriv,  ROT0,   "Unknown (Unlicensed)", "Super Mario World (Unl) [!]", 0 )
GAME( 1900, g_smwp,    g_smw,    megadriv,    megadriv,    megadriv,  ROT0,   "Unknown (Unlicensed)", "Super Mario World (Unl) [p1][!]", 0 )
GAME( 1900, g_smbro,   g_smw,    megadriv,    megadriv,    g_smbro,   ROT0,   "Unknown (Unlicensed)", "Super Mario Bros. (Unl) [!]", 0 ) // protected version



GAME( 1900, g_3in1a,  0,        megadriv,    megadriv,    megadriv,  ROT0,   "Unknown (Unlicensed)", "3-in-1 Flashback - World Champ. Soccer - Tecmo World Cup 92 [p1][!]", GAME_NOT_WORKING ) // no menu, hardwired?
GAME( 1900, g_3in1b,  0,        megadriv,    megadriv,    megadriv,  ROT0,   "Unknown (Unlicensed)", "3-in-1 Road Rash - Ms. Pac-Man - Block Out [p1][!]", GAME_NOT_WORKING ) // no menu, hardwired?

GAME( 1900, g_cches,  0,        megadriv,    megadriv,    megadriv,  ROT0,   "Unknown (Unlicensed)", "Chinese Chess (Unl)", 0 )

GAME( 1900, g_conqs,  0,        megadriv,    megadriv,    megadriv,  ROT0,   "Unknown (Unlicensed)", "Conquering the World III (Ch-Simple)", 0 )
GAME( 1900, g_conqt,  g_conqs,  megadriv,    megadriv,    megadriv,  ROT0,   "Unknown (Unlicensed)", "Conquering the World III (Ch-Trad)", 0 )

GAME( 1900, g_12i1,      0,        megadriv,    megadriv,    g_12i1, ROT0,   "Unsorted", "12-in-1 (Unl)", 0 )
GAME( 1900, g_4in1,      0,        megadriv,    megadriv,    g_12i1, ROT0,   "Unsorted", "4-in-1 [p1][!]", 0 )
GAME( 1900, g_10in1,     0,        megadriv,    megadriv,    g_12i1, ROT0,   "Unsorted", "Golden 10-in-1 (bad)", 0 )
GAME( 1900, g_19in1,     0,        megadriv,    megadriv,    g_12i1, ROT0,   "Unsorted", "Super 19-in-1 [p1][!]", GAME_NOT_WORKING ) // check banking, maybe it just doesn't have any of the titles it lists..
GAME( 1900, g_15in1,     0,        megadriv,    megadriv,    g_12i1, ROT0,   "Unsorted", "Super 15-in-1 [p1][!]", 0 ) // looks like this one just has lots of bogus titles

GAME( 1900, g_16tile,        0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "16 Tiles Mahjong (Unl)", 0 )
GAME( 1900, g_777cas,        0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "777 Casino (Unl)", 0 )

GAME( 1900, g_dmahtw,        0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Devilish Mahjong Tower (Unl)", 0 )
GAME( 1900, g_dialq,         0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Dial Q o Mawase! (Unl)", 0 )
GAME( 1900, g_domino,        0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Domino (Unl)", 0 )
GAME( 1900, g_elfwor,        0,        megadriv,    megadriv,    g_elfwor, ROT0,   "Unsorted", "Elf Wor (Unl)", 0 )
GAME( 1900, g_ghunt,         0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Ghost Hunter (Unl)", 0 )
GAME( 1900, g_herc2,         0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Hercules 2 (Unl)", 0 )
GAME( 1900, g_iraq03,        0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Iraq War 2003 (Unl)", 0 )
GAME( 1900, g_linkdr,        0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Link Dragon (Unl)", 0 )
GAME( 1900, g_smous,         0,        megadriv,    megadriv,    g_smous,  ROT0,   "Unsorted", "Smart Mouse (Unl)", 0 )
GAME( 1900, g_fengsh,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Fengshen Yingjiechuan (Ch)", 0 )
GAME( 1900, g_unkch,    g_fengsh, megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Unknown Chinese Game 1 (Ch)", 0 )
GAME( 1900, g_pgmah,         0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Pretty Girl Mahjongg (Ch)", 0 )

GAME( 1900, g_chifi3,  0,  megadriv,    megadriv,    g_chifi3,  ROT0,   "Unknown (Unlicensed)", "Chinese Fighter III (Unl)", GAME_UNEMULATED_PROTECTION ) // almost ok.. maybe still some problems
GAME( 1900, g_cf3p,    0,  megadriv,    megadriv,    megadriv,  ROT0,   "Unknown (Unlicensed)", "Chinese Fighter III (Unl) (bootleg version)", 0 )

GAME( 1900, g_kaiju,   0,  megadriv,    megadriv,    g_kaiju,  ROT0,   "Unknown (Unlicensed)", "Pokemon Stadium (Unl)", 0 )
GAME( 1900, g_kof99,   0,  megadriv,    megadriv,    g_kof99,  ROT0,   "Unknown (Unlicensed)", "The King of Fighters 99 (Unl)", 0 )
