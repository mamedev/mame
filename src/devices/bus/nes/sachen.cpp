// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Sachen PCBs


 Here we emulate the following PCBs

 * Sachen SA-009 [mapper 160]
 * Sachen SA-0036 [mapper 148]
 * Sachen SA-0037 [mapper 149]
 * Sachen SA-72007 [mapper 145]
 * Sachen SA-72008 [mapper 133]
 * Sachen TCA-01 [mapper 143]
 * Sachen TCU-01 [mapper 147]
 * Sachen TCU-02 [mapper 136]
 * Sachen Discrete PCBs [mapper 150 & 243]
 * Sachen 8259 [mapper 141 (A), 138 (B), 139 (C), 137 (D)]

 Known issues on specific mappers:

 * 133 Qi Wang starts with corrupted graphics (ingame seems better)


 ***********************************************************************************************************/


#include "emu.h"
#include "sachen.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type NES_SACHEN_SA009 = &device_creator<nes_sachen_sa009_device>;
const device_type NES_SACHEN_SA0036 = &device_creator<nes_sachen_sa0036_device>;
const device_type NES_SACHEN_SA0037 = &device_creator<nes_sachen_sa0037_device>;
const device_type NES_SACHEN_SA72007 = &device_creator<nes_sachen_sa72007_device>;
const device_type NES_SACHEN_SA72008 = &device_creator<nes_sachen_sa72008_device>;
const device_type NES_SACHEN_TCA01 = &device_creator<nes_sachen_tca01_device>;
const device_type NES_SACHEN_TCU01 = &device_creator<nes_sachen_tcu01_device>;
const device_type NES_SACHEN_TCU02 = &device_creator<nes_sachen_tcu02_device>;
const device_type NES_SACHEN_74X374 = &device_creator<nes_sachen_74x374_device>;
const device_type NES_SACHEN_74X374_ALT = &device_creator<nes_sachen_74x374_alt_device>;
const device_type NES_SACHEN_8259A = &device_creator<nes_sachen_8259a_device>;
const device_type NES_SACHEN_8259B = &device_creator<nes_sachen_8259b_device>;
const device_type NES_SACHEN_8259C = &device_creator<nes_sachen_8259c_device>;
const device_type NES_SACHEN_8259D = &device_creator<nes_sachen_8259d_device>;


nes_sachen_sa009_device::nes_sachen_sa009_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_SACHEN_SA009, "NES Cart Sachen SA009 PCB", tag, owner, clock, "nes_sa009", __FILE__)
{
}

nes_sachen_sa0036_device::nes_sachen_sa0036_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_SACHEN_SA0036, "NES Cart Sachen sa0036 PCB", tag, owner, clock, "nes_sa0036", __FILE__)
{
}

nes_sachen_sa0037_device::nes_sachen_sa0037_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_SACHEN_SA0037, "NES Cart Sachen sa0037 PCB", tag, owner, clock, "nes_sa0037", __FILE__)
{
}

nes_sachen_sa72007_device::nes_sachen_sa72007_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_SACHEN_SA72007, "NES Cart Sachen SA72007 PCB", tag, owner, clock, "nes_sa72007", __FILE__)
{
}

nes_sachen_sa72008_device::nes_sachen_sa72008_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_SACHEN_SA72008, "NES Cart Sachen SA72008 PCB", tag, owner, clock, "nes_sa72008", __FILE__)
{
}

nes_sachen_tca01_device::nes_sachen_tca01_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_SACHEN_TCA01, "NES Cart Sachen TCA-01 PCB", tag, owner, clock, "nes_tca01", __FILE__)
{
}

nes_sachen_tcu01_device::nes_sachen_tcu01_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_SACHEN_TCU01, "NES Cart Sachen TCU-01 PCB", tag, owner, clock, "nes_tcu01", __FILE__)
{
}

nes_sachen_tcu02_device::nes_sachen_tcu02_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_SACHEN_TCU02, "NES Cart Sachen TCU-02 PCB", tag, owner, clock, "nes_tcu02", __FILE__), m_latch(0)
				{
}

nes_sachen_74x374_device::nes_sachen_74x374_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: nes_nrom_device(mconfig, type, name, tag, owner, clock, shortname, source), m_latch(0), m_mmc_vrom_bank(0)
				{
}

nes_sachen_74x374_device::nes_sachen_74x374_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_SACHEN_74X374, "NES Cart Sachen 74*374 PCB", tag, owner, clock, "nes_s74x34", __FILE__), m_latch(0), m_mmc_vrom_bank(0)
				{
}

nes_sachen_74x374_alt_device::nes_sachen_74x374_alt_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_sachen_74x374_device(mconfig, NES_SACHEN_74X374_ALT, "NES Cart Sachen 74*374 Alt PCB", tag, owner, clock, "nes_s74x34a", __FILE__)
{
}

nes_sachen_8259a_device::nes_sachen_8259a_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: nes_sachen_74x374_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

nes_sachen_8259a_device::nes_sachen_8259a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_sachen_74x374_device(mconfig, NES_SACHEN_8259A, "NES Cart Sachen 8259A PCB", tag, owner, clock, "nes_s8259a", __FILE__)
{
}

nes_sachen_8259b_device::nes_sachen_8259b_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_sachen_8259a_device(mconfig, NES_SACHEN_8259B, "NES Cart Sachen 8259B PCB", tag, owner, clock, "nes_s8259b", __FILE__)
{
}

nes_sachen_8259c_device::nes_sachen_8259c_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_sachen_8259a_device(mconfig, NES_SACHEN_8259C, "NES Cart Sachen 8259C PCB", tag, owner, clock, "nes_s8259c", __FILE__)
{
}

nes_sachen_8259d_device::nes_sachen_8259d_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_sachen_8259a_device(mconfig, NES_SACHEN_8259D, "NES Cart Sachen 8259D PCB", tag, owner, clock, "nes_s8259d", __FILE__)
{
}




void nes_sachen_sa009_device::device_start()
{
	common_start();
}

void nes_sachen_sa009_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_sachen_sa0036_device::device_start()
{
	common_start();
}

void nes_sachen_sa0036_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_sachen_sa0037_device::device_start()
{
	common_start();
}

void nes_sachen_sa0037_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_sachen_sa72007_device::device_start()
{
	common_start();
}

void nes_sachen_sa72007_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_sachen_sa72008_device::device_start()
{
	common_start();
}

void nes_sachen_sa72008_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_sachen_tca01_device::device_start()
{
	common_start();
}

void nes_sachen_tca01_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(1);
	chr8(0, m_chr_source);
}

void nes_sachen_tcu01_device::device_start()
{
	common_start();
}

void nes_sachen_tcu01_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_sachen_tcu02_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_sachen_tcu02_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);

	m_latch = 0;
}

void nes_sachen_74x374_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
	save_item(NAME(m_mmc_vrom_bank));
}

void nes_sachen_74x374_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);

	m_latch = 0;
	m_mmc_vrom_bank = 0;
}

void nes_sachen_8259a_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
	save_item(NAME(m_reg));
}

void nes_sachen_8259a_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
	set_nt_mirroring(PPU_MIRROR_VERT);

	m_latch = 0;
	memset(m_reg, 0, sizeof(m_reg));
}

void nes_sachen_8259d_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(m_vrom_chunks - 1, CHRROM);
	set_nt_mirroring(PPU_MIRROR_VERT);

	m_latch = 0;
	memset(m_reg, 0, sizeof(m_reg));
}




/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Sachen SA009 bootleg boards

 Games: Pipe 5

 iNES: mapper 160

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_sachen_sa009_device::write_l)
{
	LOG_MMC(("SA009 write_l, offset: %04x, data: %02x\n", offset, data));

	chr8(data, m_chr_source);
}

/*-------------------------------------------------

 Sachen SA0036 bootleg boards

 Games: Taiwan Mahjong 16

 iNES: mapper 149

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_sachen_sa0036_device::write_h)
{
	LOG_MMC(("sa0036 write_h, offset: %04x, data: %02x\n", offset, data));

	chr8(data >> 7, CHRROM);
}

/*-------------------------------------------------

 Sachen SA0037 bootleg boards

 Games: Mahjong World, Shisen Mahjong

 iNES: mapper 148

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_sachen_sa0037_device::write_h)
{
	LOG_MMC(("sa0037 write_h, offset: %04x, data: %02x\n", offset, data));

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	prg32(data >> 3);
	chr8(data, CHRROM);
}

/*-------------------------------------------------

 Sachen SA72007 bootleg boards

 Games: Sidewinder

 iNES: mapper 145

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_sachen_sa72007_device::write_l)
{
	LOG_MMC(("SA72007 write_l, offset: %04x, data: %02x\n", offset, data));

	/* only if we are at 0x4100 + k * 0x200, but 0x4100 is offset = 0 */
	if (!(offset & 0x100))
		chr8(data >> 7, CHRROM);
}

/*-------------------------------------------------

 Sachen SA72008 bootleg boards

 Games: Jovial Race, Qi Wang

 iNES: mapper 133

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_sachen_sa72008_device::write_l)
{
	LOG_MMC(("SA72008 write_l, offset: %04x, data: %02x\n", offset, data));

	prg32(data >> 2);
	chr8(data, CHRROM);
}

/*-------------------------------------------------

 Sachen TCA-01 bootleg boards

 iNES: mapper 143

 Games: Dancing Blocks, Magic Mathematic

 In MESS: Supported.

 -------------------------------------------------*/

READ8_MEMBER(nes_sachen_tca01_device::read_l)
{
	LOG_MMC(("TCA-01 read_l, offset: %04x\n", offset));

	/* the address is read only if we are at 0x4100 + k * 0x200, but 0x4100 is offset = 0 */
	if (!(offset & 0x100))
		return (~offset & 0x3f) | 0x40;
	else
		return 0x00;
}

/*-------------------------------------------------

 Sachen TCU-01 bootleg boards

 Games: Challenge of the Dragon, Chinese Kungfu

 iNES: mapper 147

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_sachen_tcu01_device::write_l)
{
	LOG_MMC(("TCU-01 write_l, offset: %04x, data: %02x\n", offset, data));

	if ((offset & 0x103) == 0x002)
	{
		prg32(((data >> 6) & 0x02) | ((data >> 2) & 0x01));
		chr8(data >> 3, CHRROM);
	}
}

/*-------------------------------------------------

 Sachen TCU-02 bootleg boards

 Games: Wei Lai Xiao Zi

 iNES: mapper 136

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_sachen_tcu02_device::write_l)
{
	LOG_MMC(("TCU-02 write_l, offset: %04x, data: %02x\n", offset, data));

	if ((offset & 0x103) == 0x002)
	{
		m_latch = (data & 0x30) | ((data + 3) & 0x0f);
		chr8(m_latch, CHRROM);
	}
}

READ8_MEMBER(nes_sachen_tcu02_device::read_l)
{
	LOG_MMC(("TCU-02 read_l, offset: %04x\n", offset));

	if ((offset & 0x103) == 0x000)
		return m_latch | 0x40;
	else
		return 0x00;
}

/*-------------------------------------------------

 Sachen 74x374 bootleg boards

 Games: Chess Academy, Chinese Checkers Jpn, Mahjong Academy,
 Olympic IQ, Poker II, Tasac [150], Poker III [243]

 iNES: mappers 150 & 243

 -------------------------------------------------*/

void nes_sachen_74x374_device::set_mirror(UINT8 nt) // also used by mappers 137, 138, 139, 141
{
	switch (nt)
	{
		case 0:
		case 1:
			set_nt_mirroring(nt ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		case 2:
			set_nt_page(0, CIRAM, 0, 1);
			set_nt_page(1, CIRAM, 1, 1);
			set_nt_page(2, CIRAM, 1, 1);
			set_nt_page(3, CIRAM, 1, 1);
			break;
		case 3:
			set_nt_mirroring(PPU_MIRROR_LOW);
			break;
		default:
			LOG_MMC(("Mapper set NT to invalid value %02x", nt));
			break;
	}
}


WRITE8_MEMBER(nes_sachen_74x374_device::write_l)
{
	LOG_MMC(("Sachen 74*374 write_l, offset: %04x, data: %02x\n", offset, data));

	/* write happens only if we are at 0x4100 + k * 0x200, but 0x4100 is offset = 0 */
	if (!(offset & 0x100))
	{
		if (!(offset & 0x01))
			m_latch = data & 0x07;
		else
		{
			switch (m_latch)
			{
				case 0x02:
					m_mmc_vrom_bank = (m_mmc_vrom_bank & ~0x08) | ((data << 3) & 0x08);
					chr8(m_mmc_vrom_bank, CHRROM);
					prg32(data & 0x01);
					break;
				case 0x04:
					m_mmc_vrom_bank = (m_mmc_vrom_bank & ~0x04) | ((data << 2) & 0x04);
					chr8(m_mmc_vrom_bank, CHRROM);
					break;
				case 0x05:
					prg32(data & 0x07);
					break;
				case 0x06:
					m_mmc_vrom_bank = (m_mmc_vrom_bank & ~0x03) | ((data << 0) & 0x03);
					chr8(m_mmc_vrom_bank, CHRROM);
					break;
				case 0x07:
					set_mirror((data >> 1) & 0x03);
					break;
				default:
					break;
			}
		}
	}
}

READ8_MEMBER(nes_sachen_74x374_device::read_l)
{
	LOG_MMC(("Sachen 74*374 read_l, offset: %04x", offset));

	/* read  happens only if we are at 0x4100 + k * 0x200, but 0x4100 is offset = 0 */
	if (!(offset & 0x100))
		return (~m_latch & 0x3f) /* ^ dips*/;  // we would need to check the Dips here
	else
		return 0;
}

WRITE8_MEMBER(nes_sachen_74x374_alt_device::write_l)
{
	LOG_MMC(("Sachen 74*374 Alt write_l, offset: %04x, data: %02x\n", offset, data));

	/* write happens only if we are at 0x4100 + k * 0x200, but 0x4100 is offset = 0 */
	if (!(offset & 0x100))
	{
		if (!(offset & 0x01))
			m_latch = data;
		else
		{
			switch (m_latch & 0x07)
			{
				case 0x00:
					prg32(0);
					chr8(3, CHRROM);
					break;
				case 0x02:
					m_mmc_vrom_bank = (m_mmc_vrom_bank & ~0x08) | ((data << 3) & 0x08);
					chr8(m_mmc_vrom_bank, CHRROM);
					break;
				case 0x04:
					m_mmc_vrom_bank = (m_mmc_vrom_bank & ~0x01) | ((data << 0) & 0x01);
					chr8(m_mmc_vrom_bank, CHRROM);
					break;
				case 0x05:
					prg32(data & 0x01);
					break;
				case 0x06:
					m_mmc_vrom_bank = (m_mmc_vrom_bank & ~0x06) | ((data << 1) & 0x06);
					chr8(m_mmc_vrom_bank, CHRROM);
					break;
				case 0x07:
					set_mirror(BIT(data, 0));
					break;
				default:
					break;
			}
		}
	}
}

/*-------------------------------------------------

 Sachen S8259 bootleg boards

 iNES: mapper 141 (A), 138 (B), 139 (C), 137 (D)

 -------------------------------------------------*/

void nes_sachen_8259a_device::chr_update()
{
	UINT8 bank_helper1 = m_reg[7] & 0x01;
	UINT8 bank_helper2 = (m_reg[4] & 0x07) << 3;

	if (m_chr_source == CHRROM)
	{
		chr2_0(((m_reg[bank_helper1 ? 0 : 0] & 0x07) | bank_helper2) << 1 | 0, CHRROM);
		chr2_2(((m_reg[bank_helper1 ? 0 : 1] & 0x07) | bank_helper2) << 1 | 1, CHRROM);
		chr2_4(((m_reg[bank_helper1 ? 0 : 2] & 0x07) | bank_helper2) << 1 | 0, CHRROM);
		chr2_6(((m_reg[bank_helper1 ? 0 : 3] & 0x07) | bank_helper2) << 1 | 1, CHRROM);
	}
}

WRITE8_MEMBER(nes_sachen_8259a_device::write_l)
{
	LOG_MMC(("Sachen 8259 write_l, offset: %04x, data: %02x\n", offset, data));

	/* write happens only if we are at 0x4100 + k * 0x200, but 0x4100 is offset = 0 */
	if (!(offset & 0x100))
	{
		if (!(offset & 0x01))
			m_latch = data & 0x07;
		else
		{
			m_reg[m_latch] = data;

			switch (m_latch)
			{
				case 0x05:
					prg32(data);
					break;
				case 0x07:
					set_mirror(BIT(data, 0) ? 0 : (data >> 1) & 0x03);
					break;
				default:
					chr_update();
					break;
			}
		}
	}
}

void nes_sachen_8259b_device::chr_update()
{
	UINT8 bank_helper1 = m_reg[7] & 0x01;
	UINT8 bank_helper2 = (m_reg[4] & 0x07) << 3;

	if (m_chr_source == CHRROM)
	{
		chr2_0(((m_reg[bank_helper1 ? 0 : 0] & 0x07) | bank_helper2), CHRROM);
		chr2_2(((m_reg[bank_helper1 ? 0 : 1] & 0x07) | bank_helper2), CHRROM);
		chr2_4(((m_reg[bank_helper1 ? 0 : 2] & 0x07) | bank_helper2), CHRROM);
		chr2_6(((m_reg[bank_helper1 ? 0 : 3] & 0x07) | bank_helper2), CHRROM);
	}
}

void nes_sachen_8259c_device::chr_update()
{
	UINT8 bank_helper1 = m_reg[7] & 0x01;
	UINT8 bank_helper2 = (m_reg[4] & 0x07) << 3;

	if (m_chr_source == CHRROM)
	{
		chr2_0(((m_reg[bank_helper1 ? 0 : 0] & 0x07) | bank_helper2) << 2 | 0, CHRROM);
		chr2_2(((m_reg[bank_helper1 ? 0 : 1] & 0x07) | bank_helper2) << 2 | 1, CHRROM);
		chr2_4(((m_reg[bank_helper1 ? 0 : 2] & 0x07) | bank_helper2) << 2 | 2, CHRROM);
		chr2_6(((m_reg[bank_helper1 ? 0 : 3] & 0x07) | bank_helper2) << 2 | 3, CHRROM);
	}
}

void nes_sachen_8259d_device::chr_update()
{
	if (m_chr_source == CHRROM)
	{
		chr1_0((m_reg[0] & 0x07), CHRROM);
		chr1_1((m_reg[1] & 0x07) | (m_reg[4] << 4 & 0x10), CHRROM);
		chr1_2((m_reg[2] & 0x07) | (m_reg[4] << 3 & 0x10), CHRROM);
		chr1_3((m_reg[3] & 0x07) | (m_reg[4] << 2 & 0x10) | (m_reg[6] << 3 & 0x08), CHRROM);
	}
}
