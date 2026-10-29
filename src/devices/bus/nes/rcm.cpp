// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
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
#define VERBOSE (LOG_GENERAL)
#else
#define VERBOSE (0)
#endif
#include "logmacro.h"


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
	, m_pic(*this, "pic")
	, m_addr(0)
	, m_irq(false)
{
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
	LOG("gs2015 write_h, offset: %04x, data: %02x\n", offset, data);

	prg32(offset);
	chr8(offset >> 1, m_chr_source);
}

uint8_t nes_gs2015_device::read_m(offs_t offset)
{
	LOG("gs2015 read_m, offset: %04x\n", offset);
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
	LOG("gs2004 read_m, offset: %04x\n", offset);
	return m_prg[m_base + offset];    // fixed base differs per device
}

void nes_gs2004_device::write_h(offs_t offset, u8 data)
{
	LOG("gs2004 write_h, offset: %04x, data: %02x\n", offset, data);
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
	LOG("tetrisfam write_h, offset: %04x, data: %02x\n", offset, data);

	u8 bank = (offset & 0x0f) << 1 | BIT(offset, 5);
	u8 mode = !BIT(offset, 4);
	prg16_89ab(bank & ~mode);
	prg16_cdef(bank | mode);

	set_nt_mirroring(BIT(offset, 7) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 3D-BLOCK board (UNIF 3D-BLOCK, NES 2.0 mapper 355)

 Games: 3-D Block (Hwang Shinwei), Block Force (the original version)

 NROM-256 with 8K of CHR-RAM and a PIC16C54 microcontroller with its own program ROM, which watches CPU A4-A14 on
 the cartridge connector and raises the cartridge /IRQ from what it sees (NESdev Wiki, NES 2.0 Mapper 355).
 The PIC runs at one fourth of the M2 rate, its 1024 byte program ROM is the NES 2.0 miscellaneous ROM of the image
 (the iNES loader reads it), and its ports are wired as:
   RA0 /IRQ (output)   RA1 A6   RA2 A5   RA3 A4
   RB0 A12   RB1 A7   RB2 A10   RB3 A11   RB4 A9   RB5 A8   RB6 A13   RB7 A14
 The CPU data bus is not connected to it, so what the games write to $4xxx only matters through its address.

 The PIC is given the address of the latest CPU cycle, which the M2 callback provides, and the scheduler switches
 between the CPU and the PIC every few microseconds, so that the PIC sees the address bus close to the moment it
 samples it.

 The images made without the PIC's program (the GoodNES dumps) leave the PIC with nothing to run.

 -------------------------------------------------*/

ROM_START( 3dblock )
	ROM_REGION16_LE( 0x400, "pic", ROMREGION_ERASE00 )
ROM_END

const tiny_rom_entry *nes_3dblock_device::device_rom_region() const
{
	return ROM_NAME( 3dblock );
}

void nes_3dblock_device::device_add_mconfig(machine_config &config)
{
	// the PIC16C54 takes four clocks for an instruction cycle, so the M2 rate as its clock gives one fourth of it
	PIC16C54(config, m_pic, DERIVED_CLOCK(1, 1));
	m_pic->read_a().set(FUNC(nes_3dblock_device::pic_porta_r));
	m_pic->write_a().set(FUNC(nes_3dblock_device::pic_porta_w));
	m_pic->read_b().set(FUNC(nes_3dblock_device::pic_portb_r));

	config.set_maximum_quantum(attotime::from_usec(5));
}

void nes_3dblock_device::device_start()
{
	common_start();
	save_item(NAME(m_addr));
	save_item(NAME(m_irq));
}

void nes_3dblock_device::pcb_reset()
{
	nes_nrom_device::pcb_reset();

	m_addr = 0;
	m_irq = false;

	// the PIC's program ROM is the miscellaneous ROM of the image
	u8 *const rom = memregion("pic")->base();
	std::fill_n(rom, 0x400, 0);
	if (m_misc_rom)
		std::copy_n(m_misc_rom, std::min<u32>(m_misc_rom_size, 0x400), rom);
	else
		logerror("3D-BLOCK: the image has no PIC16C54 program ROM, the PIC has nothing to run\n");
}

void nes_3dblock_device::m2_w(int state, offs_t address, bool write)
{
	m_addr = address;
}

u8 nes_3dblock_device::pic_porta_r()
{
	return (m_irq ? 0 : 1) | (BIT(m_addr, 6) << 1) | (BIT(m_addr, 5) << 2) | (BIT(m_addr, 4) << 3);
}

u8 nes_3dblock_device::pic_portb_r()
{
	return BIT(m_addr, 12) | (BIT(m_addr, 7) << 1) | (BIT(m_addr, 10) << 2) | (BIT(m_addr, 11) << 3)
			| (BIT(m_addr, 9) << 4) | (BIT(m_addr, 8) << 5) | (BIT(m_addr, 13) << 6) | (BIT(m_addr, 14) << 7);
}

void nes_3dblock_device::pic_porta_w(offs_t offset, u8 data, u8 mem_mask)
{
	// /IRQ is low while the PIC drives RA0 low; as an input or high the pin is released
	const bool irq = BIT(mem_mask, 0) && !BIT(data, 0);
	if (irq != m_irq)
	{
		m_irq = irq;
		set_irq_line(irq ? ASSERT_LINE : CLEAR_LINE);
	}
}
