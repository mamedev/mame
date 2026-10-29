// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
/***********************************************************************************************************


 NES/Famicom cartridge emulation for NTDEC PCBs


 Here we emulate the following PCBs

 * NTDEC ASDER [mapper 112]
 * NTDEC Fighting Hero [mapper 193]
 * NTDEC N715021 [mapper 81]

 ***********************************************************************************************************/


#include "emu.h"
#include "ntdec.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE (LOG_GENERAL)
#else
#define VERBOSE (0)
#endif
#include "logmacro.h"


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_NTDEC_ASDER,   nes_ntdec_asder_device,   "nes_ntdec_asder",   "NES Cart NTDEC Asder PCB")
DEFINE_DEVICE_TYPE(NES_NTDEC_FH,      nes_ntdec_fh_device,      "nes_ntdec_fh",      "NES Cart NTDEC Fighting Hero PCB")
DEFINE_DEVICE_TYPE(NES_NTDEC_N715021, nes_ntdec_n715021_device, "nes_ntdec_n715021", "NES Cart NTDEC N715021 PCB")


nes_ntdec_asder_device::nes_ntdec_asder_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_NTDEC_ASDER, tag, owner, clock), m_latch(0), m_chr_outer(0), m_chr_1k{}
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
	save_item(NAME(m_chr_1k));
}

void nes_ntdec_asder_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, CHRROM);

	// Mirroring is controlled by the $E000 register, so header flags 6 bit 0 is 0 for this board (NES 2.0: "vertical
	// arrangement or mapper-controlled") and says nothing about the power-on state; the loader turns that 0 into
	// horizontal. Master Shooter never writes $E000: it fills $2000 and then $2400, and only shows its menu text with
	// vertical mirroring (nes.xml and nes.hsi both give vertical for it). The other mapper 112 games all write $E000
	// early on, so start vertical.
	set_nt_mirroring(PPU_MIRROR_VERT);

	m_latch = 0;
	m_chr_outer = 0;
	for (int i = 0; i < 4; i++)
		m_chr_1k[i] = 4 + i;    // matches chr8(0) above
}

void nes_ntdec_asder_device::update_chr_1k(int slot)
{
	// the outer bit for each 1K bank is applied whenever either register changes
	chr1_x(slot, (BIT(m_chr_outer, slot) << 8) | m_chr_1k[slot - 4], CHRROM);
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
 Shooter, San Guo Zhi - Qun Xiong Zheng Ba, Sanguozhi -
 Chibi Zhi Zhan, Zhen Ben Xiyou Ji

 This board supports swappable 8K PRG banks at 0x8000 and
 0xa000 via 2 registers. 16K above is fixed. CHR ROM is
 selected by 6 registers: 2x2K for the first two, and 4x1K
 for the remaining 4. The former can only address 256K of
 CHR while the latter combines with latch at 0xc000 to
 span 512K. Registers are not directly written to but
 selected at 0x8000.

 Registers decode with mask 0xe001 (NESdev): 0x8000 selects
 the register, 0xa000 writes it and 0xe000 sets the mirroring
 (0 = CIRAM A10 from PPU A10, i.e. vertical; 1 = A11, i.e.
 horizontal). Writes at odd addresses are ignored. The register
 at 0xc000 is not documented on NESdev: its bits 4-7 supply bit
 8 of the four 1K CHR banks (Fighting Hero III has 512K of
 CHR) and take effect at once, whichever register was written
 last, as in FCEUX and Mesen. No game was seen to write
 anything but 0 there, so this is untested.

 The games also use 8K of RAM at 0x6000 (Fighting Hero III
 fills it at boot), which is not described by NESdev. nes.xml
 lists it as battery backed, while the NES 2.0 headers in the
 NES-Roms2 set say volatile; this is not settled.

 iNES: mapper 112

 In MAME: Supported.

 -------------------------------------------------*/

void nes_ntdec_asder_device::write_h(offs_t offset, u8 data)
{
	LOG("ntdec_asder write_h, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x6001)    // mask $E001: writes at odd addresses are ignored
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
					m_chr_1k[m_latch - 4] = data;
					update_chr_1k(m_latch);
					break;
			}
			break;
		case 0x4000:
			m_chr_outer = data;
			for (int slot = 4; slot < 8; slot++)
				update_chr_1k(slot);
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
 CHR banks. Registers decode with mask 0xe007 (NESdev):
 0x6000 selects the 4K CHR bank at 0x0000 (xxxx xx..), 0x6001
 and 0x6002 the 2K CHR banks at 0x1000 and 0x1800, 0x6003 the
 8K PRG bank at 0x8000 (the other three 8K banks are fixed to
 the last ones) and 0x6004 the mirroring (0 = vertical,
 1 = horizontal). NESdev notes that no game is known to change
 mirroring at run time and that at least some boards have it
 hard-wired.

 iNES: mapper 193

 In MAME: Supported.

 -------------------------------------------------*/

void nes_ntdec_fh_device::write_m(offs_t offset, u8 data)
{
	LOG("ntdec_fh write_m, offset: %04x, data: %02x\n", offset, data);

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
 CHR banks. The board latches the four lowest address bits
 (xxxx xxxx xxxx PPCC), has no bus conflicts and ignores any
 write to 0x6000, even though the game's code suggests a
 second latch there. Mirroring is fixed by solder pads.

 iNES: mapper 81

 In MAME: Supported.

 -------------------------------------------------*/

void nes_ntdec_n715021_device::write_h(offs_t offset, u8 data)
{
	LOG("ntdec_n715021 write_h, offset: %04x, data: %02x\n", offset, data);
	prg16_89ab(BIT(offset, 2, 2));
	chr8(offset & 0x03, CHRROM);
}
