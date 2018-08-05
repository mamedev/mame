// license:BSD-3-Clause
// copyright-holders:David Haywood, Samuel Neves, Peter Wilhelmsen, Morten Shearman Kirkegaard

/* Sega Compression (and encryption) device

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
	m_read_ch.bind_relative_to(*owner());
}

void sega_315_5838_comp_device::device_reset()
{
	m_srcoffset = 0;
	m_protstate = 0;
}


/**************************
*
* Decathlete
*
**************************/

// this part is likely specific to the decathlete type chip and will differ for the others
uint16_t sega_315_5838_comp_device::decipher(uint16_t c)
{
	// TODO: use BIT macros instead of working in bytes
	uint16_t p = 0;
	uint16_t x[16];

	for (int b = 0; b < 16; ++b)
	{
		x[b] = (c >> b) & 1;
	}

	p |= (x[7] ^ x[9] ^ x[14] ? 0 : x[5] ^ x[12]) ^ x[14];
	p |= (((x[7] ^ x[9])&(x[12] ^ x[14] ^ x[5])) ^ x[14] ^ 1) << 1;
	p |= ((x[6] & x[8]) ^ (x[6] & x[15]) ^ (x[8] & x[15]) ^ 1) << 2;
	p |= (x[11] ^ x[14] ^ 1) << 3;
	p |= ((x[7] & (x[1] ^ x[8] ^ x[12])) ^ x[12]) << 4;
	p |= ((x[6] | x[8]) ^ (x[8] & x[15])) << 5;
	p |= (x[4] ^ (x[3] | x[10])) << 6;
	p |= ((x[14] & (x[5] ^ x[12])) ^ x[7] ^ x[9] ^ 1) << 7;
	p |= (x[4] ^ x[13] ^ 1) << 8;
	p |= (x[6] ^ (x[8] | (x[15] ^ 1))) << 9;
	p |= (x[7] ^ (x[12] | (x[1] ^ x[8] ^ x[7] ^ 1))) << 10;
	p |= (x[3] ^ x[10] ^ 1) << 11;
	p |= (x[0] ^ x[2]) << 12;
	p |= (x[8] ^ x[1] ? x[12] : x[7]) << 13;
	p |= (x[0] ^ x[11] ^ x[14] ^ 1) << 14;
	p |= (x[10] ^ 1) << 15;

	return p;
}


uint8_t sega_315_5838_comp_device::get_decompressed_byte(void)
{
	for (;;)
	{
		if (m_num_bits_compressed == 0)
		{
			m_val_compressed = decipher(genericdecathlt_prot_r());
			m_num_bits_compressed = 16;
		}

		m_num_bits_compressed--;
		m_val <<= 1;
		m_val |= 1 & (m_val_compressed >> m_num_bits_compressed);
		m_num_bits++;

		for (int i = 0; i < 12; i++)
		{
			if (m_num_bits != m_compstate.tree[i].len) continue;
			if (m_val < (m_compstate.tree[i].pattern >> (12 - m_num_bits))) continue;
			if (
				(m_num_bits < 12) &&
				(m_val >= (m_compstate.tree[i + 1].pattern >> (12 - m_num_bits)))
				) continue;

			int j = m_compstate.tree[i].idx + m_val - (m_compstate.tree[i].pattern >> (12 - m_num_bits));

			m_val = 0;
			m_num_bits = 0;

			return m_compstate.dictionary[j];
		}
	}
}

READ32_MEMBER(sega_315_5838_comp_device::data_r)
{
	return (get_decompressed_byte() << 24) | (get_decompressed_byte() << 16) | (get_decompressed_byte() << 8) | (get_decompressed_byte() << 0);
}

uint16_t sega_315_5838_comp_device::genericdecathlt_prot_r()
{
	uint16_t tempdata = m_read_ch(m_srcoffset);
	m_srcoffset++;
	return tempdata;
}

void sega_315_5838_comp_device::set_prot_addr(uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_srcoffset);

	m_num_bits_compressed = 0;
	m_val_compressed = 0;
	m_num_bits = 0;
	m_val = 0;
}

void sega_315_5838_comp_device::set_table_upload_mode_w(uint16_t val)
{
	m_compstate.mode = val;

	if (m_compstate.mode == 0x8000)
	{
		m_compstate.it2 = 0;
	}
	else if (m_compstate.mode == 0x8080)
	{
		m_compstate.id = 0;
	}
	else
	{
		fatalerror("Unknown mode in set_table_upload_mode_w()");
	}
}


void sega_315_5838_comp_device::upload_table_data_w(uint16_t val)
{
	if (m_compstate.mode == 0x8000)
	{
		assert(m_compstate.it2 / 2 < 12);

		if ((m_compstate.it2 & 1) == 0)
		{
			m_compstate.tree[m_compstate.it2 / 2].len = (0xFF00 & val) >> 8;
			m_compstate.tree[m_compstate.it2 / 2].idx = (0x00FF & val) >> 0;
		}
		else
		{
			m_compstate.tree[m_compstate.it2 / 2].pattern = val;
		}
		m_compstate.it2++;
	}
	else if (m_compstate.mode == 0x8080)
	{
		assert(m_compstate.id < 255);

		m_compstate.dictionary[m_compstate.id++] = (0xFF00 & val) >> 8;
		m_compstate.dictionary[m_compstate.id++] = (0x00FF & val) >> 0;
	}
	else
	{
		fatalerror("Unknown mode in upload_table_data_w()");
	}
}



void sega_315_5838_comp_device::write_prot_data(uint32_t data, uint32_t mem_mask, int rev_words)
{
	if (mem_mask==0xffff0000)
	{
		if (rev_words == 0)
		{
			set_table_upload_mode_w(data >> 16);
		}
		else
		{
			upload_table_data_w(data >> 16);
		}
	}
	else if (mem_mask == 0x0000ffff)
	{
		if (rev_words == 0)
		{
			upload_table_data_w(data & 0xffff);
		}
		else
		{
			set_table_upload_mode_w(data & 0xffff);
		}
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
