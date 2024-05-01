// license:BSD-3-Clause
// copyright-holders:Bartman/Abyss

#include "emu.h"

#include "cpu/h8/h83003.h"
#include "imagedev/floppy.h"
#include "machine/timer.h"
#include "machine/upd765.h"
#include "sound/beep.h"

#include "debug/debugcpu.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "formats/pc_dsk.h"

#include "util/endianness.h"
#include "util/utf8.h"


// command line parameters:
// -window -debug -log lw840 -flop "c:\schreibmaschine\Brother LW-840ic\LW840-ic_V1.0_Disk1_Graphic_Art_Print_Program, etc..img"

// trace:
// trace msg,0,true,{tracelog "ER0=%08X,ER1=%08X,ER2=%08X,ER3=%08X,ER4=%08X,ER5=%08X,ER6=%08X,ER7=%08X ",er0,er1,er2,er3,er4,er5,er6,er7}

/***************************************************************************

Brother LW-840ic
1997

Main-PCB B48J300

Hardware:

#1
Analog Devices
ADM211EARS
EMI/EMC Compliant, +/-15 kV ESD Protected, RS-232 Line Drivers/Receivers

#5
LG/Goldstar/SK Hynix
GM82C765B
Floppy Disk Subsystem Controller
16 MHz (XT2), 18 MHz (XT1)
clone of WD37C65

#6
NKK
N341256SJ
CMOS SRAM (32k x 8)

#8
Brother/Hitachi
UC9201-000
HG71C207FD
MPU
208 pins
H8/3003-alike; can't be H8/3002, because 'Port C' is used in ROM
H8/300H Advanced Mode
14.74 MHz CPU Core (XT4), 20 MHz Printer Control (XT3)

#10
Nippon Steel Semiconductor
NN514260J-60
Fast Page Mode CMOS 256k x 16bit Dynamic RAM

#11
Macronix
US3122-A
64316JW1
5 Volt 32-Mbit (4M x 8 / 2M x 16) Mask ROM with Page Mode

see https://github.com/BartmanAbyss/brother-hardware/tree/master/4G%20-%20Brother%20LW-840ic for datasheets, manuals, photos

Emulation Status:
- Keyboard not 100%
- Screen Refresh Rate not yet measured
- Printer not working

***************************************************************************/

class gm82c765b_device : public upd765_family_device
{
public:
	gm82c765b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map& map) override ATTR_COLD
	{
		map(0x0, 0x0).r(FUNC(gm82c765b_device::msr_r));
		map(0x1, 0x1).rw(FUNC(gm82c765b_device::fifo_r), FUNC(gm82c765b_device::fifo_w));
		map(0x2, 0x2).w(FUNC(gm82c765b_device::dor_w));
		map(0x6, 0x6).rw(FUNC(gm82c765b_device::dma_r), FUNC(gm82c765b_device::dma_w));
	}
};

DEFINE_DEVICE_TYPE(GM82C765B, gm82c765b_device, "gm82c765b", "GoldStar GM82C765B FDC") // also sold with Hynix branding

gm82c765b_device::gm82c765b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	upd765_family_device(mconfig, GM82C765B, tag, owner, clock)
{
	ready_polled = true;
	ready_connected = false;
	select_connected = true;
}

namespace {

class lw840_state : public driver_device
{
public:
	lw840_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		maincpu(*this, "maincpu"),
		screen(*this, "screen"),
		fdc(*this, "fdc"),
		beeper(*this, "beeper"),
		io_kbrow(*this, "kbrow.%u", 0),
		rom(*this, "maincpu"),
		sram(*this, "sram")
	{ }

	void lw840(machine_config &config) ATTR_COLD;

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// devices
	required_device<h83003_device> maincpu;
	required_device<screen_device> screen;
	required_device<gm82c765b_device> fdc;
	required_device<beep_device> beeper;
	required_ioport_array<9> io_kbrow;
	required_region_ptr<uint16_t> rom;
	required_shared_ptr<uint16_t> sram;

	uint8_t keyboard{};
	uint8_t irq_toggle{};

	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint8_t port7_r()
	{
		auto row = keyboard & 0x0f;
		if(row <= 8)
			return io_kbrow[row]->read();

		// seems to be able to control power-on self test if not 0xff
		return 0xff;
	}
	uint16_t keyboard_r()
	{
		return keyboard << 8;
	}
	void keyboard_w(uint16_t data)
	{
		keyboard = data >> 8;
	}
	TIMER_DEVICE_CALLBACK_MEMBER(int2_timer_callback)
	{
		irq_toggle = ~irq_toggle;
		maincpu->set_input_line(INPUT_LINE_IRQ2, irq_toggle ? ASSERT_LINE : CLEAR_LINE);
	}

	static void floppy_formats(format_registration &fr) ATTR_COLD
	{
		fr.add(FLOPPY_PC_FORMAT);
	}

	void fdc_interrupt(int state)
	{
		maincpu->set_input_line(INPUT_LINE_IRQ4, state ? ASSERT_LINE : CLEAR_LINE);
	}
	void fdc_drq(int state)
	{
		maincpu->set_input_line(H8_INPUT_LINE_DREQ0, state ? ASSERT_LINE : CLEAR_LINE);
	}
	uint16_t disk_inserted_r()
	{
		// bit#6: disk inserted
		// bit#7: ??1.44mb
		// bit#1: ???
		// bit#0: ???

		// FIXME: hack: always disk inserted, HD-disk
		return 0b01000000 << 8;
	}

	void map_program(address_map &map) ATTR_COLD
	{
		map(0x000000, 0x3fffff).rom();
		map(0x5f8000, 0x5fffff).ram().share(sram); // SRAM
		map(0x600000, 0x67ffff).ram(); // DRAM
		map(0xe00000, 0xe00007).m(fdc, FUNC(gm82c765b_device::map));
		map(0xe00030, 0xe00041).noprw(); // just to shut up the error.log
		map(0xec0000, 0xec0001).r(FUNC(lw840_state::keyboard_r)).w(FUNC(lw840_state::keyboard_w));
		map(0xec0004, 0xec0005).r(FUNC(lw840_state::disk_inserted_r));
	}
};

uint32_t lw840_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const rgb_t palette[]{
		0xffffffff,
		0xff000000,
	};

	uint16_t const *const vram = &sram[0x300 / sizeof(uint16_t)];

	for(auto y = std::max<int32_t>(cliprect.top(), 0); y <= std::min<int32_t>(cliprect.bottom(), 400 - 1); y++) {
		uint32_t *p = &bitmap.pix(y);
		for(auto x = 0; x < 640; x += 16) {
			auto gfx = vram[(y * 640 + x) / 16];
			for(int i = 15; i >= 0; i--)
				*p++ = palette[BIT(gfx, i)];
		}
	}
	return 0;
}

void lw840_state::machine_start()
{
	screen->set_visible_area(0, 640 - 1, 0, 400 - 1);
	fdc->set_rate(500'000);

	// patches here
	auto rom8 = util::big_endian_cast<uint8_t>(rom.target());
	//rom8[0x34a] = 0x47; // always branch to mem_test_error
	rom8[0x102] = rom8[0x102 + 1] = 0xff; // skip printer check
	//rom8[0xa380] = rom8[0xa380 + 1] = 0; // skip FDC init
}

void lw840_state::machine_reset()
{
}

static INPUT_PORTS_START(lw840)
	PORT_START("kbrow.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('/')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_TAB)        PORT_CHAR(UCHAR_MAMEKEY(TAB))

	PORT_START("kbrow.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR(U'£')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('_')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_UP)                 PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))

	PORT_START("kbrow.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('\'')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_G)          PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("HELP/P BREAK")          PORT_CODE(KEYCODE_END)

	PORT_START("kbrow.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('*')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))

	PORT_START("kbrow.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_START("kbrow.5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('?')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_0)          PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR(';')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_MENU)       PORT_CHAR(UCHAR_MAMEKEY(MENU))

	PORT_START("kbrow.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("INSERT")                PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';')  PORT_CHAR(':')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(0x2154) PORT_CHAR(0x2153) // ⅔ ⅓
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR(U'|') PORT_CHAR('$')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kbrow.7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PRINT/LAYOUT")          PORT_CODE(KEYCODE_PRTSCR)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CANCEL")                PORT_CODE(KEYCODE_PAUSE)      PORT_CHAR(UCHAR_MAMEKEY(CANCEL))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(UCHAR_MAMEKEY(ENTER))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(UCHAR_MAMEKEY(BACKSPACE))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    //PORT_CODE(KEYCODE_)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Horz/Vert")             //PORT_CODE(KEYCODE_)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(UCHAR_MAMEKEY(SPACE))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("kbrow.8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(U'¼') PORT_CHAR(U'¾')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_L)          PORT_CHAR('l')  PORT_CHAR('L')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('+')  PORT_CHAR('=')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_K)          PORT_CHAR('k')  PORT_CHAR('K')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_SLASH)      PORT_CHAR(U'½') PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR(U'·') PORT_CHAR(U'÷')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

static void lw840_floppies(device_slot_interface &device)
{
	device.option_add("35hd", FLOPPY_35_HD);
}

void lw840_state::lw840(machine_config &config)
{
	// basic machine hardware
	H83003(config, maincpu, 14'745'600);
	maincpu->set_addrmap(AS_PROGRAM, &lw840_state::map_program);
	maincpu->read_port7().set(FUNC(lw840_state::port7_r));
	maincpu->tend0().set("fdc", FUNC(gm82c765b_device::tc_line_w));

	TIMER(config, "2khz").configure_periodic(FUNC(lw840_state::int2_timer_callback), attotime::from_hz(2*1000));

	// video hardware
	SCREEN(config, screen, SCREEN_TYPE_RASTER);
	screen->set_color(rgb_t::white());
	screen->set_physical_aspect(640, 400);
	screen->set_screen_update(FUNC(lw840_state::screen_update));
	screen->set_refresh_hz(78.1); // TODO: measure!
	screen->set_size(640, 400);

	// floppy
	GM82C765B(config, fdc, 0);
	fdc->intrq_wr_callback().set(FUNC(lw840_state::fdc_interrupt));
	fdc->drq_wr_callback().set(FUNC(lw840_state::fdc_drq));
	FLOPPY_CONNECTOR(config, "fdc:0", lw840_floppies, "35hd", lw840_state::floppy_formats);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	BEEP(config, "beeper", 4'000).add_route(ALL_OUTPUTS, "mono", 1.0); // 4.0 kHz
}

ROM_START( lw840 )
	ROM_REGION(0x400000, "maincpu", 0)
	ROM_LOAD("us3122-a", 0x00000, 0x400000, CRC(70a3a4a6) SHA1(11e32c7da58800d69af29089f7e7deeab513b1ae))
ROM_END

} // anonymous namespace

//    YEAR  NAME  PARENT COMPAT   MACHINE INPUT  CLASS         INIT        COMPANY         FULLNAME    FLAGS
COMP( 1997, lw840,  0,   0,       lw840,  lw840, lw840_state,  empty_init, "Brother",      "LW-840ic", MACHINE_NODEVICE_PRINTER )
