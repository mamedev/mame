// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Nintendo MMC-3 & MMC-6 PCBs

 iNES mappers: 4, 37, 47, 118, 119


 Here we emulate the following PCBs

 * Nintendo TxROM, aka MMC-3 [mapper 4]
 * Nintendo HKROM, aka MMC-6 [mapper 4]
 * Nintendo TxSROM [mapper 118]
 * Nintendo TQROM [mapper 119]
 * Nintendo NES-QJ [mapper 47]
 * Nintendo PAL-ZZ [mapper 37]


 Source: NESdev "MMC3" (revision of 14 October 2026), "MMC3 pinout", "MMC6", "INES Mapper 037".

 The MMC3 has no reset input (MMC3 pinout): a console reset keeps its registers and IRQ counter on the Nintendo boards.

 NOT DOCUMENTED (the choice made here):
  - Power-on values of R0-R7, $8000, $A000 and $A001 (NESdev: R6, R7 and $8000 unspecified): as set in
    mmc3_common_initialize().
  - Which chip a submapper 4 image has (Sharp MMC3A ORs $80 into the counter on a $C001 write, NEC MMC3A $F0,
    NEC MMC3B/C $FF): $80, the Sharp value, as for submapper 0.
  - Whether a register write resets the MC-ACC's divide-by-8 prescaler: it is not reset.
  - Whether a console reset clears the MMC6's registers (its pinout was not checked): they are reinitialized.

 Older notes: Ninja Gaiden 2 was reported with flashing background graphics in its second level; not checked on this
 code (stage 1-1 is clean). Mendel Palace, once reported as never working, plays.

 ***********************************************************************************************************/


#include "emu.h"
#include "mmc3.h"

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

DEFINE_DEVICE_TYPE(NES_TXROM,  nes_txrom_device,  "nes_txrom",  "NES Cart TxROM (MMC-3) PCB")
DEFINE_DEVICE_TYPE(NES_HKROM,  nes_hkrom_device,  "nes_hkrom",  "NES Cart HKROM (MMC-6) PCB")
DEFINE_DEVICE_TYPE(NES_TXSROM, nes_txsrom_device, "nes_txsrom", "NES Cart TxSROM PCB")
DEFINE_DEVICE_TYPE(NES_TQROM,  nes_tqrom_device,  "nes_tqrom",  "NES Cart TQROM PCB")
DEFINE_DEVICE_TYPE(NES_QJ_PCB, nes_qj_device,     "nes_qj",     "NES Cart NES-QJ PCB")
DEFINE_DEVICE_TYPE(NES_ZZ_PCB, nes_zz_device,     "nes_zz",     "NES Cart PAL-ZZ PCB")


nes_txrom_device::nes_txrom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, type, tag, owner, clock), m_mmc_mirror(0), m_prg_base(0), m_prg_mask(0), m_chr_base(0), m_chr_mask(0)
	, m_latch(0), m_wram_protect(0), m_alt_irq(0), m_irq_count(0), m_irq_count_latch(0), m_irq_clear(0), m_irq_enable(0)
	, m_c001_or(0), m_reload_locked(false), m_acc_prescaler(0), m_powered(false)
{
}

nes_txrom_device::nes_txrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_txrom_device(mconfig, NES_TXROM, tag, owner, clock)
{
}

nes_hkrom_device::nes_hkrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_txrom_device(mconfig, NES_HKROM, tag, owner, clock), m_wram_enable(0), m_mmc6_reg(0)
{
}

nes_txsrom_device::nes_txsrom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, type, tag, owner, clock)
{
}

nes_txsrom_device::nes_txsrom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txsrom_device(mconfig, NES_TXSROM, tag, owner, clock)
{
}

nes_tqrom_device::nes_tqrom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, type, tag, owner, clock)
{
}

nes_tqrom_device::nes_tqrom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_tqrom_device(mconfig, NES_TQROM, tag, owner, clock)
{
}

nes_qj_device::nes_qj_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_QJ_PCB, tag, owner, clock)
{
}

nes_zz_device::nes_zz_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_ZZ_PCB, tag, owner, clock)
{
}



void nes_txrom_device::mmc3_start()
{
	common_start();
	save_item(NAME(m_mmc_prg_bank));
	save_item(NAME(m_mmc_vrom_bank));
	save_item(NAME(m_mmc_mirror));
	save_item(NAME(m_latch));
	save_item(NAME(m_wram_protect));
	save_item(NAME(m_prg_base));
	save_item(NAME(m_prg_mask));
	save_item(NAME(m_chr_base));
	save_item(NAME(m_chr_mask));
	save_item(NAME(m_alt_irq));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_count_latch));
	save_item(NAME(m_irq_clear));
	save_item(NAME(m_c001_or));
	save_item(NAME(m_reload_locked));
	save_item(NAME(m_acc_prescaler));
	save_item(NAME(m_powered));
}

void nes_txrom_device::mmc3_common_initialize( int prg_mask, int chr_mask, int irq_type )
{
	m_mmc_prg_bank[0] = m_mmc_prg_bank[2] = 0xffe; // m_mmc_prg_bank[2] & m_mmc_prg_bank[3] remain always the same in most MMC3 variants
	m_mmc_prg_bank[1] = m_mmc_prg_bank[3] = 0xfff; // but some pirate clone mappers change them after writing certain registers

	// Point CHR banks to the first 8K of memory. This is needed for a few unlicensed carts that use VRAM and don't init banks properly.
	// This includes at least some Waixing games, EverQuest and Sanguozhi, and some multicarts, New Star 6 in 1 and Famicom Yarou Vol 1.
	m_mmc_vrom_bank[0] = 0;
	m_mmc_vrom_bank[1] = 2;
	m_mmc_vrom_bank[2] = 4;
	m_mmc_vrom_bank[3] = 5;
	m_mmc_vrom_bank[4] = 6;
	m_mmc_vrom_bank[5] = 7;
	m_mmc_vrom_bank[6] = 0;  // extension reg used by clone boards
	m_mmc_vrom_bank[7] = 0;  // extension reg used by clone boards

	m_mmc_mirror = 0;
	// The mirroring register powers up as 0, which is vertical, and the iNES header's mirroring flag does not apply to the
	// MMC3 or its clones (it only matters on four-screen boards). Without this a game that never writes $A000 kept the
	// header's setting, which disagreed with the register: Super Angry Birds is flagged horizontal, scrolls sideways across
	// two nametables, and drew its right-hand screen over its left. Boards that set their own mirroring do it after this.
	if (m_mirroring != PPU_MIRROR_4SCREEN)
		set_nt_mirroring(PPU_MIRROR_VERT);
	m_latch = 0;
	m_wram_protect = 0x80;

	m_prg_base = m_chr_base = 0;
	m_prg_mask = prg_mask;
	m_chr_mask = chr_mask;

	m_alt_irq = irq_type || (m_mmc3_irq_variant == 4);       // MMC6 and NEC MMC3 use the "old" IRQ behaviour
	m_irq_enable = 0;
	m_irq_count = m_irq_count_latch = 0;
	m_irq_clear = 0;
	m_c001_or = nintendo_mmc3() ? 0x80 : 0x00;
	m_reload_locked = false;
	m_acc_prescaler = 0;

	set_prg(m_prg_base, m_prg_mask);
	set_chr(m_chr_source, m_chr_base, m_chr_mask);
}

// the Nintendo boards with a Nintendo MMC3 (clone boards keep the behaviour this file always had)
bool nes_txrom_device::nintendo_mmc3() const
{
	return type() == NES_TXROM || type() == NES_TXSROM || type() == NES_TQROM || type() == NES_QJ_PCB || type() == NES_ZZ_PCB;
}

// the MMC3 has no reset input (NESdev MMC3 pinout): a console reset leaves its registers and its IRQ counter as they were
void nes_txrom_device::mmc3_soft_reset()
{
	set_prg(m_prg_base, m_prg_mask);
	set_chr(m_chr_source, m_chr_base, m_chr_mask);
	if (m_mirroring != PPU_MIRROR_4SCREEN)
		set_nt_mirroring(BIT(m_mmc_mirror, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

void nes_txrom_device::pcb_reset()
{
	// 4-screen mirroring is taken care in pcb_start, if needed...
	if (m_powered && nintendo_mmc3())
		mmc3_soft_reset();
	else
		mmc3_common_initialize(0xff, 0xff, 0);
	m_powered = true;
}


void nes_hkrom_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_mmc6_reg));
	save_item(NAME(m_wram_enable));

	save_item(NAME(m_mmc6_ram));

	m_mapper_sram_size = 0x400;
	m_mapper_sram = m_mmc6_ram;
}

void nes_hkrom_device::pcb_reset()
{
	mmc3_common_initialize(0xff, 0xff, 1);
	m_mmc6_reg = 0x00;   // the PRG RAM starts disabled, which holds $A001 at $00
	m_wram_enable = 0;
}


void nes_qj_device::pcb_reset()
{
	mmc3_common_initialize(0x0f, 0x7f, 0);
}


void nes_zz_device::pcb_reset()
{
	// the CIC reset line clears the outer bank latch (NESdev INES Mapper 037); the MMC3 itself has no reset input
	if (m_powered)
	{
		m_prg_base = m_chr_base = 0;
		m_prg_mask = 0x07;
		mmc3_soft_reset();
	}
	else
		mmc3_common_initialize(0x07, 0x7f, 0);
	m_powered = true;
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 TxROM (MMC3 based) board emulation

 iNES: mapper 4

 In MAME: Supported.

 Tested: blargg's mmc3_test, mmc3_test_2 and mmc3_irq_tests (18 of 18 pass); Mendel Palace (USA), Ninja Gaiden
 II - The Dark Sword of Chaos (USA), Super Mario Bros. 3 (USA) and Alien 3 (USA) (MC-ACC, submapper 3) play.

 -------------------------------------------------*/

/* Here, IRQ counter decrements every scanline. */
// NESdev MMC3: on each filtered A12 rise the counter is reloaded when it is 0 or a reload was requested, otherwise
// decremented; when it is 0 afterwards and IRQs are enabled, the IRQ is asserted (the "new" Sharp behaviour, which
// blargg's mmc3_test expects; the older behaviour, which only fires on a decrement to 0, is kept for m_alt_irq boards)
void nes_txrom_device::ppu_a12_rise()
{
	// Acclaim MC-ACC: a divide-by-8 prescaler on the falling edges of A12, with no filter (NESdev MMC3, "IRQ Specifics");
	// the PPU reports each fall here (set_a12_on_fall). NESdev does not say whether a register write resets the prescaler
	if (m_mmc3_irq_variant == 3)
	{
		m_acc_prescaler = (m_acc_prescaler + 1) & 7;
		if (m_acc_prescaler)
			return;
	}

	const int prior_count = m_irq_count;
	const bool reload = m_irq_clear;
	if ((m_irq_count == 0) || reload)
		m_irq_count = m_irq_count_latch;
	else
		m_irq_count--;

	if (m_irq_enable && (m_irq_count == 0) && (prior_count || reload || !m_alt_irq))
		set_irq_line(ASSERT_LINE);
	m_irq_clear = 0;
	// the "$C001 reload glitch": after the clock that used the reload flag, $C001 cannot set it again until the next clock
	m_reload_locked = reload;
}

// NESdev MMC3: the counter is clocked by a rising edge of A12 after A12 has stayed low for three falling edges of M2,
// and M2 falls once per CPU cycle. During rendering A12 is never low that long between two pattern fetches of the same
// table: the fetches are 4 PPU clocks apart, also across the end of a line, where the idle dot carries the next
// background pattern address (NESdev PPU rendering, cycle 0).
bool nes_txrom_device::a12_rise_accepted(const attotime &low_since, const attotime &when)
{
	if (m_mmc3_irq_variant == 3)
		return true;    // MC-ACC: every pulse counts toward the prescaler
	return (when > low_since) && ((cpu_cycles_at(when) - cpu_cycles_at(low_since)) >= 3);
}

void nes_txrom_device::hblank_irq(int scanline, bool vblank, bool blanked)
{
	if (m_a12_driven)
		return;
	if (scanline < ppu2c0x_device::BOTTOM_VISIBLE_SCANLINE)
	{
		int prior_count = m_irq_count;
		if ((m_irq_count == 0) || m_irq_clear)
			m_irq_count = m_irq_count_latch;
		else
			m_irq_count--;

		if (m_irq_enable && !blanked && (m_irq_count == 0) && (prior_count || m_irq_clear)) // once-per-line fallback for a PPU that reports no A12 edges
		{
			LOG("irq fired, scanline: %d\n", scanline);
			set_irq_line(ASSERT_LINE);
		}
	}
	m_irq_clear = 0;
}

// base MMC3 simply calls prg8_x
void nes_txrom_device::prg_cb( int start, int bank )
{
	prg8_x(start, bank);
}

// base MMC3 simply calls chr1_x
void nes_txrom_device::chr_cb( int start, int bank, int source )
{
	chr1_x(start, bank, source);
}


void nes_txrom_device::set_prg( int prg_base, int prg_mask )
{
	uint8_t prg_flip = (m_latch & 0x40) ? 2 : 0;

	prg_cb(0, prg_base | (m_mmc_prg_bank[0 ^ prg_flip] & prg_mask));
	prg_cb(1, prg_base | (m_mmc_prg_bank[1] & prg_mask));
	prg_cb(2, prg_base | (m_mmc_prg_bank[2 ^ prg_flip] & prg_mask));
	prg_cb(3, prg_base | (m_mmc_prg_bank[3] & prg_mask));
}

void nes_txrom_device::set_chr( uint8_t chr, int chr_base, int chr_mask )
{
	uint8_t chr_page = (m_latch & 0x80) >> 5;

	chr_cb(chr_page ^ 0, chr_base | ((m_mmc_vrom_bank[0] & ~0x01) & chr_mask), chr);
	chr_cb(chr_page ^ 1, chr_base | ((m_mmc_vrom_bank[0] |  0x01) & chr_mask), chr);
	chr_cb(chr_page ^ 2, chr_base | ((m_mmc_vrom_bank[1] & ~0x01) & chr_mask), chr);
	chr_cb(chr_page ^ 3, chr_base | ((m_mmc_vrom_bank[1] |  0x01) & chr_mask), chr);
	chr_cb(chr_page ^ 4, chr_base | (m_mmc_vrom_bank[2] & chr_mask), chr);
	chr_cb(chr_page ^ 5, chr_base | (m_mmc_vrom_bank[3] & chr_mask), chr);
	chr_cb(chr_page ^ 6, chr_base | (m_mmc_vrom_bank[4] & chr_mask), chr);
	chr_cb(chr_page ^ 7, chr_base | (m_mmc_vrom_bank[5] & chr_mask), chr);
}

void nes_txrom_device::txrom_write(offs_t offset, uint8_t data)
{
	uint8_t mmc_helper, cmd;

	LOG("txrom_write, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x6001)
	{
		case 0x0000:
			mmc_helper = m_latch ^ data;
			m_latch = data;

			// Has PRG Mode changed?
			if (mmc_helper & 0x40)
				set_prg(m_prg_base, m_prg_mask);

			// Has CHR Mode changed?
			if (mmc_helper & 0x80)
				set_chr(m_chr_source, m_chr_base, m_chr_mask);
			break;

		case 0x0001:
			cmd = m_latch & 0x07;
			switch (cmd)
			{
				case 0: case 1: // these do not need to be separated: we take care of them in set_chr!
				case 2: case 3: case 4: case 5:
					m_mmc_vrom_bank[cmd] = data;
					set_chr(m_chr_source, m_chr_base, m_chr_mask);
					break;
				case 6:
				case 7:
					m_mmc_prg_bank[cmd - 6] = data;
					set_prg(m_prg_base, m_prg_mask);
					break;
			}
			break;

		case 0x2000:
			m_mmc_mirror = data;
			if (m_mirroring != PPU_MIRROR_4SCREEN)
				set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;

		case 0x2001:
			m_wram_protect = data;
			break;

		case 0x4000:
			m_irq_count_latch = data;
			break;

		case 0x4001:
			// NESdev MMC3: the counter is set above 127 at once (Sharp MMC3: OR $80) and the reload flag is set for the next
			// clock, unless the last clock used it ("$C001 reload glitch"). Clone boards: the counter is cleared, no glitch
			if (m_c001_or)
			{
				m_irq_count |= m_c001_or;
				if (!m_reload_locked)
					m_irq_clear = 1;
			}
			else
			{
				m_irq_count = 0;
				m_irq_clear = 1;
			}
			break;

		case 0x6000:
			m_irq_enable = 0;
			set_irq_line(CLEAR_LINE);
			break;

		case 0x6001:
			m_irq_enable = 1;
			break;

		default:
			LOGMASKED(LOG_UNHANDLED, "txrom_write uncaught: %04x value: %02x\n", offset + 0x8000, data);
			break;
	}
}

void nes_txrom_device::write_m(offs_t offset, uint8_t data)
{
	LOG("txrom write_m, offset: %04x, data: %02x\n", offset, data);

	if (BIT(m_wram_protect, 7) && !BIT(m_wram_protect, 6))
	{
		if (!m_battery.empty())
			m_battery[offset & (m_battery.size() - 1)] = data;
		else if (!m_prgram.empty())
			m_prgram[offset & (m_prgram.size() - 1)] = data;
	}
}

uint8_t nes_txrom_device::read_m(offs_t offset)
{
	LOG("txrom read_m, offset: %04x\n", offset);

	if (BIT(m_wram_protect, 7))
	{
		if (!m_battery.empty())
			return m_battery[offset & (m_battery.size() - 1)];
		if (!m_prgram.empty())
			return m_prgram[offset & (m_prgram.size() - 1)];
	}

	return get_open_bus();
}


/*-------------------------------------------------

 HKROM (MMC6 based) board emulation

 iNES: mapper 4

 In MAME: Supported.

 Tested: blargg's mmc3_test 6-MMC6 passes. No MMC6 game was run.

 -------------------------------------------------*/

void nes_hkrom_device::write_m(offs_t offset, uint8_t data)
{
	uint8_t write_hi, write_lo;
	LOG("hkrom write_m, offset: %04x, data: %02x\n", offset, data);

	if (offset < 0x1000)
		return;

	// banks can be written only if both read & write is enabled!
	write_hi = ((m_mmc6_reg & 0xc0) == 0xc0);
	write_lo = ((m_mmc6_reg & 0x30) == 0x30);

	if (BIT(offset, 9) && write_hi) // access to upper half of 1k
		m_mmc6_ram[offset & 0x3ff] = data;

	if (!BIT(offset, 9) && write_lo)    // access to lower half of 1k
		m_mmc6_ram[offset & 0x3ff] = data;
}

uint8_t nes_hkrom_device::read_m(offs_t offset)
{
	LOG("hkrom read_m, offset: %04x\n", offset);

	if (offset < 0x1000)
		return get_open_bus();

	if (!(m_mmc6_reg & 0xa0))
		return get_open_bus();

	if (BIT(offset, 9) && BIT(m_mmc6_reg, 7))   // access to upper half of 1k when upper read is enabled
		return m_mmc6_ram[offset & 0x3ff];

	if (!BIT(offset, 9) && BIT(m_mmc6_reg, 5))  // access to lower half of 1k when lower read is enabled
		return m_mmc6_ram[offset & 0x3ff];

	// If only one bank is enabled for reading, the other reads back as zero
	return 0x00;
}

void nes_hkrom_device::write_h(offs_t offset, uint8_t data)
{
	uint8_t mmc6_helper;
	LOG("hkrom write_h, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x6001)
	{
		case 0x0000:
			mmc6_helper = m_latch ^ data;
			m_latch = data;

			// NESdev MMC6: bit 5 enables the internal PRG RAM; while it is clear the mapper continuously sets $A001 to $00,
			// so the RAM area is open bus and $A001 writes are ignored
			m_wram_enable = BIT(data, 5);
			if (!m_wram_enable)
				m_mmc6_reg = 0;

			// Has PRG Mode changed?
			if (BIT(mmc6_helper, 6))
				set_prg(m_prg_base, m_prg_mask);

			// Has CHR Mode changed?
			if (BIT(mmc6_helper, 7))
				set_chr(m_chr_source, m_chr_base, m_chr_mask);
			break;

		case 0x2001:
			if (m_wram_enable)
				m_mmc6_reg = data;
			break;

		case 0x4001:
			m_irq_count = 0;
			m_irq_clear = 1;
			break;

		default:
			txrom_write(offset, data);
			break;
	}
}

/*-------------------------------------------------

 TxSROM (MMC3 based) board emulation

 Games: Armadillo, Play Action Football, Pro Hockey, RPG
 Jinsei Game, Y's 3

 iNES: mapper 118

 In MAME: Supported.

 It also uses mmc3_irq.

 Tested: Goal! Two (USA) shows its title, menus and password screen.

 -------------------------------------------------*/

void nes_txsrom_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_a12_level));
}

// the MMC3's CHR A17 pin drives CIRAM A10 on these boards (NESdev MMC3 pinout, TKSROM and TLSROM), so the CHR ROM sees bank
// bits 0-6 only
void nes_txsrom_device::set_chr(u8 chr, int chr_base, int chr_mask)
{
	nes_txrom_device::set_chr(chr, chr_base, chr_mask & 0x7f);

	// the page pointers, for PPU A12 low; nt_r and nt_w below decide every access
	for (int i = 0; i < 4; i++)
		set_nt_page(i, CIRAM, ciram_a10(i), 1);
}

// CIRAM A10: bit 7 of the bank register for 1 KiB slot 0-7 of the PPU pattern space, after the CHR A12 inversion
int nes_txsrom_device::ciram_a10(int slot) const
{
	static constexpr u8 bank[8] = { 0, 0, 1, 1, 2, 3, 4, 5 };
	return BIT(m_mmc_vrom_bank[bank[slot ^ ((m_latch & 0x80) >> 5)]], 7);
}

// offset is PPU address - $2000. Rendering fetches nametables with PPU A12 low; a PPUDATA access to $3000-$3EFF, and the
// nametable read under a PPUDATA palette read ($3F00-$3FFF, passed here as $2F00-$2FFF), have it high
int nes_txsrom_device::nt_page(offs_t offset) const
{
	const int a12 = (BIT(offset, 12) || m_a12_level) ? 4 : 0;
	return ciram_a10(a12 | BIT(offset, 10, 2));
}

u8 nes_txsrom_device::nt_r(offs_t offset)
{
	return get_ciram_base()[(nt_page(offset) << 10) | (offset & 0x3ff)];
}

void nes_txsrom_device::nt_w(offs_t offset, u8 data)
{
	get_ciram_base()[(nt_page(offset) << 10) | (offset & 0x3ff)] = data;
}

void nes_txsrom_device::write_h(offs_t offset, u8 data)
{
	LOG("txsrom write_h, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x6001)
	{
		case 0x2000:
			break;
		default:
			txrom_write(offset, data);
			break;
	}
}

/*-------------------------------------------------

 TQROM (MMC3 based) board emulation

 Games: Pin Bot, High Speed

 iNES: mapper 119

 In MAME: Supported.

 It also uses mmc3_irq.

 Tested: Pin Bot (USA) plays.

 -------------------------------------------------*/

void nes_tqrom_device::chr_cb(int start, int bank, int source)
{
	if (BIT(bank, 6))
		chr1_x(start, bank & 0x07, CHRRAM);
	else
		chr1_x(start, bank & 0x3f, CHRROM);
}

/*-------------------------------------------------

 NES-QJ board (MMC3 variant for US 2-in-1 Nintendo cart
 Super Spike V'Ball + Nintendo World Cup)

 iNES: mapper 47

 In MAME: Supported.

 Tested: both games of Super Spike V'Ball + Nintendo World Cup (USA) play.

 -------------------------------------------------*/

void nes_qj_device::write_m(offs_t offset, u8 data)
{
	LOG("qj write_m, offset: %04x, data: %02x\n", offset, data);

	if (BIT(m_wram_protect, 7) && !BIT(m_wram_protect, 6))
	{
		m_prg_base = BIT(data, 0) << 4;
		m_chr_base = BIT(data, 0) << 7;
		set_prg(m_prg_base, m_prg_mask);
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

/*-------------------------------------------------

 PAL-ZZ board (MMC3 variant for European 3-in-1 Nintendo cart
 Super Mario Bros. + Tetris + Nintendo World Cup)

 iNES: mapper 37

 TODO: this board apparently only resets to the menu
 screen on systems with a CIC (the outer PRG lines
 are somehow tied to the CIC's reset line).

 In MAME: Supported.

 Tested: all three games of Super Mario Bros. + Tetris + Nintendo World Cup (Europe) play.

 -------------------------------------------------*/

void nes_zz_device::write_m(offs_t offset, u8 data)
{
	LOG("zz write_m, offset: %04x, data: %02x\n", offset, data);

	if (BIT(m_wram_protect, 7) && !BIT(m_wram_protect, 6))
	{
		// NESdev INES Mapper 037: Q2 is PRG A17 and CHR A17; Q0 and Q1 both set force PRG A16 high, otherwise the MMC3's
		// PRG A16 passes only when Q2 is set: 0-2 = 64K at $00000, 3 = $10000, 4-6 = 128K at $20000, 7 = 64K at $30000
		m_prg_base = ((data & 0x04) | (data & (data << 1) & 0x02)) << 2;
		m_prg_mask = (BIT(data, 2) && ((data & 0x03) != 0x03)) ? 0x0f : 0x07;
		m_chr_base = BIT(data, 2) << 7;
		set_prg(m_prg_base, m_prg_mask);
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}
