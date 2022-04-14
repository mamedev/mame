// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger, yoyo_chessboard
/******************************************************************************

Fidelity Designer Display series, 6502 and 68000
(6502-based displayless Designer is in fidel_excel.cpp)

*******************************************************************************

Designer 2100 Display (model 6106) overview:
- 8KB RAM(MS6264L-10), 2*32KB ROM(27C256)
- WDC W65C02P-6 CPU, 6MHz XTAL
- 4-digit LCD panel
- PCB label 510.1130A01

Designer 2000 Display (model 6105): same hardware, no bookrom, 3MHz

*******************************************************************************

Designer Mach III Master 2265 (model 6113) overview:
- 80KB RAM(2*KM6264AL-10, 2*KM62256AP-10), 64KB ROM(2*WSI 27C256L-12)
- MC68HC000P12F CPU, 16MHz XTAL
- IRQ(IPL2) from 555 timer, 1.67ms low, 6us high
- PCB label 510.1134A02

ROM address/data lines are scrambled, presumed for easy placement on PCB and not
for obfuscation. I/O is nearly the same as Designer Display on 6502 hardware.

Designer Mach IV Master 2325 (model 6129) overview:
- 32KB(4*P5164-70) + 512KB(TC518512PL-80) RAM, 64KB ROM(TMS 27C512-120JL)
- MC68EC020RP25 CPU, 20MHz XTAL
- PCB label 510.1149A01
- It has a green "Shift" led instead of red, and ROM is not scrambled.

******************************************************************************/

#include "emu.h"

#include "cpu/m6502/r65c02.h"
#include "cpu/m6502/m65sc02.h"
#include "cpu/m68000/m68000.h"
#include "machine/clock.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "fidel_desdis.lh" // clickable
#include "fidel_desdis_68kg.lh" // clickable
#include "fidel_desdis_68kr.lh" // clickable


namespace {

// Designer Display / shared

class desdis_state : public driver_device
{
public:
	desdis_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_rombank(*this, "rombank"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.0")
	{ }

	// machine configs
	void fdes2000d(machine_config &config);
	void fdes2100d(machine_config &config);

	void init_fdes2100d();

protected:
	virtual void machine_start() override;

	// devices/pointers
	required_device<cpu_device> m_maincpu;
	optional_memory_bank m_rombank;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_bit_interface> m_dac;
	required_ioport m_inputs;

	// address maps
	void fdes2100d_map(address_map &map);

	// I/O handlers
	void update_lcd();
	virtual void control_w(offs_t offset, u8 data);
	virtual void lcd_w(offs_t offset, u8 data);
	virtual u8 input_r(offs_t offset);

	u8 m_select = 0;
	u32 m_lcd_data = 0;
};

void desdis_state::init_fdes2100d()
{
	m_rombank->configure_entries(0, 2, memregion("rombank")->base(), 0x4000);
}

void desdis_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_select));
	save_item(NAME(m_lcd_data));
}

// Designer Master

class desmas_state : public desdis_state
{
public:
	desmas_state(const machine_config &mconfig, device_type type, const char *tag) :
		desdis_state(mconfig, type, tag)
	{ }

	// machine configs
	void fdes2265(machine_config &config);
	void fdes2325(machine_config &config);

	void init_fdes2265();

private:
	// address maps
	void fdes2265_map(address_map &map);
	void fdes2325_map(address_map &map);

	// I/O handlers, slightly different (control_w is d0 instead of d7)
	virtual void control_w(offs_t offset, u8 data) override { desdis_state::control_w(offset, data << 7); }
};

void desmas_state::init_fdes2265()
{
	u16 *rom = (u16*)memregion("maincpu")->base();
	const u32 len = memregion("maincpu")->bytes() / 2;

	// descramble data lines
	for (int i = 0; i < len; i++)
		rom[i] = bitswap<16>(rom[i], 15,14,8,13,9,12,10,11, 3,4,5,7,6,0,1,2);

	// descramble address lines
	std::vector<u16> buf(len);
	memcpy(&buf[0], rom, len*2);
	for (int i = 0; i < len; i++)
		rom[i] = buf[bitswap<24>(i, 23,22,21,20,19,18,17,16, 15,14,13,12,11,8,10,9, 7,6,5,4,3,2,1,0)];
}



/******************************************************************************
    I/O
******************************************************************************/

// TTL/generic

void desdis_state::update_lcd()
{
	u8 mask = (m_select & 8) ? 0 : 0xff;
	for (int i = 0; i < 4; i++)
		m_display->write_row(i+2, (m_lcd_data >> (8*i) & 0xff) ^ mask);
}

void desdis_state::control_w(offs_t offset, u8 data)
{
	// a0-a2,d7: 74259
	u8 mask = 1 << offset;
	m_select = (m_select & ~mask) | ((data & 0x80) ? mask : 0);

	// 74259 Q4-Q7: 7442 a0-a3
	// 7442 0-8: led data, input mux
	u16 sel = 1 << (m_select >> 4 & 0xf);
	u16 led_data = sel & 0x1ff;

	// 7442 9: speaker out
	m_dac->write(BIT(sel, 9));

	// 74259 Q0,Q1: led select (active low)
	m_display->matrix_partial(0, 2, ~m_select & 3, led_data);

	// 74259 Q2: book rom A14
	if (m_rombank != nullptr)
		m_rombank->set_entry(~m_select >> 2 & 1);

	// 74259 Q3: lcd polarity
	update_lcd();
}

void desdis_state::lcd_w(offs_t offset, u8 data)
{
	// a0-a2,d0-d3: 4*74259 to lcd digit segments
	u32 mask = bitswap<8>(1 << offset,3,7,6,0,1,2,4,5);
	for (int i = 0; i < 4; i++)
	{
		m_lcd_data = (m_lcd_data & ~mask) | ((data >> i & 1) ? 0 : mask);
		mask <<= 8;
	}

	update_lcd();
}

u8 desdis_state::input_r(offs_t offset)
{
	u8 sel = m_select >> 4 & 0xf;
	u8 data = 0;

	// a0-a2,d7: multiplexed inputs (active low)
	// read chessboard sensors
	if (sel < 8)
		data = m_board->read_rank(sel ^ 7, true);

	// read button panel
	else if (sel == 8)
		data = m_inputs->read();

	return (data >> offset & 1) ? 0 : 0x80;
}



/******************************************************************************
    Address Maps
******************************************************************************/

void desdis_state::fdes2100d_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x2007).mirror(0x1ff8).rw(FUNC(desdis_state::input_r), FUNC(desdis_state::control_w));
	map(0x4000, 0x7fff).bankr("rombank");
	map(0x6000, 0x6007).mirror(0x1ff8).w(FUNC(desdis_state::lcd_w));
	map(0x8000, 0xffff).rom();
}

void desmas_state::fdes2265_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x00ffff).rom();
	map(0x000000, 0x00000f).w(FUNC(desmas_state::lcd_w)).umask16(0x00ff);
	map(0x044000, 0x047fff).ram();
	map(0x100000, 0x10ffff).ram();
	map(0x140000, 0x14000f).r(FUNC(desmas_state::input_r)).umask16(0xff00);
	map(0x140000, 0x14000f).w(FUNC(desmas_state::control_w)).umask16(0x00ff);
}

void desmas_state::fdes2325_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x00ffff).rom();
	map(0x100000, 0x10000f).w(FUNC(desmas_state::lcd_w)).umask32(0x00ff00ff);
	map(0x140000, 0x14000f).w(FUNC(desmas_state::control_w)).umask32(0x00ff00ff);
	map(0x180000, 0x18000f).r(FUNC(desmas_state::input_r)).umask32(0xff00ff00);
	map(0x300000, 0x37ffff).ram();
	map(0x500000, 0x507fff).ram();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( desdis )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL) PORT_NAME("Clear")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Move / Alternate")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Hint / Info")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Take Back / Replay")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Level / New")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Option / Time")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Verify / Problem")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Shift")
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void desdis_state::fdes2100d(machine_config &config)
{
	/* basic machine hardware */
	M65C02(config, m_maincpu, 6_MHz_XTAL); // W65C02P-6
	m_maincpu->set_addrmap(AS_PROGRAM, &desdis_state::fdes2100d_map);

	auto &irq_clock(CLOCK(config, "irq_clock", 600)); // from 556 timer (22nF, 102K, 1K), ideal frequency is 600Hz
	irq_clock.set_pulse_width(attotime::from_nsec(15250)); // active for 15.25us
	irq_clock.signal_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	/* video hardware */
	PWM_DISPLAY(config, m_display).set_size(2+4, 9);
	m_display->set_segmask(0x3c, 0x7f);
	config.set_default_layout(layout_fidel_desdis);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void desdis_state::fdes2000d(machine_config &config)
{
	fdes2100d(config);

	/* basic machine hardware */
	R65C02(config.replace(), m_maincpu, 3_MHz_XTAL); // R65C02P3
	m_maincpu->set_addrmap(AS_PROGRAM, &desdis_state::fdes2100d_map);
}

void desmas_state::fdes2265(machine_config &config)
{
	fdes2100d(config);

	/* basic machine hardware */
	M68000(config.replace(), m_maincpu, 16_MHz_XTAL); // MC68HC000P12F
	m_maincpu->set_addrmap(AS_PROGRAM, &desmas_state::fdes2265_map);

	auto &irq_clock(CLOCK(config.replace(), "irq_clock", 600)); // from 555 timer, ideal frequency is 600Hz (measured 597Hz)
	irq_clock.set_pulse_width(attotime::from_usec(6)); // active for 6us
	irq_clock.signal_handler().set_inputline(m_maincpu, M68K_IRQ_4);

	config.set_default_layout(layout_fidel_desdis_68kr);
}

void desmas_state::fdes2325(machine_config &config)
{
	fdes2265(config);

	/* basic machine hardware */
	M68EC020(config.replace(), m_maincpu, 20_MHz_XTAL); // MC68EC020RP25
	m_maincpu->set_addrmap(AS_PROGRAM, &desmas_state::fdes2325_map);

	config.set_default_layout(layout_fidel_desdis_68kg);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( fdes2100d ) // model 6106, PCB label 510.1130A01 - this dump came from a post-release bugfix by Fidelity
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("i9_orange.ic9", 0x8000, 0x8000, CRC(83fec02a) SHA1(6f43ab05bc605061989b05d0592dbd184efff9d4) ) // WSI 27C256L-12

	ROM_REGION( 0x8000, "rombank", 0 )
	ROM_LOAD("bk3_white.ic10", 0x0000, 0x8000, CRC(3857cc35) SHA1(f073dafb9fd885c7ddb7fbff10e3653f343ef1c6) ) // WSI 27C256L-12
ROM_END

ROM_START( fdes2000d ) // model 6105, PCB label 510.1130A01
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("27c256.ic9", 0x8000, 0x8000, CRC(b136d1a1) SHA1(8438790a62f45284ff33a0255c5c89f526726d3e) ) // 27C256, no label

	ROM_REGION( 0x8000, "rombank", ROMREGION_ERASEFF ) // no rom in ic10
ROM_END


ROM_START( fdes2265 ) // model 6113, PCB label 510.1134A02
	ROM_REGION16_BE( 0x10000, "maincpu", 0 )
	ROM_LOAD16_BYTE("13_e3_red.ic11",  0x00000, 0x08000, CRC(fc352636) SHA1(e87350eab0c9ec7ea6321d82246c500c9d7b463e) ) // WSI 27C256L-12
	ROM_LOAD16_BYTE("13_o3_blue.ic10", 0x00001, 0x08000, CRC(f335a2d5) SHA1(e0b861d77f394d39ab68ca2ef9ac1ceb9e829aed) ) // "
ROM_END

ROM_START( fdes2265a ) // "
	ROM_REGION16_BE( 0x10000, "maincpu", 0 )
	ROM_LOAD16_BYTE("13_e_red.ic11",  0x00000, 0x08000, CRC(15a35628) SHA1(8213862e129951c6943a80f73cd0b63a31bb1357) ) // WSI 27C256L-12
	ROM_LOAD16_BYTE("13_o_blue.ic10", 0x00001, 0x08000, CRC(81ce7ab2) SHA1(f01a70bcf2fbfe66c7a77d3c4437d897e5cc682d) ) // "
ROM_END


ROM_START( fdes2325 ) // model 6129, PCB label 510.1149A01
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("61_29_white.ic10", 0x00000, 0x10000, CRC(f74157e1) SHA1(87f3f2d584e292f81593e053240d022cc477834d) ) // 27c512

	ROM_REGION( 0x100, "pals", 0 )
	ROM_LOAD("101-1097a01.ic19", 0x000, 0x100, NO_DUMP ) // PALCE16V8Q-25PC
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME       PARENT    CMP MACHINE    INPUT   STATE         INIT            COMPANY, FULLNAME, FLAGS
CONS( 1988, fdes2100d, 0,         0, fdes2100d, desdis, desdis_state, init_fdes2100d, "Fidelity Electronics", "Designer 2100 Display", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1988, fdes2000d, fdes2100d, 0, fdes2000d, desdis, desdis_state, init_fdes2100d, "Fidelity Electronics", "Designer 2000 Display", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

CONS( 1989, fdes2265,  0,         0, fdes2265,  desdis, desmas_state, init_fdes2265,  "Fidelity Electronics", "Designer Mach III Master 2265 (set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1989, fdes2265a, fdes2265,  0, fdes2265,  desdis, desmas_state, init_fdes2265,  "Fidelity Electronics", "Designer Mach III Master 2265 (set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1991, fdes2325,  fdes2265,  0, fdes2325,  desdis, desmas_state, empty_init,     "Fidelity Electronics", "Designer Mach IV 68020 Master 2325", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
