/***********************************************************************************************************


 NES/Famicom cartridge emulation for Nintendo MMC-5 / ExROM

 Copyright MESS Team.
 Visit http://mamedev.org for licensing and usage restrictions.


 Here we emulate the Nintendo MMC-5 / ExROM PCBs [mapper 5]


 TODO:
 - improve PPU code in order to support the two sets of registers used by MMC5

 ***********************************************************************************************************/


#include "emu.h"
#include "machine/nes_mmc5.h"

#include "cpu/m6502/m6502.h"
#include "video/ppu2c0x.h"      // this has to be included so that IRQ functions can access PPU_BOTTOM_VISIBLE_SCANLINE
#include "sound/nes_apu.h"  // temp hack to pass the additional sound regs to APU...


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type NES_EXROM = &device_creator<nes_exrom_device>;


nes_exrom_device::nes_exrom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_EXROM, "NES Cart ExROM (MMC-5) PCB", tag, owner, clock, "nes_exrom", __FILE__)					
{
}


void nes_exrom_device::device_start()
{
	common_start();
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_status));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_mult1));
	save_item(NAME(m_mult2));
	save_item(NAME(m_mmc5_scanline));
	save_item(NAME(m_vrom_page_a));
	save_item(NAME(m_vrom_page_b));
	save_item(NAME(m_floodtile));
	save_item(NAME(m_floodattr));
	save_item(NAME(m_prg_mode));
	save_item(NAME(m_chr_mode));
	save_item(NAME(m_wram_base));
	save_item(NAME(m_wram_protect_1));
	save_item(NAME(m_wram_protect_2));
	save_item(NAME(m_mmc5_last_chr_a)); // basically unused in current code
	save_item(NAME(m_high_chr));
	save_item(NAME(m_split_scr));
	save_item(NAME(m_split_ctrl));
	save_item(NAME(m_split_yst));
	save_item(NAME(m_split_bank));
	save_item(NAME(m_prg_regs));
	save_item(NAME(m_vrom_bank));

	m_exram = auto_alloc_array_clear(machine(), UINT8, 0x400);
	save_pointer(NAME(m_exram), 0x400);

	m_mapper_sram_size = 0x400;
	m_mapper_sram = m_exram;
}

void nes_exrom_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(m_prg_chunks - 2);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_irq_count = 0;
	m_irq_status = 0;
	m_irq_enable = 0;

	m_mult1 = m_mult2 = 0;
	m_mmc5_scanline = 0;
	m_vrom_page_a = m_vrom_page_b = 0;

	m_floodtile = m_floodattr = 0;

	m_prg_mode = 3;
	m_chr_mode = 0;
	m_wram_base = 0;
	m_wram_protect_1 = 0;
	m_wram_protect_2 = 0;
	m_mmc5_last_chr_a = 1;
	m_high_chr = 0;
	m_split_scr = 0;
	m_split_ctrl = 0;
	m_split_yst = 0;
	m_split_bank = 0;

	memset(m_vrom_bank, 0x3ff, ARRAY_LENGTH(m_vrom_bank));
	m_prg_regs[0] = 0xfc;
	m_prg_regs[1] = 0xfd;
	m_prg_regs[2] = 0xfe;
	m_prg_regs[3] = 0xff;
}


READ8_MEMBER(nes_exrom_device::nt_r)
{
	int page = ((offset & 0xc00) >> 10);

	if (m_nt_src[page] == MMC5FILL)
	{
		if ((offset & 0x3ff) >= 0x3c0)
			return m_floodattr;

		return m_floodtile;
	}
	return m_nt_access[page][offset & 0x3ff];
}



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 ExROM (MMC5 based) board emulation

 Games: Castlevania III, Just Breed, many Koei titles

 iNES: mapper 5

 MESS status: Mostly Unsupported

 -------------------------------------------------*/

/* MMC5 can map PRG RAM to 0x8000-0xdfff */
void nes_exrom_device::prgram_bank8_x(int start, int bank)
{
	assert(start < 4);
	assert(bank >= 0);

	bank &= (m_prgram_size / 0x2000) - 1;

	// PRG RAM is mapped after PRG ROM
	m_prg_bank[start] = m_prg_chunks + bank;
	m_prg_bank_mem[start]->set_entry(m_prg_bank[start]);
}


void nes_exrom_device::update_prg()
{
	int bank0, bank1, bank2, bank3;

	switch (m_prg_mode)
	{
		case 0: // 32k banks
			bank3 = m_prg_regs[3];
			prg32(bank3 >> 2);
			break;

		case 1: // 16k banks
			bank1 = m_prg_regs[1];
			bank3 = m_prg_regs[3];

			if (!BIT(bank1, 7)) // PRG RAM
			{
				prgram_bank8_x(0, (bank1 & 0x06));
				prgram_bank8_x(1, (bank1 & 0x06) + 1);
			}
			else
				prg16_89ab(bank1 >> 1);

			prg16_cdef(bank3);
			break;

		case 2: // 16k-8k banks
			bank1 = m_prg_regs[1];
			bank2 = m_prg_regs[2];
			bank3 = m_prg_regs[3];

			if (!BIT(bank1, 7))
			{
				prgram_bank8_x(0, (bank1 & 0x06));
				prgram_bank8_x(1, (bank1 & 0x06) + 1);
			}
			else
				prg16_89ab((bank1 & 0x7f) >> 1);

			if (!BIT(bank2, 7))
				prgram_bank8_x(2, bank2 & 0x07);
			else
				prg8_cd(bank2 & 0x7f);

			prg8_ef(bank3);
			break;

		case 3: // 8k banks
			bank0 = m_prg_regs[0];
			bank1 = m_prg_regs[1];
			bank2 = m_prg_regs[2];
			bank3 = m_prg_regs[3];

			if (!BIT(bank0, 7))
				prgram_bank8_x(0, bank0 & 0x07);
			else
				prg8_89(bank0 & 0x7f);

			if (!BIT(bank1, 7))
				prgram_bank8_x(1, bank1 & 0x07);
			else
				prg8_ab(bank1 & 0x7f);

			if (!BIT(bank2, 7))
				prgram_bank8_x(2, bank2 & 0x07);
			else
				prg8_cd(bank2 & 0x7f);

			prg8_ef(bank3);
			break;
	}
}

void nes_exrom_device::update_render_mode()
{
	// if m_exram_control is 0 or 1, m_exram must be used for NT
}

void nes_exrom_device::hblank_irq(int scanline, int vblank, int blanked )
{
#if 1
	if (scanline == 0)
		m_irq_status |= 0x40;
	else if (scanline > PPU_BOTTOM_VISIBLE_SCANLINE)
		m_irq_status &= ~0x40;
#endif

	if (scanline == m_irq_count)
	{
		if (m_irq_enable)
			machine().device("maincpu")->execute().set_input_line(M6502_IRQ_LINE, ASSERT_LINE);

		m_irq_status = 0xff;
	}

	/* FIXME: this is ok, but then we would need to update them again when we have the BG Hblank
	 I leave it commented out until the PPU is updated for this */
//  if (ppu2c0x_is_sprite_8x16(m_ppu) || m_mmc5_last_chr_a)
//      update_chr_a();
//  else
//      update_chr_b();
}

void nes_exrom_device::set_mirror(int page, int src)
{
	switch (src)
	{
		case 0: /* CIRAM0 */
			set_nt_page(page, CIRAM, 0, 1);
			break;
		case 1: /* CIRAM1 */
			set_nt_page(page, CIRAM, 1, 1);
			break;
		case 2: /* ExRAM */
			set_nt_page(page, EXRAM, 0, 1);  // actually only works during rendering.
			break;
		case 3: /* Fill Registers */
			set_nt_page(page, MMC5FILL, 0, 0);
			break;
		default:
			fatalerror("This should never happen\n");
			break;
	}
}

READ8_MEMBER(nes_exrom_device::read_l)
{
	int value;

	/* $5c00 - $5fff: extended videoram attributes */
	if ((offset >= 0x1b00) && (offset <= 0x1eff))
	{
		return m_exram[offset - 0x1b00];
	}

	switch (offset)
	{
		case 0x1104: /* $5204 */
#if 0
			if (current_scanline == m_mmc5_scanline)
				return 0x80;
			else
				return 0x00;
#else
			value = m_irq_status;
			m_irq_status &= ~0x80;
			machine().device("maincpu")->execute().set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
			return value;
#endif

		case 0x1105: /* $5205 */
			return (m_mult1 * m_mult2) & 0xff;
		case 0x1106: /* $5206 */
			return ((m_mult1 * m_mult2) & 0xff00) >> 8;

		default:
			logerror("** MMC5 uncaught read, offset: %04x\n", offset + 0x4100);
			return 0x00;
	}
}


WRITE8_MEMBER(nes_exrom_device::write_l)
{
	//  LOG_MMC(("Mapper 5 write, offset: %04x, data: %02x\n", offset + 0x4100, data));
	/* Send $5000-$5015 to the sound chip */
	if ((offset >= 0xf00) && (offset <= 0xf15))
	{
		nesapu_device *m_sound = machine().device<nesapu_device>("nessound");
		m_sound->write(space, offset & 0x1f, data);
		return;
	}

	/* $5c00 - $5fff: extended videoram attributes */
	if ((offset >= 0x1b00) && (offset <= 0x1eff))
	{
		if (m_exram_control != 0x03)
			m_exram[offset - 0x1b00] = data;
		return;
	}

	switch (offset)
	{
		case 0x1000: /* $5100 */
			m_prg_mode = data & 0x03;
			// update_prg();
			LOG_MMC(("MMC5 rom bank mode: %02x\n", data));
			break;

		case 0x1001: /* $5101 */
			m_chr_mode = data & 0x03;
			// update chr
			LOG_MMC(("MMC5 vrom bank mode: %02x\n", data));
			break;

		case 0x1002: /* $5102 */
			m_wram_protect_1 = data & 0x03;
			LOG_MMC(("MMC5 vram protect 1: %02x\n", data));
			break;
		case 0x1003: /* 5103 */
			m_wram_protect_2 = data & 0x03;
			LOG_MMC(("MMC5 vram protect 2: %02x\n", data));
			break;

		case 0x1004: /* $5104 - Extra VRAM (EXRAM) control */
			m_exram_control = data & 0x03;
			// update render
			update_render_mode();
			LOG_MMC(("MMC5 exram control: %02x\n", data));
			break;

		case 0x1005: /* $5105 */
			set_mirror(0, (data & 0x03) >> 0);
			set_mirror(1, (data & 0x0c) >> 2);
			set_mirror(2, (data & 0x30) >> 4);
			set_mirror(3, (data & 0xc0) >> 6);
			// update render
			update_render_mode();
			break;

			/* tile data for MMC5 flood-fill NT mode */
		case 0x1006:
			m_floodtile = data;
			break;

			/* attr data for MMC5 flood-fill NT mode */
		case 0x1007:
			switch (data & 3)
			{
				default:
				case 0: m_floodattr = 0x00; break;
				case 1: m_floodattr = 0x55; break;
				case 2: m_floodattr = 0xaa; break;
				case 3: m_floodattr = 0xff; break;
			}
			break;

		case 0x1013: /* $5113 */
			LOG_MMC(("MMC5 mid RAM bank select: %02x\n", data & 0x07));
			m_wram_base = data & 0x07;
			break;


		case 0x1014: /* $5114 */
		case 0x1015: /* $5115 */
		case 0x1016: /* $5116 */
		case 0x1017: /* $5117 */
			m_prg_regs[offset & 3] = data;
			update_prg();
			break;

		case 0x1020: /* $5120 */
			LOG_MMC(("MMC5 $5120 vrom select: %02x (mode: %d)\n", data, m_chr_mode));
			switch (m_chr_mode)
			{
				case 0x03:
					/* 1k switch */
					m_vrom_bank[0] = data | (m_high_chr << 8);
//                  mapper5_sync_vrom(0);
					chr1_0(m_vrom_bank[0], CHRROM);
//                  m_nes_vram_sprite[0] = m_vrom_bank[0] * 64;
//                  vrom_next[0] = 4;
//                  vrom_page_a = 1;
//                  vrom_page_b = 0;
				break;
			}
			break;
		case 0x1021: /* $5121 */
			LOG_MMC(("MMC5 $5121 vrom select: %02x (mode: %d)\n", data, m_chr_mode));
			switch (m_chr_mode)
			{
				case 0x02:
					/* 2k switch */
					chr2_0(data | (m_high_chr << 8), CHRROM);
					break;
				case 0x03:
					/* 1k switch */
					m_vrom_bank[1] = data | (m_high_chr << 8);
//                  mapper5_sync_vrom(0);
					chr1_1(m_vrom_bank[1], CHRROM);
//                  m_nes_vram_sprite[1] = m_vrom_bank[0] * 64;
//                  vrom_next[1] = 5;
//                  vrom_page_a = 1;
//                  vrom_page_b = 0;
					break;
			}
			break;
		case 0x1022: /* $5122 */
			LOG_MMC(("MMC5 $5122 vrom select: %02x (mode: %d)\n", data, m_chr_mode));
			switch (m_chr_mode)
			{
				case 0x03:
					/* 1k switch */
					m_vrom_bank[2] = data | (m_high_chr << 8);
//                  mapper5_sync_vrom(0);
					chr1_2(m_vrom_bank[2], CHRROM);
//                  m_nes_vram_sprite[2] = m_vrom_bank[0] * 64;
//                  vrom_next[2] = 6;
//                  vrom_page_a = 1;
//                  vrom_page_b = 0;
					break;
			}
			break;
		case 0x1023: /* $5123 */
			LOG_MMC(("MMC5 $5123 vrom select: %02x (mode: %d)\n", data, m_chr_mode));
			switch (m_chr_mode)
			{
				case 0x01:
					chr4_0(data, CHRROM);
					break;
				case 0x02:
					/* 2k switch */
					chr2_2(data | (m_high_chr << 8), CHRROM);
					break;
				case 0x03:
					/* 1k switch */
					m_vrom_bank[3] = data | (m_high_chr << 8);
//                  mapper5_sync_vrom(0);
					chr1_3(m_vrom_bank[3], CHRROM);
//                  m_nes_vram_sprite[3] = m_vrom_bank[0] * 64;
//                  vrom_next[3] = 7;
//                  vrom_page_a = 1;
//                  vrom_page_b = 0;
					break;
			}
			break;
		case 0x1024: /* $5124 */
			LOG_MMC(("MMC5 $5124 vrom select: %02x (mode: %d)\n", data, m_chr_mode));
			switch (m_chr_mode)
			{
				case 0x03:
					/* 1k switch */
					m_vrom_bank[4] = data | (m_high_chr << 8);
//                  mapper5_sync_vrom(0);
					chr1_4(m_vrom_bank[4], CHRROM);
//                  m_nes_vram_sprite[4] = m_vrom_bank[0] * 64;
//                  vrom_next[0] = 0;
//                  vrom_page_a = 0;
//                  vrom_page_b = 0;
					break;
			}
			break;
		case 0x1025: /* $5125 */
			LOG_MMC(("MMC5 $5125 vrom select: %02x (mode: %d)\n", data, m_chr_mode));
			switch (m_chr_mode)
			{
				case 0x02:
					/* 2k switch */
					chr2_4(data | (m_high_chr << 8), CHRROM);
					break;
				case 0x03:
					/* 1k switch */
					m_vrom_bank[5] = data | (m_high_chr << 8);
//                  mapper5_sync_vrom(0);
					chr1_5(m_vrom_bank[5], CHRROM);
//                  m_nes_vram_sprite[5] = m_vrom_bank[0] * 64;
//                  vrom_next[1] = 1;
//                  vrom_page_a = 0;
//                  vrom_page_b = 0;
					break;
			}
			break;
		case 0x1026: /* $5126 */
			LOG_MMC(("MMC5 $5126 vrom select: %02x (mode: %d)\n", data, m_chr_mode));
			switch (m_chr_mode)
			{
				case 0x03:
					/* 1k switch */
					m_vrom_bank[6] = data | (m_high_chr << 8);
//                  mapper5_sync_vrom(0);
					chr1_6(m_vrom_bank[6], CHRROM);
//                  m_nes_vram_sprite[6] = m_vrom_bank[0] * 64;
//                  vrom_next[2] = 2;
//                  vrom_page_a = 0;
//                  vrom_page_b = 0;
					break;
			}
			break;
		case 0x1027: /* $5127 */
			LOG_MMC(("MMC5 $5127 vrom select: %02x (mode: %d)\n", data, m_chr_mode));
			switch (m_chr_mode)
			{
				case 0x00:
					/* 8k switch */
					chr8(data, CHRROM);
					break;
				case 0x01:
					/* 4k switch */
					chr4_4(data, CHRROM);
					break;
				case 0x02:
					/* 2k switch */
					chr2_6(data | (m_high_chr << 8), CHRROM);
					break;
				case 0x03:
					/* 1k switch */
					m_vrom_bank[7] = data | (m_high_chr << 8);
//                  mapper5_sync_vrom(0);
					chr1_7(m_vrom_bank[7], CHRROM);
//                  m_nes_vram_sprite[7] = m_vrom_bank[0] * 64;
//                  vrom_next[3] = 3;
//                  vrom_page_a = 0;
//                  vrom_page_b = 0;
					break;
			}
			break;
			case 0x1028: /* $5128 */
				LOG_MMC(("MMC5 $5128 vrom select: %02x (mode: %d)\n", data, m_chr_mode));
				switch (m_chr_mode)
				{
				case 0x03:
					/* 1k switch */
					m_vrom_bank[8] = data | (m_high_chr << 8);
//                  nes_vram[vrom_next[0]] = data * 64;
//                  nes_vram[0 + (vrom_page_a*4)] = data * 64;
//                  nes_vram[0] = data * 64;
					chr1_4(m_vrom_bank[8], CHRROM);
//                  mapper5_sync_vrom(1);
					if (!m_vrom_page_b)
					{
						m_vrom_page_a ^= 0x01;
						m_vrom_page_b = 1;
					}
					break;
			}
			break;
		case 0x1029: /* $5129 */
			LOG_MMC(("MMC5 $5129 vrom select: %02x (mode: %d)\n", data, m_chr_mode));
			switch (m_chr_mode)
			{
				case 0x02:
					/* 2k switch */
					chr2_0(data | (m_high_chr << 8), CHRROM);
					chr2_4(data | (m_high_chr << 8), CHRROM);
					break;
				case 0x03:
					/* 1k switch */
					m_vrom_bank[9] = data | (m_high_chr << 8);
//                  nes_vram[vrom_next[1]] = data * 64;
//                  nes_vram[1 + (vrom_page_a*4)] = data * 64;
//                  nes_vram[1] = data * 64;
					chr1_5(m_vrom_bank[9], CHRROM);
//                  mapper5_sync_vrom(1);
					if (!m_vrom_page_b)
					{
						m_vrom_page_a ^= 0x01;
						m_vrom_page_b = 1;
					}
					break;
			}
			break;
		case 0x102a: /* $512a */
			LOG_MMC(("MMC5 $512a vrom select: %02x (mode: %d)\n", data, m_chr_mode));
			switch (m_chr_mode)
			{
				case 0x03:
					/* 1k switch */
					m_vrom_bank[10] = data | (m_high_chr << 8);
//                  nes_vram[vrom_next[2]] = data * 64;
//                  nes_vram[2 + (vrom_page_a*4)] = data * 64;
//                  nes_vram[2] = data * 64;
					chr1_6(m_vrom_bank[10], CHRROM);
//                  mapper5_sync_vrom(1);
					if (!m_vrom_page_b)
					{
						m_vrom_page_a ^= 0x01;
						m_vrom_page_b = 1;
					}
					break;
			}
			break;
		case 0x102b: /* $512b */
			LOG_MMC(("MMC5 $512b vrom select: %02x (mode: %d)\n", data, m_chr_mode));
			switch (m_chr_mode)
			{
				case 0x00:
					/* 8k switch */
					/* switches in first half of an 8K bank!) */
					chr4_0(data << 1, CHRROM);
					chr4_4(data << 1, CHRROM);
					break;
				case 0x01:
					/* 4k switch */
					chr4_0(data, CHRROM);
					chr4_4(data, CHRROM);
					break;
				case 0x02:
					/* 2k switch */
					chr2_2(data | (m_high_chr << 8), CHRROM);
					chr2_6(data | (m_high_chr << 8), CHRROM);
					break;
				case 0x03:
					/* 1k switch */
					m_vrom_bank[11] = data | (m_high_chr << 8);
//                  nes_vram[vrom_next[3]] = data * 64;
//                  nes_vram[3 + (vrom_page_a*4)] = data * 64;
//                  nes_vram[3] = data * 64;
					chr1_7(m_vrom_bank[11], CHRROM);
//                  mapper5_sync_vrom(1);
					if (!m_vrom_page_b)
					{
						m_vrom_page_a ^= 0x01;
						m_vrom_page_b = 1;
					}
					break;
			}
			break;

		case 0x1030: /* $5130 */
			m_high_chr = data & 0x03;
			if (m_exram_control == 1)
			{
// in this case m_high_chr selects which 256KB of CHR ROM
// is to be used for all background tiles on the screen.
			}
			break;


		case 0x1100: /* $5200 */
			m_split_scr = data;
			// in EX2 and EX3 modes, no split screen
			if (m_exram_control & 0x02)
				m_split_scr &= 0x7f;
			m_split_ctrl = data;
			break;

		case 0x1101: /* $5201 */
			m_split_yst = (data >= 240) ? data - 16 : data;
			break;

		case 0x1102: /* $5202 */
			m_split_bank = data;
			break;

		case 0x1103: /* $5203 */
			m_irq_count = data;
			m_mmc5_scanline = data;
			LOG_MMC(("MMC5 irq scanline: %d\n", m_irq_count));
			break;
		case 0x1104: /* $5204 */
			m_irq_enable = data & 0x80;
			LOG_MMC(("MMC5 irq enable: %02x\n", data));
			break;
		case 0x1105: /* $5205 */
			m_mult1 = data;
			break;
		case 0x1106: /* $5206 */
			m_mult2 = data;
			break;

		default:
			logerror("** MMC5 uncaught write, offset: %04x, data: %02x\n", offset + 0x4100, data);
			break;
	}
}

// 3bits are used to access the "WRAM" banks
// bit3 select the chip (2 of them can be accessed, each up to 32KB)
// bit1 & bit2 select the 8KB banks inside the chip
// same mechanism is used also when "WRAM" is mapped in higher banks, but there we setup the bank map in a smart
// way so to access the correct bank as if the 3 bits were directly selecting the bank in a 64KB RAM area
READ8_MEMBER(nes_exrom_device::read_m)
{
	if (m_battery && m_prgram)  // 2 chips present: first is BWRAM, second is WRAM
	{
		if (m_wram_base & 0x04)
			return m_prgram[(offset + (m_wram_base & 0x03) * 0x2000) & (m_prgram_size - 1)];
		else
			return m_battery[(offset + (m_wram_base & 0x03) * 0x2000) & (m_battery_size - 1)];
	}
	else if (m_prgram)  // 1 chip, WRAM
		return m_prgram[(offset + (m_wram_base & 0x03) * 0x2000) & (m_prgram_size - 1)];
	else if (m_battery) // 1 chip, BWRAM
		return m_battery[(offset + (m_wram_base & 0x03) * 0x2000) & (m_battery_size - 1)];
	else
		return  ((offset + 0x6000) & 0xff00) >> 8;
}

WRITE8_MEMBER(nes_exrom_device::write_m)
{
	if (m_wram_protect_1 != 0x02 || m_wram_protect_2 != 0x01)
		return;

	if (m_battery)
		m_battery[offset & (m_battery_size - 1)] = data;
	if (m_prgram)
		m_prgram[offset & (m_prgram_size - 1)] = data;
}
