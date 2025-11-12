// license:BSD-3-Clause
// copyright-holders:Felipe Sanches
/****************************************************************************

    Roland PG-1000 programmer.

    To be used alongside Roland D-50/D-550/MT-32

-----------------------------------------------------------------------------

    Notes:
        midiin1 is the usual "MIDI IN" port.
        midiin2 is the "Parameter In" port.

    usage:
        mame pg1000 -midiin1 "ctrl" -midiin2 "synth" -midiout "synth"

    where:
        "ctrl" is a MIDI OUT device such as
        an external usb midi keyboard controller.

        "synth" is a D-50 or a D-550.
        (and I guess it may work with a Roland MT-32 as well)

    During development, I tested this setup using a real Roland D-550.

-----------------------------------------------------------------------------

    Known driver bug:

    This driver is almost perfectly functional, but remains marked with the
    MACHINE_NOT_WORKING flag due to an unresolved issue related to its
    MIDI ports.

    While running it hooked up to an USB-MIDI controller, the MIDI messages
    are not perfectly repeated from MIDI IN to MIDI OUT as they should be.
    Instead, sometimes we get some data corruption, specially when using
    the sustain pedal.

    I suspect this may be some CPU bug, perhaps an incorrect or incomplete
    implementation of one of the upd7810 instructions used in the serial
    port code-path.

****************************************************************************/

#include "emu.h"
#include "bus/midi/midi.h"
#include "cpu/upd7810/upd7810.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"
#include "pg1000.lh"

namespace {

class pg1000_state : public driver_device
{
public:
	pg1000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lcdc(*this, "lcdc")
		, m_row(*this, "ROW%u", 0U)
		, m_led(*this, "LED%u", 0U)
		, m_top_slider(*this, "top_slider_%u", 0U)
		, m_middle_slider(*this, "middle_slider_%u", 0U)
		, m_bottom_slider(*this, "bottom_slider_%u", 0U)
		, m_paramin(*this, "paramin")
		, m_mdin(*this, "mdin")
		, m_mdout(*this, "mdout")
		, m_scan(0)
		, m_an_select(0)
		, m_mdin_bit(true)
		, m_paramin_bit(true)
		, m_midi_in_enable(false)
		, m_param_in_enable(false)
	{
	}

	void pg1000(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	u8 sw_r();
	void led_w(u8 data);
	void mem_map(address_map &map) ATTR_COLD;
	void palette_init(palette_device &palette);

	required_device<upd7810_device> m_maincpu;
	required_device<hd44780_device> m_lcdc;
	required_ioport_array<2> m_row;
	output_finder<6> m_led;
	required_ioport_array<13> m_top_slider;
	required_ioport_array<23> m_middle_slider;
	required_ioport_array<20> m_bottom_slider;
	optional_device<midi_port_device> m_paramin;
	optional_device<midi_port_device> m_mdin;
	optional_device<midi_port_device> m_mdout;

	u8 m_scan;
	u8 m_an_select;
	bool m_mdin_bit;
	bool m_paramin_bit;
	bool m_midi_in_enable;
	bool m_param_in_enable;
};


void pg1000_state::machine_start()
{
	m_led.resolve();

	save_item(NAME(m_scan));
	save_item(NAME(m_an_select));
	save_item(NAME(m_mdin_bit));
	save_item(NAME(m_paramin_bit));
	save_item(NAME(m_midi_in_enable));
	save_item(NAME(m_param_in_enable));
}

u8 pg1000_state::sw_r()
{
	u8 value = 0xff;
	if (!BIT(m_scan, 1))
		value = m_row[1]->read();

	if (!BIT(m_scan, 0))
		value = (value & 0xfc) | (m_row[0]->read() & 3);

	return value;
}

void pg1000_state::led_w(u8 data)
{
	m_scan = data & 3;
	for (int i=0; i<=5; i++)
		m_led[i] = BIT(data, i+2);
}

static INPUT_PORTS_START(pg1000)
	PORT_START("ROW0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("MANUAL") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PARAMETER REQUEST") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PREVIOUS VALUE") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("MIDI CHANNEL") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("COMMON SELECT UPPER") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("COMMON SELECT LOWER") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PARTIAL SELECT UPPER 1") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PARTIAL SELECT UPPER 2") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PARTIAL SELECT LOWER 1") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PARTIAL SELECT LOWER 2") PORT_CODE(KEYCODE_S)


	PORT_START("top_slider_0")
	PORT_ADJUSTER(128, "WG PITCH: COARSE") PORT_MINMAX(0, 255)

	PORT_START("top_slider_1")
	PORT_ADJUSTER(128, "WG PITCH: FINE") PORT_MINMAX(0, 255)

	PORT_START("top_slider_2")
	PORT_ADJUSTER(128, "WG PITCH: KF") PORT_MINMAX(0, 255)

	PORT_START("top_slider_3")
	PORT_ADJUSTER(128, "WG PITCH: LFO MODE") PORT_MINMAX(0, 255)

	PORT_START("top_slider_4")
	PORT_ADJUSTER(128, "WG PITCH: ENV MODE") PORT_MINMAX(0, 255)

	PORT_START("top_slider_5")
	PORT_ADJUSTER(128, "WG PITCH: BEND MODE") PORT_MINMAX(0, 255)

	PORT_START("top_slider_6")
	PORT_ADJUSTER(128, "WG WAVEFORM: WF") PORT_MINMAX(0, 255)

	PORT_START("top_slider_7")
	PORT_ADJUSTER(128, "WG WAVEFORM: PW") PORT_MINMAX(0, 255)

	PORT_START("top_slider_8")
	PORT_ADJUSTER(128, "WG WAVEFORM: PW VELO") PORT_MINMAX(0, 255)

	PORT_START("top_slider_9")
	PORT_ADJUSTER(128, "WG WAVEFORM: PW AFTER") PORT_MINMAX(0, 255)

	PORT_START("top_slider_10")
	PORT_ADJUSTER(128, "WG WAVEFORM: PWM LFO SEL") PORT_MINMAX(0, 255)

	PORT_START("top_slider_11")
	PORT_ADJUSTER(128, "WG WAVEFORM: PWM DEPTH") PORT_MINMAX(0, 255)

	PORT_START("top_slider_12")
	PORT_ADJUSTER(128, "WG WAVEFORM: PCM") PORT_MINMAX(0, 255)


	PORT_START("middle_slider_0")
	PORT_ADJUSTER(128, "TVF: CUTOFF FREQ") PORT_MINMAX(0, 255)

	PORT_START("middle_slider_1")
	PORT_ADJUSTER(128, "TVF: RESO") PORT_MINMAX(0, 255)

	PORT_START("middle_slider_2")
	PORT_ADJUSTER(128, "TVF: KF") PORT_MINMAX(0, 255)

	PORT_START("middle_slider_3")
	PORT_ADJUSTER(128, "TVF: BIAS DIREC") PORT_MINMAX(0, 255)

	PORT_START("middle_slider_4")
	PORT_ADJUSTER(128, "TVF: BIAS POINT") PORT_MINMAX(0, 255)

	PORT_START("middle_slider_5")
	PORT_ADJUSTER(128, "TVF: BIAS LEVEL") PORT_MINMAX(0, 255)

	PORT_START("middle_slider_6")
	PORT_ADJUSTER(128, "TVF: ENV DEPTH") PORT_MINMAX(0, 255)

	PORT_START("middle_slider_7")
	PORT_ADJUSTER(128, "TVF: ENV VELO") PORT_MINMAX(0, 255)

	PORT_START("middle_slider_8")
	PORT_ADJUSTER(128, "TVF: LFO SELECT") PORT_MINMAX(0, 255)

	PORT_START("middle_slider_9")
	PORT_ADJUSTER(128, "TVF: LFO DEPTH") PORT_MINMAX(0, 255)

	PORT_START("middle_slider_10")
	PORT_ADJUSTER(128, "TVF: AFTER RANGE") PORT_MINMAX(0, 255)

	PORT_START("middle_slider_11")
	PORT_ADJUSTER(128, "TVF ENV: T1") PORT_MINMAX(0, 255)

	PORT_START("middle_slider_12")
	PORT_ADJUSTER(128, "TVF ENV: T2") PORT_MINMAX(0, 255)

	PORT_START("middle_slider_13")
	PORT_ADJUSTER(128, "TVF ENV: T3") PORT_MINMAX(0, 255)

	PORT_START("middle_slider_14")
	PORT_ADJUSTER(128, "TVF ENV: T4") PORT_MINMAX(0, 255)

	PORT_START("middle_slider_15")
	PORT_ADJUSTER(128, "TVF ENV: T5") PORT_MINMAX(0, 255)

	PORT_START("middle_slider_16")
	PORT_ADJUSTER(128, "TVF ENV: TKF") PORT_MINMAX(0, 255)

	PORT_START("middle_slider_17")
	PORT_ADJUSTER(128, "TVA ENV: T1") PORT_MINMAX(0, 255)

	PORT_START("middle_slider_18")
	PORT_ADJUSTER(128, "TVA ENV: T2") PORT_MINMAX(0, 255)

	PORT_START("middle_slider_19")
	PORT_ADJUSTER(128, "TVA ENV: T3") PORT_MINMAX(0, 255)

	PORT_START("middle_slider_20")
	PORT_ADJUSTER(128, "TVA ENV: T4") PORT_MINMAX(0, 255)

	PORT_START("middle_slider_21")
	PORT_ADJUSTER(128, "TVA ENV: T5") PORT_MINMAX(0, 255)

	PORT_START("middle_slider_22")
	PORT_ADJUSTER(128, "TVA ENV: TKF") PORT_MINMAX(0, 255)


	PORT_START("bottom_slider_0")
	PORT_ADJUSTER(128, "TVA: LEVEL") PORT_MINMAX(0, 255)

	PORT_START("bottom_slider_1")
	PORT_ADJUSTER(128, "TVA: VELO") PORT_MINMAX(0, 255)

	PORT_START("bottom_slider_2")
	PORT_ADJUSTER(128, "TVA: BIAS DIREC") PORT_MINMAX(0, 255)

	PORT_START("bottom_slider_3")
	PORT_ADJUSTER(128, "TVA: BIAS POINT") PORT_MINMAX(0, 255)

	PORT_START("bottom_slider_4")
	PORT_ADJUSTER(128, "TVA: BIAS LEVEL") PORT_MINMAX(0, 255)

	PORT_START("bottom_slider_5")
	PORT_ADJUSTER(128, "TVA: LFO SELECT") PORT_MINMAX(0, 255)

	PORT_START("bottom_slider_6")
	PORT_ADJUSTER(128, "TVA: LFO DEPTH") PORT_MINMAX(0, 255)

	PORT_START("bottom_slider_7")
	PORT_ADJUSTER(128, "TVA: AFTER RANGE") PORT_MINMAX(0, 255)

	PORT_START("bottom_slider_8")
	PORT_ADJUSTER(128, "TVF ENV: L1") PORT_MINMAX(0, 255)

	PORT_START("bottom_slider_9")
	PORT_ADJUSTER(128, "TVF ENV: L2") PORT_MINMAX(0, 255)

	PORT_START("bottom_slider_10")
	PORT_ADJUSTER(128, "TVF ENV: L3") PORT_MINMAX(0, 255)

	PORT_START("bottom_slider_11")
	PORT_ADJUSTER(128, "TVF ENV: SUS L") PORT_MINMAX(0, 255)

	PORT_START("bottom_slider_12")
	PORT_ADJUSTER(128, "TVF ENV: END L") PORT_MINMAX(0, 255)

	PORT_START("bottom_slider_13")
	PORT_ADJUSTER(128, "TVF ENV: DKF") PORT_MINMAX(0, 255)

	PORT_START("bottom_slider_14")
	PORT_ADJUSTER(128, "TVA ENV: L1") PORT_MINMAX(0, 255)

	PORT_START("bottom_slider_15")
	PORT_ADJUSTER(128, "TVA ENV: L2") PORT_MINMAX(0, 255)

	PORT_START("bottom_slider_16")
	PORT_ADJUSTER(128, "TVA ENV: L3") PORT_MINMAX(0, 255)

	PORT_START("bottom_slider_17")
	PORT_ADJUSTER(128, "TVA ENV: SUS L") PORT_MINMAX(0, 255)

	PORT_START("bottom_slider_18")
	PORT_ADJUSTER(128, "TVA ENV: END L") PORT_MINMAX(0, 255)

	PORT_START("bottom_slider_19")
	PORT_ADJUSTER(128, "TVA ENV: T1 VELO") PORT_MINMAX(0, 255)
INPUT_PORTS_END


void pg1000_state::mem_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("maincpu", 0);
	map(0xa000, 0xa000).lw8(NAME([this] (u8 data) {
		m_lcdc->db_w(data);
		m_lcdc->e_w(0);
		m_lcdc->e_w(1);
	}));
	map(0xc000, 0xdfff).ram();
}

void pg1000_state::palette_init(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t(11, 183, 253));
	palette.set_pen_color(1, rgb_t(0, 60, 130));
}

void pg1000_state::pg1000(machine_config &config)
{
	MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set(
		[this] (int state) { m_mdin_bit = state; }
	);

	MIDI_PORT(config, "paramin", midiin_slot, "midiin").rxd_handler().set(
		[this] (int state) { m_paramin_bit = state; }
	);

	MIDI_PORT(config, "mdout", midiout_slot, "midiout");

	UPD78C10(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &pg1000_state::mem_map);
	m_maincpu->pa_in_cb().set(FUNC(pg1000_state::sw_r));
	m_maincpu->pb_out_cb().set(FUNC(pg1000_state::led_w));
	m_maincpu->rxd_func().set([this]() {
		return int(
			(m_mdin_bit && m_midi_in_enable) ||
			(m_paramin_bit && m_param_in_enable) ||
			(!m_midi_in_enable && !m_param_in_enable) /* due to pull-up */
		);
	});
	m_maincpu->pc_out_cb().set([this](u8 data)
	{
		m_mdout->write_txd(BIT(data, 0));
		m_midi_in_enable = BIT(data, 2);
		m_param_in_enable = BIT(data, 3);
		m_lcdc->rs_w(BIT(data, 4));
		m_an_select = (data >> 5) & 7;
	});
	m_maincpu->an0_func().set([this] { /* IC11 */
		switch(m_an_select)
		{
			case 7: return m_bottom_slider[1]->read(); /* TVA: VELO */
			case 6: return m_bottom_slider[0]->read(); /* TVA: LEVEL */
			case 5: return m_bottom_slider[2]->read(); /* TVA: BIAS DIREC */
			case 4: return m_middle_slider[0]->read(); /* TVF: CUTOFF FREQ */
			case 3: return m_middle_slider[4]->read(); /* TVF: BIAS POINT */
			case 2: return m_middle_slider[1]->read(); /* TVF: RESO */
			case 1: return m_middle_slider[2]->read(); /* TVF: KF */
			case 0: return m_middle_slider[3]->read(); /* TVF: BIAS DIREC */
		}
		return (unsigned int) 0xff; // std::unreachable();
	});
	m_maincpu->an1_func().set([this] { /* IC14 */
		switch(m_an_select)
		{
			case 7: return m_bottom_slider[13]->read(); /* TVF ENV: DKF */
			case 6: return m_bottom_slider[12]->read(); /* TVF ENV: END L */
			case 5: return m_bottom_slider[14]->read(); /* TVA ENV: L1 */
			case 4: return m_bottom_slider[11]->read(); /* TVF ENV: SUS L */
			case 3: return m_middle_slider[17]->read(); /* TVA ENV: T1 */
			case 2: return m_middle_slider[14]->read(); /* TVF ENV: T4 */
			case 1: return m_middle_slider[15]->read(); /* TVF ENV: T5 */
			case 0: return m_middle_slider[16]->read(); /* TVF ENV: TKF */
		}
		return (unsigned int) 0xff; // std::unreachable();
	});
	m_maincpu->an2_func().set([this] { /* IC8 */
		switch(m_an_select)
		{
			case 7: return m_top_slider[12]->read(); /* WG WAVEFORM: PCM */
			case 6: return m_middle_slider[22]->read(); /* TVA ENV: TKF */
			case 5: return m_top_slider[11]->read(); /* WG WAVEFORM: PWM DEPTH */
			case 4: return m_middle_slider[21]->read(); /* TVA ENV: T5 */
			case 3: return m_top_slider[10]->read(); /* WG WAVEFORM: PWM LFO SEL */
			case 2: return m_top_slider[7]->read(); /* WG WAVEFORM: PW */
			case 1: return m_top_slider[8]->read(); /* WG WAVEFORM: PW VELO */
			case 0: return m_top_slider[9]->read(); /* WG WAVEFORM: PW AFTER */
		}
		return (unsigned int) 0xff; // std::unreachable();
	});
	m_maincpu->an3_func().set([this] { /* IC13 */
		switch(m_an_select)
		{
			case 7: return m_bottom_slider[9]->read(); /* TVF ENV: L2 */
			case 6: return m_bottom_slider[8]->read(); /* TVF ENV: L1 */
			case 5: return m_bottom_slider[10]->read(); /* TVF ENV: L3 */
			case 4: return m_bottom_slider[7]->read(); /* TVA: AFTER RANGE */
			case 3: return m_middle_slider[13]->read(); /* TVF ENV: T3 */
			case 2: return m_middle_slider[9]->read(); /* TVF: LFO DEPTH */
			case 1: return m_middle_slider[10]->read(); /* TVF: AFTER RANGE */
			case 0: return m_middle_slider[11]->read(); /* TVF ENV: T1 */
		}
		return (unsigned int) 0xff; // std::unreachable();
	});
	m_maincpu->an4_func().set([this] { /* IC9 */
		switch(m_an_select)
		{
			case 7: return m_top_slider[0]->read(); /* WG PITCH: COARSE */
			case 6: return m_top_slider[1]->read(); /* WG PITCH: FINE */
			case 5: return m_middle_slider[12]->read(); /* TVF ENV: T2 */
			case 4: return m_top_slider[2]->read(); /* WG PITCH: KF */
			case 3: return m_top_slider[6]->read(); /* WG WAVEFORM: WF */
			case 2: return m_top_slider[3]->read(); /* WG PITCH: LFO MODE */
			case 1: return m_top_slider[4]->read(); /* WG PITCH: ENV MODE */
			case 0: return m_top_slider[5]->read(); /* WG PITCH: BEND MODE */
		}
		return (unsigned int) 0xff; // std::unreachable();
	});
	m_maincpu->an5_func().set([this] { /* IC12 */
		switch(m_an_select)
		{
			case 7: return m_bottom_slider[5]->read(); /* TVA: LFO SELECT */
			case 6: return m_bottom_slider[4]->read(); /* TVA: BIAS LEVEL */
			case 5: return m_bottom_slider[6]->read(); /* TVA: LFO DEPTH */
			case 4: return m_bottom_slider[3]->read(); /* TVA: BIAS POINT */
			case 3: return m_middle_slider[8]->read(); /* TVF: LFO SELECT */
			case 2: return m_middle_slider[5]->read(); /* TVF: BIAS LEVEL */
			case 1: return m_middle_slider[6]->read(); /* TVF: ENV DEPTH */
			case 0: return m_middle_slider[7]->read(); /* TVF: ENV VELO */
		}
		return (unsigned int) 0xff; // std::unreachable();
	});
	m_maincpu->an6_func().set([this] { /* IC15 */
		switch(m_an_select)
		{
			case 7: return m_bottom_slider[17]->read(); /* TVA ENV: SUS L */
			case 6: return m_bottom_slider[16]->read(); /* TVA ENV: L3 */
			case 5: return m_bottom_slider[18]->read(); /* TVA ENV: END L */
			case 4: return m_bottom_slider[15]->read(); /* TVA ENV: L2 */
			case 3: return m_bottom_slider[19]->read(); /* TVA ENV: T1 VELO */
			case 2: return m_middle_slider[18]->read(); /* TVA ENV: T2 */
			case 1: return m_middle_slider[19]->read(); /* TVA ENV: T3 */
			case 0: return m_middle_slider[20]->read(); /* TVA ENV: T4 */
		}
		return (unsigned int) 0xff; // std::unreachable();
	});

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_screen_update("lcdc", FUNC(hd44780_device::screen_update));
	screen.set_size(6*16, 8*2+1);
	screen.set_visarea_full();
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(pg1000_state::palette_init), 2);

	/* Actual device is LM16255 */
	HD44780(config, m_lcdc, 270'000); // TODO: clock not measured, datasheet typical clock used
	m_lcdc->set_lcd_size(2, 16);

	config.set_default_layout(layout_pg1000);
}

ROM_START(pg1000)
	ROM_DEFAULT_BIOS("v2.00")
	ROM_SYSTEM_BIOS(0, "v2.00", "Version 2.00")
	ROM_SYSTEM_BIOS(1, "v1.01", "Version 1.01")
	ROM_SYSTEM_BIOS(2, "v1.00", "Version 1.00")

	ROM_REGION(0x8000, "maincpu", 0)
	ROMX_LOAD("roland_pg-1000_v2.00.ic4", 0x000, 0x2000, CRC(c8bc1f62) SHA1(796d5efd09b411d370f93b32283aa33a4435dec4), ROM_BIOS(0))
	ROMX_LOAD("roland_pg-1000_v1.01.ic4", 0x000, 0x2000, CRC(9f9bcf76) SHA1(da5a45c65a04c35d7a615c6f043ccaf958b0d65e), ROM_BIOS(1))
	ROMX_LOAD("roland_pg-1000_v1.00.ic4", 0x000, 0x2000, CRC(c09ef84e) SHA1(d780d4d53e57918e6ea8098f54f5c9b43aeec287), ROM_BIOS(2))
ROM_END

} // anonymous namespace

SYST(1987, pg1000,  0,   0, pg1000,  pg1000, pg1000_state, empty_init, "Roland", "PG-1000 Linear Synthesizer Programmer", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE)
