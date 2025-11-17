// license:BSD-3-Clause
// copyright-holders:David Haywood

// used by fcpocket, dgun2573, rminitv

#include "emu.h"
#include "nes_vt32_soc.h"
#include "rp2a03_vtscr.h"

#include "cpu/m6502/rp2a03.h"
#include "sound/nes_apu_vt.h"
#include "video/ppu2c0x_vt.h"

#include "screen.h"
#include "speaker.h"

// these have RGB12 output mode
DEFINE_DEVICE_TYPE(NES_VT32_SOC,     nes_vt32_soc_device,     "nes_vt32_soc", "VT32 series System on a Chip (FP) (NTSC)")
DEFINE_DEVICE_TYPE(NES_VT32_SOC_PAL, nes_vt32_soc_pal_device, "nes_vt32_soc_pal", "VT32 series System on a Chip (FP) (PAL)")

nes_vt32_soc_device::nes_vt32_soc_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, u32 clock) :
	nes_vt09_soc_device(mconfig, type, tag, owner, clock)
{
}

nes_vt32_soc_device::nes_vt32_soc_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock) :
	nes_vt32_soc_device(mconfig, NES_VT32_SOC, tag, owner, clock)
{
}

nes_vt32_soc_pal_device::nes_vt32_soc_pal_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock) :
	nes_vt32_soc_device(mconfig, NES_VT32_SOC_PAL, tag, owner, clock)
{
}


u8 nes_vt32_soc_device::read_onespace_bus(offs_t offset)
{
	address_space& spc = this->space(AS_PROGRAM);
	return spc.read_byte(offset);
}


u8 nes_vt32_soc_device::spr_r(offs_t offset)
{
	if (m_4242 & 0x1 || m_411d & 0x04) // VT32 only?
	{
		return m_chrram[offset & 0x1fff];
	}
	else
	{
		u8 ret = nes_vt09_soc_device::spr_r(offset);
		// used by some games in myaass
		if (m_ppu_chr_data_scramble == 1)
			ret = bitswap<8>(ret, 3, 5, 6, 0, 7, 1, 2, 4);

		return ret;
	}
}

u8 nes_vt32_soc_device::chr_r(offs_t offset)
{
	if (m_4242 & 0x1 || m_411d & 0x04) // VT32 only?
	{
		return m_chrram[offset & 0x1fff];
	}
	else
	{
		u8 ret = nes_vt09_soc_device::chr_r(offset);
		// used by some games in myaass
		if (m_ppu_chr_data_scramble == 1)
			ret = bitswap<8>(ret, 3, 5, 6, 0, 7, 1, 2, 4);
		return ret;
	}
}


void nes_vt32_soc_device::device_add_mconfig(machine_config& config)
{
	RP2A03_VTSCR(config, m_maincpu, NTSC_APU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vt32_soc_device::nes_vt32_soc_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60.0988);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC((113.66/(NTSC_APU_CLOCK.dvalue()/1000000)) *
							 (ppu2c0x_device::VBLANK_LAST_SCANLINE_NTSC-ppu2c0x_device::VBLANK_FIRST_SCANLINE+1+2)));
	m_screen->set_size(32*8, 262);
	m_screen->set_visarea(0*8, 32*8-1, 0*8, 30*8-1);
	m_screen->set_screen_update(FUNC(nes_vt32_soc_device::screen_update));

	PPU_VT32(config, m_ppu, RP2A03_NTSC_XTAL);
	m_ppu->set_cpu_tag(m_maincpu);
	m_ppu->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_ppu->read_bg().set(FUNC(nes_vt32_soc_device::chr_r));
	m_ppu->read_sp().set(FUNC(nes_vt32_soc_device::spr_r));
	m_ppu->set_screen(m_screen);
	m_ppu->read_onespace().set(FUNC(nes_vt32_soc_device::read_onespace_bus));


	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	NES_APU_VT(config, m_apu, NTSC_APU_CLOCK);
	m_apu->irq().set(FUNC(nes_vt32_soc_device::apu_irq));
	m_apu->mem_read().set(FUNC(nes_vt32_soc_device::apu_read_mem));
	m_apu->add_route(ALL_OUTPUTS, "mono", 0.50);
}

void nes_vt32_soc_pal_device::do_pal_timings_and_ppu_replacement(machine_config& config)
{
	m_maincpu->set_clock(PALC_APU_CLOCK);

	PPU_VT32PAL(config.replace(), m_ppu, RP2A03_PAL_XTAL);
	m_ppu->set_cpu_tag(m_maincpu);
	m_ppu->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_ppu->read_bg().set(FUNC(nes_vt32_soc_pal_device::chr_r));
	m_ppu->read_sp().set(FUNC(nes_vt32_soc_pal_device::spr_r));
	m_ppu->set_screen(m_screen);

	m_screen->set_refresh_hz(50.0070);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC((113.66 / (PALC_APU_CLOCK.dvalue() / 1000000)) *
		(ppu2c0x_device::VBLANK_LAST_SCANLINE_PAL - ppu2c0x_device::VBLANK_FIRST_SCANLINE_PALC + 1 + 2)));
	m_screen->set_size(32 * 8, 312);
	m_screen->set_visarea(0 * 8, 32 * 8 - 1, 0 * 8, 30 * 8 - 1);
}

void nes_vt32_soc_pal_device::device_add_mconfig(machine_config& config)
{
	nes_vt32_soc_device::device_add_mconfig(config);
	do_pal_timings_and_ppu_replacement(config);
}


u8 nes_vt32_soc_device::vtfp_4119_r()
{
	// would be PAL/NTSC etc. in base system, maybe different here?
	return 0x00;
}

void nes_vt32_soc_device::vtfp_411e_encryption_state_w(u8 data)
{
	logerror("%s: vtfp_411e_encryption_state_w %02x\n", machine().describe_context(), data);
	m_ppu_chr_data_scramble = 0;

	if (data == 0x05)
	{
		downcast<rp2a03_vtscr&>(*m_maincpu).set_next_scramble(0xa1);
	}
	else if (data == 0x07)
	{
		downcast<rp2a03_vtscr&>(*m_maincpu).set_next_scramble(0x7e);
	}
	else if (data == 0x00)
	{
		downcast<rp2a03_vtscr&>(*m_maincpu).set_next_scramble(0x00);
	}
	else if (data == 0xc0)
	{
		/* this seems to turn off the code encryption
		   but turns on some kind of PPU data bitswap? myaass uses it on several games

		in VRAM -> should be in VRAM
		80 -> 08
		40 -> 20
		20- > 40
		10 -> 01
		08 -> 80
		04 -> 02
		02 -> 04
		01 -> 10

		it is unclear if this affects reads from ROM, or directly alters reads or
		writes involving the VRAM

		*/
		m_ppu_chr_data_scramble = 1;
		downcast<rp2a03_vtscr&>(*m_maincpu).set_next_scramble(0x00);
	}
}

void nes_vt32_soc_device::vtfp_4a00_w(u8 data)
{
	logerror("4a00_w %02x\n", data);
	//if(data == 0x80)
	//  downcast<rp2a03_vtscr &>(*m_maincpu).set_scramble(false);
}


u8 nes_vt32_soc_device::vtfp_412c_r()
{
	return m_upper_read_412c_callback();
}

void nes_vt32_soc_device::vtfp_412c_extbank_w(u8 data)
{
	m_upper_write_412c_callback(data);
}

u8 nes_vt32_soc_device::vtfp_412d_r()
{
	return m_upper_read_412d_callback();
}

u8 nes_vt32_soc_device::vt32_4132_r()
{
	return m_4132;
}

void nes_vt32_soc_device::vt32_4132_w(u8 data)
{
	m_4132 = data;
}

u8 nes_vt32_soc_device::vt32_4134_r()
{
	return m_4134;
}

void nes_vt32_soc_device::vt32_4134_w(u8 data)
{
	m_4134 = data;
}

void nes_vt32_soc_device::vtfp_4242_w(u8 data)
{
	logerror("%s: vtfp_4242_w %02x\n", machine().describe_context(), data);
	m_4242 = data;
}

void nes_vt32_soc_device::vtfp_411d_w(u8 data)
{
	// controls chram access and mapper emulation modes in later models
	logerror("%s: vtfp_411d_w  %02x\n", machine().describe_context(), data);
	m_411d = data;
	update_banks();
}

u8 nes_vt32_soc_device::vthh_414a_r()
{
	return machine().rand();
}


void nes_vt32_soc_device::update_banks()
{
	if ((m_411d & 0x03) == 0x01)
	{
		if (BIT(m_mmc1_control, 3))
		{
			if (BIT(m_mmc1_control, 2))
			{
				set_bankaddr(0, get_banks(m_mmc1_prg_bank << 1));
				set_bankaddr(1, get_banks((m_mmc1_prg_bank << 1) | 1));
				set_bankaddr(2, get_banks(0xfe));
				set_bankaddr(3, get_banks(0xff));
			}
			else
			{
				set_bankaddr(0, get_banks(0x00));
				set_bankaddr(1, get_banks(0x01));
				set_bankaddr(2, get_banks(m_mmc1_prg_bank << 1));
				set_bankaddr(3, get_banks((m_mmc1_prg_bank << 1) | 1));
			}
		}
		else
		{
			set_bankaddr(0, get_banks((m_mmc1_prg_bank & 0x0e) << 1));
			set_bankaddr(1, get_banks(((m_mmc1_prg_bank & 0x0e) << 1) | 1));
			set_bankaddr(2, get_banks(((m_mmc1_prg_bank & 0x0e) << 1) | 2));
			set_bankaddr(3, get_banks(((m_mmc1_prg_bank & 0x0e) << 1) | 3));
		}
	}
	else
		nes_vt09_soc_device::update_banks();
}

void nes_vt32_soc_device::scrambled_8000_w(u16 offset, u8 data)
{
	offset &= 0x7fff;

	u16 addr = offset+0x8000;
	if ((m_411d & 0x03) == 0x03) // (VT32 only, not VT03/09)
	{
		//CNROM compat
		logerror("%s: vtxx_cnrom_8000_w real address: (%04x) translated address: (%04x) %02x\n", machine().describe_context(), addr, offset + 0x8000, data);
		m_ppu->set_videobank0_reg(0x4, data * 8);
		m_ppu->set_videobank0_reg(0x5, data * 8 + 2);
		m_ppu->set_videobank0_reg(0x0, data * 8 + 4);
		m_ppu->set_videobank0_reg(0x1, data * 8 + 5);
		m_ppu->set_videobank0_reg(0x2, data * 8 + 6);
		m_ppu->set_videobank0_reg(0x3, data * 8 + 7);

	}
	else if ((m_411d & 0x03) == 0x01) // (VT32 only, not VT03/09)
	{
		//MMC1 compat
		if (BIT(data, 7))
		{
			logerror("%s: MMC1 reset\n", machine().describe_context());
			m_mmc1_shift_reg = 1 << 4;
			m_mmc1_control |= 0x0c;
			update_banks();
		}
		else
		{
			bool mmc1_do_write = BIT(m_mmc1_shift_reg, 0);
			m_mmc1_shift_reg = (m_mmc1_shift_reg >> 1) | (data & 0x01) << 4;
			if (mmc1_do_write)
			{
				logerror("%s: MMC1 write (%04x) %02x\n", machine().describe_context(), addr, m_mmc1_shift_reg);
				switch (offset & 0x6000)
				{
				case 0:
					m_410x[0x6] = (m_mmc1_shift_reg & 0x03) ^ 0x02;
					m_mmc1_control = m_mmc1_shift_reg;
					update_banks();
					break;

				case 0x2000:
					if (!BIT(m_mmc1_control, 4))
						m_mmc1_shift_reg &= 0x1e;
					m_ppu->set_videobank0_reg(4, m_mmc1_shift_reg << 2);
					m_ppu->set_videobank0_reg(5, (m_mmc1_shift_reg << 2) + 2);
					if (BIT(m_mmc1_control, 4))
						break;
					m_mmc1_shift_reg |= 1;
					[[fallthrough]];

				case 0x4000:
					for (int i = 0; i < 4; i++)
						m_ppu->set_videobank0_reg(i, (m_mmc1_shift_reg << 2) + i);
					break;

				case 0x6000:
					m_mmc1_prg_bank = m_mmc1_shift_reg;
					update_banks();
					break;
				}
				m_mmc1_shift_reg = 1 << 4;
			}
		}
	}
	else if ((m_411d & 0x03) == 0x02) // (VT32 only, not VT03/09)
	{
		//UNROM compat
		logerror("%s: vtxx_unrom_8000_w real address: (%04x) translated address: (%04x) %02x\n", machine().describe_context(), addr, offset + 0x8000, data);

		m_410x[0x7] = ((data & 0x0F) << 1);
		m_410x[0x8] = ((data & 0x0F) << 1) + 1;
		update_banks();
	}
	else // standard mode (VT03/09)
	{
		nes_vt02_vt03_soc_device::scrambled_8000_w(offset, data);
	}
}


u8 nes_vt32_soc_device::vt32_palette_r(offs_t offset)
{
	if (offset < 0x100)
	{
		// can the usual nametable mirroring be enabled?
		//return nt_r(offset + 0x3e00);
		return m_ppu->vt3xx_extended_palette_r(offset);
	}
	else
	{
		return m_ppu->palette_read(offset - 0x100);
	}
}

void nes_vt32_soc_device::vt32_palette_w(offs_t offset, u8 data)
{
	if (offset < 0x100)
	{
		// can the usual nametable mirroring be enabled?
		//nt_w(offset + 0x3e00, data);
		m_ppu->vt3xx_extended_palette_w(offset, data);
	}
	else
	{
		m_ppu->palette_write(offset - 0x100, data);
		// also store an unmodified copy of the data written (no mirroring etc.) for rendering use
		// it isn't clear how the hardware does this
		m_ppu->vt3xx_extended_palette_w(offset, data);
	}
}

void nes_vt32_soc_device::device_start()
{
	nes_vt09_soc_device::device_start();
	m_ppu->space(AS_PROGRAM).install_readwrite_handler(0x3e00, 0x3fff, read8sm_delegate(*this, FUNC(nes_vt32_soc_device::vt32_palette_r)), write8sm_delegate(*this, FUNC(nes_vt32_soc_device::vt32_palette_w)));
	save_item(NAME(m_ppu_chr_data_scramble));
	save_item(NAME(m_mmc1_shift_reg));
	save_item(NAME(m_mmc1_control));
	save_item(NAME(m_mmc1_prg_bank));
	save_item(NAME(m_4132));
	save_item(NAME(m_4134));
}

void nes_vt32_soc_device::device_reset()
{
	nes_vt09_soc_device::device_reset();
	m_ppu_chr_data_scramble = 0;
	m_mmc1_shift_reg = 1 << 4;
	m_mmc1_control = 0x0c;
	m_mmc1_prg_bank = 0;
	m_4132 = 0;
	m_4134 = 0;
}

void nes_vt32_soc_device::nes_vt32_soc_map(address_map &map)
{
	map(0x0000, 0x1fff).ram(); // .mask(0x0fff).ram();

	map(0x2000, 0x2007).rw(m_ppu, FUNC(ppu2c0x_device::read), FUNC(ppu2c0x_device::write)); // standard PPU registers

	// 2010 - 201f are extended regs, and can differ between VT models
	map(0x2010, 0x2010).rw(m_ppu, FUNC(ppu_vt32_device::extended_modes_enable_r), FUNC(ppu_vt32_device::extended_modes_enable_w));
	map(0x2011, 0x2011).rw(m_ppu, FUNC(ppu_vt32_device::extended_modes2_enable_r), FUNC(ppu_vt32_device::extended_modes2_enable_w));
	map(0x2012, 0x2012).rw(m_ppu, FUNC(ppu_vt32_device::videobank0_0_r), FUNC(ppu_vt32_device::videobank0_0_w));
	map(0x2013, 0x2013).rw(m_ppu, FUNC(ppu_vt32_device::videobank0_1_r), FUNC(ppu_vt32_device::videobank0_1_w));
	map(0x2014, 0x2014).rw(m_ppu, FUNC(ppu_vt32_device::videobank0_2_r), FUNC(ppu_vt32_device::videobank0_2_w));
	map(0x2015, 0x2015).rw(m_ppu, FUNC(ppu_vt32_device::videobank0_3_r), FUNC(ppu_vt32_device::videobank0_3_w));
	map(0x2016, 0x2016).rw(m_ppu, FUNC(ppu_vt32_device::videobank0_4_r), FUNC(ppu_vt32_device::videobank0_4_w));
	map(0x2017, 0x2017).rw(m_ppu, FUNC(ppu_vt32_device::videobank0_5_r), FUNC(ppu_vt32_device::videobank0_5_w));
	map(0x2018, 0x2018).rw(m_ppu, FUNC(ppu_vt32_device::videobank1_r), FUNC(ppu_vt32_device::videobank1_w));
	map(0x2019, 0x2019).rw(m_ppu, FUNC(ppu_vt32_device::unk_2019_r), FUNC(ppu_vt32_device::gun_reset_w));
	map(0x201a, 0x201a).rw(m_ppu, FUNC(ppu_vt32_device::videobank0_extra_r), FUNC(ppu_vt32_device::videobank0_extra_w));
	map(0x201b, 0x201b).rw(m_ppu, FUNC(ppu_vt32_device::unk_201b_r), FUNC(ppu_vt32_device::m_newvid_1b_w));
	map(0x201c, 0x201c).rw(m_ppu, FUNC(ppu_vt32_device::gun_x_r), FUNC(ppu_vt32_device::m_newvid_1c_w));
	map(0x201d, 0x201d).rw(m_ppu, FUNC(ppu_vt32_device::gun_y_r), FUNC(ppu_vt32_device::m_newvid_1d_w));
	map(0x201e, 0x201e).r(m_ppu, FUNC(ppu_vt32_device::gun2_x_r));
	map(0x201f, 0x201f).r(m_ppu, FUNC(ppu_vt32_device::gun2_y_r));

	map(0x2040, 0x2049).nopw();// w(m_ppu, FUNC(ppu_vt3xx_device::lcdc_regs_w)); // LCD control like on VT369?

	map(0x4000, 0x4017).w(m_apu, FUNC(nes_apu_vt_device::write));
	map(0x4014, 0x4014).w(FUNC(nes_vt32_soc_device::vt_dma_w));
	map(0x4015, 0x4015).r(m_apu, FUNC(nes_apu_vt_device::status_r)); // PSG status / first control register
	map(0x4016, 0x4016).rw(FUNC(nes_vt32_soc_device::in0_r), FUNC(nes_vt32_soc_device::in0_w));
	map(0x4017, 0x4017).r(FUNC(nes_vt32_soc_device::in1_r));

	map(0x4034, 0x4034).w(FUNC(nes_vt32_soc_device::vt03_4034_w)); // secondary DMA

	map(0x4100, 0x410b).r(FUNC(nes_vt32_soc_device::vt03_410x_r)).w(FUNC(nes_vt32_soc_device::vt03_410x_w));
	// 0x410c unused
	map(0x410d, 0x410d).w(FUNC(nes_vt32_soc_device::extra_io_control_w));
	map(0x410e, 0x410e).rw(FUNC(nes_vt32_soc_device::extrain_01_r), FUNC(nes_vt32_soc_device::extraout_01_w));
	map(0x410f, 0x410f).rw(FUNC(nes_vt32_soc_device::extrain_23_r), FUNC(nes_vt32_soc_device::extraout_23_w));
	// 0x4114 RS232 timer (low)
	// 0x4115 RS232 timer (high)
	// 0x4116 unused
	// 0x4117 unused
	// 0x4118 unused
	map(0x4119, 0x4119).r(FUNC(nes_vt32_soc_device::vtfp_4119_r));
	// 0x411a RS232 TX data
	// 0x411b RS232 RX data
	map(0x411d, 0x411d).w(FUNC(nes_vt32_soc_device::vtfp_411d_w));
	map(0x411e, 0x411e).w(FUNC(nes_vt32_soc_device::vtfp_411e_encryption_state_w)); // encryption toggle

	map(0x412c, 0x412c).rw(FUNC(nes_vt32_soc_device::vtfp_412c_r), FUNC(nes_vt32_soc_device::vtfp_412c_extbank_w)); // GPIO
	map(0x412d, 0x412d).r(FUNC(nes_vt32_soc_device::vtfp_412d_r)).nopw(); // GPIO

	map(0x4132, 0x4132).rw(FUNC(nes_vt32_soc_device::vt32_4132_r), FUNC(nes_vt32_soc_device::vt32_4132_w));
	map(0x4134, 0x4134).rw(FUNC(nes_vt32_soc_device::vt32_4134_r), FUNC(nes_vt32_soc_device::vt32_4134_w));

	map(0x4141, 0x4141).nopw(); // ??
	map(0x4142, 0x4142).nopw(); // ??

	map(0x414a, 0x414a).r(FUNC(nes_vt32_soc_device::vthh_414a_r));

	map(0x4242, 0x4242).w(FUNC(nes_vt32_soc_device::vtfp_4242_w));

	map(0x4a00, 0x4a00).w(FUNC(nes_vt32_soc_device::vtfp_4a00_w));

	map(0x6000, 0x7fff).ram();
	map(0x8000, 0xffff).rw(FUNC(nes_vt32_soc_device::external_space_read), FUNC(nes_vt32_soc_device::external_space_write));
}

