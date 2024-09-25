// license:GPL2+
// copyright-holders:Felipe Sanches
/******************************************************************************

    Technics SX-KN5000 music keyboard driver

******************************************************************************/

#include "emu.h"
#include "cpu/tlcs900/tmp95c061.h" // TODO: tmp94c241.h
#include "imagedev/floppy.h"
#include "machine/gen_latch.h"
#include "machine/upd765.h"
#include "video/pc_vga.h"
#include "screen.h"
#include "kn5000.lh"

class mn89304_vga_device : public svga_device
{
public:
	// construction/destruction
	mn89304_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_reset() override ATTR_COLD;

	virtual void palette_update() override;
	virtual void recompute_params() override;
	virtual uint16_t offset() override;
};

DEFINE_DEVICE_TYPE(MN89304_VGA, mn89304_vga_device, "mn89304_vga", "MN89304 VGA")

// TODO: nothing is known about this, configured out of usage in here for now.
mn89304_vga_device::mn89304_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: svga_device(mconfig, MN89304_VGA, tag, owner, clock)
{
	// ...
}

void mn89304_vga_device::device_reset()
{
	svga_device::device_reset();
	svga.rgb8_en = 1;
}

// sets up mode 0, by default it will throw 155 Hz, assume divided by 3
void mn89304_vga_device::recompute_params()
{
	u8 xtal_select = (vga.miscellaneous_output & 0x0c) >> 2;
	int xtal;

	switch(xtal_select & 3)
	{
		case 0: xtal = XTAL(25'174'800).value() / 3; break;
		case 1: xtal = XTAL(28'636'363).value() / 3; break;
		case 2:
		default:
			throw emu_fatalerror("MN89304: setup ext. clock select");
	}

	recompute_params_clock(1, xtal);
}


void mn89304_vga_device::palette_update()
{
	// 4bpp RAMDAC
	for (int i = 0; i < 256; i++)
	{
		set_pen_color(
			i,
			pal4bit(vga.dac.color[3*(i & vga.dac.mask) + 0]),
			pal4bit(vga.dac.color[3*(i & vga.dac.mask) + 1]),
			pal4bit(vga.dac.color[3*(i & vga.dac.mask) + 2])
		);
	}
}

uint16_t mn89304_vga_device::offset()
{
	return svga_device::offset() << 3;
}


namespace {

class kn5000_state : public driver_device
{
public:
	kn5000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
		, m_maincpu_latch(*this, "maincpu_latch")
		, m_subcpu_latch(*this, "subcpu_latch")
		, m_fdc(*this, "fdc")
		, m_CPL_SEG(*this, "CPL_SEG%u", 0U)
		, m_CPR_SEG(*this, "CPR_SEG%u", 0U)
		, m_checking_device_led_cn11(*this, "checking_device_led_cn11")
		, m_checking_device_led_cn12(*this, "checking_device_led_cn12")
		, m_CPL_LED(*this, "CPL_%u", 0U)
		, m_CPR_LED(*this, "CPR_%u", 0U)
		, m_led_row(0)
		, m_mstat(0)
		, m_sstat(0)
	{ }

	void kn5000(machine_config &config);

private:
	required_device<tmp95c061_device> m_maincpu;
	required_device<tmp95c061_device> m_subcpu;
	required_device<generic_latch_8_device> m_maincpu_latch;
	required_device<generic_latch_8_device> m_subcpu_latch;
	required_device<upd72067_device> m_fdc;
	required_ioport_array<11> m_CPL_SEG; // buttons on "Control Panel Left" PCB
	required_ioport_array<11> m_CPR_SEG; // buttons on "Control Panel Right" PCB
	output_finder<> m_checking_device_led_cn11;
	output_finder<> m_checking_device_led_cn12;
	output_finder<50> m_CPL_LED;
	output_finder<69> m_CPR_LED;
	uint8_t m_led_row;
	uint8_t m_mstat;
	uint8_t m_sstat;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint8_t cpanel_left_buttons_r(offs_t offset);
	uint8_t cpanel_right_buttons_r(offs_t offset);
	void cpanel_leds_w(offs_t offset, uint8_t data);

	void maincpu_mem(address_map &map) ATTR_COLD;
	void subcpu_mem(address_map &map) ATTR_COLD;
};

void kn5000_state::maincpu_mem(address_map &map)
{
	map(0x000000, 0x0fffff).ram(); // 1Mbyte = 2 * 4Mbit DRAMs @ IC9, IC10 (CS3)
	map(0x008e4a, 0x008e54).r(FUNC(kn5000_state::cpanel_right_buttons_r));
	map(0x008e5a, 0x008e64).r(FUNC(kn5000_state::cpanel_left_buttons_r));
	map(0x008f38, 0x008f39).w(FUNC(kn5000_state::cpanel_leds_w));
	//FIXME: map(0x110000, 0x11ffff).m(m_fdc, FUNC(upd765a_device::map)); // Floppy Controller @ IC208
	//FIXME: map(0x120000, 0x12ffff).w(m_fdc, FUNC(upd765a_device::dack_w)); // Floppy DMA Acknowledge
	map(0x140000, 0x14ffff).r(m_maincpu_latch, FUNC(generic_latch_8_device::read)); // @ IC23
	map(0x140000, 0x14ffff).w(m_subcpu_latch, FUNC(generic_latch_8_device::write)); // @ IC22
	map(0x1703b0, 0x1703df).m("vga", FUNC(mn89304_vga_device::io_map)); // LCD controller @ IC206
	map(0x1a0000, 0x1bffff).rw("vga", FUNC(mn89304_vga_device::mem_linear_r), FUNC(mn89304_vga_device::mem_linear_w));
	map(0x1e0000, 0x1fffff).ram(); // 1Mbit SRAM @ IC21 (CS0)  Note: I think this is the message "ERROR in back-up SRAM"
	map(0x200000, 0x2fffff).noprw(); // Extension board goes here.
	map(0x300000, 0x3fffff).rom().region("custom_data", 0); // 8MBit FLASH ROM @ IC19 (CS5)
	map(0x400000, 0x7fffff).rom().region("rhythm_data", 0); // 32MBit ROM @ IC14 (A22=1 and CS5)
	map(0x800000, 0x82ffff).rom().region("subprogram", 0); // not sure yet in which chip this is stored, but I suspect it should be IC19
//  map(0xc00000, 0xdfffff).mirror(0x200000).rom().region("table_data", 0); //2 * 8MBit ROMs @ IC1, IC3 (CS2)
	map(0xe00000, 0xffffff).mask(0x1fffff).rom().region("program", 0); //2 * 8MBit FLASH ROMs @ IC4, IC6
}

void kn5000_state::subcpu_mem(address_map &map)
{
	// There seems to also be devices at 110000, 130000 and 1e0000

	map(0x000000, 0x0fffff).ram(); // 1Mbyte = 2 * 4Mbit DRAMs @ IC28, IC29
	map(0x120000, 0x12ffff).r(m_subcpu_latch, FUNC(generic_latch_8_device::read)); // @ IC22
	map(0x120000, 0x12ffff).w(m_maincpu_latch, FUNC(generic_latch_8_device::write)); // @ IC23
	//map(0x??????, 0x??????).rw(FUNC(kn5000_state::tone_generator_r), FUNC(kn5000_state::tone_generator_w)); // @ IC303
	//map(0x??????, 0x??????).rw(FUNC(kn5000_state::dsp1_r), FUNC(kn5000_state::dsp1_w)); // @ IC311

	// This is not necessarily correct.
	// Just silencing oslog messages for the subcpu while we don't have a proper ROM dump.
	map(0xfe0000, 0xffffff).rom().region("mask", 0); // 1Mbit MASK ROM @ IC30

	//Note:
	// DSP2 @ IC302 uses a serial bus
}

static void kn5000_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}

static INPUT_PORTS_START(kn5000)
	PORT_START("CN11")
	PORT_DIPNAME(0x01, 0x01, "Main CPU Checking Device")
	PORT_DIPSETTING(   0x00, DEF_STR(On))
	PORT_DIPSETTING(   0x01, DEF_STR(Off))

	PORT_START("CN12")
	PORT_DIPNAME(0x01, 0x01, "Sub CPU Checking Device")
	PORT_DIPSETTING(   0x00, DEF_STR(On))
	PORT_DIPSETTING(   0x01, DEF_STR(Off))

	PORT_START("COM_SELECT")
	PORT_DIPNAME(0xf0, 0xe0, "Computer Interface Selection")
	PORT_DIPSETTING(   0xe0, "MIDI")
	PORT_DIPSETTING(   0xd0, "PC1")
	PORT_DIPSETTING(   0xb0, "PC2")
	PORT_DIPSETTING(   0x70, "Mac")

	PORT_START("CPR_SEG0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("TRANSPOSE -")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("TRANSPOSE +")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("CPR_SEG1")  // SOUND GROUP
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ORGAN & ACCORDION")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ORCHESTRAL PAD")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SYNTH")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("BASS")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DIGITAL DRAWBAR")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ACCORDION REGISTER")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("GM SPECIAL")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DRUM KITS")

	PORT_START("CPR_SEG2")  // SOUND GROUP
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PIANO")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("GUITAR")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("STRINGS & VOCAL")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("BRASS")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("FLUTE")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SAX & REED")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("MALLET & ORCH PERC")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("WORLD PERC")

	PORT_START("CPR_SEG3")  // EFFECT
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SUSTAIN")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DIGITAL EFFECT")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DSP EFFECT")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DIGITAL REVERB")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ACOUSTIC ILLUSION")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("CPR_SEG4")  // PART SELECT
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LEFT")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RIGHT 2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RIGHT 1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ENTERTAINER")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CONDUCTOR: LEFT")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CONDUCTOR: RIGHT 2")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CONDUCTOR: RIGHT 1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("TECHNI CHORD")

	PORT_START("CPR_SEG5")  // SEQUENCER
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SEQUENCER: PLAY")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SEQUENCER: EASY REC")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SEQUENCER: MENU")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("CPR_SEG6")  // PANEL MEMORY
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PM 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PM 2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PM 3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PM 4")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PM 5")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PM 6")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PM 7")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PM 8")

	PORT_START("CPR_SEG7")  // PANEL MEMORY
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PANEL MEMORY: SET")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PANEL MEMORY: NEXT BANK")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PANEL MEMORY: BANK VIEW")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("CPR_SEG8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("R1/R2 OCTAVE -")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("R1/R2 OCTAVE +")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("START/STOP")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SYNCHRO & BREAK")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("TAP TEMPO")

	PORT_START("CPR_SEG9")  // SOUND GROUP
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("MEMORY A")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("MEMORY B")

	PORT_START("CPR_SEG10")  // MENU
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("MENU: SOUND")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("MENU: CONTROL")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("MENU: MIDI")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("MENU: DISK")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("CPL_SEG0")  // RHYTHM GROUP
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("STANDARD ROCK")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("R & ROLL & BLUES")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("POP & BALLAD")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("FUNK & FUSION")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SOUL & MODERN DANCE")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("BIG BAND & SWING")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("JAZZ COMBO")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("CPL_SEG1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("MEMORY") // Composer
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("MENU") // Composer
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SET") // Sound Arranger
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ON/OFF") // Sound Arranger
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("MUSIC STYLIST")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("FADE IN")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("FADE OUT")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("CPL_SEG2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("FILL IN 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("FILL IN 2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("INTRO & ENDING 1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("INTRO & ENDING 2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PAGE DOWN")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PAGE UP")

	PORT_START("CPL_SEG3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DEMO")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("MSP BANK")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("MSP MENU")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("MSP STOP/RECORD")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("CPL_SEG4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("VARIATION 1") // VARIATION & MSA
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("VARIATION 2") // VARIATION & MSA
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("VARIATION 3") // VARIATION & MSA
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("VARIATION 4") // VARIATION & MSA
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("MUSIC STYLE ARRANGER")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SPLIT POINT")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("AUTO PLAY CHORD")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("CPL_SEG5")  // MANUAL SEQUENCE PADS
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("MSP 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("MSP 2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("MSP 3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("MSP 4")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("MSP 5")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("MSP 6")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("CPL_SEG6")  // RHYTHM GROUP
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("U.S. TRAD")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("COUNTRY")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LATIN")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("MARCH & WALTZ")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PARTY TIME")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SHOWTIME & TRAD DANCE")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("WORLD")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CUSTOM")

	PORT_START("CPL_SEG7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RIGHT 5")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RIGHT 4")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DISPLAY HOLD")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("EXIT")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DOWN 7")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("UP 7")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DOWN 8")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("UP 8")

	PORT_START("CPL_SEG8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RIGHT 3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RIGHT 2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RIGHT 1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DOWN 5")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("UP 5")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DOWN 6")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("UP 6")

	PORT_START("CPL_SEG9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LEFT 5")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LEFT 4")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LEFT 3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DOWN 3")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("UP 3")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DOWN 4")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("UP 4")

	PORT_START("CPL_SEG10")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LEFT 2")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LEFT 1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("HELP")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("OTHER PARTS/TR")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DOWN 1")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("UP 1")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DOWN 2")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("UP 2")
INPUT_PORTS_END


uint8_t kn5000_state::cpanel_left_buttons_r(offs_t offset)
{
	return m_CPL_SEG[offset]->read();
}

uint8_t kn5000_state::cpanel_right_buttons_r(offs_t offset)
{
	return m_CPR_SEG[offset]->read();
}


void kn5000_state::cpanel_leds_w(offs_t offset, uint8_t data)
{
	if ((offset & 1) == 0)
		m_led_row = data;

	if ((offset & 1) == 1)
	{
		switch (m_led_row)
		{
			case 0x00:
				m_CPR_LED[1] = BIT(data, 0); // D101 - EFFECT: SUSTAIN
				m_CPR_LED[2] = BIT(data, 1); // D102 - EFFECT: DIGITAL EFFECT
				m_CPR_LED[3] = BIT(data, 2); // D103 - EFFECT: DSP EFFECT
				m_CPR_LED[4] = BIT(data, 3); // D104 - EFFECT: DIGITAL REVERB
				m_CPR_LED[5] = BIT(data, 4); // D105 - EFFECT: ACCOUSTIC ILLUSION
				m_CPR_LED[6] = BIT(data, 5); // D106 - SEQUENCER: PLAY
				m_CPR_LED[7] = BIT(data, 6); // D107 - SEQUENCER: EASY REC
				m_CPR_LED[8] = BIT(data, 7); // D108 - SEQUENCER: MENU
				break;

			case 0x01:
				m_CPR_LED[9] = BIT(data, 0); // D109 - PIANO
				m_CPR_LED[10] = BIT(data, 1); // D110 - GUITAR
				m_CPR_LED[11] = BIT(data, 2); // D111 - STRINGS & VOCAL
				m_CPR_LED[12] = BIT(data, 3); // D112 - BRASS
				m_CPR_LED[13] = BIT(data, 4); // D113 - FLUTE
				m_CPR_LED[14] = BIT(data, 5); // D114 - SAX & REED
				m_CPR_LED[15] = BIT(data, 6); // D115 - MALLET & ORCH PERC
				m_CPR_LED[16] = BIT(data, 7); // D116 - WORLD PERC
				break;

			case 0x02:
				m_CPR_LED[17] = BIT(data, 0); // D117 - ORGAN & ACCORDION
				m_CPR_LED[18] = BIT(data, 1); // D118 - ORCHESTRAL PAD
				m_CPR_LED[19] = BIT(data, 2); // D119 - SYNTH
				m_CPR_LED[20] = BIT(data, 3); // D120 - BASS
				m_CPR_LED[21] = BIT(data, 4); // D121 - DIGITAL DRAWBAR
				m_CPR_LED[22] = BIT(data, 5); // D122 - ACCORDION REGISTER
				m_CPR_LED[23] = BIT(data, 6); // D123 - GM SPECIAL
				m_CPR_LED[24] = BIT(data, 7); // D124 - DRUM KITS
				break;

			case 0x03:
				m_CPR_LED[25] = BIT(data, 0); // D125 - PANEL MEMORY 1
				m_CPR_LED[26] = BIT(data, 1); // D126 - PANEL MEMORY 2
				m_CPR_LED[27] = BIT(data, 2); // D127 - PANEL MEMORY 3
				m_CPR_LED[28] = BIT(data, 3); // D128 - PANEL MEMORY 4
				m_CPR_LED[29] = BIT(data, 4); // D129 - PANEL MEMORY 5
				m_CPR_LED[30] = BIT(data, 5); // D130 - PANEL MEMORY 6
				m_CPR_LED[31] = BIT(data, 6); // D131 - PANEL MEMORY 7
				m_CPR_LED[32] = BIT(data, 7); // D132 - PANEL MEMORY 8
				break;

			case 0x04:
				m_CPR_LED[33] = BIT(data, 0); // D133 - PART SELECT: LEFT
				m_CPR_LED[34] = BIT(data, 1); // D134 - PART SELECT: RIGHT 2
				m_CPR_LED[35] = BIT(data, 2); // D135 - PART SELECT: RIGHT 1
				m_CPR_LED[36] = BIT(data, 3); // D136 - ENTERTAINER
				m_CPR_LED[37] = BIT(data, 4); // D137 - CONDUCTOR: LEFT
				m_CPR_LED[38] = BIT(data, 5); // D138 - CONDUCTOR: RIGHT 2
				m_CPR_LED[39] = BIT(data, 6); // D139 - CONDUCTOR: RIGHT 1
				m_CPR_LED[40] = BIT(data, 7); // D140 - TECHNI CHORD
				break;

			case 0x08:
				m_CPR_LED[49] = BIT(data, 0); // D149 - MENU: SOUND
				m_CPR_LED[50] = BIT(data, 1); // D150 - MENU: CONTROL
				m_CPR_LED[51] = BIT(data, 2); // D151 - MENU: MIDI
				m_CPR_LED[52] = BIT(data, 3); // D152 - MENU: DISK
				break;

			case 0x0a:
				m_CPR_LED[57] = BIT(data, 0); // D157 - MEMORY A
				m_CPR_LED[58] = BIT(data, 1); // D158 - MEMORY B
				break;

			case 0x0b:
				m_CPR_LED[61] = BIT(data, 0); // D161 - SYNCHRO & BREAK
				m_CPR_LED[62] = BIT(data, 1); // D162 - R1/R2 OCTAVE MINUS
				m_CPR_LED[63] = BIT(data, 2); // D163 - R1/R2 OCTAVE PLUS
				m_CPR_LED[64] = BIT(data, 3); // D164 - BANK VIEW
				break;

			case 0x0c:
				m_CPR_LED[65] = BIT(data, 0); // D165 - START/STOP 1 BEAT
				m_CPR_LED[66] = BIT(data, 1); // D166 - START/STOP 2 BEAT
				m_CPR_LED[67] = BIT(data, 2); // D167 - START/STOP 3 BEAT
				m_CPR_LED[68] = BIT(data, 3); // D168 - START/STOP 4 BEAT
				break;

			case 0xc0:
				m_CPL_LED[1] = BIT(data, 0); // D101 - COMPOSER: MEMORY
				m_CPL_LED[2] = BIT(data, 1); // D102 - COMPOSER: MENU
				m_CPL_LED[3] = BIT(data, 2); // D103 - SOUND ARRANGER: SET
				m_CPL_LED[4] = BIT(data, 3); // D104 - SOUND ARRANGER: ON/OFF
				m_CPL_LED[5] = BIT(data, 4); // D105 - MUSIC STYLIST
				m_CPL_LED[6] = BIT(data, 5); // D106 - FADE IN
				m_CPL_LED[7] = BIT(data, 6); // D107 - FADE OUT
				m_CPL_LED[8] = BIT(data, 7); // D108 - DISPLAY HOLD
				break;

			case 0xc1:
				m_CPL_LED[9] = BIT(data, 0); // D109 - U.S. TRAD
				m_CPL_LED[10] = BIT(data, 1); // D110 - COUNTRY
				m_CPL_LED[11] = BIT(data, 2); // D111 - LATIN
				m_CPL_LED[12] = BIT(data, 3); // D112 - MARCH & WALTZ
				m_CPL_LED[13] = BIT(data, 4); // D113 - PARTY TIME
				m_CPL_LED[14] = BIT(data, 5); // D114 - SHOW TIME & TRAD DANCE
				m_CPL_LED[15] = BIT(data, 6); // D115 - WORLD
				m_CPL_LED[16] = BIT(data, 7); // D116 - CUSTOM
				break;

			case 0xc2:
				m_CPL_LED[17] = BIT(data, 0); // D117 - STANDARD ROCK
				m_CPL_LED[18] = BIT(data, 1); // D118 - R & ROLL & BLUES
				m_CPL_LED[19] = BIT(data, 2); // D119 - POP & BALLAD
				m_CPL_LED[20] = BIT(data, 3); // D120 - FUNK & FUSION
				m_CPL_LED[21] = BIT(data, 4); // D121 - SOUL & MODERN DANCE
				m_CPL_LED[22] = BIT(data, 5); // D122 - BIG BAND & SWING
				m_CPL_LED[23] = BIT(data, 6); // D123 - JAZZ COMBO
				m_CPL_LED[24] = BIT(data, 7); // D124 - MANUAL SEQUENCE PADS: MENU
				break;

			case 0xc3:
				m_CPL_LED[25] = BIT(data, 0); // D125 - VARIATION & MSA 1
				m_CPL_LED[26] = BIT(data, 1); // D126 - VARIATION & MSA 2
				m_CPL_LED[27] = BIT(data, 2); // D127 - VARIATION & MSA 3
				m_CPL_LED[28] = BIT(data, 3); // D128 - VARIATION & MSA 4
				m_CPL_LED[29] = BIT(data, 4); // D129 - MUSIC STYLE ARRANGER
				m_CPL_LED[30] = BIT(data, 5); // D130 - AUTO PLAY CHORD
				break;

			case 0xc4:
				m_CPL_LED[33] = BIT(data, 0); // D133 - FILL IN 1
				m_CPL_LED[34] = BIT(data, 1); // D134 - FILL IN 2
				m_CPL_LED[35] = BIT(data, 2); // D135 - INTRO & ENDING 1
				m_CPL_LED[36] = BIT(data, 3); // D136 - INTRO & ENDING 2
				m_CPL_LED[37] = BIT(data, 4); // D137 - SPLIT POINT INDICATOR (LEFT)
				m_CPL_LED[38] = BIT(data, 5); // D138 - SPLIT POINT INDICATOR (CENTER)
				m_CPL_LED[39] = BIT(data, 6); // D139 - SPLIT POINT INDICATOR (RIGHT)
				m_CPL_LED[40] = BIT(data, 7); // D140 - TEMPO/PROGRAM
				break;

			case 0xc8:
				m_CPL_LED[49] = BIT(data, 0); // D149 - OTHER PARTS/TR
				break;

			case 0xff:
				break;
		}
	}
	return;
}

void kn5000_state::machine_start()
{
	save_item(NAME(m_mstat));
	save_item(NAME(m_sstat));

	m_checking_device_led_cn11.resolve();
	m_checking_device_led_cn12.resolve();
	m_CPL_LED.resolve();
	m_CPR_LED.resolve();
}

void kn5000_state::machine_reset()
{
	m_checking_device_led_cn11 = 0;
	m_checking_device_led_cn12 = 0;
}

void kn5000_state::kn5000(machine_config &config)
{
	// Note: The CPU has an internal clock doubler
	TMP95C061(config, m_maincpu, 2 * 8_MHz_XTAL); // actual cpu is TMP94C241F @ IC5
	// Address bus is set to 32 bits by the pins AM1=+5v and AM0=GND
	m_maincpu->set_addrmap(AS_PROGRAM, &kn5000_state::maincpu_mem);
	// Interrupt 4: FDCINT
	// Interrupt 5: FDCIRQ
	// Interrupt 6: FDC.H/D
	// Interrupt 7: FDC.I/O
	// Interrupt 9: HDDINT
	// Interrupt A <edge>: ~CPSCK "Control Panel Serial Clock"
	// ~NMI: SNS
	// TC0: FDCTC


	// MAINCPU PORT 7:
	//   bit 5 (~BUSRQ pin): RY/~BY pin of maincpu ROMs
	m_maincpu->port7_read().set([] { return (1 << 5); }); // checked at EF3735 (v10 ROM)


	// MAINCPU PORT 8:
	//   bit 6 (~WAIT pin) (input): Something involving VGA.RDY, FDC.DMAACK
	//                              and shift-register @ IC18


	// MAINCPU PORT A:
	//   bit 0 (output) = sub_cpu ~RESET / SRST
	m_maincpu->porta_write().set([this] (u8 data) {
		m_subcpu->set_input_line(INPUT_LINE_RESET, BIT(data, 0) ? ASSERT_LINE : CLEAR_LINE);
	});

	// MAINCPU PORT C:
	//   bit 0 (input) = "check terminal" switch
	//   bit 1 (output) = "check terminal" LED
	// TODO: m_maincpu->portc_read().set([this] { return ioport("CN11")->read(); });
	// TODO: m_maincpu->portc_write().set([this] (u8 data) { m_checking_device_led_cn11 = (BIT(data, 1) == 0); });


	// MAINCPU PORT D:
	//   bit 0 (output) = FDCRST
	//   bit 6 (input) = FD.I/O
	// TODO: m_maincpu->portd_write().set([this] (u8 data) { m_fdc->reset_w(BIT(data, 0)); });
	// TODO: bit 6!


	// MAINCPU PORT E:
	//   bit 0 (input) = +5v
	//   bit 2 (input) = HDDRDY
	//   bit 4 (?) = MICSNS
	// TODO: m_maincpu->porte_read().set([] { return 1; }); //checked at EF05A6 (v10 ROM)
	// FIXME: Bit 0 should only be 1 if the
	// optional hard-drive extension board is disabled;


	// MAINCPU PORT F:
	//   bit 2 (OUTPUT) = Something related to "RESET CONTROL" circuits?


	// MAINCPU PORT G:
	//   bit 2 (input) = FS1  (Foot Switches and Foot Controler ?)
	//   bit 3 (input) = FS2
	//   bit 4 (input) = FC1
	//   bit 5 (input) = FC2
	//   bit 6 (input) = FC3
	//   bit 7 (input) = FC4


	// MAINCPU PORT H:
	//   bit 1 = TC1 Terminal count - microDMA
	// TODO: m_maincpu->porth_read().set([] { return 2; }); // area/region detection: checked at EF083E (v10 ROM)
	// FIXME: These are resistors on the pcb, but could be declared
	// in the driver as a 2 bit DIP-Switch for area/region selection.


	// MAINCPU PORT Z:
	//   bit 0 = (output) MSTAT0
	//   bit 1 = (output) MSTAT1
	//   bit 2 = (input) SSTAT0
	//   bit 3 = (input) SSTAT1
	//   bit 4 = (input) COM.PC2
	//   bit 5 = (input) COM.PC1
	//   bit 6 = (input) COM.MAC
	//   bit 7 = (input) COM.MIDI
	// TODO: m_maincpu->portz_read().set([this] {
	// TODO:    return ioport("COM_SELECT")->read() | (m_sstat << 2);
	// TODO: });
	// TODO: m_maincpu->portz_write().set([this] (u8 data) {
	// TODO:    m_mstat = data & 3;
	// TODO: });


	// RX0/TX0 = MRXD/MTXD
	// RX1/TX1 = CPDATA
	// SCLK1   = CPSCK

	// AN0 = EXP (expression pedal?)
	// AN1 = AFT

	// Note: The CPU has an internal clock doubler
	TMP95C061(config, m_subcpu, 2*10_MHz_XTAL); // actual cpu is TMP94C241F @ IC27
	// Address bus is set to 8 bits by the pins AM1=GND and AM0=GND
	m_subcpu->set_addrmap(AS_PROGRAM, &kn5000_state::subcpu_mem);

	// SUBCPU PORT C:
	//   bit 0 (input) = "check terminal" switch
	//   bit 1 (output) = "check terminal" LED
	// TODO: m_subcpu->portc_read().set([this] { return ioport("CN12")->read(); });
	// TODO: m_subcpu->portc_write().set([this] (u8 data) { m_checking_device_led_cn12 = (BIT(data, 1) == 0); });

	// SUBCPU PORT D:
	//   bit 0 = (output) SSTAT0
	//   bit 1 = (output) SSTAT1
	//   bit 2 = (input) MSTAT0
	//   bit 3 (not used)
	//   bit 4 = (input) MSTAT1
	// TODO: m_subcpu->portd_read().set([this] {
	// TODO:    return (BIT(m_mstat, 0) << 2) | (BIT(m_mstat, 1) << 4);
	// TODO: });
	// TODO: m_subcpu->portd_write().set([this] (u8 data) {
	// TODO:    m_sstat = data & 3;
	// TODO: });


	GENERIC_LATCH_8(config, m_maincpu_latch); // @ IC23
	m_maincpu_latch->data_pending_callback().set_inputline(m_maincpu, TLCS900_INT0);

	GENERIC_LATCH_8(config, m_subcpu_latch); //  @ IC22
	m_subcpu_latch->data_pending_callback().set_inputline(m_subcpu, TLCS900_INT0);

	UPD72067(config, m_fdc, 32'000'000); // actual controller is UPD72068GF-3B9 at IC208
	m_fdc->intrq_wr_callback().set_inputline(m_maincpu, TLCS900_INT4);
	m_fdc->drq_wr_callback().set_inputline(m_maincpu, TLCS900_INT5);
	m_fdc->hdl_wr_callback().set_inputline(m_maincpu, TLCS900_INT6);
	//m_fdc->??_wr_callback().set_inputline(m_maincpu, TLCS900_INT7);
	//FIXME:
	// Interrupt 4: FDCINT
	// Interrupt 5: FDCIRQ
	// Interrupt 6: FDC.H/D
	// Interrupt 7: FDC.I/O

	FLOPPY_CONNECTOR(config, "fdc:0", kn5000_floppies, "35dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	/* video hardware */
	// LCD Controller MN89304 @ IC206 24_MHz_XTAL
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_raw(XTAL(40'000'000)/6, 424, 0, 320, 262, 0, 240);
	screen.set_screen_update("vga", FUNC(mn89304_vga_device::screen_update));

	mn89304_vga_device &vga(MN89304_VGA(config, "vga", 0));
	vga.set_screen("screen");
	vga.set_vram_size(0x100000);

	config.set_default_layout(layout_kn5000);
}

ROM_START(kn5000)
	ROM_DEFAULT_BIOS("v10")
	ROM_SYSTEM_BIOS(0, "v10", "Version 10 - August 2nd, 1999")
	ROM_SYSTEM_BIOS(1, "v9", "Version 9 - January 26th, 1999")
	ROM_SYSTEM_BIOS(2, "v8", "Version 8 - November 13th, 1998")
	ROM_SYSTEM_BIOS(3, "v7", "Version 7 - June 26th, 1998")
	ROM_SYSTEM_BIOS(4, "v6", "Version 6 - January 16th, 1998") // sometimes refered to as "update6v0"
	ROM_SYSTEM_BIOS(5, "v5", "Version 5 - November 12th, 1997") // sometimes refered to as "update5v0"
	ROM_SYSTEM_BIOS(6, "v4", "Version 4") // I have a v4 board but haven't dumped it yet
	ROM_SYSTEM_BIOS(7, "v3", "Version 3") // I have a v3 board but haven't dumped it yet

	ROM_REGION16_LE(0x200000, "program" , 0) // main cpu

	// FIXME: These are actually stored in a couple flash rom chips IC6 (even) and IC4 (odd)
	//
	// Note: These ROMs from v5 to v10 were extracted from the system update floppies
	//       which were compressed using LZSS.
	//
	//       System update disks for older versions were not found yet, so dumping
	//       efforts will require other methods.
	//
	//       More info at:
	//       https://github.com/felipesanches/kn5000_homebrew/blob/main/kn5000_extract.py

	ROMX_LOAD("kn5000_v10_program.rom", 0x00000, 0x200000, CRC(00303406) SHA1(1f2abc5b1b7b9e16fdf796f26d939edaceded354), ROM_BIOS(0))
	ROMX_LOAD("kn5000_v9_program.rom",  0x00000, 0x200000, CRC(c791d765) SHA1(d9a3b462b1f9302402e8d37aacd15f069f56abd9), ROM_BIOS(1))
	ROMX_LOAD("kn5000_v8_program.rom",  0x00000, 0x200000, CRC(46b4b242) SHA1(a10a6f5a35175b74c3cfb42cef3bdf571c2858bb), ROM_BIOS(2))
	ROMX_LOAD("kn5000_v7_program.rom",  0x00000, 0x200000, CRC(a5a25eb0) SHA1(4c682cb248034a2de04c688b0a45654b8726bffb), ROM_BIOS(3))
	ROMX_LOAD("kn5000_v6_program.rom",  0x00000, 0x200000, CRC(0205db30) SHA1(51108e2d75b180a034395e90bd40ca2bd2a0adfb), ROM_BIOS(4))
	ROMX_LOAD("kn5000_v5_program.rom",  0x00000, 0x200000, CRC(fbd035e3) SHA1(7b69a8aaa84ee3d337acc0c29c34154c5da2df32), ROM_BIOS(5))
	ROMX_LOAD("kn5000_v4_program.rom",  0x00000, 0x200000, NO_DUMP, ROM_BIOS(6))
	ROMX_LOAD("kn5000_v3_program.rom",  0x00000, 0x200000, NO_DUMP, ROM_BIOS(7))

	// Note: I've never seen boards with versions 1 or 2.

	// Note: Even though this "subprogram" address range contain executable code for the subcpu, it is actually loaded by the maincpu
	//       from a flash rom and then transfered to the subcpu RAM via the inter-cpu communications latches at some point during boot.
	ROM_REGION16_LE(0x30000, "subprogram", 0)
	ROMX_LOAD("kn5000_subprogram_v142.rom", 0x000000, 0x030000, CRC(fe3b640a) SHA1(5c3a2b9311318c19e1a29ca460dea693bcb2c405), ROM_BIOS(0)) // v10
	ROMX_LOAD("kn5000_subprogram_v142.rom", 0x000000, 0x030000, CRC(fe3b640a) SHA1(5c3a2b9311318c19e1a29ca460dea693bcb2c405), ROM_BIOS(1)) // v9
	ROMX_LOAD("kn5000_subprogram_v141.rom", 0x000000, 0x030000, CRC(4f6ea155) SHA1(39b0dd7b23abd3cdfedce65dd4fef0e2ab16ab69), ROM_BIOS(2)) // v8
	ROMX_LOAD("kn5000_subprogram_v141.rom", 0x000000, 0x030000, CRC(4f6ea155) SHA1(39b0dd7b23abd3cdfedce65dd4fef0e2ab16ab69), ROM_BIOS(3)) // v7
	ROMX_LOAD("kn5000_subprogram_v140.rom", 0x000000, 0x030000, CRC(d9a537aa) SHA1(b7f471522ab3125e5eb42c7368d57a56084ce32a), ROM_BIOS(4)) // v6
	ROMX_LOAD("kn5000_subprogram_v140.rom", 0x000000, 0x030000, CRC(d9a537aa) SHA1(b7f471522ab3125e5eb42c7368d57a56084ce32a), ROM_BIOS(5)) // v5
	ROMX_LOAD("kn5000_subprogram_v139.rom", 0x000000, 0x030000, NO_DUMP, ROM_BIOS(6)) // v4

	ROM_REGION16_LE(0x20000, "mask", 0) // subcpu boot rom
	ROM_LOAD("kn5000_mask_rom.ic30", 0x00000, 0x20000, NO_DUMP)
	// hack to keep the CPU from touching SFRs arbitrarily while we do not have a proper ROM dump:
	ROM_FILL(0x000000, 1, 0x68) // 68 fe = infinite loop
	ROM_FILL(0x000001, 1, 0xfe)
	ROM_FILL(0x01ff00, 1, 0x00) // RESET vector = 0x00fe0000
	ROM_FILL(0x01ff01, 1, 0x00)
	ROM_FILL(0x01ff02, 1, 0xfe)
	ROM_FILL(0x01ff03, 1, 0x00)

	ROM_REGION16_LE(0x200000, "table_data", 0)
	ROM_LOAD16_BYTE("kn5000_table_data_rom_even.ic3", 0x000000, 0x100000, NO_DUMP)
	ROM_LOAD16_BYTE("kn5000_table_data_rom_odd.ic1",  0x000001, 0x100000, CRC(cd907eac) SHA1(bedf09d606d476f3e6d03e590709715304cf7ea5))

	ROM_REGION16_LE(0x100000, "custom_data", 0)
	ROM_LOAD("kn5000_custom_data_rom.ic19", 0x000000, 0x100000, CRC(5de11a6b) SHA1(4709f815d3d03ce749c51f4af78c62bf4a5e3d94))
	// IC19 is a flash ROM. The contents here were dumped from a system that had it already programmed by the initial data disk.
	// Maybe it could also be declared as NVRAM here?

	ROM_REGION16_LE(0x400000, "rhythm_data", 0)
	ROM_LOAD("kn5000_rhythm_data_rom.ic14", 0x000000, 0x400000, CRC(76d11a5e) SHA1(e4b572d318c9fe7ba00e5b44ea783e89da9c68bd))

	ROM_REGION16_LE(0x1000000, "waveform", 0)
	ROM_LOAD("kn5000_waveform_rom.ic304", 0x000000, 0x400000, NO_DUMP)
	ROM_LOAD("kn5000_waveform_rom.ic305", 0x400000, 0x400000, NO_DUMP)
	ROM_LOAD("kn5000_waveform_rom.ic306", 0x800000, 0x400000, NO_DUMP)
	ROM_LOAD("kn5000_waveform_rom.ic307", 0xc00000, 0x400000, CRC(20ff4629) SHA1(4b511bff6625f4655cabd96a263bf548d2ef4bf7))
ROM_END

} // anonymous namespace

//   YEAR  NAME   PARENT  COMPAT  MACHINE INPUT   STATE         INIT        COMPANY      FULLNAME             FLAGS
CONS(1998, kn5000,    0,       0, kn5000, kn5000, kn5000_state, empty_init, "Technics", "SX-KN5000", MACHINE_NOT_WORKING|MACHINE_NO_SOUND)
