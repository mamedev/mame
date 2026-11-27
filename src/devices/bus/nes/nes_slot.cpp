// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
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
 and collected on the NesDev Wiki https://www.nesdev.org/wiki

 Particular thanks go to
 - Martin Freij for his work on NEStopia
 - Cah4e3 for his efforts on FCEUMM and the reverse engineering of pirate boards
 - BootGod, lidnariq and naruko for the PCB tests which made possible


 ***********************************************************************************************************/

/*****************************************************************************************

 A few mapper numbers were used by the original dumpers for more than one kind of board. For iNES 1.0 images,
 which have no submapper, nes.hsi gives these images a variant code that the loader uses to set up the right
 board:
 * 032 - Major League needs hardwired mirroring (missing windows and glitched field in top view, otherwise)
 * 034 - Impossible Mission II is not like BxROM games (it writes to 0x7ffd-0x7fff, instead of 0x8000). It is still
 unplayable though (see above)
 * 071 - Fire Hawk has mapper controlled mirroring, unlike the other Camerica games: without its variant code the
 helicopter graphics are missing
 * 078 - Cosmo Carrier needs a different mirroring than Holy Diver
 * 113 - HES 6-in-1 requires mirroring (check Bookyman playfield), while other games break with this (check AV Soccer)
 * 153 - Famicom Jump II uses a different board (or the same in a very different way)
 * 242 - DQ8 has no mirroring (missing graphics is due to other reasons though)

 Variant codes also select a few wirings that NES 2.0 gives as submappers:
 * CHR protection pins for mapper 185
 * VRC-2, VRC-4 and VRC-6 line wiring
 * MMC6 (HKROM) and the MMC3A style IRQ for mapper 4

 Remember that the MMC # does not equal the mapper #. In particular, Mapper 4 is
 in fact MMC3, Mapper 9 is MMC2 and Mapper 10 is MMC4. Makes perfect sense, right?

 ****************************************************************************************/


#include "emu.h"
#include "hashfile.h"
#include "nes_slot.h"
#include "cpu/m6502/rp2a03.h"

#include "cpu/m6502/m6502.h"

#define NES_BATTERY_SIZE 0x2000


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(NES_CART_SLOT, nes_cart_slot_device, "nes_cart_slot", "NES Cartridge Slot")


//**************************************************************************
//    NES cartridges Interface
//**************************************************************************

//-------------------------------------------------
//  device_nes_cart_interface - constructor
//-------------------------------------------------

device_nes_cart_interface::device_nes_cart_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "nescart")
	, m_prg(nullptr)
	, m_vrom(nullptr)
	, m_ciram(nullptr)
	, m_prg_size(0)
	, m_vrom_size(0)
	, m_slot(nullptr)
	, m_maincpu(nullptr)
	, m_builtin_cpu(device, finder_base::DUMMY_TAG)
	, m_builtin_irq(device)
	, m_mapper_sram(nullptr)
	, m_misc_rom(nullptr)
	, m_mapper_sram_size(0)
	, m_misc_rom_size(0)
	, m_ce_mask(0)
	, m_ce_state(0)
	, m_mmc1_type(mmc1_type::MMC1B)
	, m_vrc_ls_prg_a(0)
	, m_vrc_ls_prg_b(0)
	, m_vrc_ls_chr(0)
	, m_n163_vol(0)
	, m_outer_prg_size(0)
	, m_outer_chr_size(0)
	, m_smd133_addr(0x6000)
	, m_mirroring(PPU_MIRROR_HORZ)
	, m_pcb_ctrl_mirror(false)
	, m_four_screen_vram(false)
	, m_has_trainer(false)
	, m_x1_005_alt_mirroring(false)
	, m_bus_conflict(true)
	, m_open_bus(0)
	, m_prg_chunks(0)
	, m_prg_mask(0xffff)
	, m_chr_source(CHRRAM)
	, m_vrom_chunks(0)
	, m_vram_chunks(0)
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

void device_nes_cart_interface::prg_alloc(size_t size)
{
	if (m_prg == nullptr)
	{
		m_prg = device().machine().memory().region_alloc(device().subtag("^cart:prg_rom"), size, 1, ENDIANNESS_LITTLE)->base();
		m_prg_size = size;
		m_prg_chunks = size / 0x4000;
		if (size % 0x2000)
		{
			// A few pirate carts have PRG made of 32K + 2K or some weird similar config
			// in this case we treat the banking as if this 'extra' PRG is not present and
			// the pcb code has to handle it by accessing directly m_prg!
			printf("Warning! The loaded PRG has size not a multiple of 8KB (0x%X)\n", (uint32_t)size);
			m_prg_chunks--;
		}

		m_prg_mask = (m_prg_chunks << 1) - 1;

//      printf("first mask %x!\n", m_prg_mask);
		if ((m_prg_chunks << 1) & m_prg_mask)
		{
			int mask_bits = 0;
			// contrary to what happens with later systems, like e.g. SNES or MD,
			// only half a dozen of NES carts have PRG which is not a power of 2
			// so we use this bank_map only as an exception
//          printf("uneven rom!\n");

			// 1. redefine mask as (next power of 2)-1
			for (int temp = m_prg_chunks << 1; temp; temp >>= 1)
				mask_bits++;

			m_prg_mask = (1 << mask_bits) - 1;
//          printf("new mask %x!\n", m_prg_mask);
			int mapsize = (1 << mask_bits) / 2;

			// 2. create a bank_map for banks in the range mask/2 -> mask
			m_prg_bank_map.resize(mapsize);

			// 3. fill the bank_map accounting for mirrors
			int j;
			for (j = mapsize; j < (m_prg_chunks << 1); j++)
				m_prg_bank_map[j - mapsize] = j;

			while (j % mapsize)
			{
				int k = 0, repeat_banks;
				while (j % (mapsize >> k) && k < mask_bits)
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

void device_nes_cart_interface::vrom_alloc(size_t size)
{
	if (m_vrom == nullptr)
	{
		m_vrom = device().machine().memory().region_alloc(device().subtag("^cart:chr_rom"), size, 1, ENDIANNESS_LITTLE)->base();
		m_vrom_size = size;
		m_vrom_chunks = size / 0x2000;
	}
}

void device_nes_cart_interface::misc_rom_alloc(size_t size)
{
	if (m_misc_rom == nullptr)
	{
		m_misc_rom = device().machine().memory().region_alloc(device().subtag("^cart:misc_rom"), size, 1, ENDIANNESS_LITTLE)->base();
		m_misc_rom_size = size;
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
	if (m_prg_mask == (m_prg_chunks << 1) - 1)
		return bank_8k & m_prg_mask;

	// only a few pirate games (less than a dozen) have PRG which is not power of 2
	// so we treat it here separately, rather than forcing all games to use m_prg_bank_map

	// case 1: if we are accessing a bank before the end of the image, just return that bank
	if (bank_8k < (m_prg_chunks << 1) - 1)
		return bank_8k;

	// case 2: otherwise return a mirror using the bank_map!
//  uint8_t temp = bank_8k;
	bank_8k &= m_prg_mask;
	bank_8k -= m_prg_mask / 2 + 1;
//  printf("bank: accessed %x (top: %x), returned %x\n", temp, (m_prg_chunks << 1) - 1, m_prg_bank_map[bank_8k]);
	return m_prg_bank_map[bank_8k];
}

inline void device_nes_cart_interface::update_prg_banks(int prg_bank_start, int prg_bank_end)
{
	for (int prg_bank = prg_bank_start; prg_bank <= prg_bank_end; prg_bank++)
	{
		assert(prg_bank >= 0);
		assert(prg_bank < std::size(m_prg_bank));
		assert(prg_bank < std::size(m_prg_bank_mem));

		m_prg_bank_mem[prg_bank]->set_entry(m_prg_bank[prg_bank]);
	}
}

void device_nes_cart_interface::prg32(int bank)
{
	// if there is only 16k PRG, return
	if (!(m_prg_chunks >> 1))
		return;

	// assumes that bank references a 32k chunk
	bank = prg_8k_bank_num(bank * 4);

	m_prg_bank[0] = bank + 0;
	m_prg_bank[1] = bank + 1;
	m_prg_bank[2] = bank + 2;
	m_prg_bank[3] = bank + 3;
	update_prg_banks(0, 3);
}

void device_nes_cart_interface::prg16_89ab(int bank)
{
	// assumes that bank references a 16k chunk
	bank = prg_8k_bank_num(bank * 2);

	m_prg_bank[0] = bank + 0;
	m_prg_bank[1] = bank + 1;
	update_prg_banks(0, 1);
}

void device_nes_cart_interface::prg16_cdef(int bank)
{
	// assumes that bank references a 16k chunk
	bank = prg_8k_bank_num(bank * 2);

	m_prg_bank[2] = bank + 0;
	m_prg_bank[3] = bank + 1;
	update_prg_banks(2, 3);
}

// We define a parameterized helper to map 8k PRG-ROM to one of the banks
void device_nes_cart_interface::prg8_x(int start, int bank)
{
	assert(start < 4);

	// assumes that bank references an 8k chunk
	bank = prg_8k_bank_num(bank);

	m_prg_bank[start] = bank;
	update_prg_banks(start, start);
}

//-------------------------------------------------
//  CHR helpers
//-------------------------------------------------

void device_nes_cart_interface::bank_chr(int shift, int start, int bank, int source)
{
	uint8_t *base_ptr;
	uint32_t chr_chunks;

	if (source == CHRROM && m_vrom == nullptr)
	{
		// the board asked for CHR-ROM that this cart does not have (a software list entry without a CHR data area):
		// map CHR-RAM instead of a null pointer that the PPU would read through
		if (m_vram.empty())
			vram_alloc(0x2000);
		source = CHRRAM;
	}

	if (source == CHRRAM)
	{
		assert(!m_vram.empty());

		base_ptr = &m_vram[0];
		chr_chunks = m_vram_chunks;
	}
	else // source == CHRROM
	{
		assert(m_vrom != nullptr);

		base_ptr = m_vrom;
		chr_chunks = m_vrom_chunks;
	}

	if (source == CHRRAM && m_vram.size() < 0x2000)
	{
		// CHR-RAM smaller than 8 KiB (e.g. the 2 KiB on Waixing mapper 74 boards): m_vram_chunks is 0, which made the mask
		// -1 and let bank numbers point past the end of the RAM - mask by the RAM's real size in 1 KiB pages instead
		const uint32_t windows = (m_vram.size() / 0x400) >> shift;
		bank = windows ? (bank & (windows - 1)) : 0;
	}
	else
	{
		// a CHR-ROM whose size is not a power of two (the 1.5 MiB of the Maxivision 30-in-1, NES 2.0 mapper 487) cannot be
		// wrapped with a mask: bank numbers inside the image are kept, the ones past its end wrap around it
		const uint32_t windows = chr_chunks << (3 - shift);
		if (windows & (windows - 1))
			bank = windows ? (uint32_t(bank) % windows) : 0;
		else
			bank &= windows - 1;
	}
	int size = 0x400 << shift;
	int bank_start = bank * size;
	int kbyte = 1 << shift;

	for (int i = 0; i < kbyte; i++)
	{
		m_chr_src[i + start] = source;
		m_chr_orig[i + start] = bank_start + i * 0x400; // for save state uses!
		m_chr_access[i + start] = &base_ptr[m_chr_orig[i + start]];
	}
}

//-------------------------------------------------
//  NT & Mirroring helpers
//-------------------------------------------------

void device_nes_cart_interface::set_nt_page(int page, int source, int bank, int writable)
{
	uint8_t *base_ptr;

	switch (source)
	{
		case CART_NTRAM:
			base_ptr = &m_ext_ntram[0];
			break;
		case VRAM_NT:
			base_ptr = &m_vram[0];
			break;
		case VROM:
			bank &= (m_vrom_chunks << 3) - 1;
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

	page &= 3; // mask down to the 4 logical pages
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
	// setup our videomem handlers based on mirroring
	switch (mirroring)
	{
		default:
		case PPU_MIRROR_HORZ:
			set_nt_page(0, CIRAM, 0, 1);
			set_nt_page(1, CIRAM, 0, 1);
			set_nt_page(2, CIRAM, 1, 1);
			set_nt_page(3, CIRAM, 1, 1);
			break;

		case PPU_MIRROR_VERT:
			set_nt_page(0, CIRAM, 0, 1);
			set_nt_page(1, CIRAM, 1, 1);
			set_nt_page(2, CIRAM, 0, 1);
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
	}
}

//-------------------------------------------------
//  Interrupt helpers
//-------------------------------------------------

void device_nes_cart_interface::set_irq_line(int state)
{
	assert(state == ASSERT_LINE || state == CLEAR_LINE);

	if (m_slot)
		m_slot->irq_w(state);
	else
		m_builtin_irq(state);
}

//-------------------------------------------------
//  interface_pre_start - find the slot and the CPU
//-------------------------------------------------

void device_nes_cart_interface::interface_pre_start()
{
	m_slot = dynamic_cast<nes_cart_slot_device *>(device().owner());
	m_maincpu = m_slot ? m_slot->cpu() : m_builtin_cpu.target();
	if (!m_maincpu)
		throw emu_fatalerror("%s: no CPU configured for the NES cartridge (on its slot, or on the cartridge if it is built in)\n", device().tag());
}

//-------------------------------------------------
//  Other helpers
//-------------------------------------------------

// Helper function for the few mappers reading from 0x8000-0xffff for protection
// so that they can access the ROM after the protection handling (which overwrites
// the memory banks)
uint8_t device_nes_cart_interface::hi_access_rom(uint32_t offset)
{
	int bank = BIT(offset, 13, 2);
	return m_prg[m_prg_bank[bank] * 0x2000 + (offset & 0x1fff)];
}

// Helper function for the few mappers subject to bus conflict at write.
// Tests by blargg showed that in many of the boards suffering of CPU/ROM
// conflicts the behaviour can be accurately emulated by writing not the
// original data, but data & rom[offset]
uint8_t device_nes_cart_interface::account_bus_conflict(uint32_t offset, uint8_t data)
{
	// pirate variants of boards subject to bus conflict are often not subject to it
	// so we allow to set m_bus_conflict to false at loading time when necessary
	if (m_bus_conflict)
		return data & hi_access_rom(offset);
	else
		return data;
}


//-------------------------------------------------
//  PPU accessors
//-------------------------------------------------

void device_nes_cart_interface::chr_w(offs_t offset, uint8_t data)
{
	int bank = BIT(offset, 10, 3);

	if (m_chr_src[bank] == CHRRAM)
		m_chr_access[bank][offset & 0x3ff] = data;
}

uint8_t device_nes_cart_interface::chr_r(offs_t offset)
{
	int bank = BIT(offset, 10, 3);
	return m_chr_access[bank][offset & 0x3ff];
}


void device_nes_cart_interface::nt_w(offs_t offset, uint8_t data)
{
	int page = BIT(offset, 10, 2);

	if (m_nt_writable[page])
		m_nt_access[page][offset & 0x3ff] = data;
}

uint8_t device_nes_cart_interface::nt_r(offs_t offset)
{
	int page = BIT(offset, 10, 2);
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

uint8_t device_nes_cart_interface::get_open_bus()
{
	if (auto *const cpu = dynamic_cast<rp2a03_core_device *>(m_maincpu))
		return cpu->open_bus();
	return m_open_bus;
}

uint8_t device_nes_cart_interface::read_l(offs_t offset)
{
	return get_open_bus();
}

uint8_t device_nes_cart_interface::read_m(offs_t offset)
{
	if (!m_battery.empty())
		return m_battery[offset & (m_battery.size() - 1)];
	if (!m_prgram.empty())
		return m_prgram[offset & (m_prgram.size() - 1)];

	return get_open_bus();
}

void device_nes_cart_interface::write_l(offs_t offset, uint8_t data)
{
}

void device_nes_cart_interface::write_m(offs_t offset, uint8_t data)
{
	if (!m_battery.empty())
		m_battery[offset & (m_battery.size() - 1)] = data;
	if (!m_prgram.empty())
		m_prgram[offset & (m_prgram.size() - 1)] = data;
}

void device_nes_cart_interface::write_h(offs_t offset, uint8_t data)
{
}


void device_nes_cart_interface::pcb_start(running_machine &machine, uint8_t *ciram_ptr, bool cart_mounted)
{
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
				// images smaller than 32 KiB (e.g. 8 KiB NROM such as Galaxian) mirror their banks across $8000-$FFFF;
				// selecting entry i directly failed for entries that do not exist
				const int entries = std::max<int>(1, m_prg_size / 0x2000);
				m_prg_bank_mem[i]->configure_entries(0, entries, m_prg, 0x2000);
				m_prg_bank_mem[i]->set_entry(i % entries);
				m_prg_bank[i] = i % entries;
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
		m_ext_ntram.resize(0x1000);
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

	// open bus
	device().save_item(NAME(m_open_bus));
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
nes_cart_slot_device::nes_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NES_CART_SLOT, tag, owner, clock)
	, device_cartrom_image_interface(mconfig, *this)
	, device_single_card_slot_interface<device_nes_cart_interface>(mconfig, *this)
	, device_mixer_interface(mconfig, *this)
	, m_cart(nullptr)
	, m_pcb_id(NO_BOARD)
	, m_cpu(*this, finder_base::DUMMY_TAG)
	, m_irq_handler(*this)
{
}

//-------------------------------------------------
//  nes_cart_slot_device - destructor
//-------------------------------------------------

nes_cart_slot_device::~nes_cart_slot_device()
{
}

//-------------------------------------------------
//  cpu - the CPU configured on this slot, or on the
//  slot a pass-through cartridge (Game Genie) is in
//-------------------------------------------------

cpu_device *nes_cart_slot_device::cpu() const
{
	for (const device_t *dev = this; dev; dev = dev->owner())
	{
		auto const *const slot = dynamic_cast<const nes_cart_slot_device *>(dev);
		if (slot && slot->m_cpu.target())
			return slot->m_cpu.target();
	}
	return nullptr;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nes_cart_slot_device::device_start()
{
	m_cart = get_card_device();
}


void nes_cart_slot_device::pcb_start(uint8_t *ciram_ptr)
{
	if (m_cart)
		m_cart->pcb_start(machine(), ciram_ptr, exists());
}

// NESdev "Cartridge connector": the Famicom's 60-pin connector has no reset line and no CIC lines at all, and the only
// reset on the NES's 72 pins is the CIC's (CIC +RST). The console's reset button therefore only reaches a board that
// takes the CIC's reset or has a circuit of its own to notice a reset. The boards below are set up at power-on only and
// keep their registers through a reset:
// - Nintendo's discrete logic boards (a latch for the bank and nothing that watches for a reset);
// - MMC2: no reset input (NESdev "MMC2 pinout"). MMC4: only on Famicom boards; three of its pins are of unknown
//   function (NESdev "MMC4 pinout"). MMC6: its inputs of unknown function are tied to supply levels on the HKROM board
//   (NESdev "MMC6 pinout");
// - Konami VRC2, VRC4, VRC6, VRC7: no reset input (NESdev "VRC2 pinout", "VRC6 pinout", "VRC7 pinout"; the VRC7's /IC
//   only exists in its debug mode);
// - Namcot 129/163/175/340: a /RESET pin, fed from the board's supply, through the battery switch where there is one,
//   and not from the console (NESdev "Namcot 163 family pinout", pins 19, 32, 43);
// - Sunsoft FME-7/5A/5B and Jaleco SS 88006: no reset input (NESdev "Sunsoft 5 pinout", "Jaleco SS 88006 pinout");
// - Namcot 108/109/118/119 and Konami VRC1: no reset input, every pin is accounted for (NESdev "Namcot 108 family
//   pinout", "VRC1 pinout");
// - Konami VRC3: no reset input among its 18 pins (NESdev "VRC3 pinout"). Sunsoft 4: no reset input; its one pin of
//   uncertain function, 10, is tied to ground (NESdev "Sunsoft 4 pinout"). The same chip with the connector for the
//   Nantettatte!! Baseball sub-cartridge: that connector carries M2, the address and data lines and four of the
//   chip's outputs, and no reset (NESdev "Nantettatte!! Baseball pinout").
// - Sunsoft 1, 2 and 3, Irem H3001 and TAM-S1, Taito TC0190 (with or without its PAL), TC0690, X1-005 and X1-017: no
//   reset input, every pin is accounted for and the unused ones are marked not connected (NESdev "Sunsoft 1 pinout",
//   "Sunsoft 2 pinout", "Sunsoft 3 pinout", "Irem H3001 pinout", "Irem TAM-S1 pinout", "Taito TC0190 pinout",
//   "Taito TC0690 pinout", "Taito X1-005 pinout", "Taito X1-017 pinout").
// - NTDEC TC-112: no reset input, every pin is accounted for (NESdev "NTDEC TC-112 pinout"). The NTDEC8801 has a /RESET
//   input whose wiring is not documented (NESdev "NTDEC8801 pinout"); the Konami VRC5 and the Sachen SA8259A each have
//   pins of unknown function (NESdev "VRC5 pinout", "Sachen SA8259A pinout"), so they stay on the default.
// - Kaiser KS-201 (mappers 312 and 554): no reset input; its one input of unknown function, pin 18, is tied to the supply
//   (NESdev "KS-201 pinout"). Sachen's "74LS374N" (mappers 150 and 243) and the SMD133/AA6023 (mappers 224 and 268): no
//   reset input, every pin is accounted for (NESdev "Sachen 74LS374N pinout", "SMD133 (AA6023) pinout", which shows
//   both its QFP and DIP packages). The Yoko PT8154 has no reset input either, but NESdev says it was used on one version
//   of mapper 189 only, so it is not matched to the board for 189 here.
// - The mapper 208 chip (Gouder) and the TXC 05-00002-010 (mappers 36, 132 and 173): no reset input, every pin is accounted
//   for (NESdev "Mapper 208 pinout", "TXC 05-00002-010 pinout"). The Bandai M60001-0115P of the Karaoke Studio (mapper 188)
//   has a pin of unknown function (19) and stays on the default (NESdev "Bandai M60001-0115P pinout").
// - The non-Nintendo discrete-logic boards whose registers are 74xx latches (the boards of the M2-fall list above that were
//   not excluded yet: Color Dreams' 74377, the 74161 and 74138 of mapper 38, the 74139 and 7474 of mappers 87 and 101,
//   Irem's mapper 78 and Jaleco's JF-11 and JF-16): a latch holds the bank and nothing watches for a reset, as on
//   Nintendo's discrete boards. Jaleco's JF-23, JF-24, JF-29 and JF-33 are boards of the SS 88006 above.
// - Jaleco's JF-13, JF-17 and JF-19 (with or without the ADPCM) and the 74161/161/32 board of mappers 70 and 152: discrete
//   logic with a 74161 on the data bus, as in Nintendo's discrete boards (NESdev "INES Mapper 072", "INES Mapper 086",
//   "INES Mapper 070"); the reset of the uPD7756 on the Jaleco boards comes from the board's own latch, not the console.
// The MMC1 and MMC3 boards do the same in their own pcb_reset(). Every other board is still re-initialized by a reset.
// That is right for the multicarts that detect one; the MMC5 detects a system reset (NESdev "MMC5") and the RAMBO-1
// takes the CIC's reset on pin 17 (NESdev "Tengen RAMBO-1 pinout"), with no documented effect on their registers; the
// Bandai LZ93D50 has seven pins of unknown function and the Bandai FCG-1/2 has an input of unknown function on pin 1
// (NESdev "Bandai FCG pinout"); Camerica's BF9093 and BF9096 have no pinout page; the rest have not been checked.
bool nes_cart_slot_device::console_reset_reaches_board() const
{
	switch (m_pcb_id)
	{
		case STD_NROM:
		case STD_NROM368:
		case STD_AXROM:
		case STD_AMROM:
		case STD_BXROM:
		case STD_CNROM:
		case STD_CPROM:
		case STD_GXROM:
		case STD_UXROM:
		case STD_UN1ROM:
		case UXROM_CC:
		case STD_PXROM:
		case STD_FXROM:
		case STD_HKROM:
		case KONAMI_VRC2:
		case KONAMI_VRC4:
		case KONAMI_VRC6:
		case KONAMI_VRC7:
		case SUNSOFT_FME7:
		case SUNSOFT_5:
		case JALECO_SS88006:
		case NAMCOT_163:
		case NAMCOT_175:
		case NAMCOT_340:
		case NAMCOT_34X3:
		case NAMCOT_3425:
		case NAMCOT_3446:
		case KONAMI_VRC1:
		case KONAMI_VRC3:
		case SUNSOFT_4:
		case SUNSOFT_DCS:
		case SUNSOFT_1:
		case SUNSOFT_2:
		case SUNSOFT_3:
		case IREM_H3001:
		case IREM_TAM_S1:
		case TAITO_TC0190FMC:
		case TAITO_TC0190FMCP:
		case TAITO_X1_005:
		case TAITO_X1_017:
		case NTDEC_FIGHTINGHERO:
		case KAISER_KS7010:
		case KAISER_KS7013B:
		case SACHEN_74LS374:
		case SACHEN_74LS374_ALT:
		case SMD133_BOARD:
		case GOUDER_37017:
		case TXC_STRIKEW:
		case TXC_22211:
		case TXC_MJBLOCK:
		case DIS_74X377:
		case DIS_74X139X74:
		case DIS_74X161X138:
		case IREM_HOLYDIVR:
		case JALECO_JF11:
		case JALECO_JF16:
		case JALECO_JF23:
		case JALECO_JF24:
		case JALECO_JF29:
		case JALECO_JF33:
		case JALECO_JF13:
		case JALECO_JF17:
		case JALECO_JF17_ADPCM:
		case JALECO_JF19:
		case JALECO_JF19_ADPCM:
		case DIS_74X161X161X32:
			return false;
		default:
			return true;
	}
}

// Boards whose register is a 74161 used as a latch: NESdev "74161": "/Load causes Q to be set to the value of D on a
// low-to-high transition of Clock", wired "Clock -- /ROMSEL, /Load -- CPU R/W". /ROMSEL is the NAND of M2 and CPU A15
// (NESdev "Cartridge connector"), so it rises when M2 falls: the register takes the written value at the end of the
// write cycle, not when the write begins. NESdev names the 74HC161 on the UxROM, CNROM, GxROM, CPROM and AxROM boards.
// The same holds for the other boards whose register is an edge-triggered part: the written value is only on the data
// bus from phi2 until M2 falls (NESdev "CPU pinout", "Cartridge connector"), so the one clock edge that can catch it is
// the end of the cycle. Color Dreams (mapper 11): a 74377, "D0-7 are copied to Q0-7 when CLK rises while /WE is low"
// (NESdev "74377"). Mapper 78 (Holy Diver, Cosmo Carrier): two 74HC161 (NESdev "INES Mapper 078"). Mapper 140: the
// GNROM register moved to $6000-$7FFF (NESdev "GxROM"). Mapper 87/101: a 74139 decoder and a 7474 dual D flip-flop.
// BNROM: "a 74HC161 binary counter used as a quad D latch" (NESdev "BNROM"). Mapper 38: a 74161 loaded through a
// 74138 (NESdev "INES Mapper 038").
bool nes_cart_slot_device::register_loads_at_m2_fall() const
{
	switch (m_pcb_id)
	{
		case STD_AXROM:
		case STD_AMROM:
		case STD_CNROM:
		case STD_CPROM:
		case STD_GXROM:
		case STD_UXROM:
		case STD_UN1ROM:
		case UXROM_CC:
		case DIS_74X377:
		case DIS_74X139X74:
		case IREM_HOLYDIVR:
		case JALECO_JF16:
		case JALECO_JF11:
		case STD_BXROM:
		case DIS_74X161X138:
			return true;
		default:
			return false;
	}
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

// Include emulation of NES PCBs for softlist
#include "nes_pcb.hxx"


/*-------------------------------------------------

 Load .unf files (UNIF boards) and identify the required slot device

 -------------------------------------------------*/

// Include emulation of UNIF Boards for .unf files
#include "nes_unif.hxx"


/*-------------------------------------------------

 Load .nes files (iNES mappers) and identify the required slot devices

 -------------------------------------------------*/

// Include emulation of iNES Mappers for .nes files
#include "nes_ines.hxx"


std::pair<std::error_condition, std::string> nes_cart_slot_device::call_load()
{
	if (m_cart)
	{
		if (!loaded_through_softlist())
		{
			char magic[4];

			// Check first 4 bytes of the image to decide if it is UNIF or iNES
			// Unfortunately, many .unf files have been released as .nes, so we cannot rely on extensions only
			fread(magic, 4);

			if ((magic[0] == 'N') && (magic[1] == 'E') && (magic[2] == 'S'))    // If header starts with 'NES' it is iNES
			{
				if (length() <= 0x10)
					return std::make_pair(image_error::INVALIDLENGTH, "File contains iNES header with no data");

				call_load_ines();
			}
			else if ((magic[0] == 'U') && (magic[1] == 'N') && (magic[2] == 'I') && (magic[3] == 'F')) // If header starts with 'UNIF' it is UNIF
			{
				if (length() <= 0x20)
					return std::make_pair(image_error::INVALIDLENGTH, "File contains UNIF header with no data");

				call_load_unif();
			}
			else
			{
				return std::make_pair(image_error::INVALIDIMAGE, "File is not an iNES or UNIF cartridge image");
			}
		}
		else
			call_load_pcb();
	}

	return std::make_pair(std::error_condition(), std::string());
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
			uint32_t tot_size = m_cart->get_battery_size() + m_cart->get_mapper_sram_size();
			std::vector<uint8_t> temp_nvram(tot_size);
			if (m_cart->get_battery_size())
				memcpy(&temp_nvram[0], m_cart->get_battery_base(), m_cart->get_battery_size());
			if (m_cart->get_mapper_sram_size())
				memcpy(&temp_nvram[m_cart->get_battery_size()], m_cart->get_mapper_sram_base(), m_cart->get_mapper_sram_size());

			battery_save(&temp_nvram[0], tot_size);
		}
	}
}


/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

std::string nes_cart_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	if (hook.image_file())
	{
		// the largest NES 2.0 image in use is a few MiB: anything that cannot be read in full, or is larger than
		// any image could sensibly be, gets the default slot (and its load reports the problem)
		static constexpr uint64_t MAX_IMAGE_SIZE = 64 * 1024 * 1024;
		uint64_t len;
		if (hook.image_file()->length(len) || (len < 4) || (len > MAX_IMAGE_SIZE))
			return software_get_default_slot("nrom");
		std::vector<uint8_t> rom(len);

		auto const [err, actual] = read(*hook.image_file(), &rom[0], len);
		if (err || (actual != len))
			return software_get_default_slot("nrom");

		const char *slot_string = "nrom";
		if ((rom[0] == 'N') && (rom[1] == 'E') && (rom[2] == 'S'))
			slot_string = get_default_card_ines(hook, &rom[0], len);
		else if ((rom[0] == 'U') && (rom[1] == 'N') && (rom[2] == 'I') && (rom[3] == 'F'))
			slot_string = get_default_card_unif(&rom[0], len);

		return std::string(slot_string);
	}
	else
		return software_get_default_slot("nrom");
}


/*-------------------------------------------------
 read
 -------------------------------------------------*/

uint8_t nes_cart_slot_device::read_l(offs_t offset)
{
	if (m_cart)
	{
		// update open bus
		m_cart->set_open_bus(((offset + 0x4100) & 0xff00) >> 8);
		return m_cart->read_l(offset);
	}
	else
		return 0xff;
}

uint8_t nes_cart_slot_device::read_m(offs_t offset)
{
	if (m_cart)
	{
		// update open bus
		m_cart->set_open_bus(((offset + 0x6000) & 0xff00) >> 8);
		return m_cart->read_m(offset);
	}
	else
		return 0xff;
}

uint8_t nes_cart_slot_device::read_h(offs_t offset)
{
	if (m_cart)
	{
		// update open bus
		m_cart->set_open_bus(((offset + 0x8000) & 0xff00) >> 8);
		return m_cart->read_h(offset);
	}
	else
		return 0xff;
}

uint8_t nes_cart_slot_device::read_ex(offs_t offset)
{
	if (m_cart)
	{
		// update open bus
		m_cart->set_open_bus(((offset + 0x4020) & 0xff00) >> 8);
		return m_cart->read_ex(offset);
	}
	else
		return 0xff;
}


/*-------------------------------------------------
 write
 -------------------------------------------------*/

void nes_cart_slot_device::write_l(offs_t offset, uint8_t data)
{
	if (m_cart)
	{
		// update open bus
		m_cart->set_open_bus(data);
		m_cart->write_l(offset, data);
	}
}

void nes_cart_slot_device::write_m(offs_t offset, uint8_t data)
{
	if (m_cart)
	{
		// update open bus
		m_cart->set_open_bus(data);
		m_cart->write_m(offset, data);
	}
}

void nes_cart_slot_device::write_h(offs_t offset, uint8_t data)
{
	if (m_cart)
	{
		// update open bus
		m_cart->set_open_bus(data);
		m_cart->write_h(offset, data);
	}
}

void nes_cart_slot_device::write_ex(offs_t offset, uint8_t data)
{
	if (m_cart)
	{
		// update open bus
		m_cart->set_open_bus(data);
		m_cart->write_ex(offset, data);
	}
}
