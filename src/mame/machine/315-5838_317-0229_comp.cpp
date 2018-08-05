// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Sega Compression (and possibly encryption) device

    315-5838 - Decathlete (ST-V)
    317-0229 - Dead or Alive (Model 2A)
    317-0229 - Name Club / Name Club Ver 2 (ST-V) (tested as RCDD2 in the service menu!)
    317-0230 - Winnie The Pooh Vol 2 / 3
    317-0231 - Print Club Love Love / Print Club Love Love Ver 2 (ST-V)

    Several Print Club (ST-V) carts have
    an unpopulated space marked '317-0229' on the PCB

    Package Type: TQFP100

    Decathlete accesses the chip at 2 different addresses.

    Dead of Alive has the source data in RAM, not ROM.
    This is similar to how some 5881 games were set up, with the ST-V versions decrypting
    data directly from ROM and the Model 2 ones using a RAM source buffer.

    Decathlete decompresses all graphic data with the chip.

    The Name Club games use the chip for decompressing data for the printer (full size
    versions of the graphics?)

    Print Club Love Love decrypts some start up code/data required for booting.

    Dead or Alive decrypts a string that is checked on startup, nothing else.

*/

#include "emu.h"
#include "machine/315-5838_317-0229_comp.h"

DEFINE_DEVICE_TYPE(SEGA315_5838_COMP, sega_315_5838_comp_device, "sega315_5838", "Sega 315-5838 / 317-0229 Compression and Encryption")

sega_315_5838_comp_device::sega_315_5838_comp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SEGA315_5838_COMP, tag, owner, clock)
{
}



void sega_315_5838_comp_device::device_start()
{
	m_decathlt_lastcount = 0;
	m_decathlt_prot_uploadmode = 0;
	m_decathlt_prot_uploadoffset = 0;

	m_read_ch.bind_relative_to(*owner());
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

READ32_MEMBER(sega_315_5838_comp_device::data_r)
{
	return genericdecathlt_prot_r(mem_mask);
}


uint32_t sega_315_5838_comp_device::genericdecathlt_prot_r(uint32_t mem_mask)
{
	switch (m_srcoffset)
	{
		default:

		m_decathlt_lastcount++;

		uint32_t tempdata = 0;
		tempdata |= m_read_ch(m_srcoffset) << 0;
		m_srcoffset++;
		tempdata |= m_read_ch(m_srcoffset) << 16;
		m_srcoffset++;

		printf("read addr %08x, blah_r %08x - read count count %08x\n", m_srcoffset*2, tempdata,  m_decathlt_lastcount*4);

		return tempdata;
	}

	return 0xffffffff;
}

void sega_315_5838_comp_device::set_prot_addr(uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_srcoffset);

	m_decathlt_lastcount = 0;

	if (mem_mask == 0x0000ffff)
	{
		printf("set source address to %08x\n", m_srcoffset);
	}
}

void sega_315_5838_comp_device::set_upload_mode(uint16_t data)
{
	if ((data == 0x8000) || (data == 0x0000))
	{
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

void sega_315_5838_comp_device::upload_table_data(uint16_t data)
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
		printf("unk table 1 %04x\n", data & 0xffff);
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
		printf("dictionary %04x\n", data & 0xffff);
	}
}

void sega_315_5838_comp_device::write_prot_data(uint32_t data, uint32_t mem_mask, int rev_words)
{
	if (mem_mask==0xffff0000)
	{
		if (rev_words==0) set_upload_mode(data >> 16);
		else upload_table_data(data >>16);
	}
	else if (mem_mask == 0x0000ffff)
	{
		if (rev_words==0) upload_table_data(data & 0xffff);
		else set_upload_mode(data & 0xffff);
	}
	else
	{
		fatalerror("write_prot_data invalid mem_mask\b");
	}
}

WRITE32_MEMBER( sega_315_5838_comp_device::data_w_doa )  { write_prot_data(data, mem_mask, 1); }
WRITE32_MEMBER( sega_315_5838_comp_device::data_w)  { write_prot_data(data, mem_mask, 0); }
WRITE32_MEMBER( sega_315_5838_comp_device::srcaddr_w ) { set_prot_addr(data, mem_mask); }


READ32_MEMBER(sega_315_5838_comp_device::data_r_doa)
{
	uint32_t retval = 0;

	if (offset == 0x7ff8/4)
	{
		// PC=2c20
		retval = m_protram[m_protstate+1] | m_protram[m_protstate]<<8;
		m_protstate+=2;
		printf("data_r_doaead %08x %08x %08x\n", offset*4, retval, mem_mask);
	}
	else if (offset == 0x400c/4) // todo, is this actually part of the protection? it's in the address range, but decathlete doesn't have it afaik.
	{
		// this actually looks a busy status flag
		m_prot_a = !m_prot_a;
		if (m_prot_a)
			retval = 0xffff;
		else
			retval = 0xfff0;
	}
	else
	{
		printf("data_r_doaead %08x %08x %08x\n", offset*4, retval, mem_mask);
		logerror("Unhandled Protection READ @ %x mask %x %s\n", offset, mem_mask, machine().describe_context());
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

	cpu->space(AS_PROGRAM).install_readwrite_handler(0x01d80000, 0x01dfffff, read32_delegate(FUNC(sega_315_5838_comp_device::data_r_doa), this), write32_delegate(FUNC(sega_315_5838_comp_device::doa_prot_w), this));
	cpu->space(AS_PROGRAM).install_write_handler(0x01d87ff0, 0x01d87ff3, write32_delegate(FUNC(sega_315_5838_comp_device::srcaddr_w), this)); // set compressed data source address (always set 0, data is in RAM)
	cpu->space(AS_PROGRAM).install_write_handler(0x01d87ff4, 0x01d87ff7, write32_delegate(FUNC(sega_315_5838_comp_device::data_w_doa), this)); // upload tab
//  cpu->space(AS_PROGRAM).install_read_handler(0x01d87ff8, 0x01d87ffb, read32_delegate(FUNC(sega_315_5838_comp_device::data_r), this)); // read decompressed data

}
