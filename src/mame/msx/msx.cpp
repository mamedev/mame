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
** - victhc90, victhc95, victhc95a: Turbo/2nd cpu not supported. Firmware not working.
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


msx_state::msx_state(const machine_config &mconfig, device_type type, const char *tag)
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
	, m_leds(*this, "led%u", 1U)
	, m_view_page0(*this, "view0")
	, m_view_page1(*this, "view1")
	, m_view_page2(*this, "view2")
	, m_view_page3(*this, "view3")
	, m_view_slot0_page0(*this, "view0_0")
	, m_view_slot0_page1(*this, "view0_1")
	, m_view_slot0_page2(*this, "view0_2")
	, m_view_slot0_page3(*this, "view0_3")
	, m_view_slot1_page0(*this, "view1_0")
	, m_view_slot1_page1(*this, "view1_1")
	, m_view_slot1_page2(*this, "view1_2")
	, m_view_slot1_page3(*this, "view1_3")
	, m_view_slot2_page0(*this, "view2_0")
	, m_view_slot2_page1(*this, "view2_1")
	, m_view_slot2_page2(*this, "view2_2")
	, m_view_slot2_page3(*this, "view2_3")
	, m_view_slot3_page0(*this, "view3_0")
	, m_view_slot3_page1(*this, "view3_1")
	, m_view_slot3_page2(*this, "view3_2")
	, m_view_slot3_page3(*this, "view3_3")
	, m_psg_b(0)
	, m_kanji_latch(0)
	, m_slot_expanded{false, false, false, false}
	, m_primary_slot(0)
	, m_secondary_slot{0, 0, 0, 0}
	, m_port_c_old(0)
	, m_keylatch(0)
{
	m_view[0] = &m_view_page0;
	m_view[1] = &m_view_page1;
	m_view[2] = &m_view_page2;
	m_view[3] = &m_view_page3;
	m_exp_view[0][0] = &m_view_slot0_page0;
	m_exp_view[0][1] = &m_view_slot0_page1;
	m_exp_view[0][2] = &m_view_slot0_page2;
	m_exp_view[0][3] = &m_view_slot0_page3;
	m_exp_view[1][0] = &m_view_slot1_page0;
	m_exp_view[1][1] = &m_view_slot1_page1;
	m_exp_view[1][2] = &m_view_slot1_page2;
	m_exp_view[1][3] = &m_view_slot1_page3;
	m_exp_view[2][0] = &m_view_slot2_page0;
	m_exp_view[2][1] = &m_view_slot2_page1;
	m_exp_view[2][2] = &m_view_slot2_page2;
	m_exp_view[2][3] = &m_view_slot2_page3;
	m_exp_view[3][0] = &m_view_slot3_page0;
	m_exp_view[3][1] = &m_view_slot3_page1;
	m_exp_view[3][2] = &m_view_slot3_page2;
	m_exp_view[3][3] = &m_view_slot3_page3;
}

void msx_state::memory_expand_slot(int slot)
{
	if (slot < 0 || slot > 3)
	{
		fatalerror("Invalid slot %d to expand\n", slot);
	}
	if (m_slot_expanded[slot])
		return;

	m_view_page0[slot](0x0000, 0x3fff).view(*m_exp_view[slot][0]);
	m_view_page1[slot](0x4000, 0x7fff).view(*m_exp_view[slot][1]);
	m_view_page2[slot](0x8000, 0xbfff).view(*m_exp_view[slot][2]);
	m_view_page3[slot](0xc000, 0xffff).view(*m_exp_view[slot][3]);
	m_view_page3[slot](0xffff, 0xffff).rw(FUNC(msx_state::expanded_slot_r), FUNC(msx_state::expanded_slot_w));
	for (int i = 0; i < 4; i++)
	{
		// Ensure that the views will exist
		(*m_exp_view[slot][0])[i];
		(*m_exp_view[slot][1])[i];
		(*m_exp_view[slot][2])[i];
		(*m_exp_view[slot][3])[i];
	}
	m_slot_expanded[slot] = true;
}

void msx_state::memory_map(address_map &map)
{
	map.unmap_value_high();

	map(0x0000, 0x3fff).view(m_view_page0);
	map(0x4000, 0x7fff).view(m_view_page1);
	map(0x8000, 0xbfff).view(m_view_page2);
	map(0xc000, 0xffff).view(m_view_page3);

	// setup defaults
	for (int i = 0; i < 4; i++)
	{
		m_view_page0[i];
		m_view_page1[i];
		m_view_page2[i];
		m_view_page3[i];
	}

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
		{
			view[entry.page + i] = get_view(entry.page + i, entry.prim, entry.sec);
		}
		entry.internal_slot->install(view[0], view[1], view[2], view[3]);
	}
}

memory_view::memory_view_entry *msx_state::get_view(int page, int prim, int sec)
{
	switch (page)
	{
	case 0:
		return m_slot_expanded[prim] ? &(*m_exp_view[prim][0])[sec] : &m_view_page0[prim];
	case 1:
		return m_slot_expanded[prim] ? &(*m_exp_view[prim][1])[sec] : &m_view_page1[prim];
	case 2:
		return m_slot_expanded[prim] ? &(*m_exp_view[prim][2])[sec] : &m_view_page2[prim];
	case 3:
		return m_slot_expanded[prim] ? &(*m_exp_view[prim][3])[sec] : &m_view_page3[prim];
	}
	return nullptr;
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
	map(0xd8, 0xd9).w(FUNC(msx_state::kanji_w));
	map(0xd9, 0xd9).r(FUNC(msx_state::kanji_r));
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
	m_view_page0.select(0);
	m_view_page1.select(0);
	m_view_page2.select(0);
	m_view_page3.select(0);
	if (m_slot_expanded[0])
	{
		m_view_slot0_page0.select(0);
		m_view_slot0_page1.select(0);
		m_view_slot0_page2.select(0);
		m_view_slot0_page3.select(0);
	}
}

void msx_state::machine_start()
{
	m_leds.resolve();
	m_port_c_old = 0xff;
}

/* A hack to add 1 wait cycle in each opcode fetch.
   Possibly worth not to use custom table at all but adjust desired icount
   directly in m_opcodes.read_byte handler. */
static const u8 cc_op[0x100] = {
	4+1,10+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1, 4+1,11+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1,
	8+1,10+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1,12+1,11+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1,
	7+1,10+1,16+1, 6+1, 4+1, 4+1, 7+1, 4+1, 7+1,11+1,16+1, 6+1, 4+1, 4+1, 7+1, 4+1,
	7+1,10+1,13+1, 6+1,11+1,11+1,10+1, 4+1, 7+1,11+1,13+1, 6+1, 4+1, 4+1, 7+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	7+1, 7+1, 7+1, 7+1, 7+1, 7+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	5+1,10+1,10+1,10+1,10+1,11+1, 7+1,11+1, 5+1,10+1,10+1, 4+1,10+1,17+1, 7+1,11+1,
	5+1,10+1,10+1,11+1,10+1,11+1, 7+1,11+1, 5+1, 4+1,10+1,11+1,10+1, 4+1, 7+1,11+1,
	5+1,10+1,10+1,19+1,10+1,11+1, 7+1,11+1, 5+1, 4+1,10+1, 4+1,10+1, 4+1, 7+1,11+1,
	5+1,10+1,10+1, 4+1,10+1,11+1, 7+1,11+1, 5+1, 6+1,10+1, 4+1,10+1, 4+1, 7+1,11+1
};

static const u8 cc_cb[0x100] = {
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 8+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 8+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 8+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 8+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 8+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 8+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 8+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 8+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1
};

static const u8 cc_ed[0x100] = {
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,
	 8+1, 8+1,15+2,16+1, 4+1,14+2, 4+1, 5+1, 8+1, 8+1,15+2,16+1, 4+1,14+2, 4+1, 5+1,
	 8+1, 8+1,15+2,16+1, 4+1,14+2, 4+1, 5+1, 8+1, 8+1,15+2,16+1, 4+1,14+2, 4+1, 5+1,
	 8+1, 8+1,15+2,16+1, 4+1,14+2, 4+1,14+1, 8+1, 8+1,15+2,16+1, 4+1,14+2, 4+1,14+1,
	 8+1, 8+1,15+2,16+1, 4+1,14+2, 4+1, 4+1, 8+1, 8+1,15+2,16+1, 4+1,14+2, 4+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,
	12+1,12+1,12+1,12+1, 4+1, 4+1, 4+1, 4+1,12+1,12+1,12+1,12+1, 4+1, 4+1, 4+1, 4+1,
	12+1,12+1,12+1,12+1, 4+1, 4+1, 4+1, 4+1,12+1,12+1,12+1,12+1, 4+1, 4+1, 4+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1
};

static const u8 cc_xy[0x100] = {
	 4+1,10+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1, 4+1,11+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1,
	 8+1,10+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1,12+1,11+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1,
	 7+1,10+1,16+1, 6+1, 4+1, 4+1, 7+1, 4+1, 7+1,11+1,16+1, 6+1, 4+1, 4+1, 7+1, 4+1,
	 7+1,10+1,13+1, 6+1,19+1,19+1,15+1, 4+1, 7+1,11+1,13+1, 6+1, 4+1, 4+1, 7+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1,
	15+1,15+1,15+1,15+1,15+1,15+1, 4+1,15+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1,
	 5+1,10+1,10+1,10+1,10+1,11+1, 7+1,11+1, 5+1,10+1,10+1, 7+1,10+1,17+1, 7+1,11+1,
	 5+1,10+1,10+1,11+1,10+1,11+1, 7+1,11+1, 5+1, 4+1,10+1,11+1,10+1, 4+1, 7+1,11+1,
	 5+1,10+1,10+1,19+1,10+1,11+1, 7+1,11+1, 5+1, 4+1,10+1, 4+1,10+1, 4+1, 7+1,11+1,
	 5+1,10+1,10+1, 4+1,10+1,11+1, 7+1,11+1, 5+1, 6+1,10+1, 4+1,10+1, 4+1, 7+1,11+1
};

/* extra cycles if jr/jp/call taken and 'interrupt latency' on rst 0-7 */
static const u8 cc_ex[0x100] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* DJNZ */
	5, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, /* JR NZ/JR Z */
	5, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, /* JR NC/JR C */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	5, 5, 5, 5, 0, 0, 0, 0, 5, 5, 5, 5, 0, 0, 0, 0, /* LDIR/CPIR/INIR/OTIR LDDR/CPDR/INDR/OTDR */
	6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
	6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
	6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
	6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2+1
};

void msx_state::driver_start()
{
	m_maincpu->set_input_line_vector(0, 0xff); // Z80

	m_maincpu->z80_set_cycle_tables(cc_op, cc_cb, cc_ed, cc_xy, nullptr, cc_ex);

	save_item(NAME(m_psg_b));
	save_item(NAME(m_kanji_latch));
	save_item(NAME(m_kanji_fsa1fx));
	save_item(NAME(m_slot_expanded));
	save_item(NAME(m_primary_slot));
	save_item(NAME(m_secondary_slot));
	save_item(NAME(m_port_c_old));
	save_item(NAME(m_keylatch));
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
	// Arabic or kana mode led
	if ((data ^ m_psg_b) & 0x80)
		m_leds[1] = BIT(~data, 7);

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
	m_view_page0.select((data >> 0) & 0x03);
	m_view_page1.select((data >> 2) & 0x03);
	m_view_page2.select((data >> 4) & 0x03);
	m_view_page3.select((data >> 6) & 0x03);
}

void msx_state::ppi_port_c_w(u8 data)
{
	m_keylatch = data & 0x0f;

	// caps lock
	if (BIT(m_port_c_old ^ data, 6))
		m_leds[0] = BIT(~data, 6);

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
	m_exp_view[slot][0]->select((data >> 0) & 0x03);
	m_exp_view[slot][1]->select((data >> 2) & 0x03);
	m_exp_view[slot][2]->select((data >> 4) & 0x03);
	m_exp_view[slot][3]->select((data >> 6) & 0x03);
}

u8 msx_state::expanded_slot_r()
{
	const int slot = (m_primary_slot >> 6) & 0x03;
	return ~m_secondary_slot[slot];
}

u8 msx_state::kanji_r(offs_t offset)
{
	u8 result = 0xff;

	if (m_region_kanji)
	{
		u32 latch = m_kanji_fsa1fx ? bitswap<17>(m_kanji_latch, 4, 3, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 2, 1, 0) : m_kanji_latch;
		result = m_region_kanji->as_u8(latch);

		if (!machine().side_effects_disabled())
		{
			m_kanji_latch = (m_kanji_latch & ~0x1f) | ((m_kanji_latch + 1) & 0x1f);
		}
	}
	return result;
}

void msx_state::kanji_w(offs_t offset, u8 data)
{
	if (offset)
		m_kanji_latch = (m_kanji_latch & 0x007e0) | ((data & 0x3f) << 11);
	else
		m_kanji_latch = (m_kanji_latch & 0x1f800) | ((data & 0x3f) << 5);
}

void msx_state::msx_base(ay8910_type ay8910_type, machine_config &config, XTAL xtal, int cpu_divider)
{
	// basic machine hardware
	Z80(config, m_maincpu, xtal / cpu_divider);         // 3.579545 MHz
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
	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, m_speaker, 0.1);

	if (ay8910_type == SND_AY8910)
		AY8910(config, m_ay8910, xtal / cpu_divider / 2);
	if (ay8910_type == SND_YM2149)
		YM2149(config, m_ay8910, xtal / cpu_divider / 2);
	m_ay8910->set_flags(AY8910_SINGLE_OUTPUT);
	m_ay8910->port_a_read_callback().set(FUNC(msx2_base_state::psg_port_a_r));
	m_ay8910->port_b_read_callback().set(FUNC(msx2_base_state::psg_port_b_r));
	m_ay8910->port_a_write_callback().set(FUNC(msx2_base_state::psg_port_a_w));
	m_ay8910->port_b_write_callback().set(FUNC(msx2_base_state::psg_port_b_w));
	m_ay8910->add_route(ALL_OUTPUTS, m_speaker, 0.3);

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
		m_cassette->add_route(ALL_OUTPUTS, m_speaker, 0.05);
		m_cassette->set_interface("msx_cass");
	}
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

void msx_state::msx1(vdp_type vdp_type, ay8910_type ay8910_type, machine_config &config)
{
	msx_base(ay8910_type, config, 10.738635_MHz_XTAL, 3);

	m_maincpu->set_addrmap(AS_IO, &msx_state::msx1_io_map);

	if (vdp_type == VDP_TMS9118)
		TMS9118(config, m_tms9928a, 10.738635_MHz_XTAL);
	if (vdp_type == VDP_TMS9128)
		TMS9128(config, m_tms9928a, 10.738635_MHz_XTAL);
	if (vdp_type == VDP_TMS9129)
		TMS9129(config, m_tms9928a, 10.738635_MHz_XTAL);
	if (vdp_type == VDP_TMS9918)
		TMS9918(config, m_tms9928a, 10.738635_MHz_XTAL);
	if (vdp_type == VDP_TMS9918A)
		TMS9918A(config, m_tms9928a, 10.738635_MHz_XTAL);
	if (vdp_type == VDP_TMS9928A)
		TMS9928A(config, m_tms9928a, 10.738635_MHz_XTAL);
	if (vdp_type == VDP_TMS9929A)
		TMS9929A(config, m_tms9928a, 10.738635_MHz_XTAL);
	m_tms9928a->set_screen(m_screen);
	m_tms9928a->set_vram_size(0x4000);
	m_tms9928a->int_callback().set(m_mainirq, FUNC(input_merger_device::in_w<0>));

	// Software lists
	msx1_add_softlists(config);
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

void msx2_base_state::msx2plus_io_map(address_map &map)
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

// Some MSX2+ can switch the z80 clock between 3.5 and 5.3 MHz
WRITE_LINE_MEMBER(msx2_base_state::turbo_w)
{
	// 0 - 5.369317 MHz
	// 1 - 3.579545 MHz
	m_maincpu->set_unscaled_clock(21.477272_MHz_XTAL / (state ? 6 : 4));
}

void msx2_base_state::msx_ym2413(machine_config &config)
{
	YM2413(config, "ym2413", 21.477272_MHz_XTAL / 6).add_route(ALL_OUTPUTS, m_speaker, 0.4);
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

void msx2_base_state::msx2plus_add_softlists(machine_config &config)
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
		SOFTWARE_LIST(config, "flop_list").set_original("msx2p_flop");
		SOFTWARE_LIST(config, "msx2_flop_list").set_compatible("msx2_flop");
		SOFTWARE_LIST(config, "msx1_flop_list").set_compatible("msx1_flop");
	}
}

void msx2_base_state::turbor_add_softlists(machine_config &config)
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
		SOFTWARE_LIST(config, "flop_list").set_original("msxr_flop");
		SOFTWARE_LIST(config, "msx2p_flop_list").set_compatible("msx2p_flop");
		SOFTWARE_LIST(config, "msx2_flop_list").set_compatible("msx2_flop");
		SOFTWARE_LIST(config, "msx1_flop_list").set_compatible("msx1_flop");
	}
}

void msx2_base_state::msx2_base(ay8910_type ay8910_type, machine_config &config)
{
	msx_base(ay8910_type, config, 21.477272_MHz_XTAL, 6);

	// real time clock
	RP5C01(config, m_rtc, 32.768_kHz_XTAL);
}

void msx2_base_state::msx2(ay8910_type ay8910_type, machine_config &config)
{
	msx2_base(ay8910_type, config);

	m_maincpu->set_addrmap(AS_IO, &msx2_base_state::msx2_io_map);

	// video hardware
	V9938(config, m_v9938, 21.477272_MHz_XTAL);
	m_v9938->set_screen_ntsc(m_screen);
	m_v9938->set_vram_size(0x20000);
	m_v9938->int_cb().set(m_mainirq, FUNC(input_merger_device::in_w<0>));

	// Software lists
	msx2_add_softlists(config);
}

void msx2_base_state::msx2_pal(ay8910_type ay8910_type, machine_config &config)
{
	msx2(ay8910_type, config);
	m_v9938->set_screen_pal(m_screen);
}

void msx2_base_state::msx2plus_base(ay8910_type ay8910_type, machine_config &config)
{
	msx2_base(ay8910_type, config);

	m_maincpu->set_addrmap(AS_IO, &msx2_base_state::msx2plus_io_map);

	// video hardware
	V9958(config, m_v9958, 21.477272_MHz_XTAL);
	m_v9958->set_screen_ntsc(m_screen);
	m_v9958->set_vram_size(0x20000);
	m_v9958->int_cb().set(m_mainirq, FUNC(input_merger_device::in_w<0>));
}

void msx2_base_state::msx2plus(ay8910_type ay8910_type, machine_config &config)
{
	msx2plus_base(ay8910_type, config);

	// Software lists
	msx2plus_add_softlists(config);
}

void msx2_base_state::msx2plus_pal(ay8910_type ay8910_type, machine_config &config)
{
	msx2plus(ay8910_type, config);
	m_v9958->set_screen_pal(m_screen);
}

void msx2_base_state::turbor(ay8910_type ay8910_type, machine_config &config)
{
	msx2plus_base(ay8910_type, config);

	R800(config.replace(), m_maincpu, 28.636363_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &msx2_base_state::memory_map);
	m_maincpu->set_addrmap(AS_IO, &msx2_base_state::msx2plus_io_map);

	// Software lists
	turbor_add_softlists(config);
}
