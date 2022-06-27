// license:BSD-3-Clause
// copyright-holders:David Haywood, James Wallace

/*
Characteriser (CHR)

As built, the CHR is a PAL which can perform basic bit manipulation according to
an as yet unknown unique key. However, the programmers decided to best use this protection device in read/write/compare
cycles, storing almost the entire 'hidden' data table in the ROMs in plain sight. Only later rebuilds by BwB
avoided this 'feature' of the development kit, and will need a different setup.

This information has been used to generate the CHR tables loaded by the programs, until a key can be determined.

For most Barcrest games, the following method was used:

The initial 'PALTEST' routine as found in the Barcrest programs simply writes the first 'call' to the CHR space,
to read back the 'response'. There is no attempt to alter the order or anything else, just
a simple runthrough of the entire data table. The only 'catch' in this is to note that the CHR chip always scans
through the table starting at the last accessed data value, unless 00 is used to reset to the beginning. This is obviously
a simplification, in fact the PAL does bit manipulation with some latching.

However, a final 8 byte row, that controls the lamp matrix is not tested - to date, no-one outside of Barcrest knows
how this is generated, and currently trial and error is the only sensible method. It is noted that the default,
of all 00, is sometimes the correct answer, particularly in non-Barcrest use of the CHR chip, though when used normally,
there are again fixed call values.

Apparently, just before the characteriser is checked bit 1 at 0x61DF is checked and if zero the characteriser
check is bypassed. This may be something to look at for prototype ROMs and hacks.

*/

#include "emu.h"

#include "mpu4_characteriser_pal.h"

DEFINE_DEVICE_TYPE(MPU4_CHARACTERISER_PAL, mpu4_characteriser_pal, "mpu4chrpal", "Barcrest MPU4 Characteriser PAL")

mpu4_characteriser_pal::mpu4_characteriser_pal(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mpu4_characteriser_pal(mconfig, MPU4_CHARACTERISER_PAL, tag, owner, clock)
{
}

mpu4_characteriser_pal::mpu4_characteriser_pal(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_current_chr_table(nullptr),
	m_prot_col(0),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_allow_6809_cheat(false),
	m_allow_68k_cheat(false),
	m_current_lamp_table(nullptr),
	m_lamp_col(0),
	m_4krow(0),
	m_is_4ksim(false),
	m_protregion(*this, "fakechr")
{
}

void mpu4_characteriser_pal::device_start()
{
	for (int i=0;i<64;i++)
		m_temp_debug_table[i] = 0x00;

	m_temp_debug_write_count = 0;
}

void mpu4_characteriser_pal::device_reset()
{
}

// this is the challenge table used for all official Barcrest games
// NOTE: some values are repeated, response search continues from last found value
static uint8_t challenge_table[64] = {  0x00, 0x1A, 0x04, 0x10, 0x18, 0x0F, 0x13, 0x1B, 0x03, 0x07, 0x17, 0x1D, 0x36, 0x35, 0x2B, 0x28,
										0x39, 0x21, 0x22, 0x25, 0x2C, 0x29, 0x31, 0x34, 0x0A, 0x1F, 0x06, 0x0E, 0x1C, 0x12, 0x1E, 0x0D,
										0x14, 0x0A, 0x19, 0x15, 0x06, 0x0F, 0x08, 0x1B, 0x1E, 0x04, 0x01, 0x0C, 0x18, 0x1A, 0x11, 0x0B,
										0x03, 0x17, 0x10, 0x1D, 0x0E, 0x07, 0x12, 0x09, 0x0D, 0x1F, 0x16, 0x05, 0x13, 0x1C, 0x02, 0x00 };

// some earlier games use this challenge table, where bit 0x20 is never set..
// is there a difference in behavior?
#if 0
static uint8_t challenge_table2[64] = { 0x00, 0x1A, 0x04, 0x10, 0x18, 0x0F, 0x13, 0x1B, 0x03, 0x07, 0x17, 0x1D, 0x16, 0x15, 0x0B, 0x08,
										0x19, 0x01, 0x02, 0x05, 0x0C, 0x09, 0x11, 0x14, 0x0A, 0x1F, 0x06, 0x0E, 0x1C, 0x12, 0x1E, 0x0D,
										0x14, 0x0A, 0x19, 0x15, 0x06, 0x0F, 0x08, 0x1B, 0x1E, 0x04, 0x01, 0x0C, 0x18, 0x1A, 0x11, 0x0B,
										0x03, 0x17, 0x10, 0x1D, 0x0E, 0x07, 0x12, 0x09, 0x0D, 0x1F, 0x16, 0x05, 0x13, 0x1C, 0x02, 0x00, };
#endif

void mpu4_characteriser_pal::protection_w(uint8_t data)
{
	logerror("%s Characteriser protection_w data %02x\n", machine().describe_context(), data);

	if (data == 0)
	{
		m_prot_col = 0;
		m_4krow = 0;
	}
	else
	{
		if (m_is_4ksim)
		{
			m_prot_col = data & 0x3f; // 6-bit writes (upper 2 bits unused)
		}
		else
		{
			for (int x = m_prot_col; x < 64; x++)
			{
				uint8_t call = challenge_table[x];

				// ignore bit 0x20 and above (see note above about alt challenge table)
				// note, the 4k tables used by the quiz games suggest this bit DOES change the sequence
				// at least on some of the PALs, maybe later ones had an extra input line?
				if ((call & 0x1f) == (data & 0x1f))
				{
					m_prot_col = x;
					logerror("Characteriser find column %02x\n", m_prot_col);
					break;
				}
			}
		}
	}
}




void mpu4_characteriser_pal::lamp_scramble_w(uint8_t data)
{
	switch (data)
	{
	case 0x00:
		m_lamp_col = 0;
		break;

	case 0x01:
		m_lamp_col = 1;
		break;

	case 0x04:
		m_lamp_col = 2;
		break;

	case 0x09:
		m_lamp_col = 3;
		break;

	case 0x10:
		m_lamp_col = 4;
		break;

	case 0x19:
		m_lamp_col = 5;
		break;

	case 0x24:
		m_lamp_col = 6;
		break;

	case 0x31:
		m_lamp_col = 7;
		break;
	}

	logerror("%s Characteriser lamp_scramble_w data %02X (picking column %d)\n", machine().describe_context(), data, m_lamp_col);
}

void mpu4_characteriser_pal::write(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0x00:
		protection_w(data);
		break;

	case 0x02:
		lamp_scramble_w(data);
		break;
	}
}

uint8_t mpu4_characteriser_pal::protection_r()
{
	if (m_allow_6809_cheat || m_allow_68k_cheat)
	{
		uint8_t ret = 0x00;
		if (m_allow_6809_cheat)
		{
			/* a cheat ... many early games use a standard check */
			int addr = m_cpu->state_int(M6809_X);
			if ((addr >= 0x800) && (addr <= 0xfff)) return 0x00; // prevent recursion, only care about ram/rom areas for this cheat.

			ret = m_cpu->space(AS_PROGRAM).read_byte(addr);
			logerror("%s: Characteriser protection_r WITH 6809 CHEAT (col is %02x returning %02x from addr %04x)\n", machine().describe_context(), m_prot_col, ret, addr);
		}
		else if (m_allow_68k_cheat)
		{
			ret = m_cpu->state_int(M68K_D0) & 0xff;
			logerror("%s: Characteriser protection_r WITH 68000 CHEAT (col is %02x returning %02x)\n", machine().describe_context(), m_prot_col, ret);
		}


		if (IDENTIFICATION_HELPER)
		{
			m_temp_debug_table[m_prot_col] = ret;

			if (m_temp_debug_write_count <= 64)
				m_temp_debug_write_count++;

			if (m_temp_debug_write_count == 64)
			{
				logerror("Characteriser Sequence:\n");
				for (int i = 0; i < 64; i++)
				{
					logerror("%02x ", m_temp_debug_table[i]);
				}
				logerror("\n");
			}
		}

		return ret;
	}

	// not cheating, but no table set
	if (!m_current_chr_table && !m_protregion)
	{
		logerror("%s: Characteriser protection_r WITH NO TABLE\n", machine().describe_context());
		return 0x00;
	}

	// use table
	uint8_t ret = 0x00;

	if (m_current_chr_table)
	{
		ret = m_current_chr_table[m_prot_col];
		logerror("%s: Characteriser protection_r WITH PASSED TABLE (returning %02x)\n", machine().describe_context(), ret);
	}
	else if (m_protregion)
	{
		ret = m_protregion[m_4krow * 64 + m_prot_col];

		// if we're simulating with a 4k table, the previous write selects
		// the next set of 64 values to use.
		if (m_is_4ksim)
		{
			m_4krow = ret;
			ret <<= 2;
		}

		logerror("%s: Characteriser protection_r WITH FAKE ROM (returning %02x)\n", machine().describe_context(), ret);
	}

	return ret;
}

uint8_t mpu4_characteriser_pal::lamp_scramble_r()
{
	if (!m_current_lamp_table && !m_protregion)
	{
		uint8_t ret = machine().rand();
		logerror("%s: Characteriser lamp_scramble_r WITH NO TABLE (table offset %02x, returning %02x)\n", machine().describe_context(), m_lamp_col, ret);
		return ret;
	}
	else
	{
		uint8_t ret = 0x00;

		if (m_current_lamp_table)
		{
			ret = m_current_lamp_table[m_lamp_col];
			logerror("%s: Characteriser lamp_scramble_r WITH PASSED TABLE (table offset %02x, returning %02x)\n", machine().describe_context(), m_lamp_col, ret);
		}
		else if (m_protregion)
		{
			ret = m_protregion[m_lamp_col + 64];
			logerror("%s: Characteriser lamp_scramble_r WITH FAKE ROM (table offset %02x, returning %02x)\n", machine().describe_context(), m_lamp_col, ret);
		}

		return ret;
	}
}

uint8_t mpu4_characteriser_pal::read(offs_t offset)
{
	switch (offset)
	{
	case 0x00: return protection_r();
	case 0x03: return lamp_scramble_r();
	}
	return 0;
}
