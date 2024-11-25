// license:BSD-3-Clause
// copyright-holders:Bartman/Abyss

#include "emu.h"

#include "cpu/z180/z180.h"
#include "machine/timer.h"
#include "video/hd44780.h"

#include "debug/debugcpu.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


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

see https://github.com/BartmanAbyss/brother-hardware/tree/master/0G%20-%20Brother%20AX-145 for datasheets, photos

// Status:
// doesn't go further than "SCHREIBWERK ÜBERPRÜFEN" (check printer)
// TODO: needs european font for HD44780

***************************************************************************/

// hw_config:
// - & 0x1f:
//    6            => deutsch
//    12           => francais/nederlands
//    13           => nederlands
//    16, 17       => francais/deutsch
//    8, 9, 14, 21 => espanol

namespace {

class ax145_state : public driver_device
{
public:
	ax145_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_lcdc(*this, "hd44780"),
		m_ram(*this, "ram", 0x8000, ENDIANNESS_LITTLE),
		m_rom(*this, "maincpu"),
		dictionary_bank(*this, "dictionary")
	{ }

	void ax145(machine_config &config) ATTR_COLD;

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// valid values (bei IO3000=0x0b,0x07) (read_config @ 0x14c87): 0 = german => 0, 1 = german => 1, 2 = espanol => 2, 4 (gehäusedeckel offen) => 3, 8 = francais => 4, 16 = german => 5
	static constexpr uint8_t ID = 1;

	// devices
	required_device<hd64180rp_device> m_maincpu;
	required_device<hd44780_device> m_lcdc;
	memory_share_creator<uint8_t> m_ram;
	required_region_ptr<uint8_t> m_rom;
	required_memory_bank dictionary_bank;

	// config switch
	uint8_t io_3000{};

	// bit 2: R/~W (Read / Not Write)
	// bit 1: RS (Register Select)
	// bit 0: E (Enable)
	uint8_t lcd_signal{};

	void map_program(address_map &map) ATTR_COLD
	{
		map(0x00000, 0x01fff).rom();
		map(0x04000, 0x1ffff).rom();
		map(0x40000, 0x5ffff).bankr(dictionary_bank);
		// RAM is installed in machine_start()
	}

	void map_io(address_map &map) ATTR_COLD
	{
		map.global_mask(0xffff);
		map(0x0000, 0x003f).noprw(); // Z180 internal registers
		//map(0x0040, 0x00ff).rw(FUNC(ax145_state::illegal_io_r), FUNC(ax145_state::illegal_io_w));
		//map(0x2000, 0x2000).w(TODO);
		//map(0x2800, 0x2800).w(TODO);
		map(0x3000, 0x3000).w(FUNC(ax145_state::io_3000_w));
		map(0x3800, 0x3800).r(FUNC(ax145_state::io_3800_r)); // config lower 4 bits
		map(0x4000, 0x4000).r(FUNC(ax145_state::io_4000_r)); // config upper 4 bits
		map(0x5000, 0x5000).w(FUNC(ax145_state::dictionary_bank_w));
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

	void palette(palette_device &palette) const ATTR_COLD
	{
		palette.set_pen_color(0, rgb_t(138, 146, 148));
		palette.set_pen_color(1, rgb_t(92, 83, 88));
	}

	// IO
	void io_3000_w(uint8_t data) { io_3000 = data; }

	// should probably return something different depending on io_3000
	// 0x0014a9 also reads but io_3000=0xfe,0xff...
	uint8_t io_3800_r()
	{
		// config lower 4 bits
		return (~ID) & 0x0f;
	}

	uint8_t io_4000_r()
	{
		// config upper 4 bits
		return (~ID) >> 4;
	}

	void lcd_signal_w(uint8_t data)
	{
		lcd_signal = data;
	}

	uint8_t lcd_data_r()
	{
		if(BIT(lcd_signal, 1)) // RS
			return m_lcdc->data_r();
		else
			return m_lcdc->control_r();
	}
	void lcd_data_w(uint8_t data)
	{
		if(BIT(lcd_signal, 0)) { // E
			if(BIT(lcd_signal, 1)) // RS
				m_lcdc->data_w(data << 4);
			else
				m_lcdc->control_w(data << 4);
		}
	}

	void dictionary_bank_w(uint8_t data)
	{
		dictionary_bank->set_entry(data & 0x03);
	}

	// int2
	TIMER_DEVICE_CALLBACK_MEMBER(int2_timer_callback)
	{
		m_maincpu->set_input_line(INPUT_LINE_IRQ2, ASSERT_LINE);
	}

	uint8_t irq_ack_r()
	{
		if(!machine().side_effects_disabled())
			m_maincpu->set_input_line(INPUT_LINE_IRQ2, CLEAR_LINE);
		return 0;
	}
};

void ax145_state::video_start()
{
}

void ax145_state::machine_start()
{
	m_maincpu->space(AS_PROGRAM).install_ram(0x02000, 0x03fff, m_ram); // first 0x2000 bytes of RAM
	m_maincpu->space(AS_PROGRAM).install_ram(0x62000, 0x69fff, m_ram); // complete 0x8000 bytes of RAM
	dictionary_bank->configure_entries(0, 4, memregion("dictionary")->base(), 0x20000);

	// TODO: ROM patch
}

void ax145_state::machine_reset()
{
}

void ax145_state::ax145(machine_config &config) {
	// basic machine hardware
	HD64180RP(config, m_maincpu, 12'000'000 / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &ax145_state::map_program);
	m_maincpu->set_addrmap(AS_IO, &ax145_state::map_io);

	TIMER(config, "1khz").configure_periodic(FUNC(ax145_state::int2_timer_callback), attotime::from_hz(1000)); // just guessing frequency, based on LW-30, 350, 450

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_color(rgb_t(6, 245, 206));
	screen.set_physical_aspect(480, 128);
	screen.set_refresh_hz(78.1);
	screen.set_size(6*40, 9*2);
	screen.set_visarea_full();
	screen.set_screen_update("hd44780", FUNC(hd44780_device::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(ax145_state::palette), 2);

	HD44780(config, m_lcdc, 270'000); // TODO: Wrong device type, should be HD44780-B02 custom character set mask; clock not measured, datasheet typical clock used
	m_lcdc->set_lcd_size(2, 40);
}

static INPUT_PORTS_START(ax145)
	// TODO
INPUT_PORTS_END

/***************************************************************************
  Machine driver(s)
***************************************************************************/

ROM_START( ax145 )
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD("ua4774-c", 0x00000, 0x20000, CRC(82e0f117) SHA1(2bb2883feb73c7c20e2e3004b3588ba354e52b3a)) // german/french/dutch/spanish
	ROM_REGION(0x80000, "dictionary", 0)
	ROM_LOAD("ua2849-a", 0x00000, 0x80000, CRC(fa8712eb) SHA1(2d3454138c79e75604b30229c05ed8fb8e7d15fe)) // german dictionary
ROM_END

} // anonymous namespace

//    YEAR  NAME  PARENT COMPAT   MACHINE INPUT   CLASS            INIT              COMPANY         FULLNAME  FLAGS
COMP( 198?, ax145,   0,   0,      ax145,  ax145,  ax145_state,     empty_init,       "Brother",      "AX-145", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
