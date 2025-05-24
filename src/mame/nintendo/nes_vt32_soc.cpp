// license:BSD-3-Clause
// copyright-holders:David Haywood

// used by fcpocket, dgun2573, rminitv

#include "emu.h"
#include "nes_vt32_soc.h"

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

void nes_vt32_soc_device::device_add_mconfig(machine_config& config)
{
	nes_vt02_vt03_soc_device::device_add_mconfig(config);

	RP2A03_VTSCR(config.replace(), m_maincpu, NTSC_APU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vt32_soc_device::nes_vt32_soc_map);
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
	if (data == 0x05)
	{
		downcast<rp2a03_vtscr&>(*m_maincpu).set_next_scramble(true);
	}
	else if (data == 0x00)
	{
		downcast<rp2a03_vtscr&>(*m_maincpu).set_next_scramble(false);
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

		downcast<rp2a03_vtscr&>(*m_maincpu).set_next_scramble(false);
	}
}

void nes_vt32_soc_device::vtfp_4a00_w(u8 data)
{
	logerror("4a00_w %02x\n", data);
	//if(data == 0x80)
	//  downcast<rp2a03_vtscr &>(*m_maincpu).set_scramble(false);
}


void nes_vt32_soc_device::vtfp_412c_extbank_w(u8 data)
{
	m_upper_write_412c_callback(data);
}

u8 nes_vt32_soc_device::vtfp_412d_r()
{
	return m_upper_read_412d_callback();
}

void nes_vt32_soc_device::vtfp_4242_w(u8 data)
{
	logerror("vtfp_4242_w %02x\n", data);
	m_4242 = data;
}

void nes_vt32_soc_device::vtfp_411d_w(u8 data)
{
	// controls chram access and mapper emulation modes in later models
	logerror("vtfp_411d_w  %02x\n", data);
	m_411d = data;
	update_banks();
}

u8 nes_vt32_soc_device::vthh_414a_r()
{
	return machine().rand();
}

void nes_vt32_soc_device::nes_vt32_soc_map(address_map &map)
{
	map(0x0000, 0x1fff).ram(); // .mask(0x0fff).ram();

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

	map(0x414a, 0x414a).r(FUNC(nes_vt32_soc_device::vthh_414a_r));

	map(0x412c, 0x412c).w(FUNC(nes_vt32_soc_device::vtfp_412c_extbank_w)); // GPIO
	map(0x412d, 0x412d).r(FUNC(nes_vt32_soc_device::vtfp_412d_r)); // GPIO

	map(0x4242, 0x4242).w(FUNC(nes_vt32_soc_device::vtfp_4242_w));

	map(0x4a00, 0x4a00).w(FUNC(nes_vt32_soc_device::vtfp_4a00_w));

	map(0x6000, 0x7fff).ram();
	map(0x8000, 0xffff).rw(FUNC(nes_vt32_soc_device::external_space_read), FUNC(nes_vt32_soc_device::external_space_write));
}

