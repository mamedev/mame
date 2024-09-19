// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Speed Basketball
    © 1992 Sega

    Exciting Boat Race (only sound PCB is dumped)
    © 1993 Sega

    Electromechnical games

    Hardware notes:
    Main PCB marked 'SEGA 1991 171-6202B' with '834-8700' and '921016.0639E' stickers.
    Main components:
    - D70008AC-8 main CPU
    - D71051G serial control unit
    - D71054G programmable timer / counter
    - 2x D71055G parallel interface unit
    - 2x D4701AC incremental encoder counter
    - 315-5338A I/O custom
    - 93C45 serial EEPROM
    - 3773P power monitor / watchdog
    - MB3780A battery power monitor + battery
    - OSC 32.000 MHz
    - 1 4-dip bank

    Sound PCB marked 'SOUND SEGA 1991 MADE IN JAPAN' with '837-8724-01' sticker ('837-9653' on Exciting Boat Race')
    Main components:
    - Z0840008PSC audio CPU
    - 315-5476A Sega PCM
    - 315-5497 Sega QFP100 ASIC
    - YM2151
    - TMP82C51AP-8 (D71051C-10 on Exciting Boat Race)
    - Oki M6253
    - NEC B6391GF QFP80 gate array (next to unpopulated QFP80 silkscreened "5381")
    - 2x NEC D6382GF DSP
    - 2x Sanyo LC7883KM digital filter / DAC
    - OSC 48.000 MHz
    - OSC 16.9344 MHz
    - 1 4-dip bank

    TODO:
    - Outputs (solenoids), LED screen with scores
    - PIT input frequencies, outputs (IRQ, baud rate?)
    - RF5C164 hookup (banking)
    - UART clocks are set to 9600 baud, real clock unknown
    - Verify all clocks, everything is guessed
    - Unknown writes on the sound PCB
    - Hook up dip switches (main PCB and sound PCB)
    - ADC on the sound PCB
    - EEPROM
    - Coin counters

    Notes:
    - Speed Basketball only has some LEDs and lamps, the rest is mechanical.
    Video: https://www.youtube.com/watch?v=efs2KNNncn8
    - Speed Soccer / Speed Shot is on the same hardware
    - The driver contains the ROMs for the sound PCB of Exciting Boat Race as a placeholder, since it's the same PCB as the one used by Speed Basketball.
    Exciting Boat Race is a main + satellites arrangement with video, so when the other PCBs are found it should be moved out of here.

***************************************************************************/

#include "emu.h"

#include "315_5338a.h"

#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/msm6253.h"
#include "machine/nvram.h"
#include "machine/pit8253.h"
#include "machine/upd4701.h"
#include "sound/rf5c68.h"
#include "sound/ymopm.h"
#include "video/hd44780.h"
#include "video/pwm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "speedbsk.lh"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class speedbsk_state : public driver_device
{
public:
	speedbsk_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ppi(*this, "ppi%u", 0U),
		m_lcd(*this, "lcd"),
		m_lamp_pwm(*this, "lamp_pwm%u", 0),
		m_soundbank(*this, "soundbank"),
		m_lamps(*this, "hole_%u", 1U),
		m_start_lamp(*this, "start_lamp")
	{ }

	void speedbsk(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device_array<i8255_device, 2> m_ppi;
	required_device<hd44780_device> m_lcd;
	required_device_array<pwm_display_device, 6> m_lamp_pwm;

	required_memory_bank m_soundbank;

	output_finder<24> m_lamps;
	output_finder<> m_start_lamp;

	void main_map(address_map &map) ATTR_COLD;
	void audio_map(address_map &map) ATTR_COLD;
	void audio_io_map(address_map &map) ATTR_COLD;
	void pcm_map(address_map &map) ATTR_COLD;

	void lcd_palette(palette_device &palette) const;
	HD44780_PIXEL_UPDATE(lcd_pixel_update);

	template<int N> void lamp_w(uint8_t data);
	template<int N> void lamp_output_w(offs_t offset, u8 data);
	void solenoid1_w(uint8_t data);
	void solenoid2_w(uint8_t data);
	void ppi1_porta_w(uint8_t data);
	void ppi1_portc_w(uint8_t data);
	void soundbank_w(uint8_t data);
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void speedbsk_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x8000, 0x8003).r("upd4701_0", FUNC(upd4701_device::read_xy));
	map(0x8200, 0x8203).r("upd4701_1", FUNC(upd4701_device::read_xy));
	map(0x8600, 0x8600).portr("region");
	map(0x8800, 0x8800).portr("photo_sensors");
	map(0x8a00, 0x8a0f).rw("io", FUNC(sega_315_5338a_device::read), FUNC(sega_315_5338a_device::write));
	map(0x8a80, 0x8a83).rw("d71054", FUNC(pit8254_device::read), FUNC(pit8254_device::write));
	map(0x8ac0, 0x8ac1).rw("d71051", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x8b00, 0x8b03).rw(m_ppi[0], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x8b40, 0x8b43).rw(m_ppi[1], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xe000, 0xffff).ram().share("nvram");
}

void speedbsk_state::audio_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("audiocpu", 0);
	map(0x8000, 0x9fff).bankr(m_soundbank);
	map(0xc000, 0xdfff).m("rfsnd", FUNC(rf5c164_device::rf5c164_map));
	map(0xe000, 0xffff).ram();
}

void speedbsk_state::audio_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).nopw();
	map(0x02, 0x03).rw("tmp82c51", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x10, 0x13).nopw(); // misc. outputs
	map(0x20, 0x20).lr8(NAME([]() -> uint8_t { return 0x01; })); // ready for more outputs?
	map(0x21, 0x23).nopw();
	map(0x30, 0x30).w(FUNC(speedbsk_state::soundbank_w));
	map(0x40, 0x41).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x50, 0x50).rw("adc", FUNC(msm6253_device::d0_r), FUNC(msm6253_device::select_w));
}

void speedbsk_state::pcm_map(address_map &map)
{
	map(0x0000, 0xffff).ram();
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( speedbsk )
	PORT_START("switches")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("SHOOT RED")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("SHOOT GRN")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("service_panel")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Service A \xe2\x86\x91 INC") PORT_WRITE_LINE_DEVICE_MEMBER("upd4701_0", upd4701_device, right_w)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Service B \xe2\x86\x93 DEC") PORT_WRITE_LINE_DEVICE_MEMBER("upd4701_0", upd4701_device, left_w)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Service C \xe2\x86\xb2 ENT") PORT_WRITE_LINE_DEVICE_MEMBER("upd4701_1", upd4701_device, right_w)

	PORT_START("unk")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_PGDN) PORT_CODE_INC(KEYCODE_PGUP)

	// this could be the dip switch on the main pcb
	PORT_START("region")
	PORT_DIPNAME( 0x03, 0x00, "Region" )
	PORT_DIPSETTING(    0x00, "Jpn" )
	PORT_DIPSETTING(    0x01, "USA" )
	PORT_DIPSETTING(    0x02, "etc" )
	PORT_DIPSETTING(    0x03, "***" )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("photo_sensors")
	PORT_DIPNAME( 0x01, 0x01, "Corner A" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Corner B" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Corner C" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Corner D" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Goal RED" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Goal GRN" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Poket R" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Poket G" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW") // on main PCB
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED ) // only 4 dips

	PORT_START("DSW2") // on audio PCB
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED ) // only 4 dips

	PORT_START("track_x_red")
	PORT_BIT( 0xfff, 0x000, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(1)

	PORT_START("track_y_red")
	PORT_BIT( 0xfff, 0x000, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(30) PORT_PLAYER(1)

	PORT_START("track_x_grn")
	PORT_BIT( 0xfff, 0x000, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(2)

	PORT_START("track_y_grn")
	PORT_BIT( 0xfff, 0x000, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(30) PORT_PLAYER(2)
INPUT_PORTS_END


//**************************************************************************
//  SERVICE LCD
//**************************************************************************

void speedbsk_state::lcd_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148)); // background
	palette.set_pen_color(1, rgb_t( 92,  83,  88)); // lcd pixel on
	palette.set_pen_color(2, rgb_t(131, 136, 139)); // lcd pixel off
}

HD44780_PIXEL_UPDATE( speedbsk_state::lcd_pixel_update )
{
	// char size is 5x8
	if (x > 4 || y > 7)
		return;

	if (line < 2 && pos < 20)
		bitmap.pix(1 + y + line*8 + line, 1 + pos*6 + x) = state ? 1 : 2;
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

template<int N>
void speedbsk_state::lamp_w(uint8_t data)
{
	// N 0-2: red, 3-5: green
	m_lamp_pwm[N]->matrix(1, data);
}

template<int N>
void speedbsk_state::lamp_output_w(offs_t offset, u8 data)
{
	offset = (N % 3) * 8 + (offset >> 6);
	int mask = (N <= 2) ? 1 : 2;
	int old = m_lamps[offset] & ~mask;

	// output state 0: off, 1: red, 2: green, 3: yellow
	m_lamps[offset] = (data ? mask : 0) | old;
}

void speedbsk_state::solenoid1_w(uint8_t data)
{
	// 7-------  solenoid direction
	// -6------  solenoid on
	// --543210  solenoid select 1xx
}

void speedbsk_state::solenoid2_w(uint8_t data)
{
	// 7-------  not used?
	// -6------  solenoid on
	// --543210  solenoid select 2xx
}

void speedbsk_state::ppi1_porta_w(uint8_t data)
{
	// 7-------  unknown
	// -6------  unknown
	// --5-----  unknown
	// ---4----  unknown
	// ----3---  unknown
	// -----2--  unknown
	// ------1-  coin counter?
	// -------0  start lamp

	m_start_lamp = BIT(data, 0);
}

void speedbsk_state::ppi1_portc_w(uint8_t data)
{
	// 76543---  unknown
	// -----2--  lcd e
	// ------1-  lcd rw
	// -------0  lcd rs

	m_lcd->rs_w(BIT(data, 0));
	m_lcd->rw_w(BIT(data, 1));
	m_lcd->e_w(BIT(data, 2));
}

void speedbsk_state::soundbank_w(uint8_t data)
{
	m_soundbank->set_entry(data);
}

void speedbsk_state::machine_start()
{
	// resolve outputs
	m_lamps.resolve();
	m_start_lamp.resolve();

	m_soundbank->configure_entries(0, 0x100, memregion("audiocpu")->base(), 0x2000);
	m_soundbank->set_entry(0);
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void speedbsk_state::speedbsk(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 32_MHz_XTAL / 4); // actually D70008AC-8, divider guessed
	m_maincpu->set_addrmap(AS_PROGRAM, &speedbsk_state::main_map);

	// placeholder
	clock_device &irqclock(CLOCK(config, "irqclock", 60));
	irqclock.signal_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // MB8464A-10LL-SK + MB8730A + battery

	// placeholder, uarts will operate at 9600 baud
	clock_device &uart_clock(CLOCK(config, "uart_clock", 153600));
	uart_clock.signal_handler().set("d71051", FUNC(i8251_device::write_txc));
	uart_clock.signal_handler().append("d71051", FUNC(i8251_device::write_rxc));
	uart_clock.signal_handler().append("tmp82c51", FUNC(i8251_device::write_txc));
	uart_clock.signal_handler().append("tmp82c51", FUNC(i8251_device::write_rxc));

	i8251_device &uart_main(I8251(config, "d71051", 0));
	uart_main.txd_handler().set("tmp82c51", FUNC(i8251_device::write_rxd));
	uart_main.rts_handler().set("tmp82c51", FUNC(i8251_device::write_cts));

	PIT8254(config, "d71054", 0);

	I8255(config, m_ppi[0]);
	m_ppi[0]->out_pa_callback().set(FUNC(speedbsk_state::solenoid1_w));
	m_ppi[0]->out_pb_callback().set(FUNC(speedbsk_state::solenoid2_w));
	m_ppi[0]->in_pc_callback().set_ioport("switches");

	I8255(config, m_ppi[1]);
	m_ppi[1]->out_pa_callback().set(FUNC(speedbsk_state::ppi1_porta_w));
	m_ppi[1]->out_pb_callback().set(m_lcd, FUNC(hd44780_device::db_w));
	m_ppi[1]->tri_pc_callback().set_constant(0);
	m_ppi[1]->out_pc_callback().set(FUNC(speedbsk_state::ppi1_portc_w));

	upd4701_device &upd1(UPD4701A(config, "upd4701_0"));
	upd1.set_portx_tag("track_x_red");
	upd1.set_porty_tag("track_y_red");

	upd4701_device &upd2(UPD4701A(config, "upd4701_1"));
	upd2.set_portx_tag("track_x_grn");
	upd2.set_porty_tag("track_y_grn");

	sega_315_5338a_device &io(SEGA_315_5338A(config, "io", 32_MHz_XTAL));
	io.out_pa_callback().set(FUNC(speedbsk_state::lamp_w<0>));
	io.out_pb_callback().set(FUNC(speedbsk_state::lamp_w<1>));
	io.out_pc_callback().set(FUNC(speedbsk_state::lamp_w<2>));
	io.out_pd_callback().set(FUNC(speedbsk_state::lamp_w<3>));
	io.out_pe_callback().set(FUNC(speedbsk_state::lamp_w<4>));
	io.out_pf_callback().set(FUNC(speedbsk_state::lamp_w<5>));

	for (int i = 0; i < 6; i++)
		PWM_DISPLAY(config, m_lamp_pwm[i]).set_size(1, 8);

	m_lamp_pwm[0]->output_x().set(FUNC(speedbsk_state::lamp_output_w<0>));
	m_lamp_pwm[1]->output_x().set(FUNC(speedbsk_state::lamp_output_w<1>));
	m_lamp_pwm[2]->output_x().set(FUNC(speedbsk_state::lamp_output_w<2>));
	m_lamp_pwm[3]->output_x().set(FUNC(speedbsk_state::lamp_output_w<3>));
	m_lamp_pwm[4]->output_x().set(FUNC(speedbsk_state::lamp_output_w<4>));
	m_lamp_pwm[5]->output_x().set(FUNC(speedbsk_state::lamp_output_w<5>));

	EEPROM_93C46_8BIT(config, "eeprom"); // Actually 93c45

	// service lcd
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(6*20+1, 19);
	screen.set_visarea(0, 6*20, 0, 19-1);
	screen.set_screen_update(m_lcd, FUNC(hd44780_device::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(speedbsk_state::lcd_palette), 3);

	HD44780(config, m_lcd, 270'000); // TODO: clock not measured, datasheet typical clock used
	m_lcd->set_lcd_size(2, 20);
	m_lcd->set_pixel_update_cb(FUNC(speedbsk_state::lcd_pixel_update));

	config.set_default_layout(layout_speedbsk);

	// TODO: LED screen

	// sound hardware, on sound PCB
	z80_device &audiocpu(Z80(config, "audiocpu", 48_MHz_XTAL / 12)); // divider guessed
	audiocpu.set_addrmap(AS_PROGRAM, &speedbsk_state::audio_map);
	audiocpu.set_addrmap(AS_IO, &speedbsk_state::audio_io_map);

	i8251_device &tmp82c51(I8251(config, "tmp82c51", 0));
	tmp82c51.rxrdy_handler().set_inputline("audiocpu", INPUT_LINE_IRQ0);
	tmp82c51.txd_handler().set("d71051", FUNC(i8251_device::write_rxd));
	tmp82c51.rts_handler().set("d71051", FUNC(i8251_device::write_cts));

	msm6253_device &adc(MSM6253(config, "adc", 0));
	adc.set_input_tag<0>("unk");

	SPEAKER(config, "mono").front_center(); // TODO: verify if stereo

	YM2151(config, "ymsnd", 48_MHz_XTAL / 12).add_route(ALL_OUTPUTS, "mono", 0.75); // unknown clock

	rf5c164_device &rfsnd(RF5C164(config, "rfsnd", 48_MHz_XTAL / 4)); // Sega 315-5476A, unknown clock
	rfsnd.add_route(ALL_OUTPUTS, "mono", 0.75);
	rfsnd.set_addrmap(0, &speedbsk_state::pcm_map);
}


//**************************************************************************
//  ROM DEFINTIONS
//**************************************************************************

ROM_START( speedbsk )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "epr-14666.ic25", 0x0000, 0x8000, CRC(9f6d896a) SHA1(d7133f7bc8225bca14249d354f35ed2e9290567a) )

	ROM_REGION( 0x200000, "audiocpu", 0 )
	ROM_LOAD( "epr-14711.ic13",   0x000000, 0x20000, CRC(9b4322c5) SHA1(f68238a85c10528f87d90d383eed64bef9feacde) ) // 11xxxxxxxxxxxxxxx = 0xFF, almost empty
	ROM_LOAD( "mpr-14712-t.ic14", 0x080000, 0x80000, CRC(4b05f4ba) SHA1(5915d735a6bdf55e651a24342f1ded9c54f5c495) )
	ROM_LOAD( "mpr-14713-t.ic15", 0x100000, 0x80000, BAD_DUMP CRC(16950cda) SHA1(e32413f9585d31624d05d0e594e70bbc7273eb06) ) // was marked as bad by the dumper
	// ic16 empty

	ROM_REGION( 0x400, "mainpcb_plds", 0 )
	ROM_LOAD( "315-5558_peel16v8.ic23", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "315-5559_peel16v8.ic24", 0x200, 0x117, NO_DUMP )

	ROM_REGION( 0x600, "soundpcb_plds", 0 )
	ROM_LOAD( "315-4499_gal16v8.ic6", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "315-5500_gal16v8.ic7", 0x200, 0x117, NO_DUMP )
	ROM_LOAD( "315-5501_gal16v8.ic8", 0x400, 0x117, NO_DUMP )
ROM_END

// TODO: move this out of this driver when rest of the PCBs are found and dumped
ROM_START( boatrace )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION( 0x200000, "audiocpu", 0 )
	ROM_LOAD( "epr-15892.ic13", 0x000000, 0x20000, CRC(3a178efc) SHA1(21b0fa0f962bb0b882d8a9649863fdfb4ecb63db) ) // 11xxxxxxxxxxxxxxx = 0xFF, almost empty
	ROM_LOAD( "epr-15893.ic14", 0x080000, 0x80000, CRC(4cf30c7b) SHA1(20e9432ec0e55451bce1b193c030707216141f96) )
	ROM_LOAD( "epr-15894.ic15", 0x100000, 0x80000, CRC(60adad47) SHA1(7a8f373c2d225794c85ee0e7354ace35e757ffa2) )
	ROM_LOAD( "epr-15895.ic16", 0x180000, 0x80000, CRC(a6bf6eff) SHA1(1a3280464b0b449521d64df665d889c35d6f1fc5) )

	ROM_REGION( 0x600, "soundpcb_plds", 0 )
	ROM_LOAD( "315-4499_gal16v8.ic6", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "315-5500_gal16v8.ic7", 0x200, 0x117, NO_DUMP )
	ROM_LOAD( "315-5501_gal16v8.ic8", 0x400, 0x117, NO_DUMP )
ROM_END


} // Anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT  MACHINE   INPUT     CLASS           INIT        ROTATION  COMPANY  FULLNAME              FLAGS
GAME( 1992, speedbsk, 0,      speedbsk, speedbsk, speedbsk_state, empty_init, ROT0,     "Sega",  "Speed Basketball",   MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME( 1993, boatrace, 0,      speedbsk, speedbsk, speedbsk_state, empty_init, ROT0,     "Sega",  "Exciting Boat Race", MACHINE_IS_SKELETON_MECHANICAL )
