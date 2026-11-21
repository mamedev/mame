// license:BSD-3-Clause
// copyright-holders:kmg,Brian H. Gilbert
/***********************************************************************************************************


 NES/Famicom cartridge emulation for MMC-1 clone PCBs

 iNES mappers: 111, 297, 323, 374, 404, 543, 550
 (mapper 111 is listed for the images with CHR-ROM only, the Chinese Ninja Ryukenden (UNL-NINJARYU): the loader gives
 every other mapper 111 image the GTROM board, see discrete.cpp)


 Here we emulate several pirate PCBs based on MMC-1 boards

 ***********************************************************************************************************/


#include "emu.h"
#include "mmc1_clones.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE (LOG_GENERAL)
#else
#define VERBOSE (0)
#endif
#include "logmacro.h"


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_BMC_JY012005,  nes_bmc_jy012005_device,  "nes_bmc_jy012005",  "NES Cart BMC JY012005 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_621,       nes_bmc_621_device,       "nes_bmc_621",       "NES Cart Predator (NES 2.0 mapper 621) PCB")
DEFINE_DEVICE_TYPE(NES_BMC_JY820845C, nes_bmc_jy820845c_device, "nes_bmc_jy820845c", "NES Cart BMC JY820845C PCB")
DEFINE_DEVICE_TYPE(NES_FARID_SLROM,   nes_farid_slrom_device,   "nes_farid_slrom",   "NES Cart Farid SLROM 8 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_NINJARYU,      nes_ninjaryu_device,      "nes_ninjaryu",      "NES Cart Ninja Ryukenden Chinese PCB")
DEFINE_DEVICE_TYPE(NES_RESETSXROM,    nes_resetsxrom_device,    "nes_resetsxrom",    "NES Cart BMC RESET-SXROM PCB")
DEFINE_DEVICE_TYPE(NES_SRPG_5IN1,     nes_srpg5in1_device,      "nes_srpg5in1",      "NES Cart Super RPG 5 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_TXC_22110,     nes_txc_22110_device,     "nes_txc_22110",     "NES Cart TXC 01-22110-000 PCB")


nes_bmc_jy012005_device::nes_bmc_jy012005_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_sxrom_device(mconfig, NES_BMC_JY012005, tag, owner, clock), m_latch0(0)
{
}

nes_bmc_jy820845c_device::nes_bmc_jy820845c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_sxrom_device(mconfig, NES_BMC_JY820845C, tag, owner, clock), m_latch0(0), m_mode(0)
{
}

nes_farid_slrom_device::nes_farid_slrom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_sxrom_device(mconfig, NES_FARID_SLROM, tag, owner, clock), m_outer(0)
{
}

nes_ninjaryu_device::nes_ninjaryu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_sxrom_device(mconfig, NES_NINJARYU, tag, owner, clock)
{
}

nes_resetsxrom_device::nes_resetsxrom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_sxrom_device(mconfig, NES_RESETSXROM, tag, owner, clock), m_reset_count(-1)
{
}

nes_srpg5in1_device::nes_srpg5in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_sxrom_device(mconfig, NES_SRPG_5IN1, tag, owner, clock), m_outer(0), m_outer_count(0), m_outer_latch(0)
{
}

nes_txc_22110_device::nes_txc_22110_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_sxrom_device(mconfig, NES_TXC_22110, tag, owner, clock), m_latch0(0), m_mode(0)
{
}



void nes_bmc_jy012005_device::device_start()
{
	nes_sxrom_device::device_start();
	save_item(NAME(m_latch0));
}

void nes_bmc_jy012005_device::pcb_reset()
{
	m_latch0 = 0;
	nes_sxrom_device::pcb_reset();
}

void nes_bmc_jy820845c_device::device_start()
{
	nes_sxrom_device::device_start();
	save_item(NAME(m_latch0));
	save_item(NAME(m_mode));
}

void nes_bmc_jy820845c_device::pcb_reset()
{
	m_latch0 = 0;
	m_mode = 0;
	nes_sxrom_device::pcb_reset();
	update_banks();
}

void nes_farid_slrom_device::device_start()
{
	nes_sxrom_device::device_start();
	save_item(NAME(m_outer));
}

void nes_farid_slrom_device::pcb_reset()
{
	m_outer = 0;
	nes_sxrom_device::pcb_reset();
}

void nes_resetsxrom_device::device_start()
{
	nes_sxrom_device::device_start();
	save_item(NAME(m_reset_count));
}

void nes_resetsxrom_device::pcb_reset()
{
	m_reset_count = (m_reset_count + 1) & 3;
	nes_sxrom_device::pcb_reset();
}

void nes_srpg5in1_device::device_start()
{
	nes_sxrom_device::device_start();
	save_item(NAME(m_outer));
	save_item(NAME(m_outer_count));
	save_item(NAME(m_outer_latch));
}

void nes_srpg5in1_device::pcb_reset()
{
	m_outer = 0;
	m_outer_count = 0;
	m_outer_latch = 0;
	nes_sxrom_device::pcb_reset();
}

void nes_txc_22110_device::device_start()
{
	nes_sxrom_device::device_start();
	save_item(NAME(m_latch0));
	save_item(NAME(m_mode));
}

void nes_txc_22110_device::pcb_reset()
{
	nes_sxrom_device::pcb_reset();

	m_latch0 = 0;
	m_mode = 0;
	update_banks();
	set_nt_mirroring(PPU_MIRROR_VERT);
}



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 UNL-NINJARYU

 Games: Ninja Ryukenden Chinese

 This board was previously assigned to mapper 111. It has
 registers akin to MMC1 but without the need to write to
 them serially. The one existing game has 256K CHR, so this
 must have at least 1 more bit for CHR banking. Other differences?

 In MAME: Preliminary supported.

 -------------------------------------------------*/

void nes_ninjaryu_device::write_h(offs_t offset, u8 data)
{
	LOG("unl_ninjaryu write_h, offset: %04x, data: %02x\n", offset, data);
	u8 reg = BIT(offset, 13, 2);
	m_reg[reg] = data;
	update_regs(reg);
}


/*-------------------------------------------------

 MULTIGAME CARTS BASED ON MMC1

 -------------------------------------------------*/

/*-------------------------------------------------

 BMC-JY012005

 Games: 1998 Super HiK 8 in 1 (JY-021B)

 MMC1 clone with banking for multigame menu.

 NES 2.0: mapper 404

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_jy012005_device::set_prg()
{
	u8 mode = !BIT(m_latch0, 6);
	nes_sxrom_device::set_prg((m_latch0 & 0x07 & ~mode) << 3, mode << 3 | 0x07);
}

void nes_bmc_jy012005_device::set_chr()
{
	nes_sxrom_device::set_chr((m_latch0 & 0x07) << 5, 0x1f);
}

void nes_bmc_jy012005_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_jy012005 write_m, offset: %04x, data: %02x\n", offset, data);

	if (!BIT(m_latch0, 7))    // outer bank lock
	{
		m_latch0 = data;
		set_prg();
		set_chr();
	}
}

/*-------------------------------------------------

 BMC-JY820845C

 Games: 7 in 1 1993 Chess Series (JY-015)

 MMC1 clone with banking for multigame menu.

 NES 2.0: mapper 550

 In MAME: Supported.

 -------------------------------------------------*/

// The MMC1 base class re-applies set_prg() and set_chr() on its own (reset writes, register updates), so both must
// follow the outer register's mode: MMC1 mode uses the MMC1's banks, MHROM mode the latch
void nes_bmc_jy820845c_device::set_prg()
{
	if ((m_mode & 0x06) == 0x06)    // MMC1 mode
		nes_sxrom_device::set_prg(0x18, 0x07);
	else
		prg32((m_mode & 0x07) << 1 | BIT(m_latch0, 4));
}

void nes_bmc_jy820845c_device::set_chr()
{
	if ((m_mode & 0x06) == 0x06)    // MMC1 mode
		nes_sxrom_device::set_chr(0x18, 0x07);
	else
		chr8((m_mode & 0x06) << 1 | (m_latch0 & 0x03), CHRROM);
}

void nes_bmc_jy820845c_device::update_banks()
{
	set_prg();
	set_chr();
}

void nes_bmc_jy820845c_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_jy820845c write_m, offset: %04x, data: %02x\n", offset, data);

	nes_sxrom_device::write_m(offset, data);    // register overlaid on WRAM

	if (offset >= 0x1000 && !BIT(m_mode, 3))
	{
		m_mode = offset & 0x0f;
		update_banks();
	}
}

void nes_bmc_jy820845c_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_jy820845c write_h, offset: %04x, data: %02x\n", offset, data);

	m_latch0 = data;

	nes_sxrom_device::write_h(offset, data);    // the MMC1 sees the writes in MHROM mode too and decides the mirroring
	update_banks();                              // MHROM mode: the latch, not the MMC1, selects the banks
}

/*-------------------------------------------------

 FARID_SLROM_8-IN-1

 Games: 8 in 1

 MMC1 clone with banking for multigame menu.

 NES 2.0: mapper 323

 In MAME: Supported.

 -------------------------------------------------*/

void nes_farid_slrom_device::write_m(offs_t offset, u8 data)
{
	LOG("farid_slrom write_m, offset: %04x, data: %02x\n", offset, data);
	if (!BIT(m_reg[3], 4) && !BIT(m_outer, 3))    // MMC1 WRAM enabled and outer bank not locked
	{
		m_outer = data;
		set_prg();
		set_chr();
	}
}

/*-------------------------------------------------

 BMC-RESET-SXROM

 Games: 4 in 1 (JY-021, JY-022, JY-051)

 MMC1 clone with outer banks switched by resetting.

 NES 2.0: mapper 374

 In MAME: Supported.

 -------------------------------------------------*/

/*-------------------------------------------------

 BMC-SRPG-5IN1 (PCB has no distinguishing label)

 Games: Super RPG 5 in 1 CH501

 MMC1 clone with banking for multigame menu. Note: This
 game does not soft reset properly on real hardware.

 The interesting feature of this board is that it has a
 serially written 4-bit shift register that selects the
 outer game bank (meaning it has two shift registers,
 since it also clones the MMC1's). The MSB (bit 3) is 0 during
 the menu and 1 during game play (NESdev). The cart writes 0
 four times at power-on and the game's value (8, 9, 10, 14 or
 15) when a game is launched, and never writes the register
 again, so nothing shows whether the hardware does more with
 the MSB (a lock, say). We don't use the MSB nor allow it to be
 read back.

 NES 2.0: mapper 543

 In MAME: Supported.

 -------------------------------------------------*/

void nes_srpg5in1_device::write_l(offs_t offset, u8 data)
{
	LOG("srpg5in1 write_l, offset: %04x, data: %02x\n", offset, data);

	offset += 0x100;
	if (offset >= 0x1000)
	{
		m_outer_latch = (data & 0x08) | m_outer_latch >> 1;
		m_outer_count = (m_outer_count + 1) & 0x03;
		if (!m_outer_count)
		{
			m_outer = m_outer_latch;
			set_prg();
		}
	}
}

void nes_srpg5in1_device::write_m(offs_t offset, u8 data)
{
	LOG("srpg5in1 write_m, offset: %04x, data: %02x\n", offset, data);

	u8 bank = BIT(m_outer, 1) ? bitswap<3>(m_outer, 1, 2, 0) : (m_outer & 1) << 1 | BIT(m_reg[1], 3);

	if (!BIT(m_reg[3], 4) && !m_battery.empty())  // WRAM enabled
		m_battery[((bank * 0x2000) + offset) & (m_battery.size() - 1)] = data;
}

u8 nes_srpg5in1_device::read_m(offs_t offset)
{
	LOG("srpg5in1 read_m, offset: %04x\n", offset);

	u8 bank = BIT(m_outer, 1) ? bitswap<3>(m_outer, 1, 2, 0) : (m_outer & 1) << 1 | BIT(m_reg[1], 3);

	if (!BIT(m_reg[3], 4) && !m_battery.empty())  // WRAM enabled
		return m_battery[((bank * 0x2000) + offset) & (m_battery.size() - 1)];

	return get_open_bus();
}

/*-------------------------------------------------

 TXC 01-22110-000 Board

 Games: 2 in 1 Uzi Lightgun (MGC-002)

 This board has an MMC1 clone for Operation Wolf and
 otherwise is mostly compatible with mapper 70 for
 Bandai's Space Shadow. The mode register answers at
 every $4100-$5FFF address with A8 set (mask $E100) and
 the mirroring is fixed to vertical in both modes.

 NES 2.0: mapper 297

 In MAME: Supported.

 -------------------------------------------------*/

void nes_txc_22110_device::update_banks()    // used by menu and Space Shadow
{
	u8 outer = (m_mode & 0x02) << 1;
	prg16_89ab(outer | BIT(m_latch0, 4, 2));
	prg16_cdef(outer | 3);
	chr8(m_latch0 & 0x0f, CHRROM);
}

void nes_txc_22110_device::write_l(offs_t offset, u8 data)
{
	LOG("TXC 22110 write_l, offset: %04x, data: %02x\n", offset, data);
	if (BIT(offset + 0x4100, 8))        // mask $E100
	{
		m_mode = data;
		if (m_mode & 1)    // MMC1 mode
		{
			set_prg();
			set_chr();
		}
		else
			update_banks();
		set_nt_mirroring(PPU_MIRROR_VERT);
	}
}

void nes_txc_22110_device::write_h(offs_t offset, u8 data)
{
	LOG("TXC 22110 write_h, offset: %04x, data: %02x\n", offset, data);

	if (m_mode & 1)
		nes_sxrom_device::write_h(offset, data);
	else
	{
		m_latch0 = data;
		update_banks();
	}
	set_nt_mirroring(PPU_MIRROR_VERT);    // CIRAM A10 is wired to PA10
}

/*-------------------------------------------------

 Predator pirate (NES 2.0 mapper 621)

 Games: Predator (Asia) (En) (Pirate)

 NESdev has no page for this mapper; it is taken from FCEUmm's
 621.c: an MMC1A whose registers can also be loaded in parallel.
 A write to $6000-$7FFF with bit 7 set resets the MMC1 shift
 register, as a write to $8000-$FFFF does; otherwise bits 0-4 of
 the data are shifted into the MMC1 one by one, to the register
 selected by A1-A0 ($8000, $A000, $C000, $E000). FCEUmm spaces
 the five bits by two CPU cycles so that the MMC1 does not
 ignore them as consecutive writes; here the consecutive-write
 filter is cleared before each bit. $6000-$7FFF has no RAM
 (reads are open bus). The $8000-$FFFF registers work as on an
 MMC1A.

 NES 2.0: mapper 621

 In MAME: Supported.

 -------------------------------------------------*/

nes_bmc_621_device::nes_bmc_621_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_sxrom_device(mconfig, NES_BMC_621, tag, owner, clock)
{
}

void nes_bmc_621_device::pcb_reset()
{
	set_mmc1_type(mmc1_type::MMC1A);
	nes_sxrom_device::pcb_reset();
}

u8 nes_bmc_621_device::read_m(offs_t offset)
{
	return get_open_bus();
}

void nes_bmc_621_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc 621 write_m, offset: %04x, data: %02x\n", offset, data);

	const offs_t reg = (offset << 13) & 0x6000;     // A1-A0 select the MMC1 register
	if (BIT(data, 7))
	{
		nes_sxrom_device::write_h(reg, data);
		return;
	}
	for (int i = 0; i < 5; i++)
	{
		m_last_write_cycle = 0;     // the board's parallel load is not subject to the consecutive-write filter
		nes_sxrom_device::write_h(reg, BIT(data, i));
	}
}
