// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/****************************************************************************************

PINBALL
Williams System 11A

Games:
- F-14 Tomcat (#554)
- Fire! (#556)
- Fire! (Champagne Edition) (#556-SE)
- Millionaire (#555)
- Pin-bot (#549)
- Pin-bot (kit for Unidesa)

If it says FACTORY SETTING, hit F3.

Here are the key codes to enable play:

Game              NUM  Start game                                End ball
-------------------------------------------------------------------------------------------------------------------------------
Pin-bot           549  Hold IJ hit 1                             unknown
F-14 Tomcat       554  Hold CDEF hit 1 then X to enable scoring. CDEF (wait for next ball to start) hit X
Millionaire       555  Hold YZ hit 1                             wait for wop-wop noise to end, then hit . hold Z hit . hold Y
Fire!             556  Hold PGUP PGDN Space hit 1                PGUP PGDN Space

Status:
- All machines are playable

ToDo:
- Some digits flicker

*****************************************************************************************/

#include "emu.h"
#include "includes/s11.h"

#include "cpu/m6809/m6809.h"
#include "speaker.h"

#include "s11a.lh"


static INPUT_PORTS_START( s11a )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_NAME("Plumb Tilt")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("High Score Reset")

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Playfield Tilt")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP10")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP11")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP12")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP13")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP14")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP15")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP16")

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP17")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP18")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP19")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP20")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP21")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP22")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP23")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP24")

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP25")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP26")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP27")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP28")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP29")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP30")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP31")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("INP32")

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP33")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP34")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP35")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP36")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP37")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP38")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP39")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP40")

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP41")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP42")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("INP43")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("INP44")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("INP45")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("INP46")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME("INP47")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP) PORT_NAME("INP48")

	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("INP49")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("INP50")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_HOME) PORT_NAME("INP51")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_END) PORT_NAME("INP52")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_NAME("INP53")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGDN) PORT_NAME("INP54")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGUP) PORT_NAME("INP55")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SPACE) PORT_NAME("INP56")

	PORT_START("X7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RALT) PORT_NAME("INP57")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME("INP58")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("INP59")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("INP60")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("INP61")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME("INP62")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME("INP63")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("INP64")

	PORT_START("DIAGS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Audio Diag") PORT_CODE(KEYCODE_9_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, s11a_state, audio_nmi, 1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Main Diag") PORT_CODE(KEYCODE_0_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, s11a_state, main_nmi, 1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Advance") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Up/Down") PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE
	PORT_CONFNAME( 0x10, 0x10, "Language" )
	PORT_CONFSETTING( 0x00, "German" )
	PORT_CONFSETTING( 0x10, "English" )
INPUT_PORTS_END

void s11a_state::s11a_dig0_w(u8 data)
{
	set_strobe(data & 15);
	u8 diag = BIT(data, 4, 3);
	if (diag == get_diag())
	{
		set_lock1(0);
		set_lock2(0);
	}
	else
		set_diag(diag);
	m_digits[60] = 0;  // +5VDC (always on)
	m_digits[61] = BIT(data, 4);  // connected to PA4
	m_digits[62] = 0;  // Blanking (pretty much always on)
	set_segment1(0);
	set_segment2(0);
}

void s11a_state::init_s11a()
{
	s11_state::init_s11();
}

void s11a_state::s11a_base(machine_config &config)
{
	/* basic machine hardware */
	M6808(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &s11a_state::s11_main_map);
	INPUT_MERGER_ANY_HIGH(config, m_mainirq).output_handler().set(FUNC(s11a_state::main_irq));
	INPUT_MERGER_ANY_HIGH(config, m_piairq).output_handler().set(FUNC(s11a_state::pia_irq));

	/* Video */
	config.set_default_layout(layout_s11a);

	/* Sound */
	genpin_audio(config);

	/* Devices */
	PIA6821(config, m_pia21, 0);
	m_pia21->readpa_handler().set(FUNC(s11a_state::sound_r));
	m_pia21->set_port_a_input_overrides_output_mask(0xff);
	m_pia21->writepa_handler().set(FUNC(s11a_state::sound_w));
	m_pia21->writepb_handler().set(FUNC(s11a_state::sol2_w));
	m_pia21->ca2_handler().set(FUNC(s11a_state::pia21_ca2_w));
	m_pia21->cb2_handler().set(FUNC(s11a_state::pia21_cb2_w));
	m_pia21->irqa_handler().set(m_piairq, FUNC(input_merger_device::in_w<1>));
	m_pia21->irqb_handler().set(m_piairq, FUNC(input_merger_device::in_w<2>));

	PIA6821(config, m_pia24, 0);
	m_pia24->writepa_handler().set(FUNC(s11a_state::lamp0_w));
	m_pia24->writepb_handler().set(FUNC(s11a_state::lamp1_w));
	m_pia24->cb2_handler().set(FUNC(s11a_state::pia24_cb2_w));
	m_pia24->irqa_handler().set(m_piairq, FUNC(input_merger_device::in_w<3>));
	m_pia24->irqb_handler().set(m_piairq, FUNC(input_merger_device::in_w<4>));

	PIA6821(config, m_pia28, 0);
	m_pia28->readpa_handler().set(FUNC(s11a_state::pia28_w7_r));
	m_pia28->set_port_a_input_overrides_output_mask(0xff);
	m_pia28->writepa_handler().set(FUNC(s11a_state::s11a_dig0_w));
	m_pia28->writepb_handler().set(FUNC(s11a_state::dig1_w));
	m_pia28->ca2_handler().set(FUNC(s11a_state::pia28_ca2_w));
	m_pia28->cb2_handler().set(FUNC(s11a_state::pia28_cb2_w));
	m_pia28->irqa_handler().set(m_piairq, FUNC(input_merger_device::in_w<5>));
	m_pia28->irqb_handler().set(m_piairq, FUNC(input_merger_device::in_w<6>));

	PIA6821(config, m_pia2c, 0);
	m_pia2c->writepa_handler().set(FUNC(s11a_state::pia2c_pa_w));
	m_pia2c->writepb_handler().set(FUNC(s11a_state::pia2c_pb_w));
	m_pia2c->ca2_handler().set(FUNC(s11a_state::pia2c_ca2_w));
	m_pia2c->cb2_handler().set(FUNC(s11a_state::pia2c_cb2_w));
	m_pia2c->irqa_handler().set(m_piairq, FUNC(input_merger_device::in_w<7>));
	m_pia2c->irqb_handler().set(m_piairq, FUNC(input_merger_device::in_w<8>));

	PIA6821(config, m_pia30, 0);
	m_pia30->readpa_handler().set(FUNC(s11a_state::switch_r));
	m_pia30->set_port_a_input_overrides_output_mask(0xff);
	m_pia30->writepb_handler().set(FUNC(s11a_state::switch_w));
	m_pia30->ca2_handler().set(FUNC(s11a_state::pia30_ca2_w));
	m_pia30->cb2_handler().set(FUNC(s11a_state::pia30_cb2_w));
	m_pia30->irqa_handler().set(m_piairq, FUNC(input_merger_device::in_w<9>));
	m_pia30->irqb_handler().set(m_piairq, FUNC(input_merger_device::in_w<10>));

	PIA6821(config, m_pia34, 0);
	m_pia34->writepa_handler().set(FUNC(s11a_state::pia34_pa_w));
	m_pia34->writepb_handler().set(FUNC(s11a_state::pia34_pb_w));
	m_pia34->ca2_handler().set_nop();
	m_pia34->cb2_handler().set(FUNC(s11a_state::pia34_cb2_w));
	m_pia34->irqa_handler().set(m_piairq, FUNC(input_merger_device::in_w<11>));
	m_pia34->irqb_handler().set(m_piairq, FUNC(input_merger_device::in_w<12>));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	/* Add the soundcard */
	M6802(config, m_audiocpu, XTAL(4'000'000));
	m_audiocpu->set_ram_enable(false);
	m_audiocpu->set_addrmap(AS_PROGRAM, &s11a_state::s11_audio_map);
	INPUT_MERGER_ANY_HIGH(config, m_audioirq).output_handler().set_inputline(m_audiocpu, M6802_IRQ_LINE);

	MC1408(config, m_dac, 0);

	// common CVSD filter for system 11 and 11a, this is also the same filter circuit as Sinistar/System 6 uses,
	// and is ALMOST the same filter from the s11 bg sound boards, see /mame/audio/s11c_bg.cpp
	// The CVSD filter has a large gain, about 4.6x
	// The filter is boosting the ~5vpp audio signal from the CVSD chip to a ~23vpp (really ~17vpp) theoretical audio signal that the s11
	// mainboard outputs on its volume control-repurposed-as-audio-out connector.
	// In reality, the S11 mainboard outputs audio at a virtual ground level between +5v and -12v (so, 17VPP balanced around -7VDC), but since
	// the CVSD chip's internal DAC can only output between a bit over +0x180/-0x180 out of 0x200, the most voltage it can ever output is
	// between (assuming 0x1ff is 5VDC and 0x300 is 0VDC) a max of 4.375VDC and a min of 0.625VDC, i.e. 3.75VPP centered on 2.5VDC.
	// In reality, the range is likely less than that.
	// This means multiplying a 3.75VPP signal by 4.6 is 17.25VPP, which is almost exactly the expected 17V (12v+5v) VPP the output should have.
	FILTER_BIQUAD(config, m_cvsd_filter2).opamp_mfb_lowpass_setup(RES_K(27), RES_K(15), RES_K(27), CAP_P(4700), CAP_P(1200));
	FILTER_BIQUAD(config, m_cvsd_filter).opamp_mfb_lowpass_setup(RES_K(43), RES_K(36), RES_K(180), CAP_P(1800), CAP_P(180));
	m_cvsd_filter->add_route(ALL_OUTPUTS, m_cvsd_filter2, 1.0);
	HC55516(config, m_hc55516, 0).add_route(ALL_OUTPUTS, m_cvsd_filter, 1.0);

	PIA6821(config, m_pias, 0);
	m_pias->readpa_handler().set(FUNC(s11a_state::sound_r));
	m_pias->set_port_a_input_overrides_output_mask(0xff);
	m_pias->writepa_handler().set(FUNC(s11a_state::sound_w));
	m_pias->writepb_handler().set(m_dac, FUNC(dac_byte_interface::data_w));
	m_pias->ca2_handler().set(m_hc55516, FUNC(hc55516_device::clock_w));
	m_pias->cb2_handler().set(m_hc55516, FUNC(hc55516_device::digit_w));
	m_pias->irqa_handler().set(m_audioirq, FUNC(input_merger_device::in_w<0>));
	m_pias->irqb_handler().set(m_audioirq, FUNC(input_merger_device::in_w<1>));
}

void s11a_state::s11a(machine_config &config)
{
	s11a_base(config);
	/* Add the background music card */
	S11_BG(config, m_bg);
	m_dac->add_route(ALL_OUTPUTS, m_bg, 0.4484/2.0);
	m_cvsd_filter2->add_route(ALL_OUTPUTS, m_bg, 0.4484/2.0);
	m_pia34->ca2_handler().set(m_bg, FUNC(s11_bg_device::resetq_w));
	m_bg->pb_cb().set(m_pia34, FUNC(pia6821_device::portb_w));
	m_bg->cb2_cb().set(m_pia34, FUNC(pia6821_device::cb1_w));
	SPEAKER(config, "speaker").front_center();
	m_bg->add_route(ALL_OUTPUTS, "speaker", 1.0);
}

void s11a_state::s11a_obg(machine_config &config)
{
	s11a_base(config);
	/* Add the older-style background music card */
	S11_OBG(config, m_bg);
	m_dac->add_route(ALL_OUTPUTS, m_bg, 0.5319/2.0);
	m_cvsd_filter2->add_route(ALL_OUTPUTS, m_bg, 0.5319/2.0);
	m_pia34->ca2_handler().set(m_bg, FUNC(s11_obg_device::resetq_w));
	m_bg->pb_cb().set(m_pia34, FUNC(pia6821_device::portb_w));
	m_bg->cb2_cb().set(m_pia34, FUNC(pia6821_device::cb1_w));
	SPEAKER(config, "speaker").front_center();
	m_bg->add_route(ALL_OUTPUTS, "speaker", 1.0);
}

/*------------------------
/ F14 Tomcat 5/87 (#554)
/-------------------------*/

ROM_START(f14_p3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("f14_l3.u26", 0x4000, 0x4000, CRC(cd607556) SHA1(2ec95085784370a071cbf5df7ae5c6b4749605e2))
	ROM_LOAD("f14_l3.u27", 0x8000, 0x8000, CRC(72951fd1) SHA1(b5f3fe1859e0abf9ab558b4b4f6754134d528c23))

	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("f14_u21.l1", 0x8000, 0x8000, CRC(e412300c) SHA1(382d0cfa47abea295f0c7501bc0a010473e9d73b))
	ROM_LOAD("f14_u22.l1", 0x0000, 0x8000, CRC(c9dd7496) SHA1(de3cb855d87033274cc912578b02d1593d2d69f9))

	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("f14_u4.l1", 0x00000, 0x8000, CRC(43ecaabf) SHA1(64b50dbff03cd556130d0cff47b951fdf37d397d))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("f14_u19.l1", 0x20000, 0x8000, CRC(d0de4a7c) SHA1(46ecd5786653add47751cc56b38d9db7c4622377))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

ROM_START(f14_p4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26_l4.128", 0x4000, 0x4000, CRC(7b39706a) SHA1(0dc0b1a1dfd12bc73e6fd8b825fe72ddc8fc1497))
	ROM_LOAD("u27_l4.256", 0x8000, 0x8000, CRC(189f9488) SHA1(7536d56cb83bf29f8d8b03b226a5f60200776095))

	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("f14_u21.l1", 0x8000, 0x8000, CRC(e412300c) SHA1(382d0cfa47abea295f0c7501bc0a010473e9d73b))
	ROM_LOAD("f14_u22.l1", 0x0000, 0x8000, CRC(c9dd7496) SHA1(de3cb855d87033274cc912578b02d1593d2d69f9))

	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("f14_u4.l1", 0x00000, 0x8000, CRC(43ecaabf) SHA1(64b50dbff03cd556130d0cff47b951fdf37d397d))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("f14_u19.l1", 0x20000, 0x8000, CRC(d0de4a7c) SHA1(46ecd5786653add47751cc56b38d9db7c4622377))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

ROM_START(f14_p5)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("f14_u26.p5", 0x4000, 0x4000, CRC(f5d9b132) SHA1(b6a5edf8f015ae86513cd28ce2436f3c07199d47))
	ROM_LOAD("f14_u27.p5", 0x8000, 0x8000, CRC(45de7e15) SHA1(a3160cbc0d3a5eb4cdd301251c40806e7c1d3ee8))

	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("f14_u21.l1", 0x8000, 0x8000, CRC(e412300c) SHA1(382d0cfa47abea295f0c7501bc0a010473e9d73b))
	ROM_LOAD("f14_u22.l1", 0x0000, 0x8000, CRC(c9dd7496) SHA1(de3cb855d87033274cc912578b02d1593d2d69f9))

	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("f14_u4.l1", 0x00000, 0x8000, CRC(43ecaabf) SHA1(64b50dbff03cd556130d0cff47b951fdf37d397d))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("f14_u19.l1", 0x20000, 0x8000, CRC(d0de4a7c) SHA1(46ecd5786653add47751cc56b38d9db7c4622377))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

ROM_START(f14_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("f14_u26.l1", 0x4000, 0x4000, CRC(62c2e615) SHA1(456ce0d1f74fa5e619c272880ba8ac6819848ddc))
	ROM_LOAD("f14_u27.l1", 0x8000, 0x8000, CRC(da1740f7) SHA1(1395a4f3891a043cfedc5426ec88af35eab8d4ea))

	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("f14_u21.l1", 0x8000, 0x8000, CRC(e412300c) SHA1(382d0cfa47abea295f0c7501bc0a010473e9d73b))
	ROM_LOAD("f14_u22.l1", 0x0000, 0x8000, CRC(c9dd7496) SHA1(de3cb855d87033274cc912578b02d1593d2d69f9))

	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("f14_u4.l1", 0x00000, 0x8000, CRC(43ecaabf) SHA1(64b50dbff03cd556130d0cff47b951fdf37d397d))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("f14_u19.l1", 0x20000, 0x8000, CRC(d0de4a7c) SHA1(46ecd5786653add47751cc56b38d9db7c4622377))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

/*--------------------
/ Fire! 8/87 (#556)
/--------------------*/
ROM_START(fire_l3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("fire_u26.l3", 0x4000, 0x4000, CRC(48abae33) SHA1(00ce24316aa007eec090ae74818003e11a141214))
	ROM_LOAD("fire_u27.l3", 0x8000, 0x8000, CRC(4ebf4888) SHA1(45dc0231404ed70be2ab5d599a673aac6271550e))

	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("fire_u21.l2", 0x8000, 0x8000, CRC(2edde0a4) SHA1(de292a340a3a06b0b996fc69fee73eb7bbfbbe64))
	ROM_LOAD("fire_u22.l2", 0x0000, 0x8000, CRC(16145c97) SHA1(523e99df3907a2c843c6e27df4d16799c4136a46))

	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("fire_u4.l1", 0x00000, 0x8000, CRC(0e058918) SHA1(4d6bf2290141119174787f8dd653c47ea4c73693))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
ROM_END

ROM_START(fire_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("fire_u26.l2", 0x4000, 0x4000, CRC(05434ea7) SHA1(462808954de18fed25e6df8f4cc66acdd05a3d85))
	ROM_LOAD("fire_u27.l2", 0x8000, 0x8000, CRC(517d0367) SHA1(b31e591cc24fe937124d8c1da69ab654c12bbb65))

	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("fire_u21.l2", 0x8000, 0x8000, CRC(2edde0a4) SHA1(de292a340a3a06b0b996fc69fee73eb7bbfbbe64))
	ROM_LOAD("fire_u22.l2", 0x0000, 0x8000, CRC(16145c97) SHA1(523e99df3907a2c843c6e27df4d16799c4136a46))

	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("fire_u4.l1", 0x00000, 0x8000, CRC(0e058918) SHA1(4d6bf2290141119174787f8dd653c47ea4c73693))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
ROM_END

/*--------------------------------------
/ Fire! Champagne Edition 9/87 (#556SE)
/---------------------------------------*/

/*-------------------------
/ Millionaire 1/87 (#555)
/--------------------------*/
ROM_START(milln_l3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mill_u26.l3", 0x4000, 0x4000, CRC(07bc9fff) SHA1(b16082fb51df3e4d2fb786cb8894b1c232521ef3))
	ROM_LOAD("mill_u27.l3", 0x8000, 0x8000, CRC(ba789c43) SHA1(c066a304882bea4cba1e215642416fcb22585aa4))

	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("mill_u21.l1", 0x8000, 0x8000, CRC(4cd1ee90) SHA1(4e24b96138ced16eff9036303ca6347e3423dbfc))
	ROM_LOAD("mill_u22.l1", 0x0000, 0x8000, CRC(73735cfc) SHA1(f74c873a20990263e0d6b35609fc51c08c9f8e31))

	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("mill_u4.l1", 0x00000, 0x8000, CRC(cf766506) SHA1(a6e4df19a513102abbce2653d4f72245f54407b1))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("mill_u19.l1", 0x20000, 0x8000, CRC(e073245a) SHA1(cbaddde6bb19292ace574a8329e18c97c2ee9763))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

/*--------------------
/ Pinbot 10/86 (#549)
/--------------------*/
ROM_START(pb_l5)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("pbot_u26.l5", 0x4000, 0x4000, CRC(daa0c8e4) SHA1(47289b350eb0d84aa0d37e53383e18625451bbe8))
	ROM_LOAD("pbot_u27.l5", 0x8000, 0x8000, CRC(e625d6ce) SHA1(1858dc2183954342b8e2e5eb9a14edcaa8dad5ae))

	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("pbot_u21.l1", 0x8000, 0x8000, CRC(3eab88d9) SHA1(667e3b675e2ae8fec6a6faddb9b0dd5531d64f8f))
	ROM_LOAD("pbot_u22.l1", 0x0000, 0x8000, CRC(a2d2c9cb) SHA1(46437dc54538f1626caf41a2818ddcf8000c44e4))

	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("pbot_u4.l1", 0x00000, 0x8000, CRC(de5926bd) SHA1(3d111e27c5f0c8c0afc5fe5cc45bf77c12b69228))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("pbot_u19.l1", 0x20000, 0x8000, CRC(40eb4e9f) SHA1(07b0557b35599a2dd5aa66a306fbbe8f50eed998))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

ROM_START(pb_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-l1.rom", 0x4000, 0x4000, CRC(e3b94ca4) SHA1(1db2acb025941cc165cc7ec70a160e07ab1eeb2e))
	ROM_LOAD("u27-l1.rom", 0x8000, 0x8000, CRC(fa0be640) SHA1(723dd96bbcc9b3043c91e0215050fb626dd6ced3))

	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("pbot_u21.l1", 0x8000, 0x8000, CRC(3eab88d9) SHA1(667e3b675e2ae8fec6a6faddb9b0dd5531d64f8f))
	ROM_LOAD("pbot_u22.l1", 0x0000, 0x8000, CRC(a2d2c9cb) SHA1(46437dc54538f1626caf41a2818ddcf8000c44e4))

	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("pbot_u4.l1", 0x00000, 0x8000, CRC(de5926bd) SHA1(3d111e27c5f0c8c0afc5fe5cc45bf77c12b69228))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("pbot_u19.l1", 0x20000, 0x8000, CRC(40eb4e9f) SHA1(07b0557b35599a2dd5aa66a306fbbe8f50eed998))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

ROM_START(pb_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-l1.rom", 0x4000, 0x4000, CRC(e3b94ca4) SHA1(1db2acb025941cc165cc7ec70a160e07ab1eeb2e))
	ROM_LOAD("u27-l2.rom", 0x8000, 0x8000, CRC(0a334fc5) SHA1(d08afe6ddc141e37f97ea588d184a316ff7f6db7))

	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("pbot_u21.l1", 0x8000, 0x8000, CRC(3eab88d9) SHA1(667e3b675e2ae8fec6a6faddb9b0dd5531d64f8f))
	ROM_LOAD("pbot_u22.l1", 0x0000, 0x8000, CRC(a2d2c9cb) SHA1(46437dc54538f1626caf41a2818ddcf8000c44e4))

	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("pbot_u4.l1", 0x00000, 0x8000, CRC(de5926bd) SHA1(3d111e27c5f0c8c0afc5fe5cc45bf77c12b69228))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("pbot_u19.l1", 0x20000, 0x8000, CRC(40eb4e9f) SHA1(07b0557b35599a2dd5aa66a306fbbe8f50eed998))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

ROM_START(pb_l3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-l1.rom", 0x4000, 0x4000, CRC(e3b94ca4) SHA1(1db2acb025941cc165cc7ec70a160e07ab1eeb2e))
	ROM_LOAD("u27-l3.rom", 0x8000, 0x8000, CRC(6f40ee84) SHA1(85453137e3fdb1e422e3903dd053e04c9f2b9607))

	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("pbot_u21.l1", 0x8000, 0x8000, CRC(3eab88d9) SHA1(667e3b675e2ae8fec6a6faddb9b0dd5531d64f8f))
	ROM_LOAD("pbot_u22.l1", 0x0000, 0x8000, CRC(a2d2c9cb) SHA1(46437dc54538f1626caf41a2818ddcf8000c44e4))

	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("pbot_u4.l1", 0x00000, 0x8000, CRC(de5926bd) SHA1(3d111e27c5f0c8c0afc5fe5cc45bf77c12b69228))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("pbot_u19.l1", 0x20000, 0x8000, CRC(40eb4e9f) SHA1(07b0557b35599a2dd5aa66a306fbbe8f50eed998))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

ROM_START(pb_p4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-l2.rom", 0x4000, 0x4000, CRC(e3b94ca4) SHA1(1db2acb025941cc165cc7ec70a160e07ab1eeb2e))
	ROM_LOAD("u27_p4.bin", 0x8000, 0x8000, CRC(fbe2c466) SHA1(ac6c8f953b00e0ec7626cd1ccf4e16851ab905d0))

	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("pbot_u21.l1", 0x8000, 0x8000, CRC(3eab88d9) SHA1(667e3b675e2ae8fec6a6faddb9b0dd5531d64f8f))
	ROM_LOAD("pbot_u22.l1", 0x0000, 0x8000, CRC(a2d2c9cb) SHA1(46437dc54538f1626caf41a2818ddcf8000c44e4))

	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("pbot_u4.l1", 0x00000, 0x8000, CRC(de5926bd) SHA1(3d111e27c5f0c8c0afc5fe5cc45bf77c12b69228))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("pbot_u19.l1", 0x20000, 0x8000, CRC(40eb4e9f) SHA1(07b0557b35599a2dd5aa66a306fbbe8f50eed998))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

GAME(1987, f14_l1,   0,       s11a, s11a, s11a_state, init_s11a, ROT0, "Williams", "F-14 Tomcat (L-1)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1987, f14_p3,   f14_l1,  s11a, s11a, s11a_state, init_s11a, ROT0, "Williams", "F-14 Tomcat (P-3)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1987, f14_p4,   f14_l1,  s11a, s11a, s11a_state, init_s11a, ROT0, "Williams", "F-14 Tomcat (P-4)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1987, f14_p5,   f14_l1,  s11a, s11a, s11a_state, init_s11a, ROT0, "Williams", "F-14 Tomcat (P-5)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1987, fire_l3,  0,       s11a, s11a, s11a_state, init_s11a, ROT0, "Williams", "Fire! (L-3)",       MACHINE_IS_SKELETON_MECHANICAL)
GAME(1987, fire_l2,  fire_l3, s11a, s11a, s11a_state, init_s11a, ROT0, "Williams", "Fire! (L-2)",       MACHINE_IS_SKELETON_MECHANICAL)
GAME(1987, milln_l3, 0,       s11a_obg, s11a, s11a_state, init_s11a, ROT0, "Williams", "Millionaire (L-3)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1986, pb_l5,    0,       s11a_obg, s11a, s11a_state, init_s11a, ROT0, "Williams", "Pin-Bot (L-5)",     MACHINE_IS_SKELETON_MECHANICAL)
GAME(1986, pb_l1,    pb_l5,   s11a_obg, s11a, s11a_state, init_s11a, ROT0, "Williams", "Pin-Bot (L-1)",     MACHINE_IS_SKELETON_MECHANICAL)
GAME(1986, pb_l2,    pb_l5,   s11a_obg, s11a, s11a_state, init_s11a, ROT0, "Williams", "Pin-Bot (L-2)",     MACHINE_IS_SKELETON_MECHANICAL)
GAME(1986, pb_l3,    pb_l5,   s11a_obg, s11a, s11a_state, init_s11a, ROT0, "Williams", "Pin-Bot (L-3)",     MACHINE_IS_SKELETON_MECHANICAL)
GAME(1986, pb_p4,    pb_l5,   s11a_obg, s11a, s11a_state, init_s11a, ROT0, "Williams", "Pin-Bot (P-4)",     MACHINE_IS_SKELETON_MECHANICAL)
