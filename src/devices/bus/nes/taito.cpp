// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Taito PCBs


 Here we emulate the following PCBs

 * Taito TC0190FMC/TC0190FMR [mapper 33]
 * Taito TC0190FMC+PAL16R4 [mapper 48]
 * Taito X1-005 [mapper 80, 207]
 * Taito X1-017 [mapper 82]

 TODO:
 - Akira does not work. Investigate why!
 - Don Doko Don 2 freezes when you get to the first boss


 ***********************************************************************************************************/


#include "emu.h"
#include "taito.h"

#include "video/ppu2c0x.h"      // this has to be included so that IRQ functions can access ppu2c0x_device::BOTTOM_VISIBLE_SCANLINE


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_TC0190FMC,         nes_tc0190fmc_device,         "nes_tc0190fmc", "NES Cart Taito TC0190FMC PCB")
DEFINE_DEVICE_TYPE(NES_TC0190FMC_PAL16R4, nes_tc0190fmc_pal16r4_device, "nes_tc0190pal", "NES Cart Taito TC0190FMC + PAL16R4 PCB")
DEFINE_DEVICE_TYPE(NES_X1_005,            nes_x1_005_device,            "nes_x1_005",    "NES Cart Taito X1-005 PCB")
DEFINE_DEVICE_TYPE(NES_X1_017,            nes_x1_017_device,            "nes_x1_017",    "NES Cart Taito X1-017 PCB")


nes_tc0190fmc_device::nes_tc0190fmc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, type, tag, owner, clock)
{
}

nes_tc0190fmc_device::nes_tc0190fmc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_tc0190fmc_device(mconfig, NES_TC0190FMC, tag, owner, clock)
{
}

nes_tc0190fmc_pal16r4_device::nes_tc0190fmc_pal16r4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_tc0190fmc_device(mconfig, NES_TC0190FMC_PAL16R4, tag, owner, clock), m_irq_count(0), m_irq_count_latch(0), m_irq_enable(0)
{
}

nes_x1_005_device::nes_x1_005_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_X1_005, tag, owner, clock), m_latch(0)
{
}

nes_x1_017_device::nes_x1_017_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_X1_017, tag, owner, clock), m_latch(0), m_irq_count(0), m_irq_count_latch(0), m_irq_enable(0), irq_timer(nullptr)
{
}



void nes_tc0190fmc_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);
}

void nes_tc0190fmc_pal16r4_device::device_start()
{
	common_start();
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_count_latch));
}

void nes_tc0190fmc_pal16r4_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_irq_enable = 0;
	m_irq_count = 0;
	m_irq_count_latch = 0;
}

void nes_x1_005_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));

	save_item(NAME(m_x1_005_ram));

	m_mapper_sram_size = 0x80;
	m_mapper_sram = m_x1_005_ram;
}

void nes_x1_005_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_latch = 0;
}

void nes_x1_017_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->adjust(attotime::zero, 0, clocks_to_attotime(1));

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_count_latch));

	save_item(NAME(m_latch));
	save_item(NAME(m_reg));
	save_item(NAME(m_mmc_vrom_bank));

	save_item(NAME(m_x1_017_ram));

	m_mapper_sram_size = 0x1400;
	m_mapper_sram = m_x1_017_ram;
}

void nes_x1_017_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_irq_enable = 0;
	m_irq_count = 0;
	m_irq_count_latch = 0;

	m_latch = 0;
	m_reg[0] = m_reg[1] = m_reg[2] = 0;
	std::fill(std::begin(m_mmc_vrom_bank), std::end(m_mmc_vrom_bank), 0x00);
}



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Taito TC0190FMC board emulation

 Games: Akira, Bakushou!! Jinsei Gekijou, Don Doko Don,
 Insector X, Operation Wolf, Power Blazer, Takeshi no
 Sengoku Fuuunji

 iNES: mapper 33

 In MESS: Supported.

 -------------------------------------------------*/

void nes_tc0190fmc_device::tc0190fmc_write(offs_t offset, uint8_t data)
{
	LOG_MMC(("tc0190fmc_write, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7003)
	{
		case 0x0000:
			set_nt_mirroring(BIT(data, 6) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			prg8_89(data);
			break;
		case 0x0001:
			prg8_ab(data);
			break;
		case 0x0002:
			chr2_0(data, CHRROM);
			break;
		case 0x0003:
			chr2_2(data, CHRROM);
			break;
		case 0x2000:
			chr1_4(data, CHRROM);
			break;
		case 0x2001:
			chr1_5(data, CHRROM);
			break;
		case 0x2002:
			chr1_6(data, CHRROM);
			break;
		case 0x2003:
			chr1_7(data, CHRROM);
			break;
	}
}

/*-------------------------------------------------

 Taito TC0190FMC + PAL16R4 board emulation

 Games: Bakushou!! Jinsei Gekijou 3, Bubble Bobble 2,
 Captain Saver, Don Doko Don 2, Flintstones, Jetsons

 This is basically Mapper 33 + IRQ. Notably, IRQ works the
 same as MMC3 irq, BUT latch values are "inverted" (XOR'ed
 with 0xff) and there is a little delay (not implemented yet)
 We simply use MMC3 IRQ and XOR the value written in the
 register 0xc000 below

 iNES: mapper 48

 In MESS: Supported.

 -------------------------------------------------*/

void nes_tc0190fmc_pal16r4_device::hblank_irq( int scanline, int vblank, int blanked )
{
	if (scanline < ppu2c0x_device::BOTTOM_VISIBLE_SCANLINE)
	{
		int prior_count = m_irq_count;
		if (m_irq_count == 0)
			m_irq_count = m_irq_count_latch;
		else
			m_irq_count--;

		if (m_irq_enable && !blanked && (m_irq_count == 0) && prior_count)
			set_irq_line(ASSERT_LINE);
	}
}

void nes_tc0190fmc_pal16r4_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("tc0190fmc pal16r4 write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7003)
	{
		case 0x0000:
			prg8_89(data);
			break;
		case 0x0001:
		case 0x0002:
		case 0x0003:
		case 0x2000:
		case 0x2001:
		case 0x2002:
		case 0x2003:
			tc0190fmc_write(offset, data);
			break;
		case 0x4000:
			m_irq_count_latch = (0x100 - data) & 0xff;
			break;
		case 0x4001:
			m_irq_count = m_irq_count_latch;
			break;
		case 0x4002:
			m_irq_enable = 1;
			break;
		case 0x4003:
			m_irq_enable = 0;
			set_irq_line(CLEAR_LINE);
			break;
		case 0x6000:
			set_nt_mirroring(BIT(data, 6) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
	}
}

/*-------------------------------------------------

 Taito X1-005 board emulation

 Actually, Fudou Myouou Den uses a variant of the board with
 CIRAM, making necessary two distinct mappers & pcb_id.

 iNES: mappers 80 & 207

 -------------------------------------------------*/

void nes_x1_005_device::write_m(offs_t offset, uint8_t data)
{
	LOG_MMC(("x1_005 write_m, offset: %04x, data: %02x\n", offset, data));

	switch (offset)
	{
		case 0x1ef0:
			chr2_0(BIT(data, 1, 6), CHRROM);
			if (m_x1_005_alt_mirroring)
			{
				set_nt_page(0, CIRAM, BIT(data, 7), 1);
				set_nt_page(1, CIRAM, BIT(data, 7), 1);
			}
			break;
		case 0x1ef1:
			chr2_2(BIT(data, 1, 6), CHRROM);
			if (m_x1_005_alt_mirroring)
			{
				set_nt_page(2, CIRAM, BIT(data, 7), 1);
				set_nt_page(3, CIRAM, BIT(data, 7), 1);
			}
			break;
		case 0x1ef2:
			chr1_4(data, CHRROM);
			break;
		case 0x1ef3:
			chr1_5(data, CHRROM);
			break;
		case 0x1ef4:
			chr1_6(data, CHRROM);
			break;
		case 0x1ef5:
			chr1_7(data, CHRROM);
			break;
		case 0x1ef6:
		case 0x1ef7:
			if (!m_x1_005_alt_mirroring)
				set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
			break;
		case 0x1ef8:
		case 0x1ef9:
			m_latch = data;
			break;
		case 0x1efa:
		case 0x1efb:
			prg8_89(data);
			break;
		case 0x1efc:
		case 0x1efd:
			prg8_ab(data);
			break;
		case 0x1efe:
		case 0x1eff:
			prg8_cd(data);
			break;
		default:
			logerror("mapper80_m_w uncaught addr: %04x, value: %02x\n", offset + 0x6000, data);
			break;
	}

	if (offset >= 0x1f00 && m_latch == 0xa3)
		m_x1_005_ram[offset & 0x7f] = data;
}

uint8_t nes_x1_005_device::read_m(offs_t offset)
{
	LOG_MMC(("x1_005 read_m, offset: %04x\n", offset));

	if (offset >= 0x1f00 && m_latch == 0xa3)
		return m_x1_005_ram[offset & 0x7f];

	return get_open_bus();
}

/*-------------------------------------------------

 Taito X1-017 board emulation

 Games: Kyuukyoku Harikiri Koushien, Kyuukyoku Harikiri
 Stadium 3, Kyuukyoku Harikiri - Heisei Gannenban,
 SD Keiji - Blader

 NES 2.0: mapper 552 (old mapper 82 are mis-ordered bad dumps)

 In MAME: Supported.

 TODO: KH Koushien seems to be working except it needs to
 be reset once with a new NVRAM file. KH Heisei won't
 load "Single Game" menu option but other game modes work.

 -------------------------------------------------*/

void nes_x1_017_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	if (id == TIMER_IRQ)
	{
		if ((m_irq_enable & 0x05) == 1 && m_irq_count) // counting enabled?
			m_irq_count--;
		if (!m_irq_count && BIT(m_irq_enable, 1))
			set_irq_line(ASSERT_LINE);
	}
}

void nes_x1_017_device::set_chr()
{
	chr2_x(0 ^ m_latch, m_mmc_vrom_bank[0] >> 1, CHRROM);
	chr2_x(2 ^ m_latch, m_mmc_vrom_bank[1] >> 1, CHRROM);
	chr1_x(4 ^ m_latch, m_mmc_vrom_bank[2], CHRROM);
	chr1_x(5 ^ m_latch, m_mmc_vrom_bank[3], CHRROM);
	chr1_x(6 ^ m_latch, m_mmc_vrom_bank[4], CHRROM);
	chr1_x(7 ^ m_latch, m_mmc_vrom_bank[5], CHRROM);
}

void nes_x1_017_device::write_m(offs_t offset, u8 data)
{
	LOG_MMC(("x1017 write_m, offset: %04x, data: %02x\n", offset, data));

	switch (offset)
	{
		case 0x1ef0:
		case 0x1ef1:
		case 0x1ef2:
		case 0x1ef3:
		case 0x1ef4:
		case 0x1ef5:
			m_mmc_vrom_bank[offset & 0x07] = data;
			set_chr();
			break;
		case 0x1ef6:
			set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
			m_latch = (data & 0x02) << 1;
			set_chr();
			break;
		case 0x1ef7:
		case 0x1ef8:
		case 0x1ef9:
			m_reg[(offset & 0x0f) - 7] = data;
			break;
		case 0x1efa:
		case 0x1efb:
		case 0x1efc:
			prg8_x((offset & 0x0f) - 0xa, bitswap<6>(data, 0, 1, 2, 3, 4, 5));
			break;
		case 0x1efd:
			m_irq_count_latch = data;
			break;
		case 0x1efe:
			m_irq_enable = data;
			if (!BIT(m_irq_enable, 0))
				m_irq_count = m_irq_count_latch ? (m_irq_count_latch + 2) * 16 : 17;
			if (!BIT(m_irq_enable, 1))
				set_irq_line(CLEAR_LINE);
			break;
		case 0x1eff:
			set_irq_line(CLEAR_LINE);
			m_irq_count = m_irq_count_latch ? (m_irq_count_latch + 1) * 16 : 1;
			break;
	}

	// 2+2+1 KB of Internal RAM can be independently enabled/disabled!
	if ((offset < 0x0800 && m_reg[0] == 0xca) ||
	    (offset < 0x1000 && m_reg[1] == 0x69) ||
	    (offset < 0x1400 && m_reg[2] == 0x84))
	{
		m_x1_017_ram[offset] = data;

		// 1st byte in each 1K page latches most recent read/write from that page
		m_x1_017_ram[offset & 0x1c00] = data;
	}
}

u8 nes_x1_017_device::read_m(offs_t offset)
{
	LOG_MMC(("x1017 read_m, offset: %04x\n", offset));

	// 2+2+1 KB of Internal RAM can be independently enabled/disabled!
	if ((offset < 0x0800 && m_reg[0] == 0xca) ||
	    (offset < 0x1000 && m_reg[1] == 0x69) ||
	    (offset < 0x1400 && m_reg[2] == 0x84))
	{
		u8 ret = m_x1_017_ram[offset];

		// 1st byte in each 1K page latches most recent read/write from that page
		m_x1_017_ram[offset & 0x1c00] = ret;

		return ret;
	}

	return 0; // no open bus behavior due to pull-down resistors
}
