/* The Decathlete 'protection' seems to be some kind of Huffman style compression on the graphics,
  the game uploads 2 dictionary tables (for different areas of the ROM) as well as an additional
  table with each one.  The secondary table doesn't initially appear to be the data needed to
  build the trees required for decompression */

#include "emu.h"
#include "stvprot.h"
#include "includes/stv.h"

/**************************
*
* Decathlete
*
**************************/


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
	UINT32 *fake0 = (UINT32*)space.machine().root_device().memregion( "fake0" )->base();

	if (offset==2)
	{
		UINT32 retvalue = 0xffff;

		switch (decathlt_protregs[0])
		{
			default:
			retvalue = ROM[(decathlt_protregs[0]*2)-2];
			retvalue <<= 8;
			retvalue |= ROM[((decathlt_protregs[0]+1)*2)+1-2];
			retvalue <<= 8;
			retvalue |= ROM[((decathlt_protregs[0]+1)*2)-2];
			retvalue <<= 8;
			retvalue |= ROM[((decathlt_protregs[0]+2)*2)+1-2];
			decathlt_lastcount++;
			logerror("read addr %08x, blah_r %08x - read count count %08x\n", decathlt_protregs[0], retvalue, decathlt_lastcount*4);
			decathlt_protregs[0]+=2;
			return retvalue;

			case 0x03228e4:
				if (fake0) retvalue = fake0[(((0x20080/4)+decathlt_lastcount))];
				decathlt_lastcount++;
				return retvalue;

			case 0x00a9f3a:
				if (fake0) retvalue = fake0[(((0x00000/4)+decathlt_lastcount))];
				decathlt_lastcount++;
				return retvalue;

			case 0x0213ab4:
				if (fake0) retvalue = fake0[(((0x40000/4)+decathlt_lastcount))];
				decathlt_lastcount++;
				return retvalue;

			case 0x01efaf0:
				if (fake0) retvalue = fake0[(((0x60000/4)+decathlt_lastcount))];
				decathlt_lastcount++;
				return retvalue;

			case 0x033f16c:
			case 0x038929c:



			case 0x00de05a:
			case 0x0334258:
			case 0x019fb82:
			case 0x033dbf6:
			case 0x0011ac6:
			case 0x00060dc:
			case 0x0000002:
			case 0x0008c90:
			case 0x035cdc8:
			case 0x0327960:
			case 0x0329b8c:
			case 0x00d6e92:
			case 0x000081e:
			case 0x00035d6:
			case 0x00089a6:
			case 0x03315f4:
			case 0x0023fe0:
			case 0x001e290:
			case 0x0026e86:
			case 0x0012494:
			case 0x001b35a:
			case 0x0018424:

				return retvalue;
		}


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
