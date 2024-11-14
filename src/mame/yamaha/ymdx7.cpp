// license:BSD-3-Clause
// copyright-holders:David Viens, R. Belmont
/***************************************************************************
    ymdx7.cpp: Preliminary driver for Yamaha DX7
    Copyright 2011-2022 David Viens

    Main CPU is a HD63B03RP
    Sub  CPU is a HD6805S1P-A33 (ROM on chip)
    The FM sound system is done by two discrete chips:
    OPS(YM2128) and EGS(YM2129)
    ///////////////// ////////////////////////////////////////////////////////
    Main CPU PCB fixed pins:
    PIN02:  XTAL : NC
    PIN03:  EXTAL: 4.713250MHz (internally divided by 4 to get 1.178312MHz)
    PIN04: !NMI  : 1 (4.7K pull-up)
    PIN05: !IRQ  : 1 (4.7K pull-up) but... linked to Sub CPU
    PIN07: !STBY : 1 (4.7K pull-up)
    /////////////////////////////////////////////////////////////////////////
    RAM access is done through logic on the PCB when
    bit 6 of port 5 (RAME) is low.
    PORT1:direction 0 (all bits read only!)
    P10 to P17 : 8bits that are mapped to Sub CPU's shared Address/Data line
    PORT2 direction 0x11 (XXX10001)
    P20 : write to   TC4053's X (pin14) (SUB CPU BUSY)
    P21 : read  from TC4053's Y (pin15) (SUB CPU handshake)
    P22 : read  from TC4053's Z (pin 4) (250KHz Clock)
    P23 : read  from MIDI IN
    P24 : write to   MIDI OUT
****************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6801.h"
#include "cpu/m6805/m68705.h"
#include "machine/adc0808.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"

#define LOG_OPS     (1U << 1)
#define LOG_EGS     (1U << 2)

// #define VERBOSE (LOG_GENERAL)
#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"


namespace {

constexpr auto DX7CLOCK    = 9'426'500;

constexpr auto LCD_E  = 0x02;
constexpr auto LCD_RW = 0x04;
constexpr auto LCD_RS = 0x01;


class yamaha_dx7_state : public driver_device
{
public:
	yamaha_dx7_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
		, m_lcdc(*this, "hd44780")
		, m_adc(*this, "adc")
		, m_ppi(*this, "i8255")
	{
	}

	void dx7(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	HD44780_PIXEL_UPDATE(lcd_pixel_update);
	void palette_init(palette_device &palette);

	void dx7_led_w (offs_t offset, u8 data)
	{
		// TODO: LEDs
	}

	u8 dx7_p1_r(offs_t offset);
	u8 dx7_p2_r(offs_t offset);
	void dx7_p2_w(offs_t offset, u8 data);
	void dx7_acept_w(offs_t offset, u8 data);

	void dx7_ops_w(offs_t offset, u8 data)
	{
		LOGMASKED(LOG_OPS, "OPS W: %02X=%02X\n", offset, data);
	};

	void dx7_egs_w(offs_t offset, u8 data)
	{
		LOGMASKED(LOG_EGS, "EGS W: %02X=%02X\n", offset, data);
	};

	void dx7_dac_w  (offs_t offset, u8 data) {}; // TODO: DAC control

	void main_map(address_map &map) ATTR_COLD;
	void sub_map (address_map &map) ATTR_COLD;

	required_device<hd6303r_cpu_device> m_maincpu;
	required_device<hd6805s1_device> m_subcpu;
	required_device<hd44780_device> m_lcdc;
	required_device<adc0808_device> m_adc;
	required_device<i8255_device> m_ppi;

	int m_clk;
	u8 m_PORT1;
	u8 m_PORT2;
	u8 m_irq0;
	u8 m_portA, m_portB;
	int m_acept;

	u8 ppi_porta_r();
	u8 ppi_portb_r();
	u8 ppi_portc_r();
	void ppi_porta_w(u8 data);
	void ppi_portb_w(u8 data);
};

 void yamaha_dx7_state::machine_start()
 {
	 m_irq0 = 1;
	 m_clk = 0;
	 m_PORT1 = m_PORT2 = 0;
	 m_acept = 0;
 }

 void yamaha_dx7_state::dx7_acept_w(offs_t address, u8 data)
{
	m_acept++;
	LOG("ACEPT reg:0x%x = 0x%x \n", address, data);
	m_irq0 = 1;
	m_maincpu->set_input_line(M6800_IRQ_LINE, CLEAR_LINE);
}
u8 yamaha_dx7_state::dx7_p1_r(offs_t)
{
	LOG("reading PORT1\n");
	return m_PORT1;
}
u8 yamaha_dx7_state::dx7_p2_r(offs_t)
{
	LOG("reading PORT2\n");
	u8 temp = 0; //P0 is WRITE! does the CPU re-reads its OWN data? can it?
	if(m_irq0)
	{
		temp = temp | 0x02; // P21 is connected to !IRQ line
	}

	if(m_clk)
	{
		temp = temp | 0x04; // P22 is connected to 250Khz clock
	}

	// TODO: MIDI INPUT
	return temp;
}
void yamaha_dx7_state::dx7_p2_w(offs_t offset, u8 data)
{
	LOG("W PORT2 0x%x\n", data);
	m_PORT2 = data;
}

u8 yamaha_dx7_state::ppi_porta_r()
{
	return 0;
}

u8 yamaha_dx7_state::ppi_portb_r()
{
	return 0;
}

u8 yamaha_dx7_state::ppi_portc_r()
{
	// Sustain and Portamento (active LOW) bits 0 & 1
	// LCD busy in bit 7
	u8 retval = 3;

	if (m_portB & LCD_RW)
	{
		retval |= (m_lcdc->read(0) & 0x80);
	}

	return retval;
}

void yamaha_dx7_state::ppi_porta_w(u8 data)
{
	m_portA = data;
}

void yamaha_dx7_state::ppi_portb_w(u8 data)
{
	// only write on the rising edge of E
	if (!(m_portB & LCD_E) && (data & LCD_E) && !(data & LCD_RW))
	{
		m_lcdc->write(data & LCD_RS, m_portA);
	}
	m_portB = data;
}

HD44780_PIXEL_UPDATE(yamaha_dx7_state::lcd_pixel_update)
{
	if (x < 5 && y < 8 && line < 2 && pos < 16)
	{
		bitmap.pix(line * 8 + y, pos * 6 + x) = state;
	}
}

void yamaha_dx7_state::palette_init(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t(131, 136, 139));
	palette.set_pen_color(1, rgb_t( 92,  83,  88));
}

void yamaha_dx7_state::main_map(address_map &map)
{
	//decoded from IC23 74LS138 using A11 to A15 (2kb blocks)
	//Y0 (0x0000) NC
	//Y1 (0x0800) NC
	//Y2 (0x1000) RAM1
	//Y3 (0x1800) RAM2
	//Y4 (0x2000) RAM3
	//Y5 (0x2800) (another 74LS138), see below
	//Y6 (0x3000) EGS
	//Y7 (0x3800) NC

	map(0x1000, 0x17ff).ram().share("ram1");/* 2kb RAM1 IC19 M5M118P (Voice Memory part1) */
	map(0x1800, 0x1fff).ram().share("ram2");/* 2kb RAM2 IC20 M5M118P (Voice Memory part2) */
	map(0x2000, 0x27ff).ram().share("ram3");/* 2kb RAM3 IC21 M5M118P (Working Area)       */

	// the 0x2800 to 0x2fff range is then resplit by IC24(another 74LS138)
	// A3 A2 A1 A0
	//  0  0  0  X:  Y0 : IC12 8255 (LCD,porta/sustain)
	//  0  0  1  X:  Y1 : IC12 8255 (LCD,porta/sustain)
	//  0  1  0  X:  Y2 : OPS
	//  0  1  1  X:  Y3 : NC
	//  1  0  0  X:  Y4 : NC
	//  1  0  1  X:  Y5 : DAC
	//  1  1  0  X:  Y6 : ACEPT (IC5)
	//  1  1  1  X:  Y7 : LED DRIVERs

	map(0x2800, 0x2803).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x2804, 0x2805).w(FUNC(yamaha_dx7_state::dx7_ops_w));
	map(0x280a, 0x280b).w(FUNC(yamaha_dx7_state::dx7_dac_w));
	map(0x280c, 0x280d).w(FUNC(yamaha_dx7_state::dx7_acept_w));
	map(0x280e, 0x280f).w(FUNC(yamaha_dx7_state::dx7_led_w));

	//EGS (YM2129)
	map(0x3000, 0x37ff).w(FUNC(yamaha_dx7_state::dx7_egs_w));

	//map(0x4000, 0x4fff).rom().region("cartridge", 0); //or RAM!
	map(0xc000, 0xffff).rom().region("program", 0);
}

void yamaha_dx7_state::sub_map(address_map &map)
{
	map(0x0000, 0x07ff).rom().region("subcpu", 0);
}

static INPUT_PORTS_START(dx7)
INPUT_PORTS_END

void yamaha_dx7_state::dx7(machine_config &config)
{
	HD6303R(config, m_maincpu, DX7CLOCK/2); // HD63B03RP
	m_maincpu->set_addrmap(AS_PROGRAM, &yamaha_dx7_state::main_map);
	m_maincpu->in_p1_cb().set(FUNC(yamaha_dx7_state::dx7_p1_r));
	m_maincpu->in_p2_cb().set(FUNC(yamaha_dx7_state::dx7_p2_r));
	m_maincpu->out_p2_cb().set(FUNC(yamaha_dx7_state::dx7_p2_w));

	HD6805S1(config, m_subcpu, 4_MHz_XTAL); // HD6805S1P-A33
	m_subcpu->set_addrmap(AS_PROGRAM, &yamaha_dx7_state::sub_map);

	NVRAM(config, "ram1", nvram_device::DEFAULT_ALL_0); /* 2kb RAM1 IC19 M5M118P (Voice Memory part1) */
	NVRAM(config, "ram2", nvram_device::DEFAULT_ALL_0); /* 2kb RAM2 IC20 M5M118P (Voice Memory part2) */
	NVRAM(config, "ram3", nvram_device::DEFAULT_ALL_0); /* 2kb RAM3 IC21 M5M118P (Working Area)       */

	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set(FUNC(yamaha_dx7_state::ppi_porta_r));
	m_ppi->in_pb_callback().set(FUNC(yamaha_dx7_state::ppi_portb_r));
	m_ppi->in_pc_callback().set(FUNC(yamaha_dx7_state::ppi_portc_r));
	m_ppi->out_pa_callback().set(FUNC(yamaha_dx7_state::ppi_porta_w));
	m_ppi->out_pb_callback().set(FUNC(yamaha_dx7_state::ppi_portb_w));

	M58990(config, m_adc, 8_MHz_XTAL / 16); // M58990P-1; divider not verified (actually clocked by P26 output of sub CPU)

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_color(rgb_t::green());
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(m_lcdc, FUNC(hd44780_device::screen_update));
	screen.set_size(6*16, 8*2);
	screen.set_visarea_full();
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(yamaha_dx7_state::palette_init), 2);

	HD44780(config, m_lcdc, 270'000); // TODO: clock not measured, datasheet typical clock used
	m_lcdc->set_lcd_size(2, 16);
	m_lcdc->set_pixel_update_cb(FUNC(yamaha_dx7_state::lcd_pixel_update));
}

// From service manual there are two rom revisions mentioned:
// one that's split into two 2764's:
// iG 10570(0)  uPD 2764-2    ROM1 DX7~#2600
// iG 10575(0)  uPD 2764-2    ROM2 DX7~#2600
// and a single 27128:
// iG 11461(0)  HN 613128PC86 ROM DX7#2661~
// ig11464, ig11467, and ig11468 are known to exist also.
// ig11464 is dumped but contains no version string.

ROM_START(dx7)
	ROM_REGION(0x4000, "program", 0)
	ROM_DEFAULT_BIOS("v1.8")
	ROM_SYSTEM_BIOS(0, "v1.8", "DX-7 v1.8 24-Oct-85 IG11469")
	ROMX_LOAD( "ig11469.ic14", 0x0000, 0x4000, CRC(6cbb0865) SHA1(715dbb8e96a4df2a7f096b368334a7654860bb26), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "ig11464", "DX-7 v1.? IG11464")
	ROMX_LOAD( "ig11464.ic14", 0x0000, 0x4000, CRC(126c5a98) SHA1(ce4df31878dda9ec27b31c7bc172f16419264b90), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "ig11461", "DX-7 v1.? IG11461")
	ROMX_LOAD( "ig11461.ic14", 0x0000, 0x4000, CRC(fb50c62b) SHA1(21c4995d65d0ae6f4868ac89bf7a4ae81dc3bd31), ROM_BIOS(2))

	ROM_REGION(0x0800, "subcpu", 0)
	ROM_LOAD("hd6805s1p-a33.ic13", 0x0000, 0x800, CRC(ac1d84b3) SHA1(ee0ebb118dd0d282d7c195d3b246a0094b2cb6ad))
ROM_END

} // anonymous namespace


SYST(1983, dx7, 0, 0, dx7, dx7, yamaha_dx7_state, empty_init, "Yamaha", "DX7 Digital Programmable Algorithm Synthesizer", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
