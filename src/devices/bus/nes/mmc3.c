// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Nintendo MMC-3 & MMC-6 PCBs


 Here we emulate the following PCBs

 * Nintendo TxROM, aka MMC-3 [mapper 4]
 * Nintendo HKROM, aka MMC-6 [mapper 4]
 * Nintendo TxSROM [mapper 118]
 * Nintendo TQROM [mapper 119]
 * Nintendo NES-QJ [mapper 47]
 * Nintendo PAL-ZZ [mapper 37]


 Known issues on specific mappers:

 * 004 Mendel Palace has never worked properly
 * 004 Ninja Gaiden 2 has flashing bg graphics in the second level
 * 119 Pin Bot has glitches when the ball is in the upper half of the screen

 ***********************************************************************************************************/


#include "emu.h"
#include "mmc3.h"

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

const device_type NES_TXROM = &device_creator<nes_txrom_device>;
const device_type NES_HKROM = &device_creator<nes_hkrom_device>;
const device_type NES_TXSROM = &device_creator<nes_txsrom_device>;
const device_type NES_TQROM = &device_creator<nes_tqrom_device>;
const device_type NES_QJ_PCB = &device_creator<nes_qj_device>;
const device_type NES_ZZ_PCB = &device_creator<nes_zz_device>;


nes_txrom_device::nes_txrom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: nes_nrom_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

nes_txrom_device::nes_txrom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_TXROM, "NES Cart TxROM (MMC-3) PCB", tag, owner, clock, "nes_txrom", __FILE__)
{
}

nes_hkrom_device::nes_hkrom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_HKROM, "NES Cart HKROM (MMC-6) PCB", tag, owner, clock, "nes_hkrom", __FILE__)
{
}

nes_txsrom_device::nes_txsrom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_TXSROM, "NES Cart TxSROM PCB", tag, owner, clock, "nes_txsrom", __FILE__)
{
}

nes_tqrom_device::nes_tqrom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_TQROM, "NES Cart TQROM PCB", tag, owner, clock, "nes_tqrom", __FILE__)
{
}

nes_qj_device::nes_qj_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_QJ_PCB, "NES Cart NES-QJ PCB", tag, owner, clock, "nes_qj", __FILE__)
{
}

nes_zz_device::nes_zz_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_txrom_device(mconfig, NES_ZZ_PCB, "NES Cart PAL-ZZ PCB", tag, owner, clock, "nes_zz", __FILE__)
{
}



void nes_txrom_device::mmc3_start()
{
	common_start();
	save_item(NAME(m_mmc_prg_bank));
	save_item(NAME(m_mmc_vrom_bank));
	save_item(NAME(m_latch));
	save_item(NAME(m_wram_protect));
	save_item(NAME(m_prg_base));
	save_item(NAME(m_prg_mask));
	save_item(NAME(m_chr_base));
	save_item(NAME(m_chr_mask));
	save_item(NAME(m_alt_irq));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_count_latch));
	save_item(NAME(m_irq_clear));
}

void nes_txrom_device::mmc3_common_initialize( int prg_mask, int chr_mask, int irq_type )
{
	m_mmc_prg_bank[0] = m_mmc_prg_bank[2] = 0xffe; // m_mmc_prg_bank[2] & m_mmc_prg_bank[3] remain always the same in most MMC3 variants
	m_mmc_prg_bank[1] = m_mmc_prg_bank[3] = 0xfff; // but some pirate clone mappers change them after writing certain registers
	memset(m_mmc_vrom_bank, 0, sizeof(m_mmc_vrom_bank));

	m_latch = 0;
	m_wram_protect = 0x80;

	m_prg_base = m_chr_base = 0;
	m_prg_mask = prg_mask;
	m_chr_mask = chr_mask;

	m_alt_irq = irq_type;       // later MMC3 boards seem to use MMC6-type IRQ... more investigations are in progress at NESDev...
	m_irq_enable = 0;
	m_irq_count = m_irq_count_latch = 0;
	m_irq_clear = 0;

	set_prg(m_prg_base, m_prg_mask);
	set_chr(m_chr_source, m_chr_base, m_chr_mask);
}


void nes_txrom_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	// 4-screen mirroring is taken care in pcb_start, if needed...
	mmc3_common_initialize(0xff, 0xff, 0);
}


void nes_hkrom_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_mmc6_reg));
	save_item(NAME(m_wram_enable));

	save_item(NAME(m_mmc6_ram));

	m_mapper_sram_size = 0x400;
	m_mapper_sram = m_mmc6_ram;
}

void nes_hkrom_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	mmc3_common_initialize(0xff, 0xff, 1);
	m_mmc6_reg = 0xf0;
	m_wram_enable = 0;
}


void nes_qj_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	mmc3_common_initialize(0x0f, 0x7f, 0);
}


void nes_zz_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	mmc3_common_initialize(0x07, 0x7f, 0);
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 TxROM (MMC3 based) board emulation

 iNES: mapper 4

 -------------------------------------------------*/

/* Here, IRQ counter decrements every scanline. */
void nes_txrom_device::hblank_irq(int scanline, int vblank, int blanked)
{
	if (scanline < PPU_BOTTOM_VISIBLE_SCANLINE)
	{
		int prior_count = m_irq_count;
		if ((m_irq_count == 0) || m_irq_clear)
			m_irq_count = m_irq_count_latch;
		else
			m_irq_count--;

		if (m_irq_enable && !blanked && (m_irq_count == 0) && (prior_count || m_irq_clear /*|| !m_mmc3_alt_irq*/)) // according to blargg the latter should be present as well, but it breaks Rampart and Joe & Mac US: they probably use the alt irq!
		{
			LOG_MMC(("irq fired, scanline: %d (MAME %d, beam pos: %d)\n", scanline,
						machine().first_screen()->vpos(), machine().first_screen()->hpos()));
			m_maincpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
		}
	}
	m_irq_clear = 0;
}

// base MMC3 simply calls prg8_x
void nes_txrom_device::prg_cb( int start, int bank )
{
	prg8_x(start, bank);
}

// base MMC3 simply calls chr1_x
void nes_txrom_device::chr_cb( int start, int bank, int source )
{
	chr1_x(start, bank, source);
}


void nes_txrom_device::set_prg( int prg_base, int prg_mask )
{
	UINT8 prg_flip = (m_latch & 0x40) ? 2 : 0;

	prg_cb(0, prg_base | (m_mmc_prg_bank[0 ^ prg_flip] & prg_mask));
	prg_cb(1, prg_base | (m_mmc_prg_bank[1] & prg_mask));
	prg_cb(2, prg_base | (m_mmc_prg_bank[2 ^ prg_flip] & prg_mask));
	prg_cb(3, prg_base | (m_mmc_prg_bank[3] & prg_mask));
}

void nes_txrom_device::set_chr( UINT8 chr, int chr_base, int chr_mask )
{
	UINT8 chr_page = (m_latch & 0x80) >> 5;

	chr_cb(chr_page ^ 0, chr_base | ((m_mmc_vrom_bank[0] & ~0x01) & chr_mask), chr);
	chr_cb(chr_page ^ 1, chr_base | ((m_mmc_vrom_bank[0] |  0x01) & chr_mask), chr);
	chr_cb(chr_page ^ 2, chr_base | ((m_mmc_vrom_bank[1] & ~0x01) & chr_mask), chr);
	chr_cb(chr_page ^ 3, chr_base | ((m_mmc_vrom_bank[1] |  0x01) & chr_mask), chr);
	chr_cb(chr_page ^ 4, chr_base | (m_mmc_vrom_bank[2] & chr_mask), chr);
	chr_cb(chr_page ^ 5, chr_base | (m_mmc_vrom_bank[3] & chr_mask), chr);
	chr_cb(chr_page ^ 6, chr_base | (m_mmc_vrom_bank[4] & chr_mask), chr);
	chr_cb(chr_page ^ 7, chr_base | (m_mmc_vrom_bank[5] & chr_mask), chr);
}

WRITE8_MEMBER(nes_txrom_device::txrom_write)
{
	UINT8 mmc_helper, cmd;

	LOG_MMC(("txrom_write, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6001)
	{
		case 0x0000:
			mmc_helper = m_latch ^ data;
			m_latch = data;

			// Has PRG Mode changed?
			if (mmc_helper & 0x40)
				set_prg(m_prg_base, m_prg_mask);

			// Has CHR Mode changed?
			if (mmc_helper & 0x80)
				set_chr(m_chr_source, m_chr_base, m_chr_mask);
			break;

		case 0x0001:
			cmd = m_latch & 0x07;
			switch (cmd)
			{
				case 0: case 1: // these do not need to be separated: we take care of them in set_chr!
				case 2: case 3: case 4: case 5:
					m_mmc_vrom_bank[cmd] = data;
					set_chr(m_chr_source, m_chr_base, m_chr_mask);
					break;
				case 6:
				case 7:
					m_mmc_prg_bank[cmd - 6] = data;
					set_prg(m_prg_base, m_prg_mask);
					break;
			}
			break;

		case 0x2000:
			set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;

		case 0x2001:
			m_wram_protect = data;
			break;

		case 0x4000:
			m_irq_count_latch = data;
			break;

		case 0x4001:
			m_irq_count = 0;
			break;

		case 0x6000:
			m_irq_enable = 0;
			m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
			break;

		case 0x6001:
			m_irq_enable = 1;
			break;

		default:
			logerror("txrom_write uncaught: %04x value: %02x\n", offset + 0x8000, data);
			break;
	}
}

WRITE8_MEMBER(nes_txrom_device::write_m)
{
	LOG_MMC(("txrom write_m, offset: %04x, data: %02x\n", offset, data));

	if (BIT(m_wram_protect, 7) && !BIT(m_wram_protect, 6))
	{
		if (!m_battery.empty())
			m_battery[offset & (m_battery.size() - 1)] = data;
		if (!m_prgram.empty())
			m_prgram[offset & (m_prgram.size() - 1)] = data;
	}
}

READ8_MEMBER(nes_txrom_device::read_m)
{
	LOG_MMC(("txrom read_m, offset: %04x\n", offset));

	if (BIT(m_wram_protect, 7))
	{
		if (!m_battery.empty())
			return m_battery[offset & (m_battery.size() - 1)];
		if (!m_prgram.empty())
			return m_prgram[offset & (m_prgram.size() - 1)];
	}

	return m_open_bus;   // open bus
}


/*-------------------------------------------------

 HKROM (MMC6 based) board emulation

 iNES: mapper 4

 -------------------------------------------------*/

WRITE8_MEMBER(nes_hkrom_device::write_m)
{
	UINT8 write_hi, write_lo;
	LOG_MMC(("hkrom write_m, offset: %04x, data: %02x\n", offset, data));

	if (offset < 0x1000)
		return;

	// banks can be written only if both read & write is enabled!
	write_hi = ((m_mmc6_reg & 0xc0) == 0xc0);
	write_lo = ((m_mmc6_reg & 0x30) == 0x30);

	if (BIT(offset, 9) && write_hi) // access to upper half of 1k
		m_mmc6_ram[offset & 0x3ff] = data;

	if (!BIT(offset, 9) && write_lo)    // access to lower half of 1k
		m_mmc6_ram[offset & 0x3ff] = data;
}

READ8_MEMBER(nes_hkrom_device::read_m)
{
	LOG_MMC(("hkrom read_m, offset: %04x\n", offset));

	if (offset < 0x1000)
		return m_open_bus;    // open bus

	if (!(m_mmc6_reg & 0xa0))
		return m_open_bus;    // open bus

	if (BIT(offset, 9) && BIT(m_mmc6_reg, 7))   // access to upper half of 1k when upper read is enabled
		return m_mmc6_ram[offset & 0x3ff];

	if (!BIT(offset, 9) && BIT(m_mmc6_reg, 5))  // access to lower half of 1k when lower read is enabled
		return m_mmc6_ram[offset & 0x3ff];

	// If only one bank is enabled for reading, the other reads back as zero
	return 0x00;
}

WRITE8_MEMBER(nes_hkrom_device::write_h)
{
	UINT8 mmc6_helper;
	LOG_MMC(("hkrom write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6001)
	{
		case 0x0000:
			mmc6_helper = m_latch ^ data;
			m_latch = data;

			if (!m_wram_enable && BIT(data, 5))  // if WRAM is disabled and has to be enabled, write
				m_wram_enable = BIT(data, 5);    // (once WRAM has been enabled, it cannot be disabled without resetting the game)

			// Has PRG Mode changed?
			if (BIT(mmc6_helper, 6))
				set_prg(m_prg_base, m_prg_mask);

			// Has CHR Mode changed?
			if (BIT(mmc6_helper, 7))
				set_chr(m_chr_source, m_chr_base, m_chr_mask);
			break;

		case 0x2001:
			if (m_wram_enable)
				m_mmc6_reg = data;
			break;

		case 0x4001:
			m_irq_count = 0;
			m_irq_clear = 1;
			break;

		default:
			txrom_write(space, offset, data, mem_mask);
			break;
	}
}

/*-------------------------------------------------

 TxSROM (MMC3 based) board emulation

 Games: Armadillo, Play Action Football, Pro Hockey, RPG
 Jinsei Game, Y's 3

 iNES: mapper 118

 In MESS: Supported. It also uses mmc3_irq.

 -------------------------------------------------*/

void nes_txsrom_device::set_mirror()
{
	if (m_latch & 0x80)
	{
		set_nt_page(0, CIRAM, BIT(m_mmc_vrom_bank[2],7), 1);
		set_nt_page(1, CIRAM, BIT(m_mmc_vrom_bank[3],7), 1);
		set_nt_page(2, CIRAM, BIT(m_mmc_vrom_bank[4],7), 1);
		set_nt_page(3, CIRAM, BIT(m_mmc_vrom_bank[5],7), 1);
	}
	else
	{
		set_nt_page(0, CIRAM, BIT(m_mmc_vrom_bank[0],7), 1);
		set_nt_page(1, CIRAM, BIT(m_mmc_vrom_bank[0],7), 1);
		set_nt_page(2, CIRAM, BIT(m_mmc_vrom_bank[1],7), 1);
		set_nt_page(3, CIRAM, BIT(m_mmc_vrom_bank[1],7), 1);
	}
}

void nes_txsrom_device::chr_cb( int start, int bank, int source )
{
	set_mirror();   // we could probably update only for one (e.g. the first) call, to slightly optimize the code
	chr1_x(start, bank, source);
}

WRITE8_MEMBER(nes_txsrom_device::write_h)
{
	LOG_MMC(("txsrom write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6001)
	{
		case 0x2000:
			break;

		default:
			txrom_write(space, offset, data, mem_mask);
			break;
	}
}

/*-------------------------------------------------

 TQROM (MMC3 based) board emulation

 Games: Pin Bot, High Speed

 iNES: mapper 119

 In MESS: Supported. It also uses mmc3_irq.

 -------------------------------------------------*/

void nes_tqrom_device::set_chr( UINT8 chr, int chr_base, int chr_mask )
{
	UINT8 chr_page = (m_latch & 0x80) >> 5;
	UINT8 src[6], mask[6];

	// TQROM ignores the source, base and mask set by the MMC3 and determines them based on vrom bank bits
	for (int i = 0; i < 6; i++)
	{
		src[i] = (m_mmc_vrom_bank[i] & 0x40) ? CHRRAM : CHRROM;
		mask[i] =  (m_mmc_vrom_bank[i] & 0x40) ? 0x07 : 0x3f;
	}

	chr1_x(chr_page ^ 0, ((m_mmc_vrom_bank[0] & ~0x01) & mask[0]), src[0]);
	chr1_x(chr_page ^ 1, ((m_mmc_vrom_bank[0] |  0x01) & mask[0]), src[0]);
	chr1_x(chr_page ^ 2, ((m_mmc_vrom_bank[1] & ~0x01) & mask[1]), src[1]);
	chr1_x(chr_page ^ 3, ((m_mmc_vrom_bank[1] |  0x01) & mask[1]), src[1]);
	chr1_x(chr_page ^ 4, (m_mmc_vrom_bank[2] & mask[2]), src[2]);
	chr1_x(chr_page ^ 5, (m_mmc_vrom_bank[3] & mask[3]), src[3]);
	chr1_x(chr_page ^ 6, (m_mmc_vrom_bank[4] & mask[4]), src[4]);
	chr1_x(chr_page ^ 7, (m_mmc_vrom_bank[5] & mask[5]), src[5]);
}


/*-------------------------------------------------

 NES-QJ board (MMC3 variant for US 2-in-1 Nintendo cart
 Super Spike V'Ball + Nintendo World Cup)

 iNES: mapper 47

 -------------------------------------------------*/

WRITE8_MEMBER(nes_qj_device::write_m)
{
	LOG_MMC(("qj write_m, offset: %04x, data: %02x\n", offset, data));

	m_prg_base = BIT(data, 0) << 4;
	m_prg_mask = 0x0f;
	m_chr_base = BIT(data, 0) << 7;
	m_chr_mask = 0x7f;
	set_prg(m_prg_base, m_prg_mask);
	set_chr(m_chr_source, m_chr_base, m_chr_mask);
}


/*-------------------------------------------------

 PAL-ZZ board (MMC3 variant for European 3-in-1 Nintendo cart
 Super Mario Bros. + Tetris + Nintendo World Cup)

 iNES: mapper 37

 -------------------------------------------------*/

WRITE8_MEMBER(nes_zz_device::write_m)
{
	UINT8 mmc_helper = data & 0x07;
	LOG_MMC(("zz write_m, offset: %04x, data: %02x\n", offset, data));

	m_prg_base = (BIT(mmc_helper, 2) << 4) | (((mmc_helper & 0x03) == 0x03) ? 0x08 : 0);
	m_prg_mask = (mmc_helper << 1) | 0x07;
	m_chr_base = BIT(mmc_helper, 2) << 7;
	m_chr_mask = 0x7f;
	set_prg(m_prg_base, m_prg_mask);
	set_chr(m_chr_source, m_chr_base, m_chr_mask);
}
