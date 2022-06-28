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
#include "315-5838_317-0229_comp.h"

DEFINE_DEVICE_TYPE(SEGA315_5838_COMP, sega_315_5838_comp_device, "sega315_5838", "Sega 315-5838 / 317-0229 Compression and Encryption")

sega_315_5838_comp_device::sega_315_5838_comp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SEGA315_5838_COMP, tag, owner, clock),
	device_rom_interface(mconfig, *this),
	m_hackmode(0)
{
}

void sega_315_5838_comp_device::device_start()
{
}

void sega_315_5838_comp_device::device_reset()
{
	m_srcoffset = 0;
	m_srcstart = 0;
	m_abort = false;
}

void sega_315_5838_comp_device::rom_bank_updated()
{
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
	if (m_hackmode == HACK_MODE_NONE)
	{
		// real algorithm, when we have a cipher function
		for (;;)
		{
			if (m_abort)
			{
				return 0xff;
			}

			if (m_num_bits_compressed == 0)
			{
				m_val_compressed = decipher(source_word_r());
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
	else
	{
		// modes where we don't have the real cipher yet, to aid with data logging etc.
		// this code will go away eventually
		uint8_t ret = 0;

		// scrreader is words, this is bytes, for unknown compression we want to log the same number of bytes as we read, so log every other access
		if (!(m_srcoffset & 1))
		{
			uint16_t temp = read_word((m_srcoffset*2)^2);
			logerror("%s: read data %04x\n", machine().describe_context(), temp);
#ifdef SEGA315_DUMP_DEBUG
			if (m_fp)
			{
				fwrite(&temp, 1, 2, m_fp);
			}
#endif
		}

		if (m_hackmode == HACK_MODE_DOA)
		{
			// this is the single decompressed string DOA needs, note, 2 spaces at start, might indicate a dummy read like with 5881 on Model 2
			// it reads 50 bytes from the device.  PC = 2C20 is a pass, PC = 2C28 is a fail
			const uint8_t prot[51] = "  TECMO LTD.  DEAD OR ALIVE  1996.10.22  VER. 1.00";
			if (m_srcoffset<50) ret = prot[m_srcoffset];
			else ret = 0x00;
			logerror("%s: doa read %08x %c\n", machine().describe_context(), m_srcoffset, ret);
		}
		else if (m_hackmode == HACK_MODE_NO_KEY)
		{
			ret = machine().rand();
		}

		m_srcoffset++;
		m_srcoffset &= 0x007fffff;
		return ret;
	}
}

uint16_t sega_315_5838_comp_device::data_r()
{
	return (get_decompressed_byte() << 8) | (get_decompressed_byte() << 0);
}

uint16_t sega_315_5838_comp_device::source_word_r()
{
	uint16_t tempdata = read_word((m_srcoffset*2)^2);
	m_srcoffset++;
	m_srcoffset &= 0x007fffff;

	if (m_srcoffset == m_srcstart) // if we've wrapped around to where we started something has gone wrong with the transfer, abandon
		m_abort = true;

#ifdef SEGA315_DUMP_DEBUG
	if (m_fp)
	{
		fwrite(&tempdata, 1, 2, m_fp);
	}
#endif

	return tempdata;
}

void sega_315_5838_comp_device::set_prot_addr(uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_srcoffset);
	m_srcoffset &= 0x007fffff;
	m_srcstart = m_srcoffset;
	m_abort = false;

	m_num_bits_compressed = 0;
	m_val_compressed = 0;
	m_num_bits = 0;
	m_val = 0;
}

void sega_315_5838_comp_device::debug_helper(int id)
{
#ifdef SEGA315_DUMP_DEBUG

	if (m_fp)
	{
		fclose(m_fp);
	}

	if (1)
	{
		char filename[256];
		sprintf(filename, "%d_%08x_table_tree_len", id, m_srcoffset * 2);
		m_fp = fopen(filename, "w+b");
		for (int i = 0; i < 12; i++)
		{
			fwrite(&m_compstate.tree[i].len, 1, 1, m_fp);
		}
		fclose(m_fp);
	}

	if (1)
	{
		char filename[256];
		sprintf(filename, "%d_%08x_table_tree_idx", id, m_srcoffset * 2);
		m_fp = fopen(filename, "w+b");
		for (int i = 0; i < 12; i++)
		{
			fwrite(&m_compstate.tree[i].idx, 1, 1, m_fp);
		}
		fclose(m_fp);
	}

	if (1)
	{
		char filename[256];
		sprintf(filename, "%d_%08x_table_tree_pattern", id, m_srcoffset * 2);
		m_fp = fopen(filename, "w+b");
		for (int i = 0; i < 12; i++)
		{
			fwrite(&m_compstate.tree[i].pattern, 1, 2, m_fp);
		}
		fclose(m_fp);
	}

	if (1)
	{
		char filename[256];
		sprintf(filename, "%d_%08x_table_dictionary", id, m_srcoffset * 2);
		m_fp = fopen(filename, "w+b");
		for (int i = 0; i < 256; i++)
		{
			fwrite(&m_compstate.dictionary[i], 1, 1, m_fp);
		}
		fclose(m_fp);
	}

	if (1)
	{
		char filename[256];
		sprintf(filename, "%d_%08x_table_data", id, m_srcoffset * 2);
		m_fp = fopen(filename, "w+b");
		// leave open for writing

	}

#endif
}


void sega_315_5838_comp_device::set_table_upload_mode_w(uint16_t val)
{
	m_compstate.mode = val;

	if (!(m_compstate.mode & 0x80)) // 0x8000 and 0x0000
	{
		m_compstate.it2 = 0;
	}
	else // 0x8080 and 0x0080
	{
		m_compstate.id = 0;
	}
}

void sega_315_5838_comp_device::upload_table_data_w(uint16_t val)
{
	if (!(m_compstate.mode & 0x80)) // 0x8000 and 0x0000
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
	else // 0x8080 and 0x0080
	{
		assert(m_compstate.id < 255);

		m_compstate.dictionary[m_compstate.id++] = (0xFF00 & val) >> 8;
		m_compstate.dictionary[m_compstate.id++] = (0x00FF & val) >> 0;
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

void sega_315_5838_comp_device::data_w_doa(offs_t offset, uint32_t data, uint32_t mem_mask) { write_prot_data(data, mem_mask, 1); }
void sega_315_5838_comp_device::data_w(offs_t offset, uint32_t data, uint32_t mem_mask) { write_prot_data(data, mem_mask, 0); }
void sega_315_5838_comp_device::srcaddr_w(offs_t offset, uint32_t data, uint32_t mem_mask) { set_prot_addr(data, mem_mask); }

