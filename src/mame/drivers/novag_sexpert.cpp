// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/******************************************************************************

Novag Super Expert (model 878/887/902) / Novag Super Forte

Hardware notes (Super Expert)
- 65C02 @ 5MHz or 6MHz (10MHz or 12MHz XTAL)
- 8KB RAM battery-backed, 3*32KB ROM
- HD44780 LCD controller (16x1)
- beeper(32KHz/32), IRQ(32KHz/128) via MC14060
- optional R65C51P2 ACIA @ 1.8432MHz, for IBM PC interface (only works in version C?)
- printer port, magnetic sensors, 8*8 chessboard leds

I/O via TTL, hardware design was very awkward.

Super Forte is very similar, just a cheaper plastic case and chessboard buttons
instead of magnet sensors.

******************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/m6502/m65c02.h"
#include "machine/sensorboard.h"
#include "machine/mos6551.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "sound/beep.h"
#include "video/hd44780.h"
#include "video/pwm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

// internal artwork
#include "novag_sexpert.lh" // clickable
#include "novag_sforte.lh" // clickable


namespace {

// Super Expert / shared

class sexpert_state : public driver_device
{
public:
	sexpert_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_irq_on(*this, "irq_on"),
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

	// machine drivers
	void sexpert(machine_config &config);

	void init_sexpert();

	DECLARE_INPUT_CHANGED_MEMBER(sexpert_cpu_freq) { sexpert_set_cpu_freq(); }

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<timer_device> m_irq_on;
	required_memory_bank m_rombank;
	required_device<screen_device> m_screen;
	required_device<pwm_display_device> m_display;
	required_device<hd44780_device> m_lcd;
	required_device<sensorboard_device> m_board;
	required_device<mos6551_device> m_acia;
	required_device<rs232_port_device> m_rs232;
	required_device<beep_device> m_beeper;
	required_ioport_array<8> m_inputs;

	void sexpert_set_cpu_freq();

	// address maps
	void sexpert_map(address_map &map);

	// periodic interrupts
	template<int Line> TIMER_DEVICE_CALLBACK_MEMBER(irq_on) { m_maincpu->set_input_line(Line, ASSERT_LINE); }
	template<int Line> TIMER_DEVICE_CALLBACK_MEMBER(irq_off) { m_maincpu->set_input_line(Line, CLEAR_LINE); }

	// I/O handlers
	void update_display();
	virtual DECLARE_WRITE8_MEMBER(lcd_control_w);
	virtual DECLARE_WRITE8_MEMBER(lcd_data_w);
	DECLARE_WRITE8_MEMBER(leds_w);
	DECLARE_WRITE8_MEMBER(mux_w);
	DECLARE_READ8_MEMBER(input1_r);
	DECLARE_READ8_MEMBER(input2_r);

	HD44780_PIXEL_UPDATE(lcd_pixel_update);
	void lcd_palette(palette_device &palette) const;

	u8 m_inp_mux;
	u8 m_led_data;
	u8 m_lcd_control;
	u8 m_lcd_data;
};

void sexpert_state::machine_start()
{
	// zerofill
	m_inp_mux = 0;
	m_led_data = 0;
	m_lcd_control = 0;
	m_lcd_data = 0;

	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_led_data));
	save_item(NAME(m_lcd_control));
	save_item(NAME(m_lcd_data));
}

void sexpert_state::sexpert_set_cpu_freq()
{
	// machines were released with either 5MHz or 6MHz CPU
	m_maincpu->set_unscaled_clock((ioport("FAKE")->read() & 1) ? (12_MHz_XTAL/2) : (10_MHz_XTAL/2));
}

void sexpert_state::machine_reset()
{
	sexpert_set_cpu_freq();
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

	// machine drivers
	void sforte(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// address maps
	void sforte_map(address_map &map);

	// I/O handlers
	virtual DECLARE_WRITE8_MEMBER(lcd_control_w) override;
	virtual DECLARE_WRITE8_MEMBER(lcd_data_w) override;

	TIMER_CALLBACK_MEMBER(beep) { m_beeper->set_state(param); }
	emu_timer *m_beeptimer;
};

void sforte_state::machine_start()
{
	sexpert_state::machine_start();
	m_beeptimer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(sforte_state::beep),this));
}



/******************************************************************************
    I/O
******************************************************************************/

// LCD

void sexpert_state::lcd_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148)); // background
	palette.set_pen_color(1, rgb_t(92, 83, 88)); // lcd pixel on
	palette.set_pen_color(2, rgb_t(131, 136, 139)); // lcd pixel off
}

HD44780_PIXEL_UPDATE(sexpert_state::lcd_pixel_update)
{
	// char size is 5x8
	if (x > 4 || y > 7)
		return;

	if (line < 2 && pos < 8)
	{
		// internal: (8+8)*1, external: 1*16
		bitmap.pix16(1 + y, 1 + line*8*6 + pos*6 + x) = state ? 1 : 2;
	}
}


// TTL/generic

void sexpert_state::update_display()
{
	// update leds (lcd is done separately)
	m_display->matrix(m_inp_mux, m_led_data);
}

WRITE8_MEMBER(sexpert_state::lcd_control_w)
{
	// d0: HD44780 RS
	// d1: HD44780 R/W
	// d2: HD44780 E
	if (m_lcd_control & ~data & 4 && ~data & 2)
		m_lcd->write(m_lcd_control & 1, m_lcd_data);
	m_lcd_control = data & 7;
}

WRITE8_MEMBER(sexpert_state::lcd_data_w)
{
	// d0-d7: HD44780 data
	m_lcd_data = data;
}

WRITE8_MEMBER(sexpert_state::leds_w)
{
	// d0-d7: chessboard leds
	m_led_data = data;
	update_display();
}

WRITE8_MEMBER(sexpert_state::mux_w)
{
	// d0: rom bankswitch
	m_rombank->set_entry(data & 1);

	// d3: enable beeper
	m_beeper->set_state(data >> 3 & 1);

	// d4-d7: 74145 to input mux/led select
	m_inp_mux = 1 << (data >> 4 & 0xf) & 0xff;
	update_display();
}

READ8_MEMBER(sexpert_state::input1_r)
{
	u8 data = 0;

	// d0-d7: multiplexed inputs (chessboard squares)
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_rank(i, true);

	return ~data;
}

READ8_MEMBER(sexpert_state::input2_r)
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

WRITE8_MEMBER(sforte_state::lcd_control_w)
{
	// d3: rom bankswitch
	m_rombank->set_entry(data >> 3 & 1);

	// LCD pins: same as sexpert
	sexpert_state::lcd_control_w(space, offset, data);
	lcd_data_w(space, 0, m_lcd_data); // refresh inp mux
}

WRITE8_MEMBER(sforte_state::lcd_data_w)
{
	// d0-d2: 74145 to input mux/led select
	// 74145 D from lcd control d2 (HD44780 E)
	m_inp_mux = 1 << ((m_lcd_control << 1 & 8) | (data & 7));

	// d5,d6: led data
	m_led_data = ~data >> 5 & 3;
	update_display();

	// d7: enable beeper
	// capacitor for noise filter (sound glitches otherwise)
	u8 param = data >> 7 & 1;
	if (param != m_beeptimer->param())
		m_beeptimer->adjust(attotime::from_msec(1), param);

	// LCD pins: same as sexpert
	sexpert_state::lcd_data_w(space, offset, data);
}



/******************************************************************************
    Address Maps
******************************************************************************/

void sexpert_state::sexpert_map(address_map &map)
{
	map(0x0000, 0x1fef).ram().share("nvram"); // 8KB RAM, but RAM CE pin is deactivated on $1ff0-$1fff
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
	map(0x1ff4, 0x1ff4).nopw();
	map(0x1ff5, 0x1ff5).nopw();
	map(0x1ff6, 0x1ff6).w(FUNC(sforte_state::lcd_control_w));
	map(0x1ff7, 0x1ff7).w(FUNC(sforte_state::lcd_data_w));
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( sexpert )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Go")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Take Back / Analyze Games")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("->")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Set Level")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Flip Display / Time Control")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("<-")

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

	PORT_START("FAKE")
	PORT_CONFNAME( 0x01, 0x00, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, sexpert_state, sexpert_cpu_freq, 0) // factory set
	PORT_CONFSETTING(    0x00, "5MHz" )
	PORT_CONFSETTING(    0x01, "6MHz" )
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

void sexpert_state::sexpert(machine_config &config)
{
	/* basic machine hardware */
	M65C02(config, m_maincpu, 10_MHz_XTAL/2); // or 12_MHz_XTAL/2
	m_maincpu->set_addrmap(AS_PROGRAM, &sexpert_state::sexpert_map);

	const attotime irq_period = attotime::from_hz(32.768_kHz_XTAL/128); // 256Hz
	TIMER(config, m_irq_on).configure_periodic(FUNC(sexpert_state::irq_on<M6502_IRQ_LINE>), irq_period);
	m_irq_on->set_start_delay(irq_period - attotime::from_nsec(21500)); // active for 21.5us
	TIMER(config, "irq_off").configure_periodic(FUNC(sexpert_state::irq_off<M6502_IRQ_LINE>), irq_period);

	MOS6551(config, m_acia).set_xtal(1.8432_MHz_XTAL); // R65C51P2 - RTS to CTS, DCD to GND
	m_acia->irq_handler().set_inputline("maincpu", m65c02_device::NMI_LINE);
	m_acia->rts_handler().set("acia", FUNC(mos6551_device::write_cts));
	m_acia->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_acia->dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set("acia", FUNC(mos6551_device::write_rxd));
	m_rs232->dsr_handler().set("acia", FUNC(mos6551_device::write_dsr));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(200));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60); // arbitrary
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(6*16+1, 10);
	m_screen->set_visarea_full();
	m_screen->set_screen_update("hd44780", FUNC(hd44780_device::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(sexpert_state::lcd_palette), 3);

	HD44780(config, m_lcd, 0);
	m_lcd->set_lcd_size(2, 8);
	m_lcd->set_pixel_update_cb(FUNC(sexpert_state::lcd_pixel_update));

	PWM_DISPLAY(config, m_display).set_size(8, 8);
	config.set_default_layout(layout_novag_sexpert);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 32.768_kHz_XTAL/32); // 1024Hz
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.25);
}

void sforte_state::sforte(machine_config &config)
{
	sexpert(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &sforte_state::sforte_map);
	m_irq_on->set_start_delay(m_irq_on->period() - attotime::from_usec(10)); // tlow measured between 8us and 12us (unstable)
	m_board->set_type(sensorboard_device::BUTTONS);

	config.set_default_layout(layout_novag_sforte);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( sexperta )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("se_lo_b15.u3", 0x0000, 0x8000, CRC(6cc9527c) SHA1(29bab809399f2863a88a9c41535ecec0a4fd65ea) )
	ROM_LOAD("se_hi1_b15.u1", 0x8000, 0x8000, CRC(6e57f0c0) SHA1(ea44769a6f54721fd4543366bda932e86e497d43) )
	ROM_LOAD("se_sf_hi0_a23.u2", 0x10000, 0x8000, CRC(7d4e1528) SHA1(53c7d458a5571afae402f00ae3d0f5066634b068) )
ROM_END

ROM_START( sexpertb )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("se_lo_619.u3", 0x0000, 0x8000, CRC(92002eb6) SHA1(ed8ca16701e00b48fa55c856fa4a8c6613079c02) )
	ROM_LOAD("se_hi1_619.u1", 0x8000, 0x8000, CRC(814b4420) SHA1(c553e6a8c048dcc1cf48d410111a86e06b99d356) )
	ROM_LOAD("se_f_hi0_605.u2", 0x10000, 0x8000, CRC(bb07ad52) SHA1(30cf9005021ab2d7b03facdf2d3588bc94dc68a6) )
ROM_END

ROM_START( sexpertb1 )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("se_lo_608.u3", 0x0000, 0x8000, CRC(5c98264c) SHA1(fbbe0d0cf64944fd3a90a7e9711b1deef8b9b51d) )
	ROM_LOAD("se_hi1_608.u1", 0x8000, 0x8000, CRC(68009cb4) SHA1(ae8d1b5058eff72d3fcfd6a011608ae7b3de5060) )
	ROM_LOAD("se_sf_hi0_c22.u2", 0x10000, 0x8000, CRC(3e42cf7c) SHA1(b2faa36a127e08e5755167a25ed4a07f12d62957) )
ROM_END

ROM_START( sexpertc )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("se_lo_v3.6.u3", 0x0000, 0x8000, CRC(5a29105e) SHA1(be37bb29b530dbba847a5e8d27d81b36525e47f7) )
	ROM_LOAD("se_hi1.u1", 0x8000, 0x8000, CRC(0085c2c4) SHA1(d84bf4afb022575db09dd9dc12e9b330acce35fa) )
	ROM_LOAD("se_hi0.u2", 0x10000, 0x8000, CRC(2d085064) SHA1(76162322aa7d23a5c07e8356d0bbbb33816419af) )
ROM_END

ROM_START( sexpertc1 )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("se_lo_v1.2.u3", 0x0000, 0x8000, CRC(43ed7a9e) SHA1(273c485e5be6b107b6c5c448003ba7686d4a6d06) )
	ROM_LOAD("se_hi1.u1", 0x8000, 0x8000, CRC(0085c2c4) SHA1(d84bf4afb022575db09dd9dc12e9b330acce35fa) )
	ROM_LOAD("se_hi0.u2", 0x10000, 0x8000, CRC(2d085064) SHA1(76162322aa7d23a5c07e8356d0bbbb33816419af) )
ROM_END


ROM_START( sfortea )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("sf_lo_609.u3", 0x0000, 0x8000, CRC(88138075) SHA1(bc1f3a2829f7299c81b48202c651cde9b8831157) )
	ROM_LOAD("sf_hi1_609.u1", 0x8000, 0x8000, CRC(ccd35d09) SHA1(9101cbdecdec00aa4de6d72c96ecdffbcf3359f6) )
	ROM_LOAD("se_f_hi0_c22.u2", 0x10000, 0x8000, CRC(3e42cf7c) SHA1(b2faa36a127e08e5755167a25ed4a07f12d62957) )
ROM_END

ROM_START( sfortea1 )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("sfa_lo_204.u3", 0x0000, 0x8000, CRC(78734bfd) SHA1(b6d8e9efccee6f6d0b0cd257a82162bf8ccec719) )
	ROM_LOAD("sfa_hi1_204.u1", 0x8000, 0x8000, CRC(e5e84580) SHA1(bae55c3da7b720bf6ccfb450e383c53cebd5e9ef) )
	ROM_LOAD("sfa_hi0_c22.u2", 0x10000, 0x8000, CRC(3e42cf7c) SHA1(b2faa36a127e08e5755167a25ed4a07f12d62957) )
ROM_END

ROM_START( sfortea2 )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("sfalo.u3", 0x0000, 0x8000, CRC(86e0230a) SHA1(0d6e18a17e636b8c7292c8f331349d361892d1a8) )
	ROM_LOAD("sfahi.u1", 0x8000, 0x8000, CRC(81c02746) SHA1(0bf68b68ade5a3263bead88da0a8965fc71483c1) )
	ROM_LOAD("sfabook.u2", 0x10000, 0x8000, CRC(3e42cf7c) SHA1(b2faa36a127e08e5755167a25ed4a07f12d62957) )
ROM_END

ROM_START( sforteb )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("forte_b_lo.u3", 0x0000, 0x8000, CRC(48bfe5d6) SHA1(323642686b6d2fb8db2b7d50c6cd431058078ce1) )
	ROM_LOAD("forte_b_hi1.u1", 0x8000, 0x8000, CRC(9778ca2c) SHA1(d8b88b9768a1a9171c68cbb0892b817d68d78351) )
	ROM_LOAD("forte_b_hi0.u2", 0x10000, 0x8000, CRC(bb07ad52) SHA1(30cf9005021ab2d7b03facdf2d3588bc94dc68a6) )
ROM_END

ROM_START( sfortec )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("sfl_c_iii.u3", 0x0000, 0x8000, CRC(f040cf30) SHA1(1fc1220b8ed67cdffa3866d230ce001721cf684f) ) // Toshiba TC57256AD-12
	ROM_LOAD("sfh_c_iii.u1", 0x8000, 0x8000, CRC(0f926b32) SHA1(9c7270ecb3f41dd9172a9a7928e6e04e64b2a340) ) // NEC D27C256AD-12
	ROM_LOAD("h0_c_c26.u2", 0x10000, 0x8000, CRC(c6a1419a) SHA1(017a0ffa9aa59438c879624a7ddea2071d1524b8) ) // Toshiba TC57256AD-12
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME       PARENT   CMP MACHINE  INPUT    STATE          INIT          COMPANY, FULLNAME, FLAGS
CONS( 1987, sexperta,  0,        0, sexpert, sexpert, sexpert_state, init_sexpert, "Novag", "Super Expert (version A)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1988, sexpertb,  sexperta, 0, sexpert, sexpert, sexpert_state, init_sexpert, "Novag", "Super Expert (version B, model 887)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1988, sexpertb1, sexperta, 0, sexpert, sexpert, sexpert_state, init_sexpert, "Novag", "Super Expert (version B, model 886)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1990, sexpertc,  sexperta, 0, sexpert, sexpert, sexpert_state, init_sexpert, "Novag", "Super Expert (version C, V3.6)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1990, sexpertc1, sexperta, 0, sexpert, sexpert, sexpert_state, init_sexpert, "Novag", "Super Expert (version C, V1.2)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

CONS( 1987, sfortea,   0,        0, sforte,  sexpert, sforte_state,  init_sexpert, "Novag", "Super Forte (version A, set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1987, sfortea1,  sfortea,  0, sforte,  sexpert, sforte_state,  init_sexpert, "Novag", "Super Forte (version A, set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1987, sfortea2,  sfortea,  0, sforte,  sexpert, sforte_state,  init_sexpert, "Novag", "Super Forte (version A, set 3)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1988, sforteb,   sfortea,  0, sforte,  sexpert, sforte_state,  init_sexpert, "Novag", "Super Forte (version B)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1990, sfortec,   sfortea,  0, sforte,  sexpert, sforte_state,  init_sexpert, "Novag", "Super Forte (version C)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
