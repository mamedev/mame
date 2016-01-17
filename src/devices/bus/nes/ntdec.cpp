// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for NTDEC PCBs


 Here we emulate the following PCBs

 * NTDEC ASDER [mapper 112]
 * NTDEC Fighting Hero [mapper 193]

 TODO:
 - why is Master Shooter not working?

 ***********************************************************************************************************/


#include "emu.h"
#include "ntdec.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type NES_NTDEC_ASDER = &device_creator<nes_ntdec_asder_device>;
const device_type NES_NTDEC_FH = &device_creator<nes_ntdec_fh_device>;


nes_ntdec_asder_device::nes_ntdec_asder_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_NTDEC_ASDER, "NES Cart NTDEC Asder PCB", tag, owner, clock, "nes_ntdec_asder", __FILE__),
	m_latch(0)
				{
}

nes_ntdec_fh_device::nes_ntdec_fh_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_NTDEC_FH, "NES Cart NTDEC Fighting Hero PCB", tag, owner, clock, "nes_ntdec_fh", __FILE__)
{
}




void nes_ntdec_asder_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_ntdec_asder_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_latch = 0;
}

void nes_ntdec_fh_device::device_start()
{
	common_start();
}

void nes_ntdec_fh_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32((m_prg_chunks - 1) >> 1);
	chr8(0, m_chr_source);
	set_nt_mirroring(PPU_MIRROR_VERT);
}





/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 NTDEC ASDER Bootleg Board

 Games: Cobra Mission, Fighting Hero III, Huang Di, Master
 Shooter

 iNES: mapper 112

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_ntdec_asder_device::write_h)
{
	LOG_MMC(("ntdec_asder write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset)
	{
		case 0x0000:
			m_latch = data & 0x07;
			break;
		case 0x2000:
			switch (m_latch)
			{
				case 0:
					prg8_89(data);
					break;
				case 1:
					prg8_ab(data);
					break;
				case 2:
					data &= 0xfe;
					chr1_0(data, CHRROM);
					chr1_1(data + 1, CHRROM);
					break;
				case 3:
					data &= 0xfe;
					chr1_2(data, CHRROM);
					chr1_3(data + 1, CHRROM);
					break;
				case 4:
					chr1_4(data, CHRROM);
					break;
				case 5:
					chr1_5(data, CHRROM);
					break;
				case 6:
					chr1_6(data, CHRROM);
					break;
				case 7:
					chr1_7(data, CHRROM);
					break;
			}
			break;
		case 0x6000:
			set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
	}
}

/*-------------------------------------------------

 Bootleg Board by NTDEC for Fighting Hero

 Games: Fighting Hero

 Very simple mapper: writes to 0x6000-0x7fff swap PRG and
 CHR banks.

 iNES: mapper 193

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_ntdec_fh_device::write_m)
{
	LOG_MMC(("ntdec_fh write_m, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x03)
	{
		case 0:
			chr4_0(data >> 2, CHRROM);
			break;
		case 1:
			chr2_4(data >> 1, CHRROM);
			break;
		case 2:
			chr2_6(data >> 1 , CHRROM);
			break;
		case 3:
			prg8_89(data);
			break;
	}
}
