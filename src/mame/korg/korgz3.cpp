// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Korg Z3 synthesizer.

****************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6801.h"
#include "cpu/nec/nec.h"
#include "machine/adc0808.h"
#include "machine/nvram.h"
#include "machine/pic8259.h"
#include "sound/ymopz.h"
#include "speaker.h"


namespace {

class korgz3_state : public driver_device
{
public:
	korgz3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_synthcpu(*this, "synthcpu")
		, m_adc(*this, "adc")
		, m_p5(0)
		, m_adc_port(0)
	{
	}

	void korgz3(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void p5_w(u8 data);
	u8 adc_port_r();
	void adc_port_w(u8 data);

	void main_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void synth_map(address_map &map) ATTR_COLD;

	required_device<v30_device> m_maincpu;
	required_device<hd6301y_cpu_device> m_synthcpu;
	required_device<adc0808_device> m_adc;

	u8 m_p5;
	u8 m_adc_port;
};

void korgz3_state::machine_start()
{
	save_item(NAME(m_p5));
	save_item(NAME(m_adc_port));
}


void korgz3_state::p5_w(u8 data)
{
	m_adc->start_w(BIT(data, 5));
	if (BIT(data, 6) && !BIT(m_p5, 6))
		m_adc->address_w(m_adc_port & 0x07);

	m_p5 = data;
}

u8 korgz3_state::adc_port_r()
{
	if (BIT(m_p5, 7))
		return m_adc->data_r();

	return 0xff;
}

void korgz3_state::adc_port_w(u8 data)
{
	m_adc_port = data;
}


void korgz3_state::main_map(address_map &map)
{
	map(0x00000, 0x07fff).ram();
	map(0xfc000, 0xfffff).rom().region("v30_program", 0xc000);
}

void korgz3_state::io_map(address_map &map)
{
	map(0x0010, 0x0013).rw("pic", FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
}

void korgz3_state::synth_map(address_map &map)
{
	map(0x2000, 0x2000).nopr();
	map(0x3800, 0x3801).rw("ymsnd", FUNC(ym2414_device::read), FUNC(ym2414_device::write));
	map(0x4000, 0x7fff).ram().share("nvram");
	map(0x8000, 0xffff).rom().region("hd6303_program", 0);
}


static INPUT_PORTS_START(korgz3)
INPUT_PORTS_END

void korgz3_state::korgz3(machine_config &config)
{
	// All clocks unknown

	V30(config, m_maincpu, 8'000'000); // D70116C-8
	m_maincpu->set_addrmap(AS_PROGRAM, &korgz3_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &korgz3_state::io_map);
	m_maincpu->set_irq_acknowledge_callback("pic", FUNC(pic8259_device::inta_cb));

	pic8259_device &pic(PIC8259(config, "pic")); // not visible on PCB; part of MB603112 ASIC?
	pic.out_int_callback().set_inputline(m_maincpu, 0);

	HD6303Y(config, m_synthcpu, 12'000'000); // HD63C03YP
	m_synthcpu->set_addrmap(AS_PROGRAM, &korgz3_state::synth_map);
	m_synthcpu->out_p5_cb().set(FUNC(korgz3_state::p5_w));
	m_synthcpu->in_p6_cb().set(FUNC(korgz3_state::adc_port_r));
	m_synthcpu->out_p6_cb().set(FUNC(korgz3_state::adc_port_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // D43256AC-12LL + battery

	M58990(config, m_adc, 1'000'000); // M58990P-1

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ym2414_device &ymsnd(YM2414(config, "ymsnd", 3'579'545)); // YM2414B
	ymsnd.add_route(0, "lspeaker", 0.60);
	ymsnd.add_route(1, "rspeaker", 0.60);
}

ROM_START(korgz3)
	ROM_REGION16_LE(0x10000, "v30_program", 0)
	ROM_LOAD16_BYTE("881405.ic5", 0x00000, 0x08000, CRC(16467e6f) SHA1(b83e10609ef0a2b095f73b1580810736fd56f693))
	ROM_LOAD16_BYTE("881505.ic4", 0x00001, 0x08000, CRC(9899a1de) SHA1(99dafee8061d3397b99f39ae74ae684fb5a1b495))

	ROM_REGION(0x8000, "hd6303_program", 0)
	ROM_LOAD("881605.ic13", 0x0000, 0x8000, CRC(39ca77fa) SHA1(b9073ef1dfad7f9d07558d2389875ebe26835068))
ROM_END

} // anonymous namespace


SYST(1988, korgz3, 0, 0, korgz3, korgz3, korgz3_state, empty_init, "Korg", "Z3 Guitar Synthesizer", MACHINE_IS_SKELETON)
