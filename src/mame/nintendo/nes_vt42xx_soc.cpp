// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "nes_vt42xx_soc.h"

#include "cpu/m6502/rp2a03.h"
#include "sound/nes_apu_vt.h"
#include "video/ppu2c0x_vt.h"

#include "screen.h"
#include "speaker.h"

DEFINE_DEVICE_TYPE(NES_VT42XX_SOC,     nes_vt42xx_soc_device,     "nes_vt42xx_soc", "unknown VT series System on a Chip with $42xx registers (NTSC)")
DEFINE_DEVICE_TYPE(NES_VT42XX_SOC_PAL, nes_vt42xx_soc_pal_device, "nes_vt42xx_soc_pal", "unknown VT series System on a Chip with $42xx registers (PAL)")

nes_vt42xx_soc_device::nes_vt42xx_soc_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, u32 clock) :
	nes_vt09_soc_device(mconfig, type, tag, owner, clock)
{
}

nes_vt42xx_soc_device::nes_vt42xx_soc_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock) :
	nes_vt42xx_soc_device(mconfig, NES_VT42XX_SOC, tag, owner, clock)
{
}

nes_vt42xx_soc_pal_device::nes_vt42xx_soc_pal_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock) :
	nes_vt42xx_soc_device(mconfig, NES_VT42XX_SOC_PAL, tag, owner, clock)
{
}


u8 nes_vt42xx_soc_device::read_onespace_bus(offs_t offset)
{
	address_space& spc = this->space(AS_PROGRAM);
	return spc.read_byte(offset);
}


void nes_vt42xx_soc_device::device_add_mconfig(machine_config& config)
{
	RP2A03_CORE(config, m_maincpu, NTSC_APU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vt42xx_soc_device::nes_vt42xx_soc_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60.0988);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC((113.66/(NTSC_APU_CLOCK.dvalue()/1000000)) *
							 (ppu2c0x_device::VBLANK_LAST_SCANLINE_NTSC-ppu2c0x_device::VBLANK_FIRST_SCANLINE+1+2)));
	m_screen->set_size(32*8, 262);
	m_screen->set_visarea(0*8, 32*8-1, 0*8, 30*8-1);
	m_screen->set_screen_update(FUNC(nes_vt42xx_soc_device::screen_update));

	PPU_VT03(config, m_ppu, RP2A03_NTSC_XTAL);
	m_ppu->set_cpu_tag(m_maincpu);
	m_ppu->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_ppu->read_bg().set(FUNC(nes_vt42xx_soc_device::chr_r));
	m_ppu->read_sp().set(FUNC(nes_vt42xx_soc_device::spr_r));
	m_ppu->set_screen(m_screen);
	m_ppu->read_onespace().set(FUNC(nes_vt42xx_soc_device::read_onespace_bus));


	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	NES_APU_VT(config, m_apu, NTSC_APU_CLOCK);
	m_apu->irq().set(FUNC(nes_vt42xx_soc_device::apu_irq));
	m_apu->mem_read().set(FUNC(nes_vt42xx_soc_device::apu_read_mem));
	m_apu->add_route(ALL_OUTPUTS, "mono", 0.50);
}

void nes_vt42xx_soc_pal_device::do_pal_timings_and_ppu_replacement(machine_config& config)
{
	m_maincpu->set_clock(PALC_APU_CLOCK);

	PPU_VT03PAL(config.replace(), m_ppu, RP2A03_PAL_XTAL);
	m_ppu->set_cpu_tag(m_maincpu);
	m_ppu->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_ppu->read_bg().set(FUNC(nes_vt42xx_soc_pal_device::chr_r));
	m_ppu->read_sp().set(FUNC(nes_vt42xx_soc_pal_device::spr_r));
	m_ppu->set_screen(m_screen);

	m_screen->set_refresh_hz(50.0070);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC((113.66 / (PALC_APU_CLOCK.dvalue() / 1000000)) *
		(ppu2c0x_device::VBLANK_LAST_SCANLINE_PAL - ppu2c0x_device::VBLANK_FIRST_SCANLINE_PALC + 1 + 2)));
	m_screen->set_size(32 * 8, 312);
	m_screen->set_visarea(0 * 8, 32 * 8 - 1, 0 * 8, 30 * 8 - 1);
}

void nes_vt42xx_soc_pal_device::device_add_mconfig(machine_config& config)
{
	nes_vt42xx_soc_device::device_add_mconfig(config);
	do_pal_timings_and_ppu_replacement(config);
}


u8 nes_vt42xx_soc_device::vtfp_412c_r()
{
	return m_upper_read_412c_callback();
}

void nes_vt42xx_soc_device::vtfp_412c_extbank_w(u8 data)
{
	m_upper_write_412c_callback(data);
}

u8 nes_vt42xx_soc_device::vtfp_412d_r()
{
	return m_upper_read_412d_callback();
}

void nes_vt42xx_soc_device::vt_420x_w(offs_t offset, u8 data)
{
	m_420x[offset] = data;
}

void nes_vt42xx_soc_device::vt_422x_w(offs_t offset, u8 data)
{
	m_422x[offset] = data;
}

void nes_vt42xx_soc_device::vt_423x_w(offs_t offset, u8 data)
{
	m_423x[offset] = data;
}

void nes_vt42xx_soc_device::vt_4233_w(u8 data)
{
	// data normally written with $94 and/or $D4
	if (BIT(data, 4))
		logerror("%s: Writing $%02X%02X to unknown (external?) register with $4233 = $%02X\n", machine().describe_context(), m_423x[0], m_423x[1], data);
	else
		logerror("%s: Writing $4233 = $%02X\n", machine().describe_context(), data);
}

void nes_vt42xx_soc_device::device_start()
{
	nes_vt09_soc_device::device_start();

	std::fill(std::begin(m_420x), std::end(m_420x), 0);
	std::fill(std::begin(m_422x), std::end(m_422x), 0);
	std::fill(std::begin(m_423x), std::end(m_423x), 0);

	save_item(NAME(m_420x));
	save_item(NAME(m_422x));
	save_item(NAME(m_423x));
}

void nes_vt42xx_soc_device::device_reset()
{
	nes_vt09_soc_device::device_reset();
}

void nes_vt42xx_soc_device::nes_vt42xx_soc_map(address_map &map)
{
	map(0x0000, 0x07ff).ram(); // .mirror(0x1800).ram();

	map(0x2000, 0x2007).rw(m_ppu, FUNC(ppu2c0x_device::read), FUNC(ppu2c0x_device::write)); // standard PPU registers

	// 2010 - 201f are extended regs, and can differ between VT models
	map(0x2010, 0x2010).rw(m_ppu, FUNC(ppu_vt03_device::extended_modes_enable_r), FUNC(ppu_vt03_device::extended_modes_enable_w));
	map(0x2011, 0x2011).rw(m_ppu, FUNC(ppu_vt03_device::extended_modes2_enable_r), FUNC(ppu_vt03_device::extended_modes2_enable_w));
	map(0x2012, 0x2012).rw(m_ppu, FUNC(ppu_vt03_device::videobank0_0_r), FUNC(ppu_vt03_device::videobank0_0_w));
	map(0x2013, 0x2013).rw(m_ppu, FUNC(ppu_vt03_device::videobank0_1_r), FUNC(ppu_vt03_device::videobank0_1_w));
	map(0x2014, 0x2014).rw(m_ppu, FUNC(ppu_vt03_device::videobank0_2_r), FUNC(ppu_vt03_device::videobank0_2_w));
	map(0x2015, 0x2015).rw(m_ppu, FUNC(ppu_vt03_device::videobank0_3_r), FUNC(ppu_vt03_device::videobank0_3_w));
	map(0x2016, 0x2016).rw(m_ppu, FUNC(ppu_vt03_device::videobank0_4_r), FUNC(ppu_vt03_device::videobank0_4_w));
	map(0x2017, 0x2017).rw(m_ppu, FUNC(ppu_vt03_device::videobank0_5_r), FUNC(ppu_vt03_device::videobank0_5_w));
	map(0x2018, 0x2018).rw(m_ppu, FUNC(ppu_vt03_device::videobank1_r), FUNC(ppu_vt03_device::videobank1_w));
	map(0x2019, 0x2019).rw(m_ppu, FUNC(ppu_vt03_device::unk_2019_r), FUNC(ppu_vt03_device::gun_reset_w));
	map(0x201a, 0x201a).rw(m_ppu, FUNC(ppu_vt03_device::videobank0_extra_r), FUNC(ppu_vt03_device::videobank0_extra_w));
	map(0x201b, 0x201b).r(m_ppu, FUNC(ppu_vt03_device::unk_201b_r));
	map(0x201c, 0x201c).r(m_ppu, FUNC(ppu_vt03_device::gun_x_r));
	map(0x201d, 0x201d).r(m_ppu, FUNC(ppu_vt03_device::gun_y_r));
	map(0x201e, 0x201e).r(m_ppu, FUNC(ppu_vt03_device::gun2_x_r));
	map(0x201f, 0x201f).r(m_ppu, FUNC(ppu_vt03_device::gun2_y_r));

	map(0x2040, 0x2049).nopw();// w(m_ppu, FUNC(ppu_vt3xx_device::lcdc_regs_w)); // LCD control like on VT369?

	map(0x4000, 0x4017).nopr().w(m_apu, FUNC(nes_apu_vt_device::write));
	map(0x4014, 0x4014).w(FUNC(nes_vt42xx_soc_device::vt_dma_w));
	map(0x4015, 0x4015).r(m_apu, FUNC(nes_apu_vt_device::status_r)); // PSG status / first control register
	map(0x4016, 0x4016).rw(FUNC(nes_vt42xx_soc_device::in0_r), FUNC(nes_vt42xx_soc_device::in0_w));
	map(0x4017, 0x4017).r(FUNC(nes_vt42xx_soc_device::in1_r));

	map(0x4034, 0x4034).w(FUNC(nes_vt42xx_soc_device::vt03_4034_w)); // secondary DMA

	map(0x4100, 0x410b).rw(FUNC(nes_vt42xx_soc_device::vt03_410x_r), FUNC(nes_vt42xx_soc_device::vt03_410x_w));
	// 0x410c unused
	map(0x410d, 0x410d).w(FUNC(nes_vt42xx_soc_device::extra_io_control_w));
	map(0x410e, 0x410e).rw(FUNC(nes_vt42xx_soc_device::extrain_01_r), FUNC(nes_vt42xx_soc_device::extraout_01_w));
	map(0x410f, 0x410f).rw(FUNC(nes_vt42xx_soc_device::extrain_23_r), FUNC(nes_vt42xx_soc_device::extraout_23_w));

	map(0x412c, 0x412c).rw(FUNC(nes_vt42xx_soc_device::vtfp_412c_r), FUNC(nes_vt42xx_soc_device::vtfp_412c_extbank_w)); // GPIO
	map(0x412d, 0x412d).r(FUNC(nes_vt42xx_soc_device::vtfp_412d_r)).nopw(); // GPIO

	map(0x4200, 0x420e).nopr().w(FUNC(nes_vt42xx_soc_device::vt_420x_w));
	map(0x4220, 0x4227).w(FUNC(nes_vt42xx_soc_device::vt_422x_w));
	map(0x4230, 0x4231).w(FUNC(nes_vt42xx_soc_device::vt_423x_w));
	map(0x4233, 0x4233).w(FUNC(nes_vt42xx_soc_device::vt_4233_w));

	map(0x8000, 0xffff).rw(FUNC(nes_vt42xx_soc_device::external_space_read), FUNC(nes_vt42xx_soc_device::external_space_write));
}
