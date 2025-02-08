// license:BSD-3-Clause
// copyright-holders:David Haywood

// used by fcpocket, dgun2573, rminitv

#include "emu.h"
#include "nes_vt32_soc.h"

// these have RGB12 output mode
DEFINE_DEVICE_TYPE(NES_VT32_SOC,     nes_vt32_soc_device,     "nes_vt32_soc", "VT32 series System on a Chip (FP) (NTSC)")
DEFINE_DEVICE_TYPE(NES_VT32_SOC_PAL, nes_vt32_soc_pal_device, "nes_vt32_soc_pal", "VT32 series System on a Chip (FP) (PAL)")

nes_vt32_soc_device::nes_vt32_soc_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock) :
	nes_vt09_soc_device(mconfig, type, tag, owner, clock)
{
}

nes_vt32_soc_device::nes_vt32_soc_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	nes_vt32_soc_device(mconfig, NES_VT32_SOC, tag, owner, clock)
{
}

nes_vt32_soc_pal_device::nes_vt32_soc_pal_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	nes_vt32_soc_device(mconfig, NES_VT32_SOC_PAL, tag, owner, clock)
{
}

void nes_vt32_soc_device::device_add_mconfig(machine_config& config)
{
	nes_vt02_vt03_soc_device::device_add_mconfig(config);

	RP2A03_VTSCR(config.replace(), m_maincpu, NTSC_APU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vt32_soc_device::nes_vt_fp_map);
}

void nes_vt32_soc_pal_device::device_add_mconfig(machine_config& config)
{
	nes_vt32_soc_device::device_add_mconfig(config);
	do_pal_timings_and_ppu_replacement(config);
}


uint8_t nes_vt32_soc_device::vtfp_4119_r()
{
	// would be PAL/NTSC etc. in base system, maybe different here?
	return 0x00;
}

void nes_vt32_soc_device::vtfp_411e_w(uint8_t data)
{
	logerror("411e_w %02x\n", data);
	if (data == 0x05)
		downcast<rp2a03_vtscr &>(*m_maincpu).set_next_scramble(true);
	else if (data == 0x00)
		downcast<rp2a03_vtscr &>(*m_maincpu).set_next_scramble(false);
}

void nes_vt32_soc_device::vtfp_4a00_w(uint8_t data)
{
	logerror("4a00_w %02x\n", data);
	//if(data == 0x80)
	//  downcast<rp2a03_vtscr &>(*m_maincpu).set_scramble(false);
}


void nes_vt32_soc_device::vtfp_412c_extbank_w(uint8_t data)
{
	m_upper_write_412c_callback(data);
}

uint8_t nes_vt32_soc_device::vtfp_412d_r()
{
	return m_upper_read_412d_callback();
}

void nes_vt32_soc_device::vtfp_4242_w(uint8_t data)
{
	logerror("vtfp_4242_w %02x\n", data);
	m_4242 = data;
}

void nes_vt32_soc_device::vtfp_411d_w(uint8_t data)
{
	// controls chram access and mapper emulation modes in later models
	logerror("vtfp_411d_w  %02x\n", data);
	m_411d = data;
	update_banks();
}

uint8_t nes_vt32_soc_device::vthh_414a_r()
{
	return 0x80;
}


void nes_vt32_soc_device::nes_vt_fp_map(address_map &map)
{
	nes_vt02_vt03_soc_device::nes_vt_map(map);

	map(0x0000, 0x1fff).mask(0x0fff).ram();

	map(0x414a, 0x414a).r(FUNC(nes_vt32_soc_device::vthh_414a_r));
	map(0x411d, 0x411d).w(FUNC(nes_vt32_soc_device::vtfp_411d_w));

	map(0x4119, 0x4119).r(FUNC(nes_vt32_soc_device::vtfp_4119_r));
	map(0x411e, 0x411e).w(FUNC(nes_vt32_soc_device::vtfp_411e_w)); // encryption toggle

	map(0x412c, 0x412c).w(FUNC(nes_vt32_soc_device::vtfp_412c_extbank_w)); // GPIO
	map(0x412d, 0x412d).r(FUNC(nes_vt32_soc_device::vtfp_412d_r)); // GPIO

	map(0x4242, 0x4242).w(FUNC(nes_vt32_soc_device::vtfp_4242_w));

	map(0x4a00, 0x4a00).w(FUNC(nes_vt32_soc_device::vtfp_4a00_w));
}

