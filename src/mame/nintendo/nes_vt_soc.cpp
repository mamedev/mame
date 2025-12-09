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

#include "m6502_swap_op_d5_d6.h"

#include "speaker.h"


DEFINE_DEVICE_TYPE(NES_VT02_VT03_SOC,              nes_vt02_vt03_soc_device,              "nes_vt02_vt03_soc",             "VT02/03 series System on a Chip (NTSC)")
DEFINE_DEVICE_TYPE(NES_VT02_VT03_SOC_PAL,          nes_vt02_vt03_soc_pal_device,          "nes_vt02_vt03_soc_pal",         "VT02/03 series System on a Chip (PAL)")

DEFINE_DEVICE_TYPE(NES_VT02_VT03_SOC_WAIXING,      nes_vt02_vt03_soc_waixing_device,      "nes_vt02_vt03_soc_waixing",     "VT02/03 series System on a Chip (Waixing, NTSC)")
DEFINE_DEVICE_TYPE(NES_VT02_VT03_SOC_WAIXING_PAL,  nes_vt02_vt03_soc_waixing_pal_device,  "nes_vt02_vt03_soc_waixing_pal", "VT02/03 series System on a Chip (Waixing, PAL)")

DEFINE_DEVICE_TYPE(NES_VT02_VT03_SOC_HUMMER,       nes_vt02_vt03_soc_hummer_device,       "nes_vt02_vt03_soc_hummer",      "VT02/03 series System on a Chip (Hummer, NTSC)")

DEFINE_DEVICE_TYPE(NES_VT02_VT03_SOC_SPORTS,       nes_vt02_vt03_soc_sports_device,       "nes_vt02_vt03_soc_sports",      "VT02/03 series System on a Chip (Sports, NTSC)")
DEFINE_DEVICE_TYPE(NES_VT02_VT03_SOC_SPORTS_PAL,   nes_vt02_vt03_soc_sports_pal_device,   "nes_vt02_vt03_soc_sports_pal",  "VT02/03 series System on a Chip (Sports, PAL)")

DEFINE_DEVICE_TYPE(NES_VT02_VT03_SOC_SCRAMBLE,     nes_vt02_vt03_soc_scramble_device,     "nes_vt02_vt03_soc_scram",       "VT02/03 series System on a Chip (NTSC, with simple Opcode scrambling)")
DEFINE_DEVICE_TYPE(NES_VT02_VT03_SOC_SCRAMBLE_PAL, nes_vt02_vt03_soc_scramble_pal_device, "nes_vt02_vt03_soc_pal_scram",   "VT02/03 series System on a Chip (PAL, with simple Opcode scrambling)")

void nes_vt02_vt03_soc_device::program_map(address_map &map)
{
}

nes_vt02_vt03_soc_device::nes_vt02_vt03_soc_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_maincpu(*this, "maincpu"),
	m_screen(*this, "screen"),
	m_ppu(*this, "ppu"),
	m_apu(*this, "apu"),
	m_initial_e000_bank(0xff),
	m_ntram(nullptr),
	m_chrram(nullptr),
	m_4150_write_cb(*this),
	m_411e_write_cb(*this),
	m_41e6_write_cb(*this),
	m_space_config("program", ENDIANNESS_LITTLE, 8, 25, 0, address_map_constructor(FUNC(nes_vt02_vt03_soc_device::program_map), this)),
	m_write_0_callback(*this),
	m_read_0_callback(*this, 0xff),
	m_read_1_callback(*this, 0xff),
	m_extra_write_0_callback(*this),
	m_extra_write_1_callback(*this),
	m_extra_write_2_callback(*this),
	m_extra_write_3_callback(*this),
	m_extra_read_0_callback(*this, 0xff),
	m_extra_read_1_callback(*this, 0xff),
	m_extra_read_2_callback(*this, 0xff),
	m_extra_read_3_callback(*this, 0xff)
{
	// 'no scramble' configuration
	m_8000_scramble[0x0] = 0x4;
	m_8000_scramble[0x1] = 0x5;
	m_8000_scramble[0x2] = 0x0;
	m_8000_scramble[0x3] = 0x1;
	m_8000_scramble[0x4] = 0x2;
	m_8000_scramble[0x5] = 0x3;

	// not for PPU?
	m_8006_scramble[0] = 0x7;
	m_8006_scramble[1] = 0x8;

	// 'no scramble' configuration
	m_410x_scramble[0x0] = 0x7;
	m_410x_scramble[0x1] = 0x8;

	m_default_palette_mode = PAL_MODE_VT0x;
	m_force_baddma = false;
	m_use_raster_timing_hack = false;
}

nes_vt02_vt03_soc_device::nes_vt02_vt03_soc_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock) :
	nes_vt02_vt03_soc_device(mconfig, NES_VT02_VT03_SOC, tag, owner, clock)
{
}

nes_vt02_vt03_soc_pal_device::nes_vt02_vt03_soc_pal_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock) :
	nes_vt02_vt03_soc_device(mconfig, NES_VT02_VT03_SOC_PAL, tag, owner, clock)
{
}



nes_vt02_vt03_soc_waixing_device::nes_vt02_vt03_soc_waixing_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, u32 clock) :
	nes_vt02_vt03_soc_device(mconfig, type, tag, owner, clock)
{
}

nes_vt02_vt03_soc_waixing_device::nes_vt02_vt03_soc_waixing_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock) :
	nes_vt02_vt03_soc_waixing_device(mconfig, NES_VT02_VT03_SOC_WAIXING, tag, owner, clock)
{
}

nes_vt02_vt03_soc_waixing_pal_device::nes_vt02_vt03_soc_waixing_pal_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock) :
	nes_vt02_vt03_soc_waixing_device(mconfig, NES_VT02_VT03_SOC_WAIXING_PAL, tag, owner, clock)
{
}


nes_vt02_vt03_soc_hummer_device::nes_vt02_vt03_soc_hummer_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock) :
	nes_vt02_vt03_soc_device(mconfig, NES_VT02_VT03_SOC_HUMMER, tag, owner, clock)
{
}

nes_vt02_vt03_soc_sports_device::nes_vt02_vt03_soc_sports_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, u32 clock) :
	nes_vt02_vt03_soc_device(mconfig, type, tag, owner, clock)
{
}

nes_vt02_vt03_soc_sports_device::nes_vt02_vt03_soc_sports_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock) :
	nes_vt02_vt03_soc_sports_device(mconfig, NES_VT02_VT03_SOC_SPORTS, tag, owner, clock)
{
}

nes_vt02_vt03_soc_sports_pal_device::nes_vt02_vt03_soc_sports_pal_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock) :
	nes_vt02_vt03_soc_sports_device(mconfig, NES_VT02_VT03_SOC_SPORTS_PAL, tag, owner, clock)
{
}


nes_vt02_vt03_soc_scramble_device::nes_vt02_vt03_soc_scramble_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock) :
	nes_vt02_vt03_soc_device(mconfig, NES_VT02_VT03_SOC_SCRAMBLE, tag, owner, clock)
{
}

nes_vt02_vt03_soc_scramble_pal_device::nes_vt02_vt03_soc_scramble_pal_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock) :
	nes_vt02_vt03_soc_device(mconfig, NES_VT02_VT03_SOC_SCRAMBLE_PAL, tag, owner, clock)
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
	save_item(NAME(m_4024_newdma));
	save_item(NAME(m_relative));
	m_ntram = std::make_unique<u8[]>(0x2000);
	save_pointer(NAME(m_ntram), 0x2000);

	m_chrram = std::make_unique<u8[]>(0x2000);
	save_pointer(NAME(m_chrram), 0x2000);

	m_ppu->set_scanline_callback(*this, FUNC(nes_vt02_vt03_soc_device::scanline_irq));
	m_ppu->set_hblank_callback(*this, FUNC(nes_vt02_vt03_soc_device::hblank_irq));

	//m_ppu->set_hblank_callback(*m_cartslot->m_cart, FUNC(device_nes_cart_interface::hblank_irq)));
	//m_ppu->space(AS_PROGRAM).install_readwrite_handler(0, 0x1fff, read8sm_delegate(*m_cartslot->m_cart, FUNC(device_nes_cart_interface::chr_r)), write8sm_delegate(*m_cartslot->m_cart, FUNC(device_nes_cart_interface::chr_w)));
	m_ppu->space(AS_PROGRAM).install_readwrite_handler(0x2000, 0x3eff, read8sm_delegate(*this, FUNC(nes_vt02_vt03_soc_device::nt_r)), write8sm_delegate(*this, FUNC(nes_vt02_vt03_soc_device::nt_w)));
	m_ppu->space(AS_PROGRAM).install_readwrite_handler(0, 0x1fff, read8sm_delegate(*this, FUNC(nes_vt02_vt03_soc_device::chr_r)), write8sm_delegate(*this, FUNC(nes_vt02_vt03_soc_device::chr_w)));
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
	m_relative[0] = m_relative[1] = 0x00;

	m_4024_newdma = 0;
	m_vdma_ctrl = 0;

	m_timer_irq_enabled = 0;
	m_timer_running = 0;
	m_timer_val = 0;
	m_vdma_ctrl = 0;

	update_banks();

	m_ppu->set_palette_mode(m_default_palette_mode);

}

u32 nes_vt02_vt03_soc_device::get_banks(u8 bnk) const
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
	u8 bank;

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

u16 nes_vt02_vt03_soc_device::decode_nt_addr(u16 addr)
{
	bool vert_mirror = !(m_410x[0x6] & 0x01);
	int a11 = (addr >> 11) & 0x01;
	int a10 = (addr >> 10) & 0x01;
	u16 base = (addr & 0x3FF);
	return ((vert_mirror ? a10 : a11) << 10) | base;
}

void nes_vt02_vt03_soc_device::vt03_410x_w(offs_t offset, u8 data)
{
	scrambled_410x_w(offset, data);
}

u8 nes_vt02_vt03_soc_device::vt03_410x_r(offs_t offset)
{
	return m_410x[offset];
}


// Source: https://wiki.nesdev.com/w/index.php/NES_2.0_submappers/Proposals#NES_2.0_Mapper_256

void nes_vt02_vt03_soc_device::scrambled_410x_w(u16 offset, u8 data)
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



u8 nes_vt02_vt03_soc_device::spr_r(offs_t offset)
{
	int realaddr = calculate_real_video_address(offset, 1);

	address_space& spc = this->space(AS_PROGRAM);
	return spc.read_byte(get_relative() + realaddr);
}

u8 nes_vt02_vt03_soc_device::chr_r(offs_t offset)
{
	int realaddr = calculate_real_video_address(offset, 0);

	address_space& spc = this->space(AS_PROGRAM);
	return spc.read_byte(get_relative() + realaddr);
}


void nes_vt02_vt03_soc_device::chr_w(offs_t offset, u8 data)
{
	if (m_4242 & 0x1 || m_411d & 0x04) // newer VT platforms only (not VT03/09), split out
	{
		logerror("vram write %04x %02x\n", offset, data);
		m_chrram[offset] = data;
	}
	else
	{
		int realaddr = calculate_real_video_address(offset, 0);

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
					//logerror("scanline_irq %d\n", scanline);
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
u8 nes_vt02_vt03_soc_device::nt_r(offs_t offset)
{
	return m_ntram[decode_nt_addr(offset)];
}

void nes_vt02_vt03_soc_device::nt_w(offs_t offset, u8 data)
{
	//logerror("nt wr %04x %02x", offset, data);
	m_ntram[decode_nt_addr(offset)] = data;
}


int nes_vt02_vt03_soc_device::calculate_va17_va10(int addr)
{
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

	int sel = (addr & 0x1c00) | ((m_410x[0x5] & 0x80) ? 0x2000 : 0x000);

	int vbank_tva17_tva10 = 0x00;

	switch ((sel >> 10) & 0xf)
	{
	case 0x0:
	case 0x1:
	case 0xc:
	case 0xd:
		vbank_tva17_tva10 = (m_ppu->get_videobank0_reg(0x4) & 0xfe) | ((addr & 0x0400) ? 1 : 0);
		break;

	case 0x2:
	case 0x3:
	case 0xe:
	case 0xf:
		vbank_tva17_tva10 = (m_ppu->get_videobank0_reg(0x5) & 0xfe) | ((addr & 0x0400) ? 1 : 0);
		break;

	case 0x4:
	case 0x8:
		vbank_tva17_tva10 = m_ppu->get_videobank0_reg(0x0);
		break;

	case 0x5:
	case 0x9:
		vbank_tva17_tva10 = m_ppu->get_videobank0_reg(0x1);
		break;

	case 0x6:
	case 0xa:
		vbank_tva17_tva10 = m_ppu->get_videobank0_reg(0x2);
		break;

	case 0x7:
	case 0xb:
		vbank_tva17_tva10 = m_ppu->get_videobank0_reg(0x3);
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

	int swit = m_ppu->get_videobank0_extra();

	switch (swit & 0x07)
	{
	case 0x0: va17_va10 = vbank_tva17_tva10; break;
	case 0x1: va17_va10 = (vbank_tva17_tva10 & 0x7f) | (m_ppu->get_videobank0_extra() & 0x80); break;
	case 0x2: va17_va10 = (vbank_tva17_tva10 & 0x3f) | (m_ppu->get_videobank0_extra() & 0xc0); break;
	case 0x3: return -1;
	case 0x4: va17_va10 = (vbank_tva17_tva10 & 0x1f) | (m_ppu->get_videobank0_extra() & 0xe0); break;
	case 0x5: va17_va10 = (vbank_tva17_tva10 & 0x0f) | (m_ppu->get_videobank0_extra() & 0xf0); break;
	case 0x6: va17_va10 = (vbank_tva17_tva10 & 0x07) | (m_ppu->get_videobank0_extra() & 0xf8); break;
	case 0x7: return -1;
	}

	return va17_va10;
}


int nes_vt02_vt03_soc_device::calculate_real_video_address(int addr, int readtype)
{
	// this is what gets passed in
	//      00Pb bbtt tttt plll
	//  P = plane3/4 select (4bpp modes)
	//  p = plane0/1 select
	//  l = tile line
	//  b = tile number bits passed into banking
	//  t = tile number (0x1ff tile number bits total)
	int va34 = (addr & 0x6000) >> 13;
	addr &= 0x1fff;

	int va17_va10 = calculate_va17_va10(addr);

	// Adjust where we actually read from for special modes

	// might be a VT09 only feature (alt 4bpp mode?)
	int alt_order = m_ppu->get_extended_modes_enable() & 0x40;
	int extended = 0;
	if (readtype == 0) // character
	{
		if (m_ppu->get_extended_modes_enable() & 0x10)
		{
			extended = 1;
		}
	}
	else // sprite
	{
		if (m_ppu->get_extended_modes_enable() & 0x08)
		{
			extended = 1;
		}
	}

	int finaladdr = 0;

	if (!extended)
	{
		int is4bpp = 0;
		if (readtype == 0) is4bpp = m_ppu->get_extended_modes_enable() & 0x02;
		else if (readtype == 1) is4bpp = m_ppu->get_extended_modes_enable() & 0x04;

		int va20_va18 = (m_ppu->get_videobank1() & 0x70) >> 4;

		finaladdr = ((m_410x[0x0] & 0x0F) << 21) | (va20_va18 << 18) | (va17_va10 << 10) | (addr & 0x03ff);

		if (is4bpp)
		{
			if (!alt_order)
			{
				finaladdr = ((finaladdr & ~0xf) << 1) | ((va34 & 1) << 4) | (finaladdr & 0xf);
			}
			else
			{
				finaladdr = (finaladdr << 1) | (va34 & 1);
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
			is4bpp = m_ppu->get_extended_modes_enable() & 0x02;

			eva2_eva0 |= m_ppu->get_m_read_bg4_bg3();

			if (m_ppu->get_extended_modes2_enable() & 0x02)
			{
				if (m_410x[0x6] & 0x1) eva2_eva0 |= 0x4;
			}
			else
			{
				if (m_ppu->get_videobank1() & 0x08) eva2_eva0 |= 0x4;
			}
			break;

		case 1: // sprite display
			is4bpp = m_ppu->get_extended_modes_enable() & 0x04; // 16 colors or 16-pixel wide (both adjust the read)

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
				finaladdr = ((finaladdr & ~0xf) << 1) | ((va34 & 1) << 4) | (finaladdr & 0xf);
			}
			else
			{
				finaladdr = (finaladdr << 1) | (va34 & 1);
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

void nes_vt02_vt03_soc_device::scrambled_8000_w(u16 offset, u8 data)
{
	offset &= 0x7fff;
	u16 addr = offset+0x8000;

	//MMC3 compat
	if ((addr < 0xa000) && !(addr & 0x01))
	{
		logerror("%s: scrambled_8000_w real address: (%04x) translated address: (%04x) %02x (banking)\n",  machine().describe_context(), addr, offset + 0x8000, data);
		// Bank select
		m_8000_addr_latch = data & 0x07;
		// Bank config
		m_410x[0x05] = data & ~(1 << 5);
		update_banks();
	}
	else if ((addr < 0xa000) && (addr & 0x01))
	{
		logerror("%s: scrambled_8000_w real address: (%04x) translated address: (%04x) %02x (other scrambled stuff)\n",  machine().describe_context(), addr, offset + 0x8000, data);

		switch (m_410x[0x05] & 0x07)
		{
		case 0x00:
			m_ppu->set_videobank0_reg(m_8000_scramble[0], data);
			break;

		case 0x01:
			m_ppu->set_videobank0_reg(m_8000_scramble[1], data);
			break;

		case 0x02: // hand?
			m_ppu->set_videobank0_reg(m_8000_scramble[2], data);
			break;

		case 0x03: // dog?
			m_ppu->set_videobank0_reg(m_8000_scramble[3], data);
			break;

		case 0x04: // ball thrown
			m_ppu->set_videobank0_reg(m_8000_scramble[4], data);
			break;

		case 0x05: // ball thrown
			m_ppu->set_videobank0_reg(m_8000_scramble[5], data);
			break;

		case 0x06:
			m_410x[m_8006_scramble[0]] = data;
			//m_410x[0x9] = data;
			update_banks();
			break;

		case 0x07:
			m_410x[m_8006_scramble[1]] = data;
			update_banks();
			break;
		}
	}
	else if ((addr >= 0xa000) && (addr < 0xc000) && !(addr & 0x01))
	{
		// Mirroring
		m_410x[0x6] &= 0xfe;
		m_410x[0x6] |= data & 0x01;
	}
	else if ((addr >= 0xa000) && (addr < 0xc000) && (addr & 0x01))
	{
		// PRG RAM control, ignore
	}
	else if ((addr >= 0xc000) && (addr < 0xe000) && !(addr & 0x01))
	{
		// IRQ latch
		vt03_410x_w(1, data);
	}
	else if ((addr >= 0xc000) && (addr < 0xe000) && (addr & 0x01))
	{
		// IRQ reload
		vt03_410x_w(2, data);
	}
	else if ((addr >= 0xe000) && !(addr & 0x01))
	{
		// IRQ disable
		vt03_410x_w(3, data);
	}
	else if ((addr >= 0xe000) && (addr & 0x01))
	{
		// IRQ enable
		vt03_410x_w(4, data);
	}
	else
	{

	}
}

// MMC3 compatibility mode

void nes_vt02_vt03_soc_device::set_8000_scramble(u8 reg0, u8 reg1, u8 reg2, u8 reg3, u8 reg4, u8 reg5)
{
	m_8000_scramble[0] = reg0; // TODO: name the regs
	m_8000_scramble[1] = reg1;
	m_8000_scramble[2] = reg2;
	m_8000_scramble[3] = reg3;
	m_8000_scramble[4] = reg4;
	m_8000_scramble[5] = reg5;

}

void nes_vt02_vt03_soc_device::set_8006_scramble(u8 reg6, u8 reg7)
{
	// not for PPU?
	m_8006_scramble[0] = reg6;
	m_8006_scramble[1] = reg7;
}


void nes_vt02_vt03_soc_device::set_410x_scramble(u8 reg0, u8 reg1)
{
	m_410x_scramble[0] = reg0; // TODO: name the regs
	m_410x_scramble[1] = reg1;
}

void nes_vt02_vt03_soc_device::vt03_8000_mapper_w(offs_t offset, u8 data)
{
	scrambled_8000_w(offset, data);
	//logerror("%s: vt03_8000_mapper_w (%04x) %02x\n", machine().describe_context(), offset+0x8000, data );
}

// early units (VT03?) have a DMA bug in NTSC mode
void nes_vt02_vt03_soc_device::vt_dma_w(u8 data)
{
	if (!m_force_baddma)
		do_dma(data, true);
	else
		do_dma(data, false);
}



void nes_vt02_vt03_soc_device::do_dma(u8 data, bool has_ntsc_bug)
{
	// only NTSC systems have 'broken' DMA which requires the DMA addresses to be shifted by 1, PAL systems work as expected
	if (m_ppu->get_is_pal())
		has_ntsc_bug = false;

	u8 dma_mode = m_vdma_ctrl & 0x01;
	u8 dma_len = (m_vdma_ctrl >> 1) & 0x07;
	u8 src_nib_74 = (m_vdma_ctrl >> 4) & 0x0F;

	int length = 256;
	switch (dma_len)
	{
	case 0x0: length = 256; break;
	case 0x4: length = 16; break;
	case 0x5: length = 32; break;
	case 0x6: length = 64; break;
	case 0x7: length = 128; break;
	}

	u16 src_addr = (data << 8) | (src_nib_74 << 4);
	logerror("%s: vthh dma start ctrl=%02x addr=%04x\n", machine().describe_context(), m_vdma_ctrl, src_addr);

	if (dma_mode == 1)
	{
		logerror("vdma dest %04x\n", m_ppu->get_vram_dest());
	}

	if (has_ntsc_bug && (dma_mode == 1) && ((m_ppu->get_vram_dest() & 0xFF00) == 0x3F00) && !(m_ppu->get_extended_modes2_enable() & 0x80))
	{
		length -= 1;
		src_addr += 1;
	}

	for (int i = 0; i < length; i++)
	{
		u8 spriteData = m_maincpu->space(AS_PROGRAM).read_byte(src_addr + i);
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

// probably VT3xx only, not earlier?
void nes_vt02_vt03_soc_device::vt3xx_4024_new_dma_middle_w(u8 data)
{
	logerror("%s: vt3xx_4024_new_dma_middle_w %02x (VT3xx newer DMA middle bits?)\n", machine().describe_context(), data);
	// can set all 8-bits of the lower address using this register
	m_4024_newdma = data;
}

void nes_vt02_vt03_soc_device::vt03_4034_w(u8 data)
{
	logerror("%s: vt03_4034_w %02x (2nd APU DMA / new DMA)\n", machine().describe_context(), data);
	m_vdma_ctrl = data;

	// this also sets the lower DMA address under certain conditions, but only 4 bits of it? - needed to stop denv150 corrupting
	// note, for regular DMA these bits are used (src_nib_74)
	// should we have a separate m_dmaaddress set by both this and 4024 and store the value written here as-is?
	// maybe this newvid_1d check could be moved to the DMA function to check which bits to use instead - check
	if ((m_ppu->get_newvid_1d() & 0x01) == 0x00)
	{
		m_4024_newdma = data & 0xf0;
	}
}

u8 nes_vt02_vt03_soc_device::in0_r()
{
	return m_read_0_callback();
}

u8 nes_vt02_vt03_soc_device::in1_r()
{
	return m_read_1_callback();
}

void nes_vt02_vt03_soc_device::in0_w(offs_t offset, u8 data)
{
	m_write_0_callback(offset, data);
}

void nes_vt02_vt03_soc_device::extra_io_control_w(u8 data)
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

u8 nes_vt02_vt03_soc_device::extrain_01_r()
{
	// TODO: check status of 410d port to make sure we only read from enabled ports
	u8 in0 = 0x00, in1 = 0x00;

	in0 = m_extra_read_0_callback() & 0x0f;
	in1 = m_extra_read_1_callback() & 0x0f;

	return in0 | (in1<<4);
}

u8 nes_vt02_vt03_soc_device::extrain_23_r()
{
	// TODO: check status of 410d port to make sure we only read from enabled ports
	u8 in2 = 0x00, in3 = 0x00;

	in2 = m_extra_read_2_callback() & 0x0f;
	in3 = m_extra_read_3_callback() & 0x0f;

	return in2 | (in3<<4);
}

void nes_vt02_vt03_soc_device::extraout_01_w(u8 data)
{
	// TODO: use callbacks for this as output can be hooked up to anything
	logerror("%s: extraout_01_w %02x\n", machine().describe_context(), data);
}

void nes_vt02_vt03_soc_device::extraout_23_w(u8 data)
{
	// TODO: use callbacks for this as output can be hooked up to anything
	logerror("%s: extraout_23_w %02x\n", machine().describe_context(), data);
}

u8 nes_vt02_vt03_soc_device::rs232flags_region_r()
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
	u8 ret = 0x00;

	// Palette DMA is buggy on NTSC systems (at least for regular VT03)
	// so the palette DMA writes will change based on the reading of these flags

	ret |= m_ppu->get_is_pal() ? 0x08 : 0x00;
	ret |= m_ppu->get_is_50hz() ? 0x10 : 0x00;

	return ret;
}


u8 nes_vt02_vt03_soc_device::external_space_read(offs_t offset)
{
	address_space& spc = this->space(AS_PROGRAM);
	int bank = (offset & 0x6000) >> 13;
	int relative = get_relative();
	int address = relative + (m_bankaddr[bank] * 0x2000) + (offset & 0x1fff);
	return spc.read_byte(address);
}

void nes_vt02_vt03_soc_device::external_space_write(offs_t offset, u8 data)
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

	// 2010 - 201f are extended regs, and can differ between VT models
	map(0x2010, 0x2010).mirror(0x00e0).rw(m_ppu, FUNC(ppu_vt03_device::extended_modes_enable_r), FUNC(ppu_vt03_device::extended_modes_enable_w));
	map(0x2011, 0x2011).mirror(0x00e0).rw(m_ppu, FUNC(ppu_vt03_device::extended_modes2_enable_r), FUNC(ppu_vt03_device::extended_modes2_enable_w));
	nes_vt_2012_to_2017_regs(map);// 2012 - 2017 map differently on some SoC types (re-ordered)
	map(0x2018, 0x2018).mirror(0x00e0).rw(m_ppu, FUNC(ppu_vt03_device::videobank1_r), FUNC(ppu_vt03_device::videobank1_w));
	map(0x2019, 0x2019).mirror(0x00e0).rw(m_ppu, FUNC(ppu_vt03_device::unk_2019_r), FUNC(ppu_vt03_device::gun_reset_w));
	map(0x201a, 0x201a).mirror(0x00e0).rw(m_ppu, FUNC(ppu_vt03_device::videobank0_extra_r), FUNC(ppu_vt03_device::videobank0_extra_w));
	map(0x201b, 0x201b).mirror(0x00e0).r(m_ppu, FUNC(ppu_vt03_device::unk_201b_r));
	map(0x201c, 0x201c).mirror(0x00e0).r(m_ppu, FUNC(ppu_vt03_device::gun_x_r));
	map(0x201d, 0x201d).mirror(0x00e0).r(m_ppu, FUNC(ppu_vt03_device::gun_y_r));
	map(0x201e, 0x201e).mirror(0x00e0).r(m_ppu, FUNC(ppu_vt03_device::gun2_x_r));
	map(0x201f, 0x201f).mirror(0x00e0).r(m_ppu, FUNC(ppu_vt03_device::gun2_y_r));

	map(0x4000, 0x4017).w(m_apu, FUNC(nes_apu_vt_device::write));
	map(0x4014, 0x4014).w(FUNC(nes_vt02_vt03_soc_device::vt_dma_w));
	map(0x4015, 0x4015).r(m_apu, FUNC(nes_apu_vt_device::status_r)); // PSG status / first control register
	map(0x4016, 0x4016).rw(FUNC(nes_vt02_vt03_soc_device::in0_r), FUNC(nes_vt02_vt03_soc_device::in0_w));
	map(0x4017, 0x4017).r(FUNC(nes_vt02_vt03_soc_device::in1_r));

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

void nes_vt02_vt03_soc_device::nes_vt_2012_to_2017_regs(address_map &map)
{
	map(0x2012, 0x2012).mirror(0x00e0).rw(m_ppu, FUNC(ppu_vt03_device::videobank0_0_r), FUNC(ppu_vt03_device::videobank0_0_w));
	map(0x2013, 0x2013).mirror(0x00e0).rw(m_ppu, FUNC(ppu_vt03_device::videobank0_1_r), FUNC(ppu_vt03_device::videobank0_1_w));
	map(0x2014, 0x2014).mirror(0x00e0).rw(m_ppu, FUNC(ppu_vt03_device::videobank0_2_r), FUNC(ppu_vt03_device::videobank0_2_w));
	map(0x2015, 0x2015).mirror(0x00e0).rw(m_ppu, FUNC(ppu_vt03_device::videobank0_3_r), FUNC(ppu_vt03_device::videobank0_3_w));
	map(0x2016, 0x2016).mirror(0x00e0).rw(m_ppu, FUNC(ppu_vt03_device::videobank0_4_r), FUNC(ppu_vt03_device::videobank0_4_w));
	map(0x2017, 0x2017).mirror(0x00e0).rw(m_ppu, FUNC(ppu_vt03_device::videobank0_5_r), FUNC(ppu_vt03_device::videobank0_5_w));
}

void nes_vt02_vt03_soc_waixing_device::nes_vt_2012_to_2017_regs(address_map& map)
{
	map(0x2012, 0x2012).mirror(0x00e0).rw(m_ppu, FUNC(ppu_vt03_device::videobank0_1_r), FUNC(ppu_vt03_device::videobank0_1_w));
	map(0x2013, 0x2013).mirror(0x00e0).rw(m_ppu, FUNC(ppu_vt03_device::videobank0_0_r), FUNC(ppu_vt03_device::videobank0_0_w));
	map(0x2014, 0x2014).mirror(0x00e0).rw(m_ppu, FUNC(ppu_vt03_device::videobank0_5_r), FUNC(ppu_vt03_device::videobank0_5_w));
	map(0x2015, 0x2015).mirror(0x00e0).rw(m_ppu, FUNC(ppu_vt03_device::videobank0_4_r), FUNC(ppu_vt03_device::videobank0_4_w));
	map(0x2016, 0x2016).mirror(0x00e0).rw(m_ppu, FUNC(ppu_vt03_device::videobank0_3_r), FUNC(ppu_vt03_device::videobank0_3_w));
	map(0x2017, 0x2017).mirror(0x00e0).rw(m_ppu, FUNC(ppu_vt03_device::videobank0_2_r), FUNC(ppu_vt03_device::videobank0_2_w));
}

void nes_vt02_vt03_soc_hummer_device::nes_vt_2012_to_2017_regs(address_map& map)
{
	map(0x2012, 0x2012).mirror(0x00e0).rw(m_ppu, FUNC(ppu_vt03_device::videobank0_5_r), FUNC(ppu_vt03_device::videobank0_5_w));
	map(0x2013, 0x2013).mirror(0x00e0).rw(m_ppu, FUNC(ppu_vt03_device::videobank0_4_r), FUNC(ppu_vt03_device::videobank0_4_w));
	map(0x2014, 0x2014).mirror(0x00e0).rw(m_ppu, FUNC(ppu_vt03_device::videobank0_3_r), FUNC(ppu_vt03_device::videobank0_3_w));
	map(0x2015, 0x2015).mirror(0x00e0).rw(m_ppu, FUNC(ppu_vt03_device::videobank0_2_r), FUNC(ppu_vt03_device::videobank0_2_w));
	map(0x2016, 0x2016).mirror(0x00e0).rw(m_ppu, FUNC(ppu_vt03_device::videobank0_0_r), FUNC(ppu_vt03_device::videobank0_0_w));
	map(0x2017, 0x2017).mirror(0x00e0).rw(m_ppu, FUNC(ppu_vt03_device::videobank0_1_r), FUNC(ppu_vt03_device::videobank0_1_w));
}

void nes_vt02_vt03_soc_sports_device::nes_vt_2012_to_2017_regs(address_map& map)
{
	map(0x2012, 0x2012).mirror(0x00e0).rw(m_ppu, FUNC(ppu_vt03_device::videobank0_2_r), FUNC(ppu_vt03_device::videobank0_2_w));
	map(0x2013, 0x2013).mirror(0x00e0).rw(m_ppu, FUNC(ppu_vt03_device::videobank0_5_r), FUNC(ppu_vt03_device::videobank0_5_w));
	map(0x2014, 0x2014).mirror(0x00e0).rw(m_ppu, FUNC(ppu_vt03_device::videobank0_0_r), FUNC(ppu_vt03_device::videobank0_0_w));
	map(0x2015, 0x2015).mirror(0x00e0).rw(m_ppu, FUNC(ppu_vt03_device::videobank0_4_r), FUNC(ppu_vt03_device::videobank0_4_w));
	map(0x2016, 0x2016).mirror(0x00e0).rw(m_ppu, FUNC(ppu_vt03_device::videobank0_3_r), FUNC(ppu_vt03_device::videobank0_3_w));
	map(0x2017, 0x2017).mirror(0x00e0).rw(m_ppu, FUNC(ppu_vt03_device::videobank0_1_r), FUNC(ppu_vt03_device::videobank0_1_w));
}



void nes_vt02_vt03_soc_device::apu_irq(int state)
{
	// TODO
//  set_input_line(RP2A03_APU_IRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

u8 nes_vt02_vt03_soc_device::apu_read_mem(offs_t offset)
{
	// TODO
	return 0x00;//mintf->program->read_byte(offset);
}

u32 nes_vt02_vt03_soc_device::screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect)
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

	PPU_VT03PAL(config.replace(), m_ppu, RP2A03_PAL_XTAL);
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
	RP2A03_CORE(config, m_maincpu, NTSC_APU_CLOCK); // Butterfly Catch in vgpocket confirms RP2A03 core type, not 6502
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vt02_vt03_soc_device::nes_vt_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60.0988);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC((113.66/(NTSC_APU_CLOCK.dvalue()/1000000)) *
							 (ppu2c0x_device::VBLANK_LAST_SCANLINE_NTSC-ppu2c0x_device::VBLANK_FIRST_SCANLINE+1+2)));
	m_screen->set_size(32*8, 262);
	m_screen->set_visarea(0*8, 32*8-1, 0*8, 30*8-1);
	m_screen->set_screen_update(FUNC(nes_vt02_vt03_soc_device::screen_update));

	PPU_VT03(config, m_ppu, RP2A03_NTSC_XTAL);
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


void nes_vt02_vt03_soc_waixing_pal_device::device_add_mconfig(machine_config& config)
{
	nes_vt02_vt03_soc_waixing_device::device_add_mconfig(config);
	do_pal_timings_and_ppu_replacement(config);
}

void nes_vt02_vt03_soc_sports_pal_device::device_add_mconfig(machine_config& config)
{
	nes_vt02_vt03_soc_sports_device::device_add_mconfig(config);
	do_pal_timings_and_ppu_replacement(config);
}

/***********************************************************************************************************************************************************/
/* 'Scramble' specifics */
/***********************************************************************************************************************************************************/

void nes_vt02_vt03_soc_scramble_device::device_add_mconfig(machine_config& config)
{
	nes_vt02_vt03_soc_device::device_add_mconfig(config);

	RP2A03_CORE_SWAP_OP_D5_D6(config.replace(), m_maincpu, NTSC_APU_CLOCK); // Insect Chase in polmega confirms RP2A03 core type, not 6502
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vt02_vt03_soc_scramble_device::nes_vt_map);
}

void nes_vt02_vt03_soc_scramble_pal_device::device_add_mconfig(machine_config& config)
{
	nes_vt02_vt03_soc_device::device_add_mconfig(config);

	RP2A03_CORE_SWAP_OP_D5_D6(config.replace(), m_maincpu, PAL_APU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vt02_vt03_soc_scramble_pal_device::nes_vt_map);
	do_pal_timings_and_ppu_replacement(config);
}
