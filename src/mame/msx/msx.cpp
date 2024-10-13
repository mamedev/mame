// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*
** msx.cpp : Emulation of the MSX family of machines
**
** Special usage notes:
**
** ax350iif:
**  The machine has a French keyboard so to select firmware options you
**  need to enter numbers as shift + number key.
**
** cpc300:
**  To get out of the MSX Tutor press the SELECT key. Entering SET SYSTEM 1 should
**  disable the MSX Tutor on next boot and SET SYSTEM 0 should enable.
**
** hx21, hx22:
**  To start the firmware, mount a HX-M200 cartridge and type: CALL JWP.
**
** tpp311:
**  This machine is supposed to boot into logo; it was made to only run logo.
**
** tps312:
**  - To get into MSX-WRITE type: CALL WRITE
**  - To get into MSX-PLAN type: CALL MSXPLAN
**
**
**
** Todo/known issues:
** - general: - Add support for kana lock
** -          - Expansion slots not emulated
** - kanji: The direct rom dump from FS-A1FX shows that the kanji font roms are accessed slightly differently. Most
**          existing kanji font roms may haven been dumped from inside a running machine. Are all other kanji font
**          roms bad? We need more direct rom dumps to know for sure.
** - rs232 support:
**   - mlg3 (working, how does the rs232 switch work?)
**   - mlg30_2 (working, how does the rs232 switch work?)
**   - hbg900ap (not working, switch for terminal/moden operation)
**   - hbg900p (not working)
**   - victhc90 (cannot test, system config not emulated)
**   - victhc95 (cannot test, system config not emulated)
**   - victhc95a (cannot test, system config not emulated)
**   - y805256 (cannot test, rs232 rom has not been dumped?)
** - inputs:
**     fs4000: Is the keypad enter exactly the same as the normal enter key? There does not appear to be a separate mapping for it.
**     piopx7: The keyboard responds like a regular international keyboard, not a japanese keyboard.
**     svi728/svi728es: How are the keypad keys mapped?
** - ax230: Some builtin games show bad graphics, example: Press 5 in main firmware screen and choose the first game
** - piopx7/piopx7uk/piopxv60: Pioneer System Remote (home entertainment/Laserdisc control) not implemented
** - spc800: How to test operation of the han rom?
** - mbh1, mbh1e, mbh2, mbh25: speed controller
** - cx5m128, yis503iir, yis604: yamaha mini cartridge slot
** - cpg120: turbo button
** - fs4000: Internal thermal printer not emulated
** - fs4500, fs4600f, fs4700:
**   - switch to bypass firmware
**   - switch to switch between internal and external printer
**   - switch to switch betweem jp50on and jis layout
**   - internal printer
** - fs5500f1, fs5500f2:
**   - switch to switch between jp50on and jis layout
**   - switcn to bypass firmware
** - fsa1fm:
**   - Firmware and modem partially emulated
** - fsa1mk2: pause button
** - nms8260: HDD not emulated
** - phc77: builtin printer, switch to turn off firmware
** - hbf1: pause button
** - hbf1ii: rensha turbo slider
** - hbf1xd, hbf1xdj, hbf1xv
**   - pause button
**   - speed controller slider
**   - rensha turbo slider
** - victhc90, victhc90a, victhc95, victhc95a: Turbo/2nd cpu not supported. Firmware not working.
** - fsa1fx, fsa1wsx, fsa1wx
**   - rensha turbo slider
**   - pause button
**   - firmware switch
** - phc35j, phc70fd: rensha turbo slider, pause button
** - y503iir, y503iir2: Switch for teacher/student mode, net not emulated
** - cpc300: Config for MSX Tutor ON/OFF is not saved
** - cpc300e: Remove joystick connectors
** - nms8280, nms8280g, and others: Digitizer functionality not emulated
** - expert3i: IDE not emulated
** - expert3t: Turbo not emulated
** - expertac: Does not boot
** - fsa1gt, fsa1st: Add Turbo-R support
** - cpc50a/cpc50b: Remove keyboard; and add an external keyboard??
** - cpc51/cpc61: Remove keyboard and add a keyboard connector
** - mbh3: touch pad not emulated
** - mbh70: Verify firmware operation
** - y805128. y805256: Floppy support broken, jumping to the irq routine in slot 0-0 fails
** - y8805128r2/e: Firmware not working
** - cpg120: Remove ports
**
** TODO:
** - Add T6950 support. T6950 is selectable between pal and ntsc by a pin.
**
************************************************************************/


#include "emu.h"

#include "msx.h"
#include "cpu/z80/r800.h"
#include "formats/fmsx_cas.h"
#include "hashfile.h"
#include "screen.h"
#include "softlist_dev.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"


msx_state::msx_state(const machine_config &mconfig, device_type type, const char *tag, XTAL main_xtal, int cpu_xtal_divider)
	: driver_device(mconfig, type, tag)
	, m_maincpu(*this, "maincpu")
	, m_cassette(*this, "cassette")
	, m_ay8910(*this, "ay8910")
	, m_dac(*this, "dac")
	, m_ppi(*this, "ppi8255")
	, m_tms9928a(*this, "tms9928a")
	, m_cent_status_in(*this, "cent_status_in")
	, m_cent_ctrl_out(*this, "cent_ctrl_out")
	, m_cent_data_out(*this, "cent_data_out")
	, m_centronics(*this, "centronics")
	, m_speaker(*this, "speaker")
	, m_mainirq(*this, "mainirq")
	, m_screen(*this, "screen")
	, m_region_kanji(*this, "kanji")
	, m_gen_port1(*this, "gen1")
	, m_gen_port2(*this, "gen2")
	, m_io_key(*this, "KEY%u", 0U)
	, m_view{ {*this, "view0"}, {*this, "view1"}, {*this, "view2"}, {*this, "view3"} }
	, m_exp_view{
		{ {*this, "view0_0"}, {*this, "view0_1"}, {*this, "view0_2"}, {*this, "view0_3"} },
		{ {*this, "view1_0"}, {*this, "view1_1"}, {*this, "view1_2"}, {*this, "view1_3"} },
		{ {*this, "view2_0"}, {*this, "view2_1"}, {*this, "view2_2"}, {*this, "view2_3"} },
		{ {*this, "view3_0"}, {*this, "view3_1"}, {*this, "view3_2"}, {*this, "view3_3"} },
	 }
	, m_psg_b(0)
	, m_kanji_latch(0)
	, m_slot_expanded{false, false, false, false}
	, m_primary_slot(0)
	, m_secondary_slot{0, 0, 0, 0}
	, m_port_c_old(0)
	, m_keylatch(0)
	, m_caps_led(*this, "caps_led")
	, m_code_led(*this, "code_led")
	, m_main_xtal(main_xtal)
	, m_cpu_xtal_divider(cpu_xtal_divider)
{
}

void msx_state::memory_expand_slot(int slot)
{
	if (slot < 0 || slot > 3)
	{
		fatalerror("Invalid slot %d to expand\n", slot);
	}
	if (m_slot_expanded[slot])
		return;

	m_view[0][slot](0x0000, 0x3fff).view(m_exp_view[slot][0]);
	m_view[1][slot](0x4000, 0x7fff).view(m_exp_view[slot][1]);
	m_view[2][slot](0x8000, 0xbfff).view(m_exp_view[slot][2]);
	m_view[3][slot](0xc000, 0xffff).view(m_exp_view[slot][3]);
	m_view[3][slot](0xffff, 0xffff).rw(FUNC(msx_state::expanded_slot_r), FUNC(msx_state::expanded_slot_w));
	for (int subslot = 0; subslot < 4; subslot++)
	{
		// Ensure that the views will exist
		for (int page = 0; page < 4; page++)
			(m_exp_view[slot][page])[subslot];
	}
	m_slot_expanded[slot] = true;
}

void msx_state::memory_map(address_map &map)
{
	map.unmap_value_high();

	map(0x0000, 0x3fff).view(m_view[0]);
	map(0x4000, 0x7fff).view(m_view[1]);
	map(0x8000, 0xbfff).view(m_view[2]);
	map(0xc000, 0xffff).view(m_view[3]);

	// setup defaults
	for (int slot = 0; slot < 4; slot++)
		for (int page = 0; page < 4; page++)
			m_view[page][slot];

	// Look for expanded slots
	for (const auto& entry : m_internal_slots)
	{
		if (entry.is_expanded)
			memory_expand_slot(entry.prim);
	}

	for (const auto& entry : m_internal_slots)
	{
		memory_view::memory_view_entry *view[4] = {nullptr, nullptr, nullptr, nullptr};
		for (int i = 0; i < entry.numpages; i++)
			view[entry.page + i] = get_view(entry.page + i, entry.prim, entry.sec);

		entry.internal_slot->install(view[0], view[1], view[2], view[3]);
	}
}

memory_view::memory_view_entry *msx_state::get_view(int page, int prim, int sec)
{
	return m_slot_expanded[prim] ? &(m_exp_view[prim][page])[sec] : &m_view[page][prim];
}

void msx_state::msx_base_io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	// 0x7c - 0x7d : MSX-MUSIC/FM-PAC write port. Handlers will be installed if MSX-MUSIC is present in a system
	if (m_hw_def.has_printer_port())
	{
		map(0x90, 0x90).r(m_cent_status_in, FUNC(input_buffer_device::read));
		map(0x90, 0x90).w(m_cent_ctrl_out, FUNC(output_latch_device::write));
		map(0x91, 0x91).w(m_cent_data_out, FUNC(output_latch_device::write));
	}
	map(0xa0, 0xa7).rw(m_ay8910, FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
	// TODO: S-3527 mirrors ac-af
	map(0xa8, 0xab).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
//  // Sanyo optical pen interface (not emulated)
//  map(0xb8, 0xbb).noprw();
	// 0xd8 - 0xdb : Kanji rom interface. I/O handlers will be installed if a kanji rom is present.
	// 0xfc - 0xff : Memory mapper I/O ports. I/O handlers will be installed if a memory mapper is present in a system
}

void msx_state::msx1_io_map(address_map &map)
{
	msx_base_io_map(map);
	map(0x98, 0x99).rw(m_tms9928a, FUNC(tms9928a_device::read), FUNC(tms9928a_device::write));
}

void msx_state::machine_reset()
{
	m_primary_slot = 0;
	m_secondary_slot[0] = 0;

	for (int page = 0; page < 4; page++)
		m_view[page].select(0);

	if (m_slot_expanded[0])
		for (int page = 0; page < 4; page++)
			m_exp_view[0][page].select(0);

	m_caps_led = 0;
	m_code_led = 0;
	m_system_control = m_has_system_control ? 0x00 : 0xff;
}

void msx_state::machine_start()
{
	m_caps_led.resolve();
	m_code_led.resolve();
	m_port_c_old = 0xff;

	if (m_region_kanji.found() && m_region_kanji.length() >= 0x20000)
	{
		get_io_space().install_write_tap(0xd8, 0xd9, "kanji_w", [this] (offs_t ofs, u8 &data, u8) { this->kanji_w(ofs, data); });
		get_io_space().install_read_tap(0xd9, 0xd9, "kanji_r", [this] (offs_t ofs, u8 &data, u8) { data &= this->kanji_r(ofs); });
		if (m_region_kanji.length() >= 0x40000)
		{
			get_io_space().install_write_tap(0xda, 0xdb, "kanji2_w", [this] (offs_t ofs, u8 &data, u8) { this->kanji2_w(ofs, data); });
			get_io_space().install_read_tap(0xdb, 0xdb, "kanji2_r", [this] (offs_t ofs, u8 &data, u8) { data &= this->kanji2_r(ofs); });
		}
	}
	if (m_has_system_control)
	{
		get_io_space().install_write_handler(0xf5, 0xf5, write8smo_delegate(*this, [this] (u8 data) { m_system_control = data; }, "system_control"));
	}
}

void msx_state::driver_start()
{
	m_maincpu->set_input_line_vector(0, 0xff); // Z80
	m_maincpu->z80_set_m1_cycles(4+1); // 1 WAIT CLK per M1

	save_item(NAME(m_psg_b));
	save_item(NAME(m_kanji_latch));
	save_item(NAME(m_kanji_fsa1fx));
	save_item(NAME(m_slot_expanded));
	save_item(NAME(m_primary_slot));
	save_item(NAME(m_secondary_slot));
	save_item(NAME(m_port_c_old));
	save_item(NAME(m_keylatch));
	save_item(NAME(m_system_control));
}

u8 msx_state::psg_port_a_r()
{
	u8 data = 0x80;
	if (m_cassette)
		data = (m_cassette->input() > 0.0038 ? 0x80 : 0);

	if (BIT(m_psg_b, 6))
		data |= m_gen_port2->read() & 0x3f;
	else
		data |= m_gen_port1->read() & 0x3f;

	return data;
}

u8 msx_state::psg_port_b_r()
{
	return m_psg_b;
}

void msx_state::psg_port_a_w(u8 data)
{
}

void msx_state::psg_port_b_w(u8 data)
{
	// Code(/Kana/Arabic/Hangul) led
	m_code_led = BIT(~data, 7);

	m_gen_port1->pin_6_w(BIT(data, 0));
	m_gen_port1->pin_7_w(BIT(data, 1));
	m_gen_port2->pin_6_w(BIT(data, 2));
	m_gen_port2->pin_7_w(BIT(data, 3));
	m_gen_port1->pin_8_w(BIT(data, 4));
	m_gen_port2->pin_8_w(BIT(data, 5));

	m_psg_b = data;
}

void msx_state::ppi_port_a_w(u8 data)
{
	m_primary_slot = data;

	LOG("write to primary slot select: %02x\n", m_primary_slot);
	for (int page = 0; page < 4; page++)
		m_view[page].select((data >> (page * 2)) & 0x03);
}

void msx_state::ppi_port_c_w(u8 data)
{
	m_keylatch = data & 0x0f;

	// caps lock
	m_caps_led = BIT(~data, 6);

	// key click
	if (BIT(m_port_c_old ^ data, 7))
		m_dac->write(BIT(data, 7));

	// cassette motor on/off
	if (BIT(m_port_c_old ^ data, 4) && m_cassette)
		m_cassette->change_state(BIT(data, 4) ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);

	// cassette signal write
	if (BIT(m_port_c_old ^ data, 5) && m_cassette)
		m_cassette->output(BIT(data, 5) ? -1.0 : 1.0);

	m_port_c_old = data;
}

u8 msx_state::ppi_port_b_r()
{
	u8 result = 0xff;

	if (m_keylatch <= 10)
	{
		return m_io_key[m_keylatch]->read();
	}
	return result;
}

void msx_state::expanded_slot_w(u8 data)
{
	const int slot = (m_primary_slot >> 6) & 0x03;
	m_secondary_slot[slot] = data;
	LOG("write to expanded slot select: %02x\n", data);
	for (int page = 0; page < 4; page++)
		m_exp_view[slot][page].select((data >> (page * 2)) & 0x03);
}

u8 msx_state::expanded_slot_r()
{
	const int slot = (m_primary_slot >> 6) & 0x03;
	return ~m_secondary_slot[slot];
}

u8 msx_state::kanji_r(offs_t offset)
{
	if (BIT(m_system_control, 0))
	{
		const u32 latch = m_kanji_fsa1fx ? bitswap<17>(m_kanji_latch, 4, 3, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 2, 1, 0) : m_kanji_latch;
		const u8 result = m_region_kanji[latch];

		if (!machine().side_effects_disabled())
		{
			m_kanji_latch = (m_kanji_latch & ~0x1f) | ((m_kanji_latch + 1) & 0x1f);
		}
		return result;
	}
	return 0xff;
}

void msx_state::kanji_w(offs_t offset, u8 data)
{
	if (BIT(m_system_control, 0))
	{
		if (BIT(offset, 0))
			m_kanji_latch = (m_kanji_latch & 0x007e0) | ((data & 0x3f) << 11);
		else
			m_kanji_latch = (m_kanji_latch & 0x1f800) | ((data & 0x3f) << 5);
	}
}

u8 msx_state::kanji2_r(offs_t offset)
{
	if (BIT(m_system_control, 1))
	{
		// TODO: Are there one or two latches in a system?
		const u32 latch = m_kanji_fsa1fx ? bitswap<17>(m_kanji_latch, 4, 3, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 2, 1, 0) : m_kanji_latch;
		const u8 result = m_region_kanji[0x20000 | latch];

		if (!machine().side_effects_disabled())
		{
			m_kanji_latch = (m_kanji_latch & ~0x1f) | ((m_kanji_latch + 1) & 0x1f);
		}
		return result;
	}
	return 0xff;
}

void msx_state::kanji2_w(offs_t offset, u8 data)
{
	if (BIT(m_system_control, 1))
	{
		if (BIT(offset, 0))
			m_kanji_latch = (m_kanji_latch & 0x007e0) | ((data & 0x3f) << 11);
		else
			m_kanji_latch = (m_kanji_latch & 0x1f800) | ((data & 0x3f) << 5);
	}
}

void msx_state::msx_base(ay8910_type ay8910_type, machine_config &config, const internal_layout &layout)
{
	// basic machine hardware
	Z80(config, m_maincpu, m_main_xtal / m_cpu_xtal_divider);         // 3.579545 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &msx_state::memory_map);
	config.set_maximum_quantum(attotime::from_hz(60));

	INPUT_MERGER_ANY_HIGH(config, m_mainirq).output_handler().set_inputline("maincpu", INPUT_LINE_IRQ0);

	I8255(config, m_ppi);
	m_ppi->out_pa_callback().set(FUNC(msx_state::ppi_port_a_w));
	m_ppi->in_pb_callback().set(FUNC(msx_state::ppi_port_b_r));
	m_ppi->out_pc_callback().set(FUNC(msx_state::ppi_port_c_w));

	// Video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);

	// sound hardware
	SPEAKER(config, m_speaker).front_center();
	DAC_1BIT(config, m_dac, 0);
	m_dac->add_route(ALL_OUTPUTS, m_speaker, 0.1);

	if (ay8910_type == SND_AY8910)
		AY8910(config, m_ay8910, m_main_xtal / m_cpu_xtal_divider / 2);
	if (ay8910_type == SND_YM2149)
		YM2149(config, m_ay8910, m_main_xtal / m_cpu_xtal_divider / 2);
	m_ay8910->set_flags(AY8910_SINGLE_OUTPUT);
	m_ay8910->port_a_read_callback().set(FUNC(msx_state::psg_port_a_r));
	m_ay8910->port_b_read_callback().set(FUNC(msx_state::psg_port_b_r));
	m_ay8910->port_a_write_callback().set(FUNC(msx_state::psg_port_a_w));
	m_ay8910->port_b_write_callback().set(FUNC(msx_state::psg_port_b_w));
	m_ay8910->add_route(ALL_OUTPUTS, m_speaker, 1.0);

	MSX_GENERAL_PURPOSE_PORT(config, m_gen_port1, msx_general_purpose_port_devices, "joystick");
	MSX_GENERAL_PURPOSE_PORT(config, m_gen_port2, msx_general_purpose_port_devices, "joystick");

	if (m_hw_def.has_printer_port())
	{
		// printer
		CENTRONICS(config, m_centronics, centronics_devices, "printer");
		m_centronics->busy_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit1));

		OUTPUT_LATCH(config, m_cent_data_out);
		m_centronics->set_output_latch(*m_cent_data_out);
		INPUT_BUFFER(config, m_cent_status_in);

		OUTPUT_LATCH(config, m_cent_ctrl_out);
		m_cent_ctrl_out->bit_handler<1>().set(m_centronics, FUNC(centronics_device::write_strobe));
	}

	if (m_hw_def.has_cassette())
	{
		// cassette
		CASSETTE(config, m_cassette);
		m_cassette->set_formats(fmsx_cassette_formats);
		m_cassette->set_default_state(CASSETTE_PLAY);
		m_cassette->add_route(ALL_OUTPUTS, m_speaker, 0.15);
		m_cassette->set_interface("msx_cass");
	}

	config.set_default_layout(layout);
}

void msx_state::msx1_add_softlists(machine_config &config)
{
	if (m_hw_def.has_cassette())
		SOFTWARE_LIST(config, "cass_list").set_original("msx1_cass");

	if (m_hw_def.has_cartslot())
		SOFTWARE_LIST(config, "cart_list").set_original("msx1_cart");

	if (m_hw_def.has_fdc())
		SOFTWARE_LIST(config, "flop_list").set_original("msx1_flop");
}

void msx_state::msx1(vdp_type vdp_type, ay8910_type ay8910_type, machine_config &config, const internal_layout &layout)
{
	msx_base(ay8910_type, config, layout);

	m_maincpu->set_addrmap(AS_IO, &msx_state::msx1_io_map);

	if (vdp_type == VDP_TMS9118)
		TMS9118(config, m_tms9928a, m_main_xtal);
	if (vdp_type == VDP_TMS9128)
		TMS9128(config, m_tms9928a, m_main_xtal);
	if (vdp_type == VDP_TMS9129)
		TMS9129(config, m_tms9928a, m_main_xtal);
	if (vdp_type == VDP_TMS9918)
		TMS9918(config, m_tms9928a, m_main_xtal);
	if (vdp_type == VDP_TMS9918A)
		TMS9918A(config, m_tms9928a, m_main_xtal);
	if (vdp_type == VDP_TMS9928A)
		TMS9928A(config, m_tms9928a, m_main_xtal);
	if (vdp_type == VDP_TMS9929A)
		TMS9929A(config, m_tms9928a, m_main_xtal);
	m_tms9928a->set_screen(m_screen);
	m_tms9928a->set_vram_size(0x4000);
	m_tms9928a->int_callback().set(m_mainirq, FUNC(input_merger_device::in_w<0>));

	// Software lists
	msx1_add_softlists(config);

	m_has_system_control = false;
}

address_space& msx_state::get_io_space()
{
	return m_maincpu->space(AS_IO);
}

void msx_state::setup_slot_spaces(msx_internal_slot_interface &device)
{
	device.set_memory_space(m_maincpu, AS_PROGRAM);
	device.set_io_space(m_maincpu, AS_IO);
	device.set_maincpu(m_maincpu);
}



msx2_base_state::msx2_base_state(const machine_config &mconfig, device_type type, const char *tag, XTAL main_xtal, int cpu_xtal_divider)
	: msx_state(mconfig, type, tag, main_xtal, cpu_xtal_divider)
	, m_v9938(*this, "v9938")
	, m_v9958(*this, "v9958")
	, m_ym2413(*this, "ym2413")
	, m_rtc(*this, "rtc")
	, m_rtc_latch(0)
{
}

void msx2_base_state::msx2_base_io_map(address_map &map)
{
	msx_base_io_map(map);
	map(0x40, 0x4f).rw(FUNC(msx2_base_state::switched_r), FUNC(msx2_base_state::switched_w));
	map(0xb4, 0xb4).w(FUNC(msx2_base_state::rtc_latch_w));
	map(0xb5, 0xb5).rw(FUNC(msx2_base_state::rtc_reg_r), FUNC(msx2_base_state::rtc_reg_w));
}

void msx2_base_state::msx2_io_map(address_map &map)
{
	msx2_base_io_map(map);
	// TODO: S-1985 mirrors 9c-9f
	map(0x98, 0x9b).rw(m_v9938, FUNC(v9938_device::read), FUNC(v9938_device::write));
}

void msx2_base_state::msx2_v9958_io_map(address_map &map)
{
	msx2_base_io_map(map);
	// TODO: S-1985 mirrors 9c-9f
	map(0x98, 0x9b).rw(m_v9958, FUNC(v9958_device::read), FUNC(v9958_device::write));
}

void msx2_base_state::machine_start()
{
	msx_state::machine_start();

	for (msx_switched_interface &switched : device_interface_enumerator<msx_switched_interface>(*this))
		m_switched.push_back(&switched);

	save_item(NAME(m_rtc_latch));
}

/*
** RTC functions
*/

void msx2_base_state::rtc_latch_w(u8 data)
{
	m_rtc_latch = data & 15;
}

void msx2_base_state::rtc_reg_w(u8 data)
{
	m_rtc->write(m_rtc_latch, data);
}

u8 msx2_base_state::rtc_reg_r()
{
	return m_rtc->read(m_rtc_latch);
}

u8 msx2_base_state::switched_r(offs_t offset)
{
	u8 data = 0xff;

	for (int i = 0; i < m_switched.size(); i++)
	{
		data &= m_switched[i]->switched_read(offset);
	}

	return data;
}

void msx2_base_state::switched_w(offs_t offset, u8 data)
{
	for (int i = 0; i < m_switched.size(); i++)
	{
		m_switched[i]->switched_write(offset, data);
	}
}

void msx2_base_state::msx_ym2413(machine_config &config)
{
	YM2413(config, m_ym2413, m_main_xtal / m_cpu_xtal_divider).add_route(ALL_OUTPUTS, m_speaker, 0.8);
}

void msx2_base_state::msx2_64kb_vram(machine_config &config)
{
	m_v9938->set_vram_size(0x10000);
}

void msx2_base_state::msx2_add_softlists(machine_config &config)
{
	if (m_hw_def.has_cassette())
	{
		SOFTWARE_LIST(config, "cass_list").set_original("msx2_cass");
		SOFTWARE_LIST(config, "msx1_cass_list").set_compatible("msx1_cass");
	}

	if (m_hw_def.has_cartslot())
	{
		SOFTWARE_LIST(config, "cart_list").set_original("msx2_cart");
		SOFTWARE_LIST(config, "msx1_cart_list").set_compatible("msx1_cart");
	}

	if (m_hw_def.has_fdc())
	{
		SOFTWARE_LIST(config, "flop_list").set_original("msx2_flop");
		SOFTWARE_LIST(config, "msx1_flop_list").set_compatible("msx1_flop");
	}
}

void msx2_base_state::msx2_base(ay8910_type ay8910_type, machine_config &config, const internal_layout &layout)
{
	msx_base(ay8910_type, config, layout);

	// real time clock
	RP5C01(config, m_rtc, 32.768_kHz_XTAL);

	m_has_system_control = true;
}

void msx2_base_state::msx2(ay8910_type ay8910_type, machine_config &config, const internal_layout &layout)
{
	msx2_base(ay8910_type, config, layout);

	m_maincpu->set_addrmap(AS_IO, &msx2_base_state::msx2_io_map);

	// video hardware
	V9938(config, m_v9938, m_main_xtal);
	m_v9938->set_screen_ntsc(m_screen);
	m_v9938->set_vram_size(0x20000);
	m_v9938->int_cb().set(m_mainirq, FUNC(input_merger_device::in_w<0>));

	// Software lists
	msx2_add_softlists(config);
}

void msx2_base_state::msx2_pal(ay8910_type ay8910_type, machine_config &config, const internal_layout &layout)
{
	msx2(ay8910_type, config, layout);
	m_v9938->set_screen_pal(m_screen);
}

void msx2_base_state::msx2_v9958_base(ay8910_type ay8910_type, machine_config &config, const internal_layout &layout)
{
	msx2_base(ay8910_type, config, layout);

	m_maincpu->set_addrmap(AS_IO, &msx2_base_state::msx2_v9958_io_map);

	// video hardware
	V9958(config, m_v9958, m_main_xtal);
	m_v9958->set_screen_ntsc(m_screen);
	m_v9958->set_vram_size(0x20000);
	m_v9958->int_cb().set(m_mainirq, FUNC(input_merger_device::in_w<0>));
}



msx2p_base_state::msx2p_base_state(const machine_config &mconfig, device_type type, const char *tag, XTAL main_xtal, int cpu_xtal_divider)
	: msx2_base_state(mconfig, type, tag, main_xtal, cpu_xtal_divider)
	, m_cold_boot_flags(0)
	, m_boot_flags(0)
{
}

void msx2p_base_state::machine_start()
{
	msx2_base_state::machine_start();

	save_item(NAME(m_boot_flags));
	save_item(NAME(m_vdp_mode));
}

void msx2p_base_state::machine_reset()
{
	msx2_base_state::machine_reset();
	m_boot_flags = m_cold_boot_flags;
}

void msx2p_base_state::msx2plus_io_map(address_map &map)
{
	msx2_v9958_io_map(map);
	map(0xf3, 0xf3).lrw8(NAME([this]() { return m_vdp_mode; }), NAME([this](u8 data) { m_vdp_mode = data; }));
	map(0xf4, 0xf4).lrw8(NAME([this]() { return m_boot_flags; }), NAME([this](u8 data) { m_boot_flags = data; }));
}

void msx2p_base_state::msx2plus_base(ay8910_type ay8910_type, machine_config &config, const internal_layout &layout)
{
	msx2_v9958_base(ay8910_type, config, layout);

	m_maincpu->set_addrmap(AS_IO, &msx2p_base_state::msx2plus_io_map);
}

void msx2p_base_state::msx2plus_pal_base(ay8910_type ay8910_type, machine_config &config, const internal_layout &layout)
{
	msx2plus_base(ay8910_type, config, layout);
	m_v9958->set_screen_pal(m_screen);
}

void msx2p_base_state::msx2plus(ay8910_type ay8910_type, machine_config &config, const internal_layout &layout)
{
	msx2plus_base(ay8910_type, config, layout);

	// Software lists
	msx2plus_add_softlists(config);
}

void msx2p_base_state::msx2plus_pal(ay8910_type ay8910_type, machine_config &config, const internal_layout &layout)
{
	msx2plus(ay8910_type, config, layout);
	m_v9958->set_screen_pal(m_screen);
}

void msx2p_base_state::msx2plus_add_softlists(machine_config &config)
{
	if (m_hw_def.has_cassette())
	{
		SOFTWARE_LIST(config, "cass_list").set_original("msx2_cass");
		SOFTWARE_LIST(config, "msx1_cass_list").set_compatible("msx1_cass");
	}

	if (m_hw_def.has_cartslot())
	{
		SOFTWARE_LIST(config, "cart_list").set_original("msx2p_cart");
		SOFTWARE_LIST(config, "msx2_cart_list").set_compatible("msx2_cart");
		SOFTWARE_LIST(config, "msx1_cart_list").set_compatible("msx1_cart");
	}

	if (m_hw_def.has_fdc())
	{
		SOFTWARE_LIST(config, "flop_list").set_original("msx2p_flop");
		SOFTWARE_LIST(config, "msx2_flop_list").set_compatible("msx2_flop");
		SOFTWARE_LIST(config, "msx1_flop_list").set_compatible("msx1_flop");
	}
}

void msx2p_base_state::turbor(ay8910_type ay8910_type, machine_config &config, const internal_layout &layout)
{
	msx2plus_base(ay8910_type, config, layout);

	R800(config.replace(), m_maincpu, 28.636363_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &msx2p_base_state::memory_map);
	m_maincpu->set_addrmap(AS_IO, &msx2p_base_state::msx2plus_io_map);

	// Software lists
	turbor_add_softlists(config);
}

void msx2p_base_state::turbor_add_softlists(machine_config &config)
{
	if (m_hw_def.has_cartslot())
	{
		SOFTWARE_LIST(config, "cart_list").set_original("msxr_cart");
		SOFTWARE_LIST(config, "msx2p_cart_list").set_compatible("msx2p_cart");
		SOFTWARE_LIST(config, "msx2_cart_list").set_compatible("msx2_cart");
		SOFTWARE_LIST(config, "msx1_cart_list").set_compatible("msx1_cart");
	}

	if (m_hw_def.has_fdc())
	{
		SOFTWARE_LIST(config, "flop_list").set_original("msxr_flop");
		SOFTWARE_LIST(config, "msx2p_flop_list").set_compatible("msx2p_flop");
		SOFTWARE_LIST(config, "msx2_flop_list").set_compatible("msx2_flop");
		SOFTWARE_LIST(config, "msx1_flop_list").set_compatible("msx1_flop");
	}
}

