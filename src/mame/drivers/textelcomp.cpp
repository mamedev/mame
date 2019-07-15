// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton driver for Textel Compact portable digital teletype machine.

*******************************************************************************/

#include "emu.h"
#include "cpu/m6502/m65sc02.h"
#include "machine/input_merger.h"
#include "machine/6522via.h"
#include "machine/mos6551.h"
#include "machine/msm58321.h"
//#include "video/sed1330.h"


class textelcomp_state : public driver_device
{
public:
	textelcomp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rtc(*this, "rtc")
		, m_chargen(*this, "chargen")
	{ }

	void textelcomp(machine_config &config);

private:
	virtual void machine_start() override;
	void rtc_w(u8 data);

	void mem_map(address_map &map);
	required_device<cpu_device> m_maincpu;
	required_device<msm58321_device> m_rtc;
	required_region_ptr<u8> m_chargen;
};


void textelcomp_state::machine_start()
{
	m_rtc->cs1_w(1);
}

void textelcomp_state::rtc_w(u8 data)
{
	// Minimum address/data setup time is given as 0 µs in Oki and Epson datasheets
	// Address and data are written to the VIA at the same time as the control strobes
	if (!BIT(data, 5))
		m_rtc->write_w(0);
	if (!BIT(data, 6))
		m_rtc->read_w(0);
	if (!BIT(data, 7))
		m_rtc->address_write_w(0);

	m_rtc->d0_w(BIT(data, 0));
	m_rtc->d1_w(BIT(data, 1));
	m_rtc->d2_w(BIT(data, 2));
	m_rtc->d3_w(BIT(data, 3));

	if (BIT(data, 5))
		m_rtc->write_w(1);
	if (BIT(data, 6))
		m_rtc->read_w(1);
	if (BIT(data, 7))
		m_rtc->address_write_w(1);
}


void textelcomp_state::mem_map(address_map &map)
{
	map(0x0000, 0x1eff).ram(); // MB8464A-10L (battery backed?)
	map(0x1f00, 0x1f0f).m("via0", FUNC(via6522_device::map));
	map(0x1f10, 0x1f1f).m("via1", FUNC(via6522_device::map));
	map(0x1f20, 0x1f2f).m("via2", FUNC(via6522_device::map));
	map(0x1f30, 0x1f3f).m("via3", FUNC(via6522_device::map));
	map(0x1f40, 0x1f43).rw("acia", FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0x1f70, 0x1f70).noprw();//rw("lcdc", FUNC(sed1330_device::status_r), FUNC(sed1330_device::data_w));
	map(0x1f71, 0x1f71).noprw();//rw("lcdc", FUNC(sed1330_device::data_r), FUNC(sed1330_device::command_w));
	map(0x4000, 0x7fff).ram(); // HY65226ALP-10 (battery backed?)
	map(0x8000, 0x9fff).ram(); // MB8464A-10L (battery backed?)
	map(0xa000, 0xffff).rom().region("maincpu", 0x2000);
}


static INPUT_PORTS_START(textelcomp)
INPUT_PORTS_END


void textelcomp_state::textelcomp(machine_config &config)
{
	M65SC02(config, m_maincpu, 3.6864_MHz_XTAL / 2); // GS65SC02P-2 (clock not verified)
	m_maincpu->set_addrmap(AS_PROGRAM, &textelcomp_state::mem_map);

	INPUT_MERGER_ANY_HIGH(config, "mainirq").output_handler().set_inputline(m_maincpu, m65sc02_device::IRQ_LINE);

	via6522_device &via0(VIA6522(config, "via0", 3.6864_MHz_XTAL / 2)); // GS65SC22P-2
	via0.irq_handler().set("mainirq", FUNC(input_merger_device::in_w<0>));
	// TODO: CA1 falling edge generates interrupt
	// TODO: PB6 falling edges are counted to generate another interrupt

	VIA6522(config, "via1", 3.6864_MHz_XTAL / 2); // GS65SC22P-2
	// IRQ might be connected on hardware, but is never enabled

	VIA6522(config, "via2", 3.6864_MHz_XTAL / 2); // GS65SC22P-2
	// TODO: CB1, CB2 and PB4 control 2x TC4094BP driving keyboard lights

	via6522_device &via3(VIA6522(config, "via3", 3.6864_MHz_XTAL / 2)); // GS65SC22P-2
	via3.writepa_handler().set(FUNC(textelcomp_state::rtc_w));
	via3.ca2_handler().set(m_rtc, FUNC(msm58321_device::cs2_w)).invert();
	via3.ca2_handler().append(m_rtc, FUNC(msm58321_device::stop_w)).invert();
	// TODO: PB7 toggling generates beeps?

	mos6551_device &acia(MOS6551(config, "acia", 3.6864_MHz_XTAL / 2)); // GS65SC51P-2
	acia.set_xtal(3.6864_MHz_XTAL / 2);
	acia.irq_handler().set("mainirq", FUNC(input_merger_device::in_w<1>));

	//SED1330(config, "lcdc", 6.4_MHz_XTAL); // SED1330F + B&W LCD

	MSM58321(config, m_rtc, 32.768_kHz_XTAL); // RTC58321A
	m_rtc->d0_handler().set("via3", FUNC(via6522_device::write_pa0));
	m_rtc->d1_handler().set("via3", FUNC(via6522_device::write_pa1));
	m_rtc->d2_handler().set("via3", FUNC(via6522_device::write_pa2));
	m_rtc->d3_handler().set("via3", FUNC(via6522_device::write_pa3));
}


ROM_START(a1010)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("d15_31.bin",  0x0000, 0x8000, CRC(5ee1175d) SHA1(87ff6a3d5c64a53b0ab23d54aa343365c44d0407))

	ROM_REGION(0x8000, "chargen", 0)
	ROM_LOAD("chargen.bin", 0x0000, 0x8000, CRC(07daa70e) SHA1(8066a0ac238b06fbeeb99c3a2a8a9e70a27db7a9))
ROM_END


COMP(1993, a1010, 0, 0, textelcomp, textelcomp, textelcomp_state, empty_init, "Humantechnik", "Textel Compact A1010-0", MACHINE_IS_SKELETON)
