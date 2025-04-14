// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/*************************************************************************************************************

PINBALL
Williams System 11B

If it says FACTORY SETTING, hit F3.

Here are the key codes to enable play:

Game                          NUM  Start game             End ball
---------------------------------------------------------------------------------------------
**** Williams ****
Space Station                 552  Hold CDE hit 1         CDE
Taxi                          553  Hold CDE hit 1         CDE
Big Guns                      557  Hold CDE hit 1         CDE
Swords of Fury                559  Hold CDE hit 1         CDE (may have to try a few times)
Black Knight 2000             563  Hold CDE hit 1         CDE
Cyclone                       564  Hold B hit 1           B
Banzai Run                    566  Hold BCD hit 1         BCD
Jokerz!                       567  Hold CDE hit 1         CDE
Earthshaker                   568  Hold CDE hit 1         CDE
Police Force                  573  Hold CD hit 1          CD
Whirlwind                     574  Hold CDE hit 1         CDE
Bad Cats                      575  Hold B hit 1           B
**** Bally (Midway) ****
Transporter the Rescue       2008  Hold CDE hit 1         CDE
Mousin' Around!              2009  Hold CDE hit 1         CDE
Elvira & the Party Monsters  2011  Hold CDE hit 1         CDE

Status:
- All machines are playable

ToDo:
- Some digits flicker

Known issues:
- Black Knight 2000 sometimes goes nuts or resets, although this is largely dependent on
   whether the 'ball 1' animation was played or not.
   if you insert 2 or more credits and hit start 2 times quickly so it doesn't
   play the animation, the game seems more stable afterwards; is this an original
   game bug or 6802 core bug or something else? Activating a switch ('x' for instance) after
   the ball 1 animation makes it significantly more stable, so this could be an original code bug,
   or a code bug which is hit if the motor drawbridge limit sensors are not working/emulated, as in MAME.
   Proximate cause is smashing the stack, after which the RTS at 61DE (in bk2k_l4)
   transfers to 0000 (where no valid code exists).
   This may or may not be an original game bug if the game is started if the
   coin door is left open (hence the memory protect switch is left disengaged.
   We do not currently emulate the memory protect function to write protect
   the ram at 0x780-0x7ff, so it is possible that the protection being
   active prevents the real machine from crashing)
- Black Knight 2000 LG-1 set reports U26 ROM FAILURE. Bad/hacked dump or original bug?

***************************************************************************************************************/

#include "emu.h"
#include "s11.h"

#include "cpu/m6809/m6809.h"
#include "speaker.h"

#include "s11b.lh"


static INPUT_PORTS_START( s11b )
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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP09")
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Audio Diag") PORT_CODE(KEYCODE_9_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(s11b_state::audio_nmi), 1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Main Diag") PORT_CODE(KEYCODE_0_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(s11b_state::main_nmi), 1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Advance") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Up/Down") PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE
	PORT_CONFNAME( 0x10, 0x10, "Language" )
	PORT_CONFSETTING( 0x00, "German" )
	PORT_CONFSETTING( 0x10, "English" )
INPUT_PORTS_END

void s11b_state::machine_start()
{
	s11_state::machine_start();
	save_item(NAME(m_invert));
	save_item(NAME(m_is7seg34));
}

void s11b_state::machine_reset()
{
	s11_state::machine_reset();
	membank("bank0")->set_entry(0);
	membank("bank1")->set_entry(0);
	// reset the CPUs again, so that the CPUs are starting with the right vectors (otherwise sound may die on reset)
	m_audiocpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
	if(m_bg)
		m_bg->device_reset();
}

void s11b_state::s11b_dig1_w(u8 data)
{
	u8 lock = get_lock2() + 1;
	if (lock == 1)
	{
		u16 seg;
		if (m_is7seg34)
		{
			seg = data | (m_invert ? 0x7700 : 0);
			if (BIT(seg, 6))
				seg |= 0x800; // fix g seg
			if (BIT(seg, 7))
				seg |= 0x8000; // fix comma
		}
		else
		{
			seg = get_segment2() & 0xff00;
			seg |= data;
			set_segment2(seg);
		}
		u16 segd = m_invert ? ~seg : seg;
		m_digits[get_strobe()+16] = bitswap<16>(segd, 7, 15, 12, 10, 8, 14, 13, 9, 11, 6, 5, 4, 3, 2, 1, 0);
	}
	set_lock2(lock);
}

void s11b_state::s11b_pia2c_pa_w(u8 data)
{
	if (get_lock1() == 1)
	{
		u16 seg = get_segment1() & 0xff;
		seg |= (data<<8);
		u16 segd = m_invert ? ~seg : seg;
		m_digits[get_strobe()] = bitswap<16>(segd, 7, 15, 12, 10, 8, 14, 13, 9, 11, 6, 5, 4, 3, 2, 1, 0);
		set_segment1(seg);
	}
}

void s11b_state::s11b_pia2c_pb_w(u8 data)
{
	u8 lock = get_lock1() + 1;
	if (lock == 1)
	{
		u16 seg = get_segment1() & 0xff00;
		seg |= data;
		u16 segd = m_invert ? ~seg : seg;
		m_digits[get_strobe()] = bitswap<16>(segd, 7, 15, 12, 10, 8, 14, 13, 9, 11, 6, 5, 4, 3, 2, 1, 0);
		set_segment1(seg);
	}
	set_lock1(lock);
}

void s11b_state::s11b_pia34_pa_w(u8 data)
{
	if ((get_lock2() == 1) && (!m_is7seg34))
	{
		u16 seg = get_segment2() & 0xff;
		seg |= (data<<8);
		u16 segd = m_invert ? ~seg : seg;
		m_digits[get_strobe()+16] = bitswap<16>(segd, 7, 15, 12, 10, 8, 14, 13, 9, 11, 6, 5, 4, 3, 2, 1, 0);
		set_segment2(seg);
	}
}

void s11b_state::init_s11bnn()
{
	s11_state::init_s11();
	m_invert = false;
	m_is7seg34 = false;
}

void s11b_state::init_s11bin()
{
	s11_state::init_s11();
	m_invert = true;
	m_is7seg34 = false;
}

void s11b_state::init_s11bn7()
{
	s11_state::init_s11();
	m_invert = false;
	m_is7seg34 = true;
}

void s11b_state::init_s11bi7()
{
	s11_state::init_s11();
	m_invert = true;
	m_is7seg34 = true;
}

void s11b_state::s11b_base(machine_config &config)
{
	/* basic machine hardware */
	M6808(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &s11b_state::s11_main_map);
	INPUT_MERGER_ANY_HIGH(config, m_mainirq).output_handler().set(FUNC(s11b_state::main_irq));
	INPUT_MERGER_ANY_HIGH(config, m_piairq).output_handler().set(FUNC(s11b_state::pia_irq));

	/* Video */
	config.set_default_layout(layout_s11b);

	/* Sound */
	genpin_audio(config);

	/* Devices */
	PIA6821(config, m_pia21);
	m_pia21->readpa_handler().set(FUNC(s11b_state::sound_r));
	m_pia21->set_port_a_input_overrides_output_mask(0xff);
	m_pia21->writepa_handler().set(FUNC(s11b_state::sound_w));
	m_pia21->writepb_handler().set(FUNC(s11b_state::sol2_w));
	m_pia21->ca2_handler().set(FUNC(s11b_state::pia21_ca2_w));
	m_pia21->cb2_handler().set(FUNC(s11b_state::pia21_cb2_w));
	m_pia21->irqa_handler().set(m_piairq, FUNC(input_merger_device::in_w<1>));
	m_pia21->irqb_handler().set(m_piairq, FUNC(input_merger_device::in_w<2>));

	PIA6821(config, m_pia24);
	m_pia24->writepa_handler().set(FUNC(s11b_state::lamp0_w));
	m_pia24->writepb_handler().set(FUNC(s11b_state::lamp1_w));
	m_pia24->cb2_handler().set(FUNC(s11b_state::pia24_cb2_w));
	m_pia24->irqa_handler().set(m_piairq, FUNC(input_merger_device::in_w<3>));
	m_pia24->irqb_handler().set(m_piairq, FUNC(input_merger_device::in_w<4>));

	PIA6821(config, m_pia28);
	m_pia28->readpa_handler().set(FUNC(s11b_state::pia28_w7_r));
	m_pia28->set_port_a_input_overrides_output_mask(0xff);
	m_pia28->writepa_handler().set(FUNC(s11b_state::s11a_dig0_w));
	m_pia28->writepb_handler().set(FUNC(s11b_state::s11b_dig1_w));
	m_pia28->ca2_handler().set(FUNC(s11b_state::pia28_ca2_w));
	m_pia28->cb2_handler().set(FUNC(s11b_state::pia28_cb2_w));
	m_pia28->irqa_handler().set(m_piairq, FUNC(input_merger_device::in_w<5>));
	m_pia28->irqb_handler().set(m_piairq, FUNC(input_merger_device::in_w<6>));

	PIA6821(config, m_pia2c);
	m_pia2c->writepa_handler().set(FUNC(s11b_state::s11b_pia2c_pa_w));
	m_pia2c->writepb_handler().set(FUNC(s11b_state::s11b_pia2c_pb_w));
	m_pia2c->ca2_handler().set(FUNC(s11b_state::pia2c_ca2_w));
	m_pia2c->cb2_handler().set(FUNC(s11b_state::pia2c_cb2_w));
	m_pia2c->irqa_handler().set(m_piairq, FUNC(input_merger_device::in_w<7>));
	m_pia2c->irqb_handler().set(m_piairq, FUNC(input_merger_device::in_w<8>));

	PIA6821(config, m_pia30);
	m_pia30->readpa_handler().set(FUNC(s11b_state::switch_r));
	m_pia30->set_port_a_input_overrides_output_mask(0xff);
	m_pia30->writepb_handler().set(FUNC(s11b_state::switch_w));
	m_pia30->ca2_handler().set(FUNC(s11b_state::pia30_ca2_w));
	m_pia30->cb2_handler().set(FUNC(s11b_state::pia30_cb2_w));
	m_pia30->irqa_handler().set(m_piairq, FUNC(input_merger_device::in_w<9>));
	m_pia30->irqb_handler().set(m_piairq, FUNC(input_merger_device::in_w<10>));

	PIA6821(config, m_pia34);
	m_pia34->writepa_handler().set(FUNC(s11b_state::s11b_pia34_pa_w));
	m_pia34->writepb_handler().set(FUNC(s11b_state::pia34_pb_w));
	m_pia34->ca2_handler().set_nop();
	m_pia34->cb2_handler().set(FUNC(s11b_state::pia34_cb2_w));
	m_pia34->irqa_handler().set(m_piairq, FUNC(input_merger_device::in_w<11>));
	m_pia34->irqb_handler().set(m_piairq, FUNC(input_merger_device::in_w<12>));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	/* Add the soundcard */
	M6802(config, m_audiocpu, XTAL(4'000'000));
	m_audiocpu->set_ram_enable(false);
	m_audiocpu->set_addrmap(AS_PROGRAM, &s11b_state::s11_audio_map);
	INPUT_MERGER_ANY_HIGH(config, m_audioirq).output_handler().set_inputline(m_audiocpu, M6802_IRQ_LINE);

	MC1408(config, m_dac, 0);

	// this CVSD filter differs from the one on system 11 and 11a, possibly simplified so it uses more of the same components, or so it has a different
	// shape/cutoff than the filter on the bg music/speech board, on purpose.
	// The CVSD filter has a large gain, about 4.6x
	// The filter is boosting the ~5vpp audio signal from the CVSD chip to a ~23vpp (really ~17vpp) theoretical audio signal that the s11
	// mainboard outputs on its volume control-repurposed-as-audio-out connector.
	// In reality, the S11 mainboard outputs audio at a virtual ground level between +5v and -12v (so, 17VPP balanced around -7VDC), but since
	// the CVSD chip's internal DAC can only output between a bit over +0x180/-0x180 out of 0x200, the most voltage it can ever output is
	// between (assuming 0x1ff is 5VDC and 0x300 is 0VDC) a max of 4.375VDC and a min of 0.625VDC, i.e. 3.75VPP centered on 2.5VDC.
	// In reality, the range is likely less than that.
	// This means multiplying a 3.75VPP signal by 4.6 is 17.25VPP, which is almost exactly the expected 17V (12v+5v) VPP the output should have.
	FILTER_BIQUAD(config, m_cvsd_filter2).opamp_mfb_lowpass_setup(RES_K(12), RES_K(12), RES_K(56), CAP_P(4700), CAP_P(470));
	FILTER_BIQUAD(config, m_cvsd_filter).opamp_mfb_lowpass_setup(RES_K(180), RES_K(180), RES_K(180), CAP_P(470), CAP_P(100));
	m_cvsd_filter->add_route(ALL_OUTPUTS, m_cvsd_filter2, 1.0);
	HC55516(config, m_hc55516, 0).add_route(ALL_OUTPUTS, m_cvsd_filter, 1.0/4.0); // to prevent massive clipping issues, we divide the signal by 4 here before going into the filters, then multiply it by 4 after it comes out the other end

	PIA6821(config, m_pias);
	m_pias->readpa_handler().set(FUNC(s11b_state::sound_r));
	m_pias->set_port_a_input_overrides_output_mask(0xff);
	m_pias->writepa_handler().set(FUNC(s11b_state::sound_w));
	m_pias->writepb_handler().set(m_dac, FUNC(dac_byte_interface::data_w));
	m_pias->ca2_handler().set(m_hc55516, FUNC(hc55516_device::clock_w));
	m_pias->cb2_handler().set(m_hc55516, FUNC(hc55516_device::digit_w));
	m_pias->irqa_handler().set(m_audioirq, FUNC(input_merger_device::in_w<0>));
	m_pias->irqb_handler().set(m_audioirq, FUNC(input_merger_device::in_w<1>));
}

void s11b_state::s11b(machine_config &config)
{
	s11b_base(config);
	/* Add the background music card */
	S11_BG(config, m_bg);
	m_dac->add_route(ALL_OUTPUTS, m_bg, 0.4484/2.0);
	m_cvsd_filter2->add_route(ALL_OUTPUTS, m_bg, (0.4484*4.0)/2.0);
	m_pia34->ca2_handler().set(m_bg, FUNC(s11_bg_device::resetq_w));
	m_bg->pb_cb().set(m_pia34, FUNC(pia6821_device::portb_w));
	m_bg->cb2_cb().set(m_pia34, FUNC(pia6821_device::cb1_w));
	SPEAKER(config, "speaker").front_center();
	m_bg->add_route(ALL_OUTPUTS, "speaker", 1.0);
}

void s11b_state::s11b_jokerz(machine_config &config)
{
	s11b_base(config);
	/* Add the pin sound 88 music card */
	PINSND88(config, m_ps88);
	// the dac and cvsd volumes should be equally mixed on the s11 board send to the audio board, whatever type it is
	// the 4 gain values in the add_route statements are actually irrelevant, the ps88 device will override them
	m_dac->add_route(ALL_OUTPUTS, m_ps88, 0.29, 0);
	m_dac->add_route(ALL_OUTPUTS, m_ps88, 0.25, 1);
	m_cvsd_filter2->add_route(ALL_OUTPUTS, m_ps88, (0.29*4.0), 0);
	m_cvsd_filter2->add_route(ALL_OUTPUTS, m_ps88, (0.25*4.0), 1);
	m_pia34->ca2_handler().set(m_ps88, FUNC(pinsnd88_device::resetq_w));
	m_ps88->syncq_cb().set(m_pia34, FUNC(pia6821_device::ca1_w)); // the sync connection comes from sound connector pin 16 to MCA1, not the usual pin 12 to MCB1
	SPEAKER(config, "cabinet").front_floor(); // the cabinet speaker is aimed down underneath the pinball table itself
	SPEAKER(config, "backbox").front_center(); // the backbox speakers are roughly level with the user, but farther in front of them than the cabinet
	m_ps88->add_route(0, "cabinet", 1.0);
	m_ps88->add_route(1, "backbox", 1.0);
}

/*-----------------------
/ Bad Cats 12/89 (#575)
/-----------------------*/
ROM_START(bcats_l5)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cats_u26.l5", 0x4000, 0x4000, CRC(32246d12) SHA1(b8aa89d197a6b992501904f5072a10ab1a31db87))
	ROM_LOAD("cats_u27.l5", 0x8000, 0x8000, CRC(ef842bbf) SHA1(854860db428795d5de5c075aa78496f0c18a380f))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("cats_u21.l1", 0x8000, 0x8000, CRC(04110d08) SHA1(4b44b26983cb5d14a93c16a19dc2bdbaa665dc69))
	ROM_LOAD("cats_u22.l1", 0x0000, 0x8000, CRC(7e152c78) SHA1(b4ab770fdd9420a5d35e55bf8fb84c99ac544b8b))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("cats_u4.l1", 0x00000, 0x8000, CRC(18c62813) SHA1(a4fb69cfedd0b92c22b599913df3cdf8b3eef42c))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("cats_u19.l1", 0x20000, 0x8000, CRC(f2fea68b) SHA1(9a41823e71342b7a162420378f122bba34ce0636))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
	ROM_LOAD("cats_u20.l1", 0x40000, 0x8000, CRC(bf4dc35a) SHA1(9920ce90d93fb6ecf98792c35bb6eb8862a969f3))
	ROM_RELOAD(0x48000,0x8000)
	ROM_RELOAD(0x50000,0x8000)
	ROM_RELOAD(0x58000,0x8000)
ROM_END

ROM_START(bcats_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("bcgu26.la2", 0x4000, 0x4000, CRC(206c7cf8) SHA1(34eb128d46a0e1ba943f4e37aa95fa6d81aefb0e))
	ROM_LOAD("bcgu27.la2", 0x8000, 0x8000, CRC(911981c6) SHA1(0d5b5c6d8399c6337300c789a0466242f91eaf94))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("cats_u21.l1", 0x8000, 0x8000, CRC(04110d08) SHA1(4b44b26983cb5d14a93c16a19dc2bdbaa665dc69))
	ROM_LOAD("cats_u22.l1", 0x0000, 0x8000, CRC(7e152c78) SHA1(b4ab770fdd9420a5d35e55bf8fb84c99ac544b8b))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("cats_u4.l1", 0x00000, 0x8000, CRC(18c62813) SHA1(a4fb69cfedd0b92c22b599913df3cdf8b3eef42c))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("cats_u19.l1", 0x20000, 0x8000, CRC(f2fea68b) SHA1(9a41823e71342b7a162420378f122bba34ce0636))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
	ROM_LOAD("cats_u20.l1", 0x40000, 0x8000, CRC(bf4dc35a) SHA1(9920ce90d93fb6ecf98792c35bb6eb8862a969f3))
	ROM_RELOAD(0x48000,0x8000)
	ROM_RELOAD(0x50000,0x8000)
	ROM_RELOAD(0x58000,0x8000)
ROM_END

ROM_START(bcats_g4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cats_u26.l5",  0x4000, 0x4000, CRC(32246d12) SHA1(b8aa89d197a6b992501904f5072a10ab1a31db87))
	ROM_LOAD("cats_u27.lg4", 0x8000, 0x8000, CRC(6af8cc3b) SHA1(ac9908dc3fbe1d3b1821c2976aaa5bbffbf24cda))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("cats_u21.l1", 0x8000, 0x8000, CRC(04110d08) SHA1(4b44b26983cb5d14a93c16a19dc2bdbaa665dc69))
	ROM_LOAD("cats_u22.l1", 0x0000, 0x8000, CRC(7e152c78) SHA1(b4ab770fdd9420a5d35e55bf8fb84c99ac544b8b))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("cats_u4.l1", 0x00000, 0x8000, CRC(18c62813) SHA1(a4fb69cfedd0b92c22b599913df3cdf8b3eef42c))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("cats_u19.l1", 0x20000, 0x8000, CRC(f2fea68b) SHA1(9a41823e71342b7a162420378f122bba34ce0636))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
	ROM_LOAD("cats_u20.l1", 0x40000, 0x8000, CRC(bf4dc35a) SHA1(9920ce90d93fb6ecf98792c35bb6eb8862a969f3))
	ROM_RELOAD(0x48000,0x8000)
	ROM_RELOAD(0x50000,0x8000)
	ROM_RELOAD(0x58000,0x8000)
ROM_END

/*----------------------
/ Banzai Run 7/88 (#566)
/----------------------*/
ROM_START(bnzai_l3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("banz_u26.l3", 0x4000, 0x4000, CRC(ca578aa3) SHA1(32c03178cc9d9514f76e084e56f6cf6f82754331))
	ROM_LOAD("banz_u27.l3", 0x8000, 0x8000, CRC(af66fac4) SHA1(84929aaad8a8e4a312a230b73f206d3b43a04dc3))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("banz_u21.l1", 0x8000, 0x8000, CRC(cd06716e) SHA1(b61a0dc017dd4a09296a43a855461c5cee07517b))
	ROM_LOAD("banz_u22.l1", 0x0000, 0x8000, CRC(e8159033) SHA1(e8f15801feefeb30768e88d685c208108aa134e8))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("banz_u4.l1", 0x00000, 0x8000, CRC(8fd69c69) SHA1(c024cda85c6616943c3a12ab5943a7be8709bfe3))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("banz_u19.l1", 0x20000, 0x8000, CRC(9104248c) SHA1(48a8c41f3a4127f4fb4de37e876c8380e3511e1f))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
	ROM_LOAD("banz_u20.l1", 0x40000, 0x8000, CRC(26b3d15c) SHA1(528084b6c62394f8ed9fc0f90b91d844060fc904))
	ROM_RELOAD(0x48000,0x8000)
	ROM_RELOAD(0x50000,0x8000)
	ROM_RELOAD(0x58000,0x8000)
ROM_END

ROM_START(bnzai_g3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("banz_u26.l3g", 0x4000, 0x4000, CRC(744b8758) SHA1(0bcd5dfd872656d0261a819e3dbd222754585ec0))
	ROM_LOAD("banz_u27.l3", 0x8000, 0x8000, CRC(af66fac4) SHA1(84929aaad8a8e4a312a230b73f206d3b43a04dc3))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("banz_u21.l1", 0x8000, 0x8000, CRC(cd06716e) SHA1(b61a0dc017dd4a09296a43a855461c5cee07517b))
	ROM_LOAD("banz_u22.l1", 0x0000, 0x8000, CRC(e8159033) SHA1(e8f15801feefeb30768e88d685c208108aa134e8))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("banz_u4.l1", 0x00000, 0x8000, CRC(8fd69c69) SHA1(c024cda85c6616943c3a12ab5943a7be8709bfe3))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("banz_u19.l1", 0x20000, 0x8000, CRC(9104248c) SHA1(48a8c41f3a4127f4fb4de37e876c8380e3511e1f))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
	ROM_LOAD("banz_u20.l1", 0x40000, 0x8000, CRC(26b3d15c) SHA1(528084b6c62394f8ed9fc0f90b91d844060fc904))
	ROM_RELOAD(0x48000,0x8000)
	ROM_RELOAD(0x50000,0x8000)
	ROM_RELOAD(0x58000,0x8000)
ROM_END

ROM_START(bnzai_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-l1.rom", 0x4000, 0x4000, CRC(556abdc0) SHA1(6de78345e5839a4ae9ff97273b6edb2635e0e8b4))
	ROM_LOAD("u27-l1.rom", 0x8000, 0x8000, CRC(7fc6de2e) SHA1(a7b42c2cd8c1e3810a319c755e52273454d5ca41))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("banz_u21.l1", 0x8000, 0x8000, CRC(cd06716e) SHA1(b61a0dc017dd4a09296a43a855461c5cee07517b))
	ROM_LOAD("banz_u22.l1", 0x0000, 0x8000, CRC(e8159033) SHA1(e8f15801feefeb30768e88d685c208108aa134e8))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("banz_u4.l1", 0x00000, 0x8000, CRC(8fd69c69) SHA1(c024cda85c6616943c3a12ab5943a7be8709bfe3))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("banz_u19.l1", 0x20000, 0x8000, CRC(9104248c) SHA1(48a8c41f3a4127f4fb4de37e876c8380e3511e1f))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
	ROM_LOAD("banz_u20.l1", 0x40000, 0x8000, CRC(26b3d15c) SHA1(528084b6c62394f8ed9fc0f90b91d844060fc904))
	ROM_RELOAD(0x48000,0x8000)
	ROM_RELOAD(0x50000,0x8000)
	ROM_RELOAD(0x58000,0x8000)
ROM_END

ROM_START(bnzai_pa)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-pa.rom", 0x4000, 0x4000, CRC(65a73e31) SHA1(0332b51ecfc548f72eaca402d83a5ad6dd223272))
	ROM_LOAD("u27-pa.rom", 0x8000, 0x8000, CRC(c64e2898) SHA1(b2291e9e65f8392f2f05f116dc47fcaf37500e60))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("banz_u21.l1", 0x8000, 0x8000, CRC(cd06716e) SHA1(b61a0dc017dd4a09296a43a855461c5cee07517b))
	ROM_LOAD("banz_u22.l1", 0x0000, 0x8000, CRC(e8159033) SHA1(e8f15801feefeb30768e88d685c208108aa134e8))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("u4-p7.rom", 0x00000, 0x8000, CRC(630d1ce9) SHA1(fb7f6004b94bf20281216519f18b53949eef4405))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("banz_u19.l1", 0x20000, 0x8000, CRC(9104248c) SHA1(48a8c41f3a4127f4fb4de37e876c8380e3511e1f))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
	ROM_LOAD("banz_u20.l1", 0x40000, 0x8000, CRC(26b3d15c) SHA1(528084b6c62394f8ed9fc0f90b91d844060fc904))
	ROM_RELOAD(0x48000,0x8000)
	ROM_RELOAD(0x50000,0x8000)
	ROM_RELOAD(0x58000,0x8000)
ROM_END

/*----------------------
/ Big Guns 10/87 (#557)
/----------------------*/
ROM_START(bguns_l8)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("guns_u26.l8", 0x4000, 0x4000, CRC(792dc1e8) SHA1(34586585bbaf579cb522569238e24d9ab891b471))
	ROM_LOAD("guns_u27.l8", 0x8000, 0x8000, CRC(ac4a1a51) SHA1(d48b5e5b550107df8c6edc2d5f78777d7d408959))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("guns_u21.l1", 0x8000, 0x8000, CRC(35c6bfe4) SHA1(83dbd10311add75f56046de58d315f8a87389703))
	ROM_LOAD("guns_u22.l1", 0x0000, 0x8000, CRC(091a5cb8) SHA1(db77314241eb6ed7f4385f99312a49b7caad1283))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("gund_u4.l1", 0x00000, 0x8000, CRC(d4a430a3) SHA1(5b44e3f313cc7cb75f51c239013d46e5eb986f9d))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("guns_u19.l1", 0x20000, 0x8000, CRC(ec1a6c23) SHA1(45bb4f78b89de9e690b5f9741d17f97766e702d6))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

ROM_START(bguns_l7)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("guns_u26.l8", 0x4000, 0x4000, CRC(792dc1e8) SHA1(34586585bbaf579cb522569238e24d9ab891b471))
	ROM_LOAD("guns_u27.l7", 0x8000, 0x8000, CRC(8ff26d24) SHA1(eab732b401144ad7efc80d336299beae85ca7d24))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("guns_u21.l1", 0x8000, 0x8000, CRC(35c6bfe4) SHA1(83dbd10311add75f56046de58d315f8a87389703))
	ROM_LOAD("guns_u22.l1", 0x0000, 0x8000, CRC(091a5cb8) SHA1(db77314241eb6ed7f4385f99312a49b7caad1283))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("gund_u4.l1", 0x00000, 0x8000, CRC(d4a430a3) SHA1(5b44e3f313cc7cb75f51c239013d46e5eb986f9d))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("guns_u19.l1", 0x20000, 0x8000, CRC(ec1a6c23) SHA1(45bb4f78b89de9e690b5f9741d17f97766e702d6))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

ROM_START(bguns_la)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-l-a.rom", 0x4000, 0x4000, CRC(613b4d5c) SHA1(7eed4ddb661cd03839a9a89ca695de9cbd1c4d45))
	ROM_LOAD("u27-l-a.rom", 0x8000, 0x8000, CRC(eee9e1cc) SHA1(32fbade5cbc9047a61d4ce0ec1e616d5324d507f))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("guns_u21.l1", 0x8000, 0x8000, CRC(35c6bfe4) SHA1(83dbd10311add75f56046de58d315f8a87389703))
	ROM_LOAD("guns_u22.l1", 0x0000, 0x8000, CRC(091a5cb8) SHA1(db77314241eb6ed7f4385f99312a49b7caad1283))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("gund_u4.l1", 0x00000, 0x8000, CRC(d4a430a3) SHA1(5b44e3f313cc7cb75f51c239013d46e5eb986f9d))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("guns_u19.l1", 0x20000, 0x8000, CRC(ec1a6c23) SHA1(45bb4f78b89de9e690b5f9741d17f97766e702d6))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

ROM_START(bguns_p1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-p-1.rom", 0x4000, 0x4000, CRC(26b8d58f) SHA1(678d4f706b862f3168d6d15859dba6288912e462))
	ROM_LOAD("u27-p-1.rom", 0x8000, 0x8000, CRC(2fba9a0d) SHA1(16629a5f009865825207378118a147e3135c51cf))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("guns_u21.l1", 0x8000, 0x8000, CRC(35c6bfe4) SHA1(83dbd10311add75f56046de58d315f8a87389703))
	ROM_LOAD("guns_u22.l1", 0x0000, 0x8000, CRC(091a5cb8) SHA1(db77314241eb6ed7f4385f99312a49b7caad1283))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("gund_u4.l1", 0x00000, 0x8000, CRC(d4a430a3) SHA1(5b44e3f313cc7cb75f51c239013d46e5eb986f9d))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("guns_u19.l1", 0x20000, 0x8000, CRC(ec1a6c23) SHA1(45bb4f78b89de9e690b5f9741d17f97766e702d6))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

/*------------------------------
/ Black Knight 2000 6/89 (#563)
/------------------------------*/
ROM_START(bk2k_l4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("bk2k_u26.l4", 0x4000, 0x4000, CRC(16c7b9e7) SHA1(b6d5edb5ac2b58da699702ece00534d18c1a9fd7))
	ROM_LOAD("bk2k_u27.l4", 0x8000, 0x8000, CRC(5cf3ab40) SHA1(ee8cb554d10478b028da4a761476d6ec8c56a042))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("bk2k_u21.l1", 0x8000, 0x8000, CRC(08be36ad) SHA1(0f4c448e003df54ed8ccf0e0c57f6123ce1e2027))
	ROM_LOAD("bk2k_u22.l1", 0x0000, 0x8000, CRC(9c8becd8) SHA1(9090e8104dad63f14246caabafec428d94d5e18d))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("bk2k_u4.l2", 0x00000, 0x8000, CRC(1d87281b) SHA1(609288b017aac6ce6da8717a35fdf87013adeb3c))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("bk2k_u19.l1", 0x20000, 0x8000, CRC(58e162b2) SHA1(891f810ae18b46593f570d719f0290a1d08a1a10))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

ROM_START(bk2k_lg1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-pu1.rom", 0x4000, 0x4000, CRC(2da07403) SHA1(4b48c5d7b0a03aa4593dc6053dc5e94df22d2a64))
	ROM_LOAD("bk2kgu27.lg1", 0x8000, 0x8000, CRC(2d6359d4) SHA1(531841dedf2acf3ac10577813f003cf077d4607d))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("bk2k_u21.l1", 0x8000, 0x8000, CRC(08be36ad) SHA1(0f4c448e003df54ed8ccf0e0c57f6123ce1e2027))
	ROM_LOAD("bk2k_u22.l1", 0x0000, 0x8000, CRC(9c8becd8) SHA1(9090e8104dad63f14246caabafec428d94d5e18d))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("bk2k_u4.l2", 0x00000, 0x8000, CRC(1d87281b) SHA1(609288b017aac6ce6da8717a35fdf87013adeb3c))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("bk2k_u19.l1", 0x20000, 0x8000, CRC(58e162b2) SHA1(891f810ae18b46593f570d719f0290a1d08a1a10))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

ROM_START(bk2k_lg3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-lg3.rom", 0x4000, 0x4000, CRC(6f468c85) SHA1(b919b436559a29c43911bd2839c5ae7c03e9b06f))
	ROM_LOAD("u27-lg3.rom", 0x8000, 0x8000, CRC(27707522) SHA1(37844e2f3c70430ee169e1c369aa8e9d47b2c8f2))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("bk2k_u21.l1", 0x8000, 0x8000, CRC(08be36ad) SHA1(0f4c448e003df54ed8ccf0e0c57f6123ce1e2027))
	ROM_LOAD("bk2k_u22.l1", 0x0000, 0x8000, CRC(9c8becd8) SHA1(9090e8104dad63f14246caabafec428d94d5e18d))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("bk2k_u4.l2", 0x00000, 0x8000, CRC(1d87281b) SHA1(609288b017aac6ce6da8717a35fdf87013adeb3c))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("bk2k_u19.l1", 0x20000, 0x8000, CRC(58e162b2) SHA1(891f810ae18b46593f570d719f0290a1d08a1a10))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

ROM_START(bk2k_pu1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-pu1.rom", 0x4000, 0x4000, CRC(2da07403) SHA1(4b48c5d7b0a03aa4593dc6053dc5e94df22d2a64))
	ROM_LOAD("u27-pu1.rom", 0x8000, 0x8000, CRC(245efbae) SHA1(e6354a6f5029f21aab2343cd90daf6cbfb51e556))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("bk2k_u21.l1", 0x8000, 0x8000, CRC(08be36ad) SHA1(0f4c448e003df54ed8ccf0e0c57f6123ce1e2027))
	ROM_LOAD("bk2k_u22.l1", 0x0000, 0x8000, CRC(9c8becd8) SHA1(9090e8104dad63f14246caabafec428d94d5e18d))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("bk2k_u4.l2", 0x00000, 0x8000, CRC(1d87281b) SHA1(609288b017aac6ce6da8717a35fdf87013adeb3c))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("bk2k_u19.l1", 0x20000, 0x8000, CRC(58e162b2) SHA1(891f810ae18b46593f570d719f0290a1d08a1a10))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

ROM_START(bk2k_pf1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("bk2k_u26.pf7", 0x4000, 0x4000, CRC(79a77d26) SHA1(dfecd3f1fa80f0e7a84cafe7f45a96dd1c847090))
	ROM_LOAD("bk2k_u27.pf1", 0x8000, 0x8000, CRC(2a84db98) SHA1(58438763ec702c1a1e73ab853c58352fe97c27e7))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("bk2k_u21.l1", 0x8000, 0x8000, CRC(08be36ad) SHA1(0f4c448e003df54ed8ccf0e0c57f6123ce1e2027))
	ROM_LOAD("bk2k_u22.l1", 0x0000, 0x8000, CRC(9c8becd8) SHA1(9090e8104dad63f14246caabafec428d94d5e18d))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("bk2k_u4.l2", 0x00000, 0x8000, CRC(1d87281b) SHA1(609288b017aac6ce6da8717a35fdf87013adeb3c))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("bk2k_u19.l1", 0x20000, 0x8000, CRC(58e162b2) SHA1(891f810ae18b46593f570d719f0290a1d08a1a10))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

ROM_START(bk2k_la2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-pu1.rom",  0x4000, 0x4000, CRC(2da07403) SHA1(4b48c5d7b0a03aa4593dc6053dc5e94df22d2a64))
	ROM_LOAD("bk2k_u27.la2", 0x8000, 0x8000, CRC(531e7752) SHA1(e51a93d40bc316876488ae0a7691ff5fd514472d))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("bk2k_u21.l1", 0x8000, 0x8000, CRC(08be36ad) SHA1(0f4c448e003df54ed8ccf0e0c57f6123ce1e2027))
	ROM_LOAD("bk2k_u22.l1", 0x0000, 0x8000, CRC(9c8becd8) SHA1(9090e8104dad63f14246caabafec428d94d5e18d))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("bk2k_u4.l2", 0x00000, 0x8000, CRC(1d87281b) SHA1(609288b017aac6ce6da8717a35fdf87013adeb3c))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("bk2k_u19.l1", 0x20000, 0x8000, CRC(58e162b2) SHA1(891f810ae18b46593f570d719f0290a1d08a1a10))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

ROM_START(bk2k_pa7)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("bk2k_u26.pa7", 0x4000, 0x4000, CRC(a0426491) SHA1(de31dd6a1c3d99fb39f8f6bd21daa5050e819614))
	ROM_LOAD("bk2k_u27.pa7", 0x8000, 0x8000, CRC(63e58e77) SHA1(961534fb09ca6db0e901b01d8ae45c82f418cd82))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("bk2k_u21.l1", 0x8000, 0x8000, CRC(08be36ad) SHA1(0f4c448e003df54ed8ccf0e0c57f6123ce1e2027))
	ROM_LOAD("bk2k_u22.l1", 0x0000, 0x8000, CRC(9c8becd8) SHA1(9090e8104dad63f14246caabafec428d94d5e18d))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("bk2k_u4.l2", 0x00000, 0x8000, CRC(1d87281b) SHA1(609288b017aac6ce6da8717a35fdf87013adeb3c))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("bk2k_u19.l1", 0x20000, 0x8000, CRC(58e162b2) SHA1(891f810ae18b46593f570d719f0290a1d08a1a10))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

ROM_START(bk2k_pa5)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("bk2k_u26.pa5", 0x4000, 0x4000, CRC(0012fe41) SHA1(06031684839268fdb73491626ec5bff6daacbf4c))
	ROM_LOAD("bk2k_u27.pa5", 0x8000, 0x8000, CRC(2e40e173) SHA1(7d961bb0bd03e836ebfe705b8895dd2e9704e6e1))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("bk2k_u21.l1", 0x8000, 0x8000, CRC(08be36ad) SHA1(0f4c448e003df54ed8ccf0e0c57f6123ce1e2027))
	ROM_LOAD("bk2k_u22.l1", 0x0000, 0x8000, CRC(9c8becd8) SHA1(9090e8104dad63f14246caabafec428d94d5e18d))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("bk2k_u4.l2", 0x00000, 0x8000, CRC(1d87281b) SHA1(609288b017aac6ce6da8717a35fdf87013adeb3c))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("bk2k_u19.l1", 0x20000, 0x8000, CRC(58e162b2) SHA1(891f810ae18b46593f570d719f0290a1d08a1a10))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

/*--------------------
/ Cyclone 2/88 (#564)
/--------------------*/
ROM_START(cycln_l5)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cycl_u26.l5", 0x4000, 0x4000, CRC(9ab15e12) SHA1(406f3212269dc42de1f3fabcf179958adbd4b5e8))
	ROM_LOAD("cycl_u27.l5", 0x8000, 0x8000, CRC(c4b6aac0) SHA1(9058e450dbf9d198b1746c258b0e437d7ee844e9))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("cycl_u21.l1", 0x8000, 0x8000, CRC(d4f69a7c) SHA1(da0ce27d92b22583be54a41fc8083cee803c987a))
	ROM_LOAD("cycl_u22.l1", 0x0000, 0x8000, CRC(28dc8f13) SHA1(bccce3a9b6b2f52da919c6df8db07e5e3de12657))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("cycl_u4.l5", 0x00000, 0x8000, CRC(d04b663b) SHA1(f54c6df08ec73b733cfeb2a989e44e5c04da3d9e))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("cycl_u19.l1", 0x20000, 0x8000, CRC(a20f6519) SHA1(63ded5f76133340fa31d4fe65420f4465866fb85))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

ROM_START(cycln_l4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cycl_u26.l4", 0x4000, 0x4000, CRC(7da30995) SHA1(3774004df22ddce508fe0604c0349df3edd513b4))
	ROM_LOAD("cycl_u27.l4", 0x8000, 0x8000, CRC(8874d65f) SHA1(600e2e8cd21faf8999ebef06fb08c43a1eb2ffd7))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("cycl_u21.l1", 0x8000, 0x8000, CRC(d4f69a7c) SHA1(da0ce27d92b22583be54a41fc8083cee803c987a))
	ROM_LOAD("cycl_u22.l1", 0x0000, 0x8000, CRC(28dc8f13) SHA1(bccce3a9b6b2f52da919c6df8db07e5e3de12657))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("cycl_u4.l5", 0x00000, 0x8000, CRC(d04b663b) SHA1(f54c6df08ec73b733cfeb2a989e44e5c04da3d9e))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("cycl_u19.l1", 0x20000, 0x8000, CRC(a20f6519) SHA1(63ded5f76133340fa31d4fe65420f4465866fb85))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

ROM_START(cycln_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cycl_u26.l1", 0x4000, 0x4000, CRC(b6b8354e) SHA1(ae07fb74ce340cf5c520ce7faea636c234c6e4db))
	ROM_LOAD("cycl_u27.l1", 0x8000, 0x8000, CRC(81340fcd) SHA1(9ec6754b25f80919f8fe0a2ff9136b37ca8c1aa0))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("cycl_u21.l1", 0x8000, 0x8000, CRC(d4f69a7c) SHA1(da0ce27d92b22583be54a41fc8083cee803c987a))
	ROM_LOAD("cycl_u22.l1", 0x0000, 0x8000, CRC(28dc8f13) SHA1(bccce3a9b6b2f52da919c6df8db07e5e3de12657))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("cycl_u4.l5", 0x00000, 0x8000, CRC(d04b663b) SHA1(f54c6df08ec73b733cfeb2a989e44e5c04da3d9e))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("cycl_u19.l1", 0x20000, 0x8000, CRC(a20f6519) SHA1(63ded5f76133340fa31d4fe65420f4465866fb85))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

/*------------------------
/ Earthshaker 4/89 (#568)
/------------------------*/
ROM_START(esha_pr4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("eshk_u26.f1", 0x4000, 0x4000, CRC(15e2bfe3) SHA1(57ce7f017a6f9ab88f221870efde91e97efbc8a6))
	ROM_LOAD("eshk_u27.f1", 0x8000, 0x8000, CRC(ddfa8edd) SHA1(e59ba6c1e8a0087abda218a8922d83ebefd84666))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("eshk_u21.l1", 0x8000, 0x8000, CRC(feac68e5) SHA1(2f12a78398bc3a468e3e0656da91260d45b0663b))
	ROM_LOAD("eshk_u22.l1", 0x0000, 0x8000, CRC(44f50fe1) SHA1(a8e24dbb0f5cf300118e1ebdcd2bb6b274d87936))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("eshk_u4.l1", 0x00000, 0x8000, CRC(40069f8c) SHA1(aafdc189259fa9c8dc49e60e978b84775e16c64e))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("eshk_u19.l1", 0x20000, 0x8000, CRC(e5593075) SHA1(549b03402e5639b449e35325eb52e78f8810b07a))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

ROM_START(esha_la3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("eshk_u26.l3", 0x4000, 0x4000, CRC(5350d132) SHA1(fbc671c89f85375c34c49610943c87336123fdc8))
	ROM_LOAD("eshk_u27.l3", 0x8000, 0x8000, CRC(91389290) SHA1(3f80b77aa0b7db2409bc6b197feb7a4d289b6ec8))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("eshk_u21.l1", 0x8000, 0x8000, CRC(feac68e5) SHA1(2f12a78398bc3a468e3e0656da91260d45b0663b))
	ROM_LOAD("eshk_u22.l1", 0x0000, 0x8000, CRC(44f50fe1) SHA1(a8e24dbb0f5cf300118e1ebdcd2bb6b274d87936))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("eshk_u4.l1", 0x00000, 0x8000, CRC(40069f8c) SHA1(aafdc189259fa9c8dc49e60e978b84775e16c64e))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("eshk_u19.l1", 0x20000, 0x8000, CRC(e5593075) SHA1(549b03402e5639b449e35325eb52e78f8810b07a))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

ROM_START(esha_ma3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("eshk_u26.ml3", 0x4000, 0x4000, CRC(7b7b1a8a) SHA1(8c1d5e4e0b4217055ad9e1490ff3dba52ef013f4))
	ROM_LOAD("eshk_u27.ml3", 0x8000, 0x8000, CRC(6197c56c) SHA1(338438e8e4c9a7790977dc8e394f6f032516f755))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("eshk_u21.l1", 0x8000, 0x8000, CRC(feac68e5) SHA1(2f12a78398bc3a468e3e0656da91260d45b0663b))
	ROM_LOAD("eshk_u22.l1", 0x0000, 0x8000, CRC(44f50fe1) SHA1(a8e24dbb0f5cf300118e1ebdcd2bb6b274d87936))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("eshk_u4.l1", 0x00000, 0x8000, CRC(40069f8c) SHA1(aafdc189259fa9c8dc49e60e978b84775e16c64e))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("eshk_u19.l1", 0x20000, 0x8000, CRC(e5593075) SHA1(549b03402e5639b449e35325eb52e78f8810b07a))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

ROM_START(esha_lg1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-lg1.rom", 0x4000, 0x4000, CRC(6b1c4d12) SHA1(8e90878ab3b6319e4b81967b4cb8c47e1b6b936c))
	ROM_LOAD("u27-lg1.rom", 0x8000, 0x8000, CRC(6ee69cda) SHA1(227a4b311b9fa5f34d38bee2b5063572a06809cf))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("eshk_u21.l1", 0x8000, 0x8000, CRC(feac68e5) SHA1(2f12a78398bc3a468e3e0656da91260d45b0663b))
	ROM_LOAD("eshk_u22.l1", 0x0000, 0x8000, CRC(44f50fe1) SHA1(a8e24dbb0f5cf300118e1ebdcd2bb6b274d87936))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("eshk_u4.l1", 0x00000, 0x8000, CRC(40069f8c) SHA1(aafdc189259fa9c8dc49e60e978b84775e16c64e))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("eshk_u19.l1", 0x20000, 0x8000, CRC(e5593075) SHA1(549b03402e5639b449e35325eb52e78f8810b07a))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

ROM_START(esha_lg2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-lg2.rom", 0x4000, 0x4000, CRC(e30361c6) SHA1(f5626aaf36348b3aad6b04901c5d84eee1153f51))
	ROM_LOAD("u27-lg1.rom", 0x8000, 0x8000, CRC(6ee69cda) SHA1(227a4b311b9fa5f34d38bee2b5063572a06809cf))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("eshk_u21.l1", 0x8000, 0x8000, CRC(feac68e5) SHA1(2f12a78398bc3a468e3e0656da91260d45b0663b))
	ROM_LOAD("eshk_u22.l1", 0x0000, 0x8000, CRC(44f50fe1) SHA1(a8e24dbb0f5cf300118e1ebdcd2bb6b274d87936))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("eshk_u4.l1", 0x00000, 0x8000, CRC(40069f8c) SHA1(aafdc189259fa9c8dc49e60e978b84775e16c64e))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("eshk_u19.l1", 0x20000, 0x8000, CRC(e5593075) SHA1(549b03402e5639b449e35325eb52e78f8810b07a))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

ROM_START(esha_la1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-la1.rom", 0x4000, 0x4000, CRC(c9c9a32d) SHA1(cd273198e777b644535836ea5785b0dfe5c792c5))
	ROM_LOAD("u27-la1.rom", 0x8000, 0x8000, CRC(3433b516) SHA1(5aff6bc72f2d6c0fd00f125ed5b4b6d8035d54bc))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("eshk_u21.l1", 0x8000, 0x8000, CRC(feac68e5) SHA1(2f12a78398bc3a468e3e0656da91260d45b0663b))
	ROM_LOAD("eshk_u22.l1", 0x0000, 0x8000, CRC(44f50fe1) SHA1(a8e24dbb0f5cf300118e1ebdcd2bb6b274d87936))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("eshk_u4.l1", 0x00000, 0x8000, CRC(40069f8c) SHA1(aafdc189259fa9c8dc49e60e978b84775e16c64e))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("eshk_u19.l1", 0x20000, 0x8000, CRC(e5593075) SHA1(549b03402e5639b449e35325eb52e78f8810b07a))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

ROM_START(esha_pa1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-pa1.rom", 0x4000, 0x4000, CRC(08c0b0d6) SHA1(36c23655e1ae07a3a5c91f68fdb27a78ca272683))
	ROM_LOAD("u27-pa1.rom", 0x8000, 0x8000, CRC(ddd6e8bb) SHA1(b46da424f9c4ac70e65af3ee7b4e08df38ffdb26))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("eshk_u21.l1", 0x8000, 0x8000, CRC(feac68e5) SHA1(2f12a78398bc3a468e3e0656da91260d45b0663b))
	ROM_LOAD("eshk_u22.l1", 0x0000, 0x8000, CRC(44f50fe1) SHA1(a8e24dbb0f5cf300118e1ebdcd2bb6b274d87936))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("u4-p1.rom", 0x00000, 0x8000, CRC(7219ffc2) SHA1(b8585b7d12f401d8ba4d95a5e2f20d35ff0ac26a))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("eshk_u19.l1", 0x20000, 0x8000, CRC(e5593075) SHA1(549b03402e5639b449e35325eb52e78f8810b07a))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

ROM_START(esha_pa4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("eshk_u26.pa4", 0x4000, 0x4000, CRC(9ffe5ee8) SHA1(533e5e90f6ce1ee4ca69463158860c7fef6b8e05))
	ROM_LOAD("eshk_u27.pa4", 0x8000, 0x8000, CRC(a1795d11) SHA1(1797aa566e7e490bca6b00891fed65ac1f115280))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("eshk_u21.l1", 0x8000, 0x8000, CRC(feac68e5) SHA1(2f12a78398bc3a468e3e0656da91260d45b0663b))
	ROM_LOAD("eshk_u22.l1", 0x0000, 0x8000, CRC(44f50fe1) SHA1(a8e24dbb0f5cf300118e1ebdcd2bb6b274d87936))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("u4-p1.rom", 0x00000, 0x8000, CRC(7219ffc2) SHA1(b8585b7d12f401d8ba4d95a5e2f20d35ff0ac26a))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("eshk_u19.l1", 0x20000, 0x8000, CRC(e5593075) SHA1(549b03402e5639b449e35325eb52e78f8810b07a))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

/*--------------------------------------------
/ Elvira and the Party Monsters 10/89 (#2011)
/--------------------------------------------*/
ROM_START(eatpm_l4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("elvi_u26.l4", 0x4000, 0x4000, CRC(24e09bf6) SHA1(0ff686c671e8cb2b2c8a9669bf44c3b0ba32ed4d))
	ROM_LOAD("elvi_u27.l4", 0x8000, 0x8000, CRC(3614f3e2) SHA1(3143fef8ab91ad357803d1e98b8ee953e6a194ef))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("elvi_u21.l1", 0x8000, 0x8000, CRC(68d44545) SHA1(8c3ea8521a44b1539cd148f142cca14184174ba7))
	ROM_LOAD("elvi_u22.l1", 0x0000, 0x8000, CRC(e525b4fe) SHA1(be728ec33a00b93c3346428a9248b588460af945))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("elvi_u4.l1", 0x00000, 0x8000, CRC(b5afa4db) SHA1(59b72dac5301a4befa01b93da5162478682e6021))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("elvi_u19.l1", 0x20000, 0x8000, CRC(806bc350) SHA1(d170aef11001096da9f2f7240726662009e26f5f))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
	ROM_LOAD("elvi_u20.l1", 0x40000, 0x8000, CRC(3d92d5fd) SHA1(834d40a59be57057103d1d8ab48fdaaf7dc5eda2))
	ROM_RELOAD(0x48000,0x8000)
	ROM_RELOAD(0x50000,0x8000)
	ROM_RELOAD(0x58000,0x8000)
ROM_END

ROM_START(eatpm_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-la1.rom", 0x4000, 0x4000, CRC(7a4873e6) SHA1(8e37ba2e428d83f6a84447761d99af12f5677c1d))
	ROM_LOAD("u27-la1.rom", 0x8000, 0x8000, CRC(d1c80549) SHA1(ab7dd88c460102e7db095a2df58c567ba43d81af))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("elvi_u21.l1", 0x8000, 0x8000, CRC(68d44545) SHA1(8c3ea8521a44b1539cd148f142cca14184174ba7))
	ROM_LOAD("elvi_u22.l1", 0x0000, 0x8000, CRC(e525b4fe) SHA1(be728ec33a00b93c3346428a9248b588460af945))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("elvi_u4.l1", 0x00000, 0x8000, CRC(b5afa4db) SHA1(59b72dac5301a4befa01b93da5162478682e6021))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("elvi_u19.l1", 0x20000, 0x8000, CRC(806bc350) SHA1(d170aef11001096da9f2f7240726662009e26f5f))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
	ROM_LOAD("elvi_u20.l1", 0x40000, 0x8000, CRC(3d92d5fd) SHA1(834d40a59be57057103d1d8ab48fdaaf7dc5eda2))
	ROM_RELOAD(0x48000,0x8000)
	ROM_RELOAD(0x50000,0x8000)
	ROM_RELOAD(0x58000,0x8000)
ROM_END

ROM_START(eatpm_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-la2.rom", 0x4000, 0x4000, CRC(c4dc967d) SHA1(e12d06282176d231ffa0e2895499ebd8dd8e6e4f))
	ROM_LOAD("u27-la2.rom", 0x8000, 0x8000, CRC(01e7aef5) SHA1(82c07635285ff9efb584043601ff5d811a1ab28b))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("elvi_u21.l1", 0x8000, 0x8000, CRC(68d44545) SHA1(8c3ea8521a44b1539cd148f142cca14184174ba7))
	ROM_LOAD("elvi_u22.l1", 0x0000, 0x8000, CRC(e525b4fe) SHA1(be728ec33a00b93c3346428a9248b588460af945))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("elvi_u4.l1", 0x00000, 0x8000, CRC(b5afa4db) SHA1(59b72dac5301a4befa01b93da5162478682e6021))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("elvi_u19.l1", 0x20000, 0x8000, CRC(806bc350) SHA1(d170aef11001096da9f2f7240726662009e26f5f))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
	ROM_LOAD("elvi_u20.l1", 0x40000, 0x8000, CRC(3d92d5fd) SHA1(834d40a59be57057103d1d8ab48fdaaf7dc5eda2))
	ROM_RELOAD(0x48000,0x8000)
	ROM_RELOAD(0x50000,0x8000)
	ROM_RELOAD(0x58000,0x8000)
ROM_END

ROM_START(eatpm_4g)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-lg4.rom", 0x4000, 0x4000, CRC(5e196382) SHA1(e948993ae100ab3d7e1b771f4ce22e3faaad84b4))
	ROM_LOAD("elvi_u27.l4", 0x8000, 0x8000, CRC(3614f3e2) SHA1(3143fef8ab91ad357803d1e98b8ee953e6a194ef))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("elvi_u21.l1", 0x8000, 0x8000, CRC(68d44545) SHA1(8c3ea8521a44b1539cd148f142cca14184174ba7))
	ROM_LOAD("elvi_u22.l1", 0x0000, 0x8000, CRC(e525b4fe) SHA1(be728ec33a00b93c3346428a9248b588460af945))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("elvi_u4.l1", 0x00000, 0x8000, CRC(b5afa4db) SHA1(59b72dac5301a4befa01b93da5162478682e6021))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("elvi_u19.l1", 0x20000, 0x8000, CRC(806bc350) SHA1(d170aef11001096da9f2f7240726662009e26f5f))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
	ROM_LOAD("elvi_u20.l1", 0x40000, 0x8000, CRC(3d92d5fd) SHA1(834d40a59be57057103d1d8ab48fdaaf7dc5eda2))
	ROM_RELOAD(0x48000,0x8000)
	ROM_RELOAD(0x50000,0x8000)
	ROM_RELOAD(0x58000,0x8000)
ROM_END

ROM_START(eatpm_3g)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-lg4.rom", 0x4000, 0x4000, BAD_DUMP CRC(5e196382) SHA1(e948993ae100ab3d7e1b771f4ce22e3faaad84b4)) // the U27 came without matching U26
	ROM_LOAD("elvira_u27_rom1_lg-3_novaapparate.bin", 0x8000, 0x8000, CRC(a3adae98) SHA1(3b94cd83ecbae1a58d780b35cac879d636c2d5b0))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("elvi_u21.l1", 0x8000, 0x8000, CRC(68d44545) SHA1(8c3ea8521a44b1539cd148f142cca14184174ba7))
	ROM_LOAD("elvi_u22.l1", 0x0000, 0x8000, CRC(e525b4fe) SHA1(be728ec33a00b93c3346428a9248b588460af945))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("elvi_u4.l1", 0x00000, 0x8000, CRC(b5afa4db) SHA1(59b72dac5301a4befa01b93da5162478682e6021))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("elvi_u19.l1", 0x20000, 0x8000, CRC(806bc350) SHA1(d170aef11001096da9f2f7240726662009e26f5f))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
	ROM_LOAD("elvi_u20.l1", 0x40000, 0x8000, CRC(3d92d5fd) SHA1(834d40a59be57057103d1d8ab48fdaaf7dc5eda2))
	ROM_RELOAD(0x48000,0x8000)
	ROM_RELOAD(0x50000,0x8000)
	ROM_RELOAD(0x58000,0x8000)
ROM_END

ROM_START(eatpm_4u)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-lu4.rom", 0x4000, 0x4000, CRC(504366c8) SHA1(1ca667208d4dcc8a09e35cad5f57798902611d7e))
	ROM_LOAD("elvi_u27.l4", 0x8000, 0x8000, CRC(3614f3e2) SHA1(3143fef8ab91ad357803d1e98b8ee953e6a194ef))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("elvi_u21.l1", 0x8000, 0x8000, CRC(68d44545) SHA1(8c3ea8521a44b1539cd148f142cca14184174ba7))
	ROM_LOAD("elvi_u22.l1", 0x0000, 0x8000, CRC(e525b4fe) SHA1(be728ec33a00b93c3346428a9248b588460af945))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("elvi_u4.l1", 0x00000, 0x8000, CRC(b5afa4db) SHA1(59b72dac5301a4befa01b93da5162478682e6021))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("elvi_u19.l1", 0x20000, 0x8000, CRC(806bc350) SHA1(d170aef11001096da9f2f7240726662009e26f5f))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
	ROM_LOAD("elvi_u20.l1", 0x40000, 0x8000, CRC(3d92d5fd) SHA1(834d40a59be57057103d1d8ab48fdaaf7dc5eda2))
	ROM_RELOAD(0x48000,0x8000)
	ROM_RELOAD(0x50000,0x8000)
	ROM_RELOAD(0x58000,0x8000)
ROM_END

ROM_START(eatpm_f1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-lf1.rom", 0x4000, 0x4000, CRC(d479fae4) SHA1(488890fa3ef185499d6f0d620e054cdc96548c40))
	ROM_LOAD("u27-la1.rom", 0x8000, 0x8000, CRC(d1c80549) SHA1(ab7dd88c460102e7db095a2df58c567ba43d81af))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("elvi_u21.l1", 0x8000, 0x8000, CRC(68d44545) SHA1(8c3ea8521a44b1539cd148f142cca14184174ba7))
	ROM_LOAD("elvi_u22.l1", 0x0000, 0x8000, CRC(e525b4fe) SHA1(be728ec33a00b93c3346428a9248b588460af945))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("elvi_u4.l1", 0x00000, 0x8000, CRC(b5afa4db) SHA1(59b72dac5301a4befa01b93da5162478682e6021))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("elvi_u19.l1", 0x20000, 0x8000, CRC(806bc350) SHA1(d170aef11001096da9f2f7240726662009e26f5f))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
	ROM_LOAD("elvi_u20.l1", 0x40000, 0x8000, CRC(3d92d5fd) SHA1(834d40a59be57057103d1d8ab48fdaaf7dc5eda2))
	ROM_RELOAD(0x48000,0x8000)
	ROM_RELOAD(0x50000,0x8000)
	ROM_RELOAD(0x58000,0x8000)
ROM_END

ROM_START(eatpm_p7)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-pa7.rom", 0x4000, 0x4000, CRC(0bcc6639) SHA1(016685f6f0ed144e673846c5d44c81baa273c949))
	ROM_LOAD("u27-pa7.rom", 0x8000, 0x8000, CRC(c9c2bbf0) SHA1(9d23ccd26dc103edee303759f10b11ce0381223b))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("elvi_u21.l1", 0x8000, 0x8000, CRC(68d44545) SHA1(8c3ea8521a44b1539cd148f142cca14184174ba7))
	ROM_LOAD("elvi_u22.l1", 0x0000, 0x8000, CRC(e525b4fe) SHA1(be728ec33a00b93c3346428a9248b588460af945))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("elvi_u4.l1", 0x00000, 0x8000, CRC(b5afa4db) SHA1(59b72dac5301a4befa01b93da5162478682e6021))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("elvi_u19.l1", 0x20000, 0x8000, CRC(806bc350) SHA1(d170aef11001096da9f2f7240726662009e26f5f))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
	ROM_LOAD("elvi_u20.l1", 0x40000, 0x8000, CRC(3d92d5fd) SHA1(834d40a59be57057103d1d8ab48fdaaf7dc5eda2))
	ROM_RELOAD(0x48000,0x8000)
	ROM_RELOAD(0x50000,0x8000)
	ROM_RELOAD(0x58000,0x8000)
ROM_END

/*--------------------
/ Jokerz! 1/89 (#567)
/--------------------*/
// NOTE: This game uses an entirely different sound/music board unique for this game: D-12338-567
ROM_START(jokrz_l6)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("jokeru26.l6", 0x4000, 0x4000, CRC(c748c1ba) SHA1(e74b3be2c5d3e81ff29bc4444384f456846111b3))
	ROM_LOAD("jokeru27.l6", 0x8000, 0x8000, CRC(612d0ea7) SHA1(35d88de615a15442689e13414117b7dfca6a4614))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("jokeru21.l1", 0x8000, 0x8000, CRC(9e2be4f6) SHA1(6e26b55935d0c8138176b54a11c1a9ab58366628))
	ROM_LOAD("jokeru22.l1", 0x0000, 0x8000, CRC(2f67160c) SHA1(f1e179fde41f9bf8226069c24b0bd5152a13e518))
	ROM_REGION(0x40000, "ps88:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("jokeru5.l2", 0x30000, 0x10000, CRC(e9dc0095) SHA1(23a99555e50461ccc8e67de01796642c080294c2))
	ROM_RELOAD(0x20000,0x10000)
ROM_END

ROM_START(jokrz_l3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-l3.rom", 0x4000, 0x4000, CRC(3bf963df) SHA1(9f7757d96deca8638dbc1fe3669eee78dc222ebb))
	ROM_LOAD("u27-l3.rom", 0x8000, 0x8000, CRC(32526aff) SHA1(c4ee4b58e90f214012addada114fc9333d2d274c))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("jokeru21.l1", 0x8000, 0x8000, CRC(9e2be4f6) SHA1(6e26b55935d0c8138176b54a11c1a9ab58366628))
	ROM_LOAD("jokeru22.l1", 0x0000, 0x8000, CRC(2f67160c) SHA1(f1e179fde41f9bf8226069c24b0bd5152a13e518))
	ROM_REGION(0x40000, "ps88:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("jokeru5.l2", 0x30000, 0x10000, CRC(e9dc0095) SHA1(23a99555e50461ccc8e67de01796642c080294c2))
	ROM_RELOAD(0x20000,0x10000)
ROM_END

ROM_START(jokrz_g4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-l3.rom", 0x4000, 0x4000, CRC(3bf963df) SHA1(9f7757d96deca8638dbc1fe3669eee78dc222ebb))
	ROM_LOAD("jokerz_u27.g4", 0x8000, 0x8000, CRC(3f3fe7ef) SHA1(afd17cea8442948b5aa0bb842dfbc17442b06a1d))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("jokeru21.l1", 0x8000, 0x8000, CRC(9e2be4f6) SHA1(6e26b55935d0c8138176b54a11c1a9ab58366628))
	ROM_LOAD("jokeru22.l1", 0x0000, 0x8000, CRC(2f67160c) SHA1(f1e179fde41f9bf8226069c24b0bd5152a13e518))
	ROM_REGION(0x40000, "ps88:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("jokeru5.l2", 0x30000, 0x10000, CRC(e9dc0095) SHA1(23a99555e50461ccc8e67de01796642c080294c2))
	ROM_RELOAD(0x20000,0x10000)
ROM_END

/*-------------------------------
/ Mousin' Around! 12/89 (#2009)
/-------------------------------*/
ROM_START(mousn_l4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mous_u26.l4", 0x4000, 0x4000, CRC(a540edc1) SHA1(c0b208369ac770f0d4cd7decfce5f8401ded082f))
	ROM_LOAD("mous_u27.l4", 0x8000, 0x8000, CRC(ff108148) SHA1(32b44286d43a39d5677c6582c5b09fc3b9833806))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("mous_u20.l2", 0x8000, 0x8000, CRC(59b1b0c5) SHA1(443426be41c1413f22b137145dbc3bcf84d9ccc7))
	ROM_LOAD("mous_u22.l1", 0x0000, 0x8000, CRC(00ad198c) SHA1(4f15696909e1f3574ad20b28e31da2c155ed129f))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("mous_u4.l2", 0x00000, 0x8000, CRC(643add1e) SHA1(45dea0f4c6f24d17e6f7dda75afaa7caefdc6b96))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("mous_u19.l2", 0x20000, 0x8000, CRC(7b4941f7) SHA1(2b2fc8e7634b1885b020b2115126d6341172cc91))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
	ROM_LOAD("mous_u20.l2", 0x40000, 0x8000, CRC(59b1b0c5) SHA1(443426be41c1413f22b137145dbc3bcf84d9ccc7))
	ROM_RELOAD(0x48000,0x8000)
	ROM_RELOAD(0x50000,0x8000)
	ROM_RELOAD(0x58000,0x8000)
ROM_END

ROM_START(mousn_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-la1.rom", 0x4000, 0x4000, CRC(0fff7946) SHA1(53bd68fd21218128f9311047ac911cff7eea8b23))
	ROM_LOAD("u27-la1.rom", 0x8000, 0x8000, CRC(a440192b) SHA1(837a9eb290f46d792f7307c569dfc627507420b8))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("mous_u20.l2", 0x8000, 0x8000, CRC(59b1b0c5) SHA1(443426be41c1413f22b137145dbc3bcf84d9ccc7))
	ROM_LOAD("mous_u22.l1", 0x0000, 0x8000, CRC(00ad198c) SHA1(4f15696909e1f3574ad20b28e31da2c155ed129f))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("mous_u4.l2", 0x00000, 0x8000, CRC(643add1e) SHA1(45dea0f4c6f24d17e6f7dda75afaa7caefdc6b96))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("mous_u19.l2", 0x20000, 0x8000, CRC(7b4941f7) SHA1(2b2fc8e7634b1885b020b2115126d6341172cc91))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
	ROM_LOAD("mous_u20.l2", 0x40000, 0x8000, CRC(59b1b0c5) SHA1(443426be41c1413f22b137145dbc3bcf84d9ccc7))
	ROM_RELOAD(0x48000,0x8000)
	ROM_RELOAD(0x50000,0x8000)
	ROM_RELOAD(0x58000,0x8000)
ROM_END

ROM_START(mousn_lu)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-la1.rom", 0x4000, 0x4000, CRC(0fff7946) SHA1(53bd68fd21218128f9311047ac911cff7eea8b23))
	ROM_LOAD("u27-lu1.rom", 0x8000, 0x8000, CRC(6e5b692c) SHA1(20c4b8d105d5df6e1b540c02c1c54bca08ec42e8))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("mous_u20.l2", 0x8000, 0x8000, CRC(59b1b0c5) SHA1(443426be41c1413f22b137145dbc3bcf84d9ccc7))
	ROM_LOAD("mous_u22.l1", 0x0000, 0x8000, CRC(00ad198c) SHA1(4f15696909e1f3574ad20b28e31da2c155ed129f))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("mous_u4.l2", 0x00000, 0x8000, CRC(643add1e) SHA1(45dea0f4c6f24d17e6f7dda75afaa7caefdc6b96))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("mous_u19.l2", 0x20000, 0x8000, CRC(7b4941f7) SHA1(2b2fc8e7634b1885b020b2115126d6341172cc91))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
	ROM_LOAD("mous_u20.l2", 0x40000, 0x8000, CRC(59b1b0c5) SHA1(443426be41c1413f22b137145dbc3bcf84d9ccc7))
	ROM_RELOAD(0x48000,0x8000)
	ROM_RELOAD(0x50000,0x8000)
	ROM_RELOAD(0x58000,0x8000)
ROM_END

ROM_START(mousn_lx)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mous_u26.l4", 0x4000, 0x4000, CRC(a540edc1) SHA1(c0b208369ac770f0d4cd7decfce5f8401ded082f))
	ROM_LOAD("mous_u27.l4", 0x8000, 0x8000, CRC(ff108148) SHA1(32b44286d43a39d5677c6582c5b09fc3b9833806))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("mous_u20.l2", 0x8000, 0x8000, CRC(59b1b0c5) SHA1(443426be41c1413f22b137145dbc3bcf84d9ccc7))
	ROM_LOAD("mous_u22.l1", 0x0000, 0x8000, CRC(00ad198c) SHA1(4f15696909e1f3574ad20b28e31da2c155ed129f))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("mous_u4.lx", 0x00000, 0x8000, CRC(d311db4a) SHA1(d9d20921eb42c19c5074c976608bfec0d3130204))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("mous_u19.lx", 0x20000, 0x8000, CRC(c7a6f494) SHA1(272f0bd3885bb81da13ee6ed3d66f9424ccf4b0d))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
	ROM_LOAD("mous_u20.l2", 0x40000, 0x8000, CRC(59b1b0c5) SHA1(443426be41c1413f22b137145dbc3bcf84d9ccc7))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
ROM_END

/*-------------------------
/ Police Force 9/89 (#573)
/-------------------------*/
ROM_START(polic_l4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("pfrc_u26.l4", 0x4000, 0x4000, CRC(1a1409e9) SHA1(775d35a22483bcf8c4b03841e0aca22b6504a48f))
	ROM_LOAD("pfrc_u27.l4", 0x8000, 0x8000, CRC(641ed5d4) SHA1(f98b8bb64184aba062715555bd1de679d6382ac3))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("pfrc_u21.l1", 0x8000, 0x8000, CRC(7729afd3) SHA1(9cd2898a7a4203cf3b2dcd203e25cde5dd582ee7))
	ROM_LOAD("pfrc_u22.l1", 0x0000, 0x8000, CRC(40f5e6b2) SHA1(4af2e2658720b08d03d24c9d314a6e5074b2c747))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("pfrc_u4.l2", 0x00000, 0x8000, CRC(8f431529) SHA1(0f479990715a31fd860c000a066cffb70da502c2))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("pfrc_u19.l1", 0x20000, 0x8000, CRC(abc4caeb) SHA1(6faef2de9a49a1015b4038ab18849de2f25dbded))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

ROM_START(polic_l3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("pfrc_u26.l4", 0x4000, 0x4000, CRC(1a1409e9) SHA1(775d35a22483bcf8c4b03841e0aca22b6504a48f))
	ROM_LOAD("pfrc_u27.lx3", 0x8000, 0x8000, CRC(ef5d4808) SHA1(89cf62640e39397899776ab1d132645a5eab9e0e))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("pfrc_u21.l1", 0x8000, 0x8000, CRC(7729afd3) SHA1(9cd2898a7a4203cf3b2dcd203e25cde5dd582ee7))
	ROM_LOAD("pfrc_u22.l1", 0x0000, 0x8000, CRC(40f5e6b2) SHA1(4af2e2658720b08d03d24c9d314a6e5074b2c747))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("pfrc_u4.l2", 0x00000, 0x8000, CRC(8f431529) SHA1(0f479990715a31fd860c000a066cffb70da502c2))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("pfrc_u19.l1", 0x20000, 0x8000, CRC(abc4caeb) SHA1(6faef2de9a49a1015b4038ab18849de2f25dbded))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

ROM_START(polic_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("pfrc_u26.l2", 0x4000, 0x4000, CRC(4bc972dc) SHA1(7d6e421945832bd2c95a7b8e27d5573a42109379))
	ROM_LOAD("pfrc_u27.l2", 0x8000, 0x8000, CRC(46ae36f2) SHA1(6685efa858a14b21fae5e3192ab714750ff51341))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("pfrc_u21.l1", 0x8000, 0x8000, CRC(7729afd3) SHA1(9cd2898a7a4203cf3b2dcd203e25cde5dd582ee7))
	ROM_LOAD("pfrc_u22.l1", 0x0000, 0x8000, CRC(40f5e6b2) SHA1(4af2e2658720b08d03d24c9d314a6e5074b2c747))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("pfrc_u4.l2", 0x00000, 0x8000, CRC(8f431529) SHA1(0f479990715a31fd860c000a066cffb70da502c2))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("pfrc_u19.l1", 0x20000, 0x8000, CRC(abc4caeb) SHA1(6faef2de9a49a1015b4038ab18849de2f25dbded))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

ROM_START(polic_g4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("pfrc_u26.l4",  0x4000, 0x4000, CRC(1a1409e9) SHA1(775d35a22483bcf8c4b03841e0aca22b6504a48f))
	ROM_LOAD("pfrc_u27.lg4", 0x8000, 0x8000, CRC(058322e7) SHA1(87847065c0785dbd4dff61cc256ed73ff929c40d))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("pfrc_u21.l1", 0x8000, 0x8000, CRC(7729afd3) SHA1(9cd2898a7a4203cf3b2dcd203e25cde5dd582ee7))
	ROM_LOAD("pfrc_u22.l1", 0x0000, 0x8000, CRC(40f5e6b2) SHA1(4af2e2658720b08d03d24c9d314a6e5074b2c747))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("pfrc_u4.l2", 0x00000, 0x8000, CRC(8f431529) SHA1(0f479990715a31fd860c000a066cffb70da502c2))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("pfrc_u19.l1", 0x20000, 0x8000, CRC(abc4caeb) SHA1(6faef2de9a49a1015b4038ab18849de2f25dbded))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

/*--------------------------
/ Space Station 1/88 (#552)
/--------------------------*/
ROM_START(spstn_l5)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("sstn_u26.l5", 0x4000, 0x4000, CRC(614c8528) SHA1(4f177e3d72a5cc302c62c756ec778ae2a98c8f2e))
	ROM_LOAD("sstn_u27.l5", 0x8000, 0x8000, CRC(4558d963) SHA1(be317310978cca4ddd616d76fe892dcf7c980473))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("sstn_u21.l1", 0x8000, 0x8000, CRC(a2ceccaa) SHA1(4c23713543e06458e49e3f2d472543c4a4246a93))
	ROM_LOAD("sstn_u22.l1", 0x0000, 0x8000, CRC(2b745994) SHA1(67ebfe13db6670237496b033611bf9d4ba8d5c30))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("sstn_u4.l1", 0x00000, 0x8000, CRC(ad7a0511) SHA1(9aa6412de12599fd0d10faef8fffb5d535f49015))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
ROM_END

/*----------------------------
/ Swords of Fury 8/88 (#559)
/----------------------------*/
ROM_START(swrds_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("swrd_u26.l2", 0x4000, 0x4000, CRC(c036f4ff) SHA1(a86840dbc117774aeca695ded1ab3ec76e134325))
	ROM_LOAD("swrd_u27.l2", 0x8000, 0x8000, CRC(33b0fb5a) SHA1(a55bdfe20b1c869eae52d3be75df1c550d0b20f5))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("swrd_u21.l1", 0x8000, 0x8000, CRC(ee8b0a64) SHA1(c2c52059a9a5f7c0abcfdd76cfc6d5b5451f7d1e))
	ROM_LOAD("swrd_u22.l1", 0x0000, 0x8000, CRC(73dcdbb0) SHA1(66f5b3804442a1742b6fb3cccf539c4df956b3f2))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("swrd_u4.l1", 0x00000, 0x8000, CRC(272b509c) SHA1(756d3783f664ca1c41dd1d12032330b74c3f89ea))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("swrd_u19.l1", 0x20000, 0x8000, CRC(a22f84fa) SHA1(1731e86e85cca2d283512d5048c787df3970c9c5))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

ROM_START(swrds_lg2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("swrd_u26.l1",  0x4000, 0x4000, CRC(e7a54874) SHA1(9f3486a0eed32ab0919805bed771ffd214a78967))
	ROM_LOAD("swrd_u27g.l2", 0x8000, 0x8000, CRC(ef7194d4) SHA1(45b4fc316b603921fdafaf2638a46a94e6efc190))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("swrd_u21.l1", 0x8000, 0x8000, CRC(ee8b0a64) SHA1(c2c52059a9a5f7c0abcfdd76cfc6d5b5451f7d1e))
	ROM_LOAD("swrd_u22.l1", 0x0000, 0x8000, CRC(73dcdbb0) SHA1(66f5b3804442a1742b6fb3cccf539c4df956b3f2))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("swrd_u4.l1", 0x00000, 0x8000, CRC(272b509c) SHA1(756d3783f664ca1c41dd1d12032330b74c3f89ea))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("swrd_u19.l1", 0x20000, 0x8000, CRC(a22f84fa) SHA1(1731e86e85cca2d283512d5048c787df3970c9c5))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

ROM_START(swrds_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("swrd_u26.l1", 0x4000, 0x4000, CRC(e7a54874) SHA1(9f3486a0eed32ab0919805bed771ffd214a78967))
	ROM_LOAD("swrd_u27.l1", 0x8000, 0x8000, CRC(885dc3d6) SHA1(26463efb476c0bbfc3cb49b450167c4a985672bf))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("swrd_u21.l1", 0x8000, 0x8000, CRC(ee8b0a64) SHA1(c2c52059a9a5f7c0abcfdd76cfc6d5b5451f7d1e))
	ROM_LOAD("swrd_u22.l1", 0x0000, 0x8000, CRC(73dcdbb0) SHA1(66f5b3804442a1742b6fb3cccf539c4df956b3f2))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("swrd_u4.l1", 0x00000, 0x8000, CRC(272b509c) SHA1(756d3783f664ca1c41dd1d12032330b74c3f89ea))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("swrd_u19.l1", 0x20000, 0x8000, CRC(a22f84fa) SHA1(1731e86e85cca2d283512d5048c787df3970c9c5))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

/*--------------------
/ Taxi 10/88 (#553)
/--------------------*/
ROM_START(taxi_l4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("taxi_u26.l4", 0x4000, 0x4000, CRC(a70d8088) SHA1(0986035436e2b1199571248dac8eb7a903b5015c))
	ROM_LOAD("taxi_u27.l4", 0x8000, 0x8000, CRC(f973f79c) SHA1(a33ab04451d8a5b2354e4d174c238878e962f228))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("taxi_u21.l1", 0x8000, 0x8000, CRC(2b20e9ab) SHA1(d785667ae0fd237dd8343bb1ecfbacf050ec2c6f))
	ROM_LOAD("taxi_u22.l1", 0x0000, 0x8000, CRC(d13055c5) SHA1(8c2959bde03567b83db425ebc9e7309d9601c2b2))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("taxi_u4.l1", 0x00000, 0x8000, CRC(6082ebb5) SHA1(37e19ad27fe05b4c8e572f6598d2d574e4ac5a7d))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("taxi_u19.l1", 0x20000, 0x8000, CRC(91c64913) SHA1(10e48977f925f6bc1be0c56854aafa99283b4047))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

ROM_START(taxi_l3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("taxi_u26.l4", 0x4000, 0x4000, CRC(a70d8088) SHA1(0986035436e2b1199571248dac8eb7a903b5015c))
	ROM_LOAD("taxi_u27.l3", 0x8000, 0x8000, CRC(e2bfb6fa) SHA1(ba1bddffe4d4e8f04131dd6f5a0380765fbcdfc5))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("taxi_u21.l1", 0x8000, 0x8000, CRC(2b20e9ab) SHA1(d785667ae0fd237dd8343bb1ecfbacf050ec2c6f))
	ROM_LOAD("taxi_u22.l1", 0x0000, 0x8000, CRC(d13055c5) SHA1(8c2959bde03567b83db425ebc9e7309d9601c2b2))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("taxi_u4.l1", 0x00000, 0x8000, CRC(6082ebb5) SHA1(37e19ad27fe05b4c8e572f6598d2d574e4ac5a7d))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("taxi_u19.l1", 0x20000, 0x8000, CRC(91c64913) SHA1(10e48977f925f6bc1be0c56854aafa99283b4047))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

ROM_START(taxi_lu1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-lu1m.rom", 0x4000, 0x4000, CRC(40a2f33c) SHA1(815910b36a5df6c63862590c42b6a41286f38236))
	ROM_LOAD("u27-lu1m.rom", 0x8000, 0x8000, CRC(d15f08b4) SHA1(adef8c2c7ee685207b146b4401dc9b732cf212b7))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("taxi_u21.l1", 0x8000, 0x8000, CRC(2b20e9ab) SHA1(d785667ae0fd237dd8343bb1ecfbacf050ec2c6f))
	ROM_LOAD("taxi_u22.l1", 0x0000, 0x8000, CRC(d13055c5) SHA1(8c2959bde03567b83db425ebc9e7309d9601c2b2))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("taxi_u4.l1", 0x00000, 0x8000, CRC(6082ebb5) SHA1(37e19ad27fe05b4c8e572f6598d2d574e4ac5a7d))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("taxi_u19.l1", 0x20000, 0x8000, CRC(91c64913) SHA1(10e48977f925f6bc1be0c56854aafa99283b4047))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

ROM_START(taxi_lg1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-lu1m.rom", 0x4000, 0x4000, CRC(40a2f33c) SHA1(815910b36a5df6c63862590c42b6a41286f38236))
	ROM_LOAD("u27-lg1m.rom", 0x8000, 0x8000, CRC(955dcbab) SHA1(e66e0da6366885ceed7618b09cf66fe11ae27627))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("taxi_u21.l1", 0x8000, 0x8000, CRC(2b20e9ab) SHA1(d785667ae0fd237dd8343bb1ecfbacf050ec2c6f))
	ROM_LOAD("taxi_u22.l1", 0x0000, 0x8000, CRC(d13055c5) SHA1(8c2959bde03567b83db425ebc9e7309d9601c2b2))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("taxi_u4.l1", 0x00000, 0x8000, CRC(6082ebb5) SHA1(37e19ad27fe05b4c8e572f6598d2d574e4ac5a7d))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("taxi_u19.l1", 0x20000, 0x8000, CRC(91c64913) SHA1(10e48977f925f6bc1be0c56854aafa99283b4047))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

ROM_START(taxi_p5)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("taxi_u26.p5", 0x4000, 0x4000, CRC(b02bf136) SHA1(e4ec9f9966be3b68d4bff3e028b03cf5248c0c54))
	ROM_LOAD("taxi_u27.p5", 0x8000, 0x8000, CRC(ea8065d1) SHA1(08cfb8f9d8f634f2719cc1ce00e6a8f8cee5cdbe))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("taxi_u21.l1", 0x8000, 0x8000, CRC(2b20e9ab) SHA1(d785667ae0fd237dd8343bb1ecfbacf050ec2c6f))
	ROM_LOAD("taxi_u22.l1", 0x0000, 0x8000, CRC(d13055c5) SHA1(8c2959bde03567b83db425ebc9e7309d9601c2b2))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("taxi_u4.l1", 0x00000, 0x8000, CRC(6082ebb5) SHA1(37e19ad27fe05b4c8e572f6598d2d574e4ac5a7d))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("taxi_u19.l1", 0x20000, 0x8000, CRC(91c64913) SHA1(10e48977f925f6bc1be0c56854aafa99283b4047))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
ROM_END

/*--------------------------------------
/ Transporter the Rescue 6/89 (#2008)
/--------------------------------------*/
ROM_START(tsptr_l3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tran_u26.l3", 0x4000, 0x4000, CRC(2d48a108) SHA1(d41bf077aab1201b08ea14725d4a0d841ee6b919))
	ROM_LOAD("tran_u27.l3", 0x8000, 0x8000, CRC(50efb01c) SHA1(941f18d51bf8a5d209ed90e0865b7fa638a6eab3))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("tran_u21.l2", 0x8000, 0x8000, CRC(b10120ee) SHA1(305a898a8b762c27dba26921ef169556bf96e518))
	ROM_LOAD("tran_u22.l2", 0x0000, 0x8000, CRC(337784b5) SHA1(30c17afd8f76118940982db946cd3a2a29445d10))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("tran_u4.l2", 0x00000, 0x8000, CRC(a06ddd61) SHA1(630fe7ab94516930c4876a95f822024a44371170))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("tran_u19.l2", 0x20000, 0x8000, CRC(3cfde8b0) SHA1(7bdc71ba1ba4fd337f052354323c86fd97b2b881))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
	ROM_LOAD("tran_u20.l2", 0x40000, 0x8000, CRC(e9890cf1) SHA1(0ae37504c704401101c79ce49df11044f8d8caa9))
	ROM_RELOAD(0x48000,0x8000)
	ROM_RELOAD(0x50000,0x8000)
	ROM_RELOAD(0x58000,0x8000)
ROM_END

ROM_START(tsptr_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("transporter-la1-u26-checksumb412.l1", 0x4000, 0x4000, CRC(3504300f) SHA1(1a8b779b7375e4087e42b31c1aa17a8a32c6d6aa))
	ROM_LOAD("transporter-la1-u27-checksum01cf.l1", 0x8000, 0x8000, CRC(49635399) SHA1(8cdc700c501f0d611152010d5ae28bcd84d06861))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("transporter-u21-checksumca54.l1", 0x8000, 0x8000, CRC(2b194ca6) SHA1(20cb956143622409a7f4b918ab1699db1b6e6b07))
	ROM_LOAD("transporter-u22-checksumd84c.l1", 0x0000, 0x8000, CRC(4c7ba6d7) SHA1(0134dce454c29c572c4ee0e0139a8ad5f0249b99))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("transporter-u4-checksum58b0.l1", 0x00000, 0x8000, CRC(63e92f8b) SHA1(57f2841419415fc3560d46a63119c76f98cade9b))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("transporter-u19-checksumc7af.l1", 0x20000, 0x8000, CRC(3cfde8b0) SHA1(7bdc71ba1ba4fd337f052354323c86fd97b2b881)) // only common ROM with L3
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
	ROM_LOAD("transporter-u20-checksum21ae.l1", 0x40000, 0x8000, CRC(fabddaaf) SHA1(7c014bb5b1ac8da61ffd265ba98bcb8256c5f666))
	ROM_RELOAD(0x48000,0x8000)
	ROM_RELOAD(0x50000,0x8000)
	ROM_RELOAD(0x58000,0x8000)
ROM_END

/*-----------------------
/ Whirlwind 4/90 (#574)
/-----------------------*/
ROM_START(whirl_l3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("whir_u26.l3", 0x4000, 0x4000, CRC(066b8fec) SHA1(017ca12ef5ebd9bb70690b0e096064be5144a512))
	ROM_LOAD("whir_u27.l3", 0x8000, 0x8000, CRC(47fc033d) SHA1(42518650ecb538323bc33ee193bc229d89ca1936))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("whir_u21.l1", 0x8000, 0x8000, CRC(fa3da322) SHA1(732107eace9eecdb97eff4abb4420a2febef7425))
	ROM_LOAD("whir_u22.l1", 0x0000, 0x8000, CRC(fcaf8c4e) SHA1(8e8cab1923a56bcef4671dce28aef1e39303c04a))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("whir_u4.l1", 0x00000, 0x8000, CRC(29952d84) SHA1(26479a341b0552c5f9d9bf9dd013855e51a7b857))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("whir_u19.l1", 0x20000, 0x8000, CRC(c63f6fe9) SHA1(947bbccb5eeae414770254d42d0a95425e2dca8c))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
	ROM_LOAD("whir_u20.l1", 0x40000, 0x8000, CRC(713007af) SHA1(3ac88bb905ccf8e227bbf3c102c74e3d2446cc88))
	ROM_RELOAD(0x48000,0x8000)
	ROM_RELOAD(0x50000,0x8000)
	ROM_RELOAD(0x58000,0x8000)
ROM_END

ROM_START(whirl_lg3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("whir_u26.l3", 0x4000, 0x4000, CRC(066b8fec) SHA1(017ca12ef5ebd9bb70690b0e096064be5144a512))
	ROM_LOAD("whir_u27.lg3", 0x8000, 0x8000, CRC(94c7a45a) SHA1(617f38f14c1cf8d6cbb6a41080e2d66c7c572b7f))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("whir_u21.l1", 0x8000, 0x8000, CRC(fa3da322) SHA1(732107eace9eecdb97eff4abb4420a2febef7425))
	ROM_LOAD("whir_u22.l1", 0x0000, 0x8000, CRC(fcaf8c4e) SHA1(8e8cab1923a56bcef4671dce28aef1e39303c04a))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("whir_u4.l1", 0x00000, 0x8000, CRC(29952d84) SHA1(26479a341b0552c5f9d9bf9dd013855e51a7b857))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("whir_u19.l1", 0x20000, 0x8000, CRC(c63f6fe9) SHA1(947bbccb5eeae414770254d42d0a95425e2dca8c))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
	ROM_LOAD("whir_u20.l1", 0x40000, 0x8000, CRC(713007af) SHA1(3ac88bb905ccf8e227bbf3c102c74e3d2446cc88))
	ROM_RELOAD(0x48000,0x8000)
	ROM_RELOAD(0x50000,0x8000)
	ROM_RELOAD(0x58000,0x8000)
ROM_END

ROM_START(whirl_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("whir_u26.l3", 0x4000, 0x4000, CRC(066b8fec) SHA1(017ca12ef5ebd9bb70690b0e096064be5144a512))
	ROM_LOAD("wwdgu27.l2", 0x8000, 0x8000, CRC(d8fb48f3) SHA1(8c64f94cca51abd6f4a7e53ac59a6f623bd2cfd7))
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("whir_u21.l1", 0x8000, 0x8000, CRC(fa3da322) SHA1(732107eace9eecdb97eff4abb4420a2febef7425))
	ROM_LOAD("whir_u22.l1", 0x0000, 0x8000, CRC(fcaf8c4e) SHA1(8e8cab1923a56bcef4671dce28aef1e39303c04a))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("whir_u4.l1", 0x00000, 0x8000, CRC(29952d84) SHA1(26479a341b0552c5f9d9bf9dd013855e51a7b857))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("whir_u19.l1", 0x20000, 0x8000, CRC(c63f6fe9) SHA1(947bbccb5eeae414770254d42d0a95425e2dca8c))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
	ROM_LOAD("whir_u20.l1", 0x40000, 0x8000, CRC(713007af) SHA1(3ac88bb905ccf8e227bbf3c102c74e3d2446cc88))
	ROM_RELOAD(0x48000,0x8000)
	ROM_RELOAD(0x50000,0x8000)
	ROM_RELOAD(0x58000,0x8000)
ROM_END

ROM_START(whirl_lg2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("whir_u26.l3", 0x4000, 0x4000, CRC(066b8fec) SHA1(017ca12ef5ebd9bb70690b0e096064be5144a512))
	ROM_LOAD("whirlwind_u27_lg2.bin", 0x8000, 0x8000, CRC(49f03cd9) SHA1(ee7837861678f86d3903842e1895de358383b6b2)) // the U27 came without U26, so unknown if these 2 really match, even though the game runs
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("whir_u21.l1", 0x8000, 0x8000, CRC(fa3da322) SHA1(732107eace9eecdb97eff4abb4420a2febef7425))
	ROM_LOAD("whir_u22.l1", 0x0000, 0x8000, CRC(fcaf8c4e) SHA1(8e8cab1923a56bcef4671dce28aef1e39303c04a))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("whir_u4.l1", 0x00000, 0x8000, CRC(29952d84) SHA1(26479a341b0552c5f9d9bf9dd013855e51a7b857))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("whir_u19.l1", 0x20000, 0x8000, CRC(c63f6fe9) SHA1(947bbccb5eeae414770254d42d0a95425e2dca8c))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
	ROM_LOAD("whir_u20.l1", 0x40000, 0x8000, CRC(713007af) SHA1(3ac88bb905ccf8e227bbf3c102c74e3d2446cc88))
	ROM_RELOAD(0x48000,0x8000)
	ROM_RELOAD(0x50000,0x8000)
	ROM_RELOAD(0x58000,0x8000)
ROM_END

ROM_START(whirl_lg1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("whir_u26.l3", 0x4000, 0x4000, CRC(066b8fec) SHA1(017ca12ef5ebd9bb70690b0e096064be5144a512))
	ROM_LOAD("whirlwind_u27_lg1.bin", 0x8000, 0x8000, CRC(e85a5004) SHA1(dd88d8b26e44df1bc9304f844cf1f8cbc46f31f7)) // the U27 came without U26, so unknown if these 2 really match, even though the game runs
	ROM_REGION(0x10000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("whir_u21.l1", 0x8000, 0x8000, CRC(fa3da322) SHA1(732107eace9eecdb97eff4abb4420a2febef7425))
	ROM_LOAD("whir_u22.l1", 0x0000, 0x8000, CRC(fcaf8c4e) SHA1(8e8cab1923a56bcef4671dce28aef1e39303c04a))
	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("whir_u4.l1", 0x00000, 0x8000, CRC(29952d84) SHA1(26479a341b0552c5f9d9bf9dd013855e51a7b857))
	ROM_RELOAD(0x08000,0x8000)
	ROM_RELOAD(0x10000,0x8000)
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("whir_u19.l1", 0x20000, 0x8000, CRC(c63f6fe9) SHA1(947bbccb5eeae414770254d42d0a95425e2dca8c))
	ROM_RELOAD(0x28000,0x8000)
	ROM_RELOAD(0x30000,0x8000)
	ROM_RELOAD(0x38000,0x8000)
	ROM_LOAD("whir_u20.l1", 0x40000, 0x8000, CRC(713007af) SHA1(3ac88bb905ccf8e227bbf3c102c74e3d2446cc88))
	ROM_RELOAD(0x48000,0x8000)
	ROM_RELOAD(0x50000,0x8000)
	ROM_RELOAD(0x58000,0x8000)
ROM_END

GAME(1989,  bcats_l5,       0,          s11b,   s11b, s11b_state, init_s11bin, ROT0, "Williams", "Bad Cats (L-5)",                               MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989,  bcats_l2,       bcats_l5,   s11b,   s11b, s11b_state, init_s11bin, ROT0, "Williams", "Bad Cats (LA-2)",                              MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989,  bcats_g4,       bcats_l5,   s11b,   s11b, s11b_state, init_s11bin, ROT0, "Williams", "Bad Cats (LG-4)",                              MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988,  bnzai_l3,       0,          s11b,   s11b, s11b_state, init_s11bn7, ROT0, "Williams", "Banzai Run (L-3)",                             MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988,  bnzai_g3,       bnzai_l3,   s11b,   s11b, s11b_state, init_s11bn7, ROT0, "Williams", "Banzai Run (L-3) Germany",                     MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988,  bnzai_l1,       bnzai_l3,   s11b,   s11b, s11b_state, init_s11bn7, ROT0, "Williams", "Banzai Run (L-1)",                             MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988,  bnzai_pa,       bnzai_l3,   s11b,   s11b, s11b_state, init_s11bn7, ROT0, "Williams", "Banzai Run (P-A)",                             MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1987,  bguns_l8,       0,          s11b,   s11b, s11b_state, init_s11bnn, ROT0, "Williams", "Big Guns (L-8)",                               MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1987,  bguns_l7,       bguns_l8,   s11b,   s11b, s11b_state, init_s11bnn, ROT0, "Williams", "Big Guns (L-7)",                               MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1987,  bguns_la,       bguns_l8,   s11b,   s11b, s11b_state, init_s11bnn, ROT0, "Williams", "Big Guns (L-A)",                               MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1987,  bguns_p1,       bguns_l8,   s11b,   s11b, s11b_state, init_s11bnn, ROT0, "Williams", "Big Guns (P-1)",                               MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989,  bk2k_l4,        0,          s11b,   s11b, s11b_state, init_s11bin, ROT0, "Williams", "Black Knight 2000 (L-4)",                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1989,  bk2k_lg1,       bk2k_l4,    s11b,   s11b, s11b_state, init_s11bin, ROT0, "Williams", "Black Knight 2000 (LG-1)",                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1989,  bk2k_lg3,       bk2k_l4,    s11b,   s11b, s11b_state, init_s11bin, ROT0, "Williams", "Black Knight 2000 (LG-3)",                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1989,  bk2k_pu1,       bk2k_l4,    s11b,   s11b, s11b_state, init_s11bin, ROT0, "Williams", "Black Knight 2000 (PU-1)",                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1989,  bk2k_pf1,       bk2k_l4,    s11b,   s11b, s11b_state, init_s11bin, ROT0, "Williams", "Black Knight 2000 (PF-1)",                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1989,  bk2k_la2,       bk2k_l4,    s11b,   s11b, s11b_state, init_s11bin, ROT0, "Williams", "Black Knight 2000 (LA-2)",                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1989,  bk2k_pa7,       bk2k_l4,    s11b,   s11b, s11b_state, init_s11bin, ROT0, "Williams", "Black Knight 2000 (PA-7)",                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1989,  bk2k_pa5,       bk2k_l4,    s11b,   s11b, s11b_state, init_s11bin, ROT0, "Williams", "Black Knight 2000 (PA-5)",                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1988,  cycln_l5,       0,          s11b,   s11b, s11b_state, init_s11bnn, ROT0, "Williams", "Cyclone (L-5)",                                MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988,  cycln_l4,       cycln_l5,   s11b,   s11b, s11b_state, init_s11bnn, ROT0, "Williams", "Cyclone (L-4)",                                MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988,  cycln_l1,       cycln_l5,   s11b,   s11b, s11b_state, init_s11bnn, ROT0, "Williams", "Cyclone (L-1)",                                MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988,  esha_la3,       0,          s11b,   s11b, s11b_state, init_s11bin, ROT0, "Williams", "Earthshaker (LA-3)",                           MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989,  esha_ma3,       esha_la3,   s11b,   s11b, s11b_state, init_s11bin, ROT0, "Williams", "Earthshaker (Metallica) (LA-3)",               MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989,  esha_pr4,       esha_la3,   s11b,   s11b, s11b_state, init_s11bin, ROT0, "Williams", "Earthshaker (Family version) (PR-4)",          MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988,  esha_lg1,       esha_la3,   s11b,   s11b, s11b_state, init_s11bin, ROT0, "Williams", "Earthshaker (German) (LG-1)",                  MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988,  esha_lg2,       esha_la3,   s11b,   s11b, s11b_state, init_s11bin, ROT0, "Williams", "Earthshaker (German) (LG-2)",                  MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988,  esha_la1,       esha_la3,   s11b,   s11b, s11b_state, init_s11bin, ROT0, "Williams", "Earthshaker (LA-1)",                           MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988,  esha_pa1,       esha_la3,   s11b,   s11b, s11b_state, init_s11bin, ROT0, "Williams", "Earthshaker (Prototype) (PA-1)",               MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988,  esha_pa4,       esha_la3,   s11b,   s11b, s11b_state, init_s11bin, ROT0, "Williams", "Earthshaker (Prototype) (PA-4)",               MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989,  eatpm_l4,       0,          s11b,   s11b, s11b_state, init_s11bin, ROT0, "Bally",    "Elvira and the Party Monsters (LA-4)",         MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989,  eatpm_l1,       eatpm_l4,   s11b,   s11b, s11b_state, init_s11bin, ROT0, "Bally",    "Elvira and the Party Monsters (LA-1)",         MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989,  eatpm_l2,       eatpm_l4,   s11b,   s11b, s11b_state, init_s11bin, ROT0, "Bally",    "Elvira and the Party Monsters (LA-2)",         MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989,  eatpm_4g,       eatpm_l4,   s11b,   s11b, s11b_state, init_s11bin, ROT0, "Bally",    "Elvira and the Party Monsters (LG-4)",         MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989,  eatpm_3g,       eatpm_l4,   s11b,   s11b, s11b_state, init_s11bin, ROT0, "Bally",    "Elvira and the Party Monsters (LG-3)",         MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989,  eatpm_4u,       eatpm_l4,   s11b,   s11b, s11b_state, init_s11bin, ROT0, "Bally",    "Elvira and the Party Monsters (LU-4)",         MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989,  eatpm_f1,       eatpm_l4,   s11b,   s11b, s11b_state, init_s11bin, ROT0, "Bally",    "Elvira and the Party Monsters (LF-1) French",  MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989,  eatpm_p7,       eatpm_l4,   s11b,   s11b, s11b_state, init_s11bin, ROT0, "Bally",    "Elvira and the Party Monsters (PA-7)",         MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989,  jokrz_l6,       0,     s11b_jokerz, s11b, s11b_state, init_s11bin, ROT0, "Williams", "Jokerz! (L-6)",                                MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989,  jokrz_l3,    jokrz_l6, s11b_jokerz, s11b, s11b_state, init_s11bin, ROT0, "Williams", "Jokerz! (L-3)",                                MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989,  jokrz_g4,    jokrz_l6, s11b_jokerz, s11b, s11b_state, init_s11bin, ROT0, "Williams", "Jokerz! (G-4)",                                MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989,  mousn_l4,       0,          s11b,   s11b, s11b_state, init_s11bin, ROT0, "Bally",    "Mousin' Around! (LA-4)",                       MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989,  mousn_l1,       mousn_l4,   s11b,   s11b, s11b_state, init_s11bin, ROT0, "Bally",    "Mousin' Around! (LA-1)",                       MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989,  mousn_lu,       mousn_l4,   s11b,   s11b, s11b_state, init_s11bin, ROT0, "Bally",    "Mousin' Around! (LU-1)",                       MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989,  mousn_lx,       mousn_l4,   s11b,   s11b, s11b_state, init_s11bin, ROT0, "Bally",    "Mousin' Around! (LX-1)",                       MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989,  polic_l4,       0,          s11b,   s11b, s11b_state, init_s11bi7, ROT0, "Williams", "Police Force (LA-4)",                          MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989,  polic_l3,       polic_l4,   s11b,   s11b, s11b_state, init_s11bi7, ROT0, "Williams", "Police Force (LA-3)",                          MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989,  polic_l2,       polic_l4,   s11b,   s11b, s11b_state, init_s11bi7, ROT0, "Williams", "Police Force (LA-2)",                          MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989,  polic_g4,       polic_l4,   s11b,   s11b, s11b_state, init_s11bi7, ROT0, "Williams", "Police Force (LG-4) Germany",                  MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988,  spstn_l5,       0,          s11b,   s11b, s11b_state, init_s11bnn, ROT0, "Williams", "Space Station (L-5)",                          MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988,  swrds_l2,       0,          s11b,   s11b, s11b_state, init_s11bn7, ROT0, "Williams", "Swords of Fury (L-2)",                         MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988,  swrds_lg2,      swrds_l2,   s11b,   s11b, s11b_state, init_s11bn7, ROT0, "Williams", "Swords of Fury (LG-2) Germany",                MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988,  swrds_l1,       swrds_l2,   s11b,   s11b, s11b_state, init_s11bn7, ROT0, "Williams", "Swords of Fury (L-1)",                         MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988,  taxi_l4,        0,          s11b,   s11b, s11b_state, init_s11bi7, ROT0, "Williams", "Taxi (Lola) (L-4)",                            MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988,  taxi_l3,        taxi_l4,    s11b,   s11b, s11b_state, init_s11bi7, ROT0, "Williams", "Taxi (Marilyn) (L-3)",                         MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988,  taxi_lu1,       taxi_l4,    s11b,   s11b, s11b_state, init_s11bi7, ROT0, "Williams", "Taxi (Marilyn) (LU-1)",                        MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988,  taxi_lg1,       taxi_l4,    s11b,   s11b, s11b_state, init_s11bi7, ROT0, "Williams", "Taxi (Marilyn) (L-1) Germany",                 MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1988,  taxi_p5,        taxi_l4,    s11b,   s11b, s11b_state, init_s11bi7, ROT0, "Williams", "Taxi (P-5)",                                   MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989,  tsptr_l3,       0,          s11b,   s11b, s11b_state, init_s11bin, ROT0, "Bally",    "Transporter the Rescue (L-3)",                 MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989,  tsptr_l1,       tsptr_l3,   s11b,   s11b, s11b_state, init_s11bin, ROT0, "Bally",    "Transporter the Rescue (LA-1)",                MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1990,  whirl_l3,       0,          s11b,   s11b, s11b_state, init_s11bin, ROT0, "Williams", "Whirlwind (LA-3)",                             MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1990,  whirl_l2,       whirl_l3,   s11b,   s11b, s11b_state, init_s11bin, ROT0, "Williams", "Whirlwind (LU-2)",                             MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1990,  whirl_lg3,      whirl_l3,   s11b,   s11b, s11b_state, init_s11bin, ROT0, "Williams", "Whirlwind (LG-3)",                             MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1990,  whirl_lg2,      whirl_l3,   s11b,   s11b, s11b_state, init_s11bin, ROT0, "Williams", "Whirlwind (LG-2)",                             MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1990,  whirl_lg1,      whirl_l3,   s11b,   s11b, s11b_state, init_s11bin, ROT0, "Williams", "Whirlwind (LG-1)",                             MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
