/* ST-V protection stuff */

/* todo:
 figure out properly
 clean up
 fix remaining games
 split them on a per-game basis

 Known Protected ST-V Games

 Astra Superstars (text layer gfx transfer)
 Elandoree (gfx transfer of textures)
 Final Fight Revenge (boot vectors etc.?)
 Radiant Silvergun (game start protection ?)
 Steep Slope Sliders (gfx transfer of character portraits)
 Decathlete (transfer of all gfx data)
 Tecmo World Cup '98 (tecmo logo, player movement?)


 Is this just scrambled, or also compressed somehow?  I've not had much luck
 locating the data some of the games want (for example decathlete)

 RSGun doesn't appear to make use of the data transfer features..

 The protection addresses are in the A-Bus area, this should map to the cartridge slot,
 is there something special in these game cartridges?

 Astra Superstars data were extracted from Saturn version of the game. It is not known if
 protection device has data stored inside, or they are read from roms (using decryption/decompression)

*/

/****************************************************************************************

Protection & cartridge handling

*****************************************************************************************

These are the known ST-V games that uses this area as a valid protection,I have written
the data used by the games in the various circumstances for reference:
-Astra Super Stars [astrass]
 [0]        [1]        [2]        [3]
 0x000y0000 0x00000000 0x06130027 0x01230000 test mode,char transfer (3)
 0x???????? 0x???????? 0x???????? 0x???????? attract mode
 0x000y0000 0x00000000 0x06130027 0x01230000 gameplay,char transfer (3)

-Elan Doree : Legend of Dragon [elandore]
 [0]        [1]        [2]        [3]
 No protection                               test mode
 No protection                               attract mode
 0x000y0000 0x00000000 0x****00** 0xff7f0000 gameplay,VDP-1 write (textures on humans)
 0x000y0000 0x00000000 0x****00** 0xffbf0000 gameplay,VDP-1 write (textures on humans)

 0x000y0000 0x00000000 0x****00** 0xf9ff0000 gameplay,VDP-1 write (textures on dragons)
 0x000y0000 0x00000000 0x****00** 0xfbff0000 gameplay,VDP-1 write (textures on dragons)
 0x000y0000 0x00000000 0x****00** 0xfe7f0000 gameplay,VDP-1 write (textures on dragons)
 0x000y0000 0x00000000 0x****00** 0xfd7f0000 gameplay,VDP-1 write (textures on dragons)
 0x000y0000 0x00000000 0x****00** 0xfeff0000 gameplay,VDP-1 write (textures on dragons)
 0x000y0000 0x00000000 0x****00** 0xf9bf0000 gameplay,VDP-1 write (textures on dragons)

-Final Fight Revenge [ffreveng]
 [0]        [1]        [2]        [3]
 0x000y0000 0x00000000 0x4bcc0013 0x10da0000 test mode,boot vectors at $06080000
 0x000y0000 0x00000000 0x0b780013 0x10d70000 attract mode,boot vectors at $06080000
 0x???????? 0x???????? 0x???????? 0x???????? gameplay

-Radiant Silvergun [rsgun]
 [0]        [1]        [2]        [3]
 No protection                               test mode
 0x000y0000 0x00000000 0x08000010 0x77770000 attract mode,work ram-h $60ff1ec and so on (1)
 0x???????? 0x???????? 0x???????? 0x???????? gameplay

-Steep Slope Sliders [sss]
 [0]        [1]        [2]        [3]
 No protection                               test mode
*0x000y0000 0x00000000 0x000000a6 0x2c5b0000 attract mode,VDP-1 write
*0x000y0000 0x00000000 0x000000a6 0x2c5b0000 gameplay,VDP-1 write character 1 (2)
*0x000y0000 0x00000000 0x0f9800a6 0x47f10000 gameplay,VDP-1 write character 2
*0x000y0000 0x00000000 0x1d4800a6 0xfcda0000 gameplay,VDP-1 write character 3
*0x000y0000 0x00000000 0x29e300a6 0xb5e60000 gameplay,VDP-1 write character 4
*0x000y0000 0x00000000 0x38e900a6 0x392c0000 gameplay,VDP-1 write character 5
*0x000y0000 0x00000000 0x462500a6 0x77c30000 gameplay,VDP-1 write character 6
*0x000y0000 0x00000000 0x555c00a6 0x8a620000 gameplay,VDP-1 write character 7

=========================================================================================
y = setted as a 0,then after the ctrl data is moved is toggled to 1 then again toggled
    to 0 after the reading,this bit is likely to be a "calculate protection values"
    if 1,use normal ram if 0.
* = working checks
[3,low word]AFAIK this is the cartridge area and it's read-only.
(1)That area is usually (but not always) used as system registers.
(2)Same as P.O.S.T. check,it was really simple to look-up because of that.
(3)Wrong offset,or it requires something else like a bitswap?
=========================================================================================
Protection works as a sort of data transfer,it could also be that it uses
encryption on the data used...

For now I'm writing this function with a command basis so I can work better with it.
****************************************************************************************/

#include "emu.h"
#include "stvprot.h"

static UINT32 a_bus[4];
static UINT32 ctrl_index;
static UINT32 internal_counter;
static UINT8 char_offset; //helper to jump the decoding of the NULL chars.

/************************
*
* Tecmo World Cup '98
*
************************/

static READ32_HANDLER( twcup98_prot_r )
{
	UINT32 *ROM = (UINT32 *)space.machine().root_device().memregion("abus")->base();

	if(a_bus[0] & 0x00010000)//protection calculation is activated
	{
		if(offset == 3)
		{
			logerror("A-Bus control protection read at %06x with data = %08x\n",space.device().safe_pc(),a_bus[3]);
			#ifdef MAME_DEBUG
			popmessage("Prot read at %06x with data = %08x",space.device().safe_pc(),a_bus[3]);
			#endif
			switch(a_bus[3])
			{

			}
		}
		return a_bus[offset];
	}
	else
	{
		if(a_bus[offset] != 0) return a_bus[offset];
		else return ROM[(0x02fffff0/4)+offset];
	}
}

static WRITE32_HANDLER ( twcup98_prot_w )
{
	COMBINE_DATA(&a_bus[offset]);
	logerror("A-Bus control protection write at %06x: [%02x] <- %08x\n",space.device().safe_pc(),offset,data);
	if(offset == 3)
	{
		logerror("MAIN : %08x  DATA : %08x\n",a_bus[3],a_bus[2]);

		//MAIN : 12120000  DATA : 0ad20069 Tecmo logo
		if(a_bus[3] == 0x12120000 && a_bus[2] == 0x0ad20069)
		{
			// ...
		}
		//MAIN : 12120000  DATA : e332006b title screen
		if(a_bus[3] == 0x12120000 && a_bus[2] == 0xe332006b)
		{
			// ...
		}
	}
	//popmessage("%04x %04x",data,offset/4);
}

void install_twcup98_protection(running_machine &machine)
{
	machine.device("maincpu")->memory().space(AS_PROGRAM).install_legacy_readwrite_handler(0x4fffff0, 0x4ffffff, FUNC(twcup98_prot_r), FUNC(twcup98_prot_w));
}

/**************************
*
* Steep Slope Sliders
*
**************************/

static READ32_HANDLER( sss_prot_r )
{
	UINT32 *ROM = (UINT32 *)space.machine().root_device().memregion("abus")->base();

	if(a_bus[0] & 0x00010000)//protection calculation is activated
	{
		if(offset == 3)
		{
			logerror("A-Bus control protection read at %06x with data = %08x\n",space.device().safe_pc(),a_bus[3]);
			#ifdef MAME_DEBUG
			popmessage("Prot read at %06x with data = %08x",space.device().safe_pc(),a_bus[3]);
			#endif
			switch(a_bus[3])
			{
				case 0x2c5b0000://sss
				case 0x47f10000:
				case 0xfcda0000:
				case 0xb5e60000:
				case 0x392c0000:
				case 0x77c30000:
				case 0x8a620000:
					ctrl_index++;
					return ROM[ctrl_index];
			}
		}
		return a_bus[offset];
	}
	else
	{
		if(a_bus[offset] != 0) return a_bus[offset];
		else return ROM[(0x02fffff0/4)+offset];
	}
}

static WRITE32_HANDLER ( sss_prot_w )
{
	COMBINE_DATA(&a_bus[offset]);
	logerror("A-Bus control protection write at %06x: [%02x] <- %08x\n",space.device().safe_pc(),offset,data);
	if(offset == 3)
	{
		//logerror("MAIN : %08x  DATA : %08x\n",a_bus[3],a_bus[2]);
		switch(a_bus[3])
		{
			case 0x2c5b0000: ctrl_index = (0x145ffac/4)-1; break;
			case 0x47f10000: ctrl_index = ((0x145ffac+0xbaf0)/4)-1; break;
			case 0xfcda0000: ctrl_index = ((0x145ffac+0x12fd0)/4)-1; break;
			case 0xb5e60000: ctrl_index = ((0x145ffac+0x1a4c4)/4)-1; break;
			case 0x392c0000: ctrl_index = ((0x145ffac+0x219b0)/4)-1; break;
			case 0x77c30000: ctrl_index = ((0x145ffac+0x28ea0)/4)-1; break;
			case 0x8a620000: ctrl_index = ((0x145ffac+0x30380)/4)-1; break;
		}
	}
}

void install_sss_protection(running_machine &machine)
{
	machine.device("maincpu")->memory().space(AS_PROGRAM).install_legacy_readwrite_handler(0x4fffff0, 0x4ffffff, FUNC(sss_prot_r), FUNC(sss_prot_w));
}

/*************************************
*
* Radiant Silvergun
*
*************************************/

static READ32_HANDLER( rsgun_prot_r )
{
	UINT32 *ROM = (UINT32 *)space.machine().root_device().memregion("abus")->base();

	if(a_bus[0] & 0x00010000)//protection calculation is activated
	{
		if(offset == 3)
		{
			logerror("A-Bus control protection read at %06x with data = %08x\n",space.device().safe_pc(),a_bus[3]);
			#ifdef MAME_DEBUG
			popmessage("Prot read at %06x with data = %08x",space.device().safe_pc(),a_bus[3]);
			#endif
			switch(a_bus[3])
			{
				case 0x77770000: {//rsgun
					UINT32 val =
						((ctrl_index & 0xff)<<24) |
						(((ctrl_index+1) & 0xff)<<16) |
						(((ctrl_index+2) & 0xff)<<8) |
						((ctrl_index+3) & 0xff);
					if(ctrl_index & 0x100)
						val &= 0x0f0f0f0f;
					else
						val &= 0xf0f0f0f0;

					ctrl_index += 4;
					return val;
				}
			}
		}
		return a_bus[offset];
	}
	else
	{
		if(a_bus[offset] != 0) return a_bus[offset];
		else return ROM[(0x02fffff0/4)+offset];
	}
}

static WRITE32_HANDLER ( rsgun_prot_w )
{
	COMBINE_DATA(&a_bus[offset]);
	logerror("A-Bus control protection write at %06x: [%02x] <- %08x\n",space.device().safe_pc(),offset,data);
	if(offset == 3)
	{
		//logerror("MAIN : %08x  DATA : %08x\n",a_bus[3],a_bus[2]);
		switch(a_bus[3])
		{
			case 0x77770000: ctrl_index = 0; break;
		}
	}
	//popmessage("%04x %04x",data,offset/4);
}

void install_rsgun_protection(running_machine &machine)
{
	machine.device("maincpu")->memory().space(AS_PROGRAM).install_legacy_readwrite_handler(0x4fffff0, 0x4ffffff, FUNC(rsgun_prot_r), FUNC(rsgun_prot_w));
}

/*************************
*
* Elandoree
*
*************************/

#define ELANDORE_CTRL_1_HUMAN   0xff7f0000
#define ELANDORE_CTRL_2_HUMAN   0xffbf0000

#define ELANDORE_CTRL_1_DRAGON  0xf9ff0000
#define ELANDORE_CTRL_2_DRAGON  0xfbff0000
#define ELANDORE_CTRL_3_DRAGON  0xfe7f0000
#define ELANDORE_CTRL_4_DRAGON  0xfd7f0000
#define ELANDORE_CTRL_5_DRAGON  0xfeff0000
#define ELANDORE_CTRL_6_DRAGON  0xf9bf0000

static READ32_HANDLER( elandore_prot_r )
{
	UINT32 *ROM = (UINT32 *)space.machine().root_device().memregion("abus")->base();

	if(a_bus[0] & 0x00010000)//protection calculation is activated
	{
		if(offset == 3)
		{
			logerror("A-Bus control protection read at %06x with data = %08x\n",space.device().safe_pc(),a_bus[3]);
			#ifdef MAME_DEBUG
			popmessage("Prot read at %06x with data = %08x",space.device().safe_pc(),a_bus[3]);
			#endif
			switch(a_bus[3])
			{
				case ELANDORE_CTRL_1_HUMAN:
				case ELANDORE_CTRL_2_HUMAN:
				case ELANDORE_CTRL_1_DRAGON:
				case ELANDORE_CTRL_2_DRAGON:
				case ELANDORE_CTRL_3_DRAGON:
				case ELANDORE_CTRL_4_DRAGON:
				case ELANDORE_CTRL_5_DRAGON:
				case ELANDORE_CTRL_6_DRAGON:
					//ctrl_index++;
					return ROM[ctrl_index];
			}
		}
		return a_bus[offset];
	}
	else
	{
		if(a_bus[offset] != 0) return a_bus[offset];
		else return ROM[(0x02fffff0/4)+offset];
	}
}

static WRITE32_HANDLER ( elandore_prot_w )
{
	COMBINE_DATA(&a_bus[offset]);
	logerror("A-Bus control protection write at %06x: [%02x] <- %08x\n",space.device().safe_pc(),offset,data);
	if(offset == 3)
	{
		/* a bus value 2 seed is used too here. */
		//logerror("MAIN : %08x  DATA : %08x\n",a_bus[3],a_bus[2]);
		switch(a_bus[3])
		{
			case ELANDORE_CTRL_1_HUMAN: // (human polygons)
				ctrl_index = ((0x0000000/4) + ((a_bus[2] & 0xff)<<12) + ((a_bus[2] & 0x0fff0000)>>16)/4);
				break;
			case ELANDORE_CTRL_2_HUMAN: // (human polygons)
				ctrl_index = ((0x0000000/4) + ((a_bus[2] & 0xff)<<12) + ((a_bus[2] & 0x0fff0000)>>16)/4);
				break;
			case ELANDORE_CTRL_1_DRAGON://KAIN / THUNDER (dragon polygons)
				ctrl_index = ((0x0000000/4) + ((a_bus[2] & 0xff)<<12) + ((a_bus[2] & 0x0fff0000)>>16)/4);
				break;
			case ELANDORE_CTRL_2_DRAGON://REVI CURIO / DARK (dragon polygons)
				ctrl_index = ((0x0000000/4) + ((a_bus[2] & 0xff)<<12) + ((a_bus[2] & 0x0fff0000)>>16)/4);
				break;
			case ELANDORE_CTRL_3_DRAGON://RUBONE / POISON (dragon polygons)
				ctrl_index = ((0x0000000/4) + ((a_bus[2] & 0xff)<<12) + ((a_bus[2] & 0x0fff0000)>>16)/4);
				break;
			case ELANDORE_CTRL_4_DRAGON://TINA / MAGICAL GIRL (dragon polygons)
				ctrl_index = ((0x0000000/4) + ((a_bus[2] & 0xff)<<12) + ((a_bus[2] & 0x0fff0000)>>16)/4);
				break;
			case ELANDORE_CTRL_5_DRAGON://KEYAKI / FIRE (dragon polygons)
				ctrl_index = ((0x0000000/4) + ((a_bus[2] & 0xff)<<12) + ((a_bus[2] & 0x0fff0000)>>16)/4);
				break;
			case ELANDORE_CTRL_6_DRAGON://SION / WIND (dragon polygons)
				ctrl_index = ((0x0000000/4) + ((a_bus[2] & 0xff)<<12) + ((a_bus[2] & 0x0fff0000)>>16)/4);
				break;
		}
	}
	//popmessage("%04x %04x",data,offset/4);
}

void install_elandore_protection(running_machine &machine)
{
	machine.device("maincpu")->memory().space(AS_PROGRAM).install_legacy_readwrite_handler(0x4fffff0, 0x4ffffff, FUNC(elandore_prot_r), FUNC(elandore_prot_w));
}

/*************************
*
* Final Fight Revenge
*
*************************/

/*
ffreveng protection notes
Global:
R2 is the vector read (where to jump to)
R3 is the vector pointer

Notes:
-0x234 is a dummy vector to get safe in the debugger without doing
anything and to read the registers.

Right vectors:
0x0603B158 (but not as first,will garbage the registers)

Wrong vectors (at least not where I tested it):
0x060016fc (1st)
0x0603AFE0 (1st) (attempts to read to the sh2 internal register fffffe11)
0x060427FC (1st) (resets the sh2)
0x0603B1B2 (1st) (crashes the sh2)
*/
static const UINT32 vector_prot[] = { 0x0603B1B2,0x234 };


static READ32_HANDLER( ffreveng_prot_r )
{
	UINT32 *ROM = (UINT32 *)space.machine().root_device().memregion("abus")->base();

	if(a_bus[0] & 0x00010000)//protection calculation is activated
	{
		if(offset == 3)
		{
			logerror("A-Bus control protection read at %06x with data = %08x\n",space.device().safe_pc(),a_bus[3]);
			#ifdef MAME_DEBUG
			popmessage("Prot read at %06x with data = %08x",space.device().safe_pc(),a_bus[3]);
			#endif
			switch(a_bus[3])
			{
				case 0x10da0000://ffreveng, boot vectors at $6080000,test mode
					ctrl_index++;
					if(ctrl_index > 2)
						return 0x234;
					else
						return vector_prot[ctrl_index-1];
				case 0x10d70000://ffreveng, boot vectors at $6080000,attract mode
					ctrl_index++;
					return ROM[ctrl_index];
			}
		}
		return a_bus[offset];
	}
	else
	{
		if(a_bus[offset] != 0) return a_bus[offset];
		else return ROM[(0x02fffff0/4)+offset];
	}
}

static WRITE32_HANDLER ( ffreveng_prot_w )
{
	COMBINE_DATA(&a_bus[offset]);
	logerror("A-Bus control protection write at %06x: [%02x] <- %08x\n",space.device().safe_pc(),offset,data);
	if(offset == 3)
	{
		//logerror("MAIN : %08x  DATA : %08x\n",a_bus[3],a_bus[2]);
		switch(a_bus[3])
		{
			/*ffreveng*/
			case 0x10d70000: ctrl_index = 0; break;
			case 0x10da0000: ctrl_index = 0; break;
		}
	}
	//popmessage("%04x %04x",data,offset/4);
}

void install_ffreveng_protection(running_machine &machine)
{
	machine.device("maincpu")->memory().space(AS_PROGRAM).install_legacy_readwrite_handler(0x4fffff0, 0x4ffffff, FUNC(ffreveng_prot_r), FUNC(ffreveng_prot_w));
}

/************************
*
* Astra Super Stars
*
************************/

static READ32_HANDLER(astrass_prot_r)
{
	if ( offset == 3 && ctrl_index != -1 )
	{
		UINT32 data = 0;
		UINT32 *prot_data = (UINT32 *)space.machine().root_device().memregion("user2")->base();

		data = prot_data[ctrl_index++];

		if ( ctrl_index >= space.machine().root_device().memregion("user2")->bytes()/4 )
		{
			ctrl_index = -1;
		}

		return data;
	}
	return a_bus[offset];
}

static WRITE32_HANDLER(astrass_prot_w)
{
	COMBINE_DATA(&a_bus[0 + offset]);
	if ( offset == 3 )
	{
		ctrl_index = 0;
	}
}

void install_astrass_protection(running_machine &machine)
{
	ctrl_index = -1;
	machine.device("maincpu")->memory().space(AS_PROGRAM).install_legacy_readwrite_handler(0x4fffff0, 0x4ffffff, FUNC(astrass_prot_r), FUNC(astrass_prot_w));
}

/**************************
*
* Decathlete
*
**************************/

/* Decathlete seems to be a variation on this ... not understood */
static UINT32 decathlt_protregs[4];
static UINT32 decathlt_lastcount = 0;
static UINT32 decathlt_part;
static UINT32 decathlt_prot_uploadmode=0;
static UINT32 decathlt_prot_uploadoffset=0;
static UINT16 decathlt_prottable1[24];
static UINT16 decathlt_prottable2[128];

static READ32_HANDLER( decathlt_prot_r )
{
	// the offsets written to the protection device definitely only refer to 2 of the roms
	//  it's a fair assumption to say that only those 2 are connected to the protection device
	UINT8 *ROM = (UINT8 *)space.machine().root_device().memregion("abus")->base()+0x1000000;

	if (offset==2)
	{
		UINT32 retvalue;

		retvalue = ROM[(decathlt_protregs[0]*2)-2]; decathlt_protregs[0]++;
		retvalue <<= 8;
		retvalue |= ROM[(decathlt_protregs[0]*2)+1-2];
		retvalue <<= 8;
		retvalue |= ROM[(decathlt_protregs[0]*2)-2]; decathlt_protregs[0]++;
		retvalue <<= 8;
		retvalue |= ROM[(decathlt_protregs[0]*2)+1-2];


		decathlt_lastcount++;
		logerror("blah_r %08x\n", retvalue);
		return retvalue;
	}
	else
	{
		logerror("%06x Decathlete prot R offset %04x mask %08x regs %08x, %08x, %08x, %08x\n",space.device().safe_pc(), offset, mem_mask, decathlt_protregs[0], decathlt_protregs[1], decathlt_protregs[2], decathlt_protregs[3]);
	}

	return decathlt_protregs[offset];
}


void write_prot_data(UINT32 data, UINT32 mem_mask, int offset, int which)
{
	decathlt_protregs[offset] = (data&mem_mask)|(decathlt_protregs[offset]&~mem_mask);
//  decathlt_protregs[0] = 0x0c00000/4;

	if (offset==0) // seems to set a (scrambled?) source address
	{
		decathlt_part ^=1;

		//if (decathlt_part==0) logerror("%d, last read count was %06x\n",which, decathlt_lastcount*4);
		decathlt_lastcount = 0;
		if (decathlt_part==1) logerror("%d Decathlete prot W offset %04x data %08x, %08x, >>> regs %08x <<<<, %08x, %08x, %08x\n",which, offset, data, decathlt_protregs[0], decathlt_protregs[0]*4, decathlt_protregs[1], decathlt_protregs[2], decathlt_protregs[3]);
	}

	if (offset==1) // uploads 2 tables...
	{
		if (mem_mask==0xffff0000)
		{
			if (data == 0x80000000)
			{
			//  logerror("changed to upload mode 1\n");
				decathlt_prot_uploadmode = 1;
				decathlt_prot_uploadoffset = 0;
			}
			else if (data == 0x80800000)
			{
			//  logerror("changed to upload mode 2\n");
				decathlt_prot_uploadmode = 2;
				decathlt_prot_uploadoffset = 0;
			}
			else
			{
			//  logerror("unknown upload mode\n");
				decathlt_prot_uploadmode = 2;
				decathlt_prot_uploadoffset = 0;
			}

//          logerror("ARGH! %08x %08x\n",mem_mask,data);
		}
		else if (mem_mask==0x0000ffff)
		{
			if (decathlt_prot_uploadmode==1)
			{
				if (decathlt_prot_uploadoffset>=24)
				{
				//  logerror("upload mode 1 error, too big\n");
					return;
				}

				//logerror("uploading table 1 %04x %04x\n",decathlt_prot_uploadoffset, data&0xffff);
				decathlt_prottable1[decathlt_prot_uploadoffset]=data&0xffff;
				decathlt_prot_uploadoffset++;

				{
					/* 0x18 (24) values in this table, rom data is 0x1800000 long, maybe it has
                       something to do with that? or 24-address bits?

                       uploaded values appear to be 12-bit, some are repeated
                    */

					{

						FILE* fp;
						if (which==1) fp = fopen("table1x","wb");
						else fp = fopen("table1","wb");

						{
							fwrite(&decathlt_prottable1,24,2,fp);
						}
						fclose(fp);
					}
				}

			}
			else if (decathlt_prot_uploadmode==2)
			{
				if (decathlt_prot_uploadoffset>=128)
				{
					//logerror("upload mode 2 error, too big\n");
					return;
				}

				//logerror("uploading table 2 %04x %04x\n",decathlt_prot_uploadoffset, data&0xffff);
				decathlt_prottable2[decathlt_prot_uploadoffset]=data&0xffff;
				decathlt_prot_uploadoffset++;

				{
					/* the table uploaded here is a 256 byte table with 256 unique values, remaps something? */

					{
						FILE* fp;
						if (which==1) fp = fopen("table2x","wb");
						else fp = fopen("table2","wb");

						{
							fwrite(&decathlt_prottable2,128,2,fp);
						}
						fclose(fp);
					}
				}
			}
			else
			{
			//  logerror("unknown upload mode!\n");
			}
		}
	}

	if (offset>1)
	{
	//  logerror("higher offset write\n");
	}

}

static WRITE32_HANDLER( decathlt_prot1_w )
{
	write_prot_data(data,mem_mask, offset, 0);

}

static WRITE32_HANDLER( decathlt_prot2_w )
{
	write_prot_data(data,mem_mask, offset, 1);


}

void install_decathlt_protection(running_machine &machine)
{
	/* It uploads 2 tables here, then performs what looks like a number of transfers, setting
       a source address of some kind (scrambled?) and then making many reads from a single address */
	memset(decathlt_protregs, 0, sizeof(decathlt_protregs));
	decathlt_lastcount = 0;
	decathlt_prot_uploadmode = 0;
	decathlt_prot_uploadoffset = 0;
	decathlt_part = 1;
	machine.device("maincpu")->memory().space(AS_PROGRAM).install_legacy_readwrite_handler(0x37FFFF0, 0x37FFFFF, FUNC(decathlt_prot_r), FUNC(decathlt_prot1_w));
	/* It accesses the device at this address too, with different tables, for the game textures, should it just act like a mirror, or a secondary device? */
	machine.device("maincpu")->memory().space(AS_PROGRAM).install_legacy_readwrite_handler(0x27FFFF0, 0x27FFFFF, FUNC(decathlt_prot_r), FUNC(decathlt_prot2_w));
}

void stv_register_protection_savestates(running_machine &machine)
{
	state_save_register_global_array(machine, a_bus);
	state_save_register_global(machine, ctrl_index);
	state_save_register_global(machine, internal_counter);
	state_save_register_global(machine, char_offset);
}
