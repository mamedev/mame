// license:BSD-3-Clause
// copyright-holders:David Haywood
/* MPU4 BWB games
 - these are BWB originals, not rebuilds..
   typically they have large 0x3f fills in rom..
   some have information about the various set
   types in unused areas

   some QPS sets also have large 0x3f fills but
   seem to be unrelated.

    ----------------------------------------
    This file is home to the following BWB originals

    Abracadabra
    Big Match, The
    Bingo Belle
    Bingo Belle (Showcase cabinet version)
    Bingo Club
    Blues Boys
    Championship Soccer
    Cup Final
    Dancing Diamonds
    Daytona
    Excalibur
    Exotic Fruits
    Fire & Ice
    Harlequin
    Heaven & Hell
    Indy Cars
    Jackpot Jokers
    Jumping Jack Flash
    Lucky Number 7
    Mad Money
    Mad Money Classic
    Money Mummy Money
    Orlando Magic
    Prize Bingo
    Quids In
    Quids In (Showcase cabinet version)
    Rack Em Up
    Rainbow Gold
    Red Hot Fever
    Sinbad
    Sky Sports
    Soul Sister
    Spin The Bottle
    Stars & Stripes
    Super League
    Super Soccer
    Sure Thing
    T-Rex
    Tutti Fruity
    Volcano
    Voodoo Express
    X-change
    X-s
    X-treme

    ----------------------------------------
    German games by BWB and Nova also in here

    Blues Boys (Nova, German version)
    Cup Final (Nova, German version)
    World Cup (Nova, German version)
    Excalibur (Nova, German version)
    Olympic Gold (Nova, German version)
    Find The Lady (Nova, German version)
    Sinbad (Nova, German version)

*/

#include "emu.h"
#include "mpu4.h"

namespace {

class mpu4bwb_machines_state : public mpu4_state
{
public:

	mpu4bwb_machines_state(const machine_config &mconfig, device_type type, const char *tag) :
		mpu4_state(mconfig, type, tag)
	{
		// Many BWB games do use the duart payout, but returning random values
		// there until it's implemented results in an 'unexpected PO' error
		// (Unexpected Pay Out) while many Barcrest games only boot if the value
		// changes.  This will go away once the hopper is properly emulated
		m_hack_duart_fixed_low = true;
	}

	void bwboki_f(machine_config &config);
	template<const uint32_t* Key> void bwboki_chr_cheat_f(machine_config &config);
	template<const uint32_t* Key, typename... T>
	auto bwboki_chr_cheat(T... traits)
	{
		return trait_wrapper(this, &mpu4bwb_machines_state::bwboki_chr_cheat_f<Key>, traits...);
	}


protected:

	void mpu4_memmap_characteriser_bwb(address_map &map) ATTR_COLD;

	void mpu4_install_mod4bwb_space(address_space &space);

	DECLARE_MACHINE_START(mpu4bwb);

};

void mpu4bwb_machines_state::mpu4_install_mod4bwb_space(address_space &space)
{
	// does anything else need to go here?
	mpu4_install_mod4oki_space(space);
}

MACHINE_START_MEMBER(mpu4bwb_machines_state,mpu4bwb)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	mpu4_config_common();

	m_link7a_connected=false;
	m_link7b_connected=true;
	mpu4_install_mod4bwb_space(space);
}

void mpu4bwb_machines_state::bwboki_f(machine_config &config)
{
	mpu4base(config);
	MCFG_MACHINE_START_OVERRIDE(mpu4bwb_machines_state,mpu4bwb)


	MPU4_OKI_SAMPLED_SOUND(config, m_okicard, MPU4_MASTER_CLOCK/4);
	m_okicard->add_route(ALL_OUTPUTS, "mono", 1.0);

	m_okicard->cb2_handler().set(FUNC(mpu4_state::pia_gb_cb2_w));
}

template<const uint32_t* Key> void mpu4bwb_machines_state::bwboki_chr_cheat_f(machine_config &config)
{
	bwboki_f(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &mpu4bwb_machines_state::mpu4_memmap_characteriser_bwb);
	MPU4_CHARACTERISER_PAL_BWB(config, m_characteriser_bwb, 0);
	m_characteriser_bwb->set_common_key(Key[0] & 0xff);
	m_characteriser_bwb->set_other_key(Key[1]);
}

void mpu4bwb_machines_state::mpu4_memmap_characteriser_bwb(address_map &map)
{
	mpu4_memmap(map);
	map(0x0800, 0x083f).rw(m_characteriser_bwb, FUNC(mpu4_characteriser_pal_bwb::read), FUNC(mpu4_characteriser_pal_bwb::write));
}

#define USE_STAKE_PORT_VALUE(value) \
	PORT_MODIFY("ORANGE1") \
	PORT_CONFNAME( 0xe0, value, "Stake Key" ) \
	PORT_CONFSETTING(    0x00, "Not fitted / 5p" ) \
	PORT_CONFSETTING(    0x20, "10p" ) \
	PORT_CONFSETTING(    0x40, "20p" ) \
	PORT_CONFSETTING(    0x60, "25p" ) \
	PORT_CONFSETTING(    0x80, "30p" ) \
	PORT_CONFSETTING(    0xa0, "40p" ) \
	PORT_CONFSETTING(    0xc0, "50p" ) \
	PORT_CONFSETTING(    0xe0, "1 GBP" )

#define USE_JACKPOT_PORT_VALUE(value) \
	PORT_MODIFY("ORANGE2") \
	PORT_CONFNAME( 0x0f, value, "Jackpot / Prize Key" ) \
	PORT_CONFSETTING(    0x00, "Not fitted" ) \
	PORT_CONFSETTING(    0x01, "3 GBP" ) \
	PORT_CONFSETTING(    0x02, "4 GBP" ) \
	PORT_CONFSETTING(    0x08, "5 GBP" ) \
	PORT_CONFSETTING(    0x03, "6 GBP" ) \
	PORT_CONFSETTING(    0x04, "6 GBP Token" ) \
	PORT_CONFSETTING(    0x05, "8 GBP" ) \
	PORT_CONFSETTING(    0x06, "8 GBP Token" ) \
	PORT_CONFSETTING(    0x07, "10 GBP" ) \
	PORT_CONFSETTING(    0x09, "15 GBP" ) \
	PORT_CONFSETTING(    0x0a, "25 GBP" ) \
	PORT_CONFSETTING(    0x0b, "25 GBP (Licensed Betting Office Profile)" ) \
	PORT_CONFSETTING(    0x0c, "35 GBP" ) \
	PORT_CONFSETTING(    0x0d, "70 GBP" ) \
	PORT_CONFSETTING(    0x0e, "Reserved" ) \
	PORT_CONFSETTING(    0x0f, "Reserved" )

#define USE_PERCENT_PORT_VALUE(value) \
	PORT_MODIFY("ORANGE2") \
	PORT_CONFNAME( 0xf0, value, "Percentage Key" ) \
	PORT_CONFSETTING(    0x00, "Not fitted / 68% (Invalid for UK Games)" ) \
	PORT_CONFSETTING(    0x10, "70" ) \
	PORT_CONFSETTING(    0x20, "72" ) \
	PORT_CONFSETTING(    0x30, "74" ) \
	PORT_CONFSETTING(    0x40, "76" ) \
	PORT_CONFSETTING(    0x50, "78" ) \
	PORT_CONFSETTING(    0x60, "80" ) \
	PORT_CONFSETTING(    0x70, "82" ) \
	PORT_CONFSETTING(    0x80, "84" ) \
	PORT_CONFSETTING(    0x90, "86" ) \
	PORT_CONFSETTING(    0xa0, "88" ) \
	PORT_CONFSETTING(    0xb0, "90" ) \
	PORT_CONFSETTING(    0xc0, "92" ) \
	PORT_CONFSETTING(    0xd0, "94" ) \
	PORT_CONFSETTING(    0xe0, "96" ) \
	PORT_CONFSETTING(    0xf0, "98" )

INPUT_PORTS_START( mpu4impcoin_20p )
	PORT_INCLUDE( mpu4_impcoin )

	USE_STAKE_PORT_VALUE(0x40)
INPUT_PORTS_END

INPUT_PORTS_START( mpu4impcoin_jackpot10_20p )
	PORT_INCLUDE( mpu4impcoin_20p )

	USE_JACKPOT_PORT_VALUE(0x07)
INPUT_PORTS_END

INPUT_PORTS_START( mpu4impcoin_jackpot15_20p )
	PORT_INCLUDE( mpu4impcoin_20p )

	USE_JACKPOT_PORT_VALUE(0x09)
INPUT_PORTS_END

INPUT_PORTS_START( mpu4impcoin_jackpot15_20p_90pc )
	PORT_INCLUDE( mpu4impcoin_jackpot15_20p )

	USE_PERCENT_PORT_VALUE(0xb0)
INPUT_PORTS_END


INPUT_PORTS_START( mpu4impcoin_jackpot8tkn_20p )
	PORT_INCLUDE( mpu4impcoin_20p )

	USE_JACKPOT_PORT_VALUE(0x06)
INPUT_PORTS_END

INPUT_PORTS_START( mpu4impcoin_jackpot8tkn_20p_90pc )
	PORT_INCLUDE( mpu4impcoin_jackpot8tkn_20p )

	USE_PERCENT_PORT_VALUE(0xb0)
INPUT_PORTS_END

INPUT_PORTS_START( mpu4impcoin_jackpot5_5p )
	PORT_INCLUDE( mpu4_impcoin )

	USE_JACKPOT_PORT_VALUE(0x08)
INPUT_PORTS_END



} // anonymous namespace

using namespace mpu4_traits;

#define GAME_FLAGS (MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL)


/*****************************************************************************************************************************************************************************
*
* Abracadabra
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4abra_keys[2] = { 0x11, 0x192703 };

#define M4ABRA_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 ) \
	/* missing? */

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent, machine, inputs, init, name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4ABRA_EXTRA_ROMS \
	ROM_END \
	GAME( year, setname, parent, machine, inputs, mpu4bwb_machines_state, init, ROT0, company, title, GAME_FLAGS )


GAME_CUSTOM( 199?, m4abra,       0,      bwboki_chr_cheat<m4abra_keys>(R5, RT3), mpu4impcoin_jackpot15_20p,      init_m4big, "nn_sj___.4_0", 0x0000, 0x040000, CRC(48437d29) SHA1(72a2e9337fc0a004c382931f3af856253c44ed61), "BWB",u8"Abracadabra (BWB) (MPU4) (20/25/30p stake / £10/£15 jackpot) (set 1)" )
GAME_CUSTOM( 199?, m4abra__a,    m4abra, bwboki_chr_cheat<m4abra_keys>(R5, RT3), mpu4impcoin_jackpot15_20p,      init_m4big, "nn_sja__.4_0", 0x0000, 0x040000, CRC(766cd4ae) SHA1(4d630b967ede615d325f524c2e4c92c7e7a60886), "BWB",u8"Abracadabra (BWB) (MPU4) (20/25/30p stake / £10/£15 jackpot) (set 2)" )
GAME_CUSTOM( 199?, m4abra__b,    m4abra, bwboki_chr_cheat<m4abra_keys>(R5, RT3), mpu4impcoin_jackpot15_20p,      init_m4big, "nn_sjb__.4_0", 0x0000, 0x040000, CRC(ca77a68a) SHA1(e753c065d299038bae4c451e647b9bcda36421d9), "BWB",u8"Abracadabra (BWB) (MPU4) (20/25/30p stake / £10/£15 jackpot) (set 3)" ) // Datapak
GAME_CUSTOM( 199?, m4abra__c,    m4abra, bwboki_chr_cheat<m4abra_keys>(R5, RT3), mpu4impcoin_jackpot15_20p_90pc, init_m4big, "nn_sjk__.4_0", 0x0000, 0x040000, CRC(19018556) SHA1(6df993939e70a24621d4e732d0670d64fac1cf56), "BWB",u8"Abracadabra (BWB) (MPU4) (20/25/30p stake / £10/£15 jackpot) (set 4)" ) // % key


/*****************************************************************************************************************************************************************************
*
* The Big Match
* - some sets boot
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4bigmt_keys[2] = { 0x10, 0x241215 };

#define M4BIGMT_EXTRA_ROMS \
	ROM_REGION( 0x180000, "okicard:altmsm6376", 0 ) /* this is NOT the same, some samples are changed */ \
	ROM_LOAD( "bigmsnd", 0x0000, 0x07db60, CRC(876c53ae) SHA1(ea2511ec9ba4ff67879212c6e2ba908873130a4e) ) \
	ROM_REGION( 0x180000, "okicard:msm6376", 0 ) \
	ROM_LOAD( "tbmsnd.hex", 0x0000, 0x080000, CRC(e98da8de) SHA1(36668f2b82778f441224c94831f5b95efb9fa92b) )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4BIGMT_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4bigmt_keys>(R5, RT3), mpu4_impcoin, mpu4bwb_machines_state, init_m4 ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4bigmt,     0,          "bigm1320",     0x0000, 0x010000, CRC(a5085347) SHA1(93a7f7656e53461270e04190ff538959d6c917c1), "BWB","The Big Match (BWB) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4bigmt__a,  m4bigmt,    "tb_20___.7_1", 0x0000, 0x010000, CRC(22fae0f0) SHA1(a875adccf96fbbff69f5fe76720514767cdcae66), "BWB","The Big Match (BWB) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4bigmt__b,  m4bigmt,    "tb_20_b_.7_1", 0x0000, 0x010000, CRC(40d140a3) SHA1(fd4de8dd827db933481f671e4f10684c3b7a363a), "BWB","The Big Match (BWB) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4bigmt__c,  m4bigmt,    "tb_20_d_.7_1", 0x0000, 0x010000, CRC(86ed18c5) SHA1(1699645532134bd830e3fc2c3ff4b67b5d67ba3e), "BWB","The Big Match (BWB) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4bigmt__d,  m4bigmt,    "tb_20_k_.7_1", 0x0000, 0x010000, CRC(e4c6b896) SHA1(ccf656d14ad68edb0aea99d4d3621540b899cdb7), "BWB","The Big Match (BWB) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4bigmt__f,  m4bigmt,    "tbi20___.7_1", 0x0000, 0x010000, CRC(104f45cc) SHA1(c28c08b44c46db9d0eade1f60aeda120c3981a03), "BWB","The Big Match (BWB) (MPU4) (set 7)" )


/*****************************************************************************************************************************************************************************
*
* Bingo Belle
* - some sets boot
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4bingbl_keys[2] = { 0x10, 0x322214 };

#define M4BINGBL_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 ) \
	/* Missing, or not OKI? */
#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4BINGBL_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4bingbl_keys>(R5, RT3), mpu4_impcoin, mpu4bwb_machines_state, init_m4 ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4bingbl,       0,          "bb_20a__.8_1", 0x0000, 0x010000, CRC(10f29ba3) SHA1(739b413f35676834ebafeb121c6059759586ec72), "BWB","Bingo Belle (BWB) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4bingbl__a,    m4bingbl,   "bb_20bg_.8_1", 0x0000, 0x010000, CRC(9969c9ce) SHA1(8c754335e7ff75bc46f02095c2c7d57df046db47), "BWB","Bingo Belle (BWB) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4bingbl__b,    m4bingbl,   "bb_20bt_.8_1", 0x0000, 0x010000, CRC(3a06f518) SHA1(79a9b1e63517f6436ee4743dfc7aa63a9cf585b8), "BWB","Bingo Belle (BWB) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4bingbl__c,    m4bingbl,   "bb_20s__.8_1", 0x0000, 0x010000, CRC(c5bd878e) SHA1(dc6048cf25b3a62b23761c338fe6398724b24159), "BWB","Bingo Belle (BWB) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4bingbl__d,    m4bingbl,   "bb_20sb_.8_1", 0x0000, 0x010000, CRC(a79627dd) SHA1(c9a6262a0e52a4d1ff073465ab6b489626ae6a2a), "BWB","Bingo Belle (BWB) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4bingbl__e,    m4bingbl,   "bb_20sd_.8_1", 0x0000, 0x010000, CRC(61aa7fbb) SHA1(0bf54a562d04c0b41fc35c5c84f529efa2ae9e10), "BWB","Bingo Belle (BWB) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4bingbl__f,    m4bingbl,   "bb_20sk_.8_1", 0x0000, 0x010000, CRC(0381dfe8) SHA1(8d3d232c10f9ec683a82464ee901f0963d9ffcd6), "BWB","Bingo Belle (BWB) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4bingbl__g,    m4bingbl,   "bbi20___.8_1", 0x0000, 0x010000, CRC(eae04eac) SHA1(bcc619868a065d92d09fa3b2411958b373b3edde), "BWB","Bingo Belle (BWB) (MPU4) (set 8)" )


/*****************************************************************************************************************************************************************************
*
* Bingo Belle (Showcase cabinet version)
* - some sets boot
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4bingbs_keys[2] = { 0x10, 0x322214 };

#define M4BINGBS_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 ) \
	/* Missing, or not OKI? */
#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4BINGBS_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4bingbs_keys>(R5, RT3), mpu4_impcoin, mpu4bwb_machines_state, init_m4 ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4bingbs,       0,          "bp_20a__.2_1", 0x0000, 0x010000, CRC(ca005003) SHA1(271ff0dbee529ca15c79c9aa1047efa8993ea073), "BWB","Bingo Belle Showcase (BWB) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4bingbs__a,    m4bingbs,   "bp_20bg_.2_1", 0x0000, 0x010000, CRC(1b59c32f) SHA1(0c7df33f921639bfedbddd969dcbcd62e38ff912), "BWB","Bingo Belle Showcase (BWB) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4bingbs__b,    m4bingbs,   "bp_20bt_.2_1", 0x0000, 0x010000, CRC(bc7a1830) SHA1(3c16432562ebaef3f17e51feebb4c35d911e90b9), "BWB","Bingo Belle Showcase (BWB) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4bingbs__c,    m4bingbs,   "bp_20s__.2_1", 0x0000, 0x010000, CRC(1f4f4c2e) SHA1(28f6361c7ee693635c718ea485687bf9fbf8532b), "BWB","Bingo Belle Showcase (BWB) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4bingbs__d,    m4bingbs,   "bp_20sb_.2_1", 0x0000, 0x010000, CRC(7d64ec7d) SHA1(bc279010642edadcba19cdfad222a783e35039b9), "BWB","Bingo Belle Showcase (BWB) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4bingbs__e,    m4bingbs,   "bp_20sd_.2_1", 0x0000, 0x010000, CRC(bb58b41b) SHA1(253859ab6d8033eeb94ad5c4586eadc8792bd436), "BWB","Bingo Belle Showcase (BWB) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4bingbs__f,    m4bingbs,   "bp_20sk_.2_1", 0x0000, 0x010000, CRC(d9731448) SHA1(2f054a99c5d1570b6e0307542c50421408a610dc), "BWB","Bingo Belle Showcase (BWB) (MPU4) (set 7)" )


/*****************************************************************************************************************************************************************************
*
* Bingo Club
* - some sets boot
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4bingcl_keys[2] = { 0x03, 0x072114 };

#define M4BINGCL_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 ) \
	/* Missing, or not OKI? */

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4BINGCL_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4bingcl_keys>(R5, RT3), mpu4_impcoin, mpu4bwb_machines_state, init_m4 ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4bingcl,       0,          "bc_xe___.2_1", 0x0000, 0x010000, CRC(3abbc215) SHA1(b5e59b30c07c4ffef69c5729f1a28d7ee55636bd), "BWB","Bingo Club (BWB) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4bingcl__a,    m4bingcl,   "bc_xe_b_.2_1", 0x0000, 0x010000, CRC(3e11c5c0) SHA1(2d9bc987fed040664f211bb9d13984b6cba5e25f), "BWB","Bingo Club (BWB) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4bingcl__b,    m4bingcl,   "bc_xe_d_.2_1", 0x0000, 0x010000, CRC(0a3a162f) SHA1(1b0857039b0442269b8fd4c063fc99b3b50cc312), "BWB","Bingo Club (BWB) (MPU4) (set 3)" )


/*****************************************************************************************************************************************************************************
*
* Blues Boys
* - some sets boot
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4blsbys_keys[2] = { 0x42, 0x362709 };
static const uint32_t m4blsbys_g_keys[2] = { 0x36, 0x010e20 };

#define M4BLSBYS_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", 0 ) \
	ROM_LOAD( "bbsnd.p1",  0x000000, 0x080000,  CRC(715c9e95) SHA1(6a0c9c63e56cfc21bf77cf29c1b844b8e0844c1e) ) \
	ROM_LOAD( "bbsnd.p2",  0x080000, 0x080000,  CRC(594a87f8) SHA1(edfef7d08fab41fb5814c92930f08a565371eae1) )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4BLSBYS_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4blsbys_keys>(R5, RT3), mpu4_impcoin, mpu4bwb_machines_state, init_m4big, ROT0, company, title, GAME_FLAGS )

GAME_CUSTOM( 199?, m4blsbys,       0,          "bbprog.bin",   0x0000, 0x020000, CRC(c262cfda) SHA1(f004895e0dd3f8420683927915554e19e41bd20b), "BWB","Blues Boys (BWB) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4blsbys__a,    m4blsbys,   "bf_20a__.3_1", 0x0000, 0x020000, CRC(fca7764f) SHA1(a88378247b6710d6122c515c31c39c5cd9678ce2), "BWB","Blues Boys (BWB) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4blsbys__b,    m4blsbys,   "bf_20a__.6_1", 0x0000, 0x020000, CRC(0822931a) SHA1(8d53321832ee56ed5ad851928ad7705e1ad059ee), "BWB","Blues Boys (BWB) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4blsbys__c,    m4blsbys,   "bf_20s__.3_1", 0x0000, 0x020000, CRC(029a3a0b) SHA1(25952cafbc351ec6d5fc65dd2acddadeb48fb649), "BWB","Blues Boys (BWB) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4blsbys__d,    m4blsbys,   "bf_20s__.6_1", 0x0000, 0x020000, CRC(af7462cb) SHA1(188935ebb574e0b09f9e0e5f094eb99ed7df5075), "BWB","Blues Boys (BWB) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4blsbys__e,    m4blsbys,   "bf_20sb_.3_1", 0x0000, 0x020000, CRC(d2ff1a12) SHA1(d425985a8d109c9e3729618a995eeac1f3bf643c), "BWB","Blues Boys (BWB) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4blsbys__f,    m4blsbys,   "bf_20sb_.6_1", 0x0000, 0x020000, CRC(7548bdf1) SHA1(a366c10f35d85b1f3c103787dd5b2e6600ecf6c8), "BWB","Blues Boys (BWB) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4blsbys__r,    m4blsbys,   "bs_20__c.1_1", 0x0000, 0x020000, CRC(328dc0a6) SHA1(7bea0bac121f2c521d891267fb891479549350aa), "BWB","Blues Boys (BWB) (MPU4) (set 8)" )
GAME_CUSTOM( 199?, m4blsbys__s,    m4blsbys,   "bs_20a__.6_1", 0x0000, 0x020000, CRC(9968e21f) SHA1(8615c8b7f0ae55f0b77f30c84a74d1fba0450a73), "BWB","Blues Boys (BWB) (MPU4) (set 9)" )
GAME_CUSTOM( 199?, m4blsbys__t,    m4blsbys,   "bs_20a__.7_1", 0x0000, 0x020000, CRC(c48bc5d6) SHA1(5aa44d85c1a33ea1aa27e2eaa987ce4fe570b713), "BWB","Blues Boys (BWB) (MPU4) (set 10)" )
GAME_CUSTOM( 199?, m4blsbys__u,    m4blsbys,   "bs_20a_c.1_1", 0x0000, 0x020000, CRC(b9483173) SHA1(a81ef9cb42cd090861b2e0a28a63906cf61c7534), "BWB","Blues Boys (BWB) (MPU4) (set 11)" )
GAME_CUSTOM( 199?, m4blsbys__v,    m4blsbys,   "bs_20a_p.4_1", 0x0000, 0x020000, CRC(fb7ec0aa) SHA1(a0b681a8eacde06825c7e4fcf5b7ef8f64723d96), "BWB","Blues Boys (BWB) (MPU4) (set 12)" )
GAME_CUSTOM( 199?, m4blsbys__w,    m4blsbys,   "bs_20a_s.6_1", 0x0000, 0x020000, CRC(72c5c16e) SHA1(18bdb0f9aff13587d95d871b4124d5a1cc07af04), "BWB","Blues Boys (BWB) (MPU4) (set 13)" )
GAME_CUSTOM( 199?, m4blsbys__x,    m4blsbys,   "bs_20s__.6_1", 0x0000, 0x020000, CRC(1862df89) SHA1(b18f15f2098dfc488a6bdb9a7adff4446268f0d3), "BWB","Blues Boys (BWB) (MPU4) (set 14)" )
GAME_CUSTOM( 199?, m4blsbys__y,    m4blsbys,   "bs_20s_s.6_1", 0x0000, 0x020000, CRC(f3cffcf8) SHA1(bda99c269e85baf64f66788f868879d21a896c5a), "BWB","Blues Boys (BWB) (MPU4) (set 15)" )
GAME_CUSTOM( 199?, m4blsbys__z,    m4blsbys,   "bs_20sb_.6_1", 0x0000, 0x020000, CRC(3278bdcb) SHA1(c694d485c79be983924410ce00d364d233a054c0), "BWB","Blues Boys (BWB) (MPU4) (set 16)" )
GAME_CUSTOM( 199?, m4blsbys__0,    m4blsbys,   "bs_20sbc.1_1", 0x0000, 0x020000, CRC(3a55b728) SHA1(948b353ec92aae1cb801e5e2d2385cd2316f0838), "BWB","Blues Boys (BWB) (MPU4) (set 17)" )
GAME_CUSTOM( 199?, m4blsbys__1,    m4blsbys,   "bs_20sbs.6_1", 0x0000, 0x020000, CRC(d9d59eba) SHA1(91f608dc33541297797f937546fa5759b26af511), "BWB","Blues Boys (BWB) (MPU4) (set 18)" )
GAME_CUSTOM( 199?, m4blsbys__2,    m4blsbys,   "bs_20sd_.6_1", 0x0000, 0x020000, CRC(df087fd9) SHA1(35c85ffe0bef5847a3a72bb33f196f4708f03b28), "BWB","Blues Boys (BWB) (MPU4) (set 19)" )
GAME_CUSTOM( 199?, m4blsbys__3,    m4blsbys,   "bs_20sdc.1_1", 0x0000, 0x020000, CRC(1439c63e) SHA1(fdac1199e98bbecd944431338d2215d434a0c004), "BWB","Blues Boys (BWB) (MPU4) (set 20)" )
GAME_CUSTOM( 199?, m4blsbys__4,    m4blsbys,   "bs_20sds.6_1", 0x0000, 0x020000, CRC(34a55ca8) SHA1(393d0164699a91fcb4eb68ca5246a0744e245873), "BWB","Blues Boys (BWB) (MPU4) (set 21)" )
GAME_CUSTOM( 199?, m4blsbys__5,    m4blsbys,   "bs_25__c.2_1", 0x0000, 0x020000, CRC(d41de6c6) SHA1(43c059ef673de9cc1ef800f3da68b3a6fd54e8f7), "BWB","Blues Boys (BWB) (MPU4) (set 22)" )
GAME_CUSTOM( 199?, m4blsbys__6,    m4blsbys,   "bs_25_bc.2_1", 0x0000, 0x020000, CRC(b692305d) SHA1(4ed96655a4fdfd97fa5783648f8cdea5af0c565e), "BWB","Blues Boys (BWB) (MPU4) (set 23)" )
GAME_CUSTOM( 199?, m4blsbys__7,    m4blsbys,   "bs_25_dc.2_1", 0x0000, 0x020000, CRC(bbe844a5) SHA1(bd1d5d8601b0c36a2f4abaae04a49626cbef23b6), "BWB","Blues Boys (BWB) (MPU4) (set 24)" )
GAME_CUSTOM( 199?, m4blsbys__8,    m4blsbys,   "bs_25a_c.2_1", 0x0000, 0x020000, CRC(0b64cc29) SHA1(43b958321ad5a04aae4629929844643dfcf17819), "BWB","Blues Boys (BWB) (MPU4) (set 25)" )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4BLSBYS_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4blsbys_g_keys>(R5, RT3), mpu4_impcoin, mpu4bwb_machines_state, init_m4big, ROT0, company, title, GAME_FLAGS )

GAME_CUSTOM( 199?, m4blsbys__g,    m4blsbys,   "bs_05___.3v1", 0x0000, 0x020000, CRC(26e8eb95) SHA1(7d8dbca127e1867714cbeb9d699b2173de724eb2), "BWB","Blues Boys (BWB) (MPU4) (set 26)" )
GAME_CUSTOM( 199?, m4blsbys__h,    m4blsbys,   "bs_05__c.3v1", 0x0000, 0x020000, CRC(12e51237) SHA1(68235cf5f36862a26d5d44464041dabb01b9f95c), "BWB","Blues Boys (BWB) (MPU4) (set 27)" )
GAME_CUSTOM( 199?, m4blsbys__i,    m4blsbys,   "bs_05_b_.3v1", 0x0000, 0x020000, CRC(a8ec1731) SHA1(5acd86b1018301df5c3caa388bbbee0c0daacb40), "BWB","Blues Boys (BWB) (MPU4) (set 28)" )
GAME_CUSTOM( 199?, m4blsbys__j,    m4blsbys,   "bs_05_d_.3v1", 0x0000, 0x020000, CRC(5def4142) SHA1(e0019979f65e240002455d5aa41e1492dc517740), "BWB","Blues Boys (BWB) (MPU4) (set 29)" )
GAME_CUSTOM( 199?, m4blsbys__k,    m4blsbys,   "bs_05_k_.3v1", 0x0000, 0x020000, CRC(d023bad9) SHA1(54c995c5166f018c41dd7007163f07f054f564e9), "BWB","Blues Boys (BWB) (MPU4) (set 30)" )
GAME_CUSTOM( 199?, m4blsbys__l,    m4blsbys,   "bs_05_kc.3v1", 0x0000, 0x020000, CRC(0b8911da) SHA1(1426944c01edbf6edb664f961fe75befd3e81e65), "BWB","Blues Boys (BWB) (MPU4) (set 31)" )
GAME_CUSTOM( 199?, m4blsbys__m,    m4blsbys,   "bs_05a__.2_1", 0x0000, 0x020000, CRC(88af47c9) SHA1(e5dfd572b47f2cff6e6ef68bd9016a3e5b2273b2), "BWB","Blues Boys (BWB) (MPU4) (set 32)" )
GAME_CUSTOM( 199?, m4blsbys__n,    m4blsbys,   "bs_05a__.3v1", 0x0000, 0x020000, CRC(055fdf72) SHA1(118272ca35d1ea059b6905a7cd515dbfc11fdbc5), "BWB","Blues Boys (BWB) (MPU4) (set 33)" )
GAME_CUSTOM( 199?, m4blsbys__o,    m4blsbys,   "bs_05a_c.3v1", 0x0000, 0x020000, CRC(737999c4) SHA1(957fd06f7937b85436ef7011d47c559729d17d51), "BWB","Blues Boys (BWB) (MPU4) (set 34)" )
GAME_CUSTOM( 199?, m4blsbys__p,    m4blsbys,   "bs_05b__.3v1", 0x0000, 0x020000, CRC(ea1b09e7) SHA1(9310e3d63b77c4bc2d0d428a630c8aafb500de74), "BWB","Blues Boys (BWB) (MPU4) (set 35)" )
GAME_CUSTOM( 199?, m4blsbys__q,    m4blsbys,   "bs_05b_c.3v1", 0x0000, 0x020000, CRC(3ec0ce9f) SHA1(09472d392350da1593f084a2591125b0d71aca29), "BWB","Blues Boys (BWB) (MPU4) (set 36)" )
GAME_CUSTOM( 199?, m4blsbys__9,    m4blsbys,   "bs_x3a__.2v1", 0x0000, 0x020000, CRC(99471e88) SHA1(e566cd1368e7234ec546b05528f2fcf345e03697), "BWB","Blues Boys (BWB) (MPU4) (set 37)" )
GAME_CUSTOM( 199?, m4blsbys__aa,   m4blsbys,   "bs_x3s__.2v1", 0x0000, 0x020000, CRC(84249d95) SHA1(3962ee9fde49b25c3485c7bfaecd63c400d8502d), "BWB","Blues Boys (BWB) (MPU4) (set 38)" )
GAME_CUSTOM( 199?, m4blsbys__ab,   m4blsbys,   "bs_x6a__.2v1", 0x0000, 0x020000, CRC(d03ef955) SHA1(03fa4b3b37b71fb61439200d5dd65dab846abc2c), "BWB","Blues Boys (BWB) (MPU4) (set 39)" )
GAME_CUSTOM( 199?, m4blsbys__ac,   m4blsbys,   "bs_x6s__.2v1", 0x0000, 0x020000, CRC(61d782b5) SHA1(70ca3875ff023fc091a6fe6e002fad662dbd639f), "BWB","Blues Boys (BWB) (MPU4) (set 40)" )
GAME_CUSTOM( 199?, m4blsbys__ad,   m4blsbys,   "bsix3___.2v1", 0x0000, 0x020000, CRC(4e7451fa) SHA1(b1417f948c7e80f506b90d6608f6dd79739389bf), "BWB","Blues Boys (BWB) (MPU4) (set 41)" )


/*****************************************************************************************************************************************************************************
*
* Championship Soccer
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4csoc_keys[2] = { 0x11, 0x101230 };

#define M4CSOC_EXTRA_ROMS \
	ROM_REGION( 0x180000, "okicard:msm6376", ROMREGION_ERASE00 ) \
	ROM_LOAD( "ch_socc.s1", 0x000000, 0x080000, CRC(abaea3f3) SHA1(cf3b6e4ee99680726efd2a839b49b4d86e2bd270) ) \
	ROM_LOAD( "ch_socc.s2", 0x080000, 0x080000, CRC(2048f5b2) SHA1(b07addfd9d861b1d19d4db248e16c597cf79b159) ) \
	ROM_LOAD( "ch_socc.s3", 0x100000, 0x080000, CRC(064224b0) SHA1(99a8bacfd3a42f72e40b93d1f7eeea633c3cf366) )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent, machine, inputs, init, name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4CSOC_EXTRA_ROMS \
	ROM_END \
	GAME( year, setname, parent, machine, inputs, mpu4bwb_machines_state, init, ROT0, company, title, GAME_FLAGS )


GAME_CUSTOM( 199?, m4csoc,       0,      bwboki_chr_cheat<m4csoc_keys>(R5, RT3), mpu4impcoin_jackpot15_20p, init_m4big ,"sg_sj___.1_0", 0x0000, 0x040000, CRC(f21cd1aa) SHA1(dc010a315a8d738ad9e5e384197499e08a8d5ef6), "BWB",u8"Championship Soccer (BWB) (MPU4) (5/10/20/25/30p stake / £5 jackpot, 20/25/30p stake / £10/£15 jackpot) (set 1)" )
GAME_CUSTOM( 199?, m4csoc__a,    m4csoc, bwboki_chr_cheat<m4csoc_keys>(R5, RT3), mpu4impcoin_jackpot15_20p, init_m4big ,"sg_sj___.2_0", 0x0000, 0x040000, CRC(5513f2a3) SHA1(e9e59461a007be02beae6cd1610b8582d367c15e), "BWB",u8"Championship Soccer (BWB) (MPU4) (5/10/20/25/30p stake / £5 jackpot, 20/25/30p stake / £10/£15 jackpot) (set 2)" )
GAME_CUSTOM( 199?, m4csoc__b,    m4csoc, bwboki_chr_cheat<m4csoc_keys>(R5, RT3), mpu4impcoin_jackpot15_20p, init_m4big ,"sg_sj_d_.2_0", 0x0000, 0x040000, CRC(b0058d0f) SHA1(635c4f729c27c5cb4356f62dcc13127043ea0e5c), "BWB",u8"Championship Soccer (BWB) (MPU4) (5/10/20/25/30p stake / £5 jackpot, 20/25/30p stake / £10/£15 jackpot) (set 3)" ) // Datapak
//
GAME_CUSTOM( 199?, m4csoc__c,    m4csoc, bwboki_chr_cheat<m4csoc_keys>(R5, RT3), mpu4impcoin_jackpot15_20p, init_m4big ,"so_sj___.b_0", 0x0000, 0x040000, CRC(65d5bb2d) SHA1(61c6896d97ed79e2f31b37b9d8998980ceac4fc5), "BWB",u8"Championship Soccer (BWB) (MPU4) (5/10/20/25/30p stake / £5 jackpot, 20/25/30p stake / £15 jackpot) (set 1)" )
GAME_CUSTOM( 199?, m4csoc__d,    m4csoc, bwboki_chr_cheat<m4csoc_keys>(R5, RT3), mpu4impcoin_jackpot15_20p, init_m4big ,"so_sj_d_.b_0", 0x0000, 0x040000, CRC(5e79ba34) SHA1(cb8c689319b5f94ce0385b3b7846a49589358ccc), "BWB",u8"Championship Soccer (BWB) (MPU4) (5/10/20/25/30p stake / £5 jackpot, 20/25/30p stake / £15 jackpot) (set 2)" ) // Datapak
GAME_CUSTOM( 199?, m4csoc__e,    m4csoc, bwboki_chr_cheat<m4csoc_keys>(R5, RT3), mpu4impcoin_jackpot15_20p, init_m4big ,"so_sjs__.b_0", 0x0000, 0x040000, CRC(2f50675e) SHA1(baed8e3a455ec5bfa810e64dc4c66996d6746bbc), "BWB",u8"Championship Soccer (BWB) (MPU4) (5/10/20/25/30p stake / £5 jackpot, 20/25/30p stake / £15 jackpot) (set 3)" ) // Datapak
GAME_CUSTOM( 199?, m4csoc__f,    m4csoc, bwboki_chr_cheat<m4csoc_keys>(R5, RT3), mpu4impcoin_jackpot15_20p, init_m4big ,"ch_socc.5",    0x0000, 0x040000, CRC(1b2ea78d) SHA1(209534ccd537c0ca9d02301830a52ebc29b93cb7), "BWB",u8"Championship Soccer (BWB) (MPU4) (5/10/20/25/30p stake / £5 jackpot, 20/25/30p stake / £15 jackpot) (set 4)" )
//
GAME_CUSTOM( 199?, m4csoc__g,    m4csoc, bwboki_chr_cheat<m4csoc_keys>(R5, RT3), mpu4impcoin_jackpot5_5p,   init_m4big ,"so_vc___.c_0", 0x0000, 0x040000, CRC(d683b202) SHA1(95803008a50229bc85ed177b587fdf05cb152df3), "BWB",u8"Championship Soccer (BWB) (MPU4) (5/10p stake / £5 jackpot) (set 1)" )
GAME_CUSTOM( 199?, m4csoc__h,    m4csoc, bwboki_chr_cheat<m4csoc_keys>(R5, RT3), mpu4impcoin_jackpot5_5p,   init_m4big ,"so_vc_d_.c_0", 0x0000, 0x040000, CRC(ed2fb31b) SHA1(de72d8abbb4a22125ed312e6ccfcab6b3e591ec2), "BWB",u8"Championship Soccer (BWB) (MPU4) (5/10p stake / £5 jackpot) (set 2)" ) // Datapak
//
GAME_CUSTOM( 199?, m4csoc__i,    m4csoc, bwboki_chr_cheat<m4csoc_keys>(R5, RT3), mpu4impcoin_jackpot15_20p, init_m4big ,"ch_socc",      0x0000, 0x040000, CRC(ea9af5bd) SHA1(99319995ee886196ddd540bf37960a4e5b9d4f34), "BWB",u8"Championship Soccer (BWB) (MPU4) (20/25/30p stake / £5/£10/£15 jackpot)" )
//
GAME_CUSTOM( 199?, m4csoc__j,    m4csoc, bwboki_chr_cheat<m4csoc_keys>(R5, RT3), mpu4impcoin_jackpot15_20p, init_m4big ,"chsoc8ac",     0x0000, 0x040000, CRC(8e0471ba) SHA1(3b7e6edbb3490e99af148c0cfe8d39c13c282880), "hack",u8"Championship Soccer (BWB) (MPU4) (5/10/20/25/30p stake / £8 jackpot, 20/25/30p stake / £15 jackpot) (hack)" ) // BWB string at the start of the demo has been blanked out


/*****************************************************************************************************************************************************************************
*
* Cup Final
* - some sets boot
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4cpfinl_keys[2] = { 0x10, 0x241215 };

#define M4CPFINL_EXTRA_ROMS \
	ROM_REGION( 0x180000, "okicard:msm6376", 0 ) \
	ROM_LOAD( "cupsnd_1.0_2", 0x000000, 0x080000, CRC(54384ce8) SHA1(ff78c4ea16722662a480bff1f85af7efe84b01e5) ) \
	ROM_LOAD( "cupsnd_1.0_3", 0x080000, 0x080000, CRC(24d3d848) SHA1(64287c3cbe2e9693954bc880d6edf2bc17b0ed65) )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4CPFINL_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4cpfinl_keys>(R5, RT3), mpu4_impcoin, mpu4bwb_machines_state, init_m4 ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4cpfinl,       0,          "cu_10___.5_1", 0x0000, 0x010000, CRC(47a85443) SHA1(d308b9a6dcb0200f72d5c5b380907d2d55f3e40d), "BWB","Cup Final (BWB) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4cpfinl__a,    m4cpfinl,   "cu_10_b_.5_1", 0x0000, 0x010000, CRC(2583f410) SHA1(447a2316e3c3da6f835699602834f7ca5bafbdf9), "BWB","Cup Final (BWB) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4cpfinl__b,    m4cpfinl,   "cu_10_d_.5_1", 0x0000, 0x010000, CRC(e3bfac76) SHA1(bf1bfa7a995dc4198de890d307718ddb2d9e0092), "BWB","Cup Final (BWB) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4cpfinl__c,    m4cpfinl,   "cu_10_k_.5_1", 0x0000, 0x010000, CRC(81940c25) SHA1(7bb98b0f6355c6cea12c2ffb2751c4dc35042120), "BWB","Cup Final (BWB) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4cpfinl__d,    m4cpfinl,   "cu_20___.5_1", 0x0000, 0x010000, CRC(69a38622) SHA1(65d83c9590bf200077046d564778e7fc7d146030), "BWB","Cup Final (BWB) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4cpfinl__e,    m4cpfinl,   "cu_20_b_.5_1", 0x0000, 0x010000, CRC(0b882671) SHA1(0f3382385c9065eb275d59cb17045a592bdc11cc), "BWB","Cup Final (BWB) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4cpfinl__f,    m4cpfinl,   "cu_20_d_.5_1", 0x0000, 0x010000, CRC(cdb47e17) SHA1(f491049cbd2f5a1f803a3f22ef12f4d41020a928), "BWB","Cup Final (BWB) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4cpfinl__g,    m4cpfinl,   "cu_20_k_.5_1", 0x0000, 0x010000, CRC(af9fde44) SHA1(916295e999ea1017a6f24a73436d97cf89f827c8), "BWB","Cup Final (BWB) (MPU4) (set 8)" )
GAME_CUSTOM( 199?, m4cpfinl__h,    m4cpfinl,   "cui20___.5_1", 0x0000, 0x010000, CRC(5b16231e) SHA1(1bf37843c6e757d7ffe9932eab72b7f49a9ef107), "BWB","Cup Final (BWB) (MPU4) (set 9)" )
GAME_CUSTOM( 199?, m4cpfinl__i,    m4cpfinl,   "cui20_b_.5_1", 0x0000, 0x010000, CRC(393d834d) SHA1(12766cdc1c75ed0d967d7937b9fea2cfb3b6c2c3), "BWB","Cup Final (BWB) (MPU4) (set 10)" )
GAME_CUSTOM( 199?, m4cpfinl__j,    m4cpfinl,   "cui20_d_.5_1", 0x0000, 0x010000, CRC(ff01db2b) SHA1(bd4f826f235e0c0bf53584abc0dc1929ac157d99), "BWB","Cup Final (BWB) (MPU4) (set 11)" )
GAME_CUSTOM( 199?, m4cpfinl__k,    m4cpfinl,   "cui20_k_.5_1", 0x0000, 0x010000, CRC(9d2a7b78) SHA1(d7c26a47dcbb836650f3021733d09a426ff3a390), "BWB","Cup Final (BWB) (MPU4) (set 12)" )


/*****************************************************************************************************************************************************************************
*
* Dancing Diamonds
* - some sets boot
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4danced_keys[2] = { 0x10, 0x3b240d };

#define M4DANCED_EXTRA_ROMS \
	ROM_REGION( 0x180000, "okicard:msm6376", 0 ) \
	ROM_LOAD( "dd______.1_2", 0x0000, 0x080000, CRC(b9043a08) SHA1(5d87a30f23e8b5e3eaa0584d0d49efc08209882b) )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4DANCED_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4danced_keys>(R5, RT3), mpu4_impcoin, mpu4bwb_machines_state, init_m4big ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4danced,       0,          "dd_22bg_.2_1", 0x0000, 0x020000, CRC(f79525a1) SHA1(babfbf8beae423626057235bcad5eae18531160e), "BWB","Dancing Diamonds (BWB) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4danced__a,    m4danced,   "dd_22bg_.4_1", 0x0000, 0x020000, CRC(e50ffa46) SHA1(b42d806422f85573bcbe284b4192f393e3e57306), "BWB","Dancing Diamonds (BWB) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4danced__b,    m4danced,   "dd_22bt_.4_1", 0x0000, 0x020000, CRC(11e910a2) SHA1(6e35ae37dbd12169ccd9cf5b32a3f08f9e3d1899), "BWB","Dancing Diamonds (BWB) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4danced__c,    m4danced,   "dd_25__c.2_1", 0x0000, 0x020000, CRC(1b072e2b) SHA1(cd40d0b7ef29979d65bfabdffee9cf686c6d6f49), "BWB","Dancing Diamonds (BWB) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4danced__d,    m4danced,   "dd_25_bc.2_1", 0x0000, 0x020000, CRC(41f544ec) SHA1(5676f3ca6abc71fdb9286a3b47cb789990298548), "BWB","Dancing Diamonds (BWB) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4danced__e,    m4danced,   "dd_25_dc.2_1", 0x0000, 0x020000, CRC(28b4b2bc) SHA1(8d6294fa4c10fa26322130e6ac1d227b777e8028), "BWB","Dancing Diamonds (BWB) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4danced__f,    m4danced,   "dd_25a_c.2_1", 0x0000, 0x020000, CRC(e243b150) SHA1(8828b73921d49fa9e61ac56325bdc35e60b06d1e), "BWB","Dancing Diamonds (BWB) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4danced__g,    m4danced,   "dd_25b_c.2_1", 0x0000, 0x020000, CRC(4fb0abb1) SHA1(bb8099c90312eb4509b15f098636d6c8e1db5a1d), "BWB","Dancing Diamonds (BWB) (MPU4) (set 8)" )
GAME_CUSTOM( 199?, m4danced__h,    m4danced,   "dd_25bgc.2_1", 0x0000, 0x020000, CRC(7e8c62bf) SHA1(63515ec61ff0e8fb4d62e14059001e2a33906bd6), "BWB","Dancing Diamonds (BWB) (MPU4) (set 9)" )
GAME_CUSTOM( 199?, m4danced__i,    m4danced,   "dd_25btc.2_1", 0x0000, 0x020000, CRC(10f3d837) SHA1(71958987e481fba5c5a43495d0c2f823e21e234c), "BWB","Dancing Diamonds (BWB) (MPU4) (set 10)" )
GAME_CUSTOM( 199?, m4danced__j,    m4danced,   "dd_26a__.5_1", 0x0000, 0x020000, CRC(e854ceb3) SHA1(1bb2eea5d353f62558993f3845bd3d11611045d1), "BWB","Dancing Diamonds (BWB) (MPU4) (set 11)" )
GAME_CUSTOM( 199?, m4danced__k,    m4danced,   "dd_26b__.4_1", 0x0000, 0x020000, CRC(b9bb8e8c) SHA1(9ab2f007e06dae2924e9ca8699dd6d451f09d2d5), "BWB","Dancing Diamonds (BWB) (MPU4) (set 12)" )
GAME_CUSTOM( 199?, m4danced__l,    m4danced,   "dd_26bg_.4_1", 0x0000, 0x020000, CRC(612d9f74) SHA1(52de3d9a8c68818c497021958a64a108a9c131c8), "BWB","Dancing Diamonds (BWB) (MPU4) (set 13)" )
GAME_CUSTOM( 199?, m4danced__m,    m4danced,   "dd_26bt_.4_1", 0x0000, 0x020000, CRC(95cb7590) SHA1(9d7bfaed5b65b73004023a1c27e17be7c844535f), "BWB","Dancing Diamonds (BWB) (MPU4) (set 14)" )
GAME_CUSTOM( 199?, m4danced__n,    m4danced,   "dd_26s__.4_1", 0x0000, 0x020000, CRC(ba1e2ec8) SHA1(c4f5a46a684219bdf531234c3dc06fda4b7c7a3e), "BWB","Dancing Diamonds (BWB) (MPU4) (set 15)" )
GAME_CUSTOM( 199?, m4danced__o,    m4danced,   "dd_26sb_.4_1", 0x0000, 0x020000, CRC(24ea3d54) SHA1(f274ade35a090ef7e2dd611e5611c59a3f5b1cf5), "BWB","Dancing Diamonds (BWB) (MPU4) (set 16)" )
GAME_CUSTOM( 199?, m4danced__p,    m4danced,   "dd_32a__.3_1", 0x0000, 0x020000, CRC(b6f7a984) SHA1(70323926114855071981f3b8f00064b0149824d1), "BWB","Dancing Diamonds (BWB) (MPU4) (set 17)" )
GAME_CUSTOM( 199?, m4danced__q,    m4danced,   "dd_32b__.3_1", 0x0000, 0x020000, CRC(013c05d2) SHA1(a332ddb36c137cc111a629adee60d2d89c5f2d88), "BWB","Dancing Diamonds (BWB) (MPU4) (set 18)" )
GAME_CUSTOM( 199?, m4danced__r,    m4danced,   "dd_32bg_.3_1", 0x0000, 0x020000, CRC(a949d679) SHA1(a282c498b263c6a85323f42d41ac706f44047adf), "BWB","Dancing Diamonds (BWB) (MPU4) (set 19)" )
GAME_CUSTOM( 199?, m4danced__s,    m4danced,   "dd_32bt_.3_1", 0x0000, 0x020000, CRC(f7935bd2) SHA1(cfa65404fa091f445a6b2f30a359739087b7d57b), "BWB","Dancing Diamonds (BWB) (MPU4) (set 20)" )
GAME_CUSTOM( 199?, m4danced__t,    m4danced,   "dd_32s__.3_1", 0x0000, 0x020000, CRC(46fc506e) SHA1(87f2a201e39e78d33ce87b9be44ddf5d5e62106d), "BWB","Dancing Diamonds (BWB) (MPU4) (set 21)" )
GAME_CUSTOM( 199?, m4danced__u,    m4danced,   "dd_32sb_.3_1", 0x0000, 0x020000, CRC(9155deb5) SHA1(5b3ee18fd003e882f80a6c14d01215b4eeb8831e), "BWB","Dancing Diamonds (BWB) (MPU4) (set 22)" )
GAME_CUSTOM( 199?, m4danced__v,    m4danced,   "dd_32sd_.3_1", 0x0000, 0x020000, CRC(d8a91ddc) SHA1(fa515287b456104b7647cc75de9ccb149b051dbd), "BWB","Dancing Diamonds (BWB) (MPU4) (set 23)" )
GAME_CUSTOM( 199?, m4danced__w,    m4danced,   "dd_sja__.2_1", 0x0000, 0x020000, CRC(45db5106) SHA1(b12a5c2c3f61cc78a7ff040e1ffff82c225e6d9e), "BWB","Dancing Diamonds (BWB) (MPU4) (set 24)" )


/*****************************************************************************************************************************************************************************
*
* Daytona
* - Multi-stake game (player chooses the stake, not operator)
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4daytn_keys[2] = { 0x13, 0x1f1e23 };

#define M4DAYTN_EXTRA_ROMS \
	ROM_REGION( 0x180000, "okicard:msm6376", ROMREGION_ERASE00 ) \
	/* missing? */

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4DAYTN_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4daytn_keys>(R5, RT3), mpu4_impcoin, mpu4bwb_machines_state, init_m4big ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4daytn,     0,          "da_78___.1_0", 0x0000, 0x040000, CRC(50beafdd) SHA1(0ef6dd4fc9c8cda596fd383e47b9c7976b5d15f0), "BWB","Daytona (BWB) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4daytn__a,  m4daytn,    "da_78_d_.1_0", 0x0000, 0x040000, CRC(d55d3f9a) SHA1(a145379237947601f2ecb84138c113b71842cd34), "BWB","Daytona (BWB) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4daytn__b,  m4daytn,    "da_80___.1_0", 0x0000, 0x040000, CRC(60f31edd) SHA1(ee016e5001cc80f6796b3a00ceebf14b5fb38ae7), "BWB","Daytona (BWB) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4daytn__c,  m4daytn,    "da_80_d_.1_0", 0x0000, 0x040000, CRC(e5108e9a) SHA1(4d0d530803bd349f57a2a3216fe284e7490a0ae0), "BWB","Daytona (BWB) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4daytn__d,  m4daytn,    "da_82___.1_0", 0x0000, 0x040000, CRC(93a22c97) SHA1(bcb0089c8d2febedd84207851a2c7997e31f8a27), "BWB","Daytona (BWB) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4daytn__e,  m4daytn,    "da_82_d_.1_0", 0x0000, 0x040000, CRC(1641bcd0) SHA1(66e4bc49d6dc8dc1926f3c2faac21498df6082d1), "BWB","Daytona (BWB) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4daytn__f,  m4daytn,    "da_84___.1_0", 0x0000, 0x040000, CRC(5d207c08) SHA1(57d6d5947c611badb1573739995037beb8017774), "BWB","Daytona (BWB) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4daytn__g,  m4daytn,    "da_84_d_.1_0", 0x0000, 0x040000, CRC(d8c3ec4f) SHA1(f3571c36114809dd440cee5d406f3da704797b01), "BWB","Daytona (BWB) (MPU4) (set 8)" )
GAME_CUSTOM( 199?, m4daytn__h,  m4daytn,    "da_86___.1_0", 0x0000, 0x040000, CRC(ae714e42) SHA1(17ec05b35c28968c4319a71b1485052e04c23c0a), "BWB","Daytona (BWB) (MPU4) (set 9)" )
GAME_CUSTOM( 199?, m4daytn__i,  m4daytn,    "da_86_d_.1_0", 0x0000, 0x040000, CRC(2b92de05) SHA1(f59e66c486c51bc0e968fc90009884e6ac851b93), "BWB","Daytona (BWB) (MPU4) (set 10)" )
GAME_CUSTOM( 199?, m4daytn__j,  m4daytn,    "da_88___.1_0", 0x0000, 0x040000, CRC(1b55db77) SHA1(147a36ea76ba30eb465df3e2ad5795d8e3f96d99), "BWB","Daytona (BWB) (MPU4) (set 11)" )
GAME_CUSTOM( 199?, m4daytn__k,  m4daytn,    "da_88_d_.1_0", 0x0000, 0x040000, CRC(9eb64b30) SHA1(f527ef38f9965774f59b5d36f45f801c7d7ce714), "BWB","Daytona (BWB) (MPU4) (set 12)" )
GAME_CUSTOM( 199?, m4daytn__l,  m4daytn,    "da_90___.1_0", 0x0000, 0x040000, CRC(a54d2e81) SHA1(65f08c83dcff2934938a7aa1b56e02ba18ba7898), "BWB","Daytona (BWB) (MPU4) (set 13)" )
GAME_CUSTOM( 199?, m4daytn__m,  m4daytn,    "da_90_d_.1_0", 0x0000, 0x040000, CRC(20aebec6) SHA1(bcf4ca9fa5723fcae0ea661b7cfa005cd0046cb1), "BWB","Daytona (BWB) (MPU4) (set 14)" )
GAME_CUSTOM( 199?, m4daytn__n,  m4daytn,    "da_92_d_.1_0", 0x0000, 0x040000, CRC(9e99647d) SHA1(34cf734808ffbfa9bc920ad1c93c0a9f7bbba791), "BWB","Daytona (BWB) (MPU4) (set 15)" )


/*****************************************************************************************************************************************************************************
*
* Excalibur
* - fails to boot (code crashes!)
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4excal_keys[2] = { 0xff, 0xffffffff };

#define M4EXCAL_EXTRA_ROMS \
	ROM_REGION( 0x180000, "okicard:msm6376", 0 ) \
	ROM_LOAD( "ex______.1_2", 0x000000, 0x080000, CRC(9305e516) SHA1(5a5c67f97761fe2e042ba31594d1881238d3227b) ) \
	ROM_LOAD( "ex______.1_3", 0x080000, 0x080000, CRC(29e3709a) SHA1(2e2f089aa2a938158930f235bf821685932d698b) ) \
	ROM_LOAD( "ex______.1_4", 0x100000, 0x080000, CRC(dd747003) SHA1(cf0a2936c897e3b833984c55f4825c358b723ab8) )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4EXCAL_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4excal_keys>(R5, RT3), mpu4_impcoin, mpu4bwb_machines_state, init_m4big ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4excal,     0,          "ex_05a__.6_1", 0x0000, 0x020000, CRC(317fa289) SHA1(8a0e83a764e2a04285367e0f7ebb814fedc81400), "BWB","Excalibur (BWB) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4excal__a,  m4excal,    "ex_20a_6.6_1", 0x0000, 0x020000, CRC(284937c8) SHA1(3be8bf21ab0ff97f67ce170cee48cd08ea325571), "BWB","Excalibur (BWB) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4excal__b,  m4excal,    "ex_20a_8.6_1", 0x0000, 0x020000, CRC(45fb3559) SHA1(9824d2e6bc8c2249de66aea2422b7a6efd0e37b3), "BWB","Excalibur (BWB) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4excal__c,  m4excal,    "ex_20a_c.7_1", 0x0000, 0x020000, CRC(ab773d70) SHA1(3a5e3aa3978bf0bc9a4924d3225665b583c90abc), "BWB","Excalibur (BWB) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4excal__d,  m4excal,    "ex_20atc.7_1", 0x0000, 0x020000, CRC(ec5c7bc7) SHA1(c57eb69044858b7ea43726d0f49e1c224aeb1377), "BWB","Excalibur (BWB) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4excal__e,  m4excal,    "ex_20s_6.6_1", 0x0000, 0x020000, CRC(44176959) SHA1(fcaad1732176330b382f1690293ed43fa66de084), "BWB","Excalibur (BWB) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4excal__f,  m4excal,    "ex_20s_8.6_1", 0x0000, 0x020000, CRC(29a56bc8) SHA1(7345b8bd4e3cf64ca222c59857ef7df9b1db45c1), "BWB","Excalibur (BWB) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4excal__g,  m4excal,    "ex_20sb6.6_1", 0x0000, 0x020000, CRC(e387c4b9) SHA1(cc83de53a1ffe8d75acb9c2f2ceca9bbff516df7), "BWB","Excalibur (BWB) (MPU4) (set 8)" )
GAME_CUSTOM( 199?, m4excal__h,  m4excal,    "ex_20sb8.6_1", 0x0000, 0x020000, CRC(8e35c628) SHA1(000811c50189cae0f7327b4e7484eb64091a989c), "BWB","Excalibur (BWB) (MPU4) (set 9)" )
GAME_CUSTOM( 199?, m4excal__i,  m4excal,    "ex_20sd6.6_1", 0x0000, 0x020000, CRC(7adbd752) SHA1(d758e202ed75ce3a99f0666c405090107cb0feca), "BWB","Excalibur (BWB) (MPU4) (set 10)" )
GAME_CUSTOM( 199?, m4excal__j,  m4excal,    "ex_20sd8.6_1", 0x0000, 0x020000, CRC(1769d5c3) SHA1(3e5ff338364781e99685cde9e45707dd88d1da11), "BWB","Excalibur (BWB) (MPU4) (set 11)" )


/*****************************************************************************************************************************************************************************
*
* Exotic Fruits
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4exotic_keys[2] = { 0x34, 0x3f0e26 };

#define M4EXOTIC_EXTRA_ROMS \
	ROM_REGION( 0x180000, "okicard:msm6376", ROMREGION_ERASE00 ) \
	/* missing? */

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent, machine, inputs, init, name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4EXOTIC_EXTRA_ROMS \
	ROM_END \
	GAME( year, setname, parent, machine, inputs, mpu4bwb_machines_state, init, ROT0, company, title, GAME_FLAGS )

// don't require stake/jp set
GAME_CUSTOM( 199?, m4exotic,       0,        bwboki_chr_cheat<m4exotic_keys>(R5, RT3), mpu4_impcoin,              init_m4big, "eo_49bm_.2_0", 0x0000, 0x020000, CRC(c748c4ca) SHA1(7d0d498f9edd792ed861c8bf9cf1bb03698d144d), "BWB",u8"Exotic Fruits (BWB) (MPU4) (30p stake / £15 jackpot) (set 1)" )
GAME_CUSTOM( 199?, m4exotic__a,    m4exotic, bwboki_chr_cheat<m4exotic_keys>(R5, RT3), mpu4_impcoin,              init_m4big, "eo_49bmd.2_0", 0x0000, 0x020000, CRC(98436c04) SHA1(1db7c95f7a0297aa3da7f1ce27c790ffa1fa4ebe), "BWB",u8"Exotic Fruits (BWB) (MPU4) (30p stake / £15 jackpot) (set 2)" ) // Datapak
GAME_CUSTOM( 199?, m4exotic__b,    m4exotic, bwboki_chr_cheat<m4exotic_keys>(R5, RT3), mpu4_impcoin,              init_m4big, "eo_49bmd.2g0", 0x0000, 0x020000, CRC(425a0152) SHA1(6a235c613f52c4b8985a589f89542eebd3574fde), "BWB",u8"Exotic Fruits (BWB) (MPU4) (30p stake / £15 jackpot) (set 3)" ) // Datapak
GAME_CUSTOM( 199?, m4exotic__c,    m4exotic, bwboki_chr_cheat<m4exotic_keys>(R5, RT3), mpu4_impcoin,              init_m4big, "eo_49bg_.2_0", 0x0000, 0x020000, CRC(c3bf2286) SHA1(74090fd0a103a6c311d426f4aae8e7af8b1d3bc0), "BWB",u8"Exotic Fruits (BWB) (MPU4) (30p stake / £15 jackpot) (set 4)" ) // Datapak
// require stake/jp set
GAME_CUSTOM( 199?, m4exotic__d,    m4exotic, bwboki_chr_cheat<m4exotic_keys>(R5, RT3), mpu4impcoin_jackpot15_20p, init_m4big, "eo_s9bt_.2g0", 0x0000, 0x020000, CRC(c527d333) SHA1(083d7be95d73d259fe8ec1d87a3a41089a4c44df), "BWB",u8"Exotic Fruits (BWB) (MPU4) (20/25/30p stake / £15 jackpot) (set 1)" )
GAME_CUSTOM( 199?, m4exotic__e,    m4exotic, bwboki_chr_cheat<m4exotic_keys>(R5, RT3), mpu4impcoin_jackpot15_20p, init_m4big, "eo_sja__.2_0", 0x0000, 0x020000, CRC(5ca9557f) SHA1(5fa42c56c67b505272d358a54ebe911fdb0b905e), "BWB",u8"Exotic Fruits (BWB) (MPU4) (20/25/30p stake / £15 jackpot) (set 2)" ) // Datapak


/*****************************************************************************************************************************************************************************
*
* Fire & Ice
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4firice_keys[2] = { 0x11, 0x162413 };

#define M4FIRICE_EXTRA_ROMS \
	ROM_REGION( 0x180000, "okicard:msm6376", ROMREGION_ERASE00 ) \
	ROM_LOAD( "fire_ice.s1", 0x000000, 0x080000, CRC(74ee37c1) SHA1(bc9b419dd1fd1c66090f8946247e754c0267baa3) ) \
	ROM_LOAD( "fire_ice.s2", 0x080000, 0x080000, CRC(b86bafeb) SHA1(ee237f601b970dc5be8096a4018cb6a3edac500f) ) \
	ROM_LOAD( "fire_ice.s3", 0x100000, 0x080000, CRC(75f349b3) SHA1(1505bec7b69e1eabd679b70d95ae58fd264ca698) )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent, machine, inputs, init, name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4FIRICE_EXTRA_ROMS \
	ROM_END \
	GAME( year, setname, parent, machine, inputs, mpu4bwb_machines_state, init, ROT0, company, title, GAME_FLAGS )

// allows you to boot with 50p stake, but still shows '20p'
GAME_CUSTOM( 199?, m4firice,       0,        bwboki_chr_cheat<m4firice_keys>(R5, RT3), mpu4impcoin_jackpot8tkn_20p, init_m4big,  "fi_20__d.5_0", 0x0000, 0x040000, CRC(ab46574c) SHA1(d233b137f8f42b9b644b34a627fbcc5b662e8ae1), "BWB",u8"Fire & Ice (BWB) (MPU4) (20p stake / £8 token jackpot) (set 1)" )
GAME_CUSTOM( 199?, m4firice__a,    m4firice, bwboki_chr_cheat<m4firice_keys>(R5, RT3), mpu4impcoin_jackpot8tkn_20p, init_m4big,  "fi_20_bd.5_0", 0x0000, 0x040000, CRC(9b2bc052) SHA1(34b970659218fde097238b852dadedcb928f69fd), "BWB",u8"Fire & Ice (BWB) (MPU4) (20p stake / £8 token jackpot) (set 2)" ) // Datapak
GAME_CUSTOM( 199?, m4firice__b,    m4firice, bwboki_chr_cheat<m4firice_keys>(R5, RT3), mpu4impcoin_jackpot8tkn_20p, init_m4big,  "fi_20_dd.5_0", 0x0000, 0x040000, CRC(2bbc9855) SHA1(84d51eeadc01ac74d630a05b933343f01f04b2af), "BWB",u8"Fire & Ice (BWB) (MPU4) (20p stake / £8 token jackpot) (set 3)" ) // Datapak
GAME_CUSTOM( 199?, m4firice__c,    m4firice, bwboki_chr_cheat<m4firice_keys>(R5, RT3), mpu4impcoin_jackpot8tkn_20p, init_m4big,  "fi_20_kd.5_0", 0x0000, 0x040000, CRC(529b43b1) SHA1(80d4d928918fdc869b1693e21a5c25045e5c9449), "BWB",u8"Fire & Ice (BWB) (MPU4) (20p stake / £8 token jackpot) (set 4)" ) // % key
GAME_CUSTOM( 199?, m4firice__d,    m4firice, bwboki_chr_cheat<m4firice_keys>(R5, RT3), mpu4impcoin_jackpot8tkn_20p, init_m4big,  "fi_20a_d.5_0", 0x0000, 0x040000, CRC(8d6ce79d) SHA1(05954ed4b34af73c065b6203a50da9af7d8373fe), "BWB",u8"Fire & Ice (BWB) (MPU4) (20p stake / £8 token jackpot) (set 5)" )
GAME_CUSTOM( 199?, m4firice__e,    m4firice, bwboki_chr_cheat<m4firice_keys>(R5, RT3), mpu4impcoin_jackpot8tkn_20p, init_m4big,  "fi_20s_d.5_0", 0x0000, 0x040000, CRC(d0aa53af) SHA1(f71801344c17a759ec4eb8958377bbcf4b4cae65), "BWB",u8"Fire & Ice (BWB) (MPU4) (20p stake / £8 token jackpot) (set 6)" ) // Datapak
//
GAME_CUSTOM( 199?, m4firice__f,    m4firice, bwboki_chr_cheat<m4firice_keys>(R5, RT3), mpu4impcoin_jackpot10_20p,   init_m4big,  "fi_sj___.e_0", 0x0000, 0x040000, CRC(7f12e37a) SHA1(fb09ff782f66972b8bdeff105c5f3d1f9f676809), "BWB",u8"Fire & Ice (BWB) (MPU4) (20/25/30p stake / £10/£15 jackpot) (set 1)" )
GAME_CUSTOM( 199?, m4firice__g,    m4firice, bwboki_chr_cheat<m4firice_keys>(R5, RT3), mpu4impcoin_jackpot10_20p,   init_m4big,  "fi_sj_b_.e_0", 0x0000, 0x040000, CRC(5aef48d2) SHA1(73f410951a737f75f3e7c14e704eca9c26cfa750), "BWB",u8"Fire & Ice (BWB) (MPU4) (20/25/30p stake / £10/£15 jackpot) (set 2)" ) // Datapak
GAME_CUSTOM( 199?, m4firice__h,    m4firice, bwboki_chr_cheat<m4firice_keys>(R5, RT3), mpu4impcoin_jackpot10_20p,   init_m4big,  "fi_sj_d_.e_0", 0x0000, 0x040000, CRC(61822af2) SHA1(8c721229a5ce9f491cbc638b8c5fa5c0c3032700), "BWB",u8"Fire & Ice (BWB) (MPU4) (20/25/30p stake / £10/£15 jackpot) (set 3)" ) // Datapak
GAME_CUSTOM( 199?, m4firice__i,    m4firice, bwboki_chr_cheat<m4firice_keys>(R5, RT3), mpu4impcoin_jackpot10_20p,   init_m4big,  "fi_sj_k_.e_0", 0x0000, 0x040000, CRC(2b57036b) SHA1(60cec130770ff643af1148f16a3afe3b102e94e2), "BWB",u8"Fire & Ice (BWB) (MPU4) (20/25/30p stake / £10/£15 jackpot) (set 4)" ) // % key
GAME_CUSTOM( 199?, m4firice__j,    m4firice, bwboki_chr_cheat<m4firice_keys>(R5, RT3), mpu4impcoin_jackpot10_20p,   init_m4big,  "fi_sja__.e_0", 0x0000, 0x040000, CRC(da5e0eff) SHA1(9f5ddce366786bdf898c9410be417c8028cebeb4), "BWB",u8"Fire & Ice (BWB) (MPU4) (20/25/30p stake / £10/£15 jackpot) (set 5)" )

/*****************************************************************************************************************************************************************************
*
* Harlequin
* - some sets boot
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4harle_keys[2] = { 0x10, 0x220525 };

#define M4HARLE_EXTRA_ROMS \
	ROM_REGION( 0x180000, "okicard:msm6376", ROMREGION_ERASE00 ) \
	ROM_LOAD( "hq______.1_2", 0x000000, 0x080000, CRC(0c8c14be) SHA1(a1268e71ea43772a532b63327f24c64fabd7e715) ) \
	ROM_LOAD( "hq______.1_3", 0x080000, 0x080000, CRC(5a07e514) SHA1(6e589756c0fc4b0458ca856e918fa3b7cd396c39) )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4HARLE_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4harle_keys>(R5, RT3), mpu4_impcoin, mpu4bwb_machines_state, init_m4 ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4harle,     0,          "hq_20a__.1_1", 0x0000, 0x010000, CRC(b8ae3025) SHA1(94a449eff103bf6ba1fc6e85b03061b9ce658ae0), "BWB","Harlequin (BWB) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4harle__a,  m4harle,    "hq_20bg_.1_1", 0x0000, 0x010000, CRC(31356248) SHA1(d8791b1c861ed4388660bbe78f2589db7f1e779e), "BWB","Harlequin (BWB) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4harle__b,  m4harle,    "hq_20bg_.2a1", 0x0000, 0x010000, CRC(f50898af) SHA1(ba4470abb85b92d647ff8da48dba571cec5f594e), "BWB","Harlequin (BWB) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4harle__c,  m4harle,    "hq_20bt_.1_1", 0x0000, 0x010000, CRC(925a5e9e) SHA1(e8a4f75d4f7e6825894e19a915db21ee21206b5b), "BWB","Harlequin (BWB) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4harle__d,  m4harle,    "hq_20s__.1_1", 0x0000, 0x010000, CRC(6de12c08) SHA1(df279a07ccd448b9a6948ccb059aff6306ddcc99), "BWB","Harlequin (BWB) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4harle__e,  m4harle,    "hq_20sb_.1_1", 0x0000, 0x010000, CRC(0fca8c5b) SHA1(f430f2d1ff71513426bb5cfed883797de6afd3a7), "BWB","Harlequin (BWB) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4harle__f,  m4harle,    "hq_20sd_.1_1", 0x0000, 0x010000, CRC(c9f6d43d) SHA1(4eef4ec6bff20fdf5b20b266ddd1646045390d42), "BWB","Harlequin (BWB) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4harle__g,  m4harle,    "hq_20sk_.1_1", 0x0000, 0x010000, CRC(abdd746e) SHA1(5bb7043a1a790e5c784c7a51fd3c023b3099439f), "BWB","Harlequin (BWB) (MPU4) (set 8)" )
GAME_CUSTOM( 199?, m4harle__h,  m4harle,    "hq_xea__.2f1", 0x0000, 0x010000, CRC(4ef7c19c) SHA1(bd8e5c69ab31c5a9c89a7f2020a61bde5e00b6b2), "BWB","Harlequin (BWB) (MPU4) (set 9)" )
GAME_CUSTOM( 199?, m4harle__i,  m4harle,    "hq_xea__.2s1", 0x0000, 0x010000, CRC(05d42107) SHA1(e1dc4e2f84ebadead994a09c53c2d9a1ed8e29aa), "BWB","Harlequin (BWB) (MPU4) (set 10)" )
GAME_CUSTOM( 199?, m4harle__j,  m4harle,    "hq_xes__.2f1", 0x0000, 0x010000, CRC(9bb8ddb1) SHA1(c8bba9bdfd7a0bbcbe89cd422d37909f7ebcaa82), "BWB","Harlequin (BWB) (MPU4) (set 11)" )
GAME_CUSTOM( 199?, m4harle__k,  m4harle,    "hq_xes__.2s1", 0x0000, 0x010000, CRC(d09b3d2a) SHA1(4de366ed5eae89f2bb39abc052d88cb59e846307), "BWB","Harlequin (BWB) (MPU4) (set 12)" )
GAME_CUSTOM( 199?, m4harle__l,  m4harle,    "hq_xesd_.2a1", 0x0000, 0x010000, CRC(b402aa45) SHA1(6c6eed7b172604112a2c00df9fd00476d07cc971), "BWB","Harlequin (BWB) (MPU4) (set 13)" )
GAME_CUSTOM( 199?, m4harle__m,  m4harle,    "hq_xesd_.2f1", 0x0000, 0x010000, CRC(3faf2584) SHA1(01b049b6ae44771f37b298dc525e16b6e9a182f2), "BWB","Harlequin (BWB) (MPU4) (set 14)" )
GAME_CUSTOM( 199?, m4harle__n,  m4harle,    "hq_xesd_.2s1", 0x0000, 0x010000, CRC(748cc51f) SHA1(d2ac7c237be40564809b006bf68d09560817f97d), "BWB","Harlequin (BWB) (MPU4) (set 15)" )
GAME_CUSTOM( 199?, m4harle__o,  m4harle,    "ph_20a__.2s1", 0x0000, 0x010000, CRC(559bf168) SHA1(3c6a47bba52481af3f987d284a2102a8ee2cc7e6), "BWB","Harlequin (BWB) (MPU4) (set 16)" )
GAME_CUSTOM( 199?, m4harle__p,  m4harle,    "ph_20bg_.1_1", 0x0000, 0x010000, CRC(4c96d2fa) SHA1(1ff8c8c5dc6a67ac187ba0b96cd93c786884aa3b), "BWB","Harlequin (BWB) (MPU4) (set 17)" )
GAME_CUSTOM( 199?, m4harle__q,  m4harle,    "ph_20bt_.1_1", 0x0000, 0x010000, CRC(eff9ee2c) SHA1(ae15ee4bbc3028580b3c6cbc8a078fbe3291283e), "BWB","Harlequin (BWB) (MPU4) (set 18)" )
GAME_CUSTOM( 199?, m4harle__r,  m4harle,    "ph_20s__.1_1", 0x0000, 0x010000, CRC(4cccba32) SHA1(6d2ec2324866555dfa3f3fd5b79ff3883e1b2ebe), "BWB","Harlequin (BWB) (MPU4) (set 19)" )
GAME_CUSTOM( 199?, m4harle__s,  m4harle,    "ph_20s__.2s1", 0x0000, 0x010000, CRC(80d4ed45) SHA1(c818f3e154cfa1efcd11efb71061aa5bfc2e668f), "BWB","Harlequin (BWB) (MPU4) (set 20)" )
GAME_CUSTOM( 199?, m4harle__t,  m4harle,    "ph_20sb_.1_1", 0x0000, 0x010000, CRC(2ee71a61) SHA1(0e91cc5b22899f9765cabb9416fcafed790951ae), "BWB","Harlequin (BWB) (MPU4) (set 21)" )
GAME_CUSTOM( 199?, m4harle__u,  m4harle,    "ph_20sb_.2f1", 0x0000, 0x010000, CRC(1f56841a) SHA1(dfd90d8af765bc981d0dcb4b0ffb5ce613bfcbed), "BWB","Harlequin (BWB) (MPU4) (set 22)" )
GAME_CUSTOM( 199?, m4harle__v,  m4harle,    "ph_20sd_.1_1", 0x0000, 0x010000, CRC(e8db4207) SHA1(bd806e5b04207b3121284c485a6ab8a385231504), "BWB","Harlequin (BWB) (MPU4) (set 23)" )
GAME_CUSTOM( 199?, m4harle__w,  m4harle,    "ph_20sd_.2s1", 0x0000, 0x010000, CRC(24c31570) SHA1(a6da77ebd80dc234add5da880e3472f2f1e2ca3d), "BWB","Harlequin (BWB) (MPU4) (set 24)" )
GAME_CUSTOM( 199?, m4harle__x,  m4harle,    "ph_20sk_.1_1", 0x0000, 0x010000, CRC(8af0e254) SHA1(b7d80ab84684bdabe169623864b9efd0d3881f2e), "BWB","Harlequin (BWB) (MPU4) (set 25)" )


/*****************************************************************************************************************************************************************************
*
* Heaven & Hell
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4hvhel_keys[2] = { 0x11, 0x3f0b26 };

#define M4HVHEL_EXTRA_ROMS \
	ROM_REGION( 0x180000, "okicard:msm6376", 0 ) \
	ROM_LOAD( "hh___snd.1_1", 0x000000, 0x080000, CRC(afa7ba60) SHA1(25278046252e49364d4a51de79295b87baf6018e) ) \
	ROM_LOAD( "hh___snd.1_2", 0x080000, 0x080000, CRC(ec1ec822) SHA1(3fdee0526cb70f4951b7bbced74e32641ded9b7b) ) \
	ROM_LOAD( "hh___snd.1_3", 0x100000, 0x080000, CRC(d4119155) SHA1(b61c71e1ee0dbfc0bb9eff1a8c019cf11731ee11) )


#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent, machine, inputs, init, name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4HVHEL_EXTRA_ROMS \
	ROM_END \
	GAME( year, setname, parent, machine, inputs, mpu4bwb_machines_state, init, ROT0, company, title, GAME_FLAGS )

GAME_CUSTOM( 199?, m4hvhel,     0,       bwboki_chr_cheat<m4hvhel_keys>(R5, RT3), mpu4impcoin_jackpot8tkn_20p, init_m4big,   "hh_20__d.2_0",     0x0000, 0x040000, CRC(801de788) SHA1(417b985714d8f0ebed93b65a3f865e03474ce9e5), "BWB", u8"Heaven & Hell (BWB) (MPU4) (20p stake / £8 token jackpot, set 1)" )
GAME_CUSTOM( 199?, m4hvhel__a,  m4hvhel, bwboki_chr_cheat<m4hvhel_keys>(R5, RT3), mpu4impcoin_jackpot8tkn_20p, init_m4big,   "hh_20a_d.2_0",     0x0000, 0x040000, CRC(ea4e7876) SHA1(5bf711c2bdff50fe745edefa0eebf719824d9e5b), "BWB", u8"Heaven & Hell (BWB) (MPU4) (20p stake / £8 token jackpot, set 2)" )
GAME_CUSTOM( 199?, m4hvhel__b,  m4hvhel, bwboki_chr_cheat<m4hvhel_keys>(R5, RT3), mpu4impcoin_jackpot8tkn_20p, init_m4big,   "hh_20s_d.2_0",     0x0000, 0x040000, CRC(a519a441) SHA1(f3c19d316c82d1ebbcfdabb6d4eaa6cfa369d287), "BWB", u8"Heaven & Hell (BWB) (MPU4) (20p stake / £8 token jackpot, set 3)" ) // Datapak
//
GAME_CUSTOM( 199?, m4hvhel__c,  m4hvhel, bwboki_chr_cheat<m4hvhel_keys>(R5, RT3), mpu4impcoin_jackpot10_20p,   init_m4big,   "hh_sj___",         0x0000, 0x040000, CRC(04577b99) SHA1(48689c3a96bc42ad64dc4d363dad38c967f0cdcc), "BWB", u8"Heaven & Hell (BWB) (MPU4) (20/25/30p stake / £10/£15 jackpot)" )
//
GAME_CUSTOM( 199?, m4hvhel__d,  m4hvhel, bwboki_chr_cheat<m4hvhel_keys>(R5, RT3), mpu4impcoin_jackpot15_20p,   init_m4big,   "hh_sj___.f_0",     0x0000, 0x040000, CRC(8ab33720) SHA1(0c9283a20c3f008baa8ce027d1266e4ef49ca56b), "BWB", u8"Heaven & Hell (BWB) (MPU4) (5/10/20/25/30p stake / £5 jackpot, 20/25/30p stake / £15 jackpot) (set 1)" )
GAME_CUSTOM( 199?, m4hvhel__e,  m4hvhel, bwboki_chr_cheat<m4hvhel_keys>(R5, RT3), mpu4impcoin_jackpot15_20p,   init_m4big,   "hh_sjs__.f_0",     0x0000, 0x040000, CRC(8854763d) SHA1(323bd76a014e52e3b12427998b0e2851463246c8), "BWB", u8"Heaven & Hell (BWB) (MPU4) (5/10/20/25/30p stake / £5 jackpot, 20/25/30p stake / £15 jackpot) (set 2)" ) // Datapak
GAME_CUSTOM( 199?, m4hvhel__f,  m4hvhel, bwboki_chr_cheat<m4hvhel_keys>(R5, RT3), mpu4impcoin_jackpot15_20p,   init_m4big,   "h_hell._pound5",   0x0000, 0x040000, CRC(cd59c0d0) SHA1(8caad9043a277fa39a3ad2d5ec3388c121e7f697), "BWB", u8"Heaven & Hell (BWB) (MPU4) (5/10/20/25/30p stake / £5 jackpot, 20/25/30p stake / £15 jackpot) (set 3)" )
//
GAME_CUSTOM( 199?, m4hvhel__g,  m4hvhel, bwboki_chr_cheat<m4hvhel_keys>(R5, RT3), mpu4impcoin_jackpot5_5p,     init_m4big,   "hh_vc___.g_0",     0x0000, 0x040000, CRC(db338fb7) SHA1(e7e92293374721e7360493e9ef189991dad0a1ee), "BWB", u8"Heaven & Hell (BWB) (MPU4) (5/10p stake / £5 jackpot, set 1)" )
GAME_CUSTOM( 199?, m4hvhel__h,  m4hvhel, bwboki_chr_cheat<m4hvhel_keys>(R5, RT3), mpu4impcoin_jackpot5_5p,     init_m4big,   "hh_vc_d_.g_0",     0x0000, 0x040000, CRC(292468bd) SHA1(f9b19f57a49c1afd670c68b7acd85d4141adfce1), "BWB", u8"Heaven & Hell (BWB) (MPU4) (5/10p stake / £5 jackpot, set 2)" ) // Datapak


/*****************************************************************************************************************************************************************************
*
* Indy Cars
* - note, the GAL dumps appear to be characterisers, are the equations valid?
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4indycr_keys[2] = { 0x13, 0x102233 };

#define M4INDYCR_EXTRA_ROMS \
	ROM_REGION( 0x200000, "gals", 0 ) \
	ROM_LOAD( "ic___.g", 0x0000, 0x000657, CRC(25cc2b32) SHA1(a065276fe4cc1e55f366ac216b5bc8cf934da9dd) ) \
	ROM_LOAD( "ic___.p", 0x0000, 0x00058b, CRC(26388ec4) SHA1(e33183d0c3e75fe23ab6df01d561f5b65923d1db) ) \
	ROM_REGION( 0x200000, "okicard:msm6376", 0 ) \
	ROM_LOAD( "da______.1_1", 0x000000, 0x080000, CRC(99d36c12) SHA1(f8848a28b5546649d6a3f8599dbc4ca84bdac77c) ) \
	ROM_LOAD( "da______.1_2", 0x080000, 0x080000, CRC(32b40094) SHA1(f02c3b088d76116f817b536cf7cec5188b2f73bf) ) \
	ROM_LOAD( "da______.1_3", 0x100000, 0x080000, CRC(2df33d18) SHA1(40afa32d6c72c6a76e3e2e61db19a16003f4e176) ) \
	ROM_LOAD( "da______.1_4", 0x180000, 0x080000, CRC(8e254a3b) SHA1(bc3643ea5878bbde110ee6971c5149b3320bcffc) )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent, machine, inputs, init, name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4INDYCR_EXTRA_ROMS \
	ROM_END \
	GAME( year, setname, parent, machine, inputs, mpu4bwb_machines_state, init, ROT0, company, title, GAME_FLAGS )


GAME_CUSTOM( 199?, m4indycr,       0,        bwboki_chr_cheat<m4indycr_keys>(R5, RT3), mpu4impcoin_jackpot15_20p,      init_m4big,  "ic_sj___.1_0", 0x0000, 0x040000, CRC(4dea0d17) SHA1(4fa19896dbb5e8f21ac7e74efc56de5cadd5bf54), u8"BWB","Indy Cars (BWB) (MPU4) (20/25/30p stake / £5/£15 jackpot) (set 1)" )
GAME_CUSTOM( 199?, m4indycr__a,    m4indycr, bwboki_chr_cheat<m4indycr_keys>(R5, RT3), mpu4impcoin_jackpot15_20p,      init_m4big,  "ic_sj_b_.1_0", 0x0000, 0x040000, CRC(4bc0cb73) SHA1(d4c048ba9578add0104f0c529f20356c3502ea71), u8"BWB","Indy Cars (BWB) (MPU4) (20/25/30p stake / £5/£15 jackpot) (set 2)" ) // Datapak
GAME_CUSTOM( 199?, m4indycr__b,    m4indycr, bwboki_chr_cheat<m4indycr_keys>(R5, RT3), mpu4impcoin_jackpot15_20p,      init_m4big,  "ic_sj_d_.1_0", 0x0000, 0x040000, CRC(165ad977) SHA1(daa444e0d128859832094d3b07026483cd3466ce), u8"BWB","Indy Cars (BWB) (MPU4) (20/25/30p stake / £5/£15 jackpot) (set 3)" ) // Datapak
GAME_CUSTOM( 199?, m4indycr__c,    m4indycr, bwboki_chr_cheat<m4indycr_keys>(R5, RT3), mpu4impcoin_jackpot15_20p_90pc, init_m4big,  "ic_sj_k_.1_0", 0x0000, 0x040000, CRC(857fda64) SHA1(3eb230ea1adf9acb4cf83422c4bb1cde40756310), u8"BWB","Indy Cars (BWB) (MPU4) (20/25/30p stake / £5/£15 jackpot) (set 4)" ) // % key
GAME_CUSTOM( 199?, m4indycr__d,    m4indycr, bwboki_chr_cheat<m4indycr_keys>(R5, RT3), mpu4impcoin_jackpot15_20p,      init_m4big,  "ic_sjs__.1_0", 0x0000, 0x040000, CRC(6310b904) SHA1(0f2cd7ed83f77423bcfb2a71144fab2047dfea13), u8"BWB","Indy Cars (BWB) (MPU4) (20/25/30p stake / £5/£15 jackpot) (set 5)" ) // Datapak
//
GAME_CUSTOM( 199?, m4indycr__e,    m4indycr, bwboki_chr_cheat<m4indycr_keys>(R5, RT3), mpu4impcoin_jackpot15_20p,      init_m4big,  "ic_sj___.2_0", 0x0000, 0x040000, CRC(6d0ddf54) SHA1(0985aa9fddb71a499d266c12893aabbab8755319), u8"BWB","Indy Cars (BWB) (MPU4) (5/10/20/25/30p stake / £5 jackpot, 20/25/30p stake / £15 jackpot) (set 1)" )
GAME_CUSTOM( 199?, m4indycr__f,    m4indycr, bwboki_chr_cheat<m4indycr_keys>(R5, RT3), mpu4impcoin_jackpot15_20p,      init_m4big,  "ic_sj_d_.2_0", 0x0000, 0x040000, CRC(36bd0b34) SHA1(306d6e6536a4137353f9b895e64c7e9a5c79561a), u8"BWB","Indy Cars (BWB) (MPU4) (5/10/20/25/30p stake / £5 jackpot, 20/25/30p stake / £15 jackpot) (set 2)" ) // Datapak

/*****************************************************************************************************************************************************************************
*
* Jackpot Jokers
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4jakjok_keys[2] = { 0x11, 0x103218 };

#define M4JAKJOK_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", 0 ) \
	ROM_LOAD( "jj_____.1_1", 0x000000, 0x080000, CRC(e759a958) SHA1(b107f4ef5a2805e56d4024940bfc632155de1eb1) ) \
	ROM_LOAD( "jj_____.1_2", 0x080000, 0x080000, CRC(aa215ff0) SHA1(4bf2c6f8153730cc3ca86f78ec14063ece7d8700) ) \
	ROM_LOAD( "jj_____.1_3", 0x100000, 0x080000, CRC(03c0ffc3) SHA1(2572f62362325df8b235b487d4a764218e7f1589) )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent, machine, inputs, init, name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4JAKJOK_EXTRA_ROMS \
	ROM_END \
	GAME( year, setname, parent, machine, inputs, mpu4bwb_machines_state, init, ROT0, company, title, GAME_FLAGS )

GAME_CUSTOM( 2000, m4jakjok,       0,        bwboki_chr_cheat<m4jakjok_keys>(R5, RT3), mpu4impcoin_jackpot15_20p,      init_m4big,  "jj_sj___.6_0", 0x0000, 0x040000, CRC(7bc45b0e) SHA1(f30fef8fccdac04859f1ff93198a497eff723020), "BWB",u8"Jackpot Jokers (BWB) (MPU4) (ver. 6) (5/10/20/25/30p stake / £5 jackpot, 20/25/30p stake / £15 jackpot)" )
//
GAME_CUSTOM( 1998, m4jakjok__a,    m4jakjok, bwboki_chr_cheat<m4jakjok_keys>(R5, RT3), mpu4impcoin_jackpot15_20p_90pc, init_m4big,  "jj_sj_k_.3_0", 0x0000, 0x040000, CRC(c33dd82f) SHA1(c1f3f6ca1c45503b7f71e897e5c27368f5efb439), "BWB",u8"Jackpot Jokers (BWB) (MPU4) (ver. 3) (20/25/30p stake / £5/£15 jackpot)" ) // % key
GAME_CUSTOM( 2000, m4jakjok__b,    m4jakjok, bwboki_chr_cheat<m4jakjok_keys>(R5, RT3), mpu4impcoin_jackpot15_20p,      init_m4big,  "jj_sjs__.6_0", 0x0000, 0x040000, CRC(4bcac6f5) SHA1(7dc07a7a61a6ba044020d6c2496143168c103a70), "BWB",u8"Jackpot Jokers (BWB) (MPU4) (ver. 6) (20/25/30p stake / £5/£15 jackpot)" ) // Datapak
//
GAME_CUSTOM( 2000, m4jakjok__c,    m4jakjok, bwboki_chr_cheat<m4jakjok_keys>(R5, RT3), mpu4impcoin_jackpot5_5p,        init_m4big,  "jj_vc___.7_0", 0x0000, 0x040000, CRC(4cdca8da) SHA1(ee7448b12380416a3bea2713ed5feca7473be8aa), "BWB",u8"Jackpot Jokers (BWB) (MPU4) (ver. 7) (5/10p stake / £5 jackpot)" )

#define M4JAKJOKA_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", 0 ) \
	ROM_LOAD( "j_joker.s1", 0x0000, 0x080000, CRC(4ad711d2) SHA1(500381ac2a5075acd606a131cd1b382342cc3a80) ) \
	ROM_LOAD( "j_joker.s2", 0x0000, 0x080000, CRC(840ba491) SHA1(f7f43d5d8e521a59fdccbd5f22935c525c3d43c2) ) \
	ROM_LOAD( "j_joker.s3", 0x0000, 0x080000, CRC(2ed74890) SHA1(a3d039b4c3c9dd792300eb045e542a212d4d50ae) )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent, machine, inputs, init, name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4JAKJOKA_EXTRA_ROMS \
	ROM_END \
	GAME( year, setname, parent, machine, inputs, mpu4bwb_machines_state, init, ROT0, company, title, GAME_FLAGS )

GAME_CUSTOM( 1998, m4jakjoka, m4jakjok, bwboki_chr_cheat<m4jakjok_keys>(R5, RT3), mpu4impcoin_jackpot15_20p, init_m4big,  "j_joker", 0x0000, 0x040000, CRC(4f0c7ab8) SHA1(af962863ee55f6c2752bbe8a997e3b2102e42431), "BWB",u8"Jackpot Jokers (BWB) (MPU4) (ver. 2) (20/25/30p stake / £5/£15 jackpot)" )


/*****************************************************************************************************************************************************************************
*
* Jumping Jack Flash
* - some sets boot
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4jflash_keys[2] = { 0x10, 0x3c3801 };

#define M4JFLASH_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 ) \
	/* missing? */

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4JFLASH_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4jflash_keys>(R5, RT3), mpu4_impcoin, mpu4bwb_machines_state, init_m4big ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4jflash,       0,          "jf_25__c.2_1", 0x0000, 0x020000, CRC(4d5f1a12) SHA1(c25b6d899b74231da505bde7b671be001bdcea5d), "BWB","Jumping Jack Flash (BWB) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4jflash__a,    m4jflash,   "jf_25a_c.2_1", 0x0000, 0x020000, CRC(76722e15) SHA1(4bd107049ad98b848cdaba3a1318373bbd06ab9f), "BWB","Jumping Jack Flash (BWB) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4jflash__b,    m4jflash,   "jf_25b_c.2_1", 0x0000, 0x020000, CRC(35a927c6) SHA1(6776fe77ad8a85feecdedfad0eac89f9cb826fbf), "BWB","Jumping Jack Flash (BWB) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4jflash__c,    m4jflash,   "jf_25bdc.2_1", 0x0000, 0x020000, CRC(d372689e) SHA1(ecad53022c7786f387586484a3e679afbf0bac37), "BWB","Jumping Jack Flash (BWB) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4jflash__d,    m4jflash,   "jf_25bgc.2_1", 0x0000, 0x020000, CRC(421084bc) SHA1(b6f847468c20a3f85d9e77b633dc48adb43c970f), "BWB","Jumping Jack Flash (BWB) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4jflash__e,    m4jflash,   "jf_25btc.2_1", 0x0000, 0x020000, CRC(1c701822) SHA1(559b4d269a3af388aa86f28ccedd22505fcdb355), "BWB","Jumping Jack Flash (BWB) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4jflash__f,    m4jflash,   "jf_25d_c.2_1", 0x0000, 0x020000, CRC(9cde7bfc) SHA1(a0b840f00c487e963f4dd9f58e3abca7b0cea31b), "BWB","Jumping Jack Flash (BWB) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4jflash__g,    m4jflash,   "jf_25dkc.2_1", 0x0000, 0x020000, CRC(ef583053) SHA1(23b33f0e49d8efceb4a6690ac58da1ccf6576a1a), "BWB","Jumping Jack Flash (BWB) (MPU4) (set 8)" )
GAME_CUSTOM( 199?, m4jflash__h,    m4jflash,   "jf_25k_c.2_1", 0x0000, 0x020000, CRC(d82701b8) SHA1(a784fa11877e05f2219f1452463ec3348c84e879), "BWB","Jumping Jack Flash (BWB) (MPU4) (set 9)" )
GAME_CUSTOM( 199?, m4jflash__i,    m4jflash,   "jf_25sbc.2_1", 0x0000, 0x020000, CRC(c6bce1c6) SHA1(708002059d307a02fec32a3cdb6eff995a438631), "BWB","Jumping Jack Flash (BWB) (MPU4) (set 10)" )


/*****************************************************************************************************************************************************************************
*
* Lucky Number 7
* - some sets boot
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4ln7_keys[2] = { 0x10, 0x050107 };

#define M4LN7_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", 0 ) \
	ROM_LOAD( "l7______.1_2", 0x000000, 0x080000, CRC(216209e3) SHA1(af274a7f27ba0e7ac03400e9919537ab36464e64) ) \
	ROM_LOAD( "l7______.1_3", 0x080000, 0x080000, CRC(e909c3ec) SHA1(68ce743729aaefd6c20ee447af40d99e0f4c072b) )
#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4LN7_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4ln7_keys>(R5, RT3), mpu4_impcoin, mpu4bwb_machines_state, init_m4 ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4ln7,     0,      "l7_20a__.1_1", 0x0000, 0x010000, CRC(bfe82d2a) SHA1(4477d737a2326602a355758d8fc06220312fc085), "BWB","Lucky Number 7 (BWB) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4ln7__a,  m4ln7,  "l7_20s__.1_1", 0x0000, 0x010000, CRC(0037cd57) SHA1(b5882027269cf71878a73009bc3e40d9fcfac60d), "BWB","Lucky Number 7 (BWB) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4ln7__b,  m4ln7,  "l7_20sb_.1_1", 0x0000, 0x010000, CRC(d53bcd66) SHA1(7f5c65d5ca3dbb8a0c38f169585fc78d512166af), "BWB","Lucky Number 7 (BWB) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4ln7__c,  m4ln7,  "l7_20sd_.1_1", 0x0000, 0x010000, CRC(a82d04cb) SHA1(1abe9e22f6f526ab076163ce79cff841d1b38b0a), "BWB","Lucky Number 7 (BWB) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4ln7__d,  m4ln7,  "l7_20sk_.1_1", 0x0000, 0x010000, CRC(7d2104fa) SHA1(a0a65042f4db8ea5184d41c68ffcf7608580d928), "BWB","Lucky Number 7 (BWB) (MPU4) (set 5)" )


/*****************************************************************************************************************************************************************************
*
* Mad Money
* - some sets boot
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4madmon_keys[2] = { 0x10, 0x351f3a };

#define M4MADMON_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 ) \
	/* missing? */

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4MADMON_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4madmon_keys>(R5, RT3), mpu4_impcoin, mpu4bwb_machines_state, init_m4big ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4madmon,       0,          "mm_20a__.7_1", 0x0000, 0x020000, CRC(7df66388) SHA1(4e5bcbcb2fb08b23989c83f11751400f666bbdc2), "BWB","Mad Money (BWB) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4madmon__a,    m4madmon,   "mm_20b__.7_1", 0x0000, 0x020000, CRC(7f592e44) SHA1(05e78347cd09d1e58f0a50a724e0563490ec5185), "BWB","Mad Money (BWB) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4madmon__b,    m4madmon,   "mm_20bg_.7_1", 0x0000, 0x020000, CRC(2cd8dcc2) SHA1(c4a2a423a55c6b0668739429c24c69b25e3824cf), "BWB","Mad Money (BWB) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4madmon__c,    m4madmon,   "mm_20bt_.7_1", 0x0000, 0x020000, CRC(6e929612) SHA1(6b1d06c3cfce440c4cefb8dd04e7a5da673d545a), "BWB","Mad Money (BWB) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4madmon__d,    m4madmon,   "mm_20s__.7_1", 0x0000, 0x020000, CRC(c23b338c) SHA1(0d3bc801132e68564c2bc01b810e71362047ade1), "BWB","Mad Money (BWB) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4madmon__e,    m4madmon,   "mm_20sb_.7_1", 0x0000, 0x020000, CRC(51be206b) SHA1(913defb24bdb7551acefefd9673f5663129edbec), "BWB","Mad Money (BWB) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4madmon__f,    m4madmon,   "mm_25__c.3_1", 0x0000, 0x020000, CRC(e1879caf) SHA1(4d0a804a8d81aab5bb9dec611654325e6a3fb741), "BWB","Mad Money (BWB) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4madmon__g,    m4madmon,   "mm_25_dc.3_1", 0x0000, 0x020000, CRC(9bc81854) SHA1(b4c42a3da0ab03a0f43846e0c3a4a0b5f3c7e65a), "BWB","Mad Money (BWB) (MPU4) (set 8)" )
GAME_CUSTOM( 199?, m4madmon__h,    m4madmon,   "mm_25_gc.3_1", 0x0000, 0x020000, CRC(8d2f1259) SHA1(239c2430bd4ce7b53615b00fac79fb7eceecabf1), "BWB","Mad Money (BWB) (MPU4) (set 9)" )
GAME_CUSTOM( 199?, m4madmon__i,    m4madmon,   "mm_25a_c.3_1", 0x0000, 0x020000, CRC(b45b88c2) SHA1(56ed8c83c68f410fcc5ac342abac6b1f4419cccd), "BWB","Mad Money (BWB) (MPU4) (set 10)" )
GAME_CUSTOM( 199?, m4madmon__j,    m4madmon,   "mm_25b_c.3_1", 0x0000, 0x020000, CRC(fc33804b) SHA1(f817a6dd691739fcf7d7d622da265f63f60503f6), "BWB","Mad Money (BWB) (MPU4) (set 11)" )
GAME_CUSTOM( 199?, m4madmon__k,    m4madmon,   "mm_25bdc.3_1", 0x0000, 0x020000, CRC(188666c0) SHA1(e8a61c327c73aac2a6b0dc674dee7bc2aa358b27), "BWB","Mad Money (BWB) (MPU4) (set 12)" )


/*****************************************************************************************************************************************************************************
*
* Mad Money Classic
* - some sets boot
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4madmnc_keys[2] = { 0x10, 0x351f3a };

#define M4MADMNC_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 ) \
	/* missing? */

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4MADMNC_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4madmnc_keys>(R5, RT3), mpu4_impcoin, mpu4bwb_machines_state, init_m4big ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4madmnc,       0,          "cm_25__c.3_1", 0x0000, 0x020000, CRC(3d9ff5fe) SHA1(b918bb15251514f50a669216c7d00ecf23e64d1b), "BWB","Mad Money Classic (BWB) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4madmnc__a,    m4madmnc,   "cm_25_bc.3_1", 0x0000, 0x020000, CRC(65a7b870) SHA1(58b910d7e002164cbbe1aa32c5e17dfe7cfb507d), "BWB","Mad Money Classic (BWB) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4madmnc__b,    m4madmnc,   "cm_25_dc.3_1", 0x0000, 0x020000, CRC(fcae8cf3) SHA1(0f1e86e2f02be2e1870f0f70509bc4a2ada6d3a5), "BWB","Mad Money Classic (BWB) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4madmnc__c,    m4madmnc,   "cm_25_kc.3_1", 0x0000, 0x020000, CRC(f66cf97b) SHA1(9b6c4da3a9d64ed2581b04ca82a945dc295931bc), "BWB","Mad Money Classic (BWB) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4madmnc__d,    m4madmnc,   "cm_25a_c.3_1", 0x0000, 0x020000, CRC(94e21dc0) SHA1(0d35a467fdcd19909d5540c8a4461364bf7e17f3), "BWB","Mad Money Classic (BWB) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4madmnc__e,    m4madmnc,   "cm_25b_c.3_1", 0x0000, 0x020000, CRC(95a1eb43) SHA1(a44d5c8fadf547187834c4686f6001e6e7df83f7), "BWB","Mad Money Classic (BWB) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4madmnc__f,    m4madmnc,   "cm_25bgc.3_1", 0x0000, 0x020000, CRC(3241cdf5) SHA1(9ac4b415fff3ee3422dc6e4df7f626cddcab6c38), "BWB","Mad Money Classic (BWB) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4madmnc__g,    m4madmnc,   "cm_25btc.3_1", 0x0000, 0x020000, CRC(b8c0f623) SHA1(62d489d82955ac492f78be77a3f2558ad080b375), "BWB","Mad Money Classic (BWB) (MPU4) (set 8)" )
GAME_CUSTOM( 199?, m4madmnc__h,    m4madmnc,   "cm_29_dc.4_1", 0x0000, 0x020000, CRC(72542e93) SHA1(f9aad1e290b345ebd93dee81a10ad3ebb61b8228), "BWB","Mad Money Classic (BWB) (MPU4) (set 9)" )
GAME_CUSTOM( 199?, m4madmnc__i,    m4madmnc,   "cm_29a_c.4_1", 0x0000, 0x020000, CRC(aa7c11ce) SHA1(3ae11b5279f975c0f6f6462d9e23180b47cbe280), "BWB","Mad Money Classic (BWB) (MPU4) (set 10)" )
GAME_CUSTOM( 199?, m4madmnc__j,    m4madmnc,   "cm_29b_c.4_1", 0x0000, 0x020000, CRC(b5d749d8) SHA1(91f318a193a7ad841ac9a7f2114385ad97555f4d), "BWB","Mad Money Classic (BWB) (MPU4) (set 11)" )
GAME_CUSTOM( 199?, m4madmnc__k,    m4madmnc,   "cm_29bgc.4_1", 0x0000, 0x020000, CRC(82f06ed1) SHA1(3f982d8fb1f689cd774f407585a8dce5d8c031a7), "BWB","Mad Money Classic (BWB) (MPU4) (set 12)" )
GAME_CUSTOM( 199?, m4madmnc__l,    m4madmnc,   "cm_29btc.4_1", 0x0000, 0x020000, CRC(dd96bbeb) SHA1(1cf461dc36a8086b3f438922e1675fbd929af771), "BWB","Mad Money Classic (BWB) (MPU4) (set 13)" )
GAME_CUSTOM( 199?, m4madmnc__m,    m4madmnc,   "cm_39_dc.4_1", 0x0000, 0x020000, CRC(c81c575e) SHA1(8c1f5151412da267381434259ca2c2d307668a74), "BWB","Mad Money Classic (BWB) (MPU4) (set 14)" )
GAME_CUSTOM( 199?, m4madmnc__n,    m4madmnc,   "cm_39a_c.4_1", 0x0000, 0x020000, CRC(10346803) SHA1(2d7b0ca2c30a24c4779c44872fc86d6ae269b51f), "BWB","Mad Money Classic (BWB) (MPU4) (set 15)" )
GAME_CUSTOM( 199?, m4madmnc__o,    m4madmnc,   "cm_39a_c.5_1", 0x0000, 0x020000, CRC(a235932f) SHA1(b6875c1119c8ed3a77d7f358298c119afac17dea), "BWB","Mad Money Classic (BWB) (MPU4) (set 16)" )
GAME_CUSTOM( 199?, m4madmnc__p,    m4madmnc,   "cm_39b_c.4_1", 0x0000, 0x020000, CRC(0f9f3015) SHA1(8a02b6a22a96c7e310a7428d36b306e8af618e9a), "BWB","Mad Money Classic (BWB) (MPU4) (set 17)" )
GAME_CUSTOM( 199?, m4madmnc__q,    m4madmnc,   "cm_39bgc.4_1", 0x0000, 0x020000, CRC(38b8171c) SHA1(5e33189d6b2d72a045c98c2bb8492961ab3d978f), "BWB","Mad Money Classic (BWB) (MPU4) (set 18)" )
GAME_CUSTOM( 199?, m4madmnc__r,    m4madmnc,   "cm_39btc.4_1", 0x0000, 0x020000, CRC(67dec226) SHA1(68bfe25ff693ea7b4469d2e7d1b6972cc421d1cc), "BWB","Mad Money Classic (BWB) (MPU4) (set 19)" )
GAME_CUSTOM( 199?, m4madmnc__s,    m4madmnc,   "cm_49_dc.4_1", 0x0000, 0x020000, CRC(e0ab38ee) SHA1(a324a1bb9f709b72de6fdd4c896326ca0004fdef), "BWB","Mad Money Classic (BWB) (MPU4) (set 20)" )
GAME_CUSTOM( 199?, m4madmnc__t,    m4madmnc,   "cm_49a_c.4_1", 0x0000, 0x020000, CRC(388307b3) SHA1(070d431171ce325f6935f5bc1fd8db3acf79b13c), "BWB","Mad Money Classic (BWB) (MPU4) (set 21)" )
GAME_CUSTOM( 199?, m4madmnc__u,    m4madmnc,   "cm_49b_c.4_1", 0x0000, 0x020000, CRC(27285fa5) SHA1(5f08bc751d7a0875b1b879d7d96aa9803fb942c6), "BWB","Mad Money Classic (BWB) (MPU4) (set 22)" )
GAME_CUSTOM( 199?, m4madmnc__v,    m4madmnc,   "cm_49bgc.4_1", 0x0000, 0x020000, CRC(100f78ac) SHA1(c5d30fa46508b00b163dbf05e572f8c23fdb6cc3), "BWB","Mad Money Classic (BWB) (MPU4) (set 23)" )
GAME_CUSTOM( 199?, m4madmnc__w,    m4madmnc,   "cm_49btc.4_1", 0x0000, 0x020000, CRC(4f69ad96) SHA1(50948166fee3cd1e6f0e378076046ee305204d61), "BWB","Mad Money Classic (BWB) (MPU4) (set 24)" )


/*****************************************************************************************************************************************************************************
*
* Money Mummy Money
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4mmm_keys[2] = { 0x11, 0x320926 };

#define M4MMM_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", 0 ) \
	ROM_LOAD( "mu___snd.1_1", 0x000000, 0x080000, CRC(570cf5f8) SHA1(48b3703bf385d037e4e870dfb671b75e9bab84a7) ) \
	ROM_LOAD( "mu___snd.1_2", 0x080000, 0x080000, CRC(6ec1910b) SHA1(4920fe0b7c7f4ddb14d56f50598aaf62e5867014) ) \
	ROM_LOAD( "mu___snd.1_3", 0x100000, 0x080000, CRC(8699378c) SHA1(55c3e310cfde8046e58bf21a8788e697c8275b8d) ) \
	ROM_LOAD( "mu___snd.1_4", 0x180000, 0x080000, CRC(54b193d8) SHA1(ab24624ce69d352a14f6bd3db127fa1c8c5f07db) )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent, machine, inputs, init, name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4MMM_EXTRA_ROMS \
	ROM_END \
	GAME( year, setname, parent, machine, inputs, mpu4bwb_machines_state, init, ROT0, company, title, GAME_FLAGS )

GAME_CUSTOM( 199?, m4mmm,     0,     bwboki_chr_cheat<m4mmm_keys>(R5, RT3), mpu4impcoin_jackpot15_20p,      init_m4big, "mu_sj___.3_0", 0x0000, 0x040000, CRC(abdf9d1c) SHA1(e8c6a056025b44e4ec995b42b2720e6366a97283), "BWB",u8"Money Mummy Money (BWB) (MPU4) (20/25/30p stake / £10/£15 jackpot) (set 1)" )
GAME_CUSTOM( 199?, m4mmm__a,  m4mmm, bwboki_chr_cheat<m4mmm_keys>(R5, RT3), mpu4impcoin_jackpot15_20p,      init_m4big, "mu_sja__.3_0", 0x0000, 0x040000, CRC(3d2a9ea4) SHA1(f2ec904c8cef84affaad603edf26a864bd34be29), "BWB",u8"Money Mummy Money (BWB) (MPU4) (20/25/30p stake / £10/£15 jackpot) (set 2)" )
GAME_CUSTOM( 199?, m4mmm__b,  m4mmm, bwboki_chr_cheat<m4mmm_keys>(R5, RT3), mpu4impcoin_jackpot15_20p_90pc, init_m4big, "mu_sjk__.3_0", 0x0000, 0x040000, CRC(34e4f8ba) SHA1(606d607faeb43190f5167aa3d10c55d9986b7e58), "BWB",u8"Money Mummy Money (BWB) (MPU4) (20/25/30p stake / £10/£15 jackpot) (set 3)" ) // % key
GAME_CUSTOM( 199?, m4mmm__c,  m4mmm, bwboki_chr_cheat<m4mmm_keys>(R5, RT3), mpu4impcoin_jackpot15_20p,      init_m4big, "mu_sjs__.3_0", 0x0000, 0x040000, CRC(26fb12b3) SHA1(d341181be75c87b44e4066653225911ce3460ed8), "BWB",u8"Money Mummy Money (BWB) (MPU4) (20/25/30p stake / £10/£15 jackpot) (set 4)" ) // Datapak
GAME_CUSTOM( 199?, m4mmm__d,  m4mmm, bwboki_chr_cheat<m4mmm_keys>(R5, RT3), mpu4impcoin_jackpot15_20p,      init_m4big, "mu_ssj__.2_0", 0x0000, 0x040000, CRC(935b6602) SHA1(d5fa5688895fe3c2ae3ad7dbbc35d9b12574c93d), "BWB",u8"Money Mummy Money (BWB) (MPU4) (20/25/30p stake / £10/£15 jackpot) (set 5)" )
GAME_CUSTOM( 199?, m4mmm__e,  m4mmm, bwboki_chr_cheat<m4mmm_keys>(R5, RT3), mpu4impcoin_jackpot15_20p,      init_m4big, "mu_ssja_.2_0", 0x0000, 0x040000, CRC(ff97814c) SHA1(8d9d74e6b0096cdc3226cfa91d7b653855600d5a), "BWB",u8"Money Mummy Money (BWB) (MPU4) (20/25/30p stake / £10/£15 jackpot) (set 6)" )
GAME_CUSTOM( 199?, m4mmm__f,  m4mmm, bwboki_chr_cheat<m4mmm_keys>(R5, RT3), mpu4impcoin_jackpot15_20p,      init_m4big, "mu_ssjb_.2_0", 0x0000, 0x040000, CRC(5728973a) SHA1(2cd9c866fcc33150fb8d456f741ac809e0bd2b15), "BWB",u8"Money Mummy Money (BWB) (MPU4) (20/25/30p stake / £10/£15 jackpot) (set 7)" ) // Datapak


/*****************************************************************************************************************************************************************************
*
* Orlando Magic
* - some sets boot
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4orland_keys[2] = { 0x10, 0x2c081f };

#define M4ORLAND_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", 0 ) \
	ROM_LOAD( "orlandosnd.p1", 0x000000, 0x080000, CRC(2735649d) SHA1(7b27bf2d4091ab581d399679b03f538f449f180c) ) \
	ROM_LOAD( "orlandosnd.p2", 0x080000, 0x080000, CRC(0741e2ff) SHA1(c49a2809073dd058ba85ae14c888e19d3eb2b133) )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4ORLAND_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4orland_keys>(R5, RT3), mpu4_impcoin, mpu4bwb_machines_state, init_m4big_low ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4orland,       0,          "or 05a v2-1,27c010",   0x0000, 0x020000, CRC(a33c22ee) SHA1(3598a2940f05622405fdef16426f3f5f30dfef29), "BWB","Orlando Magic (BWB) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4orland__a,    m4orland,   "or_05a__.1_1",         0x0000, 0x020000, CRC(3e7fe3ac) SHA1(9f4c0d5b7ba10726376b0654c8ddbc62b62c9eed), "BWB","Orlando Magic (BWB) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4orland__b,    m4orland,   "or_20a__.7_1",         0x0000, 0x020000, CRC(ae524299) SHA1(3bb2bfe1c0ca0660aca148d6f17b730b7bdc8183), "BWB","Orlando Magic (BWB) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4orland__c,    m4orland,   "or_20b__.7_1",         0x0000, 0x020000, CRC(c8a30e2e) SHA1(8abc5437751faf61c12709c963a6819cb0b2b43f), "BWB","Orlando Magic (BWB) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4orland__d,    m4orland,   "or_20bg_.7_1",         0x0000, 0x020000, CRC(552561e5) SHA1(c92c4b21182511e4880b90313a673045e20b01e8), "BWB","Orlando Magic (BWB) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4orland__e,    m4orland,   "or_20bt_.7_1",         0x0000, 0x020000, CRC(7db2e12a) SHA1(c10989d5d6bbc8f87ad21364f6c64495dc4a3047), "BWB","Orlando Magic (BWB) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4orland__f,    m4orland,   "or_20s__.7_1",         0x0000, 0x020000, CRC(9ce4b650) SHA1(26a3337526d398ce265d735cfbe6d0e69c1f5cab), "BWB","Orlando Magic (BWB) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4orland__g,    m4orland,   "or_20sb_.7_1",         0x0000, 0x020000, CRC(90f59de7) SHA1(2bb6c0680c654265c8669a5f13346ae6afb72fb5), "BWB","Orlando Magic (BWB) (MPU4) (set 8)" )
GAME_CUSTOM( 199?, m4orland__h,    m4orland,   "or_20sd_.7_1",         0x0000, 0x020000, CRC(242c552c) SHA1(dce5f0d38c8c6c101337028f30c05a7eb629e703), "BWB","Orlando Magic (BWB) (MPU4) (set 9)" )


/*****************************************************************************************************************************************************************************
*
* Prize Bingo
* - some sets boot
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4pzbing_keys[2] = { 0x10, 0x141531 };

#define M4PZBING_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 ) \
	/* missing? */

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4PZBING_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4pzbing_keys>(R5, RT3), mpu4_impcoin, mpu4bwb_machines_state, init_m4 ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4pzbing,       0,          "pb_20a__.4_1", 0x0000, 0x010000, CRC(52aa92e5) SHA1(3dc20e521677e829967e1d689c9905fb96aee639), "BWB","Prize Bingo (BWB) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4pzbing__a,    m4pzbing,   "pb_20ad_.4_1", 0x0000, 0x010000, CRC(f6bd6ad0) SHA1(092cb895c576ed2e995b62aba21851af6fb90959), "BWB","Prize Bingo (BWB) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4pzbing__b,    m4pzbing,   "pb_20bg_.4_1", 0x0000, 0x010000, CRC(593e89f4) SHA1(4ed79c889370eb5de20b434cd83b2ee3fae31ed8), "BWB","Prize Bingo (BWB) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4pzbing__c,    m4pzbing,   "pb_20bt_.4_1", 0x0000, 0x010000, CRC(fa51b522) SHA1(1fdebe63b871f700a664053251b02c8cf47fbb92), "BWB","Prize Bingo (BWB) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4pzbing__d,    m4pzbing,   "pb_20s__.4_1", 0x0000, 0x010000, CRC(601f37d9) SHA1(0971c5c5321f5ce1508a0dd8abd989939224a779), "BWB","Prize Bingo (BWB) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4pzbing__e,    m4pzbing,   "pb_20sb_.4_1", 0x0000, 0x010000, CRC(0234978a) SHA1(f24069883efb69de1024b6efbeb3d6a100ac5b9a), "BWB","Prize Bingo (BWB) (MPU4) (set 6)" )


/*****************************************************************************************************************************************************************************
*
* Quids In
* - some sets boot
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4quidin_keys[2] = { 0x10, 0x050107 };

#define M4QUIDIN_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", 0 ) \
	ROM_LOAD( "qi_____.1_2", 0x000000, 0x080000, CRC(216209e3) SHA1(af274a7f27ba0e7ac03400e9919537ab36464e64) ) \
	ROM_LOAD( "qi_____.1_3", 0x080000, 0x080000, CRC(e909c3ec) SHA1(68ce743729aaefd6c20ee447af40d99e0f4c072b) )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4QUIDIN_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4quidin_keys>(R5, RT3), mpu4_impcoin, mpu4bwb_machines_state, init_m4 ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4quidin,       0,          "qi_20a__.3_1", 0x0000, 0x010000, CRC(88873c45) SHA1(70fa65402dbbe716a089497a8ccb06e0ba2aac6d), "BWB","Quids In (BWB) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4quidin__a,    m4quidin,   "qi_20s__.3_1", 0x0000, 0x010000, CRC(3758dc38) SHA1(d22a379975e948d465e13233a796e0fb07e3c04f), "BWB","Quids In (BWB) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4quidin__b,    m4quidin,   "qi_20sb_.3_1", 0x0000, 0x010000, CRC(e254dc09) SHA1(ad5853c854f628de6203be8d6c3cbaa6a600e340), "BWB","Quids In (BWB) (MPU4) (set 3)" )


/*****************************************************************************************************************************************************************************
*
* Quids In (Showcase cabinet version)
* - some sets boot
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4quidis_keys[2] = { 0x10, 0x050107 };

#define M4QUIDIS_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 ) \
	/* missing? */

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4QUIDIS_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4quidis_keys>(R5, RT3), mpu4_impcoin, mpu4bwb_machines_state, init_m4 ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4quidis,       0,          "pq_20a__.3_1", 0x0000, 0x010000, CRC(7eb762a1) SHA1(4546a7bf43f8ab6eb9713348e3f919de7532eed2), "BWB","Quids In Showcase (BWB) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4quidis__a,    m4quidis,   "pq_20s__.3_1", 0x0000, 0x010000, CRC(71360992) SHA1(0b64f27f0edfdebca41552181ff0f2b5491ec308), "BWB","Quids In Showcase (BWB) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4quidis__b,    m4quidis,   "pq_20sb_.3_1", 0x0000, 0x010000, CRC(a43a09a3) SHA1(46d83465d1026620af2f59dd4b638444ca834ad1), "BWB","Quids In Showcase (BWB) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4quidis__c,    m4quidis,   "pq_20sd_.3_1", 0x0000, 0x010000, CRC(d92cc00e) SHA1(bff2b5da08cc34040b1d4d750ea6a654f9b77959), "BWB","Quids In Showcase (BWB) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4quidis__d,    m4quidis,   "pq_20sk_.3_1", 0x0000, 0x010000, CRC(0c20c03f) SHA1(f802daa8ff2c159ba4831ed048e0ddd8469448da), "BWB","Quids In Showcase (BWB) (MPU4) (set 5)" )


/*****************************************************************************************************************************************************************************
*
* Rack Em Up
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4rackem_keys[2] = { 0x11, 0x13222f };

#define M4RACKEM_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", 0 ) \
	ROM_LOAD( "re_snd.p1", 0x000000, 0x080000, CRC(aea88892) SHA1(457dab5cddfb9762f7e0bd61187b8052aee71c28) ) \
	ROM_LOAD( "re_snd.p2", 0x080000, 0x080000, CRC(57394ec6) SHA1(cba7abebd3ab165e9531017168f51ada6cf35991) ) \
	ROM_LOAD( "re_snd.p3", 0x100000, 0x080000, CRC(5d5d5309) SHA1(402615633976410225a1ee50c454391dc69a68cb) )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4RACKEM_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4rackem_keys>(R5, RT3), mpu4impcoin_jackpot15_20p, mpu4bwb_machines_state, init_m4big ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 1999, m4rackem,       0,          "re_sj___.3_0", 0x0000, 0x040000, CRC(2f463d2f) SHA1(3410cc8a6d097a4edfcb4c57c237d1d514b507ba), "BWB",u8"Rack Em Up (BWB) (MPU4) (ver. 3) (20/25/30p stake / £5/£15 jackpot)" )
GAME_CUSTOM( 1999, m4rackem__a,    m4rackem,   "re_sj___.2_0", 0x0000, 0x040000, CRC(e36d3f86) SHA1(a5f522c86482517b8dc735b1012f8f7668c2f18d), "BWB",u8"Rack Em Up (BWB) (MPU4) (ver. 2) (20/25/30p stake / £5/£15 jackpot) (set 1)" )
GAME_CUSTOM( 1999, m4rackem__b,    m4rackem,   "re_sj_d_.2_0", 0x0000, 0x040000, CRC(7a31658c) SHA1(4fade421b3a1a732a99f7cb6346279ad82f55362), "BWB",u8"Rack Em Up (BWB) (MPU4) (ver. 2) (20/25/30p stake / £5/£15 jackpot) (set 2)" ) // Datapak
GAME_CUSTOM( 1999, m4rackem__c,    m4rackem,   "re_sjs__.2_0", 0x0000, 0x040000, CRC(d7c499c8) SHA1(73542f54322f5ffb87d16f5f66cc3a22c2849f20), "BWB",u8"Rack Em Up (BWB) (MPU4) (ver. 2) (20/25/30p stake / £5/£15 jackpot) (set 3)" ) // Datapak
GAME_CUSTOM( 1999, m4rackem__d,    m4rackem,   "re_sjsw_.2_0", 0x0000, 0x040000, CRC(66355370) SHA1(d54aab7403e64a67edf2baeaf1321ee5c4aa553d), "BWB",u8"Rack Em Up (BWB) (MPU4) (ver. 2) (20/25/30p stake / £5/£15 jackpot) (set 4)" ) // Datapak


/*****************************************************************************************************************************************************************************
*
* Rainbow Gold
* - some sets boot
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4rbgold_keys[2] = { 0x10, 0x2d371b };

#define M4RBGOLD_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 ) \
	/* missing? */

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4RBGOLD_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4rbgold_keys>(R5, RT3), mpu4_impcoin, mpu4bwb_machines_state, init_m4 ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4rbgold,       0,          "rb_20a_p.2a1", 0x0000, 0x010000, CRC(d7e6e514) SHA1(25645b69e86335622df43113908ed88a21f27e30), "BWB","Rainbow Gold (BWB) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4rbgold__a,    m4rbgold,   "rb_20a_p.2f1", 0x0000, 0x010000, CRC(62af6db6) SHA1(0dcb679c05f090f8dab7228009a700c31f0179d8), "BWB","Rainbow Gold (BWB) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4rbgold__b,    m4rbgold,   "rb_20sbp.2a1", 0x0000, 0x010000, CRC(ba4c2e74) SHA1(fcc325754f96e742998373c6c5c13a8509f48cd5), "BWB","Rainbow Gold (BWB) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4rbgold__c,    m4rbgold,   "rb_20sbp.2f1", 0x0000, 0x010000, CRC(0f05a6d6) SHA1(1ba7ccbb3d78196d4b98a0a1173bf41c9dd62c1f), "BWB","Rainbow Gold (BWB) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4rbgold__d,    m4rbgold,   "rb_20sbp.2s1", 0x0000, 0x010000, CRC(d8278b82) SHA1(8e924096e238359fd2a7f81198c8af1515dc8a19), "BWB","Rainbow Gold (BWB) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4rbgold__e,    m4rbgold,   "rb_xea__.2a1", 0x0000, 0x010000, CRC(1bc266ff) SHA1(fcdb63613dbcf0b15744317a69bd8a1e3cf92526), "BWB","Rainbow Gold (BWB) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4rbgold__f,    m4rbgold,   "rb_xea__.2f1", 0x0000, 0x010000, CRC(ae8bee5d) SHA1(bae49779a009a29b27c99f5e5a4b2aeca39b5626), "BWB","Rainbow Gold (BWB) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4rbgold__g,    m4rbgold,   "rb_xea__.2s1", 0x0000, 0x010000, CRC(79a9c309) SHA1(8ad87ec153ed8a89e612011679a172fc66bae711), "BWB","Rainbow Gold (BWB) (MPU4) (set 8)" )
GAME_CUSTOM( 199?, m4rbgold__h,    m4rbgold,   "rb_xes__.2a1", 0x0000, 0x010000, CRC(14430dcc) SHA1(7e514a1857f6911a57aff7900af297412ef0d905), "BWB","Rainbow Gold (BWB) (MPU4) (set 9)" )
GAME_CUSTOM( 199?, m4rbgold__i,    m4rbgold,   "rb_xes__.2f1", 0x0000, 0x010000, CRC(a10a856e) SHA1(a8536d576b3698ccc539c8ac1c9136222b1cf297), "BWB","Rainbow Gold (BWB) (MPU4) (set 10)" )
GAME_CUSTOM( 199?, m4rbgold__j,    m4rbgold,   "rb_xes__.2s1", 0x0000, 0x010000, CRC(7628a83a) SHA1(da1833df2e88480dafd2410fb24f0fbffbdeb679), "BWB","Rainbow Gold (BWB) (MPU4) (set 11)" )
GAME_CUSTOM( 199?, m4rbgold__k,    m4rbgold,   "rb_xesb_.2f1", 0x0000, 0x010000, CRC(c321253d) SHA1(0d59b0cf7118c7932e9d89eca823ce200c5030f5), "BWB","Rainbow Gold (BWB) (MPU4) (set 12)" )
GAME_CUSTOM( 199?, m4rbgold__l,    m4rbgold,   "rb_xesb_.2s1", 0x0000, 0x010000, CRC(14030869) SHA1(283e7ca543a37a60f2d3a8c6d5473b591bb20e62), "BWB","Rainbow Gold (BWB) (MPU4) (set 13)" )
GAME_CUSTOM( 199?, m4rbgold__m,    m4rbgold,   "rb_xesd_.2a1", 0x0000, 0x010000, CRC(b054f5f9) SHA1(fdcca5375ff8f26f6889c5556216ff0fdf2bce94), "BWB","Rainbow Gold (BWB) (MPU4) (set 14)" )
GAME_CUSTOM( 199?, m4rbgold__n,    m4rbgold,   "rb_xesd_.2f1", 0x0000, 0x010000, CRC(051d7d5b) SHA1(b8216e505de00802e5e34d11eb3e18e0736fa772), "BWB","Rainbow Gold (BWB) (MPU4) (set 15)" )
GAME_CUSTOM( 199?, m4rbgold__o,    m4rbgold,   "rb_xesd_.2s1", 0x0000, 0x010000, CRC(d23f500f) SHA1(fe8d2825c8fb24c3885013c046a15ddec5cb3a1f), "BWB","Rainbow Gold (BWB) (MPU4) (set 16)" )
GAME_CUSTOM( 199?, m4rbgold__p,    m4rbgold,   "rbixe___.2a1", 0x0000, 0x010000, CRC(349fafdd) SHA1(da68e210c8c0a716c1ef62e7f404f6985903b00a), "BWB","Rainbow Gold (BWB) (MPU4) (set 17)" )
GAME_CUSTOM( 199?, m4rbgold__q,    m4rbgold,   "rbixe___.2s1", 0x0000, 0x010000, CRC(56f40a2b) SHA1(0c6c035d2a3dbef70b1bc95fa38ed62a70770739), "BWB","Rainbow Gold (BWB) (MPU4) (set 18)" )


/*****************************************************************************************************************************************************************************
*
* Red Hot Fever
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4rhfev_keys[2] = { 0x11, 0x3f0e27 };

#define M4RHFEV_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", 0 ) \
	ROM_LOAD( "rf_____.1_1", 0x000000, 0x080000, CRC(ac8d539d) SHA1(8baf14bece50774f93ae9eaf3effabb6882d2c43) ) \
	ROM_LOAD( "rf_____.1_2", 0x080000, 0x080000, CRC(cc2fadd8) SHA1(681850e2e6164cf8af8e7501ac44f475cc07b742) ) \
	ROM_LOAD( "rf_____.1_3", 0x100000, 0x080000, CRC(165aaf9f) SHA1(815224fe94a77628cef1dd0d8a238edcb4813006) ) \
	ROM_LOAD( "rf_____.1_4", 0x180000, 0x080000, CRC(4f7e7b49) SHA1(f9d421eeab73e0c795a08cf166c8807e0b14ec82) )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent, machine, inputs, init, name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4RHFEV_EXTRA_ROMS \
	ROM_END \
	GAME( year, setname, parent, machine, inputs, mpu4bwb_machines_state, init, ROT0, company, title, GAME_FLAGS )


GAME_CUSTOM( 1999, m4rhfev,     0,       bwboki_chr_cheat<m4rhfev_keys>(R5, RT3), mpu4impcoin_jackpot15_20p, init_m4big, "rt_sj___.7_0", 0x0000, 0x040000, CRC(3dd895ef) SHA1(433ecc268956c94c51dbccefd006b72e0ad8567b), "BWB",u8"Red Hot Fever (BWB) (MPU4) (ver. 7) (20/25/30p stake / £5/£10/£15 jackpot) (set 1)" )
GAME_CUSTOM( 1999, m4rhfev__a,  m4rhfev, bwboki_chr_cheat<m4rhfev_keys>(R5, RT3), mpu4impcoin_jackpot15_20p, init_m4big, "rt_sja__.7_0", 0x0000, 0x040000, CRC(0ab59402) SHA1(485b4d2efd8f99085ed6ce5b7e07ede001c982c4), "BWB",u8"Red Hot Fever (BWB) (MPU4) (ver. 7) (20/25/30p stake / £5/£10/£15 jackpot) (set 2)" )
GAME_CUSTOM( 1999, m4rhfev__b,  m4rhfev, bwboki_chr_cheat<m4rhfev_keys>(R5, RT3), mpu4impcoin_jackpot15_20p, init_m4big, "rt_sjs__.7_0", 0x0000, 0x040000, CRC(1a8feafb) SHA1(83151f63b7ebe1c538f9334e9c3d6889d0730144), "BWB",u8"Red Hot Fever (BWB) (MPU4) (ver. 7) (20/25/30p stake / £5/£10/£15 jackpot) (set 3)" ) // Datapak
//
GAME_CUSTOM( 1999, m4rhfev__c,  m4rhfev, bwboki_chr_cheat<m4rhfev_keys>(R5, RT3), mpu4impcoin_jackpot5_5p,   init_m4big, "rt_vc___.1_0", 0x0000, 0x040000, CRC(2a8df147) SHA1(df0e7021e9d169575a1297f9851b5a64e20d1a40), "BWB",u8"Red Hot Fever (BWB) (MPU4) (ver. 1) (5/10p stake / £5/£8 jackpot) (set 1)" )
GAME_CUSTOM( 1999, m4rhfev__d,  m4rhfev, bwboki_chr_cheat<m4rhfev_keys>(R5, RT3), mpu4impcoin_jackpot5_5p,   init_m4big, "rt_vc_d_.1_0", 0x0000, 0x040000, CRC(7adef22b) SHA1(d6a584581745c0ce64f646ef0b49cb68343990d0), "BWB",u8"Red Hot Fever (BWB) (MPU4) (ver. 1) (5/10p stake / £5/£8 jackpot) (set 2)" ) // Datapak


/*****************************************************************************************************************************************************************************
*
* Sinbad
* - some sets boot
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4sinbd_keys[2] = { 0x10, 0x0c253d };

#define M4SINBD_EXTRA_ROMS \
	ROM_REGION( 0x180000, "okicard:msm6376", ROMREGION_ERASE00 ) \
	/*These were with the last listed set, though I have no reason to believe they aren't valid for all BwB Sinbad games, both have OKI tables */ \
	ROM_LOAD( "sinbadbwb1_2snd.bin", 0x000000, 0x080000, CRC(2ee60ce6) SHA1(865860639e8471f97ace0beac2f4c7fddb8ca97c) ) \
	ROM_LOAD( "sinbadbwb1_3snd.bin", 0x080000, 0x080000, CRC(7701e5cc) SHA1(4f9ff91f2b6b15a9c08396b52fc8509ba476ed8d) )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4SINBD_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4sinbd_keys>(R5, RT3), mpu4_impcoin, mpu4bwb_machines_state, init_m4big ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4sinbd,     0,          "sd_20__c.1_1",         0x0000, 0x020000, CRC(28cd336e) SHA1(45bdf5403c04b7d3a3645b6b44ac3d12e6463a55), "BWB","Sinbad (BWB) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4sinbd__a,  m4sinbd,    "sd_20a__.4_1",         0x0000, 0x020000, CRC(12b8f629) SHA1(c8540ecb217cf0615d7a8d080136926646ca8497), "BWB","Sinbad (BWB) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4sinbd__b,  m4sinbd,    "sd_20a__.5_1",         0x0000, 0x020000, CRC(68a3d155) SHA1(5ae47f1ca860af30c77beebe3acf615958ed59e9), "BWB","Sinbad (BWB) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4sinbd__c,  m4sinbd,    "sd_20a_c.1_1",         0x0000, 0x020000, CRC(5a5eef7c) SHA1(32d801cea0593e220ac67c2c17782fc144d02cc4), "BWB","Sinbad (BWB) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4sinbd__d,  m4sinbd,    "sd_20ad_.5_1",         0x0000, 0x020000, CRC(dc9dd7b3) SHA1(e26c8a814cdb268026e31acb668f34160ea9ce7b), "BWB","Sinbad (BWB) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4sinbd__e,  m4sinbd,    "sd_20s__.1v1",         0x0000, 0x020000, CRC(acb5aa9a) SHA1(4186058f83e241f06de842d0ae79f26a2a0e6cf0), "BWB","Sinbad (BWB) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4sinbd__f,  m4sinbd,    "sd_20s__.4_1",         0x0000, 0x020000, CRC(bce21ced) SHA1(9e089fbca1dd39e356f38d0c301162affea9316c), "BWB","Sinbad (BWB) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4sinbd__g,  m4sinbd,    "sd_20s__.5_1",         0x0000, 0x020000, CRC(939ecca5) SHA1(a9cd465fbe742b910602f861ae6ac7cedc7d9be9), "BWB","Sinbad (BWB) (MPU4) (set 8)" )
GAME_CUSTOM( 199?, m4sinbd__h,  m4sinbd,    "sd_20s_p.4_1",         0x0000, 0x020000, CRC(5ae716a9) SHA1(4c9ceab423a1480f8257fad2d62b65a17788a472), "BWB","Sinbad (BWB) (MPU4) (set 9)" )
GAME_CUSTOM( 199?, m4sinbd__i,  m4sinbd,    "sd_20s_s.5_1",         0x0000, 0x020000, CRC(e47c869a) SHA1(2735613aafe71e9b2a3aabb433fbd98a83a546c7), "BWB","Sinbad (BWB) (MPU4) (set 10)" )
GAME_CUSTOM( 199?, m4sinbd__j,  m4sinbd,    "sd_20sb_.4_1",         0x0000, 0x020000, CRC(811efef2) SHA1(280c6d6697735faca58317925c58b687c5988b87), "BWB","Sinbad (BWB) (MPU4) (set 11)" )
GAME_CUSTOM( 199?, m4sinbd__k,  m4sinbd,    "sd_20sb_.5_1",         0x0000, 0x020000, CRC(594920ec) SHA1(03ac253ef35b8fbccbfe4b2c5a7d906a3001b3ec), "BWB","Sinbad (BWB) (MPU4) (set 12)" )
GAME_CUSTOM( 199?, m4sinbd__l,  m4sinbd,    "sd_20sbc.1_1",         0x0000, 0x020000, CRC(3720cf0d) SHA1(8f0e63985badbff6ac39fcf956ebff3b0655c2b9), "BWB","Sinbad (BWB) (MPU4) (set 13)" )
GAME_CUSTOM( 199?, m4sinbd__m,  m4sinbd,    "sd_20sbp.4_1",         0x0000, 0x020000, CRC(671bf4b6) SHA1(ac928b47c1392e8a82404776068233e7d1dd5d28), "BWB","Sinbad (BWB) (MPU4) (set 14)" )
GAME_CUSTOM( 199?, m4sinbd__n,  m4sinbd,    "sd_20sbs.5_1",         0x0000, 0x020000, CRC(2eab6ad3) SHA1(66261328999a139389198e772a69dcd994792439), "BWB","Sinbad (BWB) (MPU4) (set 15)" )
GAME_CUSTOM( 199?, m4sinbd__o,  m4sinbd,    "sd_20sdc.1_1",         0x0000, 0x020000, CRC(fc386926) SHA1(6e64aeb82e62ded75e48f0faab932b63732bcf08), "BWB","Sinbad (BWB) (MPU4) (set 16)" )
GAME_CUSTOM( 199?, m4sinbd__p,  m4sinbd,    "sd_25__c.1_1",         0x0000, 0x020000, CRC(348e846d) SHA1(ca0fffd59076e6e60d37f677d0c7e7f182a41b9e), "BWB","Sinbad (BWB) (MPU4) (set 17)" )
GAME_CUSTOM( 199?, m4sinbd__q,  m4sinbd,    "sd_25a_c.1_1",         0x0000, 0x020000, CRC(f50c55e9) SHA1(df72b196fb21b6359282c06b960fa53117a95fee), "BWB","Sinbad (BWB) (MPU4) (set 18)" )
GAME_CUSTOM( 199?, m4sinbd__r,  m4sinbd,    "sd_25sbc.1_1",         0x0000, 0x020000, CRC(0773632a) SHA1(34294e32243b903a6c1c54b8718ca7cddbd3316e), "BWB","Sinbad (BWB) (MPU4) (set 19)" )
GAME_CUSTOM( 199?, m4sinbd__s,  m4sinbd,    "sd_25sdc.1_1",         0x0000, 0x020000, CRC(4917a542) SHA1(23de449d536c799032afab678b9001fa8541fb8a), "BWB","Sinbad (BWB) (MPU4) (set 20)" )
GAME_CUSTOM( 199?, m4sinbd__t,  m4sinbd,    "sdi20___.1v1",         0x0000, 0x020000, CRC(c50b3555) SHA1(c2d89126b9122f48ecce52d50cd7a03cc2bf1829), "BWB","Sinbad (BWB) (MPU4) (set 21)" )
GAME_CUSTOM( 199?, m4sinbd__u,  m4sinbd,    "sv_20a__.4_1",         0x0000, 0x020000, CRC(0e790ae8) SHA1(48f055f3f1f5d3392b7fba1c5c30624c1f230327), "BWB","Sinbad (BWB) (MPU4) (set 22)" )
GAME_CUSTOM( 199?, m4sinbd__v,  m4sinbd,    "sv_20sb_.4_1",         0x0000, 0x020000, CRC(e88c1c29) SHA1(49d180068e4ae9cb65a58e65b4e1ac4d6657ae1d), "BWB","Sinbad (BWB) (MPU4) (set 23)" )
GAME_CUSTOM( 199?, m4sinbd__w,  m4sinbd,    "svi20___.4_1",         0x0000, 0x020000, CRC(643037ed) SHA1(d4063faba3069625474dd761f9ad2dcf2f710a19), "BWB","Sinbad (BWB) (MPU4) (set 24)" )
GAME_CUSTOM( 199?, m4sinbd__x,  m4sinbd,    "sinbadbwb1_1game.bin", 0x0000, 0x020000, CRC(cfe152a7) SHA1(b8ecfa8b763d04515b65eb902c18dba7198191c3), "BWB","Sinbad (BWB) (MPU4) (set 25)" )


/*****************************************************************************************************************************************************************************
*
* Sky Sports
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4sky_keys[2] = { 0x11, 0x1f2d3b };

#define M4SKY_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 ) \
	/* missing? */

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent, machine, inputs, init, name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4SKY_EXTRA_ROMS \
	ROM_END \
	GAME( year, setname, parent, machine, inputs, mpu4bwb_machines_state, init, ROT0, company, title, GAME_FLAGS )

GAME_CUSTOM( 199?, m4sky,     0,     bwboki_chr_cheat<m4sky_keys>(R5, RT3), mpu4impcoin_jackpot15_20p,      init_m4big, "sk_s____.3_1", 0x0000, 0x040000, CRC(749af008) SHA1(036514f2bcb84193cfa84313f0617f3196aea73e), "BWB",u8"Sky Sports Super Soccer (BWB) (MPU4) (20/25/30p stake / £8 token/£10/£15 jackpot)" )
// unusual 1.8 hopper error on sets below
GAME_CUSTOM( 199?, m4sky__a,  m4sky, bwboki_chr_cheat<m4sky_keys>(R5, RT3), mpu4impcoin_jackpot15_20p_90pc, init_m4big, "sk_sj___.5_0", 0x0000, 0x040000, CRC(45ae0423) SHA1(94d5b3d4aacb69a18ff3f45681eb5f7fba7657e8), "BWB",u8"Sky Sports Super Soccer (BWB) (MPU4) (20/25/30p stake £10/£15 jackpot) (set 1)" ) // % key
GAME_CUSTOM( 199?, m4sky__b,  m4sky, bwboki_chr_cheat<m4sky_keys>(R5, RT3), mpu4impcoin_jackpot15_20p_90pc, init_m4big, "sk_sj_k_.5_0", 0x0000, 0x040000, CRC(e1bab980) SHA1(1c8b127809422ab0baf1875ca907f18269a0cc17), "BWB",u8"Sky Sports Super Soccer (BWB) (MPU4) (20/25/30p stake £10/£15 jackpot) (set 2)" ) // % key
GAME_CUSTOM( 199?, m4sky__c,  m4sky, bwboki_chr_cheat<m4sky_keys>(R5, RT3), mpu4impcoin_jackpot15_20p,      init_m4big, "sk_sja__.5_0", 0x0000, 0x040000, CRC(b2a16ef7) SHA1(9012dcc320e8af8fef53e0dc91d3bcd6cbafa5ee), "BWB",u8"Sky Sports Super Soccer (BWB) (MPU4) (20/25/30p stake £10/£15 jackpot) (set 3)" )
GAME_CUSTOM( 199?, m4sky__d,  m4sky, bwboki_chr_cheat<m4sky_keys>(R5, RT3), mpu4impcoin_jackpot15_20p,      init_m4big, "sk_sjs__.5_0", 0x0000, 0x040000, CRC(d176431f) SHA1(8ca90ef61486fc5a5b6527f913cd05b42ceabe3e), "BWB",u8"Sky Sports Super Soccer (BWB) (MPU4) (20/25/30p stake £10/£15 jackpot) (set 4)" ) // Datapak


/*****************************************************************************************************************************************************************************
*
* Soul Sister
* - some sets boot, some sets 7.1 Datapack error, m4souls__f 4.4 without percent key set
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4souls_keys[2] = { 0x10, 0x3e2f0b };

#define M4SOULS_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", 0 ) \
	ROM_LOAD( "ss______.1_2", 0x000000, 0x080000, CRC(ddea9d75) SHA1(fe5f88d49434109d0f51425e790e179dc1a02767) ) \
	ROM_LOAD( "ss______.1_3", 0x080000, 0x080000, CRC(23d1e57a) SHA1(b17afdaa95522fd7ea6c12f513fa338e1fcb06f6) ) \
	ROM_LOAD( "ss______.1_4", 0x100000, 0x080000, CRC(0ba3046a) SHA1(ec21fa328669bc7a5baf1ce8b9ac05f38f98e360) )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4SOULS_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4souls_keys>(R5, RT3), mpu4_impcoin, mpu4bwb_machines_state, init_m4big ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4souls,     0,          "ss_06a__.4_1", 0x0000, 0x020000, CRC(00390a21) SHA1(d31d1307301fa4e8cf0ce3677e68a4c1723e4404), "BWB","Soul Sister (BWB) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4souls__a,  m4souls,    "ss_16a__.4_1", 0x0000, 0x020000, CRC(b9ab9612) SHA1(ad30916a0f2cc745741c99d23c23192ae4088daf), "BWB","Soul Sister (BWB) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4souls__b,  m4souls,    "ss_26a__.2_1", 0x0000, 0x020000, CRC(bf9acf05) SHA1(13698b453e975a1801631163d06468f07c181b48), "BWB","Soul Sister (BWB) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4souls__c,  m4souls,    "ss_26ad_.2_1", 0x0000, 0x020000, CRC(3e867df2) SHA1(0c7d57c05952cb4f737ee07f139a3593803d3d60), "BWB","Soul Sister (BWB) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4souls__d,  m4souls,    "ss_26sb_.2_1", 0x0000, 0x020000, CRC(2010749e) SHA1(4220b12db7870a2efd7a7ab573e4e08ff0643e70), "BWB","Soul Sister (BWB) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4souls__e,  m4souls,    "ss_26sd_.2_1", 0x0000, 0x020000, CRC(773db916) SHA1(a7d168db22d5adeb3eaf64786bbe744ce787ff68), "BWB","Soul Sister (BWB) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4souls__f,  m4souls,    "ss_26sk_.2_1", 0x0000, 0x020000, CRC(9ae1672e) SHA1(79bcd12fae38dd1b0035e956148ffeaee33b9c71), "BWB","Soul Sister (BWB) (MPU4) (set 7)" )


/*****************************************************************************************************************************************************************************
*
* Spin The Bottle
* - Reel D needs different hookup
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4spinbt_keys[2] = { 0x45, 0x250603 };

#define M4SPINBT_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 ) \
	/* missing? */

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4SPINBT_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4spinbt_keys>(R5, RT3), mpu4_impcoin, mpu4bwb_machines_state, init_m4big ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4spinbt,       0,          "sn_37ad_.5_0", 0x0000, 0x040000, CRC(42d6faaa) SHA1(3789e85981b33ffae7c50ccca3278ae62974972d), "BWB","Spin The Bottle (BWB) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4spinbt__a,    m4spinbt,   "sn_37b__.5_0", 0x0000, 0x040000, CRC(3a259a6f) SHA1(1acabb9e725ae1374b87808c4b3d06a329c824d0), "BWB","Spin The Bottle (BWB) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4spinbt__b,    m4spinbt,   "sn_37bd_.5_0", 0x0000, 0x040000, CRC(f4e3f395) SHA1(545b2ea1cf4231ba1663bea0e6770976b0797cf3), "BWB","Spin The Bottle (BWB) (MPU4) (set 3)" ) // Datapak
GAME_CUSTOM( 199?, m4spinbt__c,    m4spinbt,   "sn_37bg_.5_0", 0x0000, 0x040000, CRC(c3ad8312) SHA1(9463903fe65462e5e9b09a992dc4625dba8452f0), "BWB","Spin The Bottle (BWB) (MPU4) (set 4)" ) // Datapak
GAME_CUSTOM( 199?, m4spinbt__d,    m4spinbt,   "sn_37bt_.5_0", 0x0000, 0x040000, CRC(9fd1df18) SHA1(4c647bc887cdaa2415d0308787c710c83ce1c0d3), "BWB","Spin The Bottle (BWB) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4spinbt__e,    m4spinbt,   "sn_s7a__.5_0", 0x0000, 0x040000, CRC(69d39933) SHA1(ec2a9fb7c4977532a7a431745eb2d82d4e282159), "BWB","Spin The Bottle (BWB) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4spinbt__f,    m4spinbt,   "sn_s7ad_.5_0", 0x0000, 0x040000, CRC(a715f0c9) SHA1(bec188edcd0580465859876f8bff2ff5a392a9e1), "BWB","Spin The Bottle (BWB) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4spinbt__g,    m4spinbt,   "sn_s7s__.5_0", 0x0000, 0x040000, CRC(13288cc5) SHA1(2c47dcd8b57d10e0768729ad91dba12521c0d98a), "BWB","Spin The Bottle (BWB) (MPU4) (set 8)" )


/*****************************************************************************************************************************************************************************
*
* Stars & Stripes
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4starst_keys[2] = { 0x11, 0x3f0a26 };

#define M4STARST_EXTRA_ROMS \
	ROM_REGION( 0x180000, "okicard:msm6376", ROMREGION_ERASE00 ) \
	ROM_LOAD( "starsnd3", 0x0000, 0x080000, CRC(96882952) SHA1(bf366670aec5cb545c56caac3c63855db03d8c14) )/* (?)strange id number? */

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent, machine, inputs, init, name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4STARST_EXTRA_ROMS \
	ROM_END \
	GAME( year, setname, parent, machine, inputs, mpu4bwb_machines_state, init, ROT0, company, title, GAME_FLAGS )

GAME_CUSTOM( 199?, m4starst,       0,        bwboki_chr_cheat<m4starst_keys>(R5, RT3), mpu4impcoin_jackpot8tkn_20p,      init_m4big, "sr_20__d.3_0", 0x0000, 0x040000, CRC(98f6619b) SHA1(fc0a568e6695c9ad0fda7bc6703c752af26a7777), "BWB",u8"Stars & Stripes (BWB) (MPU4) (20p stake / £8 token jackpot) (set 1)" )
GAME_CUSTOM( 199?, m4starst__a,    m4starst, bwboki_chr_cheat<m4starst_keys>(R5, RT3), mpu4impcoin_jackpot8tkn_20p,      init_m4big, "sr_20_bd.3_0", 0x0000, 0x040000, CRC(ff8209de) SHA1(41a4c20c89b3a04612ad6298276472b888915c89), "BWB",u8"Stars & Stripes (BWB) (MPU4) (20p stake / £8 token jackpot) (set 2)" ) // Datapak
GAME_CUSTOM( 199?, m4starst__b,    m4starst, bwboki_chr_cheat<m4starst_keys>(R5, RT3), mpu4impcoin_jackpot8tkn_20p_90pc, init_m4big, "sr_20_kd.3_0", 0x0000, 0x040000, CRC(4c9a53d5) SHA1(43ebf6c06db58de9c3934e2dbba0d8126f3e2dda), "BWB",u8"Stars & Stripes (BWB) (MPU4) (20p stake / £8 token jackpot) (set 3)" ) // % key
GAME_CUSTOM( 199?, m4starst__c,    m4starst, bwboki_chr_cheat<m4starst_keys>(R5, RT3), mpu4impcoin_jackpot8tkn_20p,      init_m4big, "sr_20a_d.3_0", 0x0000, 0x040000, CRC(e9eebb4c) SHA1(60d8010140d9debe8f12d7f810de223d9abd02a4), "BWB",u8"Stars & Stripes (BWB) (MPU4) (20p stake / £8 token jackpot) (set 4)" )
GAME_CUSTOM( 199?, m4starst__d,    m4starst, bwboki_chr_cheat<m4starst_keys>(R5, RT3), mpu4impcoin_jackpot8tkn_20p,      init_m4big, "sr_20s_d.3_0", 0x0000, 0x040000, CRC(725b50e6) SHA1(3efde346022e37b09df08b8188ac76dcdfac8a4e), "BWB",u8"Stars & Stripes (BWB) (MPU4) (20p stake / £8 token jackpot) (set 5)" ) // Datapak
//
GAME_CUSTOM( 199?, m4starst__e,    m4starst, bwboki_chr_cheat<m4starst_keys>(R5, RT3), mpu4impcoin_jackpot15_20p,      init_m4big,  "sr_sj___.5_0", 0x0000, 0x040000, CRC(7964bd86) SHA1(7078de5a61b52dedb776993643f7edd8a2c863c3), "BWB",u8"Stars & Stripes (BWB) (MPU4) (20/25/30p stake £10/£15 jackpot) (set 1)" )
GAME_CUSTOM( 199?, m4starst__f,    m4starst, bwboki_chr_cheat<m4starst_keys>(R5, RT3), mpu4impcoin_jackpot15_20p,      init_m4big,  "sr_sj_b_.5_0", 0x0000, 0x040000, CRC(4ee1f95b) SHA1(1e3d52afd19a9489608d5446ef2118561c6411b0), "BWB",u8"Stars & Stripes (BWB) (MPU4) (20/25/30p stake £10/£15 jackpot) (set 2)" ) // Datapak
GAME_CUSTOM( 199?, m4starst__g,    m4starst, bwboki_chr_cheat<m4starst_keys>(R5, RT3), mpu4impcoin_jackpot15_20p,      init_m4big,  "sr_sj_d_.5_0", 0x0000, 0x040000, CRC(b4d78711) SHA1(c864c944b3fa74aa1fed22afe656a37413b024ce), "BWB",u8"Stars & Stripes (BWB) (MPU4) (20/25/30p stake £10/£15 jackpot) (set 3)" ) // Datapak
GAME_CUSTOM( 199?, m4starst__h,    m4starst, bwboki_chr_cheat<m4starst_keys>(R5, RT3), mpu4impcoin_jackpot15_20p_90pc, init_m4big,  "sr_sj_k_.5_0", 0x0000, 0x040000, CRC(c7681e28) SHA1(a8c1c75df33c85301257147c97d6af8808dad0d2), "BWB",u8"Stars & Stripes (BWB) (MPU4) (20/25/30p stake £10/£15 jackpot) (set 4)" ) // % key
GAME_CUSTOM( 199?, m4starst__i,    m4starst, bwboki_chr_cheat<m4starst_keys>(R5, RT3), mpu4impcoin_jackpot15_20p,      init_m4big,  "sr_sja__.5_0", 0x0000, 0x040000, CRC(aa86c4f2) SHA1(e90fd91f1d14b89714e3fb8236ac9e8a641e4c71), "BWB",u8"Stars & Stripes (BWB) (MPU4) (20/25/30p stake £10/£15 jackpot) (set 5)" )
GAME_CUSTOM( 199?, m4starst__j,    m4starst, bwboki_chr_cheat<m4starst_keys>(R5, RT3), mpu4impcoin_jackpot15_20p,      init_m4big,  "sr_sjs__.5_0", 0x0000, 0x040000, CRC(89e405e4) SHA1(5aa9053e08c27570731f65502c7fb31f0ea0a678), "BWB",u8"Stars & Stripes (BWB) (MPU4) (20/25/30p stake £10/£15 jackpot) (set 6)" ) // Datapak


/*****************************************************************************************************************************************************************************
*
* Super League
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4supleg_keys[2] = { 0x11, 0x310926 };

#define M4SUPLEG_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", 0 ) \
	ROM_LOAD( "sls1.hex", 0x000000, 0x080000, CRC(341e3d3e) SHA1(b42ec737e95e766b1b26d6225416ee0c5cad2663) ) \
	ROM_LOAD( "sls2.hex", 0x080000, 0x080000, CRC(f4ab6f0d) SHA1(4b59608ca16c9d158d4d1ac532e7fbe6ff0da959) ) \
	ROM_LOAD( "sls3.hex", 0x100000, 0x080000, CRC(dcba96a1) SHA1(d474c63b37cb18a0b3b1299b5cacadfd8cd5458b) )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent, machine, inputs, init, name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4SUPLEG_EXTRA_ROMS \
	ROM_END \
	GAME( year, setname, parent, machine, inputs, mpu4bwb_machines_state, init, ROT0, company, title, GAME_FLAGS )

GAME_CUSTOM( 199?, m4supleg,       0,        bwboki_chr_cheat<m4supleg_keys>(R5, RT3), mpu4impcoin_jackpot15_20p, init_m4big, "sl_sj.hex",    0x0000, 0x040000, CRC(254835f7) SHA1(2fafaa3da747edd27d393ad106008e898e465283), "BWB",u8"Super League (BWB) (MPU4) (5/10/20/25/30p stake / £5 jackpot, 20/25/30p stake / £15 jackpot) (set 1)" )
GAME_CUSTOM( 199?, m4supleg__a,    m4supleg, bwboki_chr_cheat<m4supleg_keys>(R5, RT3), mpu4impcoin_jackpot15_20p, init_m4big, "sl_sjs.hex",   0x0000, 0x040000, CRC(98942cd3) SHA1(858fde0a350159d089c6a0e0cc2e2eed6ab2092c), "BWB",u8"Super League (BWB) (MPU4) (5/10/20/25/30p stake / £5 jackpot, 20/25/30p stake / £15 jackpot) (set 2)" ) // Datapak
//
GAME_CUSTOM( 199?, m4supleg__b,    m4supleg, bwboki_chr_cheat<m4supleg_keys>(R5, RT3), mpu4impcoin_jackpot5_5p,   init_m4big, "sl_vc.hex",    0x0000, 0x040000, CRC(1940d117) SHA1(ae7338483ac39e9e1973dde5eb837443512630dd), "BWB",u8"Super League (BWB) (MPU4) (5/10p stake / £5 jackpot) (set 1)" )
GAME_CUSTOM( 199?, m4supleg__c,    m4supleg, bwboki_chr_cheat<m4supleg_keys>(R5, RT3), mpu4impcoin_jackpot5_5p,   init_m4big, "sl_vcd.hex",   0x0000, 0x040000, CRC(7aab16d1) SHA1(6da4e0d9883a48937d00bfc5929b3557de51f60e), "BWB",u8"Super League (BWB) (MPU4) (5/10p stake / £5 jackpot) (set 2)" ) // Datapak
//
GAME_CUSTOM( 199?, m4supleg__d,    m4supleg, bwboki_chr_cheat<m4supleg_keys>(R5, RT3), mpu4impcoin_jackpot15_20p, init_m4big, "sls.hex",      0x0000, 0x040000, CRC(5ad6dbb9) SHA1(ff6f9dcf14df22c7bb2b949fcd5c70f31d4c1928), "BWB",u8"Super League (BWB) (MPU4) (20/25/30p stake / £5/£10/£15 jackpot)" )

#define M4SUPLEGW_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", 0 ) \
	ROM_LOAD( "s_leag.s1",0x000000, 0x080000, CRC(e8d90896) SHA1(4c67507f18b5dc966e2df3685dc6c257f5053e61) ) \
	ROM_LOAD( "sls2.hex", 0x080000, 0x080000, CRC(f4ab6f0d) SHA1(4b59608ca16c9d158d4d1ac532e7fbe6ff0da959) ) \
	ROM_LOAD( "sls3.hex", 0x100000, 0x080000, CRC(dcba96a1) SHA1(d474c63b37cb18a0b3b1299b5cacadfd8cd5458b) )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent, machine, inputs, init, name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4SUPLEGW_EXTRA_ROMS \
	ROM_END \
	GAME( year, setname, parent, machine, inputs, mpu4bwb_machines_state, init, ROT0, company, title, GAME_FLAGS )

GAME_CUSTOM( 199?, m4suplegw, m4supleg, bwboki_chr_cheat<m4supleg_keys>(R5, RT3), mpu4impcoin_jackpot15_20p, init_m4big, "s_leag._pound5", 0x0000, 0x040000, CRC(4c6bd78e) SHA1(f67793a2a16adacc8d92b57050f02cffa50a1283), "BWB",u8"Super League (BWB) (MPU4) (5/10/20/25/30p stake / £5 jackpot, 20/25/30p stake / £15 jackpot) (set 3)" ) // maybe a Whitbread set, but not shown in attract


/*****************************************************************************************************************************************************************************
*
* Super Soccer
* - Multi-stake game (player chooses the stake, not operator)
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4supscr_keys[2] = { 0x11, 0x101231 };

#define M4SUPSCR_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 ) \
	/* missing? */
#undef GAME_CUSTOM

#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4SUPSCR_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4supscr_keys>(R5, RT3), mpu4_impcoin, mpu4bwb_machines_state, init_m4big ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4supscr,       0,          "sm_78___.6_0",         0x0000, 0x040000, CRC(e7022c44) SHA1(da3a5b9954f7e50dce73aeb9c46bd4631c8350d5), "BWB","Super Soccer (ver. 6) (BWB) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4supscr__a,    m4supscr,   "sm_78_d_.6_0",         0x0000, 0x040000, CRC(4dbe6a87) SHA1(fe2ce1fca7105afbf459ee6558744f8fee417169), "BWB","Super Soccer (ver. 6) (BWB) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4supscr__b,    m4supscr,   "sm_80___.6_0",         0x0000, 0x040000, CRC(7e9831ec) SHA1(d0e645ea1f6d34c507103550c248200313d033ef), "BWB","Super Soccer (ver. 6) (BWB) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4supscr__c,    m4supscr,   "sm_80_d_.6_0",         0x0000, 0x040000, CRC(d424772f) SHA1(938eb7a35e59db704b0737d309b7989b35ed43e0), "BWB","Super Soccer (ver. 6) (BWB) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4supscr__d,    m4supscr,   "sm_82___.6_0",         0x0000, 0x040000, CRC(019b4d42) SHA1(b27894f99dc2945fccb0de66f34eacc5b02a3463), "BWB","Super Soccer (ver. 6) (BWB) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4supscr__e,    m4supscr,   "sm_82_d_.6_0",         0x0000, 0x040000, CRC(ab270b81) SHA1(6af522e78b9b263c96cb350cbc8585be206dcd0a), "BWB","Super Soccer (ver. 6) (BWB) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4supscr__f,    m4supscr,   "sm_84___.6_0",         0x0000, 0x040000, CRC(809ec8b0) SHA1(965edf407a1fd7dfda2817b884ef65b3d57d51e5), "BWB","Super Soccer (ver. 6) (BWB) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4supscr__g,    m4supscr,   "sm_84_d_.6_0",         0x0000, 0x040000, CRC(2a228e73) SHA1(04c8d17b2f2800dac9aca81e926e6e48eb02c5ac), "BWB","Super Soccer (ver. 6) (BWB) (MPU4) (set 8)" )
GAME_CUSTOM( 199?, m4supscr__h,    m4supscr,   "sm_86___.6_0",         0x0000, 0x040000, CRC(ff9db41e) SHA1(5996438e801534ebf5d9755b340fada67cadc942), "BWB","Super Soccer (ver. 6) (BWB) (MPU4) (set 9)" )
GAME_CUSTOM( 199?, m4supscr__i,    m4supscr,   "sm_86_d_.6_0",         0x0000, 0x040000, CRC(5521f2dd) SHA1(add1b70a6cddc4e176697660aeff331535d92898), "BWB","Super Soccer (ver. 6) (BWB) (MPU4) (set 10)" )
GAME_CUSTOM( 199?, m4supscr__j,    m4supscr,   "sm_88___.6_0",         0x0000, 0x040000, CRC(59e4c515) SHA1(5a76962dc6530d326bfe6ef6498c8f2ad481d6f1), "BWB","Super Soccer (ver. 6) (BWB) (MPU4) (set 11)" )
GAME_CUSTOM( 199?, m4supscr__k,    m4supscr,   "sm_88_d_.6_0",         0x0000, 0x040000, CRC(f35883d6) SHA1(1948e06e302cdb01fbd32d711374957e9e6bd64a), "BWB","Super Soccer (ver. 6) (BWB) (MPU4) (set 12)" )
GAME_CUSTOM( 199?, m4supscr__l,    m4supscr,   "sm_90___.6_0",         0x0000, 0x040000, CRC(cdc7c594) SHA1(acb829257472bc4420c141932b6f4c708ea04f1b), "BWB","Super Soccer (ver. 6) (BWB) (MPU4) (set 13)" )
GAME_CUSTOM( 199?, m4supscr__m,    m4supscr,   "sm_90_d_.6_0",         0x0000, 0x040000, CRC(677b8357) SHA1(4ec54f8b7d28c0459152309a58bd5c0db1a2f036), "BWB","Super Soccer (ver. 6) (BWB) (MPU4) (set 14)" )
GAME_CUSTOM( 199?, m4supscr__n,    m4supscr,   "multistakesoccer.bin", 0x0000, 0x040000, CRC(ce27b6a7) SHA1(f9038336137b0642da4d1520b5d71a047d8fbe12), "BWB","Super Soccer (ver. 3) (BWB) (MPU4)" )


/*****************************************************************************************************************************************************************************
*
* Sure Thing
* - some sets boot, some sets error 7.1
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4sure_keys[2] = {0x10, 0x281215};

#define M4SURE_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 ) \
	/* missing? */
#undef GAME_CUSTOM

#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4SURE_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4sure_keys>(R5, RT3), mpu4_impcoin, mpu4bwb_machines_state, init_m4 ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4sure,       0,      "su_xf___.3_1", 0x0000, 0x010000, CRC(f85dae5c) SHA1(4c761c355fb6651f1e0cb041342f8a2ff510dfd2), "BWB","Sure Thing (BWB) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4sure__a,    m4sure, "su_xf_b_.3_1", 0x0000, 0x010000, CRC(9a760e0f) SHA1(fdacdae0e2322daa004b2385616dd34626814d42), "BWB","Sure Thing (BWB) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4sure__b,    m4sure, "su_xf_d_.3_1", 0x0000, 0x010000, CRC(5c4a5669) SHA1(55e1e853fdfdbb43e7b61b59ab642fb013a0db0e), "BWB","Sure Thing (BWB) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4sure__c,    m4sure, "suixf___.3_1", 0x0000, 0x010000, CRC(cae80b60) SHA1(23545aaf1cc3a0c8868beafb56eccedbbb6099de), "BWB","Sure Thing (BWB) (MPU4) (set 4)" )


/*****************************************************************************************************************************************************************************
*
* T-Rex
* - some sets boot
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4trex_keys[2] = {0x10, 0x112231};

#define M4TREX_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", 0 ) \
	ROM_LOAD( "tr______.1_2", 0x000000, 0x080000, CRC(75687514) SHA1(dc8f5f1db7da164175c241187cf3f0db1dd71fc9) ) \
	ROM_LOAD( "tr______.1_3", 0x080000, 0x080000, CRC(1e30d4ed) SHA1(8cd916d28f5060d74a0d795f9b75ab597de1cd60) )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4TREX_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4trex_keys>(R5, RT3), mpu4_impcoin, mpu4bwb_machines_state, init_m4 ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4trex,       0,      "tr_20a__.2_1", 0x0000, 0x010000, CRC(21150b8e) SHA1(1531bc6fdb8b787fed6f4f98c6463313c55efc3c), "BWB","T-Rex (BWB) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4trex__a,    m4trex, "tr_20a_p.2_1", 0x0000, 0x010000, CRC(ec1b35bc) SHA1(944959c6d1f8e9b0bb33c659b7c515cb7585fed0), "BWB","T-Rex (BWB) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4trex__b,    m4trex, "tr_20ab_.2_1", 0x0000, 0x010000, CRC(f4190bbf) SHA1(45c20c5e56f0bc39e3af5817eb6d705caef14b40), "BWB","T-Rex (BWB) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4trex__c,    m4trex, "tr_20bg_.2_1", 0x0000, 0x010000, CRC(14d8d80c) SHA1(6db190550b18401a067eeb26890af39612fc3012), "BWB","T-Rex (BWB) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4trex__d,    m4trex, "tr_20bt_.2_1", 0x0000, 0x010000, CRC(3ad19bf3) SHA1(4808f8bbb963be30eace75228d4d1decde3282b0), "BWB","T-Rex (BWB) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4trex__e,    m4trex, "tr_20s__.2_1", 0x0000, 0x010000, CRC(2e9460bd) SHA1(8e2ac58cb1686aadad38356a4d47e9502ddbaa52), "BWB","T-Rex (BWB) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4trex__f,    m4trex, "tr_20s_p.2_1", 0x0000, 0x010000, CRC(e39a5e8f) SHA1(d1d507040fefa959d71856395639778e898272aa), "BWB","T-Rex (BWB) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4trex__g,    m4trex, "tr_20sb_.2_1", 0x0000, 0x010000, CRC(fb98608c) SHA1(48b4fbdc289552131b621c870d7af082b6d8916e), "BWB","T-Rex (BWB) (MPU4) (set 8)" )
GAME_CUSTOM( 199?, m4trex__h,    m4trex, "tr_20sbp.2_1", 0x0000, 0x010000, CRC(36965ebe) SHA1(f2adce7cc97b30b0bf67fd5698b867603973e87a), "BWB","T-Rex (BWB) (MPU4) (set 9)" )
GAME_CUSTOM( 199?, m4trex__i,    m4trex, "tr_20sd_.2_1", 0x0000, 0x010000, CRC(868ea921) SHA1(3055816747b4773ec67669403a81420fabbe327e), "BWB","T-Rex (BWB) (MPU4) (set 10)" )
GAME_CUSTOM( 199?, m4trex__j,    m4trex, "tr_20sdp.2_1", 0x0000, 0x010000, CRC(4b809713) SHA1(d70581ff669d19cf5a91b1546f5c02f27aeda2e4), "BWB","T-Rex (BWB) (MPU4) (set 11)" )
GAME_CUSTOM( 199?, m4trex__k,    m4trex, "tr_20sk_.2_1", 0x0000, 0x010000, CRC(5382a910) SHA1(c8b2811081ec31fecd1b435e775d29e2e6406111), "BWB","T-Rex (BWB) (MPU4) (set 12)" )
GAME_CUSTOM( 199?, m4trex__l,    m4trex, "tr_20skp.2_1", 0x0000, 0x010000, CRC(9e8c9722) SHA1(7aee25966e6d2107f8a8f89acf6af62a73ff05c9), "BWB","T-Rex (BWB) (MPU4) (set 13)" )


/*****************************************************************************************************************************************************************************
*
* Tutti Fruity
* - some sets boot
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4tutbwb_keys[2] = { 0x01, 0x163428 };

#define M4TUTBWB_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 ) \
	/* missing? */
#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4TUTBWB_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4tutbwb_keys>(R5, RT3), mpu4_impcoin, mpu4bwb_machines_state, init_m4 ,ROT0,company,title,GAME_FLAGS )

// these were in with the regular Barcrest / Bwb Tutti Fruity sets, but this looks like something different, maybe a Bwn original with the same name?
// it has a 0x3f fill near the vectors etc. which is typically associated with Bwb originals.
GAME_CUSTOM( 199?, m4tutbwb,       0,          "tu_05___.1a3",         0x0000, 0x010000, CRC(97acc82d) SHA1(be53e60cb8a33b91a7f5556715ab4befe7170dd2), "BWB","Tutti Fruity (BWB) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4tutbwb_a,     m4tutbwb,   "tu_05_d_.1a3",         0x0000, 0x010000, CRC(33bb3018) SHA1(2c2f49c31919682ac03e61a665ce15d835e22467), "BWB","Tutti Fruity (BWB) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4tutbwb_b,     m4tutbwb,   "tu_10___.1a3",         0x0000, 0x010000, CRC(7878827f) SHA1(ac692ae50e63e632d45e7240c2520df83d2baaf5), "BWB","Tutti Fruity (BWB) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4tutbwb_c,     m4tutbwb,   "tu_20___.1a3",         0x0000, 0x010000, CRC(cada1c42) SHA1(6a4048da89a0bffeebfd21549c2d9812cc275bd5), "BWB","Tutti Fruity (BWB) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4tutbwb_d,     m4tutbwb,   "tu_20_b_.1a3",         0x0000, 0x010000, CRC(a8f1bc11) SHA1(03596171540e6490133f374cca69f4fd0359952e), "BWB","Tutti Fruity (BWB) (MPU4) (set 5)" )
GAME_CUSTOM( 199?, m4tutbwb_e,     m4tutbwb,   "tu_20_d_.1a3",         0x0000, 0x010000, CRC(6ecde477) SHA1(694296eb226c59069800d6936c9dee2623105db0), "BWB","Tutti Fruity (BWB) (MPU4) (set 6)" )
GAME_CUSTOM( 199?, m4tutbwb_f,     m4tutbwb,   "tu_20_k_.1a3",         0x0000, 0x010000, CRC(0ce64424) SHA1(7415c9de9982aa7f15f71ef791cbd8ad5a9331d3), "BWB","Tutti Fruity (BWB) (MPU4) (set 7)" )
GAME_CUSTOM( 199?, m4tutbwb_g,     m4tutbwb,   "tu_20bg_.1a3",         0x0000, 0x010000, CRC(31a6196d) SHA1(1113737dd3b209afda14ec273d923e2057ea7d99), "BWB","Tutti Fruity (BWB) (MPU4) (set 8)" )
GAME_CUSTOM( 199?, m4tutbwb_h,     m4tutbwb,   "tuf20__1.0",           0x0000, 0x010000, CRC(ddadbcb6) SHA1(2d2934ec73d979de45d0998f8975361d33358dd3), "BWB","Tutti Fruity (BWB) (MPU4) (set 9)" )
GAME_CUSTOM( 199?, m4tutbwb_i,     m4tutbwb,   "tuf20ad1.0",           0x0000, 0x010000, CRC(5a74ead3) SHA1(3216c8d0c67aaeb18f791a6e1f3f6e30145d6beb), "BWB","Tutti Fruity (BWB) (MPU4) (set 10)" )
GAME_CUSTOM( 199?, m4tutbwb_j,     m4tutbwb,   "tui05___.1a3",         0x0000, 0x010000, CRC(42e3d400) SHA1(4cf914141dfc1f88704403b467176da77369da06), "BWB","Tutti Fruity (BWB) (MPU4) (set 11)" )


/*****************************************************************************************************************************************************************************
*
* Volcano
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4volcan_keys[2] = { 0x11, 0x3f0d26 };

#define M4VOLCAN_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", 0 ) \
	ROM_LOAD( "vo___snd.1_1", 0x000000, 0x080000, CRC(62bb0166) SHA1(17e5557cce4e7841cbcf5d67783fe78452aacc63) ) \
	ROM_LOAD( "vo___snd.1_2", 0x080000, 0x080000, CRC(1eded545) SHA1(0010833e42b33fb0fd621a1059e1cf9a123c3fbd) ) \
	ROM_LOAD( "vo___snd.1_3", 0x100000, 0x080000, CRC(915f4adf) SHA1(fac6644329ee6ef0026d65d8b94c971e01770d45) ) \
	ROM_LOAD( "vo___snd.1_4", 0x180000, 0x080000, CRC(fec0fbe9) SHA1(26f651c5558a80e88666403d01cf916c3a13d948) )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent, machine, inputs, init, name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4VOLCAN_EXTRA_ROMS \
	ROM_END \
	GAME( year, setname, parent, machine, inputs, mpu4bwb_machines_state, init, ROT0, company, title, GAME_FLAGS )

GAME_CUSTOM( 199?, m4volcan,       0,        bwboki_chr_cheat<m4volcan_keys>(R5, RT3), mpu4impcoin_jackpot15_20p,      init_m4big, "vo_sj___.5_0",         0x0000, 0x040000, CRC(78096ebf) SHA1(96915bc2eca00fbd82fab8b3f62e697da118acdd), "BWB",u8"Volcano (BWB) (MPU4) (20/25/30p stake / £5/£10/£15 jackpot) (set 1)" )
GAME_CUSTOM( 199?, m4volcan__a,    m4volcan, bwboki_chr_cheat<m4volcan_keys>(R5, RT3), mpu4impcoin_jackpot15_20p,      init_m4big, "vo_sj_d_.5_0",         0x0000, 0x040000, CRC(87e0347d) SHA1(be5d5b90739fa8ac10f6504290aa58fcf147f323), "BWB",u8"Volcano (BWB) (MPU4) (20/25/30p stake / £5/£10/£15 jackpot) (set 2)" ) // Datapak
GAME_CUSTOM( 199?, m4volcan__b,    m4volcan, bwboki_chr_cheat<m4volcan_keys>(R5, RT3), mpu4impcoin_jackpot15_20p_90pc, init_m4big, "vo_sj_k_.5_0",         0x0000, 0x040000, CRC(8604d102) SHA1(34c7df0257ba02ace4a74ffd5b0eed11eea0c333), "BWB",u8"Volcano (BWB) (MPU4) (20/25/30p stake / £5/£10/£15 jackpot) (set 3)" ) // % key
GAME_CUSTOM( 199?, m4volcan__c,    m4volcan, bwboki_chr_cheat<m4volcan_keys>(R5, RT3), mpu4impcoin_jackpot15_20p,      init_m4big, "vo_sja__.5_0",         0x0000, 0x040000, CRC(d73bade2) SHA1(7e02493ec0710f109ae45e523ef3d7d275aaefab), "BWB",u8"Volcano (BWB) (MPU4) (20/25/30p stake / £5/£10/£15 jackpot) (set 4)" )
GAME_CUSTOM( 199?, m4volcan__d,    m4volcan, bwboki_chr_cheat<m4volcan_keys>(R5, RT3), mpu4impcoin_jackpot15_20p,      init_m4big, "vo_sjs__.5_0",         0x0000, 0x040000, CRC(51eff796) SHA1(d0efb1eb4be176906726a438fcffb50cf5ddd217), "BWB",u8"Volcano (BWB) (MPU4) (20/25/30p stake / £5/£10/£15 jackpot) (set 5)" ) // Datapak
//
GAME_CUSTOM( 199?, m4volcan__e,    m4volcan, bwboki_chr_cheat<m4volcan_keys>(R5, RT3), mpu4impcoin_jackpot5_5p,        init_m4big, "vo_vc___.2_0",         0x0000, 0x040000, CRC(24a9e5d6) SHA1(dd4223c3b5c024eb9d56bb45426e327b49f78dde), "BWB",u8"Volcano (BWB) (MPU4) (5/10p stake / £5/£8 jackpot) (set 1)" )
GAME_CUSTOM( 199?, m4volcan__f,    m4volcan, bwboki_chr_cheat<m4volcan_keys>(R5, RT3), mpu4impcoin_jackpot5_5p,        init_m4big, "vo_vc_d_.2_0",         0x0000, 0x040000, CRC(7f7341b6) SHA1(23d46ca0eed1e942b2a0d33d6ada2434ded5819b), "BWB",u8"Volcano (BWB) (MPU4) (5/10p stake / £5/£8 jackpot) (set 2)" ) // Datapak
GAME_CUSTOM( 199?, m4volcan__g,    m4volcan, bwboki_chr_cheat<m4volcan_keys>(R5, RT3), mpu4impcoin_jackpot5_5p,        init_m4big, "volcano_bwb_2-0.bin",  0x0000, 0x040000, CRC(20688684) SHA1(fe533341417a3a0b16f485351cb635f4e7d823db), "BWB",u8"Volcano (BWB) (MPU4) (5/10p stake / £5/£8 jackpot) (set 3)" )


/*****************************************************************************************************************************************************************************
*
* Voodoo Express
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4vdexpr_keys[2] = { 0x11, 0x3f0d27 };

#define M4VDEXPR_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", 0 ) \
	ROM_LOAD( "vd___snd.1_1", 0x000000, 0x080000, CRC(d66b43a9) SHA1(087cf1571a9afb8c1c7cac13640fa453b614fd53) ) \
	ROM_LOAD( "vd___snd.1_2", 0x080000, 0x080000, CRC(a501c887) SHA1(c56a05fd8196afb86e665fec3fe7d02b9bf94c1a) ) \
	ROM_LOAD( "vd___snd.1_3", 0x100000, 0x080000, CRC(70c6bd96) SHA1(ecdd4276ff72939433630e04bba5be3df569e17e) ) \
	ROM_LOAD( "vd___snd.1_4", 0x180000, 0x080000, CRC(a6753f41) SHA1(b4af3054b62c3f00f2b5a19b816507fc3a62bef4) )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent, machine, inputs, init, name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4VDEXPR_EXTRA_ROMS \
	ROM_END \
	GAME( year, setname, parent, machine, inputs, mpu4bwb_machines_state, init, ROT0, company, title, GAME_FLAGS )

GAME_CUSTOM( 199?, m4vdexpr,       0,        bwboki_chr_cheat<m4vdexpr_keys>(R5, RT3), mpu4impcoin_jackpot15_20p, init_m4big, "vd_sj___.2_0", 0x0000, 0x040000, CRC(03efd2a5) SHA1(4fc3695c24335aef11ba168f660fb519d8c9d473), "BWB","Voodoo Express (BWB) (MPU4) (20/25/30p stake / £5/£10/£15 jackpot) (set 1)" )
GAME_CUSTOM( 199?, m4vdexpr__a,    m4vdexpr, bwboki_chr_cheat<m4vdexpr_keys>(R5, RT3), mpu4impcoin_jackpot15_20p, init_m4big, "vd_sj_d_.2_0", 0x0000, 0x040000, CRC(5073b98e) SHA1(66b020b8c096e78e1c9694f1cbc139e97314ab48), "BWB","Voodoo Express (BWB) (MPU4) (20/25/30p stake / £5/£10/£15 jackpot) (set 2)" ) // Datapak
GAME_CUSTOM( 199?, m4vdexpr__b,    m4vdexpr, bwboki_chr_cheat<m4vdexpr_keys>(R5, RT3), mpu4impcoin_jackpot15_20p, init_m4big, "vd_sja__.2_0", 0x0000, 0x040000, CRC(c53dbf48) SHA1(ceee2de3ea8cb511540d90b87bc67bec3309de35), "BWB","Voodoo Express (BWB) (MPU4) (20/25/30p stake / £5/£10/£15 jackpot) (set 3)" )
GAME_CUSTOM( 199?, m4vdexpr__c,    m4vdexpr, bwboki_chr_cheat<m4vdexpr_keys>(R5, RT3), mpu4impcoin_jackpot15_20p, init_m4big, "vd_sjs__.2_0", 0x0000, 0x040000, CRC(036157b3) SHA1(b575751006c3ee59bf0404fa0e177fee9ef9c5db), "BWB","Voodoo Express (BWB) (MPU4) (20/25/30p stake / £5/£10/£15 jackpot) (set 4)" ) // Datapak
//
GAME_CUSTOM( 199?, m4vdexpr__d,    m4vdexpr, bwboki_chr_cheat<m4vdexpr_keys>(R5, RT3), mpu4impcoin_jackpot5_5p,   init_m4big, "vd_vc___.1_0", 0x0000, 0x040000, CRC(6326e14a) SHA1(3dbfbb1cfb60dc10c6972aa2fda89c8e3c3107ea), "BWB","Voodoo Express (BWB) (MPU4) (5/10p stake / £5/£8 jackpot)" )


/*****************************************************************************************************************************************************************************
*
* X-change
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4xch_keys[2] = { 0x42, 0x3f0c26 };

#define M4XCH_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", 0 ) \
	ROM_LOAD( "xchasnd.bin", 0x0000, 0x080000, CRC(32c44cd5) SHA1(baafb48e6f95ba152942d1e1c273ffb3c95afa82) ) // BADADDR     --xxxxxxxxxxxxxxxxx

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent, machine, inputs, init, name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4XCH_EXTRA_ROMS \
	ROM_END \
	GAME( year, setname, parent, machine, inputs, mpu4bwb_machines_state, init, ROT0, company, title, GAME_FLAGS )

// these don't require stake/jp to be set
GAME_CUSTOM( 199?, m4xch,     0,     bwboki_chr_cheat<m4xch_keys>(R5, RT3), mpu4_impcoin,              init_m4big, "ec_25b__.b_0", 0x0000, 0x020000, CRC(cec9e836) SHA1(460ec38566d7608e51b62f1ffebc18a395002ed4), "BWB",u8"X-change (BWB) (MPU4) (25p stake / £10 jackpot)" )
//
GAME_CUSTOM( 199?, m4xch__a,  m4xch, bwboki_chr_cheat<m4xch_keys>(R5, RT3), mpu4_impcoin,              init_m4big, "ec_36bg_.bv0", 0x0000, 0x020000, CRC(c5d1523a) SHA1(813916008d7e7576e4594a6eb79a76c514470f31), "BWB",u8"X-change (BWB) (MPU4) (25p stake / £8 jackpot) (set 1)" ) // Datapak
GAME_CUSTOM( 199?, m4xch__b,  m4xch, bwboki_chr_cheat<m4xch_keys>(R5, RT3), mpu4_impcoin,              init_m4big, "ec_36bgn.bv0", 0x0000, 0x020000, CRC(4be33ee1) SHA1(888009e09c59f30649eac3238e0b70dec258cb3c), "BWB",u8"X-change (BWB) (MPU4) (25p stake / £8 jackpot) (set 2)" )
//
GAME_CUSTOM( 199?, m4xch__c,  m4xch, bwboki_chr_cheat<m4xch_keys>(R5, RT3), mpu4_impcoin,              init_m4big, "ec_39b__.b_0", 0x0000, 0x020000, CRC(e5d5961d) SHA1(ac3916cba91a13d1e0820982a1cefabd378647c9), "BWB",u8"X-change (BWB) (MPU4) (25p stake / £15 jackpot) (set 1)" )
GAME_CUSTOM( 199?, m4xch__d,  m4xch, bwboki_chr_cheat<m4xch_keys>(R5, RT3), mpu4_impcoin,              init_m4big, "ec_39bg_.b_0", 0x0000, 0x020000, CRC(49b2be01) SHA1(dbf808d0949a9658d23e57e7eaaa520891a4f0e0), "BWB",u8"X-change (BWB) (MPU4) (25p stake / £15 jackpot) (set 2)" ) // Datapak
GAME_CUSTOM( 199?, m4xch__e,  m4xch, bwboki_chr_cheat<m4xch_keys>(R5, RT3), mpu4_impcoin,              init_m4big, "ec_39bm_.b_0", 0x0000, 0x020000, CRC(29c86cf0) SHA1(384a475d34ad5d15b68d019c3771aba471f89b4d), "BWB",u8"X-change (BWB) (MPU4) (25p stake / £15 jackpot) (set 3)" )
//
GAME_CUSTOM( 199?, m4xch__f,  m4xch, bwboki_chr_cheat<m4xch_keys>(R5, RT3), mpu4_impcoin,              init_m4big, "ec_49bd_.b_0", 0x0000, 0x020000, CRC(02854189) SHA1(beab97ccd9889cfdb24e1fb6fb373bf9fd114eab), "BWB",u8"X-change (BWB) (MPU4) (30p stake / £15 jackpot) (set 1)" ) // Datapak
GAME_CUSTOM( 199?, m4xch__g,  m4xch, bwboki_chr_cheat<m4xch_keys>(R5, RT3), mpu4_impcoin,              init_m4big, "ec_49bmd.b_0", 0x0000, 0x020000, CRC(2a5578e3) SHA1(d8f100bc83721b6b0f365be7a962249af79d6162), "BWB",u8"X-change (BWB) (MPU4) (30p stake / £15 jackpot) (set 2)" )
//
GAME_CUSTOM( 199?, m4xch__h,  m4xch, bwboki_chr_cheat<m4xch_keys>(R5, RT3), mpu4impcoin_jackpot15_20p, init_m4big, "ec_s9bt_.b_0", 0x0000, 0x020000, CRC(7e0a27d1) SHA1(d4a23a6c358e38a1a66a06b82af85c844f684830), "BWB",u8"X-change (BWB) (MPU4) (20/25/30p stake / £15 jackpot)" )
//
GAME_CUSTOM( 199?, m4xch__i,  m4xch, bwboki_chr_cheat<m4xch_keys>(R5, RT3), mpu4impcoin_jackpot15_20p, init_m4big, "ec_sja__.a_0", 0x0000, 0x020000, CRC(1f923f89) SHA1(84486287d55591c7e81c59a10e8cc722ec21e8f9), "BWB",u8"X-change (BWB) (MPU4) (10/20/25/30p stake / £5/£8/£10 jackpot, 20/25/30p stake / £15 jackpot) (set 1)" ) // 10p stake not valid with £15 jackpot
GAME_CUSTOM( 199?, m4xch__j,  m4xch, bwboki_chr_cheat<m4xch_keys>(R5, RT3), mpu4impcoin_jackpot15_20p, init_m4big, "ec_sja__.b_0", 0x0000, 0x020000, CRC(16d7b8bb) SHA1(be8ab98a64aa976e25cb302b68323c6781034f2b), "BWB",u8"X-change (BWB) (MPU4) (10/20/25/30p stake / £5/£8/£10 jackpot, 20/25/30p stake / £15 jackpot) (set 2)" )
// probably just a bad dump (half size) very close to the first half of ec_sja__.a_0
GAME_CUSTOM( 199?, m4xch__k,  m4xch, bwboki_chr_cheat<m4xch_keys>(R5, RT3), mpu4_impcoin,              init_m4,     "xchange.bin",  0x0000, 0x010000, CRC(c96cd014) SHA1(6e32d10c18b6b34dbcb21e75925a77e810ffe892), "BWB",u8"X-change (BWB) (MPU4) (unknown set, bad?)" )


/*****************************************************************************************************************************************************************************
*
* X-s
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4xs_keys[2] = { 0x34, 0x3f0e26 };

#define M4XS_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 ) \
	/* missing? */

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent, machine, inputs, init, name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4XS_EXTRA_ROMS \
	ROM_END \
	GAME( year, setname, parent, machine, inputs, mpu4bwb_machines_state, init, ROT0, company, title, GAME_FLAGS )

// these sets don't require stake/jp set
GAME_CUSTOM( 199?, m4xs,       0,    bwboki_chr_cheat<m4xs_keys>(R5, RT3), mpu4_impcoin,              init_m4big ,  "es_39b__.3_0", 0x0000, 0x020000, CRC(ba478372) SHA1(c13f9cc4261e91119aa694ec3ac81d94d9f32d22), "BWB",u8"X-s (BWB) (MPU4) (25p stake / £15 jackpot) (set 1)" )
GAME_CUSTOM( 199?, m4xs__a,    m4xs, bwboki_chr_cheat<m4xs_keys>(R5, RT3), mpu4_impcoin,              init_m4big ,  "es_39bg_.3_0", 0x0000, 0x020000, CRC(b689f14f) SHA1(0c3253e1f747a979f55d53fe637fc61cf50e01a3), "BWB",u8"X-s (BWB) (MPU4) (25p stake / £15 jackpot) (set 2)" ) // Datapak
GAME_CUSTOM( 199?, m4xs__b,    m4xs, bwboki_chr_cheat<m4xs_keys>(R5, RT3), mpu4_impcoin,              init_m4big ,  "es_39bm_.3_0", 0x0000, 0x020000, CRC(934f5d1e) SHA1(1ffd462d561d4a16f2392cc90a139499b74a234a), "BWB",u8"X-s (BWB) (MPU4) (25p stake / £15 jackpot) (set 3)" )
GAME_CUSTOM( 199?, m4xs__c,    m4xs, bwboki_chr_cheat<m4xs_keys>(R5, RT3), mpu4_impcoin,              init_m4big ,  "es_39bmd.3_0", 0x0000, 0x020000, CRC(cf663dcc) SHA1(620e65528687eb5fd6fd879e305e8b7da9b95253), "BWB",u8"X-s (BWB) (MPU4) (25p stake / £15 jackpot) (set 4)" ) // Datapak
//
GAME_CUSTOM( 199?, m4xs__d,    m4xs, bwboki_chr_cheat<m4xs_keys>(R5, RT3), mpu4_impcoin,              init_m4big ,  "es_49bg_.3_0", 0x0000, 0x020000, CRC(b76f1d7d) SHA1(9b43a9e847db3d4024f978b6f996534b8d52368b), "BWB",u8"X-s (BWB) (MPU4) (30p stake / £15 jackpot) (set 1)" ) // Datapak
GAME_CUSTOM( 199?, m4xs__e,    m4xs, bwboki_chr_cheat<m4xs_keys>(R5, RT3), mpu4_impcoin,              init_m4big ,  "es_49bmd.3_0", 0x0000, 0x020000, CRC(1150e499) SHA1(25d2c37e5287f73d2b11608c50f21072422850f0), "BWB",u8"X-s (BWB) (MPU4) (30p stake / £15 jackpot) (set 2)" ) // Datapak
// requires stake/jp
GAME_CUSTOM( 199?, m4xs__f,    m4xs, bwboki_chr_cheat<m4xs_keys>(R5, RT3), mpu4impcoin_jackpot15_20p, init_m4big ,  "es_sja__.3_0", 0x0000, 0x020000, CRC(5909092d) SHA1(64df6ad5ba5ac74592b525af2f4cab8a092a5766), "BWB",u8"X-s (BWB) (MPU4) (20/25/30p stake / £15 jackpot)" )


/*****************************************************************************************************************************************************************************
*
* X-treme
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4xtrm_keys[2] = { 0x36, 0x301210 };

#define M4XTRM_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 ) \
	/* missing? */

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent, machine, inputs, init, name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4XTRM_EXTRA_ROMS \
	ROM_END \
	GAME( year, setname, parent, machine, inputs, mpu4bwb_machines_state, init, ROT0, company, title, GAME_FLAGS )

// state/jp needed
GAME_CUSTOM( 199?, m4xtrm,       0,      bwboki_chr_cheat<m4xtrm_keys>(R5, RT3), mpu4impcoin_jackpot15_20p, init_m4big , "et_sja__.2_0", 0x0000, 0x020000, CRC(8ee2602b) SHA1(b9a779b900ac71ec842dd7eb1643f7a2f1cb6a38), "BWB",u8"X-treme (BWB) (MPU4) (20/25/30p stake / £8/£15 jackpot)" )
// no stake/jp needed on sets below
GAME_CUSTOM( 199?, m4xtrm__a,    m4xtrm, bwboki_chr_cheat<m4xtrm_keys>(R5, RT3), mpu4_impcoin,              init_m4big , "et_49bg_.2_0", 0x0000, 0x020000, CRC(f858d927) SHA1(e7ab84c8898a95075a41fb0249e4b103d60e7d85), "BWB",u8"X-treme (BWB) (MPU4) (30p stake / £15 jackpot)" ) // Datapak
//
GAME_CUSTOM( 199?, m4xtrm__b,    m4xtrm, bwboki_chr_cheat<m4xtrm_keys>(R5, RT3), mpu4_impcoin,              init_m4big , "et_39bg_.2_0", 0x0000, 0x020000, CRC(db1a3c3c) SHA1(081c934ebfc0a9dfa195bb20f51e025e53d9c4b9), "BWB",u8"X-treme (BWB) (MPU4) (25p stake / £15 jackpot)" ) // Datapak


/*****************************************************************************************************************************************************************************
*
* German versions from Nova are below.
*
* Export cabinets were often different despite having the same game name, so not paired with the above UK releases
*
*****************************************************************************************************************************************************************************/


/*****************************************************************************************************************************************************************************
*
* Blues Boys (Nova, German version)
*  - error 1.7
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4bluesn_keys[2] = { 0x23, 0x1b0b36 };

#define M4BLUESN_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 ) \
	ROM_LOAD( "bluesboyz.bi2", 0x000000, 0x080000, CRC(7f19a61b) SHA1(dd8742d84df24e118bdbffb1efffad1c71eb2283) ) \
	ROM_LOAD( "bluesboyz.bi3", 0x080000, 0x080000, CRC(32363184) SHA1(8f3f53ce4d9f9b54c441263def9d8e23880507a1) ) \
	ROM_LOAD( "bluesboyz.bi4", 0x100000, 0x080000, CRC(aa94281d) SHA1(e15b7bf97b8e307ed465d9b8cb6e5de0044f6fb5) ) \
	ROM_LOAD( "bluesboyz.bi5", 0x180000, 0x080000, CRC(d8d7aa2e) SHA1(2d8b86fa63e6649d628c7e343d8f5c329c8f8ced) )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4BLUESN_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4bluesn_keys>(R5, RT3), mpu4_impcoin, mpu4bwb_machines_state, init_m4big ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4bluesn,   m4blsbys,   "bluesboys.bin", 0x0000, 0x020000, CRC(c1395649) SHA1(3cd0eed1f966f5391fe5de496dc747385ebfb556), "BWB","Blues Boys (Nova) (MPU4)" )


/*****************************************************************************************************************************************************************************
*
* Cup Final (Nova, German version)
*  - error 1.0
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4cfinln_keys[2] = { 0x10, 0x213623 };

#define M4CFINLN_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 ) \
	/* missing? */

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4CFINLN_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4cfinln_keys>(R5, RT3), mpu4_impcoin, mpu4bwb_machines_state, init_m4big ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4cfinln,       m4cpfinl,   "cfd_7_1.bin",  0x0000, 0x020000, CRC(e42ec2aa) SHA1(6495448c1d11ce0ab9ad794bc3a0981432e22945), "BWB","Cup Final (Nova) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4cfinln__a,    m4cpfinl,   "cfd_d0.bin",   0x0000, 0x020000, CRC(179fcf13) SHA1(abd18ed28118ba0a62ab321a9d963105946d5eef), "BWB","Cup Final (Nova) (MPU4) (set 2)" )


/*****************************************************************************************************************************************************************************
*
* World Cup (Nova, German version)
*  - error 1.0
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4wcnov_keys[2] = { 0x19, 0x0f2029 };

#define M4WCNOV_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 ) \
	/* missing? */

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4WCNOV_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4wcnov_keys>(R5, RT3), mpu4_impcoin, mpu4bwb_machines_state, init_m4big ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4wcnov,     0,  "wcdsxh__.5_0", 0x0000, 0x080000, CRC(a82d11de) SHA1(ece14fd5f56da8cc788c53d5c1404275e9000b65), "BWB","World Cup (Nova) (MPU4)" )


/*****************************************************************************************************************************************************************************
*
* Excalibur (Nova, German version)
*  - error 1.0
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4excaln_keys[2] = { 0x10, 0x010e20 };

#define M4EXCALN_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 ) \
	/* missing? */

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4EXCALN_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4excaln_keys>(R5, RT3), mpu4_impcoin, mpu4bwb_machines_state, init_m4big ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4excaln,   m4excal,    "exdsx___.6_0", 0x0000, 0x080000, CRC(fcdc703c) SHA1(927870723106aebbb2b492ce9bfebe4aa25d0325), "BWB","Excalibur (Nova) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4excaln__a,m4excal,    "exdsx_e_.6_0", 0x0000, 0x080000, CRC(f6421feb) SHA1(5b3cf7fa4bf9711097ed1c9d2d5689329d73193d), "BWB","Excalibur (Nova) (MPU4) (set 2)" )


/*****************************************************************************************************************************************************************************
*
* Olympic Gold (Nova, German version)
*  - error 1.0
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4olygn_keys[2] = { 0x19, 0x0f2029};

#define M4OLYGN_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 ) \
	/* missing? */

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4OLYGN_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4olygn_keys>(R5, RT3), mpu4_invimpcoin, mpu4bwb_machines_state, init_m4big ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4olygn,     0,          "ogdsx___.8_0", 0x0000, 0x040000, CRC(b51a2538) SHA1(d54f37dc14c44ab66e6d6ba6e2df8bc9ed003054), "BWB","Olympic Gold (German) (Nova) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4olygn__a,  m4olygn,    "ogdsxe__.8_0", 0x0000, 0x040000, CRC(13aa70aa) SHA1(3878c181ec07e24060935bec96e5128e6e4baf31), "BWB","Olympic Gold (German) (Nova) (MPU4) (set 2)" )


/*****************************************************************************************************************************************************************************
*
* Find The Lady (Nova, German version)
* - error 1.0
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4ftladn_keys[2] = { 0x05, 0x200f03 };

#define M4FTLADN_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 ) \
	/* missing? */

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4FTLADN_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4ftladn_keys>(R5, RT3), mpu4_invimpcoin, mpu4bwb_machines_state, init_m4big ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4ftladn,   0,      "fidse___.5_0", 0x00000, 0x20000, CRC(62347bbf) SHA1(2b1cd5adda831a8c74c9484ee1b616259d3e3981), "BWB","Find the Lady (Nova) (MPU4)" )


/*****************************************************************************************************************************************************************************
*
* Sinbad (Nova, German version)
* - error 1.0
*
*****************************************************************************************************************************************************************************/

static const uint32_t m4sinbdn_keys[2] = { 0x10, 0x260c2c };
static const uint32_t m4sinbdn__a_keys[2] = { 0x10, 0x381d06 };
static const uint32_t m4sinbdn__b_keys[2] = { 0x10, 0x3e2f0b };


#define M4SINBDN_EXTRA_ROMS \
	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 ) \
	/* missing? */

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4SINBDN_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4sinbdn_keys>(R5, RT3), mpu4_invimpcoin, mpu4bwb_machines_state, init_m4big ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4sinbdn,       0,          "sbds3___.a_1", 0x0000, 0x020000, CRC(9bff0e40) SHA1(f8a1263a58f828554e9df77ed0db78e627666fb5), "BWB","Sinbad (Nova) (MPU4) (set 1)" )
GAME_CUSTOM( 199?, m4sinbdn__e,    m4sinbdn,   "sbds3__l.9_0", 0x0000, 0x040000, CRC(e425375a) SHA1(d2bdd8e768fc7764054eff574360f3cfb5f4f66d), "BWB","Sinbad (Nova) (MPU4) (set 6)" )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4SINBDN_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4sinbdn__a_keys>(R5, RT3), mpu4_invimpcoin, mpu4bwb_machines_state, init_m4big ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4sinbdn__a,    m4sinbdn,   "sbds3___.7w1", 0x0000, 0x020000, CRC(23bc9ce0) SHA1(f750de2b781bc902c65de7109e10a5fc2d4e1c61), "BWB","Sinbad (Nova) (MPU4) (set 2)" )
GAME_CUSTOM( 199?, m4sinbdn__f,    m4sinbdn,   "sbds3__l.aw0", 0x0000, 0x040000, CRC(c484ef9d) SHA1(62f6644b83dd6abaf80809217edf6a8230a89268), "BWB","Sinbad (Nova) (MPU4) (set 7)" )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4SINBDN_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent, bwboki_chr_cheat<m4sinbdn__b_keys>(R5, RT3), mpu4_invimpcoin, mpu4bwb_machines_state, init_m4big ,ROT0,company,title,GAME_FLAGS )

GAME_CUSTOM( 199?, m4sinbdn__b,    m4sinbdn,   "sxdsx___.2_0", 0x0000, 0x040000, CRC(4e1f98b5) SHA1(3e16e7a0cdccc9eb1a1bb6f9a0332c4582483eee), "BWB","Sinbad (Nova) (MPU4) (set 3)" )
GAME_CUSTOM( 199?, m4sinbdn__c,    m4sinbdn,   "sdd__.3_0",    0x0000, 0x040000, CRC(100098c1) SHA1(b125855c49325972f620463e32fdf124222e27d2), "BWB","Sinbad (Nova) (MPU4) (set 4)" )
GAME_CUSTOM( 199?, m4sinbdn__d,    m4sinbdn,   "sdds3__l.1w0", 0x0000, 0x040000, CRC(feedb8cf) SHA1(620b5379164d4da1200d4807199c2dc78d7d89ee), "BWB","Sinbad (Nova) (MPU4) (set 5)" )
