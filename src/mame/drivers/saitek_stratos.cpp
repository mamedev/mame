// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, hap
/***************************************************************************

Scisys Kasparov Stratos Chess Computer

TODO:
- add LCD (7x7 screen + 6 7segs)
- add Turbo King/Corona (same hardware family)
- add endgame rom (softwarelist?)
- clean up driver
- does nvram work? maybe both ram chips battery-backed
- add soft power off with STOP button(writes 0 to control_w), power-on with GO button

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m65c02.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "speaker.h"
#include "video/pwm.h"
#include "screen.h"

// internal artwork
#include "saitek_stratos.lh" // clickable

class stratos_state : public driver_device
{
public:
	stratos_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		maincpu(*this, "maincpu"),
		nvram(*this, "nvram"),
		bank_8000(*this, "bank_8000"),
		bank_4000(*this, "bank_4000"),
		nvram_bank(*this, "nvram_bank"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void stratos(machine_config &config);

	void init_stratos();

private:
	DECLARE_WRITE8_MEMBER(p2000_w);
	DECLARE_READ8_MEMBER(p2200_r);
	DECLARE_WRITE8_MEMBER(p2200_w);
	DECLARE_WRITE8_MEMBER(p2400_w);
	DECLARE_READ8_MEMBER(control_r);
	DECLARE_WRITE8_MEMBER(control_w);
	DECLARE_READ8_MEMBER(lcd_r);
	DECLARE_WRITE8_MEMBER(lcd_w);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void stratos_mem(address_map &map);

	std::unique_ptr<uint8_t[]> nvram_data;
	uint8_t control, led_latch_control;
	uint32_t individual_leds, ind_leds;
	uint8_t latch_AH_red, latch_AH_green, latch_18_red, latch_18_green;
	void show_leds();
	virtual void machine_reset() override;

	required_device<m65c02_device> maincpu;
	required_device<nvram_device> nvram;
	required_memory_bank bank_8000;
	required_memory_bank bank_4000;
	required_memory_bank nvram_bank;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_bit_interface> m_dac;
	required_ioport_array<8> m_inputs;

	bool m_lcd_busy;
};

void stratos_state::init_stratos()
{
	nvram_data = std::make_unique<uint8_t[]>(0x2000);
	nvram->set_base(nvram_data.get(), 0x2000);

	bank_8000 ->configure_entries(0, 2, memregion("roms_8000")->base(), 0x8000);
	bank_4000 ->configure_entries(0, 2, memregion("roms_4000")->base(), 0x4000);
	nvram_bank->configure_entries(0, 2, nvram_data.get(),               0x1000);
}

void stratos_state::machine_reset()
{
	control = 0x00;
	led_latch_control = 0x00;
	individual_leds = 0x00000;
	latch_AH_red = 0;
	latch_AH_green = 0;
	latch_18_red = 0;
	latch_18_green = 0;
	bank_8000 ->set_entry(0);
	bank_4000 ->set_entry(0);
	nvram_bank->set_entry(0);
}

void stratos_state::show_leds()
{
	static char const *const led_pos[18] = {
		nullptr, nullptr, "gPawn", "gKnight", "gBishop", "gRook", "gQueen", "gKing", nullptr, nullptr, "rPawn", "rKnight", "rBishop", "rRook", "rQueen", "rKing", nullptr, nullptr
	};
	char str_red[64];
	char str_green[64];

	char *pr = str_red;
	char *pg = str_green;

	*pr = *pg = 0;

	for(int i=0; i != 18; i++)
		if(individual_leds & (1 << i)) {
			const char *pos = led_pos[i];
			if(!pos)
				pr += sprintf(pr, " <%d>", i);
			else if(pos[0] == 'r')
				pr += sprintf(pr, " %s", pos+1);
			else
				pg += sprintf(pg, " %s", pos+1);
		}

	// Obviously slightly incorrect
	if(!(led_latch_control & 8)) {
		pr += sprintf(pr, " %c%c", 'A' + latch_AH_red, '1' + latch_18_red);
		pg += sprintf(pg, " %c%c", 'A' + latch_AH_green, '1' + latch_18_green);
	}

	logerror("leds R:%s -- G:%s (%s)\n", str_red, str_green, machine().describe_context());


	m_display->matrix_partial(0, 4, ~led_latch_control >> 4 & 0xf, 1 << (led_latch_control & 0xf), false);
	m_display->matrix_partial(4, 2, 1 << (control >> 5 & 1), (~ind_leds & 0xff) | (~control << 6 & 0x100));
}

uint32_t stratos_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	static bool nmi=false;

	if(machine().input().code_pressed(KEYCODE_F1)) {
		if(!nmi) {
			maincpu->pulse_input_line(M65C02_NMI_LINE, attotime::zero);
			nmi = true;
		}
	} else
		nmi = false;


	return 0;
}

WRITE8_MEMBER(stratos_state::p2000_w)
{
	m_dac->write(0);
	led_latch_control = data;

	if(!(data & 0x10))
		latch_18_red = data & 7;
	if(!(data & 0x20))
		latch_18_green = data & 7;
	if(!(data & 0x40))
		latch_AH_red = data & 7;
	if(!(data & 0x80))
		latch_AH_green = data & 7;

	show_leds();
}

READ8_MEMBER(stratos_state::p2200_r)
{
	logerror("p2200_r (%s)\n", machine().describe_context());

	//printf("%X ",led_latch_control&0xf);

	return ~m_board->read_file(led_latch_control & 0xf);
	return machine().rand();
	return 0;
}

WRITE8_MEMBER(stratos_state::p2200_w)
{
	m_dac->write(1);
	logerror("p2200_w %02x -> %02x (%s)\n", data, data^0xff, machine().describe_context());
}

WRITE8_MEMBER(stratos_state::p2400_w)
{
	ind_leds = data;

	if(control & 0x20) {
		individual_leds = individual_leds & 0x100ff;
		individual_leds |= (data ^ 0xff) << 8;
		if(!(control & 0x04))
			individual_leds |= 0x20000;
	} else {
		individual_leds = individual_leds & 0x2ff00;
		individual_leds |= data ^ 0xff;
		if(!(control & 0x04))
			individual_leds |= 0x10000;
	}

	show_leds();
}

READ8_MEMBER(stratos_state::control_r)
{
	static int xx = 0;
	xx = !xx;
	// [d659/d630]
	// d64e:
	//   2000 = f9
	//   2000 = f7
	//    8fb = (2600) & 20

	// d625: test device?
	//   8fb=00 : (00, 80) (05, 80) (06, 80)
	//   8fb=20 : (03, 20) (06, 20) (04, 20)
	//  { 2000=f9, 2000=f0 | first, test (2600) & second
	// -> 3-bit mask
	//  d518: 0c 0a 10 0e 14 18 0b 08 -> 1b, timing on loop at e788
	//  d520: 02cc 035b 0219 0266 01ad 0166 0300 0432 -> 8d7/8


	// table at f70e (index on d545 somehow):
	// 0c 0d 0e 0f 10 11 01 02

	// table at d545:
	// 00 4800 6800
	// 01 4802 6800
	// 02 4800 6802
	// 03 4801 6800
	// 04 4800 6801
	// 05 4801 6801
	// 06 4c00 6800
	// 07 4800 6c00
	// 08 4800 6c00
	// 09 4c00 6800
	// 0a 4c00 6c00
	// 0b 4c00 6c00
	// 0c 4880 6800
	// 0d 4840 6800
	// 0e 4820 6800
	// 0f 4810 6800
	// 10 4808 6800
	// 11 4804 6800
	// 12 4800 6880
	// 13 4800 6840
	// 14 4800 6820
	// 15 4800 6810
	// 16 4800 6808
	// 17 4800 6804
	// 18 4880 6880
	// 19 4840 6840
	// 1a 4820 6820
	// 1b 4810 6810
	// 1c 4808 6808
	// 1d 4804 6804

	// Power up led test table
	// 1208 Ki Green
	// 1308 Qu
	// 1408 Ro
	// 1508 Bi
	// 1608 Kn
	// 1708 Pa
	// 0727 8
	// 0026 7
	// 0025 6
	// 0024 5
	// 0023 4
	// 0022 3
	// 0021 2
	// 0020 1
	// 0080 A
	// 0081 B
	// 0082 C
	// 0083 D
	// 0084 E
	// 0085 F
	// 0086 G
	// 0087 H
	// 0008 -
	// 0c08 Ki Red
	// 0d08 Qu
	// 0e08 Ro
	// 0f08 Bi
	// 1008 Kn
	// 1108 Pa
	// 0108
	// 0208
	// 0308
	// 0408
	// 0617 8
	// 0016 7
	// 0015 6
	// 0014 5
	// 0013 4
	// 0012 3
	// 0011 2
	// 0010 1
	// 0040 A
	// 0041 B
	// 0042 C
	// 0043 D
	// 0044 E
	// 0045 F
	// 0046 G
	// 0047 H
	// 0009
	// 00c2

	// (20) = difficulty level

	logerror("control_r (%s)\n", machine().describe_context());

	u8 data = 0;

	//printf("%X ",led_latch_control&0xf);

//printf("%X ",ind_leds);

	u8 sel = led_latch_control & 0xf;

	if (sel == 8)
	{
		// lcd busy flag?
		data = m_lcd_busy ? 0x20: 0;
		m_lcd_busy = false;
	}

	if (sel < 8)
		data |= m_inputs[sel]->read() << 5;

	return data;
	//return xx ? 0x20 : 0x00;
	//return 0;
	//return 0xe0;
}

WRITE8_MEMBER(stratos_state::control_w)
{
	logerror("control_w %02x bank %d (%s)\n", data, data & 3, machine().describe_context());

	control = data;
	bank_8000->set_entry(data & 1);
	bank_4000->set_entry(data >> 1 & 1); // ?
	nvram_bank->set_entry((data >> 1) & 1);

	show_leds();
}


READ8_MEMBER(stratos_state::lcd_r)
{
	return 0;
}

WRITE8_MEMBER(stratos_state::lcd_w)
{
	m_lcd_busy = true;
	// 08 0b - 00?
	// 04 06 - 05
	// 02 0d - 07
	// 01 00 - 05

	static uint8_t vals[18];
	static int idx = 0;
	if(data == 0)
		idx = 0;
	if(idx/2 >= 18)
		return;
	if(idx & 1)
		vals[idx/2] = (vals[idx/2] & 0xf0) | (data & 0xf);
	else
		vals[idx/2] = (data & 0xf) << 4;

	idx++;
	if(idx == 18*2) {
		logerror("lcd");
		for(auto & val : vals)
			logerror(" %02x", val);
		logerror("\n");
	}
}

void stratos_state::stratos_mem(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x2000).w(FUNC(stratos_state::p2000_w));
	map(0x2200, 0x2200).rw(FUNC(stratos_state::p2200_r), FUNC(stratos_state::p2200_w));
	map(0x2400, 0x2400).w(FUNC(stratos_state::p2400_w));
	map(0x2600, 0x2600).rw(FUNC(stratos_state::control_r), FUNC(stratos_state::control_w));
	map(0x2800, 0x37ff).bankrw("nvram_bank");
	map(0x3800, 0x3800).rw(FUNC(stratos_state::lcd_r), FUNC(stratos_state::lcd_w));
	map(0x4000, 0x7fff).bankr("bank_4000");
	map(0x8000, 0xffff).bankr("bank_8000");
}

static INPUT_PORTS_START( stratos )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) // level
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) // sound
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) // stop?
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) // new game?

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) // rook
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) // pawn
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) // bishop

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) // queen
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) // knight
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) // king

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) // play
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) // tab/color
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) // -

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) // +
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O)

	PORT_START("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) // info
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D)

	PORT_START("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) // normal
INPUT_PORTS_END

void stratos_state::stratos(machine_config &config)
{
	/* basic machine hardware */
	M65C02(config, maincpu, 5.67_MHz_XTAL);
	maincpu->set_addrmap(AS_PROGRAM, &stratos_state::stratos_mem);
	maincpu->set_periodic_int(FUNC(stratos_state::irq0_line_hold), attotime::from_hz(1024));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(100));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_size(7, 7);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(stratos_state::screen_update));

	PWM_DISPLAY(config, m_display).set_size(4+2, 8+1);
	m_display->set_bri_levels(0.05);
	m_display->set_bri_maximum(0.5);

	config.set_default_layout(layout_saitek_stratos);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
}

ROM_START( stratos )
	ROM_REGION(0x10000, "roms_8000", 0)
	ROM_LOAD("w1_728m_u3.u3",  0x0000, 0x8000, CRC(b58a7256) SHA1(75b3a3a65f4ca8d52aa5b17a06319bff59d9014f))
	ROM_LOAD("bw1_918n_u4.u4", 0x8000, 0x8000, CRC(cb0de631) SHA1(f78d40213be21775966cbc832d64acd9b73de632))

	ROM_REGION(0x10000, "roms_4000", 0)
	ROM_FILL(0x00000, 0x10000, 0xff)
ROM_END

/*    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT          COMPANY    FULLNAME            FLAGS */
CONS( 1986, stratos, 0,      0,      stratos, stratos, stratos_state, init_stratos, "SciSys",  "Kasparov Stratos", MACHINE_NOT_WORKING | MACHINE_CLICKABLE_ARTWORK )
