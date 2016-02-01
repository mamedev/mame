// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Sega Compression (and possibly encryption) device

    315-5838 - Decathlete (ST-V)
    317-0229 - Dead or Alive (Model 2A)
	317-0229 - Name Club Ver 2 (ST-V) (tested as RCDD2 in the service menu!)
	317-0231 - Print Club Love Love (ST-V)

	Several Print Club (ST-V) carts have
	an unpopulated space marked '317-0229' on the PCB

    Package Type: TQFP100

    Decathlete accesses the chip at 2 different addresses, however, I don't think there
	are 2 channels / sets of registers, instead the 2nd set of addresses are just a
	mirror that allows access to a different set of source roms; the tables etc. are
	re-uploaded before every transfer.

    Dead of Alive has the source data in RAM, not ROM.
    This is similar to how some 5881 games were set up, with the ST-V versions decrypting
    data directly from ROM and the Model 2 ones using a RAM source buffer.

    Looking at the values read I don't think there is any address based encryption, for
    example many blocks where you'd expect a zero fill start with repeating patterns
    of 8f708f70 (different lengths) channel would appear to relate to compressed 0x00 data

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
	: device_t(mconfig, SEGA315_5838_COMP, "Sega 315-5838 / 317-0029 Compression (Encryption?)", tag, owner, clock, "sega315_5838", __FILE__)
{
}



void sega_315_5838_comp_device::device_start()
{
	m_decathlt_lastcount = 0;
	m_decathlt_prot_uploadmode = 0;
	m_decathlt_prot_uploadoffset = 0;

	for (auto & elem : m_channel)
	{
		elem.m_read_ch.bind_relative_to(*owner());

	}
}

void sega_315_5838_comp_device::device_reset()
{
	m_srcoffset = 0;
	m_decathlt_lastcount = 0;
	m_decathlt_prot_uploadmode = 0;
	m_decathlt_prot_uploadoffset = 0;
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


READ32_MEMBER(sega_315_5838_comp_device::decathlt_prot1_r)
{
	return genericdecathlt_prot_r(mem_mask, 0);
}

READ32_MEMBER(sega_315_5838_comp_device::decathlt_prot2_r)
{
	return genericdecathlt_prot_r(mem_mask, 1);
}


UINT32 sega_315_5838_comp_device::genericdecathlt_prot_r(UINT32 mem_mask, int channel)
{
//  UINT32 *fake0 = (UINT32*)memregion( ":fake0" )->base();
//  UINT32 retvalue = 0xffff;

	switch (m_srcoffset)
	{
		default:

		m_decathlt_lastcount++;

		UINT32 tempdata = 0;
		tempdata |= m_channel[channel].m_read_ch(m_srcoffset) << 0;
		m_srcoffset++;
		tempdata |= m_channel[channel].m_read_ch(m_srcoffset) << 16;
		m_srcoffset++;


		#ifdef DEBUG_DATA_DUMP
		//printf("read addr %08x, blah_r %08x - read count count %08x\n", m_srcoffset*2, tempdata,  m_decathlt_lastcount*4);
		fwrite(&tempdata, 1, 4, tempfile);
		#else
		logerror("read addr %08x, blah_r %08x - read count count %08x\n", m_srcoffset*2, tempdata,  m_decathlt_lastcount*4);
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

	return 0xffffffff;
}

void sega_315_5838_comp_device::set_prot_addr(UINT32 data, UINT32 mem_mask, int channel)
{
//  printf("set_prot_addr\n");
	COMBINE_DATA(&m_srcoffset);

	//if (m_decathlt_part==0) logerror("%d, last read count was %06x\n",channel, m_decathlt_lastcount*4);
	m_decathlt_lastcount = 0;

	if (mem_mask == 0x0000ffff)
	{
		printf("set source address to %08x (channel %d)\n", m_srcoffset, channel);
	}


#ifdef DEBUG_DATA_DUMP
	if (mem_mask == 0x0000ffff)
	{
		if (tempfile)
			fclose(tempfile);

		char filename[256];
		sprintf(filename, "%d_compressed_%08x", channel, m_srcoffset * 2);
		tempfile = fopen(filename, "w+b");

		// the table and dictionary are uploaded repeatedly, usually before groups of data transfers but
		// it's always the same tables (one pair for each channel)
		{
			FILE* fp;
			sprintf(filename, "%d_compressed_table1_%08x", channel, m_srcoffset * 2);
			fp = fopen(filename, "w+b");
			fwrite(&m_decathlt_prottable1, 24, 2, fp);
			fclose(fp);
		}

		{
			FILE* fp;
			sprintf(filename, "%d_compressed_dictionary_%08x", channel, m_srcoffset * 2);
			fp = fopen(filename, "w+b");
			fwrite(&m_decathlt_dictionaryy, 128, 2, fp);
			fclose(fp);
		}
	}
#endif

}

void sega_315_5838_comp_device::set_upload_mode(UINT16 data, int channel)
{
	if ((data == 0x8000) || (data == 0x0000))
	{
	//  logerror("changed to upload mode 1\n");
		m_decathlt_prot_uploadmode = 1;
		m_decathlt_prot_uploadoffset = 0;
	}
	else if ((data == 0x8080) || (data == 0x0080))
	{
		m_decathlt_prot_uploadmode = 2;
		m_decathlt_prot_uploadoffset = 0;
	}
	else
	{
		fatalerror("unknown upload mode\n");
	}
}

void sega_315_5838_comp_device::upload_table_data(UINT16 data, int channel)
{
	if (m_decathlt_prot_uploadmode == 1)
	{
		if (m_decathlt_prot_uploadoffset >= 24)
		{
			fatalerror("upload mode 1 error, too big\n");
			return;
		}

		//logerror("uploading table 1 %04x %04x\n",m_decathlt_prot_uploadoffset, data&0xffff);
		m_decathlt_prottable1[m_decathlt_prot_uploadoffset] = data & 0xffff;
		m_decathlt_prot_uploadoffset++;
		printf("unk table 1 %04x (channel %d)\n", data & 0xffff, channel);
	}
	else if (m_decathlt_prot_uploadmode == 2)
	{
		if (m_decathlt_prot_uploadoffset >= 128)
		{
			fatalerror("upload mode 2 error, too big\n");
			return;
		}

		//logerror("uploading table 2 %04x %04x\n",m_decathlt_prot_uploadoffset, data&0xffff);
		m_decathlt_dictionaryy[m_decathlt_prot_uploadoffset] = data & 0xffff;
		m_decathlt_prot_uploadoffset++;
		printf("dictionary %04x (channel %d)\n", data & 0xffff, channel);
	}
}

void sega_315_5838_comp_device::write_prot_data(UINT32 data, UINT32 mem_mask, int channel, int rev_words)
{
	if (mem_mask==0xffff0000)
	{
		if (rev_words==0) set_upload_mode(data >> 16, channel);
		else upload_table_data(data >>16, channel);
	}
	else if (mem_mask == 0x0000ffff)
	{
		if (rev_words==0) upload_table_data(data & 0xffff, channel);
		else set_upload_mode(data & 0xffff, channel);
	}
	else
	{
		fatalerror("write_prot_data invalid mem_mask\b");
	}
}



WRITE32_MEMBER( sega_315_5838_comp_device::decathlt_prot1_w_doa )  { write_prot_data(data, mem_mask, 0, 1); }
WRITE32_MEMBER( sega_315_5838_comp_device::decathlt_prot1_w)  { write_prot_data(data, mem_mask, 0, 0); }
WRITE32_MEMBER( sega_315_5838_comp_device::decathlt_prot2_w)  { write_prot_data(data, mem_mask, 1, 0); }

WRITE32_MEMBER( sega_315_5838_comp_device::decathlt_prot1_srcaddr_w ) { set_prot_addr(data, mem_mask, 0); }
WRITE32_MEMBER( sega_315_5838_comp_device::decathlt_prot2_srcaddr_w)  { set_prot_addr(data, mem_mask, 1); }


void sega_315_5838_comp_device::install_decathlt_protection()
{
	//todo, install these in the driver, they differ between games
	cpu_device* cpu = (cpu_device*)machine().device(":maincpu");

	cpu->space(AS_PROGRAM).install_write_handler(0x37FFFF0, 0x37FFFF3, write32_delegate(FUNC(sega_315_5838_comp_device::decathlt_prot1_srcaddr_w), this)); // set compressed data source address
	cpu->space(AS_PROGRAM).install_write_handler(0x37FFFF4, 0x37FFFF7, write32_delegate(FUNC(sega_315_5838_comp_device::decathlt_prot1_w), this)); // upload tables
	cpu->space(AS_PROGRAM).install_read_handler(0x37FFFF8, 0x37FFFFb, read32_delegate(FUNC(sega_315_5838_comp_device::decathlt_prot1_r), this)); // read decompressed data

	// the device is addressed here too, uploading a different set of tables and accessing a different part of ROM
	cpu->space(AS_PROGRAM).install_write_handler(0x27FFFF0, 0x27FFFF3, write32_delegate(FUNC(sega_315_5838_comp_device::decathlt_prot2_srcaddr_w), this)); // set compressed data source address
	cpu->space(AS_PROGRAM).install_write_handler(0x27FFFF4, 0x27FFFF7, write32_delegate(FUNC(sega_315_5838_comp_device::decathlt_prot2_w), this)); // upload tables
	cpu->space(AS_PROGRAM).install_read_handler(0x27FFFF8, 0x27FFFFb, read32_delegate(FUNC(sega_315_5838_comp_device::decathlt_prot2_r), this)); // read decompressed data
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

	m_protstate = 0;
}


void sega_315_5838_comp_device::install_doa_protection()
{
	//todo, install these in the driver, they differ between games
	cpu_device* cpu = (cpu_device*)machine().device(":maincpu");

	m_protstate = 0;
	strcpy((char *)m_protram, "  TECMO LTD.  DEAD OR ALIVE  1996.10.22  VER. 1.00"); // this is the single decompressed string DOA needs, note, 2 spaces at start, might indicate a dummy read like with 5881 on Model 2

	cpu->space(AS_PROGRAM).install_readwrite_handler(0x01d80000, 0x01dfffff, read32_delegate(FUNC(sega_315_5838_comp_device::doa_prot_r), this), write32_delegate(FUNC(sega_315_5838_comp_device::doa_prot_w), this));
	cpu->space(AS_PROGRAM).install_write_handler(0x01d87ff0, 0x01d87ff3, write32_delegate(FUNC(sega_315_5838_comp_device::decathlt_prot1_srcaddr_w), this)); // set compressed data source address (always set 0, data is in RAM)
	cpu->space(AS_PROGRAM).install_write_handler(0x01d87ff4, 0x01d87ff7, write32_delegate(FUNC(sega_315_5838_comp_device::decathlt_prot1_w_doa), this)); // upload tab
//  cpu->space(AS_PROGRAM).install_read_handler(0x01d87ff8, 0x01d87ffb, read32_delegate(FUNC(sega_315_5838_comp_device::decathlt_prot1_r), this)); // read decompressed data

}
