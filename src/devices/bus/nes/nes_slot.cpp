// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 Nintendo NES/FC cart emulation
 (through slot devices)

 The driver exposes address ranges
 0x4100-0x5fff to read_l/write_l
 0x6000-0x7fff to read_m/write_m (here are *usually* installed NVRAM & WRAM, if any)
 0x8000-0xffff to write_h (reads are directed to 4 x 8K PRG banks)
 Default implementations of these handlers are available here, to be rewritten by PCB-specific ones when needed.

 Additional handlers are available, but have to be manually installed at machine_start
 * read_ex/write_ex for address range 0x4020-0x40ff
 * read_h for address range 0x8000-0xffff when a cart does some protection or address scramble before reading ROM

 PPU exposes address ranges
 0x0000-0x1fff to chr_r/chr_w
 0x2000-0x3eff to nt_r/nt_w
 Default implementations of these handlers are available here, to be rewritten by PCB-specific ones when needed.

 Plus a few of latch functions are available: ppu_latch (see MMC2), hblank_irq and scanline_irq (see e.g. MMC3),
 but these might be subject to future changes when the PPU is revisited.

 Notes:
 - Differently from later systems (like SNES or MD), it is uncommon to find PRG ROMs of NES games which are not a
   power of 2K, so we do not perform any mirroring by default.
   A bunch of pcb types, though, come with 1.5MB of PRG (some Waixing translations) or with multiple PRG chips
   having peculiar size (32K + 16K, 32K + 8K, 32K + 2K). Hence, if such a configuration is detected, we provide
   a m_prg_bank_map array to handle internally PRG mirroring up to the next power of 2K, as long as the size is
   a multiple of 8K (i.e. the unit chunk for standard PRG).
   For the case of PRG chips which are not-multiple of 8K (e.g. UNL-MARIO2-MALEE pcb), the handling has to be
   handled in the pcb-specific code!
 - Our open bus emulation is very sketchy, by simply returning the higher 8bits of the accessed address. This seems
   enough for most games (only two sets have issues with this). A slightly better implementation is almost ready
   to fix these two remaining cases, but I plan to revisit the whole implementation in an accurate way at a later
   stage
 - Bus conflict is implemented based on latest tests by Blargg. There is some uncertainty about AxROM behavior
   (some AOROM pcbs suffers from bus conflict, some do not... since no AOROM game is known to glitch due to lack
   of bus conflict it seems safe to emulate the board without bus conflict, but eventually it would be good to
   differentiate the real variants)


 Many information about the mappers/pcbs come from the wonderful doc written by Disch.
 Current info (when used) are based on v0.6.1 of his docs.
 You can find the latest version of the doc at http://www.romhacking.net/docs/362/

 A lot of details have been based on the researches carried on at NesDev forums (by Blargg, Quietust and many more)
 and collected on the NesDev Wiki http://wiki.nesdev.com/

 Particular thanks go to
 - Martin Freij for his work on NEStopia
 - Cah4e3 for his efforts on FCEUMM and the reverse engineering of pirate boards
 - BootGod, lidnariq and naruko for the PCB tests which made possible


 ***********************************************************************************************************/

/*****************************************************************************************

 A few Mappers suffer of hardware conflict: original dumpers have used the same mapper number for more than
 a kind of boards. In these cases (and only in these cases) we exploit nes.hsi to set up accordingly
 emulation. Games which requires this hack are the following:
 * 032 - Major League needs hardwired mirroring (missing windows and glitched field in top view, otherwise)
 * 034 - Impossible Mission II is not like BxROM games (it writes to 0x7ffd-0x7fff, instead of 0x8000). It is still
 unplayable though (see above)
 * 071 - Fire Hawk is different from other Camerica games (no hardwired mirroring). Without crc_hack no helicopter graphics
 * 078 - Cosmo Carrier needs a different mirroring than Holy Diver
 * 113 - HES 6-in-1 requires mirroring (check Bookyman playfield), while other games break with this (check AV Soccer)
 * 153 - Famicom Jump II uses a different board (or the same in a very different way)
 * 242 - DQ8 has no mirroring (missing graphics is due to other reasons though)

 crc_hacks have been added also to handle a few wiring settings which would require submappers:
 * CHR protection pins for mapper 185
 * VRC-2, VRC-4 and VRC-6 line wiring

 Remember that the MMC # does not equal the mapper #. In particular, Mapper 4 is
 in fact MMC3, Mapper 9 is MMC2 and Mapper 10 is MMC4. Makes perfect sense, right?

 ****************************************************************************************/


#include "emu.h"
#include "hashfile.h"
#include "nes_slot.h"

#define NES_BATTERY_SIZE 0x2000


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type NES_CART_SLOT = &device_creator<nes_cart_slot_device>;


//**************************************************************************
//    NES cartridges Interface
//**************************************************************************

//-------------------------------------------------
//  device_nes_cart_interface - constructor
//-------------------------------------------------

device_nes_cart_interface::device_nes_cart_interface(const machine_config &mconfig, device_t &device)
						: device_slot_card_interface(mconfig, device),
						m_prg(nullptr),
						m_vrom(nullptr),
						m_ciram(nullptr),
						m_prg_size(0),
						m_vrom_size(0), m_maincpu(nullptr),
						m_mapper_sram(nullptr),
						m_mapper_sram_size(0),
						m_ce_mask(0),
						m_ce_state(0),
						m_vrc_ls_prg_a(0),
						m_vrc_ls_prg_b(0),
						m_vrc_ls_chr(0),
						m_mirroring(PPU_MIRROR_NONE),
						m_pcb_ctrl_mirror(FALSE),
						m_four_screen_vram(FALSE),
						m_has_trainer(FALSE),
						m_x1_005_alt_mirroring(FALSE),
						m_bus_conflict(TRUE),
						m_open_bus(0),
						m_prg_chunks(0),
						m_prg_mask(0xffff),
						m_chr_source(CHRRAM),
						m_vrom_chunks(0),
						m_vram_chunks(0)
{
}


//-------------------------------------------------
//  ~device_nes_cart_interface - destructor
//-------------------------------------------------

device_nes_cart_interface::~device_nes_cart_interface()
{
}

//-------------------------------------------------
//  pointer allocators
//-------------------------------------------------

void device_nes_cart_interface::prg_alloc(size_t size, const char *tag)
{
	if (m_prg == nullptr)
	{
		m_prg = device().machine().memory().region_alloc(std::string(tag).append(NESSLOT_PRGROM_REGION_TAG).c_str(), size, 1, ENDIANNESS_LITTLE)->base();
		m_prg_size = size;
		m_prg_chunks = size / 0x4000;
		if (size % 0x2000)
		{
			// A few pirate carts have PRG made of 32K + 2K or some weird similar config
			// in this case we treat the banking as if this 'extra' PRG is not present and
			// the pcb code has to handle it by accessing directly m_prg!
			printf("Warning! The loaded PRG has size not a multiple of 8KB (0x%X)\n", (UINT32)size);
			m_prg_chunks--;
		}

		m_prg_mask = ((m_prg_chunks << 1) - 1);

//      printf("first mask %x!\n", m_prg_mask);
		if ((m_prg_chunks << 1) & m_prg_mask)
		{
			int mask_bits = 0, temp = (m_prg_chunks << 1), mapsize;
			// contrary to what happens with later systems, like e.g. SNES or MD,
			// only half a dozen of NES carts have PRG which is not a power of 2
			// so we use this bank_map only as an exception
//          printf("uneven rom!\n");

			// 1. redefine mask as (next power of 2)-1
			for (; temp; )
			{
				mask_bits++;
				temp >>= 1;
			}
			m_prg_mask = (1 << mask_bits) - 1;
//          printf("new mask %x!\n", m_prg_mask);
			mapsize = (1 << mask_bits)/2;

			// 2. create a bank_map for banks in the range mask/2 -> mask
			m_prg_bank_map.resize(mapsize);

			// 3. fill the bank_map accounting for mirrors
			int j;
			for (j = mapsize; j < (m_prg_chunks << 1); j++)
				m_prg_bank_map[j - mapsize] = j;

			while (j % mapsize)
			{
				int k = 0, repeat_banks;
				while ((j % (mapsize >> k)) && k < mask_bits)
					k++;
				repeat_banks = j % (mapsize >> (k - 1));
				for (int l = 0; l < repeat_banks; l++)
					m_prg_bank_map[(j - mapsize) + l] = m_prg_bank_map[(j - mapsize) + l - repeat_banks];
				j += repeat_banks;
			}

			// check bank map!
//          for (int i = 0; i < mapsize; i++)
//          {
//              printf("bank %3d = %3d\t", i, m_prg_bank_map[i]);
//              if ((i%8) == 7)
//                  printf("\n");
//          }
		}
	}
}

void device_nes_cart_interface::vrom_alloc(size_t size, const char *tag)
{
	if (m_vrom == nullptr)
	{
		std::string tempstring(tag);
		tempstring.append(NESSLOT_CHRROM_REGION_TAG);
		m_vrom = device().machine().memory().region_alloc(tempstring.c_str(), size, 1, ENDIANNESS_LITTLE)->base();
		m_vrom_size = size;
		m_vrom_chunks = size / 0x2000;
	}
}

void device_nes_cart_interface::prgram_alloc(size_t size)
{
	m_prgram.resize(size);
}

void device_nes_cart_interface::vram_alloc(size_t size)
{
	m_vram.resize(size);
	m_vram_chunks = size / 0x2000;
}

void device_nes_cart_interface::battery_alloc(size_t size)
{
	m_battery.resize(size);
}


//-------------------------------------------------
//  PRG helpers
//-------------------------------------------------

inline int device_nes_cart_interface::prg_8k_bank_num(int bank_8k)
{
	if (m_prg_mask == ((m_prg_chunks << 1) - 1))
		return bank_8k & m_prg_mask;

	// only a few pirate games (less than a dozen) have PRG which is not power of 2
	// so we treat it here separately, rather than forcing all games to use m_prg_bank_map

	// case 1: if we are accessing a bank before the end of the image, just return that bank
	if (bank_8k < ((m_prg_chunks << 1) - 1))
		return bank_8k;

	// case 2: otherwise return a mirror using the bank_map!
//  UINT8 temp = bank_8k;
	bank_8k &= m_prg_mask;
	bank_8k -= (m_prg_mask/2 + 1);
//  printf("bank: accessed %x (top: %x), returned %x\n", temp, (m_prg_chunks << 1) - 1, m_prg_bank_map[bank_8k]);
	return m_prg_bank_map[bank_8k];
}

inline void device_nes_cart_interface::update_prg_banks(int prg_bank_start, int prg_bank_end)
{
	for (int prg_bank = prg_bank_start; prg_bank <= prg_bank_end; prg_bank++)
	{
		assert(prg_bank >= 0);
		assert(prg_bank < ARRAY_LENGTH(m_prg_bank));
		assert(prg_bank < ARRAY_LENGTH(m_prg_bank_mem));

		m_prg_bank_mem[prg_bank]->set_entry(m_prg_bank[prg_bank]);
	}
}

void device_nes_cart_interface::prg32(int bank)
{
	/* if there is only 16k PRG, return */
	if (!(m_prg_chunks >> 1))
		return;

	/* assumes that bank references a 32k chunk */
	bank = prg_8k_bank_num(bank * 4);

	m_prg_bank[0] = bank + 0;
	m_prg_bank[1] = bank + 1;
	m_prg_bank[2] = bank + 2;
	m_prg_bank[3] = bank + 3;
	update_prg_banks(0, 3);
}

void device_nes_cart_interface::prg16_89ab(int bank)
{
	/* assumes that bank references a 16k chunk */
	bank = prg_8k_bank_num(bank * 2);

	m_prg_bank[0] = bank + 0;
	m_prg_bank[1] = bank + 1;
	update_prg_banks(0, 1);
}

void device_nes_cart_interface::prg16_cdef(int bank)
{
	/* assumes that bank references a 16k chunk */
	bank = prg_8k_bank_num(bank * 2);

	m_prg_bank[2] = bank + 0;
	m_prg_bank[3] = bank + 1;
	update_prg_banks(2, 3);
}

void device_nes_cart_interface::prg8_89(int bank)
{
	/* assumes that bank references an 8k chunk */
	bank = prg_8k_bank_num(bank);

	m_prg_bank[0] = bank;
	update_prg_banks(0, 0);
}

void device_nes_cart_interface::prg8_ab(int bank)
{
	/* assumes that bank references an 8k chunk */
	bank = prg_8k_bank_num(bank);

	m_prg_bank[1] = bank;
	update_prg_banks(1, 1);
}

void device_nes_cart_interface::prg8_cd(int bank)
{
	/* assumes that bank references an 8k chunk */
	bank = prg_8k_bank_num(bank);

	m_prg_bank[2] = bank;
	update_prg_banks(2, 2);
}

void device_nes_cart_interface::prg8_ef(int bank)
{
	/* assumes that bank references an 8k chunk */
	bank = prg_8k_bank_num(bank);

	m_prg_bank[3] = bank;
	update_prg_banks(3, 3);
}

/* We also define an additional helper to map 8k PRG-ROM to one of the banks (passed as parameter) */
void device_nes_cart_interface::prg8_x(int start, int bank)
{
	assert(start < 4);

	/* assumes that bank references an 8k chunk */
	bank = prg_8k_bank_num(bank);

	m_prg_bank[start] = bank;
	update_prg_banks(start, start);
}

//-------------------------------------------------
//  CHR helpers
//-------------------------------------------------

inline void device_nes_cart_interface::chr_sanity_check( int source )
{
	if (source == CHRRAM && m_vram.empty())
		fatalerror("CHRRAM bankswitch with no VRAM\n");

	if (source == CHRROM && m_vrom == nullptr)
		fatalerror("CHRROM bankswitch with no VROM\n");
}

void device_nes_cart_interface::chr8(int bank, int source)
{
	chr_sanity_check(source);

	if (source == CHRRAM)
	{
		bank &= (m_vram_chunks - 1);
		for (int i = 0; i < 8; i++)
		{
			m_chr_src[i] = source;
			m_chr_orig[i] = (bank * 0x2000) + (i * 0x400); // for save state uses!
			m_chr_access[i] = &m_vram[m_chr_orig[i]];
		}
	}
	else
	{
		bank &= (m_vrom_chunks - 1);
		for (int i = 0; i < 8; i++)
		{
			m_chr_src[i] = source;
			m_chr_orig[i] = (bank * 0x2000) + (i * 0x400); // for save state uses!
			m_chr_access[i] = &m_vrom[m_chr_orig[i]];
		}
	}
}

void device_nes_cart_interface::chr4_x(int start, int bank, int source)
{
	chr_sanity_check(source);

	if (source == CHRRAM)
	{
		bank &= ((m_vram_chunks << 1) - 1);
		for (int i = 0; i < 4; i++)
		{
			m_chr_src[i + start] = source;
			m_chr_orig[i + start] = (bank * 0x1000) + (i * 0x400); // for save state uses!
			m_chr_access[i + start] = &m_vram[m_chr_orig[i + start]];
		}
	}
	else
	{
		bank &= ((m_vrom_chunks << 1) - 1);
		for (int i = 0; i < 4; i++)
		{
			m_chr_src[i + start] = source;
			m_chr_orig[i + start] = (bank * 0x1000) + (i * 0x400); // for save state uses!
			m_chr_access[i + start] = &m_vrom[m_chr_orig[i + start]];
		}
	}
}

void device_nes_cart_interface::chr2_x(int start, int bank, int source)
{
	chr_sanity_check(source);

	if (source == CHRRAM)
	{
		bank &= ((m_vram_chunks << 2) - 1);
		for (int i = 0; i < 2; i++)
		{
			m_chr_src[i + start] = source;
			m_chr_orig[i + start] = (bank * 0x800) + (i * 0x400); // for save state uses!
			m_chr_access[i + start] = &m_vram[m_chr_orig[i + start]];
		}
	}
	else
	{
		bank &= ((m_vrom_chunks << 2) - 1);
		for (int i = 0; i < 2; i++)
		{
			m_chr_src[i + start] = source;
			m_chr_orig[i + start] = (bank * 0x800) + (i * 0x400); // for save state uses!
			m_chr_access[i + start] = &m_vrom[m_chr_orig[i + start]];
		}
	}
}

void device_nes_cart_interface::chr1_x(int start, int bank, int source)
{
	chr_sanity_check(source);

	if (source == CHRRAM)
	{
		bank &= ((m_vram_chunks << 3) - 1);
		m_chr_src[start] = source;
		m_chr_orig[start] = (bank * 0x400); // for save state uses!
		m_chr_access[start] = &m_vram[m_chr_orig[start]];
	}
	else
	{
		bank &= ((m_vrom_chunks << 3) - 1);
		m_chr_src[start] = source;
		m_chr_orig[start] = (bank * 0x400); // for save state uses!
		m_chr_access[start] = &m_vrom[m_chr_orig[start]];
	}
}

//-------------------------------------------------
//  NT & Mirroring helpers
//-------------------------------------------------

void device_nes_cart_interface::set_nt_page(int page, int source, int bank, int writable)
{
	UINT8* base_ptr;

	switch (source)
	{
		case CART_NTRAM:
			base_ptr = &m_ext_ntram[0];
			break;
		case VROM:
			bank &= ((m_vrom_chunks << 3) - 1);
			base_ptr = m_vrom;
			break;
		case EXRAM:
		case MMC5FILL:
			base_ptr = nullptr;
			break;
		case CIRAM:
		default:
			base_ptr = m_ciram;
			break;
	}

	page &= 3; /* mask down to the 4 logical pages */
	m_nt_src[page] = source;

	if (base_ptr)
	{
		m_nt_orig[page] = bank * 0x400;
		m_nt_access[page] = base_ptr + m_nt_orig[page];
	}

	m_nt_writable[page] = writable;
}

void device_nes_cart_interface::set_nt_mirroring(int mirroring)
{
	/* setup our videomem handlers based on mirroring */
	switch (mirroring)
	{
		case PPU_MIRROR_VERT:
			set_nt_page(0, CIRAM, 0, 1);
			set_nt_page(1, CIRAM, 1, 1);
			set_nt_page(2, CIRAM, 0, 1);
			set_nt_page(3, CIRAM, 1, 1);
			break;

		case PPU_MIRROR_HORZ:
			set_nt_page(0, CIRAM, 0, 1);
			set_nt_page(1, CIRAM, 0, 1);
			set_nt_page(2, CIRAM, 1, 1);
			set_nt_page(3, CIRAM, 1, 1);
			break;

		case PPU_MIRROR_HIGH:
			set_nt_page(0, CIRAM, 1, 1);
			set_nt_page(1, CIRAM, 1, 1);
			set_nt_page(2, CIRAM, 1, 1);
			set_nt_page(3, CIRAM, 1, 1);
			break;

		case PPU_MIRROR_LOW:
			set_nt_page(0, CIRAM, 0, 1);
			set_nt_page(1, CIRAM, 0, 1);
			set_nt_page(2, CIRAM, 0, 1);
			set_nt_page(3, CIRAM, 0, 1);
			break;

		case PPU_MIRROR_4SCREEN:
			if (m_ext_ntram.empty()) fatalerror("4-screen mirroring without on-cart NTRAM!\n");
			set_nt_page(0, CART_NTRAM, 0, 1);
			set_nt_page(1, CART_NTRAM, 1, 1);
			set_nt_page(2, CART_NTRAM, 2, 1);
			set_nt_page(3, CART_NTRAM, 3, 1);
			break;

		case PPU_MIRROR_NONE:
		default:
			set_nt_page(0, CIRAM, 0, 1);
			set_nt_page(1, CIRAM, 0, 1);
			set_nt_page(2, CIRAM, 1, 1);
			set_nt_page(3, CIRAM, 1, 1);
			break;
	}
}

//-------------------------------------------------
//  Other helpers
//-------------------------------------------------

// Helper function for the few mappers reading from 0x8000-0xffff for protection
// so that they can access the ROM after the protection handling (which overwrites
// the memory banks)
UINT8 device_nes_cart_interface::hi_access_rom(UINT32 offset)
{
	int bank = (offset & 0x6000) >> 13;
	return m_prg[m_prg_bank[bank] * 0x2000 + (offset & 0x1fff)];
}

// Helper function for the few mappers subject to bus conflict at write.
// Tests by blargg showed that in many of the boards suffering of CPU/ROM
// conflicts the behaviour can be accurately emulated by writing not the
// original data, but data & rom[offset]
UINT8 device_nes_cart_interface::account_bus_conflict(UINT32 offset, UINT8 data)
{
	// pirate variants of boards subject to bus conflict are often not subject to it
	// so we allow to set m_bus_conflict to FALSE at loading time when necessary
	if (m_bus_conflict)
		return data & hi_access_rom(offset);
	else
		return data;
}


//-------------------------------------------------
//  PPU accessors
//-------------------------------------------------

WRITE8_MEMBER(device_nes_cart_interface::chr_w)
{
	int bank = offset >> 10;

	if (m_chr_src[bank] == CHRRAM)
		m_chr_access[bank][offset & 0x3ff] = data;
}

READ8_MEMBER(device_nes_cart_interface::chr_r)
{
	int bank = offset >> 10;
	return m_chr_access[bank][offset & 0x3ff];
}


WRITE8_MEMBER(device_nes_cart_interface::nt_w)
{
	int page = ((offset & 0xc00) >> 10);

	if (!m_nt_writable[page])
		return;

	m_nt_access[page][offset & 0x3ff] = data;
}

READ8_MEMBER(device_nes_cart_interface::nt_r)
{
	int page = ((offset & 0xc00) >> 10);
	return m_nt_access[page][offset & 0x3ff];
}


//-------------------------------------------------
//  Base memory accessors (emulating open bus
//  behaviour and/or WRAM accesses)
//  Open bus emulation is defective, but it should
//  be enough for the few cases known to rely on
//  this (more in the comments at the top of the
//  source)
//-------------------------------------------------

READ8_MEMBER(device_nes_cart_interface::read_l)
{
	return m_open_bus;
}

READ8_MEMBER(device_nes_cart_interface::read_m)
{
	if (!m_battery.empty())
		return m_battery[offset & (m_battery.size() - 1)];
	if (!m_prgram.empty())
		return m_prgram[offset & (m_prgram.size() - 1)];

	return m_open_bus;
}

WRITE8_MEMBER(device_nes_cart_interface::write_l)
{
}

WRITE8_MEMBER(device_nes_cart_interface::write_m)
{
	if (!m_battery.empty())
		m_battery[offset & (m_battery.size() - 1)] = data;
	if (!m_prgram.empty())
		m_prgram[offset & (m_prgram.size() - 1)] = data;
}

WRITE8_MEMBER(device_nes_cart_interface::write_h)
{
}


void device_nes_cart_interface::pcb_start(running_machine &machine, UINT8 *ciram_ptr, bool cart_mounted)
{
	// HACK: to reduce tagmap lookups for PPU-related IRQs, we add a hook to the
	// main NES CPU here, even if it does not belong to this device.
	m_maincpu = machine.device<cpu_device>("maincpu");

	if (cart_mounted)       // disksys expansion can arrive here without the memory banks!
	{
		// Setup PRG
		m_prg_bank_mem[0] = machine.root_device().membank("prg0");
		m_prg_bank_mem[1] = machine.root_device().membank("prg1");
		m_prg_bank_mem[2] = machine.root_device().membank("prg2");
		m_prg_bank_mem[3] = machine.root_device().membank("prg3");
		for (int i = 0; i < 4; i++)
		{
			if (m_prg_bank_mem[i])
			{
				m_prg_bank_mem[i]->configure_entries(0, m_prg_size / 0x2000, m_prg, 0x2000);
				m_prg_bank_mem[i]->set_entry(i);
				m_prg_bank[i] = i;
			}
		}
	}

	// Setup CHR (VRAM can be present also without PRG rom)
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	chr8(0, m_chr_source);

	// Setup NT
	m_ciram = ciram_ptr;

	if (m_four_screen_vram)
	{
		m_ext_ntram.resize(0x2000);
		device().save_item(NAME(m_ext_ntram));
	}

	// at loading time we have configured m_mirroring, now setup NT pages
	set_nt_mirroring(m_mirroring);

	// save the on-cart RAM pointers
	if (!m_prgram.empty())
		device().save_item(NAME(m_prgram));
	if (!m_vram.empty())
		device().save_item(NAME(m_vram));
	if (!m_battery.empty())
		device().save_item(NAME(m_battery));
}

void device_nes_cart_interface::pcb_reg_postload(running_machine &machine)
{
	machine.save().register_postload(save_prepost_delegate(FUNC(device_nes_cart_interface::nes_banks_restore), this));
}

void device_nes_cart_interface::nes_banks_restore()
{
	for (int i = 0; i < 4; i++)
		m_prg_bank_mem[i]->set_entry(m_prg_bank[i]);

	for (int i = 0; i < 8; i++)
		chr1_x(i, m_chr_orig[i] / 0x400, m_chr_src[i]);

	for (int i = 0; i < 4; i++)
		set_nt_page(i, m_nt_src[i], m_nt_orig[i] / 0x400, m_nt_writable[i]);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nes_cart_slot_device - constructor
//-------------------------------------------------
nes_cart_slot_device::nes_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
						device_t(mconfig, NES_CART_SLOT, "NES Cartridge Slot", tag, owner, clock, "nes_cart_slot", __FILE__),
						device_image_interface(mconfig, *this),
						device_slot_interface(mconfig, *this),
						m_crc_hack(0), m_cart(nullptr),
						m_pcb_id(NO_BOARD),
						m_must_be_loaded(1)
{
}

//-------------------------------------------------
//  nes_cart_slot_device - destructor
//-------------------------------------------------

nes_cart_slot_device::~nes_cart_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nes_cart_slot_device::device_start()
{
	m_cart = dynamic_cast<device_nes_cart_interface *>(get_card_device());
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void nes_cart_slot_device::device_config_complete()
{
	// set brief and instance name
	update_names();
}


void nes_cart_slot_device::pcb_start(UINT8 *ciram_ptr)
{
	if (m_cart)
		m_cart->pcb_start(machine(), ciram_ptr, exists());
}

void nes_cart_slot_device::pcb_reset()
{
	if (m_cart)
		m_cart->pcb_reset();
}

/*-------------------------------------------------
 call load
 -------------------------------------------------*/


/*-------------------------------------------------

 Load from xml list and identify the required slot device

 -------------------------------------------------*/

/* Include emulation of NES PCBs for softlist */
#include "nes_pcb.inc"


/*-------------------------------------------------

 Load .unf files (UNIF boards) and identify the required slot device

 -------------------------------------------------*/

/* Include emulation of UNIF Boards for .unf files */
#include "nes_unif.inc"


/*-------------------------------------------------

 Load .nes files (iNES mappers) and identify the required slot devices

 -------------------------------------------------*/

/* Include emulation of iNES Mappers for .nes files */
#include "nes_ines.inc"


bool nes_cart_slot_device::call_load()
{
	if (m_cart)
	{
		if (software_entry() == nullptr)
		{
			char magic[4];

			/* Check first 4 bytes of the image to decide if it is UNIF or iNES */
			/* Unfortunately, many .unf files have been released as .nes, so we cannot rely on extensions only */
			fread(magic, 4);

			if ((magic[0] == 'N') && (magic[1] == 'E') && (magic[2] == 'S'))    /* If header starts with 'NES' it is iNES */
			{
				if (length() <= 0x10)
				{
					logerror("%s only contains the iNES header and no data.\n", filename());
					return IMAGE_INIT_FAIL;
				}

				call_load_ines();
			}
			else if ((magic[0] == 'U') && (magic[1] == 'N') && (magic[2] == 'I') && (magic[3] == 'F')) /* If header starts with 'UNIF' it is UNIF */
			{
				if (length() <= 0x20)
				{
					logerror("%s only contains the UNIF header and no data.\n", filename());
					return IMAGE_INIT_FAIL;
				}

				call_load_unif();
			}
			else
			{
				logerror("%s is NOT a file in either iNES or UNIF format.\n", filename());
				return IMAGE_INIT_FAIL;
			}
		}
		else
			call_load_pcb();
	}

	return IMAGE_INIT_PASS;
}


/*-------------------------------------------------
 call_unload
 -------------------------------------------------*/

void nes_cart_slot_device::call_unload()
{
	if (m_cart)
	{
		if (m_cart->get_battery_size() || m_cart->get_mapper_sram_size())
		{
			UINT32 tot_size = m_cart->get_battery_size() + m_cart->get_mapper_sram_size();
			dynamic_buffer temp_nvram(tot_size);
			if (m_cart->get_battery_size())
				memcpy(&temp_nvram[0], m_cart->get_battery_base(), m_cart->get_battery_size());
			if (m_cart->get_mapper_sram_size())
				memcpy(&temp_nvram[m_cart->get_battery_size()], m_cart->get_mapper_sram_base(), m_cart->get_mapper_sram_size());

			battery_save(&temp_nvram[0], tot_size);
		}
	}
}


/*-------------------------------------------------
 call softlist load
 -------------------------------------------------*/

bool nes_cart_slot_device::call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry)
{
	load_software_part_region(*this, swlist, swname, start_entry);
	return TRUE;
}

/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

void nes_cart_slot_device::get_default_card_software(std::string &result)
{
	if (open_image_file(mconfig().options()))
	{
		const char *slot_string = "nrom";
		UINT32 len = core_fsize(m_file);
		dynamic_buffer rom(len);

		core_fread(m_file, &rom[0], len);

		if ((rom[0] == 'N') && (rom[1] == 'E') && (rom[2] == 'S'))
			slot_string = get_default_card_ines(&rom[0], len);

		if ((rom[0] == 'U') && (rom[1] == 'N') && (rom[2] == 'I') && (rom[3] == 'F'))
			slot_string = get_default_card_unif(&rom[0], len);

		clear();

		result.assign(slot_string);
	}
	else
		software_get_default_slot(result, "nrom");
}


/*-------------------------------------------------
 read
 -------------------------------------------------*/

READ8_MEMBER(nes_cart_slot_device::read_l)
{
	if (m_cart)
	{
		UINT8 val = m_cart->read_l(space, offset);
		// update open bus
		m_cart->set_open_bus(((offset + 0x4100) & 0xff00) >> 8);
		return val;
	}
	else
		return 0xff;
}

READ8_MEMBER(nes_cart_slot_device::read_m)
{
	if (m_cart)
	{
		UINT8 val = m_cart->read_m(space, offset);
		// update open bus
		m_cart->set_open_bus(((offset + 0x6000) & 0xff00) >> 8);
		return val;
	}
	else
		return 0xff;
}

READ8_MEMBER(nes_cart_slot_device::read_h)
{
	if (m_cart)
	{
		UINT8 val = m_cart->read_h(space, offset);
		// update open bus
		m_cart->set_open_bus(((offset + 0x8000) & 0xff00) >> 8);
		return val;
	}
	else
		return 0xff;
}

READ8_MEMBER(nes_cart_slot_device::read_ex)
{
	if (m_cart)
	{
		UINT8 val = m_cart->read_ex(space, offset);
		// update open bus
		m_cart->set_open_bus(((offset + 0x4020) & 0xff00) >> 8);
		return val;
	}
	else
		return 0xff;
}


/*-------------------------------------------------
 write
 -------------------------------------------------*/

WRITE8_MEMBER(nes_cart_slot_device::write_l)
{
	if (m_cart)
	{
		m_cart->write_l(space, offset, data);
		// update open bus
		m_cart->set_open_bus(((offset + 0x4100) & 0xff00) >> 8);
	}
}

WRITE8_MEMBER(nes_cart_slot_device::write_m)
{
	if (m_cart)
	{
		m_cart->write_m(space, offset, data);
		// update open bus
		m_cart->set_open_bus(((offset + 0x6000) & 0xff00) >> 8);
	}
}

WRITE8_MEMBER(nes_cart_slot_device::write_h)
{
	if (m_cart)
	{
		m_cart->write_h(space, offset, data);
		// update open bus
		m_cart->set_open_bus(((offset + 0x8000) & 0xff00) >> 8);
	}
}

WRITE8_MEMBER(nes_cart_slot_device::write_ex)
{
	if (m_cart)
	{
		m_cart->write_ex(space, offset, data);
		// update open bus
		m_cart->set_open_bus(((offset + 0x4020) & 0xff00) >> 8);
	}
}


//-------------------------------------------------
//  partial hash function to be used by
//  device_image_partialhash_func
//-------------------------------------------------

void nes_partialhash(hash_collection &dest, const unsigned char *data,
						unsigned long length, const char *functions)
{
	if (length <= 16)
		return;
	dest.compute(&data[16], length - 16, functions);
}
