// license:BSD-3-Clause
// copyright-holders:David Haywood

/***************************************************************************

  The 'VT' series are SoC solutions that implement enhanced NES hardware
  there are several generations of these chips each adding additional
  functionality.

  This list is incomplete

  VT01 - only enhancement over plain Famiclone is ability to drive STN displays directly
  VT02 - banking scheme to access 32MB, Dual APU with PCM support
  VT03 - above + 4bpp sprite / bg modes, enhanced palette

  VT08 - ?

  VT09 - alt 4bpp mode, 4KB system RAM, DMA isn't bugged in NTSC mode?

  VT16 - ?
  VT18 - ?

    VT33 (?) - used in FC Pocket, dgun2573
        Adds scrambled opcodes (XORed with 0xA1) and RGB444 palette mode,
        and more advanced PCM modes (CPU and video working, sound NYI)

    VT368 (?) - used in DGUN2561, lxcmcy
        Various enhancements not yet emulated. Different banking, possibly an ALU,
        larger palette space

    VT36x (?) - used in SY889
        Uses SQI rather than parallel flash
        Vaguely OneBus compatible but some registers different ($411C in particular)
        Uses RGB format for palettes
        Credit to NewRisingSun2 for much of the reverse engineering
                 same chipset used in Mogis M320, but uses more advanced feature set.

  (more)



  todo (VT03):

  APU refactoring to allow for mostly doubled up functionality + PCM channel
  *more*

  TODO:

  (newer VTxx)
  new PCM audio in FC Pocket and DGUN-2573
    add support for VT368 (?) in DGUN-2561 and lxcmcy
    add support for the VT369 (?) featurs used by the MOGIS M320

**************************************************************************/

#include "emu.h"
#include "nes_vt_soc.h"


DEFINE_DEVICE_TYPE(NES_VT02_VT03_SOC,          nes_vt02_vt03_soc_device,          "nes_vt02_vt03_soc",       "VT02/03 series System on a Chip (NTSC)")
DEFINE_DEVICE_TYPE(NES_VT02_VT03_SOC_PAL,      nes_vt02_vt03_soc_pal_device,      "nes_vt02_vt03_soc_pal",   "VT02/03 series System on a Chip (PAL)")
DEFINE_DEVICE_TYPE(NES_VT02_VT03_SOC_SCRAMBLE, nes_vt02_vt03_soc_scramble_device, "nes_vt02_vt03_soc_scram", "VT02/03 series System on a Chip (NTSC, with simple Opcode scrambling)")

void nes_vt02_vt03_soc_device::program_map(address_map &map)
{
}

nes_vt02_vt03_soc_device::nes_vt02_vt03_soc_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_maincpu(*this, "maincpu"),
	m_screen(*this, "screen"),
	m_ppu(*this, "ppu"),
	m_apu(*this, "apu"),
	m_initial_e000_bank(0xff),
	m_ntram(nullptr),
	m_chrram(nullptr),
	m_space_config("program", ENDIANNESS_LITTLE, 8, 25, 0, address_map_constructor(FUNC(nes_vt02_vt03_soc_device::program_map), this)),
	m_write_0_callback(*this),
	m_read_0_callback(*this),
	m_read_1_callback(*this),
	m_extra_write_0_callback(*this),
	m_extra_write_1_callback(*this),
	m_extra_write_2_callback(*this),
	m_extra_write_3_callback(*this),
	m_extra_read_0_callback(*this),
	m_extra_read_1_callback(*this),
	m_extra_read_2_callback(*this),
	m_extra_read_3_callback(*this)
{
	// 'no scramble' configuration
	for (int i = 0; i < 6; i++)
		m_2012_2017_descramble[i] = 2 + i;

	// 'no scramble' configuration
	m_8000_scramble[0x0] = 0x6;
	m_8000_scramble[0x1] = 0x7;
	m_8000_scramble[0x2] = 0x2;
	m_8000_scramble[0x3] = 0x3;
	m_8000_scramble[0x4] = 0x4;
	m_8000_scramble[0x5] = 0x5;
	m_8000_scramble[0x6] = 0x7;
	m_8000_scramble[0x7] = 0x8;

	// 'no scramble' configuration
	m_410x_scramble[0x0] = 0x7;
	m_410x_scramble[0x1] = 0x8;

	m_default_palette_mode = PAL_MODE_VT0x;
	m_force_baddma = false;
	m_use_raster_timing_hack = false;
}

nes_vt02_vt03_soc_device::nes_vt02_vt03_soc_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	nes_vt02_vt03_soc_device(mconfig, NES_VT02_VT03_SOC, tag, owner, clock)
{
}

nes_vt02_vt03_soc_pal_device::nes_vt02_vt03_soc_pal_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	nes_vt02_vt03_soc_device(mconfig, NES_VT02_VT03_SOC_PAL, tag, owner, clock)
{
}


nes_vt02_vt03_soc_scramble_device::nes_vt02_vt03_soc_scramble_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	nes_vt02_vt03_soc_device(mconfig, NES_VT02_VT03_SOC_SCRAMBLE, tag, owner, clock)
{
}

void nes_vt02_vt03_soc_device::device_start()
{
	save_item(NAME(m_410x));

	save_item(NAME(m_411c));
	save_item(NAME(m_411d));
	save_item(NAME(m_4242));

	save_item(NAME(m_8000_addr_latch));

	save_item(NAME(m_timer_irq_enabled));
	save_item(NAME(m_timer_running));
	save_item(NAME(m_timer_val));
	save_item(NAME(m_vdma_ctrl));

	m_ntram = std::make_unique<uint8_t[]>(0x2000);
	save_pointer(NAME(m_ntram), 0x2000);

	m_chrram = std::make_unique<uint8_t[]>(0x2000);
	save_pointer(NAME(m_chrram), 0x2000);

	m_ppu->set_scanline_callback(*this, FUNC(nes_vt02_vt03_soc_device::scanline_irq));
	m_ppu->set_hblank_callback(*this, FUNC(nes_vt02_vt03_soc_device::hblank_irq));

	//m_ppu->set_hblank_callback(*m_cartslot->m_cart, FUNC(device_nes_cart_interface::hblank_irq)));
	//m_ppu->space(AS_PROGRAM).install_readwrite_handler(0, 0x1fff, read8sm_delegate(*m_cartslot->m_cart, FUNC(device_nes_cart_interface::chr_r)), write8sm_delegate(*m_cartslot->m_cart, FUNC(device_nes_cart_interface::chr_w)));
	m_ppu->space(AS_PROGRAM).install_readwrite_handler(0x2000, 0x3eff, read8sm_delegate(*this, FUNC(nes_vt02_vt03_soc_device::nt_r)), write8sm_delegate(*this, FUNC(nes_vt02_vt03_soc_device::nt_w)));
	m_ppu->space(AS_PROGRAM).install_readwrite_handler(0, 0x1fff, read8sm_delegate(*this, FUNC(nes_vt02_vt03_soc_device::chr_r)), write8sm_delegate(*this, FUNC(nes_vt02_vt03_soc_device::chr_w)));

	m_write_0_callback.resolve_safe();
	m_read_0_callback.resolve_safe(0xff);
	m_read_1_callback.resolve_safe(0xff);

	m_extra_write_0_callback.resolve_safe();
	m_extra_write_1_callback.resolve_safe();
	m_extra_write_2_callback.resolve_safe();
	m_extra_write_3_callback.resolve_safe();

	m_extra_read_0_callback.resolve_safe(0xff);
	m_extra_read_1_callback.resolve_safe(0xff);
	m_extra_read_2_callback.resolve_safe(0xff);
	m_extra_read_3_callback.resolve_safe(0xff);
}

void nes_vt02_vt03_soc_device::device_reset()
{
	// what are the actual defaults?
	m_410x[0x0] = 0x00;
	m_410x[0x1] = 0x00;
	m_410x[0x2] = 0x00;
	m_410x[0x3] = 0x00;
	m_410x[0x4] = 0x00;
	m_410x[0x5] = 0x00;
	m_410x[0x6] = 0x00;
	m_410x[0x7] = 0x00;
	m_410x[0x8] = 0x01;
	m_410x[0x9] = 0x02;
	m_410x[0xa] = 0x00;
	m_410x[0xb] = 0x00;
	m_411c = 0x00;
	m_411d = 0x00;
	m_4242 = 0x00;

	m_timer_irq_enabled = 0;
	m_timer_running = 0;
	m_timer_val = 0;
	m_vdma_ctrl = 0;

	update_banks();

	m_ppu->set_201x_descramble(m_2012_2017_descramble[0], m_2012_2017_descramble[1], m_2012_2017_descramble[2], m_2012_2017_descramble[3], m_2012_2017_descramble[4], m_2012_2017_descramble[5]);
	m_ppu->set_palette_mode(m_default_palette_mode);

}

uint32_t nes_vt02_vt03_soc_device::get_banks(uint8_t bnk)
{
	switch (m_410x[0xb] & 0x07)
	{
	case 0: return ((m_410x[0x0] & 0xF0) << 4) + ((m_410x[0xa] & 0xC0) | (bnk & 0x3F)); // makes bank 0xff at 0xe000 map to 0x07e000 by default for vectors at 0x007fffx
	case 1: return ((m_410x[0x0] & 0xF0) << 4) + ((m_410x[0xa] & 0xE0) | (bnk & 0x1F));
	case 2: return ((m_410x[0x0] & 0xF0) << 4) + ((m_410x[0xa] & 0xF0) | (bnk & 0x0F));
	case 3: return ((m_410x[0x0] & 0xF0) << 4) + ((m_410x[0xa] & 0xF8) | (bnk & 0x07));
	case 4: return ((m_410x[0x0] & 0xF0) << 4) + ((m_410x[0xa] & 0xFC) | (bnk & 0x03));
	case 5: return ((m_410x[0x0] & 0xF0) << 4) + ((m_410x[0xa] & 0xFE) | (bnk & 0x01));
	case 6: return ((m_410x[0x0] & 0xF0) << 4) + (m_410x[0xa]);
	case 7: return ((m_410x[0x0] & 0xF0) << 4) + bnk;
	}

	return 0;
}

// 8000 needs to bank in 60000  ( bank 0x30 )
void nes_vt02_vt03_soc_device::update_banks()
{
	uint8_t bank;

	// 8000-9fff
	if ((m_410x[0xb] & 0x40) != 0 || (m_410x[0x5] & 0x40) == 0)
	{
		if ((m_410x[0x5] & 0x40) == 0)
			bank = m_410x[0x7];
		else
			bank = m_410x[0x9];
	}
	else
		bank = 0xfe;

	m_bankaddr[0] = get_banks(bank);

	// a000-bfff
	bank = m_410x[0x8];
	m_bankaddr[1] = get_banks(bank);

	// c000-dfff
	if ((m_410x[0xb] & 0x40) != 0 || (m_410x[0x5] & 0x40) != 0)
	{
		if ((m_410x[0x5] & 0x40) == 0)
			bank = m_410x[0x9];
		else
			bank = m_410x[0x7];
	}
	else
		bank = 0xfe;

	m_bankaddr[2] = get_banks(bank);

	// e000 - ffff
	bank = m_initial_e000_bank;
	m_bankaddr[3] = get_banks(bank);
}

uint16_t nes_vt02_vt03_soc_device::decode_nt_addr(uint16_t addr)
{
	bool vert_mirror = !(m_410x[0x6] & 0x01);
	int a11 = (addr >> 11) & 0x01;
	int a10 = (addr >> 10) & 0x01;
	uint16_t base = (addr & 0x3FF);
	return ((vert_mirror ? a10 : a11) << 10) | base;
}

void nes_vt02_vt03_soc_device::vt03_410x_w(offs_t offset, uint8_t data)
{
	scrambled_410x_w(offset, data);
}

uint8_t nes_vt02_vt03_soc_device::vt03_410x_r(offs_t offset)
{
	return m_410x[offset];
}


// Source: https://wiki.nesdev.com/w/index.php/NES_2.0_submappers/Proposals#NES_2.0_Mapper_256

void nes_vt02_vt03_soc_device::scrambled_410x_w(uint16_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0x0:
		m_410x[0x0] = data;
		update_banks();
		break;

	case 0x1:
		// latch timer value
		m_410x[0x1] = data;
		m_timer_running = 0;
		break;

	case 0x2:
		//logerror("vt03_4102_w %02x\n", data);
		// load latched value and start counting
		m_410x[0x2] = data; // value doesn't matter?
		m_timer_val = m_410x[0x1];

		// HACK for some one line errors in various games and completely broken rasters in msifrog, TOOD: find real source of issue (bad timing of interrupt or counter changes, or latching of data?)
		if (m_use_raster_timing_hack)
			if (m_ppu->in_vblanking())
				m_timer_val--;

		m_timer_running = 1;
		break;

	case 0x3:
		//logerror("vt03_4103_w %02x\n", data);
		m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
		// disable timer irq
		m_410x[0x3] = data; // value doesn't matter?
		m_timer_irq_enabled = 0;
		break;

	case 0x4:
		//logerror("vt03_4104_w %02x\n", data);
		// enable timer irq
		m_410x[0x4] = data; // value doesn't matter?
		m_timer_irq_enabled = 1;
		break;

	case 0x5:
		logerror("vt03_4105_w %02x\n", data);
		m_410x[0x5] = data;
		update_banks();
		break;

	case 0x6:
		m_410x[0x6] = data;
		break;

	case 0x7:
		m_410x[m_410x_scramble[0]] = data;
		update_banks();
		break;

	case 0x8:
		m_410x[m_410x_scramble[1]] = data;
		update_banks();
		break;

	case 0x9:
		logerror("vt03_4109_w %02x\n", data);
		m_410x[0x9] = data;
		update_banks();
		break;

	case 0xa:
		logerror("vt03_410a_w %02x\n", data);
		m_410x[0xa] = data;
		update_banks();
		break;

	case 0xb:
		/*

		D7 TSYNEN - Timer clock select 0:AD12, 1:HSYNC
		D6 Prg Bank 0 Reg 2 enable / disable  0:Disable 1:Enable
		D5 RS232 enable / disable  0:Disable 1:Enable
		D4 Bus output control  0: normal  1: tristate
		D3 6000-7fff and 8000-ffff control - 0 will not active XRWB, 1 will activate
		D2-D0 - program bank 0 selector

		*/

		logerror("vt03_410b_w %02x\n", data);
		m_410x[0xb] = data;
		update_banks();
		break;
	}
}



uint8_t nes_vt02_vt03_soc_device::spr_r(offs_t offset)
{
	if (m_4242 & 0x1 || m_411d & 0x04)
	{
		return m_chrram[offset];
	}
	else
	{
		int realaddr = calculate_real_video_address(offset, 0, 1);

		address_space& spc = this->space(AS_PROGRAM);
		return spc.read_byte(realaddr);
	}
}

uint8_t nes_vt02_vt03_soc_device::chr_r(offs_t offset)
{
	if (m_4242 & 0x1 || m_411d & 0x04) // newer VT platforms only (not VT03/09), split out
	{
		return m_chrram[offset];
	}
	else
	{
		int realaddr = calculate_real_video_address(offset, 1, 0);

		address_space& spc = this->space(AS_PROGRAM);
		return spc.read_byte(realaddr);
	}
}


void nes_vt02_vt03_soc_device::chr_w(offs_t offset, uint8_t data)
{
	if (m_4242 & 0x1 || m_411d & 0x04) // newer VT platforms only (not VT03/09), split out
	{
		logerror("vram write %04x %02x\n", offset, data);
		m_chrram[offset] = data;
	}
	else
	{
		int realaddr = calculate_real_video_address(offset, 1, 0);

		address_space& spc = this->space(AS_PROGRAM);
		return spc.write_byte(realaddr, data);
	}
}



void nes_vt02_vt03_soc_device::scanline_irq(int scanline, bool vblank, bool blanked)
{
	video_irq(false, scanline, vblank, blanked);
}

void nes_vt02_vt03_soc_device::hblank_irq(int scanline, bool vblank, bool blanked)
{
	video_irq(true, scanline, vblank, blanked);
}

void nes_vt02_vt03_soc_device::video_irq(bool hblank, int scanline, bool vblank, bool blanked)
{
	//TSYNEN
	if (((m_410x[0xb] >> 7) & 0x01) == hblank)
	{
		int irqstate = 0;

		//logerror("scanline_irq %d\n", scanline);

		if (m_timer_running && scanline < 0xe0)
		{
			m_timer_val--;

			if (m_timer_val < 0)
			{
				if (m_timer_irq_enabled && !blanked)
				{
					logerror("scanline_irq %d\n", scanline);
					irqstate = 1;
				}
			}
		}

		if (irqstate)
			m_maincpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
		//else
		//  m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
	}
}

/* todo, handle custom VT nametable stuff here */
uint8_t nes_vt02_vt03_soc_device::nt_r(offs_t offset)
{
	return m_ntram[decode_nt_addr(offset)];
}

void nes_vt02_vt03_soc_device::nt_w(offs_t offset, uint8_t data)
{
	//logerror("nt wr %04x %02x", offset, data);
	m_ntram[decode_nt_addr(offset)] = data;
}






int nes_vt02_vt03_soc_device::calculate_real_video_address(int addr, int extended, int readtype)
{
	// might be a VT09 only feature (alt 4bpp mode?)
	int alt_order = m_ppu->get_201x_reg(0x0) & 0x40;

	if (readtype == 0)
	{
		if (m_ppu->get_201x_reg(0x0) & 0x10)
		{
			extended = 1;
		}
		else
		{
			extended = 0;
		}
	}
	else if (readtype == 1)
	{
		if (m_ppu->get_201x_reg(0x0) & 0x08)
		{
			extended = 1;
		}
		else
		{
			extended = 0;
		}
	}

	/*
	Calculating TVA17 - TVA10

	--------------------------------------------------------------------------------------------
	| COMR7        | AD[12:10] | TVA17 | TVA16 | TVA15 | TVA14 | TVA13 | TVA12 | TVA11 | TVA10 |
	| (4105, 0x80) | (2006)    |       |       |       |       |       |       |       |       |
	--------------------------------------------------------------------------------------------
	| 0/1/C/D                  | RV47  | RV46  | RV45  | RV44  | RV43  | RV42  | RV41  | AD10  | ** RV40 is never used
	| 2/3/E/F                  | RV57  | RV56  | RV55  | RV54  | RV53  | RV52  | RV51  | AD10  | ** RV50 is never used
	| 4/8                      | RV07  | RV06  | RV05  | RV04  | RV03  | RV02  | RV01  | RV00  |
	| 5/9                      | RV17  | RV16  | RV15  | RV14  | RV13  | RV12  | RV11  | RV10  |
	| 6/A                      | RV27  | RV26  | RV25  | RV24  | RV23  | RV22  | RV21  | RV20  |
	| 7/B                      | RV37  | RV36  | RV35  | RV34  | RV33  | RV32  | RV31  | RV30  |
	--------------------------------------------------------------------------------------------

	m_r2012 = rv0x
	m_r2013 = rv1x
	m_r2014 = rv2x
	m_r2015 = rv3x
	m_r2016 = rv4x
	m_r2017 = rv5x

	*/
	int finaladdr = 0;

	int sel = (addr & 0x1c00) | ((m_410x[0x5] & 0x80) ? 0x2000 : 0x000);

	int vbank_tva17_tva10 = 0x00;

	switch ((sel >> 10) & 0xf)
	{
	case 0x0:
	case 0x1:
	case 0xc:
	case 0xd:
		vbank_tva17_tva10 = (m_ppu->get_201x_reg(0x6) & 0xfe) | ((addr & 0x0400) ? 1 : 0);
		break;

	case 0x2:
	case 0x3:
	case 0xe:
	case 0xf:
		vbank_tva17_tva10 = (m_ppu->get_201x_reg(0x7) & 0xfe) | ((addr & 0x0400) ? 1 : 0);
		break;

	case 0x4:
	case 0x8:
		vbank_tva17_tva10 = m_ppu->get_201x_reg(0x2);
		break;

	case 0x5:
	case 0x9:
		vbank_tva17_tva10 = m_ppu->get_201x_reg(0x3);
		break;

	case 0x6:
	case 0xa:
		vbank_tva17_tva10 = m_ppu->get_201x_reg(0x4);
		break;

	case 0x7:
	case 0xb:
		vbank_tva17_tva10 = m_ppu->get_201x_reg(0x5);
		break;

	}

	/*
	Calculating VA17 - VA10 (requires TVA17-TVA10 to have been calculated)

	------------------------------------------------------------------------------
	|  VB0S[2:0] |   VA[17:10]                                                   |
	| 201a & 0x7 |  VA17 |  VA16 |  VA15 |  VA14 |  VA13 |  VA12 |  VA11 |  VA10 |
	|-----------------------------------------------------------------------------
	| 0x0        | TVA17 | TVA16 | TVA15 | TVA14 | TVA13 | TVA12 | TVA11 | TVA10 |
	| 0x1        |  TV67 | TVA16 | TVA15 | TVA14 | TVA13 | TVA12 | TVA11 | TVA10 |
	| 0x2        |  RV67 |  RV66 | TVA15 | TVA14 | TVA13 | TVA12 | TVA11 | TVA10 |
	| 0x3        | INVALID ***************************************************** |
	| 0x4        |  RV67 |  RV66 |  RV65 | TVA14 | TVA13 | TVA12 | TVA11 | TVA10 |
	| 0x5        |  RV67 |  RV66 |  RV65 |  RV64 | TVA13 | TVA12 | TVA11 | TVA10 |
	| 0x6        |  RV67 |  RV66 |  RV65 |  RV64 |  RV63 | TVA12 | TVA11 | TVA10 |
	| 0x7        | INVALID ***************************************************** |
	------------------------------------------------------------------------------

	RV67- RV63 = 0x201a & 0xf8

	*/

	int va17_va10 = 0;

	int swit = m_ppu->get_201x_reg(0xa);

	switch (swit & 0x07)
	{
	case 0x0: va17_va10 = vbank_tva17_tva10; break;
	case 0x1: va17_va10 = (vbank_tva17_tva10 & 0x7f) | (m_ppu->get_201x_reg(0xa) & 0x80); break;
	case 0x2: va17_va10 = (vbank_tva17_tva10 & 0x3f) | (m_ppu->get_201x_reg(0xa) & 0xc0); break;
	case 0x3: return -1;
	case 0x4: va17_va10 = (vbank_tva17_tva10 & 0x1f) | (m_ppu->get_201x_reg(0xa) & 0xe0); break;
	case 0x5: va17_va10 = (vbank_tva17_tva10 & 0x0f) | (m_ppu->get_201x_reg(0xa) & 0xf0); break;
	case 0x6: va17_va10 = (vbank_tva17_tva10 & 0x07) | (m_ppu->get_201x_reg(0xa) & 0xf8); break;
	case 0x7: return -1;
	}

	int va34 = m_ppu->get_va34();

	if (!extended)
	{
		int is4bpp = 0;
		if (readtype == 0) is4bpp = m_ppu->get_201x_reg(0x0) & 0x02;
		else if (readtype == 1) is4bpp = m_ppu->get_201x_reg(0x0) & 0x04;

		int va20_va18 = (m_ppu->get_201x_reg(0x8) & 0x70) >> 4;

		finaladdr = ((m_410x[0x0] & 0x0F) << 21) | (va20_va18 << 18) | (va17_va10 << 10) | (addr & 0x03ff);

		if (is4bpp)
		{
			if (!alt_order)
			{
				finaladdr = ((finaladdr & ~0xf) << 1) | (va34 << 4) | (finaladdr & 0xf);
			}
			else
			{
				finaladdr = (finaladdr << 1) | va34;
			}
		}
	}
	else
	{
		int eva2_eva0 = 0x00;
		int is4bpp = 0;

		switch (readtype)
		{
		case 0: // background display
			is4bpp = m_ppu->get_201x_reg(0x0) & 0x02;

			eva2_eva0 |= m_ppu->get_m_read_bg4_bg3();

			if (m_ppu->get_201x_reg(0x1) & 0x02)
			{
				if (m_410x[0x6] & 0x1) eva2_eva0 |= 0x4;
			}
			else
			{
				if (m_ppu->get_201x_reg(0x8) & 0x08) eva2_eva0 |= 0x4;
			}
			break;

		case 1: // sprite display
			is4bpp = m_ppu->get_201x_reg(0x0) & 0x04; // 16 colors or 16-pixel wide (both adjust the read)

			eva2_eva0 |= m_ppu->get_speva2_speva0();

			break;

		case 2: // CPU R/W access
			// todo
			break;
		}

		finaladdr = ((m_410x[0x0] & 0x0f) << 21) | (va17_va10 << 13) | (eva2_eva0 << 10) | (addr & 0x03ff);

		if (is4bpp)
		{
			if (!alt_order)
			{
				finaladdr = ((finaladdr & ~0xf) << 1) | (va34 << 4) | (finaladdr & 0xf);
			}
			else
			{
				finaladdr = (finaladdr << 1) | va34;
			}

		}
	}
	return finaladdr;
}

/*
   nes_vt02_vt03_soc_device::vt03_8000_mapper_w notes

     used for MMC3/other mapper compatibility
     some consoles have scrambled registers for crude copy protection
*/

void nes_vt02_vt03_soc_device::scrambled_8000_w(uint16_t offset, uint8_t data)
{
	offset &= 0x7fff;

	uint16_t addr = offset+0x8000;
	if ((m_411d & 0x03) == 0x03) // (VT32 only, not VT03/09, split)
	{
		//CNROM compat
		logerror("%s: vtxx_cnrom_8000_w real address: (%04x) translated address: (%04x) %02x\n", machine().describe_context(), addr, offset + 0x8000, data);
		m_ppu->set_201x_reg(0x6, data * 8);
		m_ppu->set_201x_reg(0x7, data * 8 + 2);
		m_ppu->set_201x_reg(0x2, data * 8 + 4);
		m_ppu->set_201x_reg(0x3, data * 8 + 5);
		m_ppu->set_201x_reg(0x4, data * 8 + 6);
		m_ppu->set_201x_reg(0x5, data * 8 + 7);

	}
	else if ((m_411d & 0x03) == 0x01) // (VT32 only, not VT03/09, split)
	{
		//MMC1 compat, TODO
		logerror("%s: vtxx_mmc1_8000_w real address: (%04x) translated address: (%04x) %02x\n", machine().describe_context(), addr, offset + 0x8000, data);

	}
	else if ((m_411d & 0x03) == 0x02) // (VT32 only, not VT03/09, split)
	{
		//UNROM compat
		logerror("%s: vtxx_unrom_8000_w real address: (%04x) translated address: (%04x) %02x\n", machine().describe_context(), addr, offset + 0x8000, data);

		m_410x[0x7] = ((data & 0x0F) << 1);
		m_410x[0x8] = ((data & 0x0F) << 1) + 1;
		update_banks();
	}
	else // standard mode (VT03/09)
	{
		//logerror("%s: vtxx_mmc3_8000_w real address: (%04x) translated address: (%04x) %02x\n",  machine().describe_context(), addr, offset+0x8000, data );

		//MMC3 compat
		if ((addr < 0xA000) && !(addr & 0x01))
		{
			logerror("%s: scrambled_8000_w real address: (%04x) translated address: (%04x) %02x (banking)\n",  machine().describe_context(), addr, offset + 0x8000, data);
			// Bank select
			m_8000_addr_latch = data & 0x07;
			// Bank config
			m_410x[0x05] = data & ~(1 << 5);
			update_banks();
		}
		else if ((addr < 0xA000) && (addr & 0x01))
		{
			logerror("%s: scrambled_8000_w real address: (%04x) translated address: (%04x) %02x (other scrambled stuff)\n",  machine().describe_context(), addr, offset + 0x8000, data);

			switch (m_410x[0x05] & 0x07)
			{
			case 0x00:
				m_ppu->set_201x_reg(m_8000_scramble[0], data);
				break;

			case 0x01:
				m_ppu->set_201x_reg(m_8000_scramble[1], data);
				break;

			case 0x02: // hand?
				m_ppu->set_201x_reg(m_8000_scramble[2], data);
				break;

			case 0x03: // dog?
				m_ppu->set_201x_reg(m_8000_scramble[3], data);
				break;

			case 0x04: // ball thrown
				m_ppu->set_201x_reg(m_8000_scramble[4], data);
				break;

			case 0x05: // ball thrown
				m_ppu->set_201x_reg(m_8000_scramble[5], data);
				break;
			case 0x06:
				m_410x[m_8000_scramble[6]] = data;
				//m_410x[0x9] = data;
				update_banks();
				break;

			case 0x07:
				m_410x[m_8000_scramble[7]] = data;
				update_banks();
				break;
			}
		}
		else if ((addr >= 0xA000) && (addr < 0xC000) && !(addr & 0x01))
		{
			// Mirroring
			m_410x[0x6] &= 0xFE;
			m_410x[0x6] |= data & 0x01;
		}
		else if ((addr >= 0xA000) && (addr < 0xC000) && (addr & 0x01))
		{
			// PRG RAM control, ignore
		}
		else if ((addr >= 0xC000) && (addr < 0xE000) && !(addr & 0x01))
		{
			// IRQ latch
			vt03_410x_w(1, data);
		}
		else if ((addr >= 0xC000) && (addr < 0xE000) && (addr & 0x01))
		{
			// IRQ reload
			vt03_410x_w(2, data);
		}
		else if ((addr >= 0xE000) && !(addr & 0x01))
		{
			// IRQ disable
			vt03_410x_w(3, data);
		}
		else if ((addr >= 0xE000) && (addr & 0x01))
		{
			// IRQ enable
			vt03_410x_w(4, data);
		}
		else
		{

		}
	}
}

// MMC3 compatibility mode

void nes_vt02_vt03_soc_device::set_8000_scramble(uint8_t reg0, uint8_t reg1, uint8_t reg2, uint8_t reg3, uint8_t reg4, uint8_t reg5, uint8_t reg6, uint8_t reg7)
{
	m_8000_scramble[0] = reg0; // TODO: name the regs
	m_8000_scramble[1] = reg1;
	m_8000_scramble[2] = reg2;
	m_8000_scramble[3] = reg3;
	m_8000_scramble[4] = reg4;
	m_8000_scramble[5] = reg5;
	m_8000_scramble[6] = reg6;
	m_8000_scramble[7] = reg7;
}

void nes_vt02_vt03_soc_device::set_410x_scramble(uint8_t reg0, uint8_t reg1)
{
	m_410x_scramble[0] = reg0; // TODO: name the regs
	m_410x_scramble[1] = reg1;
}

void nes_vt02_vt03_soc_device::vt03_8000_mapper_w(offs_t offset, uint8_t data)
{
	scrambled_8000_w(offset, data);
	//logerror("%s: vt03_8000_mapper_w (%04x) %02x\n", machine().describe_context(), offset+0x8000, data );
}

/* APU plumbing, this is because we have a plain M6502 core in the VT03, otherwise this is handled in the core */

uint8_t nes_vt02_vt03_soc_device::psg1_4014_r()
{
	//return m_apu->read(0x14);
	return 0x00;
}

uint8_t nes_vt02_vt03_soc_device::psg1_4015_r()
{
	return m_apu->read(0x15);
}

void nes_vt02_vt03_soc_device::psg1_4015_w(uint8_t data)
{
	m_apu->write(0x15, data);
}

void nes_vt02_vt03_soc_device::psg1_4017_w(uint8_t data)
{
	m_apu->write(0x17, data);
}

// early units (VT03?) have a DMA bug in NTSC mode
void nes_vt02_vt03_soc_device::vt_dma_w(uint8_t data)
{
	if (!m_force_baddma)
		do_dma(data, true);
	else
		do_dma(data, false);
}



void nes_vt02_vt03_soc_device::do_dma(uint8_t data, bool has_ntsc_bug)
{
	// only NTSC systems have 'broken' DMA which requires the DMA addresses to be shifted by 1, PAL systems work as expected
	if (m_ppu->get_is_pal())
		has_ntsc_bug = false;

	uint8_t dma_mode = m_vdma_ctrl & 0x01;
	uint8_t dma_len = (m_vdma_ctrl >> 1) & 0x07;
	uint8_t src_nib_74 = (m_vdma_ctrl >> 4) & 0x0F;

	int length = 256;
	switch (dma_len)
	{
	case 0x0: length = 256; break;
	case 0x4: length = 16; break;
	case 0x5: length = 32; break;
	case 0x6: length = 64; break;
	case 0x7: length = 128; break;
	}

	uint16_t src_addr = (data << 8) | (src_nib_74 << 4);
	logerror("vthh dma start ctrl=%02x addr=%04x\n", m_vdma_ctrl, src_addr);

	if (dma_mode == 1)
	{
		logerror("vdma dest %04x\n", m_ppu->get_vram_dest());
	}

	if (has_ntsc_bug && (dma_mode == 1) && ((m_ppu->get_vram_dest() & 0xFF00) == 0x3F00) && !(m_ppu->get_201x_reg(0x1) & 0x80))
	{
		length -= 1;
		src_addr += 1;
	}

	for (int i = 0; i < length; i++)
	{
		uint8_t spriteData = m_maincpu->space(AS_PROGRAM).read_byte(src_addr + i);
		if (dma_mode)
		{
			m_maincpu->space(AS_PROGRAM).write_byte(0x2007, spriteData);
		}
		else
		{
			m_maincpu->space(AS_PROGRAM).write_byte(0x2004, spriteData);
		}
		//if(((src_addr + i) & 0xFF) == length && (i != 0)) break;
	}

	// should last (length * 4 - 1) CPU cycles.
	//((device_t*)m_maincpu)->execute().adjust_icount(-(length * 4 - 1));
}


void nes_vt02_vt03_soc_device::vt03_4034_w(uint8_t data)
{
	logerror("vt03_4034_w %02x (2nd APU DMA)\n", data);
	m_vdma_ctrl = data;
}

uint8_t nes_vt02_vt03_soc_device::in0_r()
{
	return m_read_0_callback();
}

uint8_t nes_vt02_vt03_soc_device::in1_r()
{
	return m_read_1_callback();
}

void nes_vt02_vt03_soc_device::in0_w(offs_t offset, uint8_t data)
{
	m_write_0_callback(offset, data);
}

void nes_vt02_vt03_soc_device::extra_io_control_w(uint8_t data)
{
	/*
	410d Extra I/O control

	0x01 Extra I/O port 0 mode (1 = output, 0 = input)
	0x02 Extra I/O port 0 enable (1 = enable, 0 = disable)
	0x04 Extra I/O port 1 mode (1 = output, 0 = input)
	0x08 Extra I/O port 1 enable (1 = enable, 0 = disable)
	0x10 Extra I/O port 2 mode (1 = output, 0 = input)
	0x20 Extra I/O port 2 enable (1 = enable, 0 = disable)
	0x40 Extra I/O port 3 mode (1 = output, 0 = input)
	0x80 Extra I/O port 3 enable (1 = enable, 0 = disable)
	*/

	logerror("%s: extra_io_control_w %02x\n", machine().describe_context(), data);
}

uint8_t nes_vt02_vt03_soc_device::extrain_01_r()
{
	// TODO: check status of 410d port to make sure we only read from enabled ports
	uint8_t in0 = 0x00, in1 = 0x00;

	in0 = m_extra_read_0_callback() & 0x0f;
	in1 = m_extra_read_1_callback() & 0x0f;

	return in0 | (in1<<4);
}

uint8_t nes_vt02_vt03_soc_device::extrain_23_r()
{
	// TODO: check status of 410d port to make sure we only read from enabled ports
	uint8_t in2 = 0x00, in3 = 0x00;

	in2 = m_extra_read_2_callback() & 0x0f;
	in3 = m_extra_read_3_callback() & 0x0f;

	return in2 | (in3<<4);
}

void nes_vt02_vt03_soc_device::extraout_01_w(uint8_t data)
{
	// TODO: use callbacks for this as output can be hooked up to anything
	logerror("%s: extraout_01_w %02x\n", machine().describe_context(), data);
}

void nes_vt02_vt03_soc_device::extraout_23_w(uint8_t data)
{
	// TODO: use callbacks for this as output can be hooked up to anything
	logerror("%s: extraout_23_w %02x\n", machine().describe_context(), data);
}

uint8_t nes_vt02_vt03_soc_device::rs232flags_region_r()
{
	/*
	0x4119 RS232 Flags + Region

	0x01 - RX bit 8
	0x02 - RERFF (error status)
	0x04 - unused
	0x08 - XPORN (PAL = 1 NTSC = 0)
	0x10 - XF5OR6 (50hz = 1 60hz = 0)
	0x20 - RINGF (receive status)
	0x40 - TIFLAG (completed sending data status)
	0x80 - RIFLAG (completed receiving data status)
	*/
	uint8_t ret = 0x00;

	// Palette DMA is buggy on NTSC systems (at least for regular VT03)
	// so the palette DMA writes will change based on the reading of these flags

	ret |= m_ppu->get_is_pal() ? 0x08 : 0x00;
	ret |= m_ppu->get_is_50hz() ? 0x10 : 0x00;

	return ret;
}


uint8_t nes_vt02_vt03_soc_device::external_space_read(offs_t offset)
{
	address_space& spc = this->space(AS_PROGRAM);
	int bank = (offset & 0x6000) >> 13;
	int address = (m_bankaddr[bank] * 0x2000) + (offset & 0x1fff);
	return spc.read_byte(address);
}

void nes_vt02_vt03_soc_device::external_space_write(offs_t offset, uint8_t data)
{
	if ((m_410x[0xb] & 0x08))
	{
		address_space& spc = this->space(AS_PROGRAM);
		int bank = (offset & 0x6000) >> 13;
		int address = (m_bankaddr[bank] * 0x2000) + (offset & 0x1fff);
		spc.write_byte(address, data);
	}
	else
	{
		vt03_8000_mapper_w(offset, data);
	}
};

void nes_vt02_vt03_soc_device::nes_vt_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().mirror(0x1800); // zudugo relies on mirror when selecting 'game' menu

	// ddrdismx relies on the mirroring
	map(0x2000, 0x2007).mirror(0x00e0).rw(m_ppu, FUNC(ppu2c0x_device::read), FUNC(ppu2c0x_device::write));                      // standard PPU registers
	map(0x2010, 0x201f).mirror(0x00e0).rw(m_ppu, FUNC(ppu_vt03_device::read_extended), FUNC(ppu_vt03_device::write_extended));  //  extra VT PPU registers

	map(0x4000, 0x4013).rw(m_apu, FUNC(nesapu_device::read), FUNC(nesapu_device::write));

	map(0x4014, 0x4014).r(FUNC(nes_vt02_vt03_soc_device::psg1_4014_r)).w(FUNC(nes_vt02_vt03_soc_device::vt_dma_w));
	map(0x4015, 0x4015).rw(FUNC(nes_vt02_vt03_soc_device::psg1_4015_r), FUNC(nes_vt02_vt03_soc_device::psg1_4015_w)); // PSG status / first control register
	map(0x4016, 0x4016).rw(FUNC(nes_vt02_vt03_soc_device::in0_r), FUNC(nes_vt02_vt03_soc_device::in0_w));
	map(0x4017, 0x4017).r(FUNC(nes_vt02_vt03_soc_device::in1_r)).w(FUNC(nes_vt02_vt03_soc_device::psg1_4017_w));


	map(0x4034, 0x4034).w(FUNC(nes_vt02_vt03_soc_device::vt03_4034_w)); // secondary DMA

	map(0x4100, 0x410b).r(FUNC(nes_vt02_vt03_soc_device::vt03_410x_r)).w(FUNC(nes_vt02_vt03_soc_device::vt03_410x_w));
	// 0x410c unused
	map(0x410d, 0x410d).w(FUNC(nes_vt02_vt03_soc_device::extra_io_control_w));
	map(0x410e, 0x410e).rw(FUNC(nes_vt02_vt03_soc_device::extrain_01_r), FUNC(nes_vt02_vt03_soc_device::extraout_01_w));
	map(0x410f, 0x410f).rw(FUNC(nes_vt02_vt03_soc_device::extrain_23_r), FUNC(nes_vt02_vt03_soc_device::extraout_23_w));
	// 0x4114 RS232 timer (low)
	// 0x4115 RS232 timer (high)
	// 0x4116 unused
	// 0x4117 unused
	// 0x4118 unused
	map(0x4119, 0x4119).r(FUNC(nes_vt02_vt03_soc_device::rs232flags_region_r));
	// 0x411a RS232 TX data
	// 0x411b RS232 RX data


	map(0x8000, 0xffff).rw(FUNC(nes_vt02_vt03_soc_device::external_space_read), FUNC(nes_vt02_vt03_soc_device::external_space_write));
	map(0x6000, 0x7fff).ram();
}



WRITE_LINE_MEMBER(nes_vt02_vt03_soc_device::apu_irq)
{
	// TODO
//  set_input_line(N2A03_APU_IRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

uint8_t nes_vt02_vt03_soc_device::apu_read_mem(offs_t offset)
{
	// TODO
	return 0x00;//mintf->program->read_byte(offset);
}

uint32_t nes_vt02_vt03_soc_device::screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect)
{
	return m_ppu->screen_update(screen, bitmap, cliprect);
}


device_memory_interface::space_config_vector nes_vt02_vt03_soc_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_space_config)
	};
}

void nes_vt02_vt03_soc_device::do_pal_timings_and_ppu_replacement(machine_config& config)
{
	m_maincpu->set_clock(PALC_APU_CLOCK);

	PPU_VT03PAL(config.replace(), m_ppu, N2A03_PAL_XTAL);
	m_ppu->set_cpu_tag(m_maincpu);
	m_ppu->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_ppu->read_bg().set(FUNC(nes_vt02_vt03_soc_device::chr_r));
	m_ppu->read_sp().set(FUNC(nes_vt02_vt03_soc_device::spr_r));
	m_ppu->set_screen(m_screen);

	m_screen->set_refresh_hz(50.0070);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC((113.66 / (PALC_APU_CLOCK.dvalue() / 1000000)) *
		(ppu2c0x_device::VBLANK_LAST_SCANLINE_PAL - ppu2c0x_device::VBLANK_FIRST_SCANLINE_PALC + 1 + 2)));
	m_screen->set_size(32 * 8, 312);
	m_screen->set_visarea(0 * 8, 32 * 8 - 1, 0 * 8, 30 * 8 - 1);
}


void nes_vt02_vt03_soc_device::device_add_mconfig(machine_config &config)
{
	N2A03_CORE(config, m_maincpu, NTSC_APU_CLOCK); // Butterfly Catch in vgpocket confirms N2A03 core type, not 6502
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vt02_vt03_soc_device::nes_vt_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60.0988);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC((113.66/(NTSC_APU_CLOCK.dvalue()/1000000)) *
							 (ppu2c0x_device::VBLANK_LAST_SCANLINE_NTSC-ppu2c0x_device::VBLANK_FIRST_SCANLINE+1+2)));
	m_screen->set_size(32*8, 262);
	m_screen->set_visarea(0*8, 32*8-1, 0*8, 30*8-1);
	m_screen->set_screen_update(FUNC(nes_vt02_vt03_soc_device::screen_update));

	PPU_VT03(config, m_ppu, N2A03_NTSC_XTAL);
	m_ppu->set_cpu_tag(m_maincpu);
	m_ppu->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_ppu->read_bg().set(FUNC(nes_vt02_vt03_soc_device::chr_r));
	m_ppu->read_sp().set(FUNC(nes_vt02_vt03_soc_device::spr_r));
	m_ppu->set_screen(m_screen);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	NES_APU_VT(config, m_apu, NTSC_APU_CLOCK);
	m_apu->irq().set(FUNC(nes_vt02_vt03_soc_device::apu_irq));
	m_apu->mem_read().set(FUNC(nes_vt02_vt03_soc_device::apu_read_mem));
	m_apu->add_route(ALL_OUTPUTS, "mono", 0.50);
}

void nes_vt02_vt03_soc_pal_device::device_add_mconfig(machine_config& config)
{
	nes_vt02_vt03_soc_device::device_add_mconfig(config);
	do_pal_timings_and_ppu_replacement(config);
}


/***********************************************************************************************************************************************************/
/* 'Scramble' specifics */
/***********************************************************************************************************************************************************/

void nes_vt02_vt03_soc_scramble_device::device_add_mconfig(machine_config& config)
{
	nes_vt02_vt03_soc_device::device_add_mconfig(config);

	N2A03_CORE_SWAP_OP_D5_D6(config.replace(), m_maincpu, NTSC_APU_CLOCK); // Insect Chase in polmega confirms N2A03 core type, not 6502
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vt02_vt03_soc_scramble_device::nes_vt_map);
}
