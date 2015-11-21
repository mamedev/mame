// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Nintendo MMC-5 / ExROM


 Here we emulate the Nintendo MMC-5 / ExROM PCBs [mapper 5]


 TODO:
 - improve PPU code in order to support the two sets of registers used by MMC5

 ***********************************************************************************************************/


#include "emu.h"
#include "mmc5.h"

#include "cpu/m6502/m6502.h"
#include "video/ppu2c0x.h"      // this has to be included so that IRQ functions can access PPU_BOTTOM_VISIBLE_SCANLINE
#include "sound/nes_apu.h"  // temp hack to pass the additional sound regs to APU...


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


#define LAST_CHR_REG_A 0
#define LAST_CHR_REG_B 1

static const int m_mmc5_attrib[4] = {0x00, 0x55, 0xaa, 0xff};

//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type NES_EXROM = &device_creator<nes_exrom_device>;


nes_exrom_device::nes_exrom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_EXROM, "NES Cart ExROM (MMC-5) PCB", tag, owner, clock, "nes_exrom", __FILE__), m_irq_count(0), 
	m_irq_status(0), m_irq_enable(0), m_mult1(0), m_mult2(0), m_mmc5_scanline(0), m_vrom_page_a(0), m_vrom_page_b(0), m_floodtile(0), m_floodattr(0),
	m_prg_mode(0), m_chr_mode(0), m_wram_protect_1(0), m_wram_protect_2(0), m_exram_control(0), m_wram_base(0), m_last_chr(0), m_ex1_chr(0), 
	m_split_chr(0), m_ex1_bank(0), m_high_chr(0), m_split_scr(0), m_split_rev(0), m_split_ctrl(0), m_split_yst(0), m_split_bank(0), m_vcount(0)
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
	save_item(NAME(m_vrom_page_a));
	save_item(NAME(m_vrom_page_b));
	save_item(NAME(m_floodtile));
	save_item(NAME(m_floodattr));
	save_item(NAME(m_prg_mode));
	save_item(NAME(m_chr_mode));
	save_item(NAME(m_wram_base));
	save_item(NAME(m_wram_protect_1));
	save_item(NAME(m_wram_protect_2));
	save_item(NAME(m_vrom_bank));
	save_item(NAME(m_last_chr));
	save_item(NAME(m_ex1_chr));
	save_item(NAME(m_split_chr));
	save_item(NAME(m_prg_regs));
	save_item(NAME(m_prg_ram_mapped));
	save_item(NAME(m_ex1_bank));
	save_item(NAME(m_high_chr));
	save_item(NAME(m_split_scr));
	save_item(NAME(m_split_rev));
	save_item(NAME(m_split_ctrl));
	save_item(NAME(m_split_yst));
	save_item(NAME(m_split_bank));
	save_item(NAME(m_vcount));
	save_item(NAME(m_exram));
	save_item(NAME(m_ram_hi_banks));

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
	m_vrom_page_a = m_vrom_page_b = 0;

	m_floodtile = m_floodattr = 0;

	m_prg_mode = 3;
	m_chr_mode = 0;
	m_wram_base = 0;
	m_wram_protect_1 = 0;
	m_wram_protect_2 = 0;
	m_high_chr = 0;
	m_split_scr = 0;
	m_split_rev = 0;
	m_split_ctrl = 0;
	m_split_yst = 0;
	m_split_bank = 0;
	m_last_chr = LAST_CHR_REG_A;
	m_ex1_chr = 0;
	m_split_chr = 0;
	m_ex1_bank = 0;
	m_vcount = 0;

	for (int i = 0; i < 12; i++)
		m_vrom_bank[i] = 0x3ff;

	m_prg_regs[0] = 0xfc;
	m_prg_regs[1] = 0xfd;
	m_prg_regs[2] = 0xfe;
	m_prg_regs[3] = 0xff;
	m_prg_ram_mapped[0] = 0;
	m_prg_ram_mapped[1] = 0;
	m_prg_ram_mapped[2] = 0;
	m_prg_ram_mapped[3] = 0;

	m_ram_hi_banks[0] = 0;
	m_ram_hi_banks[1] = 0;
	m_ram_hi_banks[2] = 0;
	m_ram_hi_banks[3] = 0;
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 ExROM (MMC5 based) board emulation

 Games: Castlevania III, Just Breed, many Koei titles

 iNES: mapper 5

 MESS status: Partially supported

 -------------------------------------------------*/

void nes_exrom_device::update_prg()
{
	int bank0, bank1, bank2, bank3;

	switch (m_prg_mode)
	{
		case 0: // 32k banks
			bank3 = m_prg_regs[3] >> 2;
			prg32(bank3);
			break;

		case 1: // 16k banks
			bank1 = m_prg_regs[1] >> 1;
			bank3 = m_prg_regs[3] >> 1;

			if (m_prg_ram_mapped[1])
			{
				m_ram_hi_banks[0] = ((bank1 << 1) & 0x07);
				m_ram_hi_banks[1] = ((bank1 << 1) & 0x07) | 1;
			}
			else
				prg16_89ab(bank1);

			prg16_cdef(bank3);
			break;

		case 2: // 16k-8k banks
			bank1 = m_prg_regs[1] >> 1;
			bank2 = m_prg_regs[2];
			bank3 = m_prg_regs[3];

			if (m_prg_ram_mapped[1])
			{
				m_ram_hi_banks[0] = ((bank1 << 1) & 0x07);
				m_ram_hi_banks[1] = ((bank1 << 1) & 0x07) | 1;
			}
			else
				prg16_89ab(bank1);

			if (m_prg_ram_mapped[2])
				m_ram_hi_banks[2] = (bank2 & 0x07);
			else
				prg8_cd(bank2);

			prg8_ef(bank3);
			break;

		case 3: // 8k banks
			bank0 = m_prg_regs[0];
			bank1 = m_prg_regs[1];
			bank2 = m_prg_regs[2];
			bank3 = m_prg_regs[3];

			if (m_prg_ram_mapped[0])
				m_ram_hi_banks[0] = (bank0 & 0x07);
			else
				prg8_89(bank0);

			if (m_prg_ram_mapped[1])
				m_ram_hi_banks[1] = (bank1 & 0x07);
			else
				prg8_ab(bank1);

			if (m_prg_ram_mapped[2])
				m_ram_hi_banks[2] = (bank2 & 0x07);
			else
				prg8_cd(bank2);

			prg8_ef(bank3);
			break;
	}
}

void nes_exrom_device::hblank_irq(int scanline, int vblank, int blanked )
{
	m_vcount = scanline;

	if (scanline == m_irq_count)
	{
		if (m_irq_enable)
			m_maincpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);

		m_irq_status = 0xff;
	}

	// "In Frame" flag
	if (scanline == 0)
		m_irq_status |= 0x40;
	else if (scanline > PPU_BOTTOM_VISIBLE_SCANLINE)
		m_irq_status &= ~0x40;
}


void nes_exrom_device::set_mirror(int page, int src)
{
	switch (src)
	{
		case 0:
			set_nt_page(page, CIRAM, 0, 1);
			break;
		case 1:
			set_nt_page(page, CIRAM, 1, 1);
			break;
		case 2:
			set_nt_page(page, EXRAM, 0, 1);
			break;
		case 3:
			set_nt_page(page, MMC5FILL, 0, 0);
			break;
		default:
			fatalerror("This should never happen\n");
	}
}

inline bool nes_exrom_device::in_split()
{
	ppu2c0x_device *ppu = machine().device<ppu2c0x_device>("ppu");
	int tile = ppu->get_tilenum();

	if (tile < 34)
	{
		if (!m_split_rev && tile < m_split_ctrl)
			return TRUE;
		if (m_split_rev && tile >= m_split_ctrl)
			return TRUE;
	}
	return FALSE;
}

READ8_MEMBER(nes_exrom_device::nt_r)
{
	int page = ((offset & 0xc00) >> 10);

	switch (m_nt_src[page])
	{
		case MMC5FILL:
			if ((offset & 0x3ff) >= 0x3c0)
				return m_floodattr;
			return m_floodtile;

		case EXRAM:
			// to investigate: can split screen affect this too?
			if (!BIT(m_exram_control, 1))
				return m_exram[offset & 0x3ff];
			else
				return 0x00;

		case CIRAM:
		default:
			// Uchuu Keibitai SDF uses extensively split screen for its intro,
			// but it does not work yet
			if (m_split_scr && !(m_exram_control & 0x02) && in_split())
			{
				ppu2c0x_device *ppu = machine().device<ppu2c0x_device>("ppu");
				int tile = ppu->get_tilenum();

				if ((offset & 0x3ff) >= 0x3c0)
				{
					int pos = (((m_split_yst + m_vcount) & ~0x1f) | (tile & 0x1f)) >> 2;
					return m_exram[0x3c0 | pos];
				}
				else
				{
					int pos = (((m_split_yst + m_vcount) & 0xf8) << 2) | (tile & 0x1f);
					return m_exram[pos];
				}
			}

			if (m_exram_control == 1)
			{
				if ((offset & 0x3ff) >= 0x3c0)
					return m_mmc5_attrib[(m_exram[offset & 0x3ff] >> 6) & 0x03];
				else    // in this case, we write Ex1 CHR bank, but then access NT normally!
				{
					m_ex1_chr = 1;
					m_ex1_bank = (m_exram[offset & 0x3ff] & 0x3f) | (m_high_chr << 6);
				}
			}
			return m_nt_access[page][offset & 0x3ff];
	}
}

WRITE8_MEMBER(nes_exrom_device::nt_w)
{
	int page = ((offset & 0xc00) >> 10);

	if (!m_nt_writable[page])
		return;

	switch (m_nt_src[page])
	{
		case EXRAM:
			m_exram[offset & 0x3ff] = data;
			break;

		case CIRAM:
		default:
			m_nt_access[page][offset & 0x3ff] = data;
			break;
	}
}

inline UINT8 nes_exrom_device::base_chr_r(int bank, UINT32 offset)
{
	UINT32 helper = 0;

	switch (m_chr_mode)
	{
		case 0:
			if (bank < 8)
				helper = ((m_vrom_bank[bank | 7] & 0xff) * 0x2000) + (offset & 0x1fff);
			else
				helper = ((m_vrom_bank[bank | 3] & 0xff) * 0x2000) + (offset & 0xfff);
			break;
		case 1:
			helper = ((m_vrom_bank[bank | 3] & 0xff) * 0x1000) + (offset & 0xfff);
			break;
		case 2:
			helper = (m_vrom_bank[bank | 1] * 0x800) + (offset & 0x7ff);
			break;
		case 3:
			helper = (m_vrom_bank[bank] * 0x400) + (offset & 0x3ff);
			break;
	}

	return m_vrom[helper & (m_vrom_size - 1)];
}

inline UINT8 nes_exrom_device::split_chr_r(UINT32 offset)
{
	UINT32 helper = (m_split_bank * 0x1000) + (offset & 0x3f8) + (m_split_yst & 7);
	return m_vrom[helper & (m_vrom_size - 1)];
}

inline UINT8 nes_exrom_device::bg_ex1_chr_r(UINT32 offset)
{
	UINT32 helper = (m_ex1_bank * 0x1000) + (offset & 0xfff);
	return m_vrom[helper & (m_vrom_size - 1)];
}

READ8_MEMBER(nes_exrom_device::chr_r)
{
	int bank = offset >> 10;
	ppu2c0x_device *ppu = machine().device<ppu2c0x_device>("ppu");

	// Extended Attribute Mode (Ex1) does affect BG drawing even for 8x16 sprites (JustBreed uses it extensively!)
	// However, if a game enables Ex1 but does not write a new m_ex1_bank, I'm not sure here we get the correct behavior
	if (m_exram_control == 1 && ppu->get_draw_phase() == PPU_DRAW_BG && m_ex1_chr)
		return bg_ex1_chr_r(offset & 0xfff);

	if (m_split_scr && !(m_exram_control & 0x02) && in_split() && ppu->get_draw_phase() == PPU_DRAW_BG && m_split_chr)
		return split_chr_r(offset & 0xfff);

	if (ppu->is_sprite_8x16())
	{
		if (ppu->get_draw_phase() == PPU_DRAW_OAM)
			return base_chr_r(bank & 7, offset & 0x1fff);

		if (ppu->get_draw_phase() == PPU_DRAW_BG)
			return base_chr_r((bank & 3) + 8, offset & 0x1fff);
	}

	if (m_last_chr == LAST_CHR_REG_A)
		return base_chr_r(bank & 7, offset & 0x1fff);
	else
		return base_chr_r((bank & 3) + 8, offset & 0x1fff);
}


READ8_MEMBER(nes_exrom_device::read_l)
{
	int value;
	LOG_MMC(("exrom read_l, offset: %04x\n", offset));
	offset += 0x100;

	if ((offset >= 0x1c00) && (offset <= 0x1fff))
	{
		// EXRAM
		if (BIT(m_exram_control, 1))    // Modes 2,3 = read
			return m_exram[offset - 0x1c00];
		else
			return m_open_bus;   // Modes 0,1 = open bus
	}

	switch (offset)
	{
		case 0x1204:
			value = m_irq_status;
			m_irq_status &= ~0x80;
			m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
			return value;

		case 0x1205:
			return (m_mult1 * m_mult2) & 0xff;
		case 0x1206:
			return ((m_mult1 * m_mult2) & 0xff00) >> 8;

		default:
			logerror("MMC5 uncaught read, offset: %04x\n", offset + 0x4100);
			return m_open_bus;
	}
}


WRITE8_MEMBER(nes_exrom_device::write_l)
{
	LOG_MMC(("exrom write_l, offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;

	if ((offset >= 0x1000) && (offset <= 0x1015))
	{
		// SOUND
		nesapu_device *m_sound = machine().device<nesapu_device>("maincpu::nessound");
		m_sound->write(space, offset & 0x1f, data);
		return;
	}

	if ((offset >= 0x1c00) && (offset <= 0x1fff))
	{
		// EXRAM
		if (m_exram_control == 0x02)    // Mode 2 = write data
			m_exram[offset - 0x1c00] = data;
		else if (m_exram_control != 0x03)   // Modes 0,1 = write data in frame / write 0 otherwise
		{
			if (m_irq_status & 0x40)
				m_exram[offset - 0x1c00] = data;
			else
				m_exram[offset - 0x1c00] = 0x00;
		}
		// Mode 3 = read only
		return;
	}

	switch (offset)
	{
		case 0x1100:
			m_prg_mode = data & 0x03;
			update_prg();
			//LOG_MMC(("MMC5 rom bank mode: %02x\n", data));
			break;

		case 0x1101:
			m_chr_mode = data & 0x03;
			m_ex1_chr = 0;
			m_split_chr = 0;
			//LOG_MMC(("MMC5 vrom bank mode: %02x\n", data));
			break;

		case 0x1102:
			m_wram_protect_1 = data & 0x03;
			LOG_MMC(("MMC5 vram protect 1: %02x\n", data));
			break;
		case 0x1103:
			m_wram_protect_2 = data & 0x03;
			LOG_MMC(("MMC5 vram protect 2: %02x\n", data));
			break;

		case 0x1104: // Extra VRAM (EXRAM)
			m_exram_control = data & 0x03;
			LOG_MMC(("MMC5 exram control: %02x\n", data));
			break;

		case 0x1105:
			set_mirror(0, (data & 0x03) >> 0);
			set_mirror(1, (data & 0x0c) >> 2);
			set_mirror(2, (data & 0x30) >> 4);
			set_mirror(3, (data & 0xc0) >> 6);
			break;

		case 0x1106:
			m_floodtile = data;
			break;

		case 0x1107:
			m_floodattr = m_mmc5_attrib[data & 3];
			break;

		case 0x1113:
			LOG_MMC(("MMC5 mid RAM bank select: %02x\n", data & 0x07));
			m_wram_base = data & 0x07;
			break;


		case 0x1114:
		case 0x1115:
		case 0x1116:
		case 0x1117:
			m_prg_regs[offset & 3] = data & 0x7f;
			m_prg_ram_mapped[offset & 3] = !BIT(data, 7);   // m_prg_ram_mapped[3] is not used, in fact!
			update_prg();
			break;

		case 0x1120:
		case 0x1121:
		case 0x1122:
		case 0x1123:
		case 0x1124:
		case 0x1125:
		case 0x1126:
		case 0x1127:
			m_vrom_bank[offset & 0x07] = data | (m_high_chr << 8);
			m_last_chr = LAST_CHR_REG_A;
			m_ex1_chr = 0;
			m_split_chr = 0;
			break;

		case 0x1128:
		case 0x1129:
		case 0x112a:
		case 0x112b:
			m_vrom_bank[offset & 0x0f] = data | (m_high_chr << 8);
			m_last_chr = LAST_CHR_REG_B;
			m_ex1_chr = 0;
			m_split_chr = 0;
			break;

		case 0x1130:
			m_high_chr = data & 0x03;
			m_ex1_chr = 0;
			m_split_chr = 0;
			break;


		case 0x1200:
			// in EX2 and EX3 modes, no split screen
			m_split_scr = BIT(data, 7);
			m_split_rev = BIT(data, 6);
			m_split_ctrl = data & 0x1f;
			break;

		case 0x1201:
			m_split_yst = (data >= 240) ? data - 16 : data;
			break;

		case 0x1202:
			m_split_bank = data;
			m_split_chr = 1;
			break;

		case 0x1203:
			m_irq_count = data;
			LOG_MMC(("MMC5 irq scanline: %d\n", m_irq_count));
			break;
		case 0x1204:
			m_irq_enable = data & 0x80;
			LOG_MMC(("MMC5 irq enable: %02x\n", data));
			break;
		case 0x1205:
			m_mult1 = data;
			break;
		case 0x1206:
			m_mult2 = data;
			break;

		default:
			logerror("MMC5 uncaught write, offset: %04x, data: %02x\n", offset + 0x4100, data);
			break;
	}
}

// 3bits are used to access the "WRAM" banks
// bit3 select the chip (2 of them can be accessed, each up to 32KB)
// bit1 & bit2 select the 8KB banks inside the chip
// same mechanism is used also when "WRAM" is mapped in higher banks
READ8_MEMBER(nes_exrom_device::read_m)
{
	LOG_MMC(("exrom read_m, offset: %04x\n", offset));
	if (!m_battery.empty() && !m_prgram.empty())  // 2 chips present: first is BWRAM, second is WRAM
	{
		if (m_wram_base & 0x04)
			return m_prgram[(offset + (m_wram_base & 0x03) * 0x2000) & (m_prgram.size() - 1)];
		else
			return m_battery[(offset + (m_wram_base & 0x03) * 0x2000) & (m_battery.size() - 1)];
	}
	else if (!m_prgram.empty())  // 1 chip, WRAM
		return m_prgram[(offset + (m_wram_base & 0x03) * 0x2000) & (m_prgram.size() - 1)];
	else if (!m_battery.empty()) // 1 chip, BWRAM
		return m_battery[(offset + (m_wram_base & 0x03) * 0x2000) & (m_battery.size() - 1)];
	else
		return m_open_bus;
}

WRITE8_MEMBER(nes_exrom_device::write_m)
{
	LOG_MMC(("exrom write_m, offset: %04x, data: %02x\n", offset, data));
	if (m_wram_protect_1 != 0x02 || m_wram_protect_2 != 0x01)
		return;

	if (!m_battery.empty() && m_wram_base < 4)
		m_battery[(offset + m_wram_base * 0x2000) & (m_battery.size() - 1)] = data;
	else if (!m_prgram.empty())
		m_prgram[(offset + (m_wram_base & 0x03) * 0x2000) & (m_prgram.size() - 1)] = data;
}

// some games (e.g. Bandit Kings of Ancient China) write to PRG-RAM through 0x8000-0xdfff
READ8_MEMBER(nes_exrom_device::read_h)
{
	LOG_MMC(("exrom read_h, offset: %04x\n", offset));
	int bank = offset / 0x2000;

	if (bank < 3 && offset >= bank * 0x2000 && offset < (bank + 1) * 0x2000 && m_prg_ram_mapped[bank])
	{
		if (!m_battery.empty() && m_ram_hi_banks[bank] < 4)
			return m_battery[((m_ram_hi_banks[bank] * 0x2000) + (offset & 0x1fff)) & (m_battery.size() - 1)];
		else if (!m_prgram.empty())
			return m_prgram[(((m_ram_hi_banks[bank] & 3) * 0x2000) + (offset & 0x1fff)) & (m_prgram.size() - 1)];
	}

	return hi_access_rom(offset);
}

WRITE8_MEMBER(nes_exrom_device::write_h)
{
	LOG_MMC(("exrom write_h, offset: %04x, data: %02x\n", offset, data));
	int bank = offset / 0x2000;
	if (m_wram_protect_1 != 0x02 || m_wram_protect_2 != 0x01 || bank == 3 || !m_prg_ram_mapped[bank])
		return;

	if (!m_battery.empty() && m_ram_hi_banks[bank] < 4)
		m_battery[((m_ram_hi_banks[bank] * 0x2000) + (offset & 0x1fff)) & (m_battery.size() - 1)] = data;
	else if (!m_prgram.empty())
		m_prgram[(((m_ram_hi_banks[bank] & 3) * 0x2000) + (offset & 0x1fff)) & (m_prgram.size() - 1)] = data;
}
