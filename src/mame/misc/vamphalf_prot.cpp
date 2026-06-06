// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Mission Craft / Wivern Wings / World Adventure FPGA (x2) based protection devices

Filename is a misnomer, just to keep it near misc/vamphalf.cpp

TODO:
- all games: emulate properly
\- black boxing close to impossible without the PCB at hand if not straight decap of the FPGA ports;
- wyvernwg: complete, upper limit of credits count not really understood, if there's any;
- worldadv: understand if the second FPGA at $0a0 is really used;
- Deduplicate stuff once everything works correctly;

**************************************************************************************************/

#include "emu.h"
#include "vamphalf_prot.h"

/*
 * Mission Craft
 *
 * Tests protection device on two places:
 * 1. at POST on $680 (PC=0xf81c)
 * 2. If check one is successful it attempts two new checks after about 15 minutes at $1a0 and $0d0,
 *    both seeds depends on number_of_credits % 3 inserted up to an arbitrary point in time
 *    (i.e. credit insertion is definitely disarmed after about 14 minutes)
 * If protection check fails then game intentionally add massive refresh hiccups after aforementioned 15 minutes.
 * It doesn't seem to matter if game(s) is being played, is left in attract or on title screen during this time.
 */

DEFINE_DEVICE_TYPE(MISNCRFT_FPGA_PROT, misncrft_fpga_prot_device, "misncrft_fpga_prot", "Mission Craft FPGA protection chips")

misncrft_fpga_prot_device::misncrft_fpga_prot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MISNCRFT_FPGA_PROT, tag, owner, clock)
{
}

void misncrft_fpga_prot_device::device_start()
{
	save_item(NAME(m_seed));
	save_item(NAME(m_retval));
	save_item(NAME(m_idx));
	save_item(NAME(m_is_armed));
}

void misncrft_fpga_prot_device::device_reset()
{
	m_retval = 0xff;
	m_idx = 0;
	m_is_armed = false;
}

u16 misncrft_fpga_prot_device::data_r(offs_t offset)
{
	u8 retval = (m_retval >> (7 - m_idx)) & 1;
	if (!machine().side_effects_disabled())
		m_idx ++;
	return retval ? 0xffff : 0;
}

u8 misncrft_fpga_prot_device::prot_value_check(std::vector<std::vector<u8>> &prot_table, int seed_size)
{
	const int prot_size = prot_table.size();
	int i;

	for (i = 0; i < prot_size; i++)
	{
		bool result = true;

		for (int j = 0; j < seed_size; j ++)
		{
			if (prot_table[i][j] != m_seed[j])
			{
				result = false;
				break;
			}
		}

		if (result == true)
			return (u8)prot_table[i][seed_size];
	}

	std::ostringstream outbuffer;
	util::stream_format(outbuffer, "%s: unhandled protection seed {", machine().describe_context());
	for (i = 0; i < seed_size; i++)
		util::stream_format(outbuffer, "0x%02x, ", m_seed[i]);
	util::stream_format(outbuffer, "},\n");

	logerror(outbuffer.str());
	//std::cout << outbuffer.str();
	// tested up to 3 hours, leave this in anyway for checking out if any inp testing triggers here.
	// machine().debug_break();
	return 0x00;
}

template <int seed_size> void misncrft_fpga_prot_device::seed_w(offs_t offset, u16 data)
{
	// Seed uploads with a 0xffff -> 1 byte x 8 times -> 0xffff
	// This should be used as a commit/reset chip mechanism,
	// likely in a flip-flop transition with any of the upper bits.
	// Note: the $340 version actually checks for 16 times instead of 8
	// Also note: return value is read 8 times but only first 4 bits are compared against, why?
	if (data == 0xffff)
	{
		m_idx = 0;

		if (m_is_armed)
		{
			if (seed_size == 8)
			{
				std::vector<std::vector<u8>> prot_table_8 = {
					{ 0x80, 0x6b, 0x4b, 0xfb, 0xe3, 0xfb, 0x55, 0xf6, 0xf0 },
					{ 0xdf, 0x7c, 0x1d, 0xe1, 0x87, 0x01, 0xbe, 0x31, 0xe0 },
					{ 0x57, 0x73, 0x0e, 0x47, 0x67, 0x67, 0x87, 0x58, 0xa0 },
					{ 0x89, 0x3c, 0x59, 0xea, 0x57, 0x12, 0x7b, 0xd2, 0x00 },
					{ 0xa1, 0xd8, 0x3c, 0x54, 0x54, 0x2f, 0x36, 0xaf, 0x80 },
					// 15 minutes checks (PC=f81c):
					// 0 credits
					{ 0x5f, 0xd8, 0x98, 0x8e, 0x1f, 0x3f, 0x37, 0xee, 0x20 },
					// 1 credit
					{ 0xda, 0x03, 0x78, 0x99, 0xcd, 0xe2, 0x1b, 0x77, 0x90 },
				};
				m_retval = prot_value_check(prot_table_8, seed_size);
			}
			else
			{
				std::vector<std::vector<u8>> prot_table_16 = {
					{
						0x4d, 0x4d, 0xfa, 0xbe, 0xa6, 0x5a, 0xa4, 0x86,
						0x8e, 0xdc, 0x09, 0x2d, 0x4e, 0xef, 0x56, 0xe1, 0xa0
					},
					// 2 credits
					{
						0xd9, 0xd9, 0x8f, 0x5f, 0x3f, 0xb6, 0xee, 0xc3,
						0xf8, 0x4d, 0x0d, 0xea, 0xbe, 0xa6, 0xda, 0x64, 0xc0
					},
				};
				m_retval = prot_value_check(prot_table_16, seed_size);
			}
		}

		// clear write latches
		for (int i = 0; i < seed_size; i++)
			m_seed[i] = 0;
		m_is_armed = false;
	}
	else
	{
		//printf("%02x %d\n", data & 0xff, m_idx);
		m_seed[m_idx] = data & 0xff;
		m_is_armed = true;
		m_idx ++;
	}
}

// Instantiate write accessors
template void misncrft_fpga_prot_device::seed_w<8>(offs_t offset, u16 data);
template void misncrft_fpga_prot_device::seed_w<16>(offs_t offset, u16 data);

/*
 * Wywern Wings
 *
 * Tested with 16 words seed after ~1 hour
 * Looks a similar pattern to Mission Craft except parallel instead of serial
 */

DEFINE_DEVICE_TYPE(WYVERNWG_FPGA_PROT, wyvernwg_fpga_prot_device, "wyvernwg_fpga_prot", "Wywern Wings FPGA protection chips")

wyvernwg_fpga_prot_device::wyvernwg_fpga_prot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, WYVERNWG_FPGA_PROT, tag, owner, clock)
{
}

void wyvernwg_fpga_prot_device::device_start()
{
	save_item(NAME(m_seed));
	save_item(NAME(m_retval));
	save_item(NAME(m_idx));
	save_item(NAME(m_is_armed));
}

void wyvernwg_fpga_prot_device::device_reset()
{
	m_retval = 0xff;
	m_idx = 0;
	m_is_armed = false;
}


 u8 wyvernwg_fpga_prot_device::prot_value_check(std::vector<std::vector<u16>> &prot_table, int seed_size)
{
	const int prot_size = prot_table.size();
	int i;

	for (i = 0; i < prot_size; i++)
	{
		bool result = true;

		for (int j = 0; j < seed_size; j ++)
		{
			if (prot_table[i][j] != m_seed[j])
			{
				result = false;
				break;
			}
		}

		if (result == true)
			return (u8)prot_table[i][seed_size];
	}

	std::ostringstream outbuffer;
	util::stream_format(outbuffer, "%s: unhandled protection seed { ", machine().describe_context());
	for (i = 0; i < seed_size; i++)
		util::stream_format(outbuffer, "0x%04x, ", m_seed[i]);
	util::stream_format(outbuffer, "},\n");

	logerror(outbuffer.str());
	//std::cout << outbuffer.str();
	// machine().debug_break();
	return 0x00;
}

u16 wyvernwg_fpga_prot_device::data_r(offs_t offset)
{
	// PC=f6b4
	return m_retval;
}

void wyvernwg_fpga_prot_device::seed_w(offs_t offset, u16 data)
{
	const int seed_size = 16;
	if (data == 0xffff)
	{
		m_idx = 0;

		if (m_is_armed)
		{
			std::vector<std::vector<u16>> prot_table = {
				// PC=1bb74 subroutine
				// expected value is stored from PC=f5f4 in L20 at PC=f664
				// $b5f78 initially contains the number of credits inserted
				// (gets modified on the fly for obfuscation reasons)
				// 0
				{
					0x53dc, 0x2704, 0x5664, 0x0daa, 0x421e, 0x3eac, 0x4d1c, 0x2f5a,
					0x20da, 0x2fe4, 0x69ac, 0x161a, 0x261e, 0x525e, 0x6512, 0x7e70, 0x00
				},
				// 1
				{
					0x167e, 0x2781, 0x446a, 0x794a, 0x15fa, 0x59e2, 0x1cfa, 0x3f55,
					0x0ff7, 0x0abd, 0x31de, 0x237c, 0x2f1c, 0x7de1, 0x4487, 0x6200, 0x08
				},
				// 2
				{
					0x5920, 0x27fe, 0x3272, 0x64ec, 0x69d6, 0x7518, 0x6cd9, 0x4f4f,
					0x7f13, 0x6594, 0x7a12, 0x30dc, 0x381b, 0x2963, 0x23fc, 0x4592, 0x0f
				},
				// 3
				{
					0x1bc2, 0x287d, 0x2078, 0x508e, 0x3db4, 0x104d, 0x3cb6, 0x5f49,
					0x6e2e, 0x406d, 0x4246, 0x3e3d, 0x4118, 0x54e4, 0x0371, 0x2925, 0x0d
				},
				// 4
				{
					0x5e66, 0x28fa, 0x0e7e, 0x3c2e, 0x1190, 0x2b83, 0x0c95, 0x6f43,
					0x5d4a, 0x1b44, 0x0a78, 0x4b9d, 0x4a17, 0x0066, 0x62e4, 0x0cb5, 0x0e
				},
				// 5
				{
					0x2108, 0x2979, 0x7c84, 0x27d0, 0x656d, 0x46b8, 0x5c72, 0x7f3c,
					0x4c69, 0x761d, 0x52ac, 0x58fd, 0x5315, 0x2be9, 0x4259, 0x7047, 0x0e
				},
				// 6
				{
					0x63aa, 0x29f6, 0x6a8c, 0x1372, 0x3949, 0x61ec, 0x2c51, 0x0f36,
					0x3b85, 0x50f4, 0x1adf, 0x665e, 0x5c14, 0x576b, 0x21cf, 0x53d8, 0x0c
				},
				// 7
				{
					0x264e, 0x2a75, 0x5892, 0x7f12, 0x0d25, 0x7d23, 0x7c2e, 0x1f30,
					0x2aa0, 0x2bcd, 0x6313, 0x73be, 0x6513, 0x02ed, 0x0142, 0x3768, 0x0a
				},
				// 8
				{
					0x68f0, 0x2af2, 0x4698, 0x6ab4, 0x6101, 0x1857, 0x4c0d, 0x2f2a,
					0x19bc, 0x06a4, 0x2b47, 0x011f, 0x6e10, 0x2e6e, 0x60b7, 0x1afa, 0x0c
				},
				// 9
				{
					0x2b92, 0x2b6f, 0x349e, 0x5656, 0x34df, 0x338d, 0x1beb, 0x3f25,
					0x08d9, 0x617d, 0x7379, 0x0e7f, 0x770f, 0x59f0, 0x402a, 0x7e8a, 0x07
				},
				// 10
				{
					0x6e36, 0x2bec, 0x22a6, 0x41f7, 0x08ba, 0x4ec2, 0x6bc8, 0x4f1f,
					0x77f7, 0x3c54, 0x3bad, 0x1be1, 0x000d, 0x0573, 0x1fa1, 0x621d, 0x03
				},
			};
			m_retval = prot_value_check(prot_table, seed_size);
		}

		// clear write latches
		for (int i = 0; i < seed_size; i++)
			m_seed[i] = 0;
		m_is_armed = false;
	}
	else
	{
		//printf("%02x %d\n", data & 0xffff, m_idx);
		m_seed[m_idx] = data & 0xffff;
		m_is_armed = true;
		m_idx ++;
	}
}

/*
 * World Adventure
 *
 * Accesses the protection device at ~30 minutes, then every 15 minutes up to 2 hours where
 * it actually kicks in.
 * Doesn't seem to be credit affected, it first do a dummy read (for flushing any previous read
 * sequence?) then write a 0xffff then proceeds to write 33-bits of serial data that are read back
 * as a 8-bit result.
 * PC=c55a8 is where the return value comparison happens, more specifically:
 * bp c55a8,1,{printf "%02x %02x",L3,L6}
 *
 * TODO: also access $0a0 I/O after the 2 hours mark (write only? left-over? Extra protection?)
 * Game has been run for 5 hours straight without any visible issue ...
 */

 DEFINE_DEVICE_TYPE(WORLDADV_FPGA_PROT, worldadv_fpga_prot_device, "worldadv_fpga_prot", "World Adventure FPGA protection chips")

worldadv_fpga_prot_device::worldadv_fpga_prot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, WORLDADV_FPGA_PROT, tag, owner, clock)
{
}

void worldadv_fpga_prot_device::device_start()
{
	save_item(NAME(m_read_idx));
	save_item(NAME(m_write_idx));
	save_item(NAME(m_seed));
	save_item(NAME(m_retval));
}

void worldadv_fpga_prot_device::device_reset()
{
	m_read_idx = 0;
	m_write_idx = 0;
	m_is_armed = false;
}

u16 worldadv_fpga_prot_device::data_r(offs_t offset)
{
	if (!m_is_armed)
		return 0;

	u8 retval = (m_retval >> (7 - m_read_idx)) & 1;
	if (!machine().side_effects_disabled())
	{
		m_read_idx ++;
		if (m_read_idx > 7)
		{
			m_is_armed = false;
		}
	}
	return retval ? 0xffff : 0;
}

void worldadv_fpga_prot_device::seed_w(offs_t offset, u16 data)
{
	// bit 15 may just reset while bit 0 is data write
	if (data == 0xffff)
	{
		m_write_idx = 0;
		m_seed = 0;
	}
	else
	{
		if (data & 0xfffe)
			logerror("Warning: unexpected data write %04x at write phase %d\n", data, m_write_idx);
		m_seed = (m_seed << 1) | (data & 1);
		m_write_idx ++;
		if (m_write_idx == 33)
		{
			m_is_armed = true;
			m_read_idx = 0;
			switch(m_seed)
			{
				// ~0:30 minutes
				// 110001100100101111111011011010111
				case 0x18c97f6d7: m_retval = 0xa7; break;
				// ~0:45
				// 010111010101010011110110111110111
				case 0x0baa9edf7: m_retval = 0x6d; break;
				// ~1:00
				// 000111000111110000011100110111111
				case 0x038f839bf: m_retval = 0x20; break;
				// ~1:15
				// 100010000000000110111111100001111
				case 0x110037f0f: m_retval = 0x58; break;
				// ~1:30
				// 100001010101011001110010110111101
				case 0x10aace5bd: m_retval = 0x55; break;
				// ~1:45
				// 011111000110011101100110010001111
				case 0x0f8cecc8f: m_retval = 0x74; break;
				// ~2:00
				// 110010110011110001011001100010001
				case 0x19678b311: m_retval = 0xf5; break;
				default:
					logerror("Warning: unemulated seed value %09lx\n", m_seed);
					break;
			}
		}
	}
}
