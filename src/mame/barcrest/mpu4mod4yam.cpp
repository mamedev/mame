// license:BSD-3-Clause
// copyright-holders:David Haywood
/* This is MPU4 MOD4 with a Y2413 instead of an OKI */

#include "emu.h"
#include "mpu4.h"

#include "sound/ymopl.h"

namespace {

class mpu4mod4yam_machines_state : public mpu4_state
{
public:
	mpu4mod4yam_machines_state(const machine_config &mconfig, device_type type, const char *tag)
		: mpu4_state(mconfig, type, tag)
		, m_ym2413(*this, "ym2413")
	{
	}

	void base_f(machine_config &config);
	void no_bacta_f(machine_config &config);
	template<const uint8_t* Table> void cheatchr_pal_f(machine_config &config);
	template<uint8_t Fixed> void bootleg_fixedret_f(machine_config &config);
	void cheatchr_gambal_f(machine_config &config);

	template<typename... T>
	auto base(T... traits)
	{
		return trait_wrapper(this, &mpu4mod4yam_machines_state::base_f, traits...);
	}
	template<typename... T>
	auto no_bacta(T... traits)
	{
		return trait_wrapper(this, &mpu4mod4yam_machines_state::no_bacta_f, traits...);
	}
	template<const uint8_t* Table, typename... T>
	auto cheatchr_pal(T... traits)
	{
		return trait_wrapper(this, &mpu4mod4yam_machines_state::cheatchr_pal_f<Table>, traits...);
	}
	template<uint8_t Fixed, typename... T>
	auto bootleg_fixedret(T... traits)
	{
		return trait_wrapper(this, &mpu4mod4yam_machines_state::bootleg_fixedret_f<Fixed>, traits...);
	}
	template<typename... T>
	auto cheatchr_gambal(T... traits)
	{
		return trait_wrapper(this, &mpu4mod4yam_machines_state::cheatchr_gambal_f, traits...);
	}

	void init_m4_806prot();

private:
	void add_ym2413(machine_config& config);

	void memmap_ym2413(address_map &map) ATTR_COLD;
	void memmap_characteriser_ym2413(address_map &map) ATTR_COLD;
	void memmap_bootleg_characteriser_ym2413(address_map &map) ATTR_COLD;

	DECLARE_MACHINE_START(mpu4yam);

	void pia_ic5_porta_gambal_w(uint8_t data);

	required_device<ym2413_device> m_ym2413;
};

#include "gamball.lh"
#include "m4addr.lh"



/***********************************************************************************************

  Configs for Mod4 with YM

***********************************************************************************************/

void mpu4mod4yam_machines_state::add_ym2413(machine_config &config)
{
	YM2413(config, m_ym2413, XTAL(3'579'545)); // XTAL on sound board
	m_ym2413->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void mpu4mod4yam_machines_state::base_f(machine_config &config)
{
	mpu4base(config);
	MCFG_MACHINE_START_OVERRIDE(mpu4mod4yam_machines_state,mpu4yam)

	m_maincpu->set_addrmap(AS_PROGRAM, &mpu4mod4yam_machines_state::memmap_ym2413);

	add_ym2413(config);
}

void mpu4mod4yam_machines_state::no_bacta_f(machine_config &config)
{
	base_f(config);
	config.device_remove("dataport");
	m_pia5->ca2_handler().set(m_pia4, FUNC(pia6821_device::cb1_w));
}

template<const uint8_t* Table> void mpu4mod4yam_machines_state::cheatchr_pal_f(machine_config &config)
{
	base_f(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &mpu4mod4yam_machines_state::memmap_characteriser_ym2413);

	MPU4_CHARACTERISER_PAL(config, m_characteriser, 0);
	m_characteriser->set_cpu_tag("maincpu");
	m_characteriser->set_allow_6809_cheat(true);
	m_characteriser->set_lamp_table(Table);
}

template<uint8_t Fixed> void mpu4mod4yam_machines_state::bootleg_fixedret_f(machine_config &config)
{
	base_f(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &mpu4mod4yam_machines_state::memmap_bootleg_characteriser_ym2413);

	MPU4_CHARACTERISER_BL(config, m_characteriser_bl, 0);
	m_characteriser_bl->set_bl_fixed_return(Fixed);
}

void mpu4mod4yam_machines_state::cheatchr_gambal_f(machine_config &config)
{
	cheatchr_pal_f<mpu4_characteriser_pal::gambal_characteriser_prot>(config);

	// custom hookup for gambal feature
	m_pia5->writepa_handler().set(FUNC(mpu4mod4yam_machines_state::pia_ic5_porta_gambal_w));
}




MACHINE_START_MEMBER(mpu4mod4yam_machines_state,mpu4yam)
{
	mpu4_config_common();

	m_link7a_connected=false;
	m_link7b_connected=true;
}

void mpu4mod4yam_machines_state::memmap_ym2413(address_map &map)
{
	mpu4_memmap(map);
	map(0x0880, 0x0881).w(m_ym2413, FUNC(ym2413_device::write));
}

void mpu4mod4yam_machines_state::memmap_characteriser_ym2413(address_map &map)
{
	mpu4_memmap_characteriser(map);
	map(0x0880, 0x0881).w(m_ym2413, FUNC(ym2413_device::write));
}

void mpu4mod4yam_machines_state::memmap_bootleg_characteriser_ym2413(address_map &map)
{
	mpu4_memmap_bootleg_characteriser(map);
	map(0x0880, 0x0881).w(m_ym2413, FUNC(ym2413_device::write));
}


void mpu4mod4yam_machines_state::pia_ic5_porta_gambal_w(uint8_t data)
{
	pia_ic5_porta_w(data);

	/* The 'Gamball' device is a unique piece of mechanical equipment, designed to
	provide a truly fair hi-lo gamble for an AWP. Functionally, it consists of
	a ping-pong ball or similar enclosed in the machine's backbox, on a platform with 12
	holes. When the low 4 bytes of AUX1 are triggered, this fires the ball out from the
	hole it's currently in, to land in another. Landing in the same hole causes the machine to
	refire the ball. The ball detection is done by the high 4 bytes of AUX1.
	Here we call the MAME RNG, once to pick a row, once to pick from the four pockets within it. We
	then trigger the switches corresponding to the correct number. This appears to be the best way
	of making the game fair, short of simulating the physics of a bouncing ball ;)*/
	if (data & 0x0f)
	{
		switch ((machine().rand()>>5) % 0x3)
		{
		case 0x00: //Top row
			switch (machine().rand() & 0x3)
			{
			case 0x00: //7
				m_aux1_input = (m_aux1_input & 0x0f);
				m_aux1_input|= 0xa0;
				break;

			case 0x01://4
				m_aux1_input = (m_aux1_input & 0x0f);
				m_aux1_input|= 0xb0;
				break;

			case 0x02://9
				m_aux1_input = (m_aux1_input & 0x0f);
				m_aux1_input|= 0xc0;
				break;

			case 0x03://8
				m_aux1_input = (m_aux1_input & 0x0f);
				m_aux1_input|= 0xd0;
				break;
			}
			break;

		case 0x01: //Middle row - note switches don't match pattern
			switch (machine().rand() & 0x3)
			{
			case 0x00://12
				m_aux1_input = (m_aux1_input & 0x0f);
				m_aux1_input|= 0x40;
				break;

			case 0x01://1
				m_aux1_input = (m_aux1_input & 0x0f);
				m_aux1_input|= 0x50;
				break;

			case 0x02://11
				m_aux1_input = (m_aux1_input & 0x0f);
				m_aux1_input|= 0x80;
				break;

			case 0x03://2
				m_aux1_input = (m_aux1_input & 0x0f);
				m_aux1_input|= 0x90;
				break;
			}
			break;

		case 0x02: //Bottom row
			switch (machine().rand() & 0x3)
			{
			case 0x00://5
				m_aux1_input = (m_aux1_input & 0x0f);
				m_aux1_input|= 0x00;
				break;

			case 0x01://10
				m_aux1_input = (m_aux1_input & 0x0f);
				m_aux1_input|= 0x10;
				break;

			case 0x02://3
				m_aux1_input = (m_aux1_input & 0x0f);
				m_aux1_input|= 0x20;
				break;

			case 0x03://6
				m_aux1_input = (m_aux1_input & 0x0f);
				m_aux1_input|= 0x30;
				break;
			}
			break;
		}
	}
}

void mpu4mod4yam_machines_state::init_m4_806prot()
{
	init_m4();
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x0806, 0x0806, read8smo_delegate(*this, []() -> uint8_t { return 0x6a; }, "806"));
}


/*****************************************************************************************************************************************************************************
*
* Classic Adders and Ladders
*
*****************************************************************************************************************************************************************************/


//PCKEY =0
//STKEY =0
//JPKEY =0
//JPSET =0
//DIP1_0=false
//DIP1_1=false
//DIP1_2=false
//DIP1_3=false
//DIP1_4=false
//DIP1_5=false
//DIP1_6=false
//DIP1_7=false
//DIP2_0=false
//DIP2_1=false
//DIP2_2=false
//DIP2_3=false
//DIP2_4=false
//DIP2_5=false
//DIP2_6=false
//DIP2_7=false
//Sound barcrest1
//Standard
//Volume 0 Stereo= 1
//Sample rate 16000
//Front door code 0 Cash door code 0

} // anonymous namespace

#define GAME_FLAGS (MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL)


INPUT_PORTS_START( m4addr )
	PORT_INCLUDE(mpu4)

	PORT_MODIFY("BLACK1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("BLACK2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Collect / Take Feature")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Hold 1/Nudge 1")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Hold 2/Nudge 2")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Hold 3/Nudge 3")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Hi")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Lo")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME("Exchange Feature")

	PORT_MODIFY("DIL2")
	PORT_DIPNAME( 0x08, 0x00, "Show Attract Sequence" ) PORT_DIPLOCATION("DIL2:04")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On  ) )
INPUT_PORTS_END

using namespace mpu4_traits;

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent, machine, inputs, init, name,offset,length,hash,company,title, flags) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAMEL( year, setname, parent, machine, inputs, mpu4mod4yam_machines_state, init, ROT0, company, title, flags, layout_m4addr )

// "(C)1991 BARCREST"  and "A6L 0.1"
GAME_CUSTOM( 1991, m4addr,       0,      cheatchr_pal<mpu4_characteriser_pal::addr_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "a6ls.p1",                  0x0000, 0x010000, CRC(9f97f57b) SHA1(402d1518bb78fdc489b06c2aabc771e5ce151847), "Barcrest","Classic Adders & Ladders (Barcrest) (MPU4) (A6L 0.1, set 1)", 0 )
GAME_CUSTOM( 199?, m4addrc__d,   m4addr, cheatchr_pal<mpu4_characteriser_pal::addr_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "alddr20",                  0x0000, 0x010000, CRC(19cf4437) SHA1(b528823c476bebd1a9a6c720a4144294743693d2), "Barcrest","Classic Adders & Ladders (Barcrest) (MPU4) (A6L 0.1, set 2)", 0 ) // hack?
GAME_CUSTOM( 1991, m4addr6ld,    m4addr, cheatchr_pal<mpu4_characteriser_pal::addr_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "a6ld.p1",                  0x0000, 0x010000, CRC(de555e12) SHA1(2233160f1c734c889c1c00dee202a928f18ad763), "Barcrest","Classic Adders & Ladders (Barcrest) (MPU4) (A6L 0.1 D)", 0 )
GAME_CUSTOM( 1991, m4addr6lc,    m4addr, cheatchr_pal<mpu4_characteriser_pal::addr_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "a6lc.p1",                  0x0000, 0x010000, CRC(1e75fe67) SHA1(4497b19d4c512c934d445b4acf607dc2dc080d44), "Barcrest","Classic Adders & Ladders (Barcrest) (MPU4) (A6L 0.1 C)", 0 )
GAME_CUSTOM( 1991, m4addr6lk,    m4addr, cheatchr_pal<mpu4_characteriser_pal::addr_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "a6lk.p1",                  0x0000, 0x010000, CRC(af5ae5c4) SHA1(20e40cf996c2c3b7b18ec104a374be1da193b94e), "Barcrest","Classic Adders & Ladders (Barcrest) (MPU4) (A6L 0.1 K)", 0 )
GAME_CUSTOM( 1991, m4addr6ly,    m4addr, cheatchr_pal<mpu4_characteriser_pal::addr_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "adders ladders 20p 6.bin", 0x0000, 0x010000, CRC(62abeb34) SHA1(8069e6fde0673fdbc124a1a172dc988bb3205ff6), "Barcrest","Classic Adders & Ladders (Barcrest) (MPU4) (A6L 0.1 Y)", 0 )
GAME_CUSTOM( 1991, m4addr6lyd,   m4addr, cheatchr_pal<mpu4_characteriser_pal::addr_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "a6ldy.p1",                 0x0000, 0x010000, CRC(82f060a5) SHA1(2e8474e6c17def07e35448b5bf8d453cce0f292c), "Barcrest","Classic Adders & Ladders (Barcrest) (MPU4) (A6L 0.1 YD)", 0 )
GAME_CUSTOM( 1991, m4addr6lybd,  m4addr, cheatchr_pal<mpu4_characteriser_pal::addr_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "a6lbdy.p1",                0x0000, 0x010000, CRC(28064099) SHA1(c916f73911974440d4c79ecb51b343aad78f115b), "Barcrest","Classic Adders & Ladders (Barcrest) (MPU4) (A6L 0.1 YBD)", 0 )
// "(C)1993  B.W.B." and "ADD 1.0"
GAME_CUSTOM( 199?, m4addrc__l,   m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "al10",                     0x0000, 0x010000, CRC(3c3c82b6) SHA1(cc5ffdd0837c9af31d5737a70430a01d1989cdcc), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 1.0, 1993)", GAME_FLAGS )
// "(C)1994  B.W.B." and "ADD 1.0" (actually version 10?)
GAME_CUSTOM( 1994, m4addr10,     m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "ad_05___.1o3",             0x0000, 0x010000, CRC(8d9e0f5d) SHA1(fecc844908876e161d0134ce3cc098e79e74e0b1), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 1.0, set 1)", GAME_FLAGS )
GAME_CUSTOM( 1994, m4addr10d,    m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "ad_05_d_.1o3",             0x0000, 0x010000, CRC(2d29040f) SHA1(ee2bdd5da1a7e4146419ffd8bad521a9c1b49aa2), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 1.0 D, set 1)", GAME_FLAGS )
GAME_CUSTOM( 1994, m4addr10c,    m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "adi05___.1o3",             0x0000, 0x010000, CRC(050764b1) SHA1(364c50e4887c9fdd7ff62e63a6be4513336b4814), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 1.0 C, set 1)", GAME_FLAGS )
GAME_CUSTOM( 1994, m4addr10yd,   m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "ad_05_b_.1o3",             0x0000, 0x010000, CRC(b10b194a) SHA1(4dc3f14ff3b903c49829f4a91136f9b03a5cb1ae), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 1.0 YD, set 1)", GAME_FLAGS )
GAME_CUSTOM( 1994, m4addr10_a,   m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "ad_10___.1o3",             0x0000, 0x010000, CRC(d587cb00) SHA1(6574c42402f13e5f9cb8f951e0f59b499b2d025d), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 1.0, set 2)", GAME_FLAGS )
GAME_CUSTOM( 1994, m4addr10d_a,  m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "ad_10_d_.1o3",             0x0000, 0x010000, CRC(d7670d32) SHA1(09dfe2a7fe267f485efed234411efc92d9cce414), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 1.0 D, set 2)", GAME_FLAGS )
GAME_CUSTOM( 1994, m4addr10c_a,  m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "adi10___.1o3",             0x0000, 0x010000, CRC(005caaa1) SHA1(b4b421c045012b5fbeaca95fa09d087a9c5e6b5b), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 1.0 C, set 2)", GAME_FLAGS )
GAME_CUSTOM( 1994, m4addr10yd_a, m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "ad_10_b_.1o3",             0x0000, 0x010000, CRC(e2b5c0db) SHA1(9e1716186bb049c61dddaef2465fb1e55d2d93fd), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 1.0 YD, set 2)", GAME_FLAGS )
// "(C)1993  B.W.B."  and "ADD 3.0"
GAME_CUSTOM( 1993, m4addr3,      m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "ad_05___.3q3",             0x0000, 0x010000, CRC(ec6ed7ce) SHA1(dfad04b5f6c4ff0fd784ad20471f1cf84586f2cd), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 3.0, set 1)", GAME_FLAGS )
GAME_CUSTOM( 1993, m4addr3d,     m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "ad_05_d_.3q3",             0x0000, 0x010000, CRC(8d05fba9) SHA1(9c69d7eec7ce0d647d4f8b8b0a6b7e54daa7a79f), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 3.0 D, set 1)", GAME_FLAGS )
GAME_CUSTOM( 1993, m4addr3yd,    m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "ad_05_b_.3q3",             0x0000, 0x010000, CRC(d4c06db1) SHA1(dacb66b98f9d1d51eddc48b6946d517c277e588e), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 3.0 YD, set 1)", GAME_FLAGS )
GAME_CUSTOM( 1993, m4addr3_a,    m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "ad_20___.3a3",             0x0000, 0x010000, CRC(c2431657) SHA1(b2b7541207eb3c898f9cf3df520bff396213b78a), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 3.0, set 2)", GAME_FLAGS )
GAME_CUSTOM( 1993, m4addr3d_a,   m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "ad_20_d_.3a3",             0x0000, 0x010000, CRC(62304025) SHA1(59b7815bf1b5337f46083cef186fedd078a4ad37), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 3.0 D, set 2)", GAME_FLAGS )
GAME_CUSTOM( 1993, m4addr3yd_a,  m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "ad_20_b_.3a3",             0x0000, 0x010000, CRC(19990a19) SHA1(ab1031513fb1e499da4a3001b5b26ff1e86cc628), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 3.0 YD, set 2)", GAME_FLAGS )
GAME_CUSTOM( 1993, m4addr3_b,    m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "ad_20___.3n3",             0x0000, 0x010000, CRC(883ff001) SHA1(50540270dba31820ad99a4a4034c69d4a58d87c5), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 3.0, set 3)", GAME_FLAGS )
GAME_CUSTOM( 1993, m4addr3d_b,   m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "ad_20_d_.3n3",             0x0000, 0x010000, CRC(cf254a00) SHA1(1e430b652e4023e28b5648b8bea63e778c6dafc9), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 3.0 D, set 3)", GAME_FLAGS )
GAME_CUSTOM( 1993, m4addr3yd_b,  m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "ad_20_b_.3n3",             0x0000, 0x010000, CRC(65f9946f) SHA1(6bf6f315ed2dc6f603381d36dd408e951ace76bc), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 3.0 YD, set 3)", GAME_FLAGS )
GAME_CUSTOM( 1993, m4addr3_c,    m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "ad_20___.3s3",             0x0000, 0x010000, CRC(b1d54cb6) SHA1(35205975ccdaccd5bf3c1b7bf9a26c5ef30050b3), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 3.0, set 4)", GAME_FLAGS )
GAME_CUSTOM( 1993, m4addr3d_c,   m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "ad_20_d_.3s3",             0x0000, 0x010000, CRC(89d2301b) SHA1(62ad1a9e008063eb16442b50af806f061669dba7), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 3.0 D, set 4)", GAME_FLAGS )
GAME_CUSTOM( 1993, m4addr3yd_c,  m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "ad_20_b_.3s3",             0x0000, 0x010000, CRC(86982248) SHA1(a6d876333777a29eb0504fa3636727ebcc104f0a), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 3.0 YD, set 4)", GAME_FLAGS )
GAME_CUSTOM( 1993, m4addr3_d,    m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "adl5pv2",                  0x0000, 0x010000, CRC(09c39527) SHA1(16af3e552a7d6c6b802d2b1923523e9aa9de766a), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 3.0, set 5)", GAME_FLAGS )
// "(C)1994  B.W.B."  and "ADD 5.0"
GAME_CUSTOM( 1994, m4addr5,      m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "ad_05___.5a3",             0x0000, 0x010000, CRC(9821a988) SHA1(2be85a0b68e5e31401a5c753b40f3cf803589444), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 5.0, set 1)", GAME_FLAGS )
GAME_CUSTOM( 1994, m4addr5d,     m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "ad_05_d_.5a3",             0x0000, 0x010000, CRC(b5be8114) SHA1(28dfe1d1cc1d9fc2bcc13fd6437602a6e8c90de2), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 5.0 D, set 1)", GAME_FLAGS )
GAME_CUSTOM( 1994, m4addr5c,     m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "adi05___.5a3",             0x0000, 0x010000, CRC(03777f8c) SHA1(9e3fddc2130600f343df0531bf3e636b82c2f108), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 5.0 C, set 1)", GAME_FLAGS )
GAME_CUSTOM( 1994, m4addr5yd,    m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "ad_05_b_.5a3",             0x0000, 0x010000, CRC(592cb1ae) SHA1(5696ecb3e9e6419f73087120b6a832fde606bacc), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 5.0 YD, set 1)", GAME_FLAGS )
GAME_CUSTOM( 1994, m4addr5_a,    m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "ad_05___.5n3",             0x0000, 0x010000, CRC(86ac3564) SHA1(1dd9cf39d2aee11a3e1bbc68460c12f10e62aeaf), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 5.0, set 2)", GAME_FLAGS )
GAME_CUSTOM( 1994, m4addr5d_a,   m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "ad_05_d_.5n3",             0x0000, 0x010000, CRC(ca2653d5) SHA1(30cd35627be8fb4fff2f0d61a6ab43cf3e4c1742), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 5.0 D, set 2)", GAME_FLAGS )
GAME_CUSTOM( 1994, m4addr5c_a,   m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "adi05___.5n3",             0x0000, 0x010000, CRC(13003560) SHA1(aabad24748f9b1b09f1820bf1af932160e64fe3e), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 5.0 C, set 2)", GAME_FLAGS )
GAME_CUSTOM( 1994, m4addr5yd_a,  m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "ad_05_b_.5n3",             0x0000, 0x010000, CRC(cdc8ca39) SHA1(33fdeef8ab8908f6908120aedf501ec3e9d7d23e), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 5.0 YD, set 2)", GAME_FLAGS )
// "(C)1993  B.W.B."  and "ADD 4.0"
GAME_CUSTOM( 1993, m4addr4,      m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "ad_05___.4s3",             0x0000, 0x010000, CRC(6d1a3c51) SHA1(0e4b985173c7c3bd5804573d99913d66a05d54fb), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 4.0, set 1)", GAME_FLAGS )
GAME_CUSTOM( 1993, m4addr4c,     m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "adi05___.4s3",             0x0000, 0x010000, CRC(a4343a89) SHA1(cef67bbe03e6f535b530fc099f1b9a8bc7a2f864), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 4.0 C, set 1)", GAME_FLAGS )
GAME_CUSTOM( 1993, m4addr4d,     m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "ad_05_d_.4s3",             0x0000, 0x010000, CRC(e672baf0) SHA1(bae2e2fe9f51b3b8da20fcefb145f6d35fa2d604), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 4.0 D, set 1)", GAME_FLAGS )
GAME_CUSTOM( 1993, m4addr4yd,    m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "ad_05_b_.4s3",             0x0000, 0x010000, CRC(6bd6fdb6) SHA1(7ee1e80da5833b3eaf4b23035690a09379781584), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 4.0 YD, set 1)", GAME_FLAGS )
// "(C)1994  B.W.B."  and "ADD 4.0"
GAME_CUSTOM( 1994, m4addr4_a,    m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "ad_10___.4a3",             0x0000, 0x010000, CRC(9151dac3) SHA1(bf1c065a62e84a8073f8f9854981bedad60805be), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 4.0, set 2)", GAME_FLAGS )
GAME_CUSTOM( 1994, m4addr4c_a,   m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "adi10___.4a3",             0x0000, 0x010000, CRC(2d2aa3cc) SHA1(21a7690c3fb7d158f4b4e6da63663778246ac902), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 4.0 C, set 2)", GAME_FLAGS )
GAME_CUSTOM( 1994, m4addr4c_b,   m4addr, cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1, HT, P4L), m4addr, init_m4, "adi10___.4n3",             0x0000, 0x010000, CRC(af9aad00) SHA1(09729e73f27d9ac5d6ac7171191ed76aeaac3e3d), "BWB","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 4.0 C, set 3)", GAME_FLAGS )

// These have different protection
// "(C)1991 BARCREST" and "A6L 0.1" (but hack?)
GAME_CUSTOM( 199?, m4addrc__b,   m4addr, bootleg_fixedret<0x43>(R4, RT1, HT, P4L), m4addr, init_m4, "add20_101",                0x0000, 0x010000, CRC(361b7173) SHA1(dea2b1b0f5910e2fd3f45d220554f0e712dedada), "hack","Classic Adders & Ladders (Barcrest) (MPU4) (A6L 0.1, hack, set 1)", GAME_FLAGS )
GAME_CUSTOM( 199?, m4addrc__k,   m4addr, bootleg_fixedret<0x63>(R4, RT1, HT, P4L), m4addr, init_m4, "addl_20_.8",               0x0000, 0x010000, CRC(43c98f46) SHA1(0ca4a093b38fc04639e3f4bb742a8923b90d2ed1), "hack","Classic Adders & Ladders (Barcrest) (MPU4) (A6L 0.1, hack, set 2)", GAME_FLAGS )
// "BIG DIPPER"  and ADD 1.0
GAME_CUSTOM( 199?, m4addrc__h,   m4addr, bootleg_fixedret<0x1d>(R4, RT1, HT, P4L), m4addr, init_m4, "adders classic.bin",       0x0000, 0x010000, CRC(6bc1d2aa) SHA1(cf17e697ff0cfba999f6511f24051dbc3d0384ef), "hack","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 1.0, hack)", GAME_FLAGS )
GAME_CUSTOM( 1994, m4addr_h1,    m4addr, bootleg_fixedret<0x1d>(R4, RT1, HT, P4L), m4addr, init_m4, "5p4addersladders.bin",     0x0000, 0x010000, CRC(03fc43da) SHA1(cf2fdb0d1ad702331ba004fd39072484b05e2b97), "hack","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 1.0 C, hack, set 1)", GAME_FLAGS )
GAME_CUSTOM( 199?, m4addrc__m,   m4addr, bootleg_fixedret<0x1d>(R4, RT1, HT, P4L), m4addr, init_m4, "alad58c",                  0x0000, 0x010000, CRC(df9c46b8) SHA1(439ea1ce17aa89e19cedb78465b4388b72c8c5ed), "hack","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 1.0 C, hack, set 5)", GAME_FLAGS )
GAME_CUSTOM( 1994, m4addr_h2,    m4addr, bootleg_fixedret<0x61>(R4, RT1, HT, P4L), m4addr, init_m4, "ad05.6c",                  0x0000, 0x010000, CRC(0940e4aa) SHA1(e8e7f7249a18386af990999a4c06f001db7003c5), "hack","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 1.0 C, hack, set 2)", GAME_FLAGS )
GAME_CUSTOM( 199?, m4addrc,      m4addr, bootleg_fixedret<0x2d>(R4, RT1, HT, P4L), m4addr, init_m4, "add05_101",                0x0000, 0x010000, CRC(4b3fb104) SHA1(9dba619019a476ce317122a3553965b279c684ba), "hack","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 1.0 C, hack, set 3)", GAME_FLAGS )
GAME_CUSTOM( 199?, m4addrc__c,   m4addr, bootleg_fixedret<0x25>(R4, RT1, HT, P4L), m4addr, init_m4, "add55",                    0x0000, 0x010000, CRC(48c5bc73) SHA1(18c9f70bad6141cca95b6bbcb4fc621e71f87700), "hack","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 1.0 C, hack, set 4)", GAME_FLAGS )
// "DADS  ARMY" and "ADD 1.0"
GAME_CUSTOM( 199?, m4addrc__a,   m4addr, bootleg_fixedret<0x2b>(R4, RT1, HT, P4L), m4addr, init_m4, "add10_101",                0x0000, 0x010000, CRC(af8f8b4e) SHA1(712c33ed0f425dc10b79780b0cfce0ac5768e2d5), "hack","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 1.0 C, hack, set 6)", GAME_FLAGS )
GAME_CUSTOM( 199?, m4addrc__i,   m4addr, bootleg_fixedret<0x5f>(R4, RT1, HT, P4L), m4addr, init_m4, "addl_10_.4",               0x0000, 0x010000, CRC(c2d11126) SHA1(0eafe9dc30013ed5817ac303a4eea5ea82d62715), "hack","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 1.0 C, hack, set 7)", GAME_FLAGS )
GAME_CUSTOM( 199?, m4addrc__j,   m4addr, bootleg_fixedret<0x5f>(R4, RT1, HT, P4L), m4addr, init_m4, "addl_10_.8",               0x0000, 0x010000, CRC(9fc82c47) SHA1(0f56afc33f09fe22afc5ec74aeb496c32f9e623c), "hack","Classic Adders & Ladders (BWB / Barcrest) (MPU4) (ADD 1.0 C, hack, set 8)", GAME_FLAGS )


/*****************************************************************************************************************************************************************************
*
* Classic Adders and Ladders CLUB
*  - small extender
*
*****************************************************************************************************************************************************************************/

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAME(year, setname, parent, cheatchr_pal<mpu4_characteriser_pal::m578_characteriser_prot>(R5R, RT1, LPS), mpu420p, mpu4mod4yam_machines_state, init_m4, ROT0, company, title, GAME_FLAGS )

// "(C)1991 BARCREST" and "ADC 1.1"
GAME_CUSTOM( 199?, m4addrcc,       0,          "adcs.p1", 0x0000, 0x010000, CRC(7247de78) SHA1(e390b4e912d7bc8c1ca5e42bf2e2753d4c2b4d17), "Barcrest","Adders & Ladders Classic Club (Barcrest) (MPU4) (ADC 1.1)" )
GAME_CUSTOM( 199?, m4addrcc__c,    m4addrcc,   "adcd.p1", 0x0000, 0x010000, CRC(47e41c9a) SHA1(546aaaa5765b3bc91eeb9bf5a979ed68a2e72da8), "Barcrest","Adders & Ladders Classic Club (Barcrest) (MPU4) (ADC 1.1 D)" )
GAME_CUSTOM( 199?, m4addrcc__a,    m4addrcc,   "adcf.p1", 0x0000, 0x010000, CRC(1dbbc990) SHA1(fb9439b43089e3135a719ab94b24dd65561d17cf), "Barcrest","Adders & Ladders Classic Club (Barcrest) (MPU4) (ADC 1.1 F)" )
GAME_CUSTOM( 199?, m4addrcc__b,    m4addrcc,   "adcl.p1", 0x0000, 0x010000, CRC(89299196) SHA1(9a92b250b47b11536f8708429d69c95111ecdb98), "Barcrest","Adders & Ladders Classic Club (Barcrest) (MPU4) (ADC 1.1 L)" )
// "(C)1991 BARCREST" and "ADC 0.5"
GAME_CUSTOM( 199?, m4addrcc__d,    m4addrcc,   "adrscfm", 0x0000, 0x010000, CRC(6c95881a) SHA1(db658bd722c54fc84734105f1a9b0028b23179fb), "Barcrest","Adders & Ladders Classic Club (Barcrest) (MPU4) (ADC 0.5)" )


/*****************************************************************************************************************************************************************************
*
* Carry On Joker
* - no extender?
*
*****************************************************************************************************************************************************************************/

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAME(year, setname, parent, cheatchr_pal<mpu4_characteriser_pal::m4lv_characteriser_prot>(R4, RT1), mpu4, mpu4mod4yam_machines_state, init_m4, ROT0, company, title, GAME_FLAGS )

GAME_CUSTOM( 199?, m4cojok,     0,          "cojx.p1",      0x0000, 0x010000, CRC(a9c0aefb) SHA1(c5b367a01ddee2cb90e266f1e62459b9b96eb3e3), "Barcrest","Carry On Joker (Barcrest) (MPU4) (COJ 2.1, set 1)" )
GAME_CUSTOM( 199?, m4cojok__a,  m4cojok,    "cojxb.p1",     0x0000, 0x010000, CRC(2680c84a) SHA1(6cf9bb72df41ea1389334597a772fd197aba4fc4), "Barcrest","Carry On Joker (Barcrest) (MPU4) (COJ 2.1, set 2)" )
GAME_CUSTOM( 199?, m4cojok__b,  m4cojok,    "cojxc.p1",     0x0000, 0x010000, CRC(a67db981) SHA1(08ac65baf774c63705c3a4db36248777375404f6), "Barcrest","Carry On Joker (Barcrest) (MPU4) (COJ 2.1, set 3)" )
GAME_CUSTOM( 199?, m4cojok__c,  m4cojok,    "cojxcd.p1",    0x0000, 0x010000, CRC(33d31701) SHA1(a7ccaa5a3b1c97cc84cdca2f77381ea4a8d743a3), "Barcrest","Carry On Joker (Barcrest) (MPU4) (COJ 2.1, set 4)" )
GAME_CUSTOM( 199?, m4cojok__d,  m4cojok,    "cojxd.p1",     0x0000, 0x010000, CRC(97c12c95) SHA1(282dfc5bc66fd4ad57f442c3ae75f6645919352d), "Barcrest","Carry On Joker (Barcrest) (MPU4) (COJ 2.1, set 5)" )
GAME_CUSTOM( 199?, m4cojok__e,  m4cojok,    "cojxdy.p1",    0x0000, 0x010000, CRC(4f0be63b) SHA1(d701b5c2d2c71942f8574598a4ba687f532c16a8), "Barcrest","Carry On Joker (Barcrest) (MPU4) (COJ 2.1, set 6)" )
GAME_CUSTOM( 199?, m4cojok__f,  m4cojok,    "cojxy.p1",     0x0000, 0x010000, CRC(88f1b57a) SHA1(cfc98d6ec90e7c186741d62d3ec68bd350196878), "Barcrest","Carry On Joker (Barcrest) (MPU4) (COJ 2.1, set 7)" )


/*****************************************************************************************************************************************************************************
*
* Gamball
*
* has a Mechanical ball launcher to simulate random number generation
*
*****************************************************************************************************************************************************************************/

INPUT_PORTS_START( m4gambal )
	PORT_START("ORANGE1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("00")//  20p level
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("01")// 100p level
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("02")// Token 1 level
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("03")// Token 2 level
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("04")
	PORT_CONFNAME( 0xe0, 0x00, "Stake Key" )
	PORT_CONFSETTING(    0x00, "Not fitted / 5p"  )
	PORT_CONFSETTING(    0x20, "10p" )
	PORT_CONFSETTING(    0x40, "20p" )
	PORT_CONFSETTING(    0x60, "25p" )
	PORT_CONFSETTING(    0x80, "30p" )
	PORT_CONFSETTING(    0xa0, "40p" )
	PORT_CONFSETTING(    0xc0, "50p" )
	PORT_CONFSETTING(    0xe0, "1 GBP" )

	PORT_START("ORANGE2")
	PORT_CONFNAME( 0x0f, 0x00, "Jackpot / Prize Key" )
	PORT_CONFSETTING(    0x00, "Not fitted"  )
	PORT_CONFSETTING(    0x01, "3 GBP"  )
	PORT_CONFSETTING(    0x02, "4 GBP"  )
	PORT_CONFSETTING(    0x08, "5 GBP"  )
	PORT_CONFSETTING(    0x03, "6 GBP"  )
	PORT_CONFSETTING(    0x04, "6 GBP Token"  )
	PORT_CONFSETTING(    0x05, "8 GBP"  )
	PORT_CONFSETTING(    0x06, "8 GBP Token"  )
	PORT_CONFSETTING(    0x07, "10 GBP"  )
	PORT_CONFSETTING(    0x09, "15 GBP"  )
	PORT_CONFSETTING(    0x0a, "25 GBP"  )
	PORT_CONFSETTING(    0x0b, "25 GBP (Licensed Betting Office Profile)"  )
	PORT_CONFSETTING(    0x0c, "35 GBP"  )
	PORT_CONFSETTING(    0x0e, "70 GBP"  )
	PORT_CONFSETTING(    0x0d, "Reserved"  )
	PORT_CONFSETTING(    0x0f, "Reserved"  )

	PORT_CONFNAME( 0xf0, 0x00, "Percentage Key" )
	PORT_CONFSETTING(    0x00, "As Option Switches"  )
	PORT_CONFSETTING(    0x10, "70" )
	PORT_CONFSETTING(    0x20, "72" )
	PORT_CONFSETTING(    0x30, "74" )
	PORT_CONFSETTING(    0x40, "76" )
	PORT_CONFSETTING(    0x50, "78" )
	PORT_CONFSETTING(    0x60, "80" )
	PORT_CONFSETTING(    0x70, "82" )
	PORT_CONFSETTING(    0x80, "84" )
	PORT_CONFSETTING(    0x90, "86" )
	PORT_CONFSETTING(    0xa0, "88" )
	PORT_CONFSETTING(    0xb0, "90" )
	PORT_CONFSETTING(    0xc0, "92" )
	PORT_CONFSETTING(    0xd0, "94" )
	PORT_CONFSETTING(    0xe0, "96" )
	PORT_CONFSETTING(    0xf0, "98" )

	PORT_START("BLACK1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Hi")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Lo")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("18")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("19")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("20")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Test Button") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_INTERLOCK) PORT_NAME("Cashbox Door")  PORT_CODE(KEYCODE_Q) PORT_TOGGLE

	PORT_START("BLACK2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("24")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("25")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Cancel/Collect")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Hold/Nudge 1")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Hold/Nudge 2")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Hold/Nudge 3")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME("Hold/Nudge 4")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_START1)

	PORT_START("DIL1")
	PORT_DIPNAME( 0x80, 0x00, "DIL101" ) PORT_DIPLOCATION("DIL1:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "DIL102" ) PORT_DIPLOCATION("DIL1:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "DIL103" ) PORT_DIPLOCATION("DIL1:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL104" ) PORT_DIPLOCATION("DIL1:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL105" ) PORT_DIPLOCATION("DIL1:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL106" ) PORT_DIPLOCATION("DIL1:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL107" ) PORT_DIPLOCATION("DIL1:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x01, 0x00, "DIL108" ) PORT_DIPLOCATION("DIL1:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )

	PORT_START("DIL2")
	PORT_DIPNAME( 0x80, 0x00, "DIL201" ) PORT_DIPLOCATION("DIL2:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "DIL202" ) PORT_DIPLOCATION("DIL2:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "DIL203" ) PORT_DIPLOCATION("DIL2:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL204" ) PORT_DIPLOCATION("DIL2:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL205" ) PORT_DIPLOCATION("DIL2:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL206" ) PORT_DIPLOCATION("DIL2:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL207" ) PORT_DIPLOCATION("DIL2:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x01, 0x00, "DIL208" ) PORT_DIPLOCATION("DIL2:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )

	PORT_START("AUX1")
	PORT_BIT(0xFF, IP_ACTIVE_HIGH, IPT_CUSTOM)//Handled by Gamball unit

	PORT_START("AUX2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_CUSTOM)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_CUSTOM)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_CUSTOM)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_CUSTOM)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("10p")PORT_IMPULSE(5)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("20p")PORT_IMPULSE(5)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("50p")PORT_IMPULSE(5)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("100p")PORT_IMPULSE(5)
INPUT_PORTS_END


#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAMEL( year, setname, parent, cheatchr_gambal(R4, RT1), mpu4, mpu4mod4yam_machines_state, init_m4, ROT0, company, title, MACHINE_REQUIRES_ARTWORK | MACHINE_MECHANICAL, layout_gamball )

GAME_CUSTOM( 199?, m4gambal,       0,          "gbbx.p1",  0x0000, 0x010000, CRC(0b5adcd0) SHA1(1a198bd4a1e7d6bf4cf025c43d35aaef351415fc), "Barcrest","Gamball (Barcrest) (MPU4) (GBB 2.0)" )
GAME_CUSTOM( 199?, m4gambal__a,    m4gambal,   "gabcx.p1", 0x0000, 0x010000, CRC(52c35266) SHA1(bda49005de88094fbc84621f63b33f0e0a9c0bd3), "Barcrest","Gamball (Barcrest) (MPU4) (GAB 2.0, set 1)" )
GAME_CUSTOM( 199?, m4gambal__b,    m4gambal,   "gabx.p1",  0x0000, 0x010000, CRC(74a8ed7e) SHA1(7363031c8a634ac13de957c62f32611963f797bd), "Barcrest","Gamball (Barcrest) (MPU4) (GAB 2.0, set 2)" )
GAME_CUSTOM( 199?, m4gambal__c,    m4gambal,   "gbll20-6", 0x0000, 0x010000, CRC(f34d233a) SHA1(3f13563b2821b2f36267470c36ba346879521bc9), "Barcrest","Gamball (Barcrest) (MPU4) (GAB 2.0, set 3)" )


/*****************************************************************************************************************************************************************************
*
* Graffiti
* - no extender?
*
*****************************************************************************************************************************************************************************/

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAME(year, setname, parent, cheatchr_pal<mpu4_characteriser_pal::graff_characteriser_prot>(R4, RT1), mpu4, mpu4mod4yam_machines_state, init_m4, ROT0, company, title, GAME_FLAGS )

GAME_CUSTOM( 199?, m4graff,     0,          "graffo6",  0x0000, 0x010000, CRC(7349c9ca) SHA1(2744035d6c7897394c8fead27f48779047590fba), "Barcrest","Graffiti (Barcrest) (MPU4) (GRA 2.0X)" )
GAME_CUSTOM( 199?, m4graff__a,  m4graff,    "grax.p1",  0x0000, 0x010000, CRC(2e03a7d8) SHA1(333373fe15ae165dd24d5c11fef23f2e9b0388bf), "Barcrest","Graffiti (Barcrest) (MPU4) (GRA 2.1X)" )
GAME_CUSTOM( 199?, m4graff__b,  m4graff,    "graxc.p1", 0x0000, 0x010000, CRC(7620657b) SHA1(2aec38ee0f826c7bb012522fd098a6fdb857c9da), "Barcrest","Graffiti (Barcrest) (MPU4) (GRA 2.1CX)" )


/*****************************************************************************************************************************************************************************
*
* Monte Carlo
* - small extender
*
*****************************************************************************************************************************************************************************/

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAME(year, setname, parent, cheatchr_pal<mpu4_characteriser_pal::m435_characteriser_prot>(R4, RT1, LPS), mpu4, mpu4mod4yam_machines_state, init_m4, ROT0, company, title, GAME_FLAGS )

// 'with 8GBP Jackpot' sets
GAME_CUSTOM( 199?, m4monte,     0,          "nm8b.p1",  0x0000, 0x010000, CRC(1632080e) SHA1(9ca2cd8f00e49c29f4a216d3c9eacba221ada6ce), "Barcrest","Monte Carlo (NM8 0.1 B) (Barcrest) (MPU4)" )
GAME_CUSTOM( 199?, m4monte__a,  m4monte,    "nm8ad.p1", 0x0000, 0x010000, CRC(92a07e05) SHA1(94015b219fffb8ad9a40a804a4e0b0fad61cdf21), "Barcrest","Monte Carlo (NM8 0.1 AD) (Barcrest) (MPU4)" )
GAME_CUSTOM( 199?, m4monte__b,  m4monte,    "nm8bd.p1", 0x0000, 0x010000, CRC(a4bc134f) SHA1(72af6b66a5ea7566289bd9bdf8975c29dbb547cf), "Barcrest","Monte Carlo (NM8 0.1 BD) (Barcrest) (MPU4)" )
GAME_CUSTOM( 199?, m4monte__c,  m4monte,    "nm8c.p1",  0x0000, 0x010000, CRC(7e558a64) SHA1(9f325aa9a5b036c317686b901b4c65c1e23fd845), "Barcrest","Monte Carlo (NM8 0.1 C) (Barcrest) (MPU4)" )
GAME_CUSTOM( 199?, m4monte__d,  m4monte,    "nm8d.p1",  0x0000, 0x010000, CRC(66716e7d) SHA1(719d32a3486accfa1c2e8e2ca53c05f916927e7a), "Barcrest","Monte Carlo (NM8 0.1 D) (Barcrest) (MPU4)" )
GAME_CUSTOM( 199?, m4monte__e,  m4monte,    "nm8dk.p1", 0x0000, 0x010000, CRC(ae4866e8) SHA1(5ec210b6b69f72b85abe5844b800b251fef20fc5), "Barcrest","Monte Carlo (NM8 0.1 KD) (Barcrest) (MPU4)" )
GAME_CUSTOM( 199?, m4monte__f,  m4monte,    "nm8dy.p1", 0x0000, 0x010000, CRC(9d6f71a5) SHA1(577d39eef82761fff30f851282cd85b84ac22953), "Barcrest","Monte Carlo (NM8 0.1 YD) (Barcrest) (MPU4)" )
GAME_CUSTOM( 199?, m4monte__g,  m4monte,    "nm8k.p1",  0x0000, 0x010000, CRC(47c00612) SHA1(647216e7489043f90e0cd807ddc3d631842b3f7f), "Barcrest","Monte Carlo (NM8 0.1 K) (Barcrest) (MPU4)" )
GAME_CUSTOM( 199?, m4monte__h,  m4monte,    "nm8s.p1",  0x0000, 0x010000, CRC(cf8fd333) SHA1(4b2b98d0c3d043a6425a6d82f7a98cf662582832), "Barcrest","Monte Carlo (NM8 0.1) (Barcrest) (MPU4)" )
GAME_CUSTOM( 199?, m4monte__i,  m4monte,    "nm8y.p1",  0x0000, 0x010000, CRC(cbb96053) SHA1(9fb6c449d8e26ecacfa9ba40979134c705ecb1be), "Barcrest","Monte Carlo (NM8 0.1 Y) (Barcrest) (MPU4)" )
// 'with 6GBP Jackpot' sets
GAME_CUSTOM( 199?, m4monte__j,  m4monte,    "nmnc.p1",  0x0000, 0x010000, CRC(c2fdcc91) SHA1(aa3ec11425adee94c24b3a1472541e7e04e4000a), "Barcrest","Monte Carlo (NMN 0.1 C) (Barcrest) (MPU4)" )
GAME_CUSTOM( 199?, m4monte__k,  m4monte,    "nmnd.p1",  0x0000, 0x010000, CRC(94985809) SHA1(636b9106ea330a238f3d4168636fbf21021a7216), "Barcrest","Monte Carlo (NMN 0.1 D) (Barcrest) (MPU4)" )
GAME_CUSTOM( 199?, m4monte__l,  m4monte,    "nmnk.p1",  0x0000, 0x010000, CRC(8d022ae6) SHA1(01e12acbed34a2d4fb81dc9da12441ddc31f605b), "Barcrest","Monte Carlo (NMN 0.1 K) (Barcrest) (MPU4)" )
GAME_CUSTOM( 199?, m4monte__m,  m4monte,    "nmns.p1",  0x0000, 0x010000, CRC(48e2ab70) SHA1(bc452a36374a6e62516aad1a4887876ee9da37f7), "Barcrest","Monte Carlo (NMN 0.1) (Barcrest) (MPU4)" )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAME(year, setname, parent, cheatchr_pal<mpu4_characteriser_pal::toplot_characteriser_prot>(R4, RT1, LPS), mpu4, mpu4mod4yam_machines_state, init_m4, ROT0, company, title, GAME_FLAGS )

// Bwb sets
GAME_CUSTOM( 1995, m4monteza,   m4monte,    "mx_05a__.2_1", 0x0000, 0x010000, CRC(a1a03e03) SHA1(bf49b516e6824a47cd9bf1408bf676f9f1e43d62), "BWB","Monte Carlo (MX052.0 K) (BWB) (MPU4)" )
GAME_CUSTOM( 1995, m4montezi,   m4monte,    "mxi05___.2_1", 0x0000, 0x010000, CRC(de425b55) SHA1(2aa63bbd32c766e7e2d888345115c3185dc03bff), "BWB","Monte Carlo (MX052.0 C) (BWB) (MPU4)" )
GAME_CUSTOM( 1995, m4montezb,   m4monte,    "mx_10a__.2_1", 0x0000, 0x010000, CRC(bbf21e9f) SHA1(901b14b96cdb0945f491c39707ab9d2b9a2d25dd), "BWB","Monte Carlo (MX102.0 K) (BWB) (MPU4)" )
GAME_CUSTOM( 1995, m4montezj,   m4monte,    "mxi10___.2_1", 0x0000, 0x010000, CRC(19077425) SHA1(e31da38a903345c65b083cac192555f1f4ba2e5a), "BWB","Monte Carlo (MX102.0 C) (BWB) (MPU4)" )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAME(year, setname, parent, cheatchr_pal<mpu4_characteriser_pal::montealt_characteriser_prot>(R4, RT1, LPS), mpu4, mpu4mod4yam_machines_state, init_m4, ROT0, company, title, GAME_FLAGS )

GAME_CUSTOM( 1996, m4montezc,   m4monte,    "mx_20__c.1_1", 0x0000, 0x010000, CRC(a753798d) SHA1(ae1f5f14a37dead66f6b2d075a5bfc019d59f806), "BWB","Monte Carlo (MC  2.0 C) (BWB) (MPU4)" )
GAME_CUSTOM( 1996, m4montezd,   m4monte,    "mx_20a_c.1_1", 0x0000, 0x010000, CRC(9ec6f5fb) SHA1(ee181a64557053349cc8bff86bba937b191cab01), "BWB","Monte Carlo (MC  2.0 K) (BWB) (MPU4)" )
GAME_CUSTOM( 1996, m4monteze,   m4monte,    "mx_20dkc.1_1", 0x0000, 0x010000, CRC(d580f742) SHA1(3c1d6aba4068d60ab53eceecf65bc920f8b5604e), "BWB","Monte Carlo (MC  2.0 YD) (BWB) (MPU4)" )
GAME_CUSTOM( 1996, m4montezf,   m4monte,    "mx_25__c.3_1", 0x0000, 0x010000, CRC(11ae121d) SHA1(11e61db1c645410ac18ef429cde167a7774be5f5), "BWB","Monte Carlo (MC_ 3.0 C) (BWB) (MPU4)" )
GAME_CUSTOM( 1996, m4montezh,   m4monte,    "mx_25a_c.3_1", 0x0000, 0x010000, CRC(283b9e6b) SHA1(937da8bda49a7a0fa1f728770f96d10a65bfe7bc), "BWB","Monte Carlo (MC_ 3.0 K) (BWB) (MPU4)" )
GAME_CUSTOM( 1996, m4montezg,   m4monte,    "mx_25_bc.3_1", 0x0000, 0x010000, CRC(4228139c) SHA1(a448ddc034923cba58ee298fd2a4c2cdd4f84f04), "BWB","Monte Carlo (MC_ 3.0 YD) (BWB) (MPU4)" )

/*****************************************************************************************************************************************************************************
*
* Monte Carlo (with Prizes)
*
*****************************************************************************************************************************************************************************/

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAME(year, setname, parent, cheatchr_pal<mpu4_characteriser_pal::m435_characteriser_prot>(R4, RT1, LPS), mpu4, mpu4mod4yam_machines_state, init_m4, ROT0, company, title, GAME_FLAGS )

GAME_CUSTOM( 199?, m4przmc,     0,          "mssb.p1",  0x0000, 0x010000, CRC(5210dae0) SHA1(cc9916718249505e031ccdbc126f3fa1e6675f27), "Barcrest","Prize Monte Carlo (MSS 1.6 B) (Barcrest) (MPU4)" )
GAME_CUSTOM( 199?, m4przmc__a,  m4przmc,    "mssad.p1", 0x0000, 0x010000, CRC(e3690c35) SHA1(fdaacda0d03ce8d54841525feff2529b1ee1f970), "Barcrest","Prize Monte Carlo (MSS 1.6 AD) (Barcrest) (MPU4)" )
GAME_CUSTOM( 199?, m4przmc__b,  m4przmc,    "mssd.p1",  0x0000, 0x010000, CRC(cf59305e) SHA1(7ba6f37aa1077561129f66ab663730fb6e5108ed), "Barcrest","Prize Monte Carlo (MSS 1.6 D) (Barcrest) (MPU4)" )
GAME_CUSTOM( 199?, m4przmc__c,  m4przmc,    "mssdy.p1", 0x0000, 0x010000, CRC(12d7db63) SHA1(6e1e6b13783888f3d508d7cbecc52c65ffc99fb0), "Barcrest","Prize Monte Carlo (MSS 1.6 YD) (Barcrest) (MPU4)" )
GAME_CUSTOM( 199?, m4przmc__d,  m4przmc,    "mssk.p1",  0x0000, 0x010000, CRC(d56f62dc) SHA1(7df1fad20901607e710e8a7f64033f77d613a0fa), "Barcrest","Prize Monte Carlo (MSS 1.6 K) (Barcrest) (MPU4)" )
GAME_CUSTOM( 199?, m4przmc__e,  m4przmc,    "msss.p1",  0x0000, 0x010000, CRC(c854c12f) SHA1(917d091383b07a995dc2c441717885b181a02d3c), "Barcrest","Prize Monte Carlo (MSS 1.6) (Barcrest) (MPU4)" )
GAME_CUSTOM( 199?, m4przmc__f,  m4przmc,    "mssy.p1",  0x0000, 0x010000, CRC(159f4baa) SHA1(073c13e6bff4a641b29e5a45f88e3533aff460e4), "Barcrest","Prize Monte Carlo (MSS 1.6 Y) (Barcrest) (MPU4)" )
GAME_CUSTOM( 1991, m4przmc__g,  m4przmc,    "montvnd",  0x0000, 0x010000, CRC(9858bb1d) SHA1(a2d3de2cec7420cc6f7da2239bdc79d7c4b7394e), "Barcrest","Prize Monte Carlo (MSS 1.6 C) (Barcrest) (MPU4)" )

// sets below were in Prize Money sets, but boot to show 'Monte Carlo with Prizes'
// "(C)1995  B.W.B." and "MC 53.0"
GAME_CUSTOM( 199?, m4przmc__h,    m4przmc,   "mt_05a__.3o3", 0x0000, 0x010000, CRC(4175f4a9) SHA1(b0e172e4862aa3b7be7accefc90e98d07d449b65), "BWB","Prize Monte Carlo (MC 53.0 K) (Barcrest) (MPU4)" )
GAME_CUSTOM( 199?, m4przmc__i,    m4przmc,   "mt_05s__.3o3", 0x0000, 0x010000, CRC(92d674b7) SHA1(a828a9b0d870122bc09d865de90b8efa428f3fd0), "BWB","Prize Monte Carlo (MC 53.0) (Barcrest) (MPU4)" )
GAME_CUSTOM( 199?, m4przmc__j,    m4przmc,   "mt_05sb_.3o3", 0x0000, 0x010000, CRC(1158e506) SHA1(8c91bfe29545bbbc0d136a8c9abef785cadc3c64), "BWB","Prize Monte Carlo (MC 53.0 YD) (Barcrest) (MPU4)" )
GAME_CUSTOM( 199?, m4przmc__k,    m4przmc,   "mt_05sd_.3o3", 0x0000, 0x010000, CRC(5ed3d947) SHA1(4b9bc9be6e79014ad6ca95293eb464af39e40dc1), "BWB","Prize Monte Carlo (MC 53.0 D) (Barcrest) (MPU4)" )
// "(C)1995  B.W.B." and "MC103.0"
GAME_CUSTOM( 199?, m4przmc__l,    m4przmc,   "mt_10a__.3o3", 0x0000, 0x010000, CRC(6a8172a4) SHA1(92c081535258677e90d9f9748a168926c7a0cbed), "BWB","Prize Monte Carlo (MC103.0 K) (Barcrest) (MPU4)" )
GAME_CUSTOM( 199?, m4przmc__m,    m4przmc,   "mt_10s__.3o3", 0x0000, 0x010000, CRC(1b66f0f8) SHA1(308227b0144f0568df8190810e0de627b413a742), "BWB","Prize Monte Carlo (MC103.0) (Barcrest) (MPU4)" )
GAME_CUSTOM( 199?, m4przmc__n,    m4przmc,   "mt_10sb_.3o3", 0x0000, 0x010000, CRC(06a33d34) SHA1(5fa1269a7cf42ef14e2a19143a07bf28b38ad920), "BWB","Prize Monte Carlo (MC103.0 YD) (Barcrest) (MPU4)" )
GAME_CUSTOM( 199?, m4przmc__o,    m4przmc,   "mt_10sd_.3o3", 0x0000, 0x010000, CRC(42629cb1) SHA1(12f695e1f70bf93100c1af8052dcee9131711510), "BWB","Prize Monte Carlo (MC103.0 D) (Barcrest) (MPU4)" )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAME(year, setname, parent, cheatchr_pal<mpu4_characteriser_pal::przmontealt_characteriser_prot>(R4, RT1, LPS), mpu4, mpu4mod4yam_machines_state, init_m4, ROT0, company, title, GAME_FLAGS )

// "(C)1995  B.W.B." and "MT054.0"
GAME_CUSTOM( 199?, m4przmc__p,    m4przmc,   "mt_05a__.4o1", 0x0000, 0x010000, CRC(637fecee) SHA1(8c970bdf703177c71dde5c774c75929ac42b6eb0), "BWB","Prize Monte Carlo (MT054.0 K) (Barcrest) (MPU4)" )
GAME_CUSTOM( 199?, m4przmc__q,    m4przmc,   "mti05___.4o1", 0x0000, 0x010000, CRC(0e82c258) SHA1(c4aa7d32bcd9418e2919be8be8a2f9e60d46f316), "BWB","Prize Monte Carlo (MT054.0 C) (Barcrest) (MPU4)" )
// "(C)1995  B.W.B." and "MT104.0"
GAME_CUSTOM( 199?, m4przmc__r,    m4przmc,   "mt_10a__.4o1", 0x0000, 0x010000, CRC(36eeac30) SHA1(daa662392874806d18d4a161d39caed7e0abca73), "BWB","Prize Monte Carlo (MT104.0 K) (Barcrest) (MPU4)" )
GAME_CUSTOM( 199?, m4przmc__s,    m4przmc,   "mti10___.4o1", 0x0000, 0x010000, CRC(a35e0571) SHA1(9a22946047e76392f0c4534f892ee9ae9e700503), "BWB","Prize Monte Carlo (MT104.0 C) (Barcrest) (MPU4)" )



/*****************************************************************************************************************************************************************************
*
* Nudge Banker
* - no extender?
*
*****************************************************************************************************************************************************************************/

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAME(year, setname, parent, cheatchr_pal<mpu4_characteriser_pal::clbveg_characteriser_prot>(R4, RT1), mpu4, mpu4mod4yam_machines_state, init_m4, ROT0, company, title, GAME_FLAGS )

// (C)1991 BARCREST and SBN 2.0
GAME_CUSTOM( 199?, m4nudbnk,       0,          "nb6",      0x0000, 0x010000, CRC(010dd3fc) SHA1(645cbe54200a6c3327e10909b1ef3a80579e96e5), "Barcrest","Nudge Banker (Barcrest) (MPU4) (SBN 2.0X)" )
// (C)1991 BARCREST and NBN 2.0
GAME_CUSTOM( 199?, m4nudbnk__a,    m4nudbnk,   "nbncx.p1", 0x0000, 0x010000, CRC(57bbbedf) SHA1(d42d3176f41aedf2ddc15cdf73ab97e963b92213), "Barcrest","Nudge Banker (Barcrest) (MPU4) (NBN 1.0CX)" )
GAME_CUSTOM( 199?, m4nudbnk__b,    m4nudbnk,   "nbnx.p1",  0x0000, 0x010000, CRC(075053d5) SHA1(43b9f6bb3a4ab531eb168007ceaf713261736144), "Barcrest","Nudge Banker (Barcrest) (MPU4) (NBN 1.0X)" )
// (C)1991 BARCREST and SBN 1.1
GAME_CUSTOM( 199?, m4nudbnk__c,    m4nudbnk,   "sbns.p1",  0x0000, 0x010000, CRC(92aa5b8d) SHA1(4f6e309e152266b8f40077a7d734b2b9042570d2), "Barcrest","Nudge Banker (Barcrest) (MPU4) (SBN 1.1)" )
GAME_CUSTOM( 199?, m4nudbnk__d,    m4nudbnk,   "sbnx.p1",  0x0000, 0x010000, CRC(861cbc50) SHA1(61166ea9092e2890ea9de421cc031d3a79335233), "Barcrest","Nudge Banker (Barcrest) (MPU4) (SBN 1.1X)" )


/*****************************************************************************************************************************************************************************
*
* Spend Spend Spend
* - small extender
*
*****************************************************************************************************************************************************************************/

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent, machine, inputs, init, name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAME(year, setname, parent, machine, inputs, mpu4mod4yam_machines_state, init, ROT0, company, title, GAME_FLAGS )

// (C)1995  B.W.B. and SP5 1.0
GAME_CUSTOM( 199?, m4sss,     0,     cheatchr_pal<mpu4_characteriser_pal::viva_sss_characteriser_prot>(R4, RT1, LPS), mpu4,init_m4, "sp_05a__.1o3",     0x0000, 0x010000, CRC(044a0133) SHA1(7cf85cf19f5c3f588daf5c0d7efe4204d67161a2), "BWB","Spend Spend Spend (BWB) (MPU4) (SP5 1.0, set 1)" )
GAME_CUSTOM( 199?, m4sss__b,  m4sss, cheatchr_pal<mpu4_characteriser_pal::viva_sss_characteriser_prot>(R4, RT1, LPS), mpu4,init_m4, "sp_05s__.1o3",     0x0000, 0x010000, CRC(2e000a62) SHA1(e60390a383388e385bbde79ca14c63e5d69a8869), "BWB","Spend Spend Spend (BWB) (MPU4) (SP5 1.0, set 2)" )
GAME_CUSTOM( 199?, m4sss__c,  m4sss, cheatchr_pal<mpu4_characteriser_pal::viva_sss_characteriser_prot>(R4, RT1, LPS), mpu4,init_m4, "sp_05sb_.1o3",     0x0000, 0x010000, CRC(c6380ef5) SHA1(673044aae9998dfe52205a5e4a3d26361f01c518), "BWB","Spend Spend Spend (BWB) (MPU4) (SP5 1.0, set 3)" )
GAME_CUSTOM( 199?, m4sss__d,  m4sss, cheatchr_pal<mpu4_characteriser_pal::viva_sss_characteriser_prot>(R4, RT1, LPS), mpu4,init_m4, "sp_05sd_.1o3",     0x0000, 0x010000, CRC(31f818e1) SHA1(bbfa45ef63a73aa726a8223be234fb8ffba45e24), "BWB","Spend Spend Spend (BWB) (MPU4) (SP5 1.0, set 4)" )
// (C)1995  B.W.B. and SP101.0
GAME_CUSTOM( 199?, m4sss__e,  m4sss, cheatchr_pal<mpu4_characteriser_pal::viva_sss_characteriser_prot>(R4, RT1, LPS), mpu4,init_m4, "sp_10a__.1o3",     0x0000, 0x010000, CRC(918c038c) SHA1(608062dc4e39c15967e16d95945b65ef7feabea2), "BWB","Spend Spend Spend (BWB) (MPU4) (SP101.0, set 1)" )
GAME_CUSTOM( 199?, m4sss__f,  m4sss, cheatchr_pal<mpu4_characteriser_pal::viva_sss_characteriser_prot>(R4, RT1, LPS), mpu4,init_m4, "sp_10s__.1o3",     0x0000, 0x010000, CRC(1bc5780a) SHA1(df1b5d0d6f4751a480aef77be40fb2cfd153bf18), "BWB","Spend Spend Spend (BWB) (MPU4) (SP101.0, set 2)" )
GAME_CUSTOM( 199?, m4sss__g,  m4sss, cheatchr_pal<mpu4_characteriser_pal::viva_sss_characteriser_prot>(R4, RT1, LPS), mpu4,init_m4, "sp_10sb_.1o3",     0x0000, 0x010000, CRC(2dfc3926) SHA1(b6b201c65c182f9b18a590910183ce88b245af2b), "BWB","Spend Spend Spend (BWB) (MPU4) (SP101.0, set 3)" )
GAME_CUSTOM( 199?, m4sss__h,  m4sss, cheatchr_pal<mpu4_characteriser_pal::viva_sss_characteriser_prot>(R4, RT1, LPS), mpu4,init_m4, "sp_10sd_.1o3",     0x0000, 0x010000, CRC(fe5c7e3e) SHA1(f5066f1f0c2220da874cbac0ce510cbac6fff8e7), "BWB","Spend Spend Spend (BWB) (MPU4) (SP101.0, set 4)" )
// (C)1995  B.W.B. and SX5 2.0
GAME_CUSTOM( 199?, m4sss__i,  m4sss, cheatchr_pal<mpu4_characteriser_pal::m450_characteriser_prot>(R4, RT1, LPS), mpu4,init_m4, "sx_05a__.2_1",     0x0000, 0x010000, CRC(ceb830a1) SHA1(c9bef44d64a64872460ae3c450533fd14c92ca43), "BWB","Spend Spend Spend (BWB) (MPU4) (SX5 2.0, set 1)" )
GAME_CUSTOM( 199?, m4sss__k,  m4sss, cheatchr_pal<mpu4_characteriser_pal::m450_characteriser_prot>(R4, RT1, LPS), mpu4,init_m4, "sxi05___.2_1",     0x0000, 0x010000, CRC(a804a20b) SHA1(477d2a750c0c252ffa215c3cf89916cb3a296b92), "BWB","Spend Spend Spend (BWB) (MPU4) (SX5 2.0, set 2)" )
// (C)1995  B.W.B. and SX102.0
GAME_CUSTOM( 199?, m4sss__j,  m4sss, cheatchr_pal<mpu4_characteriser_pal::m450_characteriser_prot>(R4, RT1, LPS), mpu4,init_m4, "sx_10a__.2_1",     0x0000, 0x010000, CRC(73e3bc13) SHA1(004097cc9cd62b8fa4c584fcb9874cf998c7b89d), "BWB","Spend Spend Spend (BWB) (MPU4) (SX102.0, set 1)" )
GAME_CUSTOM( 199?, m4sss__l,  m4sss, cheatchr_pal<mpu4_characteriser_pal::m450_characteriser_prot>(R4, RT1, LPS), mpu4,init_m4, "sxi10___.2_1",     0x0000, 0x010000, CRC(bbb23438) SHA1(2cc4376f6393c69c1e18ad06be18933592b6bdae), "BWB","Spend Spend Spend (BWB) (MPU4) (SX102.0, set 2)" )


/*****************************************************************************************************************************************************************************
*
* Spend Spend Spend (with Prizes)
* - small extender
*
*****************************************************************************************************************************************************************************/

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAME(year, setname, parent, cheatchr_pal<mpu4_characteriser_pal::duty_characteriser_prot>(R4, RT1, LPS), mpu4, mpu4mod4yam_machines_state, init_m4, ROT0, company, title, GAME_FLAGS )

// (C)1991 BARCREST and PS3 0.2
GAME_CUSTOM( 199?, m4przsss,       0,          "ps302b.p1",    0x0000, 0x010000, CRC(1749ae18) SHA1(f04f91a1d534f2d2dc844862bb21160c5903d1df), "Barcrest","Prize Spend Spend Spend (Barcrest) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4przsss__a,    m4przsss,   "ps302ad.p1",   0x0000, 0x010000, CRC(e57f52d7) SHA1(25384517b68c488acd38956aeb69dda26d63c3ca), "Barcrest","Prize Spend Spend Spend (Barcrest) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4przsss__b,    m4przsss,   "ps302bd.p1",   0x0000, 0x010000, CRC(d3633f9d) SHA1(2500425d736a5c45f5bf40a7660b549f822266dc), "Barcrest","Prize Spend Spend Spend (Barcrest) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4przsss__c,    m4przsss,   "ps302d.p1",    0x0000, 0x010000, CRC(df1bfe3b) SHA1(a82574ff9eb04deccfbb8907ca8936b53f691b2c), "Barcrest","Prize Spend Spend Spend (Barcrest) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4przsss__d,    m4przsss,   "ps302dk.p1",   0x0000, 0x010000, CRC(88b49246) SHA1(122384d6c350e28fdbb3e2a02e5db7076ec4bb43), "Barcrest","Prize Spend Spend Spend (Barcrest) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4przsss__e,    m4przsss,   "ps302dy.p1",   0x0000, 0x010000, CRC(ada3ab8c) SHA1(421aaf0951cb1d47b7138ca611d2ebd6caf24a61), "Barcrest","Prize Spend Spend Spend (Barcrest) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4przsss__f,    m4przsss,   "ps302k.p1",    0x0000, 0x010000, CRC(23719bee) SHA1(13b7fd4f9edc60727e37078f6f2e24a63abd09f1), "Barcrest","Prize Spend Spend Spend (Barcrest) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4przsss__g,    m4przsss,   "ps302s.p1",    0x0000, 0x010000, CRC(4521c521) SHA1(90b5e444829ecc9a9b3e46f942830d263fbf02d3), "Barcrest","Prize Spend Spend Spend (Barcrest) (MPU4) (set 8)" )
GAME_CUSTOM( 199?, m4przsss__h,    m4przsss,   "ps302y.p1",    0x0000, 0x010000, CRC(2ffed329) SHA1(a917161a7ea8312ef6a4a9a85f36f3b0a42b3a0c), "Barcrest","Prize Spend Spend Spend (Barcrest) (MPU4) (set 9)" )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAME(year, setname, parent, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1, LPS), mpu4, mpu4mod4yam_machines_state, init_m4, ROT0, company, title, GAME_FLAGS )

// (C)1991 BARCREST and PS8 0.1
GAME_CUSTOM( 199?, m4przsss__i,    m4przsss,   "ps8ad.p1",     0x0000, 0x010000, CRC(48917a87) SHA1(d32ac9e30ebddb6ca1d6a7d6c38026338c6df2cd), "Barcrest","Prize Spend Spend Spend (Barcrest) (MPU4) (set 10)" )
GAME_CUSTOM( 199?, m4przsss__j,    m4przsss,   "ps8b.p1",      0x0000, 0x010000, CRC(7633226d) SHA1(581dfb56719682a744fe2b4f63bd1c20eb943903), "Barcrest","Prize Spend Spend Spend (Barcrest) (MPU4) (set 11)" )
GAME_CUSTOM( 199?, m4przsss__k,    m4przsss,   "ps8bd.p1",     0x0000, 0x010000, CRC(92e384db) SHA1(ab1c2c7aebb9c8c0cff6dd43d74551c15de0c805), "Barcrest","Prize Spend Spend Spend (Barcrest) (MPU4) (set 12)" )
GAME_CUSTOM( 199?, m4przsss__l,    m4przsss,   "ps8d.p1",      0x0000, 0x010000, CRC(4b8a1374) SHA1(112fc0f0d1311482d292704ab807e15024b37cb9), "Barcrest","Prize Spend Spend Spend (Barcrest) (MPU4) (set 13)" )
GAME_CUSTOM( 199?, m4przsss__m,    m4przsss,   "ps8dj.p1",     0x0000, 0x010000, CRC(9949fe88) SHA1(8ba8fd30bb12e47b97ddb9f4aba1eac880e5a12e), "Barcrest","Prize Spend Spend Spend (Barcrest) (MPU4) (set 14)" )
GAME_CUSTOM( 199?, m4przsss__n,    m4przsss,   "ps8dk.p1",     0x0000, 0x010000, CRC(61e56c80) SHA1(93ef6601397063f412b35cbe90a5f7ecb3af2491), "Barcrest","Prize Spend Spend Spend (Barcrest) (MPU4) (set 15)" )
GAME_CUSTOM( 199?, m4przsss__o,    m4przsss,   "ps8dy.p1",     0x0000, 0x010000, CRC(d4080a4a) SHA1(9907fea71237742595e5acd583c190a6180b4af9), "Barcrest","Prize Spend Spend Spend (Barcrest) (MPU4) (set 16)" )
GAME_CUSTOM( 199?, m4przsss__p,    m4przsss,   "ps8j.p1",      0x0000, 0x010000, CRC(a9dcd1a8) SHA1(ec840aace95cab8c626a54636b47058401ef1eed), "Barcrest","Prize Spend Spend Spend (Barcrest) (MPU4) (set 17)" )
GAME_CUSTOM( 199?, m4przsss__q,    m4przsss,   "ps8k.p1",      0x0000, 0x010000, CRC(7ed46dac) SHA1(481556298696d7f73d834034d0ce8628eb95b76c), "Barcrest","Prize Spend Spend Spend (Barcrest) (MPU4) (set 18)" )
GAME_CUSTOM( 199?, m4przsss__r,    m4przsss,   "ps8s.p1",      0x0000, 0x010000, CRC(820a600a) SHA1(48701e315a94f92048ceb2e98df2bac1f04415e1), "Barcrest","Prize Spend Spend Spend (Barcrest) (MPU4) (set 19)" )
GAME_CUSTOM( 199?, m4przsss__s,    m4przsss,   "ps8y.p1",      0x0000, 0x010000, CRC(a4d6934b) SHA1(215ed246f37daf1f8cdd0113b7b87e89c1aa2514), "Barcrest","Prize Spend Spend Spend (Barcrest) (MPU4) (set 20)" )

// (C)1991 BARCREST and SSP 0.5
GAME_CUSTOM( 199?, m4przsss__t,    m4przsss,   "sspb.p1",      0x0000, 0x010000, CRC(a781cdb8) SHA1(cbb1b9a85a80db7c91752349546bf55df4aea3f2), "Barcrest","Prize Spend Spend Spend (Barcrest) (MPU4) (SSP 0.5 B)" )
GAME_CUSTOM( 199?, m4przsss__u,    m4przsss,   "sspd.p1",      0x0000, 0x010000, CRC(bcce54d7) SHA1(00a967188ddf1588331cda60e2589f6635e0a7ea), "Barcrest","Prize Spend Spend Spend (Barcrest) (MPU4) (SSP 0.5 D)" )
GAME_CUSTOM( 199?, m4przsss__v,    m4przsss,   "sspdb.p1",     0x0000, 0x010000, CRC(edb5961e) SHA1(e1127d34148f04f9e34074269ee3740269105c63), "Barcrest","Prize Spend Spend Spend (Barcrest) (MPU4) (SSP 0.5 BD)" )
GAME_CUSTOM( 199?, m4przsss__w,    m4przsss,   "sspdy.p1",     0x0000, 0x010000, CRC(a368812e) SHA1(f377f13b866196fdbba07529f25713f9b5b91df5), "Barcrest","Prize Spend Spend Spend (Barcrest) (MPU4) (SSP 0.5 YD)" )
GAME_CUSTOM( 199?, m4przsss__x,    m4przsss,   "sspr.p1",      0x0000, 0x010000, CRC(720bad67) SHA1(3ee25abfc15e1c36a3ac6ac94e5229f938a39991), "Barcrest","Prize Spend Spend Spend (Barcrest) (MPU4) (SSP 0.5 R)" )
GAME_CUSTOM( 199?, m4przsss__y,    m4przsss,   "ssprd.p1",     0x0000, 0x010000, CRC(b2ec7b80) SHA1(b562fbf2501dbaf0ec7c66d993df867384e750ff), "Barcrest","Prize Spend Spend Spend (Barcrest) (MPU4) (SSP 0.5 RD)" )
GAME_CUSTOM( 199?, m4przsss__z,    m4przsss,   "ssps.p1",      0x0000, 0x010000, CRC(e36f4d48) SHA1(fb88e8bcddb7dd2722b203a0ebb3a64c6b75ff24), "Barcrest","Prize Spend Spend Spend (Barcrest) (MPU4) (SSP 0.5)" )
GAME_CUSTOM( 199?, m4przsss__0,    m4przsss,   "sspy.p1",      0x0000, 0x010000, CRC(0ea8f052) SHA1(3134ff47e6c5c4d200ffcdf0a5a3cb7b05b0fc2c), "Barcrest","Prize Spend Spend Spend (Barcrest) (MPU4) (SSP 0.5 Y)" )
GAME_CUSTOM( 199?, m4przsss__1,    m4przsss,   "sspc.p1",      0x0000, 0x010000, CRC(a7519725) SHA1(cdab0ae00b865291ff7389122d174ef2e2676c6e), "Barcrest","Prize Spend Spend Spend (Barcrest) (MPU4) (SSP 0.5 C)" )


/*****************************************************************************************************************************************************************************
*
* Red Alert
* - no extender
*
*****************************************************************************************************************************************************************************/

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent, machine, inputs, init, name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAME(year, setname, parent, machine, inputs, mpu4mod4yam_machines_state, init, ROT0, company, title, GAME_FLAGS )

// (C)1991 BARCREST and  R2T 3.3
GAME_CUSTOM( 199?, m4ra,       0,    cheatchr_pal<mpu4_characteriser_pal::m683_characteriser_prot>(R4, RT1), mpu4, init_m4,  "r2tx.p1",      0x0000, 0x010000, CRC(7efffe3d) SHA1(5472bc76f4450726fc49fce281a6ec69693d0923), "Barcrest","Red Alert (Barcrest) (MPU4) (R2T 3.3, set 1)" )
GAME_CUSTOM( 199?, m4ra__a,    m4ra, cheatchr_pal<mpu4_characteriser_pal::m683_characteriser_prot>(R4, RT1), mpu4, init_m4,  "r2txr.p1",     0x0000, 0x010000, CRC(9ff95e34) SHA1(79d19602b88e1c9d23e910332a968e6b820a39f5), "Barcrest","Red Alert (Barcrest) (MPU4) (R2T 3.3, set 2)" )
GAME_CUSTOM( 199?, m4ra__b,    m4ra, cheatchr_pal<mpu4_characteriser_pal::m683_characteriser_prot>(R4, RT1), mpu4, init_m4,  "ra2s.p1",      0x0000, 0x010000, CRC(cd0fd068) SHA1(a347372f7f737ca87f44e692015338831465f123), "Barcrest","Red Alert (Barcrest) (MPU4) (R2T 3.3, set 3)" )
GAME_CUSTOM( 199?, m4ra__c,    m4ra, cheatchr_pal<mpu4_characteriser_pal::m683_characteriser_prot>(R4, RT1), mpu4, init_m4,  "ra2x.p1",      0x0000, 0x010000, CRC(8217e235) SHA1(e17483afea2a9d9e70e88687f899e1b98b982b63), "Barcrest","Red Alert (Barcrest) (MPU4) (R2T 3.3, set 4)" )
GAME_CUSTOM( 199?, m4ra__d,    m4ra, cheatchr_pal<mpu4_characteriser_pal::m683_characteriser_prot>(R4, RT1), mpu4, init_m4,  "ra2xa.p1",     0x0000, 0x010000, CRC(0e6b2123) SHA1(af7c5ddddbfffef6fa5746a7b7927845457d02f8), "Barcrest","Red Alert (Barcrest) (MPU4) (R2T 3.3, set 5)" )
GAME_CUSTOM( 199?, m4ra__e,    m4ra, cheatchr_pal<mpu4_characteriser_pal::m683_characteriser_prot>(R4, RT1), mpu4, init_m4,  "ra2xb.p1",     0x0000, 0x010000, CRC(97fe4933) SHA1(201860b64577828547adb8a216a6a205c4a4f34b), "Barcrest","Red Alert (Barcrest) (MPU4) (R2T 3.3, set 6)" )
GAME_CUSTOM( 199?, m4ra__f,    m4ra, cheatchr_pal<mpu4_characteriser_pal::m683_characteriser_prot>(R4, RT1), mpu4, init_m4,  "ra2xr.p1",     0x0000, 0x010000, CRC(12e8eb9b) SHA1(2bcd2c911626a2cb2419f9540649e99d7f335b3b), "Barcrest","Red Alert (Barcrest) (MPU4) (R2T 3.3, set 7)" )
// different protection, also reads from 811 at start, but check doesn't matter?
// (C)1991 BARCREST and R2T 3.1
GAME_CUSTOM( 199?, m4ra__q,    m4ra, bootleg_fixedret<0x11>(R4, RT1), mpu4, init_m4,  "reda_20_.8",   0x0000, 0x010000, CRC(915aff5b) SHA1(e8e58c263e2bdb64a80e9355ac5e114fff1d59f8), "bootleg","Red Alert (Barcrest) (bootleg) (MPU4) (R2T 3.1)" )

// This appears to be a very different type of game based on the lamping
// (C)1991 BARCREST and RAH 3.3
GAME_CUSTOM( 199?, m4ra__p,    m4ra, cheatchr_pal<mpu4_characteriser_pal::m683_characteriser_prot>(R4, RT1), mpu4, init_m4,  "rahx.p1",      0x0000, 0x010000, CRC(6887014e) SHA1(25e4c008588a219895c1b326314fd11e1f0ad35f), "Barcrest","Red Alert (Barcrest) (MPU4) (RAH 3.3)" )

// This also appears to be a very different type of game based on the lamping
// (C)1991 BARCREST and RA3 0.2
GAME_CUSTOM( 199?, m4ra__g,    m4ra, cheatchr_pal<mpu4_characteriser_pal::graff_characteriser_prot>(R4, RT1), mpu4, init_m4,  "ra3xad.p1",    0x0000, 0x010000, CRC(75957d43) SHA1(f7d00842b8390f5464733a6fe1d61d7431a16817), "Barcrest","Red Alert (Barcrest) (MPU4) (RA3 0.2, set 1)" )
GAME_CUSTOM( 199?, m4ra__h,    m4ra, cheatchr_pal<mpu4_characteriser_pal::graff_characteriser_prot>(R4, RT1), mpu4, init_m4,  "ra3xb.p1",     0x0000, 0x010000, CRC(f37e9bd5) SHA1(584a1f6f1bfb35de813466448e35fc1251fa90bc), "Barcrest","Red Alert (Barcrest) (MPU4) (RA3 0.2, set 2)" )
GAME_CUSTOM( 199?, m4ra__i,    m4ra, cheatchr_pal<mpu4_characteriser_pal::graff_characteriser_prot>(R4, RT1), mpu4, init_m4,  "ra3xbd.p1",    0x0000, 0x010000, CRC(43891009) SHA1(5d9ebe9d48a39f0a121ae7b832b277910bfd0ad6), "Barcrest","Red Alert (Barcrest) (MPU4) (RA3 0.2, set 3)" )
GAME_CUSTOM( 199?, m4ra__j,    m4ra, cheatchr_pal<mpu4_characteriser_pal::graff_characteriser_prot>(R4, RT1), mpu4, init_m4,  "ra3xd.p1",     0x0000, 0x010000, CRC(bc59a07a) SHA1(3a8fc99690759ea376660feaf65bfda5386dcf0d), "Barcrest","Red Alert (Barcrest) (MPU4) (RA3 0.2, set 4)" )
GAME_CUSTOM( 199?, m4ra__k,    m4ra, cheatchr_pal<mpu4_characteriser_pal::graff_characteriser_prot>(R4, RT1), mpu4, init_m4,  "ra3xdr.p1",    0x0000, 0x010000, CRC(036950ba) SHA1(f0a534352b41c2762330762c3c7024d9a6d49cd4), "Barcrest","Red Alert (Barcrest) (MPU4) (RA3 0.2, set 5)" )
GAME_CUSTOM( 199?, m4ra__l,    m4ra, cheatchr_pal<mpu4_characteriser_pal::graff_characteriser_prot>(R4, RT1), mpu4, init_m4,  "ra3xdy.p1",    0x0000, 0x010000, CRC(468508d4) SHA1(ba6db1e1f7bca13b9c40173fb68418f319e2a9d8), "Barcrest","Red Alert (Barcrest) (MPU4) (RA3 0.2, set 6)" )
GAME_CUSTOM( 199?, m4ra__m,    m4ra, cheatchr_pal<mpu4_characteriser_pal::graff_characteriser_prot>(R4, RT1), mpu4, init_m4,  "ra3xr.p1",     0x0000, 0x010000, CRC(1a2b813d) SHA1(5d3b5d4ab31dd1848b3d0b2a5ff5798cc01e0c6f), "Barcrest","Red Alert (Barcrest) (MPU4) (RA3 0.2, set 7)" )
GAME_CUSTOM( 199?, m4ra__n,    m4ra, cheatchr_pal<mpu4_characteriser_pal::graff_characteriser_prot>(R4, RT1), mpu4, init_m4,  "ra3xs.p1",     0x0000, 0x010000, CRC(a1ba9673) SHA1(7d5441522e8676805f7e75a3d445acae83d8a03b), "Barcrest","Red Alert (Barcrest) (MPU4) (RA3 0.2, set 8)" )
GAME_CUSTOM( 199?, m4ra__o,    m4ra, cheatchr_pal<mpu4_characteriser_pal::graff_characteriser_prot>(R4, RT1), mpu4, init_m4,  "ra3xy.p1",     0x0000, 0x010000, CRC(3e2287de) SHA1(ba0861a0bfb6eb76f9786c0a4c098db362117618), "Barcrest","Red Alert (Barcrest) (MPU4) (RA3 0.2, set 9)" )


/*****************************************************************************************************************************************************************************
*
* Say No More
* - no extender
*
*****************************************************************************************************************************************************************************/

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAME(year, setname, parent, cheatchr_pal<mpu4_characteriser_pal::saynomore_characteriser_prot>(R4, RT1), mpu4, mpu4mod4yam_machines_state, init_m4, ROT0, company, title, GAME_FLAGS )

// (C)1991 BARCREST and SNM 2.0
GAME_CUSTOM( 199?, m4sayno,     0,          "snms.p1",              0x0000, 0x010000, CRC(be1f2222) SHA1(7d8319796e1d45a3d0246bf13b6d818f20796db3), "Barcrest","Say No More (Barcrest) (MPU4) (SNM 2.0)" )
GAME_CUSTOM( 199?, m4sayno__d,  m4sayno,    "snmx.p1",              0x0000, 0x010000, CRC(61a78035) SHA1(1d6c553c60fee0b80e06f8421b8a3806d1f3a587), "Barcrest","Say No More (Barcrest) (MPU4) (SNM 2.0 X)" )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAME(year, setname, parent, bootleg_fixedret<0x08>(R4, RT1), mpu4, mpu4mod4yam_machines_state, init_m4, ROT0, company, title, GAME_FLAGS )

// no copyright string and SNM 0.5, different protection, hack?
GAME_CUSTOM( 199?, m4sayno__a,  m4sayno,    "snm 5p.bin",           0x0000, 0x010000, CRC(4fba5c0a) SHA1(85438c531d4122bc31f59127a577dc6d71a4ba9d), "hack?","Say No More (Barcrest) (MPU4) (SNM 0.5, hack, set 1)" )
GAME_CUSTOM( 199?, m4sayno__b,  m4sayno,    "say no more 425b.bin", 0x0000, 0x010000, CRC(2cf27394) SHA1(fb7688b7d9d2e68f0c84a57b66dd02dbbc6accc7), "hack?","Say No More (Barcrest) (MPU4) (SNM 0.5, hack, set 2)" )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAME(year, setname, parent, bootleg_fixedret<0xb0>(R4, RT1), mpu4, mpu4mod4yam_machines_state, init_m4, ROT0, company, title, GAME_FLAGS )

// no copyright string and SNM 0.4, different protection, hack?
GAME_CUSTOM( 199?, m4sayno__c,  m4sayno,    "snm 6.bin",            0x0000, 0x010000, CRC(0d14730b) SHA1(2a35d72bdcc9402b00153621ec852f902720c104), "hack?","Say No More (Barcrest) (MPU4) (SNM 0.4, hack)" )


/*****************************************************************************************************************************************************************************
*
* Ace Chase
* - no extender
*
*****************************************************************************************************************************************************************************/

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAME(year, setname, parent, cheatchr_pal<mpu4_characteriser_pal::rr6_characteriser_prot>(R4, RT1), mpu4, mpu4mod4yam_machines_state, init_m4, ROT0, company, title, GAME_FLAGS )

// (C)1995  B.W.B. and AE5 3.0
GAME_CUSTOM( 199?, m4acechs__a,    m4acechs,   "ae_05a__.3_1", 0x0000, 0x010000, CRC(900e1789) SHA1(dbb13f1728d8527a7de5d257c866732adb0a95b5), "BWB","Ace Chase (BWB) (MPU4) (AE5 3.0, set 1)" )
GAME_CUSTOM( 199?, m4acechs__r,    m4acechs,   "aei05___.3_1", 0x0000, 0x010000, CRC(bb84d01f) SHA1(f1653590e8cd642faf09a16c5c1b0a4b267d42e7), "BWB","Ace Chase (BWB) (MPU4) (AE5 3.0, set 2)" )
// (C)1995  B.W.B. and AE10 3.0
GAME_CUSTOM( 199?, m4acechs__f,    m4acechs,   "ae_10a__.3_1", 0x0000, 0x010000, CRC(e20c2513) SHA1(857ed8a6b155863c769ee9c3aca5e4702c1372b6), "BWB","Ace Chase (BWB) (MPU4) (AE10 3.0, set 1)" )
GAME_CUSTOM( 199?, m4acechs__t,    m4acechs,   "aei10___.3_1", 0x0000, 0x010000, CRC(db99a965) SHA1(1fb200b30e10d502af39bcd2e58d3e36e13f3695), "BWB","Ace Chase (BWB) (MPU4) (AE10 3.0, set 2)" )
// (C)1994  B.W.B. and AE  1.0
GAME_CUSTOM( 199?, m4acechs__g,    m4acechs,   "ae_10bg_.2_3", 0x0000, 0x010000, CRC(7ed7fcee) SHA1(7b2b0c47dc8a75d11f49f09441a4320815d838ac), "BWB","Ace Chase (BWB) (MPU4) (AE  1.0)" )
// (C)1995  B.W.B. and AE20 3.0
GAME_CUSTOM( 199?, m4acechs__k,    m4acechs,   "ae_20a__.3_1", 0x0000, 0x010000, CRC(43f6cc19) SHA1(3eda49477b141c649a4ba7a4ecc021694d9830db), "BWB","Ace Chase (BWB) (MPU4) (AE20 3.0, set 1)" )
GAME_CUSTOM( 199?, m4acechs__l,    m4acechs,   "ae_20b__.3_1", 0x0000, 0x010000, CRC(30060ac4) SHA1(488263a1d3cfe067d43de29c57e58fe55024437c), "BWB","Ace Chase (BWB) (MPU4) (AE20 3.0, set 2)" )
GAME_CUSTOM( 199?, m4acechs__m,    m4acechs,   "ae_20bd_.3_1", 0x0000, 0x010000, CRC(f9b922c2) SHA1(fc0deb79fc6c33732872da8925a6729f3d11bcaf), "BWB","Ace Chase (BWB) (MPU4) (AE20 3.0, set 3)" )
GAME_CUSTOM( 199?, m4acechs__n,    m4acechs,   "ae_20bg_.3_1", 0x0000, 0x010000, CRC(02706741) SHA1(8388d91091945d1f73aa5e68a86f930f5d9dafa2), "BWB","Ace Chase (BWB) (MPU4) (AE20 3.0, set 4)" )
GAME_CUSTOM( 199?, m4acechs__o,    m4acechs,   "ae_20bt_.3_1", 0x0000, 0x010000, CRC(3b313958) SHA1(9fe4cb99dc30d1305816f9a27079d97c4d07cb15), "BWB","Ace Chase (BWB) (MPU4) (AE20 3.0, set 5)" )
GAME_CUSTOM( 199?, m4acechs__p,    m4acechs,   "ae_20sb_.3_1", 0x0000, 0x010000, CRC(471f2ba4) SHA1(baaf8339d8ee15365886cea2ecb36ad298975633), "BWB","Ace Chase (BWB) (MPU4) (AE20 3.0, set 6)" )
GAME_CUSTOM( 199?, m4acechs__u,    m4acechs,   "aei20___.3_1", 0x0000, 0x010000, CRC(1744e7f4) SHA1(bf2f1b720a1a2610aff46a1de5c789a17828eae0), "BWB","Ace Chase (BWB) (MPU4) (AE20 3.0, set 7)" )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAME(year, setname, parent, cheatchr_pal<mpu4_characteriser_pal::acechasealt_characteriser_prot>(R4, RT1), mpu4, mpu4mod4yam_machines_state, init_m4, ROT0, company, title, GAME_FLAGS )

// (C)1994  B.W.B. and AE5 2.0
GAME_CUSTOM( 199?, m4acechs,       0,          "ae_05a__.2_3", 0x0000, 0x010000, CRC(c9a03623) SHA1(8daf7e71057528c481915eb8506e03ce9cf372c8), "BWB","Ace Chase (BWB) (MPU4) (AE5 2.0, set 1)" )
GAME_CUSTOM( 199?, m4acechs__b,    m4acechs,   "ae_05s__.2_3", 0x0000, 0x010000, CRC(eb64ab0a) SHA1(4d4c6908c8ca8b1d3c39c8973c8386da079cbd39), "BWB","Ace Chase (BWB) (MPU4) (AE5 2.0, set 2)" )
GAME_CUSTOM( 199?, m4acechs__c,    m4acechs,   "ae_05sb_.2_3", 0x0000, 0x010000, CRC(5d67c6f6) SHA1(213225405defb3be7f564459d71aeca6f5856f8f), "BWB","Ace Chase (BWB) (MPU4) (AE5 2.0, set 3)" )
GAME_CUSTOM( 199?, m4acechs__d,    m4acechs,   "ae_05sd_.2_3", 0x0000, 0x010000, CRC(2bdbe356) SHA1(a328a8f50847cbb199b31672ca50e1e95a474e4b), "BWB","Ace Chase (BWB) (MPU4) (AE5 2.0, set 4)" )
GAME_CUSTOM( 199?, m4acechs__q,    m4acechs,   "aei05___.2_3", 0x0000, 0x010000, CRC(f035ba55) SHA1(d13bebec00650018a9236cc18df73b06c970cfd0), "BWB","Ace Chase (BWB) (MPU4) (AE5 2.0, set 5)" )
// (C)1994  B.W.B. and AE10 2.0
GAME_CUSTOM( 199?, m4acechs__e,    m4acechs,   "ae_10a__.2_3", 0x0000, 0x010000, CRC(d718d498) SHA1(d13970b0ca86b988bcc91cd3c2dbee4c637944ca), "BWB","Ace Chase (BWB) (MPU4) (AE10 2.0, set 1)" )
GAME_CUSTOM( 199?, m4acechs__h,    m4acechs,   "ae_10s__.2_3", 0x0000, 0x010000, CRC(31932d3f) SHA1(a1809c7baaea22d24491829a8638f232e2d75849), "BWB","Ace Chase (BWB) (MPU4) (AE10 2.0, set 2)" )
GAME_CUSTOM( 199?, m4acechs__i,    m4acechs,   "ae_10sb_.2_3", 0x0000, 0x010000, CRC(d6bcd1fd) SHA1(664ec7e7821c09bddfd1996892ae3f9fbdbc6809), "BWB","Ace Chase (BWB) (MPU4) (AE10 2.0, set 3)" )
GAME_CUSTOM( 199?, m4acechs__j,    m4acechs,   "ae_10sd_.2_3", 0x0000, 0x010000, CRC(5920b9ad) SHA1(fb8de53e7877505fe53ff874b396707ee8e01e5e), "BWB","Ace Chase (BWB) (MPU4) (AE10 2.0, set 4)" )
GAME_CUSTOM( 199?, m4acechs__s,    m4acechs,   "aei10___.2_3", 0x0000, 0x010000, CRC(96edf44f) SHA1(8abcb5d4018e0a4c879eb1a1550af09f55f75135), "BWB","Ace Chase (BWB) (MPU4) (AE10 2.0, set 5)" )


/*****************************************************************************************************************************************************************************
*
* Super Streak
* - no extender
*
*****************************************************************************************************************************************************************************/

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent, machine, inputs, init, name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAME(year, setname, parent, machine, inputs, mpu4mod4yam_machines_state, init, ROT0, company, title, GAME_FLAGS )

// boot
GAME_CUSTOM( 199?, m4supst__au, m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "sp8b.p1",              0x0000, 0x010000, CRC(3b12d7e8) SHA1(92a15e5f8391d74c192e8386abdb8853a76bff05), "Barcrest","Super Streak (Barcrest) (MPU4) (SP8 0.1, set 1)" )
GAME_CUSTOM( 199?, m4supst__av, m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "sp8bd.p1",             0x0000, 0x010000, CRC(e0d7f789) SHA1(f6157469e43059adb44e7f2eff5bf73861d5636c), "Barcrest","Super Streak (Barcrest) (MPU4) (SP8 0.1, set 2)" )
GAME_CUSTOM( 199?, m4supst__aw, m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "sp8c.p1",              0x0000, 0x010000, CRC(da0af8ae) SHA1(91042506050967c508b30c3dc2bfa6f6a6e8b532), "Barcrest","Super Streak (Barcrest) (MPU4) (SP8 0.1, set 3)" )
GAME_CUSTOM( 199?, m4supst__ax, m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "sp8dk.p1",             0x0000, 0x010000, CRC(92432e8f) SHA1(5e6df963ccf92a89c71ae1edd7b71ec1e3f97522), "Barcrest","Super Streak (Barcrest) (MPU4) (SP8 0.1, set 4)" )
GAME_CUSTOM( 199?, m4supst__ay, m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "sp8k.p1",              0x0000, 0x010000, CRC(e39f74d8) SHA1(9d776e7d67859f4514c69fc4f9f43160da9a2ca1), "Barcrest","Super Streak (Barcrest) (MPU4) (SP8 0.1, set 5)" )
GAME_CUSTOM( 199?, m4supst__az, m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "sp8s.p1",              0x0000, 0x010000, CRC(fab99461) SHA1(82f8ca06bb04396f86124dfe4de46265b2edc393), "Barcrest","Super Streak (Barcrest) (MPU4) (SP8 0.1, set 6)" )

// boot
GAME_CUSTOM( 199?, m4supst__a0, m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "spsbd.p1",             0x0000, 0x010000, CRC(b621b32d) SHA1(9aab0e074c120cb12beac585f9c513053502955c), "Barcrest","Super Streak (Barcrest) (MPU4) (SPS 0.8, set 1)" )
GAME_CUSTOM( 199?, m4supst__a1, m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "spsc.p1",              0x0000, 0x010000, CRC(8c7a24f5) SHA1(f86be164e05235281fb275e950cedaf6f630d29a), "Barcrest","Super Streak (Barcrest) (MPU4) (SPS 0.8, set 2)" )
GAME_CUSTOM( 199?, m4supst__a2, m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "spsd.p1",              0x0000, 0x010000, CRC(d34d3617) SHA1(5373335557e4bbb21264bbd9d0fbaf3640f9ab35), "Barcrest","Super Streak (Barcrest) (MPU4) (SPS 0.8, set 3)" )
GAME_CUSTOM( 199?, m4supst__a3, m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "spsdk.p1",             0x0000, 0x010000, CRC(cf2fd3e7) SHA1(50d3c0851bec90037cd65a5c55654b0e688b96ca), "Barcrest","Super Streak (Barcrest) (MPU4) (SPS 0.8, set 4)" )
GAME_CUSTOM( 199?, m4supst__a4, m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "spsk.p1",              0x0000, 0x010000, CRC(873a1414) SHA1(47b2bbef168382112cd12ace2d6a58695f4b0254), "Barcrest","Super Streak (Barcrest) (MPU4) (SPS 0.8, set 5)" )
GAME_CUSTOM( 199?, m4supst__a5, m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "spss.p1",              0x0000, 0x010000, CRC(5e28bdb7) SHA1(3865c891178feb744ad11b2dea491350efc48bea), "Barcrest","Super Streak (Barcrest) (MPU4) (SPS 0.8, set 6)" )

// Hopper error
GAME_CUSTOM( 199?, m4supst,     0,       cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "cs4b.p1",              0x0000, 0x010000, CRC(fb0aac20) SHA1(3a40be78f7add7905afa8d1226ad41bf0041a2ec), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.7, set 1)" )
GAME_CUSTOM( 199?, m4supst__a,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "cs4ad.p1",             0x0000, 0x010000, CRC(c0e81dfd) SHA1(2da922df6c102f8d0f1678e974df9e4d356e5133), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.7, set 2)" )
GAME_CUSTOM( 199?, m4supst__b,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "cs4bd.p1",             0x0000, 0x010000, CRC(dafc7ed6) SHA1(3e92d5557d2f587132f4b3b633978ab7d4333fcc), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.7, set 3)" )
GAME_CUSTOM( 199?, m4supst__c,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "cs4d.p1",              0x0000, 0x010000, CRC(c1fcda65) SHA1(11f2a45f3f821eac6b98b1988824d77aada3d759), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.7, set 4)" )
GAME_CUSTOM( 199?, m4supst__d,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "cs4dk.p1",             0x0000, 0x010000, CRC(30a46171) SHA1(ef1f2951b478ba2b2d42dfb0ec4ed59f28d79972), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.7, set 5)" )
GAME_CUSTOM( 199?, m4supst__e,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "cs4dy.p1",             0x0000, 0x010000, CRC(72b15ce7) SHA1(c451ac552ffe9bcde1990b97a60b0ed8918bf8c8), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.7, set 6)" )
GAME_CUSTOM( 199?, m4supst__f,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "cs4k.p1",              0x0000, 0x010000, CRC(f252f9ea) SHA1(251998ea752deb4f4a05c833b19e89d334334fac), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.7, set 7)" )
GAME_CUSTOM( 199?, m4supst__g,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "cs4s.p1",              0x0000, 0x010000, CRC(10f7b88d) SHA1(0aac0ebbe0ce04db49fc7de4325eea9abdfd74b5), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.7, set 8)" )
GAME_CUSTOM( 199?, m4supst__h,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "cs4y.p1",              0x0000, 0x010000, CRC(a464d09d) SHA1(d38c0f8c7c9b7f560b685781a7dcf82bc031a191), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.7, set 9)" )

// Hopper error
// CS4 0.4 in header, CST 0.4 on boot
GAME_CUSTOM( 199?, m4supst__u,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "cst04ad.p1",           0x0000, 0x010000, CRC(b946d40d) SHA1(c03fa48f8b64c3cf4504f472f21a38f8a55f12e6), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.4 / CST 0.4, set 1)" )
GAME_CUSTOM( 199?, m4supst__v,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "cst04b.p1",            0x0000, 0x010000, CRC(45333d45) SHA1(d6ccb39ee9b316772052f856f79424c34ff273c5), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.4 / CST 0.4, set 2)" )
GAME_CUSTOM( 199?, m4supst__w,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "cst04bd.p1",           0x0000, 0x010000, CRC(03b56b07) SHA1(903b24ab93f9584f228278729b5a99451b8e81f7), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.4 / CST 0.4, set 3)" )
GAME_CUSTOM( 199?, m4supst__x,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "cst04c.p1",            0x0000, 0x010000, CRC(9c000883) SHA1(da0a9f1afc218c14a57a46fe2ea63e166f4e3739), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.4 / CST 0.4, set 4)" )
GAME_CUSTOM( 199?, m4supst__y,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "cst04d.p1",            0x0000, 0x010000, CRC(32281bec) SHA1(a043fb615c2a66d23d85ae80cb0b1705523f411c), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.4 / CST 0.4, set 5)" )
GAME_CUSTOM( 199?, m4supst__z,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "cst04dk.p1",           0x0000, 0x010000, CRC(9345e7b7) SHA1(8bff80d2b847fbae050f77215efe3e55b98a4657), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.4 / CST 0.4, set 6)" )
GAME_CUSTOM( 199?, m4supst__0,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "cst04dr.p1",           0x0000, 0x010000, CRC(8d397063) SHA1(45642de2629e89e2495d1cbd5aed90cf2a4cf1c1), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.4 / CST 0.4, set 7)" )
GAME_CUSTOM( 199?, m4supst__1,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "cst04dy.p1",           0x0000, 0x010000, CRC(4a303ced) SHA1(6c12b956358753c8bf99bd3316646721c9ec2585), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.4 / CST 0.4, set 8)" )
GAME_CUSTOM( 199?, m4supst__2,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "cst04k.p1",            0x0000, 0x010000, CRC(a59584f5) SHA1(8cfcf069ad905277f1925e682602e129e97e619b), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.4 / CST 0.4, set 9)" )
GAME_CUSTOM( 199?, m4supst__3,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "cst04r.p1",            0x0000, 0x010000, CRC(c9771997) SHA1(ed98650c0d73f2db0fe380777d10404ccabced31), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.4 / CST 0.4, set 10)" )
GAME_CUSTOM( 199?, m4supst__4,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "cst04s.p1",            0x0000, 0x010000, CRC(cd5b848d) SHA1(4dd3dd1c883552c7b5c475156308604b12eff75a), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.4 / CST 0.4, set 11)" )
GAME_CUSTOM( 199?, m4supst__5,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "cst04y.p1",            0x0000, 0x010000, CRC(7adc00ae) SHA1(5688f0876c18faf474a6d8487fdd85f20f9fc144), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.4 / CST 0.4, set 12)" )

// Hopper error
// CS4 0.3 in header, CSU 0.3 on boot
GAME_CUSTOM( 199?, m4supst__6,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "csu03ad.p1",           0x0000, 0x010000, CRC(5d7b6393) SHA1(19c24f4113efb6a1499936e5f89a8ad859ff8df0), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.3 / CSU 0.3, set 1)" )
GAME_CUSTOM( 199?, m4supst__7,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "csu03b.p1",            0x0000, 0x010000, CRC(57826c2a) SHA1(b835eb3066fec468ab55851d1dd023484e2d57e3), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.3 / CSU 0.3, set 2)" )
GAME_CUSTOM( 199?, m4supst__8,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "csu03bd.p1",           0x0000, 0x010000, CRC(092e7039) SHA1(36a7c18872e4012e3acce0d01d2cc2c201a3c867), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.3 / CSU 0.3, set 3)" )
GAME_CUSTOM( 199?, m4supst__9,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "csu03c.p1",            0x0000, 0x010000, CRC(b30a3c00) SHA1(066b0007092720a6f89edf8eafffe2f8fd83edbc), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.3 / CSU 0.3, set 4)" )
GAME_CUSTOM( 199?, m4supst__aa, m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "csu03d.p1",            0x0000, 0x010000, CRC(03ff9d99) SHA1(390087c136e4c314de9086adb7b020e8adabe34a), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.3 / CSU 0.3, set 5)" )
GAME_CUSTOM( 199?, m4supst__ab, m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "csu03dk.p1",           0x0000, 0x010000, CRC(cf7e61ff) SHA1(0e328ce5ff86770fabaf91d48a8de039323d112a), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.3 / CSU 0.3, set 6)" )
GAME_CUSTOM( 199?, m4supst__ac, m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "csu03dr.p1",           0x0000, 0x010000, CRC(00d700d1) SHA1(8bcc3c470c42780b1f1404fc6ff53e6ec7d89ad0), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.3 / CSU 0.3, set 7)" )
GAME_CUSTOM( 199?, m4supst__ad, m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "csu03dy.p1",           0x0000, 0x010000, CRC(8ec77c04) SHA1(64708460439a7e124f90eef6b9628e57f7d78ebc), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.3 / CSU 0.3, set 8)" )
GAME_CUSTOM( 199?, m4supst__ae, m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "csu03k.p1",            0x0000, 0x010000, CRC(701a0837) SHA1(31237fd108b354fb2afc449efa3a53dee2cf7be8), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.3 / CSU 0.3, set 9)" )
GAME_CUSTOM( 199?, m4supst__af, m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "csu03r.p1",            0x0000, 0x010000, CRC(d86a6895) SHA1(2c42bcf5de739f01e18bd1b766eec26a6da5aa52), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.3 / CSU 0.3, set 10)" )
GAME_CUSTOM( 199?, m4supst__ag, m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "csu03s.p1",            0x0000, 0x010000, CRC(197bb032) SHA1(06e98713ff5fc72bffccde1cc92fc8cb63665fad), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.3 / CSU 0.3, set 11)" )
GAME_CUSTOM( 199?, m4supst__ah, m4supst, cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "csu03y.p1",            0x0000, 0x010000, CRC(bee0e7e1) SHA1(6a1ab766af9147f0d4a7c1d2a95c9a6e3e3f4986), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.3 / CSU 0.3, set 12)" )

// different CHR - boots
GAME_CUSTOM( 199?, m4supst__bi, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4lv_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "sttad.p1",             0x0000, 0x010000, CRC(af615f05) SHA1(b2c1b8ba086a4d33f1269c28d4caa7286a27f085), "Barcrest","Super Streak (Barcrest) (MPU4) (STT 0.3, set 1)" )
GAME_CUSTOM( 199?, m4supst__bj, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4lv_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "sttb.p1",              0x0000, 0x010000, CRC(3119149f) SHA1(e749fcc5f95ccd29f42bfd0b140cf3cbb84d9599), "Barcrest","Super Streak (Barcrest) (MPU4) (STT 0.3, set 2)" )
GAME_CUSTOM( 199?, m4supst__bk, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4lv_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "sttbd.p1",             0x0000, 0x010000, CRC(cfddaf39) SHA1(0f24b5e691e1d43f6604087f0b3bc2571d2c4002), "Barcrest","Super Streak (Barcrest) (MPU4) (STT 0.3, set 3)" )
GAME_CUSTOM( 199?, m4supst__bl, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4lv_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "sttd.p1",              0x0000, 0x010000, CRC(8bc2498c) SHA1(a9cd3a6968186818a8c4033b1f304eac152244cf), "Barcrest","Super Streak (Barcrest) (MPU4) (STT 0.3, set 4)" )
GAME_CUSTOM( 199?, m4supst__bm, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4lv_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "sttdk.p1",             0x0000, 0x010000, CRC(39903dde) SHA1(f92c4380051ada7bbc5739550c8dfdd6ddaaa3fe), "Barcrest","Super Streak (Barcrest) (MPU4) (STT 0.3, set 5)" )
GAME_CUSTOM( 199?, m4supst__bn, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4lv_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "sttdr.p1",             0x0000, 0x010000, CRC(866f69f0) SHA1(ef9717f89b9718f1bcf8d3592f240ec9cf48bca3), "Barcrest","Super Streak (Barcrest) (MPU4) (STT 0.3, set 6)" )
GAME_CUSTOM( 199?, m4supst__bo, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4lv_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "sttdy.p1",             0x0000, 0x010000, CRC(74ebd933) SHA1(b308c8cae2c74e4e07c6e4afb505068220714824), "Barcrest","Super Streak (Barcrest) (MPU4) (STT 0.3, set 7)" )
GAME_CUSTOM( 199?, m4supst__bp, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4lv_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "sttk.p1",              0x0000, 0x010000, CRC(461db2f5) SHA1(8b97342d7ebfb33aa6aff246e8d799f4435788b7), "Barcrest","Super Streak (Barcrest) (MPU4) (STT 0.3, set 8)" )
GAME_CUSTOM( 199?, m4supst__bq, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4lv_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "sttr.p1",              0x0000, 0x010000, CRC(2591f6ec) SHA1(3d83d930e41e164e71d67b529967320e1eee8354), "Barcrest","Super Streak (Barcrest) (MPU4) (STT 0.3, set 9)" )
GAME_CUSTOM( 199?, m4supst__br, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4lv_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "stts.p1",              0x0000, 0x010000, CRC(a5e29c32) SHA1(8ba2f76505c2f40493c918b9d9524fa67999f7c1), "Barcrest","Super Streak (Barcrest) (MPU4) (STT 0.3, set 10)" )
GAME_CUSTOM( 199?, m4supst__bs, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4lv_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "stty.p1",              0x0000, 0x010000, CRC(7306fab9) SHA1(0da1612490fcff9b7a17f97190b6b561016c3b18), "Barcrest","Super Streak (Barcrest) (MPU4) (STT 0.3, set 11)" )
// different CHR - boots
GAME_CUSTOM( 199?, m4supst__bt, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4lv_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "stuad.p1",             0x0000, 0x010000, CRC(e7a01b7b) SHA1(3db08800a35d440f012ca69d84c30465818b4993), "Barcrest","Super Streak (Barcrest) (MPU4) (STU 0.1, set 1)" )
GAME_CUSTOM( 199?, m4supst__bu, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4lv_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "stub.p1",              0x0000, 0x010000, CRC(9044badf) SHA1(af8e218e3dc457bb5f24e3f2d74a8639466c3f11), "Barcrest","Super Streak (Barcrest) (MPU4) (STU 0.1, set 2)" )
GAME_CUSTOM( 199?, m4supst__bv, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4lv_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "stubd.p1",             0x0000, 0x010000, CRC(438e1687) SHA1(5e0f27e95bf861d4edc55709efc79496c7353e8b), "Barcrest","Super Streak (Barcrest) (MPU4) (STU 0.1, set 3)" )
GAME_CUSTOM( 199?, m4supst__bw, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4lv_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "stud.p1",              0x0000, 0x010000, CRC(1cbe3bec) SHA1(005dde84e57c5517fc6d6b975cc882dae11cbf63), "Barcrest","Super Streak (Barcrest) (MPU4) (STU 0.1, set 4)" )
GAME_CUSTOM( 199?, m4supst__bx, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4lv_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "studk.p1",             0x0000, 0x010000, CRC(0931d501) SHA1(afa078248230cbc0acc9d3af641ec63ed0424a75), "Barcrest","Super Streak (Barcrest) (MPU4) (STU 0.1, set 5)" )
GAME_CUSTOM( 199?, m4supst__by, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4lv_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "studr.p1",             0x0000, 0x010000, CRC(e06e1c59) SHA1(f4454f640335dbf6f9b8154d7805102253f605b4), "Barcrest","Super Streak (Barcrest) (MPU4) (STU 0.1, set 6)" )
GAME_CUSTOM( 199?, m4supst__bz, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4lv_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "study.p1",             0x0000, 0x010000, CRC(8b4275e0) SHA1(267a9d2eddf41b8838eeaee06bba45f0a8b8451f), "Barcrest","Super Streak (Barcrest) (MPU4) (STU 0.1, set 7)" )
GAME_CUSTOM( 199?, m4supst__b0, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4lv_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "stuk.p1",              0x0000, 0x010000, CRC(a66fb54f) SHA1(4351edbf6c5de817cf6972885ff1f6c7df837c37), "Barcrest","Super Streak (Barcrest) (MPU4) (STU 0.1, set 8)" )
GAME_CUSTOM( 199?, m4supst__b1, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4lv_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "stur.p1",              0x0000, 0x010000, CRC(eeb3bfed) SHA1(87a753511fb384a505d3cc69ca67fe4e288cf3bb), "Barcrest","Super Streak (Barcrest) (MPU4) (STU 0.1, set 9)" )
GAME_CUSTOM( 199?, m4supst__b2, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4lv_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "stus.p1",              0x0000, 0x010000, CRC(19aca6ad) SHA1(1583e76a4e1058fa97efdd9a7e6f7c4fe806b2f4), "Barcrest","Super Streak (Barcrest) (MPU4) (STU 0.1, set 10)" )
GAME_CUSTOM( 199?, m4supst__b3, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4lv_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "stuy.p1",              0x0000, 0x010000, CRC(e6b2b76f) SHA1(bf251b751e6a8d2764c63e92d48e1a64666b9a47), "Barcrest","Super Streak (Barcrest) (MPU4) (STU 0.1, set 11)" )
// different CHR - boots
GAME_CUSTOM( 199?, m4supst__b4, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4lv_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "superstreak1deb.bin",  0x0000, 0x010000, CRC(892ccad9) SHA1(c88daadd9778e363e154b674b57ccd07cea59836), "Barcrest","Super Streak (Barcrest) (MPU4) (STT 0.2K)" )

// different CHR - hopper
// CS4 0.2 in header, CSP 0.2 on boot
GAME_CUSTOM( 199?, m4supst__i,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "csp02ad.p1",           0x0000, 0x010000, CRC(96bbbc26) SHA1(ca127151c771963c07f0f368102ede8095d11863), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / CSP 0.2, set 1)" )
GAME_CUSTOM( 199?, m4supst__j,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "csp02b.p1",            0x0000, 0x010000, CRC(913ea9ff) SHA1(182bcc007d007a1c7f57767358600d2de7d1e3cf), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / CSP 0.2, set 2)" )
GAME_CUSTOM( 199?, m4supst__k,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "csp02bd.p1",           0x0000, 0x010000, CRC(ad0137a1) SHA1(d043372ba09081dd4e807f009a6460b4b30e6453), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / CSP 0.2, set 3)" )
GAME_CUSTOM( 199?, m4supst__l,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "csp02c.p1",            0x0000, 0x010000, CRC(fdad4b22) SHA1(4f19922821a9d1663bd9355447209384272e7542), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / CSP 0.2, set 4)" )
GAME_CUSTOM( 199?, m4supst__m,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "csp02d.p1",            0x0000, 0x010000, CRC(9717a58d) SHA1(8bc495dc4db0041718ae2db14a01a789616c8764), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / CSP 0.2, set 5)" )
GAME_CUSTOM( 199?, m4supst__n,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "csp02dk.p1",           0x0000, 0x010000, CRC(cd8aa547) SHA1(a13dcb75507878cb133b9ef739fb41d932d4eed5), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / CSP 0.2, set 6)" )
GAME_CUSTOM( 199?, m4supst__o,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "csp02dr.p1",           0x0000, 0x010000, CRC(6656e588) SHA1(4001ec0d1145ef0107e62ccda61e22ba8b0cdc92), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / CSP 0.2, set 7)" )
GAME_CUSTOM( 199?, m4supst__p,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "csp02dy.p1",           0x0000, 0x010000, CRC(14ff7e1d) SHA1(455b6ff93a5f25dc5f43c62a6c1d9a18de1ce94b), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / CSP 0.2, set 8)" )
GAME_CUSTOM( 199?, m4supst__q,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "csp02k.p1",            0x0000, 0x010000, CRC(c438c754) SHA1(c1d2e664091c1eaf1e4d964a3bfd446b11d7ba41), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / CSP 0.2, set 9)" )
GAME_CUSTOM( 199?, m4supst__r,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "csp02r.p1",            0x0000, 0x010000, CRC(4abe0f80) SHA1(67f7f9946a26b5097b6ce719dbd599790078f365), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / CSP 0.2, set 10)" )
GAME_CUSTOM( 199?, m4supst__s,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "csp02s.p1",            0x0000, 0x010000, CRC(47c0068d) SHA1(5480a519a6e6df2757e66cfcf904dd6c2873cc43), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / CSP 0.2, set 11)" )
GAME_CUSTOM( 199?, m4supst__t,  m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "csp02y.p1",            0x0000, 0x010000, CRC(d51d18d8) SHA1(a65fd4326872775364d2d7a886e98a1ee07596b7), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / CSP 0.2, set 12)" )
// different CHR - hopper
// CS4 0.2 in header, EEH 0.2 on boot
GAME_CUSTOM( 199?, m4supst__ai, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "eeh02ad.p1",           0x0000, 0x010000, CRC(25874a6d) SHA1(12e4fb36d231c3104df3613dd3851f411a876eb0), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / EEH 0.2, set 1)" )
GAME_CUSTOM( 199?, m4supst__aj, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "eeh02b.p1",            0x0000, 0x010000, CRC(ef280a8a) SHA1(912a825e69482a540cf0cadfc49a37a2822f3ecb), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / EEH 0.2, set 2)" )
GAME_CUSTOM( 199?, m4supst__ak, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "eeh02bd.p1",           0x0000, 0x010000, CRC(5f126810) SHA1(8fe1cbc7d93e2db35225388ee0773f6a98762ca1), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / EEH 0.2, set 3)" )
GAME_CUSTOM( 199?, m4supst__al, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "eeh02c.p1",            0x0000, 0x010000, CRC(3f49b936) SHA1(a0d07e0101f8cc38ebc28cfc1b239793b961f5ab), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / EEH 0.2, set 4)" )
GAME_CUSTOM( 199?, m4supst__am, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "eeh02d.p1",            0x0000, 0x010000, CRC(14dcfe63) SHA1(3ac77c9aa9b3b77fb1df98d2b427564be41dca78), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / EEH 0.2, set 5)" )
GAME_CUSTOM( 199?, m4supst__an, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "eeh02dk.p1",           0x0000, 0x010000, CRC(81a39421) SHA1(6fa43e8cb83e7fb940cc224eed5ee3f254c18c4d), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / EEH 0.2, set 6)" )
GAME_CUSTOM( 199?, m4supst__ao, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "eeh02dr.p1",           0x0000, 0x010000, CRC(c7755823) SHA1(05626ed49a2f800555f3f404273fa910b68de75c), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / EEH 0.2, set 7)" )
GAME_CUSTOM( 199?, m4supst__ap, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "eeh02dy.p1",           0x0000, 0x010000, CRC(5a1e70cd) SHA1(88bb29fd52d2331b72bb04652f9578f2c2f5a9ac), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / EEH 0.2, set 8)" )
GAME_CUSTOM( 199?, m4supst__aq, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "eeh02k.p1",            0x0000, 0x010000, CRC(b78882ec) SHA1(79c6a6d2cfe113743d3a93eb825fccab2b025933), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / EEH 0.2, set 9)" )
GAME_CUSTOM( 199?, m4supst__ar, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "eeh02r.p1",            0x0000, 0x010000, CRC(ff54884e) SHA1(2783f0e562e946597288ddbec4dcd1101e188d1d), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / EEH 0.2, set 10)" )
GAME_CUSTOM( 199?, m4supst__as, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "eeh02s.p1",            0x0000, 0x010000, CRC(c5856c3c) SHA1(5a0e5a7188913e1c36eac894bbeeae47a4f3589c), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / EEH 0.2, set 11)" )
GAME_CUSTOM( 199?, m4supst__at, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "eeh02y.p1",            0x0000, 0x010000, CRC(623fa0a0) SHA1(5a49cea5e94afccbf965cbda7a8d9a74f9734a6e), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / EEH 0.2, set 12)" )
// different CHR - hopper
// CS4 0.2 in header, STC 0.2 in boot
GAME_CUSTOM( 199?, m4supst__a6, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "stc02ad.p1",           0x0000, 0x010000, CRC(d9a2b4d1) SHA1(9a6862a44817b3ec465f126fd2a5d2c9825d846e), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / STC 0.2, set 1)" )
GAME_CUSTOM( 199?, m4supst__a7, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "stc02b.p1",            0x0000, 0x010000, CRC(bd2e8e6c) SHA1(71670dccedc2f47888c1205de59a81677ffeabaa), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / STC 0.2, set 2)" )
GAME_CUSTOM( 199?, m4supst__a8, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "stc02bd.p1",           0x0000, 0x010000, CRC(efbed99b) SHA1(62d80248bb666bfb49ed7546936da744e43fa870), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / STC 0.2, set 3)" )
GAME_CUSTOM( 199?, m4supst__a9, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "stc02c.p1",            0x0000, 0x010000, CRC(9d342386) SHA1(b50f64d66d89dbd3dee1ff2cb430a2caa050e7c8), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / STC 0.2, set 4)" )
GAME_CUSTOM( 199?, m4supst__ba, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "stc02d.p1",            0x0000, 0x010000, CRC(c43f6e65) SHA1(0278cf389f8289d7b819125ae0a612c81ea75fab), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / STC 0.2, set 5)" )
GAME_CUSTOM( 199?, m4supst__bb, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "stc02dk.p1",           0x0000, 0x010000, CRC(36576570) SHA1(214a57344d8e161b3dbd07457291ed9bce011842), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / STC 0.2, set 6)" )
GAME_CUSTOM( 199?, m4supst__bc, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "stc02dr.p1",           0x0000, 0x010000, CRC(450c553f) SHA1(46050285eeb10dc368ad501c61d41351c4e2fcde), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / STC 0.2, set 7)" )
GAME_CUSTOM( 199?, m4supst__bd, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "stc02dy.p1",           0x0000, 0x010000, CRC(d8677dd1) SHA1(18abc0a1d28458c3b26a0d1dbf6ca8aba3f3e240), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / STC 0.2, set 8)" )
GAME_CUSTOM( 199?, m4supst__be, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "stc02k.p1",            0x0000, 0x010000, CRC(c6e8d110) SHA1(9e05961b9bba502f52a03de27e608afc52f6c025), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / STC 0.2, set 9)" )
GAME_CUSTOM( 199?, m4supst__bf, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "stc02r.p1",            0x0000, 0x010000, CRC(918d769f) SHA1(2a4438828d9e7efd3a94eaebe56585e7ae23d9d1), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / STC 0.2, set 10)" )
GAME_CUSTOM( 199?, m4supst__bg, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "stc02s.p1",            0x0000, 0x010000, CRC(9c50fff7) SHA1(3468340d2d04cbdecd669817f8a9c4028e301eeb), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / STC 0.2, set 11)" )
GAME_CUSTOM( 199?, m4supst__bh, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "stc02y.p1",            0x0000, 0x010000, CRC(0ce65e71) SHA1(02ae1fd5a41ab5a96ddcfe1cf3e8567561291961), "Barcrest","Super Streak (Barcrest) (MPU4) (CS4 0.2 / STC 0.2, set 12)" )

GAME_CUSTOM( 199?, m4stc, m4supst, cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,  init_m4,   "stc01s", 0x0000, 0x010000, CRC(8371bb8f) SHA1(bd60825b3f5011c218b34f00886b6b54afe61b9f), "Barcrest","Super Streak (Barcrest) (MPU4) (STC 0.1)" )

// different protection
// was in SC2 Super Star set, but seems to fit here, ident hacked to "BILL    BIXBY" and "V1   0.1"
GAME_CUSTOM( 199?, m4supst__b6,    m4supst, bootleg_fixedret<0x46>(R4, RT1), mpu4,  init_m4,  "supst20.15",   0x0000, 0x010000, CRC(c3446ec4) SHA1(3c1ad27385547a33993a839b53873d8b92214ade), "hack","Super Streak (Barcrest) (MPU4) (hack)" )

// different protection style
	GAME_CUSTOM( 199?, m4supst__b5, m4supst, bootleg_fixedret<0x52>(R4, RT1), mpu4,  init_m4,   "supst2515",            0x0000, 0x010000, CRC(c073a249) SHA1(4ae37eb61dd5e50687f433fb89f65b97926b7358), "hack","Super Streak (Barcrest) (MPU4) (STT 0.3, hack)" )

// different protection

// "(C)1998  B.W.B." and "SS2 1.0"
GAME_CUSTOM( 199?, m4supst__b7,  m4supst, bootleg_fixedret<0x74>(R4, RT1), mpu4,  init_m4, "rhr2pprg.bin",     0x0000, 0x010000, CRC(f97047b2) SHA1(d3ed8c93e405f9e7448b3924ff9aa84223b76046), "hack","Super Streak (Barcrest) (MPU4) (SS2 1.0, hack?)" )

ROM_START( m4sstrek )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rhr2pprgpatched.bin", 0x0000, 0x010000, CRC(a0b3439d) SHA1(0976537a5170bf4c4f595f7fa04243a68f14b2ae) )
ROM_END
// no sequence
GAME(199?, m4sstrek,  m4supst,  base(R4, RT1),          mpu4, mpu4mod4yam_machines_state, init_m4,  ROT0,   "bootleg","Super Streak (bootleg) (MPU4) (SS2 1.0)",GAME_FLAGS) // unprotected, no characteriser PAL required


/*****************************************************************************************************************************************************************************
*
* Fast Forward
* - no extender
*
*****************************************************************************************************************************************************************************/

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAME(year, setname, parent, cheatchr_pal<mpu4_characteriser_pal::viva_characteriser_prot>(R4, RT1), mpu4, mpu4mod4yam_machines_state, init_m4, ROT0, company, title, GAME_FLAGS )

GAME_CUSTOM( 199?, m4fastfw,       0,          "ffo05__1.0",   0x0000, 0x010000, CRC(8b683969) SHA1(7469b551e4d6f65550d54ee39b2bac07cf3dbd4b), "Bwb / Barcrest","Fast Forward (Barcrest) (MPU4) (SFF 3.0, set 1)" )
GAME_CUSTOM( 199?, m4fastfw__a,    m4fastfw,   "ffo10__1.0",   0x0000, 0x010000, CRC(294288fd) SHA1(87d25f6333b6862fcc57a550b5cc7c0bc64e72cd), "Bwb / Barcrest","Fast Forward (Barcrest) (MPU4) (SFF 3.0, set 2)" )
GAME_CUSTOM( 199?, m4fastfw__b,    m4fastfw,   "ffo10d_1.0",   0x0000, 0x010000, CRC(8d96f3d4) SHA1(2070a335cfa3f9de1bd9e9094d91cce81b91347d), "Bwb / Barcrest","Fast Forward (Barcrest) (MPU4) (SFF 3.0, set 3)" )
GAME_CUSTOM( 199?, m4fastfw__c,    m4fastfw,   "ffo20__1.0",   0x0000, 0x010000, CRC(9528291e) SHA1(61c0eb8ce955f708e8a68a28f253706267e28254), "Bwb / Barcrest","Fast Forward (Barcrest) (MPU4) (SFF 3.0, set 4)" )
GAME_CUSTOM( 199?, m4fastfw__d,    m4fastfw,   "ffo20d_1.0",   0x0000, 0x010000, CRC(5bae35fe) SHA1(7e4d61ed97ddd170bd1424f34d0327093668da3f), "Bwb / Barcrest","Fast Forward (Barcrest) (MPU4) (SFF 3.0, set 5)" )
GAME_CUSTOM( 199?, m4fastfw__e,    m4fastfw,   "ffo20dy1.0",   0x0000, 0x010000, CRC(37167d46) SHA1(94b87697615f81b746ce3bcc64fc893f865e00dc), "Bwb / Barcrest","Fast Forward (Barcrest) (MPU4) (SFF 3.0, set 6)" )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAME(year, setname, parent, bootleg_fixedret<0x80>(R4, RT1), mpu4, mpu4mod4yam_machines_state, init_m4, ROT0, company, title, GAME_FLAGS )


// different protection style, has (C)1993 BYTEFREE, reads from 811 on startup, and 821 later, but only check on 806 matters?
GAME_CUSTOM( 199?, m4fastfw__f,    m4fastfw,   "fastf206",     0x0000, 0x010000, CRC(a830b121) SHA1(0bf813ee75bd8e109e6688b91bd0983d341a6695), "hack","Fast Forward (Barcrest) (MPU4) (FFD 1.0, hack)" )


/*****************************************************************************************************************************************************************************
*
* Viva Las Vegas
* - small extender
*
*****************************************************************************************************************************************************************************/

// This ROM was included in the sets, but it appears to be a 68k family (not plain 68000) program ROM, and has a (c)1998 MAB Systems in it, it does not belong here
//  ROM_LOAD( "vivalasvegas4.bin", 0x0000, 0x080000, CRC(76971425) SHA1(0974a9dce51cc3dd4e26cec11a948c9c8021fde4) )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent, machine, inputs, init, name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAME(year, setname, parent, machine, inputs, mpu4mod4yam_machines_state, init, ROT0, company, title, GAME_FLAGS )

// "(C)1991 BARCREST" and "VLV 1.1"
GAME_CUSTOM( 199?, m4vivalv,       0,        cheatchr_pal<mpu4_characteriser_pal::vivlv_characteriser_prot>(R4, RT1, LPS), mpu4, init_m4,  "vlvs.p1",                      0x0000, 0x010000, CRC(b7fb3e19) SHA1(c6cc4175f8c100fc37e6e7014b0744054b4e547a), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (VLV 1.1, set 7)" )
GAME_CUSTOM( 199?, m4vivalv__a,    m4vivalv, cheatchr_pal<mpu4_characteriser_pal::vivlv_characteriser_prot>(R4, RT1, LPS), mpu4, init_m4,  "vlvad.p1",                     0x0000, 0x010000, CRC(88262812) SHA1(f0a31d510c1b06af122df493585c04a49177f06d), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (VLV 1.1, set 1)" )
GAME_CUSTOM( 199?, m4vivalv__b,    m4vivalv, cheatchr_pal<mpu4_characteriser_pal::vivlv_characteriser_prot>(R4, RT1, LPS), mpu4, init_m4,  "vlvb.p1",                      0x0000, 0x010000, CRC(c4caec15) SHA1(d88c6e081a6bbdd80f773713b038293cabdeee8c), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (VLV 1.1, set 2)" )
GAME_CUSTOM( 199?, m4vivalv__c,    m4vivalv, cheatchr_pal<mpu4_characteriser_pal::vivlv_characteriser_prot>(R4, RT1, LPS), mpu4, init_m4,  "vlvc.p1",                      0x0000, 0x010000, CRC(4d651ba4) SHA1(7746656f0a9f8af8e265568f7479edef9a2247d9), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (VLV 1.1, set 3)" )
GAME_CUSTOM( 199?, m4vivalv__d,    m4vivalv, cheatchr_pal<mpu4_characteriser_pal::vivlv_characteriser_prot>(R4, RT1, LPS), mpu4, init_m4,  "vlvd.p1",                      0x0000, 0x010000, CRC(cce926c7) SHA1(8e3a0cef0cbee66d264da5d6dfc7ec2fbdcd9584), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (VLV 1.1, set 4)" )
GAME_CUSTOM( 199?, m4vivalv__e,    m4vivalv, cheatchr_pal<mpu4_characteriser_pal::vivlv_characteriser_prot>(R4, RT1, LPS), mpu4, init_m4,  "vlvdy.p1",                     0x0000, 0x010000, CRC(6e17cbc8) SHA1(5c69eda0ff6a01d9d0d434ff7ce1ac1e67b16362), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (VLV 1.1, set 5)" )
GAME_CUSTOM( 199?, m4vivalv__f,    m4vivalv, cheatchr_pal<mpu4_characteriser_pal::vivlv_characteriser_prot>(R4, RT1, LPS), mpu4, init_m4,  "vlvk.p1",                      0x0000, 0x010000, CRC(b5f2157e) SHA1(574f3e2890ac5479790ea92760c6500d37e6637d), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (VLV 1.1, set 6)" )
GAME_CUSTOM( 199?, m4vivalv__g,    m4vivalv, cheatchr_pal<mpu4_characteriser_pal::vivlv_characteriser_prot>(R4, RT1, LPS), mpu4, init_m4,  "vlvy.p1",                      0x0000, 0x010000, CRC(3211caf3) SHA1(3634ef11099c2f4938529bb262cc2556ad96a675), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (VLV 1.1, set 8)" )
// "(C)1991 BARCREST" and "VLV 1.0"
GAME_CUSTOM( 199?, m4vivalv__h,    m4vivalv, cheatchr_pal<mpu4_characteriser_pal::vivlv_characteriser_prot>(R4, RT1, LPS), mpu4, init_m4,  "viva206",                      0x0000, 0x010000, CRC(76ab9a5d) SHA1(455699cbc05f744eafe58881a8fb120b24cfe5c8), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (VLV 1.0)" )
// (C)1993  B.W.B. and "VL_ 2.0" - boots with cheatchr
GAME_CUSTOM( 199?, m4vivalv__i,    m4vivalv, cheatchr_pal<mpu4_characteriser_pal::tentendia_characteriser_prot>(R4, RT1, LPS), mpu4, init_m4,  "v.las vegas 6 10p 10m.bin",    0x0000, 0x010000, CRC(f09d5a2c) SHA1(6f9df58767e88a1ca7fc7dd17c618d30ab97067d), "BWB","Viva Las Vegas (BWB) (MPU4) (VL_ 2.0, set 1)" )
GAME_CUSTOM( 199?, m4vivalv__j,    m4vivalv, cheatchr_pal<mpu4_characteriser_pal::tentendia_characteriser_prot>(R4, RT1, LPS), mpu4, init_m4,  "vv_05___.3_3",                 0x0000, 0x010000, CRC(bb8361f6) SHA1(d5f651a66be1cab3662798751a290a65c29bba64), "BWB","Viva Las Vegas (BWB) (MPU4) (VL_ 2.0, set 2)" )
GAME_CUSTOM( 199?, m4vivalv__k,    m4vivalv, cheatchr_pal<mpu4_characteriser_pal::tentendia_characteriser_prot>(R4, RT1, LPS), mpu4, init_m4,  "vv_05_b_.3_3",                 0x0000, 0x010000, CRC(12079321) SHA1(5b5dd55080c04393a45d3ef9c63b6fef5de9b7cd), "BWB","Viva Las Vegas (BWB) (MPU4) (VL_ 2.0, set 3)" )
GAME_CUSTOM( 199?, m4vivalv__l,    m4vivalv, cheatchr_pal<mpu4_characteriser_pal::tentendia_characteriser_prot>(R4, RT1, LPS), mpu4, init_m4,  "vv_05_d_.3_3",                 0x0000, 0x010000, CRC(b758df52) SHA1(f4d47a93fa1b1deb84654bb2272767093f3463c2), "BWB","Viva Las Vegas (BWB) (MPU4) (VL_ 2.0, set 4)" )
GAME_CUSTOM( 199?, m4vivalv__m,    m4vivalv, cheatchr_pal<mpu4_characteriser_pal::tentendia_characteriser_prot>(R4, RT1, LPS), mpu4, init_m4,  "vv_05_k_.3_3",                 0x0000, 0x010000, CRC(9875c59c) SHA1(c31a7fc5df8af9d931353bc095a59befe808434b), "BWB","Viva Las Vegas (BWB) (MPU4) (VL_ 2.0, set 5)" )
GAME_CUSTOM( 199?, m4vivalv__n,    m4vivalv, cheatchr_pal<mpu4_characteriser_pal::tentendia_characteriser_prot>(R4, RT1, LPS), mpu4, init_m4,  "vv_05a__.3_3",                 0x0000, 0x010000, CRC(0f416e47) SHA1(54338fbef5f227c440c04448b51e8f0ec04a4cc7), "BWB","Viva Las Vegas (BWB) (MPU4) (VL_ 2.0, set 6)" )
GAME_CUSTOM( 199?, m4vivalv__o,    m4vivalv, cheatchr_pal<mpu4_characteriser_pal::tentendia_characteriser_prot>(R4, RT1, LPS), mpu4, init_m4,  "vv_10___.3_3",                 0x0000, 0x010000, CRC(dc8db002) SHA1(305547b4f0b1e1bde9354e5ed9f18f99c6829cab), "BWB","Viva Las Vegas (BWB) (MPU4) (VL_ 2.0, set 7)" )
GAME_CUSTOM( 199?, m4vivalv__p,    m4vivalv, cheatchr_pal<mpu4_characteriser_pal::tentendia_characteriser_prot>(R4, RT1, LPS), mpu4, init_m4,  "vv_10_b_.3_3",                 0x0000, 0x010000, CRC(e1c4b292) SHA1(4516c7d918935862824e206626a5a24f936ec514), "BWB","Viva Las Vegas (BWB) (MPU4) (VL_ 2.0, set 8)" )
GAME_CUSTOM( 199?, m4vivalv__q,    m4vivalv, cheatchr_pal<mpu4_characteriser_pal::tentendia_characteriser_prot>(R4, RT1, LPS), mpu4, init_m4,  "vv_10_d_.3_3",                 0x0000, 0x010000, CRC(e9dda1ee) SHA1(6363b5b26be22cb1f5aac71e98c5e5a5064839f4), "BWB","Viva Las Vegas (BWB) (MPU4) (VL_ 2.0, set 9)" )
GAME_CUSTOM( 199?, m4vivalv__r,    m4vivalv, cheatchr_pal<mpu4_characteriser_pal::tentendia_characteriser_prot>(R4, RT1, LPS), mpu4, init_m4,  "vv_10_k_.3_3",                 0x0000, 0x010000, CRC(70fc4c56) SHA1(02cbaadd3575ef0d9dc192aabbe39a735893a662), "BWB","Viva Las Vegas (BWB) (MPU4) (VL_ 2.0, set 10)" )
GAME_CUSTOM( 199?, m4vivalv__s,    m4vivalv, cheatchr_pal<mpu4_characteriser_pal::tentendia_characteriser_prot>(R4, RT1, LPS), mpu4, init_m4,  "vv_10a__.3_3",                 0x0000, 0x010000, CRC(c908d65a) SHA1(5af180e697c22c27380e275d76708103e298cf41), "BWB","Viva Las Vegas (BWB) (MPU4) (VL_ 2.0, set 11)" )
GAME_CUSTOM( 199?, m4vivalv__t,    m4vivalv, cheatchr_pal<mpu4_characteriser_pal::tentendia_characteriser_prot>(R4, RT1, LPS), mpu4, init_m4,  "vvi05___.3_3",                 0x0000, 0x010000, CRC(a5829d5c) SHA1(4cd1a2185579898db7be75f8c3f565043f0691b6), "BWB","Viva Las Vegas (BWB) (MPU4) (VL_ 2.0, set 12)" )

// "(C)1991 BARCREST" and "VLV 1.0" but different protection, hacks / bootlegs
// this one is unusual, the 2nd and 3rd checks expect different values, or you get scrambled lamps, usually they want the same
GAME_CUSTOM( 199?, m4vivalv__u,    m4vivalv, bootleg_fixedret<0x5a>(R4, RT1, LPS), mpu4, init_m4_806prot,  "viva20_1.1",                   0x0000, 0x010000, CRC(80ea2429) SHA1(e5d258967340fe85dd5baf6ba16f82ce83307b68), "hack?","Viva Las Vegas (Barcrest) (MPU4) (VLV 1.0, hack?, set 1)" )
// these expect the same for each test
GAME_CUSTOM( 199?, m4vivalv__w,    m4vivalv, bootleg_fixedret<0x6a>(R4, RT1, LPS), mpu4, init_m4,  "viva_20_.4",                   0x0000, 0x010000, CRC(e1efc846) SHA1(a4bf7f5c4febe5a71a09e23876387328e1bba87b), "hack?","Viva Las Vegas (Barcrest) (MPU4) (VLV 1.0, hack?, set 3)" )
GAME_CUSTOM( 199?, m4vivalv__x,    m4vivalv, bootleg_fixedret<0x6a>(R4, RT1, LPS), mpu4, init_m4,  "viva_20_.8",                   0x0000, 0x010000, CRC(f538a1fc) SHA1(d0dbd22a1cb4b7ec5bfa304ba544806e01150662), "hack?","Viva Las Vegas (Barcrest) (MPU4) (VLV 1.0, hack?, set 4)" )
GAME_CUSTOM( 199?, m4vivalv__y,    m4vivalv, bootleg_fixedret<0x6a>(R4, RT1, LPS), mpu4, init_m4,  "vlv208ac",                     0x0000, 0x010000, CRC(416535ee) SHA1(f2b0177fecd5076d9d89c819fe9402fc944c8d77), "hack?","Viva Las Vegas (Barcrest) (MPU4) (VLV 1.0, hack?, set 5)" )
GAME_CUSTOM( 199?, m4vivalv__v,    m4vivalv, bootleg_fixedret<0x5a>(R4, RT1, LPS), mpu4, init_m4,  "viva20_11",                    0x0000, 0x010000, CRC(51b93018) SHA1(fc13179e3e1939839c3b90d7600a7eb301ea03da), "hack?","Viva Las Vegas (Barcrest) (MPU4) (VLV 1.0, hack?, set 2)" )

// no copyright string, "VL_ 2.0", hack
GAME_CUSTOM( 199?, m4vivalv__z,    m4vivalv, bootleg_fixedret<0x38>(R4, RT1, LPS), mpu4, init_m4,  "5p5vivalasvegas6.bin",         0x0000, 0x010000, CRC(4d365b57) SHA1(69ff75ccc91f1f7b867a0914d350d1649834a48e), "hack?","Viva Las Vegas (BWB) (MPU4) (VL_ 2.0, hack?, set 1)" )
GAME_CUSTOM( 199?, m4vivalv__0,    m4vivalv, bootleg_fixedret<0x5c>(R4, RT1, LPS), mpu4, init_m4,  "viva05_11",                    0x0000, 0x010000, CRC(1e6ea483) SHA1(e6a53eb1bf3b8e661287c0d57fc6ab5ed41755a3), "hack?","Viva Las Vegas (BWB) (MPU4) (VL_ 2.0, hack?, set 2)" )
GAME_CUSTOM( 199?, m4vivalv__1,    m4vivalv, bootleg_fixedret<0x12>(R4, RT1, LPS), mpu4, init_m4,  "viva10_11",                    0x0000, 0x010000, CRC(246a39b7) SHA1(c0f5c21374e43b42df5df0ada0967a34ecefbdb4), "hack?","Viva Las Vegas (BWB) (MPU4) (VL_ 2.0, hack?, set 3)" )
GAME_CUSTOM( 199?, m4vivalv__2,    m4vivalv, bootleg_fixedret<0x28>(R4, RT1, LPS), mpu4, init_m4,  "viva58c",                      0x0000, 0x010000, CRC(719d0802) SHA1(ba6bd5fbf49f0ada383cb2e8faa037b78f6af587), "hack?","Viva Las Vegas (BWB) (MPU4) (VL_ 2.0, hack?, set 4)" )
GAME_CUSTOM( 199?, m4vivalv__3,    m4vivalv, bootleg_fixedret<0x28>(R4, RT1, LPS), mpu4, init_m4,  "viva_05_.4",                   0x0000, 0x010000, CRC(b094914f) SHA1(8217b4bb7a8d55fb8e86018ffc520a63f41a79b8), "hack?","Viva Las Vegas (BWB) (MPU4) (VL_ 2.0, hack?, set 5)" )
GAME_CUSTOM( 199?, m4vivalv__4,    m4vivalv, bootleg_fixedret<0x28>(R4, RT1, LPS), mpu4, init_m4,  "viva_05_.8",                   0x0000, 0x010000, CRC(c5c09c10) SHA1(47890d0ba1c2ca53231ac148a02f046452dce1b4), "hack?","Viva Las Vegas (BWB) (MPU4) (VL_ 2.0, hack?, set 6)" )
GAME_CUSTOM( 199?, m4vivalv__5,    m4vivalv, bootleg_fixedret<0x66>(R4, RT1, LPS), mpu4, init_m4,  "viva_10_.4",                   0x0000, 0x010000, CRC(b1d5e820) SHA1(68012216d7e82168c7468d1e54c527c15d268917), "hack?","Viva Las Vegas (BWB) (MPU4) (VL_ 2.0, hack?, set 7)" )
GAME_CUSTOM( 199?, m4vivalv__6,    m4vivalv, bootleg_fixedret<0x66>(R4, RT1, LPS), mpu4, init_m4,  "viva_10_.8",                   0x0000, 0x010000, CRC(f392c81c) SHA1(cb3320b688b315dbc226f45b78490fed439ee9a2), "hack?","Viva Las Vegas (BWB) (MPU4) (VL_ 2.0, hack ? , set 8)" )



/*****************************************************************************************************************************************************************************
*
* Super Hyper Viper
* - no extender
*
*****************************************************************************************************************************************************************************/

// These were mixed in Hyper Viper and Super Hyper Viper sets
// Super Hyper Viper is the MPU4 version, while the original game ran on MPU3?

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent, machine, inputs, init, name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAME(year, setname, parent, machine, inputs, mpu4mod4yam_machines_state, init, ROT0, company, title, GAME_FLAGS )

// "(C)1993  B.W.B." and "HVP 3.0"
GAME_CUSTOM( 199?, m4shv__h,    m4shv, cheatchr_pal<mpu4_characteriser_pal::m407_characteriser_prot>(R4, RT1), mpu4,  init_m4,  "hvyp10p",              0x0000, 0x010000, CRC(b4af635a) SHA1(420cdf3a6899e432d74e3b10a57414cbedc0913e), "BWB","Super Hyper Viper (Barcrest) (MPU4) (HVP 3.0, set 1)" )
GAME_CUSTOM( 199?, m4shv__i,    m4shv, cheatchr_pal<mpu4_characteriser_pal::m407_characteriser_prot>(R4, RT1), mpu4,  init_m4,  "hv_05___.3h3",         0x0000, 0x010000, CRC(13bfa891) SHA1(ffddd14a019d52029bf8d4f680d8d05413a9f0b7), "BWB","Super Hyper Viper (Barcrest) (MPU4) (HVP 3.0, set 2)" )
GAME_CUSTOM( 199?, m4shv__j,    m4shv, cheatchr_pal<mpu4_characteriser_pal::m407_characteriser_prot>(R4, RT1), mpu4,  init_m4,  "hv_05_d_.3h3",         0x0000, 0x010000, CRC(50c66ce8) SHA1(ef12525fc3ac82caf80326edaac81bb9fbc3245c), "BWB","Super Hyper Viper (Barcrest) (MPU4) (HVP 3.0, set 3)" )
GAME_CUSTOM( 199?, m4shv__k,    m4shv, cheatchr_pal<mpu4_characteriser_pal::m407_characteriser_prot>(R4, RT1), mpu4,  init_m4,  "hv_10___.3h3",         0x0000, 0x010000, CRC(627caac7) SHA1(4851ce2441850743ea68ecbf89bde3f4cd6c2b4c), "BWB","Super Hyper Viper (Barcrest) (MPU4) (HVP 3.0, set 4)" )
GAME_CUSTOM( 199?, m4shv__l,    m4shv, cheatchr_pal<mpu4_characteriser_pal::m407_characteriser_prot>(R4, RT1), mpu4,  init_m4,  "hv_10_d_.3h3",         0x0000, 0x010000, CRC(15cfa26e) SHA1(6bc3feaba65d1797b9945f23a89e983f56b13f79), "BWB","Super Hyper Viper (Barcrest) (MPU4) (HVP 3.0, set 5)" )
GAME_CUSTOM( 199?, m4shv__m,    m4shv, cheatchr_pal<mpu4_characteriser_pal::m407_characteriser_prot>(R4, RT1), mpu4,  init_m4,  "hv_10_d_.3n3",         0x0000, 0x010000, CRC(b81f1d0a) SHA1(5fd293be2b75393069c9f5e099b4700ff930f081), "BWB","Super Hyper Viper (Barcrest) (MPU4) (HVP 3.0, set 6)" )
GAME_CUSTOM( 199?, m4shv__n,    m4shv, cheatchr_pal<mpu4_characteriser_pal::m407_characteriser_prot>(R4, RT1), mpu4,  init_m4,  "hvi05___.3h3",         0x0000, 0x010000, CRC(6959332e) SHA1(edaa5f86ad4389b0a3bc2e6679fe8f62520be3ae), "BWB","Super Hyper Viper (Barcrest) (MPU4) (HVP 3.0, set 7)" )
GAME_CUSTOM( 199?, m4shv__o,    m4shv, cheatchr_pal<mpu4_characteriser_pal::m407_characteriser_prot>(R4, RT1), mpu4,  init_m4,  "hvi10___.3h3",         0x0000, 0x010000, CRC(6c1b4b89) SHA1(e8eb4e689d43c5b9e8354aa7375ca3ba12ed1160), "BWB","Super Hyper Viper (Barcrest) (MPU4) (HVP 3.0, set 8)" )
GAME_CUSTOM( 199?, m4shv__p,    m4shv, cheatchr_pal<mpu4_characteriser_pal::m407_characteriser_prot>(R4, RT1), mpu4,  init_m4,  "hvi10___.3n3",         0x0000, 0x010000, CRC(9d95cf8c) SHA1(26daf3975e1e3a605bc4392700c5470b52450d6e), "BWB","Super Hyper Viper (Barcrest) (MPU4) (HVP 3.0, set 9)" )
// "(C)1994  B.W.B." and "HVP 3.0"
GAME_CUSTOM( 199?, m4shv__q,    m4shv, cheatchr_pal<mpu4_characteriser_pal::m407_characteriser_prot>(R4, RT1), mpu4,  init_m4,  "h.viper10p610m.bin",   0x0000, 0x010000, CRC(104b0c48) SHA1(ab4cdb596a0cfb877ed1b6bf801e4a759b53971f), "BWB","Super Hyper Viper (Barcrest) (MPU4) (HVP 3.0, 1994, C)" )
GAME_CUSTOM( 199?, m4shv__r,    m4shv, cheatchr_pal<mpu4_characteriser_pal::m407_characteriser_prot>(R4, RT1), mpu4,  init_m4,  "hvyp56c",              0x0000, 0x010000, CRC(297d3cf8) SHA1(78f4de2ed69fb38b944a54d4d5927ff791e7876c), "BWB","Super Hyper Viper (Barcrest) (MPU4) (HVP 3.0, 1994, C, bad?)" ) // bad rom? checksum alarm
GAME_CUSTOM( 199?, m4shv__s,    m4shv, cheatchr_pal<mpu4_characteriser_pal::m407_characteriser_prot>(R4, RT1), mpu4,  init_m4,  "hv_05___.3o3",         0x0000, 0x010000, CRC(9ae86366) SHA1(614ae0ab184645c9f568796783f29a177eda3208), "BWB","Super Hyper Viper (Barcrest) (MPU4) (HVP 3.0, 1994, set 1)" )
GAME_CUSTOM( 199?, m4shv__t,    m4shv, cheatchr_pal<mpu4_characteriser_pal::m407_characteriser_prot>(R4, RT1), mpu4,  init_m4,  "hv_05_d_.3o3",         0x0000, 0x010000, CRC(87dfca0e) SHA1(3ab4105680acc46d3633a722f40ff1af0a520a7f), "BWB","Super Hyper Viper (Barcrest) (MPU4) (HVP 3.0, 1994, set 2)" )
GAME_CUSTOM( 199?, m4shv__u,    m4shv, cheatchr_pal<mpu4_characteriser_pal::m407_characteriser_prot>(R4, RT1), mpu4,  init_m4,  "hv_10___.3o3",         0x0000, 0x010000, CRC(02e4d86a) SHA1(47aa83e8bcd85e8ba7fb972cdd1ead7fe21e0418), "BWB","Super Hyper Viper (Barcrest) (MPU4) (HVP 3.0, 1994, set 3)" )
GAME_CUSTOM( 199?, m4shv__v,    m4shv, cheatchr_pal<mpu4_characteriser_pal::m407_characteriser_prot>(R4, RT1), mpu4,  init_m4,  "hv_10_d_.3o3",         0x0000, 0x010000, CRC(85f176b9) SHA1(30380d58bf2834829764cbdbdc7d950632e61e6d), "BWB","Super Hyper Viper (Barcrest) (MPU4) (HVP 3.0, 1994, set 4)" )
GAME_CUSTOM( 199?, m4shv__w,    m4shv, cheatchr_pal<mpu4_characteriser_pal::m407_characteriser_prot>(R4, RT1), mpu4,  init_m4,  "hvi05___.3o3",         0x0000, 0x010000, CRC(cdba80a5) SHA1(6c9fac7e5ee324b18922cc7a053495f1977bcb6d), "BWB","Super Hyper Viper (Barcrest) (MPU4) (HVP 3.0, 1994, set 5)" )
// "(C)1993  B.W.B." and "HVP 4.0"
GAME_CUSTOM( 199?, m4shv__x, m4shv, cheatchr_pal<mpu4_characteriser_pal::m407_characteriser_prot>(R4, RT1), mpu4,  init_m4,     "hv_05___.4n3",         0x0000, 0x010000, CRC(f607f351) SHA1(d7b779b80fa964a27b106bd9d5ca3be16a11d5e9), "BWB","Super Hyper Viper (Barcrest) (MPU4) (HVP 4.0, set 1)" )
GAME_CUSTOM( 199?, m4shv__y, m4shv, cheatchr_pal<mpu4_characteriser_pal::m407_characteriser_prot>(R4, RT1), mpu4,  init_m4,     "hv_05_d_.4n3",         0x0000, 0x010000, CRC(f4d702d7) SHA1(268c7f6443c7ae587caf5b227fcd438530a06bcc), "BWB","Super Hyper Viper (Barcrest) (MPU4) (HVP 4.0, set 2)" )
GAME_CUSTOM( 199?, m4shv__z, m4shv, cheatchr_pal<mpu4_characteriser_pal::m407_characteriser_prot>(R4, RT1), mpu4,  init_m4,     "hvi05___.4n3",         0x0000, 0x010000, CRC(38a33c2b) SHA1(21004092b81e08146291fd3a025652f0edbe47dc), "BWB","Super Hyper Viper (Barcrest) (MPU4) (HVP 4.0, set 3)" )
// "(C)1991 BARCREST" and "H6Y 0.3"
GAME_CUSTOM( 199?, m4shv,    0,     cheatchr_pal<mpu4_characteriser_pal::m450_characteriser_prot>(R4, RT1), mpu4,  init_m4,     "h6ys.p1",              0x0000, 0x010000, CRC(4af914ff) SHA1(3d9b7c65ec1129ee64e3f4e14e43e4c39c76166b), "Barcrest","Super Hyper Viper (Barcrest) (MPU4) (H6Y 0.3, set 1)" )
GAME_CUSTOM( 199?, m4shv__a, m4shv, cheatchr_pal<mpu4_characteriser_pal::m450_characteriser_prot>(R4, RT1), mpu4,  init_m4,     "h6yc.p1",              0x0000, 0x010000, CRC(8faca3bc) SHA1(9d666371f1118ccb1a94bfc4e6c79b540a84842b), "Barcrest","Super Hyper Viper (Barcrest) (MPU4) (H6Y 0.3 C)" )
GAME_CUSTOM( 199?, m4shv__b, m4shv, cheatchr_pal<mpu4_characteriser_pal::m450_characteriser_prot>(R4, RT1), mpu4,  init_m4,     "h6yd.p1",              0x0000, 0x010000, CRC(862e7f5b) SHA1(2f5bbc31978fb9fd0ba17f0de220152da87cf06f), "Barcrest","Super Hyper Viper (Barcrest) (MPU4) (H6Y 0.3 D)" )
GAME_CUSTOM( 199?, m4shv__c, m4shv, cheatchr_pal<mpu4_characteriser_pal::m450_characteriser_prot>(R4, RT1), mpu4,  init_m4,     "h6yk.p1",              0x0000, 0x010000, CRC(51f43c88) SHA1(d6ee4f537d09b33e9b13c972e1bda01a28f54f8e), "Barcrest","Super Hyper Viper (Barcrest) (MPU4) (H6Y 0.3 K)" )
GAME_CUSTOM( 199?, m4shv__d, m4shv, cheatchr_pal<mpu4_characteriser_pal::m450_characteriser_prot>(R4, RT1), mpu4,  init_m4,     "h6yy.p1",              0x0000, 0x010000, CRC(bed4b3bb) SHA1(7c592fbc6541c03777ff0498db90c575b3193222), "Barcrest","Super Hyper Viper (Barcrest) (MPU4) (H6Y 0.3 Y)" )
GAME_CUSTOM( 199?, m4shv__e, m4shv, cheatchr_pal<mpu4_characteriser_pal::m450_characteriser_prot>(R4, RT1), mpu4,  init_m4,     "hyperviper.bin",       0x0000, 0x010000, CRC(8373f6a3) SHA1(79bff20ab80ffe11447595c6fe8e5ab90d432e17), "Barcrest","Super Hyper Viper (Barcrest) (MPU4) (H6Y 0.3, set 2)" ) // hack?
// "(C)1991 BARCREST" and "H6Y 0.2"
GAME_CUSTOM( 199?, m4shv__f, m4shv, cheatchr_pal<mpu4_characteriser_pal::m450_characteriser_prot>(R4, RT1), mpu4,  init_m4,     "hvypr206",             0x0000, 0x010000, CRC(e1d96b8c) SHA1(e21b1bdbca1bae41f0e7274e3521f99eb984759e), "Barcrest","Super Hyper Viper (Barcrest) (MPU4) (H6Y 0.2 Y)" )
 // "(C)1995  B.W.B." and "HVC 1.0"
GAME_CUSTOM( 199?, m4shv__g, m4shv, cheatchr_pal<mpu4_characteriser_pal::hypvipalt_characteriser_prot>(R4, RT1), mpu4,  init_m4,"5p4hypervyper.bin",    0x0000, 0x010000, CRC(51ac9288) SHA1(1580079b6e710506ab03e1d8a89af65cd06cedd2), "BWB","Super Hyper Viper (Barcrest) (MPU4) (HVC 1.0 C)" )

// different protection
// no copyright string and "HVP 3.0"
GAME_CUSTOM( 199?, m4shv__0,    m4shv, bootleg_fixedret<0xb1>(R4, RT1), mpu4,  init_m4,  "hv056c",               0x0000, 0x010000, CRC(91dcef99) SHA1(8fb6245fa8731b58799c0d2edc0e6c6942984a6f), "hack","Super Hyper Viper (Barcrest) (MPU4) (HVP 3.0 C, hack, set 1)" )
GAME_CUSTOM( 199?, m4shv__1,    m4shv, bootleg_fixedret<0x65>(R4, RT1), mpu4,  init_m4,  "hv05_101",             0x0000, 0x010000, CRC(e1fa633d) SHA1(3f446c3396142631141cf85db507f3ae288847e3), "hack","Super Hyper Viper (Barcrest) (MPU4) (HVP 3.0 C, hack, set 2)" )
GAME_CUSTOM( 199?, m4shv__2,    m4shv, bootleg_fixedret<0x15>(R4, RT1), mpu4,  init_m4,  "hyp55",                0x0000, 0x010000, CRC(07bd7455) SHA1(0d0a017c90e8d28500594f55c9a60dfc08aff5c3), "hack","Super Hyper Viper (Barcrest) (MPU4) (HVP 3.0 C, hack, set 3)" )
GAME_CUSTOM( 199?, m4shv__3,    m4shv, bootleg_fixedret<0x11>(R4, RT1), mpu4,  init_m4,  "hypr58c",              0x0000, 0x010000, CRC(d6028f8f) SHA1(54a3188ddb5196808a1161a0e1e6a8c1fe8bfde3), "hack","Super Hyper Viper (Barcrest) (MPU4) (HVP 3.0 C, hack, set 4)" )
GAME_CUSTOM( 199?, m4shv__4,    m4shv, bootleg_fixedret<0x11>(R4, RT1), mpu4,  init_m4,  "hypv_05_.4",           0x0000, 0x010000, CRC(246f171c) SHA1(7bbefb0cae57cf8097aa6d033df1a428e8bfe744), "hack","Super Hyper Viper (Barcrest) (MPU4) (HVP 3.0 C, hack, set 5)" )
GAME_CUSTOM( 199?, m4shv__5,    m4shv, bootleg_fixedret<0x11>(R4, RT1), mpu4,  init_m4,  "hvip_05_.8",           0x0000, 0x010000, CRC(625f1b9d) SHA1(f8dc0cde774f3fc4fb3d66d014ad47e9576c0f44), "hack","Super Hyper Viper (Barcrest) (MPU4) (HVP 3.0, hack, set 1)" )
// "DICK WAS ERE" and "HVP 3.0"
GAME_CUSTOM( 199?, m4shv__6,    m4shv, bootleg_fixedret<0x5b>(R4, RT1), mpu4,  init_m4,  "hv108c",               0x0000, 0x010000, CRC(4d40ebfe) SHA1(0e355fe5b185ba595c5040335956037b8ed21599), "hack","Super Hyper Viper (Barcrest) (MPU4) (HVP 3.0 C, hack, set 6)" )
GAME_CUSTOM( 199?, m4shv__8,    m4shv, bootleg_fixedret<0x5b>(R4, RT1), mpu4,  init_m4,  "hypv_10_.4",           0x0000, 0x010000, CRC(f85d21a1) SHA1(55ed92147335a1471b7b443f68dd700f579d21f3), "hack","Super Hyper Viper (Barcrest) (MPU4) (HVP 3.0 C, hack, set 8)" )
GAME_CUSTOM( 199?, m4shv__9,    m4shv, bootleg_fixedret<0x5b>(R4, RT1), mpu4,  init_m4,  "hvip_10_.8",           0x0000, 0x010000, CRC(f91d7fec) SHA1(4c8130f9ce0ee3b14744e2b3cab79d4a65767e78), "hack","Super Hyper Viper (Barcrest) (MPU4) (HVP 3.0, hack, set 2)" )
GAME_CUSTOM( 199?, m4shv__7,    m4shv, bootleg_fixedret<0x6b>(R4, RT1), mpu4,  init_m4,  "hv10_101",             0x0000, 0x010000, CRC(57714454) SHA1(de99f5a66081191a7280c54e875fd17cc94e111b), "hack","Super Hyper Viper (Barcrest) (MPU4) (HVP 3.0 C, hack, set 7)" )
// "(C)1991 BARCREST" and "H6Y 0.3" (but hack, doesn't want usual characterizer)
GAME_CUSTOM( 199?, m4shv__10,   m4shv,  bootleg_fixedret<0x7a>(R4, RT1), mpu4,  init_m4,  "hv20_101",             0x0000, 0x010000, CRC(b2ab79c9) SHA1(fd097b5b062d725fa0607117d6b52be6cbf7e597), "hack","Super Hyper Viper (Barcrest) (MPU4) (H6Y 0.3, hack, set 1)" )
GAME_CUSTOM( 199?, m4shv__11,   m4shv,  bootleg_fixedret<0x4a>(R4, RT1), mpu4,  init_m4,  "hvip_20_.8",           0x0000, 0x010000, CRC(61a608c7) SHA1(1ed98c8bd90a3a789ba00b6b39f49e3aa0fcb1ca), "hack","Super Hyper Viper (Barcrest) (MPU4) (H6Y 0.3, hack, set 2)" )
GAME_CUSTOM( 199?, m4shv__12,   m4shv,  bootleg_fixedret<0x4a>(R4, RT1), mpu4,  init_m4,  "hypv_20_.4",           0x0000, 0x010000, CRC(27a0162b) SHA1(2d1342edbfa29c4f2ee1f1a825f3eeb0489fbaf5), "hack","Super Hyper Viper (Barcrest) (MPU4) (H6Y 0.3, hack, set 3)" )


/*****************************************************************************************************************************************************************************
*
* Hyper Viper Club
* - small extender
*
*****************************************************************************************************************************************************************************/

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAME(year, setname, parent, cheatchr_pal<mpu4_characteriser_pal::age_characteriser_prot>(R5R, RT1, LPS), mpu420p, mpu4mod4yam_machines_state, init_m4, ROT0, company, title, GAME_FLAGS )

// "(C)1991 BARCREST" and "HPC 0.5"
GAME_CUSTOM( 199?, m4hypclb,       0,          "hpcs.p1",  0x0000, 0x010000, CRC(55601e10) SHA1(78c3f13cd122e86ff8b7750b375c26e56c6b27c6), "Barcrest","Hyper Viper Club (Barcrest) (MPU4) (HPC 0.5)" )
GAME_CUSTOM( 199?, m4hypclb__c,    m4hypclb,   "hpcd.p1",  0x0000, 0x010000, CRC(7fac8944) SHA1(32f0f16ef6c4b99fe70464341a1ce226f6221122), "Barcrest","Hyper Viper Club (Barcrest) (MPU4) (HPC 0.5 D)" )
GAME_CUSTOM( 199?, m4hypclb__a,    m4hypclb,   "hpcf.p1",  0x0000, 0x010000, CRC(2931a558) SHA1(2f7fe541edc502738dd6603435deaef1cb26a1e2), "Barcrest","Hyper Viper Club (Barcrest) (MPU4) (HPC 0.5 F)" )
GAME_CUSTOM( 199?, m4hypclb__b,    m4hypclb,   "hpcfd.p1", 0x0000, 0x010000, CRC(b127e577) SHA1(da034086bb92934f73d1a2be776f91462274479d), "Barcrest","Hyper Viper Club (Barcrest) (MPU4) (HPC 0.5 FD)" )


/*****************************************************************************************************************************************************************************
*
* Ghost Buster
* - no extender?
*
*****************************************************************************************************************************************************************************/

// code crashes, why? are the alarms buggy?

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAME(year, setname, parent, cheatchr_pal<mpu4_characteriser_pal::viva_characteriser_prot>(R4, RT1), mpu4, mpu4mod4yam_machines_state, init_m4, ROT0, company, title, GAME_FLAGS )

// some of these ROMs (the non-D ones?) contain a 'Barcrest Video' string, why? there's no footage to support it being a video game

// "(C)1994  B.W.B." and "GB  5.0"
GAME_CUSTOM( 199?, m4gbust,     0,          "gb_05___.4s3",         0x0000, 0x010000, CRC(e2227701) SHA1(271682c7bf6e0f6f49f6d6b138aa19b6ef6bc626), "BWB","Ghost Buster (Barcrest) (MPU4) (GB 5.0)" )
// "(C)1994  B.W.B." and "GB  4.0"
GAME_CUSTOM( 199?, m4gbust__d,  m4gbust,    "gb_05___.4a3",         0x0000, 0x010000, CRC(8be6949e) SHA1(9731a1cb0d17c3cec2bec263cd6348f05662d917), "BWB","Ghost Buster (Barcrest) (MPU4) (GB 4.0, set 1)" )
GAME_CUSTOM( 199?, m4gbust__e,  m4gbust,    "gb_05___.4n3",         0x0000, 0x010000, CRC(621b25f0) SHA1(bf699068284def8bad9143c5841f667f2cb6f20f), "BWB","Ghost Buster (Barcrest) (MPU4) (GB 4.0, set 2)" )
GAME_CUSTOM( 199?, m4gbust__g,  m4gbust,    "gb_05_d_.4a3",         0x0000, 0x010000, CRC(a1b2b32f) SHA1(c1504b3768920f90dbd441b9d50db9676528ca97), "BWB","Ghost Buster (Barcrest) (MPU4) (GB 4.0 D)" )
// "(C)1994  B.W.B." and "GB  3.0"
GAME_CUSTOM( 199?, m4gbust__b,  m4gbust,    "gb_02___.3n3",         0x0000, 0x010000, CRC(99514ddd) SHA1(432d484525867c6ad68cd93a4bfded4dba36cf56), "BWB","Ghost Buster (Barcrest) (MPU4) (GB 3.0, 1994, set 1)" )
GAME_CUSTOM( 199?, m4gbust__a,  m4gbust,    "gb_02___.3a3",         0x0000, 0x010000, CRC(2b9d94b6) SHA1(ca433240f9e926cdf5240209589951e6018a496a), "BWB","Ghost Buster (Barcrest) (MPU4) (GB 3.0, 1994, set 2)" )
// "(C)1994  B.W.B." and "GB  2.0"
GAME_CUSTOM( 199?, m4gbust__c,  m4gbust,    "gb_02___.3s3",         0x0000, 0x010000, CRC(2634aa5f) SHA1(58ab973940138bdfd2690867e2ac3eb52bffb633), "BWB","Ghost Buster (Barcrest) (MPU4) (GB 2.0, 1994)" )
// "(C)1993  B.W.B." and "GB  3.0"
GAME_CUSTOM( 199?, m4gbust__j,  m4gbust,    "gb_10___.3s3",         0x0000, 0x010000, CRC(427e043b) SHA1(2f64c11a04306692ac5eb9919892f7226156dce0), "BWB","Ghost Buster (Barcrest) (MPU4) (GB 3.0, set 1)" )
GAME_CUSTOM( 199?, m4gbust__p,  m4gbust,    "gb_20___.3s3",         0x0000, 0x010000, CRC(4a86d879) SHA1(72e92b6482fdeb4dca36d9426a712ac24d60f7f7), "BWB","Ghost Buster (Barcrest) (MPU4) (GB 3.0, set 2)" )
GAME_CUSTOM( 199?, m4gbust__n,  m4gbust,    "gb_10_d_.3s3",         0x0000, 0x010000, CRC(776736de) SHA1(4f80d9ffdf4468801cf830e9774b6028f7684864), "BWB","Ghost Buster (Barcrest) (MPU4) (GB 3.0 D, set 1)" )
GAME_CUSTOM( 199?, m4gbust__v,  m4gbust,    "gb_20_d_.3s3",         0x0000, 0x010000, CRC(4fc69155) SHA1(09a0f2122893d9fd90204a74c8862e01386503a4), "BWB","Ghost Buster (Barcrest) (MPU4) (GB 3.0 D, set 2)" )
GAME_CUSTOM( 199?, m4gbust__k,  m4gbust,    "gb_10_b_.3s3",         0x0000, 0x010000, CRC(091afb66) SHA1(ac32d7be1e1f4f1453e37017966990a481506024), "BWB","Ghost Buster (Barcrest) (MPU4) (GB 3.0 YD, set 1)" )
GAME_CUSTOM( 199?, m4gbust__s,  m4gbust,    "gb_20_b_.3s3",         0x0000, 0x010000, CRC(1a7cc3cf) SHA1(0d5764d35489bde284965c197b217a06f26a3e3b), "BWB","Ghost Buster (Barcrest) (MPU4) (GB 3.0 YD, set 2)" )
// "(C)1993  B.W.B." and "GB  2.0"
GAME_CUSTOM( 199?, m4gbust__f,  m4gbust,    "gb_02___.2n3",         0x0000, 0x010000, CRC(973b3538) SHA1(31df04d9f35cbde4d5e395256927f146d1613178), "BWB","Ghost Buster (Barcrest) (MPU4) (GB 2.0, set 1)" )
GAME_CUSTOM( 199?, m4gbust__i,  m4gbust,    "gb_10___.2n3",         0x0000, 0x010000, CRC(de18c441) SHA1(5a7055fcd755c1ac58e1b94af243801f169f29f5), "BWB","Ghost Buster (Barcrest) (MPU4) (GB 2.0, set 2)" )
GAME_CUSTOM( 199?, m4gbust__o,  m4gbust,    "gb_20___.2n3",         0x0000, 0x010000, CRC(27fc2ee1) SHA1(2e6a042f7117b4594b2601ae166ee0db72c70ed5), "BWB","Ghost Buster (Barcrest) (MPU4) (GB 2.0, set 3)" )
GAME_CUSTOM( 199?, m4gbust__h,  m4gbust,    "gb_10___.2a3",         0x0000, 0x010000, CRC(a5c692f3) SHA1(8305c88ab8b80b407f4723df25135c25a4c0794f), "BWB","Ghost Buster (Barcrest) (MPU4) (GB 2.0, set 4)" )
GAME_CUSTOM( 199?, m4gbust__w,  m4gbust,    "ghostbusters 2p.bin",  0x0000, 0x010000, CRC(abb288c4) SHA1(2012e027711996a552ab59674ae3bce1bf14f44b), "BWB","Ghost Buster (Barcrest) (MPU4) (GB 2.0, set 5)" )
GAME_CUSTOM( 199?, m4gbust__m,  m4gbust,    "gb_10_d_.2n3",         0x0000, 0x010000, CRC(cac5057d) SHA1(afcc21dbd07515ed134675b7dbfb53c048a465b0), "BWB","Ghost Buster (Barcrest) (MPU4) (GB 2.0 D, set 1)" )
GAME_CUSTOM( 199?, m4gbust__u,  m4gbust,    "gb_20_d_.2n3",         0x0000, 0x010000, CRC(431c2965) SHA1(eb24e560d5c4bf419465fc760621a4fa853fff95), "BWB","Ghost Buster (Barcrest) (MPU4) (GB 2.0 D, set 2)" )
GAME_CUSTOM( 199?, m4gbust__l,  m4gbust,    "gb_10_d_.2a3",         0x0000, 0x010000, CRC(f1446bf5) SHA1(4011d60e13045476741c5a02c64dabbe6a1ae2d6), "BWB","Ghost Buster (Barcrest) (MPU4) (GB 2.0 D, set 3)" )
GAME_CUSTOM( 199?, m4gbust__t,  m4gbust,    "gb_20_d_.2a3",         0x0000, 0x010000, CRC(70f40688) SHA1(ed14f8f460825ffa087394ef5984ae064e02f7b6), "BWB","Ghost Buster (Barcrest) (MPU4) (GB 2.0 D, set 4)" )
GAME_CUSTOM( 199?, m4gbust__r,  m4gbust,    "gb_20_b_.2n3",         0x0000, 0x010000, CRC(28cbb217) SHA1(a74978ff5e1511a33f543006b3f8ad30a77ea462), "BWB","Ghost Buster (Barcrest) (MPU4) (GB 2.0 YD, set 1)" )
GAME_CUSTOM( 199?, m4gbust__q,  m4gbust,    "gb_20_b_.2a3",         0x0000, 0x010000, CRC(4dd7d38f) SHA1(8a71c27189ec3089c016a8292db68f7cdc91b083), "BWB","Ghost Buster (Barcrest) (MPU4) (GB 2.0 YD, set 2)" )


/*****************************************************************************************************************************************************************************
*
* Bucks Fizz Club / Super Bucks Fizz Club
* - no extender
*
*****************************************************************************************************************************************************************************/


ROM_START( m4bucks )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bufs.p1", 0x0000, 0x010000, CRC(e394ae40) SHA1(911077053c47cebba1bed9d359cd38bd676a46f1) )
ROM_END

ROM_START( m4bucksa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bufd.p1", 0x0000, 0x010000, CRC(02c575d3) SHA1(92dc7a0c298e4d2d19bf754a5c82cc15e4e6456c) )
ROM_END

ROM_START( m4supbf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sbff.p1", 0x0000, 0x010000, CRC(f27feba0) SHA1(157bf28e2d5fc2fa58bed11b3285cf56ae18abb8) )
ROM_END

ROM_START( m4supbfa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sbfs.p1", 0x0000, 0x010000, CRC(c8c52d5e) SHA1(d53513b9faabc307623a7c2f5be0225fb812beeb) )
ROM_END


GAME(199?, m4bucks,   0,          cheatchr_pal<mpu4_characteriser_pal::bucksfizz_characteriser_prot>(R5R, RT1),   mpu4,    mpu4mod4yam_machines_state, init_m4, ROT0,   "Barcrest","Bucks Fizz Club (Barcrest) (MPU4) (BUF 1.2, set 1)",GAME_FLAGS )
GAME(199?, m4bucksa,  m4bucks,    cheatchr_pal<mpu4_characteriser_pal::bucksfizz_characteriser_prot>(R5R, RT1),   mpu4,    mpu4mod4yam_machines_state, init_m4, ROT0,   "Barcrest","Bucks Fizz Club (Barcrest) (MPU4) (BUF 1.2, set 2)",GAME_FLAGS )
GAME(199?, m4supbf,   0,          cheatchr_pal<mpu4_characteriser_pal::bucksfizz_characteriser_prot>(R5R, RT1),   mpu4,    mpu4mod4yam_machines_state, init_m4, ROT0,   "Barcrest","Super Bucks Fizz Club (Barcrest) (MPU4) (SBF 2.0, set 1)",GAME_FLAGS )
GAME(199?, m4supbfa,  m4supbf,    cheatchr_pal<mpu4_characteriser_pal::bucksfizz_characteriser_prot>(R5R, RT1),   mpu4,    mpu4mod4yam_machines_state, init_m4, ROT0,   "Barcrest","Super Bucks Fizz Club (Barcrest) (MPU4) (SBF 2.0, set 2)",GAME_FLAGS )

/*****************************************************************************************************************************************************************************
*
* California Club
*  - can't see credit display, needs extender? (but not standard small/large one?)
*
*****************************************************************************************************************************************************************************/


ROM_START( m4calicl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ca2s.p1", 0x0000, 0x010000, CRC(fad153fd) SHA1(bd1f1a5c73624df45d01cb4853d87e998e434d7a) )
ROM_END

ROM_START( m4calicla )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ca2d.p1", 0x0000, 0x010000, CRC(75eb8c6f) SHA1(1bb923d06dcfa24eaf9533c083f68f4bd840834f) )
ROM_END

ROM_START( m4caliclb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ca2f.p1", 0x0000, 0x010000, CRC(6c53cf29) SHA1(2e58453891ab4faa17ef58a81c5f3c0618d046a5) )
ROM_END

ROM_START( m4caliclc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cald.p1", 0x0000, 0x010000, CRC(296fdeeb) SHA1(7782c0c7d8f44e2c0d48cc24c13015241e47b9ec) )
ROM_END

ROM_START( m4calicld )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cals.p1", 0x0000, 0x010000, CRC(28a1c5fe) SHA1(e8474df609ea7f3517780b54d6f493987aad3650) )
ROM_END


GAME(199?, m4calicl,  0,          cheatchr_pal<mpu4_characteriser_pal::shuffle_characteriser_prot>(R5R, RT1, LPS),   mpu420p,    mpu4mod4yam_machines_state, init_m4, ROT0,   "Barcrest","California Club (Barcrest) (MPU4) (CA2 1.0, set 1)",GAME_FLAGS )
GAME(199?, m4calicla, m4calicl,   cheatchr_pal<mpu4_characteriser_pal::shuffle_characteriser_prot>(R5R, RT1, LPS),   mpu420p,    mpu4mod4yam_machines_state, init_m4, ROT0,   "Barcrest","California Club (Barcrest) (MPU4) (CA2 1.0, set 2)",GAME_FLAGS )
GAME(199?, m4caliclb, m4calicl,   cheatchr_pal<mpu4_characteriser_pal::shuffle_characteriser_prot>(R5R, RT1, LPS),   mpu420p,    mpu4mod4yam_machines_state, init_m4, ROT0,   "Barcrest","California Club (Barcrest) (MPU4) (CA2 1.0, set 3)",GAME_FLAGS )
GAME(199?, m4caliclc, m4calicl,   cheatchr_pal<mpu4_characteriser_pal::shuffle_characteriser_prot>(R5R, RT1, LPS),   mpu420p,    mpu4mod4yam_machines_state, init_m4, ROT0,   "Barcrest","California Club (Barcrest) (MPU4) (CAL 2.0, set 1)",GAME_FLAGS )
GAME(199?, m4calicld, m4calicl,   cheatchr_pal<mpu4_characteriser_pal::shuffle_characteriser_prot>(R5R, RT1, LPS),   mpu420p,    mpu4mod4yam_machines_state, init_m4, ROT0,   "Barcrest","California Club (Barcrest) (MPU4) (CAL 2.0, set 2)",GAME_FLAGS )

/*****************************************************************************************************************************************************************************
*
* Sunset Club
*  - can't see credit display, needs extender? (but not standard small/large one?)
*
*****************************************************************************************************************************************************************************/

ROM_START( m4sunscl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sc_xe___.3_3", 0x0000, 0x010000, CRC(e3732cc6) SHA1(77f0368bb29ad00030f83af794a52df92fe97e5d) )
ROM_END

ROM_START( m4sunscla )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sc_xe_d_.3_3", 0x0000, 0x010000, CRC(b8627c4a) SHA1(ad616d38773cbd82376b518aa15dc3d7027237c5) )
ROM_END

ROM_START( m4sunsclb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sc_xef__.3_3", 0x0000, 0x010000, CRC(8e7e1100) SHA1(7648ea860a546081388a213845e27312730f46d9) )
ROM_END

GAME(199?, m4sunscl,  0,          cheatchr_pal<mpu4_characteriser_pal::sunsetclub_characteriser_prot>(R5R, RT1, LPS),       mpu420p,    mpu4mod4yam_machines_state, init_m4, ROT0,   "BWB", "Sunset Club (BWB) (MPU4) (SSC 3.0, set 1)", GAME_FLAGS )
GAME(199?, m4sunscla, m4sunscl,   cheatchr_pal<mpu4_characteriser_pal::sunsetclub_characteriser_prot>(R5R, RT1, LPS),       mpu420p,    mpu4mod4yam_machines_state, init_m4, ROT0,   "BWB", "Sunset Club (BWB) (MPU4) (SSC 3.0, set 2)", GAME_FLAGS )
GAME(199?, m4sunsclb, m4sunscl,   cheatchr_pal<mpu4_characteriser_pal::sunsetclub_characteriser_prot>(R5R, RT1, LPS),       mpu420p,    mpu4mod4yam_machines_state, init_m4, ROT0,   "BWB", "Sunset Club (BWB) (MPU4) (SSC 3.0, set 3)", GAME_FLAGS )


/*****************************************************************************************************************************************************************************
*
* Tropicana Club
*  - can't see credit display, needs extender? (but not standard small/large one?)
*
*****************************************************************************************************************************************************************************/

ROM_START( m4tropcl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tros.p1", 0x0000, 0x010000, CRC(5e86c3fc) SHA1(ce2419991559839a8875060c1afe0f030190010a) )
ROM_END

ROM_START( m4tropcla )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tr2d.p1", 0x0000, 0x010000, CRC(0cc23f89) SHA1(a66c8c28073f53381c43e3e597f15f81c5c61479) )
ROM_END

ROM_START( m4tropclb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tr2f.p1", 0x0000, 0x010000, CRC(fbdcd06f) SHA1(27ccdc83e60a62227d33d8cf3d516fc43908ab99) )
ROM_END

ROM_START( m4tropclc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tr2s.p1", 0x0000, 0x010000, CRC(6d43375c) SHA1(5be1dc85374c6a1235e0b137b46ebd7a2d7d922a) )
ROM_END

ROM_START( m4tropcld )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "trod.p1", 0x0000, 0x010000, CRC(60c84612) SHA1(84dc8b34e41436331832c1a32ddac0fce269488a) )
ROM_END

GAME(199?, m4tropcl,  0,          cheatchr_pal<mpu4_characteriser_pal::shuffle_characteriser_prot>(R5R, RT2, LPS),   mpu420p,    mpu4mod4yam_machines_state, init_m4, ROT0,   "Barcrest","Tropicana Club (Barcrest) (MPU4) (TRO 2.0, set 1)",GAME_FLAGS )
GAME(199?, m4tropcld, m4tropcl,   cheatchr_pal<mpu4_characteriser_pal::shuffle_characteriser_prot>(R5R, RT2, LPS),   mpu420p,    mpu4mod4yam_machines_state, init_m4, ROT0,   "Barcrest","Tropicana Club (Barcrest) (MPU4) (TRO 2.0, set 2)",GAME_FLAGS )
GAME(199?, m4tropcla, m4tropcl,   cheatchr_pal<mpu4_characteriser_pal::shuffle_characteriser_prot>(R5R, RT2, LPS),   mpu420p,    mpu4mod4yam_machines_state, init_m4, ROT0,   "Barcrest","Tropicana Club (Barcrest) (MPU4) (TR2 1.1, set 1)",GAME_FLAGS )
GAME(199?, m4tropclb, m4tropcl,   cheatchr_pal<mpu4_characteriser_pal::shuffle_characteriser_prot>(R5R, RT2, LPS),   mpu420p,    mpu4mod4yam_machines_state, init_m4, ROT0,   "Barcrest","Tropicana Club (Barcrest) (MPU4) (TR2 1.1, set 2)",GAME_FLAGS )
GAME(199?, m4tropclc, m4tropcl,   cheatchr_pal<mpu4_characteriser_pal::shuffle_characteriser_prot>(R5R, RT2, LPS),   mpu420p,    mpu4mod4yam_machines_state, init_m4, ROT0,   "Barcrest","Tropicana Club (Barcrest) (MPU4) (TR2 1.1, set 3)",GAME_FLAGS )


/*****************************************************************************************************************************************************************************
*
* Nudge Shuffle
* - no extender
*
*****************************************************************************************************************************************************************************/

ROM_START( m4nudshf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nusx.p1", 0x0000, 0x010000, CRC(87caab84) SHA1(e2492ad0d25ded4d760c4cbe05e9b51ca1a10544) )
ROM_END

ROM_START( m4nudshfa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nus6", 0x0000, 0x010000, CRC(017c5354) SHA1(07491e4b03ab62ad923f8479300c1af4633e3e8c) )
ROM_END

ROM_START( m4nudshfb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nuss.bin", 0x0000, 0x010000, CRC(d3b860ee) SHA1(d5d1262c715e4684748b0cae708eeed31b1dc50f) )
ROM_END

ROM_START( m4nudshfc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nusxc.p1", 0x0000, 0x010000, CRC(e2557b45) SHA1(a9d1514d4fe3897f6fcef22a5039d6bdff8126ff) )
ROM_END


GAME(199?, m4nudshf,  0,          cheatchr_pal<mpu4_characteriser_pal::nudshf_characteriser_prot>(R4, RT2),   mpu4,    mpu4mod4yam_machines_state, init_m4, ROT0,   "Barcrest","Nudge Shuffle (Barcrest) (MPU4) (NUS 3.1) (set 1)",GAME_FLAGS )
GAME(199?, m4nudshfb, m4nudshf,   cheatchr_pal<mpu4_characteriser_pal::nudshf_characteriser_prot>(R4, RT2),   mpu4,    mpu4mod4yam_machines_state, init_m4, ROT0,   "Barcrest","Nudge Shuffle (Barcrest) (MPU4) (NUS 3.1) (set 2)",GAME_FLAGS )
GAME(199?, m4nudshfc, m4nudshf,   cheatchr_pal<mpu4_characteriser_pal::nudshf_characteriser_prot>(R4, RT2),   mpu4,    mpu4mod4yam_machines_state, init_m4, ROT0,   "Barcrest","Nudge Shuffle (Barcrest) (MPU4) (NUS 3.1) (set 3)",GAME_FLAGS )
GAME(199?, m4nudshfa, m4nudshf,   cheatchr_pal<mpu4_characteriser_pal::nudshf_characteriser_prot>(R4, RT2),   mpu4,    mpu4mod4yam_machines_state, init_m4, ROT0,   "Barcrest","Nudge Shuffle (Barcrest) (MPU4) (NUS 3.0)",GAME_FLAGS )


/*****************************************************************************************************************************************************************************
*
* Night Spot
* - seems to be using an extender, but doesn't look quite right with small one
*
*****************************************************************************************************************************************************************************/


ROM_START( m4nspot )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ns2s.p1", 0x0000, 0x010000, CRC(ba0f5a81) SHA1(7015176d4528636cb8a753249c824c37941e8eae) )
ROM_END

ROM_START( m4nspota )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ns2d.p1", 0x0000, 0x010000, CRC(5e66b7e0) SHA1(e82044e3c1e5cf3a2baf1fde7b7ab8b6e221d360) )
ROM_END

ROM_START( m4nspotb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nits.p1", 0x0000, 0x010000, CRC(47c965e6) SHA1(41a337a9a367c4e704a60e32d56b262d03f97b59) )
ROM_END

GAME(199?, m4nspot,   0,          cheatchr_pal<mpu4_characteriser_pal::celclb_characteriser_prot>(R5R, RT1, LPS),   mpu420p,    mpu4mod4yam_machines_state, init_m4, ROT0,   "Barcrest","Night Spot Club (Barcrest) (MPU4) (NS2 2.2, set 1)",GAME_FLAGS )
GAME(199?, m4nspota,  m4nspot,    cheatchr_pal<mpu4_characteriser_pal::celclb_characteriser_prot>(R5R, RT1, LPS),   mpu420p,    mpu4mod4yam_machines_state, init_m4, ROT0,   "Barcrest","Night Spot Club (Barcrest) (MPU4) (NS2 2.2, set 2)",GAME_FLAGS )
GAME(199?, m4nspotb,  m4nspot,    cheatchr_pal<mpu4_characteriser_pal::celclb_characteriser_prot>(R5R, RT1, LPS),   mpu420p,    mpu4mod4yam_machines_state, init_m4, ROT0,   "Barcrest","Night Spot Club (Barcrest) (MPU4) (NIT 1.1)",GAME_FLAGS )


/*****************************************************************************************************************************************************************************
*
* Misc Other Sets
*
*****************************************************************************************************************************************************************************/

ROM_START( m4joljokd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "djj15.bin", 0x0000, 0x010000, CRC(155cb134) SHA1(c1026effeceba131df9681afd91ccd6fb43b738a) )
ROM_END

ROM_START( m4clbshf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "csss.p1", 0x0000, 0x010000, CRC(32dd9b96) SHA1(93831858b2f0ada8e4a0aa2fae59d12c53287df1) )
ROM_END


ROM_START( m4voodoo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ddo32", 0x0000, 0x010000, CRC(260dfef1) SHA1(2b4918e40808963a86d289cd251740a9b0bed70a) )
ROM_END

ROM_START( m4magdrg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dmd10.bin", 0x0000, 0x010000, CRC(9cc4f2f8) SHA1(46a90ffa18d35ad2b06542f91120c02bc34f0c40) )
ROM_END

ROM_START( m4hslo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hot30", 0x0000, 0x010000, CRC(62f2c420) SHA1(5ae89a1b585738255e8d9ae153c3c63b4a2893e4) )
ROM_END

ROM_START( m4addrd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dal12.bin", 0x0000, 0x010000, CRC(4affa79a) SHA1(68bceab42b3616641a34a64a83306175ffc1ce32) )
ROM_END


GAME(199?, m4joljokd, 0,       cheatchr_pal<mpu4_characteriser_pal::celclb_characteriser_prot>(R4, RT1), mpu4, mpu4mod4yam_machines_state, init_m4,  ROT0,   "Barcrest","Jolly Joker (Barcrest) (Dutch) (MPU4) (DJJ 1.5)",GAME_FLAGS) // Geen Tubes

GAME(199?, m4clbshf,  0,        cheatchr_pal<mpu4_characteriser_pal::shuffle_characteriser_prot>(R4, RT1), mpu420p, mpu4mod4yam_machines_state, init_m4,  ROT0,   "Barcrest","Club Shuffle (Barcrest) (MPU4) (CSS 1.0)",GAME_FLAGS) // set stake (runs if you do)

GAME(199?, m4voodoo,  0,        cheatchr_pal<mpu4_characteriser_pal::m435_characteriser_prot>(R6, RT1), mpu4_dutch, mpu4mod4yam_machines_state, init_m4,  ROT0,   "Barcrest","Voodoo 1000 (Barcrest) (Dutch) (MPU4) (DDO 3.2)",GAME_FLAGS ) // ROL F SETUP ALM

GAME(199?, m4magdrg,  0,          cheatchr_pal<mpu4_characteriser_pal::magicdragon_characteriser_prot>(R7, RT1), mpu4_dutch,    mpu4mod4yam_machines_state, init_m4, ROT0,   "Barcrest","Magic Dragon (Barcrest) (MPU4) (DMD1.0)",GAME_FLAGS )

// non-standard protection
GAME(199?, m4hslo,    0,          bootleg_fixedret<0x56>(R4, RT1),       mpu4_70pc,    mpu4mod4yam_machines_state, init_m4, ROT0,   "bootleg","Hot Slot (bootleg) (MPU4) (HOT 3.0)",GAME_FLAGS )

GAME(199?, m4addrd,   0,          cheatchr_pal<mpu4_characteriser_pal::m470_characteriser_prot>(R5R, RT1),   mpu4_dutch,    mpu4mod4yam_machines_state, init_m4, ROT0,   "Barcrest","Adders & Ladders (Barcrest) (Dutch) (MPU4) (DAL 1.2)",GAME_FLAGS )
