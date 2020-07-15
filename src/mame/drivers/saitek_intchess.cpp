// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger, Achim
/******************************************************************************


Hardware notes:
- Synertek 6502A @ ~1.1MHz
- Synertek 6522 VIA
- 2*4KB ROM(Synertek 2332), 2KB RAM(4*M5L2114LP)
- 256 bytes PROM(MMI 6336-1J), 256x4 VRAM(2101-1), RF video
- MM74C923N keyboard encoder, 20 buttons
- tape deck with microphone
- 4-digit 7seg display

TODO: WIP

******************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/6522via.h"
#include "machine/mm74c922.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "video/pwm.h"

#include "screen.h"
#include "speaker.h"

// internal artwork


namespace {

class intchess_state : public driver_device
{
public:
	intchess_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_via(*this, "via"),
		m_encoder(*this, "encoder"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_vram(*this, "vram")
	{ }

	void intchess(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<via6522_device> m_via;
	required_device<mm74c923_device> m_encoder;
	required_device<pwm_display_device> m_display;
	required_device<dac_bit_interface> m_dac;
	required_shared_ptr<u8> m_vram;

	// address maps
	void main_map(address_map &map);

	// I/O handlers
	void vram_w(offs_t offset, u8 data);
	void update_display();
	void seg_w(u8 data);
	void control_w(u8 data);
	u8 control_r();

	u8 m_select = 0;
	u8 m_7seg_data = 0;
};

void intchess_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_select));
	save_item(NAME(m_7seg_data));
}



/******************************************************************************
    I/O
******************************************************************************/

void intchess_state::vram_w(offs_t offset, u8 data)
{
	m_vram[offset] = data & 0xf;
}

void intchess_state::update_display()
{
	m_display->matrix(m_select, m_7seg_data);
}

void intchess_state::seg_w(u8 data)
{
	//printf("a_%X ",data);

	// PA1-PA7: 7seg data
	// PA0: ?
	m_7seg_data = bitswap<8>(~data,0,1,2,3,4,5,6,7);
	update_display();



}


void intchess_state::control_w(u8 data)
{
	//printf("b_%X ",data);

	// PB0-PB3: digit select
	m_select = data & 0xf;
	update_display();

	// PB5-PB7 to tape deck
	// PB5: speaker
	// PB6: ?
	// PB7: output
	m_dac->write(BIT(data, 5));



	//printf("%d",data>>6&1);
}

u8 intchess_state::control_r()
{
	// PB4: 74C923 data available
	return m_encoder->da_r() ? 0x10 : 0x00;
}



/******************************************************************************
    Address Maps
******************************************************************************/

void intchess_state::main_map(address_map &map)
{
	map(0x0000, 0x03ff).mirror(0x0400).ram();
	map(0x0800, 0x0bff).mirror(0x0400).ram();
	map(0x1000, 0x1000).r(m_encoder, FUNC(mm74c923_device::read));
	map(0x1800, 0x18ff).ram().w(FUNC(intchess_state::vram_w)).share("vram");
	map(0xa800, 0xa80f).m(m_via, FUNC(via6522_device::map));
	map(0xc000, 0xdfff).mirror(0x2000).rom();
}




/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( intchess )
	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_1) // a1
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_2) // e5
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_3) // level
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_4) // clear?
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_5) // flash

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) // b2
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_W) // f6
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_E) // newgame?
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_R) // enter
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_T) // zuruck

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_A) // c3
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_S) // g7
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_D) // modus
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_F) // check?
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_G) // altern?

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) // d4
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_X) // h8
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_C) // speichern?
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_V) // setzen
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_B) // vor
INPUT_PORTS_END




/******************************************************************************
    Machine Configs
******************************************************************************/

void intchess_state::intchess(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, 4.433619_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &intchess_state::main_map);

	VIA6522(config, m_via, 4.433619_MHz_XTAL / 4); // DDRA = 0xff, DDRB = 0xef
	m_via->writepa_handler().set(FUNC(intchess_state::seg_w));
	m_via->writepb_handler().set(FUNC(intchess_state::control_w));
	m_via->readpb_handler().set(FUNC(intchess_state::control_r));
	m_via->irq_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	MM74C923(config, m_encoder, 0); // timing parameters unknown
	m_encoder->da_wr_callback().set(m_via, FUNC(via6522_device::write_ca2));
	m_encoder->x1_rd_callback().set_ioport("X1");
	m_encoder->x2_rd_callback().set_ioport("X2");
	m_encoder->x3_rd_callback().set_ioport("X3");
	m_encoder->x4_rd_callback().set_ioport("X4");

	// video hardware
	//screen.screen_vblank().set(m_via, FUNC(via6522_device::write_cb2));


	PWM_DISPLAY(config, m_display).set_size(4, 8);
	m_display->set_segmask(0xf, 0x7f);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( intchess )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("c45015_ytv-lrom.u9", 0xc000, 0x1000, CRC(eef04467) SHA1(5bdcb8d596b91aa06c6ef1ed53ef14d0d13f4194) ) // 2332
	ROM_LOAD("c45016_ytv-hrom.u8", 0xd000, 0x1000, CRC(7e6f85b4) SHA1(4cd15257eae04067160026f9a062a28581f46227) ) // "

	ROM_REGION( 0x100, "gfx", 0 )
	ROM_LOAD("igp.u15", 0x000, 0x100, CRC(bf8358e0) SHA1(880e0d9bd8a75874ba9e51dfb5999b8fcd321a4f) ) // 6336-1
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME      PARENT CMP MACHINE   INPUT     STATE           INIT        COMPANY, FULLNAME, FLAGS
CONS( 1980, intchess, 0,      0, intchess, intchess, intchess_state, empty_init, "SciSys / Intelligent Games", "Intelligent Chess", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_NOT_WORKING )
