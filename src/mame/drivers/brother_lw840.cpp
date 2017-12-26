// license:BSD-3-Clause
// copyright-holders:Bartman/Abyss

#include "emu.h"
#include "screen.h"
#include "speaker.h"
#include "machine/timer.h"
#include "cpu/h8/h83002.h"
#include "debug/debugcpu.h"
#include "sound/beep.h"
#include "machine/upd765.h"
#include "formats/pc_dsk.h"

// fixed eepmov.b instruction in h8.lst

// if updating project, c:\msys64\win32env.bat
// cd \schreibmaschine\mame_src
// make SUBTARGET=schreibmaschine NO_USE_MIDI=1 NO_USE_PORTAUDIO=1 vs2017

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

Emulation Status:
- Keyboard not 100% (copied from german LW-350)
- Screen Refresh Rate not yet measured
- Printer not working

***************************************************************************/

#pragma region(Floppy)
class gm82c765b_device : public upd765_family_device {
public:
	gm82c765b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_ADDRESS_MAP(map, 8) override;
};

DEFINE_DEVICE_TYPE(GM82C765B, gm82c765b_device, "gm82c765b", "GM82C765B")

#define MCFG_GM82C765B_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, GM82C765B, 0)

DEVICE_ADDRESS_MAP_START(map, 8, gm82c765b_device)
	AM_RANGE(0x0, 0x0) AM_READ(msr_r)
	AM_RANGE(0x1, 0x1) AM_READWRITE(fifo_r, fifo_w)
	AM_RANGE(0x2, 0x2) AM_WRITE(dor_w)
	AM_RANGE(0x6, 0x6) AM_READWRITE(mdma_r, mdma_w)
ADDRESS_MAP_END

gm82c765b_device::gm82c765b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : 
	upd765_family_device(mconfig, GM82C765B, tag, owner, clock)
{
	ready_polled = true;
	ready_connected = false;
	select_connected = true;
}

#pragma endregion

class lw840_state : public driver_device
{
public:
	lw840_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		maincpu(*this, "maincpu"),
		palette(*this, "palette"),
		io_kbrow(*this, "kbrow.%u", 0),
		fdc(*this, "fdc"),
		//floppy(*this, "floppy"),
		beeper(*this, "beeper")
	{ }

	// helpers
	std::string pc();
	std::string symbolize(uint32_t adr);
	std::string callstack();

	// devices
	required_device<h83002_device> maincpu;
	required_device<palette_device> palette;
	//required_device<lw840_floppy_connector> floppy;
	required_device<gm82c765b_device> fdc;
	required_device<beep_device> beeper;
	optional_ioport_array<9> io_kbrow;

	// screen updates
	uint32_t screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect);

	DECLARE_READ8_MEMBER(illegal_r); DECLARE_WRITE8_MEMBER(illegal_w);
	DECLARE_READ16_MEMBER(port7_r) {
		auto row = keyboard & 0x0f;
		if(row <= 8) {
			if(io_kbrow[row].found())
				return io_kbrow[row].read_safe(0);
		}

		// seems to be able to control power-on self test if not 0xff
		return 0xffff & mem_mask;
	}
	DECLARE_READ16_MEMBER(keyboard_r) { 
		return (keyboard << 8) & mem_mask; 
	}
	DECLARE_WRITE16_MEMBER(keyboard_w) { 
		keyboard = data >> 8; 
	}
	TIMER_DEVICE_CALLBACK_MEMBER(int2_timer_callback) {
		irq_toggle = ~irq_toggle;
		maincpu->set_input_line(INPUT_LINE_IRQ2, irq_toggle ? ASSERT_LINE : CLEAR_LINE);
	}

	DECLARE_FLOPPY_FORMATS(floppy_formats);
	DECLARE_WRITE_LINE_MEMBER(fdc_interrupt) {
		maincpu->set_input_line(INPUT_LINE_IRQ4, state ? ASSERT_LINE : CLEAR_LINE);
	}
	DECLARE_WRITE_LINE_MEMBER(fdc_drq) {
		maincpu->set_input_line(H8_INPUT_LINE_DREQ0, state ? ASSERT_LINE : CLEAR_LINE);
	}
	DECLARE_READ16_MEMBER(disk_inserted_r) {
		// bit#6: disk inserted
		// bit#7: ??1.44mb
		// bit#1: ???
		// bit#0: ???

		// hack: always disk inserted, HD-disk
		return (0b01000000 << 8) & mem_mask;
	}

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

	uint8_t keyboard{};
	uint8_t irq_toggle{};

	uint8_t* rom{};
	uint16_t* vram{};
	std::map<uint32_t, std::string> symbols;
};

static ADDRESS_MAP_START(lw840_map, AS_PROGRAM, 16, lw840_state)
 	AM_RANGE(0x000000, 0x3fffff) AM_ROM
	AM_RANGE(0x5f8000, 0x5fffff) AM_RAM AM_SHARE("sram") // SRAM
	AM_RANGE(0x600000, 0x67ffff) AM_RAM // DRAM
	AM_RANGE(0xe00000, 0xe00007) AM_DEVICE8("fdc", gm82c765b_device, map, 0xff)
	AM_RANGE(0xe00030, 0xe00041) AM_NOP // just to shut up the error.log
	AM_RANGE(0xec0000, 0xec0001) AM_READWRITE(keyboard_r, keyboard_w)
	AM_RANGE(0xec0004, 0xec0005) AM_READ(disk_inserted_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START(lw840_io_map, AS_IO, 16, lw840_state)
	AM_RANGE(h8_device::PORT_7, h8_device::PORT_7) AM_READ(port7_r)
ADDRESS_MAP_END

void lw840_state::video_start()
{
}

std::string lw840_state::pc()
{
	auto pc = machine().scheduler().currently_executing()->device().safe_pc();
 	return symbolize(pc);
}

std::string lw840_state::symbolize(uint32_t adr)
{
	auto floor_it = symbols.lower_bound(adr);
	if((floor_it == symbols.end() && !symbols.empty()) || floor_it->first != adr)
		--floor_it;
	if(floor_it != symbols.end())
		return string_format("%s+%x (%06x)", floor_it->second, adr - floor_it->first, adr);
	else
		return string_format("%06x", adr);
}

std::string lw840_state::callstack()
{
	return "";
}

uint32_t lw840_state::screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect)
{
	const rgb_t *palette = this->palette->palette()->entry_list_raw();

	for(auto y = 0; y < 400; y++) {
		uint32_t* p = &bitmap.pix32(y);
		for(auto x = 0; x < 640; x += 16) {
			auto gfx = vram[(y * 640 + x) / 16];
			for(int i = 15; i >= 0; i--)
				*p++ = palette[BIT(gfx, i)];
		}
	}
	return 0;
}

READ8_MEMBER(lw840_state::illegal_r)
{
	space.device().logerror("%s: unmapped %s memory read from %0*X & %0*X\n", pc(), space.name(), space.addrchars(), space.byte_to_address(offset), 2, mem_mask);
	return 0;
}

WRITE8_MEMBER(lw840_state::illegal_w)
{
	space.device().logerror("%s: unmapped %s memory write to %0*X = %0*X & %0*X\n", pc(), space.name(), space.addrchars(), space.byte_to_address(offset), 2, data, 2, mem_mask);
}

void lw840_state::machine_start()
{
	// try to load map file
	FILE* f;
	if(fopen_s(&f, "lw840.map", "rt") == 0) {
		char line[512];
		do {
			if(fgets(line, sizeof(line), f)) {
				int segment, offset;
				char symbol[512];
				if(sscanf(line, "%x:%x %512s", &segment, &offset, symbol) == 3) {
					uint32_t phys = (segment << 4) + offset;
					//TRACE(_T("%04x:%04x => %02x:%04x\n"), segment, offset, bank, offset);
					symbols[phys] = symbol;
				}
			}
		} while(!feof(f));
		fclose(f);
	}

	vram = reinterpret_cast<uint16_t *>(reinterpret_cast<uintptr_t>(memshare("sram")->ptr()) + 0x300);

	fdc->set_rate(500000);


	// patches here; byte-swapped!!
	rom = memregion("maincpu")->base();
	//rom[0x34a+1] = 0x47; // always branch to mem_test_error
	rom[0x102] = rom[0x102 + 1] = 0xff; // skip printer check
	//rom[0xa380] = rom[0xa380 + 1] = 0; // skip FDC init
}

void lw840_state::machine_reset()
{
}

static INPUT_PORTS_START(lw840)
	PORT_START("kbrow.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_4)      PORT_CHAR('4')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_3)      PORT_CHAR('3')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_W)      PORT_CHAR('w')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_E)      PORT_CHAR('e')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_D)      PORT_CHAR('d')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_X)      PORT_CHAR('x')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_TAB)    PORT_CHAR(UCHAR_MAMEKEY(TAB))

	PORT_START("kbrow.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_5)      PORT_CHAR('5')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_6)      PORT_CHAR('6')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_R)      PORT_CHAR('r')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_T)      PORT_CHAR('t')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_C)      PORT_CHAR('c')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_F)      PORT_CHAR('f')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_UP)                 PORT_CODE(KEYCODE_UP)     PORT_CHAR(UCHAR_MAMEKEY(UP))

	PORT_START("kbrow.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_8)      PORT_CHAR('8')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_7)      PORT_CHAR('7')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_Z)      PORT_CHAR('z')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_H)      PORT_CHAR('h')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_G)      PORT_CHAR('g')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_V)      PORT_CHAR('v')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G.S.END")               PORT_CODE(KEYCODE_END)    PORT_CHAR(UCHAR_MAMEKEY(END))

	PORT_START("kbrow.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_1)          PORT_CHAR('1')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_2)          PORT_CHAR('2')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_Q)          PORT_CHAR('q')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_Y)          PORT_CHAR('y')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_A)          PORT_CHAR('a')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_S)          PORT_CHAR('s')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))

	PORT_START("kbrow.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_9)          PORT_CHAR('9')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_J)          PORT_CHAR('j')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_I)          PORT_CHAR('i')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_U)          PORT_CHAR('u')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_B)          PORT_CHAR('b')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_N)          PORT_CHAR('n')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) //works

	PORT_START("kbrow.5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_MINUS)      PORT_CHAR(L'ß') PORT_CHAR('?')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_0)          PORT_CHAR('0')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_P)          PORT_CHAR('p')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_O)          PORT_CHAR('o')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_M)          PORT_CHAR('m')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR(';')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_MENU)       PORT_CHAR(UCHAR_MAMEKEY(MENU))

	PORT_START("kbrow.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Inhalt")                PORT_CODE(KEYCODE_HOME)       PORT_CHAR(UCHAR_MAMEKEY(HOME)) //works
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_COLON)      PORT_CHAR(L'ö') PORT_CHAR(L'Ö') //works
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('+') PORT_CHAR('*') //works
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR(L'ü') PORT_CHAR(L'Ü') //works
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT)) //works
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN)) //works
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kbrow.7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SM/Layout")             PORT_CODE(KEYCODE_PRTSCR)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("STORNO")                PORT_CODE(KEYCODE_PAUSE)      PORT_CHAR(UCHAR_MAMEKEY(CANCEL))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(UCHAR_MAMEKEY(ENTER)) //works
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(UCHAR_MAMEKEY(BACKSPACE)) //works
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    //PORT_CODE(KEYCODE_)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Horz/Vert")             //PORT_CODE(KEYCODE_)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(UCHAR_MAMEKEY(SPACE))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_MAMEKEY(LSHIFT)) //works

	PORT_START("kbrow.8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(L'´') PORT_CHAR('`') //works
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_L)          PORT_CHAR('l') //works
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('\'')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_K)          PORT_CHAR('k') //works
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR(':') //works
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('-') PORT_CHAR('_') //works
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(L'ä') PORT_CHAR(L'Ä')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

FLOPPY_FORMATS_MEMBER(lw840_state::floppy_formats)
	FLOPPY_PC_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( lw840_floppies )
	SLOT_INTERFACE( "35hd", FLOPPY_35_HD )
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( lw840 )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", H83002, XTAL_14_7456MHz)
	MCFG_CPU_PROGRAM_MAP(lw840_map)
	MCFG_CPU_IO_MAP(lw840_io_map)
	MCFG_H83002_TEND0_CALLBACK(DEVWRITELINE("fdc", gm82c765b_device, tc_line_w))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("1khz", lw840_state, int2_timer_callback, attotime::from_hz(2*1000))

	// video hardware
	MCFG_SCREEN_ADD_MONOCHROME("screen", RASTER, rgb_t::white())
	MCFG_SCREEN_REFRESH_RATE(78.1) // TODO: measure!
	MCFG_SCREEN_UPDATE_DRIVER(lw840_state, screen_update)
	MCFG_SCREEN_SIZE(640, 400)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 400-1)
	MCFG_PALETTE_ADD_MONOCHROME_INVERTED("palette")

	// floppy
	MCFG_GM82C765B_ADD("fdc")
	MCFG_UPD765_INTRQ_CALLBACK(WRITELINE(lw840_state, fdc_interrupt))
	MCFG_UPD765_DRQ_CALLBACK(WRITELINE(lw840_state, fdc_drq))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", lw840_floppies, "35hd", lw840_state::floppy_formats)

	// sound hardware - dummy
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 4000) // 4.0 kHz
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

ROM_START( lw840 )
	ROM_REGION(0x400000, "maincpu", 0)
	ROM_LOAD("us3122-a", 0x00000, 0x400000, CRC(70A3A4A6) SHA1(11e32c7da58800d69af29089f7e7deeab513b1ae))
ROM_END

//    YEAR  NAME  PARENT COMPAT   MACHINE INPUT  CLASS           INIT     COMPANY         FULLNAME            FLAGS
COMP( 1997, lw840,  0,   0,       lw840,  lw840, lw840_state,    0,       "Brother",      "Brother LW-840ic", MACHINE_NODEVICE_PRINTER )
