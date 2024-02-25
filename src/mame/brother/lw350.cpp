// license:BSD-3-Clause
// copyright-holders:Bartman/Abyss

#include "emu.h"

#include "cpu/z180/z180.h"
#include "imagedev/floppy.h"
#include "imagedev/floppy.h"
#include "machine/timer.h"
#include "machine/upd765.h"
#include "sound/beep.h"
#include "video/mc6845.h"

#include "debug/debugcpu.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "util/utf8.h"

// command line parameters:
// -log -debug -window -intscalex 2 -intscaley 2 lw350 -resolution 960x256 -flop roms\lw350\Brother_LW-200-300_GW-24-45_Ver1.0_SpreadsheetProgramAndDataStorageDisk.img

/***************************************************************************

Brother LW-350
1995

Hardware:

#4
Hitachi HG62F33R63FH
US0021-A
CMOS Gate Array
3,297 gates, QFPS-136
Murata Ceralock CST-MXW 16.00 MHz Ceramic Resonator

#1
Hitachi HD63266F
CMOS Floppy Disk Controller
QFP-64
Murata Ceralock CST-MXW 16.00 MHz Ceramic Resonator

#2
Hitachi HM658128ALP-10
01105330
131072-word x 8-bit High Speed CMOS Pseudo Static RAM
DP-32
100 ns

#3
Hitachi HN62334BP
UC6273-A-LWB6
524288-word x 8-bit CMOS Mask Programmable ROM
DP-32
150 ns

#5
Hitachi HD64180ZP8
8-bit CMOS Micro Processing Unit
fully compatible with Zilog Z80180 (Z180)
8 MHz, DP-64S, Address Space 512 K Byte
Murata Ceralock CST-MXW 16.00 MHz Ceramic Resonator

1.44MB Floppy Drive
MS-DOS compatible FAT12 disk format

Hidden Keys during "DECKEL OFFEN!" ("Case Open!")
- Ctrl+Shift+Cursor Right: LCD Test Menu
- Ctrl+Shift+Backspace: Adjustment Printer Menu

Hidden Keys during "SCHREIBMASCHINE" ("Typewriter")
- Ctrl+Shift+Cursor Right: LCD Test Menu
- Ctrl+Shift+Backspace: Self Test Menu
- Ctrl+Shift+Enter: Self Print Menu

Emulation Status:
- Printer not working

see https://github.com/BartmanAbyss/brother-hardware/tree/master/2G%20-%20Brother%20LW-350 for datasheets, photos

***************************************************************************/

namespace {

class lw350_state : public driver_device
{
public:
	lw350_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		maincpu(*this, "maincpu"),
		screen(*this, "screen"),
		floppy(*this, "fdc:0"),
		fdc(*this, "fdc"),
		beeper(*this, "beeper"),
		io_kbrow(*this, "kbrow.%u", 0),
		rombank(*this, "rom"),
		vram(*this, "vram")
	{ }

	void lw350(machine_config &config) ATTR_COLD;

protected:
	// driver_device overrides
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;
	void video_start() override ATTR_COLD;

private:
	// devices
	required_device<hd64180rp_device> maincpu;
	required_device<screen_device> screen;
	required_device<floppy_connector> floppy;
	required_device<hd63266f_device> fdc;
	required_device<beep_device> beeper;
	required_ioport_array<9> io_kbrow;
	required_memory_bank rombank;

	required_shared_ptr<u8> vram;
	uint8_t io_70, io_b8, io_90;
	int fdc_drq;

	// screen updates
	uint32_t screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect);

	uint8_t illegal_r(offs_t offset, uint8_t mem_mask = ~0) {
		logerror("%s: unmapped memory read from %0*X & %0*X\n", machine().describe_context(), 6, offset, 2, mem_mask);
		return 0;
	}
	void illegal_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0) {
		logerror("%s: unmapped memory write to %0*X = %0*X & %0*X\n", machine().describe_context(), 6, offset, 2, data, 2, mem_mask);
	}

	// IO
	void io_70_w(uint8_t data) {
		io_70 = data;
	}
	uint8_t io_74_r() {
		// 0x00: 7 lines display (64 pixels height)
		// 0x80: 14 lines display (128 pixels height)
		return 0x80;
	}
	uint8_t io_a8_r() {
		// bit 0: case open
		// bit 2: carriage return indicator
		//return 0x01; // case open
		return 0x00;
	}
	uint8_t io_b8_r() {
		// keyboard matrix
		if(io_b8 <= 8)
			return io_kbrow[io_b8]->read();

		switch(io_b8) {
		// get language
		case 0x09:  // valid results (see get_index_from_language)
			//return ~0x20; // french
			//return ~0x10; // french
			//return ~0x08; // german
			//return ~0x04; // french
			//return ~0x02; // french
			//return ~0x01; // german
			return ~0x00; // german
		default:   return 0x00;
		}
	}
	void io_b8_w(uint8_t data) {
		io_b8 = data;
	}
	void rombank_w(uint8_t data) { // E0
		rombank->set_entry(data & 0x03);
	}
	void beeper_w(uint8_t data) { // F0
		beeper->set_state(~data & 0x01);
	}
	void irqack_w(uint8_t) { // F8
		maincpu->set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE);
	}

	uint8_t io_7e_r() {
		return 0x80; // 1.44mb floppy
	}
	void io_7e_w(uint8_t data) { // 7e
	}
	uint8_t io_90_r() { // 90
		return floppy ? (!floppy->get_device()->idx_r()) << 6 : 0;
	}

	TIMER_DEVICE_CALLBACK_MEMBER(io_90_timer_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(int1_timer_callback);

	void map_program(address_map &map) ATTR_COLD {
		map(0x00000, 0x01fff).rom();
		map(0x02000, 0x05fff).ram();
		map(0x06000, 0x3ffff).rom();
		map(0x40000, 0x5ffff).bankr(rombank);
		map(0x60000, 0x617ff).ram();
		map(0x61800, 0x63fff).ram().share(vram);
		map(0x64000, 0x71fff).ram();
		map(0x72000, 0x75fff).rom().region("maincpu", 0x2000); // => ROM 0x02000-0x05fff
		map(0x76000, 0x7ffff).ram();
	}

	void map_io(address_map &map) ATTR_COLD {
		map.global_mask(0xff);
		map(0x00, 0x3f).noprw(); // Z180 internal registers
		map(0x70, 0x70).w(FUNC(lw350_state::io_70_w));
		map(0x74, 0x74).r(FUNC(lw350_state::io_74_r));

		// floppy
		map(0x78, 0x78).rw(fdc, FUNC(upd765a_device::msr_r), FUNC(hd63266f_device::abort_w));
		map(0x79, 0x79).lrw8(
				[this] () { return (fdc_drq ? fdc->dma_r() : fdc->fifo_r()); }, "fdc_r",
				[this] (u8 data) { fdc_drq ? fdc->dma_w(data) : fdc->fifo_w(data); }, "fdc_w");
		map(0x7a, 0x7a).r(fdc, FUNC(hd63266f_device::extstat_r));
		map(0x7e, 0x7e).rw(FUNC(lw350_state::io_7e_r), FUNC(lw350_state::io_7e_w));
		map(0x90, 0x90).r(FUNC(lw350_state::io_90_r));

		// printer
		map(0xa8, 0xa8).r(FUNC(lw350_state::io_a8_r));

		map(0xb8, 0xb8).rw(FUNC(lw350_state::io_b8_r), FUNC(lw350_state::io_b8_w));
		map(0xd8, 0xd8).nopw();
		map(0xe0, 0xe0).w(FUNC(lw350_state::rombank_w));
		map(0xf0, 0xf0).w(FUNC(lw350_state::beeper_w));
		map(0xf8, 0xf8).w(FUNC(lw350_state::irqack_w));

		//map(0x40, 0xff).rw(FUNC(lw350_state::illegal_io_r), FUNC(lw350_state::illegal_io_w));
	}
};

void lw350_state::video_start()
{
}

uint32_t lw350_state::screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect)
{
	// video on?
	if(!BIT(io_70, 0))
		return 0;

	// backlight on?
	//if(BIT(io_70, 7))
		//...

	const rgb_t palette[]{
		0xffffffff,
		0xff000000,
	};

	for(auto y = 0; y < 128; y++) {
		uint32_t* p = &bitmap.pix(y);
		for(auto x = 0; x < 640; x += 8) {
			auto gfx = vram[y * 80 + x / 8];
			//*p++ = palette[BIT(gfx, 7)];
			//*p++ = palette[BIT(gfx, 6)];
			*p++ = palette[BIT(gfx, 5)];
			*p++ = palette[BIT(gfx, 4)];
			*p++ = palette[BIT(gfx, 3)];
			*p++ = palette[BIT(gfx, 2)];
			*p++ = palette[BIT(gfx, 1)];
			*p++ = palette[BIT(gfx, 0)];
		}
	}
	return 0;
}

static INPUT_PORTS_START(lw350)
	PORT_START("kbrow.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_4)      PORT_CHAR('4') PORT_CHAR('%')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_3)      PORT_CHAR('3') PORT_CHAR(U'§')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_W)      PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_E)      PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_D)      PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_X)      PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_TAB)    PORT_CHAR(UCHAR_MAMEKEY(TAB))

	PORT_START("kbrow.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_5)      PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_6)      PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_R)      PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_T)      PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_C)      PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_F)      PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_UP)                 PORT_CODE(KEYCODE_UP)     PORT_CHAR(UCHAR_MAMEKEY(UP))

	PORT_START("kbrow.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_8)      PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_7)      PORT_CHAR('7') PORT_CHAR('/')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_Z)      PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_H)      PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_G)      PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_V)      PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G.S.END")               PORT_CODE(KEYCODE_END)    PORT_CHAR(UCHAR_MAMEKEY(END))

	PORT_START("kbrow.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))

	PORT_START("kbrow.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_START("kbrow.5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_MINUS)      PORT_CHAR(U'ß') PORT_CHAR('?')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_0)          PORT_CHAR('0')  PORT_CHAR('=')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_P)          PORT_CHAR('p')  PORT_CHAR('P')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_O)          PORT_CHAR('o')  PORT_CHAR('O')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_M)          PORT_CHAR('m')  PORT_CHAR('M')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')  PORT_CHAR(';')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_MENU)       PORT_CHAR(UCHAR_MAMEKEY(MENU))

	PORT_START("kbrow.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Inhalt")                PORT_CODE(KEYCODE_HOME)       PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_COLON)      PORT_CHAR(U'ö') PORT_CHAR(U'Ö')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('+')  PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR(U'ü') PORT_CHAR(U'Ü')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kbrow.7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SM/Layout")             PORT_CODE(KEYCODE_PRTSCR)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("STORNO")                PORT_CODE(KEYCODE_PAUSE)      PORT_CHAR(UCHAR_MAMEKEY(CANCEL))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(UCHAR_MAMEKEY(ENTER))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(UCHAR_MAMEKEY(BACKSPACE))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    //PORT_CODE(KEYCODE_)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Horz/Vert")             //PORT_CODE(KEYCODE_)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(UCHAR_MAMEKEY(SPACE))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("kbrow.8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(U'´')  PORT_CHAR(U'`')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_L)          PORT_CHAR('l')   PORT_CHAR('L')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('\'')  PORT_CHAR(U'°')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_K)          PORT_CHAR('k')   PORT_CHAR('K')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.')   PORT_CHAR(':')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('-')   PORT_CHAR('_')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(U'ä')  PORT_CHAR(U'Ä')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

TIMER_DEVICE_CALLBACK_MEMBER(lw350_state::int1_timer_callback)
{
	maincpu->set_input_line(INPUT_LINE_IRQ1, ASSERT_LINE);
}

//uint8_t char_attribute = 0x00;

void lw350_state::machine_start()
{
	auto rom = memregion("maincpu")->base();
	rombank->configure_entries(0, 4, rom, 0x20000);
	screen->set_visible_area(0, 480 - 1, 0, 128 - 1);

	fdc->set_floppy(floppy->get_device());

	// ROM patches

	// force jump to self-test menu
//  rom[0x280f2] = 0x20;
//  rom[0x280f2+1] = 0x05;

	// force jump to lcd-test menu
//  rom[0x280f2] = 0x2c;
//  rom[0x280f2+1] = 0x05;

	// force jump to self-print menu
//  rom[0x280f2] = 0x32;
//  rom[0x280f2 + 1] = 0x05;

	// set initial mode
	//rom[0x29a12] = 0x0a;

	// patch out printer init
	rom[0x280df] = 0x00;

	// force RAM DOWN
	//rom[0x280c3] = rom[0x280be];
	//rom[0x280c3 + 1] = rom[0x280be + 1];

	// patch out self test bit 5 check; causes reboot loop
	//rom[0x280c0] = 0x00;
	//rom[0x280c1] = 0x00;
	//rom[0x280c2] = 0x00;

	// force bold font
	//rom[0x6b29] = 0x00;
	//rom[0x6b29+1] = 0x00;

	// char attributes
	//rom[0x6b47] = 0x3e;
	//rom[0x6b48] = char_attribute;
	//rom[0x6b49] = 0x00;
	//rom[0x6b4a] = 0x00;
	//rom[0x6b4b] = 0x00;
	//rom[0x6b4c] = 0x00;
}

void lw350_state::machine_reset()
{
	io_90 = 0x00;
	fdc->reset();
	fdc_drq = 0;
}

static void lw350_floppies(device_slot_interface& device) {
	device.option_add("35dd", FLOPPY_35_DD);
	device.option_add("35hd", FLOPPY_35_HD);
}

void lw350_state::lw350(machine_config &config) {
	// basic machine hardware
	HD64180RP(config, maincpu, 16'000'000 / 2);
	maincpu->set_addrmap(AS_PROGRAM, &lw350_state::map_program);
	maincpu->set_addrmap(AS_IO, &lw350_state::map_io);
	maincpu->tend0_wr_callback().set([this] (int state) { fdc->tc_w((fdc_drq && state) ? 1 : 0); });
	maincpu->rts0_wr_callback().set(fdc, FUNC(hd63266f_device::rate_w));
	TIMER(config, "1khz").configure_periodic(FUNC(lw350_state::int1_timer_callback), attotime::from_hz(1000));

	// video hardware
	SCREEN(config, screen, SCREEN_TYPE_RASTER);
	screen->set_color(rgb_t(6, 245, 206));
	screen->set_physical_aspect(480, 128);
	screen->set_refresh_hz(78.1);
	screen->set_screen_update(FUNC(lw350_state::screen_update));
	screen->set_size(480, 128);

	HD63266F(config, fdc, XTAL(16'000'000));
	fdc->drq_wr_callback().set([this] (int state) { fdc_drq = state; maincpu->set_input_line(Z180_INPUT_LINE_DREQ0, state); });
	fdc->set_ready_line_connected(false);
	fdc->set_select_lines_connected(false);
	fdc->inp_rd_callback().set([this] () { return floppy->get_device()->dskchg_r(); });

	FLOPPY_CONNECTOR(config, floppy, lw350_floppies, "35hd", floppy_image_device::default_pc_floppy_formats);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	BEEP(config, "beeper", 4'000).add_route(ALL_OUTPUTS, "mono", 1.0); // 4.0 kHz
}

//////////////////////////////////////////////////////////////////////////
// LW-450
//////////////////////////////////////////////////////////////////////////

/***************************************************************************

Brother LW-450
1992

Hardware:

#13
Hitachi
HD64180RF6X
8-bit CMOS Micro Processing Unit
fully compatible with Zilog Z80180 (Z180)
6 MHz, FP-80B, Address Space 1 M Byte
MuRata CST12MTW 12.00 MHz Ceramic Resonator

#12
Mitsubishi
M65020FP
UA7777-A

#15
Hitachi
HD74LS368AP

#10, #11
Hitachi
HD74LS157P

#7, #8
NEC
D41464C-10
65,536 x 4-Bit
Dynamic NMOS RAM
100 ns
18-pin plastic DIP

#1
NEC
D23C4001EC-172
UA2849-A
4MBit Mask ROM

#2
AMD
AM27C020
2BC04
2MBit (256K x 8-Bit) CMOS EPROM

#3
Hitachi
HM65256BSP-10
32,768 word x 8-bit High Speed Pseudo Static RAM
100 ns
DP-28N

#4
Hitachi HD63266F
CMOS Floppy Disk Controller
QFP-64
Murata Ceralock CST-MXW 16.00 MHz Ceramic Resonator

#5
Hitachi
HD6445P4
CRTC-II (CRT Controller)
DP-40
80 system Bus Interface
4.0 MHz Bus Timing

#9
Mitsubishi
M65133FP
UA7550-A

#14
Hitachi
HM62256AF-8
32,768 word x 8-bit High Speed CMOS Static RAM
85 ns
FP-28

720kb Floppy Drive

D-SUB9 CRT Connector
HSYNC 18.5 kHz
VSYNC 50 Hz
PC MDA standard
  DB9 connector 1+2 GND, 3+4+5 nc, 6 Intensity, 7 Brightness, 8 HSync, 9 VSync
  HSync positive, VSync negative active

see https://github.com/BartmanAbyss/brother-hardware/tree/master/1G%20-%20Brother%20LW-450 for datasheets, photos

Emulation Status:
Printer not working
Floppy Read has some problems (directory working, but LW-450 reports illegal file format when trying to load a .wpt file (content verified with LW-350), but writing seems fine)
Dictionary ROM probably not correctly mapped

***************************************************************************/

constexpr int MDA_CLOCK = 16'257'000;

class lw450_state : public driver_device
{
public:
	lw450_state(const machine_config &mconfig, device_type type, const char* tag) :
		driver_device(mconfig, type, tag),
		maincpu(*this, "maincpu"),
		screen(*this, "screen"),
		palette(*this, "palette"),
		floppy(*this, "fdc:0"),
		fdc(*this, "fdc"),
		m_crtc(*this, "hd6445"),
		vram(*this, "vram"),
		speaker(*this, "beeper"),
		io_kbrow(*this, "kbrow.%u", 0),
		rom(*this, "maincpu"),
		rombank(*this, "dictionary")
	{ }

	void lw450(machine_config &config) ATTR_COLD;

protected:
	// driver_device overrides
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;
	void video_start() override ATTR_COLD;

private:
	// devices
	required_device<z80180_device> maincpu; // use z80180 instead of hd64180rp, because hd64180rf doesn't have hd64180rp's 19-bit address width
	required_device<screen_device> screen;
	required_device<palette_device> palette;
	required_device<floppy_connector> floppy;
	required_device<hd63266f_device> fdc;
	required_device<hd6345_device> m_crtc;
	required_shared_ptr<uint8_t> vram;
	required_device<beep_device> speaker;
	required_ioport_array<9> io_kbrow;
	required_region_ptr<uint8_t> rom;
	required_memory_bank rombank;

	uint8_t io_72, io_73, io_74, io_75; // gfx
	uint8_t io_b8;
	uint32_t framecnt;

	uint8_t illegal_r(offs_t offset) {
		if(!machine().side_effects_disabled())
			logerror("%s: unmapped memory read from %0*X\n", machine().describe_context(), 6, offset);
		return 0;
	}
	void illegal_w(offs_t offset, uint8_t data) {
		if(!machine().side_effects_disabled())
			logerror("%s: unmapped memory write to %0*X = %0*X\n", machine().describe_context(), 6, offset, 2, data);
	}

	uint8_t rom72000_r(offs_t offset) {
		return rom[0x02000 + offset];
	}

	// IO
	uint8_t io_b0_r() { return ~0x00; }
	uint8_t io_b8_r() {
		// keyboard matrix
		if(io_b8 <= 8)
			return io_kbrow[io_b8]->read();
		return 0x00;
	}
	void io_b8_w(uint8_t data) {
		io_b8 = data;
	}
	void rombank_w(uint8_t data) { // E0
		if(data >= 4 && data < 8)
			rombank->set_entry(data - 4);
	}
	void beeper_w(uint8_t data) { // F0
		speaker->set_state(~data & 0x01);
	}
	void irqack_w(uint8_t) { // F8
		maincpu->set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE);
	}

	// CRTC
	MC6845_UPDATE_ROW(crtc_update_row);
	void crtc_vsync(int state);
	void io_72_w(uint8_t data) { io_72 = data; }
	void io_73_w(uint8_t data) { io_73 = data; }
	void io_74_w(uint8_t data);
	void io_75_w(uint8_t data) { io_75 = data; }

	TIMER_DEVICE_CALLBACK_MEMBER(int1_timer_callback);

	void map_program(address_map& map) ATTR_COLD;
	void map_io(address_map& map) ATTR_COLD;
};

void lw450_state::video_start()
{
}

void lw450_state::map_program(address_map &map)
{
	map(0x00000, 0x01fff).rom();
	map(0x02000, 0x05fff).ram();
	map(0x06000, 0x3ffff).rom();
	map(0x40000, 0x5ffff).bankr(rombank);
	map(0x62000, 0x71fff).ram(); // D-RAM UPPER/LOWER
	map(0x72000, 0x75fff).r(FUNC(lw450_state::rom72000_r)); // => ROM 0x02000-0x05fff
	map(0x78000, 0x7ffff).ram(); // PS-RAM
	map(0xf8000, 0xfffff).ram().share(vram); // VRAM
	// text vram @ F8000-F8C80 (2*80 bytes/line)
	// font @ FC000-FD000 pitch 16
}

void lw450_state::map_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x3f).noprw(); // Z180 internal registers

	map(0x70, 0x70).w("hd6445", FUNC(hd6345_device::address_w));
	map(0x71, 0x71).w("hd6445", FUNC(hd6345_device::register_w));
	map(0x72, 0x72).w(FUNC(lw450_state::io_72_w));
	map(0x73, 0x73).w(FUNC(lw450_state::io_73_w));
	map(0x74, 0x74).w(FUNC(lw450_state::io_74_w));
	map(0x75, 0x75).w(FUNC(lw450_state::io_75_w));

	// floppy
	map(0x78, 0x7a).m(fdc, FUNC(hd63266f_device::map));

	map(0xb0, 0xb0).r(FUNC(lw450_state::io_b0_r));
	map(0xb8, 0xb8).rw(FUNC(lw450_state::io_b8_r), FUNC(lw450_state::io_b8_w));
	map(0xe0, 0xe0).w(FUNC(lw450_state::rombank_w));
	map(0xf0, 0xf0).w(FUNC(lw450_state::beeper_w));
	map(0xf8, 0xf8).w(FUNC(lw450_state::irqack_w));

	//map(0x40, 0xff) AM_READWRITE(illegal_io_r, illegal_io_w)
}

// CRTC
//////////////////////////////////////////////////////////////////////////

TIMER_DEVICE_CALLBACK_MEMBER(lw450_state::int1_timer_callback)
{
	maincpu->set_input_line(INPUT_LINE_IRQ1, ASSERT_LINE);
}

void lw450_state::crtc_vsync(int state)
{
	if(state) {
		framecnt++;
	}
}

void lw450_state::io_74_w(uint8_t data)
{
	io_74 = data;
	if(io_74 & 0x04) {
		// graphics mode
		m_crtc->set_char_width(8);
		m_crtc->set_clock(MDA_CLOCK / 8);
	} else {
		// text mode
		m_crtc->set_char_width(9);
		m_crtc->set_clock(MDA_CLOCK / 9);
	}
}

//(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint16_t ma, uint8_t ra, uint16_t y, uint8_t x_count, int8_t cursor_x, int de, int hbp, int vbp)
MC6845_UPDATE_ROW(lw450_state::crtc_update_row)
{
	// IO             72 73 74 75
	// ============================
	// typewriter:    05 02 08 f8 text     inverted
	// crt test menu: 07 02 00 f8 text
	// main menu:     05 02 0c f8 graphics inverted

	// based on LW-450 CRT Test Menu
	enum attrs {
		underline           = 0b00000001,
		extended_charset    = 0b00000100,
		bold                = 0b00001000,
		reverse             = 0b00010000,
		blink               = 0b10000000,
	};

	rgb_t const *const palette = this->palette->palette()->entry_list_raw();
	uint32_t* p = &bitmap.pix(y);
	uint16_t chr_base = ra;

	// video off
	if(!(io_72 & 0b1)) {
		for(int x = 0; x < cliprect.width(); x++)
			*p++ = palette[0];
		return;
	}

	// graphics mode
	if(io_74 & 0x04) {
		uint8_t bg = 0, fg = 1;
		// inverse display
		if(!(io_72 & 0x02))
			std::swap(bg, fg);

		for(int x = 0; x < x_count; x++) {
			auto data = vram[(ma + x + (ra << 14)) & 0x7fff];
			*p++ = palette[(data & 0x80) ? fg : bg];
			*p++ = palette[(data & 0x40) ? fg : bg];
			*p++ = palette[(data & 0x20) ? fg : bg];
			*p++ = palette[(data & 0x10) ? fg : bg];
			*p++ = palette[(data & 0x08) ? fg : bg];
			*p++ = palette[(data & 0x04) ? fg : bg];
			*p++ = palette[(data & 0x02) ? fg : bg];
			*p++ = palette[(data & 0x01) ? fg : bg];
		}
	} else {
		// text mode
		auto charram = &vram[0];
		auto fontram = &vram[0x4000];

		for(int x = 0; x < x_count; x++) {
			uint16_t offset = ((ma + x) << 1) & 0x0FFF;
			auto atr = charram[offset];
			uint32_t chr = charram[offset + 1];
			if(atr & extended_charset) chr += 0x100;
			auto data = fontram[chr_base + chr * 16];
			uint8_t bit9 = 0x00;
			if(atr & extended_charset) bit9 = BIT(data, 0) ? 0xff : 0x00;
			uint8_t bg = 0, fg = 1;

			if(atr & bold)
				fg = 2;

			// inverse display
			if(!(io_72 & 0x02))
				std::swap(bg, fg);

			if((atr & underline) && ra == 13) {
				data = bit9 = 0xff;
			}
			if(atr & reverse)
				std::swap(bg, fg);

			if(x == cursor_x) {
				data = 0xff;
				bit9 = 0x00;
			} else {
				if((atr & blink) && (framecnt & 0x10)) // TODO: check blinking frequency
					data = 0x00;
			}

			for(int b = 0; b < 8; b++)
				*p++ = BIT(data, 7 - b) ? palette[fg] : palette[bg];
			*p++ = BIT(bit9, 7) ? palette[fg] : palette[bg];
		}
	}
}

// PIN 21 (Character Clock) of CRTC-II: menu: 2.0 MHz; schreibmaschine: 1.8 MHz

// these timings are all at MDA clock
// bootup:          [:hd6445] M6845 config screen: HTOTAL: 882  VTOTAL: 370  MAX_X: 719  MAX_Y: 319  HSYNC: 729-863  VSYNC: 320-335  Freq: 49.816133fps
// menu:            [:hd6445] M6845 config screen: HTOTAL: 990  VTOTAL: 370  MAX_X: 809  MAX_Y: 319  HSYNC: 819-953  VSYNC: 334-349  Freq: 44.381646fps
// schreibmaschine: [:hd6445] M6845 config screen: HTOTAL: 882  VTOTAL: 370  MAX_X: 719  MAX_Y: 319  HSYNC: 729-863  VSYNC: 320-335  Freq: 49.816133fps

void lw450_state::machine_start()
{
	rombank->configure_entries(0, 4, memregion("dictionary")->base(), 0x20000);

	palette->set_pen_color(0, rgb_t(0, 0, 0));
	palette->set_pen_color(1, rgb_t(0xaa, 0xaa, 0xaa));
	palette->set_pen_color(2, rgb_t(0xff, 0xff, 0xff));

	// patch out printer init
	rom[0x280db] = 0x00;
}

void lw450_state::machine_reset()
{
	framecnt = 0;
}

static const gfx_layout pc_16_charlayout {
	8, 16,                  // 8 x 16 characters
	256,                    // 256 characters
	1,                      // 1 bits per pixel
	{ 0 },                  // no bitplanes
	{ 0, 1, 2, 3, 4, 5, 6, 7 }, // x offsets
	{ 0 * 8, 1 * 8, 2 * 8, 3 * 8, 4 * 8, 5 * 8, 6 * 8, 7 * 8, 8 * 8, 9 * 8, 10 * 8, 11 * 8, 12 * 8, 13 * 8, 14 * 8, 15 * 8 },   // y offsets
	16*8                 // every char takes 2 x 8 bytes
};

static GFXDECODE_START( gfx_lw450 )
	GFXDECODE_RAM("vram", 0x4000, pc_16_charlayout, 0, 1)
GFXDECODE_END

void lw450_state::lw450(machine_config &config) {
	// basic machine hardware
	Z80180(config, maincpu, 12'000'000 / 2);
	maincpu->set_addrmap(AS_PROGRAM, &lw450_state::map_program);
	maincpu->set_addrmap(AS_IO, &lw450_state::map_io);
	maincpu->tend0_wr_callback().set(fdc, FUNC(hd63266f_device::tc_line_w));
	TIMER(config, "1khz").configure_periodic(FUNC(lw450_state::int1_timer_callback), attotime::from_hz(1000));

	// video hardware
	SCREEN(config, screen, SCREEN_TYPE_RASTER);
	screen->set_color(rgb_t::white());
	screen->set_physical_aspect(720, 320);
	screen->set_screen_update("hd6445", FUNC(hd6345_device::screen_update));
	screen->set_raw(MDA_CLOCK, 882, 0, 729, 370, 0, 320); // based on bootup crtc values

	GFXDECODE(config, "gfxdecode", palette, gfx_lw450);
	PALETTE(config, palette).set_entries(4);

	// CRTC
	HD6345(config, m_crtc, MDA_CLOCK / 9);
	m_crtc->set_screen(screen);
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(9);
	m_crtc->set_update_row_callback(FUNC(lw450_state::crtc_update_row));
	m_crtc->out_vsync_callback().set(FUNC(lw450_state::crtc_vsync));

	HD63266F(config, fdc, XTAL(16'000'000));
	fdc->drq_wr_callback().set_inputline(maincpu, Z180_INPUT_LINE_DREQ0);
	fdc->set_ready_line_connected(false);
	fdc->set_select_lines_connected(false);

	FLOPPY_CONNECTOR(config, floppy, lw350_floppies, "35hd", floppy_image_device::default_pc_floppy_formats);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	BEEP(config, "beeper", 4'000).add_route(ALL_OUTPUTS, "mono", 1.0); // 4.0 kHz
}

/***************************************************************************
  Machine driver(s)
***************************************************************************/

ROM_START( lw350 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD("uc6273-a-lwb6", 0x00000, 0x80000, CRC(5e85d1ec) SHA1(4ca68186fc70f30ccac95429604c88db4f0c34d2))
//  ROM_LOAD("patched", 0x00000, 0x80000, CRC(5e85d1ec) SHA1(4ca68186fc70f30ccac95429604c88db4f0c34d2))
ROM_END

ROM_START( lw450 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD("2bc04", 0x00000, 0x40000, CRC(96c2a6f1) SHA1(eb47e37ea46e3becc1b4453286f120682a0a1ddc))
	ROM_REGION(0x80000, "dictionary", 0)
	ROM_LOAD("ua2849-a", 0x00000, 0x80000, CRC(fa8712eb) SHA1(2d3454138c79e75604b30229c05ed8fb8e7d15fe))
ROM_END

} // anonymous namespace

//    YEAR  NAME  PARENT COMPAT   MACHINE INPUT  CLASS           INIT              COMPANY         FULLNAME  FLAGS
COMP( 1995, lw350,  0,   0,       lw350,  lw350, lw350_state,    empty_init,       "Brother",      "LW-350", MACHINE_NODEVICE_PRINTER )
COMP( 1992, lw450,  0,   0,       lw450,  lw350, lw450_state,    empty_init,       "Brother",      "LW-450", MACHINE_NODEVICE_PRINTER )
