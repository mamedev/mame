// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

    Data East 32 bit ARM based games:

    Fighter's History
    Night Slashers
    Tattoo Assassins


    Emulation by Bryan McPhail, mish@tendril.co.uk.  Thank you to Tim,
    Avedis and Stiletto for many things including work on Fighter's
    History protection and tracking down Tattoo Assassins!

    Fighter's History - reset with both start buttons held down for
	test mode.  Reset with player 1 start held in Fighter's History
	for 'Pattern Editor'.

    For version information:
     Fighter's History - Reset with Player 1 button 1 & 2 held
     Night Slashers    - Reset with Player 1 & 2 start held

    Tattoo Assassins is a prototype, it is thought only 25 test units
    were manufactured and distributed to test arcades before the game
    was recalled.  TA is the only game developed by Data East Pinball
    in USA, rather than Data East Corporation in Japan.

    Tattoo Assassins uses DE Pinball soundboard 520-5077-00 R

    TODO:

    Tattoo Assassins & Night slashers use an less emulated chip (Ace/Jack) for
    special blending effects.  It's exact effect is unclear.

    Z80 Sound cpu version games:
	Music tempo is unverified (it has external timer / IRQ controller?).


Night Slashers
Data East, 1993

PCB Layout
----------

DE-0397-0   DEC-22VO
|-----------------------------------------------------|
| TA8205AH   Z80          |-----|              HBM-07 |
|         6164    YM2151  | 52  |                     |
|         LX02            |     | HBM-09       HBM-06 |
|     YM3012      32MHz   |-----|                     |
|  JP1   HBM-11   93C45                        HBM-05 |
|CN2     HBM-10           |-----|                     |
|  M6295(1)               | 52  |              HBM-04 |
|     M6295(2)            |     |                     |
|      |-----| |-----|    |-----|    |-----|   HBM-03 |
|J     | 104 | | 153 |               | 52  |          |
|A     |     | |     |     |-----|   |     |   HBM-02 |
|M     |-----| |-----|     | 153 |   |-----|          |
|M                         |     |           28.322MHz|
|A                         |-----|                    |
|    |-----| PAL                                      |
|    | 99  | PROM              |-----| 6164    HBM-01 |
|    |     |                   | 74  | 6164           |
|    |-----|                   |     |         HBM-00 |
|      |-----|  |-----|        |-----|                |
|      | 153 |  | 200 |        |-----| 6164           |
|      |     |  |     |        | 141 | 6164           |
|      |-----|  |-----|        |     |   PAL          |
|TEST_SW                       |-----|   PAL  |-----| |
|                                             | 156 | |
|           LH52250  LH52250   LX-01          |     | |
|  CN3      LH52250  LH52250   LX-00          |-----| |
|-----------------------------------------------------|
Notes:
       Also seen with 28.0MHz and 32.22MHz XTALs (matches the other games in this driver, MAME chooses this configuration).
       The CPU is chip 156. It's an encrypted ARM-based CPU. The CPU is running at 7.0805MHz [28.322/4]

       Z80B      - Goldstar Z8400B, running at 3.5555MHz [32/9]
       YM2151    - Yamaha YM2151 sound chip, running at 3.5555MHz [32/9]
       OKI M6295 - Oki M6295 PCM Sample chip, (1) running at 1.000MHz [32/32]. Sample rate = 1000000 / 132
                                              (2) running at 2.000MHz [32/16]. Sample rate = 2000000 / 132
       6164      - UM6164BK-20 8K x8 SRAM
       LH52250   - Sharp LH52250 32K x8 SRAM
       93C45     - 128bytes x8 Serial EEPROM
       PALs      - PAL 16L8ACN (x 2, near program ROMs, one at 3D labelled 'VM-00', one at 4D labelled 'VM-01')
                   PAL 16l8ACN (near chip 99, located at 8J, labelled 'VM-02')
       HSync     - 15.86kHz
       VSync     - 58Hz
       Custom ICs-
                   DE #            Package Type      Additional #'s (for reference of scratched-off chips on other PCB's)
                   ------------------------------------------------------------------------------------------------------
                   156 (CPU)       100 Pin PQFP      932EV 301801
                   141             160 Pin PQFP      24220F008
                   74              160 Pin PQFP      24220F009
                   99              208 Pin PQFP      L7A0967
                   52 (x3)         128 Pin PQFP      9322EV 298251 VC5259-0001
                   153 (x3)        144 Pin PQFP      L7A0888 9328
                   104             100 Pin PQFP      L7A0717 9148
                   200             100 Pin PQFP      JAPAN 9320PD027  (chip is darker black)

       Other     - There's a small push button near the JAMMA connector to access test mode.
                   All settings are via an on-board menu.

                   All pinouts conform to standard JAMMA pinout.  Joystick is 8-way with 3 buttons used.

                   JP1: 1-2 Sound Output in MONO
                        2-3 Sound Output in STEREO

                   CN2: 4 Pin connector (use when JP1 = 2-3)
                        Pin #   Signal
                         1      L-Speaker +
                         2      L-Speaker -
                         3      R-Speaker -
                         4      R-Speaker +

                   CN3: 15 Pin connector (Player 3)
                        Pin #   Signal
                         1      COIN SW3
                         2      3P PUSH 3
                         3      3P LEFT
                         4      3P RIGHT
                         5      3P UP
                         6      3P DOWN
                         7      3P PUSH 1
                         8      3P PUSH 2
                         9-13   NOT USED
                         14-15  GND


       ROMs      - MAINPRG1.1F     HN27C4096      \
                   MAINPRG2.2F     HN27C4096      /  Main Program (no ROM stickers attached, DE ROM code unknown)

                   Japanese Version
                   LX01-.2F        HN27C4096      \
                   LX00-.1F        HN27C4096      /  Main Program (Japan version)

                   MBH-00.8C   42 pin 16M MASK  (read as 27C160)    \
                   MBH-01.9C   42 pin 16M MASK  (read as 27C160)    |
                   MBH-02.14C  42 pin 16M MASK  (read as 27C160)    |
                   MBH-03.15C  40 pin 4M MASK   (read as MX27C4100) |
                   MBH-04.16C  42 pin 16M MASK  (read as 27C160)    |
                   MBH-05.17C  40 pin 4M MASK   (read as MX27C4100) | GFX
                   MBH-06.18C  32 pin 8M MASK   (read as 27C080)    |
                   MBH-07.19C  32 pin 2M MASK   (read as 27C020)    |
                   MBH-08.16E  40 pin 4M MASK   (read as MX27C2100) |
                   MBH-09.18E  40 pin 4M MASK   (read as MX27C2100) /

                   MBH-10.14L  32 pin 4M MASK   (read as 27C040)    \
                   MBH-11.16L  32 pin 4M MASK   (read as 27C040)    / Sound (Samples)

                   LX02-.17L   27C512                                 Sound Program

                   PROM.9J     Fujitsu MB7124 compatible with 82S147  Labelled 'LN-00'

Night Slashers
Data East, 1993

DE-0395-1   DEC-22VO
|-----------------------------------------------------|
| TA8205AH  6164            |-----|   2M-16M*   MBH-07|
|            02-  HuC6280A  | 52  |   MBH-09    MBH-06|
|           YM2151          |     |   2M-16M*   MBH-05|
|     YM3012      32.220MHz |-----|   MBH-08    MBH-04|
|  JP1   MBH-11   93C45     |-----|             MBH-03|
|CN2     MBH-10             | 52  |             MBH-02|
|  M6295(1)                 |     |                   |
|     M6295(2)              |-----|                   |
|      |-----|                       |-----|          |
|J     | 104 |    |-----|  |-----|   | 52  |          |
|A     |     |    | 153 |  | 113 |   |     |          |
|M     |-----|    |     |  |     |   |-----|          |
|M                |-----|  |-----|            28MHz   |
|A  |-----|                                           |
|   | 99  |  VM-02                                    |
|   |     |  LN-00             |-----| 6164    MBH-01 |
|   |-----|                    | 74  | 6164           |
|                              |     |         MBH-00 |
|      |-----|  |-----|        |-----|                |
|      | 113 |  | 200 |        |-----| 6164           |
|      |     |  |     |        | 141 | 6164           |
|      |-----|  |-----|        |     |  VE-01A        |
|TEST_SW                       |-----|  VE-00 |-----| |
|                                             | 156 | |
|  CN4      LH52250  LH52250     01-          |     | |
|  CN3      LH52250  LH52250     00-          |-----| |
|-----------------------------------------------------|

NOTE: Program EPROMs did NOT have regional letter codes, just 00, 01 & 02

Same PCB as shown below for Fighter's History, but populated with more
chips as needed. It is worth noting that Night Slashers uses the encryption
features of the 156 chip where as Fighter's History does not.

*****************************************************************************************************

Fighter's History
Data East, 1993

PCB Layout
----------

DE-0395-1   DEC-22VO
|-----------------------------------------------------|
| TA8205AH  6164                      MBF-05    2M-8M*|
|           LE02  HuC6280A            MBF-04       8M*|
|           YM2151            52*     MBF-03   4M-16M*|
|     YM3012      32.220MHz           MBF-02      16M*|
|  JP1   MBF-07   93C45                        4M-16M*|
|CN2     MBF-06                                   16M*|
|  M6295(1)                   52*                     |
|     M6295(2)                                        |
|      |-----|                       |-----|          |
|J     | 75  |   113/153*            | 52  |          |
|A     |     |             |-----|   |     |          |
|M     |-----|             | 113 |   |-----|          |
|M                         |     |            28MHz   |
|A                         |-----|                    |
|            PAL*                                     |
|      99*   KT-00             |-----| 6164    MBF-01 |
|                              | 74  | 6164           |
|                              |     |         MBF-00 |
|      |-----|  |-----|        |-----|                |
|      | 113 |  | 200 |        |-----| 6164           |
|      |     |  |     |        | 56  | 6164           |
|      |-----|  |-----|        |     |  VE-01A        |
|TEST_SW                       |-----|  VE-00 |-----| |
|                                             | 156 | |
|  CN4      LH52250  LH52250   LE01           |     | |
|  CN3*     LH52250  LH52250   LE00           |-----| |
|-----------------------------------------------------|


Very similar to the DE-0396-0 described below with the notable exceptions:
  The sound area reworked to use the HuC6280A instead of a standard Z80
  Uses the larger 113 instead of the 153 chips, however both PCBs have
    solder pads in the same locations for either chip.
  Uses the 56 instead of the 141


DE-0396-0   DEC-22VO
|-----------------------------------------------------|
| TA8205AH  Z80                       MBF-05    2M-8M*|
|           6164  YM2151              MBF-04       8M*|
|           LJ02              52*     MBF-03   4M-16M*|
|     YM3012      32.220MHz           MBF-02      16M*|
|  JP1   MBF-07   93C45                        4M-16M*|
|CN2     MBF-06                                   16M*|
|  M6295(1)                   52*                     |
|     M6295(2)                                        |
|      |-----|                       |-----|          |
|J     | 75  |   113/153*            | 52  |          |
|A     |     |             |-----|   |     |          |
|M     |-----|             | 153 |   |-----|          |
|M                         |     |            28MHz   |
|A                         |-----|                    |
|            PAL*                                     |
|      99*   KT-00             |-----| 6164    MBF-01 |
|                              | 74  | 6164           |
|                              |     |         MBF-00 |
|      |-----|  |-----|        |-----|                |
|      | 153 |  | 200 |        |-----| 6164           |
|      |     |  |     |        | 141 | 6164           |
|      |-----|  |-----|        |     |  VE-01A        |
|TEST_SW                       |-----|  VE-00 |-----| |
|                                             | 156 | |
|  CN4      LH52250  LH52250   LJ01-3         |     | |
|  CN3*     LH52250  LH52250   LJ00-3         |-----| |
|-----------------------------------------------------|

This PCB is very close to the DE-397-0 listed above

       Custom ICs-
                   DE #            Package Type      Additional #'s (for reference of scratched-off chips on other PCB's)
                   ------------------------------------------------------------------------------------------------------
                   156 (CPU)       100 Pin PQFP      9321EV 301811 (Doesn't use encryption functions of the 156)
                   141             160 Pin PQFP      24220F008
                   74              160 Pin PQFP      24220F009
                   52              128 Pin PQFP      9313EV 211771 VC5259-0001
                   153 (x2)        144 Pin PQFP      L7A0888 9312
                   75              100 Pin PQFP      L7A0680 9143
                   200             100 Pin PQFP      JAPAN 9315PP002  (chip is darker black)

NOTE: There are several unpopulated locations (denoted by *) for additional rom chips
      or more custom ICs.

***************************************************************************/

#include "emu.h"
#include "fghthist.h"

#include "deco156.h"
#include "decocrpt.h"

#include "cpu/arm/arm.h"
#include "cpu/m6809/m6809.h"
#include "cpu/z80/z80.h"
#include "machine/input_merger.h"

#include "speaker.h"

#include <algorithm>


//**************************************************************************
//  PROTECTION
//**************************************************************************

u16 fghthist_common_state::ioprot_r(offs_t offset)
{
	const offs_t real_address = 0 + (offset * 2);
	const offs_t deco146_addr = (BIT(real_address, 14, 4) << 11) | BIT(real_address, 0, 11); // NC 31-18, 13-11
	u8 cs = 0;

	return m_ioprot->read_data(deco146_addr, cs);
}

void fghthist_common_state::ioprot_w(offs_t offset, u16 data, u16 mem_mask)
{
	const offs_t real_address = 0 + (offset * 2);
	const offs_t deco146_addr = (BIT(real_address, 14, 4) << 11) | BIT(real_address, 0, 11); // NC 31-18, 13-11
	u8 cs = 0;

	m_ioprot->write_data(deco146_addr, data, mem_mask, cs);
}


//**************************************************************************
//  SOUND
//**************************************************************************

void fghthist_common_state::volume_w(u8 data)
{
	// TODO: Assumes linear scaling
	// values go from 0x00 (max volume) to 0xff (min volume)
	const u8 raw_vol = 0xff - data;
	const float vol_output = float(raw_vol) / 255.0f;

	m_ym2151->set_output_gain(ALL_OUTPUTS, vol_output);
	m_oki[0]->set_output_gain(ALL_OUTPUTS, vol_output);
	m_oki[1]->set_output_gain(ALL_OUTPUTS, vol_output);
}

void tattass_state::tattass_sound_irq_w(int state)
{
	if (state)
	{
		u8 data = m_ioprot->soundlatch_r();
		// Swap bits 0 and 3 to correct for design error from BSMT schematic
		data = bitswap<8>(data, 7, 6, 5, 4, 0, 2, 1, 3);
		m_decobsmt->bsmt_comms_w(data);
	}
}

void fghthist_common_state::sound_bankswitch_w(u8 data)
{
	m_oki[0]->set_rom_bank((data >> 0) & 1);
	m_oki[1]->set_rom_bank((data >> 1) & 1);
}


//**************************************************************************
//  VIDEO
//**************************************************************************

void fghthist_common_state::vblank_ack_w(u32 data)
{
	m_maincpu->set_input_line(ARM_IRQ_LINE, CLEAR_LINE);
}

template<int Chip>
u32 fghthist_common_state::spriteram_r(offs_t offset)
{
	return m_spriteram16[Chip][offset] ^ 0xffff0000;
}

template<int Chip>
void fghthist_common_state::spriteram_w(offs_t offset, u32 data, u32 mem_mask)
{
	data &= 0x0000ffff;
	mem_mask &= 0x0000ffff;
	COMBINE_DATA(&m_spriteram16[Chip][offset]);
}

template<int Chip>
void fghthist_common_state::buffer_spriteram_w(u32 data)
{
	std::copy(&m_spriteram16[Chip][0], &m_spriteram16[Chip][0x2000/4], &m_spriteram16_buffered[Chip][0]);
}

// tattass tests these as 32-bit ram, even if only 16-bits are hooked up to the tilemap chip - does it mirror parts of the dword?
template <int TileMap> void fghthist_common_state::pf_rowscroll_w(offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_pf_rowscroll32[TileMap][offset]); data &= 0x0000ffff; mem_mask &= 0x0000ffff; COMBINE_DATA(&m_pf_rowscroll[TileMap][offset]); }

DECOSPR_PRIORITY_CB_MEMBER( fghthist_state::fghthist_pri_callback )
{
	u32 mask = GFX_PMASK_8; // under the text layer
	if (extpri)
		mask |= GFX_PMASK_4; // under the uppermost playfield

	return mask;
}

DECO16IC_BANK_CB_MEMBER( fghthist_state::bank_callback )
{
	bank = (bank & 0x10) | ((bank & 0x40) >> 1) | ((bank & 0x20) << 1);

	return bank << 8;
}

DECO16IC_BANK_CB_MEMBER( nslasher_state::bank_callback )
{
	return (bank & ~0xf) << 8;
}

u16 nslasher_state::mix_callback(u16 p, u16 p2)
{
	return ((p & 0x70f) + (((p & 0x30) | (p2 & 0x0f)) << 4)) & 0x7ff;
}


//**************************************************************************
//  EEPROM
//**************************************************************************

u8 fghthist_common_state::eeprom_r()
{
	return 0xfe | m_eeprom->do_read();
}

void fghthist_common_state::eeprom_w(u8 data)
{
	// 7-------  unknown
	// -6------  eeprom cs
	// --5-----  eeprom clk
	// ---4----  eeprom di
	// ----3---  unknown
	// -----2--  color fading effect mode (0 = without obj1, 1 = with obj1)
	// ------1-  bg2/3 joint mode (8bpp) (not used by fghthist?)
	// -------0  layer priority

	m_eeprom->clk_write(BIT(data, 5) ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->di_write(BIT(data, 4));
	m_eeprom->cs_write(BIT(data, 6) ? ASSERT_LINE : CLEAR_LINE);

	pri_w(data & 0x07);
}

void tattass_state::tattass_control_w(offs_t offset, u32 data, u32 mem_mask)
{
	// Eprom in low byte
	if (ACCESSING_BITS_0_7)
	{ // Byte write to low byte only (different from word writing including low byte)
		/*
		    The Tattoo Assassins eprom seems strange...  It's 1024 bytes in size, and 8 bit
		    in width, but offers a 'multiple read' mode where a bit stream can be read
		    starting at any byte boundary.

		    Multiple read mode:
		    Write 110aa000      [Read command, top two bits of address, 4 zeroes]
		    Write 00000000      [8 zeroes]
		    Write aaaaaaaa      [Bottom 8 bits of address]

		    Then bits are read back per clock, for as many bits as needed (NOT limited to byte
		    boundaries).

		    Write mode:
		    Write 000aa000      [Write command, top two bits of address, 4 zeroes]
		    Write 00000000      [8 zeroes]
		    Write aaaaaaaa      [Bottom 8 bits of address]
		    Write dddddddd      [8 data bits]

		*/
		if ((BIT(data, 6)) == 0)
		{
			if (m_buf_ptr)
			{
				logerror("%s: Eprom reset (bit count %d): ", machine().describe_context(), m_read_bit_count);
				for (int i = 0; i < m_buf_ptr; i++)
					logerror("%s", BIT(m_buffer, m_buf_ptr - 1 - i) ? "1" : "0");
				logerror("\n");

			}
			m_buf_ptr = 0;
			m_pending_command = 0;
			m_read_bit_count = 0;
		}

		// Eprom has been clocked
		if (m_last_clock == 0 && BIT(data, 5) && BIT(data, 6))
		{
			if (m_buf_ptr >= 32)
			{
				logerror("%s: Eprom overflow!\n", machine().describe_context());
				m_buf_ptr = 0;
			}

			// Handle pending read
			if (m_pending_command == 1)
			{
				const int d = m_read_bit_count >> 3;
				const int m = 7 - (m_read_bit_count & 0x7);
				const int a = (m_byte_addr + d) & 0x3ff;
				const int b = m_eeprom->internal_read(a);

				m_tattass_eprom_bit = (b >> m) & 1;

				m_read_bit_count++;
				m_last_clock = BIT(data, 5);
				return;
			}

			// Handle pending write
			if (m_pending_command == 2)
			{
				m_buffer = (m_buffer << 1) | BIT(data, 4);

				m_buf_ptr++;
				if (m_buf_ptr == 32)
				{
					m_eeprom->internal_write(m_byte_addr, m_buffer & 0xff);
				}
				m_last_clock = BIT(data, 5);
				return;
			}

			m_buffer = (m_buffer << 1) | BIT(data, 4);

			m_buf_ptr++;
			if (m_buf_ptr == 24)
			{
				// Decode addr
				m_byte_addr = ((m_buffer & 0x180000) >> 11) | (m_buffer & 0x0000ff);

				// Check for read command
				if ((m_buffer & 0xc00000) == 0xc00000)
				{
					m_tattass_eprom_bit = (m_eeprom->internal_read(m_byte_addr) >> 7) & 1;
					m_read_bit_count = 1;
					m_pending_command = 1;
				}

				// Check for write command
				else if ((m_buffer & 0xc00000) == 0x0)
				{
					m_pending_command = 2;
				}
				else
				{
					logerror("%s: Detected unknown eprom command\n", machine().describe_context());
				}
			}
		}
		else
		{
			if (!(BIT(data, 6)))
			{
				logerror("%s: Cs set low\n", machine().describe_context());
				m_buf_ptr = 0;
			}
		}

		m_last_clock = BIT(data, 5);
	}

	// Volume in high byte
	if (ACCESSING_BITS_8_15)
	{
		//TODO:  volume attenuation == ((data >> 8) & 0xff);
		// TODO: is it really there?
	}

	// Playfield control - Only written in full word memory accesses
	pri_w(data & 0x7); // Bit 0 - layer priority toggle, Bit 1 - BG2/3 Joint mode (8bpp), Bit 2 - color fading effect mode (with/without OBJ1)?

	// Sound board reset control
	if (BIT(data, 7))
		m_decobsmt->bsmt_reset_line(CLEAR_LINE);
	else
		m_decobsmt->bsmt_reset_line(ASSERT_LINE);

	// bit 0x4 fade cancel?
	// bit 0x8 ??
	// Bit 0x100 ??
	//logerror("%08x: %08x data\n",data,mem_mask);
}

u16 tattass_state::port_b_tattass()
{
	return m_tattass_eprom_bit;
}


//**************************************************************************
//  MACHINE
//**************************************************************************

u32 fghthist_state::unk_status_r()
{
	// bit 3 needs to be 0
	return 0xfffffff7;
}

u16 nslasher_state::nslasher_debug_r()
{
	return 0xffff;
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void fghthist_common_state::common_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x11ffff).ram();
	map(0x140000, 0x140003).w(FUNC(fghthist_state::vblank_ack_w));
	map(0x182000, 0x183fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf1_data_dword_r), FUNC(deco16ic_device::pf1_data_dword_w));
	map(0x184000, 0x185fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf2_data_dword_r), FUNC(deco16ic_device::pf2_data_dword_w));
	map(0x192000, 0x193fff).ram().w(FUNC(fghthist_state::pf_rowscroll_w<0>)).share(m_pf_rowscroll32[0]);
	map(0x194000, 0x195fff).ram().w(FUNC(fghthist_state::pf_rowscroll_w<1>)).share(m_pf_rowscroll32[1]);
	map(0x1a0000, 0x1a001f).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf_control_dword_r), FUNC(deco16ic_device::pf_control_dword_w));
	map(0x1c2000, 0x1c3fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf1_data_dword_r), FUNC(deco16ic_device::pf1_data_dword_w));
	map(0x1c4000, 0x1c5fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf2_data_dword_r), FUNC(deco16ic_device::pf2_data_dword_w));
	map(0x1d2000, 0x1d3fff).ram().w(FUNC(fghthist_state::pf_rowscroll_w<2>)).share(m_pf_rowscroll32[2]);
	map(0x1d4000, 0x1d5fff).ram().w(FUNC(fghthist_state::pf_rowscroll_w<3>)).share(m_pf_rowscroll32[3]);
	map(0x1e0000, 0x1e001f).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf_control_dword_r), FUNC(deco16ic_device::pf_control_dword_w));
	map(0x200000, 0x207fff).rw(FUNC(fghthist_state::ioprot_r), FUNC(fghthist_state::ioprot_w)).umask32(0xffff0000).share("prot32ram"); // only maps on 16-bits
}

void fghthist_state::fghthist_common_map(address_map &map)
{
	common_map(map);
	map(0x168000, 0x169fff).ram().w(FUNC(fghthist_state::buffered_palette_w)).share(m_paletteram);
	map(0x16c008, 0x16c00b).w(FUNC(fghthist_state::palette_dma_w));
	map(0x16c010, 0x16c013).r(FUNC(fghthist_state::unk_status_r));
	map(0x178000, 0x179fff).rw(FUNC(fghthist_state::spriteram_r<0>), FUNC(fghthist_state::spriteram_w<0>));
	map(0x17c010, 0x17c013).w(FUNC(fghthist_state::buffer_spriteram_w<0>));
	map(0x17c020, 0x17c023).r(FUNC(fghthist_state::unk_status_r));
}

void fghthist_state::fghthisto_map(address_map &map)
{
	map.unmap_value_high();
	fghthist_common_map(map);
//  map(0x000000, 0x001fff).rom().w(FUNC(fghthist_state::pf1_data_w)); // wtf??
	map(0x120020, 0x120021).lr16(NAME([this] () { return m_io_in[0]->read(); }));
	map(0x120024, 0x120025).lr16(NAME([this] () { return m_io_in[1]->read(); }));
	map(0x120028, 0x120028).r(FUNC(fghthist_state::eeprom_r));
	map(0x12002c, 0x12002c).w(FUNC(fghthist_state::eeprom_w));
	map(0x12002d, 0x12002d).w(FUNC(fghthist_state::volume_w));
	map(0x1201fc, 0x1201fc).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x208800, 0x208803).nopw(); // ?
}

void fghthist_state::fghthist_map(address_map &map)
{
	fghthist_common_map(map);
	map(0x150000, 0x150000).w(FUNC(fghthist_state::eeprom_w));
	map(0x150001, 0x150001).w(FUNC(fghthist_state::volume_w));
}

void nslasher_state::nslasher_common_map(address_map &map)
{
	common_map(map);
	map(0x163000, 0x16309f).rw(m_deco_ace, FUNC(deco_ace_device::ace_r), FUNC(deco_ace_device::ace_w)).umask32(0x0000ffff); // 'Ace' RAM
	map(0x164000, 0x164000).w(FUNC(nslasher_state::tilemap_color_bank_w));
	map(0x164004, 0x164004).w(FUNC(nslasher_state::sprite1_color_bank_w));
	map(0x164008, 0x164008).w(FUNC(nslasher_state::sprite2_color_bank_w));
	map(0x16400c, 0x16400f).nopw();
	map(0x168000, 0x169fff).rw(m_deco_ace, FUNC(deco_ace_device::buffered_palette_r), FUNC(deco_ace_device::buffered_palette_w));
	map(0x16c000, 0x16c003).nopw();
	map(0x16c008, 0x16c00b).w(m_deco_ace, FUNC(deco_ace_device::palette_dma_w));
	map(0x170000, 0x171fff).rw(FUNC(nslasher_state::spriteram_r<0>), FUNC(nslasher_state::spriteram_w<0>));
	map(0x174000, 0x174003).nopw(); // Sprite DMA mode (2)
	map(0x174010, 0x174013).w(FUNC(nslasher_state::buffer_spriteram_w<0>));
	map(0x174018, 0x17401b).nopw(); // Sprite 'CPU' (unused)
	map(0x178000, 0x179fff).rw(FUNC(nslasher_state::spriteram_r<1>), FUNC(nslasher_state::spriteram_w<1>));
	map(0x17c000, 0x17c003).nopw(); // Sprite DMA mode (2)
	map(0x17c010, 0x17c013).w(FUNC(nslasher_state::buffer_spriteram_w<1>));
	map(0x17c018, 0x17c01b).nopw(); // Sprite 'CPU' (unused)
	map(0x200000, 0x207fff).r(FUNC(nslasher_state::nslasher_debug_r)).umask32(0x0000ffff); // seems to be debug switches / code activated by this?
}

void nslasher_state::nslasher_map(address_map &map)
{
	nslasher_common_map(map);
	map(0x120000, 0x1200ff).noprw();                         // ACIA (unused)
	map(0x150000, 0x150000).w(FUNC(nslasher_state::eeprom_w));
	map(0x150001, 0x150001).w(FUNC(nslasher_state::volume_w));
}

void tattass_state::tattass_map(address_map &map)
{
	nslasher_common_map(map);
	map(0x0f8000, 0x0fffff).rom().nopw();
	map(0x120000, 0x120003).noprw();             // ACIA (unused)
	map(0x130000, 0x130003).nopw();        // Coin port (unused?)
	map(0x150000, 0x150003).w(FUNC(tattass_state::tattass_control_w)); // Volume port/Eprom/Priority
	map(0x162000, 0x162fff).ram();             // 'Jack' RAM!?
}

// H6280 based sound
void fghthist_common_state::h6280_sound_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x110000, 0x110001).rw(m_ym2151, FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x120000, 0x120001).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x130000, 0x130001).rw(m_oki[1], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x140000, 0x140000).r(m_ioprot, FUNC(deco_146_base_device::soundlatch_r));
	map(0x1f0000, 0x1f1fff).ram();
}

void fghthist_state::h6280_sound_custom_latch_map(address_map &map)
{
	h6280_sound_map(map);
	map(0x140000, 0x140000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}

// Z80 based sound
void fghthist_common_state::z80_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0xa000, 0xa001).rw(m_ym2151, FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xb000, 0xb000).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xc000, 0xc000).rw(m_oki[1], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xd000, 0xd000).r(m_ioprot, FUNC(deco_146_base_device::soundlatch_r));
}

void fghthist_common_state::z80_sound_io(address_map &map)
{
	map(0x0000, 0xffff).rom().region("audiocpu", 0);
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( fghthist )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )        PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 )        PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( tattass )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )        PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 )        PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED ) // 'soundmask'
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( nslasher )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )        PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 )        PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED ) // 'soundmask'
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(3)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(3)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 )        PORT_PLAYER(3)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START3 )
INPUT_PORTS_END


//**************************************************************************
//  GFXDECODE LAYOUTS
//**************************************************************************

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2), 8, 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8*2) },
	16*8    // every char takes 8 consecutive bytes
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2), 8, 0 },
	{ STEP8(16*8*2,1), STEP8(0,1) },
	{ STEP16(0,8*2) },
	64*8
};

static const gfx_layout tilelayout_5bpp =
{
	16,16,
	RGN_FRAC(1,1),
	5,
	{ 8*0, 8*1, 8*2, 8*3, 8*4 },
	{ STEP8(16*8*5,1), STEP8(0,1) },
	{ STEP16(0,8*5) },
	16*16*5
};

static GFXDECODE_START( gfx_fghthist )
	GFXDECODE_ENTRY( "tiles1", 0, charlayout,    0, 128 ) // Characters 8x8
	GFXDECODE_ENTRY( "tiles1", 0, tilelayout,    0, 128 ) // Tiles 16x16
	GFXDECODE_ENTRY( "tiles2", 0, tilelayout,    0, 128 ) // Tiles 16x16
GFXDECODE_END

static GFXDECODE_START( gfx_fghthist_spr )
	GFXDECODE_ENTRY( "sprites1", 0, tilelayout, 1024,  32 ) // Sprites 16x16
GFXDECODE_END

static GFXDECODE_START( gfx_nslasher )
	GFXDECODE_ENTRY( "tiles1", 0, charlayout,  0x800, 128 ) // Characters 8x8
	GFXDECODE_ENTRY( "tiles1", 0, tilelayout,      0, 128 ) // Tiles 16x16
	GFXDECODE_ENTRY( "tiles2", 0, tilelayout,      0, 128 ) // Tiles 16x16
GFXDECODE_END

static GFXDECODE_START( gfx_nslasher_spr1 )
	GFXDECODE_ENTRY( "sprites1", 0, tilelayout_5bpp, 0,  16 ) // Sprites 16x16
GFXDECODE_END

static GFXDECODE_START( gfx_nslasher_spr2 )
	GFXDECODE_ENTRY( "sprites2", 0, tilelayout,      0,  16 ) // Sprites 16x16
GFXDECODE_END


//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

void tattass_state::machine_start()
{
	nslasher_state::machine_start();

	save_item(NAME(m_tattass_eprom_bit));
	save_item(NAME(m_last_clock));
	save_item(NAME(m_buffer));
	save_item(NAME(m_buf_ptr));
	save_item(NAME(m_pending_command));
	save_item(NAME(m_read_bit_count));
	save_item(NAME(m_byte_addr));
}

// DE-0380-2
void fghthist_state::fghthisto(machine_config &config)
{
	ARM(config, m_maincpu, 28_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &fghthist_state::fghthisto_map);
	m_maincpu->set_vblank_int("screen", FUNC(fghthist_common_state::irq0_line_assert));

	h6280_device &audiocpu(H6280(config, m_audiocpu, 32.22_MHz_XTAL / 4 / 3)); // assume same as captaven
	audiocpu.set_addrmap(AS_PROGRAM, &fghthist_state::h6280_sound_custom_latch_map);
	audiocpu.add_route(ALL_OUTPUTS, "speaker", 0, 0); // internal sound unused
	audiocpu.add_route(ALL_OUTPUTS, "speaker", 0, 1);

	EEPROM_93C46_16BIT(config, m_eeprom);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(28_MHz_XTAL / 4, 442, 0, 320, 274, 8, 248);
	m_screen->set_screen_update(FUNC(fghthist_state::screen_update));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_fghthist);
	PALETTE(config, m_palette).set_entries(2048);

	DECO16IC(config, m_deco_tilegen[0]);
	m_deco_tilegen[0]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf1_col_bank(0x00);
	m_deco_tilegen[0]->set_pf2_col_bank(0x10);
	m_deco_tilegen[0]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[0]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[0]->set_bank1_callback(FUNC(fghthist_state::bank_callback));
	m_deco_tilegen[0]->set_bank2_callback(FUNC(fghthist_state::bank_callback));
	m_deco_tilegen[0]->set_pf12_8x8_bank(0);
	m_deco_tilegen[0]->set_pf12_16x16_bank(1);
	m_deco_tilegen[0]->set_gfxdecode_tag(m_gfxdecode);

	DECO16IC(config, m_deco_tilegen[1]);
	m_deco_tilegen[1]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf1_col_bank(0x20);
	m_deco_tilegen[1]->set_pf2_col_bank(0x30);
	m_deco_tilegen[1]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[1]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[1]->set_bank1_callback(FUNC(fghthist_state::bank_callback));
	m_deco_tilegen[1]->set_bank2_callback(FUNC(fghthist_state::bank_callback));
	m_deco_tilegen[1]->set_pf12_8x8_bank(0);
	m_deco_tilegen[1]->set_pf12_16x16_bank(2);
	m_deco_tilegen[1]->set_gfxdecode_tag(m_gfxdecode);

	DECO_SPRITE(config, m_sprgen[0], m_palette, gfx_fghthist_spr);
	m_sprgen[0]->set_pri_callback(FUNC(fghthist_state::fghthist_pri_callback));

	DECO146PROT(config, m_ioprot);
	m_ioprot->port_a_cb().set_ioport("IN0");
	m_ioprot->port_b_cb().set("eeprom", FUNC(eeprom_serial_93cxx_device::do_read)).lshift(0);
	m_ioprot->port_c_cb().set_ioport("IN1");
	m_ioprot->set_interface_scramble_interleave();
	m_ioprot->set_use_magic_read_address_xor(true);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, 0);

	YM2151(config, m_ym2151, 32.22_MHz_XTAL / 9);
	m_ym2151->irq_handler().set_inputline(m_audiocpu, 1);
	m_ym2151->port_write_handler().set(FUNC(fghthist_state::sound_bankswitch_w));
	m_ym2151->add_route(0, "speaker", 0.42, 0);
	m_ym2151->add_route(1, "speaker", 0.42, 1);

	OKIM6295(config, m_oki[0], 32.22_MHz_XTAL / 32, okim6295_device::PIN7_HIGH);
	m_oki[0]->add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	m_oki[0]->add_route(ALL_OUTPUTS, "speaker", 1.0, 1);

	OKIM6295(config, m_oki[1], 32.22_MHz_XTAL / 16, okim6295_device::PIN7_HIGH);
	m_oki[1]->add_route(ALL_OUTPUTS, "speaker", 0.35, 0);
	m_oki[1]->add_route(ALL_OUTPUTS, "speaker", 0.35, 1);
}

// DE-0395-1
void fghthist_state::fghthist(machine_config &config)
{
	fghthisto(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &fghthist_state::fghthist_map);
	m_audiocpu->set_addrmap(AS_PROGRAM, &fghthist_state::h6280_sound_map);

	config.device_remove("soundlatch");

	m_ioprot->soundlatch_irq_cb().set_inputline(m_audiocpu, 0);
}

// DE-0396-0
void fghthist_state::fghthistu(machine_config &config)
{
	fghthist(config);

	Z80(config.replace(), m_audiocpu, 32.22_MHz_XTAL / 9);
	m_audiocpu->set_addrmap(AS_PROGRAM, &fghthist_state::z80_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &fghthist_state::z80_sound_io);

	INPUT_MERGER_ANY_HIGH(config, "sound_irq_merger").output_handler().set_inputline(m_audiocpu, INPUT_LINE_IRQ0);

	m_ioprot->soundlatch_irq_cb().set("sound_irq_merger", FUNC(input_merger_any_high_device::in_w<0>));

	m_ym2151->irq_handler().set("sound_irq_merger", FUNC(input_merger_any_high_device::in_w<1>));
	m_ym2151->reset_routes();
	m_ym2151->add_route(0, "speaker", 0.40, 0);
	m_ym2151->add_route(1, "speaker", 0.40, 1);
}

void nslasher_state::nslasher(machine_config &config)
{
	// basic machine hardware
	ARM(config, m_maincpu, 28_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &nslasher_state::nslasher_map);
	m_maincpu->set_vblank_int("screen", FUNC(fghthist_common_state::irq0_line_assert));

	Z80(config, m_audiocpu, 32.22_MHz_XTAL / 9);
	m_audiocpu->set_addrmap(AS_PROGRAM, &nslasher_state::z80_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &nslasher_state::z80_sound_io);

	INPUT_MERGER_ANY_HIGH(config, "sound_irq_merger").output_handler().set_inputline("audiocpu", INPUT_LINE_IRQ0);

	config.set_maximum_quantum(attotime::from_hz(6000)); // to improve main<->audio comms

	EEPROM_93C46_16BIT(config, m_eeprom);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(28_MHz_XTAL / 4, 442, 0, 320, 274, 8, 248);
	m_screen->set_screen_update(FUNC(nslasher_state::screen_update_nslasher));

	DECO_ACE(config, m_deco_ace);

	DECO16IC(config, m_deco_tilegen[0]);
	m_deco_tilegen[0]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf1_col_bank(0x00);
	m_deco_tilegen[0]->set_pf2_col_bank(0x10);
	m_deco_tilegen[0]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[0]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[0]->set_bank1_callback(FUNC(nslasher_state::bank_callback));
	m_deco_tilegen[0]->set_bank2_callback(FUNC(nslasher_state::bank_callback));
	m_deco_tilegen[0]->set_pf12_8x8_bank(0);
	m_deco_tilegen[0]->set_pf12_16x16_bank(1);
	m_deco_tilegen[0]->set_gfxdecode_tag(m_gfxdecode);

	DECO16IC(config, m_deco_tilegen[1]);
	m_deco_tilegen[1]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf1_col_bank(0x20);
	m_deco_tilegen[1]->set_pf2_col_bank(0x30);
	m_deco_tilegen[1]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[1]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[1]->set_bank1_callback(FUNC(nslasher_state::bank_callback));
	m_deco_tilegen[1]->set_bank2_callback(FUNC(nslasher_state::bank_callback));
	m_deco_tilegen[1]->set_mix_callback(FUNC(nslasher_state::mix_callback));
	m_deco_tilegen[1]->set_pf12_8x8_bank(0);
	m_deco_tilegen[1]->set_pf12_16x16_bank(2);
	m_deco_tilegen[1]->set_gfxdecode_tag(m_gfxdecode);

	DECO_SPRITE(config, m_sprgen[0], m_deco_ace, gfx_nslasher_spr1);
	DECO_SPRITE(config, m_sprgen[1], m_deco_ace, gfx_nslasher_spr2);

	GFXDECODE(config, m_gfxdecode, m_deco_ace, gfx_nslasher);

	DECO104PROT(config, m_ioprot);
	m_ioprot->port_a_cb().set_ioport("IN0");
	m_ioprot->port_b_cb().set("eeprom", FUNC(eeprom_serial_93cxx_device::do_read)).lshift(0);
	m_ioprot->port_c_cb().set_ioport("IN1");
	m_ioprot->soundlatch_irq_cb().set("sound_irq_merger", FUNC(input_merger_any_high_device::in_w<0>));
	m_ioprot->set_interface_scramble_interleave();

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	YM2151(config, m_ym2151, 32.22_MHz_XTAL / 9);
	m_ym2151->irq_handler().set("sound_irq_merger", FUNC(input_merger_any_high_device::in_w<1>));
	m_ym2151->port_write_handler().set(FUNC(nslasher_state::sound_bankswitch_w));
	m_ym2151->add_route(0, "speaker", 0.40, 0);
	m_ym2151->add_route(1, "speaker", 0.40, 1);

	OKIM6295(config, m_oki[0], 32.22_MHz_XTAL / 32, okim6295_device::PIN7_HIGH);
	m_oki[0]->add_route(ALL_OUTPUTS, "speaker", 0.80, 0);
	m_oki[0]->add_route(ALL_OUTPUTS, "speaker", 0.80, 1);

	OKIM6295(config, m_oki[1], 32.22_MHz_XTAL / 16, okim6295_device::PIN7_HIGH);
	m_oki[1]->add_route(ALL_OUTPUTS, "speaker", 0.10, 0);
	m_oki[1]->add_route(ALL_OUTPUTS, "speaker", 0.10, 1);
}

// the US release uses a H6280 instead of a Z80, much like Lock 'n' Loaded
void nslasher_state::nslasheru(machine_config &config)
{
	nslasher(config);
	config.device_remove("audiocpu");

	h6280_device &audiocpu(H6280(config, m_audiocpu, 32.22_MHz_XTAL / 4 / 3)); // assume same as captaven
	audiocpu.set_addrmap(AS_PROGRAM, &nslasher_state::h6280_sound_map);
	audiocpu.add_route(ALL_OUTPUTS, "speaker", 0, 0); // internal sound unused
	audiocpu.add_route(ALL_OUTPUTS, "speaker", 0, 1);

	config.device_remove("sound_irq_merger");

	m_ym2151->irq_handler().set_inputline(m_audiocpu, 1);

	m_ioprot->soundlatch_irq_cb().set_inputline("audiocpu", 0);
}

void tattass_state::tattass(machine_config &config)
{
	// basic machine hardware
	ARM(config, m_maincpu, 28_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &tattass_state::tattass_map);
	m_maincpu->set_vblank_int("screen", FUNC(fghthist_common_state::irq0_line_assert));

	config.set_maximum_quantum(attotime::from_hz(6000)); // to improve main<->audio comms

	EEPROM_93C76_8BIT(config, m_eeprom);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(28_MHz_XTAL / 4, 442, 0, 320, 274, 8, 248);
	m_screen->set_screen_update(FUNC(tattass_state::screen_update_tattass));

	DECO_ACE(config, m_deco_ace);

	DECO16IC(config, m_deco_tilegen[0]);
	m_deco_tilegen[0]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf1_col_bank(0x00);
	m_deco_tilegen[0]->set_pf2_col_bank(0x10);
	m_deco_tilegen[0]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[0]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[0]->set_bank1_callback(FUNC(tattass_state::bank_callback));
	m_deco_tilegen[0]->set_bank2_callback(FUNC(tattass_state::bank_callback));
	m_deco_tilegen[0]->set_pf12_8x8_bank(0);
	m_deco_tilegen[0]->set_pf12_16x16_bank(1);
	m_deco_tilegen[0]->set_gfxdecode_tag(m_gfxdecode);

	DECO16IC(config, m_deco_tilegen[1]);
	m_deco_tilegen[1]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf1_col_bank(0x20);
	m_deco_tilegen[1]->set_pf2_col_bank(0x30);
	m_deco_tilegen[1]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[1]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[1]->set_bank1_callback(FUNC(tattass_state::bank_callback));
	m_deco_tilegen[1]->set_bank2_callback(FUNC(tattass_state::bank_callback));
	m_deco_tilegen[1]->set_mix_callback(FUNC(tattass_state::mix_callback));
	m_deco_tilegen[1]->set_pf12_8x8_bank(0);
	m_deco_tilegen[1]->set_pf12_16x16_bank(2);
	m_deco_tilegen[1]->set_gfxdecode_tag(m_gfxdecode);

	DECO_SPRITE(config, m_sprgen[0], m_deco_ace, gfx_nslasher_spr1);
	DECO_SPRITE(config, m_sprgen[1], m_deco_ace, gfx_nslasher_spr2);

	GFXDECODE(config, m_gfxdecode, m_deco_ace, gfx_nslasher);

	DECO104PROT(config, m_ioprot);
	m_ioprot->port_a_cb().set_ioport("IN0");
	m_ioprot->port_b_cb().set(FUNC(tattass_state::port_b_tattass));
	m_ioprot->port_c_cb().set_ioport("IN1");
	m_ioprot->soundlatch_irq_cb().set(FUNC(tattass_state::tattass_sound_irq_w));
	m_ioprot->set_interface_scramble_interleave();

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	DECOBSMT(config, m_decobsmt);
	m_decobsmt->add_route(0, "speaker", 1.0, 0);
	m_decobsmt->add_route(1, "speaker", 1.0, 1);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

// used for 5bpp gfxs
#define ROM_LOAD40_BYTE(name,offset,length,hash)        ROMX_LOAD(name, offset, length, hash, ROM_SKIP(4))
#define ROM_LOAD40_WORD(name,offset,length,hash)        ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_SKIP(3))
#define ROM_LOAD40_WORD_SWAP(name,offset,length,hash)   ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(3))

ROM_START( fghthist ) // DE-0395-1 PCB
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_WORD( "lc00-1.1f", 0x000000, 0x80000, CRC(61a76a16) SHA1(b69cd3e11cf133f1b14a017391035855a5038d46) ) // Version 43-09, Overseas
	ROM_LOAD32_WORD( "lc01-1.2f", 0x000002, 0x80000, CRC(6f2740d1) SHA1(4fa1fe4714236028ef70d42e15a58cfd25e45363) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "lc02-1.18k",  0x00000,  0x10000,  CRC(5fd2309c) SHA1(2fb7af54d5cd9bf7dd6fb4f6b82aa52b03294f1f) )

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "mbf00-8.8a",  0x000000,  0x100000,  CRC(d3e9b580) SHA1(fc4676e0ecc6c32441ff66fa1f990cc3158237db) ) // Encrypted tiles

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD( "mbf01-8.9a",  0x000000,  0x100000,  CRC(0c6ed2eb) SHA1(8e37ef4b1f0b6d3370a08758bfd602cb5f221282) ) // Encrypted tiles

	ROM_REGION( 0x800000, "sprites1", 0 ) // Sprites
	ROM_LOAD( "mbf02-16.16d",  0x000000,  0x200000,  CRC(c19c5953) SHA1(e6ed26f932c6c86bbd1fc4c000aa2f510c268009) )
	ROM_LOAD( "mbf03-16.17d",  0x200000,  0x200000,  CRC(37d25c75) SHA1(8219d31091b4317190618edd8acc49f97cba6a1e) )
	ROM_LOAD( "mbf04-16.18d",  0x400000,  0x200000,  CRC(f6a23fd7) SHA1(74e5559f17cd591aa25d2ed6c34ac9ed89e2e9ba) )
	ROM_LOAD( "mbf05-16.19d",  0x600000,  0x200000,  CRC(137be66d) SHA1(3fde345183ce04a7a65b4cedfd050d771df7d026) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbf06.15k",  0x000000,  0x80000,  CRC(fb513903) SHA1(7727a49ff7977f159ed36d097020edef3b5b36ba) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbf07.16k",  0x000000,  0x80000,  CRC(51d4adc7) SHA1(22106ed7a05db94adc5a783ce34529e29d24d41a) )

	ROM_REGION(512, "proms", 0 )
	ROM_LOAD( "kt-00.8j",  0,  512,  CRC(7294354b) SHA1(14fe42ad5d26d022c0fe9a46a4a9017af2296f40) ) // MB7124H type prom

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "ve-00.3d",  0x0000, 0x0117, CRC(384d316c) SHA1(61b50c695d4210c199cf6f7bbe50c8a5ecd1d21c) )
	ROM_LOAD( "ve-01a.4d", 0x0200, 0x0117, CRC(109613c8) SHA1(5991e010c1bc2a827c8ee2c85a9b40e00a3167b3) )
	// PAL16L8BCN at 8J is unpopulated
ROM_END

ROM_START( fghthista ) // DE-0380-2 PCB
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_WORD( "kx00-3.1f", 0x000000, 0x80000, CRC(fe5eaba1) SHA1(c8a3784af487a1bbd2150abf4b1c8f3ad33da8a4) ) // Version 43-07, Overseas
	ROM_LOAD32_WORD( "kx01-3.2f", 0x000002, 0x80000, CRC(3fb8d738) SHA1(2fca7a3ea483f01c97fb28a0adfa6d7980d8236c) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "kx02.18k",  0x00000,  0x10000,  CRC(5fd2309c) SHA1(2fb7af54d5cd9bf7dd6fb4f6b82aa52b03294f1f) )

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "mbf00-8.8a",  0x000000,  0x100000,  CRC(d3e9b580) SHA1(fc4676e0ecc6c32441ff66fa1f990cc3158237db) ) // Encrypted tiles

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD( "mbf01-8.9a",  0x000000,  0x100000,  CRC(0c6ed2eb) SHA1(8e37ef4b1f0b6d3370a08758bfd602cb5f221282) ) // Encrypted tiles

	ROM_REGION( 0x800000, "sprites1", 0 ) // Sprites
	ROM_LOAD( "mbf02-16.16d",  0x000000,  0x200000,  CRC(c19c5953) SHA1(e6ed26f932c6c86bbd1fc4c000aa2f510c268009) )
	ROM_LOAD( "mbf03-16.17d",  0x200000,  0x200000,  CRC(37d25c75) SHA1(8219d31091b4317190618edd8acc49f97cba6a1e) )
	ROM_LOAD( "mbf04-16.18d",  0x400000,  0x200000,  CRC(f6a23fd7) SHA1(74e5559f17cd591aa25d2ed6c34ac9ed89e2e9ba) )
	ROM_LOAD( "mbf05-16.19d",  0x600000,  0x200000,  CRC(137be66d) SHA1(3fde345183ce04a7a65b4cedfd050d771df7d026) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbf06.15k",  0x000000,  0x80000,  CRC(fb513903) SHA1(7727a49ff7977f159ed36d097020edef3b5b36ba) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbf07.16k",  0x000000,  0x80000,  CRC(51d4adc7) SHA1(22106ed7a05db94adc5a783ce34529e29d24d41a) )

	ROM_REGION(512, "proms", 0 )
	ROM_LOAD( "kt-00.8j",  0,  512,  CRC(7294354b) SHA1(14fe42ad5d26d022c0fe9a46a4a9017af2296f40) ) // MB7124H type prom

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "ve-00.3d",  0x0000, 0x0117, CRC(384d316c) SHA1(61b50c695d4210c199cf6f7bbe50c8a5ecd1d21c) )
	ROM_LOAD( "ve-01.4d",  0x0200, 0x0117, CRC(4ba7e6a9) SHA1(b65d696a3519e792df226f9f148c759cdb0e1e43) )
ROM_END

ROM_START( fghthistb ) // DE-0380-2 PCB
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_WORD( "kx00-2.1f", 0x000000, 0x80000, CRC(a7c36bbd) SHA1(590937818343da53a6bccbd3ea1d7102abd4f27e) ) // Version 43-05, Overseas
	ROM_LOAD32_WORD( "kx01-2.2f", 0x000002, 0x80000, CRC(bdc60bb1) SHA1(e621c5cf357f49aa62deef4da1e2227021f552ce) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "kx02.18k",  0x00000,  0x10000,  CRC(5fd2309c) SHA1(2fb7af54d5cd9bf7dd6fb4f6b82aa52b03294f1f) )

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "mbf00-8.8a",  0x000000,  0x100000,  CRC(d3e9b580) SHA1(fc4676e0ecc6c32441ff66fa1f990cc3158237db) ) // Encrypted tiles

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD( "mbf01-8.9a",  0x000000,  0x100000,  CRC(0c6ed2eb) SHA1(8e37ef4b1f0b6d3370a08758bfd602cb5f221282) ) // Encrypted tiles

	ROM_REGION( 0x800000, "sprites1", 0 ) // Sprites
	ROM_LOAD( "mbf02-16.16d",  0x000000,  0x200000,  CRC(c19c5953) SHA1(e6ed26f932c6c86bbd1fc4c000aa2f510c268009) )
	ROM_LOAD( "mbf03-16.17d",  0x200000,  0x200000,  CRC(37d25c75) SHA1(8219d31091b4317190618edd8acc49f97cba6a1e) )
	ROM_LOAD( "mbf04-16.18d",  0x400000,  0x200000,  CRC(f6a23fd7) SHA1(74e5559f17cd591aa25d2ed6c34ac9ed89e2e9ba) )
	ROM_LOAD( "mbf05-16.19d",  0x600000,  0x200000,  CRC(137be66d) SHA1(3fde345183ce04a7a65b4cedfd050d771df7d026) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbf06.15k",  0x000000,  0x80000,  CRC(fb513903) SHA1(7727a49ff7977f159ed36d097020edef3b5b36ba) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbf07.16k",  0x000000,  0x80000,  CRC(51d4adc7) SHA1(22106ed7a05db94adc5a783ce34529e29d24d41a) )

	ROM_REGION(512, "proms", 0 )
	ROM_LOAD( "kt-00.8j",  0,  512,  CRC(7294354b) SHA1(14fe42ad5d26d022c0fe9a46a4a9017af2296f40) ) // MB7124H type prom

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "ve-00.3d",  0x0000, 0x0117, CRC(384d316c) SHA1(61b50c695d4210c199cf6f7bbe50c8a5ecd1d21c) )
	ROM_LOAD( "ve-01.4d",  0x0200, 0x0117, CRC(4ba7e6a9) SHA1(b65d696a3519e792df226f9f148c759cdb0e1e43) )
	// PAL16L8BCN at 8J is unpopulated
ROM_END

ROM_START( fghthistu ) // DE-0396-0 PCB
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_WORD( "lj00-3.1f", 0x000000, 0x80000, CRC(17543d60) SHA1(ff206e8552587b41d075b3c99f9ad733f1c2b5e0) ) // Version 42-09, US
	ROM_LOAD32_WORD( "lj01-3.2f", 0x000002, 0x80000, CRC(e255d48f) SHA1(30444832cfed7eeb6082010eb219362adbafb826) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "lj02-.17k",  0x00000,  0x10000,  CRC(146a1063) SHA1(d16734c2443bf38add54040b9dd2628ba523638d) )

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "mbf00-8.8a",  0x000000,  0x100000,  CRC(d3e9b580) SHA1(fc4676e0ecc6c32441ff66fa1f990cc3158237db) ) // Encrypted tiles

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD( "mbf01-8.9a",  0x000000,  0x100000,  CRC(0c6ed2eb) SHA1(8e37ef4b1f0b6d3370a08758bfd602cb5f221282) ) // Encrypted tiles

	ROM_REGION( 0x800000, "sprites1", 0 ) // Sprites
	ROM_LOAD( "mbf02-16.16d",  0x000000,  0x200000,  CRC(c19c5953) SHA1(e6ed26f932c6c86bbd1fc4c000aa2f510c268009) )
	ROM_LOAD( "mbf03-16.17d",  0x200000,  0x200000,  CRC(37d25c75) SHA1(8219d31091b4317190618edd8acc49f97cba6a1e) )
	ROM_LOAD( "mbf04-16.18d",  0x400000,  0x200000,  CRC(f6a23fd7) SHA1(74e5559f17cd591aa25d2ed6c34ac9ed89e2e9ba) )
	ROM_LOAD( "mbf05-16.19d",  0x600000,  0x200000,  CRC(137be66d) SHA1(3fde345183ce04a7a65b4cedfd050d771df7d026) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbf06.15k",  0x000000,  0x80000,  CRC(fb513903) SHA1(7727a49ff7977f159ed36d097020edef3b5b36ba) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbf07.16k",  0x000000,  0x80000,  CRC(51d4adc7) SHA1(22106ed7a05db94adc5a783ce34529e29d24d41a) )

	ROM_REGION(512, "proms", 0 )
	ROM_LOAD( "kt-00.8j",  0,  512,  CRC(7294354b) SHA1(14fe42ad5d26d022c0fe9a46a4a9017af2296f40) ) // MB7124H type prom

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "ve-00.3d",  0x0000, 0x0117, CRC(384d316c) SHA1(61b50c695d4210c199cf6f7bbe50c8a5ecd1d21c) )
	ROM_LOAD( "ve-01.4d",  0x0200, 0x0117, CRC(4ba7e6a9) SHA1(b65d696a3519e792df226f9f148c759cdb0e1e43) )
	// PAL16L8BCN at 8J is unpopulated
ROM_END

ROM_START( fghthistua ) // DE-0395-1 PCB
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_WORD( "le00-1.1f", 0x000000, 0x80000, CRC(fccacafb) SHA1(b7236a90a09dbd5870a16aa4e4eac5ab5c098418) ) // Version 42-06, US
	ROM_LOAD32_WORD( "le01-1.2f", 0x000002, 0x80000, CRC(06a3c326) SHA1(3d8842fb69def93fc544e89fd0e56ada416157dc) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "le02.18k",  0x00000,  0x10000,  CRC(5fd2309c) SHA1(2fb7af54d5cd9bf7dd6fb4f6b82aa52b03294f1f) )

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "mbf00-8.8a",  0x000000,  0x100000,  CRC(d3e9b580) SHA1(fc4676e0ecc6c32441ff66fa1f990cc3158237db) ) // Encrypted tiles

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD( "mbf01-8.9a",  0x000000,  0x100000,  CRC(0c6ed2eb) SHA1(8e37ef4b1f0b6d3370a08758bfd602cb5f221282) ) // Encrypted tiles

	ROM_REGION( 0x800000, "sprites1", 0 ) // Sprites
	ROM_LOAD( "mbf02-16.16d",  0x000000,  0x200000,  CRC(c19c5953) SHA1(e6ed26f932c6c86bbd1fc4c000aa2f510c268009) )
	ROM_LOAD( "mbf03-16.17d",  0x200000,  0x200000,  CRC(37d25c75) SHA1(8219d31091b4317190618edd8acc49f97cba6a1e) )
	ROM_LOAD( "mbf04-16.18d",  0x400000,  0x200000,  CRC(f6a23fd7) SHA1(74e5559f17cd591aa25d2ed6c34ac9ed89e2e9ba) )
	ROM_LOAD( "mbf05-16.19d",  0x600000,  0x200000,  CRC(137be66d) SHA1(3fde345183ce04a7a65b4cedfd050d771df7d026) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbf06.15k",  0x000000,  0x80000,  CRC(fb513903) SHA1(7727a49ff7977f159ed36d097020edef3b5b36ba) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbf07.16k",  0x000000,  0x80000,  CRC(51d4adc7) SHA1(22106ed7a05db94adc5a783ce34529e29d24d41a) )

	ROM_REGION(512, "proms", 0 )
	ROM_LOAD( "kt-00.8j",  0,  512,  CRC(7294354b) SHA1(14fe42ad5d26d022c0fe9a46a4a9017af2296f40) ) // MB7124H type prom

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "ve-00.3d",  0x0000, 0x0117, CRC(384d316c) SHA1(61b50c695d4210c199cf6f7bbe50c8a5ecd1d21c) )
	ROM_LOAD( "ve-01a.4d", 0x0200, 0x0117, CRC(109613c8) SHA1(5991e010c1bc2a827c8ee2c85a9b40e00a3167b3) )
	// PAL16L8BCN at 8J is unpopulated
ROM_END

ROM_START( fghthistub ) // DE-0395-1 PCB
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_WORD( "le00.1f", 0x000000, 0x80000, CRC(a5c410eb) SHA1(e2b0cb2351782e1155ecc4029010beb7326fd874) ) // Version 42-05, US
	ROM_LOAD32_WORD( "le01.2f", 0x000002, 0x80000, CRC(7e148aa2) SHA1(b21e16604c4d29611f91d629deb9f041eaf41e9b) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "le02.18k",  0x00000,  0x10000,  CRC(5fd2309c) SHA1(2fb7af54d5cd9bf7dd6fb4f6b82aa52b03294f1f) )

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "mbf00-8.8a",  0x000000,  0x100000,  CRC(d3e9b580) SHA1(fc4676e0ecc6c32441ff66fa1f990cc3158237db) ) // Encrypted tiles

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD( "mbf01-8.9a",  0x000000,  0x100000,  CRC(0c6ed2eb) SHA1(8e37ef4b1f0b6d3370a08758bfd602cb5f221282) ) // Encrypted tiles

	ROM_REGION( 0x800000, "sprites1", 0 ) // Sprites
	ROM_LOAD( "mbf02-16.16d",  0x000000,  0x200000,  CRC(c19c5953) SHA1(e6ed26f932c6c86bbd1fc4c000aa2f510c268009) )
	ROM_LOAD( "mbf03-16.17d",  0x200000,  0x200000,  CRC(37d25c75) SHA1(8219d31091b4317190618edd8acc49f97cba6a1e) )
	ROM_LOAD( "mbf04-16.18d",  0x400000,  0x200000,  CRC(f6a23fd7) SHA1(74e5559f17cd591aa25d2ed6c34ac9ed89e2e9ba) )
	ROM_LOAD( "mbf05-16.19d",  0x600000,  0x200000,  CRC(137be66d) SHA1(3fde345183ce04a7a65b4cedfd050d771df7d026) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbf06.15k",  0x000000,  0x80000,  CRC(fb513903) SHA1(7727a49ff7977f159ed36d097020edef3b5b36ba) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbf07.16k",  0x000000,  0x80000,  CRC(51d4adc7) SHA1(22106ed7a05db94adc5a783ce34529e29d24d41a) )

	ROM_REGION(512, "proms", 0 )
	ROM_LOAD( "kt-00.8j",  0,  512,  CRC(7294354b) SHA1(14fe42ad5d26d022c0fe9a46a4a9017af2296f40) ) // MB7124H type prom

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "ve-00.3d",  0x0000, 0x0117, CRC(384d316c) SHA1(61b50c695d4210c199cf6f7bbe50c8a5ecd1d21c) )
	ROM_LOAD( "ve-01a.4d", 0x0200, 0x0117, CRC(109613c8) SHA1(5991e010c1bc2a827c8ee2c85a9b40e00a3167b3) )
	// PAL16L8BCN at 8J is unpopulated
ROM_END

ROM_START( fghthistuc ) // DE-0380-2 PCB
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_WORD( "kz00-1.1f", 0x000000, 0x80000, CRC(3a3dd15c) SHA1(689b51adf73402b12191a75061b8e709468c91bc) ) // Version 42-03, US
	ROM_LOAD32_WORD( "kz01-1.2f", 0x000002, 0x80000, CRC(86796cd6) SHA1(c397c07d7a1d03ba96ccb2fe7a0ad25b8331e945) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "kz02.18k",  0x00000,  0x10000,  CRC(5fd2309c) SHA1(2fb7af54d5cd9bf7dd6fb4f6b82aa52b03294f1f) )

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "mbf00-8.8a",  0x000000,  0x100000,  CRC(d3e9b580) SHA1(fc4676e0ecc6c32441ff66fa1f990cc3158237db) ) // Encrypted tiles

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD( "mbf01-8.9a",  0x000000,  0x100000,  CRC(0c6ed2eb) SHA1(8e37ef4b1f0b6d3370a08758bfd602cb5f221282) ) // Encrypted tiles

	ROM_REGION( 0x800000, "sprites1", 0 ) // Sprites
	ROM_LOAD( "mbf02-16.16d",  0x000000,  0x200000,  CRC(c19c5953) SHA1(e6ed26f932c6c86bbd1fc4c000aa2f510c268009) )
	ROM_LOAD( "mbf03-16.17d",  0x200000,  0x200000,  CRC(37d25c75) SHA1(8219d31091b4317190618edd8acc49f97cba6a1e) )
	ROM_LOAD( "mbf04-16.18d",  0x400000,  0x200000,  CRC(f6a23fd7) SHA1(74e5559f17cd591aa25d2ed6c34ac9ed89e2e9ba) )
	ROM_LOAD( "mbf05-16.19d",  0x600000,  0x200000,  CRC(137be66d) SHA1(3fde345183ce04a7a65b4cedfd050d771df7d026) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbf06.15k",  0x000000,  0x80000,  CRC(fb513903) SHA1(7727a49ff7977f159ed36d097020edef3b5b36ba) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbf07.16k",  0x000000,  0x80000,  CRC(51d4adc7) SHA1(22106ed7a05db94adc5a783ce34529e29d24d41a) )

	ROM_REGION(512, "proms", 0 )
	ROM_LOAD( "kt-00.8j",  0,  512,  CRC(7294354b) SHA1(14fe42ad5d26d022c0fe9a46a4a9017af2296f40) ) // MB7124H type prom

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "ve-00.3d",  0x0000, 0x0117, CRC(384d316c) SHA1(61b50c695d4210c199cf6f7bbe50c8a5ecd1d21c) )
	ROM_LOAD( "ve-01.4d",  0x0200, 0x0117, CRC(4ba7e6a9) SHA1(b65d696a3519e792df226f9f148c759cdb0e1e43) )
	// PAL16L8BCN at 8J is unpopulated
ROM_END

ROM_START( fghthistj ) // DE-0395-1 PCB
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_WORD( "lb00.1f", 0x000000, 0x80000, CRC(321099ad) SHA1(c5f8cedc1d349fb24b0d7b942dcda02190b1b536) ) // Version 41-07, Japan
	ROM_LOAD32_WORD( "lb01.2f", 0x000002, 0x80000, CRC(22f45755) SHA1(02ba35b557085e379be98705ca5395b677a264fd) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "lb02.18k",  0x00000,  0x10000,  CRC(5fd2309c) SHA1(2fb7af54d5cd9bf7dd6fb4f6b82aa52b03294f1f) )

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "mbf00-8.8a",  0x000000,  0x100000,  CRC(d3e9b580) SHA1(fc4676e0ecc6c32441ff66fa1f990cc3158237db) ) // Encrypted tiles

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD( "mbf01-8.9a",  0x000000,  0x100000,  CRC(0c6ed2eb) SHA1(8e37ef4b1f0b6d3370a08758bfd602cb5f221282) ) // Encrypted tiles

	ROM_REGION( 0x800000, "sprites1", 0 ) // Sprites
	ROM_LOAD( "mbf02-16.16d",  0x000000,  0x200000,  CRC(c19c5953) SHA1(e6ed26f932c6c86bbd1fc4c000aa2f510c268009) )
	ROM_LOAD( "mbf03-16.17d",  0x200000,  0x200000,  CRC(37d25c75) SHA1(8219d31091b4317190618edd8acc49f97cba6a1e) )
	ROM_LOAD( "mbf04-16.18d",  0x400000,  0x200000,  CRC(f6a23fd7) SHA1(74e5559f17cd591aa25d2ed6c34ac9ed89e2e9ba) )
	ROM_LOAD( "mbf05-16.19d",  0x600000,  0x200000,  CRC(137be66d) SHA1(3fde345183ce04a7a65b4cedfd050d771df7d026) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbf06.15k",  0x000000,  0x80000,  CRC(fb513903) SHA1(7727a49ff7977f159ed36d097020edef3b5b36ba) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbf07.16k",  0x000000,  0x80000,  CRC(51d4adc7) SHA1(22106ed7a05db94adc5a783ce34529e29d24d41a) )

	ROM_REGION(512, "proms", 0 )
	ROM_LOAD( "kt-00.8j",  0,  512,  CRC(7294354b) SHA1(14fe42ad5d26d022c0fe9a46a4a9017af2296f40) ) // MB7124H type prom

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "ve-00.3d",  0x0000, 0x0117, CRC(384d316c) SHA1(61b50c695d4210c199cf6f7bbe50c8a5ecd1d21c) )
	ROM_LOAD( "ve-01a.4d", 0x0200, 0x0117, CRC(109613c8) SHA1(5991e010c1bc2a827c8ee2c85a9b40e00a3167b3) )
	// PAL16L8BCN at 8J is unpopulated
ROM_END

ROM_START( fghthistja ) // DE-0380-2 PCB
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_WORD( "kw00-3.1f", 0x000000, 0x80000, CRC(ade9581a) SHA1(c1302e921f119ff9baeb52f9c338df652e64a9ee) ) // Version 41-05, Japan
	ROM_LOAD32_WORD( "kw01-3.2f", 0x000002, 0x80000, CRC(63580acf) SHA1(03372b168fe461542dd1cf64b4021d948d07e15c) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "kw02-.18k",  0x00000,  0x10000,  CRC(5fd2309c) SHA1(2fb7af54d5cd9bf7dd6fb4f6b82aa52b03294f1f) )

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "mbf00-8.8a",  0x000000,  0x100000,  CRC(d3e9b580) SHA1(fc4676e0ecc6c32441ff66fa1f990cc3158237db) ) // Encrypted tiles

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD( "mbf01-8.9a",  0x000000,  0x100000,  CRC(0c6ed2eb) SHA1(8e37ef4b1f0b6d3370a08758bfd602cb5f221282) ) // Encrypted tiles

	ROM_REGION( 0x800000, "sprites1", 0 ) // Sprites
	ROM_LOAD( "mbf02-16.16d",  0x000000,  0x200000,  CRC(c19c5953) SHA1(e6ed26f932c6c86bbd1fc4c000aa2f510c268009) )
	ROM_LOAD( "mbf03-16.17d",  0x200000,  0x200000,  CRC(37d25c75) SHA1(8219d31091b4317190618edd8acc49f97cba6a1e) )
	ROM_LOAD( "mbf04-16.18d",  0x400000,  0x200000,  CRC(f6a23fd7) SHA1(74e5559f17cd591aa25d2ed6c34ac9ed89e2e9ba) )
	ROM_LOAD( "mbf05-16.19d",  0x600000,  0x200000,  CRC(137be66d) SHA1(3fde345183ce04a7a65b4cedfd050d771df7d026) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbf06.15k",  0x000000,  0x80000,  CRC(fb513903) SHA1(7727a49ff7977f159ed36d097020edef3b5b36ba) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbf07.16k",  0x000000,  0x80000,  CRC(51d4adc7) SHA1(22106ed7a05db94adc5a783ce34529e29d24d41a) )

	ROM_REGION(512, "proms", 0 )
	ROM_LOAD( "kt-00.8j",  0,  512,  CRC(7294354b) SHA1(14fe42ad5d26d022c0fe9a46a4a9017af2296f40) ) // MB7124H type prom

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "ve-00.3d",  0x0000, 0x0117, CRC(384d316c) SHA1(61b50c695d4210c199cf6f7bbe50c8a5ecd1d21c) )
	ROM_LOAD( "ve-01.4d",  0x0200, 0x0117, CRC(4ba7e6a9) SHA1(b65d696a3519e792df226f9f148c759cdb0e1e43) )
	// PAL16L8BCN at 8J is unpopulated
ROM_END

ROM_START( fghthistjb ) // DE-0380-1 PCB
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_WORD( "kw00-2.1f", 0x000000, 0x80000, CRC(f4749806) SHA1(acdbd19b350d5d8670db879c446633a991e28c05) ) // Version 41-04, Japan
	ROM_LOAD32_WORD( "kw01-2.2f", 0x000002, 0x80000, CRC(7e0ee66a) SHA1(d62321eb9942bfe8629010fabeb42356cf7dd4d6) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "kw02-.18k",  0x00000,  0x10000,  CRC(5fd2309c) SHA1(2fb7af54d5cd9bf7dd6fb4f6b82aa52b03294f1f) )

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "mbf00-8.8a",  0x000000,  0x100000,  CRC(d3e9b580) SHA1(fc4676e0ecc6c32441ff66fa1f990cc3158237db) ) // Encrypted tiles

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD( "mbf01-8.9a",  0x000000,  0x100000,  CRC(0c6ed2eb) SHA1(8e37ef4b1f0b6d3370a08758bfd602cb5f221282) ) // Encrypted tiles

	ROM_REGION( 0x800000, "sprites1", 0 ) // Sprites
	ROM_LOAD( "mbf02-16.16d",  0x000000,  0x200000,  CRC(c19c5953) SHA1(e6ed26f932c6c86bbd1fc4c000aa2f510c268009) )
	ROM_LOAD( "mbf03-16.17d",  0x200000,  0x200000,  CRC(37d25c75) SHA1(8219d31091b4317190618edd8acc49f97cba6a1e) )
	ROM_LOAD( "mbf04-16.18d",  0x400000,  0x200000,  CRC(f6a23fd7) SHA1(74e5559f17cd591aa25d2ed6c34ac9ed89e2e9ba) )
	ROM_LOAD( "mbf05-16.19d",  0x600000,  0x200000,  CRC(137be66d) SHA1(3fde345183ce04a7a65b4cedfd050d771df7d026) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbf06.15k",  0x000000,  0x80000,  CRC(fb513903) SHA1(7727a49ff7977f159ed36d097020edef3b5b36ba) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbf07.16k",  0x000000,  0x80000,  CRC(51d4adc7) SHA1(22106ed7a05db94adc5a783ce34529e29d24d41a) )

	ROM_REGION(512, "proms", 0 )
	ROM_LOAD( "kt-00.8j",  0,  512,  CRC(7294354b) SHA1(14fe42ad5d26d022c0fe9a46a4a9017af2296f40) ) // MB7124H type prom

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "ve-00.3d",  0x0000, 0x0117, CRC(384d316c) SHA1(61b50c695d4210c199cf6f7bbe50c8a5ecd1d21c) )
	ROM_LOAD( "ve-01.4d",  0x0200, 0x0117, CRC(4ba7e6a9) SHA1(b65d696a3519e792df226f9f148c759cdb0e1e43) )
	// PAL16L8BCN at 8J is unpopulated
ROM_END

ROM_START( tattass )
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_WORD( "pp44.cpu", 0x000000, 0x80000, CRC(c3ca5b49) SHA1(c6420b0c20df1ae166b279504880ade65b1d8048) )
	ROM_LOAD32_WORD( "pp45.cpu", 0x000002, 0x80000, CRC(d3f30de0) SHA1(5a0aa0f96d29299b3b337b4b51bc84e447eb74d0) )

	ROM_REGION(0x10000, "decobsmt:soundcpu", 0 ) // Sound CPU
	ROM_LOAD( "u7.snd",  0x00000, 0x10000,  CRC(a7228077) SHA1(4d01575e51958a4d4686968008f5ed0a46380d70) )

	ROM_REGION( 0x200000, "tiles1", 0 )
	ROM_LOAD16_BYTE( "abak_b01.s02",  0x000000, 0x80000,  CRC(bc805680) SHA1(ccdbca23fc843ef82a3524020999542f43b3c618) )
	ROM_LOAD16_BYTE( "abak_b01.s13",  0x000001, 0x80000,  CRC(350effcd) SHA1(0452d95be9fc28bd00d846a2cc5828899d69601e) )
	ROM_LOAD16_BYTE( "abak_b23.s02",  0x100000, 0x80000,  CRC(91abdc21) SHA1(ba08e59bc0417e863d35ea295cf58cfe8faf57b5) )
	ROM_LOAD16_BYTE( "abak_b23.s13",  0x100001, 0x80000,  CRC(80eb50fe) SHA1(abfe1a5417ceff9d6d52372d11993bf9b1db9432) )

	ROM_REGION( 0x200000, "tiles2", 0 )
	ROM_LOAD16_BYTE( "bbak_b01.s02",  0x000000, 0x80000,  CRC(611be9a6) SHA1(86263c8beb562e0607a65aa30fbbe030a044cd75) )
	ROM_LOAD16_BYTE( "bbak_b01.s13",  0x000001, 0x80000,  CRC(097e0604) SHA1(6ae241b37b6bb15fc66679cf66f500b8f8a19f44) )
	ROM_LOAD16_BYTE( "bbak_b23.s02",  0x100000, 0x80000,  CRC(3836531a) SHA1(57bead820ac396ee0ed8fb2ac5c15929896d75bf) )
	ROM_LOAD16_BYTE( "bbak_b23.s13",  0x100001, 0x80000,  CRC(1210485a) SHA1(9edc4c96f389e231066ef164a7b2851cd7ade038) )

	ROM_REGION( 0xa00000, "sprites1", 0 )
	ROM_LOAD40_BYTE( "ob1_c0.b0",  0x000004, 0x80000,  CRC(053fecca) SHA1(319efc71042238d9012d2c3dddab9d11205decc6) )
	ROM_LOAD40_BYTE( "ob1_c1.b0",  0x000003, 0x80000,  CRC(e183e6bc) SHA1(d9cce277861967f403a882879e1baefa84696bdc) )
	ROM_LOAD40_BYTE( "ob1_c2.b0",  0x000002, 0x80000,  CRC(1314f828) SHA1(6a91543d4e70af30de287ba775c69ffb1cde719d) )
	ROM_LOAD40_BYTE( "ob1_c3.b0",  0x000001, 0x80000,  CRC(c63866df) SHA1(a897835d8a33002f1bd54f27d1a6393c4e1864b9) )
	ROM_LOAD40_BYTE( "ob1_c4.b0",  0x000000, 0x80000,  CRC(f71cdd1b) SHA1(6fbccdbe460c8ddfeed972ebe766a6f8a2d4c466) )

	ROM_LOAD40_BYTE( "ob1_c0.b1",  0x280004, 0x80000,  CRC(385434b0) SHA1(ea764bd9844e13f5b10207022135dbe07bf0258a) )
	ROM_LOAD40_BYTE( "ob1_c1.b1",  0x280003, 0x80000,  CRC(0a3ec489) SHA1(1a2e1252d6acda43019ded5a31ae60bef40e4bd9) )
	ROM_LOAD40_BYTE( "ob1_c2.b1",  0x280002, 0x80000,  CRC(52f06081) SHA1(c630f45b110b9423dfb0bf92359fdb28b75c8cf1) )
	ROM_LOAD40_BYTE( "ob1_c3.b1",  0x280001, 0x80000,  CRC(a8a5cfbe) SHA1(7afc8f7c7f3826a276e4840e4fc8b8bb645dd3bd) )
	ROM_LOAD40_BYTE( "ob1_c4.b1",  0x280000, 0x80000,  CRC(09d0acd6) SHA1(1b162f5b76852e49ae6a24db2031d66ca59d87e9) )

	ROM_LOAD40_BYTE( "ob1_c0.b2",  0x500004, 0x80000,  CRC(946e9f59) SHA1(46a0d35641b381fe553caa00451c30f1950b5dfd) )
	ROM_LOAD40_BYTE( "ob1_c1.b2",  0x500003, 0x80000,  CRC(9f66ad54) SHA1(6e6ac6edee2f2dda46e7cd85db8d79c8335c73cd) )
	ROM_LOAD40_BYTE( "ob1_c2.b2",  0x500002, 0x80000,  CRC(a8df60eb) SHA1(c971e66eec6accccaf2bdd87dde7adde79322da9) )
	ROM_LOAD40_BYTE( "ob1_c3.b2",  0x500001, 0x80000,  CRC(a1a753be) SHA1(1666a32bb69db36dba029a835592d00a21ad8c5e) )
	ROM_LOAD40_BYTE( "ob1_c4.b2",  0x500000, 0x80000,  CRC(b65b3c4b) SHA1(f636a682b506e3ce5ca07ba8fd3166158d1ab667) )

	ROM_LOAD40_BYTE( "ob1_c0.b3",  0x780004, 0x80000,  CRC(cbbbc696) SHA1(6f2383655461ac35f3178e0f7c0146cff89c8295) )
	ROM_LOAD40_BYTE( "ob1_c1.b3",  0x780003, 0x80000,  CRC(f7b1bdee) SHA1(1d505d8d4ede55246de0b5fbc6ca20f836699b60) )
	ROM_LOAD40_BYTE( "ob1_c2.b3",  0x780002, 0x80000,  CRC(97815619) SHA1(b1b694310064971aa5438671d0f9992b7e4bf277) )
	ROM_LOAD40_BYTE( "ob1_c3.b3",  0x780001, 0x80000,  CRC(fc3ccb7a) SHA1(4436fcbd830912589bd6c838eb63b7d41a2bb56e) )
	ROM_LOAD40_BYTE( "ob1_c4.b3",  0x780000, 0x80000,  CRC(dfdfd0ff) SHA1(79dc686351d41d635359936efe97c7ade305dc84) )

	ROM_REGION( 0x800000, "sprites2", 0 )
	ROM_LOAD16_BYTE( "ob2_c0.b0",  0x000000, 0x80000,  CRC(9080ebe4) SHA1(1cfabe045532e16f203fe054449149451a280f56) )
	ROM_LOAD16_BYTE( "ob2_c1.b0",  0x000001, 0x80000,  CRC(c0464970) SHA1(2bd87c9a7ed0742a8f1ee0c0de225e18a0351168) )
	ROM_LOAD16_BYTE( "ob2_c2.b0",  0x400000, 0x80000,  CRC(35a2e621) SHA1(ff7687e30c379cbcee4f80c0c737cef891509881) )
	ROM_LOAD16_BYTE( "ob2_c3.b0",  0x400001, 0x80000,  CRC(99c7cc2d) SHA1(c761e5b7f1e2afdafef36390f7141ebcb5e8f254) )
	ROM_LOAD16_BYTE( "ob2_c0.b1",  0x100000, 0x80000,  CRC(2c2c15c9) SHA1(fdc48fab6dad97d16d4e77479fa77bb320eb3767) )
	ROM_LOAD16_BYTE( "ob2_c1.b1",  0x100001, 0x80000,  CRC(d2c49a14) SHA1(49d92233d6d5f77fbbf9d31607c568efef6d94f0) )
	ROM_LOAD16_BYTE( "ob2_c2.b1",  0x500000, 0x80000,  CRC(fbe957e8) SHA1(4f0bb0e434771316bcd8796878ffd3e5cafebb2b) )
	ROM_LOAD16_BYTE( "ob2_c3.b1",  0x500001, 0x80000,  CRC(d7238829) SHA1(6fef08a518be69251852d3204413b4b8b6615be2) )
	ROM_LOAD16_BYTE( "ob2_c0.b2",  0x200000, 0x80000,  CRC(aefa1b01) SHA1(bbd4b432b36d64f80065c56559ea9675acf3151e) )
	ROM_LOAD16_BYTE( "ob2_c1.b2",  0x200001, 0x80000,  CRC(4af620ca) SHA1(f3753235b2e72f011c9b94f26a425b9a79577201) )
	ROM_LOAD16_BYTE( "ob2_c2.b2",  0x600000, 0x80000,  CRC(8e58be07) SHA1(d8a8662e800da0892d70c628de0ca27ff983006c) )
	ROM_LOAD16_BYTE( "ob2_c3.b2",  0x600001, 0x80000,  CRC(1b5188c5) SHA1(4792a36b889a2c2dfab9ec78d848d3d8bf10d20f) )
	ROM_LOAD16_BYTE( "ob2_c0.b3",  0x300000, 0x80000,  CRC(a2a5dafd) SHA1(2baadcfe9ae8fa30ae4226caa10fe3d58f8af3e0) )
	ROM_LOAD16_BYTE( "ob2_c1.b3",  0x300001, 0x80000,  CRC(6f0afd05) SHA1(6a4bf3466a77d14b3bc18377537f86108774badd) )
	ROM_LOAD16_BYTE( "ob2_c2.b3",  0x700000, 0x80000,  CRC(90fe5f4f) SHA1(2149e9eae152556c632ebd4d0b2de49e40916a77) )
	ROM_LOAD16_BYTE( "ob2_c3.b3",  0x700001, 0x80000,  CRC(e3517e6e) SHA1(68ac60570423d8f0d7cff3db1901c9c050d0be91) )

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD( "u17.snd",  0x000000, 0x80000,  CRC(49303539) SHA1(3c4b6bbdcb039e355b66f72ed5ffe0bbd363f179) )
	ROM_LOAD( "u21.snd",  0x080000, 0x80000,  CRC(0ad8bc18) SHA1(b06654e682080b3baeb7872d3c4eb3a04b22d79b) )
	ROM_LOAD( "u36.snd",  0x100000, 0x80000,  CRC(f558947b) SHA1(57c0dc2e23beb388c1cf10a175be7dabf07da458) )
	ROM_LOAD( "u37.snd",  0x180000, 0x80000,  CRC(7a3190bc) SHA1(e78b77268e5aa2879e7dcb5e8ff9e5dbed228cb1) )

	ROM_REGION( 0x400, "eeprom", 0 )
	ROM_LOAD( "eeprom-tattass.bin", 0x0000, 0x0400, CRC(7140f40c) SHA1(4fb7897933046b6adaf00b4626d5fd23d0e8a666) )
ROM_END

ROM_START( tattasso )
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_WORD( "pp44.cpu", 0x000000, 0x80000, CRC(c3ca5b49) SHA1(c6420b0c20df1ae166b279504880ade65b1d8048) )
	ROM_LOAD32_WORD( "pp45.cpu", 0x000002, 0x80000, CRC(d3f30de0) SHA1(5a0aa0f96d29299b3b337b4b51bc84e447eb74d0) )

	ROM_REGION(0x10000, "decobsmt:soundcpu", 0 ) // Sound CPU
	ROM_LOAD( "u7.snd",  0x00000, 0x10000,  CRC(6947be8a) SHA1(4ac6c3c7f54501f23c434708cea6bf327bc8cf95) )

	ROM_REGION( 0x200000, "tiles1", 0 )
	ROM_LOAD16_BYTE( "abak_b01.s02",  0x000000, 0x80000,  CRC(bc805680) SHA1(ccdbca23fc843ef82a3524020999542f43b3c618) )
	ROM_LOAD16_BYTE( "abak_b01.s13",  0x000001, 0x80000,  CRC(350effcd) SHA1(0452d95be9fc28bd00d846a2cc5828899d69601e) )
	ROM_LOAD16_BYTE( "abak_b23.s02",  0x100000, 0x80000,  CRC(91abdc21) SHA1(ba08e59bc0417e863d35ea295cf58cfe8faf57b5) )
	ROM_LOAD16_BYTE( "abak_b23.s13",  0x100001, 0x80000,  CRC(80eb50fe) SHA1(abfe1a5417ceff9d6d52372d11993bf9b1db9432) )

	ROM_REGION( 0x200000, "tiles2", 0 )
	ROM_LOAD16_BYTE( "bbak_b01.s02",  0x000000, 0x80000,  CRC(611be9a6) SHA1(86263c8beb562e0607a65aa30fbbe030a044cd75) )
	ROM_LOAD16_BYTE( "bbak_b01.s13",  0x000001, 0x80000,  CRC(097e0604) SHA1(6ae241b37b6bb15fc66679cf66f500b8f8a19f44) )
	ROM_LOAD16_BYTE( "bbak_b23.s02",  0x100000, 0x80000,  CRC(3836531a) SHA1(57bead820ac396ee0ed8fb2ac5c15929896d75bf) )
	ROM_LOAD16_BYTE( "bbak_b23.s13",  0x100001, 0x80000,  CRC(1210485a) SHA1(9edc4c96f389e231066ef164a7b2851cd7ade038) )

	ROM_REGION( 0xa00000, "sprites1", 0 )
	ROM_LOAD40_BYTE( "ob1_c0.b0",  0x000004, 0x80000,  CRC(053fecca) SHA1(319efc71042238d9012d2c3dddab9d11205decc6) )
	ROM_LOAD40_BYTE( "ob1_c1.b0",  0x000003, 0x80000,  CRC(e183e6bc) SHA1(d9cce277861967f403a882879e1baefa84696bdc) )
	ROM_LOAD40_BYTE( "ob1_c2.b0",  0x000002, 0x80000,  CRC(1314f828) SHA1(6a91543d4e70af30de287ba775c69ffb1cde719d) )
	ROM_LOAD40_BYTE( "ob1_c3.b0",  0x000001, 0x80000,  CRC(c63866df) SHA1(a897835d8a33002f1bd54f27d1a6393c4e1864b9) )
	ROM_LOAD40_BYTE( "ob1_c4.b0",  0x000000, 0x80000,  CRC(f71cdd1b) SHA1(6fbccdbe460c8ddfeed972ebe766a6f8a2d4c466) )

	ROM_LOAD40_BYTE( "ob1_c0.b1",  0x280004, 0x80000,  CRC(385434b0) SHA1(ea764bd9844e13f5b10207022135dbe07bf0258a) )
	ROM_LOAD40_BYTE( "ob1_c1.b1",  0x280003, 0x80000,  CRC(0a3ec489) SHA1(1a2e1252d6acda43019ded5a31ae60bef40e4bd9) )
	ROM_LOAD40_BYTE( "ob1_c2.b1",  0x280002, 0x80000,  CRC(52f06081) SHA1(c630f45b110b9423dfb0bf92359fdb28b75c8cf1) )
	ROM_LOAD40_BYTE( "ob1_c3.b1",  0x280001, 0x80000,  CRC(a8a5cfbe) SHA1(7afc8f7c7f3826a276e4840e4fc8b8bb645dd3bd) )
	ROM_LOAD40_BYTE( "ob1_c4.b1",  0x280000, 0x80000,  CRC(09d0acd6) SHA1(1b162f5b76852e49ae6a24db2031d66ca59d87e9) )

	ROM_LOAD40_BYTE( "ob1_c0.b2",  0x500004, 0x80000,  CRC(946e9f59) SHA1(46a0d35641b381fe553caa00451c30f1950b5dfd) )
	ROM_LOAD40_BYTE( "ob1_c1.b2",  0x500003, 0x80000,  CRC(9f66ad54) SHA1(6e6ac6edee2f2dda46e7cd85db8d79c8335c73cd) )
	ROM_LOAD40_BYTE( "ob1_c2.b2",  0x500002, 0x80000,  CRC(a8df60eb) SHA1(c971e66eec6accccaf2bdd87dde7adde79322da9) )
	ROM_LOAD40_BYTE( "ob1_c3.b2",  0x500001, 0x80000,  CRC(a1a753be) SHA1(1666a32bb69db36dba029a835592d00a21ad8c5e) )
	ROM_LOAD40_BYTE( "ob1_c4.b2",  0x500000, 0x80000,  CRC(b65b3c4b) SHA1(f636a682b506e3ce5ca07ba8fd3166158d1ab667) )

	ROM_LOAD40_BYTE( "ob1_c0.b3",  0x780004, 0x80000,  CRC(cbbbc696) SHA1(6f2383655461ac35f3178e0f7c0146cff89c8295) )
	ROM_LOAD40_BYTE( "ob1_c1.b3",  0x780003, 0x80000,  CRC(f7b1bdee) SHA1(1d505d8d4ede55246de0b5fbc6ca20f836699b60) )
	ROM_LOAD40_BYTE( "ob1_c2.b3",  0x780002, 0x80000,  CRC(97815619) SHA1(b1b694310064971aa5438671d0f9992b7e4bf277) )
	ROM_LOAD40_BYTE( "ob1_c3.b3",  0x780001, 0x80000,  CRC(fc3ccb7a) SHA1(4436fcbd830912589bd6c838eb63b7d41a2bb56e) )
	ROM_LOAD40_BYTE( "ob1_c4.b3",  0x780000, 0x80000,  CRC(dfdfd0ff) SHA1(79dc686351d41d635359936efe97c7ade305dc84) )

	ROM_REGION( 0x800000, "sprites2", 0 )
	ROM_LOAD16_BYTE( "ob2_c0.b0",  0x000000, 0x80000,  CRC(9080ebe4) SHA1(1cfabe045532e16f203fe054449149451a280f56) )
	ROM_LOAD16_BYTE( "ob2_c1.b0",  0x000001, 0x80000,  CRC(c0464970) SHA1(2bd87c9a7ed0742a8f1ee0c0de225e18a0351168) )
	ROM_LOAD16_BYTE( "ob2_c2.b0",  0x400000, 0x80000,  CRC(35a2e621) SHA1(ff7687e30c379cbcee4f80c0c737cef891509881) )
	ROM_LOAD16_BYTE( "ob2_c3.b0",  0x400001, 0x80000,  CRC(99c7cc2d) SHA1(c761e5b7f1e2afdafef36390f7141ebcb5e8f254) )
	ROM_LOAD16_BYTE( "ob2_c0.b1",  0x100000, 0x80000,  CRC(2c2c15c9) SHA1(fdc48fab6dad97d16d4e77479fa77bb320eb3767) )
	ROM_LOAD16_BYTE( "ob2_c1.b1",  0x100001, 0x80000,  CRC(d2c49a14) SHA1(49d92233d6d5f77fbbf9d31607c568efef6d94f0) )
	ROM_LOAD16_BYTE( "ob2_c2.b1",  0x500000, 0x80000,  CRC(fbe957e8) SHA1(4f0bb0e434771316bcd8796878ffd3e5cafebb2b) )
	ROM_LOAD16_BYTE( "ob2_c3.b1",  0x500001, 0x80000,  CRC(d7238829) SHA1(6fef08a518be69251852d3204413b4b8b6615be2) )
	ROM_LOAD16_BYTE( "ob2_c0.b2",  0x200000, 0x80000,  CRC(aefa1b01) SHA1(bbd4b432b36d64f80065c56559ea9675acf3151e) )
	ROM_LOAD16_BYTE( "ob2_c1.b2",  0x200001, 0x80000,  CRC(4af620ca) SHA1(f3753235b2e72f011c9b94f26a425b9a79577201) )
	ROM_LOAD16_BYTE( "ob2_c2.b2",  0x600000, 0x80000,  CRC(8e58be07) SHA1(d8a8662e800da0892d70c628de0ca27ff983006c) )
	ROM_LOAD16_BYTE( "ob2_c3.b2",  0x600001, 0x80000,  CRC(1b5188c5) SHA1(4792a36b889a2c2dfab9ec78d848d3d8bf10d20f) )
	ROM_LOAD16_BYTE( "ob2_c0.b3",  0x300000, 0x80000,  CRC(a2a5dafd) SHA1(2baadcfe9ae8fa30ae4226caa10fe3d58f8af3e0) )
	ROM_LOAD16_BYTE( "ob2_c1.b3",  0x300001, 0x80000,  CRC(6f0afd05) SHA1(6a4bf3466a77d14b3bc18377537f86108774badd) )
	ROM_LOAD16_BYTE( "ob2_c2.b3",  0x700000, 0x80000,  CRC(90fe5f4f) SHA1(2149e9eae152556c632ebd4d0b2de49e40916a77) )
	ROM_LOAD16_BYTE( "ob2_c3.b3",  0x700001, 0x80000,  CRC(e3517e6e) SHA1(68ac60570423d8f0d7cff3db1901c9c050d0be91) )

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD( "u17.snd",  0x000000, 0x80000,  CRC(b945c18d) SHA1(6556bbb4a7057df3680132f24687fa944006c784) )
	ROM_LOAD( "u21.snd",  0x080000, 0x80000,  CRC(10b2110c) SHA1(83e5938ed22da2874022e1dc8df76c72d95c448d) )
	ROM_LOAD( "u36.snd",  0x100000, 0x80000,  CRC(3b73abe2) SHA1(195096e2302e84123b23b4ccd982fb3ab9afe42c) )
	ROM_LOAD( "u37.snd",  0x180000, 0x80000,  CRC(986066b5) SHA1(9dd1a14de81733617cf51293674a8e26fc5cec68) )

	ROM_REGION( 0x400, "eeprom", 0 )
	ROM_LOAD( "eeprom-tattass.bin", 0x0000, 0x0400, CRC(7140f40c) SHA1(4fb7897933046b6adaf00b4626d5fd23d0e8a666) )
ROM_END

ROM_START( tattassa )
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_WORD( "rev232a.000", 0x000000, 0x80000, CRC(1a357112) SHA1(d7f78f90970fd56ca1452a4c138168568b06d868) )
	ROM_LOAD32_WORD( "rev232a.001", 0x000002, 0x80000, CRC(550245d4) SHA1(c1b2b31768da9becebd907a8622d05aa68ecaa29) )

	ROM_REGION(0x10000, "decobsmt:soundcpu", 0 ) // Sound CPU
	ROM_LOAD( "u7.snd",  0x00000, 0x10000,  CRC(a7228077) SHA1(4d01575e51958a4d4686968008f5ed0a46380d70) )

	ROM_REGION( 0x200000, "tiles1", 0 )
	ROM_LOAD16_BYTE( "abak_b01.s02",  0x000000, 0x80000,  CRC(bc805680) SHA1(ccdbca23fc843ef82a3524020999542f43b3c618) )
	ROM_LOAD16_BYTE( "abak_b01.s13",  0x000001, 0x80000,  CRC(350effcd) SHA1(0452d95be9fc28bd00d846a2cc5828899d69601e) )
	ROM_LOAD16_BYTE( "abak_b23.s02",  0x100000, 0x80000,  CRC(91abdc21) SHA1(ba08e59bc0417e863d35ea295cf58cfe8faf57b5) )
	ROM_LOAD16_BYTE( "abak_b23.s13",  0x100001, 0x80000,  CRC(80eb50fe) SHA1(abfe1a5417ceff9d6d52372d11993bf9b1db9432) )

	ROM_REGION( 0x200000, "tiles2", 0 )
	ROM_LOAD16_BYTE( "bbak_b01.s02",  0x000000, 0x80000,  CRC(611be9a6) SHA1(86263c8beb562e0607a65aa30fbbe030a044cd75) )
	ROM_LOAD16_BYTE( "bbak_b01.s13",  0x000001, 0x80000,  CRC(097e0604) SHA1(6ae241b37b6bb15fc66679cf66f500b8f8a19f44) )
	ROM_LOAD16_BYTE( "bbak_b23.s02",  0x100000, 0x80000,  CRC(3836531a) SHA1(57bead820ac396ee0ed8fb2ac5c15929896d75bf) )
	ROM_LOAD16_BYTE( "bbak_b23.s13",  0x100001, 0x80000,  CRC(1210485a) SHA1(9edc4c96f389e231066ef164a7b2851cd7ade038) )

	ROM_REGION( 0xa00000, "sprites1", 0 )
	ROM_LOAD40_BYTE( "ob1_c0.b0",  0x000004, 0x80000,  CRC(053fecca) SHA1(319efc71042238d9012d2c3dddab9d11205decc6) )
	ROM_LOAD40_BYTE( "ob1_c1.b0",  0x000003, 0x80000,  CRC(e183e6bc) SHA1(d9cce277861967f403a882879e1baefa84696bdc) )
	ROM_LOAD40_BYTE( "ob1_c2.b0",  0x000002, 0x80000,  CRC(1314f828) SHA1(6a91543d4e70af30de287ba775c69ffb1cde719d) )
	ROM_LOAD40_BYTE( "ob1_c3.b0",  0x000001, 0x80000,  CRC(c63866df) SHA1(a897835d8a33002f1bd54f27d1a6393c4e1864b9) )
	ROM_LOAD40_BYTE( "ob1_c4.b0",  0x000000, 0x80000,  CRC(f71cdd1b) SHA1(6fbccdbe460c8ddfeed972ebe766a6f8a2d4c466) )

	ROM_LOAD40_BYTE( "ob1_c0.b1",  0x280004, 0x80000,  CRC(385434b0) SHA1(ea764bd9844e13f5b10207022135dbe07bf0258a) )
	ROM_LOAD40_BYTE( "ob1_c1.b1",  0x280003, 0x80000,  CRC(0a3ec489) SHA1(1a2e1252d6acda43019ded5a31ae60bef40e4bd9) )
	ROM_LOAD40_BYTE( "ob1_c2.b1",  0x280002, 0x80000,  CRC(52f06081) SHA1(c630f45b110b9423dfb0bf92359fdb28b75c8cf1) )
	ROM_LOAD40_BYTE( "ob1_c3.b1",  0x280001, 0x80000,  CRC(a8a5cfbe) SHA1(7afc8f7c7f3826a276e4840e4fc8b8bb645dd3bd) )
	ROM_LOAD40_BYTE( "ob1_c4.b1",  0x280000, 0x80000,  CRC(09d0acd6) SHA1(1b162f5b76852e49ae6a24db2031d66ca59d87e9) )

	ROM_LOAD40_BYTE( "ob1_c0.b2",  0x500004, 0x80000,  CRC(946e9f59) SHA1(46a0d35641b381fe553caa00451c30f1950b5dfd) )
	ROM_LOAD40_BYTE( "ob1_c1.b2",  0x500003, 0x80000,  CRC(9f66ad54) SHA1(6e6ac6edee2f2dda46e7cd85db8d79c8335c73cd) )
	ROM_LOAD40_BYTE( "ob1_c2.b2",  0x500002, 0x80000,  CRC(a8df60eb) SHA1(c971e66eec6accccaf2bdd87dde7adde79322da9) )
	ROM_LOAD40_BYTE( "ob1_c3.b2",  0x500001, 0x80000,  CRC(a1a753be) SHA1(1666a32bb69db36dba029a835592d00a21ad8c5e) )
	ROM_LOAD40_BYTE( "ob1_c4.b2",  0x500000, 0x80000,  CRC(b65b3c4b) SHA1(f636a682b506e3ce5ca07ba8fd3166158d1ab667) )

	ROM_LOAD40_BYTE( "ob1_c0.b3",  0x780004, 0x80000,  CRC(cbbbc696) SHA1(6f2383655461ac35f3178e0f7c0146cff89c8295) )
	ROM_LOAD40_BYTE( "ob1_c1.b3",  0x780003, 0x80000,  CRC(f7b1bdee) SHA1(1d505d8d4ede55246de0b5fbc6ca20f836699b60) )
	ROM_LOAD40_BYTE( "ob1_c2.b3",  0x780002, 0x80000,  CRC(97815619) SHA1(b1b694310064971aa5438671d0f9992b7e4bf277) )
	ROM_LOAD40_BYTE( "ob1_c3.b3",  0x780001, 0x80000,  CRC(fc3ccb7a) SHA1(4436fcbd830912589bd6c838eb63b7d41a2bb56e) )
	ROM_LOAD40_BYTE( "ob1_c4.b3",  0x780000, 0x80000,  CRC(dfdfd0ff) SHA1(79dc686351d41d635359936efe97c7ade305dc84) )

	ROM_REGION( 0x800000, "sprites2", 0 )
	ROM_LOAD16_BYTE( "ob2_c0.b0",  0x000000, 0x80000,  CRC(9080ebe4) SHA1(1cfabe045532e16f203fe054449149451a280f56) )
	ROM_LOAD16_BYTE( "ob2_c1.b0",  0x000001, 0x80000,  CRC(c0464970) SHA1(2bd87c9a7ed0742a8f1ee0c0de225e18a0351168) )
	ROM_LOAD16_BYTE( "ob2_c2.b0",  0x400000, 0x80000,  CRC(35a2e621) SHA1(ff7687e30c379cbcee4f80c0c737cef891509881) )
	ROM_LOAD16_BYTE( "ob2_c3.b0",  0x400001, 0x80000,  CRC(99c7cc2d) SHA1(c761e5b7f1e2afdafef36390f7141ebcb5e8f254) )
	ROM_LOAD16_BYTE( "ob2_c0.b1",  0x100000, 0x80000,  CRC(2c2c15c9) SHA1(fdc48fab6dad97d16d4e77479fa77bb320eb3767) )
	ROM_LOAD16_BYTE( "ob2_c1.b1",  0x100001, 0x80000,  CRC(d2c49a14) SHA1(49d92233d6d5f77fbbf9d31607c568efef6d94f0) )
	ROM_LOAD16_BYTE( "ob2_c2.b1",  0x500000, 0x80000,  CRC(fbe957e8) SHA1(4f0bb0e434771316bcd8796878ffd3e5cafebb2b) )
	ROM_LOAD16_BYTE( "ob2_c3.b1",  0x500001, 0x80000,  CRC(d7238829) SHA1(6fef08a518be69251852d3204413b4b8b6615be2) )
	ROM_LOAD16_BYTE( "ob2_c0.b2",  0x200000, 0x80000,  CRC(aefa1b01) SHA1(bbd4b432b36d64f80065c56559ea9675acf3151e) )
	ROM_LOAD16_BYTE( "ob2_c1.b2",  0x200001, 0x80000,  CRC(4af620ca) SHA1(f3753235b2e72f011c9b94f26a425b9a79577201) )
	ROM_LOAD16_BYTE( "ob2_c2.b2",  0x600000, 0x80000,  CRC(8e58be07) SHA1(d8a8662e800da0892d70c628de0ca27ff983006c) )
	ROM_LOAD16_BYTE( "ob2_c3.b2",  0x600001, 0x80000,  CRC(1b5188c5) SHA1(4792a36b889a2c2dfab9ec78d848d3d8bf10d20f) )
	ROM_LOAD16_BYTE( "ob2_c0.b3",  0x300000, 0x80000,  CRC(a2a5dafd) SHA1(2baadcfe9ae8fa30ae4226caa10fe3d58f8af3e0) )
	ROM_LOAD16_BYTE( "ob2_c1.b3",  0x300001, 0x80000,  CRC(6f0afd05) SHA1(6a4bf3466a77d14b3bc18377537f86108774badd) )
	ROM_LOAD16_BYTE( "ob2_c2.b3",  0x700000, 0x80000,  CRC(90fe5f4f) SHA1(2149e9eae152556c632ebd4d0b2de49e40916a77) )
	ROM_LOAD16_BYTE( "ob2_c3.b3",  0x700001, 0x80000,  CRC(e3517e6e) SHA1(68ac60570423d8f0d7cff3db1901c9c050d0be91) )

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD( "u17.snd",  0x000000, 0x80000,  CRC(49303539) SHA1(3c4b6bbdcb039e355b66f72ed5ffe0bbd363f179) )
	ROM_LOAD( "u21.snd",  0x080000, 0x80000,  CRC(0ad8bc18) SHA1(b06654e682080b3baeb7872d3c4eb3a04b22d79b) )
	ROM_LOAD( "u36.snd",  0x100000, 0x80000,  CRC(f558947b) SHA1(57c0dc2e23beb388c1cf10a175be7dabf07da458) )
	ROM_LOAD( "u37.snd",  0x180000, 0x80000,  CRC(7a3190bc) SHA1(e78b77268e5aa2879e7dcb5e8ff9e5dbed228cb1) )

	ROM_REGION( 0x400, "eeprom", 0 )
	ROM_LOAD( "eeprom-tattass.bin", 0x0000, 0x0400, CRC(7140f40c) SHA1(4fb7897933046b6adaf00b4626d5fd23d0e8a666) )
ROM_END

ROM_START( nslasher ) // DE-0397-0 PCB
	ROM_REGION(0x100000, "maincpu", 0 ) // Encrypted ARM 32 bit code
	ROM_LOAD32_WORD( "mainprg.1f", 0x000000, 0x80000, CRC(507acbae) SHA1(329a2bb244f2f3adb8d75cab5aa2dcb129d70712) )
	ROM_LOAD32_WORD( "mainprg.2f", 0x000002, 0x80000, CRC(931fc7ee) SHA1(54eb12abfa3f332ce9b43a45ec424aaee88641a6) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "sndprg.17l",  0x00000,  0x10000,  CRC(18939e92) SHA1(50b37a78d9d2259d4b140dd17393c4e5ca92bca5) )

	ROM_REGION( 0x200000, "tiles1", 0 )
	ROM_LOAD( "mbh-00.8c",  0x000000,  0x200000,  CRC(a877f8a3) SHA1(79253525f360a73161894f31e211e4d6b38d307a) ) // Encrypted tiles

	ROM_REGION( 0x200000, "tiles2", 0 )
	ROM_LOAD( "mbh-01.9c",  0x000000,  0x200000,  CRC(1853dafc) SHA1(b1183c0db301cbed9f079c782202dbfc553b198e) ) // Encrypted tiles

	ROM_REGION( 0x640000, "sprites1", 0 ) // Sprites
	ROM_LOAD40_WORD_SWAP( "mbh-02.14c",  0x000003,  0x200000, CRC(b2f158a1) SHA1(4f8c0b324813db198fe1dad7fff4185b828b94de) )
	ROM_LOAD40_WORD_SWAP( "mbh-04.16c",  0x000001,  0x200000, CRC(eecfe06d) SHA1(2df817fe5e2ea31207b217bb03dc58979c05d0d2) )
	ROM_LOAD40_BYTE(      "mbh-06.18c",  0x000000,  0x100000, CRC(038c2127) SHA1(5bdb215305f1a419fde27a83b623a38b9328e560) )
	ROM_LOAD40_WORD_SWAP( "mbh-03.15c",  0x500003,  0x080000, CRC(787787e3) SHA1(531444e3f28aa9a7539a5a76ca94a9d6b97274c5) )
	ROM_LOAD40_WORD_SWAP( "mbh-05.17c",  0x500001,  0x080000, CRC(1d2b7c17) SHA1(ae0b8e0448a1a8180fb424fb0bc8a4302f8ff602) )
	ROM_LOAD40_BYTE(      "mbh-07.19c",  0x500000,  0x040000, CRC(bbd22323) SHA1(6ab665b2e6d04cdadc48c52e15098e978b61fe10) )

	ROM_REGION( 0x100000, "sprites2", 0 ) // Sprites
	ROM_LOAD( "mbh-08.16e",  0x000000,  0x80000,  CRC(cdd7f8cb) SHA1(910bbe8783c0ba722e9d6399b332d658fa059fdb) )
	ROM_LOAD( "mbh-09.18e",  0x080000,  0x80000,  CRC(33fa2121) SHA1(eb0e99d29b1ad9995df28e5b7cfc89d53efb53c3) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbh-10.14l", 0x000000,  0x80000,  CRC(c4d6b116) SHA1(c5685bce6a6c6a74ca600ebf766ba1007f0dc666) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbh-11.16l", 0x000000,  0x80000,  CRC(0ec40b6b) SHA1(9fef44149608ae2a00f6a75a6f77f2efcab6e78e) )

	ROM_REGION(0x200, "prom", 0 )
	ROM_LOAD( "ln-00.j7", 0x000000,  0x200, CRC(5e83eaf3) SHA1(95f5eb8e56dff6c2dce7c39a6dd458bfc38fe1cf) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "vm-00.3d",  0x0000, 0x0117, NO_DUMP ) // 16L8ACN is read protected
	ROM_LOAD( "vm-01.4d",  0x0200, 0x0117, NO_DUMP ) // 16L8ACN is read protected
	ROM_LOAD( "vm-02.8j",  0x0400, 0x0117, CRC(53692426) SHA1(b8f8cf6b1f6b637fcd1fcd62474e637f5d4a6901) )
ROM_END

ROM_START( nslasherj ) // DE-0397-0 PCB
	ROM_REGION(0x100000, "maincpu", 0 ) // Encrypted ARM 32 bit code
	ROM_LOAD32_WORD( "lx-00.1f", 0x000000, 0x80000, CRC(6ed5fb88) SHA1(84350da7939a479968a523c84e254e3ee54b8da2) )
	ROM_LOAD32_WORD( "lx-01.2f", 0x000002, 0x80000, CRC(a6df2152) SHA1(6fe7e0b2e71c5f807951dcc81a6a3cff55247961) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "sndprg.17l",  0x00000,  0x10000,  CRC(18939e92) SHA1(50b37a78d9d2259d4b140dd17393c4e5ca92bca5) )

	ROM_REGION( 0x200000, "tiles1", 0 )
	ROM_LOAD( "mbh-00.8c",  0x000000,  0x200000,  CRC(a877f8a3) SHA1(79253525f360a73161894f31e211e4d6b38d307a) ) // Encrypted tiles

	ROM_REGION( 0x200000, "tiles2", 0 )
	ROM_LOAD( "mbh-01.9c",  0x000000,  0x200000,  CRC(1853dafc) SHA1(b1183c0db301cbed9f079c782202dbfc553b198e) ) // Encrypted tiles

	ROM_REGION( 0x640000, "sprites1", 0 ) // Sprites
	ROM_LOAD40_WORD_SWAP( "mbh-02.14c",  0x000003,  0x200000, CRC(b2f158a1) SHA1(4f8c0b324813db198fe1dad7fff4185b828b94de) )
	ROM_LOAD40_WORD_SWAP( "mbh-04.16c",  0x000001,  0x200000, CRC(eecfe06d) SHA1(2df817fe5e2ea31207b217bb03dc58979c05d0d2) )
	ROM_LOAD40_BYTE(      "mbh-06.18c",  0x000000,  0x100000, CRC(038c2127) SHA1(5bdb215305f1a419fde27a83b623a38b9328e560) )
	ROM_LOAD40_WORD_SWAP( "mbh-03.15c",  0x500003,  0x080000, CRC(787787e3) SHA1(531444e3f28aa9a7539a5a76ca94a9d6b97274c5) )
	ROM_LOAD40_WORD_SWAP( "mbh-05.17c",  0x500001,  0x080000, CRC(1d2b7c17) SHA1(ae0b8e0448a1a8180fb424fb0bc8a4302f8ff602) )
	ROM_LOAD40_BYTE(      "mbh-07.19c",  0x500000,  0x040000, CRC(bbd22323) SHA1(6ab665b2e6d04cdadc48c52e15098e978b61fe10) )

	ROM_REGION( 0x100000, "sprites2", 0 ) // Sprites
	ROM_LOAD( "mbh-08.16e",  0x000000,  0x80000,  CRC(cdd7f8cb) SHA1(910bbe8783c0ba722e9d6399b332d658fa059fdb) )
	ROM_LOAD( "mbh-09.18e",  0x080000,  0x80000,  CRC(33fa2121) SHA1(eb0e99d29b1ad9995df28e5b7cfc89d53efb53c3) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbh-10.14l", 0x000000,  0x80000,  CRC(c4d6b116) SHA1(c5685bce6a6c6a74ca600ebf766ba1007f0dc666) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbh-11.16l", 0x000000,  0x80000,  CRC(0ec40b6b) SHA1(9fef44149608ae2a00f6a75a6f77f2efcab6e78e) )

	ROM_REGION(0x200, "prom", 0 )
	ROM_LOAD( "ln-00.j7", 0x000000,  0x200, CRC(5e83eaf3) SHA1(95f5eb8e56dff6c2dce7c39a6dd458bfc38fe1cf) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "vm-00.3d",  0x0000, 0x0117, NO_DUMP ) // 16L8ACN is read protected
	ROM_LOAD( "vm-01.4d",  0x0200, 0x0117, NO_DUMP ) // 16L8ACN is read protected
	ROM_LOAD( "vm-02.8j",  0x0400, 0x0117, CRC(53692426) SHA1(b8f8cf6b1f6b637fcd1fcd62474e637f5d4a6901) )
ROM_END

ROM_START( nslashers ) // DE-0397-0 PCB
	ROM_REGION(0x100000, "maincpu", 0 ) // Encrypted ARM 32 bit code
	ROM_LOAD32_WORD( "ly-00.1f", 0x000000, 0x80000, CRC(fa0646f9) SHA1(7f9633bda230a0ced59171cdc5ab40a6d56c3d34) )
	ROM_LOAD32_WORD( "ly-01.2f", 0x000002, 0x80000, CRC(ae508149) SHA1(3592949e5fb2770adb9c9daa4e38c4e75f3e2554) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "sndprg.17l",  0x00000,  0x10000,  CRC(18939e92) SHA1(50b37a78d9d2259d4b140dd17393c4e5ca92bca5) )

	ROM_REGION( 0x200000, "tiles1", 0 )
	ROM_LOAD( "mbh-00.8c",  0x000000,  0x200000,  CRC(a877f8a3) SHA1(79253525f360a73161894f31e211e4d6b38d307a) ) // Encrypted tiles

	ROM_REGION( 0x200000, "tiles2", 0 )
	ROM_LOAD( "mbh-01.9c",  0x000000,  0x200000,  CRC(1853dafc) SHA1(b1183c0db301cbed9f079c782202dbfc553b198e) ) // Encrypted tiles

	ROM_REGION( 0x640000, "sprites1", 0 ) // Sprites
	ROM_LOAD40_WORD_SWAP( "mbh-02.14c",  0x000003,  0x200000, CRC(b2f158a1) SHA1(4f8c0b324813db198fe1dad7fff4185b828b94de) )
	ROM_LOAD40_WORD_SWAP( "mbh-04.16c",  0x000001,  0x200000, CRC(eecfe06d) SHA1(2df817fe5e2ea31207b217bb03dc58979c05d0d2) )
	ROM_LOAD40_BYTE(      "mbh-06.18c",  0x000000,  0x100000, CRC(038c2127) SHA1(5bdb215305f1a419fde27a83b623a38b9328e560) )
	ROM_LOAD40_WORD_SWAP( "mbh-03.15c",  0x500003,  0x080000, CRC(787787e3) SHA1(531444e3f28aa9a7539a5a76ca94a9d6b97274c5) )
	ROM_LOAD40_WORD_SWAP( "mbh-05.17c",  0x500001,  0x080000, CRC(1d2b7c17) SHA1(ae0b8e0448a1a8180fb424fb0bc8a4302f8ff602) )
	ROM_LOAD40_BYTE(      "mbh-07.19c",  0x500000,  0x040000, CRC(bbd22323) SHA1(6ab665b2e6d04cdadc48c52e15098e978b61fe10) )

	ROM_REGION( 0x100000, "sprites2", 0 ) // Sprites
	ROM_LOAD( "mbh-08.16e",  0x000000,  0x80000,  CRC(cdd7f8cb) SHA1(910bbe8783c0ba722e9d6399b332d658fa059fdb) )
	ROM_LOAD( "mbh-09.18e",  0x080000,  0x80000,  CRC(33fa2121) SHA1(eb0e99d29b1ad9995df28e5b7cfc89d53efb53c3) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbh-10.14l", 0x000000,  0x80000,  CRC(c4d6b116) SHA1(c5685bce6a6c6a74ca600ebf766ba1007f0dc666) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbh-11.16l", 0x000000,  0x80000,  CRC(0ec40b6b) SHA1(9fef44149608ae2a00f6a75a6f77f2efcab6e78e) )

	ROM_REGION(0x200, "prom", 0 )
	ROM_LOAD( "ln-00.j7", 0x000000,  0x200, CRC(5e83eaf3) SHA1(95f5eb8e56dff6c2dce7c39a6dd458bfc38fe1cf) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "vm-00.3d",  0x0000, 0x0117, NO_DUMP ) // 16L8ACN is read protected
	ROM_LOAD( "vm-01.4d",  0x0200, 0x0117, NO_DUMP ) // 16L8ACN is read protected
	ROM_LOAD( "vm-02.8j",  0x0400, 0x0117, CRC(53692426) SHA1(b8f8cf6b1f6b637fcd1fcd62474e637f5d4a6901) )
ROM_END

ROM_START( nslasheru ) // DE-0395-1 PCB
	ROM_REGION(0x100000, "maincpu", 0 ) // Encrypted ARM 32 bit code
	ROM_LOAD32_WORD( "00.f1", 0x000000, 0x80000, CRC(944f3329) SHA1(7e7909e203b9752de3d3d798c6f84ac6ae824a07) )
	ROM_LOAD32_WORD( "01.f2", 0x000002, 0x80000, CRC(ac12d18a) SHA1(7cd4e843bf575c70c5c39a8afa78b803106f59b0) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "02.l18",  0x00000,  0x10000, CRC(5e63bd91) SHA1(a6ac3c8c50f44cf2e6cf029aef1c974d1fc16ed5) )

	ROM_REGION( 0x200000, "tiles1", 0 )
	ROM_LOAD( "mbh-00.8c",  0x000000,  0x200000,  CRC(a877f8a3) SHA1(79253525f360a73161894f31e211e4d6b38d307a) ) // Encrypted tiles

	ROM_REGION( 0x200000, "tiles2", 0 )
	ROM_LOAD( "mbh-01.9c",  0x000000,  0x200000,  CRC(1853dafc) SHA1(b1183c0db301cbed9f079c782202dbfc553b198e) ) // Encrypted tiles

	ROM_REGION( 0x640000, "sprites1", 0 ) // Sprites
	ROM_LOAD40_WORD_SWAP( "mbh-02.14c",  0x000003,  0x200000, CRC(b2f158a1) SHA1(4f8c0b324813db198fe1dad7fff4185b828b94de) )
	ROM_LOAD40_WORD_SWAP( "mbh-04.16c",  0x000001,  0x200000, CRC(eecfe06d) SHA1(2df817fe5e2ea31207b217bb03dc58979c05d0d2) )
	ROM_LOAD40_BYTE(      "mbh-06.18c",  0x000000,  0x100000, CRC(038c2127) SHA1(5bdb215305f1a419fde27a83b623a38b9328e560) )
	ROM_LOAD40_WORD_SWAP( "mbh-03.15c",  0x500003,  0x080000, CRC(787787e3) SHA1(531444e3f28aa9a7539a5a76ca94a9d6b97274c5) )
	ROM_LOAD40_WORD_SWAP( "mbh-05.17c",  0x500001,  0x080000, CRC(1d2b7c17) SHA1(ae0b8e0448a1a8180fb424fb0bc8a4302f8ff602) )
	ROM_LOAD40_BYTE(      "mbh-07.19c",  0x500000,  0x040000, CRC(bbd22323) SHA1(6ab665b2e6d04cdadc48c52e15098e978b61fe10) )

	ROM_REGION( 0x100000, "sprites2", 0 ) // Sprites
	ROM_LOAD( "mbh-08.16e",  0x000000,  0x80000,  CRC(cdd7f8cb) SHA1(910bbe8783c0ba722e9d6399b332d658fa059fdb) )
	ROM_LOAD( "mbh-09.18e",  0x080000,  0x80000,  CRC(33fa2121) SHA1(eb0e99d29b1ad9995df28e5b7cfc89d53efb53c3) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbh-10.14l", 0x000000,  0x80000,  CRC(c4d6b116) SHA1(c5685bce6a6c6a74ca600ebf766ba1007f0dc666) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbh-11.16l", 0x000000,  0x80000,  CRC(0ec40b6b) SHA1(9fef44149608ae2a00f6a75a6f77f2efcab6e78e) )

	ROM_REGION(0x200, "prom", 0 )
	ROM_LOAD( "ln-00.j7", 0x000000,  0x200, CRC(5e83eaf3) SHA1(95f5eb8e56dff6c2dce7c39a6dd458bfc38fe1cf) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "ve-00.3d",  0x0000, 0x0117, CRC(384d316c) SHA1(61b50c695d4210c199cf6f7bbe50c8a5ecd1d21c) )
	ROM_LOAD( "ve-01a.4d", 0x0200, 0x0117, CRC(109613c8) SHA1(5991e010c1bc2a827c8ee2c85a9b40e00a3167b3) )
	ROM_LOAD( "vm-02.8j",  0x0400, 0x0117, CRC(53692426) SHA1(b8f8cf6b1f6b637fcd1fcd62474e637f5d4a6901) )
ROM_END


//**************************************************************************
//  INITIALIZERS
//**************************************************************************

void fghthist_state::init_fghthist()
{
	deco56_decrypt_gfx(machine(), "tiles1");
	deco74_decrypt_gfx(machine(), "tiles2");
}

void nslasher_state::reorder_tilegfx()
{
	u8 *RAM = memregion("tiles1")->base();
	std::vector<u8> tmp(0x80000);

	// Reorder bitplanes to make decoding easier
	std::copy(&RAM[0x080000], &RAM[0x100000], tmp.begin());
	std::copy(&RAM[0x100000], &RAM[0x180000], &RAM[0x080000]);
	std::copy(tmp.begin(),    tmp.end(),      &RAM[0x100000]);

	RAM = memregion("tiles2")->base();
	std::copy(&RAM[0x080000], &RAM[0x100000], tmp.begin());
	std::copy(&RAM[0x100000], &RAM[0x180000], &RAM[0x080000]);
	std::copy(tmp.begin(),    tmp.end(),      &RAM[0x100000]);

}

void tattass_state::init_tattass()
{
	reorder_tilegfx();
	deco56_decrypt_gfx(machine(), "tiles1"); // 141
	deco56_decrypt_gfx(machine(), "tiles2"); // 141
}

void nslasher_state::init_nslasher()
{
	reorder_tilegfx();
	deco56_decrypt_gfx(machine(), "tiles1"); // 141
	deco74_decrypt_gfx(machine(), "tiles2");

	deco156_decrypt(machine());

	// The board for Night Slashers is very close to the Fighter's History and
	// Tattoo Assassins boards, but has an encrypted ARM cpu.
}


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME        PARENT    MACHINE    INPUT     CLASS           INIT            ROT   COMPANY                  FULLNAME                                              FLAGS
// DE-0396-0 PCB sets (Z80 for sound)
GAME( 1993, fghthistu,  fghthist, fghthistu, fghthist, fghthist_state, init_fghthist,  ROT0, "Data East Corporation", "Fighter's History (US Version 42-09, DE-0396-0 PCB)",       MACHINE_SUPPORTS_SAVE )
// DE-0395-1 PCB sets (HuC6280 for sound)
GAME( 1993, fghthist,   0,        fghthist,  fghthist, fghthist_state, init_fghthist,  ROT0, "Data East Corporation", "Fighter's History (World Version 43-09, DE-0395-1 PCB)",    MACHINE_SUPPORTS_SAVE )
GAME( 1993, fghthistua, fghthist, fghthist,  fghthist, fghthist_state, init_fghthist,  ROT0, "Data East Corporation", "Fighter's History (US Version 42-06, DE-0395-1 PCB)",       MACHINE_SUPPORTS_SAVE )
GAME( 1993, fghthistub, fghthist, fghthist,  fghthist, fghthist_state, init_fghthist,  ROT0, "Data East Corporation", "Fighter's History (US Version 42-05, DE-0395-1 PCB)",       MACHINE_SUPPORTS_SAVE )
GAME( 1993, fghthistj,  fghthist, fghthist,  fghthist, fghthist_state, init_fghthist,  ROT0, "Data East Corporation", "Fighter's History (Japan Version 41-07, DE-0395-1 PCB)",    MACHINE_SUPPORTS_SAVE )
// DE-0380-2 PCB sets (HuC6280 for sound)
GAME( 1993, fghthista,  fghthist, fghthisto, fghthist, fghthist_state, init_fghthist,  ROT0, "Data East Corporation", "Fighter's History (World Version 43-07, DE-0380-2 PCB)",    MACHINE_SUPPORTS_SAVE )
GAME( 1993, fghthistb,  fghthist, fghthisto, fghthist, fghthist_state, init_fghthist,  ROT0, "Data East Corporation", "Fighter's History (World Version 43-05, DE-0380-2 PCB)",    MACHINE_SUPPORTS_SAVE )
GAME( 1993, fghthistuc, fghthist, fghthisto, fghthist, fghthist_state, init_fghthist,  ROT0, "Data East Corporation", "Fighter's History (US Version 42-03, DE-0380-2 PCB)",       MACHINE_SUPPORTS_SAVE )
GAME( 1993, fghthistja, fghthist, fghthisto, fghthist, fghthist_state, init_fghthist,  ROT0, "Data East Corporation", "Fighter's History (Japan Version 41-05, DE-0380-2 PCB)",    MACHINE_SUPPORTS_SAVE )
// DE-0380-1 PCB sets (HuC6280 for sound)
GAME( 1993, fghthistjb, fghthist, fghthisto, fghthist, fghthist_state, init_fghthist,  ROT0, "Data East Corporation", "Fighter's History (Japan Version 41-04, DE-0380-1 PCB)",    MACHINE_SUPPORTS_SAVE )

// DE-0397-0 PCB sets (Z80 for sound)
GAME( 1994, nslasher,   0,        nslasher,  nslasher, nslasher_state, init_nslasher,  ROT0, "Data East Corporation", "Night Slashers (Korea Ver. 1.3, DE-0397-0 PCB)",            MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1994, nslasherj,  nslasher, nslasher,  nslasher, nslasher_state, init_nslasher,  ROT0, "Data East Corporation", "Night Slashers (Japan Ver. 1.2, DE-0397-0 PCB)",            MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1994, nslashers,  nslasher, nslasher,  nslasher, nslasher_state, init_nslasher,  ROT0, "Data East Corporation", "Night Slashers (World Ver. 1.2, DE-0397-0 PCB)",            MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
// DE-0395-1 PCB sets (HuC6280 for sound)
GAME( 1994, nslasheru,  nslasher, nslasheru, nslasher, nslasher_state, init_nslasher,  ROT0, "Data East Corporation", "Night Slashers (US Ver. 1.2, DE-0395-1 PCB)",               MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

GAME( 1994, tattass,    0,        tattass,   tattass,  tattass_state,  init_tattass,   ROT0, "Data East Pinball",     "Tattoo Assassins (US prototype, Mar 14 1995)",              MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1994, tattasso,   tattass,  tattass,   tattass,  tattass_state,  init_tattass,   ROT0, "Data East Pinball",     "Tattoo Assassins (US prototype, Mar 14 1995, older sound)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1994, tattassa,   tattass,  tattass,   tattass,  tattass_state,  init_tattass,   ROT0, "Data East Pinball",     "Tattoo Assassins (Asia prototype, Mar 14 1995)",            MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
