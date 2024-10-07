// license:BSD-3-Clause
// copyright-holders:ANY
/***********************************************************************************

    mgavegas.cpp

    Coin pusher

    TODO:
    - better analog audio out/mixer
    - some output (mostly not used)
    - translate the driver's variables into intelligible English

Ver. 1.33 has no speech and no change funcion implemented in software
Ver. 2.1 and 2.3 have change function working and speech
Ver. 2.2 should exist

************************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/timer.h"
#include "sound/ay8910.h"
#include "sound/flt_rc.h"
#include "sound/msm5205.h"
#include "speaker.h"

#include "mgavegas.lh"

#define LOG_AY8910  (1U << 1)
#define LOG_MSM5205 (1U << 2)
#define LOG_CSO1    (1U << 3)
#define LOG_CSO2    (1U << 4)

#define VERBOSE (0)
#include "logmacro.h"


namespace {

/****************************
*    Clock defines          *
****************************/
#define MAIN_XTAL XTAL(8'000'000)
#define CPU_CLK MAIN_XTAL/2
#define AY_CLK  CPU_CLK/2
#define MSM_CLK   384000


class mgavegas_state : public driver_device
{
public:
	mgavegas_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ay(*this, "aysnd"),
		m_msm(*this, "5205"),
		m_ticket(*this, "hopper"),
		m_filter1(*this, "filter1"),
		m_filter2(*this, "filter2"),
		m_mga4(*this, "MGA4"),
		m_mga3(*this, "MGA3"),
		m_mga2(*this, "MGA2"),
		m_mga(*this, "MGA"),
		m_pl(*this, "PL"),
		m_pc(*this, "PC"),
		m_pr(*this, "PR"),
		m_luz_250_rul(*this, "250"),
		m_luz_100_rul(*this, "1002"),
		m_luz_50_rlul(*this, "50"),
		m_luz_25_lrul(*this, "252"),
		m_luz_25_rrul(*this, "25"),
		m_fl(*this, "FL"),
		m_fc(*this, "FC"),
		m_fr(*this, "FR"),
		m_insert_coin(*this, "INSERTCOIN"),
		m_no_cambio(*this, "NOCAMBIO"),
		m_fuse(*this, "FUSE"),
		m_falta(*this, "FALTA"),
		m_cl(*this, "CL"),
		m_cc(*this, "CC"),
		m_cr(*this, "CR"),
		m_premio_s(*this, "PREMIOS"),
		m_100(*this, "100"),
		m_200(*this, "200"),
		m_300(*this, "300"),
		m_500(*this, "500"),
		m_ml(*this, "ML"),
		m_mc(*this, "MC"),
		m_mr(*this, "MR")
	{ }

	void mgavegas(machine_config &config);

	void init_mgavegas();
	void init_mgavegas21();
	void init_mgavegas133();

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void mgavegas_map(address_map &map) ATTR_COLD;

	uint8_t start_read();

	void w_a0(offs_t offset, uint8_t data);
	uint8_t r_a0(offs_t offset);
	void cso1_w(uint8_t data);
	void cso2_w(uint8_t data);
	void csoki_w(offs_t offset, uint8_t data);
	uint8_t csoki_r(offs_t offset);

	uint8_t ay8910_a_r();
	uint8_t ay8910_b_r();

	TIMER_DEVICE_CALLBACK_MEMBER(int_0);

	void update_custom();

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<ay8910_device> m_ay;
	required_device<msm5205_device> m_msm;
	required_device<ticket_dispenser_device> m_ticket;
	required_device<filter_rc_device> m_filter1;
	required_device<filter_rc_device> m_filter2;

	// lamp outputs
	uint64_t m_custom_data = 0ULL;
	output_finder<> m_mga4;
	output_finder<> m_mga3;
	output_finder<> m_mga2;
	output_finder<> m_mga;
	output_finder<> m_pl;
	output_finder<> m_pc;
	output_finder<> m_pr;
	output_finder<> m_luz_250_rul;
	output_finder<> m_luz_100_rul;
	output_finder<> m_luz_50_rlul;
	output_finder<> m_luz_25_lrul;
	output_finder<> m_luz_25_rrul;
	output_finder<> m_fl;
	output_finder<> m_fc;
	output_finder<> m_fr;
	output_finder<> m_insert_coin;
	output_finder<> m_no_cambio;
	output_finder<> m_fuse;
	output_finder<> m_falta;
	output_finder<> m_cl;
	output_finder<> m_cc;
	output_finder<> m_cr;
	output_finder<> m_premio_s;
	output_finder<> m_100;
	output_finder<> m_200;
	output_finder<> m_300;
	output_finder<> m_500;
	output_finder<> m_ml;
	output_finder<> m_mc;
	output_finder<> m_mr;

	uint8_t m_int = 0;

	// output group 1
	uint8_t m_ckmod = 0;
	uint8_t m_dmod = 0;
	uint8_t m_emod = 0;
	uint8_t m_inh = 0;
	uint8_t m_hop = 0;
	uint8_t m_seg = 0;

	// helpers
	uint8_t m_old_ckmod = 0;
	uint8_t m_old_emod = 0;

	// output group 2
	uint8_t m_bobina_ctrl = 0;
	uint8_t m_timbre = 0;
	uint8_t m_coil_1 = 0;
	uint8_t m_coil_2 = 0;
	uint8_t m_coil_3 = 0;
	uint8_t m_cont_ent = 0;
	uint8_t m_cont_sal = 0;
	uint8_t m_cont_caj = 0;
};


/*************************
* Memory Map Information *
*************************/

void mgavegas_state::mgavegas_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).ram().share("nvram");
	map(0xa000, 0xa003).rw(FUNC(mgavegas_state::r_a0), FUNC(mgavegas_state::w_a0));            // AY-3-8910
	map(0xc000, 0xc001).w(FUNC(mgavegas_state::cso1_w));                   // /CSout1
	map(0xc400, 0xc401).w(FUNC(mgavegas_state::cso2_w));                   // /CSout2
	map(0xc800, 0xc801).rw(FUNC(mgavegas_state::csoki_r), FUNC(mgavegas_state::csoki_w));      // /CSoki
	//map(0xcc00, 0xcc01).rw(FUNC(mgavegas_state::cso3_r), FUNC(mgavegas_state::cso3_w));      // /CSout3, unused
	//map(0xe000, 0xe003).rw(FUNC(mgavegas_state::r_e0), FUNC(mgavegas_state::w_e0));          // /CSaux, unused
}


/*************************
* Machine Initialization *
*************************/

void mgavegas_state::machine_start()
{
	m_mga4.resolve();
	m_mga3.resolve();
	m_mga2.resolve();
	m_mga.resolve();
	m_pl.resolve();
	m_pc.resolve();
	m_pr.resolve();
	m_luz_250_rul.resolve();
	m_luz_100_rul.resolve();
	m_luz_50_rlul.resolve();
	m_luz_25_lrul.resolve();
	m_luz_25_rrul.resolve();
	m_fl.resolve();
	m_fc.resolve();
	m_fr.resolve();
	m_insert_coin.resolve();
	m_no_cambio.resolve();
	m_fuse.resolve();
	m_falta.resolve();
	m_cl.resolve();
	m_cc.resolve();
	m_cr.resolve();
	m_premio_s.resolve();
	m_100.resolve();
	m_200.resolve();
	m_300.resolve();
	m_500.resolve();
	m_ml.resolve();
	m_mc.resolve();
	m_mr.resolve();
}

void mgavegas_state::machine_reset()
{
	m_int = 1;
	m_custom_data = 0xffffffffffffffffULL;

	m_old_ckmod = 1;
	m_old_emod = 0;

	m_ckmod = 0;
	m_dmod = 0;
	m_emod = 0;
	m_inh = 0;
	m_hop = 0;
	m_seg = 0;

	m_bobina_ctrl = 0;
	m_timbre = 0;
	m_coil_1 = 0;
	m_coil_2 = 0;
	m_coil_3 = 0;
	m_cont_ent = 0;
	m_cont_sal = 0;
	m_cont_caj = 0;

	m_filter1->filter_rc_set_RC(filter_rc_device::LOWPASS, 1000, 0, 0, CAP_N(1));     // RC out of MSM5205 R=1K C=1nF
	m_filter2->filter_rc_set_RC(filter_rc_device::HIGHPASS, 3846, 0, 0, CAP_N(100));  // ALP3B active-hybrid filter fc=2.6Khz - 2-pole?
}


/*************************
*      Lamp Updates      *
*************************/

void mgavegas_state::update_custom()
{
	if (m_ckmod == 1 && m_old_ckmod == 0)
	{
		// valid clock, sample the data
		m_custom_data = (m_custom_data << 1) | (m_dmod & 0x01);
	}

	if (m_emod == 0 && m_old_emod == 1)
	{
		// valid emod, check for valid data and updatae custom output lamps - this is how the hardware works
		if (BIT(m_custom_data, 32) == 0 && BIT(m_custom_data, 33) == 0 && BIT(m_custom_data, 34) == 0 && BIT(m_custom_data, 35) == 0)
		{
			uint64_t tmp = ~m_custom_data;
			m_luz_50_rlul = BIT(tmp, 1);
			m_luz_25_lrul = BIT(tmp, 2);
			m_mga2 =        BIT(tmp, 3);
			m_mga =         BIT(tmp, 4);
			m_pl =          BIT(tmp, 5);
			m_pc =          BIT(tmp, 6);
			m_pr =          BIT(tmp, 7);
			m_luz_250_rul = BIT(tmp, 8);
			m_luz_100_rul = BIT(tmp, 9);
			m_mga3 =        BIT(tmp, 10);
			m_mga4 =        BIT(tmp, 11);
			m_luz_25_rrul = BIT(tmp, 12);
			m_fl =          BIT(tmp, 13);
			m_fc =          BIT(tmp, 14);
			m_fr =          BIT(tmp, 15);
			m_insert_coin = BIT(tmp, 16);
			m_no_cambio =   BIT(tmp, 17);
			m_fuse =        BIT(tmp, 18);
			m_falta =       BIT(tmp, 19);
			m_cl =          BIT(tmp, 21);
			m_cc =          BIT(tmp, 22);
			m_cr =          BIT(tmp, 23);
			m_premio_s =    BIT(tmp, 24);
			m_100 =         BIT(tmp, 25);
			m_200 =         BIT(tmp, 26);
			m_300 =         BIT(tmp, 27);
			m_500 =         BIT(tmp, 28);
			m_ml =          BIT(tmp, 29);
			m_mc =          BIT(tmp, 30);
			m_mr =          BIT(tmp, 31);
		}
	}

	m_old_ckmod = m_ckmod;
	m_old_emod = m_emod;
}


uint8_t mgavegas_state::start_read()
{
	// Hardware looks for falling /IOREQ to clear the IRQ line
	if (m_int)
	{
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
		m_int = 0;
	}
	return 0xed;
}


/****************************
*    Read/Write Handlers    *
****************************/

uint8_t mgavegas_state::r_a0(offs_t offset)
{
	uint8_t ret = 0;
	switch (offset&0x03)
	{
		case 1: // BDIR = 0, BC1 = 1
			ret = m_ay->data_r();
			break;
		default:
			LOGMASKED(LOG_AY8910, "Unknown AY-3-8910 read\n");
			break;
	}

	LOGMASKED(LOG_AY8910, "read from %04X return %02X\n", offset + 0xa000, ret);
	return ret;
}

void mgavegas_state::w_a0(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_AY8910, "write to %04X data = %02X \n", offset + 0xa000, data);

	switch (offset&0x03)
	{
		case 0: // BDIR = 1, BC1 = 1
			m_ay->address_w(data);
			break;
		case 2: // BDIR = 1, BC1 = 0
			m_ay->data_w(data);
			break;
/*
        case 1: // BDIR = 0, BC1 = 1
            break;
        case 3: // BDIR = 0, BC1 = 0
            break;
*/
		default:
			LOGMASKED(LOG_AY8910, "Unknown AY-3-8910 write\n");
			break;
	}
}


uint8_t mgavegas_state::csoki_r(offs_t offset)
{
	uint8_t ret = 0;
	LOGMASKED(LOG_MSM5205, "read from %04X return %02X\n", offset + 0xc800, ret);
	return ret;
}

void mgavegas_state::csoki_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_MSM5205, "MSM5205 write to %04X data = %02X \n", offset + 0xc800, data);
	m_msm->reset_w(BIT(data, 4));
	m_msm->data_w(data & 0x0f);
}


void mgavegas_state::cso1_w(uint8_t data)
{
	LOGMASKED(LOG_CSO1, "write to CSO1 data = %02X\n", data);

	m_ckmod = BIT(data, 0);
	m_dmod = BIT(data, 1);
	m_emod = BIT(data, 2);
	m_inh = BIT(data, 3);
	m_hop = BIT(data, 4);
	m_seg = BIT(data, 5);

	update_custom();

	m_ticket->motor_w(m_hop);
}


void mgavegas_state::cso2_w(uint8_t data)
{
	LOGMASKED(LOG_CSO2, "write to CSO2 data = %02X\n", data);

	m_bobina_ctrl = BIT(data, 0);
	m_timbre = BIT(data, 1);
	m_coil_1 = BIT(data, 2);
	m_coil_2 = BIT(data, 3);
	m_coil_3 = BIT(data, 4);
	m_cont_ent = BIT(data, 5);
	m_cont_sal = BIT(data, 6);
	m_cont_caj = BIT(data, 7);
}


uint8_t mgavegas_state::ay8910_a_r()
{
	uint8_t ret = ioport("INA")->read();
	LOGMASKED(LOG_AY8910, "read from port A return %02X\n",ret);
	return ret;
}

uint8_t mgavegas_state::ay8910_b_r()
{
	uint8_t ret = ioport("DSW1")->read();
	LOGMASKED(LOG_AY8910, "read from AY-3-910 port B: %02X\n",ret);
	return ret;
}


/*************************
*      Input Ports       *
*************************/

static INPUT_PORTS_START( mgavegas )

	PORT_START("INA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) // 200ptas in for change with 8 25 ptas coins
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) // 25 ptas in to play
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) // 100ptas in for change with 4 25 ptas coins
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Y) PORT_NAME("25 ptas level")     //"hack" hopper always full
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U) PORT_NAME("Door")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I) PORT_NAME("Channel")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x02, "Percentage" )
	PORT_DIPSETTING(    0x00, "70%" )
	PORT_DIPSETTING(    0x01, "70%" )
	PORT_DIPSETTING(    0x02, "72%" )
	PORT_DIPSETTING(    0x03, "74%" )
	PORT_DIPSETTING(    0x04, "76%" )
	PORT_DIPSETTING(    0x05, "78%" )
	PORT_DIPSETTING(    0x06, "80%" )
	PORT_DIPSETTING(    0x07, "82%" )

	PORT_DIPNAME( 0x08, 0x08, "Sound" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On )  )

	PORT_DIPNAME( 0x10, 0x10, "Speech" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On )  )

	PORT_DIPNAME( 0x60, 0x00, "Jackpot" )
	PORT_DIPSETTING(    0x40, "Jackpot 1%" )
	PORT_DIPSETTING(    0x60, "Jackpot 2,5%" )
	PORT_DIPSETTING(    0x00, "Jackpot 5%" )
	//PORT_DIPSETTING(    0x20, DEF_STR( On ) ) // not listed

	PORT_DIPNAME( 0x80, 0x80, "Reset" )
	PORT_DIPSETTING(    0x80, "Normal Gameplay" )
	PORT_DIPSETTING(    0x00, "Reset" )

INPUT_PORTS_END


/*************************
*    Machine Driver      *
*************************/

void mgavegas_state::mgavegas(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, CPU_CLK);
	m_maincpu->set_addrmap(AS_PROGRAM, &mgavegas_state::mgavegas_map);

	TIMER(config, "int_0").configure_periodic(FUNC(mgavegas_state::int_0), attotime::from_hz(6000));    //6KHz from MSM5205 /VCK

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	TICKET_DISPENSER(config, "hopper", attotime::from_msec(200));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	AY8910(config, m_ay, AY_CLK);
	m_ay->add_route(ALL_OUTPUTS, "mono", 0.3);
	m_ay->port_a_read_callback().set(FUNC(mgavegas_state::ay8910_a_r));
	m_ay->port_b_read_callback().set(FUNC(mgavegas_state::ay8910_b_r));

	MSM5205(config, m_msm, MSM_CLK);
	m_msm->set_prescaler_selector(msm5205_device::S64_4B);
	m_msm->add_route(ALL_OUTPUTS, "filter1", 2.0);

	FILTER_RC(config, "filter1").add_route(ALL_OUTPUTS, "filter2", 2.0);
	FILTER_RC(config, "filter2").add_route(ALL_OUTPUTS, "mono", 2.0);

	// video
	config.set_default_layout(layout_mgavegas);
}


/*******************
*   Driver Init    *
*******************/

void mgavegas_state::init_mgavegas21()
{
	// hack to clear the IRQ on RETI instruction
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00ea, 0x00ea, read8smo_delegate(*this, FUNC(mgavegas_state::start_read)));
}

void mgavegas_state::init_mgavegas()
{
	// hack to clear the IRQ on RETI instruction
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00e2, 0x00e2, read8smo_delegate(*this, FUNC(mgavegas_state::start_read)));
}


TIMER_DEVICE_CALLBACK_MEMBER(mgavegas_state::int_0)
{
	if (m_int == 0)
	{
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, HOLD_LINE);
	}
}

void mgavegas_state::init_mgavegas133()
{
	// hack to clear the IRQ on RETI instruction
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00dd, 0x00dd, read8smo_delegate(*this, FUNC(mgavegas_state::start_read)));
}


/**********************
*      ROM Loads      *
**********************/


ROM_START(mgavegas)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("vegas1-k2.3.bin", 0x0000, 0x8000, CRC(418b1d04) SHA1(27669a85ed52d5dab25d6ebea6ef3d9b01a4795d) )

	ROM_REGION( 0x2000, "nvram", 0 )
	ROM_LOAD( "mgavegas23.nv", 0x0000, 0x2000, CRC(d0a175b0) SHA1(7698135dbc020f459fdaa660bf488595b67b77d0) )  //default setting
ROM_END

ROM_START(mgavegas21)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("vegas1-2.1.bin", 0x0000, 0x8000, CRC(a7e988a4) SHA1(32fa1684c79f4a132553fa41006f243d4b51cef6) )

	ROM_REGION( 0x2000, "nvram", 0 )
	ROM_LOAD( "mgavegas21.nv", 0x0000, 0x2000, CRC(a4471550) SHA1(b8527e9158b5563460febd1009b44c8d74dbae4e) )  //default setting
ROM_END

ROM_START(mgavegas133)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("vegas1-1.33.bin", 0x0000, 0x8000, CRC(1eea7f0f) SHA1(6fb54b8e2ab19e5378a95192e5007175ad76bc7a) )

	ROM_REGION( 0x2000, "nvram", 0 )
	ROM_LOAD( "mgavegas133.nv", 0x0000, 0x2000, CRC(20fe4db7) SHA1(887b69468ac7e6490827a06cd1f0ff15228a9c73) )  //default setting
ROM_END

} // anonymous namespace


/*************************
*      Game Drivers      *
*************************/
//    YEAR  NAME            PARENT      MACHINE   INPUT     STATE           INIT        ROT     COMPANY   FULLNAME                                      FLAGS
GAME( 1985, mgavegas,       0,          mgavegas, mgavegas, mgavegas_state, init_mgavegas,   ROT0,   "MGA",    "Vegas 1 (Ver 2.3 dual coin pulse, shorter)", MACHINE_MECHANICAL )
GAME( 1985, mgavegas21,     mgavegas,   mgavegas, mgavegas, mgavegas_state, init_mgavegas21, ROT0,   "MGA",    "Vegas 1 (Ver 2.1 dual coin pulse, longer)",  MACHINE_MECHANICAL )
GAME( 1985, mgavegas133,    mgavegas,   mgavegas, mgavegas, mgavegas_state, init_mgavegas133,ROT0,   "MGA",    "Vegas 1 (Ver 1.33 single coin pulse)",       MACHINE_MECHANICAL )
