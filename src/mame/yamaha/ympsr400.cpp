// license:BSD-3-Clause
// copyright-holders:AJR, R. Belmont
/*******************************************************************************

    Yamaha PSR-400 & PSR-500 PortaTone keyboards.

    The main firmware plays the built-in demos through the GEW8.
    TODO: keyboard scanner MCU (undumped) and MIDI.

*******************************************************************************/

#include "emu.h"

#include "cpu/m6805/hd6305.h"
#include "cpu/mn1880/mn1880.h"
#include "sound/multipcm.h"
#include "sound/ym3413.h"
#include "video/pwm.h"

#include "speaker.h"

#include <algorithm>
#include <vector>

#include "psr500.lh"

namespace {

class psr400_state : public driver_device
{
public:
	psr400_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mpscpu(*this, "mpscpu")
		, m_display(*this, "display")
		, m_panel(*this, "S%u", 0U)
		, m_panel_select{0, 0}
		, m_display_select(0xff)
	{
	}

	void psr500(machine_config &config) ATTR_COLD;

	void init_psr500() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void program_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;

	u8 panel_r();
	template <int N> void panel_w(u8 data);
	void display_w(u8 data);
	void update_display();

	required_device<mn18801a_device> m_maincpu;
	required_device<cpu_device> m_mpscpu;
	required_device<pwm_display_device> m_display;
	required_ioport_array<13> m_panel;

	u8 m_panel_select[2];
	u8 m_display_select;
};

void psr400_state::machine_start()
{
	save_item(NAME(m_panel_select));
	save_item(NAME(m_display_select));
}

void psr400_state::machine_reset()
{
	// the MCU's port latches reset to no columns selected and all commons off
	m_panel_select[0] = m_panel_select[1] = 0;
	m_display_select = 0xff;
	update_display();
}

u8 psr400_state::panel_r()
{
	// SO0-SO7 and HS0-HS4 select the panel columns; TCIO0-TCIO7 read the active-low rows
	const u16 select = m_panel_select[0] | (u16(m_panel_select[1]) << 8);
	u8 data = 0xff;
	for (int column = 0; column < m_panel.size(); ++column)
	{
		if (BIT(select, column))
			data &= m_panel[column]->read();
	}
	return data;
}

template <int N>
void psr400_state::panel_w(u8 data)
{
	m_panel_select[N] = data;
	update_display();
}

void psr400_state::display_w(u8 data)
{
	m_display_select = data;
	update_display();
}

void psr400_state::update_display()
{
	// TO0-TO5 select the active-low commons; HS0-HS7 drive segments a-h and SO0-SO7 the LEDs
	m_display->matrix(~m_display_select & 0x3f, m_panel_select[1] | (u16(m_panel_select[0]) << 8));
}

void psr400_state::program_map(address_map &map)
{
	// 2 MB external program memory space using MMU
	map(0x000000, 0x1fffff).rom().region("program", 0);
}

void psr400_state::data_map(address_map &map)
{
	// 2 MB external data memory space using MMU
	map(0x000000, 0x000000).nopw(); // ?
	map(0x000001, 0x000001).nopr(); // ?
	map(0x000011, 0x000011).noprw(); // ?
	map(0x000014, 0x000014).noprw(); // ?
	map(0x000018, 0x000018).lr8(NAME([]() { return 0; })); // serial status?
	map(0x00001a, 0x00001a).nopw(); // serial transmit buffer?
	map(0x000030, 0x000031).ram(); // ?
	map(0x000032, 0x00003f).ram(); // firmware command staging and banked ROM pointers
	map(0x000050, 0x000053).ram(); // ?
	map(0x000055, 0x000055).noprw(); // ?
	map(0x000080, 0x03ffff).mirror(0xc0000).ram(); // 2x 1M-bit PSRAM (only one on PSR-400)
	map(0x003fe0, 0x003fff).unmaprw(); // window for more internal SFRs?
	map(0x003fe6, 0x003fe6).nopw(); // ?
	map(0x100000, 0x10000f).mirror(0xffff0).rw("gew8", FUNC(multipcm_device::read), FUNC(multipcm_device::write));
}

static INPUT_PORTS_START(psr500)
	PORT_START("S0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Accomp. Volume +")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Accomp. Volume -")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Transpose +")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Transpose -")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Tuning +")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Tuning -")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Tempo +") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Tempo -") PORT_CODE(KEYCODE_MINUS)

	PORT_START("S1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Mode")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Reverb")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Split Point")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Orchestration Rhythm")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Orchestration Bass")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Orchestration 1")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Orchestration 2")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Orchestration 3")

	PORT_START("S2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Style 0")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Style 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Style 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Style 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Style 4")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Style +")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("S3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Style 5")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Style 6")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Style 7")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Style 8")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Style 9")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Style -")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Custom Record/End")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Custom Clear")

	PORT_START("S4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Voice 0")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Voice 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Voice 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Voice 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Voice 4")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Voice +")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Type +")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Type -")

	PORT_START("S5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Voice 5")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Voice 6")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Voice 7")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Voice 8")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Voice 9")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Voice -")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Reverb Depth +")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Reverb Depth -")

	PORT_START("S6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Auto Harmony On/Off")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dual Voice On/Off")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Volume +")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Volume -")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Octave +")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Octave -")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Pan +")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Pan -")

	PORT_START("S7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Song Memory Melody 1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Song Memory Melody 2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Song Memory Melody 3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Song Memory Melody 4")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Song Memory Melody 5")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("MIDI Channel +")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("MIDI Channel -")

	PORT_START("S8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Song Memory Chord 1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Song Memory Chord 2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Song Memory Chord 3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Song Memory Chord 4")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Song Memory Chord 5")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Song Memory Conductor")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Song Memory Record")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("S9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("MIDI On/Off")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("MIDI Mode")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("MIDI Receive Channel +")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("MIDI Receive Channel -")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Demo 1") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Demo 2") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Demo 3") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Bulk Dump")

	PORT_START("S10")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Panel Memory Record")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Panel Memory Page 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Panel Memory Page 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Panel Memory Page 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Panel Memory Page 4")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Accomp. Intro")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Accomp. Ending")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Accomp. Normal")

	PORT_START("S11")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Accomp. Bridge")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Accomp. Fill 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Accomp. Fill 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Synchro Start On/Off")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Start/Stop") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Voice Memory Record")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Voice Memory 1")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Voice Memory 2")

	PORT_START("S12")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Voice Memory 3")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Voice Memory 4")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Song Memory Play/Stop")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Multi Pad Record/Stop")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Multi Pad 1")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Multi Pad 2")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Multi Pad 3")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Multi Pad 4")
INPUT_PORTS_END

void psr400_state::psr500(machine_config &config)
{
	MN18801A(config, m_maincpu, 10_MHz_XTAL); // MN18801A (also has 500 kHz secondary resonator connected to XI)
	m_maincpu->set_addrmap(AS_PROGRAM, &psr400_state::program_map);
	m_maincpu->set_addrmap(AS_DATA, &psr400_state::data_map);
	m_maincpu->so_out_cb().set(FUNC(psr400_state::panel_w<0>));
	m_maincpu->hs_out_cb().set(FUNC(psr400_state::panel_w<1>));
	m_maincpu->to_out_cb().set(FUNC(psr400_state::display_w));
	m_maincpu->tcio_in_cb().set(FUNC(psr400_state::panel_r));

	HD6305V0(config, m_mpscpu, 8_MHz_XTAL).set_disable(); // HD63B05V0D73P (mislabeled HD63B50 on schematic)

	PWM_DISPLAY(config, m_display).set_size(6, 16);
	m_display->set_segmask(0x0f, 0xff);
	config.set_default_layout(layout_psr500);

	SPEAKER(config, "speaker", 2).front();

	ym3413_device &ldsp(YM3413(config, "ldsp", 9.4_MHz_XTAL * 2 / 7)); // GEW8 DSPCLK, 64 * fs
	ldsp.set_ram_words(0x4000); // PSR-500 only: one 256K-bit PSRAM
	ldsp.add_route(1, "speaker", 1.0, 0);
	ldsp.add_route(0, "speaker", 1.0, 1);

	multipcm_device &gew8(MULTIPCM(config, "gew8", 9.4_MHz_XTAL)); // YMW-258-F
	gew8.enable_dsp_send();
	gew8.dsp_cd_callback().set(ldsp, FUNC(ym3413_device::cd_w));
	gew8.add_route(1, "speaker", 1.0, 0);
	gew8.add_route(0, "speaker", 1.0, 1);
	gew8.add_route(2, ldsp, 1.0, 0);
}

ROM_START(psr500)
	ROM_REGION(0x200000, "program", 0)
	ROM_LOAD("xj920c0.ic4", 0x000000, 0x040000, CRC(bd45d962) SHA1(fe46ceae5584b56e36f31f27bedd9e7d578eb35b))
	ROM_RELOAD(0x040000, 0x040000)
	ROM_RELOAD(0x080000, 0x040000)
	ROM_RELOAD(0x0c0000, 0x040000)
	ROM_LOAD("xj921b0.ic5", 0x100000, 0x100000, CRC(dd1a8afc) SHA1(5d5b47577faeed165f0bd73283f148d112e4d1e9))

	ROM_REGION(0x100000, "gew8", 0)
	ROM_LOAD("xj426b0.ic3", 0x000000, 0x100000, CRC(ef566734) SHA1(864f5689dbaa82bd8a1be4e53bdb21ec71be03cc))

	ROM_REGION(0x1000, "mpscpu", 0)
	ROM_LOAD("xj450a00.ic1", 0x0000, 0x1000, NO_DUMP)
ROM_END

void psr400_state::init_psr500()
{
	memory_region *region = memregion("program");
	u8 *program = static_cast<u8 *>(region->base());
	std::vector<u8> buf(0x20000);

	// A8-A14 & A16 are scrambled
	for (u32 offset = 0; offset < region->bytes(); )
	{
		std::copy(&program[offset], &program[offset + 0x20000], buf.begin());
		for (int blocks = 0x200; blocks-- > 0; offset += 0x100)
			std::copy_n(&buf[bitswap<9>(offset, 9, 15, 10, 11, 8, 14, 16, 13, 12) << 8], 0x100, &program[offset]);
	}
}

} // anonymous namespace

SYST(1991, psr500, 0, 0, psr500, psr500, psr400_state, init_psr500, "Yamaha", "PortaTone PSR-500", MACHINE_NOT_WORKING)
