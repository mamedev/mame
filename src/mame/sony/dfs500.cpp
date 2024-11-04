// license:GPL-2.0+
// copyright-holders:Felipe Sanches
/****************************************************************************

    Sony DFS-500 DME Video Mixer

    An artwork layout for this is under development at:
    https://github.com/felipesanches/dfs500_MAME_artwork

    ===== Current status of this driver =====
    Upon power up, the ROM version number is displayed: "1.03"
    And after a while, we get an error: "Er02"

    This is likely due to bugs in this driver rendering the control panel unable to
    communicate properly with the video mixer maincpu via a serial interface.

    Apparently the maincpu does not respond to the control panel commands because it gets
    lost earlier, while trying to communicate with the secondary cpu (the "effects" cpu)
    via a pair of 16 bit latches.

    The effects CPU does not yet reply due to some other problem in the driver.
    I guess it is related to the "OREG2" in its memory map which have not yet been
    properly declared. We'll need to further study that portion of the circuit in the
    service manual in order to finish declaring the effects_cpu memory map layout.

    ===== Usage: =====
    According to the service manual, the LED test mode can be enabled by simultaneously
    pressing these buttons in the control panel:
    * FOREGROUND VTR A
    * BACKGROUND VTR B
    * Location
    With the default key mappings in this driver, this can be achieved by pressing Q + 2 + L

    * All LEDs and 7-seg displays blink sequentialy.
    * If you press any button on the control panel, the sequence of blinking resumes from
      that selected area of the panel.
    * To stop the blinking sequence you can press the keypad ENTER key.

    ===== Notes on the video hardware: =====
    This video mixer accepts 4 input video signals to be mixed. This driver currently emulates
    this by using static PNG images provided in the command line with the -inputN flags (with N = 1 to 4)

    The DSP circuitry was not yet emulated, and the minimal video code in this driver currently
    simply bypasses the input #1 into the output. Once the serial communication between control panel and
    maincpu is fixed, we should at least see the video output changing when pressing the BACKGROUND input
    selector buttons VTR A, VTR B, 3 and 4 (to select ammong the 4 video inputs).

****************************************************************************/
#include "emu.h"
#include "cpu/nec/nec.h"
#include "machine/gen_latch.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "machine/i8251.h"
#include "machine/upd7004.h"
#include "imagedev/picture.h"
#include "sound/beep.h"
#include "speaker.h"
#include "screen.h"
#include "dfs500.lh"


namespace {

#define VIDEO_WIDTH 768
#define VIDEO_HEIGHT 256

class dfs500_state : public driver_device
{
public:
	dfs500_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cpanelcpu(*this, "cpanelcpu")
		, m_maincpu(*this, "maincpu")
		, m_effectcpu(*this, "effectcpu")
		, m_pit(*this, "pit")
		, m_pic(*this, "pic")
		, m_serial1(*this, "serial1")
		, m_serial2(*this, "serial2")
		, m_cpanel_serial(*this, "cpanel_serial")
		, m_cpanel_pit(*this, "cpanel_pit")
		, m_adc(*this, "adc")
		, m_rombank1(*this, "rombank1")
		, m_rombank2(*this, "rombank2")
		, m_buzzer(*this, "buzzer")
		, m_screen(*this, "screen")
		, m_input(*this, "input%u", 1U)
		, m_DSW_S1(*this, "DSW_S1")
		, m_DSW_S2(*this, "DSW_S2")
		, m_DSW_S3(*this, "DSW_S3")
		, m_RD(*this, "RD%u", 0U)
		, m_transition(*this, "transition_%u", 0U)
		, m_7seg_status(*this, "status")
		, m_7seg_edit(*this, "edit")
		, m_7seg_trail_shadow_frames(*this, "trail_shadow_frames_%u", 0U)
		, m_7seg_snapshot(*this, "snapshot_%u", 0U)
		, m_7seg_trans_rate(*this, "trans_rate_%u", 0U)
		, m_7seg_pattern_number(*this, "pattern_number_%u", 0U)
		, m_LD(*this, "LD%u", 0U)
		, m_LD_effect_ctrl_shift(*this, "LD234")
		, m_LD_effect_ctrl_mask(*this, "LD235")
	{
	}

	void dfs500(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void cpanelcpu_mem_map(address_map &map) ATTR_COLD;
	void cpanelcpu_io_map(address_map &map) ATTR_COLD;
	void maincpu_mem_map(address_map &map) ATTR_COLD;
	[[maybe_unused]] void maincpu_io_map(address_map &map) ATTR_COLD;
	void effectcpu_mem_map(address_map &map) ATTR_COLD;
	uint8_t pit_r(offs_t offset);
	void pit_w(offs_t offset, uint8_t data);
	uint8_t cpanel_pit_r(offs_t offset);
	void cpanel_pit_w(offs_t offset, uint8_t data);
	uint8_t pic_r(offs_t offset);
	void pic_w(offs_t offset, uint8_t data);
	void rombank1_entry_w(offs_t offset, uint8_t data);
	void rombank2_entry_w(offs_t offset, uint8_t data);
	void input_select_w(offs_t offset, uint8_t data);
	uint16_t RA0_r(offs_t offset);
	uint16_t RB0_r(offs_t offset);
	void WA0_w(offs_t offset, uint16_t data);
	void WB0_w(offs_t offset, uint16_t data);
	uint16_t RA1_r(offs_t offset);
	uint8_t RB1_r(offs_t offset);
	uint8_t RB2_r(offs_t offset);
	uint8_t cpanel_reg0_r(offs_t offset);
	uint8_t cpanel_reg2_r(offs_t offset);
	void cpanel_reg0_w(offs_t offset, uint8_t data);
	void cpanel_reg1_w(offs_t offset, uint8_t data);
	void cpanel_reg2_w(offs_t offset, uint8_t data);
	uint8_t cpanel_buzzer_r(offs_t offset);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect);
	IRQ_CALLBACK_MEMBER(irq_callback);

	uint8_t m_int_vector;
	uint8_t m_sel10;
	uint8_t m_sel32;
	uint8_t m_sel54;
	uint8_t m_input_sel_A;
	uint8_t m_input_sel_B;
	uint16_t m_maincpu_latch16;
	uint16_t m_effectcpu_latch16;
	bool m_TOC;
	bool m_TOE;
	// TODO: bool m_BVS;
	uint8_t m_trans_rate[2];
	uint8_t m_pattern_number[3];
	uint8_t m_userprogram_status;
	uint8_t m_userprogram_edit;
	uint8_t m_trail_duration;
	uint8_t m_snapshot;
	uint8_t m_dot_points1;
	uint8_t m_dot_points2;
	bool m_buzzer_state;

	required_device<v20_device> m_cpanelcpu;
	required_device<v30_device> m_maincpu;
	required_device<v30_device> m_effectcpu;
	required_device<pit8254_device> m_pit;
	required_device<pic8259_device> m_pic;
	required_device<i8251_device> m_serial1;
	required_device<i8251_device> m_serial2;
	required_device<i8251_device> m_cpanel_serial;
	required_device<pit8254_device> m_cpanel_pit;
	required_device<upd7004_device> m_adc;
	required_memory_bank m_rombank1;
	required_memory_bank m_rombank2;
	required_device<beep_device> m_buzzer;
	required_device<screen_device> m_screen;
	required_device_array<picture_image_device, 4> m_input;
	required_ioport m_DSW_S1;
	required_ioport m_DSW_S2;
	required_ioport m_DSW_S3;
	optional_ioport_array<13> m_RD;
	output_finder<20> m_transition;
	output_finder<> m_7seg_status;
	output_finder<> m_7seg_edit;
	output_finder<2> m_7seg_trail_shadow_frames;
	output_finder<2> m_7seg_snapshot;
	output_finder<3> m_7seg_trans_rate;
	output_finder<4> m_7seg_pattern_number;
	output_finder<110> m_LD;
	output_finder<> m_LD_effect_ctrl_shift;
	output_finder<> m_LD_effect_ctrl_mask;
};

IRQ_CALLBACK_MEMBER(dfs500_state::irq_callback)
{
	return m_int_vector;
}

void dfs500_state::machine_start()
{
	m_serial1->write_cts(0);
	m_serial2->write_cts(0);
	m_cpanel_serial->write_cts(0);
	m_rombank1->configure_entries(0, 128, memregion("effectdata")->base(), 0x4000);
	m_rombank2->configure_entries(0, 128, memregion("effectdata")->base(), 0x4000);

	m_transition.resolve();
	m_7seg_status.resolve();
	m_7seg_edit.resolve();
	m_7seg_trail_shadow_frames.resolve();
	m_7seg_snapshot.resolve();
	m_7seg_trans_rate.resolve();
	m_7seg_pattern_number.resolve();
	m_LD.resolve();
	m_LD_effect_ctrl_shift.resolve();
	m_LD_effect_ctrl_mask.resolve();
}

void dfs500_state::machine_reset()
{
	m_buzzer_state = false;
	m_buzzer->set_state(0);
	m_rombank1->set_entry(0);
	m_rombank2->set_entry(0);
	m_maincpu_latch16 = 0x0000;
	m_effectcpu_latch16 = 0x0000;
	m_TOC = false;
	m_TOE = false;
	m_int_vector = 0x00;
	m_sel10 = 0;
	m_sel32 = 0;
	m_sel54 = 0;
}

uint32_t dfs500_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect)
{
	const bitmap_argb32 &input_bitmap = m_input[m_input_sel_A & 3]->get_bitmap();
	// FIXME: This is simply bypassing the inputs directly into the output.
	//        Emulation of the video hardware (DSP signal path) for GFX processing is still needed here.
	if (input_bitmap.valid())
	{
		// convert arbitrary sized ARGB32 image to a full-screen image
		double stepx = double (input_bitmap.width()) / VIDEO_WIDTH;
		double stepy = double (input_bitmap.height()) / VIDEO_HEIGHT;

		for (unsigned screen_y = screen.visible_area().min_y; screen_y <= screen.visible_area().max_y; screen_y++)
		{
			for (unsigned screen_x = screen.visible_area().min_x; screen_x <= screen.visible_area().max_x; screen_x++)
			{
				bitmap.pix(screen_y, screen_x) = input_bitmap.pix(
					int(double (screen_y % VIDEO_HEIGHT) * stepy),
					int(double (screen_x % VIDEO_WIDTH) * stepx));
			}
		}
	}
	return 0;
}

uint8_t dfs500_state::pit_r(offs_t offset)
{
	// CXQ71054P (Programmable Timer / Counter)
	return m_pit->read((offset >> 1) & 3); // addressed by CPU's address bits AB2 and AB1
}

void dfs500_state::pit_w(offs_t offset, uint8_t data)
{
	// CXQ71054P (Programmable Timer / Counter)
	m_pit->write((offset >> 1) & 3, data); // addressed by CPU's address bits AB2 and AB1
}

uint8_t dfs500_state::cpanel_pit_r(offs_t offset)
{
	// CXQ71054P (Programmable Timer / Counter)
	return m_cpanel_pit->read((offset >> 1) & 3); // addressed by CPU's address bits AB2 and AB1
}

void dfs500_state::cpanel_pit_w(offs_t offset, uint8_t data)
{
	// CXQ71054P (Programmable Timer / Counter)
	m_cpanel_pit->write((offset >> 1) & 3, data); // addressed by CPU's address bits AB2 and AB1
}

uint8_t dfs500_state::pic_r(offs_t offset)
{
	// PD71059C (Programmable Interrupt Controller)
	return m_pic->read((offset >> 1) & 1); // addressed by CPU's address bit AB1
}

void dfs500_state::pic_w(offs_t offset, uint8_t data)
{
	// PD71059C (Programmable Interrupt Controller)
	m_pic->write((offset >> 1) & 1, data); // addressed by CPU's address bit AB1
}

void dfs500_state::rombank1_entry_w(offs_t offset, uint8_t data)
{
	m_rombank1->set_entry(((data >> 1) & 0x40) | (data & 0x3f));
}

void dfs500_state::rombank2_entry_w(offs_t offset, uint8_t data)
{
	m_rombank2->set_entry(((data >> 1) & 0x40) | (data & 0x3f));
}

void dfs500_state::input_select_w(offs_t offset, uint8_t data)
{
	// Selects sources of video input on the AD-76 board.
	m_input_sel_A = (data >> 3) & 0x7;
	m_input_sel_B = data & 0x7;
}

void dfs500_state::WA0_w(offs_t offset, uint16_t data)
{
	m_effectcpu_latch16 = data;
	m_TOC = true;
}

uint16_t dfs500_state::RA0_r(offs_t offset)
{
	m_TOE = false;
	return m_maincpu_latch16;
}

uint16_t dfs500_state::RA1_r(offs_t offset)
{
	// "TEST, 1, OPT2, OPT1, RFLD, VD, TOC, TOE"
	uint8_t value = 0x40;
	// FIXME! Add other signals.
	if (m_TOC) value |= 0x02;
	if (m_TOE) value |= 0x01;
	value |= ((m_DSW_S3->read() & 0x0f) << 8); // "DSW_S3": (Unknown)
	return value;
}

void dfs500_state::WB0_w(offs_t offset, uint16_t data)
{
	m_maincpu_latch16 = data;
	m_TOE = true;
}

uint16_t dfs500_state::RB0_r(offs_t offset)
{
	m_TOC = false;
	return m_effectcpu_latch16;
}

uint8_t dfs500_state::RB1_r(offs_t offset)
{
	//"TEST, OPT2, OPT1, VD, T2, T1, TOE, TOC"
	uint8_t value = 0;
	// FIXME! Add other signals.
	if (m_TOE) value |= 0x02;
	if (m_TOC) value |= 0x01;
	return value;
}

uint8_t dfs500_state::RB2_r(offs_t offset)
{
	uint8_t value = 0;
	value |= ((m_DSW_S1->read() & 0x0f) << 4); // ("DSW_S1": Editing Control Unit Select)
	value |= (m_DSW_S2->read() & 0x0f);        // ("DSW_S2": Freeze Timing)
	// TODO:
	// if (m_BVS) value |= 0x10;
	return value;
}

uint8_t dfs500_state::cpanel_reg0_r(offs_t offset)
{
	uint8_t data;
	switch (offset & 0x0f)
	{
		case 0: // RD0
			data = 0x00;
			//TODO: if (m_RVD) data |= 0x02;
			//TODO: if (m_adc->eoc_r()) data |= 0x01;
			return data;
		case 6: // RD6
		case 7: // RD7
			return 0xff; //FIXME: Implement these ports
		default:
			if (m_RD[offset & 0x0f])
				return m_RD[offset & 0x0f]->read();
			else
				return 0xff;
	}
}

uint8_t dfs500_state::cpanel_reg2_r(offs_t offset)
{
	if ((offset & 0x07) > 2) return 0xff; // unused ports
	return m_RD[10 + (offset & 0x07)]->read(); // ports RD10, RD11 and RD12
}

void dfs500_state::cpanel_reg0_w(offs_t offset, uint8_t data)
{
	static const uint8_t ls247_map[16] =
		{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x58,0x4c,0x62,0x69,0x78,0x00 };

	switch (offset & 0x0f)
	{
		case 0: // WR0 on IC48 KY-223
			// Dot-points on 7seg digits:
			// D7: Not connected
			// D6: LD206 = DP on trans_rate_2
			// D5: LD198 = DP on trans_rate_1
			// D4: LD190 = DP on trans_rate_0
			// D3: LD182 = DP on pattern_number_1
			// D2: LD174 = DP on pattern_number_0
			// D1: LD186 or LD168 (?) = DP on pattern_number_1
			// D0: LD158 = DP on pattern_number_0
			m_dot_points2 = data;
			m_7seg_trans_rate[2] = ls247_map[m_trans_rate[1] & 0x0f] | (BIT(data >> 6, 0) << 7);
			m_7seg_trans_rate[1] = ls247_map[(m_trans_rate[0] >> 4) & 0x0f] | (BIT(data >> 5, 0) << 7);
			m_7seg_trans_rate[0] = ls247_map[m_trans_rate[0] & 0x0f] | (BIT(data >> 4, 0) << 7);
			m_7seg_pattern_number[3] = (m_pattern_number[2] & 0x7f) | (BIT(data >> 3, 0) << 7);
			m_7seg_pattern_number[2] = (m_pattern_number[1] & 0x7f) | (BIT(data >> 2, 0) << 7);
			m_7seg_pattern_number[1] = ls247_map[(m_pattern_number[0] >> 4) & 0x0f] | (BIT(data >> 1, 0) << 7);
			m_7seg_pattern_number[0] = ls247_map[m_pattern_number[0] & 0x0f] | (BIT(data >> 0, 0) << 7);
			break;
		case 1: // WR1 on IC50 KY-223
			m_pattern_number[2] = data;
			m_7seg_pattern_number[3] = (data & 0x7f) | (BIT(m_dot_points2 >> 3, 0) << 7);
			break;
		case 2: // WR2 on IC52 KY-223
			m_pattern_number[1] = data;
			m_7seg_pattern_number[2] = (data & 0x7f) | (BIT(m_dot_points2 >> 2, 0) << 7);
			break;
		case 3: // WR3 on IC53 KY-223
			m_pattern_number[0] = data;
			m_7seg_pattern_number[1] = ls247_map[(data >> 4) & 0x0f] | (BIT(m_dot_points2 >> 1, 0) << 7);
			m_7seg_pattern_number[0] = ls247_map[data & 0x0f] | (BIT(m_dot_points2 >> 0, 0) << 7);
			break;
		case 4: // WR4 on IC56 KY-223
			m_LD[63] = BIT(data, 7); // matte_copy
			m_LD[62] = BIT(data, 6); // mattes_col_bkgd
			m_LD[64] = BIT(data, 5); // pattern_number_set
			m_LD[65] = BIT(data, 4); // effect_ctrl_title
			m_LD[66] = BIT(data, 3); // effect_ctrl_dsk
			m_LD[69] = BIT(data, 2); // effect_ctrl_modify
			m_LD[68] = BIT(data, 1); // effect_ctrl_linear
			m_LD[67] = BIT(data, 0); // effect_ctrl_nonlin
			break;
		case 5: // WR5 on IC58 KY-223
			m_LD[61] = BIT(data, 7); // mattes_bord_mat
			m_LD[60] = BIT(data, 6); // mattes_shad_mat
			m_LD[59] = BIT(data, 5); // mattes_dsk_mat
			m_LD[58] = BIT(data, 4); // mattes_dsk_bord
			m_LD[57] = BIT(data, 3); // top_left
			m_LD[56] = BIT(data, 2); // top_right
			m_LD[55] = BIT(data, 1); // btm_left    // The service manual seems to be incorrect here.
			m_LD[54] = BIT(data, 0); // btm_right  // It seems to mistakenly swap LD54 and LD55.
			break;
		case 6: // WR6 on IC60 KY-223
			m_LD[53] = BIT(data, 6); // wide_bord
			m_LD[52] = BIT(data, 5); // narw_bord
			m_LD[51] = BIT(data, 4); // drop_bord
			m_LD[50] = BIT(data, 3); // double
			m_LD[48] = BIT(data, 2); // dsk_fill_video
			m_LD[47] = BIT(data, 1); // dsk_fill_mat
			m_LD[46] = BIT(data, 0); // dsk_fill_none
			break;
		case 7: // WR7 on IC82 KY-223
			m_LD[42] = BIT(data, 7); // dsk_mask_normal
			m_LD[44] = BIT(data, 6); // dsk_mask_invert
			m_LD[41] = BIT(data, 5); // dsk_key_inv
			m_LD[43] = BIT(data, 4); // dsk_ext_key
			m_LD[49] = BIT(data, 3); // border
			m_LD[40] = BIT(data, 2); // title_frgd_bus
			m_LD[39] = BIT(data, 1); // title_bord_mat
			m_LD[38] = BIT(data, 0); // title_shad_mat
			break;
		case 8: // WR8 on IC64 KY-223
			m_LD[78] = BIT(data, 7) << 1 | BIT(data, 3); // background_4
			m_LD[77] = BIT(data, 6) << 1 | BIT(data, 2); // background_3
			m_LD[76] = BIT(data, 5) << 1 | BIT(data, 1); // background_vtr_b
			m_LD[75] = BIT(data, 4) << 1 | BIT(data, 0); // background_vtr_a
			break;
		case 9: // WR9 on IC66 KY-223
			m_LD[73] = BIT(data, 7) << 1 | BIT(data, 3); // foreground_4
			m_LD[72] = BIT(data, 6) << 1 | BIT(data, 2); // foreground_3
			m_LD[71] = BIT(data, 5) << 1 | BIT(data, 1); // foreground_vtr_b
			m_LD[70] = BIT(data, 4) << 1 | BIT(data, 0); // foreground_vtr_a
			break;
		case 10: // WR10 on IC68 KY-223
			m_LD[34] = BIT(data, 7); // mask_normal
			m_LD[36] = BIT(data, 6); // mask_invert
			m_LD[33] = BIT(data, 5); // key_inv
			m_LD[35] = BIT(data, 4); // ext_key
			m_LD[37] = BIT(data, 3); // title
			m_LD[79] = BIT(data, 2) << 1 | BIT(data, 1); // background_int_video
			break;
		case 11: // WR11 on IC70 KY-223
			m_LD[82] = BIT(data, 7); // int_video_col_bkgd
			m_LD[81] = BIT(data, 6); // int_video_col_bar
			m_LD[80] = BIT(data, 5); // int_video_grid
			m_LD[74] = BIT(data, 4) << 1 | BIT(data, 3); // foreground_int_video
			m_LD[83] = BIT(data, 2); // freeze_field
			m_LD[84] = BIT(data, 1); // freeze_frame
			m_LD[88] = BIT(data, 0); // trans_rate_effect
			break;
		case 12: // WR12 on IC72 KY-223
			m_trans_rate[0] = data;
			m_7seg_trans_rate[1] = ls247_map[(data >> 4) & 0x0f] | (BIT(m_dot_points2 >> 5, 0) << 7);
			m_7seg_trans_rate[0] = ls247_map[data & 0x0f] | (BIT(m_dot_points2 >> 4, 0) << 7);
			break;
		case 13: // WR13 on IC75 KY-223
			m_trans_rate[0] = data & 0x0f;
			m_LD_effect_ctrl_shift = BIT(data, 5); // LD234
			m_LD_effect_ctrl_mask = BIT(data, 4); // LD235
			m_LD[45] = BIT(data, 7) << 1 | BIT(data, 6); // dsk_mix
			m_7seg_trans_rate[2] = (ls247_map[data & 0x0F]) | (BIT(m_dot_points2 >> 6, 0) << 7);
			break;
		case 14: // WR14 on IC77 KY-223
			m_LD[85] = BIT(data, 7); // transition_effect
			m_LD[86] = BIT(data, 6); // trans_rate_dsk
			m_LD[87] = BIT(data, 5); // transition_dsk
			m_LD[91] = BIT(data, 4); // transition_reverse
			m_transition[0] = BIT(data, 3); // LD233
			m_transition[1] = BIT(data, 2); // LD232
			m_transition[2] = BIT(data, 1); // LD231
			m_transition[3] = BIT(data, 0); // LD230
			break;
		case 15: // WR15 on IC79 KY-223
			m_transition[4] = BIT(data, 7); // LD229
			m_transition[5] = BIT(data, 6); // LD228
			m_transition[6] = BIT(data, 5); // LD227
			m_transition[7] = BIT(data, 4); // LD226
			m_transition[8] = BIT(data, 3); // LD225
			m_transition[9] = BIT(data, 2); // LD224
			m_transition[10] = BIT(data, 1); // LD223
			m_transition[11] = BIT(data, 0); // LD222
			break;
	}
}


void dfs500_state::cpanel_reg1_w(offs_t offset, uint8_t data)
{
	switch (offset & 7)
	{
		case 0: // WR16 on IC81 KY-223
			m_transition[12] = BIT(data, 7); // LD221
			m_transition[13] = BIT(data, 6); // LD220
			m_transition[14] = BIT(data, 5); // LD219
			m_transition[15] = BIT(data, 4); // LD218
			m_transition[16] = BIT(data, 3); // LD217
			m_transition[17] = BIT(data, 2); // LD216
			m_transition[18] = BIT(data, 1); // LD215
			m_transition[19] = BIT(data, 0); // LD214
			break;
		case 1: // WR17 on IC83 KY-223
			m_LD[90] = BIT(data, 7); // trans_rate_norm_rev
			m_LD[89] = BIT(data, 6); // transition_auto_trans
			m_LD[96] = BIT(data, 5); // direct_pattern
			m_LD[97] = BIT(data, 4); // keypad_0
			m_LD[98] = BIT(data, 3); // keypad_1
			m_LD[99] = BIT(data, 2); // keypad_2
			m_LD[100] = BIT(data, 1); // keypad_3
			m_LD[101] = BIT(data, 0); // keypad_rst
			break;
		case 2: // WR18 on IC81 KY-223
			m_LD[102] = BIT(data, 7); // keypad_4
			m_LD[103] = BIT(data, 6); // keypad_5
			m_LD[104] = BIT(data, 5); // keypad_6
			m_LD[105] = BIT(data, 4); // keypad_del
			m_LD[106] = BIT(data, 3); // keypad_7
			m_LD[107] = BIT(data, 2); // keypad_8
			m_LD[108] = BIT(data, 1); // keypad_9
			m_LD[109] = BIT(data, 0); // keypad_ins
			break;
		case 3: // WR19 on IC87 KY-223
			m_LD[95] = BIT(data, 7); // mode_pattern
			m_LD[94] = BIT(data, 6); // mode_trans
			m_LD[93] = BIT(data, 5); // mode_user_pgm
			m_LD[92] = BIT(data, 4); // mode_snap_shot

			//Selectors for analog inputs to ADC:
			m_sel10 = data & 0x03;
			m_sel32 = (data >> 2) & 0x03;
			break;
		case 7: // WR20 on IC47 KY-223
			// Here "data" holds the 8-bit value of the "interrupt vector number"
						//  to be put on the data bus when the CPU asserts /INTAK (Interrupt Acknowledge)
			m_int_vector = data;
			// FIXME: Is this an alternative way of doing it?
			//        m_cpanelcpu->set_input_line_and_vector(0, HOLD_LINE, data);
			break;
		default:
			break;
	}
}

void dfs500_state::cpanel_reg2_w(offs_t offset, uint8_t data)
{
	static const uint8_t ls247_map[16] =
		{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x58,0x4c,0x62,0x69,0x78,0x00 };

	switch (offset&7)
	{
		case 0: // WR21 on IC7 KY-225
			m_snapshot = data;
			m_7seg_snapshot[1] = ls247_map[(data >> 4) & 0x0F] | (BIT(m_dot_points1 >> 5, 0) << 7);
			m_7seg_snapshot[0] = ls247_map[data & 0x0F] | (BIT(m_dot_points1 >> 4, 0) << 7);
			break;
		case 1: // WR22 on IC10 KY-225
			m_LD[1] = BIT(data, 7); // editor_enable
			m_LD[4] = BIT(data, 6); // learn
			m_LD[3] = BIT(data, 5); // recall
			m_LD[2] = BIT(data, 4); // hold_input
			m_LD[5] = BIT(data, 3); // lighting
			m_LD[8] = BIT(data, 2); // lighting_spot
			m_LD[7] = BIT(data, 1); // lighting_line
			m_LD[6] = BIT(data, 0); // lighting_plane
			break;
		case 2: // WR23 on IC12 KY-225
			m_LD[15] = BIT(data, 7); // trail
			m_LD[20] = BIT(data, 6); // drop_border
			m_LD[11] = BIT(data, 5); // lighting_width_wide
			m_LD[10] = BIT(data, 4); // lighting_width_medium
			m_LD[9] = BIT(data, 3); // lighting_width_narrow
			m_LD[14] = BIT(data, 2); // lighting_intensity_high
			m_LD[13] = BIT(data, 1); // lighting_intensity_medium
			m_LD[12] = BIT(data, 0); // lighting_intensity_low
			break;
		case 3: // WR24 on IC14 KY-225
			m_LD[19] = BIT(data, 7); // trail_drop_type_hard
			m_LD[18] = BIT(data, 6); // trail_drop_type_soft
			m_LD[17] = BIT(data, 5); // trail_drop_type_hard_star
			m_LD[16] = BIT(data, 4); // trail_drop_type_soft_star
			m_LD[24] = BIT(data, 3); // trail_drop_fill_self
			m_LD[23] = BIT(data, 2); // trail_drop_fill_bord_mat
			m_LD[22] = BIT(data, 1); // trail_drop_fill_shad_mat
			m_LD[21] = BIT(data, 0); // trail_drop_fill_rndm_mat
			break;
		case 4: // WR25 on IC18 KY-225
			m_trail_duration = data;
			m_7seg_trail_shadow_frames[1] = ls247_map[(data >> 4) & 0x0F] | (BIT(m_dot_points1 >> 3, 0) << 7);
			m_7seg_trail_shadow_frames[0] = ls247_map[data & 0x0F] | (BIT(m_dot_points1 >> 2, 0) << 7);
			break;
		case 5: // WR26 on IC16 KY-225
			m_LD[25] = BIT(data, 7); // shadow
			m_LD[28] = BIT(data, 6); // trail_frames_duration
			m_LD[27] = BIT(data, 5); // trail_frames_wid_pos
			m_LD[26] = BIT(data, 4); // trail_frames_density
			m_LD[29] = BIT(data, 3); // edge_border
			m_LD[30] = BIT(data, 2); // edge_soft
			m_LD[31] = BIT(data, 1); // edit_led
			m_LD[32] = BIT(data, 0); // location
			break;
		case 6: // WR27 on IC21 KY-225
			m_sel54 = (data >> 6) & 3; // D7/D6 = SEL5/SEL4 signals to IC26
			// Dot-points on 7seg digits:
			// D5 = LD150 = DP on user program status
			// D4 = LD142 = DP on user program edit
			// D3 = LD134 = DP on trail_duration_1
			// D2 = LD126 = DP on trail_duration_0
			// D1 = LD118 = DP on snap_shot_1
			// D0 = LD110 = DP on snap_shot_0
			m_dot_points1 = data;
			m_7seg_status = ls247_map[m_userprogram_status] | (BIT(data >> 5, 0) << 7);
			m_7seg_edit = ls247_map[m_userprogram_edit] | (BIT(data >> 4, 0) << 7);
			m_7seg_trail_shadow_frames[1] = ls247_map[(m_trail_duration >> 4) & 0x0F] | (BIT(data >> 3, 0) << 7);
			m_7seg_trail_shadow_frames[0] = ls247_map[m_trail_duration & 0x0F] | (BIT(data >> 2, 0) << 7);
			m_7seg_snapshot[1] = ls247_map[(m_snapshot >> 4) & 0x0F] | (BIT(data >> 1, 0) << 7);
			m_7seg_snapshot[0] = ls247_map[m_snapshot & 0x0F] | (BIT(data >> 0, 0) << 7);
			break;
		case 7: // WR28 on IC23 KY-225
			m_userprogram_status = (data >> 4) & 0x0F;
			m_userprogram_edit = data & 0x0F;
			m_7seg_status = ls247_map[(data >> 4) & 0x0F] | (BIT(m_dot_points1 >> 5, 0) << 7);
			m_7seg_edit = ls247_map[data & 0x0F] | (BIT(m_dot_points1 >> 4, 0) << 7);
			break;
	}
}

uint8_t dfs500_state::cpanel_buzzer_r(offs_t offset)
{
// FIXME: Not sure yet what to do here...
// TODO:    m_buzzer_state = !m_buzzer_state;
// TODO:    m_buzzer->set_state(m_buzzer_state);
	return 0;
}

void dfs500_state::cpanelcpu_mem_map(address_map &map)
{
	map(0x00000, 0x07fff).mirror(0xe0000).ram();                        // 32kb SRAM chip at IC15 on KY-223
	map(0x08000, 0x08000).mirror(0xe0ffd).rw(m_cpanel_serial, FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));       // "IO" IC17 on KY-223
	map(0x08002, 0x08002).mirror(0xe0ffd).rw(m_cpanel_serial, FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0x09000, 0x09007).mirror(0xe0ff8).rw(FUNC(dfs500_state::cpanel_pit_r), FUNC(dfs500_state::cpanel_pit_w));       // "TIMER" IC16 on KY-223
	map(0x0a000, 0x0a001).mirror(0xe0ffe).rw("adc", FUNC(upd7004_device::read), FUNC(upd7004_device::write));       // ADC at IC19 on KY-223
	map(0x0b000, 0x0b00f).mirror(0xe0ff0).rw(FUNC(dfs500_state::cpanel_reg0_r), FUNC(dfs500_state::cpanel_reg0_w));       // "REG0" Switches(?) & LEDs on KY-223
	map(0x0c000, 0x0c007).mirror(0xe0ff8).w(FUNC(dfs500_state::cpanel_reg1_w));       // "REG1" LEDs on KY-223
	map(0x0d000, 0x0d007).mirror(0xe0ff8).rw(FUNC(dfs500_state::cpanel_reg2_r), FUNC(dfs500_state::cpanel_reg2_w));       // "REG2" Switches(?) & LEDs on KY-225
	map(0x0f000, 0x0f007).mirror(0xe0ff8).r(FUNC(dfs500_state::cpanel_buzzer_r)); // "BUZZER" IC89
	map(0x10000, 0x17fff).mirror(0xe8000).rom().region("cpanelcpu", 0); // 32kb EPROM at IC14
}

void dfs500_state::cpanelcpu_io_map(address_map &map)
{
	//FIXME! map(0x0000, 0x0007).mirror(0x8ff8).rw(FUNC(dfs500_state::...), FUNC(dfs500_state::...));
}

void dfs500_state::maincpu_mem_map(address_map &map)
{
	//FIXME: The RAM mirror should be 0xe0000 according to IC49 on board SY-172 (as it is wired on the service manual schematics)
	//       but I saw unmapped accesses to the A15 mirror of this range on the MAME debugger, which suggests the 0xe8000 value used below:
	map(0x00000, 0x07fff).mirror(0xe8000).ram();                      // 4x 8kb SRAM chips at IC59/IC60/IC61/IC62
	map(0x10000, 0x1ffff).mirror(0xe0000).rom().region("maincpu", 0); // 2x 32kb EPROMs at IC1/IC2
}

void dfs500_state::maincpu_io_map(address_map &map)
{
	map(0x0000, 0x0007).mirror(0x8ff8).rw(FUNC(dfs500_state::pit_r), FUNC(dfs500_state::pit_w));                    // "IO1" IC51
	map(0x1000, 0x1001).mirror(0x8ffc).rw(FUNC(dfs500_state::pic_r), FUNC(dfs500_state::pic_w));                    // "IO2" IC52
	map(0x2000, 0x2000).mirror(0x8ffd).rw(m_serial1, FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));       // "IO3" IC53
	map(0x2002, 0x2002).mirror(0x8ffd).rw(m_serial1, FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0x3000, 0x3000).mirror(0x8ffd).rw(m_serial2, FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));       // "IO4" IC54
	map(0x3002, 0x3002).mirror(0x8ffd).rw(m_serial2, FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0x4000, 0x4001).mirror(0x8ff0).r(FUNC(dfs500_state::RB0_r));                                                // "RB0" IC32/IC33
	map(0x4002, 0x4003).mirror(0x8ff0).r(FUNC(dfs500_state::RB1_r));                                                // "RB1" IC55
	map(0x4004, 0x4005).mirror(0x8ff0).r(FUNC(dfs500_state::RB2_r));                                                // "RB2" IC56
	map(0x5000, 0x5001).mirror(0x8ff0).w(FUNC(dfs500_state::WB0_w));                                                // "WB0" IC26/IC27
}

void dfs500_state::effectcpu_mem_map(address_map &map)
{
	// Note: As far as I can tell by the schematics in the service manual, the ram mirror should be 0x90000.
	//       But I see unmapped read acesses to 0x4800, which induces me to make the mirror 0x94000, instead.
	//   FIXME: This should be double-checked!
	map(0x00000, 0x03fff).mirror(0x94000).ram();                        // 2x 8kb SRAM chips at IC23/IC24
	map(0x08000, 0x0bfff).mirror(0x90000).bankr("rombank1");            // Effect data on 4x 512kb EPROMs at IC5/IC6/IC7/IC8
	map(0x0c000, 0x0ffff).mirror(0x90000).bankr("rombank2");            // Second banked view of the contents of the same effect data EPROMs
	map(0x20000, 0x20001).mirror(0x9fffe).noprw(); // FIXME: Do something with the MTRX signal generated by this memory access
	map(0x46000, 0x46fff).mirror(0x91000).ram().share("sgram"); // IC80/IC81 at FM-29 (4/6)
																// Selected by IC201 at FM-29 (3/6)
	map(0x68000, 0x68001).mirror(0x907fe).rw(FUNC(dfs500_state::RA0_r), FUNC(dfs500_state::WA0_w)); // "RA0" IC26/IC27 & "WA0" IC32/IC33
																									// 16-bit data latches for communication between CPUs
	map(0x68800, 0x68801).mirror(0x907fe).r(FUNC(dfs500_state::RA1_r));            // "RA1" IC25/IC64
	map(0x69000, 0x69000).mirror(0x907ff).w(FUNC(dfs500_state::rombank2_entry_w)); // "WA2" IC29
	map(0x69800, 0x69800).mirror(0x907ff).w(FUNC(dfs500_state::rombank1_entry_w)); // "WA3" IC30
	map(0x6a000, 0x6a000).mirror(0x907ff).w(FUNC(dfs500_state::input_select_w));   // "WA4" IC31
	map(0x70000, 0x7ffff).mirror(0x80000).rom().region("effectcpu", 0); // 2x 64kb EPROMs at IC3/IC4,
																		// Note: the 1st half of each is entirely made of 0xFF

	// ==== "ORG1" registers ====
	// "controlsignals" (Not sure yet of their actual use)
	// D13: PS2
	// D12: PS1
	// D11: FGS1
	// D10: FGS0
	// D9: BGS1
	// D8: BGS0
	// D7: TKON
	// D6: TKCONT
	// D5: AM2
	// D4: AM1
	// D3: AM0
	// D2: BM2
	// D1: BM1
	// D0: BM0
	map(0x6b078, 0x6b079).mirror(0x90700).w("controlsignals", FUNC(generic_latch_16_device::write));

	// "Reg7_5":
	//  Not sure yet of their actual use...
	map(0x6b07a, 0x6b07b).mirror(0x90700).w("reg7_5", FUNC(generic_latch_16_device::write));

	// XFLT: Feeds into chips IC107 (CKD8070K "Horizontal Variable Filter") and IC108 (CXD8276Q "CMOS Linear Interpolation")
	//        at PCB FM-29 (6/6); Foreground Bus Digital Lowpass Filter
	map(0x6b07c, 0x6b07d).mirror(0x90700).w("xflt", FUNC(generic_latch_16_device::write));

	// YFLT: Feeds into chips IC114 (CKD8263Q "Vertical Variable Filter") and IC115 (CXD8276Q "CMOS Linear Interpolation")
	//        at PCB FM-29 (6/6); Foreground Bus Digital Lowpass Filter
	map(0x6b07e, 0x6b07f).mirror(0x90700).w("yflt", FUNC(generic_latch_16_device::write));

	// ==== "ORG2" registers: ====
	// VE,6-8B/DA,2-23B
	//map(0x6B800, 0x6B800).mirror(0x907ff).w(...);
	map(0xc0000, 0xdffff).nopw(); // FIXME! Temporarily quieting unmapped access log messages on this range...

	// TODO: CKD8263Q: "Color Bar Pattern Generator"

	// ==== Under development ====
	// Here are some temporary notes on the bit-patterns for decoding the addresses
	// of portions of the circuit related to the remainder of this memory map
	//
	// IC73:
	// BA15..12 = 0110 MEMPRG 0x06000

	// REGA: AA18=0 AA17=0 AA15=0 AA14=1
	// !REGA: AA18=1 AA17=1 AA15=1 AA14=0
	// WRA:  /WR
	// WAn:  !REGA & !WRA & n=AA13/12/11
	// WAn:  ?11? 10nn n??? ???? ????

	// ARAM = AA18/17=10
	// ARAMW = ?
	// MTRX = ?
	// OREG1 = REGA=0 (AA18...14=00x01) AA13..11=110 => x00x 0111 0xxx xxxx xxxx => map(0x07000, 0x07001).mirror(0x90000)
	// OREG2 = REGA=0 (AA18...14=00x01) AA13..11=111 => x00x 0111 1xxx xxxx xxxx => map(0x07800, 0x07801).mirror(0x90000)
	// OBUS = active-low "ARAM or MTR or OREG1 or OREG2"
}

static INPUT_PORTS_START(dfs500)
	PORT_START("DSW_S1")
	PORT_DIPNAME( 0x0f, 0x02, "Editing Control Unit Select" )   PORT_DIPLOCATION("S1:4,3,2,1")
	PORT_DIPSETTING(    0x08, "BVE-600" )
	PORT_DIPSETTING(    0x04, "ONE-GPI" )
	PORT_DIPSETTING(    0x02, "BVE-900" )
	PORT_DIPSETTING(    0x01, "BVS-300" )

	PORT_START("DSW_S2")
	PORT_DIPNAME( 0x0f, 0x07, "Freeze Timing" )   PORT_DIPLOCATION("S2:4,3,2,1")
	PORT_DIPSETTING(    0x07, "8" )
	PORT_DIPSETTING(    0x0b, "4" )
	PORT_DIPSETTING(    0x0d, "2" )
	PORT_DIPSETTING(    0x0e, "1" )

	PORT_START("DSW_S3")
	PORT_DIPNAME( 0x08, 0x08, "Field freeze" )   PORT_DIPLOCATION("S3:4")
	PORT_DIPSETTING(    0x00, "Odd Field" )
	PORT_DIPSETTING(    0x08, "Even Freeze" )
	PORT_DIPNAME( 0x04, 0x04, "Color-Matte Compensation" )   PORT_DIPLOCATION("S3:3")
	PORT_DIPSETTING(    0x00, "Illegal compensation" )
	PORT_DIPSETTING(    0x04, "Limit compensation" )
	PORT_DIPNAME( 0x02, 0x02, "Set up" )   PORT_DIPLOCATION("S3:2")
	PORT_DIPSETTING(    0x00, "7.5%" )
	PORT_DIPSETTING(    0x02, "0%" )
	PORT_DIPNAME( 0x01, 0x00, "Freeze (When changing the crosspoint)" )   PORT_DIPLOCATION("S3:1")
	PORT_DIPSETTING(    0x00, "2 Frames" )
	PORT_DIPSETTING(    0x01, "0 Frame" )

	PORT_START("RD1")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("EFFECT CONTROL: SHIFT") // SW74
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("DSK BORDER") // SW32
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("MATTES/BKGD: SELECT") // SW35
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("MATTES/BKGD: MATTE COPY") // SW36
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("PATTERN NUMBER: SET") // SW37
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("EFFECT CONTROL: TITLE") // SW38
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("EFFECT CONTROL: DSK") // SW39

	PORT_START("RD2")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("DSK KEY INV") // SW26
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("DSK EXT KEY") // SW28
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("DSK NORMAL") // SW27
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("DSK INVERT") // SW29
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("DOWNSTREAM KEYER: FILL") // SW31
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("DSK MIX") // SW30
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("DOWNSTREAM KEYER: TYPE") // SW33
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("DOWNSTREAM KEYER: POSITION") // SW34

	PORT_START("RD3")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("BKGD BUS INT VIDEO") PORT_CODE(KEYCODE_5) // SW49
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("FRGD BUS INT VIDEO") PORT_CODE(KEYCODE_T) // SW44
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("KEY INV") // SW20
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("EXT KEY") // SW22
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("TITLE: FILL") // SW25
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("TITLE") // SW24
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("NORMAL") // SW21
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("INVERT") // SW23

	PORT_START("RD4")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("BKGD BUS 4") PORT_CODE(KEYCODE_4) // SW48
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("BKGD BUS 3") PORT_CODE(KEYCODE_3) // SW47
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("BKGD BUS 2") PORT_CODE(KEYCODE_2) // SW46
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("BKGD BUS 1") PORT_CODE(KEYCODE_1) // SW45
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("FRGD BUS 4") PORT_CODE(KEYCODE_R) // SW43
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("FRGD BUS 3") PORT_CODE(KEYCODE_E) // SW42
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("FRGD BUS 2") PORT_CODE(KEYCODE_W) // SW41
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("FRGD BUS 1") PORT_CODE(KEYCODE_Q) // SW40

	PORT_START("RD5")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("KEYPAD: DIRECT") // SW57
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("EFFECT TRANSITION: REVERSE") // SW56
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("EFFECT TRANSITION: AUTO TRANS") // SW55
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("TRANS RATE: EFFECT") // SW53
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("TRANS RATE: DSK") // SW54
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("FREEZE FIELD") // SW51
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("FREEZE FRAME") // SW52
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("INT VIDEO SELECT") PORT_CODE(KEYCODE_I)// SW50

	PORT_START("RD8")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("KEYPAD 0") PORT_CODE(KEYCODE_0_PAD) // SW58
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("KEYPAD DOWN") PORT_CODE(KEYCODE_DOWN) // SW59
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("KEYPAD UP") PORT_CODE(KEYCODE_UP) // SW60
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("KEYPAD ENTER") PORT_CODE(KEYCODE_ENTER_PAD) // SW61
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("KEYPAD 1") PORT_CODE(KEYCODE_1_PAD) // SW62
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("KEYPAD 2") PORT_CODE(KEYCODE_2_PAD) // SW63
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("KEYPAD 3") PORT_CODE(KEYCODE_3_PAD) // SW64
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("KEYPAD RST") // SW65

	PORT_START("RD9")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("KEYPAD 4") PORT_CODE(KEYCODE_4_PAD) // SW66
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("KEYPAD 5") PORT_CODE(KEYCODE_5_PAD) // SW67
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("KEYPAD 6") PORT_CODE(KEYCODE_6_PAD) // SW68
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("KEYPAD DEL") PORT_CODE(KEYCODE_DEL)// SW69
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("KEYPAD 7") PORT_CODE(KEYCODE_7_PAD) // SW70
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("KEYPAD 8") PORT_CODE(KEYCODE_8_PAD) // SW71
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("KEYPAD 9") PORT_CODE(KEYCODE_9_PAD) // SW72
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("KEYPAD INS") PORT_CODE(KEYCODE_INSERT)// SW73

	PORT_START("RD10")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("LIGHTING INTENSITY") // SW8
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("LIGHTING WIDTH") // SW7
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("LIGHTING TYPE") // SW6
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("LIGHTING") // SW5
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("LEARN") // SW4
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("RECALL") // SW3
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("HOLD INPUT") // SW2
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("EDITOR ENABLE") // SW1

	PORT_START("RD11")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("EDGE SOFT") // SW16
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("EDGE BORDER") // SW15
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("TRAIL/SHADOW DENSITY/POSITION") // SW14
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("SHADOW") // SW13
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("TRAIL/SHADOW FILL") // SW12
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("DROP BORDER") // SW11
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("TRAIL/SHADOW TYPE") // SW10
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("TRAIL") // SW9

	PORT_START("RD12")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("USER PGM LOCATION") PORT_CODE(KEYCODE_L) // SW19
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("USER PGM EDIT") // SW17
INPUT_PORTS_END

void dfs500_state::dfs500(machine_config &config)
{
	/******************* Control Panel ******************************/
	// NEC D70108C-8 at IC10 (a CPU compatible with Intel 8088)
	V20(config, m_cpanelcpu, 8_MHz_XTAL);
	m_cpanelcpu->set_addrmap(AS_PROGRAM, &dfs500_state::cpanelcpu_mem_map);
	m_cpanelcpu->set_addrmap(AS_IO, &dfs500_state::cpanelcpu_io_map);
	m_cpanelcpu->set_irq_acknowledge_callback(FUNC(dfs500_state::irq_callback));

	// CXQ71054P at IC16 (Programmable Timer / Counter)
	PIT8254(config, m_cpanel_pit, 0);
	m_cpanel_pit->set_clk<0>(8_MHz_XTAL);
	m_cpanel_pit->set_clk<1>(8_MHz_XTAL/2);
	m_cpanel_pit->out_handler<1>().set(m_cpanel_pit, FUNC(pit8254_device::write_clk2));
	m_cpanel_pit->out_handler<0>().set(m_cpanel_serial, FUNC(i8251_device::write_txc));
	m_cpanel_pit->out_handler<0>().append(m_cpanel_serial, FUNC(i8251_device::write_rxc));

	// CXQ71051P at IC17 (Serial Interface Unit)
	I8251(config, m_cpanel_serial, 8_MHz_XTAL/2);
	m_cpanel_serial->txd_handler().set(m_serial1, FUNC(i8251_device::write_rxd));
	m_cpanel_serial->rxrdy_handler().set_inputline(m_cpanelcpu, 0);

	UPD7004(config, m_adc, 8_MHz_XTAL/2);
	// FIXME! m_adc->eoc_ff_callback(). [...] Set bit D0 on register RD0 IC35 KY223
	// TODO: m_adc->in_callback<7>().set_ioport("XCOM");
	// TODO: m_adc->in_callback<6>().set_ioport("YCOM");
	// TODO: m_adc->in_callback<5>().set_ioport("XCOM1");
	// TODO: m_adc->in_callback<4>().set_ioport("YCOM1");
	// TODO: m_adc->in_callback<3>().set_ioport("XCOM0");
	// TODO: m_adc->in_callback<2>().set_ioport("YCOM0");
	// TODO: m_adc->in_callback<1>().set_ioport("TCLIP2");
	// TODO: m_adc->in_callback<0>().set_ioport("TCLIP1");

	//Buzzer
	SPEAKER(config, "mono").front_center();
	BEEP(config, "buzzer", 4000).add_route(ALL_OUTPUTS, "mono", 0.05); // incorrect/arbitrary freq.
																	   // I did not calculate the correct one yet.

	/******************* Effects Processing Unit ********************/
	// CXQ70116P-10 at IC40 (same as V20, but with a 16-bit data bus)
	V30(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &dfs500_state::maincpu_mem_map);
	m_maincpu->set_irq_acknowledge_callback("pic", FUNC(pic8259_device::inta_cb));

	// CXQ70116P-10 at IC9
	V30(config, m_effectcpu, 8_MHz_XTAL);
	m_effectcpu->set_addrmap(AS_PROGRAM, &dfs500_state::effectcpu_mem_map);

	// CXQ71054P at IC51 (Programmable Timer / Counter)
	PIT8254(config, m_pit, 0);
	m_pit->set_clk<0>(8_MHz_XTAL);
	m_pit->set_clk<1>(8_MHz_XTAL);
	m_pit->out_handler<1>().set(m_pit, FUNC(pit8254_device::write_clk2));
	m_pit->out_handler<0>().set(m_serial1, FUNC(i8251_device::write_txc));
	m_pit->out_handler<0>().append(m_serial1, FUNC(i8251_device::write_rxc));
	m_pit->out_handler<0>().append(m_serial2, FUNC(i8251_device::write_txc));
	m_pit->out_handler<0>().append(m_serial2, FUNC(i8251_device::write_rxc));

	// NEC D71059C at IC52 (Programmable Interruption Controller)
	PIC8259(config, m_pic, 0);
	m_pic->out_int_callback().set_inputline(m_maincpu, 0);

	// CXQ71051P at IC53 (Serial Interface Unit)
	I8251(config, m_serial1, 8_MHz_XTAL);
	m_serial1->txd_handler().set(m_cpanel_serial, FUNC(i8251_device::write_rxd));
	m_serial1->txrdy_handler().set(m_pic, FUNC(pic8259_device::ir7_w));

	// CXQ71051P at IC54 (Serial Interface Unit)
	I8251(config, m_serial2, 8_MHz_XTAL);
	m_serial2->txrdy_handler().set(m_pic, FUNC(pic8259_device::ir6_w));
	// FIXME: Declare an interface to hook this up to another emulated device in MAME (such as pve500)
	//
	// This goes to the CN21 EDITOR D-SUB CONNECTOR on board CN-573
	// I think the purpose of this is to connect to an editor such as the Sony PVE-500.
	//
	// m_serial2->txd_handler().set(m_..., FUNC(..._device::write_txd)); "XMIT"
	// m_...->rxd_handler().set(m_serial2, FUNC(i8251_device::write_rxd)); "RCV"

	GENERIC_LATCH_16(config, "controlsignals");
	GENERIC_LATCH_16(config, "reg7_5");
	GENERIC_LATCH_16(config, "xflt");
	GENERIC_LATCH_16(config, "yflt");

	// In the future this could become IMAGE_AVIVIDEO (or even, perhaps, we
	// should add support for capturing frames from real video devices such
	// as a webcam on /dev/video0)
	IMAGE_PICTURE(config, m_input[0]);
	IMAGE_PICTURE(config, m_input[1]);
	IMAGE_PICTURE(config, m_input[2]);
	IMAGE_PICTURE(config, m_input[3]);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(VIDEO_WIDTH, VIDEO_HEIGHT);
	m_screen->set_visarea(0, VIDEO_WIDTH-1, 0, VIDEO_HEIGHT-1);
	m_screen->set_screen_update(FUNC(dfs500_state::screen_update));

	config.set_default_layout(layout_dfs500);
}

ROM_START(dfs500)
	// Process Unit System Control:
	ROM_REGION(0x8000, "cpanelcpu", 0)
	ROM_LOAD("27c256b_npky14_v1.03_293-83_5500_ky223_sony94.ic14", 0x0000, 0x8000, CRC(8b9e564a) SHA1(aa8a1f211a7834fb15f7ecbc58570f566c0ef5ab))

	// Process Unit System Control:
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD16_BYTE("27c256b_npsys1_v1.03_293-84_3eb5_sy172_sony94.ic1", 0x0001, 0x8000, CRC(4604e7c0) SHA1(80f965b69a163a6278d6f54db741f4c5ada1cb59))
	ROM_LOAD16_BYTE("27c256b_npsys2_v1.03_293-85_3ecd_sy172_sony94.ic2", 0x0000, 0x8000, CRC(b80a66e6) SHA1(407ddc5fee61920bfbe90c20faf4482ceef1ad4f))

	// Process Unit Effect Control:
	ROM_REGION(0x10000, "effectcpu", 0)
	ROM_LOAD16_BYTE("27c512_npsys3_v1.04_293-86_b7d0_sy172_sony94.ic3", 0x0001, 0x8000, CRC(69238d02) SHA1(288babc7547858a3ca3f65af0be76f72335392ea))
	ROM_CONTINUE(0x0001, 0x8000)
	ROM_LOAD16_BYTE("27c512_npsys4_v1.04_293-87_b771_sy172_sony94.ic4", 0x0000, 0x8000, CRC(541abd4f) SHA1(e51f5ca6416c17535f2d2a13a7bedfb3b4b4a58b))
	ROM_CONTINUE(0x0000, 0x8000)

	// Process Unit Effect Data:
	ROM_REGION(0x200000, "effectdata", 0)
	ROM_LOAD("27c4001-12f1_sy172_v1.01_d216.ic5", 0x000000,  0x80000, CRC(ae094fcb) SHA1(c29c27b3c80e67caba2078bb60696c1b8692eb8b))
	ROM_LOAD("27c4001-12f1_sy172_v1.01_d225.ic6", 0x080000,  0x80000, CRC(caa6ccb2) SHA1(9b72dc47cf4cc9c2f9915ea4f1bd7b5136e29db5))
	ROM_LOAD("27c4001-12f1_sy172_v1.01_cc13.ic7", 0x100000,  0x80000, CRC(e1fe8606) SHA1(a573c7023daeb84d5a1182db4051b1bccfcfc1f8))
	ROM_LOAD("27c4001-12f1_sy172_v1.01_c42d.ic8", 0x180000,  0x80000, CRC(66e0f20f) SHA1(e82562ae1eeecc5c97b0f40e01102c2ebe0d6276))
ROM_END

} // anonymous namespace


//   YEAR  NAME   PARENT/COMPAT MACHINE  INPUT    CLASS          INIT     COMPANY  FULLNAME                     FLAGS
SYST(1994, dfs500,    0, 0,     dfs500, dfs500, dfs500_state, empty_init, "Sony", "DFS-500 DME Video Switcher", MACHINE_NOT_WORKING)
