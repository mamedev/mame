// license:BSD-3-Clause
// copyright-holders:Bartman/Abyss

#include "emu.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "machine/timer.h"
#include "cpu/z180/z180.h"
#include "debug/debugcpu.h"
#include "sound/beep.h"
#include "video/hd44780.h"

// 240x18x4 = 960x72
// -log -debug -window -intscalex 4 -intscaley 4 -resolution 960x72 ax145

//////////////////////////////////////////////////////////////////////////
// AX-145
//////////////////////////////////////////////////////////////////////////

/***************************************************************************

Brother AX-145
198?

Hardware:

Main PCB B482360-1
========

#7 Hitachi
HD64180RP6
6 MHz, 80xx Peripherals

#6 Texas Instruments
LM393P
Dual differential comparator, commercial grade

#5 not populated
µPD23C2001

#2 NEC
D43256AC-10L
NEC
Static CMOS RAM 32,768 x 8-Bit

#3 NEC
D23C4001EC-172
UA2849-A
4MBit Mask ROM for Dictionary

#4 NEC
D23C1001EC-522
UA4474-C
1MBit Mask ROM

#1 Texas Instruments
SN74HC00N
Quad 2-Input Positive-NAND Gates

#8 NEC
D65013-A73
UA3001-A
CMOS Gate Array

LCD PCB connected via 12-pin ribbon cable
========
Hitachi
HD44780S-B02
U17978-A
Dot Matrix Liquid Crystal Display Controller/Driver
Custom Font
(driven in 4-bit mode, R/W connected)

2x Hitachi
HD66100F
LCD Driver with 80-Channel Outputs

LCD: 40 characters x 2 lines

// Status:
// doesn't go further than "SCHREIBWERK ÜBERPRÜFEN" (check printer)
// needs european font for HD44780

***************************************************************************/

// hw_config:
// - & 0x1f:
//    6            => deutsch
//    12           => francais/nederlands
//    13           => nederlands
//    16, 17       => francais/deutsch
//    8, 9, 14, 21 => espanol

class ax145_state : public driver_device
{
public:
	ax145_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		maincpu(*this, "maincpu"),
		lcdc(*this, "hd44780"),
		ram(*this, "ram", 0x8000, ENDIANNESS_LITTLE),
		rom(*this, "maincpu"),
		dict_rom(*this, "dictionary")
	{ }

	void ax145(machine_config& config);

private:
	// devices
	required_device<hd64180rp_device> maincpu;
	required_device<hd44780_device> lcdc;
	memory_share_creator<uint8_t> ram;

	required_region_ptr<uint8_t> rom, dict_rom;
	std::map<uint32_t, std::string> symbols;

	// valid values (bei IO3000=0x0b,0x07) (read_config @ 0x14c87): 0 = german => 0, 1 = german => 1, 2 = espanol => 2, 4 (gehäusedeckel offen) => 3, 8 = francais => 4, 16 = german => 5
	static constexpr uint8_t id = 1;

	// config switch
	uint8_t io_3000{};

	// bit 2: R/~W (Read / Not Write)
	// bit 1: RS (Register Select)
	// bit 0: E (Enable)
	uint8_t lcd_signal{};

	// dictionary ROM banking
	uint8_t dict_bank{};

	// driver_device overrides
	void machine_start() override;
	void machine_reset() override;
	void video_start() override;

	void map_program(address_map& map) {
		map(0x00000, 0x01fff).rom();
		map(0x04000, 0x1ffff).rom();
		map(0x40000, 0x5ffff).r(FUNC(ax145_state::dict_r));
		// RAM is installed in machine_start()
	}

	void map_io(address_map& map) {
		map.global_mask(0xffff);
		map(0x0000, 0x003f).noprw(); // Z180 internal registers
		//map(0x0040, 0x00ff).rw(FUNC(ax145_state::illegal_io_r), FUNC(ax145_state::illegal_io_w));
		//map(0x2000, 0x2000).w(TODO);
		//map(0x2800, 0x2800).w(TODO);
		map(0x3000, 0x3000).w(FUNC(ax145_state::io_3000_w));
		map(0x3800, 0x3800).r(FUNC(ax145_state::io_3800_r)); // config lower 4 bits
		map(0x4000, 0x4000).r(FUNC(ax145_state::io_4000_r)); // config upper 4 bits
		map(0x5000, 0x5000).w(FUNC(ax145_state::io_5000_w)); // dictionary banking
		map(0x5800, 0x5800).w(FUNC(ax145_state::lcd_signal_w));
		map(0x6000, 0x6000).rw(FUNC(ax145_state::lcd_data_r), FUNC(ax145_state::lcd_data_w));
		//map(0x6800, 0x6800).w(TODO);
		//map(0x7000, 0x7000).w(TODO);
		//map(0x7800, 0x7800).w(TODO);
		//map(0x8000, 0x8000).w(TODO);
		//map(0x8800, 0x8800).w(TODO);
		//map(0x9000, 0x9000).r(TODO);
		map(0xb000, 0xb000).r(FUNC(ax145_state::irq_ack_r));
		//map(0xb800, 0xb800).rw(TODO, TODO);
	}

	void palette(palette_device &palette) const {
		palette.set_pen_color(0, rgb_t(138, 146, 148));
		palette.set_pen_color(1, rgb_t(92, 83, 88));
	}

	// IO
	void io_3000_w(uint8_t data) { io_3000 = data; }

	// should probably return something different depending on io_3000
	// 0x0014a9 also reads but io_3000=0xfe,0xff...
	uint8_t io_3800_r() {
		//space.device().logerror("%s: IO 3000=%02x\n", pc(), io_3000);

		// config lower 4 bits
		return (~id) & 0x0f;
	}

	uint8_t io_4000_r() {
		//space.device().logerror("%s: IO 3000=%02x\n", pc(), io_3000);

		// config upper 4 bits
		return (~id) >> 4;
	}

	void lcd_signal_w(uint8_t data) {
		lcd_signal = data;
	}

	uint8_t lcd_data_r() {
		if(BIT(lcd_signal, 1)) // RS
			return lcdc->data_r();
		else
			return lcdc->control_r();
	}
	void lcd_data_w(uint8_t data) {
		if(BIT(lcd_signal, 0)) { // E
			if(BIT(lcd_signal, 1)) // RS
				lcdc->data_w(data << 4);
			else
				lcdc->control_w(data << 4);
		}
	}

	void io_5000_w(uint8_t data) {
		dict_bank = data;
	}

	uint8_t dict_r(offs_t offset, uint8_t mem_mask = ~0) {
		if(dict_bank >= 4) {
			logerror("%s: illegal rombank (IO 5000=%02x) read offset %06x\n", pc(), dict_bank, offset);
			return 0x00;
		}
		return dict_rom[offset + dict_bank * 0x20000] & mem_mask;
	}

	// int2
	TIMER_DEVICE_CALLBACK_MEMBER(int2_timer_callback) {
		maincpu->set_input_line(INPUT_LINE_IRQ2, ASSERT_LINE);
	}

	uint8_t irq_ack_r() {
		maincpu->set_input_line(INPUT_LINE_IRQ2, CLEAR_LINE);
		return 0;
	}

	// helpers
	std::string pc();
	std::string symbolize(uint32_t adr);
	std::string callstack();
};

void ax145_state::video_start()
{
}

std::string ax145_state::pc()
{
	class z180_friend : public z180_device { public: using z180_device::memory_translate; friend class ax145_state; };
	auto cpu = static_cast<z180_friend*>(dynamic_cast<z180_device*>(&machine().scheduler().currently_executing()->device()));
	offs_t phys = cpu->pc();
	cpu->memory_translate(AS_PROGRAM, 0, phys);

	return symbolize(phys);
}

std::string ax145_state::symbolize(uint32_t adr)
{
	if(symbols.empty())
		return string_format("%06x", adr);

	auto floor_it = symbols.lower_bound(adr);
	if((floor_it == symbols.end() && !symbols.empty()) || floor_it->first != adr)
		--floor_it;
	if(floor_it != symbols.end())
		return string_format("%s+%x (%06x)", floor_it->second, adr - floor_it->first, adr);
	else
		return string_format("%06x", adr);
}

std::string ax145_state::callstack()
{
	class z180_friend : public z180_device { public: using z180_device::memory_translate; friend class ax145_state; };
	auto cpu = static_cast<z180_friend*>(dynamic_cast<z180_device*>(&machine().scheduler().currently_executing()->device()));
	offs_t pc = cpu->pc();
	cpu->memory_translate(AS_PROGRAM, 0, pc);

	//int depth = 0;
	std::string output;
	output += symbolize(pc) + " >> ";

//	if(output.find("abort") != std::string::npos)
//		__debugbreak();

	return output;
}

void ax145_state::machine_start()
{
/*	// try to load map file
	FILE* f;
	if(fopen_s(&f, "ax145.map", "rt") == 0) {
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
*/

	maincpu->space(AS_PROGRAM).install_ram(0x02000, 0x03fff, ram); // first 0x2000 bytes of RAM
	maincpu->space(AS_PROGRAM).install_ram(0x62000, 0x69fff, ram); // complete 0x8000 bytes of RAM

	// ROM patch
}

void ax145_state::machine_reset()
{
}

void ax145_state::ax145(machine_config& config) {
	// basic machine hardware
	HD64180RP(config, maincpu, 12'000'000 / 2);
	maincpu->set_addrmap(AS_PROGRAM, &ax145_state::map_program);
	maincpu->set_addrmap(AS_IO, &ax145_state::map_io);
	TIMER(config, "1khz").configure_periodic(FUNC(ax145_state::int2_timer_callback), attotime::from_hz(1000)); // just guessing frequency, based on LW-30, 350, 450

	// video hardware
	screen_device& screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_color(rgb_t(6, 245, 206));
	screen.set_physical_aspect(480, 128);
	screen.set_refresh_hz(78.1);
	screen.set_size(6*40, 9*2);
	screen.set_visarea_full();
	screen.set_screen_update("hd44780", FUNC(hd44780_device::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(ax145_state::palette), 2);

	HD44780(config, lcdc, 0);
	lcdc->set_lcd_size(2, 40);
}

static INPUT_PORTS_START(ax145)
	// TODO
INPUT_PORTS_END

/***************************************************************************
  Machine driver(s)
***************************************************************************/

ROM_START( ax145 )
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD("ua4774-c", 0x00000, 0x20000, CRC(82E0F117) SHA1(2bb2883feb73c7c20e2e3004b3588ba354e52b3a)) // german/french/dutch/spanish
	ROM_REGION(0x80000, "dictionary", 0)
	ROM_LOAD("ua2849-a", 0x00000, 0x80000, CRC(FA8712EB) SHA1(2d3454138c79e75604b30229c05ed8fb8e7d15fe)) // german dictionary
ROM_END

//    YEAR  NAME  PARENT COMPAT   MACHINE INPUT   CLASS            INIT              COMPANY         FULLNAME           FLAGS
COMP( 198?, ax145,   0,   0,      ax145,  ax145,  ax145_state,     empty_init,       "Brother",      "Brother AX-145",  MACHINE_NODEVICE_PRINTER | MACHINE_NO_SOUND )
