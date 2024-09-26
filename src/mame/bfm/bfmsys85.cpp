// license:BSD-3-Clause
// copyright-holders:James Wallace
/***************************************************************************

    Bellfruit system85 driver, (under heavy construction !!!)

  ******************************************************************************************




     04-2011: J Wallace: Fixed lamping code.
  19-08-2005: Re-Animator

Standard system85 memorymap
___________________________________________________________________________
   hex     |r/w| D D D D D D D D |
 location  |   | 7 6 5 4 3 2 1 0 | function
-----------+---+-----------------+-----------------------------------------
0000-1FFF  |R/W| D D D D D D D D | RAM (8k) battery backed up
-----------+---+-----------------+-----------------------------------------
2000-21FF  | W | D D D D D D D D | Reel 3 + 4 stepper latch
-----------+---+-----------------+-----------------------------------------
2200-23FF  | W | D D D D D D D D | Reel 1 + 2 stepper latch
-----------+---+-----------------+-----------------------------------------
2400-25FF  | W | D D D D D D D D | vfd + coin inhibits
-----------+---+-----------------+-----------------------------------------
2600-27FF  | W | D D D D D D D D | electro mechanical meters
-----------+---+-----------------+-----------------------------------------
2800-28FF  | W | D D D D D D D D | triacs used for payslides/hoppers
-----------+---+-----------------+-----------------------------------------
2A00       |R/W| D D D D D D D D | MUX data
-----------+---+-----------------+-----------------------------------------
2A01       | W | D D D D D D D D | MUX control
-----------+---+-----------------+-----------------------------------------
2E00       | R | ? ? ? ? ? ? D D | IRQ status
-----------+---+-----------------+-----------------------------------------
3000       | W | D D D D D D D D | AY8912 data
-----------+---+-----------------+-----------------------------------------
3200       | W | D D D D D D D D | AY8912 address reg
-----------+---+-----------------+-----------------------------------------
3402       |R/W| D D D D D D D D | MC6850 control reg
-----------+---+-----------------+-----------------------------------------
3403       |R/W| D D D D D D D D | MC6850 data
-----------+---+-----------------+-----------------------------------------
3600       | W | ? ? ? ? ? ? D D | MUX enable
-----------+---+-----------------+-----------------------------------------
4000-5FFF  | R | D D D D D D D D | ROM (8k)
-----------+---+-----------------+-----------------------------------------
6000-7FFF  | R | D D D D D D D D | ROM (8k)
-----------+---+-----------------+-----------------------------------------
8000-FFFF  | R | D D D D D D D D | ROM (32k)
-----------+---+-----------------+-----------------------------------------

  TODO: - change this

***************************************************************************/

#include "emu.h"
#include "bfm_comn.h"

#include "awpvid.h"

#include "cpu/m6809/m6809.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/meters.h"
#include "machine/nvram.h"
#include "machine/roc10937.h"  // vfd
#include "machine/steppers.h" // stepper motor
#include "sound/ay8910.h"

#include "speaker.h"

#include "bfmsys85.lh"


namespace {

class bfmsys85_state : public driver_device
{
public:
	bfmsys85_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_vfd(*this, "vfd"),
		m_maincpu(*this, "maincpu"),
		m_reel(*this, "reel%u", 0U),
		m_acia6850_0(*this, "acia6850_0"),
		m_meters(*this, "meters"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void bfmsys85(machine_config &config);
	void memmap(address_map &map) ATTR_COLD;

	INTERRUPT_GEN_MEMBER(timer_irq);

	void init_decode();
	void init_nodecode();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	int m_mmtr_latch = 0;
	int m_triac_latch = 0;
	int m_alpha_clock = 0; // always 0
	int m_irq_status = 0;
	int m_optic_pattern = 0;
	int m_locked = 0; // always 0
	int m_is_timer_enabled = 1; // always 1
	int m_coin_inhibits = 0; // always 0
	int m_mux_output_strobe = 0;
	int m_mux_input_strobe = 0;
	int m_mux_input = 0;
	uint8_t m_Inputs[64]{};
	uint8_t m_codec_data[256]{};
	uint8_t m_sys85_data_line_t = 0; // never read
	optional_device<rocvfd_device> m_vfd;
	required_device<cpu_device> m_maincpu;
	required_device_array<stepper_device, 4> m_reel;
	required_device<acia6850_device> m_acia6850_0;
	required_device<meters_device> m_meters;
	output_finder<256> m_lamps;

	template <unsigned N> void reel_optic_cb(int state) { if (state) m_optic_pattern |= (1 << N); else m_optic_pattern &= ~(1 << N); }
	void watchdog_w(uint8_t data);
	uint8_t irqlatch_r();
	void reel12_w(uint8_t data);
	void reel34_w(uint8_t data);
	void mmtr_w(uint8_t data);
	uint8_t mmtr_r();
	void vfd_w(uint8_t data);
	void mux_ctrl_w(uint8_t data);
	uint8_t mux_ctrl_r();
	void mux_data_w(uint8_t data);
	uint8_t mux_data_r();
	void mux_enable_w(uint8_t data);
	void triac_w(uint8_t data);
	uint8_t triac_r();
	void sys85_data_w(int state);
	void write_acia_clock(int state);
	int b85_find_project_string();
};

#define MASTER_CLOCK    (XTAL(4'000'000))

///////////////////////////////////////////////////////////////////////////
// Serial Communications (Where does this go?) ////////////////////////////
///////////////////////////////////////////////////////////////////////////


void bfmsys85_state::sys85_data_w(int state)
{
	m_sys85_data_line_t = state;
}


void bfmsys85_state::write_acia_clock(int state)
{
	m_acia6850_0->write_txc(state);
	m_acia6850_0->write_rxc(state);
}


///////////////////////////////////////////////////////////////////////////
// called if board is reset ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void bfmsys85_state::machine_reset()
{
	m_alpha_clock       = 0;
	m_mmtr_latch        = 0;
	m_triac_latch       = 0;
	m_irq_status        = 0;
	m_is_timer_enabled  = 1;
	m_coin_inhibits     = 0;
	m_mux_output_strobe = 0;
	m_mux_input_strobe  = 0;
	m_mux_input         = 0;

	m_vfd->reset(); // reset display1

	m_locked          = 0x00; // hardware is open
}

///////////////////////////////////////////////////////////////////////////

void bfmsys85_state::watchdog_w(uint8_t data)
{
}

///////////////////////////////////////////////////////////////////////////

INTERRUPT_GEN_MEMBER(bfmsys85_state::timer_irq)
{
	if ( m_is_timer_enabled )
	{
		m_irq_status = 0x01 |0x02; //0xff;
		device.execute().set_input_line(M6809_IRQ_LINE, HOLD_LINE);
	}
}

///////////////////////////////////////////////////////////////////////////

uint8_t bfmsys85_state::irqlatch_r()
{
	int result = m_irq_status | 0x02;

	m_irq_status = 0;

	return result;
}

///////////////////////////////////////////////////////////////////////////

void bfmsys85_state::reel12_w(uint8_t data)
{
	m_reel[0]->update((data>>4)&0x0f);
	m_reel[1]->update( data    &0x0f);

	awp_draw_reel(machine(),"reel1", *m_reel[0]);
	awp_draw_reel(machine(),"reel2", *m_reel[1]);
}

///////////////////////////////////////////////////////////////////////////

void bfmsys85_state::reel34_w(uint8_t data)
{
	m_reel[2]->update((data>>4)&0x0f);
	m_reel[3]->update( data    &0x0f);

	awp_draw_reel(machine(),"reel3", *m_reel[2]);
	awp_draw_reel(machine(),"reel4", *m_reel[3]);
}

///////////////////////////////////////////////////////////////////////////
// mechanical meters //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void bfmsys85_state::mmtr_w(uint8_t data)
{
	int  changed = m_mmtr_latch ^ data;

	m_mmtr_latch = data;

	for (int i=0; i<8; i++)
	if ( changed & (1 << i) )
		m_meters->update(i, data & (1 << i) );

	if ( data ) m_maincpu->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
}
///////////////////////////////////////////////////////////////////////////

uint8_t bfmsys85_state::mmtr_r()
{
	return m_mmtr_latch;
}
///////////////////////////////////////////////////////////////////////////

void bfmsys85_state::vfd_w(uint8_t data)
{
//reset 0x20, clock 0x80, data 0x40

	m_vfd->por(data & 0x20);//inverted?
	m_vfd->sclk(data & 0x80);
	m_vfd->data(!(data & 0x40));
}

//////////////////////////////////////////////////////////////////////////////////
// input / output multiplexers ///////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

void bfmsys85_state::mux_ctrl_w(uint8_t data)
{
	switch ( data & 0xF0 )
	{
		case 0x10:
		//logerror(" sys85 mux: left entry matrix scan %02X\n", data & 0x0F);
		break;

		case 0x20:
		//logerror(" sys85 mux: set scan rate %02X\n", data & 0x0F);
		break;

		case 0x40:
		//logerror(" sys85 mux: read strobe");
		m_mux_input_strobe = data & 0x07;

		if ( m_mux_input_strobe == 5 ) m_Inputs[5] = 0xFF ^ m_optic_pattern;

		m_mux_input = ~m_Inputs[m_mux_input_strobe];
		break;

		case 0x80:
		m_mux_output_strobe = data & 0x0F;
		break;

		case 0xC0:
		//logerror(" sys85 mux: clear all outputs\n");
		break;

		case 0xE0:    // End of interrupt
		break;

	}
}

uint8_t bfmsys85_state::mux_ctrl_r()
{
	// software waits for bit7 to become low

	return 0;
}

void bfmsys85_state::mux_data_w(uint8_t data)
{
	int off = m_mux_output_strobe<<4;

	for (int i = 0; i < 8; i++ )
	{
		m_lamps[off] = BIT(data, i);
		off++;
	}
}

uint8_t bfmsys85_state::mux_data_r()
{
	return m_mux_input;
}

///////////////////////////////////////////////////////////////////////////

void bfmsys85_state::mux_enable_w(uint8_t data)
{
}

///////////////////////////////////////////////////////////////////////////
// payslide triacs ////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void bfmsys85_state::triac_w(uint8_t data)
{
	m_triac_latch = data;
}

///////////////////////////////////////////////////////////////////////////

uint8_t bfmsys85_state::triac_r()
{
	return m_triac_latch;
}

// machine start (called only once) ////////////////////////////////////////

void bfmsys85_state::machine_start()
{
	m_lamps.resolve();

	save_item(NAME(m_mmtr_latch));
	save_item(NAME(m_triac_latch));
	// save_item(NAME(m_alpha_clock));
	save_item(NAME(m_irq_status));
	save_item(NAME(m_optic_pattern));
	// save_item(NAME(m_locked));
	// save_item(NAME(m_is_timer_enabled));
	// save_item(NAME(m_coin_inhibits));
	save_item(NAME(m_mux_output_strobe));
	save_item(NAME(m_mux_input_strobe));
	save_item(NAME(m_mux_input));
	save_item(NAME(m_Inputs));
}

// memory map for bellfruit system85 board ////////////////////////////////

void bfmsys85_state::memmap(address_map &map)
{

	map(0x0000, 0x1fff).ram().share("nvram"); //8k RAM
	map(0x2000, 0x21FF).w(FUNC(bfmsys85_state::reel34_w));         // reel 3+4 latch
	map(0x2200, 0x23FF).w(FUNC(bfmsys85_state::reel12_w));         // reel 1+2 latch
	map(0x2400, 0x25FF).w(FUNC(bfmsys85_state::vfd_w));            // vfd latch

	map(0x2600, 0x27FF).rw(FUNC(bfmsys85_state::mmtr_r), FUNC(bfmsys85_state::mmtr_w));// mechanical meter latch
	map(0x2800, 0x2800).r(FUNC(bfmsys85_state::triac_r));           // payslide triacs
	map(0x2800, 0x29FF).w(FUNC(bfmsys85_state::triac_w));          // triacs

	map(0x2A00, 0x2A00).rw(FUNC(bfmsys85_state::mux_data_r), FUNC(bfmsys85_state::mux_data_w));// mux
	map(0x2A01, 0x2A01).rw(FUNC(bfmsys85_state::mux_ctrl_r), FUNC(bfmsys85_state::mux_ctrl_w));// mux status register
	map(0x2E00, 0x2E00).r(FUNC(bfmsys85_state::irqlatch_r));        // irq latch ( MC6850 / timer )

	map(0x3000, 0x3000).w("aysnd", FUNC(ay8910_device::data_w));
	map(0x3001, 0x3001).nopr(); //sound latch
	map(0x3200, 0x3200).w("aysnd", FUNC(ay8910_device::address_w));

	map(0x3402, 0x3403).w(m_acia6850_0, FUNC(acia6850_device::write));
	map(0x3406, 0x3407).r(m_acia6850_0, FUNC(acia6850_device::read));

	map(0x3600, 0x3600).w(FUNC(bfmsys85_state::mux_enable_w));     // mux enable

	map(0x4000, 0xffff).rom();                     // 48K ROM
	map(0x8000, 0xFFFF).w(FUNC(bfmsys85_state::watchdog_w));       // kick watchdog

}

// machine driver for system85 board //////////////////////////////////////

void bfmsys85_state::bfmsys85(machine_config &config)
{
	MC6809(config, m_maincpu, MASTER_CLOCK);          // 6809 CPU, 1 Mhz Q/E
	m_maincpu->set_addrmap(AS_PROGRAM, &bfmsys85_state::memmap);                        // setup read and write memorymap
	m_maincpu->set_periodic_int(FUNC(bfmsys85_state::timer_irq), attotime::from_hz(1000));              // generate 1000 IRQ's per second

	MSC1937(config, m_vfd);

	ACIA6850(config, m_acia6850_0, 0);
	m_acia6850_0->txd_handler().set(FUNC(bfmsys85_state::sys85_data_w));

	clock_device &acia_clock(CLOCK(config, "acia_clock", 31250*16)); // What are the correct ACIA clocks ?
	acia_clock.signal_handler().set(FUNC(bfmsys85_state::write_acia_clock));

	SPEAKER(config, "mono").front_center();
	AY8912(config, "aysnd", MASTER_CLOCK/4).add_route(ALL_OUTPUTS, "mono", 0.25);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);                       // load/save nv RAM

	REEL(config, m_reel[0], STARPOINT_48STEP_REEL, 1, 3, 0x09, 4);
	m_reel[0]->optic_handler().set(FUNC(bfmsys85_state::reel_optic_cb<0>));
	REEL(config, m_reel[1], STARPOINT_48STEP_REEL, 1, 3, 0x09, 4);
	m_reel[1]->optic_handler().set(FUNC(bfmsys85_state::reel_optic_cb<1>));
	REEL(config, m_reel[2], STARPOINT_48STEP_REEL, 1, 3, 0x09, 4);
	m_reel[2]->optic_handler().set(FUNC(bfmsys85_state::reel_optic_cb<2>));
	REEL(config, m_reel[3], STARPOINT_48STEP_REEL, 1, 3, 0x09, 4);
	m_reel[3]->optic_handler().set(FUNC(bfmsys85_state::reel_optic_cb<3>));

	METERS(config, m_meters, 0).set_number(8);

	config.set_default_layout(layout_bfmsys85);
}

// input ports for system85 board /////////////////////////////////////////

static INPUT_PORTS_START( bfmsys85 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("Fl 5.00")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("Fl 2.50")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("Fl 1.00")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("Fl 0.25")
	PORT_BIT( 0xF0, IP_ACTIVE_HIGH, IPT_UNKNOWN  )

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("10")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("11")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("12")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("13")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("14")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("15")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME("16")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_BUTTON8) PORT_NAME("17")

	//PORT_BIT( 0x01, IP_ACTIVE_HIGH,IPT_SERVICE) PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	//PORT_BIT( 0x02, IP_ACTIVE_HIGH,IPT_SERVICE) PORT_NAME("Bookkeeping") PORT_CODE(KEYCODE_F1) PORT_TOGGLE
INPUT_PORTS_END


// ROM definition /////////////////////////////////////////////////////////

ROM_START( b85scard )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sc271.bin",  0x8000, 0x8000,  CRC(58e9c9df) SHA1(345c5aa279327d7142edc6823aad0cfd40cbeb73))
ROM_END




ROM_START( b85cexpl ) // 350-190 B2LNCE21
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2pceic1.bin", 0x8000, 0x008000, CRC(f90bebe0) SHA1(164b4b9c6fcf493933771d6fa29cbb654bfa3812) )
	ROM_LOAD( "2pceic2.bin", 0x6000, 0x002000, CRC(935e1910) SHA1(8ecdfdccbc77534a68e4c7338882391751243a3a) )
ROM_END

ROM_START( b85royal ) // LNRY11350-128 BE
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bigdeal1.bin", 0x6000, 0x002000, CRC(bded44d2) SHA1(9983d9645aa2d205e07ff7245afc2d0253e7a861) )
	ROM_LOAD( "bigdeal2.bin", 0x8000, 0x008000, CRC(d759641a) SHA1(1992ed59eef02bdb68db5fdaa179f37ed885882c) )
ROM_END



ROM_START( b85bdclb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "39350055 5p 60.bin", 0x8000, 0x008000, CRC(252b6b25) SHA1(1dea905ff366e9519a38251127209964d388b67a) )
	ROM_LOAD( "95715639 5p 60.bin", 0x6000, 0x002000, CRC(57734397) SHA1(3a51b902f93a2bec510029131887b50d0fe3f405) )
ROM_END


ROM_START( b85bdclba )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "15632 5p 75.bin", 0x6000, 0x002000, CRC(6e00c17b) SHA1(43ec1ba1c90d2faf0874f7b43c246db639d85ac8) )
	ROM_LOAD( "50045 5p 75.bin", 0x8000, 0x008000, CRC(2c6ce8f6) SHA1(21c1f078e6199ccf373b8db90dac87e2b28a5697) )
ROM_END

ROM_START( b85bdclbb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "15634 20p 150.bin", 0x6000, 0x002000, CRC(e84e9716) SHA1(1e1383eb7ad251a720547225ac9e2204ba8d2dab) )
	ROM_LOAD( "50047 20p 150.bin", 0x8000, 0x008000, CRC(bef6e8c0) SHA1(d7323ad745a5168a53809eb846d9b64a0f2d5702) )
ROM_END

ROM_START( b85cblit ) // 351-091 BELPCZ22
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cash blitz 39350091 protocol.bin", 0x6000, 0x002000, CRC(d6b99ee5) SHA1(7b70716fb8b592657fc9ce0a22aa12ab75690c79) )
	ROM_LOAD( "cash blitz 39351091protocol.bin", 0x8000, 0x008000, CRC(41e87617) SHA1(ab3565939c296074ddcb3fb46fb364df8cbc47e0) )
ROM_END

ROM_START( b85cblita ) // 350-091 BELNCZ22
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cash blitz 39350091 protocol.bin", 0x6000, 0x002000, CRC(d6b99ee5) SHA1(7b70716fb8b592657fc9ce0a22aa12ab75690c79) ) // is this correct here?
	ROM_LOAD( "39350091.bin", 0x8000, 0x008000, CRC(33eed09e) SHA1(a34861323c7256b7720613989a787049517c716f) )
ROM_END

ROM_START( b85cblitb ) // 350-102 BELNCZ23
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95715670.bin", 0x6000, 0x002000, CRC(d6b99ee5) SHA1(7b70716fb8b592657fc9ce0a22aa12ab75690c79) )  // is this correct here?
	ROM_LOAD( "cash blitz 95717034 p1.bin", 0x8000, 0x008000, CRC(3208ba1d) SHA1(2913eeb09b3699871aedbff92c570affcef5bfcc) )
ROM_END

ROM_START( b85clbpm ) // 350-187 BILNCP11
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club premier 39350187 a.bin", 0x8000, 0x008000, CRC(eafe52c5) SHA1(7aa9550e3adacd6864adf2928734b687ab5b55c6) )
	ROM_LOAD( "club premier 95715167 b.bin", 0x6000, 0x002000, CRC(5cb13935) SHA1(cb79df5c7f57facc33682f582a8d34be6445927d) )
ROM_END

ROM_START( b85dbldl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "d dealer 95717891a.bin", 0x8000, 0x008000, CRC(6adb9552) SHA1(bb586f85f89de6018427c4aceae30f6702cd147a) )
	ROM_LOAD( "d dealer 95715153b.bin", 0x6000, 0x002000, CRC(ace28302) SHA1(887b13a12faf3913dca837c105c21d7fd6e274bd) )
ROM_END

ROM_START( b85dbldla )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "d dealer 95717910.bin", 0x8000, 0x008000, CRC(863710ce) SHA1(22463bfd1f2188c6de6ebf1ba89590359a6099f0) )
	ROM_LOAD( "d dealer 95715161.bin", 0x6000, 0x002000, CRC(90517a9b) SHA1(958ccf624e00ed040f8da2cbaf614cc2ce3c9885) )
ROM_END

ROM_START( b85potp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "potp 95717908 2x1.bin", 0x8000, 0x8000, CRC(953c3e78) SHA1(f14ab2c4337e93605be4baac51b8ad3b9bf0e155) )
	ROM_LOAD( "potp 95715159 2x1.bin", 0x6000, 0x2000, CRC(b47cd8f3) SHA1(bf26fdc440a111dc1326b200281c2dff5c517c67) )
ROM_END

ROM_START( b85dbldlb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "potp 95350166.bin", 0x8000, 0x8000, CRC(45f0effa) SHA1(afd7aabac7da04b5960c2cc55863b917a2692c4f) )
	ROM_LOAD( "potp 95715146.bin", 0x6000, 0x2000, CRC(9557ebc4) SHA1(a9d3b2d901875b9d53ac9500acdb9b725b4edcb5) )
ROM_END


ROM_START( b85hilo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95715636 b.bin", 0x6000, 0x002000, CRC(4a4ee7d8) SHA1(a1e74d063d1a4be28548f70b4786ffb6417382dd) )
	ROM_LOAD( "95717737 a.bin", 0x8000, 0x008000, CRC(75437be0) SHA1(f77e292b5835ab55a1f712dd516490e16b7d5b79) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "hilosil-bfm-95717737-p1.bin", 0x6000, 0x002000, CRC(3f9570e8) SHA1(cf9dfeb2d75833a1002b75b7348f6998124fbb70) ) // this doesn't seem to work with anything..
	ROM_LOAD( "95718737 protocol a.bin", 0x8000, 0x008000, CRC(03fb44bc) SHA1(7054b404f1a35e434525ee19eca203c24b7cc60c) ) // alt 95717737
ROM_END

ROM_START( b85hiloa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95715122 b.bin", 0x6000, 0x002000, CRC(31eb023d) SHA1(1e4c4d3d7f9734a3b7956a8f3ffc2a88fce699b7) )
	ROM_LOAD( "876 2p.bin", 0x8000, 0x008000, CRC(a68bf5f3) SHA1(33953f76f8199083131271290866ec5bb4dc51ba) )
ROM_END

ROM_START( b85ritz )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ritz 5p 95715117.bin", 0x6000, 0x002000, CRC(ba8efd16) SHA1(bdd00209e2d86e18930c100d4ae3d9d042a7afa6) )
	ROM_LOAD( "ritz 5p 95717828.bin", 0x8000, 0x008000, CRC(09110c89) SHA1(3659f567aaab6c2d83a3ed94875843ccd43c0a27) )
ROM_END

ROM_START( b85ritza )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ritz 20p 95715118 2x1.bin", 0x6000, 0x002000, CRC(e24e4bb2) SHA1(6c189220866f057fa35801abc39880bf199c4646) )
	ROM_LOAD( "ritz 20p 95717830 2x1.bin", 0x8000, 0x008000, CRC(b3d84f6c) SHA1(561172b5f729b64aa57b56078ef265de1172df38) )
ROM_END

ROM_START( b85ritzb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ritz 20p 95715116.bin", 0x6000, 0x002000, CRC(96cee69e) SHA1(60bbd48c6da1a5921206340788d9abbf59c10569) )
	ROM_LOAD( "ritz 20p 95717827.bin", 0x8000, 0x008000, CRC(bf31303c) SHA1(820befceeef315e6cf6f43fe17404ab88bbd7926) )
ROM_END

ROM_START( b85ritzc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ritz 10p 95715119 2x1.bin", 0x6000, 0x002000, CRC(a558a02e) SHA1(bd1c2bf2f1eff10cc56abc3db3160576bd8351dc) )
	ROM_LOAD( "ritz 10p 95717831 2x1.bin", 0x8000, 0x008000, CRC(6a8fbf83) SHA1(19c87e40541973e7fc59c76f2c96f0cf52ee74c1) )
ROM_END

ROM_START( b85ritzd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ritz 10p 95715662.bin", 0x6000, 0x002000, CRC(3b7fcc40) SHA1(ba60f91974cd02240822051dcdf1d13cbdaf3455) )
	ROM_LOAD( "ritz 10p 95715798.bin", 0x8000, 0x008000, CRC(597088ac) SHA1(1c1c2a66494c64ba0ced129e044b87538d9d9a38) )
ROM_END

ROM_START( b85jpclb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jkpot2.bin", 0x6000, 0x002000, CRC(1cd88c27) SHA1(126cc0eba69cce6bd3abad9d2eeff64de3ab2477) )
	ROM_LOAD( "jkpot1.bin", 0x8000, 0x008000, CRC(bd63aaa9) SHA1(697d914c1d3a267ef69cfe854e028fdeb8136519) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	// sets below don't boot.. check why
	ROM_LOAD( "5p b.bin", 0x6000, 0x002000, CRC(d39b2517) SHA1(e21b702779aff900f528c2a3002f1df77990e7fe) )
	ROM_LOAD( "5p a.bin", 0x8000, 0x008000, CRC(23b4f619) SHA1(03bdb736b260d96435fe1eab028b4e03c5e733f7) )
	ROM_LOAD( "95715692 jpot 100.bin", 0x6000, 0x002000, CRC(1cd88c27) SHA1(126cc0eba69cce6bd3abad9d2eeff64de3ab2477) )
	ROM_LOAD( "39350115 jpot 100.bin", 0x8000, 0x008000, CRC(dfb05807) SHA1(2375f30e37c7350ccfeb0483e74b51f4c2a090a0) )
ROM_END

ROM_START( b85jpclba )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95715690 20p.bin", 0x6000, 0x002000, CRC(84595df8) SHA1(42315df92c7e98655036e5314638e468a26699d0) )
	ROM_LOAD( "39350112 20p.bin", 0x8000, 0x008000, CRC(59eb8649) SHA1(b42999d34fbf863e92098d08164d4624c576ab06) )
ROM_END

ROM_START( b85jpclbb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95715121 2x10p.bin", 0x6000, 0x002000, CRC(c63280a0) SHA1(db812676d64eb6fbffb80d0af457eb958b5bfc54) )
	ROM_LOAD( "39350141 2x10p.bin", 0x8000, 0x008000, CRC(1088d262) SHA1(02da1bacb90d4e06f1fc97b5bcb9b2b90263d657) )
ROM_END

ROM_START( b85jpclbc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "9715120 2x20p.bin", 0x6000, 0x002000, CRC(5a901360) SHA1(d134151f366a3a64e74fc3d350ec64548cb172b5) )
	ROM_LOAD( "95717832 2x20p.bin", 0x8000, 0x008000, CRC(11622bc8) SHA1(6167238a00e738ea07b46cbf3f3dd5408731d248) )
ROM_END

ROM_START( b85jkwld )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jw345.bin", 0x8000, 0x008000, CRC(d32de80f) SHA1(0afac559ee5a0f0144b3cb449a7b18a8a26a7573) )
ROM_END

ROM_START( b85lucky )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lucky.bin", 0x0000, 0x010000, CRC(00673dc3) SHA1(2d9b839fa0c96dde728d10017dfa809497744453) )
ROM_END

ROM_START( b85luckd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "luckydice.bin", 0x8000, 0x008000, CRC(6cb54fe5) SHA1(ccdf32813ce8a8f1ef65ca65ce2cf9fe654e473e) )
ROM_END


ROM_START( b85sngam )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "supernudgegambler.bin", 0x8000, 0x008000, CRC(c7344abc) SHA1(c1417d9011d4ed94dd25b57bae2fb84a0129fdaf) )
ROM_END

ROM_START( b85cops )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cop215.bin", 0x8000, 0x008000, CRC(5600047c) SHA1(8d6ce9f75c45519838def8686586e55a913a0885) )
ROM_END


ROM_START( b85koc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "king of clubs 39340026.bin", 0x8000, 0x008000, CRC(d3b0746e) SHA1(2847cec108a99747a7e3e31a0f7bcf766cdc1546) )
ROM_END


ROM_START( b85koca )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "king of clubs 39340002.bin", 0x8000, 0x008000, CRC(028708bf) SHA1(9e6942f6a25b260faa4c14c4d61a373be1518f40) )
ROM_END


ROM_START( b85disc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ds2.bin", 0x0000, 0x008000, CRC(fa549c55) SHA1(93a31e4f847dcd326760d17753c994f6210fb6ed) )
	ROM_LOAD( "ds1.bin", 0x8000, 0x008000, CRC(22f6ce92) SHA1(5db8f54bc83e963687ebe2f13769e3f2f678d356) )
ROM_END


ROM_START( b85cb7p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bar7protocol.bin", 0x8000, 0x008000, CRC(e9c022ed) SHA1(e93b4506830a2f098eceb0b419d648bf3a9d02a4) )
	ROM_REGION( 0x20000, "upd", ROMREGION_ERASEFF )
ROM_END


int bfmsys85_state::b85_find_project_string( )
{
	// search for the project string to find the title (usually just at ff00)
	char title_string[7][32] = { "PROJECT NUMBER", "PROJECT PR", "PROJECT ", "CASH ON THE NILE 2", "PR6121", "CHINA TOWN\x0d\x0a", "PROJECTNUMBER" };
	uint8_t *src = memregion( "maincpu" )->base();
	int size = memregion( "maincpu" )->bytes();

	for (auto & elem : title_string)
	{
		int strlength = strlen(elem);

		for (int i=0;i<size-strlength;i++)
		{
			int j;
			int found = 1;
			for (j=0;j<strlength;j+=1)
			{
				uint8_t rom = src[(i+j)];
				uint8_t chr = elem[j];

				if (rom != chr)
				{
					found = 0;
					break;
				}
			}

			if (found!=0)
			{
				int end=0;
				int count = 0;
				int blankcount = 0;
				printf("ID String @ %08x\n", i);

				while (!end)
				{
					uint8_t rom;
					int addr;

					addr = (i+count);

					if (addr<size)
					{
						rom = src[addr];

						if ((rom>=0x20) && (rom<0x7f))
						{
							printf("%c", rom);
							blankcount = 0;
						}
						else
						{
							blankcount++;
							if (blankcount<10) printf(" ");
						}

						count++;
					}
					else
						end = 1;

					if (count>=0x100)
						end = 1;
				}
				printf("\n");

				return 1;
			}
		}
	}

	return 0;
}


void bfmsys85_state::init_decode()
{
	bfm_decode_mainrom(machine(),"maincpu", m_codec_data);
	b85_find_project_string();
}

void bfmsys85_state::init_nodecode()
{
	b85_find_project_string();
}

} // anonymous namespace


#define MACHINE_FLAGS                   MACHINE_NOT_WORKING|MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK
#define MACHINE_FLAGS_MECHANICAL        MACHINE_FLAGS|MACHINE_MECHANICAL

// PROJECT NUMBER 5539  2P CASH EXPLOSION  GAME No 39-350-190 -   29-MAR-1989 11:45:25
GAME( 1989, b85cexpl,   0,          bfmsys85, bfmsys85, bfmsys85_state, init_decode,   0, "BFM",      "Cash Explosion (System 85)", MACHINE_FLAGS )

// PROJECT NUMBER 5150  THE ROYAL 10P PLAY  GAME No 39-350-128 -   21-JAN-1988 12:42:53
GAME( 1988, b85royal,   0,          bfmsys85, bfmsys85, bfmsys85_state, init_decode,   0, "BFM",      "The Royal (System 85)", MACHINE_FLAGS ) // 'The Royal' ?? hack of the Ritz or Big Deal Club?

// PROJECT NUMBER 4957  BIGDEAL 5P PLAY  GAME No 39-350-055 -    9-MAR-1987 11:12:05
GAME( 1987, b85bdclb,   0,          bfmsys85, bfmsys85, bfmsys85_state, init_decode,   0, "BFM",      "Big Deal Club (System 85, set 1)", MACHINE_FLAGS )
// PROJECT NUMBER 5035  BIGDEAL 5P PLAY  GAME No 39-350-045 -   25-FEB-1987 14:19:41
GAME( 1987, b85bdclba,  b85bdclb,   bfmsys85, bfmsys85, bfmsys85_state, init_decode,   0, "BFM",      "Big Deal Club (System 85, set 2)", MACHINE_FLAGS )
// PROJECT NUMBER 5034  BIGDEAL 20P PLAY  GAME No 39-350-047 -   25-FEB-1987 12:44:21
GAME( 1987, b85bdclbb,  b85bdclb,   bfmsys85, bfmsys85, bfmsys85_state, init_decode,   0, "BFM",      "Big Deal Club (System 85, set 3)", MACHINE_FLAGS )


// PROJECT NUMBER 5145  CASH BLITZ  GAME No 39-351-091 -   13-AUG-1987 11:25:29
GAME( 1987, b85cblit,   0,          bfmsys85, bfmsys85, bfmsys85_state, init_decode,   0, "BFM",      "Cash Blitz (System 85, set 1)", MACHINE_FLAGS )
// PROJECT NUMBER 5145  CASH BLITZ  GAME No 39-350-091 -   13-AUG-1987 11:08:54
GAME( 1987, b85cblita,  b85cblit,   bfmsys85, bfmsys85, bfmsys85_state, init_decode,   0, "BFM",      "Cash Blitz (System 85, set 2)", MACHINE_FLAGS )
// PROJECT NUMBER 5145  CASH BLITZ  GAME No 39-350-102 -    3-NOV-1987 16:24:39
GAME( 1987, b85cblitb,  b85cblit,   bfmsys85, bfmsys85, bfmsys85_state, init_decode,   0, "BFM",      "Cash Blitz (System 85, set 3)", MACHINE_FLAGS )

// PROJECT NUMBER 5495  CLUB PREMIER 5P,10P AND 20P PLAY  GAME No 39-350-187 -   28-FEB-1989 15:26:47
GAME( 1989, b85clbpm,   0,          bfmsys85, bfmsys85, bfmsys85_state, init_decode,   0, "BFM",      "Club Premier (System 85)", MACHINE_FLAGS )


// PROJECT NUMBER 5116  HI LO SILVER DX  GAME No 39-350-049 -   27-FEB-1987 10:49:08
GAME( 1987, b85hilo,    0,          bfmsys85, bfmsys85, bfmsys85_state, init_decode,   0, "BFM",      "Hi-Lo Silver (System 85, set 1)", MACHINE_FLAGS )
// PROJECT NUMBER 5407  HI LO SILVER 2P  GAME No 39-350-142 -   12-OCT-1988 09:39:26
GAME( 1988, b85hiloa,   b85hilo,    bfmsys85, bfmsys85, bfmsys85_state, init_decode,   0, "BFM",      "Hi-Lo Silver (System 85, set 2)", MACHINE_FLAGS )


// PROJECT NUMBER 5104  THE RITZ 10P PLAY  GAME No 39-350-084 -   28-AUG-1987 08:44:30
GAME( 1987, b85ritzd,   b85ritz,    bfmsys85, bfmsys85, bfmsys85_state, init_decode,   0, "BFM",      "The Ritz (System 85, set 5)", MACHINE_FLAGS )
// PROJECT NUMBER 5184  THE RITZ 5P PLAY  GAME No 39-350-137 -   25-FEB-1988 11:07:18
GAME( 1988, b85ritz,    0,          bfmsys85, bfmsys85, bfmsys85_state, init_decode,   0, "BFM",      "The Ritz (System 85, set 1)", MACHINE_FLAGS ) // alt version of Big Deal Club?
// PROJECT NUMBER 5183  THE RITZ 20P PLAY  GAME No 39-350-136 -   25-FEB-1988 11:25:52
GAME( 1988, b85ritzb,   b85ritz,    bfmsys85, bfmsys85, bfmsys85_state, init_decode,   0, "BFM",      "The Ritz (System 85, set 3)", MACHINE_FLAGS )
// PROJECT NUMBER 5183  THE RITZ 20P PLAY  GAME No 39-350-138 -   16-MAR-1988 10:46:30
GAME( 1988, b85ritza,   b85ritz,    bfmsys85, bfmsys85, bfmsys85_state, init_decode,   0, "BFM",      "The Ritz (System 85, set 2)", MACHINE_FLAGS )
// PROJECT NUMBER 5104  THE RITZ 10P PLAY  GAME No 39-350-139 -   16-MAR-1988 11:04:27
GAME( 1988, b85ritzc,   b85ritz,    bfmsys85, bfmsys85, bfmsys85_state, init_decode,   0, "BFM",      "The Ritz (System 85, set 4)", MACHINE_FLAGS )

// PROJECT NUMBER 5137  V2 10P PLAY  GAME No 39-350-115 -    9-DEC-1987 12:39:16
GAME( 1987, b85jpclb,   0,          bfmsys85, bfmsys85, bfmsys85_state, init_decode,   0, "BFM",      "Jackpot Club (System 85, set 1)", MACHINE_FLAGS )
// PROJECT NUMBER 5357  V2 20P PLAY  GAME No 39-350-112 -    7-DEC-1987 14:32:31
GAME( 1987, b85jpclba,  b85jpclb,   bfmsys85, bfmsys85, bfmsys85_state, init_decode,   0, "BFM",      "Jackpot Club (System 85, set 2)", MACHINE_FLAGS )
// PROJECT NUMBER 5137  V2 10P PLAY  GAME No 39-350-141 -   16-MAR-1988 11:46:48
GAME( 1988, b85jpclbb,  b85jpclb,   bfmsys85, bfmsys85, bfmsys85_state, init_decode,   0, "BFM",      "Jackpot Club (System 85, set 3)", MACHINE_FLAGS )
// PROJECT NUMBER 5357  V2 20P PLAY  GAME No 39-350-140 -   16-MAR-1988 11:21:43
GAME( 1988, b85jpclbc,  b85jpclb,   bfmsys85, bfmsys85, bfmsys85_state, init_decode,   0, "BFM",      "Jackpot Club (System 85, set 4)", MACHINE_FLAGS )


// PROJECT NUMBER 5368  SUPER NUDGE GAMBLER #4.00  GAME No 39-340-230 -   27-JAN-1988 14:20:43
GAME( 1988, b85sngam,   0,          bfmsys85, bfmsys85, bfmsys85_state, init_decode,   0, "BFM",      "Super Nudge Gambler (System 85)", MACHINE_FLAGS )

// PROJECT NUMBER 4766  10P KING OF CLUBS  GAME No 39-340-026 -   25-NOV-1985 08:49:11
GAME( 199?, b85koc,     0,          bfmsys85, bfmsys85, bfmsys85_state, init_decode,   0, "BFM",      "King of Clubs (Bellfruit) (System 85, set 1)", MACHINE_FLAGS_MECHANICAL) // this has valid strings in it BEFORE the bfm decode, but decodes to valid code, does it use some funky mapping, or did they just fill unused space with valid looking data?
// PROJECT NUMBER 4766  10P KING OF CLUBS  GAME No 39340002 -   16-AUG-1985 15:53:13
GAME( 199?, b85koca,    b85koc,     bfmsys85, bfmsys85, bfmsys85_state, init_decode,   0, "BFM",      "King of Clubs (Bellfruit) (System 85, set 2)", MACHINE_FLAGS_MECHANICAL) // this has valid strings in it BEFORE the bfm decode, but decodes to valid code, does it use some funky mapping, or did they just fill unused space with valid looking data?

// PROJECT NUMBER 5425  BAR SEVEN ARCADE  GAME No 39-341-236 -   11-APR-1988 11:30:33
GAME( 199?, b85cb7p,    0,          bfmsys85, bfmsys85, bfmsys85_state, init_decode,   0, "BFM",      "Bar Sevens (Bellfruit) (Protocol) (System 85)",  MACHINE_FLAGS) // seems to work better here than in sc1

// PROJECT NUMBER 5596  DISCOVERY 85 - 06-APR-1990 08:57:39
GAME( 199?, b85disc,    0,          bfmsys85, bfmsys85, bfmsys85_state, init_decode,   0, "BFM/ELAM", "Discovey (Dutch) (Bellfruit) (System 85)", MACHINE_FLAGS_MECHANICAL ) // GAME No 39-350-251

// PROJECT NUMBER 5452  DUTCH SUPER CARDS  GAME No 39-340-271 - 04-JAN-1989 14:39:00
GAME( 1989, b85scard,   0,          bfmsys85, bfmsys85, bfmsys85_state, init_nodecode, 0, "BFM/ELAM", "Supercards (Dutch, Game Card 39-340-271?) (System 85)", MACHINE_FLAGS )

// PROJECT NUMBER 4840  DUTCH JOKERS WILD PO  GAME No 39-340-345 - 31-JUL-1992 20:01:55
GAME( 1992, b85jkwld,   0,          bfmsys85, bfmsys85, bfmsys85_state, init_nodecode, 0, "BFM/ELAM", "Jokers Wild (Dutch) (System 85)", MACHINE_FLAGS )

// PROJECT NUMBER 4823  LUCKY CARDS 200 PO  GAME No 39-332-217 -    2-DEC-1986 15:57:19
GAME( 1986, b85lucky,   0,          bfmsys85, bfmsys85, bfmsys85_state, init_nodecode, 0, "BFM/ELAM", "Lucky Cards (Dutch) (System 85)", MACHINE_FLAGS )

// PROJECT NUMBER 4902  DUTCH LUCKY DICE PO  GAME No 39-340-346 - 03-AUG-1992 16:30:00
GAME( 1992, b85luckd,   0,          bfmsys85, bfmsys85, bfmsys85_state, init_decode,   0, "BFM/ELAM", "Lucky Dice (Dutch) (System 85)", MACHINE_FLAGS )

// PROJECT NUMBER 4758  DUTCH C+R 200 PO  GAME No 39-332-215 -    2-DEC-1986 15:50:43
GAME( 199?, b85cops,    0,          bfmsys85, bfmsys85, bfmsys85_state, init_nodecode, 0, "BFM/ELAM", "Cops 'n' Robbers (Dutch) (Bellfruit) (System 85)", MACHINE_FLAGS_MECHANICAL)


// this might be system 85 or sc1, the rom config is 0x2000 + 0x8000, and it writes to the AY address we map on S85 for the alarm
// however it still gives the same error message in both, has offset alpha text in s85 and appears to attempt to communicate with something we don't map, maybe it's some video based board / game with bits missing?

// PROJECT NUMBER 5464  V3 10P/20P PLAY  GAME No 39-350-173 -   24-JAN-1989 10:48:53
GAME( 1989, b85dbldl,   0,          bfmsys85, bfmsys85, bfmsys85_state, init_decode,   0, "BFM",      "Double Dealer (System 85, set 1)", MACHINE_FLAGS )
// PROJECT NUMBER 5464  V3 10P/20P PLAY  GAME No 39-350-181 -   02-FEB-1989 15:19:20
GAME( 1985, b85dbldla,  b85dbldl,   bfmsys85, bfmsys85, bfmsys85_state, init_decode,   0, "BFM",      "Double Dealer (System 85, set 2)", MACHINE_FLAGS )
// PROJECT NUMBER 5464  V3 10P/20P PLAY  GAME No 39-350-166 -   17-OCT-1988 14:56:38
GAME( 199?, b85dbldlb,  b85dbldl,   bfmsys85, bfmsys85, bfmsys85_state, init_decode,   0, "BFM",      "Double Dealer (System 85, set 3)", MACHINE_FLAGS ) // found in a sc4 potp set ...

// appears to be the same as above with a different title

// PROJECT NUMBER 5165  V1 10P PLAY  GAME No 39-350-179 -   02-FEB-1989 14:42:57
GAME( 199?, b85potp,    0,          bfmsys85, bfmsys85, bfmsys85_state, init_decode,   0, "BFM",      "Pick Of The Pack (System 85)", MACHINE_FLAGS ) // found in a sc4 potp set ...
