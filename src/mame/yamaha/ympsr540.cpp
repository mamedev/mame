// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"

#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "cpu/sh/sh7042.h"
#include "imagedev/floppy.h"
#include "machine/nvram.h"
#include "machine/upd765.h"
#include "sound/swx00.h"
#include "video/hd44780.h"

#include "debugger.h"
#include "screen.h"
#include "speaker.h"

namespace {

class psr540_state : public driver_device {
public:
	psr540_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_swx00(*this, "swx00"),
		m_lcdc(*this, "ks0066"),
		m_floppy(*this, "fdc:0"),
		m_fdc(*this, "fdc"),
		m_nvram(*this, "ram"),
		m_inputs(*this, "B%u", 1U),
		m_outputs(*this, "%02d.%x.%x", 0U, 0U, 0U),
		m_sustain(*this, "SUSTAIN"),
		m_pitch_bend(*this, "PITCH_BEND")
	{ }

	void psr540(machine_config &config);

private:
	required_device<sh7042_device> m_maincpu;
	required_device<swx00_sound_device> m_swx00;
	required_device<hd44780_device> m_lcdc;
	required_device<floppy_connector> m_floppy;
	required_device<hd63266f_device> m_fdc;
	required_device<nvram_device> m_nvram;
	required_ioport_array<8> m_inputs;
	output_finder<80, 8, 5> m_outputs;
	required_ioport m_sustain;
	required_ioport m_pitch_bend;

	u16 m_pe, m_led, m_scan;

	void map(address_map &map) ATTR_COLD;

	void machine_start() override ATTR_COLD;

	u16 adc_sustain_r();
	u16 adc_midisw_r();
	u16 adc_battery_r();

	u16 pe_r();
	void pe_w(u16 data);
	u8 pf_r();

	void lcd_data_w(u8 data);
	void led_data_w(offs_t, u16 data, u16 mem_mask);
	void render_w(int state);
};

void psr540_state::render_w(int state)
{
	if(!state)
		return;

	const u8 *render = m_lcdc->render();
	for(int yy=0; yy != 8; yy++)
		for(int x=0; x != 80; x++) {
			uint8_t v = render[16*x + yy];
			for(int xx=0; xx != 5; xx++)
				m_outputs[x][yy][xx] = (v >> xx) & 1;
		}
}

void psr540_state::machine_start()
{
	m_outputs.resolve();

	save_item(NAME(m_pe));
	save_item(NAME(m_led));
	save_item(NAME(m_scan));
	m_pe = 0;
	m_led = 0;
	m_scan = 0;

	m_fdc->set_floppy(m_floppy->get_device());
}

u16 psr540_state::adc_sustain_r()
{
	return m_sustain->read() ? 0x3ff : 0;
}

u16 psr540_state::adc_midisw_r()
{
	return 0;
}

u16 psr540_state::adc_battery_r()
{
	return 0x3ff;
}

static void psr540_floppies(device_slot_interface &device)
{
	device.option_add("35hd", FLOPPY_35_HD);
}

void psr540_state::psr540(machine_config &config)
{
	SH7042A(config, m_maincpu, 7_MHz_XTAL*4); // // md=a, on-chip rom, 32-bit space, pll 4x -- XW25610 6437042F14F 9M1 A
	m_maincpu->set_addrmap(AS_PROGRAM, &psr540_state::map);
	m_maincpu->read_adc<0>().set(FUNC(psr540_state::adc_midisw_r));
	m_maincpu->read_adc<1>().set(FUNC(psr540_state::adc_sustain_r));
	m_maincpu->read_adc<2>().set(FUNC(psr540_state::adc_battery_r));
	m_maincpu->read_adc<3>().set_constant(0);
	m_maincpu->read_adc<4>().set_ioport(m_pitch_bend);
	m_maincpu->read_adc<5>().set_constant(0); // Actually used as pf5
	m_maincpu->read_porte().set(FUNC(psr540_state::pe_r));
	m_maincpu->write_porte().set(FUNC(psr540_state::pe_w));
	m_maincpu->read_portf().set(FUNC(psr540_state::pf_r));

	SWX00_SOUND(config, m_swx00);
	m_swx00->add_route(0, "lspeaker", 1.0);
	m_swx00->add_route(1, "rspeaker", 1.0);

	KS0066(config, m_lcdc, 270000); // OSC = 91K resistor, TODO: actually KS0066U-10B
	m_lcdc->set_default_bios_tag("f00");
	m_lcdc->set_lcd_size(2, 40);

	HD63266F(config, m_fdc, 16_MHz_XTAL);
	//m_fdc->drq_wr_callback().set([this](int state){ fdc_drq = state; maincpu->set_input_line(Z180_INPUT_LINE_DREQ0, state); });
	m_fdc->set_ready_line_connected(false);
	m_fdc->set_select_lines_connected(false);
	m_fdc->inp_rd_callback().set([this](){ return m_floppy->get_device()->dskchg_r(); });
	m_fdc->intrq_wr_callback().set_inputline(m_maincpu, 0);

	FLOPPY_CONNECTOR(config, m_floppy, psr540_floppies, "35hd", floppy_image_device::default_pc_floppy_formats, true);

	NVRAM(config, m_nvram, nvram_device::DEFAULT_NONE);

	/* video hardware */
	auto &screen = SCREEN(config, "screen", SCREEN_TYPE_SVG);
	screen.set_refresh_hz(60);
	screen.set_size(1080, 360);
	screen.set_visarea_full();
	screen.screen_vblank().set(FUNC(psr540_state::render_w));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	auto &mdin(MIDI_PORT(config, "mdin"));
	midiin_slot(mdin);
	mdin.rxd_handler().set(m_maincpu, FUNC(sh7042a_device::sci_rx_w<0>));

	auto &mdout(MIDI_PORT(config, "mdout"));
	midiout_slot(mdout);
	m_maincpu->write_sci_tx<0>().set(mdout, FUNC(midi_port_device::write_txd));
}

// Port E:
//  15:    N10
//  14:    (dack0)
//  13-12: N9-8
//  11:    lcd_rs
//  10:    N7
//   9:    lcd_en
//   8:    rdens
//   7-5:  N6-4

// Port F:
//  7-5:   N3-1
//  4-0:   (ad inputs)

u16 psr540_state::pe_r()
{
	u32 n = m_inputs[m_scan]->read();
	return util::bitswap(~n | 0x8000, 9, 15, 8, 7, 15, 6, 15, 15, 5, 4, 3, 15, 15, 15, 15, 15);
}

u8 psr540_state::pf_r()
{
	u32 n = m_inputs[m_scan]->read();
	return util::bitswap(~n | 0x8000, 2, 1, 0, 15, 15, 15, 15, 15);
}

void psr540_state::pe_w(u16 data)
{
	m_pe = data;
	//logerror("pe lcd_rs=%x lcd_en=%x rdens=%d ldcic=%d fdcic=%d (%s)\n", BIT(m_pe, 11), BIT(m_pe, 9), BIT(m_pe, 8), BIT(m_pe, 4), BIT(m_pe, 3), machine().describe_context());
	m_lcdc->rs_w(BIT(m_pe, 11));
	m_lcdc->e_w(BIT(m_pe, 9));
	m_fdc->rate_w(!BIT(m_pe, 8));

	if(BIT(m_pe, 4))
		m_scan = m_led & 7;
}

void psr540_state::lcd_data_w(u8 data)
{
	m_lcdc->db_w(data);
}

void psr540_state::led_data_w(offs_t, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_led);
	if(BIT(m_pe, 4))
		m_scan = m_led & 7;
}

void psr540_state::map(address_map &map)
{
	map(0x00000000, 0x0001ffff).rom().region("kernel", 0).mirror(0x1e0000);  // Internal rom, single-cycle

	// 200000-3fffff: cs0 space, 8bits, 1 cycle between accesses, cs assert extension, 6 wait states
	// 200000 fdc
	map(0x00200000, 0x00200003).m(m_fdc, FUNC(hd63266f_device::map));
	// 280000 sram (battery-backed)
	map(0x00280000, 0x0029ffff).ram().share("ram");
	// 2c0000 leds/scanning
	map(0x002c0000, 0x002c0001).w(FUNC(psr540_state::led_data_w));
	// 300000 lcd
	map(0x00300000, 0x00300000).w(FUNC(psr540_state::lcd_data_w));

	// 400000-7fffff: cs1 space, 16 bits, 2 wait states
	map(0x00400000, 0x007fffff).rom().region("program_rom", 0);

	// c00000-ffffff: cs3 space, 8 bits, cs assert extension, 3 wait states
	map(0x00c00000, 0x00c00fff).m(m_swx00, FUNC(swx00_sound_device::map));

	// Dedicated dram space, ras precharge = 1.5, ras-cas delay 2, cas-before-ras 2.5, dram write 4, read 3, idle 0, burst, ras down, 16bits, 9-bit address
	// Automatic refresh every 436 cycles, cas-before-ras
	map(0x01000000, 0x0107ffff).ram(); // dram
}

static INPUT_PORTS_START( psr540 )
	PORT_START("SUSTAIN")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Sustain Pedal")

	PORT_START("PITCH_BEND")
	PORT_BIT(0x3ff, 0x200, IPT_PADDLE) PORT_NAME("Pitch Bend") PORT_SENSITIVITY(30)

	PORT_START("B1")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Exit") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Utility") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Save") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Load") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Part R2") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Part R1") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Part L") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("8") PORT_CODE(KEYCODE_8)

	PORT_START("B2")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Voice L") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Voice R1") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Voice R2") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Track 11")
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Track 12")
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Track 13")
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Track 14")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Track 15")
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Track 16")
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Style") PORT_CODE(KEYCODE_P)

	PORT_START("B3")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Track 1")
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Track 2")
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Track 3")
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Track 4")
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Track 5")
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Track 6")
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Track 7")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Track 8")
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Track 9")
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Track 10")

	PORT_START("B4")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Vocal Chg") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Mixer") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Direct Access") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Next") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Best") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Harmony/Echo") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Sustain") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Touch") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Fast/Slow") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("DSP") PORT_CODE(KEYCODE_COLON)

	PORT_START("B5")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("2")     PORT_CODE(KEYCODE_2)
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("1")     PORT_CODE(KEYCODE_1)
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("6")     PORT_CODE(KEYCODE_6)
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("5")     PORT_CODE(KEYCODE_5)
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("4")     PORT_CODE(KEYCODE_4)
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("7")     PORT_CODE(KEYCODE_7)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("+/Yes") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("0")     PORT_CODE(KEYCODE_0)
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("-/No")  PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("3")     PORT_CODE(KEYCODE_3)

	PORT_START("B6")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Song") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Function") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Acmp/Song Vol") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Transpose") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Tempo/Tap") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Fingering") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Acmp On/Off") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Record") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Demo") PORT_CODE(KEYCODE_STOP)

	PORT_START("B7")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Pad 1")
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Pad 2")
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Pad 3")
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Pad 4")
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Pad Stop")
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Intro")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Sync Stop")
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Sync Start")
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Start/Stop")

	PORT_START("B8")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Reg 4")
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Reg 3")
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Reg 1")
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Memory")
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Freeze")
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Reg 2")
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Main A")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Main B")
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Ending")
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Reg./OTS")
INPUT_PORTS_END

ROM_START( psr540 )
	// Hitachi HI-7 RTOS, version 1.6.  Internal to the sh-2
	ROM_REGION32_BE( 0x400000, "kernel", 0 )
	ROM_LOAD( "hi_7_v1_6.bin", 0, 0x20000, CRC(1f1992d9) SHA1(4f037cc5d7928ace240e31c65dfdff7ce91bd33a))

	ROM_REGION32_BE( 0x400000, "program_rom", 0 )
	ROM_LOAD16_WORD_SWAP( "xw25320.ic310", 0, 0x400000, CRC(e8d29e49) SHA1(079e0ccf6cf5d5bd2d2d82076b09dd702fcd1421))

	ROM_REGION16_LE( 0x600000, "swx00", 0)
	ROM_LOAD16_WORD_SWAP( "xw25410.ic210", 0, 0x400000, CRC(c7c4736d) SHA1(ff1052eb076557071ed8652e6c2fc0925144fbd5))
	ROM_LOAD16_WORD_SWAP( "xw25520.ic220", 0x400000, 0x200000, CRC(9ef56c4e) SHA1(f26b588f9bcfd7bdbf1c0b38e4a1ea57e2f29f10))

	ROM_REGION(634772, "screen", ROMREGION_ERASE00)
	ROM_LOAD("psr540-lcd.svg", 0, 634772, CRC(606d85ab) SHA1(6eff1f028c531cdcd070b21949e4624af0a586a1))
ROM_END

} // anonymous namespace

SYST( 1999, psr540, 0, 0, psr540, psr540, psr540_state, empty_init, "Yamaha", "PSR540", MACHINE_IS_SKELETON )
