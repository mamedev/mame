// license:BSD-3-Clause
// copyright-holders:David Haywood


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
DEFINE_DEVICE_TYPE(MPU4_CHARACTERISER_PAL_BWB, mpu4_characteriser_pal_bwb, "mpu4chrpalbwb", "Barcrest MPU4 Characteriser PAL (BWB type)")

DEFINE_DEVICE_TYPE(MPU4_CHARACTERISER_BOOTLEG_PAL45, mpu4_characteriser_bootleg45, "mpu4chrpalboot45", "Barcrest MPU4 Characteriser PAL (bootleg type 1)")
DEFINE_DEVICE_TYPE(MPU4_CHARACTERISER_BOOTLEG_PAL51, mpu4_characteriser_bootleg51, "mpu4chrpalboot51", "Barcrest MPU4 Characteriser PAL (bootleg type 2)")


mpu4_characteriser_pal::mpu4_characteriser_pal(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mpu4_characteriser_pal(mconfig, MPU4_CHARACTERISER_PAL, tag, owner, clock)
{
}



mpu4_characteriser_pal::mpu4_characteriser_pal(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_current_chr_table(nullptr),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_allow_6809_cheat(false),
	m_allow_68k_cheat(false),
	m_current_lamp_table(nullptr),
	m_prot_col(0),
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

#if 0

static mpu4_chr_table andycp10c_data[72] = {
{0x00, 0x00},{0x1a, 0x14},{0x04, 0x04},{0x10, 0x54},{0x18, 0x4c},{0x0f, 0x20},{0x13, 0x50},{0x1b, 0x44},
{0x03, 0x5c},{0x07, 0x78},{0x17, 0x70},{0x1d, 0x48},{0x36, 0x6c},{0x35, 0x60},{0x2b, 0x14},{0x28, 0x48},
{0x39, 0x2c},{0x21, 0x6c},{0x22, 0x6c},{0x25, 0x28},{0x2c, 0x64},{0x29, 0x10},{0x31, 0x08},{0x34, 0x6c},
{0x0a, 0x24},{0x1f, 0x5c},{0x06, 0x78},{0x0e, 0x34},{0x1c, 0x00},{0x12, 0x50},{0x1e, 0x00},{0x0d, 0x50},
{0x14, 0x0c},{0x0a, 0x6c},{0x19, 0x2c},{0x15, 0x60},{0x06, 0x54},{0x0f, 0x00},{0x08, 0x58},{0x1b, 0x74},
{0x1e, 0x00},{0x04, 0x14},{0x01, 0x4c},{0x0c, 0x60},{0x18, 0x1c},{0x1a, 0x74},{0x11, 0x4c},{0x0b, 0x64},
{0x03, 0x5c},{0x17, 0x78},{0x10, 0x78},{0x1d, 0x78},{0x0e, 0x34},{0x07, 0x44},{0x12, 0x54},{0x09, 0x40},
{0x0d, 0x50},{0x1f, 0x48},{0x16, 0x6c},{0x05, 0x28},{0x13, 0x60},{0x1c, 0x14},{0x02, 0x4c},{0x00, 0x00},
{0x00, 0x04},{0x01, 0x58},{0x04, 0x14},{0x09, 0x58},{0x10, 0x50},{0x19, 0x1c},{0x24, 0x10},{0x31, 0x10}
};

static mpu4_chr_table ccelbr_data[72] = {
{0x00, 0x00},{0x1a, 0x84},{0x04, 0x8c},{0x10, 0xb8},{0x18, 0x74},{0x0f, 0x80},{0x13, 0x1c},{0x1b, 0xb4},
{0x03, 0xd8},{0x07, 0x74},{0x17, 0x00},{0x1d, 0xd4},{0x36, 0xc8},{0x35, 0x78},{0x2b, 0xa4},{0x28, 0x4c},
{0x39, 0xe0},{0x21, 0xdc},{0x22, 0xf4},{0x25, 0x88},{0x2c, 0x78},{0x29, 0x24},{0x31, 0x84},{0x34, 0xcc},
{0x0a, 0xb8},{0x1f, 0x74},{0x06, 0x90},{0x0e, 0x48},{0x1c, 0xa0},{0x12, 0x1c},{0x1e, 0x24},{0x0d, 0x94},
{0x14, 0xc8},{0x0a, 0xb8},{0x19, 0x74},{0x15, 0x00},{0x06, 0x94},{0x0f, 0x48},{0x08, 0x30},{0x1b, 0x90},
{0x1e, 0x08},{0x04, 0x60},{0x01, 0xd4},{0x0c, 0x58},{0x18, 0xf4},{0x1a, 0x18},{0x11, 0x74},{0x0b, 0x80},
{0x03, 0xdc},{0x17, 0x74},{0x10, 0xd0},{0x1d, 0x58},{0x0e, 0x24},{0x07, 0x94},{0x12, 0xd8},{0x09, 0x34},
{0x0d, 0x90},{0x1f, 0x58},{0x16, 0xf4},{0x05, 0x88},{0x13, 0x38},{0x1c, 0x24},{0x02, 0xd4},{0x00, 0x00},
{0x00, 0x00},{0x01, 0x50},{0x04, 0x00},{0x09, 0x50},{0x10, 0x10},{0x19, 0x40},{0x24, 0x04},{0x31, 0x00}
};






static mpu4_chr_table grtecp_data[72] = {
{0x00, 0x00},{0x1a, 0x84},{0x04, 0xa4},{0x10, 0xac},{0x18, 0x70},{0x0f, 0x80},{0x13, 0x2c},{0x1b, 0xc0},
{0x03, 0xbc},{0x07, 0x5c},{0x17, 0x5c},{0x1d, 0x5c},{0x36, 0xdc},{0x35, 0x5c},{0x2b, 0xcc},{0x28, 0x68},
{0x39, 0xd0},{0x21, 0xb8},{0x22, 0xdc},{0x25, 0x54},{0x2c, 0x08},{0x29, 0x58},{0x31, 0x54},{0x34, 0x90},
{0x0a, 0xb8},{0x1f, 0x5c},{0x06, 0x5c},{0x0e, 0x44},{0x1c, 0x84},{0x12, 0xac},{0x1e, 0xe0},{0x0d, 0xbc},
{0x14, 0xcc},{0x0a, 0xe8},{0x19, 0x70},{0x15, 0x00},{0x06, 0x8c},{0x0f, 0x70},{0x08, 0x00},{0x1b, 0x84},
{0x1e, 0xa4},{0x04, 0xa4},{0x01, 0xbc},{0x0c, 0xdc},{0x18, 0x5c},{0x1a, 0xcc},{0x11, 0xe8},{0x0b, 0xe0},
{0x03, 0xbc},{0x17, 0x4c},{0x10, 0xc8},{0x1d, 0xf8},{0x0e, 0xd4},{0x07, 0xa8},{0x12, 0x68},{0x09, 0x40},
{0x0d, 0x0c},{0x1f, 0xd8},{0x16, 0xdc},{0x05, 0x54},{0x13, 0x98},{0x1c, 0x44},{0x02, 0x9c},{0x00, 0x00},
{0x00, 0x00},{0x01, 0x18},{0x04, 0x00},{0x09, 0x18},{0x10, 0x08},{0x19, 0x10},{0x24, 0x00},{0x31, 0x00}
};



static const uint8_t cybcas_data1[5] = {
//Magic num4ber 724A

// PAL Codes
// 0   1   2  3  4  5  6  7  8
// ??  ?? 20 0F 24 3C 36 27 09

	{0x67},{0x17},{0x0f},{0x24},{0x3c},
};

static mpu4_chr_table cybcas_data[8] = {
{0xEF, 0x02},{0x81, 0x00},{0xCE, 0x00},{0x00, 0x2e},
{0x06, 0x20},{0xC6, 0x0f},{0xF8, 0x24},{0x8E, 0x3c},
};

/* CHR Tables */


static const uint8_t blsbys_data1[5] = {
//Magic number 724A

// PAL Codes
// 0   1   2  3  4  5  6  7  8
// ??  ?? 20 0F 24 3C 36 27 09

	{0x67},{0x17},{0x0f},{0x24},{0x3c},
};

static mpu4_chr_table blsbys_data[8] = {
{0xEF, 0x02},{0x81, 0x00},{0xCE, 0x00},{0x00, 0x2e},
{0x06, 0x20},{0xC6, 0x0f},{0xF8, 0x24},{0x8E, 0x3c},
};

// set percentage and other options. 2e 20 0f
// PAL Codes
// 0   1   2  3  4  5  6  7  8
// 42  2E 20 0F 24 3C 36 27 09
	//      6  0  7  0  8  0  7  0  0  8
//request 36 42 27 42 09 42 27 42 42 09
//verify  00 04 04 0C 0C 1C 14 2C 5C 2C

#endif

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
				printf("Characteriser Sequence:\n");
				for (int i = 0; i < 64; i++)
				{
					printf("%02x ", m_temp_debug_table[i]);
				}
				printf("\n");
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



/*
BwB Characteriser (CHR)

The BwB method of protection is considerably different to the Barcrest one, with any
incorrect behaviour manifesting in ridiculously large payouts. The hardware is the
same, however the main weakness of the software has been eliminated.

In fact, the software seems deliberately designed to mislead, but is (fortunately for
us) prone to similar weaknesses that allow a per game solution.

Project Amber performed a source analysis (available on request) which appears to make things work.
Said weaknesses (A Cheats Guide according to Project Amber)

The common initialisation sequence is "00 04 04 0C 0C 1C 14 2C 5C 2C"
                                        0  1  2  3  4  5  6  7  8
Using debug search for the first read from said string (best to find it first).

At this point, the X index on the CPU is at the magic number address.

The subsequent calls for each can be found based on the magic address

           (0) = ( (BWBMagicAddress))
           (1) = ( (BWBMagicAddress + 1))
           (2) = ( (BWBMagicAddress + 2))
           (3) = ( (BWBMagicAddress + 4))
           (4) = ( (BWBMagicAddress - 5))
           (5) = ( (BWBMagicAddress - 4))
           (6) = ( (BWBMagicAddress - 3))
           (7) = ( (BWBMagicAddress - 2))
           (8) = ( (BWBMagicAddress - 1))

These return the standard init sequence as above.

For ease of understanding, we use three tables, one holding the common responses
and two holding the appropriate call and response pairs for the two stages of operation
*/


mpu4_characteriser_pal_bwb::mpu4_characteriser_pal_bwb(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mpu4_characteriser_pal(mconfig, MPU4_CHARACTERISER_PAL_BWB, tag, owner, clock)
{
}

void mpu4_characteriser_pal_bwb::write(offs_t offset, uint8_t data)
{
	int x;
	int call = data;
	logerror("%s Characteriser write offset %02x data %02x\n", machine().describe_context(), offset, data);
	if (!m_current_chr_table)
		fatalerror("%s No Characteriser Table\n", machine().describe_context().c_str());

	if ((offset & 0x3f) == 0)//initialisation is always at 0x800
	{
		if (!m_chr_state)
		{
			m_chr_state = 1;
			m_chr_counter = 0;
		}
		if (call == 0)
		{
			m_init_col++;
		}
		else
		{
			m_init_col = 0;
		}
	}

	m_chr_value = machine().rand();
	for (x = 0; x < 4; x++)
	{
		if (m_current_chr_table[(x)]/*.call*/ == call)
		{
			if (x == 0) // reinit
			{
				m_bwb_return = 0;
			}
			m_chr_value = bwb_chr_table_common[(m_bwb_return)];
			m_bwb_return++;
			break;
		}
	}
}

uint8_t mpu4_characteriser_pal_bwb::read(offs_t offset)
{
	logerror("Characteriser read offset %02x \n", offset);

	if (offset == 0)
	{
		switch (m_chr_counter)
		{
		case 6:
		case 13:
		case 20:
		case 27:
		case 34:
			return m_bwb_chr_table1[(((m_chr_counter + 1) / 7) - 1)];

		default:
			if (m_chr_counter > 34)
			{
				m_chr_counter = 35;
				m_chr_state = 2;
			}
			m_chr_counter++;
			return m_chr_value;
		}
	}
	else
	{
		return m_chr_value;
	}
}

mpu4_characteriser_bootleg45::mpu4_characteriser_bootleg45(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mpu4_characteriser_pal(mconfig, MPU4_CHARACTERISER_BOOTLEG_PAL45, tag, owner, clock)
{
}

uint8_t mpu4_characteriser_bootleg45::read(offs_t offset)
{
	logerror("%s: Characteriser read offset %02x\n", machine().describe_context(), offset);
	return 0x45;
}

void mpu4_characteriser_bootleg45::write(offs_t offset, uint8_t data)
{
	logerror("%s: Characteriser write offset %02x data %02x\n", machine().describe_context(), offset, data);
}


mpu4_characteriser_bootleg51::mpu4_characteriser_bootleg51(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mpu4_characteriser_pal(mconfig, MPU4_CHARACTERISER_BOOTLEG_PAL51, tag, owner, clock)
{
}

uint8_t mpu4_characteriser_bootleg51::read(offs_t offset)
{
	logerror("%s: Characteriser read offset %02x\n", machine().describe_context(), offset);
	return 0x51;
}

void mpu4_characteriser_bootleg51::write(offs_t offset, uint8_t data)
{
	logerror("%s: Characteriser write offset %02x data %02x\n", machine().describe_context(), offset, data);
}
