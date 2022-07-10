// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for NTDEC PCBs


 Here we emulate the following PCBs

 * NTDEC ASDER [mapper 112]
 * NTDEC Fighting Hero [mapper 193]
 * NTDEC N715021 [mapper 81]

 TODO:
 - why is Master Shooter not working? (correctly aimed shots score as a hit on random targets)

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

DEFINE_DEVICE_TYPE(NES_NTDEC_ASDER,   nes_ntdec_asder_device,   "nes_ntdec_asder",   "NES Cart NTDEC Asder PCB")
DEFINE_DEVICE_TYPE(NES_NTDEC_FH,      nes_ntdec_fh_device,      "nes_ntdec_fh",      "NES Cart NTDEC Fighting Hero PCB")
DEFINE_DEVICE_TYPE(NES_NTDEC_N715021, nes_ntdec_n715021_device, "nes_ntdec_n715021", "NES Cart NTDEC N715021 PCB")


nes_ntdec_asder_device::nes_ntdec_asder_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_NTDEC_ASDER, tag, owner, clock), m_latch(0), m_chr_outer(0)
{
}

nes_ntdec_fh_device::nes_ntdec_fh_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_NTDEC_FH, tag, owner, clock)
{
}

nes_ntdec_n715021_device::nes_ntdec_n715021_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_NTDEC_N715021, tag, owner, clock)
{
}




void nes_ntdec_asder_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
	save_item(NAME(m_chr_outer));
}

void nes_ntdec_asder_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, CHRROM);

	m_latch = 0;
	m_chr_outer = 0;
}

void nes_ntdec_fh_device::pcb_reset()
{
	prg32((m_prg_chunks - 1) >> 1);
	chr8(0, CHRROM);
}

void nes_ntdec_n715021_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, CHRROM);
}



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 NTDEC ASDER Bootleg Board

 Games: Cobra Mission, Fighting Hero III, Huang Di, Master
 Shooter

 This board supports swappable 8K PRG banks at 0x8000 and
 0xa000 via 2 registers. 16K above is fixed. CHR ROM is
 selected by 6 registers: 2x2K for the first two, and 4x1K
 for the remaining 4. The former can only address 256K of
 CHR while the latter combines with latch at 0xc000 to
 span 512K. Registers are not directly written to but
 selected at 0x8000.

 iNES: mapper 112

 In MAME: Supported.

 -------------------------------------------------*/

void nes_ntdec_asder_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("ntdec_asder write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6001)    // writes only at even addresses?
	{
		case 0x0000:
			m_latch = data & 0x07;
			break;
		case 0x2000:
			switch (m_latch)
			{
				case 0: case 1:
					prg8_x(m_latch, data);
					break;
				case 2: case 3:
					chr2_x((m_latch & 0x01) << 1, data >> 1, CHRROM);
					break;
				case 4: case 5: case 6: case 7:
					u16 high = BIT(m_chr_outer, m_latch) << 8;
					chr1_x(m_latch, high | data, CHRROM);
					break;
			}
			break;
		case 0x4000:
			m_chr_outer = data;
			break;
		case 0x6000:
			set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
	}
}

/*-------------------------------------------------

 Bootleg Board by NTDEC for Fighting Hero

 Games: Fighting Hero, War in the Gulf

 Very simple mapper: writes to 0x6000-0x7fff swap PRG and
 CHR banks. The mirroring register at 0x6004 appears to
 not be used by any known software?

 iNES: mapper 193

 In MAME: Supported.

 -------------------------------------------------*/

void nes_ntdec_fh_device::write_m(offs_t offset, u8 data)
{
	LOG_MMC(("ntdec_fh write_m, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x07)
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
			prg8_89(data & 0x0f);
			break;
		case 4:
			set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
	}
}

/*-------------------------------------------------

 NTDEC Board N715021

 Games: Super Gun

 Very simple mapper: writes to 0x8000-0xffff swap PRG and
 CHR banks. PCB also has a latch at 0x6000 but it is not used.

 iNES: mapper 81

 In MAME: Supported.

 -------------------------------------------------*/

void nes_ntdec_n715021_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("ntdec_n715021 write_h, offset: %04x, data: %02x\n", offset, data));
	prg16_89ab(BIT(offset, 2, 2));
	chr8(offset & 0x03, CHRROM);
}
