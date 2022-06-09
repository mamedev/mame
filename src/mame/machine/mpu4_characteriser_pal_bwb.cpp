// license:BSD-3-Clause
// copyright-holders:David Haywood



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



#include "emu.h"

#include "mpu4_characteriser_pal_bwb.h"

DEFINE_DEVICE_TYPE(MPU4_CHARACTERISER_PAL_BWB, mpu4_characteriser_pal_bwb, "mpu4chrpal_bwb", "BWB MPU4 Characteriser PAL")

mpu4_characteriser_pal_bwb::mpu4_characteriser_pal_bwb(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mpu4_characteriser_pal_bwb(mconfig, MPU4_CHARACTERISER_PAL_BWB, tag, owner, clock)
{
}

mpu4_characteriser_pal_bwb::mpu4_characteriser_pal_bwb(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_current_chr_table(nullptr),
	m_prot_col(0),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_allow_6809_cheat(false),
	m_allow_68k_cheat(false),
	m_current_lamp_table(nullptr),
	m_lamp_col(0),
	m_4krow(0),
	m_protregion(*this, "fakechr")
{
}

void mpu4_characteriser_pal_bwb::device_start()
{
}

void mpu4_characteriser_pal_bwb::device_reset()
{
}

#if 0

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




void mpu4_characteriser_pal_bwb::write(offs_t offset, uint8_t data)
{
	int x;
	int call = data;
	logerror("%s Characteriser write offset %02x data %02x\n", machine().describe_context(), offset, data);

	if (!m_current_chr_table)
	{
		popmessage("%s No Characteriser Table\n", machine().describe_context());
		return;
	}

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

	if (!m_current_chr_table)
	{
		return 0x00;
	}

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
