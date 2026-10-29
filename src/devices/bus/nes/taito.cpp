// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Taito PCBs

 iNES mappers: 33, 48, 80, 82, 207, 552


 Here we emulate the following PCBs

 * Taito TC0190FMC/TC0190FMR [mapper 33]
 * Taito TC0690 and TC0190FMC+PAL16R4 [mapper 48]
 * Taito X1-005 [mapper 80, 207]
 * Taito X1-017 [mapper 552; mapper 82 for the older dumps whose PRG is in the pre-2020 bank order]


 ***********************************************************************************************************/

#include "emu.h"
#include "taito.h"

#include "video/ppu2c0x.h"      // this has to be included so that IRQ functions can access ppu2c0x_device::BOTTOM_VISIBLE_SCANLINE

#define LOG_UNHANDLED (1U << 1)

#ifdef NES_PCB_DEBUG
#define VERBOSE (LOG_UNHANDLED | LOG_GENERAL)
#else
#define VERBOSE (LOG_UNHANDLED)
#endif
#include "logmacro.h"


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_TC0190FMC,         nes_tc0190fmc_device,         "nes_tc0190fmc", "NES Cart Taito TC0190FMC PCB")
DEFINE_DEVICE_TYPE(NES_TC0190FMC_PAL16R4, nes_tc0190fmc_pal16r4_device, "nes_tc0190pal", "NES Cart Taito TC0190FMC + PAL16R4 PCB")
DEFINE_DEVICE_TYPE(NES_X1_005,            nes_x1_005_device,            "nes_x1_005",    "NES Cart Taito X1-005 PCB")
DEFINE_DEVICE_TYPE(NES_X1_017,            nes_x1_017_device,            "nes_x1_017",    "NES Cart Taito X1-017 PCB")


nes_tc0190fmc_device::nes_tc0190fmc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, type, tag, owner, clock)
{
}

nes_tc0190fmc_device::nes_tc0190fmc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_tc0190fmc_device(mconfig, NES_TC0190FMC, tag, owner, clock)
{
}

nes_tc0190fmc_pal16r4_device::nes_tc0190fmc_pal16r4_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_tc0190fmc_device(mconfig, NES_TC0190FMC_PAL16R4, tag, owner, clock), m_irq_count(0), m_irq_count_latch(0), m_irq_enable(0)
{
}

nes_x1_005_device::nes_x1_005_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_X1_005, tag, owner, clock), m_latch(0)
{
}

nes_x1_017_device::nes_x1_017_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_X1_017, tag, owner, clock), m_latch(0), m_irq_count(0), m_irq_count_latch(0), m_irq_enable(0)
{
}



void nes_tc0190fmc_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);
}

void nes_tc0190fmc_pal16r4_device::device_start()
{
	nes_tc0190fmc_device::device_start();
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_count_latch));
	save_item(NAME(m_irq_delay));
	save_item(NAME(m_irq_pending));
}

void nes_tc0190fmc_pal16r4_device::pcb_reset()
{
	nes_tc0190fmc_device::pcb_reset();

	m_irq_enable = 0;
	m_irq_delay = 0;
	m_irq_pending = false;
	m_irq_count = 0;
	m_irq_count_latch = 0;
}

void nes_x1_005_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));

	save_item(NAME(m_x1_005_ram));

	m_mapper_sram_size = 0x80;
	m_mapper_sram = m_x1_005_ram;
}

void nes_x1_005_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_latch = 0;
}

void nes_x1_017_device::device_start()
{
	common_start();

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_count_latch));

	save_item(NAME(m_latch));
	save_item(NAME(m_reg));
	save_item(NAME(m_mmc_vrom_bank));

	save_item(NAME(m_x1_017_ram));

	m_mapper_sram_size = 0x1400;
	m_mapper_sram = m_x1_017_ram;
}

void nes_x1_017_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_irq_enable = 0;
	m_irq_count = 0;
	m_irq_count_latch = 0;

	m_latch = 0;
	m_reg[0] = m_reg[1] = m_reg[2] = 0;
	std::fill(std::begin(m_mmc_vrom_bank), std::end(m_mmc_vrom_bank), 0x00);
}



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Taito TC0190FMC board emulation

 Games: Akira, Bakushou!! Jinsei Gekijou, Don Doko Don,
 Insector X, Operation Wolf, Power Blazer, Takeshi no
 Sengoku Fuuunji

 The registers are only decoded at 0x8000-0xbfff, by address bits A15, A13, A1
 and A0: 0x8000 is .MPP PPPP (mirroring and the 8K PRG bank at 0x8000), 0x8001
 is the 8K PRG bank at 0xa000, 0x8002 and 0x8003 are the 2K CHR banks and
 0xa000-0xa003 are the 1K CHR banks.

 iNES: mapper 33

 In MAME: Supported.

 -------------------------------------------------*/

void nes_tc0190fmc_device::write_h(offs_t offset, u8 data)
{
	LOG("tc0190fmc write_h, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x6003)
	{
		case 0x0000:
			set_nt_mirroring(BIT(data, 6) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			prg8_89(data & 0x3f);
			break;
		case 0x0001:
			prg8_ab(data & 0x3f);
			break;
		case 0x0002:
		case 0x0003:
			chr2_x((offset & 1) << 1, data, CHRROM);
			break;
		case 0x2000:
		case 0x2001:
		case 0x2002:
		case 0x2003:
			chr1_x((offset & 0x03) | 4, data, CHRROM);
			break;
	}
}

/*-------------------------------------------------

 Taito TC0690 (and TC0190FMC + PAL16R4) board emulation

 Games: Bakushou!! Jinsei Gekijou 3, Bubble Bobble 2,
 Captain Saver, Don Doko Don 2, Flintstones, Jetsons

 Mapper 33's banking with the mirroring bit at $E000 and
 an IRQ counter at $C000-$C003.

 The counter follows krzysiobal's test of the TC0690 of a
 Flintstones cartridge (the forum thread NESdev's INES
 Mapper 048 page links):
 - $C000 holds the value $C001 loads into an 8-bit counter
   that counts up
 - a clock that finds the counter at $FF sets the IRQ flag;
   the counter wraps to 0 and goes on counting, there is no
   automatic reload
 - /IRQ is low while the flag is set and IRQs are enabled
   ($C002 enables, $C003 disables); only $C001 clears the
   flag
 - the clock is a pattern table read with PPU A12 low that
   follows one with A12 high, and there is no M2 filter.
   With the background at $0000 and the sprites at $1000 that
   is the first background fetch after the sprite fetches,
   65 PPU clocks (21.67 CPU cycles) after the rise an MMC3
   counts. The PPU reports that rise and the counter is
   clocked 22 M2 falls later. The other arrangement
   (background at $1000, sprites at $0000), where the chip
   counts at the first sprite fetch, is not emulated: none
   of the six games uses it.

 The Jetsons - Cogswell's Caper: on the tested TC0690 this
 game's title shows a displaced scanline, and it does here.
 The chip of its own cartridge has not been tested (NESdev:
 "possible that there's actually yet another ASIC that we
 have not yet documented"). Dumps of it carry NES 2.0
 submapper 1, which NESdev does not define and which selects
 nothing here.

 iNES: mapper 48

 In MAME: Supported.

 -------------------------------------------------*/

void nes_tc0190fmc_pal16r4_device::irq_clock()
{
	if (m_irq_count == 0xff)
		m_irq_pending = true;
	m_irq_count++;
	update_irq();
}

void nes_tc0190fmc_pal16r4_device::update_irq()
{
	set_irq_line((m_irq_pending && m_irq_enable) ? ASSERT_LINE : CLEAR_LINE);
}

// counts the CPU cycles of the delay on the falls of M2
void nes_tc0190fmc_pal16r4_device::m2_w(int state, offs_t address, bool write)
{
	if (!state && m_irq_delay && !--m_irq_delay)
		irq_clock();
}

void nes_tc0190fmc_pal16r4_device::ppu_a12_rise()
{
	m_irq_delay = 22;
}

// the MMC3's filter is used to find the one rise per scanline the delayed clock is timed from (the chip itself has
// no M2 filter, see above)
bool nes_tc0190fmc_pal16r4_device::a12_rise_accepted(const attotime &low_since, const attotime &when)
{
	return (when > low_since) && ((cpu_cycles_at(when) - cpu_cycles_at(low_since)) >= 4);
}

// fallback path for a PPU not running in dot-renderer mode; see wants_a12()/m_a12_driven
void nes_tc0190fmc_pal16r4_device::hblank_irq(int scanline, bool vblank, bool blanked)
{
	if (m_a12_driven)
		return;
	if (scanline < ppu2c0x_device::BOTTOM_VISIBLE_SCANLINE && !blanked)
		irq_clock();
}

void nes_tc0190fmc_pal16r4_device::write_h(offs_t offset, u8 data)
{
	LOG("tc0190fmc pal16r4 write_h, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x6003)
	{
		case 0x0000:
			prg8_89(data);
			break;
		case 0x0001:
		case 0x0002:
		case 0x0003:
		case 0x2000:
		case 0x2001:
		case 0x2002:
		case 0x2003:
			nes_tc0190fmc_device::write_h(offset, data);
			break;
		case 0x4000:
			m_irq_count_latch = data;
			break;
		case 0x4001:
			// loads the counter and clears the IRQ flag (the only write that does)
			m_irq_count = m_irq_count_latch;
			m_irq_pending = false;
			update_irq();
			break;
		case 0x4002:
			m_irq_enable = 1;
			update_irq();
			break;
		case 0x4003:
			m_irq_enable = 0;
			update_irq();   // only gates /IRQ: the flag stays set
			break;
		case 0x6000:
			set_nt_mirroring(BIT(data, 6) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
	}
}

/*-------------------------------------------------

 Taito X1-005 board emulation

 Actually, Fudou Myouou Den uses a variant of the board with
 CIRAM, making necessary two distinct mappers & pcb_id.

 The registers are at 0x7ef0-0x7eff (and 0x7e70-0x7e7f, since A7 is not connected).
 Writing 0xa3 to 0x7ef8 or 0x7ef9 enables the 128 bytes of internal RAM at
 0x7f00-0x7fff.

 iNES: mappers 80 & 207

 -------------------------------------------------*/

void nes_x1_005_device::write_m(offs_t offset, u8 data)
{
	LOG("x1_005 write_m, offset: %04x, data: %02x\n", offset, data);

	if ((offset & 0x1f70) == 0x1e70) // A7 is not connected
	{
		u8 reg = offset & 0x0f;

		switch (reg)
		{
			case 0x0:
			case 0x1:
				reg <<= 1;
				chr2_x(reg, BIT(data, 1, 6), CHRROM);
				if (m_x1_005_alt_mirroring)
				{
					set_nt_page(reg + 0, CIRAM, BIT(data, 7), 1);
					set_nt_page(reg + 1, CIRAM, BIT(data, 7), 1);
				}
				break;
			case 0x2:
			case 0x3:
			case 0x4:
			case 0x5:
				chr1_x(reg + 2, data, CHRROM);
				break;
			case 0x6:
			case 0x7:
				if (!m_x1_005_alt_mirroring)
					set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
				break;
			case 0x8:
			case 0x9:
				m_latch = data;
				break;
			case 0xa:
			case 0xb:
			case 0xc:
			case 0xd:
			case 0xe:
			case 0xf:
				prg8_x((reg - 0x0a) >> 1, data);
				break;
		}
	}
	else if (offset >= 0x1f00 && m_latch == 0xa3)
		m_x1_005_ram[offset & 0x7f] = data;
	else
		LOGMASKED(LOG_UNHANDLED, "Taito X1-005 uncaught addr: %04x, value: %02x\n", offset + 0x6000, data);
}

u8 nes_x1_005_device::read_m(offs_t offset)
{
	LOG("x1_005 read_m, offset: %04x\n", offset);

	if (offset >= 0x1f00 && m_latch == 0xa3)
		return m_x1_005_ram[offset & 0x7f];

	return get_open_bus();
}

/*-------------------------------------------------

 Taito X1-017 board emulation

 Games: Kyuukyoku Harikiri Koushien, Kyuukyoku Harikiri
 Stadium 3, Kyuukyoku Harikiri - Heisei Gannenban,
 SD Keiji - Blader

 NESdev Taito X1-017: NES 2.0 mapper 552 is the order the mask
 ROM is really wired in (PRG A13-A18 on data bits 5-0); iNES
 mapper 82 is the order understood before January 2020, used by
 the older dumps: ..DC BA.., PRG A13-A16 on data bits 2-5.

 iNES: mapper 82
 NES 2.0: mapper 552

 In MAME: Supported.

 TODO: KH Koushien seems to be working except it needs to
 be reset once with a new NVRAM file. KH Heisei won't
 load "Single Game" menu option but other game modes work.

 -------------------------------------------------*/

void nes_x1_017_device::irq_clock()
{
	if ((m_irq_enable & 0x05) == 1 && m_irq_count) // counting enabled?
		m_irq_count--;
	if (!m_irq_count && BIT(m_irq_enable, 1))
		set_irq_line(ASSERT_LINE);
}

void nes_x1_017_device::set_chr()
{
	chr2_x(0 ^ m_latch, m_mmc_vrom_bank[0] >> 1, CHRROM);
	chr2_x(2 ^ m_latch, m_mmc_vrom_bank[1] >> 1, CHRROM);
	chr1_x(4 ^ m_latch, m_mmc_vrom_bank[2], CHRROM);
	chr1_x(5 ^ m_latch, m_mmc_vrom_bank[3], CHRROM);
	chr1_x(6 ^ m_latch, m_mmc_vrom_bank[4], CHRROM);
	chr1_x(7 ^ m_latch, m_mmc_vrom_bank[5], CHRROM);
}

void nes_x1_017_device::write_m(offs_t offset, u8 data)
{
	LOG("x1017 write_m, offset: %04x, data: %02x\n", offset, data);

	switch (offset)
	{
		case 0x1ef0:
		case 0x1ef1:
		case 0x1ef2:
		case 0x1ef3:
		case 0x1ef4:
		case 0x1ef5:
			m_mmc_vrom_bank[offset & 0x07] = data;
			set_chr();
			break;
		case 0x1ef6:
			set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
			m_latch = (data & 0x02) << 1;
			set_chr();
			break;
		case 0x1ef7:
		case 0x1ef8:
		case 0x1ef9:
			m_reg[(offset & 0x0f) - 7] = data;
			break;
		case 0x1efa:
		case 0x1efb:
		case 0x1efc:
			prg8_x((offset & 0x0f) - 0x0a, m_old_order ? ((data >> 2) & 0x0f) : bitswap<6>(data, 0, 1, 2, 3, 4, 5));
			break;
		case 0x1efd:
			m_irq_count_latch = data;
			break;
		case 0x1efe:
			m_irq_enable = data;
			if (!BIT(m_irq_enable, 0))
				m_irq_count = m_irq_count_latch ? (m_irq_count_latch + 2) * 16 : 17;
			if (!BIT(m_irq_enable, 1))
				set_irq_line(CLEAR_LINE);
			break;
		case 0x1eff:
			set_irq_line(CLEAR_LINE);
			m_irq_count = m_irq_count_latch ? (m_irq_count_latch + 1) * 16 : 1;
			break;
	}

	// 2+2+1 KB of Internal RAM can be independently enabled/disabled!
	if ((offset < 0x0800 && m_reg[0] == 0xca) ||
		(offset < 0x1000 && m_reg[1] == 0x69) ||
		(offset < 0x1400 && m_reg[2] == 0x84))
	{
		m_x1_017_ram[offset] = data;

		// 1st byte in each 1K page latches most recent read/write from that page
		m_x1_017_ram[offset & 0x1c00] = data;
	}
}

u8 nes_x1_017_device::read_m(offs_t offset)
{
	LOG("x1017 read_m, offset: %04x\n", offset);

	// 2+2+1 KB of Internal RAM can be independently enabled/disabled!
	if ((offset < 0x0800 && m_reg[0] == 0xca) ||
		(offset < 0x1000 && m_reg[1] == 0x69) ||
		(offset < 0x1400 && m_reg[2] == 0x84))
	{
		u8 ret = m_x1_017_ram[offset];

		// 1st byte in each 1K page latches most recent read/write from that page
		m_x1_017_ram[offset & 0x1c00] = ret;

		return ret;
	}

	return 0; // no open bus behavior due to pull-down resistors
}
