// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

Novag Super Expert (model 878/886/887/902) / Novag Super Forte (model 879/?/901)

Hardware notes (Super Expert)
- R65C02P4 or W65C802P-6 @ 5MHz/6MHz (10MHz/12MHz XTAL)
- 8KB RAM battery-backed, 3*32KB ROM
- HD44780A00 LCD controller (16x1)
- beeper(32KHz/32), IRQ(32KHz/128) via MC14060
- optional R65C51P2 ACIA @ 1.8432MHz, for Super System (only works in version C)
- printer port, magnetic sensors, 8*8 chessboard leds

I/O via TTL, hardware design was very awkward.

Super Forte is very similar, just a cheaper plastic case and chessboard buttons
instead of magnet sensors.

To distinguish between versions, press the Set Level button.
- version A: no selectivity setting (but can turn on/off with level H8)
- version B: default selectivity 3
- version C: default selectivity 5

Note that the H8 option doesn't appear to work with sexperta2, but when doing a
hex compare with sexperta, the program differences are minor.

To identify C program version, start MAME with -rs232 terminal, and afterwards,
input I followed by Enter.

TODO:
- use W65C802 device for version B/C? it works ok but this cpu core emulation is
  not as accurate, and the program doesn't enable extended mode (in other words,
  it always runs in W65C02 emulation mode)

*******************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/m6502/m65c02.h"
#include "machine/clock.h"
#include "machine/mos6551.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "sound/beep.h"
#include "video/hd44780.h"
#include "video/pwm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

// internal artwork
#include "novag_sexpert.lh"
#include "novag_sforte.lh"


namespace {

// Super Expert / shared

class sexpert_state : public driver_device
{
public:
	sexpert_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_rombank(*this, "rombank"),
		m_screen(*this, "screen"),
		m_display(*this, "display"),
		m_lcd(*this, "hd44780"),
		m_board(*this, "board"),
		m_acia(*this, "acia"),
		m_rs232(*this, "rs232"),
		m_beeper(*this, "beeper"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	// machine configs
	void sexpert(machine_config &config);
	void sexpertb(machine_config &config);

	void init_sexpert();

	DECLARE_INPUT_CHANGED_MEMBER(change_cpu_freq);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_memory_bank m_rombank;
	required_device<screen_device> m_screen;
	required_device<pwm_display_device> m_display;
	required_device<hd44780_device> m_lcd;
	required_device<sensorboard_device> m_board;
	required_device<mos6551_device> m_acia;
	required_device<rs232_port_device> m_rs232;
	required_device<beep_device> m_beeper;
	required_ioport_array<8> m_inputs;

	u8 m_inp_mux = 0;
	u8 m_lcd_control = 0;
	u8 m_lcd_data = 0;

	// address maps
	void sexpert_map(address_map &map) ATTR_COLD;

	// I/O handlers
	virtual void lcd_control_w(u8 data);
	virtual void lcd_data_w(u8 data);
	void leds_w(u8 data);
	void mux_w(u8 data);
	u8 input1_r();
	u8 input2_r();

	HD44780_PIXEL_UPDATE(lcd_pixel_update);
	void lcd_palette(palette_device &palette) const;
};

void sexpert_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_lcd_control));
	save_item(NAME(m_lcd_data));
}

INPUT_CHANGED_MEMBER(sexpert_state::change_cpu_freq)
{
	// machines were released with either 5MHz or 6MHz CPU
	m_maincpu->set_unscaled_clock((newval & 1) ? (12_MHz_XTAL/2) : (10_MHz_XTAL/2));
}

void sexpert_state::machine_reset()
{
	m_rombank->set_entry(0);
}

void sexpert_state::init_sexpert()
{
	m_rombank->configure_entries(0, 2, memregion("maincpu")->base() + 0x8000, 0x8000);
}


// Super Forte

class sforte_state : public sexpert_state
{
public:
	sforte_state(const machine_config &mconfig, device_type type, const char *tag) :
		sexpert_state(mconfig, type, tag)
	{ }

	// machine configs
	void sforte(machine_config &config);
	void sforteb(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	emu_timer *m_beeptimer = nullptr;

	// address maps
	void sforte_map(address_map &map) ATTR_COLD;

	// I/O handlers
	virtual void lcd_control_w(u8 data) override;
	virtual void lcd_data_w(u8 data) override;

	TIMER_CALLBACK_MEMBER(beep) { m_beeper->set_state(param); }
};

void sforte_state::machine_start()
{
	sexpert_state::machine_start();
	m_beeptimer = timer_alloc(FUNC(sforte_state::beep), this);
}



/*******************************************************************************
    I/O
*******************************************************************************/

// LCD

void sexpert_state::lcd_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0xff, 0xff, 0xff)); // background
	palette.set_pen_color(1, rgb_t(0x00, 0x00, 0x00)); // lcd pixel on
	palette.set_pen_color(2, rgb_t(0xe8, 0xe8, 0xe8)); // lcd pixel off
}

HD44780_PIXEL_UPDATE(sexpert_state::lcd_pixel_update)
{
	// char size is 5x8
	if (x > 4 || y > 7)
		return;

	if (line < 2 && pos < 8)
	{
		// internal: (8+8)*1, external: 1*16
		bitmap.pix(1 + y, 1 + line*8*6 + pos*6 + x) = state ? 1 : 2;
	}
}


// common

void sexpert_state::lcd_control_w(u8 data)
{
	// d0: HD44780 RS
	// d1: HD44780 R/W
	// d2: HD44780 E
	if (m_lcd_control & ~data & 4 && ~data & 2)
		m_lcd->write(m_lcd_control & 1, m_lcd_data);
	m_lcd_control = data & 7;
}

void sexpert_state::lcd_data_w(u8 data)
{
	// d0-d7: HD44780 data
	m_lcd_data = data;
}

void sexpert_state::leds_w(u8 data)
{
	// d0-d7: chessboard leds
	m_display->write_mx(data);
}

void sexpert_state::mux_w(u8 data)
{
	// d0: rom bankswitch
	m_rombank->set_entry(data & 1);

	// d3: enable beeper
	m_beeper->set_state(BIT(data, 3));

	// d4-d7: 74145 to input mux/led select
	m_inp_mux = 1 << (data >> 4 & 0xf) & 0xff;
	m_display->write_my(m_inp_mux);
}

u8 sexpert_state::input1_r()
{
	u8 data = 0;

	// d0-d7: multiplexed inputs (chessboard squares)
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_rank(i, true);

	return ~data;
}

u8 sexpert_state::input2_r()
{
	u8 data = 0;

	// d0-d2: printer port

	// d5-d7: multiplexed inputs (side panel)
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_inputs[i]->read() << 5;

	return ~data & 0xe0;
}


// sforte-specific

void sforte_state::lcd_control_w(u8 data)
{
	// d3: rom bankswitch
	m_rombank->set_entry(data >> 3 & 1);

	// LCD pins: same as sexpert
	sexpert_state::lcd_control_w(data);
	lcd_data_w(m_lcd_data); // refresh inp mux
}

void sforte_state::lcd_data_w(u8 data)
{
	// d0-d2: 74145 to input mux/led select
	// 74145 D from lcd control d2 (HD44780 E)
	m_inp_mux = 1 << ((m_lcd_control << 1 & 8) | (data & 7));
	m_display->write_my(m_inp_mux);

	// d5,d6: led data
	m_display->write_mx(~data >> 5 & 3);

	// d7: enable beeper
	// capacitor for noise filter (sound glitches otherwise)
	if (m_beeptimer->param() != BIT(data, 7))
		m_beeptimer->adjust(attotime::from_msec(1), BIT(data, 7));

	// LCD pins: same as sexpert
	sexpert_state::lcd_data_w(data);
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void sexpert_state::sexpert_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram");
	map(0x1ff0, 0x1fff).unmaprw(); // 8KB RAM, but RAM CE pin is deactivated on $1ff0-$1fff
	map(0x1ff0, 0x1ff0).r(FUNC(sexpert_state::input1_r));
	map(0x1ff1, 0x1ff1).r(FUNC(sexpert_state::input2_r));
	map(0x1ff2, 0x1ff2).nopw(); // printer
	map(0x1ff3, 0x1ff3).nopw(); // printer
	map(0x1ff4, 0x1ff4).w(FUNC(sexpert_state::leds_w));
	map(0x1ff5, 0x1ff5).w(FUNC(sexpert_state::mux_w));
	map(0x1ff6, 0x1ff6).w(FUNC(sexpert_state::lcd_control_w));
	map(0x1ff7, 0x1ff7).w(FUNC(sexpert_state::lcd_data_w));
	map(0x1ffc, 0x1fff).rw("acia", FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0x2000, 0x7fff).rom();
	map(0x8000, 0xffff).bankr("rombank");
}

void sforte_state::sforte_map(address_map &map)
{
	sexpert_map(map);
	map(0x1ff4, 0x1ff5).nopw();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( sexpert )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Go")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Take Back / Analyze Games")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Right")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Set Level")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Flip Display / Time Control")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Left")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Hint / Next Best")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Priority / Tournament Book / Pawn")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Yes/Start / Start of Game")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Trace Forward / AutoPlay")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Pro-Op / Restore Game / Rook")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("No/End / End of Game")

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Clear Board / Delete Pro-Op")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Best Move/Random / Review / Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("Print Book / Store Game")

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Change Color")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Sound / Info / Bishop")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Print Moves / Print Evaluations")

	PORT_START("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Verify/Set Up / Pro-Op Book/Both Books")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Solve Mate / Infinite / Queen")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("Print List / Acc. Time")

	PORT_START("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("New Game")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Player/Player / Gambit Book / King")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("Print Board / Interface")

	PORT_START("CPU")
	PORT_CONFNAME( 0x01, 0x00, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, sexpert_state, change_cpu_freq, 0) // factory set
	PORT_CONFSETTING(    0x00, "5MHz" )
	PORT_CONFSETTING(    0x01, "6MHz" )
INPUT_PORTS_END

static INPUT_PORTS_START( sexpertb )
	PORT_INCLUDE( sexpert )

	PORT_MODIFY("CPU") // default CPU for B/C is W65C802P-6 @ 6MHz
	PORT_CONFNAME( 0x01, 0x01, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, sexpert_state, change_cpu_freq, 0) // factory set
	PORT_CONFSETTING(    0x00, "5MHz" )
	PORT_CONFSETTING(    0x01, "6MHz" )
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void sexpert_state::sexpert(machine_config &config)
{
	// basic machine hardware
	M65C02(config, m_maincpu, 10_MHz_XTAL/2); // or 12_MHz_XTAL/2, also seen with R65C02
	m_maincpu->set_addrmap(AS_PROGRAM, &sexpert_state::sexpert_map);

	auto &irq_clock(CLOCK(config, "irq_clock", 32.768_kHz_XTAL/128)); // 256Hz
	irq_clock.set_pulse_width(attotime::from_nsec(21500)); // active for 21.5us
	irq_clock.signal_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(200));
	m_board->set_nvram_enable(true);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60); // arbitrary
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(6*16+1, 10);
	m_screen->set_visarea_full();
	m_screen->set_screen_update("hd44780", FUNC(hd44780_device::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(sexpert_state::lcd_palette), 3);

	HD44780(config, m_lcd, 270'000); // OSC = 91K resistor
	m_lcd->set_lcd_size(2, 8);
	m_lcd->set_pixel_update_cb(FUNC(sexpert_state::lcd_pixel_update));

	PWM_DISPLAY(config, m_display).set_size(8, 8);
	config.set_default_layout(layout_novag_sexpert);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 32.768_kHz_XTAL/32); // 1024Hz
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.25);

	// uart (configure after video)
	MOS6551(config, m_acia).set_xtal(1.8432_MHz_XTAL); // R65C51P2 - RTS to CTS, DCD to GND
	m_acia->irq_handler().set_inputline("maincpu", m65c02_device::NMI_LINE);
	m_acia->rts_handler().set("acia", FUNC(mos6551_device::write_cts));
	m_acia->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_acia->dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set("acia", FUNC(mos6551_device::write_rxd));
	m_rs232->dsr_handler().set("acia", FUNC(mos6551_device::write_dsr));
}

void sexpert_state::sexpertb(machine_config &config)
{
	sexpert(config);
	m_maincpu->set_clock(12_MHz_XTAL/2);
}

void sforte_state::sforte(machine_config &config)
{
	sexpert(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &sforte_state::sforte_map);
	subdevice<clock_device>("irq_clock")->set_pulse_width(attotime::from_usec(10)); // measured between 8us and 12us (unstable)

	m_board->set_type(sensorboard_device::BUTTONS);

	m_display->set_width(2);
	config.set_default_layout(layout_novag_sforte);
}

void sforte_state::sforteb(machine_config &config)
{
	sforte(config);
	m_maincpu->set_clock(12_MHz_XTAL/2);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( sexperta )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("se_lo_608.u3",     0x00000, 0x8000, CRC(5c98264c) SHA1(fbbe0d0cf64944fd3a90a7e9711b1deef8b9b51d) )
	ROM_LOAD("se_hi1_608.u1",    0x08000, 0x8000, CRC(68009cb4) SHA1(ae8d1b5058eff72d3fcfd6a011608ae7b3de5060) )
	ROM_LOAD("se_sf_hi0_c22.u2", 0x10000, 0x8000, CRC(3e42cf7c) SHA1(b2faa36a127e08e5755167a25ed4a07f12d62957) )
ROM_END

ROM_START( sexperta1 )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("se_202_l.u3",  0x00000, 0x8000, CRC(d51fed16) SHA1(01aaddab36a721a4b9efde94979256d397cd1953) ) // NEC D27C256AD-12
	ROM_LOAD("se_202_h1.u1", 0x08000, 0x8000, CRC(933eafa8) SHA1(293f63a5bc7d760ad675522c98f9f0a49e61aef5) ) // "
	ROM_LOAD("se_c22_h0.u2", 0x10000, 0x8000, CRC(3e42cf7c) SHA1(b2faa36a127e08e5755167a25ed4a07f12d62957) ) // "
ROM_END

ROM_START( sexperta2 )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("se_lo_b15.u3",     0x00000, 0x8000, CRC(6cc9527c) SHA1(29bab809399f2863a88a9c41535ecec0a4fd65ea) )
	ROM_LOAD("se_hi1_b15.u1",    0x08000, 0x8000, CRC(6e57f0c0) SHA1(ea44769a6f54721fd4543366bda932e86e497d43) )
	ROM_LOAD("se_sf_hi0_a23.u2", 0x10000, 0x8000, CRC(7d4e1528) SHA1(53c7d458a5571afae402f00ae3d0f5066634b068) )
ROM_END

ROM_START( sexpertb )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("se_lo_619.u3",    0x00000, 0x8000, CRC(92002eb6) SHA1(ed8ca16701e00b48fa55c856fa4a8c6613079c02) )
	ROM_LOAD("se_hi1_619.u1",   0x08000, 0x8000, CRC(814b4420) SHA1(c553e6a8c048dcc1cf48d410111a86e06b99d356) )
	ROM_LOAD("se_f_hi0_605.u2", 0x10000, 0x8000, CRC(bb07ad52) SHA1(30cf9005021ab2d7b03facdf2d3588bc94dc68a6) )
ROM_END

ROM_START( sexpertc ) // ID = E3.6
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("se_lo_v3.6.u3", 0x00000, 0x8000, CRC(5a29105e) SHA1(be37bb29b530dbba847a5e8d27d81b36525e47f7) )
	ROM_LOAD("se_hi1.u1",     0x08000, 0x8000, CRC(0085c2c4) SHA1(d84bf4afb022575db09dd9dc12e9b330acce35fa) )
	ROM_LOAD("se_hi0.u2",     0x10000, 0x8000, CRC(2d085064) SHA1(76162322aa7d23a5c07e8356d0bbbb33816419af) )
ROM_END

ROM_START( sexpertc1 ) // ID = E3.0
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("se_c24_l.u3",  0x00000, 0x8000, CRC(43ed7a9e) SHA1(273c485e5be6b107b6c5c448003ba7686d4a6d06) )
	ROM_LOAD("se_c23_h1.u1", 0x08000, 0x8000, CRC(0085c2c4) SHA1(d84bf4afb022575db09dd9dc12e9b330acce35fa) )
	ROM_LOAD("se_c22_h0.u2", 0x10000, 0x8000, CRC(2d085064) SHA1(76162322aa7d23a5c07e8356d0bbbb33816419af) )
ROM_END

ROM_START( sexpertc2 ) // ID = E1.2
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("e_111_black.u3", 0x00000, 0x8000, CRC(d0f65341) SHA1(e8ebbfdbcf8ad613cc68acdb0db011eed855cb9f) ) // Toshiba TC57256AD-12
	ROM_LOAD("e_111_red.u1",   0x08000, 0x8000, CRC(59dc112b) SHA1(e1031648da8fc9479d1134d3fd205af254610c1d) ) // "
	ROM_LOAD("c26_green.u2",   0x10000, 0x8000, CRC(c6a1419a) SHA1(017a0ffa9aa59438c879624a7ddea2071d1524b8) ) // "
ROM_END


ROM_START( sfortea )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("sf_lo_609.u13",    0x00000, 0x8000, CRC(88138075) SHA1(bc1f3a2829f7299c81b48202c651cde9b8831157) )
	ROM_LOAD("sf_hi1_609.u11",   0x08000, 0x8000, CRC(ccd35d09) SHA1(9101cbdecdec00aa4de6d72c96ecdffbcf3359f6) )
	ROM_LOAD("se_f_hi0_c22.u12", 0x10000, 0x8000, CRC(3e42cf7c) SHA1(b2faa36a127e08e5755167a25ed4a07f12d62957) )
ROM_END

ROM_START( sfortea1 )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("sfa_lo_204.u13",  0x00000, 0x8000, CRC(78734bfd) SHA1(b6d8e9efccee6f6d0b0cd257a82162bf8ccec719) )
	ROM_LOAD("sfa_hi1_204.u11", 0x08000, 0x8000, CRC(e5e84580) SHA1(bae55c3da7b720bf6ccfb450e383c53cebd5e9ef) )
	ROM_LOAD("sfa_hi0_c22.u12", 0x10000, 0x8000, CRC(3e42cf7c) SHA1(b2faa36a127e08e5755167a25ed4a07f12d62957) )
ROM_END

ROM_START( sfortea2 )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("sfalo.u13",   0x00000, 0x8000, CRC(86e0230a) SHA1(0d6e18a17e636b8c7292c8f331349d361892d1a8) )
	ROM_LOAD("sfahi.u11",   0x08000, 0x8000, CRC(81c02746) SHA1(0bf68b68ade5a3263bead88da0a8965fc71483c1) )
	ROM_LOAD("sfabook.u12", 0x10000, 0x8000, CRC(3e42cf7c) SHA1(b2faa36a127e08e5755167a25ed4a07f12d62957) )
ROM_END

ROM_START( sforteb )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("forte_b_lo.u13",  0x00000, 0x8000, CRC(48bfe5d6) SHA1(323642686b6d2fb8db2b7d50c6cd431058078ce1) )
	ROM_LOAD("forte_b_hi1.u11", 0x08000, 0x8000, CRC(9778ca2c) SHA1(d8b88b9768a1a9171c68cbb0892b817d68d78351) )
	ROM_LOAD("forte_b_hi0.u12", 0x10000, 0x8000, CRC(bb07ad52) SHA1(30cf9005021ab2d7b03facdf2d3588bc94dc68a6) )
ROM_END

ROM_START( sfortec ) // ID = F3.6
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("sfl_124.u13", 0x00000, 0x8000, CRC(c0cba797) SHA1(c630c3552178854a46275c18b0741b0ad1ae3c75) ) // Toshiba TC57256AD-12
	ROM_LOAD("sfh_b15.u11", 0x08000, 0x8000, CRC(e129ec69) SHA1(76feb0d3110a1c7746233cd89c0b2aaae9d0e427) ) // "
	ROM_LOAD("sef_b07.u12", 0x10000, 0x8000, CRC(2d085064) SHA1(76162322aa7d23a5c07e8356d0bbbb33816419af) ) // NEC D27C256AD-12
ROM_END

ROM_START( sfortec1 ) // ID = F1.2
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("sfl_c_111.u13", 0x00000, 0x8000, CRC(f040cf30) SHA1(1fc1220b8ed67cdffa3866d230ce001721cf684f) ) // Toshiba TC57256AD-12
	ROM_LOAD("sfh_c_111.u11", 0x08000, 0x8000, CRC(0f926b32) SHA1(9c7270ecb3f41dd9172a9a7928e6e04e64b2a340) ) // NEC D27C256AD-12
	ROM_LOAD("h0_c_c26.u12",  0x10000, 0x8000, CRC(c6a1419a) SHA1(017a0ffa9aa59438c879624a7ddea2071d1524b8) ) // Toshiba TC57256AD-12
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME       PARENT    COMPAT  MACHINE   INPUT     CLASS          INIT          COMPANY, FULLNAME, FLAGS
SYST( 1988, sexperta,  0,        0,      sexpert,  sexpert,  sexpert_state, init_sexpert, "Novag Industries / Intelligent Heuristic Programming", "Super Expert (version A, set 1)", MACHINE_SUPPORTS_SAVE ) // 886
SYST( 1987, sexperta1, sexperta, 0,      sexpert,  sexpert,  sexpert_state, init_sexpert, "Novag Industries / Intelligent Heuristic Programming", "Super Expert (version A, set 2)", MACHINE_SUPPORTS_SAVE ) // 878
SYST( 1987, sexperta2, sexperta, 0,      sexpert,  sexpert,  sexpert_state, init_sexpert, "Novag Industries / Intelligent Heuristic Programming", "Super Expert (version A, set 3)", MACHINE_SUPPORTS_SAVE ) // 878
SYST( 1988, sexpertb,  sexperta, 0,      sexpertb, sexpertb, sexpert_state, init_sexpert, "Novag Industries / Intelligent Heuristic Programming", "Super Expert (version B)", MACHINE_SUPPORTS_SAVE ) // 887
SYST( 1990, sexpertc,  sexperta, 0,      sexpertb, sexpertb, sexpert_state, init_sexpert, "Novag Industries / Intelligent Heuristic Programming", "Super Expert (version C, v3.6)", MACHINE_SUPPORTS_SAVE ) // 902
SYST( 1990, sexpertc1, sexperta, 0,      sexpertb, sexpertb, sexpert_state, init_sexpert, "Novag Industries / Intelligent Heuristic Programming", "Super Expert (version C, v3.0)", MACHINE_SUPPORTS_SAVE ) // 902
SYST( 1990, sexpertc2, sexperta, 0,      sexpertb, sexpertb, sexpert_state, init_sexpert, "Novag Industries / Intelligent Heuristic Programming", "Super Expert (version C, v1.2)", MACHINE_SUPPORTS_SAVE ) // 902

SYST( 1987, sfortea,   0,        0,      sforte,   sexpert,  sforte_state,  init_sexpert, "Novag Industries / Intelligent Heuristic Programming", "Super Forte (version A, set 1)", MACHINE_SUPPORTS_SAVE )
SYST( 1987, sfortea1,  sfortea,  0,      sforte,   sexpert,  sforte_state,  init_sexpert, "Novag Industries / Intelligent Heuristic Programming", "Super Forte (version A, set 2)", MACHINE_SUPPORTS_SAVE )
SYST( 1987, sfortea2,  sfortea,  0,      sforte,   sexpert,  sforte_state,  init_sexpert, "Novag Industries / Intelligent Heuristic Programming", "Super Forte (version A, set 3)", MACHINE_SUPPORTS_SAVE )
SYST( 1988, sforteb,   sfortea,  0,      sforteb,  sexpertb, sforte_state,  init_sexpert, "Novag Industries / Intelligent Heuristic Programming", "Super Forte (version B)", MACHINE_SUPPORTS_SAVE )
SYST( 1990, sfortec,   sfortea,  0,      sforteb,  sexpertb, sforte_state,  init_sexpert, "Novag Industries / Intelligent Heuristic Programming", "Super Forte (version C, v3.6)", MACHINE_SUPPORTS_SAVE )
SYST( 1990, sfortec1,  sfortea,  0,      sforteb,  sexpertb, sforte_state,  init_sexpert, "Novag Industries / Intelligent Heuristic Programming", "Super Forte (version C, v1.2)", MACHINE_SUPPORTS_SAVE )
