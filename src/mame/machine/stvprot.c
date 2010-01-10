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

#define ELANDORE_CTRL_1_HUMAN   0xff7f0000
#define ELANDORE_CTRL_2_HUMAN   0xffbf0000

#define ELANDORE_CTRL_1_DRAGON  0xf9ff0000
#define ELANDORE_CTRL_2_DRAGON  0xfbff0000
#define ELANDORE_CTRL_3_DRAGON  0xfe7f0000
#define ELANDORE_CTRL_4_DRAGON  0xfd7f0000
#define ELANDORE_CTRL_5_DRAGON  0xfeff0000
#define ELANDORE_CTRL_6_DRAGON  0xf9bf0000


static READ32_HANDLER( a_bus_ctrl_r )
{
	UINT32 *ROM = (UINT32 *)memory_region(space->machine, "user1");

	if(a_bus[0] & 0x00010000)//protection calculation is activated
	{
		if(offset == 3)
		{
			logerror("A-Bus control protection read at %06x with data = %08x\n",cpu_get_pc(space->cpu),a_bus[3]);
			#ifdef MAME_DEBUG
			popmessage("Prot read at %06x with data = %08x",cpu_get_pc(space->cpu),a_bus[3]);
			#endif
			switch(a_bus[3])
			{
				case 0x01230000://astrass,char data in test mode PC=60118f2
				/*
                TODO: the chars that are used are 8x8x4,but we need to convert them to
                16x16x4 format.
                [01] NULL
                [02] NULL
                [03] NULL
                [04] NULL
                [05] NULL
                [06] NULL
                [07] NULL
                [08] NULL
                [09] data 1
                [10] data 2
                [11] data 3
                [12] data 4
                [13] data 5
                [14] data 6
                [15] data 7
                [16] data 8

                2b5 = A (up-left)
                2b6 = A (up-right)
                2d1 = A (down-left)
                2d0 = A (down-right)
                2f2 = S (up-left)
                2f3 = S (up-right)
                311 = S (down-left)
                310 = S (down-right)
                2f4 = T (up-left)
                2f5 = T (up-right)
                2b7 = E (up-left) (not 2bc,maybe because the up-left corner of E is equal of B?)
                2bd = E (up-right)
                */
					internal_counter++;
					if(char_offset == 0)
					{
						if(internal_counter > ((8*0x234)-1))
						{
							internal_counter = 0;
							char_offset = 1;
						}

						return 0;
					}
					else
					{
						if(internal_counter > 8)
						{
							if(internal_counter > 15)
							{
								ctrl_index-=8;
								internal_counter = 0;
							}

							ctrl_index++;
							return ROM[ctrl_index];
						}
						else
						{
							ctrl_index++;
							return ROM[ctrl_index];
						}
					}
				case 0x10da0000://ffreveng, boot vectors at $6080000,test mode
					ctrl_index++;
					if(ctrl_index > 2)
						return 0x234;
					else
						return vector_prot[ctrl_index-1];
				case 0x10d70000://ffreveng, boot vectors at $6080000,attract mode
					ctrl_index++;
					return ROM[ctrl_index];
				case 0x2c5b0000://sss
				case 0x47f10000:
				case 0xfcda0000:
				case 0xb5e60000:
				case 0x392c0000:
				case 0x77c30000:
				case 0x8a620000:
					ctrl_index++;
					return ROM[ctrl_index];
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
				//elandore
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

static WRITE32_HANDLER ( a_bus_ctrl_w )
{
	COMBINE_DATA(&a_bus[offset]);
	logerror("A-Bus control protection write at %06x: [%02x] <- %08x\n",cpu_get_pc(space->cpu),offset,data);
	if(offset == 3)
	{
		//printf("MAIN : %08x  DATA : %08x\n",a_bus[3],a_bus[2]);
		switch(a_bus[3])
		{
			/*astrass,I need an original test mode screen to compare...*/
			case 0x01230000:
			char_offset = 0;
			internal_counter = 0;
			ctrl_index = ((0x400000)/4)-1; break;
			/*ffreveng*/
			case 0x10d70000: ctrl_index = 0; break;
			case 0x10da0000: ctrl_index = 0; break;
			/*sss,TRUSTED*/
			case 0x2c5b0000: ctrl_index = (0x145ffac/4)-1; break;
			/*sss,might be offset...*/
			case 0x47f10000: ctrl_index = ((0x145ffac+0xbaf0)/4)-1; break;
			case 0xfcda0000: ctrl_index = ((0x145ffac+0x12fd0)/4)-1; break;
			case 0xb5e60000: ctrl_index = ((0x145ffac+0x1a4c4)/4)-1; break;
			case 0x392c0000: ctrl_index = ((0x145ffac+0x219b0)/4)-1; break;
			case 0x77c30000: ctrl_index = ((0x145ffac+0x28ea0)/4)-1; break;
			case 0x8a620000: ctrl_index = ((0x145ffac+0x30380)/4)-1; break;
			/*rsgun*/
			case 0x77770000: ctrl_index = 0; break;
			/*elandore*/
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

void install_standard_protection(running_machine *machine)
{
	memory_install_readwrite32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x4fffff0, 0x4ffffff, 0, 0, a_bus_ctrl_r, a_bus_ctrl_w);
}

static READ32_HANDLER(astrass_prot_r)
{
	if ( offset == 3 && ctrl_index != -1 )
	{
		UINT32 data = 0;
		UINT32 *prot_data = (UINT32 *)memory_region(space->machine, "user2");

		data = prot_data[ctrl_index++];

		if ( ctrl_index >= memory_region_length(space->machine, "user2")/4 )
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

void install_astrass_protection(running_machine *machine)
{
	ctrl_index = -1;
	memory_install_readwrite32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x4fffff0, 0x4ffffff, 0, 0, astrass_prot_r, astrass_prot_w);
}


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
	UINT32 *ROM = (UINT32 *)memory_region(space->machine, "user1");

	if (offset==2)
	{
		//UINT32 retval;
		/* I think the address and data are scrambled.. */
		UINT32 retvalue = /*rand() | (rand()<<16);*/ ROM[(decathlt_protregs[0])];
		decathlt_protregs[0]++;
		decathlt_lastcount++;
		return retvalue; // reads this, then the game writes it to vram...
	}
	else
	{
		mame_printf_info("%06x Decathlete prot R offset %04x mask %08x regs %08x, %08x, %08x, %08x\n",cpu_get_pc(space->cpu), offset, mem_mask, decathlt_protregs[0], decathlt_protregs[1], decathlt_protregs[2], decathlt_protregs[3]);
	}

	return decathlt_protregs[offset];
}

static WRITE32_HANDLER( decathlt_prot_w )
{
	decathlt_protregs[offset] = (data&mem_mask)|(decathlt_protregs[offset]&~mem_mask);
//  decathlt_protregs[0] = 0x0c00000/4;

	if (offset==0) // seems to set a (scrambled?) source address
	{
		decathlt_part ^=1;

		if (decathlt_part==0) mame_printf_info("last count was %06x\n",decathlt_lastcount);
		decathlt_lastcount = 0;
		mame_printf_info("%06x Decathlete prot W offset %04x data %08x, regs %08x, %08x, %08x, %08x\n",cpu_get_pc(space->cpu), offset, data, decathlt_protregs[0], decathlt_protregs[1], decathlt_protregs[2], decathlt_protregs[3]);
	}

	if (offset==1) // uploads 2 tables...
	{
		if (mem_mask==0xffff0000)
		{
			if (data == 0x80000000)
			{
				mame_printf_info("changed to upload mode 1\n");
				decathlt_prot_uploadmode = 1;
				decathlt_prot_uploadoffset = 0;
			}
			else if (data == 0x80800000)
			{
				mame_printf_info("changed to upload mode 2\n");
				decathlt_prot_uploadmode = 2;
				decathlt_prot_uploadoffset = 0;
			}
			else
			{
				mame_printf_info("unknown upload mode\n");
				decathlt_prot_uploadmode = 2;
				decathlt_prot_uploadoffset = 0;
			}

//          mame_printf_info("ARGH! %08x %08x\n",mem_mask,data);
		}
		else if (mem_mask==0x0000ffff)
		{
			if (decathlt_prot_uploadmode==1)
			{
				if (decathlt_prot_uploadoffset>=24)
				{
					mame_printf_info("upload mode 1 error, too big\n");
					return;
				}

				mame_printf_info("uploading table 1 %04x %04x\n",decathlt_prot_uploadoffset, data&0xffff);
				decathlt_prottable1[decathlt_prot_uploadoffset]=data&0xffff;
				decathlt_prot_uploadoffset++;

				{
					/* 0x18 (24) values in this table, rom data is 0x1800000 long, maybe it has
                       something to do with that? or 24-address bits?

                       uploaded values appear to be 12-bit, some are repeated
                    */
					FILE* fp;
					fp = fopen("table1","wb");
					{
						fwrite(&decathlt_prottable2,24,2,fp);
					}
					fclose(fp);
				}

			}
			else if (decathlt_prot_uploadmode==2)
			{
				if (decathlt_prot_uploadoffset>=128)
				{
					mame_printf_info("upload mode 2 error, too big\n");
					return;
				}

				mame_printf_info("uploading table 2 %04x %04x\n",decathlt_prot_uploadoffset, data&0xffff);
				decathlt_prottable2[decathlt_prot_uploadoffset]=data&0xffff;
				decathlt_prot_uploadoffset++;

				{
					/* the table uploaded here is a 256 byte table with 256 unique values, remaps something? */
					FILE* fp;
					fp = fopen("table2","wb");
					{
						fwrite(&decathlt_prottable2,128,2,fp);
					}
					fclose(fp);
				}
			}
			else
			{
				mame_printf_info("unknown upload mode!\n");
			}
		}
	}

	if (offset>1)
	{
		mame_printf_info("higher offset write\n");
	}

}

void install_decathlt_protection(running_machine *machine)
{
	/* It uploads 2 tables here, then performs what looks like a number of transfers, setting
       a source address of some kind (scrambled?) and then making many reads from a single address */
	memset(decathlt_protregs, 0, sizeof(decathlt_protregs));
	decathlt_lastcount = 0;
	decathlt_prot_uploadmode = 0;
	decathlt_prot_uploadoffset = 0;
	decathlt_part = 1;
	memory_install_readwrite32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x37FFFF0, 0x37FFFFF, 0, 0, decathlt_prot_r, decathlt_prot_w);
	/* It uploads 2 tables here too, but nothing else, mirror? unused? */
//  memory_install_readwrite32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x27FFFF0, 0x27FFFFF, 0, 0, decathlt_prot_r, decathlt_prot_w);
}

void stv_register_protection_savestates(running_machine *machine)
{
	state_save_register_global_array(machine, a_bus);
	state_save_register_global(machine, ctrl_index);
	state_save_register_global(machine, internal_counter);
	state_save_register_global(machine, char_offset);
}
