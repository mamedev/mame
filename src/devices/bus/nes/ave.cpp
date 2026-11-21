// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
/***********************************************************************************************************


 NES/Famicom cartridge emulation for AVE PCBs

 iNES mappers: 34, 79, 146, 234, 487, 493


 Here we emulate the following PCBs

   * AVE Nina-001 [mapper 34]
   * AVE Nina-006/Nina-003/MB-91 [mapper 79 & 146]
   * AVE Maxi 15 [mapper 234]
   * AVE NINA-08 [mapper 487]
   * Maxivision 30 Super Games prototype board [mapper 493]


 ***********************************************************************************************************/


#include "emu.h"
#include "ave.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE (LOG_GENERAL)
#else
#define VERBOSE (0)
#endif
#include "logmacro.h"


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_NINA001, nes_nina001_device, "nes_nina001", "NES Cart AVE Nina-001 PCB")
DEFINE_DEVICE_TYPE(NES_NINA006, nes_nina006_device, "nes_nina006", "NES Cart AVE Nina-006 PCB")
DEFINE_DEVICE_TYPE(NES_MAXI15,  nes_maxi15_device,  "nes_maxi15",  "NES Cart AVE Maxi 15 PCB")
DEFINE_DEVICE_TYPE(NES_NINA08,  nes_nina08_device,  "nes_nina08",  "NES Cart AVE NINA-08 PCB")
DEFINE_DEVICE_TYPE(NES_MAXI30P, nes_maxi30p_device, "nes_maxi30p", "NES Cart Maxivision 30 Super Games prototype PCB")


nes_nina001_device::nes_nina001_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_NINA001, tag, owner, clock)
{
}

nes_nina006_device::nes_nina006_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_NINA006, tag, owner, clock)
{
}

nes_maxi15_device::nes_maxi15_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_MAXI15, tag, owner, clock)
{
}




void nes_maxi15_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_maxi15_device::pcb_reset()
{
	prg32(0);
	chr8(0, CHRROM);

	m_reg[0] = m_reg[1] = 0;
}




/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 AVE NINA-001 board emulation

 The registers are decoded at exactly 0x7ffd (1 bit of 32K PRG bank) and at 0x7ffe
 and 0x7fff (4K CHR banks at PPU 0x0000 and 0x1000, 4 bits each). There are no
 bus conflicts. NES 2.0 submapper 1 of mapper 34 selects this board.

 iNES: mapper 34

 Notice that in this board the bankswitch regs
 overlaps WRAM, so that writes to the regs are
 then readable back in WRAM (WRAM is tested by
 Impossible Mission II at start)

 In MAME: Supported.

 -------------------------------------------------*/

void nes_nina001_device::write_m(offs_t offset, u8 data)
{
	LOG("nina-001 write_m, offset: %04x, data: %02x\n", offset, data);

	device_nes_cart_interface::write_m(offset, data); // write WRAM

	switch (offset)
	{
		case 0x1ffd:
			prg32(data);
			break;
		case 0x1ffe:
			chr4_0(data, CHRROM);
			break;
		case 0x1fff:
			chr4_4(data, CHRROM);
			break;
	}
}

/*-------------------------------------------------

 AVE NINA-003, NINA-006 and MB-91 boards emulation

 Games: Krazy Kreatures, Poke Block, Puzzle, Pyramid,
 Solitaire, Ultimate League Soccer

 The register is decoded at 0x4100-0x5fff wherever address bit 8 is set. Bit 3
 selects the 32K PRG bank and bits 2-0 the 8K CHR bank. There are no bus conflicts.
 Mapper 146 (Sachen 3015) is functionally identical.

 iNES: mapper 79

 In MAME: Supported.

 -------------------------------------------------*/

void nes_nina006_device::write_l(offs_t offset, u8 data)
{
	LOG("nina-006 write_l, offset: %04x, data: %02x\n", offset, data);

	offset += 0x100;
	if (BIT(offset, 8)) // $41xx, $43xx, ... $5fxx
	{
		prg32(BIT(data, 3));
		chr8(data & 7, CHRROM);
	}
}

/*-------------------------------------------------

 AVE Maxi 15 boards emulation

 Games: Maxi 15

 Banking happens on the value on the bus when the CPU accesses 0xff80-0xff9f (outer
 register, MOQq BBBb) or 0xffe8-0xfff7 (inner register, .cCC ...P). A read gives the
 ROM byte; a write has an AND-type bus conflict with the ROM, so the register takes
 the written value ANDed with the ROM byte. The Q and q bits, which select and disable
 ROMs 3 and 4, are not emulated: the usual cart has only ROMs 1 and 2 fitted. The
 lockout defeat register at 0xffc0-0xffdf is not emulated either (NESdev: not needed).

 iNES: mapper 234

 In MAME: Supported.

 -------------------------------------------------*/

void nes_maxi15_device::latch(offs_t offset, u8 value)
{
	if ((offset >= 0x7f80 && offset < 0x7fa0) || (offset >= 0x7fe8 && offset < 0x7ff8))
	{
		int reg = BIT(offset, 6);
		if (reg || !(m_reg[0] & 0x3f))    // inner banks always modifiable, outer banks locked once set
		{
			m_reg[reg] = value;

			u8 mode = !BIT(m_reg[0], 6);
			u8 outer = m_reg[0] & (0x0e | mode);
			prg32(outer | (m_reg[1] & !mode));
			chr8(outer << 2 | ((m_reg[1] >> 4) & (7 >> mode)), CHRROM);
			set_nt_mirroring(BIT(m_reg[0], 7) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
		}
	}
}

u8 nes_maxi15_device::read_h(offs_t offset)
{
	LOG("Maxi 15 read_h, offset: %04x\n", offset);

	u8 temp = hi_access_rom(offset);
	latch(offset, temp);

	return temp;
}

void nes_maxi15_device::write_h(offs_t offset, u8 data)
{
	LOG("Maxi 15 write_h, offset: %04x, data: %02x\n", offset, data);

	latch(offset, data & hi_access_rom(offset));
}


/*-------------------------------------------------

 AVE NINA-08 board

 Games: Maxivision 30-in-1 (the unreleased original version)

 NESdev NES 2.0 Mapper 487. The board carries the AVE games
 (NINA-03 banking) in its first 512 KiB of PRG and CHR and
 the Color Dreams games (Color Dreams banking) in the next
 1024 KiB, both unmodified:
 - $4180 (A~01.. ...1 1... ....): NMCB BBBb
   N: 0 horizontal arrangement (vertical mirroring),
      1 vertical arrangement (horizontal mirroring)
   M: 0 32 KiB inner banks, PRG/CHR A15 from b
      1 64 KiB inner banks, PRG/CHR A15 from the inner bank
   C: 0 first chip, NINA-03 banking at $4100
      1 second and third chips, Color Dreams banking at $8000
   BBBB: PRG/CHR A19..A16 (A19 only matters when C=1)
 - $4100 (A~01.. ...1 0... ....), only while C=0: .... PcCC
   P: PRG A15 if M=1, c: CHR A15 if M=1, CC: CHR A14..A13
 - $8000-$FFFF, only while C=1: .cCC ...P (same meanings)
 Only the inner register selected by C answers: NESdev notes
 that Mermaids of Atlantis keeps writing GNROM values to
 $8000-$FFFF, which must be ignored.

 Not documented, so not emulated: power-on and reset values
 (all registers start at 0 here) and bus conflicts on the
 $8000 register.

 NES 2.0: mapper 487

 In MAME: Supported.

 -------------------------------------------------*/

nes_nina08_device::nes_nina08_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_NINA08, tag, owner, clock), m_outer(0), m_nina(0), m_cd(0)
{
}

void nes_nina08_device::device_start()
{
	common_start();
	save_item(NAME(m_outer));
	save_item(NAME(m_nina));
	save_item(NAME(m_cd));
}

void nes_nina08_device::pcb_reset()
{
	m_outer = m_nina = m_cd = 0;
	update_banks();
}

void nes_nina08_device::update_banks()
{
	const bool cd = BIT(m_outer, 5);
	const bool wide = BIT(m_outer, 6);
	// A19..A16: the first chip is 512 KiB, so A19 only reaches the other two
	const u32 a16 = (m_outer >> 1) & (cd ? 0x0f : 0x07);
	u8 prg_a15, chr_a15, chr_a13;
	if (cd)
	{
		prg_a15 = wide ? BIT(m_cd, 0) : BIT(m_outer, 0);
		chr_a15 = wide ? BIT(m_cd, 6) : BIT(m_outer, 0);
		chr_a13 = (m_cd >> 4) & 3;
	}
	else
	{
		prg_a15 = wide ? BIT(m_nina, 3) : BIT(m_outer, 0);
		chr_a15 = wide ? BIT(m_nina, 2) : BIT(m_outer, 0);
		chr_a13 = m_nina & 3;
	}
	// the second and third chips start 512 KiB into the PRG and CHR images
	prg32((cd ? 0x10 : 0x00) + ((a16 << 1) | prg_a15));
	chr8((cd ? 0x40 : 0x00) + ((a16 << 3) | (chr_a15 << 2) | chr_a13), CHRROM);
	set_nt_mirroring(BIT(m_outer, 7) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

void nes_nina08_device::write_l(offs_t offset, u8 data)
{
	LOG("nina-08 write_l, offset: %04x, data: %02x\n", offset, data);

	offset += 0x100;   // offset from $4000
	switch (offset & 0x0180)
	{
		case 0x0180:   // $4180
			m_outer = data;
			update_banks();
			break;
		case 0x0100:   // $4100
			if (!BIT(m_outer, 5))
			{
				m_nina = data;
				update_banks();
			}
			break;
	}
}

void nes_nina08_device::write_h(offs_t offset, u8 data)
{
	LOG("nina-08 write_h, offset: %04x, data: %02x\n", offset, data);

	if (BIT(m_outer, 5))
	{
		m_cd = data;
		update_banks();
	}
}


/*-------------------------------------------------

 Maxivision 30 Super Games prototype board

 Games: Maxivision 30-in-1 (prototype of the released
 30 Super Games)

 NESdev NES 2.0 Mapper 493: INES Mapper 011 limited to 64 KiB
 of PRG and CHR per game, with an outer bank register.
 - $6000-$7FFF: .... BBBB, PRG/CHR A19..A16
 - $8000-$FFFF, with bus conflicts: .CCC ...P,
   P: PRG A15, CCC: CHR A15..A13
 Nametables are hard-wired to the horizontal arrangement
 (vertical mirroring).

 Not documented, so not emulated: power-on and reset values
 (both registers start at 0 here).

 NES 2.0: mapper 493

 In MAME: Supported.

 -------------------------------------------------*/

nes_maxi30p_device::nes_maxi30p_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_MAXI30P, tag, owner, clock), m_outer(0), m_inner(0)
{
}

void nes_maxi30p_device::device_start()
{
	common_start();
	save_item(NAME(m_outer));
	save_item(NAME(m_inner));
}

void nes_maxi30p_device::pcb_reset()
{
	m_outer = m_inner = 0;
	set_nt_mirroring(PPU_MIRROR_VERT);
	update_banks();
}

void nes_maxi30p_device::update_banks()
{
	prg32((m_outer << 1) | BIT(m_inner, 0));
	chr8((m_outer << 3) | ((m_inner >> 4) & 7), CHRROM);
}

void nes_maxi30p_device::write_m(offs_t offset, u8 data)
{
	LOG("maxi30p write_m, offset: %04x, data: %02x\n", offset, data);

	m_outer = data & 0x0f;
	update_banks();
}

void nes_maxi30p_device::write_h(offs_t offset, u8 data)
{
	LOG("maxi30p write_h, offset: %04x, data: %02x\n", offset, data);

	m_inner = data & hi_access_rom(offset);   // bus conflicts
	update_banks();
}
