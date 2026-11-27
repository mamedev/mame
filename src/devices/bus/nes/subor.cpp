// license:BSD-3-Clause
// copyright-holders:Kaz, Fabio Priuli,Brian H. Gilbert
/***********************************************************************************************************

 NES/Famicom cartridge emulation for Subor PCBs

 * Subor Type 0 [mapper 167]
 * Subor Type 1 [mapper 166, an old designation of 167 dumps with a different bank order]
 * Subor Type 2 / UNL-DANCE2000 [NES 2.0 mapper 518] (Subor 999, Subor V, Dance 2000 12-in-1)
 * Subor Karaoke [NES 2.0 mapper 514]

 NOT EMULATED: the speech chip that the Subor SB-97 computer (whose built-in BIOS is Subor V, mapper 518) has on
 its PRG-ROM add-on board, written and read at $5300. krzysiobal's reverse engineering of the SB-97 (NESdev forum
 "SUBOR SB97 - Famiclone with floppy+LPT+Keyboard", 2019) shows a 16-pin blob chip on D0-D7 with D7 as busy/ready,
 fed packets once per frame, whose pinout NewRisingSun found to resemble the TI TSP50C10/11; the chip has not been
 identified for certain, and neither its internal program nor its protocol has been documented, so there is nothing
 to emulate it from. Reads of $5300 report "ready" and end of message.

 ***********************************************************************************************************/

#include "emu.h"
#include "subor.h"

#ifdef NES_PCB_DEBUG
#define VERBOSE (LOG_GENERAL)
#else
#define VERBOSE (0)
#endif
#include "logmacro.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(NES_SUBOR0, nes_subor0_device, "nes_subor0", "NES Cart Subor Type 0 PCB")
DEFINE_DEVICE_TYPE(NES_SUBOR1, nes_subor1_device, "nes_subor1", "NES Cart Subor Type 1 PCB")
DEFINE_DEVICE_TYPE(NES_SUBOR2, nes_subor2_device, "nes_subor2", "NES Cart Subor Type 2 PCB")
DEFINE_DEVICE_TYPE(NES_SUBOR_KARAOKE, nes_subor_karaoke_device, "nes_subor_kara", "NES Cart Subor Karaoke PCB")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nes_subor0_device - constructor
//-------------------------------------------------

nes_subor0_device::nes_subor0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_SUBOR0, tag, owner, clock)
{
}

//-------------------------------------------------
//  nes_subor1_device - constructor
//-------------------------------------------------

nes_subor1_device::nes_subor1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_SUBOR1, tag, owner, clock)
{
}

//-------------------------------------------------
//  nes_subor2_device - constructor
//-------------------------------------------------

nes_subor2_device::nes_subor2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_subor2_device(mconfig, NES_SUBOR2, tag, owner, clock)
{
}

nes_subor2_device::nes_subor2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, type, tag, owner, clock)
	, m_bank_reg(0)
	, m_mode_reg(0)
	, m_ciram_a10(0)
{
}

/*-------------------------------------------------
    device_start
-------------------------------------------------*/

void nes_subor0_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_subor1_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_subor2_device::device_start()
{
	common_start();
	save_item(NAME(m_bank_reg));
	save_item(NAME(m_mode_reg));
	save_item(NAME(m_ciram_a10));
}

/*-------------------------------------------------
    pcb_reset
-------------------------------------------------*/

void nes_subor0_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(0x20);
	chr8(0, m_chr_source);

	memset(m_reg, 0, sizeof(m_reg));
}

void nes_subor1_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(0x07);
	chr8(0, m_chr_source);

	memset(m_reg, 0, sizeof(m_reg));
}

void nes_subor2_device::pcb_reset()
{
	m_bank_reg = 0;
	m_mode_reg = 0;
	m_ciram_a10 = 0;
	chr8(0, CHRRAM);   // $1000-$1FFF stays on the second 4 KiB of CHR-RAM
	update_banks();
}

/*-------------------------------------------------
    mapper specific handlers
-------------------------------------------------*/

/*-------------------------------------------------

 Subor educational cartridge board Type 0:
 iNES Mapper 167

 Subor educational cartridge board Type 1:
 iNES Mapper 166

 Subor educational cartridge board Type 2: NES 2.0 mapper 518
 (16/32 KiB PRG banking, registers at $5000/$5200, speech chip
 status at $5300), and Subor Karaoke, NES 2.0 mapper 514 (32 KiB
 PRG banking, register at $8000); see the board notes below.

-------------------------------------------------*/

/*-------------------------------------------------

 Subor Type 2 board (NES 2.0 mapper 518, UNL-DANCE2000), after the NESdev wiki:
   $5000 [REPP PPPP] PRG bank; E (expansion cartridge) is PRG A20 of a combined
         2 MiB image, R selects the SB-97's secondary PRG-RAM (not on cartridges)
   $5200 [.... .SBM] M: 0 = vertical mirroring, 1 = horizontal mirroring
                     B: CHR-RAM auto-bankswitch
                     S: 0 = 16 KiB at $8000 with $C000 fixed to bank 0, 1 = 32 KiB
   $5300 LPC speech chip (read: [R... EEEE] ready / frame energy)
 With auto-bankswitch on, CHR A12 is replaced by the CIRAM A10 latched at the last
 rising edge of PPU A13, for reads from PPU $0000-$0FFF only: the first/second
 nametable draws its tiles from the first/second 4 KiB of CHR-RAM. Writes are never
 bankswitched. These games need Dendy video timing.

 -------------------------------------------------*/

uint8_t nes_subor2_device::nt_r(offs_t offset)
{
	// every nametable or attribute fetch raises PPU A13; CIRAM A10 is PPU A10 with vertical
	// mirroring and PPU A11 with horizontal mirroring
	m_ciram_a10 = BIT(m_mode_reg, 0) ? BIT(offset, 11) : BIT(offset, 10);
	return device_nes_cart_interface::nt_r(offset);
}

uint8_t nes_subor2_device::chr_r(offs_t offset)
{
	if (BIT(m_mode_reg, 1) && offset < 0x1000 && m_vram.size() >= 0x2000)
		return m_vram[(m_ciram_a10 << 12) | (offset & 0x0fff)];
	return device_nes_cart_interface::chr_r(offset);
}

void nes_subor2_device::update_banks()
{
	set_nt_mirroring(BIT(m_mode_reg, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	if (BIT(m_mode_reg, 2))
	{
		// 32 KiB mode: P is the 32 KiB bank (Mesen2's Dance2000), and E is PRG A20 in both modes (NESdev: it "merely
		// maps to PRG A20 of the combined 2 MiB address space"), i.e. bit 5 of the 32 KiB bank number, not bit 6.
		// VERIFY: P bit 5 would also be PRG A20 here; it is ignored (a 1 MiB main ROM needs 5 bits of 32 KiB bank)
		prg32((m_bank_reg & 0x1f) | (BIT(m_bank_reg, 6) << 5));
	}
	else
	{
		prg16_89ab(m_bank_reg & 0x7f);
		prg16_cdef(0);
	}
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

uint8_t nes_subor2_device::read_l(offs_t offset)
{
	LOG("subor2 read_l, offset: %04x\n", offset);
	// $5300: the SB-97 speech chip, not emulated (see the file header): report "ready" with an end-of-message energy of 15
	if (offset == 0x1200)
		return 0x8f;
	return get_open_bus();
}

/*-------------------------------------------------
    write
-------------------------------------------------*/

void nes_subor0_device::write_h(offs_t offset, uint8_t data)
{
	uint8_t subor_helper1, subor_helper2;
	LOG("subor0 write_h, offset: %04x, data: %02x\n", offset, data);

	m_reg[BIT(offset, 13, 2)] = data;
	subor_helper1 = ((m_reg[0] ^ m_reg[1]) << 1) & 0x20;
	subor_helper2 = ((m_reg[2] ^ m_reg[3]) << 0) & 0x1f;

	if (m_reg[1] & 0x08)
	{
		subor_helper1 += subor_helper2 & 0xfe;
		subor_helper2 = subor_helper1;
		subor_helper1 += 1;
	}
	else if (m_reg[1] & 0x04)
	{
		subor_helper2 += subor_helper1;
		subor_helper1 = 0x1f;
	}
	else
	{
		subor_helper1 += subor_helper2;
		subor_helper2 = 0x20;
	}

	prg16_89ab(subor_helper1);
	prg16_cdef(subor_helper2);
}

void nes_subor1_device::write_h(offs_t offset, uint8_t data)
{
	uint8_t subor_helper1, subor_helper2;
	LOG("subor1 write_h, offset: %04x, data: %02x\n", offset, data);

	m_reg[BIT(offset, 13, 2)] = data;
	subor_helper1 = ((m_reg[0] ^ m_reg[1]) << 1) & 0x20;
	subor_helper2 = ((m_reg[2] ^ m_reg[3]) << 0) & 0x1f;

	if (m_reg[1] & 0x08)
	{
		subor_helper1 += subor_helper2 & 0xfe;
		subor_helper2 = subor_helper1;
		subor_helper2 += 1;
	}
	else if (m_reg[1] & 0x04)
	{
		subor_helper2 += subor_helper1;
		subor_helper1 = 0x1f;
	}
	else
	{
		subor_helper1 += subor_helper2;
		subor_helper2 = 0x07;
	}

	prg16_89ab(subor_helper1);
	prg16_cdef(subor_helper2);
}

void nes_subor2_device::write_l(offs_t offset, uint8_t data)
{
	LOG("subor2 write_l, offset: %04x, data: %02x\n", offset, data);
	switch (offset)
	{
		case 0x0f00:   // $5000
			m_bank_reg = data;
			update_banks();
			break;
		case 0x1100:   // $5200
			m_mode_reg = data & 0x07;
			update_banks();
			break;
	}
}

/*-------------------------------------------------

 Subor Karaoke (NES 2.0 mapper 514), after the NESdev wiki: the mapper 518 board's
 CHR-RAM auto-bankswitch with a single register at $8000:
   [BMPP PPPP] P = 32 KiB PRG bank, M = mirroring (0: vertical, 1: horizontal, the
   same polarity as mapper 518's M bit), B = CHR-RAM auto-bankswitch during PPU reads
 8 KiB WRAM. The microphones are mixed in analog on the cartridge. Needs Dendy timing.

 -------------------------------------------------*/

nes_subor_karaoke_device::nes_subor_karaoke_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_subor2_device(mconfig, NES_SUBOR_KARAOKE, tag, owner, clock)
{
}

void nes_subor_karaoke_device::pcb_reset()
{
	nes_subor2_device::pcb_reset();
	m_mode_reg = 0x04;   // this board only banks 32 KiB: bank 0 at $8000-$FFFF holds the reset vector
	update_banks();
}

void nes_subor_karaoke_device::write_h(offs_t offset, uint8_t data)
{
	LOG("subor karaoke write_h, offset: %04x, data: %02x\n", offset, data);
	m_bank_reg = data & 0x3f;
	// translate to the mapper 518 mode register [.... .SBM]: 32 KiB banking, B, and M unchanged: as on mapper 518,
	// M = 1 selects horizontal mirroring (CIRAM A10 = PPU A11), M = 0 vertical mirroring. The title screen's
	// CHR split only renders correctly this way (verified against screenshots of the real cartridge)
	m_mode_reg = 0x04 | (BIT(data, 7) << 1) | BIT(data, 6);
	update_banks();
}
