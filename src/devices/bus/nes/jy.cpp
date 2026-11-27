// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
/***********************************************************************************************************

 NES/Famicom cartridge emulation for J.Y. Company

 iNES mappers: 35, 90, 209, 211, 394

 The J.Y. Company ASIC (NESdev "J.Y. Company ASIC"), used by the later single-game cartridges and most of the
 multicarts. Mapper 209 is the standard board, mapper 90 the one whose solder jumper inhibits ROM nametables and
 extended mirroring, mapper 211 a duplicate of 209 (NESdev: no PCB with the nametable features forced on exists).
 Mapper 35 is routed to the mapper 209 board in nes_ines.hxx: NESdev says it is like mapper 209 with 8K of WRAM
 stated explicitly, and an exact duplicate of 209 when the NES 2.0 header has its own PRG-RAM field. CaH4e3, who
 assigned the number for his dump, says it is not a 209 clone (FCEUX bug 841); that is not settled here. The one
 sheet ROM, Wario Land 2 (Asia) (Ja) (Pirate) (a Kirby's Adventure hack: 512K PRG, 256K CHR, 8K battery PRG-NVRAM),
 plays on the 209 board (file select, Vegetable Valley, the copy ability screens).

 Registers (decoded with the masks given by NESdev; writes with address bit 11 set are ignored where noted):
   $5000/$5400/$5C00  read: solder pad register, bits 7-6 (fully decoded)
   $5800-$5803 (F803) multiplier (write operand 1, operand 2; read product low, high), accumulator ($5802: add,
					  $5803: clear and set the test register; both readable)
   $6000-$7FFF        WRAM or open bus, or the PRG-ROM bank of $8003 when $D000 bit 7 is set
   $8000-$8003 (F803) PRG banks, 7 bits
   $9000-$9007 (F807) CHR bank low bytes, $A000-$A007 high bytes
   $B000-$B003 (F807) nametable low bytes, $B004-$B007 high bytes
   $C000-$C007 (F007) IRQ; address bit 11 does not matter here
   $D000-$D003 (F803) mode, nametable arrangement, PPU memory configuration, outer bank

 IRQ: the prescaler is clocked from CPU M2, unfiltered PPU A12 rises, every PPU read or every CPU write cycle
 ($C001 bits 0-1), counting up or down ($C001 bits 6-7); a prescaler wrap clocks the counter and a counter wrap
 raises the IRQ. Disabling the IRQ ($C000 bit 0 clear, $C002) acknowledges it, inhibits counting and clears the
 prescaler.

 The J.Y. multicart PCBs (NES 2.0 mappers 281, 282, 295, 358, 386, 387, 388, 397) are the ASIC with the outer bank
 register $D003 wired per PCB (NESdev, each mapper's page). PRG/CHR inner sizes and the outer bits, b = $D003 bit:
   281: PRG 256K, CHR 256K; b1-0 = PRG A19-A18 and CHR A19-A18
   282: PRG 256K, b2-1 = PRG A19-A18; CHR 256K (b0 = A18) or 512K (b5 set), CHR A19 = b3 while b4 is set
		(NESdev: b4 "0: ignore bit 3, 1: obey bit 3"; FCEUmm instead takes b4 as CHR A20)
   295: PRG 128K, CHR 128K; b2-0 = PRG A19-A17 and CHR A19-A17
   358: PRG 256K, b2-1 = PRG A19-A18; CHR 256K (b0 = A18) or 512K (b5 set), b2 = CHR A19
   386: PRG 256K, b1 = PRG A18, b3 = PRG A19; CHR 256K (b0 = A18) or 512K (b5 set), b2 = CHR A19
   387: as 386 with PRG 128K: b1 = PRG A17, b3 = PRG A18
   388: PRG 256K, b2 = PRG A18, b3 = PRG A19; CHR 256K (b0 = A18) or 512K (b5 set), b1 = CHR A19; the PCB inhibits
		ROM nametables and extended mirroring, as mapper 90
   397: PRG 256K, b2-1 = PRG A19-A18; CHR 128K, b2-0 = CHR A19-A17
 Not documented, so taken from FCEUmm: the ROM nametable banks of these PCBs get the same mask and outer bits as the
 CHR banks (NESdev only names the $9000-$A007 banks; the standard board applies no outer bits to them).

 Mapper 394 is the Realtec HSK007 multicart board, which can run as BNROM, as an MMC3 clone or as this ASIC. It is
 here because it extends the ASIC's bank logic with its own outer bits (see nes_jy_hsk007_device below).

 Not emulated, because nothing is documented about it (NESdev: no known game uses it):
 - the "funky" IRQ mode ($C001 bit 3, $C007)
 - the intermediate results of the multiplier during the 8 M2 cycles after the second operand is written
 - boards that do not connect CPU A11 (address masks $F000, multiplier unreachable)

 The power-on state of the registers is not known: all of them are cleared here (32K PRG mode with the last
 bank fixed, 8K CHR mode, bank 0).

 ***********************************************************************************************************/


#include "emu.h"
#include "jy.h"

#ifdef NES_PCB_DEBUG
#define VERBOSE (LOG_GENERAL)
#else
#define VERBOSE (0)
#endif
#include "logmacro.h"


//-------------------------------------------------
//  constructor
//-------------------------------------------------

// The three boards and the iNES mappers that select them (nes_ines.hxx):
//   Type A (JYCOMPANY_A, slot options "jyc_a" and "tek90"): mapper 90
//   Type B (JYCOMPANY_B, slot option "jyc_b"):              mapper 211
//   Type C (JYCOMPANY_C, slot option "jyc_c"):              mappers 209 and 35
DEFINE_DEVICE_TYPE(NES_JY_TYPEA, nes_jy_typea_device, "nes_jya", "NES Cart JY Company Type A PCB")   // iNES mapper 90
DEFINE_DEVICE_TYPE(NES_JY_TYPEB, nes_jy_typeb_device, "nes_jyb", "NES Cart JY Company Type B PCB")   // iNES mapper 211
DEFINE_DEVICE_TYPE(NES_JY_TYPEC, nes_jy_typec_device, "nes_jyc", "NES Cart JY Company Type C PCB")   // iNES mappers 209 and 35
DEFINE_DEVICE_TYPE(NES_JY_281, nes_jy_281_device, "nes_jy281", "NES Cart JY Company multicart PCB (NES 2.0 mapper 281)")
DEFINE_DEVICE_TYPE(NES_JY_282, nes_jy_282_device, "nes_jy282", "NES Cart JY Company multicart PCB (NES 2.0 mapper 282)")
DEFINE_DEVICE_TYPE(NES_JY_295, nes_jy_295_device, "nes_jy295", "NES Cart JY Company multicart PCB (NES 2.0 mapper 295)")
DEFINE_DEVICE_TYPE(NES_JY_358, nes_jy_358_device, "nes_jy358", "NES Cart JY Company multicart PCB (NES 2.0 mapper 358)")
DEFINE_DEVICE_TYPE(NES_JY_386, nes_jy_386_device, "nes_jy386", "NES Cart JY Company multicart PCB (NES 2.0 mapper 386)")
DEFINE_DEVICE_TYPE(NES_JY_387, nes_jy_387_device, "nes_jy387", "NES Cart JY Company multicart PCB (NES 2.0 mapper 387)")
DEFINE_DEVICE_TYPE(NES_JY_388, nes_jy_388_device, "nes_jy388", "NES Cart JY Company multicart PCB (NES 2.0 mapper 388)")
DEFINE_DEVICE_TYPE(NES_JY_397, nes_jy_397_device, "nes_jy397", "NES Cart JY Company multicart PCB (NES 2.0 mapper 397)")
DEFINE_DEVICE_TYPE(NES_JY_HSK007, nes_jy_hsk007_device, "nes_jy_hsk007", "NES Cart Realtec HSK007 PCB")


static INPUT_PORTS_START( jy_asic )
	PORT_START("JUMPER")
	PORT_CONFNAME( 0xc0, 0x00, "Solder Pads" )
	PORT_CONFSETTING(    0x00, "00" )
	PORT_CONFSETTING(    0x40, "01" )
	PORT_CONFSETTING(    0x80, "10" )
	PORT_CONFSETTING(    0xc0, "11" )
INPUT_PORTS_END

ioport_constructor nes_jy_typea_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( jy_asic );
}


nes_jy_typea_device::nes_jy_typea_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool nt_jumper)
	: nes_nrom_device(mconfig, type, tag, owner, clock)
	, m_jumper(*this, "JUMPER")
	, m_nt_jumper(nt_jumper)
	, m_acc(0), m_test(0)
	, m_mode(0), m_mirror(0), m_ppucfg(0), m_outer(0)
	, m_bank_6000(0)
	, m_irq_enable(0), m_irq_source(0), m_irq_dir(0), m_irq_small(0), m_irq_funky(0)
	, m_irq_prescaler(0), m_irq_counter(0), m_irq_xor(0), m_irq_c007(0), m_a12(0)
{
}

nes_jy_typea_device::nes_jy_typea_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_jy_typea_device(mconfig, NES_JY_TYPEA, tag, owner, clock, true)
{
}

nes_jy_typeb_device::nes_jy_typeb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: nes_jy_typea_device(mconfig, type, tag, owner, clock, false)
{
}

nes_jy_typeb_device::nes_jy_typeb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_jy_typeb_device(mconfig, NES_JY_TYPEB, tag, owner, clock)
{
}

nes_jy_typec_device::nes_jy_typec_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_jy_typeb_device(mconfig, NES_JY_TYPEC, tag, owner, clock)
{
}

nes_jy_mc_device::nes_jy_mc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int pcb, bool nt_jumper)
	: nes_jy_typea_device(mconfig, type, tag, owner, clock, nt_jumper)
	, m_pcb(pcb)
{
}

nes_jy_281_device::nes_jy_281_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_jy_mc_device(mconfig, NES_JY_281, tag, owner, clock, 281, false)
{
}

nes_jy_282_device::nes_jy_282_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_jy_mc_device(mconfig, NES_JY_282, tag, owner, clock, 282, false)
{
}

nes_jy_295_device::nes_jy_295_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_jy_mc_device(mconfig, NES_JY_295, tag, owner, clock, 295, false)
{
}

nes_jy_358_device::nes_jy_358_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_jy_mc_device(mconfig, NES_JY_358, tag, owner, clock, 358, false)
{
}

nes_jy_386_device::nes_jy_386_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_jy_mc_device(mconfig, NES_JY_386, tag, owner, clock, 386, false)
{
}

nes_jy_387_device::nes_jy_387_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_jy_mc_device(mconfig, NES_JY_387, tag, owner, clock, 387, false)
{
}

nes_jy_388_device::nes_jy_388_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_jy_mc_device(mconfig, NES_JY_388, tag, owner, clock, 388, true)
{
}

nes_jy_397_device::nes_jy_397_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_jy_mc_device(mconfig, NES_JY_397, tag, owner, clock, 397, false)
{
}


void nes_jy_typea_device::device_start()
{
	common_start();

	save_item(NAME(m_mul));
	save_item(NAME(m_acc));
	save_item(NAME(m_test));
	save_item(NAME(m_prg_reg));
	save_item(NAME(m_chr_lo));
	save_item(NAME(m_chr_hi));
	save_item(NAME(m_nt_lo));
	save_item(NAME(m_nt_hi));
	save_item(NAME(m_mode));
	save_item(NAME(m_mirror));
	save_item(NAME(m_ppucfg));
	save_item(NAME(m_outer));
	save_item(NAME(m_chr_latch));
	save_item(NAME(m_bank_6000));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_source));
	save_item(NAME(m_irq_dir));
	save_item(NAME(m_irq_small));
	save_item(NAME(m_irq_funky));
	save_item(NAME(m_irq_prescaler));
	save_item(NAME(m_irq_counter));
	save_item(NAME(m_irq_xor));
	save_item(NAME(m_irq_c007));
	save_item(NAME(m_a12));
}

void nes_jy_typea_device::pcb_reset()
{
	std::fill(std::begin(m_mul), std::end(m_mul), 0);
	m_acc = m_test = 0;
	std::fill(std::begin(m_prg_reg), std::end(m_prg_reg), 0);
	std::fill(std::begin(m_chr_lo), std::end(m_chr_lo), 0);
	std::fill(std::begin(m_chr_hi), std::end(m_chr_hi), 0);
	std::fill(std::begin(m_nt_lo), std::end(m_nt_lo), 0);
	std::fill(std::begin(m_nt_hi), std::end(m_nt_hi), 0);
	m_mode = m_mirror = m_ppucfg = m_outer = 0;
	m_chr_latch[0] = m_chr_latch[1] = 0;
	m_bank_6000 = 0;

	m_irq_enable = 0;
	m_irq_source = 0;
	m_irq_dir = 0;
	m_irq_small = 0;
	m_irq_funky = 0;
	m_irq_prescaler = 0;
	m_irq_counter = 0;
	m_irq_xor = 0;
	m_irq_c007 = 0;
	m_a12 = 0;

	update_prg();
	update_chr();
	update_nt();
}


//-------------------------------------------------
//  banking
//-------------------------------------------------

void nes_jy_typea_device::update_prg()
{
	auto prg_x = [this] (int slot, int bank) { prg8_x(slot, prg_bank_filter(bank)); };

	const int mode = m_mode & 3;
	const bool sw_last = BIT(m_mode, 2);
	const int outer = asic_prg_outer();          // outer PRG bits, in 8K banks
	const int m8 = asic_prg_mask();        // the $8000-$8003 banks are masked to the outer bank size
	const int m16 = m8 >> 1;
	const int m32 = m8 >> 2;

	// mode 3 is mode 2 with bits 0-6 of the bank numbers reversed
	auto reg = [this, mode] (int i)
	{
		const uint8_t r = m_prg_reg[i] & 0x7f;
		return (mode == 3) ? bitswap<7>(r, 0, 1, 2, 3, 4, 5, 6) : r;
	};

	switch (mode)
	{
	case 0: // 32K
		{
			const int bank = outer | ((sw_last ? (reg(3) & m32) : m32) << 2);
			for (int i = 0; i < 4; i++)
				prg_x(i, bank + i);
			m_bank_6000 = prg_bank_filter(outer | (((reg(3) << 2) | 3) & m8));
		}
		break;

	case 1: // 16K
		{
			const int lower = outer | ((reg(1) & m16) << 1);
			const int upper = outer | ((sw_last ? (reg(3) & m16) : m16) << 1);
			prg_x(0, lower);
			prg_x(1, lower + 1);
			prg_x(2, upper);
			prg_x(3, upper + 1);
			m_bank_6000 = prg_bank_filter(outer | (((reg(3) << 1) | 1) & m8));
		}
		break;

	case 2: // 8K
	case 3: // 8K, bank bits reversed
		prg_x(0, outer | (reg(0) & m8));
		prg_x(1, outer | (reg(1) & m8));
		prg_x(2, outer | (reg(2) & m8));
		prg_x(3, outer | (sw_last ? (reg(3) & m8) : m8));
		m_bank_6000 = prg_bank_filter(outer | (reg(3) & m8));
		break;
	}
}

// bank number, in units of 1K >> shift, of CHR register reg: the register is masked to 256K (outer bit A18 from the
// outer bank register) or to 512K (outer bit ignored), with A19-A20 always coming from the outer bank register
uint32_t nes_jy_typea_device::chr_window(int reg, int shift) const
{
	const uint32_t mask = asic_chr_mask();
	const uint32_t outer = asic_chr_outer();
	const uint32_t value = m_chr_lo[reg] | (uint32_t(m_chr_hi[reg]) << 8);

	return chr_bank_filter(((value & (mask >> shift)) | (outer >> shift)) << shift) >> shift;
}

void nes_jy_typea_device::update_chr()
{
	const bool mmc4 = BIT(m_outer, 7);

	switch (BIT(m_mode, 3, 2))
	{
	case 0: // 8K
		chr8(chr_window(0, 3), m_chr_source);
		break;

	case 1: // 4K, with the MMC4-like latches picking the second register of each half
		chr4_x(0, chr_window(mmc4 ? (m_chr_latch[0] << 1) : 0, 2), m_chr_source);
		chr4_x(4, chr_window(mmc4 ? (4 | (m_chr_latch[1] << 1)) : 4, 2), m_chr_source);
		break;

	case 2: // 2K
		for (int i = 0; i < 8; i += 2)
			chr2_x(i, chr_window(i, 1), m_chr_source);
		break;

	case 3: // 1K
		for (int i = 0; i < 8; i++)
			chr1_x(i, chr_window(i, 0), m_chr_source);
		break;
	}
}

// J.Y. multicart PCBs: inner masks and outer bits of $D003 (see the header comment)
uint32_t nes_jy_mc_device::asic_prg_mask() const
{
	switch (m_pcb)
	{
	case 295: case 387: return 0x0f;    // 128K
	default:            return 0x1f;    // 256K
	}
}

uint32_t nes_jy_mc_device::asic_prg_outer() const
{
	const uint8_t o = asic_outer_reg();
	switch (m_pcb)
	{
	case 281: return uint32_t(BIT(o, 0, 2)) << 5;
	case 295: return uint32_t(BIT(o, 0, 3)) << 4;
	case 386: return (uint32_t(BIT(o, 1)) << 5) | (uint32_t(BIT(o, 3)) << 6);
	case 387: return (uint32_t(BIT(o, 1)) << 4) | (uint32_t(BIT(o, 3)) << 5);
	case 388: return uint32_t(BIT(o, 2, 2)) << 5;
	default:  return uint32_t(BIT(o, 1, 2)) << 5;   // 282, 358, 397
	}
}

uint32_t nes_jy_mc_device::asic_chr_mask() const
{
	switch (m_pcb)
	{
	case 281:           return 0xff;    // 256K
	case 295: case 397: return 0x7f;    // 128K
	default:            return BIT(asic_outer_reg(), 5) ? 0x1ff : 0xff;
	}
}

uint32_t nes_jy_mc_device::asic_chr_outer() const
{
	const uint8_t o = asic_outer_reg();
	const uint32_t a18 = BIT(o, 5) ? 0 : (uint32_t(BIT(o, 0)) << 8);   // used only with the 256K mask
	switch (m_pcb)
	{
	case 281:           return uint32_t(BIT(o, 0, 2)) << 8;
	case 295: case 397: return uint32_t(BIT(o, 0, 3)) << 7;
	case 282:           return a18 | ((BIT(o, 4) ? uint32_t(BIT(o, 3)) : 0) << 9);
	case 388:           return a18 | (uint32_t(BIT(o, 1)) << 9);
	default:            return a18 | (uint32_t(BIT(o, 2)) << 9);    // 358, 386, 387
	}
}

uint32_t nes_jy_mc_device::asic_nt_bank(uint32_t bank) const
{
	return (bank & asic_chr_mask()) | asic_chr_outer();
}

void nes_jy_typea_device::update_nt()
{
	// mapper 90 inhibits ROM nametables and extended mirroring by jumper
	const bool rom_nt = !m_nt_jumper && BIT(m_mode, 5);
	const bool extended = !m_nt_jumper && BIT(m_mirror, 3);

	if (rom_nt && m_vrom_size)
	{
		for (int i = 0; i < 4; i++)
		{
			// all nametables from ROM, or per nametable: $B00x bit 7 against $D002 bit 7
			if (BIT(m_mode, 6) || (BIT(m_nt_lo[i], 7) != BIT(m_ppucfg, 7)))
				set_nt_page(i, VROM, chr_bank_filter(asic_nt_bank(m_nt_lo[i] | (m_nt_hi[i] << 8))), 0);
			else
				set_nt_page(i, CIRAM, m_nt_lo[i] & 1, 1);
		}
	}
	else if (rom_nt || extended)
	{
		for (int i = 0; i < 4; i++)
			set_nt_page(i, CIRAM, m_nt_lo[i] & 1, 1);
	}
	else
	{
		switch (m_mirror & 3)
		{
		case 0: set_nt_mirroring(PPU_MIRROR_VERT); break;   // A10
		case 1: set_nt_mirroring(PPU_MIRROR_HORZ); break;   // A11
		case 2: set_nt_mirroring(PPU_MIRROR_LOW); break;
		case 3: set_nt_mirroring(PPU_MIRROR_HIGH); break;
		}
	}
}


//-------------------------------------------------
//  IRQ
//-------------------------------------------------

void nes_jy_typea_device::irq_disable()
{
	// acknowledges a pending IRQ, inhibits counting and clears the prescaler
	m_irq_enable = 0;
	m_irq_prescaler = 0;
	set_irq_line(CLEAR_LINE);
}

void nes_jy_typea_device::irq_tick()
{
	if (!m_irq_enable || (m_irq_dir != 1 && m_irq_dir != 2))
		return;

	const uint8_t mask = m_irq_small ? 0x07 : 0xff;
	uint8_t prescaler = m_irq_prescaler & mask;
	bool wrap;

	if (m_irq_dir == 1)
	{
		prescaler = (prescaler + 1) & mask;
		wrap = (prescaler == 0);
	}
	else
	{
		prescaler = (prescaler - 1) & mask;
		wrap = (prescaler == mask);
	}
	m_irq_prescaler = (m_irq_prescaler & ~mask) | prescaler;

	if (wrap)
	{
		if (m_irq_dir == 1)
		{
			m_irq_counter++;
			if (m_irq_counter == 0)
				set_irq_line(ASSERT_LINE);
		}
		else
		{
			m_irq_counter--;
			if (m_irq_counter == 0xff)
				set_irq_line(ASSERT_LINE);
		}
	}
}

void nes_jy_typea_device::m2_w(int state, offs_t address, bool write)
{
	if (!state)
		return;

	if (m_irq_source == 0 || (m_irq_source == 3 && write))
		irq_tick();
}

void nes_jy_typea_device::ppu_a12_level(int state)
{
	const int rise = state && !m_a12;
	m_a12 = state ? 1 : 0;

	if (rise && m_irq_source == 1)
		irq_tick();
}


//-------------------------------------------------
//  PPU accessors
//-------------------------------------------------

// IRQ source 2: the prescaler counts every PPU read, i.e. every /RD strobe on the PPU bus (rendering fetches, the
// garbage and dummy fetches included, and PPUDATA reads); counted from the PPU's strobe, not from calls to chr_r/nt_r,
// which debugger and other side-effect-free reads also make
void nes_jy_typea_device::ppu_rd(offs_t addr)
{
	if (m_irq_source == 2)
		irq_tick();
}

uint8_t nes_jy_typea_device::chr_r(offs_t offset)
{
	return nes_nrom_device::chr_r(offset);
}

uint8_t nes_jy_typea_device::nt_r(offs_t offset)
{
	return nes_nrom_device::nt_r(offset);
}

void nes_jy_typea_device::chr_w(offs_t offset, uint8_t data)
{
	// $D002 bit 6: CHR write enable
	if (BIT(m_ppucfg, 6))
		nes_nrom_device::chr_w(offset, data);
}

// MMC4-like mode ($D003 bit 7): the latch of each 4K half is set by PPU reads of $xFD8-$xFDF (0) and $xFE8-$xFEF (1)
// and takes effect for the following reads
void nes_jy_typea_device::ppu_read_latch(offs_t offset)
{
	if (!BIT(m_outer, 7))
		return;

	const int half = BIT(offset, 12);
	int latch;

	switch (offset & 0x0ff8)
	{
	case 0x0fd8: latch = 0; break;
	case 0x0fe8: latch = 1; break;
	default: return;
	}

	if (m_chr_latch[half] != latch)
	{
		m_chr_latch[half] = latch;
		if (BIT(m_mode, 3, 2) == 1)
			update_chr();
	}
}


//-------------------------------------------------
//  CPU accessors
//-------------------------------------------------

uint8_t nes_jy_typea_device::read_l(offs_t offset)
{
	const offs_t addr = offset + 0x4100;

	LOG("JY Company read_l, addr: %04x\n", addr);

	// solder pads, fully decoded
	if (addr == 0x5000 || addr == 0x5400 || addr == 0x5c00)
		return (get_open_bus() & 0x3f) | (m_jumper->read() & 0xc0);

	if ((addr & 0xf800) == 0x5800)
	{
		switch (addr & 3)
		{
		case 0: return (m_mul[0] * m_mul[1]) & 0xff;
		case 1: return ((m_mul[0] * m_mul[1]) >> 8) & 0xff;
		case 2: return m_acc;
		case 3: return m_test;
		}
	}

	return get_open_bus();
}

void nes_jy_typea_device::write_l(offs_t offset, uint8_t data)
{
	const offs_t addr = offset + 0x4100;

	LOG("JY Company write_l, addr: %04x, data: %02x\n", addr, data);

	if ((addr & 0xf800) == 0x5800)
	{
		switch (addr & 3)
		{
		case 0: m_mul[0] = data; break;
		case 1: m_mul[1] = data; break;
		case 2: m_acc += data; break;
		case 3: m_acc = 0; m_test = data; break;
		}
	}
}

uint8_t nes_jy_typea_device::read_m(offs_t offset)
{
	LOG("JY Company read_m, offset: %04x\n", offset);

	if (BIT(m_mode, 7))
		return m_prg[(m_bank_6000 & m_prg_mask) * 0x2000 + (offset & 0x1fff)];

	return nes_nrom_device::read_m(offset);
}

void nes_jy_typea_device::write_m(offs_t offset, uint8_t data)
{
	if (!BIT(m_mode, 7))
		nes_nrom_device::write_m(offset, data);
}

void nes_jy_typea_device::write_h(offs_t offset, uint8_t data)
{
	const offs_t addr = offset | 0x8000;

	LOG("JY Company write_h, addr: %04x, data: %02x\n", addr, data);

	switch (addr & 0xf000)
	{
	case 0x8000:
		if (!BIT(addr, 11))
		{
			m_prg_reg[addr & 3] = data & 0x7f;
			update_prg();
		}
		break;

	case 0x9000:
		if (!BIT(addr, 11))
		{
			m_chr_lo[addr & 7] = data;
			update_chr();
		}
		break;

	case 0xa000:
		if (!BIT(addr, 11))
		{
			m_chr_hi[addr & 7] = data;
			update_chr();
		}
		break;

	case 0xb000:
		if (!BIT(addr, 11))
		{
			if (BIT(addr, 2))
				m_nt_hi[addr & 3] = data;
			else
				m_nt_lo[addr & 3] = data;
			update_nt();
		}
		break;

	case 0xc000:
		switch (addr & 7)
		{
		case 0:
			if (BIT(data, 0))
				m_irq_enable = 1;
			else
				irq_disable();
			break;
		case 1:
			m_irq_source = data & 3;
			m_irq_small = BIT(data, 2);
			m_irq_funky = BIT(data, 3);
			m_irq_dir = BIT(data, 6, 2);
			break;
		case 2:
			irq_disable();
			break;
		case 3:
			m_irq_enable = 1;
			break;
		case 4:
			m_irq_prescaler = data ^ m_irq_xor;
			break;
		case 5:
			m_irq_counter = data ^ m_irq_xor;
			break;
		case 6:
			m_irq_xor = data;
			break;
		case 7:
			m_irq_c007 = data;      // "funky" mode configuration, not emulated
			break;
		}
		break;

	case 0xd000:
		if (!BIT(addr, 11))
		{
			switch (addr & 3)
			{
			case 0:
				m_mode = data;
				update_prg();
				update_chr();
				update_nt();
				break;
			case 1:
				m_mirror = data;
				update_nt();
				break;
			case 2:
				m_ppucfg = data;
				update_nt();
				break;
			case 3:
				m_outer = data;
				update_prg();
				update_chr();
				break;
			}
		}
		break;
	}
}


//-------------------------------------------------
//  Realtec HSK007 (NES 2.0 mapper 394)
//
//  NESdev: the mode register at $5009 and the outer bank register at $500B (mask unknown, but A3 must be set; A1-A0
//  tell them apart, so the ASIC's multiplier at $5800-$5803, where A3 is clear, stays reachable)
//
//  $5009 mode, power-on $0F:  ...M MCBA
//     A: PRG A18 and, in submapper 0, CHR A18          B: PRG A19 and CHR A19         C: CHR A18 in submapper 1
//     M: 0 BNROM, 1 MMC3, 2 and 3 J.Y. Company ASIC
//  $500B outer bank, power-on $90:  cC.p PBB.
//     BB: PRG A16-A15 in BNROM mode        P: PRG A17 when p = 0       p: PRG A17 from the MMC3 (256K) or from P (128K)
//     C: CHR A17 when c = 0                c: CHR A17 from the MMC3 (256K) or from C (128K)
//  In ASIC mode PRG and CHR A17 always come from the ASIC. Both registers go back to their power-on values on reset.
//
//  FCEUmm (jyasic.c) also decodes the registers on A1-A0 only and resets the ASIC when it is selected and the MMC3 when
//  it is deselected, which is what is done here. It differs from NESdev in letting P and C override A17 in ASIC mode;
//  NESdev is followed. FCEUmm uses the CHR A18 of submapper 1 in ASIC mode for both submappers; here the submapper decides.
//
//  Not verified against a real board or a game that uses the MMC3 mode's IRQ: the games were only run to their menus
//  and into the first game of each mode that they reach.
//-------------------------------------------------

nes_jy_hsk007_device::nes_jy_hsk007_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_jy_typeb_device(mconfig, NES_JY_HSK007, tag, owner, clock)
	, m_submapper(0)
	, m_mmc3_cmd(0)
	, m_mmc3_irq_latch(0), m_mmc3_irq_count(0), m_mmc3_irq_reload(false), m_mmc3_irq_enable(false)
{
	m_reg[0] = 0x00; m_reg[1] = 0x0f; m_reg[2] = 0x00; m_reg[3] = 0x90;
	std::fill(std::begin(m_mmc3_bank), std::end(m_mmc3_bank), 0);
}

void nes_jy_hsk007_device::device_start()
{
	nes_jy_typeb_device::device_start();

	save_item(NAME(m_reg));
	save_item(NAME(m_mmc3_cmd));
	save_item(NAME(m_mmc3_bank));
	save_item(NAME(m_mmc3_irq_latch));
	save_item(NAME(m_mmc3_irq_count));
	save_item(NAME(m_mmc3_irq_reload));
	save_item(NAME(m_mmc3_irq_enable));
}

uint32_t nes_jy_hsk007_device::chr_outer_ab() const
{
	// CHR A18 is the A bit in submapper 0 and the C bit in submapper 1, CHR A19 is the B bit in both
	if (m_submapper == 1)
		return ((m_reg[1] << 8) & 0x200) | ((m_reg[1] << 6) & 0x100);
	return (m_reg[1] << 8) & 0x300;
}

// the ASIC's bank numbers in ASIC mode: 256K windows (A17 from the ASIC), the outer A18-A19 from the mode register
uint32_t nes_jy_hsk007_device::prg_bank_filter(uint32_t bank) const
{
	return (bank & 0x1f) | prg_outer_ab();
}

uint32_t nes_jy_hsk007_device::chr_bank_filter(uint32_t bank) const
{
	return (bank & 0xff) | chr_outer_ab();
}

// the MMC3's bank numbers: the inner block is 128K or 256K and the outer bits fill in what the inner block leaves out
uint32_t nes_jy_hsk007_device::mmc3_prg(uint32_t bank) const
{
	const uint32_t mask = BIT(m_reg[3], 4) ? 0x1f : 0x0f;
	return (bank & mask) | (prg_outer() & ~mask);
}

uint32_t nes_jy_hsk007_device::mmc3_chr(uint32_t bank) const
{
	const uint32_t mask = BIT(m_reg[3], 7) ? 0xff : 0x7f;
	return (bank & mask) | (chr_outer() & ~mask);
}

void nes_jy_hsk007_device::mmc3_power()
{
	m_mmc3_cmd = 0;
	static const uint8_t init[8] = { 0, 2, 4, 5, 6, 7, 0, 1 };
	std::copy(std::begin(init), std::end(init), std::begin(m_mmc3_bank));
	m_mmc3_irq_latch = m_mmc3_irq_count = 0;
	m_mmc3_irq_reload = m_mmc3_irq_enable = false;
	set_irq_line(CLEAR_LINE);
	set_nt_mirroring(PPU_MIRROR_VERT);
}

void nes_jy_hsk007_device::update_mmc3()
{
	if (BIT(m_reg[1], 3, 2) == 0)
	{
		// BNROM: the outer register picks a 32K bank
		const uint32_t block = (prg_outer() | ((m_reg[3] << 1) & 0x0f)) >> 2;
		for (int i = 0; i < 4; i++)
			prg8_x(i, (block << 2) + i);
	}
	else
	{
		const bool swap = BIT(m_mmc3_cmd, 6);
		prg8_x(0, mmc3_prg(swap ? 0xfe : m_mmc3_bank[6]));
		prg8_x(1, mmc3_prg(m_mmc3_bank[7]));
		prg8_x(2, mmc3_prg(swap ? m_mmc3_bank[6] : 0xfe));
		prg8_x(3, mmc3_prg(0xff));
	}

	// the CHR banks are the MMC3's in both modes
	const int a = BIT(m_mmc3_cmd, 7) ? 4 : 0;
	const int b = a ^ 4;
	chr1_x(a + 0, mmc3_chr(m_mmc3_bank[0] & 0xfe), m_chr_source);
	chr1_x(a + 1, mmc3_chr(m_mmc3_bank[0] | 0x01), m_chr_source);
	chr1_x(a + 2, mmc3_chr(m_mmc3_bank[1] & 0xfe), m_chr_source);
	chr1_x(a + 3, mmc3_chr(m_mmc3_bank[1] | 0x01), m_chr_source);
	chr1_x(b + 0, mmc3_chr(m_mmc3_bank[2]), m_chr_source);
	chr1_x(b + 1, mmc3_chr(m_mmc3_bank[3]), m_chr_source);
	chr1_x(b + 2, mmc3_chr(m_mmc3_bank[4]), m_chr_source);
	chr1_x(b + 3, mmc3_chr(m_mmc3_bank[5]), m_chr_source);
}

void nes_jy_hsk007_device::update_banks()
{
	if (jy_mode())
		refresh_banks();
	else
		update_mmc3();
}

void nes_jy_hsk007_device::pcb_reset()
{
	m_reg[0] = 0x00; m_reg[1] = 0x0f; m_reg[2] = 0x00; m_reg[3] = 0x90;

	nes_jy_typeb_device::pcb_reset();
	mmc3_power();
	update_mmc3();
}


//-------------------------------------------------
//  CPU accessors
//-------------------------------------------------

uint8_t nes_jy_hsk007_device::read_l(offs_t offset)
{
	return jy_mode() ? nes_jy_typeb_device::read_l(offset) : get_open_bus();
}

void nes_jy_hsk007_device::write_l(offs_t offset, uint8_t data)
{
	const offs_t addr = 0x4100 + offset;

	if ((addr >= 0x5000) && BIT(addr, 3))
	{
		const int r = addr & 3;
		const uint8_t old = m_reg[r];
		m_reg[r] = data;

		if (r == 1)
		{
			if (!BIT(old, 4) && BIT(data, 4))
			{
				// the ASIC powers up when it is selected
				nes_jy_typeb_device::pcb_reset();
				set_irq_line(CLEAR_LINE);
			}
			else if (BIT(old, 4) && !BIT(data, 4))
			{
				// and the MMC3 when the ASIC is deselected
				mmc3_power();
			}
			update_banks();
		}
		else if (r == 3)
		{
			update_banks();
		}
	}
	else if (jy_mode())
	{
		nes_jy_typeb_device::write_l(offset, data);
	}
}

uint8_t nes_jy_hsk007_device::read_m(offs_t offset)
{
	return jy_mode() ? nes_jy_typeb_device::read_m(offset) : nes_nrom_device::read_m(offset);
}

void nes_jy_hsk007_device::write_m(offs_t offset, uint8_t data)
{
	if (jy_mode())
		nes_jy_typeb_device::write_m(offset, data);
	else
		nes_nrom_device::write_m(offset, data);
}

void nes_jy_hsk007_device::write_h(offs_t offset, uint8_t data)
{
	if (jy_mode())
	{
		nes_jy_typeb_device::write_h(offset, data);
		return;
	}

	// BNROM and MMC3 modes: the MMC3's registers
	switch (offset & 0x6001)
	{
	case 0x0000:    // bank select
		m_mmc3_cmd = data;
		update_mmc3();
		break;

	case 0x0001:    // bank data
		m_mmc3_bank[m_mmc3_cmd & 7] = data;
		update_mmc3();
		break;

	case 0x2000:    // mirroring
		set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
		break;

	case 0x2001:    // PRG-RAM protect: there is no PRG-RAM
		break;

	case 0x4000:    // IRQ latch
		m_mmc3_irq_latch = data;
		break;

	case 0x4001:    // IRQ reload
		m_mmc3_irq_count = 0;
		m_mmc3_irq_reload = true;
		break;

	case 0x6000:    // IRQ disable and acknowledge
		m_mmc3_irq_enable = false;
		set_irq_line(CLEAR_LINE);
		break;

	case 0x6001:    // IRQ enable
		m_mmc3_irq_enable = true;
		break;
	}
}


//-------------------------------------------------
//  PPU accessors and IRQs
//-------------------------------------------------

uint8_t nes_jy_hsk007_device::chr_r(offs_t offset)
{
	return jy_mode() ? nes_jy_typeb_device::chr_r(offset) : nes_nrom_device::chr_r(offset);
}

void nes_jy_hsk007_device::chr_w(offs_t offset, uint8_t data)
{
	if (jy_mode())
		nes_jy_typeb_device::chr_w(offset, data);
	else
		nes_nrom_device::chr_w(offset, data);
}

uint8_t nes_jy_hsk007_device::nt_r(offs_t offset)
{
	return jy_mode() ? nes_jy_typeb_device::nt_r(offset) : nes_nrom_device::nt_r(offset);
}

void nes_jy_hsk007_device::m2_w(int state, offs_t address, bool write)
{
	if (jy_mode())
		nes_jy_typeb_device::m2_w(state, address, write);
}

void nes_jy_hsk007_device::ppu_a12_level(int state)
{
	if (jy_mode())
		nes_jy_typeb_device::ppu_a12_level(state);
}

void nes_jy_hsk007_device::ppu_read_latch(offs_t offset)
{
	if (jy_mode())
		nes_jy_typeb_device::ppu_read_latch(offset);
}

// the MMC3's scanline counter, as in nes_txrom_device: on each accepted A12 rise the counter is reloaded when it is 0 or a
// reload was requested, otherwise decremented, and the IRQ is asserted when it is 0 afterwards and IRQs are enabled
void nes_jy_hsk007_device::ppu_a12_rise()
{
	if (jy_mode())
		return;

	if ((m_mmc3_irq_count == 0) || m_mmc3_irq_reload)
		m_mmc3_irq_count = m_mmc3_irq_latch;
	else
		m_mmc3_irq_count--;

	if (m_mmc3_irq_enable && (m_mmc3_irq_count == 0))
		set_irq_line(ASSERT_LINE);
	m_mmc3_irq_reload = false;
}

// NESdev MMC3: a rising edge of A12 only clocks the counter if A12 was low for three falling edges of M2
bool nes_jy_hsk007_device::a12_rise_accepted(const attotime &low_since, const attotime &when)
{
	return (when > low_since) && ((cpu_cycles_at(when) - cpu_cycles_at(low_since)) >= 3);
}
