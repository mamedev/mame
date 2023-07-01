// license:BSD-3-Clause
// copyright-holders:Bartman/Abyss

#include "emu.h"

#include "cpu/z180/z180.h"
#include "imagedev/floppy.h"
#include "machine/timer.h"
#include "sound/beep.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "formats/lw30_dsk.h"

#include "util/utf8.h"


#define LOG_FLOPPY (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_FLOPPY)
//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

#define LOGFLOPPY(...) LOGMASKED(LOG_FLOPPY, __VA_ARGS__)


// command line parameters:
// -log -debug -window -intscalex 2 -intscaley 2 lw30 -resolution 960x256 -flop roms\lw30\tetris.img

// floppy: see src\devices\bus\vtech\memexp\floppy.cpp

//////////////////////////////////////////////////////////////////////////
// LW-30
//////////////////////////////////////////////////////////////////////////

// *** Hit Ctrl+Q during bootup to be able to start programs (like Tetris) from disk

/***************************************************************************

Brother LW-30
1991

Hardware:

#7
Hitachi
HD64180RP6
8-bit CMOS Micro Processing Unit
fully compatible with Zilog Z80180 (Z180)
6 MHz, DP-64S, Address Space 512 K Byte
MuRata CST12MTW 12.00 MHz Ceramic Resonator

#8
Mitsubishi
M65122ASP
UA5445-B LC-1

#6
NEC
D23C4001EC-172
UA2849-A
4MBit Mask ROM for Dictionary

#5
LH532H07
UA5362-A
2MBit Mask ROM

#11
Hitachi
HM6264ALP-15
High Speed CMOS Static RAM (8kbit x 8) 150ns

#10
Mitsubishi
M65017FP
UA5498-A
Murata CST4MGW 4.00 MHz Ceramic Resonator

#3, #4
Mitsubishi
HD74LS157P

#1, #2
NEC
D41464C-10
Dynamic NMOS RAM (64kbit x 4) 100ns

QA1, QA2
Mitsubishi
M54587P

#12
Texas Instruments
SN74HC04N

Floppy - custom single sided 3.5" DD
240kb capacity
256 bytes/sector
12 sectors/track
78 tracks
custom 5-to-8 GCR encoding (very similar to Apple II's 5-and-3 encoding)
300 rpm
FF FF FF used as sync-start, AB sync-mark for sector header, DE sync-mark for sector data
FAT12 File System

ROHM
BA6580DK
Read/Write Amplifier for FDD

see https://github.com/BartmanAbyss/brother-hardware/tree/master/1G%20-%20Brother%20LW-30 for datasheets, photos

Emulation Status:
Printer not working
Floppy Disk writing not working
Floppy Disk Sync not implemented (reading works)
Dictionary ROM not working
Cursor shapes not implemented except block cursor
Keyboard not 100% (mostly copied from LW-350)

TODO: find self-test; verify RAM address map
// 8kb SRAM, 64kb DRAM <- where?

***************************************************************************/

namespace {

class lw30_state : public driver_device
{
public:
	lw30_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		maincpu(*this, "maincpu"),
		screen(*this, "screen"),
		floppy(*this, "floppy"),
		beeper(*this, "beeper"),
		m_io_kbrow(*this, "kbrow.%u", 0),
		rom(*this, "maincpu"),
		font_normal(*this, "font_normal"),
		font_bold(*this, "font_bold")
	{
		//video_control = 0b00000010; // TEST LW-10 screen height
	}

	void lw30(machine_config& config);

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// devices
	required_device<hd64180rp_device> maincpu;
	required_device<screen_device> screen;

	required_device<floppy_connector> floppy;
	required_device<beep_device> beeper;
	required_ioport_array<9> m_io_kbrow;
	required_region_ptr<uint8_t> rom, font_normal, font_bold;

	// floppy
	uint8_t floppy_data = 0;
	uint8_t io_88 = 0;
	uint8_t io_98 = 0;
	uint8_t floppy_control = 0; // stepper motor control
	uint8_t floppy_steps = 0; // quarter track
	uint8_t floppy_shifter = 0, floppy_latch = 0;
	bool floppy_read_until_zerobit = false;

	// video
	uint8_t videoram[0x2000]; // 80 chars * 14 lines; 2 bytes per char (attribute, char)
	uint8_t video_cursor_x, video_cursor_y, video_pos_x, video_pos_y, video_control, io_b8;
	uint8_t cursor_state;

	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint8_t illegal_r(offs_t offset)
	{
		if(!machine().side_effects_disabled())
			LOG("%s: unmapped memory read from %0*X\n", machine().describe_context(), 6, offset);
		return 0;
	}
	void illegal_w(offs_t offset, uint8_t data)
	{
		LOG("%s: unmapped memory write to %0*X = %0*X\n", machine().describe_context(), 6, offset, 2, data);
	}

	// IO
	void video_cursor_x_w(uint8_t data) // 70
	{
		video_cursor_x = data;
	}
	void video_cursor_y_w(uint8_t data) // 71
	{
		video_cursor_y = data;
	}
	void video_pos_x_w(uint8_t data) // 72
	{
		video_pos_x = data;
	}
	void video_pos_y_w(uint8_t data) // 73
	{
		video_pos_y = data;
	}
	uint8_t video_data_r() // 74
	{
		uint8_t data = 0x00;
		if(video_pos_y < 0x20)
			data = videoram[video_pos_y * 256 + video_pos_x];
		else {
			if(!machine().side_effects_disabled())
				LOG("%s: video_data_r out of range: x=%u, y=%u\n", machine().describe_context(), video_pos_x, video_pos_y);
		}

		return data;
	}

	void video_data_w(uint8_t data) // 74
	{
		if(video_pos_y < 0x20)
			videoram[video_pos_y * 256 + video_pos_x] = data;
		else
			LOG("%s: video_data_w out of range: x=%u, y=%u\n", machine().describe_context(), video_pos_x, video_pos_y);

		video_pos_x++;
		if(video_pos_x == 0)
			video_pos_y++;
	}

	uint8_t video_control_r() // 75
	{
		return video_control;
	}
	void video_control_w(uint8_t data)
	{
		video_control = data; // | 0b00000010; // TEST LW-10 screen height
	}
	// 76
	uint8_t io_77_r() // config
	{
		// TODO: use PORT_CONFNAME, etc
		uint8_t out = 0x20; // 14 lines
		out |= 0x00; // german
		//out |= 0x01; // french
		//out |= 0x02; // german
		return ~out;
	}

	// Floppy
	TIMER_DEVICE_CALLBACK_MEMBER(floppy_timer_callback)
	{
		auto floppy_device = floppy->get_device();
		if(floppy_device->ready_r() != false)
			return;

		floppy_latch <<= 1;

		attotime now = machine().time();
		attotime when = now - attotime::from_usec(4);
		attotime reversal = floppy_device->get_next_transition(when);
		if(reversal > when && reversal <= now)
			floppy_latch |= 1;

		floppy_shifter++;
		if((floppy_read_until_zerobit && (floppy_latch & 1) == 0) || (!floppy_read_until_zerobit && floppy_shifter == 8)) {
			//LOGFLOPPY("%s: floppy_timer_callback: floppy_read_until_zerobit=%d latch=%02X\n", machine().describe_context(), floppy_read_until_zerobit, floppy_latch);
			floppy_control |= 0x80; // floppy_data_available = true;
			floppy_data = floppy_latch;
			floppy_latch = floppy_shifter = 0;
			floppy_read_until_zerobit = false;
		}
		//LOGFLOPPY("%s: floppy_timer_callback: IO_80=%02X, shifter=%u offset=%u\n", machine().describe_context(), io_80, floppy_shifter, floppy_track_offset % cache.size());
		//LOGFLOPPY("%s: read_io_80 track=%d,offset=%4x => %02x\n", callstack(), floppy_steps / 4, floppy_track_offset, ret);
	}

	uint8_t floppy_data_r() // 80
	{
		if(!machine().side_effects_disabled()) {
			floppy_control &= ~0x80; // floppy_data_available = false;
			LOGFLOPPY("%s: read %02X from IO 80\n", machine().describe_context(), floppy_data);
		}
		return floppy_data;
	}
	void floppy_data_w(uint8_t data)
	{
		LOGFLOPPY("%s: write %02X to IO 80\n", machine().describe_context(), data);
		floppy_data = data;
	}

	uint8_t io_88_r()
	{
		// bit 0: set in start_write; cleared in end_write
		// bit 1: pulsed after 3*0xFF sync (read next floppydata until zero-bit)
		// bit 2: cleared in stepper routines, rst28_06
		// bit 3: set in start_write; cleared in end_write
		// bit 5: cleared in rst28_06; motor-on?
		if(!machine().side_effects_disabled())
			LOGFLOPPY("%s: read %02X from IO 88\n", machine().describe_context(), io_88);
		return io_88;
	}
	void io_88_w(uint8_t data)
	{
		LOGFLOPPY("%s: write %02X to IO 88\n", machine().describe_context(), data);
		io_88 = data;
		floppy->get_device()->mon_w((io_88 & (1 << 5)) == 0);
	}

	uint8_t floppy_status_r() // 90
	{
		// bit 7 set; data ready from floppy
		// bit 6 clear; unknown meaning
		// bit 5 clear; unknown meaning
		// bit 4 clear; unknown meaning
		// bit 3-0: stepper motor
		if(!machine().side_effects_disabled())
			LOGFLOPPY("%s: read %02X from IO 90\n", machine().describe_context(), floppy_control);
		return floppy_control;
	}
	void floppy_stepper_w(uint8_t data)
	{
		LOGFLOPPY("%s: write %02X to IO 90\n", machine().describe_context(), data);
		// write directly to 4-wire bipolar stepper motor (see stepper_table)
		// a rotation to the left means decrease quarter-track
		// a rotation to the right means increase quarter-track
		const auto rol4 = [](uint8_t d) { return ((d << 1) & 0b1111) | ((d >> 3) & 0b0001); };
		const auto ror4 = [](uint8_t d) { return ((d >> 1) & 0b0111) | ((d << 3) & 0b1000); };
		const auto old_track = floppy_steps / 4;
		switch(data & 0xf) {
		case 0b0011:
		case 0b0110:
		case 0b1100:
		case 0b1001:
			if((data & 0x0f) == rol4(floppy_control))
				floppy_steps--;
			else if((data & 0x0f) == ror4(floppy_control))
				floppy_steps++;
			else
				LOGFLOPPY("%s: illegal step %02x=>%02x\n", machine().describe_context(), floppy_control, data);
			break;
		default:
			LOGFLOPPY("%s: initial step %02x=>%02x\n", machine().describe_context(), floppy_control, data);
			break;
		}
		const auto new_track = floppy_steps / 4;
		auto floppy_device = floppy->get_device();
		if(new_track != old_track) {
			floppy_device->dir_w(new_track < old_track);
			floppy_device->stp_w(true);
			floppy_device->stp_w(false);
		}
		LOGFLOPPY("%s: floppy_steps=%3d => old_track=%2d new_track=%2d cyl=%2d\n", machine().describe_context(), floppy_steps, old_track, new_track, floppy_device->get_cyl());
		assert(floppy_device->get_cyl() == new_track);
		floppy_control = (floppy_control & 0xf0) | (data & 0x0f);
	}

	uint8_t io_98_r()
	{
		// mirrored in RAM
		// bit 0: cleared in rst28_06 in mirror
		// bit 2: cleared before formatting in mirror; set after formatting
		// bit 3: cleared before formatting in mirror
		// bit 4: cleared before writing in mirror; set after writing
		if(!machine().side_effects_disabled()) {
			if(io_88 & 0b10)
				floppy_read_until_zerobit = true;
			else
				floppy_read_until_zerobit = false;

			LOGFLOPPY("%s: read %02X from IO 98\n", machine().describe_context(), io_98);
		}
		return io_98;
	}
	void io_98_w(uint8_t data)
	{
		LOGFLOPPY("%s: write %02X to IO 98\n", machine().describe_context(), data);
		io_98 = data;
	}

	uint8_t illegal_io_r(offs_t offset, uint8_t mem_mask = ~0)
	{
		if(!machine().side_effects_disabled())
			LOGFLOPPY("%s: unmapped IO read from %0*X & %0*X\n", machine().describe_context(), 4, offset + 0x40, 2, mem_mask);
		return 0;
	}
	void illegal_io_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0)
	{
		LOGFLOPPY("%s: unmapped IO write to %0*X = %0*X & %0*X\n", machine().describe_context(), 4, offset + 0x40, 2, data, 2, mem_mask);
	}

	uint8_t io_b0_r()
	{
		// Tetris reads bit 3, needed for correct keyboard layout
		return 0b1000;
	}
	uint8_t io_b8_r() // B8 (keyboard)
	{
		// keyboard matrix
		if(io_b8 <= 8)
			return m_io_kbrow[io_b8]->read();
		return 0x00;
	}
	void io_b8_w(uint8_t data)
	{
		io_b8 = data;
	}

	void beeper_w(uint8_t data) // F0
	{
		beeper->set_state(~data & 0x01);
	}

	void irqack_w(uint8_t) // F8
	{
		maincpu->set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE);
	}

	TIMER_DEVICE_CALLBACK_MEMBER(int1_timer_callback)
	{
		maincpu->set_input_line(INPUT_LINE_IRQ1, ASSERT_LINE);
	}

	TIMER_DEVICE_CALLBACK_MEMBER(cursor_timer_callback)
	{
		cursor_state = !cursor_state;
	}

	static void floppy_formats(format_registration &fr)
	{
		fr.add(FLOPPY_LW30_FORMAT);
	}

	static void lw30_floppies(device_slot_interface &device) ATTR_COLD
	{
		device.option_add("35ssdd", FLOPPY_35_SSDD);
	}

	void map_program(address_map &map) ATTR_COLD
	{
		map(0x00000, 0x01fff).rom();
		map(0x02000, 0x05fff).ram();
		map(0x06000, 0x3ffff).rom();
		map(0x50000, 0x51fff).ram(); // ???
		map(0x61000, 0x61fff).ram();
		map(0x42000, 0x45fff).rom().region("maincpu", 0x02000).w(FUNC(lw30_state::illegal_w)); // => ROM 0x02000-0x05fff
		map(0x65000, 0x70fff).ram();
	}

	void map_io(address_map &map) ATTR_COLD
	{
		map.global_mask(0xff);
		map(0x00, 0x3f).noprw(); // Z180 internal registers

		// video
		map(0x70, 0x70).w(FUNC(lw30_state::video_cursor_x_w));
		map(0x71, 0x71).w(FUNC(lw30_state::video_cursor_y_w));
		map(0x72, 0x72).w(FUNC(lw30_state::video_pos_x_w));
		map(0x73, 0x73).w(FUNC(lw30_state::video_pos_y_w));
		map(0x74, 0x74).rw(FUNC(lw30_state::video_data_r), FUNC(lw30_state::video_data_w));
		map(0x75, 0x75).rw(FUNC(lw30_state::video_control_r), FUNC(lw30_state::video_control_w));
		map(0x76, 0x76).noprw(); // NOP just to shut up the log
		map(0x77, 0x77).r(FUNC(lw30_state::io_77_r)).nopw(); // NOP just to shut up the log

		// floppy
		map(0x80, 0x80).rw(FUNC(lw30_state::floppy_data_r), FUNC(lw30_state::floppy_data_w));
		map(0x88, 0x88).rw(FUNC(lw30_state::io_88_r), FUNC(lw30_state::io_88_w));
		map(0x90, 0x90).rw(FUNC(lw30_state::floppy_status_r), FUNC(lw30_state::floppy_stepper_w));
		map(0x98, 0x98).rw(FUNC(lw30_state::io_98_r), FUNC(lw30_state::io_98_w));

		map(0xa8, 0xa8).noprw(); // NOP just to shut up the log
		map(0xb0, 0xb0).r(FUNC(lw30_state::io_b0_r));
		map(0xb8, 0xb8).rw(FUNC(lw30_state::io_b8_r), FUNC(lw30_state::io_b8_w));
		map(0xd8, 0xd8).noprw(); // NOP just to shut up the log
		map(0xf0, 0xf0).w(FUNC(lw30_state::beeper_w));
		map(0xf8, 0xf8).w(FUNC(lw30_state::irqack_w));

		//map(0x40, 0xff).rw(FUNC(lw30_state::illegal_io_r), FUNC(lw30_state::illegal_io_w));
	}
};

uint32_t lw30_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// based on LW-350 ROM draw_char routine @ 6B14
	enum attrs : uint8_t {
		UNDERLINE           = 0b00000001,
		OVERLINE            = 0b00000010,
		BOLD                = 0b00000100,
		VERTICAL_LINE       = 0b00001000,
		INVERT_FULL         = 0b00010000,
		INVERT_UPPER_HALF   = 0b00100000,
		INVERT_LOWER_HALF   = 0b01000000
	};

	const rgb_t palette[]{
		0xffffffff,
		0xff000000,
	};

	enum control : uint8_t {
		DISPLAY_ON          = 0b00000001,
		HALF_HEIGHT         = 0b00000010, // 64px height (LW-10/20) instead of 128px height (LW-30)
		BITMAP_MODE         = 0b00001000,
		TILE_MODE           = 0b00100000, // 8x8 tiles at videoram[0x1000]
	};

	if(video_control & DISPLAY_ON) {
		if(video_control & TILE_MODE) {
			uint8_t pixmap[60 * 128]; // pixel data
			for(auto y = 0; y < 16; y++) {
				for(auto x = 0; x < 60; x++) {
					const auto atr = videoram[y * 256 + x * 2 + 0];
					const auto chr = videoram[y * 256 + x * 2 + 1];
					const auto fnt = &videoram[0x1000 + chr * 8 + ((atr & BOLD) ? 0x800 : 0)];
					uint8_t charbuf[8];
					for(int i = 0; i < 8; i++) {
						charbuf[i] = fnt[i];
					}
					if(atr & UNDERLINE) {
						charbuf[7] = 0xff;
					}
					if(atr & VERTICAL_LINE) {
						for(int i = 0; i < 8; i++) {
							charbuf[i] |= 0b1;
						}
					}

					for(int i = 0; i < 8; i++) {
						pixmap[(y * 8 + i) * 60 + x] = charbuf[i];
					}
				}
			}
			for(auto y = 0; y < 128; y++) {
				uint32_t *p = &bitmap.pix(y);
				for(auto x = 0; x < 480; x += 8) {
					const auto gfx = pixmap[y * 60 + x / 8];
					*p++ = palette[BIT(gfx, 7)];
					*p++ = palette[BIT(gfx, 6)];
					*p++ = palette[BIT(gfx, 5)];
					*p++ = palette[BIT(gfx, 4)];
					*p++ = palette[BIT(gfx, 3)];
					*p++ = palette[BIT(gfx, 2)];
					*p++ = palette[BIT(gfx, 1)];
					*p++ = palette[BIT(gfx, 0)];
				}
			}
		} else if(video_control & BITMAP_MODE) {
			for(auto y = 0; y < 128; y++) {
				uint32_t *p = &bitmap.pix(y);
				for(auto x = 0; x < 480; x += 8) {
					const auto gfx = videoram[y * 64 + x / 8];
					*p++ = palette[BIT(gfx, 7)];
					*p++ = palette[BIT(gfx, 6)];
					*p++ = palette[BIT(gfx, 5)];
					*p++ = palette[BIT(gfx, 4)];
					*p++ = palette[BIT(gfx, 3)];
					*p++ = palette[BIT(gfx, 2)];
					*p++ = palette[BIT(gfx, 1)];
					*p++ = palette[BIT(gfx, 0)];
				}
			}
		} else {
			// default font
			uint8_t pixmap[80 * 128]{}; // pixel data
			for(auto y = 0; y < 14; y++) {
				for(auto x = 0; x < 80; x++) {
					const auto atr = videoram[y * 256 + x * 2 + 0];
					const auto chr = videoram[y * 256 + x * 2 + 1];
					const auto fnt = (atr & BOLD) ? &font_bold[chr * 8] : &font_normal[chr * 8];
					uint8_t charbuf[9];
					charbuf[0] = 0x00;
					for(int i = 0; i < 8; i++) {
						charbuf[i + 1] = fnt[i];
					}

					if(atr & UNDERLINE) {
						charbuf[8] = 0xff;
					}
					if(atr & OVERLINE) {
						charbuf[0] = 0xff;
					}
					if(atr & VERTICAL_LINE) {
						for(int i = 0; i < 9; i++) {
							charbuf[i] |= 0b1;
						}
					}
					if(atr & INVERT_FULL) {
						for(int i = 0; i < 9; i++) {
							charbuf[i] ^= 0xff;
						}
					}
					if(atr & INVERT_LOWER_HALF) {
						for(int i = 4; i < 9; i++) {
							charbuf[i] ^= 0xff;
						}
					}
					if(atr & INVERT_UPPER_HALF) {
						for(int i = 0; i < 5; i++) {
							charbuf[i] ^= 0xff;
						}
					}

					for(int i = 0; i < 9; i++) {
						pixmap[(y * 9 + i) * 80 + x] = charbuf[i];
					}
				}
			}

			// draw cursor; TODO: shape
			if(cursor_state) {
				const auto cursor_x = video_cursor_x & 0x7f;
				const auto cursor_y = (video_cursor_x >> 7) | ((video_cursor_y & 7) << 1);
				if(cursor_x < 80 && cursor_y < 14) {
					for(int i = 0; i < 9; i++) {
						pixmap[(cursor_y * 9 + i) * 80 + cursor_x] ^= 0xff;
					}
				}
			}
			for(auto y = 0; y < 128; y++) {
				uint32_t *p = &bitmap.pix(y);
				for(auto x = 0; x < 640; x += 8) {
					const auto gfx = pixmap[y * 80 + x / 8];
					*p++ = palette[BIT(gfx, 5)];
					*p++ = palette[BIT(gfx, 4)];
					*p++ = palette[BIT(gfx, 3)];
					*p++ = palette[BIT(gfx, 2)];
					*p++ = palette[BIT(gfx, 1)];
					*p++ = palette[BIT(gfx, 0)];
				}
			}
		}
	} else {
		// display off
		for(auto y = 0; y < 128; y++) {
			uint32_t *p = &bitmap.pix(y);
			for(auto x = 0; x < 480; x++) {
				*p++ = palette[0];
			}
		}
	}

	return 0;
}

void lw30_state::machine_start()
{
	screen->set_visible_area(0, 480 - 1, 0, 128 - 1);

	// patch out printer init
	rom[0x280f4] = 0x00;

	// patch out autosave load
	rom[0x28c3a] = rom[0x28c3a + 1] = rom[0x28c3a + 2] = 0x00;

	// always jump to "zusatzprogramme" (otherwise hit Ctrl+Q during bootup)
	//rom[0x28103] = 0xc3;

	// floppy debugging
	//if(machine().debug_enabled()) {
	//  machine().debugger().console().execute_command(R"(bp 6a2c,1,{logerror "expect AB; A=%02X\n",a; g})", false);
	//  machine().debugger().console().execute_command(R"(bp 6617,1,{logerror "expect DE; A=%02X\n",a; g})", false);
	//}
}

void lw30_state::machine_reset()
{
	cursor_state = 0;
	video_cursor_x = video_cursor_y = 0;
	video_pos_x = video_pos_y = 0;
	video_control = 0;
	// TODO more reset variables

	memcpy(&videoram[0x1000], font_normal, 0x800);
}

static INPUT_PORTS_START(lw30)
	PORT_START("kbrow.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_4)      PORT_CHAR('4') PORT_CHAR('$')
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
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_END)    PORT_CHAR(UCHAR_MAMEKEY(END))

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
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("FILE/SPELL")            PORT_CODE(KEYCODE_HOME)       PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_COLON)      PORT_CHAR(U'ö') PORT_CHAR(U'Ö')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('+') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR(U'ü') PORT_CHAR(U'Ü')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kbrow.7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TW/WP/LAYOUT")          PORT_CODE(KEYCODE_PRTSCR)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CANCEL")                PORT_CODE(KEYCODE_PAUSE)      PORT_CHAR(UCHAR_MAMEKEY(CANCEL))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(UCHAR_MAMEKEY(ENTER))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(UCHAR_MAMEKEY(BACKSPACE))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    //PORT_CODE(KEYCODE_)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("WORD OUT/LINE OUT")     //PORT_CODE(KEYCODE_)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(UCHAR_MAMEKEY(SPACE))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("kbrow.8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(U'´') PORT_CHAR(U'`')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_L)          PORT_CHAR('l')  PORT_CHAR('K')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    //PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_K)          PORT_CHAR('k')  PORT_CHAR('K')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.')  PORT_CHAR(':')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('-')  PORT_CHAR('_')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(U'ä') PORT_CHAR(U'Ä')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

void lw30_state::lw30(machine_config &config)
{
	// basic machine hardware
	HD64180RP(config, maincpu, 12'000'000 / 2);
	maincpu->set_addrmap(AS_PROGRAM, &lw30_state::map_program);
	maincpu->set_addrmap(AS_IO, &lw30_state::map_io);

	// video hardware
	SCREEN(config, screen, SCREEN_TYPE_RASTER);
	screen->set_color(rgb_t(6, 245, 206));
	screen->set_physical_aspect(480, 128);
	screen->set_refresh_hz(78.1);
	screen->set_screen_update(FUNC(lw30_state::screen_update));
	screen->set_size(480, 128);

	// floppy disk
	FLOPPY_CONNECTOR(config, floppy, lw30_state::lw30_floppies, "35ssdd", lw30_state::floppy_formats).enable_sound(true);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	BEEP(config, beeper, 4'000).add_route(ALL_OUTPUTS, "mono", 1.0); // 4.0 kHz

	// timers
	TIMER(config, "timer_1khz").configure_periodic(FUNC(lw30_state::int1_timer_callback), attotime::from_hz(1000));
	TIMER(config, "timer_floppy").configure_periodic(FUNC(lw30_state::floppy_timer_callback), attotime::from_usec(4));
	TIMER(config, "timer_cursor").configure_periodic(FUNC(lw30_state::cursor_timer_callback), attotime::from_msec(512));
}

/***************************************************************************
  Machine driver(s)
***************************************************************************/

ROM_START( lw30 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD("ua5362-a", 0x00000, 0x40000, CRC(dac77867) SHA1(5c7ab30dec55a24eb1b7f241e5015e3836ebf077))
	ROM_REGION(0x80000, "dictionary", 0)
	ROM_LOAD("ua2849-a", 0x00000, 0x80000, CRC(fa8712eb) SHA1(2d3454138c79e75604b30229c05ed8fb8e7d15fe))
	ROM_REGION(0x800, "font_normal", 0)
	ROM_LOAD("font-normal", 0x00000, 0x800, CRC(56a8b45d) SHA1(3f2860667ee56944cf5a79bfd4e80bebf532b51a))
	ROM_REGION(0x800, "font_bold", 0)
	ROM_LOAD("font-bold", 0x00000, 0x800, CRC(d81b79c4) SHA1(fa6be6f9dd0d7ae6d001802778272ecce8f425bc))
ROM_END

} // anonymous namespace

//    YEAR  NAME  PARENT COMPAT   MACHINE INPUT  CLASS           INIT              COMPANY         FULLNAME  FLAGS
COMP( 1991, lw30, 0,     0,       lw30,   lw30,  lw30_state,     empty_init,       "Brother",      "LW-30",  MACHINE_NODEVICE_PRINTER )
