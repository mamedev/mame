// license:BSD-3-Clause
// copyright-holders:David Haywood, James Wallace

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

 - in m4blsbys this string is at 0x17383 in ROM, which is banked into memory at 0x7383
 - this is first accessed by the code at 50c3, a CMPA $0138,X opcode, where X is 0x724B (0x724B + 0x138 = 0x7383)


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
	device_t(mconfig, type, tag, owner, clock)
{
}

void mpu4_characteriser_pal_bwb::device_start()
{
}

void mpu4_characteriser_pal_bwb::device_reset()
{
}

void mpu4_characteriser_pal_bwb::write(offs_t offset, uint8_t data)
{
	m_call = data;
	logerror("%s Characteriser write offset %02x data %02x\n", machine().describe_context(), offset, data);
	m_initval_ready = true;

	if (m_chr_counter > 35)
	{
		m_chr_counter = 36;
	}
	else
	{
		m_chr_counter++;
	}

	if (m_call == ((m_otherkey >> 16) & 0xff))
		m_bwb_return = 0;

	if ((m_call == m_commonkey) || (m_call == ((m_otherkey >> 16) & 0xff)) || (m_call == ((m_otherkey >> 8) & 0xff)) || (m_call == ((m_otherkey >> 0) & 0xff)))
	{
		if (m_bwb_return < 16)
		{
			m_chr_value = bwb_chr_table_common[m_bwb_return];
		}

		m_bwb_return++;
	}
	else
	{
		m_chr_value = machine().rand();
		m_bwb_return = 0;
	}
}

uint8_t mpu4_characteriser_pal_bwb::read(offs_t offset)
{
	logerror("%s Characteriser read offset %02x\n", machine().describe_context(), offset);

	if ((offset == 0) && m_initval_ready)
	{
		m_initval_ready = false;

		switch (m_chr_counter)
		{
		case 7:
		case 14:
		case 21:
		case 28:
		case 35:
			logerror("m_call %02x\n", m_call);

			// it is unclear what we actually need to return here for normal operation
			return machine().rand();

		default:
			return m_chr_value;
		}

	}
	else
	{
		return m_chr_value;
	}
}
