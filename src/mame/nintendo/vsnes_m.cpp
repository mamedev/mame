// license:BSD-3-Clause
// copyright-holders:Brad Oliver, Fabio Priuli
/***************************************************************************

Nintendo VS UniSystem and DualSystem - (c) 1984 Nintendo of America

    Portions of this code are heavily based on
    Brad Oliver's MESS implementation of the NES.

***************************************************************************/

#include "emu.h"
#include "screen.h"
#include "video/ppu2c0x.h"
#include "vsnes.h"


/* PPU notes */
/* nametable is, per Lord Nightmare, always 4K per PPU */
/* The vsnes system has relatively few banking options for CHR */
/* Each driver will use ROM or RAM for CHR, never both, and RAM is never banked */
/* This leads to the memory system being an optimal place to perform banking */


/*************************************
 *
 *  Init machine
 *
 *************************************/

void vsnes_state::machine_reset()
{
	m_input_latch[0] = m_input_latch[1] = 0;
	m_input_latch[2] = m_input_latch[3] = 0;
	m_input_strobe[0] = m_input_strobe[1] = 0;
}

/*************************************
 *
 *  Machine start functions
 *
 *************************************/

void vsnes_state::init_prg_banking()
{
	u8 *base = memregion("prg")->base();
	m_prg_chunks = memregion("prg")->bytes() / 0x2000;

	for (int i = 0; i < 4; i++)
	{
		m_prg_banks[i]->configure_entries(0, m_prg_chunks, base, 0x2000);
		m_prg_banks[i]->set_entry(m_prg_chunks - 4 + i);
	}

	m_prg_view.select(0);
}

// safe banking helpers (only work when PRG size is a power of 2, i.e. not Gumshoe)

void vsnes_state::prg32(int bank)
{
	bank = (bank << 2) & (m_prg_chunks - 1);

	for (int i = 0; i < 4; i++)
		m_prg_banks[i]->set_entry(bank + i);
}

void vsnes_state::prg16(int slot, int bank)
{
	bank = (bank << 1) & (m_prg_chunks - 1);
	slot = (slot & 1) << 1;

	for (int i = 0; i < 2; i++)
		m_prg_banks[slot + i]->set_entry(bank + i);
}

void vsnes_state::prg8(int slot, int bank)
{
	m_prg_banks[slot & 0x03]->set_entry(bank & (m_prg_chunks - 1));
}

void vsnes_state::v_set_videorom_bank(int start, int count, int vrom_start_bank)
{
	assert(start + count <= 8);

	vrom_start_bank &= m_chr_chunks - 1;
	assert(vrom_start_bank + count <= m_chr_chunks);

	// count determines the size of the area mapped
	for (int i = 0; i < count; i++)
		m_chr_banks[i + start]->set_entry(vrom_start_bank + i);
}

MACHINE_START_MEMBER(vsnes_state, vsnes)
{
	// establish chr banks
	// DRIVER_INIT is called first - means we can handle this different for VRAM games!
	if (m_gfx1_rom != nullptr)
	{
		u8 *base = m_gfx1_rom->base();
		m_chr_chunks = m_gfx1_rom->bytes() / 0x400;

		for (int i = 0; i < 8; i++)
		{
			m_ppu1->space(AS_PROGRAM).install_read_bank(0x0400 * i, 0x0400 * i + 0x03ff, m_chr_banks[i]);
			m_chr_banks[i]->configure_entries(0, m_chr_chunks, base, 0x400);
		}
		v_set_videorom_bank(0, 8, 0);
	}
	else
		m_chr_view.select(0);
}

MACHINE_START_MEMBER(vsnes_state, vsdual)
{
	for (int i = 0; i < 2; i++)
	{
		const char *region = i ? "gfx2" : "gfx1";
		u8 *base = memregion(region)->base();
		int entries = memregion(region)->bytes() / 0x2000;
		m_chr_banks[i]->configure_entries(0, entries, base, 0x2000);
		m_chr_banks[i]->set_entry(0);
	}
}

MACHINE_START_MEMBER(vsnes_state, bootleg)
{
	u8 *base = m_gfx1_rom->base();
	int entries = m_gfx1_rom->bytes() / 0x2000;
	m_chr_banks[0]->configure_entries(0, entries, base, 0x2000);
	m_chr_banks[0]->set_entry(0);
}

/**********************************************************************************
 *
 *  Game and Board-specific initialization
 *
 **********************************************************************************/

//**********************************************************************************
// Most games: VROM Banking in controller 0 write

void vsnes_state::vsnormal_vrom_banking(u8 data)
{
	// switch vrom
	v_set_videorom_bank(0, 8, (data & 4) ? 8 : 0);

	// bit 1 ( data & 2 ) enables writes to extra ram, we ignore it

	// move along
	vsnes_in0_w<MAIN>(data);
}

void vsnes_state::init_vsnormal()
{
	// vrom switching is enabled with bit 2 of $4016
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x4016, 0x4016, write8smo_delegate(*this, FUNC(vsnes_state::vsnormal_vrom_banking)));
}

//**********************************************************************************
// Gun games: VROM Banking in controller 0 write

void vsnes_state::init_vsgun()
{
	init_vsnormal();
	m_has_gun = true;
}

//**********************************************************************************
// Konami VRC1 games: ROM banking at $8000-$ffff

void vsnes_state::vskonami_rom_banking(offs_t offset, u8 data)
{
	int reg = BIT(offset, 12, 3);

	switch (reg)
	{
		case 0: // prg bank 0
		case 2: // prg bank 1
		case 4: // prg bank 2
			prg8(reg >> 1, data);
			break;

		case 6: // vrom bank 0
			v_set_videorom_bank(0, 4, data * 4);
			break;

		case 7: // vrom bank 1
			v_set_videorom_bank(4, 4, data * 4);
			break;
	}
}

void vsnes_state::init_vskonami()
{
	// point program banks to last 32K
	init_prg_banking();

	// banking is done with writes to the $8000-$ffff area
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x8000, 0xffff, write8sm_delegate(*this, FUNC(vsnes_state::vskonami_rom_banking)));
}

//**********************************************************************************
// Vs. Gumshoe

void vsnes_state::vsgshoe_gun_in0_w(u8 data)
{
	// Gumshoe uniquely has a bankable 16K EPROM in addition to the normal unbanked 8K slots
	m_prg_banks[0]->set_entry(BIT(data, 2));

	// otherwise do normal CHR banking and IO write
	vsnormal_vrom_banking(data);
}

void vsnes_state::init_vsgshoe()
{
	// point program banks to last 32K
	init_prg_banking();

	// vrom switching is enabled with bit 2 of $4016
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x4016, 0x4016, write8smo_delegate(*this, FUNC(vsnes_state::vsgshoe_gun_in0_w)));

	m_has_gun = true;
}

//**********************************************************************************
// MMC1 (Dr Mario): ROM banking at $8000-$ffff

void vsnes_state::drmario_rom_banking(offs_t offset, u8 data)
{
	// reset mapper
	if (data & 0x80)
	{
		m_mmc1_shiftcount = 0;
		m_mmc1_prg16k = 1;
		m_mmc1_switchlow = 1;
		m_mmc1_chr4k = 0;

		return;
	}

	// update shift register
	m_mmc1_shiftreg = (m_mmc1_shiftreg >> 1) | (data & 1) << 4;
	m_mmc1_shiftcount = (m_mmc1_shiftcount + 1) % 5;

	// are we done shifting?
	if (!m_mmc1_shiftcount)
	{
		// apply data to registers
		switch (BIT(offset, 13, 2))
		{
			case 0: // mirroring and options
				m_mmc1_chr4k = m_mmc1_shiftreg & 0x10;
				m_mmc1_prg16k = m_mmc1_shiftreg & 0x08;
				m_mmc1_switchlow = m_mmc1_shiftreg & 0x04;
				// 0x03: mirroring bits unused on VS
				break;

			case 1: // video rom banking - bank 0 - 4k or 8k
				if (m_mmc1_chr4k)
					v_set_videorom_bank(0, 4, m_mmc1_shiftreg * 4);
				else
					v_set_videorom_bank(0, 8, (m_mmc1_shiftreg & ~1) * 4);
				break;

			case 2: // video rom banking - bank 1 - 4k only
				if (m_mmc1_chr4k)
					v_set_videorom_bank(4, 4, m_mmc1_shiftreg * 4);
				break;

			case 3: // program banking
				if (m_mmc1_prg16k)
					prg16(!m_mmc1_switchlow, m_mmc1_shiftreg);
				else
					prg32(m_mmc1_shiftreg >> 1);
				break;
		}
	}
}

void vsnes_state::init_drmario()
{
	// point program banks to last 32K
	init_prg_banking();

	// MMC1 mapper at $8000-$ffff
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x8000, 0xffff, write8sm_delegate(*this, FUNC(vsnes_state::drmario_rom_banking)));

	m_mmc1_shiftreg = 0;
	m_mmc1_shiftcount = 0;
}

//**********************************************************************************
// (UNROM) Games with VRAM instead of graphics ROMs: ROM banking at $8000-$ffff

void vsnes_state::vsvram_rom_banking(u8 data)
{
	prg16(0, data);
}

void vsnes_state::init_vsvram()
{
	// point program banks to last 32K
	init_prg_banking();

	// banking is done with writes to the $8000-$ffff area
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x8000, 0xffff, write8smo_delegate(*this, FUNC(vsnes_state::vsvram_rom_banking)));
}

//**********************************************************************************
// (Namco) 108 (MMC3 predecessor) games

void vsnes_state::vs108_rom_banking(offs_t offset, u8 data)
{
	switch (offset & 0x6001)
	{
		case 0x0000: // $8000
			m_108_reg = data & 0x07;
			break;

		case 0x0001: // $8001
			switch (m_108_reg)
			{
				case 0: case 1:
					v_set_videorom_bank(m_108_reg * 2, 2, data);
					break;
				case 2: case 3: case 4: case 5:
					v_set_videorom_bank(m_108_reg + 2, 1, data);
					break;
				case 6: case 7:
					prg8(m_108_reg - 6, data);
					break;
			}
			break;

		default:
			logerror("vs108_rom_banking uncaught: %04x value: %02x\n", offset + 0x8000, data);
			break;
	}
}

// Common init for (Namco) 108 games

void vsnes_state::init_vs108()
{
	// point program banks to last 32K
	init_prg_banking();

	m_108_reg = 0;

	// 108 chip at $8000-$9fff
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x8000, 0xffff, write8sm_delegate(*this, FUNC(vsnes_state::vs108_rom_banking)));
}

// Vs. RBI Baseball
// rbibb uses a protection chip labeled '127'
// Tests indicate that '127' uses a LFSR with taps: 23, 18, 0
// It is still unknown how bits are then shuffled for readback at 0x5601/0x5e01
u8 vsnes_state::rbibb_prot_r(offs_t offset)
{
	static constexpr u8 prot_data[32] = {
		0xff, 0xfd, 0xf5, 0xf4, 0xb4, 0xb4, 0xa6, 0x2e,
		0x2f, 0x6f, 0x6f, 0x7d, 0xd5, 0xd4, 0x94, 0x94,
		0x86, 0x2e, 0x2f, 0x6f, 0x6b, 0x79, 0xd1, 0xd0,
		0x92, 0x92, 0x8d, 0x65, 0x64, 0x34, 0xb0, 0xa2
	};

	if (offset == 0)
	{
		m_prot_index = 0;
		return 0;
	}

	return prot_data[m_prot_index++];
}

void vsnes_state::init_rbibb()
{
	init_vs108();

	// RBI Baseball protection, address fully decoded except A11
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x5600, 0x5601, read8sm_delegate(*this, FUNC(vsnes_state::rbibb_prot_r)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x5e00, 0x5e01, read8sm_delegate(*this, FUNC(vsnes_state::rbibb_prot_r)));
}

// Vs. Super Xevious

u8 vsnes_state::supxevs_prot_1_r()
{
	m_prot_index ^= 1;
	return 0x05;
}

u8 vsnes_state::supxevs_prot_2_r()
{
	return m_prot_index ? 0x01 : 0x00;
}

u8 vsnes_state::supxevs_prot_3_r()
{
	return m_prot_index ? 0x89 : 0xd1;
}

u8 vsnes_state::supxevs_prot_4_r()
{
	return m_prot_index ? 0x37 : 0x3e;
}

void vsnes_state::init_supxevs()
{
	init_vs108();

	// Vs. Super Xevious Protection
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x54ff, 0x54ff, read8smo_delegate(*this, FUNC(vsnes_state::supxevs_prot_1_r)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x5678, 0x5678, read8smo_delegate(*this, FUNC(vsnes_state::supxevs_prot_2_r)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x578f, 0x578f, read8smo_delegate(*this, FUNC(vsnes_state::supxevs_prot_3_r)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x5567, 0x5567, read8smo_delegate(*this, FUNC(vsnes_state::supxevs_prot_4_r)));

	m_prot_index = 0;
}

// Vs. TKO Boxing
// tkoboxng uses a protection chip labeled '128'
u8 vsnes_state::tkoboxng_prot_r(offs_t offset)
{
	static constexpr u8 prot_data[32] = {
		0xff, 0xbf, 0xb7, 0x97, 0x97, 0x17, 0x57, 0x4f,
		0x6f, 0x6b, 0xeb, 0xa9, 0xb1, 0x90, 0x94, 0x14,
		0x56, 0x4e, 0x6f, 0x6b, 0xeb, 0xa9, 0xb1, 0x90,
		0xd4, 0x5c, 0x3e, 0x26, 0x87, 0x83, 0x13, 0x00
	};

	if (offset == 0)
	{
		m_prot_index = 0;
		return 0;
	}

	return prot_data[m_prot_index++];
}

void vsnes_state::init_tkoboxng()
{
	init_vs108();

	// security device at $5e00-$5e01
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x5e00, 0x5e01, read8sm_delegate(*this, FUNC(vsnes_state::tkoboxng_prot_r)));
}

// Vs. Freedom Force

void vsnes_state::init_vsfdf()
{
	init_vs108();
	m_has_gun = true;
}

//**********************************************************************************
// Sunsoft-3 (Platoon) rom banking

void vsnes_state::sunsoft3_rom_banking(offs_t offset, u8 data)
{
	switch (offset & 0x7800)
	{
		case 0x0800:
		case 0x1800:
		case 0x2800:
		case 0x3800:
			v_set_videorom_bank((offset >> 11) & 0x06, 2, data * 2);
			break;

		case 0x7800:
			prg16(0, data);
			break;
	}

}

void vsnes_state::init_platoon()
{
	// point program banks to last 32K
	init_prg_banking();

	m_maincpu->space(AS_PROGRAM).install_write_handler(0x8000, 0xffff, write8sm_delegate(*this, FUNC(vsnes_state::sunsoft3_rom_banking)));
}

//**********************************************************************************
// Vs. Raid on Bungeling Bay (Japan)

// FIXME: this is a bad hack! The unused 8K ROM in the driver supposedly runs on the main CPU (protection?)
// and does some sort of handshake (with IRQs) with the game running on the sub CPU.
void vsnes_state::set_bnglngby_irq_w(uint8_t data)
{
	m_ret = data;
	m_maincpu->set_input_line(0, (data & 2) ? ASSERT_LINE : CLEAR_LINE);
	/* other values ??? */
	/* 0, 4, 84 */
}

uint8_t vsnes_state::set_bnglngby_irq_r()
{
	return m_ret;
}

void vsnes_state::init_bnglngby()
{
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x0231, 0x0231, read8smo_delegate(*this, FUNC(vsnes_state::set_bnglngby_irq_r)), write8smo_delegate(*this, FUNC(vsnes_state::set_bnglngby_irq_w)));

	m_ret = 0;

	/* normal banking */
	init_vsnormal();
}

//**********************************************************************************
// VS Dualsystem

void vsnes_state::vsdual_vrom_banking_main(u8 data)
{
	// switch vrom
	m_chr_banks[0]->set_entry(BIT(data, 2));

	// bit 1 ( data & 2 ) triggers irq on the other cpu
	m_subcpu->set_input_line(0, (data & 2) ? CLEAR_LINE : ASSERT_LINE);

	// move along
	vsnes_in0_w<MAIN>(data);
}

void vsnes_state::vsdual_vrom_banking_sub(u8 data)
{
	// switch vrom
	m_chr_banks[1]->set_entry(BIT(data, 2));

	// bit 1 ( data & 2 ) triggers irq on the other cpu
	m_maincpu->set_input_line(0, (data & 2) ? CLEAR_LINE : ASSERT_LINE);

	// move along
	vsnes_in0_w<SUB>(data);
}

void vsnes_state::init_vsdual()
{
	// vrom switching is enabled with bit 2 of $4016
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x4016, 0x4016, write8smo_delegate(*this, FUNC(vsnes_state::vsdual_vrom_banking_main)));
	m_subcpu->space(AS_PROGRAM).install_write_handler(0x4016, 0x4016, write8smo_delegate(*this, FUNC(vsnes_state::vsdual_vrom_banking_sub)));
}

//**********************************************************************************
// Vs. Super Mario Bros (Bootleg)

void vsnes_state::vsnes_bootleg_scanline(int scanline, bool vblank, bool blanked)
{
	// Z80 IRQ is controlled by two factors:
	// - bit 6 of current (next) scanline number
	// - bit 6 of latched scanline number from Z80 reading $4000
	if (!(m_bootleg_latched_scanline & 0x40))
	{
		m_subcpu->set_input_line(INPUT_LINE_IRQ0, ((scanline + 1) & 0x40) ? ASSERT_LINE : CLEAR_LINE);
	}
}

u8 vsnes_state::vsnes_bootleg_ppudata()
{
	// CPU always reads higher CHR ROM banks from $2007, PPU always reads lower ones
	m_chr_banks[0]->set_entry(1);
	u8 data = m_ppu1->read(0x2007);
	m_chr_banks[0]->set_entry(0);

	return data;
}

void vsnes_state::init_bootleg()
{
	m_bootleg_sound_offset = 0;
	m_bootleg_sound_data = 0;
	m_bootleg_latched_scanline = 0;

	m_ppu1->set_scanline_callback(*this, FUNC(vsnes_state::vsnes_bootleg_scanline));

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x2007, 0x2007, read8smo_delegate(*this, FUNC(vsnes_state::vsnes_bootleg_ppudata)));
}
