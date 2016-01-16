// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for TXC PCBs

 Here we emulate the following PCBs

 * TXC 22111 [mapper 132]
 * TXC Du Ma Racing [mapper 172]
 * TXC Mahjong Block [mapper 172]
 * TXC Strike Wolf [mapper 36]
 * TXC Commandos (and many more) [mapper 241]

 TODO:
 - Does Commandos indeed use this board?


 ***********************************************************************************************************/


#include "emu.h"
#include "txc.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type NES_TXC_22211 = &device_creator<nes_txc_22211_device>;
const device_type NES_TXC_DUMARACING = &device_creator<nes_txc_dumarc_device>;
const device_type NES_TXC_MJBLOCK = &device_creator<nes_txc_mjblock_device>;
const device_type NES_TXC_STRIKEW = &device_creator<nes_txc_strikew_device>;
const device_type NES_TXC_COMMANDOS = &device_creator<nes_txc_commandos_device>;


nes_txc_22211_device::nes_txc_22211_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
					: nes_nrom_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

nes_txc_22211_device::nes_txc_22211_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_TXC_22211, "NES Cart TXC 22211 PCB", tag, owner, clock, "nes_txc_22211", __FILE__)
{
}

nes_txc_dumarc_device::nes_txc_dumarc_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_txc_22211_device(mconfig, NES_TXC_DUMARACING, "NES Cart TXC Du Ma Racing PCB", tag, owner, clock, "nes_dumarc", __FILE__)
{
}

nes_txc_mjblock_device::nes_txc_mjblock_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_txc_22211_device(mconfig, NES_TXC_MJBLOCK, "NES Cart TXC Mahjong Block PCB", tag, owner, clock, "nes_mjblock", __FILE__)
{
}

nes_txc_strikew_device::nes_txc_strikew_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_TXC_STRIKEW, "NES Cart Strike Wolf PCB", tag, owner, clock, "nes_txc_strikew", __FILE__)
{
}

nes_txc_commandos_device::nes_txc_commandos_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_TXC_COMMANDOS, "NES Cart Commandos (and others) PCB", tag, owner, clock, "nes_txc_comm", __FILE__)
{
}




void nes_txc_22211_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_txc_22211_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
	m_reg[0] = m_reg[1] = m_reg[2] = m_reg[3] = 0;
}

void nes_txc_strikew_device::device_start()
{
	common_start();
}

void nes_txc_strikew_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_txc_commandos_device::device_start()
{
	common_start();
}

void nes_txc_commandos_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}





/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Bootleg Board 22211 by TXC (Type A)

 Games: Creatom

 Info from NEStopia: this mapper features write to four
 registers (0x4100-0x4103). The third one is used to select
 PRG and CHR banks.

 iNES: mapper 132

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_txc_22211_device::write_l)
{
	LOG_MMC(("TXC 22111 write_l, offset: %04x, data: %02x\n", offset, data));

	if (offset < 4)
		m_reg[offset & 0x03] = data;
}

READ8_MEMBER(nes_txc_22211_device::read_l)
{
	LOG_MMC(("TXC 22111 read_l, offset: %04x\n", offset));

	if (offset == 0x0000)
		return (m_reg[1] ^ m_reg[2]) | 0x40;
	else
		return 0x00;
}

WRITE8_MEMBER(nes_txc_22211_device::write_h)
{
	LOG_MMC(("TXC 22111 write_h, offset: %04x, data: %02x\n", offset, data));

	prg32(m_reg[2] >> 2);
	chr8(m_reg[2], CHRROM);
}

/*-------------------------------------------------

 Bootleg Board 22211 by TXC (Type B)

 Games: 1991 Du Ma Racing

 This mapper is basically the same as Type A. Only difference is
 in the way CHR banks are selected (see below)

 iNES: mapper 172

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_txc_dumarc_device::write_h)
{
	LOG_MMC(("TXC Du Ma Racing write_h, offset: %04x, data: %02x\n", offset, data));

	prg32(m_reg[2] >> 2);
	chr8((((data ^ m_reg[2]) >> 3) & 0x02) | (((data ^ m_reg[2]) >> 5) & 0x01), CHRROM);
}

/*-------------------------------------------------

 Bootleg Board 22211 by TXC (Type C)

 Games: Mahjong Block, Xiao Ma Li

 This mapper is basically the same as 132 too. Only difference is
 in 0x4100 reads which expect also bit 0 to be set

 iNES: mapper 172

 In MESS: Supported.

 -------------------------------------------------*/

READ8_MEMBER(nes_txc_mjblock_device::read_l)
{
	LOG_MMC(("TXC mjblock read_l, offset: %04x\n", offset));

	if (offset == 0x0000)
		return (m_reg[1] ^ m_reg[2]) | 0x41;
	else
		return 0x00;
}

/*-------------------------------------------------

 Bootleg Board 'Strike Wolf' by TXC

 Games: Strike Wolf and Policeman

 iNES: mapper 36

 In MESS: Partially Supported (Policeman has glitches)

 -------------------------------------------------*/

WRITE8_MEMBER(nes_txc_strikew_device::write_h)
{
	LOG_MMC(("TXC Strike Wolf write_h, offset: %04x, data: %02x\n", offset, data));

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	if ((offset >= 0x400) && (offset < 0x7fff))
	{
		prg32(data >> 4);
		chr8(data & 0x0f, CHRROM);
	}
}

/*-------------------------------------------------

 Bootleg Board MXMDHTWO by TXC

 Games: Commandos, Journey to the West, Ma Bu Mi Zhen &
 Qu Wei Cheng Yu Wu, Si Lu Chuan Qi

 Simple Mapper: writes to 0x8000-0xffff sets the prg32 bank.
 Not sure if returning 0x50 for reads in 0x4100-0x5000 is correct.

 iNES: mapper 241

 In MESS: Supported.

 -------------------------------------------------*/

READ8_MEMBER(nes_txc_commandos_device::read_l)
{
	return 0x50;
}

WRITE8_MEMBER(nes_txc_commandos_device::write_h)
{
	LOG_MMC(("TXC Commandos write_h, offset: %04x, data: %02x\n", offset, data));

	prg32(data);
}
