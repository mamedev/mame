// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for C&E PCBs


 Here we emulate the following PCBs

 * C&E Decathlon [mapper 244]
 * C&E Feng Shen Bang [mapper 246]
 * C&E Sheng Huo Lie Zhuan [mapper 240]

 ***********************************************************************************************************/


#include "emu.h"
#include "cne.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type NES_CNE_DECATHL = &device_creator<nes_cne_decathl_device>;
const device_type NES_CNE_FSB = &device_creator<nes_cne_fsb_device>;
const device_type NES_CNE_SHLZ = &device_creator<nes_cne_shlz_device>;


nes_cne_decathl_device::nes_cne_decathl_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_CNE_DECATHL, "NES Cart C&E Decathlon PCB", tag, owner, clock, "nes_cne_deca", __FILE__)
{
}

nes_cne_fsb_device::nes_cne_fsb_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_CNE_FSB, "NES Cart C&E Feng Shen Bang PCB", tag, owner, clock, "nes_cne_fsb", __FILE__)
{
}

nes_cne_shlz_device::nes_cne_shlz_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_CNE_SHLZ, "NES Cart C&E Sheng Huo Lie Zhuan PCB", tag, owner, clock, "nes_cne_shlz", __FILE__)
{
}



void nes_cne_decathl_device::device_start()
{
	common_start();
}

void nes_cne_decathl_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_cne_fsb_device::device_start()
{
	common_start();
}

void nes_cne_fsb_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0xff);
	chr8(0, m_chr_source);
}

void nes_cne_shlz_device::device_start()
{
	common_start();
}

void nes_cne_shlz_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}




/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 C & E Bootleg Board for Decathlon

 Games: Decathlon

 Pretty simple mapper: writes to 0x8065-0x80a4 set prg32 to
 offset & 3; writes to 0x80a5-0x80e4 set chr8 to offset & 7

 iNES: mapper 244

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_cne_decathl_device::write_h)
{
	LOG_MMC(("cne_decathl_w, offset: %04x, data: %02x\n", offset, data));

	if (offset < 0x0065)
		return;
	if (offset < 0x00a5)
	{
		prg32((offset - 0x0065) & 0x03);
		return;
	}
	if (offset < 0x00e5)
	{
		chr8((offset - 0x00a5) & 0x07, CHRROM);
	}
}

/*-------------------------------------------------

 C & E Bootleg Board for Fong Shen Bang

 Games: Fong Shen Bang - Zhu Lu Zhi Zhan

 Simple mapper: writes to 0x6000-0x67ff set PRG and CHR banks.
 Namely, 0x6000->0x6003 select resp. prg8_89, prg8_ab, prg8_cd
 and prg8_ef. 0x6004->0x6007 select resp. crh2_0, chr2_2,
 chr2_4 and chr2_6. In 0x6800-0x7fff lies WRAM. Battery backed?

 iNES: mapper 246

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_cne_fsb_device::write_m)
{
	LOG_MMC(("cne_fsb write_m, offset: %04x, data: %02x\n", offset, data));

	if (offset < 0x0800)
	{
		switch (offset & 0x0007)
		{
			case 0x0000:
				prg8_89(data);
				break;
			case 0x0001:
				prg8_ab(data);
				break;
			case 0x0002:
				prg8_cd(data);
				break;
			case 0x0003:
				prg8_ef(data);
				break;
			case 0x0004:
				chr2_0(data, CHRROM);
				break;
			case 0x0005:
				chr2_2(data, CHRROM);
				break;
			case 0x0006:
				chr2_4(data, CHRROM);
				break;
			case 0x0007:
				chr2_6(data, CHRROM);
				break;
		}
	}
	else
		m_battery[offset] = data;
}

READ8_MEMBER(nes_cne_fsb_device::read_m)
{
	LOG_MMC(("cne_fsb read_m, offset: %04x\n", offset));

	if (offset >= 0x0800)
		return m_battery[offset];

	return 0xff;
}

/*-------------------------------------------------

 C & E Bootleg Board for Sheng Huo Lie Zhuan

 Games: Jing Ke Xin Zhuan, Sheng Huo Lie Zhuan

 Simple Mapper: writes to 0x4020-0x5fff sets prg32 to
 data>>4 and chr8 to data&f. We currently do not map
 writes to 0x4020-0x40ff (to do: verify if this produces
 issues)

 iNES: mapper 240

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_cne_shlz_device::write_l)
{
	LOG_MMC(("cne_shlz write_l, offset: %04x, data: %02x\n", offset, data));

	prg32(data >> 4);
	chr8(data & 0x0f, CHRROM);
}
