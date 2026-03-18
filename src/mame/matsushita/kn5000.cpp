// license:GPL2+
// copyright-holders:Felipe Sanches
/******************************************************************************

    Technics SX-KN5000 music keyboard driver

******************************************************************************/

#include "emu.h"
#include "bus/technics/kn5000/hdae5000.h"
#include "cpu/tlcs900/tmp94c241.h"
#include "cpu/tlcs900/tmp94c241_serial.h"
#include "imagedev/floppy.h"
#include "machine/gen_latch.h"
#include "machine/upd765.h"
#include "video/pc_vga.h"
#include "screen.h"
#include "kn5000.lh"
#include "kn5000_cpanel.h"

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
		, m_cpanel(*this, "cpanel")
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
		, m_maincpu_latch(*this, "maincpu_latch")
		, m_subcpu_latch(*this, "subcpu_latch")
		, m_fdc(*this, "fdc")
		, m_com_select(*this, "COM_SELECT")
		, m_extension(*this, "extension")
		, m_CPL_SEG(*this, "CPL_SEG%u", 0U)
		, m_CPR_SEG(*this, "CPR_SEG%u", 0U)
		, m_checking_device_led_cn11(*this, "checking_device_led_cn11")
		, m_checking_device_led_cn12(*this, "checking_device_led_cn12")
		, m_mstat(0)
		, m_sstat(0)
		, m_cpanel_inta(0)
	{ }

	void kn5000(machine_config &config);

private:
	required_device<kn5000_cpanel_device> m_cpanel;
	required_device<tmp94c241_device> m_maincpu;
	required_device<tmp94c241_device> m_subcpu;
	required_device<generic_latch_8_device> m_maincpu_latch;
	required_device<generic_latch_8_device> m_subcpu_latch;
	required_device<upd72067_device> m_fdc;
	required_ioport m_com_select;
	required_device<kn5000_extension_connector> m_extension;

	required_ioport_array<11> m_CPL_SEG; // buttons on "Control Panel Left" PCB
	required_ioport_array<11> m_CPR_SEG; // buttons on "Control Panel Right" PCB
	output_finder<> m_checking_device_led_cn11;
	output_finder<> m_checking_device_led_cn12;
	uint8_t m_mstat;
	uint8_t m_sstat;
	uint8_t m_cpanel_inta;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void maincpu_mem(address_map &map) ATTR_COLD;
	void subcpu_mem(address_map &map) ATTR_COLD;
};

void kn5000_state::maincpu_mem(address_map &map)
{
	map(0x000000, 0x0fffff).ram(); // 1Mbyte = 2 * 4Mbit DRAMs @ IC9, IC10 (CS3)
	// Button states and LED control are handled via serial protocol to cpanel HLE device
	//FIXME: map(0x110000, 0x11ffff).m(m_fdc, FUNC(upd765a_device::map)); // Floppy Controller @ IC208
	//FIXME: map(0x120000, 0x12ffff).w(m_fdc, FUNC(upd765a_device::dack_w)); // Floppy DMA Acknowledge
	map(0x140000, 0x14ffff).r(m_maincpu_latch, FUNC(generic_latch_8_device::read)); // @ IC23
	map(0x140000, 0x14ffff).w(m_subcpu_latch, FUNC(generic_latch_8_device::write)); // @ IC22
	map(0x1703b0, 0x1703df).m("vga", FUNC(mn89304_vga_device::io_map)); // LCD controller @ IC206
	map(0x1a0000, 0x1dffff).rw("vga", FUNC(mn89304_vga_device::mem_linear_r), FUNC(mn89304_vga_device::mem_linear_w));
	map(0x1e0000, 0x1fffff).ram(); // 1Mbit SRAM @ IC21 (CS0)  Note: I think this is the message "ERROR in back-up SRAM"
	map(0x300000, 0x3fffff).rom().region("custom_data", 0); // 8MBit FLASH ROM @ IC19 (CS5)
	map(0x400000, 0x7fffff).rom().region("rhythm_data", 0); // 32MBit ROM @ IC14 (A22=1 and CS5)
	//map(0x800000, 0x82ffff).rom().region("subprogram", 0); // not sure yet in which chip this is stored, but I suspect it should be IC19
	map(0x800000, 0x9fffff).mirror(0x200000).rom().region("table_data", 0); //2 * 8MBit ROMs @ IC1, IC3 (CS2)
	map(0xe00000, 0xffffff).mask(0x1fffff).rom().region("program", 0); //2 * 8MBit FLASH ROMs @ IC4, IC6
}

void kn5000_state::subcpu_mem(address_map &map)
{
	map(0x000000, 0x0fffff).ram(); // 1Mbyte = 2 * 4Mbit DRAMs @ IC28, IC29
	map(0x100000, 0x100003).noprw(); // Tone generator @ IC303 (stub)
	map(0x110000, 0x110003).noprw(); // Tone generator keybed data/status (stub)
	map(0x120000, 0x12ffff).r(m_subcpu_latch, FUNC(generic_latch_8_device::read)); // @ IC22
	map(0x120000, 0x12ffff).w(m_maincpu_latch, FUNC(generic_latch_8_device::write)); // @ IC23
	map(0x130000, 0x130003).noprw(); // DSP1 @ IC311 (stub)
	map(0x1e0000, 0x1effff).noprw(); // Waveform/sample RAM (stub)
	map(0xfe0000, 0xffffff).rom().region("subcpu", 0); // 1Mbit MASK ROM @ IC30

	// DSP2 @ IC310 (MN19413) uses GPIO serial: PF.0=SDA, PF.2=SCLK, PE.6=CS2
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

	PORT_START("AREA")
	PORT_DIPNAME(0x06, 0x06, "Area Selection")
	PORT_DIPSETTING(   0x02, "Thailand, Indonesia, Iran, U.A.E., Panama, Argentina, Peru, Brazil")
	PORT_DIPSETTING(   0x04, "USA, Mexico")
	PORT_DIPSETTING(   0x06, "Other")

/*
    Actual full list of regions (but it is unclear if there's any
    other hardware difference among them):

    PORT_DIPSETTING(   0x04, "(M): U.S.A.")
    PORT_DIPSETTING(   0x06, "(MC): Canada")
    PORT_DIPSETTING(   0x04, "(XM): Mexico")
    PORT_DIPSETTING(   0x06, "(EN): Norway, Sweden, Denmark, Finland")
    PORT_DIPSETTING(   0x06, "(EH): Holland, Belgium")
    PORT_DIPSETTING(   0x06, "(EF): France, Italy")
    PORT_DIPSETTING(   0x06, "(EZ): Germany")
    PORT_DIPSETTING(   0x06, "(EW): Switzerland")
    PORT_DIPSETTING(   0x06, "(EA): Austria")
    PORT_DIPSETTING(   0x06, "(EP): Spain, Portugal, Greece, South Africa")
    PORT_DIPSETTING(   0x06, "(EK): United Kingdom")
    PORT_DIPSETTING(   0x06, "(XL): New Zealand")
    PORT_DIPSETTING(   0x06, "(XR): Australia")
    PORT_DIPSETTING(   0x06, "(XS): Malaysia")
    PORT_DIPSETTING(   0x06, "(MD): Saudi Arabia, Hong Kong, Kuwait")
    PORT_DIPSETTING(   0x06, "(XT): Taiwan")
    PORT_DIPSETTING(   0x02, "(X): Thailand, Indonesia, Iran, U.A.E., Panama, Argentina, Peru, Brazil")
    PORT_DIPSETTING(   0x06, "(XP): Philippines")
    PORT_DIPSETTING(   0x06, "(XW): Singapore")
*/

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



void kn5000_state::machine_start()
{
	save_item(NAME(m_mstat));
	save_item(NAME(m_sstat));
	save_item(NAME(m_cpanel_inta));

	m_extension->program_map(m_maincpu->space(AS_PROGRAM));

	m_checking_device_led_cn11.resolve();
	m_checking_device_led_cn12.resolve();

	// Connect button input ports to control panel HLE device
	for (int i = 0; i < 11; i++)
	{
		m_cpanel->set_cpl_port(i, m_CPL_SEG[i].target());
		m_cpanel->set_cpr_port(i, m_CPR_SEG[i].target());
	}
}

void kn5000_state::machine_reset()
{
	m_checking_device_led_cn11 = 0;
	m_checking_device_led_cn12 = 0;
}

void kn5000_state::kn5000(machine_config &config)
{
	// Note: The CPU has an internal clock doubler
	TMP94C241(config, m_maincpu, 2 * 8_MHz_XTAL); // TMP94C241F @ IC5
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
	m_maincpu->port7_read().set_constant(1 << 5); // checked at EF3735 (v10 ROM)


	// MAINCPU PORT 8:
	//   bit 6 (~WAIT pin) (input): Something involving VGA.RDY, FDC.DMAACK
	//                              and shift-register @ IC18


	// MAINCPU PORT A:
	//   bit 0 (output) = sub_cpu ~RESET / SRST
	m_maincpu->porta_write().set([this] (u8 data) {
		m_subcpu->set_input_line(INPUT_LINE_RESET, BIT(data, 0) ? CLEAR_LINE : ASSERT_LINE);
	});

	// MAINCPU PORT C:
	//   bit 0 (input) = "check terminal" switch
	//   bit 1 (output) = "check terminal" LED
	m_maincpu->portc_read().set_ioport("CN11");
	m_maincpu->portc_write().set([this] (u8 data) {
		m_checking_device_led_cn11 = (BIT(data, 1) == 0);
	});


	// MAINCPU PORT D:
	//   bit 0 (output) = FDCRST
	//   bit 6 (input) = FD.I/O
	m_maincpu->portd_write().set(m_fdc, FUNC(upd72067_device::reset_w)).bit(0);
	// TODO: bit 6!


	// MAINCPU PORT E:
	//   bit 0 (input) = +5v
	//   bit 2 (input) = HDDRDY
	//   bit 4 (?) = MICSNS
	//   bit 5 (input) = INTA (control panel interrupt)
	m_maincpu->porte_read().set([this] {
		// Bit 0: +5v (always 1 when no HDD extension)
		// Bit 5: INTA from control panel (active HIGH — firmware checks BIT 5,(PE); JR NZ)
		return 0x01 | (m_cpanel_inta ? 0x20 : 0x00);
	});


	// MAINCPU PORT F: shared with serial interface pins
	//   bit 0 = TXD0 (MIDI TX)
	//   bit 1 = RXD0 (MIDI RX)
	//   bit 2 = SCLK0 (disabled by firmware — MIDI uses no clock)
	//   bit 4 = TXD1 (control panel data)
	//   bit 5 = RXD1 (control panel data)
	//   bit 6 (input) = SCLK1 pin state — a routine in the main CPU's
	//     implementation of the control panel protocol polls this to confirm
	//     the serial clock is idle (HIGH) before sending commands.
	//     Without it, the firmware times out after 200 retries and displays
	//     "ERROR in CPU data transmission".
	m_maincpu->portf_read().set_constant(0x40);


	// MAINCPU PORT G:
	//   bit 2 (input) = FS1  (Foot Switches and Foot Controler ?)
	//   bit 3 (input) = FS2
	//   bit 4 (input) = FC1
	//   bit 5 (input) = FC2
	//   bit 6 (input) = FC3
	//   bit 7 (input) = FC4


	// MAINCPU PORT H:
	m_maincpu->porth_read().set_ioport("AREA"); // checked at EF083E (v10 ROM)


	// MAINCPU PORT Z:
	//   bit 0 = (output) MSTAT0
	//   bit 1 = (output) MSTAT1
	//   bit 2 = (input) SSTAT0
	//   bit 3 = (input) SSTAT1
	//   bit 4 = (input) COM.PC2
	//   bit 5 = (input) COM.PC1
	//   bit 6 = (input) COM.MAC
	//   bit 7 = (input) COM.MIDI
	m_maincpu->portz_read().set([this] {
		return m_com_select->read() | (m_sstat << 2);
	});
	m_maincpu->portz_write().set([this] (u8 data) {
		m_mstat = data & 3;
	});


	// RX0/TX0 = MRXD/MTXD

	// RX1/TX1 = CPDATA, SCLK1 = CPSCK — wired to control panel HLE
	auto &cpanel(KN5000_CPANEL(config, "cpanel"));
	m_maincpu->m_serial[1].lookup()->txd().set(cpanel, FUNC(kn5000_cpanel_device::rxd));
	m_maincpu->m_serial[1].lookup()->sclk_out().set(cpanel, FUNC(kn5000_cpanel_device::sioclk));
	m_maincpu->m_serial[1].lookup()->tx_start().set(cpanel, FUNC(kn5000_cpanel_device::tx_start));
	cpanel.txd().set(m_maincpu->m_serial[1], FUNC(tmp94c241_serial_device::rxd));
	cpanel.sclk_out().set(m_maincpu->m_serial[1], FUNC(tmp94c241_serial_device::sioclk));
	cpanel.inta().set([this] (int state) {
		m_cpanel_inta = state;
		m_maincpu->set_input_line(TLCS900_INTA, state ? ASSERT_LINE : CLEAR_LINE);
	});

	// AN0 = EXP (expression pedal?)
	// AN1 = AFT

	// Note: The CPU has an internal clock doubler
	TMP94C241(config, m_subcpu, 2*10_MHz_XTAL); // TMP94C241F @ IC27
	// Address bus is set to 8 bits by the pins AM1=GND and AM0=GND
	m_subcpu->set_addrmap(AS_PROGRAM, &kn5000_state::subcpu_mem);

	// SUBCPU PORT C:
	//   bit 0 (input) = "check terminal" switch
	//   bit 1 (output) = "check terminal" LED
	m_subcpu->portc_read().set_ioport("CN12");
	m_subcpu->portc_write().set([this] (u8 data) {
		m_checking_device_led_cn12 = (BIT(data, 1) == 0);
	});


	// SUBCPU PORT D:
	//   bit 0 = (output) SSTAT0
	//   bit 1 = (output) SSTAT1
	//   bit 2 = (input) MSTAT0
	//   bit 3 (not used)
	//   bit 4 = (input) MSTAT1
	m_subcpu->portd_read().set([this] {
		return (BIT(m_mstat, 0) << 2) | (BIT(m_mstat, 1) << 4);
	});
	m_subcpu->portd_write().set([this] (u8 data) {
		m_sstat = data & 3;
	});


	GENERIC_LATCH_8(config, m_maincpu_latch); // @ IC23
	m_maincpu_latch->data_pending_callback().set_inputline(m_maincpu, TLCS900_INT0);

	GENERIC_LATCH_8(config, m_subcpu_latch); //  @ IC22
	m_subcpu_latch->data_pending_callback().set_inputline(m_subcpu, TLCS900_INT0);

	UPD72067(config, m_fdc, 32'000'000); // actual controller is UPD72068GF-3B9 at IC208
	m_fdc->intrq_wr_callback().set_inputline(m_maincpu, TLCS900_INT4);
	m_fdc->drq_wr_callback().set_inputline(m_maincpu, TLCS900_INT5);
	m_fdc->hdl_wr_callback().set_inputline(m_maincpu, TLCS900_INT6);
	// TODO: tc coming from maincpu TC0 signal
	//m_fdc->??_wr_callback().set_inputline(m_maincpu, TLCS900_INT7);
	//FIXME:
	// Interrupt 4: FDCINT
	// Interrupt 5: FDCIRQ
	// Interrupt 6: FDC.H/D
	// Interrupt 7: FDC.I/O

	FLOPPY_CONNECTOR(config, "fdc:0", kn5000_floppies, "35dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	/* Extension port */
	KN5000_EXTENSION(config, m_extension, kn5000_extension_intf, nullptr);
	m_extension->irq_callback().set_inputline(m_maincpu, TLCS900_INT9);

	/* video hardware */
	// LCD Controller MN89304 @ IC206 24_MHz_XTAL
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_raw(XTAL(40'000'000)/6, 424, 0, 320, 262, 0, 240);
	screen.set_screen_update("vga", FUNC(mn89304_vga_device::screen_update));

	mn89304_vga_device &vga(MN89304_VGA(config, "vga", 0));
	vga.set_screen("screen");
	// 4 Mbit, M5M44265CJ6S
	vga.set_vram_size(0x80000);
	// iochrdy tied to refresh pin and SA19, A21 and A20 to GND
	// TODO: VGA.A18 signal, banking? From maincpu thru a T7W139F decoder

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

	ROM_REGION16_LE(0x20000, "subcpu", 0)
	ROM_LOAD("kn5000_subcpu_boot.ic30", 0x00000, 0x20000, BAD_DUMP CRC(a45ceb77) SHA1(d29429a9a1ef7a718fa88c1aa38d0f7238ba5d94)) // Ranges fe0800-ff7800 and ff9800-fff000 not dumped yet. Assumed here as being filled with 0xFF.

	ROM_REGION16_LE(0x200000, "table_data", 0)
	ROM_LOAD32_WORD("kn5000_table_data_rom_even.ic3", 0x000000, 0x100000, CRC(b6f0becd) SHA1(1fd2604236b8d12ea7281fad64d72746eb00c525))
	ROM_LOAD32_WORD("kn5000_table_data_rom_odd.ic1",  0x000002, 0x100000, CRC(cd907eac) SHA1(bedf09d606d476f3e6d03e590709715304cf7ea5))

	ROM_REGION16_LE(0x100000, "custom_data", 0)
	ROM_LOAD("kn5000_custom_data_rom.ic19", 0x000000, 0x100000, CRC(5de11a6b) SHA1(4709f815d3d03ce749c51f4af78c62bf4a5e3d94))
	// IC19 is a flash ROM. The contents here were dumped from a system that had it already programmed by the initial data disk.
	//
	// The subcpu payload is stored compressed (LZSS SLIDE4K format) in IC19 flash at address 0x3E0000 (offset 0xE0000).
	// During boot, the maincpu decompresses it and transfers it to the subcpu RAM via the inter-cpu latches.
	// The compressed payloads below were extracted from the system update floppy disk images.
	ROMX_LOAD("kn5000_subprogram_v142_compressed.rom", 0x0e0000, 0x16c13, CRC(f81e598f) SHA1(13718900afd55cb2e5ff0be213ba1f5dd14bc174), ROM_BIOS(0)) // v10
	ROMX_LOAD("kn5000_subprogram_v142_compressed.rom", 0x0e0000, 0x16c13, CRC(f81e598f) SHA1(13718900afd55cb2e5ff0be213ba1f5dd14bc174), ROM_BIOS(1)) // v9
	ROMX_LOAD("kn5000_subprogram_v141_compressed.rom", 0x0e0000, 0x16bfd, CRC(c6d4ad98) SHA1(ac9791441ceb13748a2196a0a6a400431d6aed5e), ROM_BIOS(2)) // v8
	ROMX_LOAD("kn5000_subprogram_v141_compressed.rom", 0x0e0000, 0x16bfd, CRC(c6d4ad98) SHA1(ac9791441ceb13748a2196a0a6a400431d6aed5e), ROM_BIOS(3)) // v7
	ROMX_LOAD("kn5000_subprogram_v140_compressed.rom", 0x0e0000, 0x16bc4, CRC(5b182629) SHA1(13098dd150c5a6083a5d15a63d5d785802d8e8ae), ROM_BIOS(4)) // v6
	ROMX_LOAD("kn5000_subprogram_v140_compressed.rom", 0x0e0000, 0x16bc4, CRC(5b182629) SHA1(13098dd150c5a6083a5d15a63d5d785802d8e8ae), ROM_BIOS(5)) // v5

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
