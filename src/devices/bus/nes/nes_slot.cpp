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
	// HACK: to reduce tagmap lookups for PPU-related IRQs, we add a hook to the
	// main NES CPU here, even if it does not belong to this device.
	, m_maincpu(*this, ":maincpu")
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

void device_nes_cart_interface::misc_rom_alloc(size_t size, const char *tag)
{
	if (m_misc_rom == nullptr)
	{
		std::string tempstring(tag);
		tempstring.append(NESSLOT_MISC_ROM_REGION_TAG);
		m_misc_rom = device().machine().memory().region_alloc(tempstring.c_str(), size, 1, ENDIANNESS_LITTLE)->base();
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

	bank &= (chr_chunks << (3 - shift)) - 1;
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

DECLARE_WRITE_LINE_MEMBER(device_nes_cart_interface::set_irq_line)
{
	// use hold_irq_line for HOLD_LINE semantics (not recommended)
	assert(state == ASSERT_LINE || state == CLEAR_LINE);

	m_maincpu->set_input_line(m6502_device::IRQ_LINE, state);
}

void device_nes_cart_interface::hold_irq_line()
{
	// hack which requires the CPU object
	m_maincpu->set_input_line(m6502_device::IRQ_LINE, HOLD_LINE);
}

void device_nes_cart_interface::reset_cpu()
{
	// another hack
	m_maincpu->set_pc(0xfffc);
}

//-------------------------------------------------
//  Other helpers
//-------------------------------------------------

// Helper function for the few mappers reading from 0x8000-0xffff for protection
// so that they can access the ROM after the protection handling (which overwrites
// the memory banks)
uint8_t device_nes_cart_interface::hi_access_rom(uint32_t offset)
{
	int bank = (offset & 0x6000) >> 13;
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
	int bank = offset >> 10;

	if (m_chr_src[bank] == CHRRAM)
		m_chr_access[bank][offset & 0x3ff] = data;
}

uint8_t device_nes_cart_interface::chr_r(offs_t offset)
{
	int bank = offset >> 10;
	return m_chr_access[bank][offset & 0x3ff];
}


void device_nes_cart_interface::nt_w(offs_t offset, uint8_t data)
{
	int page = (offset & 0xc00) >> 10;

	if (!m_nt_writable[page])
		return;

	m_nt_access[page][offset & 0x3ff] = data;
}

uint8_t device_nes_cart_interface::nt_r(offs_t offset)
{
	int page = (offset & 0xc00) >> 10;
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
	, m_crc_hack(0)
	, m_cart(nullptr)
	, m_pcb_id(NO_BOARD)
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
	m_cart = get_card_device();
}


void nes_cart_slot_device::pcb_start(uint8_t *ciram_ptr)
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


image_init_result nes_cart_slot_device::call_load()
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
				{
					logerror("%s only contains the iNES header and no data.\n", filename());
					return image_init_result::FAIL;
				}

				call_load_ines();
			}
			else if ((magic[0] == 'U') && (magic[1] == 'N') && (magic[2] == 'I') && (magic[3] == 'F')) // If header starts with 'UNIF' it is UNIF
			{
				if (length() <= 0x20)
				{
					logerror("%s only contains the UNIF header and no data.\n", filename());
					return image_init_result::FAIL;
				}

				call_load_unif();
			}
			else
			{
				logerror("%s is NOT a file in either iNES or UNIF format.\n", filename());
				seterror(image_error::INVALIDIMAGE, "File is neither iNES or UNIF format");
				return image_init_result::FAIL;
			}
		}
		else
			call_load_pcb();
	}

	return image_init_result::PASS;
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
		uint64_t len;
		hook.image_file()->length(len); // FIXME: check error return, guard against excessively large files
		std::vector<uint8_t> rom(len);

		size_t actual;
		hook.image_file()->read(&rom[0], len, actual); // FIXME: check error return or read returning short

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
		uint8_t val = m_cart->read_l(offset);
		// update open bus
		m_cart->set_open_bus(((offset + 0x4100) & 0xff00) >> 8);
		return val;
	}
	else
		return 0xff;
}

uint8_t nes_cart_slot_device::read_m(offs_t offset)
{
	if (m_cart)
	{
		uint8_t val = m_cart->read_m(offset);
		// update open bus
		m_cart->set_open_bus(((offset + 0x6000) & 0xff00) >> 8);
		return val;
	}
	else
		return 0xff;
}

uint8_t nes_cart_slot_device::read_h(offs_t offset)
{
	if (m_cart)
	{
		uint8_t val = m_cart->read_h(offset);
		// update open bus
		m_cart->set_open_bus(((offset + 0x8000) & 0xff00) >> 8);
		return val;
	}
	else
		return 0xff;
}

uint8_t nes_cart_slot_device::read_ex(offs_t offset)
{
	if (m_cart)
	{
		uint8_t val = m_cart->read_ex(offset);
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

void nes_cart_slot_device::write_l(offs_t offset, uint8_t data)
{
	if (m_cart)
	{
		m_cart->write_l(offset, data);
		// update open bus
		m_cart->set_open_bus(((offset + 0x4100) & 0xff00) >> 8);
	}
}

void nes_cart_slot_device::write_m(offs_t offset, uint8_t data)
{
	if (m_cart)
	{
		m_cart->write_m(offset, data);
		// update open bus
		m_cart->set_open_bus(((offset + 0x6000) & 0xff00) >> 8);
	}
}

void nes_cart_slot_device::write_h(offs_t offset, uint8_t data)
{
	if (m_cart)
	{
		m_cart->write_h(offset, data);
		// update open bus
		m_cart->set_open_bus(((offset + 0x8000) & 0xff00) >> 8);
	}
}

void nes_cart_slot_device::write_ex(offs_t offset, uint8_t data)
{
	if (m_cart)
	{
		m_cart->write_ex(offset, data);
		// update open bus
		m_cart->set_open_bus(((offset + 0x4020) & 0xff00) >> 8);
	}
}
