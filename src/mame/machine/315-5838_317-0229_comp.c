/* Sega Compression (and possibly encryption) device

	315-5838 - Decathlete (ST-V)
	317-0229 - Dead or Alive (Model 2A)

	Package Type: TQFP100

	This appears to be a dual channel compression chip, used in 1996, predating the 5881.
	Decathlete uses it to compress ALL the game graphics, Dead or Alive uses it for a
	dumb security check, decompressing a single string.

	Each channel appears to be connected to a different set of ROMs, however there is
	defintiely only 315-5838 single chip.

	Dead of Alive only uses a single channel, and has the source data in RAM, not ROM.
	This is similar to how some 5881 games were set up, with the ST-V versions decrypting
	data directly from ROM and the Model 2 ones using a RAM source buffer.

	Looking at the values read I don't think there is any address based encryption, for
	example many blocks where you'd expect a zero fill start with repeating patterns
	of 8f708f70 (different lengths) which would appear to relate to compressed 0x00 data

	read addr 0071253c, blah_r 8f708f70 - read count count 00000004
	read addr 00712540, blah_r 8f708f70 - read count count 00000008
	read addr 00712544, blah_r 8f708f70 - read count count 0000000c
	read addr 00712548, blah_r 8f708f70 - read count count 00000010
	read addr 0071254c, blah_r 8f708f70 - read count count 00000014
	read addr 00712550, blah_r 8f708f70 - read count count 00000018
	read addr 00712554, blah_r 8f708f70 - read count count 0000001c

*/

#include "emu.h"
#include "machine/315-5838_317-0229_comp.h"

extern const device_type SEGA315_5838_COMP = &device_creator<sega_315_5838_comp_device>;

//#define DEBUG_DATA_DUMP

sega_315_5838_comp_device::sega_315_5838_comp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SEGA315_5838_COMP, "Sega 315-5838 / 317-0029 Compression (Encryption?)", tag, owner, clock, "SEGA315_5838", __FILE__)
{
}



void sega_315_5838_comp_device::device_start()
{
	m_decathlt_lastcount = 0;
	m_decathlt_prot_uploadmode = 0;
	m_decathlt_prot_uploadoffset = 0;

	m_read_ch1.bind_relative_to(*owner());
	m_read_ch2.bind_relative_to(*owner());
}

void sega_315_5838_comp_device::device_reset()
{
	memset(m_decathlt_protregs, 0, sizeof(m_decathlt_protregs));
	m_decathlt_lastcount = 0;
	m_decathlt_prot_uploadmode = 0;
	m_decathlt_prot_uploadoffset = 0;
	m_decathlt_part = 1;

	m_protstate = 0;
}


/**************************
*
* Decathlete
*
**************************/

#ifdef DEBUG_DATA_DUMP
FILE* tempfile;
#endif


READ32_MEMBER(sega_315_5838_comp_device::decathlt_prot_r)
{
	return genericdecathlt_prot_r(offset, mem_mask, 0);
}

READ32_MEMBER(sega_315_5838_comp_device::decathlt_prot_ch2_r)
{
	return genericdecathlt_prot_r(offset, mem_mask, 1);
}


UINT32 sega_315_5838_comp_device::genericdecathlt_prot_r(UINT32 offset, UINT32 mem_mask, int which)
{

//	UINT32 *fake0 = (UINT32*)memregion( ":fake0" )->base();

	if (offset==2)
	{
//		UINT32 retvalue = 0xffff;

		switch (m_decathlt_protregs[0])
		{
			default:

			m_decathlt_lastcount++;

			UINT32 tempdata = 0;

			if (which == 0)
			{
				tempdata |= m_read_ch1(m_decathlt_protregs[0]) << 16;
				m_decathlt_protregs[0]++;
				tempdata |= m_read_ch1(m_decathlt_protregs[0]) << 0;
				m_decathlt_protregs[0]++;
			}
			else
			{
				tempdata |= m_read_ch2(m_decathlt_protregs[0]) << 16;
				m_decathlt_protregs[0]++;
				tempdata |= m_read_ch2(m_decathlt_protregs[0]) << 0;
				m_decathlt_protregs[0]++;
			}

			#ifdef DEBUG_DATA_DUMP
			//printf("read addr %08x, blah_r %08x - read count count %08x\n", m_decathlt_protregs[0]*2, tempdata,  m_decathlt_lastcount*4);
			fwrite(&tempdata, 1, 4, tempfile);
			#else
			logerror("read addr %08x, blah_r %08x - read count count %08x\n", m_decathlt_protregs[0]*2, tempdata,  m_decathlt_lastcount*4);
			#endif

			return tempdata;
#if 0
			case 0x03228e4:
				if (fake0) retvalue = fake0[(((0x20080/4)+m_decathlt_lastcount))];
				m_decathlt_lastcount++;
				return retvalue;

			case 0x00a9f3a:
				if (fake0) retvalue = fake0[(((0x00000/4)+m_decathlt_lastcount))];
				m_decathlt_lastcount++;
				return retvalue;

			case 0x0213ab4:
				if (fake0) retvalue = fake0[(((0x40000/4)+m_decathlt_lastcount))];
				m_decathlt_lastcount++;
				return retvalue;

			case 0x01efaf0:
				if (fake0) retvalue = fake0[(((0x60000/4)+m_decathlt_lastcount))];
				m_decathlt_lastcount++;
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
#endif
		}


	}
	else
	{
		logerror("%06x Decathlete prot R offset %04x mask %08x regs %08x, %08x, %08x, %08x\n", safe_pc(), offset, mem_mask, m_decathlt_protregs[0], m_decathlt_protregs[1], m_decathlt_protregs[2], m_decathlt_protregs[3]);
	}

	return m_decathlt_protregs[offset];
}


void sega_315_5838_comp_device::write_prot_data(UINT32 data, UINT32 mem_mask, int offset, int which)
{

	printf("write_prot_data %08x %08x %08x\n", offset, data, mem_mask);

	m_decathlt_protregs[offset] = (data&mem_mask)|(m_decathlt_protregs[offset]&~mem_mask);
//  m_decathlt_protregs[0] = 0x0c00000/4;

	if (offset==0) // seems to set a source address
	{
		m_decathlt_part ^=1;

		//if (m_decathlt_part==0) logerror("%d, last read count was %06x\n",which, m_decathlt_lastcount*4);
		m_decathlt_lastcount = 0;
		if (m_decathlt_part==1) logerror("%d Decathlete prot W offset %04x data %08x, %08x, >>> regs %08x <<<<, %08x, %08x, %08x\n",which, offset, data, m_decathlt_protregs[0], m_decathlt_protregs[0]*4, m_decathlt_protregs[1], m_decathlt_protregs[2], m_decathlt_protregs[3]);

#ifdef DEBUG_DATA_DUMP
		if (mem_mask == 0x0000ffff)
		{
			if (tempfile)
				fclose(tempfile);

			char filename[256];
			sprintf(filename, "%d_compressed_%08x", which, m_decathlt_protregs[0] );
			tempfile = fopen(filename, "w+b");
		}
#endif

	}

	if (offset==1) // uploads 2 tables...
	{
		if (mem_mask==0xffff0000)
		{
			if (data == 0x80000000)
			{
			//  logerror("changed to upload mode 1\n");
				m_decathlt_prot_uploadmode = 1;
				m_decathlt_prot_uploadoffset = 0;
			}
			else if (data == 0x80800000)
			{
			//  logerror("changed to upload mode 2\n");
				m_decathlt_prot_uploadmode = 2;
				m_decathlt_prot_uploadoffset = 0;
			}
			else
			{
			//  logerror("unknown upload mode\n");
				m_decathlt_prot_uploadmode = 2;
				m_decathlt_prot_uploadoffset = 0;
			}

//          logerror("ARGH! %08x %08x\n",mem_mask,data);
		}
		else if (mem_mask==0x0000ffff)
		{
			if (m_decathlt_prot_uploadmode==1)
			{
				if (m_decathlt_prot_uploadoffset>=24)
				{
				//  logerror("upload mode 1 error, too big\n");
					return;
				}

				//logerror("uploading table 1 %04x %04x\n",m_decathlt_prot_uploadoffset, data&0xffff);
				m_decathlt_prottable1[m_decathlt_prot_uploadoffset]=data&0xffff;
				m_decathlt_prot_uploadoffset++;

				printf("table 1 %04x\n", data & 0xffff);

				{
					/* 0x18 (24) values in this table, rom data is 0x1800000 long, maybe it has
					   something to do with that? or 24-address b	 its?

					   uploaded values appear to be 12-bit, some are repeated
					*/

					{
						FILE* fp;
						if (which==1) fp = fopen("table1x","wb");
						else fp = fopen("table1","wb");

						{
							fwrite(&m_decathlt_prottable1,24,2,fp);
						}
						fclose(fp);
					}
				}

			}
			else if (m_decathlt_prot_uploadmode==2)
			{
				if (m_decathlt_prot_uploadoffset>=128)
				{
					//logerror("upload mode 2 error, too big\n");
					return;
				}

				//logerror("uploading table 2 %04x %04x\n",m_decathlt_prot_uploadoffset, data&0xffff);
				m_decathlt_prottable2[m_decathlt_prot_uploadoffset]=data&0xffff;
				m_decathlt_prot_uploadoffset++;

				printf("dictionary %04x\n", data & 0xffff);


				{
					/* the table uploaded here is a 256 byte table with 256 unique values, remaps something? */

					{
						FILE* fp;
						if (which==1) fp = fopen("table2x","wb");
						else fp = fopen("table2","wb");

						{
							fwrite(&m_decathlt_prottable2,128,2,fp);
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

WRITE32_MEMBER( sega_315_5838_comp_device::decathlt_prot1_w )
{
	write_prot_data(data,mem_mask, offset, 0);

}

WRITE32_MEMBER( sega_315_5838_comp_device::decathlt_prot2_w )
{
	write_prot_data(data,mem_mask, offset, 1);


}

void sega_315_5838_comp_device::install_decathlt_protection()
{
	/* It uploads 2 tables here, then performs what looks like a number of transfers, setting
	   a source address of some kind (scrambled?) and then making many reads from a single address */

	//todo, install these in the driver, they differ between games
	cpu_device* cpu = (cpu_device*)machine().device(":maincpu");


	cpu->space(AS_PROGRAM).install_readwrite_handler(0x37FFFF0, 0x37FFFFF, read32_delegate(FUNC(sega_315_5838_comp_device::decathlt_prot_r), this), write32_delegate(FUNC(sega_315_5838_comp_device::decathlt_prot1_w), this));
	/* It accesses the device at this address too, with different tables, for the game textures, should it just act like a mirror, or a secondary device? */
	cpu->space(AS_PROGRAM).install_readwrite_handler(0x27FFFF0, 0x27FFFFF, read32_delegate(FUNC(sega_315_5838_comp_device::decathlt_prot_ch2_r), this), write32_delegate(FUNC(sega_315_5838_comp_device::decathlt_prot2_w), this));
}


READ32_MEMBER(sega_315_5838_comp_device::doa_prot_r)
{
	UINT32 retval = 0;

	if (offset == 0x7ff8/4)
	{
		retval = m_protram[m_protstate+1] | m_protram[m_protstate]<<8;
		m_protstate+=2;
		printf("doa_prot_read %08x %08x %08x\n", offset*4, retval, mem_mask);
	}
	else if (offset == 0x400c/4) // todo, is this actually part of the protection? it's in the address range, but decathlete doesn't have it afaik.
	{
		m_prot_a = !m_prot_a;
		if (m_prot_a)
			retval = 0xffff;
		else
			retval = 0xfff0;
	}
	else
	{
		printf("doa_prot_read %08x %08x %08x\n", offset*4, retval, mem_mask);
		logerror("Unhandled Protection READ @ %x mask %x (PC=%x)\n", offset, mem_mask, space.device().safe_pc());
	}

	return retval;
}


WRITE32_MEMBER(sega_315_5838_comp_device::doa_prot_w)
{
	printf("doa_prot_w %08x %08x %08x\n", offset*4, data, mem_mask);

	if (offset == 0x7ff2 / 4)
	{
		if (data == 0)
		{
			m_protstate = 0;
			strcpy((char *)m_protram, "  TECMO LTD.  DEAD OR ALIVE  1996.10.22  VER. 1.00"); // this is the single decompressed string DOA needs
		}
	}
	else logerror("Unhandled Protection WRITE %x @ %x mask %x (PC=%x)\n", data, offset, mem_mask, space.device().safe_pc());
}


void sega_315_5838_comp_device::install_doa_protection()
{
	//todo, install these in the driver, they differ between games
	cpu_device* cpu = (cpu_device*)machine().device(":maincpu");
	cpu->space(AS_PROGRAM).install_readwrite_handler(0x01d80000, 0x01dfffff, read32_delegate(FUNC(sega_315_5838_comp_device::doa_prot_r), this), write32_delegate(FUNC(sega_315_5838_comp_device::doa_prot_w), this));
}