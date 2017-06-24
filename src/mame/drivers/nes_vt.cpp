// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

  nes_vt.c

  The 'VT' series are SoC solutions that implement enhanced NES hardware
  there are several generations of these chips each adding additional
  functionality.

  This list is incomplete

  VT01 - plain famiclone?
  VT02 - banking scheme to access 32MB, Dual APU with PCM support
  VT03 - above + 4bpp sprite / bg modes, enhanced palette

  VT08 - ?

  VT09 - 8bpp or direct colour modes?

  VT16 - ?
  VT18 - ?

  (more)

  VT1682 - NOT compatible with NES, different video system, sound CPU (4x
           main CPU clock), optional internal ROM etc. (will need it's own
           driver)

  todo (VT03):

  add support for basic NES PPU page mirroring (selectable with register)
  work out format of the 12-bit palette, it's meant to be
  4-bits saturation, 4-bits luminance, 4-bits phase
  as opposed to the basic NES format of 2-luminance, 4-phase
  but getting it correct is tricky:
  APU refactoring to allow for mostly doubled up functionality + PCM channel
  *more*

  todo (newer VTxx):

  everything

  todo (general)

  Add more Famiclone roms here, there should be plenty more dumps of VTxx
  based systems floating about.


  NON-bugs (same happens on real hardware)

  Pause screen has corrupt GFX on enhanced version of Octopus

***************************************************************************/

#include "emu.h"
#include "includes/nes.h"
#include "cpu/m6502/n2a03.h"
#include "machine/bankdev.h"
#include "video/ppu2c0x_vt.h"
#include "cpu/m6502/m6502.h"
#include "screen.h"
#include "speaker.h"

class nes_vt_state : public nes_base_state
{
public:
	nes_vt_state(const machine_config &mconfig, device_type type, const char *tag)
		: nes_base_state(mconfig, type, tag)
		, m_ppu(*this, "ppu")
		, m_apu(*this, "apu")
		, m_prg(*this, "prg")
		, m_prgbank0(*this, "prg_bank0")
		, m_prgbank1(*this, "prg_bank1")
		, m_prgbank2(*this, "prg_bank2")
		, m_prgbank3(*this, "prg_bank3")
		, m_prgrom(*this, "mainrom")
		{ }

	/* APU handling */
	DECLARE_WRITE_LINE_MEMBER(apu_irq);
	DECLARE_READ8_MEMBER(apu_read_mem);
	DECLARE_READ8_MEMBER(psg1_4014_r);
	DECLARE_READ8_MEMBER(psg1_4015_r);
	DECLARE_WRITE8_MEMBER(psg1_4015_w);
	DECLARE_WRITE8_MEMBER(psg1_4017_w);

	/* Misc PPU */
	DECLARE_WRITE8_MEMBER(nes_vh_sprite_dma_w);
	void ppu_nmi(int *ppu_regs);
	uint32_t screen_update_vt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_PALETTE_INIT(nesvt);

	/* VT03 extension handling */
	DECLARE_WRITE8_MEMBER(vt03_410x_w);
	DECLARE_WRITE8_MEMBER(vt03_8000_w);

	/* OneBus read callbacks for getting sprite and tile data during rendering*/
	DECLARE_READ8_MEMBER(spr_r);
	DECLARE_READ8_MEMBER(chr_r);

private:
	void scanline_irq(int scanline, int vblank, int blanked);
	uint8_t m_410x[0xc];

	int m_timer_irq_enabled;
	int m_timer_running;
	int m_timer_val;

	uint8_t m_8000_addr_latch;

	/* banking etc. */
	int m_romsize;
	int m_numbanks;
	uint32_t get_banks(uint8_t bnk);
	void update_banks();

	int calculate_real_video_address(int addr, int extended, int readtype);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_device<ppu_vt03_device> m_ppu;
	required_device<nesapu_device> m_apu;
	required_device<address_map_bank_device> m_prg;
	required_memory_bank m_prgbank0;
	required_memory_bank m_prgbank1;
	required_memory_bank m_prgbank2;
	required_memory_bank m_prgbank3;
	required_region_ptr<uint8_t> m_prgrom;
};

uint32_t nes_vt_state::get_banks(uint8_t bnk)
{
	switch (m_410x[0xb] & 0x07)
	{
	case 0: return ((m_410x[0x0] & 0xF0) << 4) | (m_410x[0xa] & 0xC0) | (bnk & 0x3F); // makes bank 0xff at 0xe000 map to 0x07e000 by default for vectors at 0x007fffx
	case 1: return ((m_410x[0x0] & 0xF0) << 4) | (m_410x[0xa] & 0xE0) | (bnk & 0x1F);
	case 2: return ((m_410x[0x0] & 0xF0) << 4) | (m_410x[0xa] & 0xF0) | (bnk & 0x0F);
	case 3: return ((m_410x[0x0] & 0xF0) << 4) | (m_410x[0xa] & 0xF8) | (bnk & 0x07);
	case 4: return ((m_410x[0x0] & 0xF0) << 4) | (m_410x[0xa] & 0xFC) | (bnk & 0x03);
	case 5: return ((m_410x[0x0] & 0xF0) << 4) | (m_410x[0xa] & 0xFE) | (bnk & 0x01);
	case 6: return ((m_410x[0x0] & 0xF0) << 4) | m_410x[0xa];
	case 7: return ((m_410x[0x0] & 0xF0) << 4) | bnk;
	}

	return 0;
}

// 8000 needs to bank in 60000  ( bank 0x30 )
void nes_vt_state::update_banks()
{
	uint8_t bank;

	// 8000-9fff
	if ((m_410x[0xb] & 0x40) == 0)
	{
		if ((m_410x[0x5] & 0x40) == 0)
			bank = m_410x[0x7];
		else
			bank = m_410x[0x9];
	}
	else
		bank = 0xfe;

	m_prgbank0->set_entry(get_banks(bank) & (m_numbanks-1));

	// a000-bfff
	bank = m_410x[0x8];
	m_prgbank1->set_entry(get_banks(bank) & (m_numbanks-1));

	// c000-dfff
	if ((m_410x[0xb] & 0x40) != 0)
	{
		if ((m_410x[0x5] & 0x40) == 0)
			bank = m_410x[0x9];
		else
			bank = m_410x[0x7];
	}
	else
		bank = 0xfe;

	m_prgbank2->set_entry(get_banks(bank) & (m_numbanks-1));

	// e000 - ffff
	bank = 0xff;
	m_prgbank3->set_entry(get_banks(bank) & (m_numbanks-1));
}

WRITE8_MEMBER(nes_vt_state::vt03_410x_w)
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
		// load latched value and start counting
		m_410x[0x2] = data; // value doesn't matter?
		m_timer_val = m_410x[0x1];
		m_timer_running = 1;
		break;

	case 0x3:
		// disable timer irq
		m_410x[0x3] = data; // value doesn't matter?
		m_timer_irq_enabled = 0;
		break;

	case 0x4:
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
		m_410x[0x7] = data;
		update_banks();
		break;

	case 0x8:
		m_410x[0x8] = data;
		update_banks();
		break;

	case 0x9:
		logerror("vt03_4109_w %02x\n", data);
		m_410x[0x9] = data;
		update_banks();
		break;

	case 0xa:
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


uint32_t nes_vt_state::screen_update_vt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// render the ppu
	m_ppu->render(bitmap, 0, 0, 0, 0);
	return 0;
}

READ8_MEMBER(nes_vt_state::spr_r)
{
	int realaddr = calculate_real_video_address(offset, 0, 1);
	return m_prgrom[realaddr & (m_romsize-1)];
}

READ8_MEMBER(nes_vt_state::chr_r)
{
	int realaddr = calculate_real_video_address(offset, 1, 0);
	return m_prgrom[realaddr &  (m_romsize-1)];
}

PALETTE_INIT_MEMBER(nes_vt_state, nesvt)
{
	m_ppu->init_palette(palette, 0);
}

void nes_vt_state::scanline_irq(int scanline, int vblank, int blanked)
{
	int irqstate = 0;

	//logerror("scanline_irq %d\n", scanline);

	if (m_timer_running && scanline < 0xe0)
	{
		m_timer_val--;

		if (m_timer_val < 0)
		{
			if (m_timer_irq_enabled)
			{
				irqstate = 1;
			}
		}
	}

	if (irqstate)
		m_maincpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
	else
		m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
}



void nes_vt_state::machine_start()
{
	m_romsize =  memregion("mainrom")->bytes();
	m_numbanks = m_romsize / 0x2000;

	m_prg->set_bank(0);

	m_prgbank0->configure_entries(0, m_numbanks, &m_prgrom[0x00000], 0x2000);
	m_prgbank1->configure_entries(0, m_numbanks, &m_prgrom[0x00000], 0x2000);
	m_prgbank2->configure_entries(0, m_numbanks, &m_prgrom[0x00000], 0x2000);
	m_prgbank3->configure_entries(0, m_numbanks, &m_prgrom[0x00000], 0x2000);

	save_item(NAME(m_410x));
	save_item(NAME(m_8000_addr_latch));

	save_item(NAME(m_timer_irq_enabled));
	save_item(NAME(m_timer_running));
	save_item(NAME(m_timer_val));

	m_ppu->set_scanline_callback(ppu2c0x_device::scanline_delegate(FUNC(nes_vt_state::scanline_irq),this));
// m_ppu->set_hblank_callback(ppu2c0x_device::hblank_delegate(FUNC(device_nes_cart_interface::hblank_irq),m_cartslot->m_cart));
}

void nes_vt_state::machine_reset()
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

	m_timer_irq_enabled = 0;
	m_timer_running = 0;
	m_timer_val = 0;

	update_banks();
}

int nes_vt_state::calculate_real_video_address(int addr, int extended, int readtype)
{
	// might be a VT09 only feature (8bpp?) but where are the other 4 bits?
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

	switch ((sel>>10) & 0xf)
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

	int va34 =m_ppu->get_va34();

	if (!extended)
	{
		int is4bpp = 0;
		if (readtype==0) is4bpp = m_ppu->get_201x_reg(0x0) & 0x02;
		else if (readtype==1) is4bpp = m_ppu->get_201x_reg(0x0) & 0x04;

		int va20_va18 = (m_ppu->get_201x_reg(0x8) & 0x70) >> 4;

		finaladdr = ((m_410x[0x0] & 0x0F) << 21) | (va20_va18 << 18) | (va17_va10 << 10) | (addr & 0x03ff);

		if (is4bpp)
		{
			if (!alt_order)
			{
				finaladdr = ((finaladdr &~0xf) << 1) | (va34 << 4) | (finaladdr & 0xf);
			}
			else
			{
				finaladdr = (finaladdr << 1) | (va34 << 4);
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
				finaladdr = ((finaladdr &~0xf) << 1) | (va34 << 4) | (finaladdr & 0xf);
			}
			else
			{
				finaladdr = (finaladdr << 1) | (va34 << 4);
			}

		}
	}

	return finaladdr;
}

/*
   nes_vt_state::vt03_8000_w notes

   this is used by
   VT03dogA.bin
   at least, but even in EmuVT the sprite graphics there are corrupt
   so I'm not sure if it's just an incomplete demo, or there is more to this
*/

// the demo program writes to the banking registers like this.. how does this really work? is this mode selectable? some kind of legacy mode?
WRITE8_MEMBER(nes_vt_state::vt03_8000_w)
{
	switch (offset + 0x8000)
	{
	case 0x8000:
		m_8000_addr_latch = data;
		break;

	case 0x8001:
		switch (m_8000_addr_latch)
		{
		case 0x00:
			m_ppu->set_201x_reg(0x6, data);
			break;

		case 0x01:
			m_ppu->set_201x_reg(0x7, data);
			break;

		case 0x02: // hand?
			//if ((data != 0x00) && (data != 0x2f) && (data != 0x31) && (data != 0x32) )
			//  logerror("%s vt03_8001_data_w latch %02x data %02x\n", machine().describe_context(), m_8000_addr_latch, data);
			m_ppu->set_201x_reg(0x2, data);
			break;

		case 0x03: // dog?
			//if ((data != 0x00) && (data != 0x2c) && (data != 0x2d) && (data != 0x2e) && (data != 0x2f) && (data != 0x32) && (data != 0x3d) && (data != 0x3e) && (data != 0x3f) && (data != 0x40) && (data != 0x41) && (data != 0x42) && (data != 0x43) && (data != 0x44) && (data != 0x45) && (data != 0x46))
			//  logerror("%s vt03_8001_data_w latch %02x data %02x\n", machine().describe_context(), m_8000_addr_latch, data);
			m_ppu->set_201x_reg(0x3, data);
			break;

		case 0x04: // ball thrown
			//if ((data != 0x00) && (data != 0x10) && (data != 0x12))
			//  logerror("%s vt03_8001_data_w latch %02x data %02x\n", machine().describe_context(), m_8000_addr_latch, data);
			m_ppu->set_201x_reg(0x4, data);
			break;

		case 0x05: // ball thrown
			//if ((data != 0x00) && (data != 0x11))
			//  logerror("%s vt03_8001_data_w latch %02x data %02x\n", machine().describe_context(), m_8000_addr_latch, data);
			m_ppu->set_201x_reg(0x5, data);
			break;

		case 0x06:
			m_410x[0x7] = data;
			update_banks();
			break;

		case 0x07:
			m_410x[0x8] = data;
			update_banks();
			break;
		default:
			logerror("%s vt03_8001_data_w latch %02x data %02x\n", machine().describe_context(), m_8000_addr_latch, data);
			break;
		}
		break;

	case 0xa000:
		logerror("%s: vt03_a000_w %02x\n", machine().describe_context(), data);
		break;

	case 0xa001:
		logerror("%s: vt03_a001_w %02x\n", machine().describe_context(), data);
		break;

		// registers below appear to provide an alt way of setting the scanline counter/timer

	case 0xc000:
		// seems to be a mirror of 4101 (timer latch)
		//logerror("%s: vt03_c000_w %02x\n", machine().describe_context(), data);
		vt03_410x_w(space, 1, data);
		break;

	case 0xc001:
		// seems to be a mirror of 4102 (load timer with latched data, start counting)
		// logerror("%s: vt03_c001_w %02x\n", machine().describe_context(), data);
		vt03_410x_w(space, 2, data);
		break;

	case 0xe000:
		// seems to be a mirror of 4103 (disable timer interrupt)
		// logerror("%s: vt03_e000_w %02x\n", machine().describe_context(), data);
		vt03_410x_w(space, 3, data);
		break;

	case 0xe001:
		// seems to be a mirror of 4104 (enable timer interrupt)
		//logerror("%s: vt03_e001_w %02x\n", machine().describe_context(), data);
		vt03_410x_w(space, 4, data);
		break;

	default:
		logerror("%s: vt03_8000_w (%04x) unhandled %02x\n", machine().describe_context(), offset+0x8000, data );
		break;
	}
}

/* APU plumbing, this is because we have a plain M6502 core in the VT03, otherwise this is handled in the core */

READ8_MEMBER(nes_vt_state::psg1_4014_r)
{
	return m_apu->read(space, 0x14);
}

READ8_MEMBER(nes_vt_state::psg1_4015_r)
{
	return m_apu->read(space, 0x15);
}

WRITE8_MEMBER(nes_vt_state::psg1_4015_w)
{
	m_apu->write(space, 0x15, data);
}

WRITE8_MEMBER(nes_vt_state::psg1_4017_w)
{
	m_apu->write(space, 0x17, data);
}

WRITE8_MEMBER(nes_vt_state::nes_vh_sprite_dma_w)
{
	m_ppu->spriteram_dma(space, data);
}

static ADDRESS_MAP_START( nes_vt_map, AS_PROGRAM, 8, nes_vt_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x2000, 0x3fff) AM_DEVREADWRITE("ppu", ppu2c0x_device, read, write)        /* PPU registers */

	AM_RANGE(0x4000, 0x4013) AM_DEVREADWRITE("apu", nesapu_device, read, write)
	AM_RANGE(0x4014, 0x4014) AM_READ(psg1_4014_r) AM_WRITE(nes_vh_sprite_dma_w)
	AM_RANGE(0x4015, 0x4015) AM_READWRITE(psg1_4015_r, psg1_4015_w) /* PSG status / first control register */
	AM_RANGE(0x4016, 0x4016) AM_READWRITE(nes_in0_r, nes_in0_w)
	AM_RANGE(0x4017, 0x4017) AM_READ(nes_in1_r) AM_WRITE(psg1_4017_w)

	AM_RANGE(0x4100, 0x410b) AM_WRITE(vt03_410x_w)

	AM_RANGE(0x8000, 0xffff) AM_WRITE(vt03_8000_w)
	AM_RANGE(0x8000, 0xffff) AM_DEVICE("prg", address_map_bank_device, amap8)
ADDRESS_MAP_END

/* Some later VT models have more RAM */
static ADDRESS_MAP_START( nes_vt_xx_map, AS_PROGRAM, 8, nes_vt_state )
	AM_IMPORT_FROM(nes_vt_map)
	AM_RANGE(0x0800, 0x0fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( prg_map, AS_PROGRAM, 8, nes_vt_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROMBANK("prg_bank0")
	AM_RANGE(0x2000, 0x3fff) AM_ROMBANK("prg_bank1")
	AM_RANGE(0x4000, 0x5fff) AM_ROMBANK("prg_bank2")
	AM_RANGE(0x6000, 0x7fff) AM_ROMBANK("prg_bank3")
ADDRESS_MAP_END

WRITE_LINE_MEMBER(nes_vt_state::apu_irq)
{
//  set_input_line(N2A03_APU_IRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

READ8_MEMBER(nes_vt_state::apu_read_mem)
{
	return 0x00;//mintf->program->read_byte(offset);
}

void nes_vt_state::ppu_nmi(int *ppu_regs)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

/* not strictly needed, but helps us see where things are in ROM to aid with figuring out banking schemes*/
static const gfx_layout helper_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0*64, 1*64, 2*64, 3*64 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	4*64
};

static const gfx_layout helper2_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0*8, 1*8, 2*8, 3*8 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*16, 1*16, 2*16, 3*16,4*16,5*16,5*16,6*16,7*16 },
	4*64
};



static GFXDECODE_START( vt03_helper )
	GFXDECODE_ENTRY( "mainrom", 0, helper_layout,  0x0, 2  )
	GFXDECODE_ENTRY( "mainrom", 0, helper2_layout,  0x0, 2  )
GFXDECODE_END


static MACHINE_CONFIG_START( nes_vt  )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, NTSC_APU_CLOCK) // selectable speed?
	MCFG_CPU_PROGRAM_MAP(nes_vt_map)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60.0988)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC((113.66/(NTSC_APU_CLOCK/1000000)) * (ppu2c0x_device::VBLANK_LAST_SCANLINE_NTSC-ppu2c0x_device::VBLANK_FIRST_SCANLINE+1+2)))
	MCFG_SCREEN_SIZE(32*8, 262)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(nes_vt_state, screen_update_vt)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", vt03_helper)

	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_INIT_OWNER(nes_vt_state, nesvt)
	MCFG_PALETTE_INDIRECT_ENTRIES(4*16*8)

	MCFG_PPU_VT03_ADD("ppu")
	MCFG_PPU2C0X_CPU("maincpu")
	MCFG_PPU2C0X_SET_NMI(nes_vt_state, ppu_nmi)
	MCFG_PPU_VT03_READ_BG_CB(READ8(nes_vt_state,chr_r))
	MCFG_PPU_VT03_READ_SP_CB(READ8(nes_vt_state,spr_r))

	MCFG_DEVICE_ADD("prg", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(prg_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(15)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x8000)

	MCFG_NES_CONTROL_PORT_ADD("ctrl1", nes_control_port1_devices, "joypad")
	//MCFG_NESCTRL_BRIGHTPIXEL_CB(nes_state, bright_pixel)
	MCFG_NES_CONTROL_PORT_ADD("ctrl2", nes_control_port2_devices, "joypad")
	//MCFG_NESCTRL_BRIGHTPIXEL_CB(nes_state, bright_pixel)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	/* this should actually be a custom *almost* doubled up APU, however requires more thought
	   than just using 2 APUs as registers in the 2nd one affect the PCM channel mode but the
	   DMA control still comes from the 1st, but in the new mode, sound always outputs via the
	   2nd.  Probably need to split the APU into interface and sound gen logic. */
	MCFG_SOUND_ADD("apu", NES_APU, NTSC_APU_CLOCK )
	MCFG_NES_APU_IRQ_HANDLER(WRITELINE(nes_vt_state, apu_irq))
	MCFG_NES_APU_MEM_READ_CALLBACK(READ8(nes_vt_state, apu_read_mem))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( nes_vt_xx, nes_vt )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(nes_vt_xx_map)
MACHINE_CONFIG_END


static INPUT_PORTS_START( nes_vt )
INPUT_PORTS_END



ROM_START( vdogdeme )
	ROM_REGION( 0x100000, "mainrom", 0 )
	ROM_LOAD( "vdog.bin", 0x00000, 0x100000, CRC(29dae36d) SHA1(e7192c5b16f3e658b0802e5c50fab244e974d9c2) )
ROM_END

ROM_START( vdogdemo )
	ROM_REGION( 0x80000, "mainrom", 0 )
	ROM_LOAD( "rom.bin", 0x00000, 0x80000, CRC(054af705) SHA1(e730aeaa94b9cc28aa8b512a5bf411ec45226831) )
ROM_END

ROM_START( mc_dgear )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "dreamgear 75-in-1(unl)[!].prg", 0x00000, 0x400000, CRC(9aabcb8f) SHA1(aa9446b7777fa64503871225fcaf2a17aafd9af1) )
ROM_END

ROM_START( dgun2500 )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "dgun2500.bin", 0x00000, 0x2000000, CRC(a2f963f3) SHA1(e29ed20ccdcf25b5640a607b3d2c9e6a4944e172) ) // 1ST AND 2ND HALF IDENTICAL
ROM_END

ROM_START( dgun2561 )
	ROM_REGION( 0x4000000, "mainrom", 0 )
	ROM_LOAD( "dgun2561.bin", 0x00000, 0x4000000, CRC(a6e627b4) SHA1(2667d2feb02de349387f9dcfa5418e7ed3afeef6) )
ROM_END

ROM_START( lexcyber )
	ROM_REGION( 0x4000000, "mainrom", 0 )
	ROM_LOAD( "lexcyber.bin", 0x00000, 0x4000000, CRC(3f3af72c) SHA1(76127054291568fcce1431d21af71f775cfb05a6) )
ROM_END

ROM_START( cybar120 )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "M2500P-VT09-EPSON_(20091222VER05,_30R-SX1067-01_PCB,_12R0COB128M_12001-3D05_FW).bin", 0x00000, 0x1000000, CRC(f7138980) SHA1(de31264ee3a5a5c77a86733b2e2d6845fee91ea5) )
ROM_END

ROM_START( ii8in1 )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "ii8in1.bin", 0x00000, 0x2000000, CRC(7aee7464) SHA1(7a9cf7f54a350f0853a17459f2dcbef34f4f7c30) ) // 2ND HALF EMPTY
ROM_END

ROM_START( ii32in1 )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "ii32in1.bin", 0x00000, 0x2000000, CRC(ddee4eac) SHA1(828c0c18a66bb4872299f9a43d5e3647482c5925) )
ROM_END

ROM_START( mc_dg101 )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "dreamgear 101-in-1(unl)[u][!].prg", 0x00000, 0x400000, CRC(6a7cd8f4) SHA1(9a5ceb8e5e38eb93699dbb14c2c36f3a501d9c45) )
ROM_END

ROM_START( mc_aa2 )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "100 in 1 arcade action ii (at-103)(unl)[!].prg", 0x00000, 0x400000, CRC(33923995) SHA1(a206e8c0ee6e86adb800cf66697defabcbd01902) )
ROM_END

ROM_START( mc_105te )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "2011 super hik 105-in-1 turbo edition.prg", 0x00000, 0x800000, CRC(c0f85771) SHA1(8c4182b1de3be10dd895089823cc67a9d12589ef) )
ROM_END

ROM_START( mc_sp69 )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "sports game 69-in-1 (unl)[u][!].prg", 0x00000, 0x400000, CRC(1242da7f) SHA1(bb8f99b1f4a4783b3f7e54d74f1f2a6a628da154) )
ROM_END

ROM_START( pjoyn50 )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "power joy navigator 50-in-1 (unl)[u][!].prg", 0x00000, 0x400000, CRC(d1bbadd4) SHA1(2186c71bcedf6c2eedf58233faa26fca9586aa40) )
ROM_END

ROM_START( pjoys30 )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "power joy supermax 30-in-1 (unl)[u][!].prg", 0x00000, 0x400000, CRC(947ac898) SHA1(08bb99a8ad39c56780bc66f4e0a9830fba7372dc) )
ROM_END

ROM_START( pjoys60 )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "power joy supermax 60-in-1 (unl)[u][!].prg", 0x00000, 0x400000, CRC(1ab45228) SHA1(d148924afc39fc588235331a1a30df6e0d8e1e18) )
ROM_END

ROM_START( sarc110 )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "ic1.prg", 0x00000, 0x400000, CRC(de76f71f) SHA1(ff6b37a76c6463af7ae901918fc008b4a2863951) )
ROM_END

ROM_START( sarc110a )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "ic1_ver2.prg", 0x00000, 0x400000, CRC(b97a0dc7) SHA1(bace32d73184df914113de5336e29a7a6f4c03fa) )
ROM_END

// CoolBoy AEF-390 8bit Console, B8VPCBVer03 20130703 0401E2015897A
ROM_START( mc_8x6cb )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "888888-in-1 (coolboy aef-390 8bit console, b8vpcbver03 20130703 0401e2015897a)(unl)[u][!].prg", 0x00000, 0x400000, CRC(ca4bd948) SHA1(cfd6c0b03bb432de43d070100031b223c9ee7496) )
ROM_END

ROM_START( mc_110cb )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "29w320dt.bin", 0x00000, 0x400000, CRC(a4bed7eb) SHA1(f1aa89916264ba781d3f1390a2336ef42129b607) )
ROM_END

ROM_START( mc_138cb )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "138-in-1 coolbaby (coolboy rs-5, pcb060-10009011v1.3)(unl)[u][!].bin", 0x00000, 0x400000, CRC(6b5b1a1a) SHA1(2df0cd717bd0de0b0c973ac356426ddbb0d736fa) )
ROM_END

ROM_START( mc_7x6ss )
	ROM_REGION( 0x100000, "mainrom", 0 )
	ROM_LOAD( "777777-in-1 (8 bit slim station, newpxp-dvt22-a pcb)(unl)[u][!].bin", 0x00000, 0x100000, CRC(7790c21a) SHA1(f320f3dd18b88ae5f65bb51f58d4cb869997bab3) )
ROM_END

ROM_START( mc_8x6ss )
	ROM_REGION( 0x200000, "mainrom", 0 ) // odd size rom, does it need stripping?
	ROM_LOAD( "888888-in-1 (8 bit slim station, newpxp-dvt22-a pcb)(unl)[u][!].bin", 0x00000, 0x100ce1, CRC(47149d0b) SHA1(5a8733886b550e3235dd90fb415b5a602e967f91) )
ROM_END

// PXP2 8Bit Slim Station
ROM_START( mc_9x6ss )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "s29gl032.u3", 0x00000, 0x400000, CRC(9f4194e8) SHA1(bd2a356aea56188ea78169095cbbe603d00e0063) )
ROM_END

// same machine as above? is one of these bad?
ROM_START( mc_9x6sa )
	ROM_REGION( 0x200000, "mainrom", 0 )
	ROM_LOAD( "999999-in-1 (8 bit slim station, newpxp-dvt22-a pcb)(unl)[u][!].bin", 0x00000, 0x200000, CRC(6a47c6a0) SHA1(b4dd376167a57dbee3dea70eb16f1a38e16bcdaa) )
ROM_END

ROM_START( mc_sam60 )
	ROM_REGION( 0x200000, "mainrom", 0 )
	ROM_LOAD( "29lv160b.bin", 0x00000, 0x200000, CRC(7dac8efe) SHA1(ffb27ebb4299d5b9a4b976c418fcc7695200060c) )
ROM_END

ROM_START( mc_dcat8 )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "100-in-1 (d-cat8 8bit console, v5.01.11-frd, bl 20041217)(unl)[u][!].prg", 0x00000, 0x800000, CRC(97d20611) SHA1(d49796e66d7b1dff0ee2781cb0e48b777969d83f) )
ROM_END

ROM_START( mc_dcat8a )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "s29gl064.u6", 0x00000, 0x800000, CRC(e28b1ef8) SHA1(4a6f107d2189cbe1bb0b86b3738d0af58e24e0f7) )
ROM_END

ROM_START( vgtablet )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "vgtablet.bin", 0x00000, 0x400000, CRC(99ef3978) SHA1(0074445708d66a04ab02b4993069ce1ae0514c2f) )
ROM_END

ROM_START( gprnrs1 )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "gprnrs1.bin", 0x00000, 0x800000, CRC(c3ffcec8) SHA1(313a790fb51d0b155257f9de84726ed67da43a8f) )
ROM_END

ROM_START( gprnrs16 )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "gprnrs16.bin", 0x00000, 0x2000000, CRC(bdffa40a) SHA1(3d01907211f18e8415171dfc6c1d23cf5952e7bb) )
ROM_END


// earlier version of vdogdemo
CONS( 200?, vdogdeme,  0,  0,  nes_vt,    nes_vt, nes_vt_state,  0, "VRT", "V-Dog (prototype, earlier)", MACHINE_NOT_WORKING )

// this is glitchy even in other emulators, might just be entirely unfinished, it selects banks but they don't contain the required gfx?
CONS( 200?, vdogdemo,  0,  0,  nes_vt,    nes_vt, nes_vt_state,  0, "VRT", "V-Dog (prototype)", MACHINE_NOT_WORKING )

// should be VT03 based
// for testing 'Shark', 'Octopus', 'Harbor', and 'Earth Fighter' use the extended colour modes, other games just seem to use standard NES modes
CONS( 200?, mc_dgear,  0,  0,  nes_vt,    nes_vt, nes_vt_state,  0, "dreamGEAR", "dreamGEAR 75-in-1", MACHINE_NOT_WORKING )
// all software in this runs in the VT03 enhanced mode, it also includes an actual licensed VT03 port of Frogger.  If running on EmuVT set to PAL or the colours are broken
CONS( 200?, vgtablet,   0,        0,  nes_vt,    nes_vt, nes_vt_state,  0, "<unknown> / Konami", "VG Pocket Tablet", MACHINE_NOT_WORKING )

// this is VT09 based, and needs 8bpp modes at least
// it boots, but gfx look wrong due to unsupported mode
CONS( 2009, cybar120,  0,  0,  nes_vt_xx, nes_vt, nes_vt_state,  0, "Defender", "Defender M2500P 120-in-1", MACHINE_NOT_WORKING )

// these are NOT VT03, but something newer but based around the same basic designs
// (no visible tiles in ROM using standard decodes tho, might need moving out of here)
CONS( 200?, dgun2500,  0,  0,  nes_vt,    nes_vt, nes_vt_state,  0, "dreamGEAR", "dreamGEAR Wireless Motion Control with 130 games (DGUN-2500)", MACHINE_NOT_WORKING )
CONS( 2012, dgun2561,  0,  0,  nes_vt,    nes_vt, nes_vt_state,  0, "dreamGEAR", "dreamGEAR My Arcade Portable Gaming System (DGUN-2561)", MACHINE_NOT_WORKING )
CONS( 200?, lexcyber,  0,  0,  nes_vt_xx, nes_vt, nes_vt_state,  0, "Lexibook", "Lexibook Compact Cyber Arcade", MACHINE_NOT_WORKING )

// these seem to have custom CPU opcodes? looks similar to the above, has many of the same games, but isn't 100% valid 6502 
// (no visible tiles in ROM using standard decodes tho, might need moving out of here)
CONS( 200?, ii8in1,    0,  0,  nes_vt,    nes_vt, nes_vt_state,  0, "Intec", "InterAct 8-in-1", MACHINE_NOT_WORKING )
CONS( 200?, ii32in1,   0,  0,  nes_vt,    nes_vt, nes_vt_state,  0, "Intec", "InterAct 32-in-1", MACHINE_NOT_WORKING )

// this has 'Shark' and 'Octopus' etc. like mc_dgear but the graphics are entirely corrupt for any game using the extended modes, bad dump or different banking?
CONS( 200?, mc_sp69,    0,        0,  nes_vt,    nes_vt, nes_vt_state,  0, "<unknown>", "Sports Game 69 in 1", MACHINE_NOT_WORKING )

// doesn't boot
CONS( 200?, mc_sam60,   0,        0,  nes_vt,    nes_vt, nes_vt_state,  0, "Hummer Technology Co., Ltd.", "Samuri (60 in 1)", MACHINE_NOT_WORKING )

// titles below don't seem to use the enhanced modes, so probably VT01 / VT02 or plain standalone famiclones?

// very plain menus
CONS( 200?, pjoyn50,    0,        0,  nes_vt,    nes_vt, nes_vt_state,  0, "<unknown>", "PowerJoy Navigator 50 in 1", MACHINE_NOT_WORKING )
CONS( 200?, pjoys30,    0,        0,  nes_vt,    nes_vt, nes_vt_state,  0, "<unknown>", "PowerJoy Supermax 30 in 1", MACHINE_NOT_WORKING )
CONS( 200?, pjoys60,    0,        0,  nes_vt,    nes_vt, nes_vt_state,  0, "<unknown>", "PowerJoy Supermax 60 in 1", MACHINE_NOT_WORKING )
// has a non-enhanced version of 'Octopus' as game 30
CONS( 200?, sarc110,    0,        0,  nes_vt,    nes_vt, nes_vt_state,  0, "<unknown>", "Super Arcade 110 (set 1)", MACHINE_NOT_WORKING )
CONS( 200?, sarc110a,   sarc110,  0,  nes_vt,    nes_vt, nes_vt_state,  0, "<unknown>", "Super Arcade 110 (set 2)", MACHINE_NOT_WORKING )
// both offer chinese or english menus
CONS( 200?, mc_110cb,   0,        0,  nes_vt,    nes_vt, nes_vt_state,  0, "CoolBoy", "110 in 1 CoolBaby (CoolBoy RS-1S)", MACHINE_NOT_WORKING )
CONS( 200?, mc_138cb,   0,        0,  nes_vt,    nes_vt, nes_vt_state,  0, "CoolBoy", "138 in 1 CoolBaby (CoolBoy RS-5, PCB060-10009011V1.3)", MACHINE_NOT_WORKING )

CONS( 200?, gprnrs1,   0,        0,  nes_vt,    nes_vt, nes_vt_state,  0, "<unknown>", "Game Prince RS-1", MACHINE_NOT_WORKING )
CONS( 200?, gprnrs16,  0,        0,  nes_vt,    nes_vt, nes_vt_state,  0, "<unknown>", "Game Prince RS-16", MACHINE_NOT_WORKING )
// unsorted, these were all in nes.xml listed as ONE BUS systems
CONS( 200?, mc_dg101,   0,        0,  nes_vt,    nes_vt, nes_vt_state,  0, "dreamGEAR", "dreamGEAR 101 in 1", MACHINE_NOT_WORKING ) // dreamGear, but no enhanced games?
CONS( 200?, mc_aa2,     0,        0,  nes_vt,    nes_vt, nes_vt_state,  0, "<unknown>", "100 in 1 Arcade Action II (AT-103)", MACHINE_NOT_WORKING )
CONS( 200?, mc_105te,   0,        0,  nes_vt,    nes_vt, nes_vt_state,  0, "<unknown>", "2011 Super HiK 105 in 1 Turbo Edition", MACHINE_NOT_WORKING )
CONS( 200?, mc_8x6cb,   0,        0,  nes_vt,    nes_vt, nes_vt_state,  0, "CoolBoy",   "888888 in 1 (Coolboy AEF-390)", MACHINE_NOT_WORKING )
CONS( 200?, mc_9x6ss,   0,        0,  nes_vt,    nes_vt, nes_vt_state,  0, "<unknown>", "999999 in 1 (PXP2 Slim Station)", MACHINE_NOT_WORKING )
CONS( 200?, mc_9x6sa,   mc_9x6ss, 0,  nes_vt,    nes_vt, nes_vt_state,  0, "<unknown>", "999999 in 1 (8 bit Slim Station, NEWPXP-DVT22-A PCB)", MACHINE_NOT_WORKING )
CONS( 200?, mc_7x6ss,   0,        0,  nes_vt,    nes_vt, nes_vt_state,  0, "<unknown>", "777777 in 1 (8 bit Slim Station, NEWPXP-DVT22-A PCB)", MACHINE_NOT_WORKING )
CONS( 200?, mc_8x6ss,   0,        0,  nes_vt,    nes_vt, nes_vt_state,  0, "<unknown>", "888888 in 1 (8 bit Slim Station, NEWPXP-DVT22-A PCB)", MACHINE_NOT_WORKING )
CONS( 2004, mc_dcat8,   0,        0,  nes_vt,    nes_vt, nes_vt_state,  0, "<unknown>", "100 in 1 (D-CAT8 8bit Console, set 1) (v5.01.11-frd, BL 20041217)", MACHINE_NOT_WORKING )
CONS( 2004, mc_dcat8a,  mc_dcat8, 0,  nes_vt,    nes_vt, nes_vt_state,  0, "<unknown>", "100 in 1 (D-CAT8 8bit Console, set 2)", MACHINE_NOT_WORKING )


