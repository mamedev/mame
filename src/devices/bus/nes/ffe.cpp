// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
/***********************************************************************************************************


 NES/Famicom cartridge emulation for FFE PCBs


 iNES mappers: 6, 8, 17


 Here we emulate the Front Fareast (FFE) Super Magic Card (SMC-801) and its predecessors
 (Magic Card 1M/2M). The three mapper numbers are the same physical hardware, differing only in
 documented initial register state:
 * nes_ffe4_device: iNES mapper 6 (Magic Card 1M/2M initial state)
 * nes_ffe3_device: iNES mapper 8 (synonym of mapper 6 submapper 4; "FFE3" is the copier board
                    name, it is not iNES mapper 3)
 * nes_ffe8_device: iNES mapper 17 (Super Magic Card initial state; likewise "FFE8" is the copier
                    board name, not iNES mapper 8)

 Register bit layouts and banking behaviour follow the NESdev Wiki "Super Magic Card" article
 (https://www.nesdev.org/wiki/Super_Magic_Card), cross-checked against "INES Mapper 006" and
 "INES Mapper 017". Where the older FanWen Yang document (nesdev.org/mapper6.txt) disagrees (notably
 which of $43FC/$43FE selects 2M vs. 4M banking), the wiki is followed.

 Known issues / scope limits:
 * GUI mode, the floppy disk controller/parallel port, and the remote-control protocol are not
   emulated: their driving code lives in the SMC's own 16 KiB BIOS ROM, which is never present
   in an iNES/NES 2.0 ROM dump. Only Play mode is implemented.
 * PA12 IRQ clocking (Super Magic Card mode register bit 4 = 1) uses wants_a12_level()/
   ppu_a12_level(), the unfiltered A12 level hook also used by MMC1 4K CHR mode.
   ppu2c0x_device::ppu_bus() reports each A12 level transition exactly once (both rise and fall),
   so ppu_a12_level(), which only counts state 1, increments the IRQ counter once per rising edge.
 * Nametables sourced from CHR memory (Super Magic Card mode register bit 1 = 0, "N") map the
   four nametables onto the 1 KiB CHR-RAM banks selected by $4518-$451B (NESdev Super Magic Card),
   using the cartridge-CHR-RAM nametable source (VRAM_NT); see update_nt().
 * CHR-RAM is allocated at the full 256 KiB the SMC-801 supports via vram_alloc() in pcb_start(),
   and WRAM at the full 32 KiB via prgram_alloc(), regardless of the sizes in the iNES/NES 2.0
   header.


 ***********************************************************************************************************/


#include "emu.h"
#include "ffe.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE (LOG_GENERAL)
#else
#define VERBOSE (0)
#endif
#include "logmacro.h"


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_FFE3, nes_ffe3_device, "nes_ff3", "NES Cart FFE Magic Card 1M PCB (iNES Mapper 008)")
DEFINE_DEVICE_TYPE(NES_FFE4, nes_ffe4_device, "nes_ff4", "NES Cart FFE Magic Card 1M/2M PCB (iNES Mapper 006)")
DEFINE_DEVICE_TYPE(NES_FFE8, nes_ffe8_device, "nes_ff8", "NES Cart FFE Super Magic Card PCB (iNES Mapper 017)")


nes_ffe4_device::nes_ffe4_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, type, tag, owner, clock)
	, m_latch_mode(0), m_prg_wprotect(false), m_mirror_group(false), m_mirror_sub(false)
	, m_banking_2m4m_enable(false), m_banking_is_2m(false), m_shared_chr_bank(0)
	, m_chr_1k_mode(false), m_nt_from_chr(false), m_mmc4_enabled(false)
	, m_irq_source_pa12(false), m_wram_bank(0)
	, m_play_mode(true), m_pass_through(false)
	, m_irq_count(0), m_irq_enable(false)
{
	std::fill(std::begin(m_quad_latch), std::end(m_quad_latch), 0);
	std::fill(std::begin(m_prg_4m), std::end(m_prg_4m), 0);
	std::fill(std::begin(m_chr_1k), std::end(m_chr_1k), 0);
	std::fill(std::begin(m_mmc4_latch), std::end(m_mmc4_latch), 1); // default to the $FE-side bank, matching MMC2/4 convention
}

nes_ffe4_device::nes_ffe4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_ffe4_device(mconfig, NES_FFE4, tag, owner, clock)
{
}

nes_ffe3_device::nes_ffe3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_ffe4_device(mconfig, NES_FFE3, tag, owner, clock)
{
}

nes_ffe8_device::nes_ffe8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_ffe4_device(mconfig, NES_FFE8, tag, owner, clock)
{
}


//-------------------------------------------------
//  start/reset
//-------------------------------------------------

void nes_ffe4_device::device_start()
{
	common_start();

	m_scratch.resize(SCRATCH_SIZE);

	save_item(NAME(m_latch_mode));
	save_item(NAME(m_prg_wprotect));
	save_item(NAME(m_mirror_group));
	save_item(NAME(m_mirror_sub));
	save_item(NAME(m_banking_2m4m_enable));
	save_item(NAME(m_banking_is_2m));
	save_item(NAME(m_shared_chr_bank));
	save_item(NAME(m_quad_latch));
	save_item(NAME(m_prg_4m));
	save_item(NAME(m_chr_1k_mode));
	save_item(NAME(m_nt_from_chr));
	save_item(NAME(m_mmc4_enabled));
	save_item(NAME(m_irq_source_pa12));
	save_item(NAME(m_wram_bank));
	save_item(NAME(m_play_mode));
	save_item(NAME(m_pass_through));
	save_item(NAME(m_chr_1k));
	save_item(NAME(m_mmc4_latch));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_scratch));
	// the trainer boot patches the reset vector in the PRG copy, and the copier's PRG memory is RAM that software can
	// write (see write_h()): both belong in the state with the flags that undo the patch
	save_item(NAME(m_power_on));
	save_item(NAME(m_trainer_boot));
	save_item(NAME(m_trainer_vec_ofs));
	save_item(NAME(m_trainer_entry));
	save_item(NAME(m_trainer_vec_save));
	save_item(NAME(m_trainer_stub_save));
	save_item(NAME(m_trainer_stub));
}

void nes_ffe4_device::pcb_start(running_machine &machine, uint8_t *ciram_ptr, bool cart_mounted)
{
	// the loader has already sized these from the header; vram_alloc()/prgram_alloc() are plain resizes, and the base
	// pcb_start() called below registers the vectors for save states after them, so the full sizes are what is saved
	vram_alloc(CHRRAM_SIZE);
	prgram_alloc(WRAM_SIZE);

	// the Super Magic Card holds all CHR in RAM, loaded from disk before the game starts: an image that carries CHR data
	// (mapper 17 files do) has it copied into that RAM, which is what every CHR mode here reads from
	if (m_vrom && m_vrom_size)
		std::copy_n(m_vrom, std::min<uint32_t>(m_vrom_size, m_vram.size()), m_vram.begin());

	nes_nrom_device::pcb_start(machine, ciram_ptr, cart_mounted);

	// the copier's PRG memory is DRAM: it is written by the trainer boot and by software (see write_h())
	if (m_prg && m_prg_size)
		save_pointer(NAME(m_prg), m_prg_size);
	m_power_on = true;   // the next pcb_reset() is the hard reset
}

void nes_ffe4_device::begin_trainer_boot(uint16_t entry)
{
	// the reset vector is read from the 8 KiB PRG bank currently mapped at $E000
	uint8_t *prg = get_prg_base();
	const offs_t vector_offset = m_prg_bank[3] * 0x2000 + 0x1ffc;
	m_trainer_vec_ofs = vector_offset;
	m_trainer_entry = entry;
	m_trainer_vec_save[0] = prg[vector_offset];
	m_trainer_vec_save[1] = prg[vector_offset + 1];
	prg[vector_offset] = entry & 0xff;
	prg[vector_offset + 1] = entry >> 8;
	m_trainer_boot = true;
}

// NESdev INES Mapper 006 (and 008 = 006 submapper 4): on a hard reset the trainer is initialized by JSRing to $7003
// before JMPing to the game's reset vector. The copier's BIOS does this; here a stub (JSR $7003 / JMP <reset address>)
// is placed in the board's PRG copy at $FFF4-$FFF9 and the reset vector points at it. Nothing in RAM is touched (trainers
// use their WRAM freely); every patched PRG byte is restored when the CPU reaches the game's own reset address, which is
// itself never patched
void nes_ffe4_device::begin_trainer_call_boot()
{
	uint8_t *prg = get_prg_base();
	const offs_t bank = m_prg_bank[3] * 0x2000;
	const uint16_t reset = prg[bank + 0x1ffc] | (prg[bank + 0x1ffd] << 8);
	if (reset >= 0xfff4 && reset <= 0xfff9)
		return;   // would overlap the stub
	const uint8_t stub[6] = { 0x20, 0x03, 0x70, 0x4c, uint8_t(reset & 0xff), uint8_t(reset >> 8) };
	for (int i = 0; i < 6; i++)
	{
		m_trainer_stub_save[i] = prg[bank + 0x1ff4 + i];
		prg[bank + 0x1ff4 + i] = stub[i];
	}
	m_trainer_stub = true;
	begin_trainer_boot(0xfff4);
	m_trainer_entry = reset;   // restore everything once the game's reset code is reached
}

void nes_ffe4_device::end_trainer_boot()
{
	uint8_t *prg = get_prg_base();
	prg[m_trainer_vec_ofs] = m_trainer_vec_save[0];
	prg[m_trainer_vec_ofs + 1] = m_trainer_vec_save[1];
	if (m_trainer_stub)
	{
		const offs_t stub_ofs = m_trainer_vec_ofs - 0x1ffc + 0x1ff4;
		for (int i = 0; i < 6; i++)
			prg[stub_ofs + i] = m_trainer_stub_save[i];
		m_trainer_stub = false;
	}
	m_trainer_boot = false;
}

void nes_ffe4_device::ffe_common_initialize(uint8_t config_4500, uint8_t reg_42ff, int banking_2m4m_init)
{
	std::fill(m_scratch.begin(), m_scratch.end(), 0);

	std::fill(std::begin(m_quad_latch), std::end(m_quad_latch), 0);
	std::fill(std::begin(m_prg_4m), std::end(m_prg_4m), 0);
	std::fill(std::begin(m_chr_1k), std::end(m_chr_1k), 0);
	m_mmc4_latch[0] = 1;
	m_mmc4_latch[1] = 1;

	// ---- 1M banking mode register: both known init points ($42FF for mapper 6 and mapper
	// 17) target an address with A1=1 (PRG write-protected / latch enabled) and A0=1 (V/H
	// mirroring group) ----
	m_latch_mode   = (reg_42ff >> 5) & 0x07;
	m_prg_wprotect = true;
	m_mirror_group = true;
	m_mirror_sub   = BIT(reg_42ff, 4);
	set_nt_mirroring(m_mirror_sub ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	m_shared_chr_bank = 0;

	// ---- 2M/4M PRG banking mode register ----
	switch (banking_2m4m_init)
	{
		case BANK_INIT_4M:
			m_banking_2m4m_enable = true;
			m_banking_is_2m = false;
			break;
		case BANK_INIT_2M:
			m_banking_2m4m_enable = true;
			m_banking_is_2m = true;
			break;
		default:
			m_banking_2m4m_enable = false;
			m_banking_is_2m = false;
			break;
	}

	// ---- Super Magic Card mode register ($4500) ----
	write_4500(config_4500);

	// ---- IRQ ----
	m_irq_count = 0;
	m_irq_enable = false;
	set_irq_line(CLEAR_LINE);

	update_prg();
	update_chr();
}

void nes_ffe4_device::pcb_reset()
{
	// iNES Mapper 006 documented init (INES Mapper 006 wiki article):
	//   $4500 = $42   (Play mode, WRAM bank 0, 8 KiB CHR-RAM granularity)
	//   $42FF = (submapper << 5) | (vertical ? 0x00 : 0x10)
	//   $43FF = $00   (2M/4M banking mode disabled)
	// The NES 2.0 submapper (the initial latch mode 0-7) is not passed to this device; it starts in latch mode 1,
	// which NESdev assigns to NES 1.0 files ("NES 1.0 files correspond to submapper 1").
	ffe_common_initialize(0x42, 0x20 | ((get_mirroring() == PPU_MIRROR_HORZ) ? 0x10 : 0x00), BANK_INIT_DISABLED);   // NESdev: $42FF = mode | (vertical ? 0x00 : 0x10)

	if (m_power_on && get_trainer())
		begin_trainer_call_boot();
	m_power_on = false;
}


/*-------------------------------------------------

 Mapper 8 (nes_ffe3_device, "FFE3 Copier Board")

 Known Boards: FFE3 Copier Board
 Games: Hacked versions of games

 Documented as a synonym of Mapper 6 submapper 4 (latch mode 4 = GNROM); same SMC-801
 hardware/register set as nes_ffe4_device, just with the latch mode forced to GNROM at reset.

 -------------------------------------------------*/

void nes_ffe3_device::pcb_reset()
{
	// NESdev: mapper 8 is mapper 6 submapper 4, $42FF = (submapper << 5) | (vertical ? 0x00 : 0x10)
	ffe_common_initialize(0x42, (4 << 5) | ((get_mirroring() == PPU_MIRROR_HORZ) ? 0x10 : 0x00), BANK_INIT_DISABLED);

	if (m_power_on && get_trainer())
		begin_trainer_call_boot();
	m_power_on = false;
}


/*-------------------------------------------------

 Mapper 17 (nes_ffe8_device, "FFE8 Copier Board")

 Known Boards: FFE8 Copier Board
 Games: Hacked versions of games

 -------------------------------------------------*/

void nes_ffe8_device::pcb_reset()
{
	// iNES Mapper 017 documented init (INES Mapper 017 wiki article):
	//   $4500 = $47   (Play mode, WRAM bank 0, 1 KiB CHR-RAM granularity enabled)
	//   $42FF = $20 | (vertical ? 0x00 : 0x10)
	//   $43FC = $00   (4M banking mode enabled)
	//   $4504-$4507 = number of 8 KiB PRG banks -4, -3, -2, -1 (the last 32 KiB of PRG)
	ffe_common_initialize(0x47, 0x20 | ((get_mirroring() == PPU_MIRROR_HORZ) ? 0x10 : 0x00), BANK_INIT_4M);   // NESdev: $42FF = $20 | (vertical ? 0x00 : 0x10)

	// the PRG registers must hold the last four banks at power-on: with all four at bank 0 the reset vector is read from
	// the wrong bank, and games without a trainer (Saiyuuki World 2) never start
	const int banks_8k = m_prg_chunks * 2;
	for (int i = 0; i < 4; i++)
		m_prg_4m[i] = (banks_8k - 4 + i) & 0xff;
	update_prg();

	// no trainer start for mapper 17: NESdev's +$000 rule breaks Parodius Da! [p1] (no entry at $7000) and Mesen does
	// not run trainers either; every other mapper 17 dump tested also runs without it
	m_power_on = false;
}


/*-------------------------------------------------
 shared banking helpers (all three mappers above)
 -------------------------------------------------*/

void nes_ffe4_device::update_prg()
{
	if (m_banking_2m4m_enable && !m_banking_is_2m)
	{
		// 4M PRG banking mode: $4504-$4507, 8 KiB bank granularity, full 512 KiB reach
		for (int i = 0; i < 4; i++)
			prg8_x(i, m_prg_4m[i]);
	}
	else if (m_banking_2m4m_enable && m_banking_is_2m)
	{
		// 2M PRG banking mode: top 6 bits of each $8000-FFFF quadrant latch, full 512 KiB reach
		for (int i = 0; i < 4; i++)
			prg8_x(i, (m_quad_latch[i] >> 2) & 0x3f);
	}
	else
	{
		// Latch-based compatibility modes (section 2.1 of the Super Magic Card article),
		// selected by m_latch_mode (0-7). All of these observe the single physical latch at
		// $8000-$FFFF, which write_8000() keeps mirrored into all 4 m_quad_latch[] slots
		// whenever 2M mode is not active - so reading slot [3] here is equivalent to reading
		// "the" latch.
		const uint8_t d = m_quad_latch[3];

		switch (m_latch_mode)
		{
			case 0: // UNROM:  D~[.... .PPP]
				prg16_89ab(d & 0x07);
				prg16_cdef(0x07);
				break;
			case 1: // UN1ROM+CHRSW:  D~[..BB BBCC]
				prg16_89ab((d >> 2) & 0x0f);
				prg16_cdef(0x07);
				break;
			case 2: // UOROM:  D~[.... PPPP]
				prg16_89ab(d & 0x0f);
				prg16_cdef(0x0f);
				break;
			case 3: // Reverse UOROM+CHRSW:  D~[..CC PPPP]
				prg16_89ab(0x0f);
				prg16_cdef(d & 0x0f);
				break;
			case 4: // GNROM:  D~[..PP ..CC]
				prg32((d >> 4) & 0x03);
				break;
			case 5: // CNROM-256:  D~[.... ..CC]
			case 6: // CNROM-128:  D~[.... ...C]
			case 7: // NROM-256:   D~[.... ...C]
				prg32(0x03);
				break;
		}
	}
}

void nes_ffe4_device::update_chr()
{
	if (m_chr_1k_mode)
	{
		if (!m_mmc4_enabled)
		{
			chr1_0(m_chr_1k[0], CHRRAM); chr1_1(m_chr_1k[1], CHRRAM);
			chr1_2(m_chr_1k[2], CHRRAM); chr1_3(m_chr_1k[3], CHRRAM);
			chr1_4(m_chr_1k[4], CHRRAM); chr1_5(m_chr_1k[5], CHRRAM);
			chr1_6(m_chr_1k[6], CHRRAM); chr1_7(m_chr_1k[7], CHRRAM);
			// nametables (CIRAM or CHR memory via $4518-$451B) are set by update_nt()
		}
		else
		{
			// MMC4 mode (NESdev Super Magic Card, 1 KiB CHR banking mode): PPU $0000-$0FFF takes its 4 KiB bank from
			// $4510/$4511 after a read of $0FD8-$0FDF and from $4512/$4513 after $0FE8-$0FEF; PPU $1000-$1FFF from
			// $4514/$4515 and $4516/$4517 likewise. The bank is in bits 7-2 (D~[CCCC CC..]), PPU A11-A10 pass through,
			// and PPU A10 still picks the even or the odd register of each pair. The latches change in ppu_latch().
			for (int slot = 0; slot < 8; slot++)
			{
				const int window = slot >> 2;
				const u8 reg = m_chr_1k[window * 4 + m_mmc4_latch[window] * 2 + (slot & 1)];
				chr1_x(slot, (reg & 0xfc) | (slot & 3), CHRRAM);
			}
		}
		return;
	}

	// 8 KiB CHR granularity. CC comes from either the 2M/4M banking register's shared CC
	// bits (when that mode is enabled) or directly from the active $8000-FFFF latch, which
	// the wiki states are always kept as mirrors of one another. NOTE: the CC field sits at
	// a *different* bit position depending on latch mode - bits1-0 for modes 1/4/5, bit0
	// only for mode 6, but bits5-4 for mode 3 (D~[..CC PPPP]) - so it must be extracted
	// per-mode rather than with one shared mask.
	const bool use_shared = m_banking_2m4m_enable;
	const uint8_t raw = use_shared ? 0 : m_quad_latch[3]; // only consulted below when !use_shared

	switch (m_latch_mode)
	{
		case 0: // UNROM: CHR fixed, bank 0, writable
		case 2: // UOROM: CHR fixed, bank 0, writable
			chr8(0, CHRRAM);
			break;
		case 1: // UN1ROM+CHRSW: CC at bits1-0
		case 4: // GNROM: CC at bits1-0
		case 5: // CNROM-256: CC at bits1-0
		{
			const uint8_t cc = use_shared ? m_shared_chr_bank : (raw & 0x03);
			chr8(cc, CHRRAM);
			break;
		}
		case 3: // Reverse UOROM+CHRSW: CC at bits5-4 (D~[..CC PPPP])
		{
			const uint8_t cc = use_shared ? m_shared_chr_bank : ((raw >> 4) & 0x03);
			chr8(cc, CHRRAM);
			break;
		}
		case 6: // CNROM-128: single bit (bit0) selects 8 KiB bank 0-1
		{
			const uint8_t c = use_shared ? (m_shared_chr_bank & 0x01) : (raw & 0x01);
			chr8(c, CHRRAM);
			break;
		}
		case 7: // NROM-256: CHR fixed, bank 0
			chr8(0, CHRRAM);
			break;
	}
}


//-------------------------------------------------
//  $42FC-$42FF: 1M banking mode register
//-------------------------------------------------

void nes_ffe4_device::write_42fx(offs_t offset, uint8_t data)
{
	// NESdev Super Magic Card, A~0100 0010 1111 11bM: A1 ("b") is PRG write-protect / latch enable, A0 ("M") with D4
	// selects the mirroring type (A0=0: one-screen page D4, A0=1: vertical/horizontal by D4). This matches the
	// documented power-on value $42FF = $20 | (vertical ? $00 : $10)
	const bool addr_hv_group = BIT(offset, 0); // address bit "M" (A0)
	const bool addr_wprotect = BIT(offset, 1); // address bit "b" (A1)
	const bool data_sub      = BIT(data, 4);   // data bit "M" (D4)

	m_latch_mode   = (data >> 5) & 0x07;
	m_prg_wprotect = addr_wprotect;
	m_mirror_group = addr_hv_group;
	m_mirror_sub   = data_sub;

	update_nt();

	update_prg();
	update_chr();
}


//-------------------------------------------------
//  $43FC-$43FF: 2M/4M PRG banking mode register
//-------------------------------------------------

void nes_ffe4_device::write_43fx(offs_t offset, uint8_t data)
{
	const bool addr_select_2m = BIT(offset, 1); // address bit "N"
	const bool addr_disable   = BIT(offset, 0); // address bit "M"

	if (addr_disable)
		m_banking_2m4m_enable = false;
	else
	{
		m_banking_2m4m_enable = true;
		m_banking_is_2m = addr_select_2m;
	}

	m_shared_chr_bank = data & 0x03;

	update_prg();
	update_chr();
}


//-------------------------------------------------
//  $8000-$FFFF: latch-based mode / 2M mode shared latch
//-------------------------------------------------

void nes_ffe4_device::write_8000(offs_t offset, uint8_t data)
{
	// "A bus-conflict-free latch at CPU $8000-$FFFF" - the wiki describes one logical latch
	// for the plain latch-based modes, but four independent per-quadrant latches for 2M PRG
	// banking mode (one per 8 KiB window, each also carrying mirror CC bits). We keep four
	// latch slots; outside of 2M mode every write updates all four in lock-step so that
	// switching into 2M mode later sees consistent values rather than stale per-quadrant data.
	const int quadrant = (offset >> 13) & 0x03; // $8000/$A000/$C000/$E000 -> 0-3

	if (m_banking_2m4m_enable && m_banking_is_2m)
		m_quad_latch[quadrant] = data;
	else
		std::fill(std::begin(m_quad_latch), std::end(m_quad_latch), data);

	m_shared_chr_bank = data & 0x03;

	update_prg();
	update_chr();
}

void nes_ffe4_device::write_h(offs_t offset, uint8_t data)
{
	// NESdev Super Magic Card: the latch is only active while PRG memory is write-protected ($42FC-$42FF address bit 1
	// set). With the bit clear the PRG memory, which is RAM on the copier, takes the write and the latch does not.
	if (m_prg_wprotect)
		write_8000(offset, data);
	else
		m_prg[m_prg_bank[BIT(offset, 13, 2)] * 0x2000 + (offset & 0x1fff)] = data;
}


//-------------------------------------------------
//  $4500: Super Magic Card mode register (write)
//-------------------------------------------------

void nes_ffe4_device::write_4500(uint8_t data)
{
	m_chr_1k_mode     = BIT(data, 0);
	m_nt_from_chr     = !BIT(data, 1); // N: 0 = nametables from CHR memory via $4518-$451B, 1 = CIRAM (update_nt)
	m_mmc4_enabled    = !BIT(data, 2); // 0 = enabled, 1 = disabled per spec
	m_irq_source_pa12 = BIT(data, 3);
	m_wram_bank       = (data >> 4) & 0x03;
	m_play_mode       = BIT(data, 6);
	m_pass_through    = BIT(data, 7);

	scratch_mirror(0x4500, data);
	update_chr();
	update_nt();
}


//-------------------------------------------------
//  $4510-$451B: 1 KiB CHR banking mode registers
//-------------------------------------------------

void nes_ffe4_device::write_1k_chr(offs_t offset, uint8_t data)
{
	const int reg = offset & 0x0f; // 0-7 = $4510-$4517, 8-11 = $4518-$451B

	if (reg < 12)
		m_chr_1k[reg] = data;

	scratch_mirror(0x4510 + reg, data);
	update_chr();
	if (reg >= 8)
		update_nt();
}

// NESdev Super Magic Card, $4500 N: 0 = the four nametables come from 1 KiB banks of the 256 KiB CHR-RAM selected by
// $4518-$451B; 1 = CIRAM, mirrored as set by the 1M banking mode register ($42Fx: A0 = group, D4 = page/arrangement)
void nes_ffe4_device::update_nt()
{
	if (m_nt_from_chr && get_vram_size() >= 0x400)
	{
		const int pages = get_vram_size() / 0x400;
		for (int page = 0; page < 4; page++)
			set_nt_page(page, VRAM_NT, m_chr_1k[8 + page] % pages, 1);
	}
	else if (!m_mirror_group)
		set_nt_mirroring(m_mirror_sub ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
	else
		set_nt_mirroring(m_mirror_sub ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}


//-------------------------------------------------
//  MMC4-style ppu_latch: paired 1 KiB/4 KiB CHR banking when MMC4 mode is active
//-------------------------------------------------

void nes_ffe4_device::ppu_latch(offs_t offset)
{
	if (!m_chr_1k_mode || !m_mmc4_enabled)
		return;

	if ((offset & 0x1ff8) == 0x0fd8)
		m_mmc4_latch[0] = 0;
	else if ((offset & 0x1ff8) == 0x0fe8)
		m_mmc4_latch[0] = 1;
	else if ((offset & 0x1ff8) == 0x1fd8)
		m_mmc4_latch[1] = 0;
	else if ((offset & 0x1ff8) == 0x1fe8)
		m_mmc4_latch[1] = 1;
	else
		return;

	update_chr();
}


//-------------------------------------------------
//  Scratch RAM write-through mirror ($5500-$551F cache of $4500-$451F)
//-------------------------------------------------

void nes_ffe4_device::scratch_mirror(uint16_t address, uint8_t data)
{
	if (address >= 0x4500 && address <= 0x451f)
		m_scratch[0x0500 + (address - 0x4500)] = data;
}


//-------------------------------------------------
//  IRQ counter ($4501 disable, $4502/$4503 LSB/MSB)
//
//  Counter is a 16-bit up-counter clocked by either M2 (CPU clock, handled below by
//  m2_w() once per M2 rise) or unfiltered PA12 (PPU A12 line, handled via
//  ppu_a12_level() - see note in the file header and on wants_a12_level() in ffe.h).
//  It fires an IRQ on the $FFFF -> $0000 wraparound in both cases.
//-------------------------------------------------

void nes_ffe4_device::irq_m2_tick()
{
	if (!m_irq_enable || m_irq_source_pa12)
		return;

	const uint16_t prior = m_irq_count;
	m_irq_count++;

	if (prior == 0xffff && m_irq_count == 0x0000)
	{
		LOG("ffe irq fired (M2)\n");
		set_irq_line(ASSERT_LINE);
	}
}

void nes_ffe4_device::ppu_a12_level(int state)
{
	if (!m_irq_enable || !m_irq_source_pa12 || !state) // only count rising edges (state == 1)
		return;

	const uint16_t prior = m_irq_count;
	m_irq_count++;

	if (prior == 0xffff && m_irq_count == 0x0000)
	{
		LOG("ffe irq fired (PA12)\n");
		set_irq_line(ASSERT_LINE);
	}
}


//-------------------------------------------------
//  memory map
//
//  Offsets below are relative to CPU $4100 (matching this device's pre-existing read_l/
//  write_l convention, confirmed by the register addresses used in the previous revision of
//  this file, e.g. case 0x1fe => $42FE, case 0x401 => $4501, case 0x410 => $4510).
//-------------------------------------------------

uint8_t nes_ffe4_device::read_l(offs_t offset)
{
	const uint16_t address = offset + 0x4100;

	if (address == 0x4500)
	{
		// State register 1 (read-only), NESdev bit order: bit 0 = A0 of $42Fx (mirroring group), bit 1 = A1 (PRG
		// write-protect), bit 2 = D4. The write side decodes A0/A1 the same way.
		uint8_t v = 0;
		v |= (m_mirror_group ? 1 : 0) << 0;
		v |= (m_prg_wprotect ? 1 : 0) << 1;
		v |= (m_mirror_sub   ? 1 : 0) << 2;
		v |= (m_latch_mode & 0x07) << 3;
		v |= 1 << 6; // button pressed: 0 = pressed; no physical button exists in emulation, report "not pressed"
		v |= 0 << 7; // byte waiting from parallel port: always false, parallel port not emulated
		return v;
	}
	if (address == 0x4501)
	{
		// State register 2 (read-only). Bits 2-7 report "current latch value for latch-based
		// modes" - interpreted as the same top-6-bits-of-the-raw-latch field that 2M PRG
		// banking mode itself uses from the identical physical latch (D~[PPPP PPCC]).
		uint8_t v = 0;
		v |= (m_banking_2m4m_enable && m_banking_is_2m)  ? 0x01 : 0x00;
		v |= (m_banking_2m4m_enable && !m_banking_is_2m) ? 0x02 : 0x00;
		v |= ((m_quad_latch[3] >> 2) & 0x3f) << 2;
		return v;
	}
	if (address >= 0x5000 && address <= 0x5fff)
		return m_scratch[address - 0x5000];

	LOG("ffe read_l uncaught: %04x\n", address);
	return get_open_bus();
}

void nes_ffe4_device::write_l(offs_t offset, uint8_t data)
{
	const uint16_t address = offset + 0x4100;

	LOG("ffe write_l, address: %04x, data: %02x\n", address, data);

	if (address >= 0x42fc && address <= 0x42ff)
	{
		write_42fx(address, data);
		return;
	}
	if (address >= 0x43fc && address <= 0x43ff)
	{
		write_43fx(address, data);
		return;
	}
	if (address == 0x4500)
	{
		write_4500(data);
		return;
	}
	if (address == 0x4501)
	{
		// IRQ Disable: acknowledges a pending IRQ and disables counting
		m_irq_enable = false;
		set_irq_line(CLEAR_LINE);
		return;
	}
	if (address == 0x4502)
	{
		// IRQ counter LSB: acknowledges a pending IRQ, loads low byte, does not arm
		set_irq_line(CLEAR_LINE);
		m_irq_count = (m_irq_count & 0xff00) | data;
		return;
	}
	if (address == 0x4503)
	{
		// IRQ counter MSB: acknowledges a pending IRQ, loads high byte, arms counting
		set_irq_line(CLEAR_LINE);
		m_irq_count = (uint16_t(data) << 8) | (m_irq_count & 0x00ff);
		m_irq_enable = true;
		return;
	}
	if (address >= 0x4504 && address <= 0x4507)
	{
		m_prg_4m[address - 0x4504] = data & 0x3f;
		scratch_mirror(address, data);
		update_prg();
		return;
	}
	if (address >= 0x4510 && address <= 0x451b)
	{
		write_1k_chr(address - 0x4500, data);
		return;
	}
	if (address >= 0x5000 && address <= 0x5fff)
	{
		// Scratch RAM is nominally a live-updated cache of write-only registers, but the BIOS
		// (GUI mode) also uses it as general-purpose working RAM; since GUI mode is out of
		// scope here, we simply allow direct writes for save-state/debug consistency - this
		// cannot be observed by any code path we actually emulate.
		m_scratch[address - 0x5000] = data;
		return;
	}

	LOG("ffe write_l uncaught: %04x value: %02x\n", address, data);
}

uint8_t nes_ffe4_device::read_m(offs_t offset)
{
	return m_prgram[m_wram_bank * 0x2000 + offset];
}

void nes_ffe4_device::write_m(offs_t offset, uint8_t data)
{
	m_prgram[m_wram_bank * 0x2000 + offset] = data;
}
