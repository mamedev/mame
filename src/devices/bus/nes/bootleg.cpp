// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Bootleg PCBs


 Here we emulate the PCBs used in FDS2NES conversions which are common in the Taiwanese & HK markets
 Notice that many of these have unusual PRG sizes (32KB+8KB, 32KB+24KB) partially mapped in the WRAM
 area, or WRAM overlapping the usual PRG area, so that we often skip the usual bankswitching mechanisms
 in favor of direct handling of the PRG accesses

 TODO:
 - review all PCBs and fix the starting banks (which are often the main problem of not working games)
 - investigate pcbs listed in FCEUmm but with apparently no dumps available (LE05 and LH53)

 ***********************************************************************************************************/


#include "emu.h"
#include "bootleg.h"

#include "cpu/m6502/m6502.h"
#include "video/ppu2c0x.h"      // this has to be included so that IRQ functions can access PPU_BOTTOM_VISIBLE_SCANLINE


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type NES_AX5705 = &device_creator<nes_ax5705_device>;
const device_type NES_SC127 = &device_creator<nes_sc127_device>;
const device_type NES_MARIOBABY = &device_creator<nes_mbaby_device>;
const device_type NES_ASN = &device_creator<nes_asn_device>;
const device_type NES_SMB3PIRATE = &device_creator<nes_smb3p_device>;
const device_type NES_BTL_DNINJA = &device_creator<nes_btl_dn_device>;
const device_type NES_WHIRLWIND_2706 = &device_creator<nes_whirl2706_device>;
const device_type NES_SMB2J = &device_creator<nes_smb2j_device>;
const device_type NES_SMB2JA = &device_creator<nes_smb2ja_device>;
const device_type NES_SMB2JB = &device_creator<nes_smb2jb_device>;
const device_type NES_09034A = &device_creator<nes_09034a_device>;
const device_type NES_TOBIDASE = &device_creator<nes_tobidase_device>;
const device_type NES_LH32 = &device_creator<nes_lh32_device>;
const device_type NES_LH10 = &device_creator<nes_lh10_device>;
const device_type NES_LH53 = &device_creator<nes_lh53_device>;
const device_type NES_2708 = &device_creator<nes_2708_device>;
const device_type NES_AC08 = &device_creator<nes_ac08_device>;
const device_type NES_UNL_BB = &device_creator<nes_unl_bb_device>;
const device_type NES_MMALEE = &device_creator<nes_mmalee_device>;
const device_type NES_SHUIGUAN = &device_creator<nes_shuiguan_device>;


nes_ax5705_device::nes_ax5705_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_AX5705, "NES Cart AX5705 PCB", tag, owner, clock, "nes_ax5705", __FILE__)
{
}

nes_sc127_device::nes_sc127_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_SC127, "NES Cart SC-127 PCB", tag, owner, clock, "nes_sc127", __FILE__), m_irq_count(0), m_irq_enable(0)
				{
}

nes_mbaby_device::nes_mbaby_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_MARIOBABY, "NES Cart Mario Baby Bootleg PCB", tag, owner, clock, "nes_mbaby", __FILE__), m_latch(0), m_irq_enable(0), irq_timer(nullptr)
				{
}

nes_asn_device::nes_asn_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_ASN, "NES Cart Ai Senshi Nicol Bootleg PCB", tag, owner, clock, "nes_asn", __FILE__), m_latch(0)
				{
}

nes_smb3p_device::nes_smb3p_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_SMB3PIRATE, "NES Cart Super Mario Bros. 3 Pirate PCB", tag, owner, clock, "nes_smb3p", __FILE__), m_irq_count(0), m_irq_enable(0), irq_timer(nullptr)
				{
}

nes_btl_dn_device::nes_btl_dn_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_BTL_DNINJA, "NES Cart DragonNinja Pirate PCB", tag, owner, clock, "nes_btl_dn", __FILE__), m_irq_count(0)
				{
}

nes_whirl2706_device::nes_whirl2706_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_WHIRLWIND_2706, "NES Cart Whirlwind 2706 PCB", tag, owner, clock, "nes_whirl2706", __FILE__), m_latch(0)
				{
}

nes_smb2j_device::nes_smb2j_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_SMB2J, "NES Cart Super Mario Bros. 2 Jpn PCB", tag, owner, clock, "nes_smb2j", __FILE__), m_irq_count(0), m_irq_enable(0), irq_timer(nullptr)
				{
}

nes_smb2ja_device::nes_smb2ja_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_SMB2JA, "NES Cart Super Mario Bros. 2 Jpn (Alt) PCB", tag, owner, clock, "nes_smb2ja", __FILE__), m_irq_count(0), m_irq_enable(0), irq_timer(nullptr)
				{
}

nes_smb2jb_device::nes_smb2jb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_SMB2JB, "NES Cart Super Mario Bros. 2 Jpn (Alt 2) PCB", tag, owner, clock, "nes_smb2jb", __FILE__), m_irq_count(0), m_irq_enable(0), irq_timer(nullptr)
				{
}

nes_09034a_device::nes_09034a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_09034A, "NES Cart 09-034A PCB", tag, owner, clock, "nes_09034a", __FILE__), m_reg(0)
				{
}

nes_tobidase_device::nes_tobidase_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_TOBIDASE, "NES Cart Tobidase Daisakusen Pirate PCB", tag, owner, clock, "nes_tobidase", __FILE__), m_latch(0)
				{
}

nes_lh32_device::nes_lh32_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_LH32, "NES Cart LH-32 Pirate PCB", tag, owner, clock, "nes_lh32", __FILE__), m_latch(0)
				{
}

nes_lh10_device::nes_lh10_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_LH10, "NES Cart LH-10 Pirate PCB", tag, owner, clock, "nes_lh10", __FILE__), m_latch(0)
				{
}

nes_lh53_device::nes_lh53_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_LH53, "NES Cart LH-53 Pirate PCB", tag, owner, clock, "nes_lh53", __FILE__), m_irq_count(0), m_irq_enable(0), m_reg(0), irq_timer(nullptr)
				{
}

nes_2708_device::nes_2708_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_2708, "NES Cart BTL-2708 Pirate PCB", tag, owner, clock, "nes_2708", __FILE__)
{
}

nes_ac08_device::nes_ac08_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_AC08, "NES Cart AC08 Pirate PCB", tag, owner, clock, "nes_ac08", __FILE__), m_latch(0)
				{
}

nes_unl_bb_device::nes_unl_bb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_UNL_BB, "NES Cart FDS+CHR Pirate PCB", tag, owner, clock, "nes_unl_bb", __FILE__)
{
}

nes_mmalee_device::nes_mmalee_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_MMALEE, "NES Cart Super Mario Bros. Malee 2 Pirate PCB", tag, owner, clock, "nes_mmalee", __FILE__)
{
}

nes_shuiguan_device::nes_shuiguan_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_SHUIGUAN, "NES Cart Shui Guan Pipe Pirate PCB", tag, owner, clock, "nes_shuiguan", __FILE__), m_irq_count(0), m_irq_enable(0), irq_timer(nullptr)
				{
}




void nes_ax5705_device::device_start()
{
	common_start();
	save_item(NAME(m_mmc_prg_bank));
	save_item(NAME(m_mmc_vrom_bank));
}

void nes_ax5705_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	chr8(0, m_chr_source);

	m_mmc_prg_bank[0] = 0;
	m_mmc_prg_bank[1] = 1;
	prg8_89(m_mmc_prg_bank[0]);
	prg8_ab(m_mmc_prg_bank[1]);
	prg8_cd(0xfe);
	prg8_ef(0xff);

	memset(m_mmc_vrom_bank, 0, sizeof(m_mmc_vrom_bank));
}

void nes_sc127_device::device_start()
{
	common_start();
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
}

void nes_sc127_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0xff);
	chr8(0, m_chr_source);

	m_irq_enable = 0;
	m_irq_count = 0;
}

void nes_mbaby_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->reset();
	timer_freq = machine().device<cpu_device>("maincpu")->cycles_to_attotime(24576);

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_latch));
}

void nes_mbaby_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32((m_prg_chunks - 1) >> 1);
	chr8(0, m_chr_source);

	m_irq_enable = 0;
	m_latch = 0;
}

void nes_asn_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_asn_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32((m_prg_chunks - 1) >> 1);
	chr8(0, m_chr_source);

	m_latch = 0;
}

void nes_smb3p_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->adjust(attotime::zero, 0, machine().device<cpu_device>("maincpu")->cycles_to_attotime(1));

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
}

void nes_smb3p_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg8_89((m_prg_chunks << 1) - 1);
	prg8_ab(0);
	prg8_cd(0);
	prg8_ef((m_prg_chunks << 1) - 1);
	chr8(0, m_chr_source);

	m_irq_enable = 0;
	m_irq_count = 0;
}

void nes_btl_dn_device::device_start()
{
	common_start();
	save_item(NAME(m_irq_count));
}

void nes_btl_dn_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_irq_count = 0;
}

void nes_whirl2706_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_whirl2706_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0xff);
	chr8(0, m_chr_source);

	m_latch = 0;
}

void nes_smb2j_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->adjust(attotime::zero, 0, machine().device<cpu_device>("maincpu")->cycles_to_attotime(1));

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
}

void nes_smb2j_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	chr8(0, m_chr_source);
	prg8_89(1);
	prg8_ab(0);
	prg8_cd(0);
	prg8_ef(9);

	m_irq_enable = 0;
	m_irq_count = 0;
}

void nes_smb2ja_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->adjust(attotime::zero, 0, machine().device<cpu_device>("maincpu")->cycles_to_attotime(1));

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
}

void nes_smb2ja_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg8_89(0xfc);
	prg8_ab(0xfd);
	prg8_cd(0xfe);
	prg8_ef(0xff);
	chr8(0, m_chr_source);

	m_irq_enable = 0;
	m_irq_count = 0;
}

void nes_smb2jb_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->adjust(attotime::zero, 0, machine().device<cpu_device>("maincpu")->cycles_to_attotime(1));

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
}

void nes_smb2jb_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg8_89(0x08);
	prg8_ab(0x09);
	prg8_cd(0);
	prg8_ef(0x0b);
	chr8(0, m_chr_source);

	m_irq_enable = 0;
	m_irq_count = 0;
}

void nes_09034a_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_09034a_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
	m_reg = 0;
}

void nes_tobidase_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_tobidase_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(2);
	chr8(0, m_chr_source);

	m_latch = 0;
}

void nes_lh32_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_lh32_device::pcb_reset()
{
	chr8(0, CHRRAM);

	prg32((m_prg_chunks - 1) >> 1);
	// 0xc000-0xdfff reads/writes WRAM
	m_latch = 0xf;
}

void nes_lh10_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
	save_item(NAME(m_reg));
}

void nes_lh10_device::pcb_reset()
{
	chr8(0, CHRRAM);

	prg8_89(0);
	prg8_ab(0);
	// 0xc000-0xdfff reads/writes WRAM
	prg8_ef(0xff);
	memset(m_reg, 0, sizeof(m_reg));
	m_latch = 0;
}

void nes_lh53_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->adjust(attotime::zero, 0, machine().device<cpu_device>("maincpu")->cycles_to_attotime(1));

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_reg));
}

void nes_lh53_device::pcb_reset()
{
	chr8(0, CHRRAM);

	prg8_89(0xc);
	prg8_ab(0xd);   // last 2K are overlayed by WRAM
	prg8_cd(0xe);   // first 6K are overlayed by WRAM
	prg8_ef(0xf);
	m_reg = 0;
	m_irq_count = 0;
	m_irq_enable = 0;
}

void nes_2708_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_2708_device::pcb_reset()
{
	chr8(0, CHRRAM);

	prg32(7);
	// the upper PRG banks never change, but there are 8K of WRAM overlayed to the ROM area based on reg1
	m_reg[0] = 0;
	m_reg[1] = 0;
}

void nes_ac08_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_ac08_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	chr8(0, m_chr_source);
	prg32(0xff);
	m_latch = 0xff;
}

void nes_unl_bb_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_unl_bb_device::pcb_reset()
{
	chr8(0, CHRROM);
	prg32(0xff);
	// the upper PRG banks never change, but there are 8K of WRAM overlayed to the ROM area based on reg1
	m_reg[0] = 0xff;
	m_reg[1] = 0;
}

void nes_mmalee_device::device_start()
{
	common_start();
}

void nes_mmalee_device::pcb_reset()
{
	chr8(0, CHRROM);
	prg32(0);
}

void nes_shuiguan_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	// always running and checking for IRQ every 114 cycles? or resetting every frame?
	irq_timer->adjust(attotime::zero, 0, machine().device<cpu_device>("maincpu")->cycles_to_attotime(114));

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_mmc_vrom_bank));
}

void nes_shuiguan_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32((m_prg_chunks << 1) - 1);
	chr8(0, m_chr_source);

	m_irq_enable = 0;
	m_irq_count = 0;
	memset(m_mmc_vrom_bank, 0, sizeof(m_mmc_vrom_bank));
}



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Board UNL-AX5705

 Games: Super Mario Bros. Pocker Mali (Crayon Shin-chan pirate hack)

 In MESS: Supported

 -------------------------------------------------*/

void nes_ax5705_device::set_prg()
{
	prg8_89(m_mmc_prg_bank[0]);
	prg8_ab(m_mmc_prg_bank[1]);
}

WRITE8_MEMBER(nes_ax5705_device::write_h)
{
	UINT8 bank;
	LOG_MMC(("ax5705 write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x700f)
	{
		case 0x0000:
			m_mmc_prg_bank[0] = (data & 0x05) | ((data & 0x08) >> 2) | ((data & 0x02) << 2);
			set_prg();
			break;
		case 0x0008:
			set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		case 0x2000:
			m_mmc_prg_bank[1] = (data & 0x05) | ((data & 0x08) >> 2) | ((data & 0x02) << 2);
			set_prg();
			break;
			/* CHR banks 0, 1, 4, 5 */
		case 0x2008:
		case 0x200a:
		case 0x4008:
		case 0x400a:
			bank = ((offset & 0x4000) ? 4 : 0) + ((offset & 0x0002) ? 1 : 0);
			m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & 0xf0) | (data & 0x0f);
			chr1_x(bank, m_mmc_vrom_bank[bank], CHRROM);
			break;
		case 0x2009:
		case 0x200b:
		case 0x4009:
		case 0x400b:
			bank = ((offset & 0x4000) ? 4 : 0) + ((offset & 0x0002) ? 1 : 0);
			m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & 0x0f) | ((data & 0x04) << 3) | ((data & 0x02) << 5) | ((data & 0x09) << 4);
			chr1_x(bank, m_mmc_vrom_bank[bank], CHRROM);
			break;
			/* CHR banks 2, 3, 6, 7 */
		case 0x4000:
		case 0x4002:
		case 0x6000:
		case 0x6002:
			bank = 2 + ((offset & 0x2000) ? 4 : 0) + ((offset & 0x0002) ? 1 : 0);
			m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & 0xf0) | (data & 0x0f);
			chr1_x(bank, m_mmc_vrom_bank[bank], CHRROM);
			break;
		case 0x4001:
		case 0x4003:
		case 0x6001:
		case 0x6003:
			bank = 2 + ((offset & 0x2000) ? 4 : 0) + ((offset & 0x0002) ? 1 : 0);
			m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & 0x0f) | ((data & 0x04) << 3) | ((data & 0x02) << 5) | ((data & 0x09) << 4);
			chr1_x(bank, m_mmc_vrom_bank[bank], CHRROM);
			break;
	}
}

/*-------------------------------------------------

 SC-127 Board

 Games: Wario World II (Kirby Hack)

 iNES: mapper 35

 In MESS: Supported

 -------------------------------------------------*/

void nes_sc127_device::hblank_irq(int scanline, int vblank, int blanked)
{
	if (scanline < PPU_BOTTOM_VISIBLE_SCANLINE && m_irq_enable)
	{
		m_irq_count--;

		if (!blanked && (m_irq_count == 0))
		{
			LOG_MMC(("irq fired, scanline: %d (MAME %d, beam pos: %d)\n", scanline,
						machine().first_screen()->vpos(), machine().first_screen()->hpos()));
			m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
			m_irq_enable = 0;
		}
	}
}

WRITE8_MEMBER(nes_sc127_device::write_h)
{
	LOG_MMC(("sc127 write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset)
	{
		case 0x0000:
			prg8_89(data);
			break;
		case 0x0001:
			prg8_ab(data);
			break;
		case 0x0002:
//          m_mmc_prg_bank[offset & 0x02] = data;
			prg8_cd(data);
			break;
		case 0x1000:
		case 0x1001:
		case 0x1002:
		case 0x1003:
		case 0x1004:
		case 0x1005:
		case 0x1006:
		case 0x1007:
//          m_mmc_vrom_bank[offset & 0x07] = data;
			chr1_x(offset & 0x07, data, CHRROM);
			break;
		case 0x4002:
			m_irq_enable = 0;
			break;
		case 0x4003:
			m_irq_enable = 1;
			break;
		case 0x4005:
			m_irq_count = data;
			break;
		case 0x5001:
			set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
	}
}

/*-------------------------------------------------

 BTL-MARIOBABY

 Games: Mario Baby, Ai Senshi Nicol

 iNES: mapper 42

 In MESS: Supported.

 -------------------------------------------------*/

void nes_mbaby_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_IRQ)
	{
		m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
		irq_timer->adjust(attotime::never);
	}
}

WRITE8_MEMBER(nes_mbaby_device::write_h)
{
	LOG_MMC(("Mario Baby write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset >= 0x7000)
	{
		switch (offset & 0x03)
		{
			case 0x00:
				m_latch = data;
				break;
			case 0x01:
				set_nt_mirroring(BIT(data, 3) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
				break;
			case 0x02:
				/* Check if IRQ is being enabled */
				if (!m_irq_enable && (data & 0x02))
				{
					m_irq_enable = 1;
					irq_timer->adjust(timer_freq);
				}
				if (!(data & 0x02))
				{
					m_irq_enable = 0;
					irq_timer->adjust(attotime::never);
				}
				break;
		}
	}
}

READ8_MEMBER(nes_mbaby_device::read_m)
{
	LOG_MMC(("Mario Baby read_m, offset: %04x\n", offset));
	return m_prg[(m_latch * 0x2000) + (offset & 0x1fff)];
}

/*-------------------------------------------------

 BTL-AISENSHINICOL

 Games: Ai Senshi Nicol

 iNES: mapper 42 with no IRQ and no NT, but CHR switch

 In MESS: Partially Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_asn_device::write_h)
{
	LOG_MMC(("Ai Senshi Nicol write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset == 0x0000)
		chr8(data, CHRROM);

	if (offset == 0x7000)
		m_latch = data;
}

READ8_MEMBER(nes_asn_device::read_m)
{
	LOG_MMC(("Ai Senshi Nicol read_m, offset: %04x\n", offset));
	return m_prg[((m_latch * 0x2000) + (offset & 0x1fff)) & (m_prg_size - 1)];
}


/*-------------------------------------------------

 BTL-SMB3

 Games: Super Mario Bros. 3 Pirate

 iNES: mapper 106

 In MESS: Supported.

 -------------------------------------------------*/

void nes_smb3p_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_IRQ)
	{
		if (m_irq_enable)
		{
			if (m_irq_count == 0xffff)
			{
				m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
				m_irq_enable = 0;
			}
			else
				m_irq_count++;
		}
	}
}

WRITE8_MEMBER(nes_smb3p_device::write_h)
{
	LOG_MMC(("btl_smb3_w, offset: %04x, data: %02x\n", offset, data));
	switch (offset & 0x0f)
	{
		case 0x00:
		case 0x02:
			chr1_x(offset & 0x07, data & 0xfe, CHRROM);
			break;
		case 0x01:
		case 0x03:
			chr1_x(offset & 0x07, data | 0x01, CHRROM);
			break;
		case 0x04: case 0x05:
		case 0x06: case 0x07:
			chr1_x(offset & 0x07, data, CHRROM);
			break;
		case 0x08:
			prg8_89(data | 0x10);
			break;
		case 0x09:
			prg8_ab(data);
			break;
		case 0x0a:
			prg8_cd(data);
			break;
		case 0x0b:
			prg8_ef(data | 0x10);
			break;
		case 0x0c:
			set_nt_mirroring(BIT(data, 0) ?  PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		case 0x0d:
			m_irq_count = 0;
			m_irq_enable = 0;
			break;
		case 0x0e:
			m_irq_count = (m_irq_count & 0xff00) | data;
			break;
		case 0x0f:
			m_irq_count = (m_irq_count & 0x00ff) | (data << 8);
			m_irq_enable = 1;
			break;
	}
}

/*-------------------------------------------------

 BTL-DRAGONNINJA

 Games: Dragon Ninja (Bootleg), Super Mario Bros. 8

 iNES: mapper 222

 In MESS: Unsupported.

 -------------------------------------------------*/

/* Scanline based IRQ ? */
void nes_btl_dn_device::hblank_irq(int scanline, int vblank, int blanked )
{
	if (!m_irq_count || ++m_irq_count < 240)
		return;

	m_irq_count = 0;
	m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
}

WRITE8_MEMBER(nes_btl_dn_device::write_h)
{
	UINT8 bank;
	LOG_MMC(("btl_dn write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7003)
	{
		case 0x0000:
			prg8_89(data);
			break;
		case 0x1000:
			set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		case 0x2000:
			prg8_ab(data);
			break;
		case 0x3000:
		case 0x3002:
		case 0x4000:
		case 0x4002:
		case 0x5000:
		case 0x5002:
		case 0x6000:
		case 0x6002:
			bank = ((offset & 0x7000) - 0x3000) / 0x0800 + ((offset & 0x0002) >> 1);
			chr1_x(bank, data, CHRROM);
			break;
		case 0x7000:
			m_irq_count = data;
			break;
	}
}

/*-------------------------------------------------

 BOOTLEG FC VERSIONS OF FDS GAMES

 -------------------------------------------------*/

/*-------------------------------------------------

 WHIRLWIND-2706

 Games: Meikyuu Jiin Dababa (FDS conversion) and a few
 others

 This PCB maps PRG in 0x6000-0x7fff

 iNES: mapper 108

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_whirl2706_device::write_h)
{
	LOG_MMC(("whirl2706 write_h, offset: %04x, data: %02x\n", offset, data));
	m_latch = data;
}

READ8_MEMBER(nes_whirl2706_device::read_m)
{
	LOG_MMC(("whirl2706 read_m, offset: %04x\n", offset));
	return m_prg[(m_latch * 0x2000 + (offset & 0x1fff)) & (m_prg_size - 1)];
}

/*-------------------------------------------------

 Bootleg Board SMB2J

 Games: Super Mario Bros. 2 Pirate (LF36)

 iNES: mapper 43

 In MESS: Supported.

 -------------------------------------------------*/

void nes_smb2j_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_IRQ)
	{
		if (m_irq_enable)
		{
			if (m_irq_count == 0xfff)
			{
				m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
				m_irq_enable = 0;
				m_irq_count = 0;
			}
			else
				m_irq_count++;
		}
	}
}

WRITE8_MEMBER(nes_smb2j_device::write_l)
{
	LOG_MMC(("smb2j write_l, offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;

	if (offset == 0x122)    // $4122
		m_irq_enable = data & 3;    // maybe also m_irq_count = 0?!?
}

WRITE8_MEMBER(nes_smb2j_device::write_h)
{
	LOG_MMC(("smb2j write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset == 0x122)    // $8122 too?
		m_irq_enable = data & 3;
}

WRITE8_MEMBER(nes_smb2j_device::write_ex)
{
	LOG_MMC(("smb2j write_ex, offset: %04x, data: %02x\n", offset, data));

	if (offset == 2)
	{
		int temp = 0;

		// According to hardware tests
		if (data & 1)
			temp = 3;
		else
			temp = 4 + ((data & 7) >> 1);

		prg8_cd(temp);
	}
}

READ8_MEMBER(nes_smb2j_device::read_l)
{
	LOG_MMC(("smb2j read_l, offset: %04x\n", offset));
	offset += 0x100;

	if (offset >= 0x1000)
		return m_prg[0x10000 + (offset & 0x0fff)];

	return m_open_bus;   // open bus
}

READ8_MEMBER(nes_smb2j_device::read_m)
{
	LOG_MMC(("smb2j read_m, offset: %04x\n", offset));
	return m_prg[0x4000 + offset];
}

/*-------------------------------------------------

 BTL-SMB2A

 Games: Super Mario Bros. 2 Pirate (Jpn version of SMB2)

 iNES: mapper 40

 In MESS: Supported.

 -------------------------------------------------*/

void nes_smb2ja_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_IRQ)
	{
		if (m_irq_enable)
		{
			if (m_irq_count == 0xfff)
			{
				m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
				m_irq_enable = 0;
				m_irq_count = 0;
			}
			else
				m_irq_count++;
		}
	}
}

WRITE8_MEMBER(nes_smb2ja_device::write_h)
{
	LOG_MMC(("smb2ja write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6000)
	{
		case 0x0000:
			m_irq_enable = 0;
			m_irq_count = 0;
			break;
		case 0x2000:
			m_irq_enable = 1;
			break;
		case 0x6000:
			prg8_cd(data);
			break;
	}
}

READ8_MEMBER(nes_smb2ja_device::read_m)
{
	LOG_MMC(("smb2ja read_m, offset: %04x\n", offset));
	return m_prg[(0xfe * 0x2000 + (offset & 0x1fff)) & (m_prg_size - 1)];
}

/*-------------------------------------------------

 BTL-SMB2B

 Games: Super Mario Bros. 2 Pirate (Jpn version of SMB2)

 This was marked as Alt. Levels. is it true?

 iNES: mapper 50

 In MESS: Partially Supported.

 -------------------------------------------------*/

void nes_smb2jb_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_IRQ)
	{
		if (m_irq_enable)
		{
			if (m_irq_count == 0xfff)
			{
				m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
				m_irq_enable = 0;
				m_irq_count = 0;
			}
			else
				m_irq_count++;
		}
	}
}

WRITE8_MEMBER(nes_smb2jb_device::write_l)
{
	UINT8 prg;
	LOG_MMC(("smb2jb write_l, offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;

	switch (offset & 0x1e0)
	{
		case 0x020:
		case 0x0a0:
			prg = (data & 0x08) | ((data & 0x06) >> 1) | ((data & 0x01) << 2);
			prg8_cd(prg);
			break;
		case 0x120:
		case 0x1a0:
			m_irq_enable = data & 0x01;
			break;
	}
}

READ8_MEMBER(nes_smb2jb_device::read_m)
{
	LOG_MMC(("smb2jb read_m, offset: %04x\n", offset));
	return m_prg[((0x0f * 0x2000) + (offset & 0x1fff)) & (m_prg_size - 1)];
}

/* This goes to 0x4020-0x403f & 0x40a0-0x40bf */
WRITE8_MEMBER(nes_smb2jb_device::write_ex)
{
	UINT8 prg;
	LOG_MMC(("smb2jb write_ex, offset: %04x, data: %02x\n", offset, data));

	if ((offset < 0x20) || (offset >= 0x80 && offset < 0xa0))
	{
		prg = (data & 0x08) | ((data & 0x06) >> 1) | ((data & 0x01) << 2);
		prg8_cd(prg);
	}
}

/*-------------------------------------------------

 (UNL-)09-034A

 Games: Zanac FDS conversion with two PRG chips and
 no CHRROM and Volleyball FDS conversion with two PRG
 chips and CHRROM.
 Originally dumps were marked as UNL-SMB2J pcb

 iNES:

 In MESS: Partially Supported. Need to emulate IRQ
 (needed by smb2 conversion?)

 -------------------------------------------------*/

WRITE8_MEMBER(nes_09034a_device::write_ex)
{
	LOG_MMC(("09-034a write_ex, offset: %04x, data: %02x\n", offset, data));

	if (offset == 7)    // $4027
		m_reg = data & 1;
}

READ8_MEMBER(nes_09034a_device::read_m)
{
	LOG_MMC(("09-034a read_m, offset: %04x\n", offset));
	// in 0x6000-0x7fff is mapped the 2nd PRG chip which starts after 32K (hence the +4)
	return m_prg[((m_reg + 4) * 0x2000) + offset];
}

/*-------------------------------------------------

 Bootleg Board used for FDS conversion

 Games: Tobidase Daisakusen (FDS conversion)

 This PCB maps PRG in 0x6000-0x7fff

 iNES: mapper 120

 In MESS: Partially Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_tobidase_device::write_l)
{
	LOG_MMC(("tobidase write_h, offset: %04x, data: %02x\n", offset, data));
	offset += 0x4100;

	if ((offset & 0x63c0) == 0x41c0)
		m_latch = data & 0x0f;
}

READ8_MEMBER(nes_tobidase_device::read_m)
{
	LOG_MMC(("tobidase read_m, offset: %04x\n", offset));
	if (m_latch >= 0x0c)
		m_latch -= 4;
	return m_prg[(m_latch * 0x2000) + (offset & 0x1fff)];
}

/*-------------------------------------------------

 UNL-LH32

 Games: Monty no Doki Doki Daidassou (FDS conversion)

 This PCB maps WRAM in 0xc000-0xdfff and PRG in 0x6000-0x7fff

 iNES:

 In MESS: Supported.

-------------------------------------------------*/

READ8_MEMBER(nes_lh32_device::read_m)
{
	LOG_MMC(("lh32 read_m, offset: %04x\n", offset));
	return m_prg[(m_latch * 0x2000) + (offset & 0x1fff)];
}

READ8_MEMBER(nes_lh32_device::read_h)
{
	LOG_MMC(("lh32 read_h, offset: %04x\n", offset));

	if (offset >= 0x4000 && offset < 0x6000)
		return m_prgram[offset & 0x1fff];

	return hi_access_rom(offset);
}

WRITE8_MEMBER(nes_lh32_device::write_m)
{
	LOG_MMC(("lh32 write_m, offset: %04x, data: %02x\n", offset, data));

	if (offset == 0)    // 0x6000 only?
	{
//      printf("write %x\n", data);
		m_latch = data & 0xf;
	}
}

WRITE8_MEMBER(nes_lh32_device::write_h)
{
	LOG_MMC(("lh32 write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset >= 0x4000 && offset < 0x6000)
		m_prgram[offset & 0x1fff] = data;
}

/*-------------------------------------------------

 UNL-LH10

 Games: Fuuun Shaolin Kyo (FDS conversion)

 This PCB maps WRAM in 0xc000-0xdfff and PRG in 0x6000-0x7fff
 This is very similar to KS7037 (not sure which conversion
 uses that one)

 iNES:

 In MESS: Supported.

 -------------------------------------------------*/

void nes_lh10_device::update_prg()
{
	prg8_89(m_reg[6]);
	prg8_ab(m_reg[7]);
}

READ8_MEMBER(nes_lh10_device::read_m)
{
	LOG_MMC(("lh10 read_m, offset: %04x\n", offset));
	return m_prg[(0x0e * 0x2000) + (offset & 0x1fff)];
}

READ8_MEMBER(nes_lh10_device::read_h)
{
	LOG_MMC(("lh10 read_h, offset: %04x\n", offset));

	if (offset >= 0x4000 && offset < 0x6000)
		return m_prgram[offset & 0x1fff];

	return hi_access_rom(offset);
}

WRITE8_MEMBER(nes_lh10_device::write_h)
{
	LOG_MMC(("lh10 write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset >= 0x4000 && offset < 0x6000)
		m_prgram[offset & 0x1fff] = data;

	else
	{
		switch (offset & 0x6001)
		{
			case 0x0000:
				m_latch = data & 7;
				break;
			case 0x0001:
				m_reg[m_latch] = data;
				update_prg();
				break;
		}
	}
}

/*-------------------------------------------------

 UNL-LH53

 Games: Nazo no Murasamejou (FDS conversion)

 This PCB maps WRAM (w/battery) in 0xb800-0xd7ff and
 PRG in 0x6000-0x7fff

 iNES:

 In MESS: Preliminar Support only.

 -------------------------------------------------*/

void nes_lh53_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_IRQ)
	{
		if (m_irq_enable)
		{
			m_irq_count++;
			if (m_irq_count > 7560)//value from FCEUMM...
			{
				m_irq_count = 0;
				m_maincpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
			}
		}
	}
}

READ8_MEMBER(nes_lh53_device::read_m)
{
	LOG_MMC(("lh53 read_m, offset: %04x\n", offset));
	return m_prg[(m_reg * 0x2000) + (offset & 0x1fff)];
}

READ8_MEMBER(nes_lh53_device::read_h)
{
	LOG_MMC(("lh53 read_h, offset: %04x\n", offset));

	if (offset >= 0x3800 && offset < 0x5800)
		return m_battery[offset & 0x1fff];

	return hi_access_rom(offset);
}

WRITE8_MEMBER(nes_lh53_device::write_h)
{
	LOG_MMC(("lh53 write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset >= 0x3800 && offset < 0x5800)
		m_battery[offset & 0x1fff] = data;

	else
	{
		switch (offset & 0x7000)
		{
			case 0x6000:
				m_irq_enable = BIT(data, 1);
				m_irq_count = 0;
				if (!m_irq_enable)
					m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
				break;
			case 0x7000:
				m_reg = data & 0x0f;
				break;
		}
	}
}

/*-------------------------------------------------

 BTL-2708

 Games: Doki Doki Panic (FDS conversion)

 iNES: mapper 103?

 This board has a very unique setup, with 8KB of WRAM
 in 0x6000-0x7fff and other 8KB of WRAM in 0xb800-0xd7ff
 which can be switched in 2KB chunks (we attempt to
 emulate this by intercepting reads in that area before
 they get to the PRG banks...)

 In MESS: Supported.

 -------------------------------------------------*/

READ8_MEMBER(nes_2708_device::read_m)
{
	LOG_MMC(("btl-2708 read_m, offset: %04x\n", offset));
	if (!m_reg[1])
		return m_prgram[offset];    // lower 8K of WRAM
	else
		return m_prg[(m_reg[0] * 0x2000) + (offset & 0x1fff)];
}

WRITE8_MEMBER(nes_2708_device::write_m)
{
	LOG_MMC(("btl-2708 write_m, offset: %04x, data: %02x\n", offset, data));
//  if (!m_reg[1])
		m_prgram[offset] = data;    // lower 8K of WRAM
}

READ8_MEMBER(nes_2708_device::read_h)
{
	LOG_MMC(("btl-2708 read_h, offset: %04x\n", offset));

	if (offset >= 0x3800 && offset < 0x5800 && !m_reg[1])
		return m_prgram[0x2000 + ((offset - 0x3800) & 0x1fff)]; // higher 8K of WRAM

	return hi_access_rom(offset);
}

WRITE8_MEMBER(nes_2708_device::write_h)
{
	LOG_MMC(("btl-2708 write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset >= 0x3800 && offset < 0x5800/* && !m_reg[1]*/)
		m_prgram[0x2000 + ((offset - 0x3800) & 0x1fff)] = data; // higher 8K of WRAM

	switch (offset & 0x7000)
	{
		case 0x0000:
			m_reg[0] = data & 0x0f; // PRG bank in 0x6000-0x7fff
			break;
		case 0x6000:
			set_nt_mirroring(BIT(data, 3) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		case 0x7000:
			m_reg[1] = BIT(data, 4);    // bit4 enables the two WRAM banks
			break;
	}
}

/*-------------------------------------------------

 UNL-AC08

 Games: Green Beret (FDS conversions)

 iNES:

 This board has two PRG chips. The first (128K) is
 connected to 0x6000-0x7fff and switches among the
 16x8K banks; the second (32K) is fixed in 0x8000-0xffff

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_ac08_device::write_ex)
{
	LOG_MMC(("AC-08 write_ex, offset: %04x, data: %02x\n", offset, data));
	if (offset == 5)    // $4025
		set_nt_mirroring(!BIT(data, 3) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
}

READ8_MEMBER(nes_ac08_device::read_m)
{
	LOG_MMC(("AC-08 read_m, offset: %04x\n", offset));
	return m_prg[(m_latch * 0x2000) + (offset & 0x1fff)];
}

WRITE8_MEMBER(nes_ac08_device::write_h)
{
	LOG_MMC(("AC-08 write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset == 1)
		m_latch = (data >> 1) & 0x0f;
	else
		m_latch = data & 0x0f;  // apparently there also is a Castlevania FDS conversion using same board with different banking lines
}

/*-------------------------------------------------

 UNL-BB

 Games: Bubble Bobble and other FDS conversions with CHRROM!

 iNES:


 In MESS: Supported.

 -------------------------------------------------*/

READ8_MEMBER(nes_unl_bb_device::read_m)
{
	LOG_MMC(("unl-bb read_m, offset: %04x\n", offset));
	return m_prg[(((m_reg[0] & 3 & m_prg_mask) * 0x2000) + (offset & 0x1fff))];
}

WRITE8_MEMBER(nes_unl_bb_device::write_h)
{
	LOG_MMC(("unl-bb write_h, offset: %04x, data: %02x\n", offset, data));

	if (!(offset & 0x1000))
	{
		m_reg[0] = data;
		m_reg[1] = data;
	}
	else
		m_reg[1] = data & 1;    // Pro Wrestling uses this

	chr8(m_reg[1] & 3, m_chr_source);
}

/*-------------------------------------------------

 BTL-MARIO1-MALEE2 (aka Genius Merio Bros)

 Games: Super Mario Bros Malee 2

 iNES:

 This PCB has two PRG chips (32K+2K) + one CHR chip (8K)
 + 2KB of WRAM
 The second PRG chip (2K) is connected at 0x6000-0x6800
 while WRAM is at 0x7000-0x7800

 In MESS: Supported.

 -------------------------------------------------*/

READ8_MEMBER(nes_mmalee_device::read_m)
{
	LOG_MMC(("mmalee read_m, offset: %04x\n", offset));

	if (offset < 0x0800)
		return m_prg[0x8000 + offset];
	else if (!m_prgram.empty() && offset >= 0x1000 && offset < 0x1800)   // WRAM only in these 2K
		return m_prgram[offset & 0x7ff];

	return ((offset + 0x6000) & 0xff00) >> 8;
}

WRITE8_MEMBER(nes_mmalee_device::write_m)
{
	LOG_MMC(("mmalee write_m, offset: %04x, data: %02x\n", offset, data));

	if (!m_prgram.empty() && offset >= 0x1000 && offset < 0x1800)    // WRAM only in these 2K
		m_prgram[offset & 0x7ff] = data;
}

/*-------------------------------------------------

 BTL-SHUIGUANPIPE

 Games: Shui Guan Pipe (Gimmick Pirate)

 iNES:

 In MESS: Supported, but there are glitches (PPU or IRQ?)

 -------------------------------------------------*/

// timer always running and checking IRQ every 114 CPU cycles?
void nes_shuiguan_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_IRQ)
	{
		m_irq_count++;
		m_irq_count &= 0xff;

		if (m_irq_enable && !m_irq_count)
			m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
	}
}

WRITE8_MEMBER(nes_shuiguan_device::write_h)
{
	int bank;
	LOG_MMC(("shuiguan write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7000)
	{
		case 0x0000:
			if (offset & 0x800 && !(offset & 0x0c)) // 0x8800-0x8803 + N*0x10
				prg8_89(data);
			break;
		case 0x1000:
			if (offset & 0x800 && !(offset & 0x0c)) // 0x9800-0x9803 + N*0x10
			{
				switch (data & 0x03)
				{
					case 0: set_nt_mirroring(PPU_MIRROR_VERT); break;
					case 1: set_nt_mirroring(PPU_MIRROR_HORZ); break;
					case 2: set_nt_mirroring(PPU_MIRROR_LOW); break;
					case 3: set_nt_mirroring(PPU_MIRROR_HIGH); break;
				}
			}
			break;
		case 0x2000:
			if (!(offset & 0x800) && !(offset & 0x0c))  // 0xa000-0xa003 + N*0x10
				prg8_cd(data);
			if (offset & 0x800 && !(offset & 0x0c)) // 0xa800-0xa803 + N*0x10
				prg8_ab(data);
			break;
		case 0x3000:
		case 0x4000:
		case 0x5000:
		case 0x6000:
			bank = (((offset + 0x1000) >> 11) | (offset >> 3)) & 0x07;
			if (offset & 4)
				m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & 0x0f) | ((data & 0x0f) << 4);
			else
				m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & 0xf0) | ((data & 0x0f) << 0);
			chr1_x(bank, m_mmc_vrom_bank[bank], m_chr_source);
			break;
		case 0x7000:
			switch (offset & 0x0c)
			{
				case 0x00: m_irq_count = (m_irq_count & 0xf0) | ((data & 0x0f) << 0); break;
				case 0x04: m_irq_count = (m_irq_count & 0x0f) | ((data & 0x0f) << 4); break;
				case 0x08: m_irq_enable= data; break;
				case 0x0c: break;
			}
			break;
	}
}

READ8_MEMBER(nes_shuiguan_device::read_m)
{
	// always first bank??
	LOG_MMC(("shuiguan read_m, offset: %04x\n", offset));
	return m_prg[offset & 0x1fff];
}
