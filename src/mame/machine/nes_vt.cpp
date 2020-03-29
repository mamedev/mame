// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "nes_vt.h"

DEFINE_DEVICE_TYPE(NES_VT_SOC, nes_vt_soc_device, "nes_vt_soc", "VTxx series System on a Chip")

void nes_vt_soc_device::program_map(address_map &map)
{
}

nes_vt_soc_device::nes_vt_soc_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	device_t(mconfig, NES_VT_SOC, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_maincpu(*this, "maincpu"),
	m_screen(*this, "screen"),
	m_ppu(*this, "ppu"),
	m_apu(*this, "apu"),
	m_program_space_config("program", ENDIANNESS_LITTLE, 8, 25, 0, address_map_constructor(FUNC(nes_vt_soc_device::program_map), this))
{
}

void nes_vt_soc_device::device_start()
{
}

void nes_vt_soc_device::nes_vt_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
/*
	// ddrdismx relies on the mirroring
	map(0x2000, 0x2007).mirror(0x00e0).rw(m_ppu, FUNC(ppu2c0x_device::read), FUNC(ppu2c0x_device::write));                      // standard PPU registers
	map(0x2010, 0x201f).mirror(0x00e0).rw(m_ppu, FUNC(ppu_vt03_device::read_extended), FUNC(ppu_vt03_device::write_extended));  //  extra VT PPU registers

	map(0x4000, 0x4013).rw(m_apu, FUNC(nesapu_device::read), FUNC(nesapu_device::write));
	map(0x4014, 0x4014).r(FUNC(nes_vt_state::psg1_4014_r)).w(FUNC(nes_vt_state::vt_dma_w));
	map(0x4015, 0x4015).rw(FUNC(nes_vt_state::psg1_4015_r), FUNC(nes_vt_state::psg1_4015_w)); // PSG status / first control register
	map(0x4016, 0x4016).rw(FUNC(nes_vt_state::in0_r), FUNC(nes_vt_state::in0_w));
	map(0x4017, 0x4017).r(FUNC(nes_vt_state::in1_r)).w(FUNC(nes_vt_state::psg1_4017_w));

	map(0x4034, 0x4034).w(FUNC(nes_vt_state::vt03_4034_w));

	map(0x4100, 0x410b).r(FUNC(nes_vt_state::vt03_410x_r)).w(FUNC(nes_vt_state::vt03_410x_w));
	// 0x410c unused
	map(0x410d, 0x410d).w(FUNC(nes_vt_state::extra_io_control_w));
	map(0x410e, 0x410e).rw(FUNC(nes_vt_state::extrain_01_r), FUNC(nes_vt_state::extraout_01_w));
	map(0x410f, 0x410f).rw(FUNC(nes_vt_state::extrain_23_r), FUNC(nes_vt_state::extraout_23_w));
	// 0x4114 RS232 timer (low)
	// 0x4115 RS232 timer (high)
	// 0x4116 unused
	// 0x4117 unused
	// 0x4118 unused
	map(0x4119, 0x4119).r(FUNC(nes_vt_state::rs232flags_region_r));
	// 0x411a RS232 TX data
	// 0x411b RS232 RX data

	map(0x8000, 0xffff).rw(FUNC(nes_vt_state::external_space_read), FUNC(nes_vt_state::external_space_write));
	map(0x6000, 0x7fff).ram();
	*/
}


READ8_MEMBER(nes_vt_soc_device::spr_r)
{
	/*
	if (m_4242 & 0x1 || m_411d & 0x04)
	{
		return m_chrram[offset];
	}
	else
	{
		int realaddr = calculate_real_video_address(offset, 0, 1);

		return m_vt_external_space->read8(realaddr);
	}
	*/
	return 0x00;
}

READ8_MEMBER(nes_vt_soc_device::chr_r)
{
	/*
	if (m_4242 & 0x1 || m_411d & 0x04)
	{
		return m_chrram[offset];
	}
	else
	{
		int realaddr = calculate_real_video_address(offset, 1, 0);
		return m_vt_external_space->read8(realaddr);
	}
	*/
	return 0x00;
}

WRITE_LINE_MEMBER(nes_vt_soc_device::apu_irq)
{
	// TODO
//  set_input_line(N2A03_APU_IRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

READ8_MEMBER(nes_vt_soc_device::apu_read_mem)
{
	// TODO
	return 0x00;//mintf->program->read_byte(offset);
}


void nes_vt_soc_device::device_add_mconfig(machine_config &config)
{
	M6502_VTSCR(config, m_maincpu, NTSC_APU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vt_soc_device::nes_vt_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60.0988);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC((113.66/(NTSC_APU_CLOCK.dvalue()/1000000)) *
							 (ppu2c0x_device::VBLANK_LAST_SCANLINE_NTSC-ppu2c0x_device::VBLANK_FIRST_SCANLINE+1+2)));
	m_screen->set_size(32*8, 262);
	m_screen->set_visarea(0*8, 32*8-1, 0*8, 30*8-1);
	m_screen->set_screen_update(FUNC(nes_vt_soc_device::screen_update));

	PPU_VT03(config, m_ppu, N2A03_NTSC_XTAL);
	m_ppu->set_cpu_tag(m_maincpu);
	m_ppu->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_ppu->read_bg().set(FUNC(nes_vt_soc_device::chr_r));
	m_ppu->read_sp().set(FUNC(nes_vt_soc_device::spr_r));
	m_ppu->set_screen(m_screen);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	/* this should actually be a custom *almost* doubled up APU, however requires more thought
	   than just using 2 APUs as registers in the 2nd one affect the PCM channel mode but the
	   DMA control still comes from the 1st, but in the new mode, sound always outputs via the
	   2nd.  Probably need to split the APU into interface and sound gen logic. */
	NES_APU(config, m_apu, NTSC_APU_CLOCK);
	m_apu->irq().set(FUNC(nes_vt_soc_device::apu_irq));
	m_apu->mem_read().set(FUNC(nes_vt_soc_device::apu_read_mem));
	m_apu->add_route(ALL_OUTPUTS, "mono", 0.50);
}

uint32_t nes_vt_soc_device::screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect)
{
	return 0;// m_ppu->screen_update(screen, bitmap, cliprect);
}


device_memory_interface::space_config_vector nes_vt_soc_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_space_config)
	};
}
