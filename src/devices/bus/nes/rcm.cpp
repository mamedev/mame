// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for RCM PCBs


 Here we emulate the following PCBs

 * RCM GS2004 [mapper 283]
 * RCM GS2013 [mapper 283 also]
 * RCM GS2015 [mapper 216]
 * RCM Tetris Family 9in1 [mapper 61]
 * RCM 3D Block [mapper 355]

 ***********************************************************************************************************/


#include "emu.h"
#include "rcm.h"



#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_GS2004,  nes_gs2004_device,  "nes_gs2004",    "NES Cart RCM GS-2004 PCB")
DEFINE_DEVICE_TYPE(NES_GS2013,  nes_gs2013_device,  "nes_gs2013",    "NES Cart RCM GS-2013 PCB")
DEFINE_DEVICE_TYPE(NES_GS2015,  nes_gs2015_device,  "nes_gs2015",    "NES Cart RCM GS-2015 PCB")
DEFINE_DEVICE_TYPE(NES_TF9IN1,  nes_tf9_device,     "nes_tetrisfam", "NES Cart RCM Tetris Family 9 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_3DBLOCK, nes_3dblock_device, "nes_3dblock",   "NES Cart RCM 3D Block PCB")


nes_gs2015_device::nes_gs2015_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_GS2015, tag, owner, clock)
{
}

nes_gs2004_device::nes_gs2004_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int bank)
	: nes_nrom_device(mconfig, type, tag, owner, clock), m_base(bank * 0x2000)
{
}

nes_gs2004_device::nes_gs2004_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_gs2004_device(mconfig, NES_GS2004, tag, owner, clock, 0x20)
{
}

nes_gs2013_device::nes_gs2013_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_gs2004_device(mconfig, NES_GS2013, tag, owner, clock, 0x1f)
{
}

nes_tf9_device::nes_tf9_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_TF9IN1, tag, owner, clock)
{
}

nes_3dblock_device::nes_3dblock_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_3DBLOCK, tag, owner, clock)
{
}




void nes_gs2015_device::device_start()
{
	common_start();
}

void nes_gs2015_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_3dblock_device::device_start()
{
	common_start();
}

void nes_3dblock_device::pcb_reset()
{
	prg32(0);
	chr8(0, CHRRAM);
}




/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 RCM GS2015 Board

 Games: Bonza, Magic Jewelry 2

 Very simple mapper: writes to 0x8000-0xffff sets prg32
 to offset and chr8 to offset>>1 (when chrrom is present)

 iNES: mapper 216

 In MAME: Partially supported. Bonza games have some
 sort of card inputs for gambling purposes.

 -------------------------------------------------*/

void nes_gs2015_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("gs2015 write_h, offset: %04x, data: %02x\n", offset, data));

	prg32(offset);
	chr8(offset >> 1, m_chr_source);
}

uint8_t nes_gs2015_device::read_m(offs_t offset)
{
	LOG_MMC(("gs2015 read_m, offset: %04x\n", offset));
	return 0;   // Videopoker Bonza needs this (sort of protection? or related to inputs?)
}

/*-------------------------------------------------

 Boards BMC-GS2004, BMC-GS2013

 Games: Tetris Family 6-in-1, 5-in-1, 12-in-1

 NES 2.0: mapper 283

 In MAME: Supported.

 -------------------------------------------------*/

u8 nes_gs2004_device::read_m(offs_t offset)
{
	LOG_MMC(("gs2004 read_m, offset: %04x\n", offset));
	return m_prg[m_base + offset];    // fixed base differs per device
}

void nes_gs2004_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("gs2004 write_h, offset: %04x, data: %02x\n", offset, data));
	prg32(data);
}

/*-------------------------------------------------

 Bootleg Board by RCM for Tetris Family

 Games: Tetris Family 9 in 1, 20 in 1

 iNES: mapper 61

 In MAME: Supported.

 -------------------------------------------------*/

void nes_tf9_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("tetrisfam write_h, offset: %04x, data: %02x\n", offset, data));

	u8 bank = (offset & 0x0f) << 1 | BIT(offset, 5);
	u8 mode = !BIT(offset, 4);
	prg16_89ab(bank & ~mode);
	prg16_cdef(bank | mode);

	set_nt_mirroring(BIT(offset, 7) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 Bootleg Board by RCM for 3-D Block

 Games: 3-D Block Hwang Shinwei version

 NES 2.0: mapper 355

 In MAME: Not supported.

 TODO: implement PIC16C54 protection

 -------------------------------------------------*/
