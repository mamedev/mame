// license:BSD-3-Clause
// copyright-holders:David Haywood
/* MPU4 'Mod2' Sets

 This is the original MPU4 board, with AY-3-8913 chip.
 If a set requires samples it doesn't belong here!

 The AY-3-8913 was only present on the original MOD2
 board, although you could apparently fit an AY-3-8912
 (which also has one I/O port) on a MOD4 if you wanted.

*/

#include "emu.h"
#include "mpu4.h"

#include "machine/timekpr.h"

namespace {

class mpu4mod2_machines_state : public mpu4_state
{
public:
	mpu4mod2_machines_state(const machine_config &mconfig, device_type type, const char *tag) :
		mpu4_state(mconfig, type, tag)
	{
	}

	template<const uint8_t* Table> void mod2_cheatchr_pal_f(machine_config &config);
	template<const uint8_t* Table> void mod2_cheatchr_pal_rtc_f(machine_config &config);

	// bootleg mod2
	template<uint8_t Fixed> void mod2_bootleg_fixedret_f(machine_config &config);

	void mod2_chr_blastbnk_f(machine_config &config);
	void mod2_chr_copcash_f(machine_config &config);

	void mod4psg_f(machine_config &config);

	template<const uint8_t* Table, typename... T>
	auto mod2_cheatchr_pal(T... traits)
	{
		return trait_wrapper(this, &mpu4mod2_machines_state::mod2_cheatchr_pal_f<Table>, traits...);
	}
	template<const uint8_t* Table, typename... T>
	auto mod2_cheatchr_pal_rtc(T... traits)
	{
		return trait_wrapper(this, &mpu4mod2_machines_state::mod2_cheatchr_pal_rtc_f<Table>, traits...);
	}
	template<uint8_t Fixed, typename... T>
	auto mod2_bootleg_fixedret(T... traits)
	{
		return trait_wrapper(this, &mpu4mod2_machines_state::mod2_bootleg_fixedret_f<Fixed>, traits...);
	}
	template<typename... T>
	auto mod2_chr_blastbnk(T... traits)
	{
		return trait_wrapper(this, &mpu4mod2_machines_state::mod2_chr_blastbnk_f, traits...);
	}
	template<typename... T>
	auto mod2_chr_copcash(T... traits)
	{
		return trait_wrapper(this, &mpu4mod2_machines_state::mod2_chr_copcash_f, traits...);
	}
	template<typename... T>
	auto mod4psg(T... traits)
	{
		return trait_wrapper(this, &mpu4mod2_machines_state::mod4psg_f, traits...);
	}

	void init_m4test();

private:
	void mpu4_memmap_characteriser_rtc(address_map &map) ATTR_COLD;

	void ay8912_outport_w(uint8_t data);
};

template<const uint8_t* Table> void mpu4mod2_machines_state::mod2_cheatchr_pal_f(machine_config &config)
{
	mod2_f(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &mpu4mod2_machines_state::mpu4_memmap_characteriser);

	MPU4_CHARACTERISER_PAL(config, m_characteriser, 0);
	m_characteriser->set_cpu_tag("maincpu");
	m_characteriser->set_allow_6809_cheat(true);
	m_characteriser->set_lamp_table(Table);
}


// bootleg mod2
template<uint8_t Fixed> void mpu4mod2_machines_state::mod2_bootleg_fixedret_f(machine_config &config)
{
	mod2_f(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &mpu4mod2_machines_state::mpu4_memmap_bootleg_characteriser);

	MPU4_CHARACTERISER_BL(config, m_characteriser_bl, 0);
	m_characteriser_bl->set_bl_fixed_return(Fixed);
}

void mpu4mod2_machines_state::mpu4_memmap_characteriser_rtc(address_map &map)
{
	mpu4_memmap_characteriser(map);

	map(0x1000, 0x17ff).rw("rtc", FUNC(timekeeper_device::read), FUNC(timekeeper_device::write));
}

template<const uint8_t* Table> void mpu4mod2_machines_state::mod2_cheatchr_pal_rtc_f(machine_config &config)
{
	mod2_cheatchr_pal_f<Table>(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &mpu4mod2_machines_state::mpu4_memmap_characteriser_rtc);

	M48T02(config, "rtc");
}


void mpu4mod2_machines_state::mod2_chr_blastbnk_f(machine_config &config)
{
	mod2_f(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &mpu4mod2_machines_state::mpu4_memmap_bl_characteriser_blastbank);

	MPU4_CHARACTERISER_BL_BLASTBANK(config, m_characteriser_blastbank, 0);
}

void mpu4mod2_machines_state::mod2_chr_copcash_f(machine_config &config)
{
	mod2_f(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &mpu4mod2_machines_state::mpu4_memmap_bl_characteriser_blastbank);

	MPU4_CHARACTERISER_BL_BLASTBANK(config, m_characteriser_blastbank, 0);
	m_characteriser_blastbank->set_retxor(0x03);
}



void mpu4mod2_machines_state::ay8912_outport_w(uint8_t data)
{
	// TODO: figure out what maps here
	logerror("%s: AY-3-8912 output port <- $%02X\n", machine().describe_context(), data);
}

void mpu4mod2_machines_state::mod4psg_f(machine_config &config)
{
	mpu4base(config);
	AY8912(config, m_ay8913, MPU4_MASTER_CLOCK/4);
	m_ay8913->port_a_write_callback().set(FUNC(mpu4mod2_machines_state::ay8912_outport_w));
	m_ay8913->set_flags(AY8910_SINGLE_OUTPUT);
	m_ay8913->set_resistors_load(820, 0, 0);
	m_ay8913->add_route(ALL_OUTPUTS, "mono", 1.0);
}


#include "m4actclb.lh"
#include "m4actpak.lh"
#include "m4alladv.lh"
#include "m4alpha.lh"
#include "connect4.lh"

} // anonymous namespace


INPUT_PORTS_START( connect4 )
	PORT_START("ORANGE1")
	PORT_BIT(0xFF, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("ORANGE2")
	PORT_BIT(0xFF, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("BLACK1")
	PORT_BIT(0x1F, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Test Switch")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_INTERLOCK) PORT_NAME("Cashbox (Back) Door")  PORT_CODE(KEYCODE_Q) PORT_TOGGLE

	PORT_START("BLACK2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Select")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START2) PORT_NAME("Pass / Collect")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Play / Continue")
	PORT_BIT(0x70, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Drop")

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
	PORT_BIT(0xFF, IP_ACTIVE_HIGH, IPT_UNUSED)

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



ROM_START( m4rsg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rsg1.bin", 0xc000, 0x004000, CRC(10c64308) SHA1(b681720bb6edc0fd241c2961b6b2b065d1bbe38c) )
	ROM_LOAD( "rsg2.bin", 0x8000, 0x004000, CRC(975f66e1) SHA1(f74fb6db0b41596c7f0e12ff643b078ac6b93beb) )
	ROM_LOAD( "rsg3.bin", 0x6000, 0x002000, CRC(9a355e95) SHA1(041ec72af65e5dca32dc524b3874449ffd40df64) )
ROM_END

ROM_START( m4rsga )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rsg_p1.bin", 0xc000, 0x004000, CRC(cd582779) SHA1(6970568298229cfa9ef9f3dd7793c1637842f396) )
	ROM_LOAD( "rsg_p2.bin", 0x8000, 0x004000, CRC(1a167031) SHA1(b539bc4ba79cd316c0492082d54e148fd50beb19) )
	ROM_LOAD( "rsg_p3.bin", 0x6000, 0x002000, CRC(a7112951) SHA1(5abc89d360ef60d7e6cf5c89562cbc648b615544) )
ROM_END


ROM_START( m4crkpot )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cp1_2.p1", 0xc000, 0x004000, CRC(fae352c7) SHA1(a0a15e6e4af58376d871fb5e719526d71727b098) )
	ROM_LOAD( "cp1_2.p2", 0x8000, 0x004000, CRC(96e095bb) SHA1(329b6e146a3021fcd79e03fab5e7067aeb65324b) )
	ROM_LOAD( "cp1_2.p3", 0x6000, 0x002000, CRC(ab648a6e) SHA1(b2d2a27bc2e1cee05f442817eb243aef04ed2894) )
ROM_END

ROM_START( m4crkpota )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cp38jp1.bin", 0xc000, 0x004000, CRC(0b0794f3) SHA1(7a039c26dd7e3c0fe22c1da5f07167378a3eb2ca) )
	ROM_LOAD( "cp38jp2.bin", 0x8000, 0x004000, CRC(5e70192e) SHA1(840418f2de02876700fe4865745a66a45694114e) )
	ROM_LOAD( "cp38jp3.bin", 0x6000, 0x002000, CRC(54b85efc) SHA1(7750ef653afd44ce5fbd67f0622688b92fcf22e5) )
ROM_END

ROM_START( m4crkpotb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cp_3_1.p1", 0xc000, 0x004000, CRC(f9e0d95a) SHA1(ba69ae5b77e58deb288e13152f89b65cdc44a80d) )
	ROM_LOAD( "cp_3_1.p2", 0x8000, 0x004000, CRC(e013dadc) SHA1(f57c0732cc258cbac1b5605f0cdbcbe6c7464041) )
	ROM_LOAD( "cp_3_1.p3", 0x6000, 0x002000, CRC(e2935a4e) SHA1(8af34d1dd156cf9c529ad0cf8496c8fc4365711c) )
ROM_END


ROM_START( m4multcl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mp28p1.bin", 0xc000, 0x004000, CRC(f0173557) SHA1(c86419fe12cca2b7395d83f6de68c5928bce64cb) )
	ROM_LOAD( "mp28p2.bin", 0x8000, 0x004000, CRC(416f6372) SHA1(745d5663e857cadd6fbee68786b6a04aa384f9c9) )
	ROM_LOAD( "mp28p3.bin", 0x6000, 0x002000, CRC(eedd1bb8) SHA1(518ad1d7b38c38613b42f2e00bcf47f92b4d7828) )
ROM_END

ROM_START( m4clbclm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c1cs.p1", 0x8000, 0x008000, CRC(d646bb63) SHA1(98194ae09e07229c8a44b01813290d1b9c89641c) )
	ROM_LOAD( "c1c.p2", 0x4000, 0x004000, CRC(9726be3a) SHA1(cfe0e3aa448ddce23f9a0591634930204f8c3fe6) )
ROM_END

ROM_START( m4clbclma )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "clubclimber.hex", 0x4000, 0x00c000, CRC(d40180a9) SHA1(30edc2dd1d27bf63ad81456bda43fdaba6f4d1fe) ) // needs to be split
ROM_END

ROM_START( m4grbbnk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "g4b20p1.bin", 0xc000, 0x004000, CRC(35354cf5) SHA1(d14414bf257dbc44652e8b1f8a329709fb8912c5) )
	ROM_LOAD( "g4b20p2.bin", 0x8000, 0x004000, CRC(039cb9aa) SHA1(6e648f7af5f04701bb70a8f68593b80f777f57bb) )
ROM_END

ROM_START( m4grbbnka )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "g4b_2_1.p1", 0xc000, 0x004000, CRC(60bdeb67) SHA1(7f9b61391d785e5ecfc1f7555fb38c5f1c7f7c58) )
	ROM_LOAD( "g4b_2_1.p2", 0x8000, 0x004000, CRC(20952ee9) SHA1(46a9195814440471537b415c2e71ed5e9719e513) )
ROM_END

ROM_START( m4grbbnkb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "g4b_1_0.p1", 0xc000, 0x004000, CRC(56e5be9e) SHA1(46a4643b16648441fa7e03844d8f40794400dc9a) )
	ROM_LOAD( "g4b_1_0.p2", 0x8000, 0x004000, NO_DUMP )
ROM_END

ROM_START( m4hiroll )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hr-3.0-p1.bin", 0xc000, 0x004000, CRC(73d022c6) SHA1(de94f5903cab94a8f93aa293397d32840cc37ab4) )
	ROM_LOAD( "hr-3.0-p2.bin", 0x8000, 0x004000, CRC(617d0e2c) SHA1(5e5d59133e3b7a4eea205b79194754994cfe6d07) )
ROM_END

ROM_START( m4potlck )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pil22p1.bin", 0xc000, 0x004000, CRC(b7a926ad) SHA1(2a624256244cd0d3f29f96f8611356b2ec3b6cac) )
	ROM_LOAD( "pil22p2.bin", 0x8000, 0x004000, CRC(b967583e) SHA1(b60f61f09b036b7acb2d740f0d47bfa5f8fba7a5) )
	ROM_LOAD( "pil22p3.bin", 0x6000, 0x002000, CRC(498d8e8a) SHA1(e03d67e22e5709497d9139963113cc1d07fd50d9) )
ROM_END

ROM_START( m4potlcka )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pl2_7.p1", 0xc000, 0x004000, CRC(859fad87) SHA1(4bfc2d699a3abf66cbe52ad8be3f57f3eb66eb92) )
	ROM_LOAD( "pl2_7.p2", 0x8000, 0x004000, CRC(da7f8c1e) SHA1(05957f1767f7558ca10c4190ebeb0081bee6bb66) )
	ROM_LOAD( "pl2_7.p3", 0x6000, 0x002000, CRC(35f70dd5) SHA1(07baebb4904a41f00b50112b89fb70dd80e59493) )
ROM_END

ROM_START( m4sgrab )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1b.bin", 0xe000, 0x002000, CRC(980e4e38) SHA1(0e1db8ec5ba21c97351e91e09e6daaeea7391f07) )
	ROM_LOAD( "2b.bin", 0xc000, 0x002000, CRC(8b407449) SHA1(99ab60a3acde4f5f785d33d1266d5dd28b43665e) )
	ROM_LOAD( "3b.bin", 0xa000, 0x002000, CRC(72c2735a) SHA1(afedc6bd0e270d0f40da3ae01b57698767fdb212) )
	ROM_LOAD( "4b.bin", 0x8000, 0x002000, CRC(92d9c12f) SHA1(e62646af5d4d53bf418629a29037c5a3df6304f7) )
ROM_END

ROM_START( m4sgraba )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "smashandgrabv1prom1", 0xc000, 0x004000, CRC(5f75b90d) SHA1(b8baa2a3cb70d102950c3caf21ab90be5befc3ca) )
	ROM_LOAD( "smashandgrabv1prom2", 0x8000, 0x004000, CRC(a91ebd37) SHA1(9db197ad81d888eaf829b278be2ddc62639337fe) )
ROM_END


ROM_START( m4sgrabb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sag34.hex", 0x8000, 0x008000, CRC(e066ffdf) SHA1(3d906af27c13e1514225ae176e9e49e9b22552f4) )
ROM_END


ROM_START( m4stakeu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "su4_4.p1", 0xc000, 0x004000, CRC(e534edba) SHA1(0295013e3e9271f7e5023b8de670a4903578bf05) )
	ROM_LOAD( "su4_4.p2", 0x8000, 0x004000, CRC(8c2e0872) SHA1(9b0f1195d740e51085007417041428b449b3ee51) )
	ROM_LOAD( "su4_4.p3", 0x6000, 0x002000, CRC(0c25955b) SHA1(71f0ebbf088abc3ad860f9a1c7a830348a979289) )
ROM_END

ROM_START( m4stakeua )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "stakeup.hex", 0x6000, 0x00a000, CRC(a7ac8f19) SHA1(ec87512e16ff0252012067ad655c3fcee1d2e908) ) // needs to be split
ROM_END

//Derived from Action_Pack_(Barcrest)_[C02_800_4jp].gam
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
//DIP2_5=true
//DIP2_6=false
//DIP2_7=false
//Sound barcrest1
//Standard
//Volume 0 Stereo= 1
//Sample rate 16000
//Front door code 0 Cash door code 0

ROM_START( m4actpak )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ap04_p1.bin", 0x8000, 0x008000, CRC(16a16ec0) SHA1(ff3b9413864572b3a2fadfe13d73f60928d9ae73) )
	ROM_LOAD( "ap04_p2.bin", 0x4000, 0x004000, CRC(75b77ba0) SHA1(cac8350a9a3c2bd6770d0e2dfb133ff06f5b3db3) )
ROM_END

ROM_START( m4actpaka )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "action.hex", 0x0000, 0x010000, CRC(c5808b5d) SHA1(577950166c91e7f1ca390ebcf34be2da945c0a5f) )
ROM_END

//Derived from All_Cash_Advance_(Barcrest)_[C01_800_4jp].gam
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
//Sound empire
//Standard
//Volume 0 Stereo= 1
//Sample rate 16000
//Front door code 0 Cash door code 0

ROM_START( m4alladv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c2a60p1.bin", 0x8000, 0x008000, CRC(6630b4a4) SHA1(e4b35fde196d544b41e1dc9cee94442cc5c7223f) )
	ROM_LOAD( "c2a60p2.bin", 0x4000, 0x004000, CRC(0f0c9c2d) SHA1(7dc154048aeaf2842e9eb748a9d6d197cc429c69) )
ROM_END


ROM_START( m4clbdbl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cd16p1.bin", 0x8000, 0x008000, CRC(368b182e) SHA1(ca09510970e0aaab3897381415b142c1b4dbb56a) )
	ROM_LOAD( "cd16p2.bin", 0x4000, 0x004000, CRC(61d64f04) SHA1(2d7f0d5d98da3259cdd20aa5f201c2748056dec5) )
ROM_END

ROM_START( m4clbrpl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "clrpy1p1.bin", 0xc000, 0x004000, CRC(ee145d39) SHA1(02b9b882ec1b0662f5770181de3482592afd8621) )
	ROM_LOAD( "clrpy1p2.bin", 0x8000, 0x004000, CRC(0d5ca26e) SHA1(a582feb09d4bb6c5ee4c555a1741b0594b110c34) )
ROM_END

ROM_START( m4hittp2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "h4t20lp1.bin", 0xc000, 0x004000, CRC(c0b2c9df) SHA1(29b3bb78da7d1848321b81a123e8247a74170bc2) )
	ROM_LOAD( "h4t20lp2.bin", 0x8000, 0x004000, CRC(dc3921c7) SHA1(413d4a9c2f273129ca898e8da66e27254da5565e) )
ROM_END

ROM_START( m4hittp2a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "h4t2_0.1", 0xc000, 0x004000, CRC(8293cf7b) SHA1(6e7014bc0257b37406e59651ef0ac0f0563d5aa6) )
	ROM_LOAD( "h4t20lp2.bin", 0x8000, 0x004000, CRC(dc3921c7) SHA1(413d4a9c2f273129ca898e8da66e27254da5565e) )
ROM_END

ROM_START( m4stopcl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sc251.bin", 0xc000, 0x004000, CRC(a0205ecb) SHA1(e3f33c7d692dbe39669eb8121c42e1ca6024e611) )
	ROM_LOAD( "sc252.bin", 0x8000, 0x004000, CRC(84f4e19c) SHA1(c077ed83001b9dd29265941d173c939a7f815068) )
	ROM_LOAD( "sc253.bin", 0x6000, 0x002000, CRC(7e4d0526) SHA1(c5d4e6e2662157dd111337e619bee215b3514a91) )
ROM_END


ROM_START( m4toptena )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "toptenner2_1.bin", 0x8000, 0x008000, CRC(0ffa15d3) SHA1(4c2e5762ddfc34e18d82019b6edfaf9a612719fb) )
	ROM_LOAD( "toptenner2_2.bin", 0x4000, 0x004000, CRC(b3b9a66b) SHA1(ca744e6481c47619abc3e133fafac6457df5b746) )
ROM_END

ROM_START( m4toplot )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "topthelot.bin", 0x8000, 0x008000, CRC(52051209) SHA1(e34720521bc9e391ffb3bcdae18ff9d6449d83bd) )
ROM_END


ROM_START( m4topgr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "topgear.hex", 0x6000, 0x00a000, CRC(539cc3d7) SHA1(7d5c9eccd2d929189e8d82783fc630b2f3cacd24) ) // needs to be split
ROM_END

ROM_START( m4bj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dbj16p1.dat", 0x8000, 0x008000, CRC(025b2ad7) SHA1(1cd043ce3c550b0dcec8b3a7ed137c0032c4e4dc) )
	ROM_LOAD( "dbj16p2.dat", 0x4000, 0x004000, CRC(b961e9cb) SHA1(ffad698986b5ab9fe76f1addcb2bfa64f5b06fe4) )
ROM_END

ROM_START( m4flash )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fc10.bin", 0x0000, 0x010000, CRC(497862b8) SHA1(3409ff455e77ed85baf540a2ebeb979ab1b6e1e7) )
ROM_END

ROM_START( m4swpnot )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "san3.3.bin", 0x0000, 0x010000, CRC(d90479e6) SHA1(f634d4db62946017d70a1cd70b67fcb28587a66c) )
ROM_END

ROM_START( m4swpnota )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "swap-a-note-v32.bin", 0x0000, 0x010000, CRC(8f0b45f5) SHA1(766239927cfbdaedb98c9eca5401b82114e85207) )
ROM_END

ROM_START( m4actnot )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "an12.bin", 0x0000, 0x010000, CRC(54c6a33b) SHA1(91870c46b538abf56c356c96290cfedcf41db21f) )
ROM_END


ROM_START( m4cardcs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cshcd19l.bin", 0x0000, 0x010000, CRC(539040f6) SHA1(f3a1a876c4f17b8d9a0cf2cd54ebf52f4b31dbd1) )
ROM_END




ROM_START( m4cashcn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cc032h2.bin", 0x0000, 0x010000, CRC(f29c01e0) SHA1(6e302ce02e4568d582c6f55741334b3fb6ea0746) )
ROM_END


ROM_START( m4cashco )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c324.bin", 0x0000, 0x010000, CRC(d559bde5) SHA1(aed48c699118258c5dc915d5982d3de75c0213a1) )
ROM_END

ROM_START( m4cashcoa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c318d.bin", 0x0000, 0x010000, CRC(980df80c) SHA1(bf513d72a3e56c7f8c2a1c50d8ebf82496cd7ee3) )
ROM_END

ROM_START( m4cashcob )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cash counter v0-5", 0x0000, 0x010000, CRC(69012475) SHA1(e850de5b9c3ff83f758bee0e74f0f44108997305) )
ROM_END

ROM_START( m4cashcoc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cashcounter-1-2.bin", 0x0000, 0x010000, CRC(e4fa4e0f) SHA1(e78fbf0fc550aca3dc9139cdd5791875206d80bf) )
ROM_END

ROM_START( m4cashcod )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cash connect 70.bin", 0x0000, 0x010000, CRC(8b4f8056) SHA1(a9e7927ac9aa2ee1dccf207f4b249ae90c4c713d) )
ROM_END

ROM_START( m4cashmx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cash_matrix_20p.bin", 0x0000, 0x010000, CRC(3f2ebfeb) SHA1(1dbabe81204f4b149c125aca3413d8e521a690ca) )
ROM_END

ROM_START( m4cashmxa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cashmat1.hex", 0x0000, 0x010000, CRC(36f1a4bb) SHA1(7eefcbb1be539fcc302d226fa567e8691e85c360) )
ROM_END

ROM_START( m4cashzn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "caz_1_2.p1", 0x0000, 0x010000, CRC(78383d52) SHA1(cd69d4e600273e3da7aa61e51266e8f1765ad5dc) )
ROM_END

ROM_START( m4cashzna )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "caz_1_5.p1", 0x0000, 0x010000, CRC(c0d80d62) SHA1(e062be12159a67b5da883345565f3a52a1dd2ebe) )
ROM_END

ROM_START( m4czne )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "czone 6.bin", 0x0000, 0x010000, CRC(e5b2b64e) SHA1(b73a2aed7b04184bc7c5c3d0a11d44e624a47428) )
ROM_END


ROM_START( m4copcsh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "copcash2.bin", 0x0000, 0x010000, CRC(7ba83a9f) SHA1(ce294b7978b4edc64ee35795f017c2e066d1cb61) )
ROM_END

ROM_START( m4dblup )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "du15.bin", 0x0000, 0x010000, CRC(76d4f30d) SHA1(91b131bec6d38d09ffafbde985d03ba4d8fcb307) )
ROM_END

ROM_START( m4eighth )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wonder.hex", 0x0000, 0x010000, CRC(6fcaab11) SHA1(a462d4c50000e62af4c52980338cee073e4175a9) )
ROM_END

ROM_START( m4eightha )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ewc02__0.3", 0x0000, 0x010000, CRC(3ea4c626) SHA1(c10b3ba1f806f9d685b9de25fab3a15cbb8e94c3) )
ROM_END

ROM_START( m4eighthb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ewc05__0.3", 0x0000, 0x010000, CRC(691d2694) SHA1(08deeb25a23a65059be877d11c570db2db66564c) )
ROM_END

ROM_START( m4eighthc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ewc10__0.3", 0x0000, 0x010000, CRC(7a3b03d5) SHA1(13fb31ffb17edb0502ec47488d7f6b169834b0e4) )
ROM_END

ROM_START( m4eighthd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ewc10d_0.3", 0x0000, 0x010000, BAD_DUMP CRC(9131f8d4) SHA1(404ab0e359b81a26fcddaa9773ed3234dae0a754) )
ROM_END

ROM_START( m4eighthe )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ewc20__0.3", 0x0000, 0x010000, CRC(1b6babd3) SHA1(7b919f48a1a0a1a5ecc930a59fd27b2f9fe7509b) )
ROM_END

ROM_START( m4eighthf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ewc20c_0.3", 0x0000, 0x010000, CRC(9f6f8836) SHA1(e519f3c42ef7f157c88e68a706a96e42a8cfba4d) )
ROM_END

ROM_START( m4eighthg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ewc20d_0.3", 0x0000, 0x010000, CRC(dd26057e) SHA1(dfada02b18f748620966b351b2fe5e03af2b9c7e) )
ROM_END


ROM_START( m4frtprs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "f4p11lp1.bin", 0x8000, 0x008000, CRC(817bb364) SHA1(65a9ca6fd5689e547d8286c482d8cb9fe1a5fb61) )
	ROM_LOAD( "f4p11lp2.bin", 0x6000, 0x002000, CRC(ba10c3bc) SHA1(8f5f294b13a0d71cfe9c0e400ac72ce6aed6b763) )
ROM_END

ROM_START( m4frtprsa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fruitp.hex", 0x6000, 0x00a000, CRC(28ab9a79) SHA1(83be8b7ee4de93c426d93daa64718407709b0b1a) ) // needs to be split
ROM_END

ROM_START( m4gldstr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gs20mp1.bin", 0x8000, 0x008000, CRC(409e0375) SHA1(da231769cd439bc665634d73b0b18f31da332b47) )
	ROM_LOAD( "gs20mp2.bin", 0x6000, 0x002000, CRC(4979bc57) SHA1(53683600c8b3140deab8eac55786460b45c3b143) )
ROM_END

ROM_START( m4grands )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "g2d4_0.p1", 0x8000, 0x008000, CRC(550eaf03) SHA1(789e4bd7ffe71841dbf22413aa0fd16be5477482) )
	ROM_LOAD( "g2d4_0.p2", 0x6000, 0x002000, CRC(fecf1270) SHA1(7826abc21988f751a2b1190706fe4717a4f028ec) )
ROM_END

ROM_START( m4grandsa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gd11p1.bin", 0x8000, 0x008000, CRC(f9989c86) SHA1(a4e3fa71ab0a07259751cbb07f943fea04b83946) )
	ROM_LOAD( "gd11p2.bin", 0x6000, 0x002000, CRC(3fe26112) SHA1(bfc1c83fc6f341c6f4be937496f9bd0d69e49485) )
ROM_END

// 00 BC B8 FC BC DC FC FC FC
ROM_START( m4intcep )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ints.p1", 0x0000, 0x010000, CRC(6c6b61ca) SHA1(521cf68b9086baffe739d2b12dd28a13afb84b80) )
	ROM_REGION( 0x48, "characteriser:fakechr", 0 )
	ROM_LOAD( "int.chr", 0x0000, 0x000048, CRC(5915f545) SHA1(f8490a74aefe2a27d8e59b13dbd5fa78b8ab2166) )
ROM_END

ROM_START( m4intcepa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "intx.p1", 0x0000, 0x010000, CRC(6e435db6) SHA1(17114fe54c74a140e116bec5a027219f7f5a70d6) )
	ROM_REGION( 0x48, "characteriser:fakechr", 0 )
	ROM_LOAD( "int.chr", 0x0000, 0x000048, CRC(5915f545) SHA1(f8490a74aefe2a27d8e59b13dbd5fa78b8ab2166) )
ROM_END

ROM_START( m4intcepb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "int11.bin", 0x0000, 0x010000, CRC(0beb156f) SHA1(9ac318a524c549df907a995e896cd90434634e72) )
	ROM_REGION( 0x48, "characteriser:fakechr", 0 )
	ROM_LOAD( "int.chr", 0x0000, 0x000048, CRC(5915f545) SHA1(f8490a74aefe2a27d8e59b13dbd5fa78b8ab2166) )
ROM_END


ROM_START( m4megbks )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bucx.p1", 0x0000, 0x010000, CRC(8ca051da) SHA1(d70c462a9c90b637b1a32c5e333d0d1e9d779caa) )
ROM_END

ROM_START( m4megbksa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bucxc.p1", 0x0000, 0x010000, CRC(f2a8c98c) SHA1(97096b899d8873c8484bcb23cb2fc74bfe96fbb8) )
ROM_END

ROM_START( m4megbksb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bucxd.p1", 0x0000, 0x010000, CRC(7b0746f9) SHA1(9edd13b2ce75b149604ed032f4871c03e469712d) )
ROM_END

ROM_START( m4megbksc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mbucks.hex", 0x0000, 0x010000, CRC(9a3329b1) SHA1(77af020a278db0c50886bbf5c629e6509bdba0c8) )
ROM_END

ROM_START( m4mirage )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ragy.p1", 0x0000, 0x010000, CRC(4f56b698) SHA1(6b2726ee951c9c08ed8b8e7130bcd52210b8bf92) )
ROM_END


ROM_START( m4moneym )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "moneymaker-v16.bin", 0x0000, 0x010000, CRC(77f52665) SHA1(48a67b651385834658f1acb11ccde12b73393ced) )
ROM_END

ROM_START( m4nifty )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nifty.hex", 0x0000, 0x010000, CRC(84931755) SHA1(b4e568e1e4c237ea3f7ca6156b8a89cb40faf425) )
ROM_END

ROM_START( m4niftya )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nf2_1.p2", 0x6000, 0x002000, CRC(fde4010e) SHA1(440c888e80bd65f4d8c4081be66ad79db8e19618) )
	ROM_LOAD( "nf2_1c.p1", 0x8000, 0x008000, CRC(52e954a5) SHA1(c402eaaf3f482d996f9a1312f97f057627734416) )
ROM_END

ROM_START( m4niftyb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nf2_1.p2", 0x6000, 0x002000, CRC(fde4010e) SHA1(440c888e80bd65f4d8c4081be66ad79db8e19618) )
	ROM_LOAD( "nf2_1l.p1", 0x8000, 0x008000, CRC(bb297210) SHA1(cb570a015699d396dacb9fb09397ef157ceb8c97) )
ROM_END

ROM_START( m4nudqst )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nq20h.bin", 0x0000, 0x010000, CRC(438329d4) SHA1(6c67d01785e116aa4b22cadab8065e801f4e89c8) )
ROM_END

ROM_START( m4r2r )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "r2r_p1.bin", 0x8000, 0x008000, CRC(50be12e1) SHA1(65967b54ba2cc29bc5bebc8acf374245b7c5fbed) )
	ROM_LOAD( "r2r_p2.bin", 0x0000, 0x008000, CRC(ac6dd496) SHA1(6cf8c77e28a5dcdf6e05d2794eadb90703fb994b) )
ROM_END

ROM_START( m4reelpk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "reelpoker3_0.bin", 0x8000, 0x008000, CRC(6a2a0a48) SHA1(6291cb3512a3f03c0cea06b33bcc5846144cc12e) )
ROM_END


ROM_START( m4runawy )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "r4t1_1.1", 0x8000, 0x008000, CRC(5c3c7276) SHA1(3e1e1e159f79524cb4eb69e4f5e81317598fcfd1) )
	ROM_LOAD( "r4t1_1.2", 0x6000, 0x002000, CRC(b25001ce) SHA1(73f11579094d38fc369805d18ebf73dee3c206ac) )
ROM_END

ROM_START( m4runawyb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "runawaytrail-p1.bin", 0x8000, 0x008000, CRC(35ce4948) SHA1(dcffdf1f3eaeeb1facebf5d5ceb3f4c4d0e32236) )
	ROM_LOAD( "runawaytrail-p2.bin", 0x6000, 0x002000, CRC(78c803c1) SHA1(6473939f77d56ea29de96cce66bc21c54ec93666) )
ROM_END


ROM_START( m4silshd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sh1-0.p1", 0x8000, 0x008000, CRC(35a298bc) SHA1(d64ccb8a1ecfbb9c01b66789ff3d0b5fe48bc6f9) )
	ROM_LOAD( "sh1-0.p2", 0x0000, 0x008000, CRC(81921f9b) SHA1(dcb1c6bf14eb7c41d6cb37e0a9f49c79f187ab94) )
ROM_END

ROM_START( m4silshda )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "shadow1.bin", 0x8000, 0x008000, CRC(8072340d) SHA1(7346e98421a2355bba490ccbafbf97250bf189de) )
	ROM_LOAD( "shadow2.bin", 0x0000, 0x008000, BAD_DUMP CRC(4f65cb62) SHA1(d8616fc07f2d42fac7ca50edf7c5c8596092761b) ) // solid F7 fill
ROM_END

ROM_START( m4silshdb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sshad1.bin", 0x8000, 0x007f3b, BAD_DUMP CRC(88dbe1f5) SHA1(65b2ad2f6c40da9ad894d2cd6ac4a5bfd925f415) ) // very similar to shadow1.bin except is too short and has a section banked out with 0x20
	ROM_LOAD( "sshad2.bin", 0x0000, 0x008000, CRC(c8c0f458) SHA1(a86866aec9412e623681e97ad0cea0f95a3e7427) )
ROM_END


ROM_START( m4solsil )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sos22p1.bin", 0x8000, 0x008000, CRC(7075a3bd) SHA1(9a797e1612d3c532d414efe1f826c13663b67bcb) )
	ROM_LOAD( "sos22p2.bin", 0x6000, 0x002000, CRC(ab196907) SHA1(a4b1f0d29286f4143ef980d5b4d927367871a334) )
ROM_END

ROM_START( m4solsila )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sos_2_1.p1", 0x8000, 0x008000, CRC(303706fa) SHA1(cf9df67358f3517a8b18d3b40839e839369786a9) )
	ROM_LOAD( "sos_2_1.p2", 0x6000, 0x002000, CRC(31f53d41) SHA1(41966952d18e4ee006d0a5b0992b5d908dc84adc) )
ROM_END


ROM_START( m4starbr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dsb28j.bin", 0x0000, 0x010000, CRC(2f21b614) SHA1(9cc47b879ad8ff81147a5d66fe9a41118aa722f2) )
ROM_END

ROM_START( m4sunset )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sunsetboulivard2p.bin", 0x0000, 0x010000, CRC(c83a0b84) SHA1(a345d1e4914ad08d40980022f63e10879e2ef32f) )
ROM_END

ROM_START( m4sb5 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sb5pcz", 0x0000, 0x010000, CRC(331bfd68) SHA1(7436994de8617c31cb044540da7a2867daa5f829) )
ROM_END

ROM_START( m4sunsetc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sunboul-5p3.bin", 0x0000, 0x010000, CRC(5ccbf062) SHA1(cf587018511d1a06624d271f2fde4e40f16ec87c) )
ROM_END

ROM_START( m4sunsetd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sunsetb.hex", 0x0000, 0x010000, CRC(53e848a9) SHA1(62cd50003a8bd580b68128324ac98974470ce803) )
ROM_END

ROM_START( m4sunsete )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sbc02__1.1", 0x0000, 0x010000, CRC(0c4cad79) SHA1(0595d149958ccc4e5bfeb67fb021c314232a7ca0) )
ROM_END

ROM_START( m4sunsetf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sbc05__1.0", 0x0000, 0x010000, CRC(e70acd03) SHA1(96339f9e2011bd1f3ac1eab42ae55466526b48f8) )
ROM_END

ROM_START( m4sunsetg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sbc10__1.0", 0x0000, 0x010000, CRC(c93b9185) SHA1(209307c7a2dec14e3d65d45c3eb4f1faa1881671) )
ROM_END

ROM_START( m4sunseth )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sbc10d_1.0", 0x0000, 0x010000, CRC(ab1e65c5) SHA1(6a8e497765dc4f4a41e0128856447fcdf5319975) )
ROM_END

ROM_START( m4sunseti )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sbc20__1.0", 0x0000, 0x010000, CRC(4d1651c5) SHA1(f8e2fd4e8afc068f23ef32f81b7eb20e8f33d787) )
ROM_END

ROM_START( m4sunsetj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sbc20d_1.0", 0x0000, 0x010000, CRC(9c1dea06) SHA1(0433d892143e5d74c29b99917571af4675d0d9c4) )
ROM_END

ROM_START( m4sunsetk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sbo02__1.1", 0x0000, 0x010000, CRC(3a2541ef) SHA1(fd5e84bed8fdb3e3cedf8bc5c8d8070c781d5b44) )
ROM_END

ROM_START( m4sunsetl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sbo05__1.0", 0x0000, 0x010000, CRC(87e2ff53) SHA1(02b4f87aaf5786a3f0f0eb4bac4bcc756960885c) )
ROM_END

ROM_START( m4sunsetm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sbo10__1.0", 0x0000, 0x010000, CRC(5bb9c3ff) SHA1(ea2d30713e351de72f76bb642d5a778899c56634) )
ROM_END

ROM_START( m4sunsetn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sbo10d_1.0", 0x0000, 0x010000, CRC(50b5dd43) SHA1(1a8e2725b688c989695bdb71fa8ee3c7a5fa035b) )
ROM_END

ROM_START( m4sunseto )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sbo10dy1.0", 0x0000, 0x010000, CRC(8e7508c0) SHA1(e3000e7f8dc806392faab235b50f3584c8ce61eb) )
ROM_END

ROM_START( m4sunsetp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sbo20__1.0", 0x0000, 0x010000, CRC(6a652ff2) SHA1(b51d6e59948cf8b1f5ca926d0930b3f8c18cec18) )
ROM_END

ROM_START( m4sunsetq )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sbo20c_1.0", 0x0000, 0x010000, CRC(d6ba347a) SHA1(0ac93bad181b462036c5c28ea871716c1d393b49) )
ROM_END

ROM_START( m4sunsetr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sbo20d_1.0", 0x0000, 0x010000, CRC(ffe841a8) SHA1(50ab96c9c28d11f17316bd386d7a83cd5198a583) )
ROM_END

ROM_START( m4sunsets )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sbo20dy1.0", 0x0000, 0x010000, CRC(7d0023ef) SHA1(efb0202082acdbb37d5ed8af5222cfac3b4bbaca) )
ROM_END

ROM_START( m4sunsett )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sbo20y_1.0", 0x0000, 0x010000, CRC(9b25ba02) SHA1(0b6b7c53c136724d74837dd5bcdf544e4a2696b0) )
ROM_END

ROM_START( m4supslt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ss", 0x8000, 0x008000, CRC(6995e373) SHA1(2b121ff53da66d9febeba65e96cf2da4058c3b45) )
	ROM_LOAD( "ss2", 0x6000, 0x002000, CRC(3e53e684) SHA1(89de2ccd21c5c7e41586e737dcdaf56056bbd98c) )
ROM_END

ROM_START( m4suptub )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "s4t10p1.bin", 0x8000, 0x008000, CRC(3c49a2a1) SHA1(9715584f8aa5bf8c97ebeecd011476a603aea0d3) )
	ROM_LOAD( "s4t10p2.bin", 0x6000, 0x002000, CRC(6e22096a) SHA1(f9923f272497fb7438eac2b46ca5e60babb01a0b) )
ROM_END

ROM_START( m4suptuba )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "st1.bin", 0x8000, 0x008000, CRC(ef0ca8bc) SHA1(464d18df3d3519f4b3f1cc160cca766ad18802cc) )
	ROM_LOAD( "s4t10p2.bin", 0x6000, 0x002000, CRC(6e22096a) SHA1(f9923f272497fb7438eac2b46ca5e60babb01a0b) )
ROM_END

ROM_START( m4suptwo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "suts.p1", 0x0000, 0x010000, CRC(0476a1f9) SHA1(5dfd26cf7307c9fb98852e3b2daa65f81f13380d) )
ROM_END

ROM_START( m4tiktak )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tictak.bin", 0x0000, 0x010000, CRC(00c15733) SHA1(e8e7ef0de7266f910e96248ea923030e7b507fec) )
ROM_END

ROM_START( m4topact )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dta22p.bin", 0x0000, 0x010000, CRC(e90697d6) SHA1(e92d94f30ecb474d2e04b81ed78dbe99f8ae32f5) )
ROM_END

ROM_START( m4topacta )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dta22j.bin", 0x0000, 0x010000, CRC(42375128) SHA1(82cb16cd64726b7eca5a9cad29d8f630a5af6a6b) )
ROM_END

ROM_START( m4topst )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tsp0.5.bin", 0x0000, 0x010000, CRC(d77dc5c5) SHA1(cadec7070e6e7fcca769b6c94a60c711e445eba1) )
ROM_END

ROM_START( m4toptak )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "toptake.bin", 0x8000, 0x008000, CRC(52e74d55) SHA1(72c14474b9c799d682ccd8bc20af86ce7dd6be64) )
ROM_END

ROM_START( m4tribnk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "triplebank_1.bin", 0x0000, 0x010000, CRC(efeb5810) SHA1(e882f3ce028dcd4705365e0c17dab8d5839e4901) )
ROM_END

ROM_START( m4tupen )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ctn.p1", 0xc000, 0x004000, CRC(bd44cf68) SHA1(6efc88987f330acfc44c248e71ec8acf1bc37228) )
	ROM_LOAD( "ctn.p2", 0x8000, 0x004000, CRC(afb93378) SHA1(974a2fe8ecac264d23cd7f4d8cb3fe77152485a0) )
ROM_END

ROM_START( m421 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "twentyone.bin", 0x0000, 0x010000, CRC(243f3bc1) SHA1(141df3dcdd8d70ad26a76ec071e0cd927357ee6e) )
ROM_END

//Derived from Alphabet_(Barcrest)_[C03_1024_4jp].gam
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
//Sound empire
//Standard
//Volume 0 Stereo= 0
//Sample rate 16000
//Front door code 255 Cash door code 255

ROM_START( m4alpha )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "alphabet.hex", 0x6000, 0x00a000, CRC(2bf0d7fd) SHA1(143543f45bfae379233a8c21959618e5ad8034e4) )
ROM_END

ROM_START( m4bnknot )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bankanote.bin", 0x0000, 0x010000, CRC(73021b17) SHA1(71040af105ef832247b90ac9fac246068e75193b) )
ROM_END

ROM_START( m4bjacka )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bla20s.p1", 0x0000, 0x010000, CRC(675e6b72) SHA1(c8e5e5cb108fdf521e1f4edc7fb02b0075b0ab99) )
ROM_END

ROM_START( m4bjack )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b2j2-2.bin", 0x0000, 0x010000, CRC(b6a6df89) SHA1(193cb2eab57d783a28c7811081b076ec87274658) )
ROM_END


ROM_START( m4bjsm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bjsmh.p1", 0x0000, 0x010000, CRC(9bb07e46) SHA1(989e343eab79c5382ca1d30d9923e7284b04a4b9) )
ROM_END

ROM_START( m4bjsma )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "blackjacksupermulti.p1", 0x0000, 0x010000, CRC(6265cd77) SHA1(6dcc77f17079c36a7eb2b75f042ce4c204dba0ca) )
ROM_END

ROM_START( m4blstbk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bab.p1", 0x8000, 0x008000, CRC(c38d33f4) SHA1(1337d802d609dbf972e3e99ae8344e97054726e5) )
	ROM_LOAD( "bab.p2", 0x6000, 0x002000, CRC(1a8a6fd7) SHA1(9e31f2c33aa1d9982f1493a842fa637a65922d23) )
ROM_END


ROM_START( m4cshino )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cashino.bin", 0x0000, 0x010000, CRC(0b269457) SHA1(835d28bf361fe9f1410a85f30aa919894c8d9a8e) )
ROM_END


ROM_START( m4exlin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "extra lines 10p 4.80-2.40 16.3.90.bin", 0x8000, 0x008000, CRC(5e9b81e9) SHA1(84d446542533495b28692ba5f12cfd4fa5deabdd) )
ROM_END

ROM_START( m4exlina )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "extra lines 2p 2.40 21.3.90.bin", 0x8000, 0x008000, CRC(b9736cd1) SHA1(c275e5c4ea550a6814b2b2143e4b84198739524c) )
ROM_END

ROM_START( m4jjc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jjc.bin", 0x0000, 0x010000, CRC(781095ce) SHA1(f6beee15b45abf4a4f550b527e512bc6a1b603d8) )
ROM_END

ROM_START( m4jjca )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jjcash4pndv6.bin", 0x0000, 0x010000, CRC(3f28e657) SHA1(2382b96eb6f2b0580464c53a5122a48e25b4c002) )
ROM_END

ROM_START( m4spton )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "spot_on_v8 2_5_10_20p.bin", 0x0000, 0x010000, CRC(d0e27431) SHA1(cbcf637f2c8cf349e6386a54343d35da8aa24186) )
ROM_END

ROM_START( m4supjst )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "super jester 16.2.90 2p 1.bin", 0x8000, 0x008000, CRC(a55dae4f) SHA1(c426a202c312e0aefc5718c844f70b0af9b1c724) )
ROM_END

ROM_START( m4supjsta )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "super jester 19.2.90 5p 2.40.bin", 0x8000, 0x008000, CRC(754d821a) SHA1(9b00d0e028e46f8dfed60f3b49ef7c5cc40ac70c) )
ROM_END

ROM_START( m4supjstb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "super jester 2.1.90 10p 4.80 p1.bin", 0x8000, 0x008000, CRC(14f2dc51) SHA1(7dcf54a25bcaa2081f906d0ad44bcae183a32371) )
//  ROM_LOAD( "31.1.90cj10p4.80v6.bin", 0x8000, 0x008000, CRC(14f2dc51) SHA1(7dcf54a25bcaa2081f906d0ad44bcae183a32371) ) // marked crown jester?
ROM_END

ROM_START( m4supjstc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "super jester 20p.bin", 0x8000, 0x008000, CRC(be725101) SHA1(b2352e56bec5c8929075697f29526d25626cbc38) )
ROM_END

ROM_START( m4supjstd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "super jester 4.10.89 10p 4 cash.bin", 0x8000, 0x008000, CRC(ca1797b2) SHA1(afd6b365e7bef721084dc33442bb685fef6f7b76) )
ROM_END

ROM_START( m4supjste )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "super jester 4.10.89 10p 4 token.bin", 0x8000, 0x008000, CRC(3b4e4444) SHA1(a3cbbd2657be087c346a5ada7c301584125f8fbc) )
ROM_END


ROM_START( m4spnwin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "saw.bin", 0x0000, 0x010000, CRC(f8aac65f) SHA1(2cf8402bffe1638bddc0c2dd145d7be3cc7bd02b) )
ROM_END

ROM_START( m4spnwina )
	ROM_REGION( 0x10000, "maincpu", 0 )
	//Found in a pile of ROMs for Concept's version of the game
	ROM_LOAD( "spawv2.0", 0x8000, 0x008000, CRC(cc6eb567) SHA1(9293e0e72b1143f762cce80bf3b942e3958ddab7) )
ROM_END

ROM_START( m4pick )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pickafr2.nl7", 0x0c000, 0x02000, CRC(7a174563) SHA1(83203392171ba09bd7201cdca5c70c52ec2e65bc) )
	ROM_LOAD( "pickafr1.nl7", 0x0e000, 0x02000, CRC(6ae6e508) SHA1(a7da4151527d0c35f74e971e79ad1ce380315eac) )
ROM_END

ROM_START( m4frcrak )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "fc_p1.bin", 0xe000, 0x002000, CRC(5716c8cf) SHA1(ddf2c6a70d67932310346bc042239fbe10069f52) )
	ROM_LOAD( "fc_p2.bin", 0xc000, 0x002000, CRC(dc4669f4) SHA1(1112c50792e6976649e4be9314f103acec0c73b3) )
	ROM_LOAD( "fc_p3.bin", 0xa000, 0x002000, CRC(067e3da7) SHA1(6dd0992e57bc68e39a9220a3513705f510f8e9b8) )
ROM_END

ROM_START( m4supsl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "s2v1.0.lp2.64k.bin", 0x6000, 0x002000, CRC(8c622799) SHA1(ffe8a59d37c21c32fd16c812baff2c83b241a43d) )
	ROM_LOAD( "s2v1.0.lp1.256k.bin", 0x8000, 0x008000, CRC(4d963ad0) SHA1(8ec45a33243868afb66d1ea1863124c005bad221) )
ROM_END

ROM_START( m4conn4 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "connect4.p2",  0x8000, 0x4000,  CRC(6090633c) SHA1(0cd2725a235bf93cfe94f2ca648d5fccb87b8e5c) )
	ROM_LOAD( "connect4.p1",  0xC000, 0x4000,  CRC(b1af50c0) SHA1(7c9645ea378f0857b849ca24a239d9114f62da7f) )
ROM_END

ROM_START( m4ttak )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tictacp1.bin", 0x8000, 0x008000, CRC(7ae5d93c) SHA1(8066341a267be357b67b1c7e315989e9bda99856) )
	ROM_LOAD( "tictacp2.bin", 0x4000, 0x004000, CRC(33f3d2d1) SHA1(fea9c5766bfb3fb49d88a76458450c15bf38b2b1) )
ROM_END


ROM_START( m4flshlt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "flt02__1.0", 0x0000, 0x010000, CRC(3e060051) SHA1(1ef132ecf514d3ad0b5f2a4d04062fcc95713a46) )
ROM_END

ROM_START( m4flshlta )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "flt05__0.5", 0x0000, 0x010000, CRC(8b290639) SHA1(ec75be98fc1afd95d5ffff0ece288c4f51b9f43c) )
ROM_END

ROM_START( m4flshltb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "flt05__1.0", 0x0000, 0x010000, CRC(b6c0b96e) SHA1(d1d526671444e96115439270c48632d626e5439f) )
ROM_END

ROM_START( m4flshltc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "flt10__1.0", 0x0000, 0x010000, CRC(df16930c) SHA1(d230a524b1757ea52d2e00cf01a97a6ce762a7c2) )
ROM_END

ROM_START( m4flshltd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "flt10d_1.0", 0x0000, 0x010000, CRC(6908ddb1) SHA1(2aaae24340b2ddc9571d2e1bca64597cf9836ef9) )
ROM_END

ROM_START( m4flshlte )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "flt20__1.0", 0x0000, 0x010000, CRC(fdac16fa) SHA1(615cae707b3994d4935a92b3991b966d6a528fbc) )
ROM_END

ROM_START( m4flshltf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "flt20d_1.0", 0x0000, 0x010000, CRC(d1d88302) SHA1(b0234897170ae535e4571d61a73cf34183c73aab) )
ROM_END

ROM_START( m4flshltg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "flt20dy1.0", 0x0000, 0x010000, CRC(a9962461) SHA1(22fc22125846ca73b0ccb1ff5338600f52184493) )
ROM_END


ROM_START( m4blflsh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bfl20__0.3t", 0x0000, 0x010000, CRC(3cf3aa54) SHA1(98472e72cf48f938b756617d71f3860b34c05c3a) )
ROM_END

ROM_START( m4blflsha )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bflxc__0.3", 0x0000, 0x010000, CRC(04149b9e) SHA1(9d2d783acccf36e5ad85bb7b6ccdfae88201748b) )
ROM_END

ROM_START( m4blflshb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bflxcb_0.3", 0x0000, 0x010000, CRC(0f4538ec) SHA1(a1bf9513e5018a38548691338dfe53ff0a382625) )
ROM_END

ROM_START( m4blflshc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bflxcbd0.3", 0x0000, 0x010000, CRC(49d59b00) SHA1(18759ab3eae9956d2b20c608e66d9c01b8647817) )
ROM_END

ROM_START( m4blflshd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bflxcc_0.3", 0x0000, 0x010000, CRC(eb13ee16) SHA1(eabedab917bbc9ea2d71f868e9a7fa8c594e36ab) )
ROM_END

ROM_START( m4blflshe )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bflxcd_0.3", 0x0000, 0x010000, CRC(4eae4984) SHA1(4f4626fc4f71a9ccad7f6d99d2b28c079e20fa41) )
ROM_END

ROM_START( m4wayin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ws1.0p1128k.bin", 0xc000, 0x004000, CRC(ba5dea02) SHA1(12ae92c802b085550dc5fc2d2024c73a834bf0b8) )
	ROM_LOAD( "ws1.0p2128k.bin", 0x8000, 0x004000, CRC(3fd3bdb4) SHA1(79b748d613db3132d08ae0a4e805e5494ea56ea0) )
	ROM_LOAD( "ws1.0p364k.bin", 0x6000, 0x002000, CRC(c0ec66e5) SHA1(703a6265f305d600aeb062981c39e09f1d059443) )
ROM_END

ROM_START( m4wayina )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wi1.bin", 0xc000, 0x004000, CRC(d59d351a) SHA1(815ff595a74dc165e1996a46de29095174c80442) )
	ROM_LOAD( "wi2.bin", 0x8000, 0x004000, CRC(cba2bcc5) SHA1(6412fa614ac8eef4eda6213afa35aabaf14ba6ce) )
	ROM_LOAD( "wi3.bin", 0x6000, 0x002000, CRC(1ba06ef1) SHA1(495c4e21a5b34cba0859e4e8fc842036c97ec063) )
ROM_END

ROM_START( m4funh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "funhouse.bin", 0x00000, 0x10000, CRC(4e342025) SHA1(288125ff5e3da7249d89dfcc3cd0915f791f7d43) )
ROM_END


ROM_START( m4actbnka )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ac30_prom2.bin", 0x4000, 0x04000, CRC(595615cd) SHA1(42d2028d11e2160c67f0dfde5a1ad805c0dc0fe1) )
	ROM_LOAD( "ac30_prom1.bin", 0x8000, 0x08000, CRC(6765aef1) SHA1(4158bd2ff65434f040ef6505adc5026710031756) )
ROM_END

ROM_START( m4actbnkb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "abank.hex", 0x6000, 0x00a000, CRC(2cd1a269) SHA1(5ce22b2736844a2de6cda04abdd0fe435391e033) ) // split me
ROM_END

//Derived from Action_Club_(Barcrest)_[C03_800_150jp]_[c].gam
//PCKEY =0
//STKEY =0
//JPKEY =0
//JPSET =0
//DIP1_0=true
//DIP1_1=true
//DIP1_2=true
//DIP1_3=true
//DIP1_4=true
//DIP1_5=true
//DIP1_6=true
//DIP1_7=true
//DIP2_0=false
//DIP2_1=false
//DIP2_2=false
//DIP2_3=false
//DIP2_4=false
//DIP2_5=true
//DIP2_6=false
//DIP2_7=false
//Sound barcrest1
//Standard
//Volume 2 Stereo= 1
//Sample rate 16000
//Front door code 0 Cash door code 0

ROM_START( m4actclb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "abcs.p1", 0x0000, 0x010000, CRC(cf730606) SHA1(35b95b924a24b306428c6c81136c14d7732e2356) )
ROM_END


ROM_START( m4actclba )
	ROM_REGION( 0x10000, "maincpu", 0 ) // this seems to also contain a bunch of extra (unused?) data for another game
	ROM_LOAD( "a2c1-1mkii.bin", 0x0000, 0x010000, CRC(4c8ee662) SHA1(17e710c2bda21db609b619dfc0c9280a211da151) )
ROM_END


ROM_START( m4bluemn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "blu23.bin", 0x0000, 0x010000, CRC(499d41c7) SHA1(1a741cf2c6ed6910717324ca2b0a2630338479e0) )
ROM_END

ROM_START( m4bluemna )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bmoon.hex", 0x0000, 0x010000, CRC(109aa258) SHA1(88820b1090ce6b6538b4ca0428c02979535895c3) )
ROM_END

ROM_START( m4bluemnb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "blm20p-6", 0x0000, 0x010000, CRC(4b9f83cf) SHA1(7014da9f7fc20443251dd5b2817f06a4ef862afd) ) // contains inaccessible data before 0x1000
ROM_END

ROM_START( m4take2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ttos.p1", 0x0000, 0x010000, CRC(d7943729) SHA1(76fcaf7dbfa7863a4dfe2804e2d472dcfec13124) )
ROM_END

ROM_START( m4take2a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "take220p", 0x0000, 0x010000, CRC(b536311a) SHA1(234945d2419c8391307db5b5d01d228894441faf) ) // contains inaccessible data before 0x1000
ROM_END

ROM_START( m4t266 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "t2 66.bin", 0x0000, 0x010000, CRC(5c99c6bb) SHA1(7b74e0e5207c00b31cb1859e0cc458c0412a1a07) )
ROM_END

ROM_START( m4pont )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pon3.0.bin", 0x0000, 0x010000, CRC(12d83177) SHA1(5b88b9618f53af2b2a4f75e73c3eb334a17791c0) )
ROM_END

ROM_START( m4ponta )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pons.p1", 0x0000, 0x010000, CRC(5186daa7) SHA1(78f853e221307270e1725895201d08f358e34986) )
ROM_END



ROM_START( m4loadmn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "la10h.bin", 0x0000, 0x010000, CRC(1f90520e) SHA1(c052c2c751d1ded6077a800be4dedf91ca0bd5ba) )
ROM_END

ROM_START( m4loadmna )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "la11c.bin", 0x0000, 0x010000, CRC(8e6ab8f9) SHA1(cd3367d368c64a74e108fdfda00c4898ca8262c8) )
ROM_END

ROM_START( m4loadmnb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "la11l.bin", 0x0000, 0x010000, CRC(2d1f8e7a) SHA1(9e5a8b7827925f757784ea4726e3c4897056cdf6) )
ROM_END


ROM_START( m4celclb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cels.p1",  0x00000, 0x10000,  CRC(19d2162f) SHA1(24fe435809352725e7614c32e2184142f355298e))
ROM_END

ROM_START( m4swpnotb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "centrepoint v1.6.bin", 0x0000, 0x010000, CRC(adcf0330) SHA1(b68b8c808b2dc1965d37d912adec714547466522) )
ROM_END

ROM_START( m4centpt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "centrepoint v1.3.bin", 0x0000, 0x010000, CRC(24d117a5) SHA1(bd48a1687d11e32ea8cda19318e8936d1ffd9fd7) )
ROM_END

ROM_START( m4clbcls )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cl11s.bin", 0x0000, 0x010000, CRC(d0064e6e) SHA1(17235d69ef56989a3f05458423a6b101bd635095) )
ROM_END


ROM_START( m4c999c )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c999 2p unprotected.bin", 0x0000, 0x010000, CRC(7637e074) SHA1(3b9e724cc1e657ab2a6cf6fe237f0ca43990aa53) )
ROM_END

ROM_START( m4c999d )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c99910p6", 0x0000, 0x010000, CRC(e3f6710a) SHA1(d527541ec6e799c8bc12e1e31519415eaf11fbe5) )
ROM_END

ROM_START( m4c999a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c99920p2", 0x0000, 0x010000, CRC(94f8f03e) SHA1(a99c3c60f2e9c15d5dd6265cfa73fad1058ce7fa) )
ROM_END

ROM_START( m4c999b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c99920p6", 0x0000, 0x010000, CRC(f88f3bfc) SHA1(8dd1bd13645b8c3e38d45a8a6941e56d6268c21d) )
ROM_END

ROM_START( m4c999 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "clnv.p1", 0x0000, 0x010000, CRC(486097d8) SHA1(33e9eab0fb1c750160a8cb2b75eca73145d6956e) )
ROM_END


ROM_START( m4ambass )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ambassador.bin", 0x0000, 0x010000, CRC(310313ac) SHA1(8e11515615754090d716b428adc4e2718ee1211d) )
ROM_END

ROM_START( m4atlan )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dat14.bin", 0x0000, 0x010000, CRC(d91fb9b2) SHA1(a06a868a17f84e2a012b0fe28025458e4f899c1d) ) // == m4tridic
ROM_END



ROM_START( m4bjc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dbc11.bin", 0x0000, 0x010000, CRC(ce28b677) SHA1(81006768e937b42f051e580f093b7182ad59236a) )
ROM_END


ROM_START( m4exprs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dxp20.bin", 0x0000, 0x010000, CRC(09e68942) SHA1(2253ab76286b7c7af34ff99cc6d8e60b26edcacb) )
ROM_END

ROM_START( m4brdway )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dbr11.bin", 0x0000, 0x010000, CRC(5cbb8a0f) SHA1(bee8b2b7d70c24f98b7626caa278cb84136941a4) )
ROM_END

ROM_START( m4bigbn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dbb12.bin", 0x0000, 0x010000, CRC(7acec20d) SHA1(5f3a21227329608c0afdb5facac977dee94ab9f5) )
ROM_END

ROM_START( m4cheryo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dch14.bin", 0x0000, 0x010000, CRC(47333745) SHA1(479bec721ccaa2c4b11f3022d3d1eb12de92ac81) )
ROM_END

ROM_START( m4giant )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dgi21.bin", 0x0000, 0x010000, CRC(07d8685a) SHA1(1b51db748543f2e4b6b7d7ad16b77864bbfe5a66) )
ROM_END

ROM_START( m4holdon )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dho25.bin", 0x0000, 0x010000, CRC(9c22690d) SHA1(a2474dd1901628551804ba2bf652a8a5a1de5739) )
ROM_END

ROM_START( m4libty )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dlt10.bin", 0x0000, 0x010000, CRC(25d91c01) SHA1(788ba8669bae5b4cdfb7231c7225d6745038a575) )
ROM_END

ROM_START( m4meglnk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dml20.bin", 0x0000, 0x010000, CRC(bbf48b45) SHA1(0ca9adf6a4171efad1af7b411e713dc35c654d30) )
ROM_END

ROM_START( m4multwy )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dmu17.bin", 0x0000, 0x010000, CRC(336b128e) SHA1(1d8268bfa0ffee62c76ffbf0ee89731626cf90ca) )
ROM_END

ROM_START( m4num1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dno17.bin", 0x0000, 0x010000, CRC(3b302160) SHA1(ff52803472e119aa46fe1cff134b5503858dfee1) )
ROM_END

ROM_START( m4nudup )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dnu25.bin", 0x0000, 0x010000, CRC(c8d83f94) SHA1(fa0834d41c7506cab14e50b4036943a61411ed0e) )
ROM_END

ROM_START( m4omega )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dom23.bin", 0x0000, 0x010000, CRC(d51e078c) SHA1(2c38d271b9ce4731ce26106764529839b5110b3e) )
ROM_END


ROM_START( m4randr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "drr22.bin", 0x0000, 0x010000, CRC(9fdcee66) SHA1(196c8d3ea4141c209ecc5b0acdab7a872f791dc0) )
ROM_END

ROM_START( m4samu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dsm10.bin", 0x0000, 0x010000, CRC(c2a10732) SHA1(9a9dcd0445662d301320a7fb0f4e5da8a719a86b) )
ROM_END

ROM_START( m4stards )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dsd13.bin", 0x0000, 0x010000, CRC(a17fbe93) SHA1(f2a9e0c059f309f63e6da3e47740644ad4839fa8) )
ROM_END

ROM_START( m4tbreel )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dtr12.bin", 0x0000, 0x010000, CRC(cdb63ef5) SHA1(748cc06e6a274b125d189dd66f2adad8bd2fb166) ) // aka dtr31.dat
ROM_END

ROM_START( m4tbrldx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dtv30.dat", 0x0000, 0x010000, CRC(c314846c) SHA1(bfa6539b204477a04a5bbc8d13c3a666c52b597b) )
ROM_END

ROM_START( m4taj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tajmahal.bin", 0x0000, 0x010000, CRC(c2db162a) SHA1(358e7bb858f0a34d39f43494cea13bf00a67e48e) )
ROM_END

ROM_START( m4tricol )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dtc25.bin", 0x0000, 0x010000, CRC(d3318dde) SHA1(19194d206deee920a1b0122dddc3d5bc0a7a48c5) )
ROM_END

ROM_START( m4twilgt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dtl22.bin", 0x0000, 0x010000, CRC(ed4c8b6a) SHA1(7644046a273304104eaa6260f6cc75950592d4b6) )
ROM_END

ROM_START( m4wildms )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wild.bin", 0x0000, 0x010000, CRC(33519799) SHA1(d5154fa5307f25f6a3ee8759520907eb4c06fdf9) )
ROM_END



ROM_START( m4suptrn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dsu21", 0x0000, 0x010000, CRC(b09f63e3) SHA1(8dba0731e1ed5e7056ec6ad1fa269b5b77629745) )
ROM_END


ROM_START( m4bjac )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bjak1.8", 0x0000, 0x010000, CRC(e6a3c263) SHA1(fb28657cb43a0f24354382518d5d5be9cfdfa1d1) )
ROM_END

ROM_START( m4bjaca )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c2js.p1", 0x0000, 0x010000, CRC(3e6dd1f3) SHA1(dbd87368124244931b8868eb740b02a1775ed734) )
ROM_END


ROM_START( m421club )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dtw27.bin", 0x0000, 0x010000, CRC(8e37977e) SHA1(8e996e50b2a87b97f999bfd00166c32240b74690) )
ROM_END

ROM_START( m4clbcnt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "con12.bin", 0x0000, 0x010000, CRC(e577d7bc) SHA1(be4d6d75f33782c503f91659b5f69d1fb4c220da) )
ROM_END

ROM_START( m4clbcnta )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "con1_0.bin", 0x0000, 0x010000, CRC(2a758619) SHA1(77f993090b7d01901635c56fd9256f57d2371c6d) )
ROM_END

ROM_START( m4clbcntb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "con1-1.bin", 0x0000, 0x010000, CRC(9fc1fefc) SHA1(5638c978687526858cbcb105bdb499dce2d234d3) )
ROM_END

ROM_START( m4clbcntc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "conf.p1", 0x0000, 0x010000, CRC(7900cee0) SHA1(afcd77e5bc05a21b5eb7b26c66d94ce21f2ce501) )
ROM_END

ROM_START( m4clbcntd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cons.p1", 0x0000, 0x010000, CRC(afe71a40) SHA1(b0065e131eb8fcf7a2fd420f60e2174d927db450) )
ROM_END

ROM_START( m4class )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dfc20.bin", 0x0000, 0x010000, CRC(1fa0c771) SHA1(b374d2cdc1bfc35a2e6fe35b9d21f2784b8c52e8) )
ROM_END

ROM_START( m4classa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dfc20dsl.bin", 0x0000, 0x010000, CRC(dc1c8b87) SHA1(52235f9d393c574fdd26aa2ec60e6db70538fb9d) )
ROM_END

ROM_START( m4frtfl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "frtfull", 0x0000, 0x010000, CRC(4f5389e2) SHA1(bb6d43d428c1e8db07fe58d1b83c05ce5fcdcc7d) )
ROM_END

ROM_START( m4frtfla )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fruitfull.hex", 0x0000, 0x010000, CRC(c264d497) SHA1(93843efbf1b4207a4722f49dd5dddf2c52bb1b8f) )
ROM_END

ROM_START( m4frtflc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ffcs.p1", 0x0000, 0x010000, CRC(db917142) SHA1(0f32f0c1ed6733b4557fd19f24f2b1dda26ccc44) )
ROM_END


ROM_START( m4frtlnk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "flcs.p1", 0x0000, 0x010000, CRC(f66a6810) SHA1(e91cdba6b52df6e633d6ce5036a82f5c2dbb1d19) )
ROM_END

ROM_START( m4frtlnka )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "flinkv6", 0x0000, 0x010000, CRC(ca9c1034) SHA1(f7e02372a1c7cd41097db63d4a921387d22e02b4) )
ROM_END

ROM_START( m4thehit )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dth17.bin", 0x0000, 0x010000, CRC(93947de4) SHA1(e04c34edf39d264e3fa91bf6dfd757088e1c08e4) )
ROM_END

ROM_START( m4gldgat )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dgg22.bin", 0x0000, 0x010000, CRC(ef8498df) SHA1(6bf164ef18445e83e4510a000bc924cbe916ad99) )
ROM_END

ROM_START( m4toma )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dtk23.bin", 0x0000, 0x010000, CRC(ffba2b96) SHA1(c7635023ac5181e661e808c6b44ac1add58f4f56) )
ROM_END

ROM_START( m4topdk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dtd26pj.bin", 0x0000, 0x010000, CRC(1f84d995) SHA1(7412632cf79008b980e48f14aea89c3f8d742ed2) )
ROM_END

ROM_START( m4jpjmp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "vjcs.p1", 0x0000, 0x010000, CRC(90280752) SHA1(bc2fcefc00adbae9ca2e116108b53ab932ab57b2) )
ROM_END

ROM_START( m4jpjmpa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jackjump-vjc1-3.bin", 0x0000, 0x010000, CRC(fa713c10) SHA1(766912ea891166d7d7f0360e81bdad58b6064eb1) )
ROM_END

ROM_START( m4milclb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mi2d.p1", 0x0000, 0x010000, CRC(ce697bbd) SHA1(86c1729014eff9925a5f62189236a9c5bd11534b) )
ROM_END

ROM_START( m4milclba )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mi2f.p1", 0x0000, 0x010000, CRC(224922a4) SHA1(59bc1fbfe20c533eb6462f01196a5f2d35ceb92d) )
ROM_END

ROM_START( m4milclbb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mi2s.p1", 0x0000, 0x010000, CRC(f69b69c7) SHA1(4f881f5307db2c100535fa75a8eb42d0f7382c93) )
ROM_END

ROM_START( m4milclbc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mild.p1", 0x0000, 0x010000, CRC(16da3df9) SHA1(0f1838f99c14763132c2a3b79363496c6baa5e88) )
ROM_END

ROM_START( m4milclbd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mils.p1", 0x0000, 0x010000, CRC(0742defd) SHA1(ac25d8adb40bc5b4124241cc5d970d4c10c6f5fd) )
ROM_END


ROM_START( m4bigchd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bch15.bin", 0x0000, 0x010000, CRC(b745b19f) SHA1(b654af35200c69604a3c30e3df1252f8bedc2000) )
ROM_END

ROM_START( m4dbl9 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "du91.0", 0x0000, 0x010000, CRC(6207753d) SHA1(b19bcb60707b73f37e9bd8177d0b15847af0213f) )
ROM_END

ROM_START( m4dbl9a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "d9s_6.bin", 0x0000, 0x010000, CRC(6029d46a) SHA1(0823f29f17562675a6f250429e46655c0b2e8f2c) )
ROM_END

ROM_START( m4nick )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nilx.p1", 0x0000, 0x010000, CRC(210be67b) SHA1(b4f7b955ffe6a991f06334cb7eb2aebcf5fe11b3) )
ROM_END

ROM_START( m4nicka )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nilc.p1", 0x0000, 0x010000, CRC(8e612b50) SHA1(a33142ca3988e449ae94978946ed0f171c52c5fa) )
ROM_END

ROM_START( m4nickb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nilxb.p1", 0x0000, 0x010000, CRC(41fde39d) SHA1(fbb179d942a1ffb9c84925402179c7c7fd0a7692) )
ROM_END

ROM_START( m4nickc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nilxc.p1", 0x0000, 0x010000, CRC(38cabacf) SHA1(67aafaecc93a348dcdf7beaf6c93c16101fccb55) )
ROM_END

ROM_START( m4nickd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nilxk.p1", 0x0000, 0x010000, CRC(ceb04af2) SHA1(1cd65356fba532b4c34e03b418708b982e8e0828) )
ROM_END

ROM_START( m4nicke )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nick_21.bin", 0x0000, 0x010000, CRC(77a0ba57) SHA1(d56bdd52f81d707b5138f00e68b10130cac1225f) )
ROM_END


ROM_START( m4joljok )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jlyjk16l.bin", 0x0000, 0x010000, CRC(a0af938f) SHA1(484e075c3b9199d0d9e20185c6fa0be560845029) )
ROM_END


ROM_START( m4unkjok )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "joker 10p 3.bin", 0x0000, 0x010000, CRC(009823ac) SHA1(5ab25da5876c87a8d8701f84446bb3d377e4c1ca) )
ROM_END

ROM_START( m4unkjoka )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "joker 10p 6.bin", 0x0000, 0x010000, CRC(f25f0704) SHA1(35298b49f79c5029277f4777fe88d5e4344c115f) )
ROM_END



ROM_START( m4joltav )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tavs.p1", 0x0000, 0x010000, CRC(12bdf083) SHA1(c1b73bfd05ae6128d1760083383805fdaf328003) )
ROM_END

ROM_START( m4joltava )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jto20__1.1", 0x0000, 0x010000, CRC(4790c4ec) SHA1(2caab4ccc91158f6b76817e76c1d092ef1a79cd9) )
ROM_END

ROM_START( m4joltavb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jto20d_1.1", 0x0000, 0x010000, CRC(dff09dfc) SHA1(c13f31f7d96075f7c94ae5e79fc1f9b8ce7e4c80) )
ROM_END

ROM_START( m4unkjokb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "joker 20p 3 or 6.bin", 0x0000, 0x010000, CRC(cae4397e) SHA1(53b61fd41c97a6ed29ce6a7b555e061ecf2b0ae2) )
ROM_END

ROM_START( m4unkjokc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "joker new 20p 6 or 3.bin", 0x0000, 0x010000, CRC(b8d77b97) SHA1(54f69823bb3fd9c2cca014dc7c51913b2d6c8058) )
ROM_END

ROM_START( m4btclok )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "beattheclock.hex", 0x6000, 0x00a000, CRC(a0d4e463) SHA1(45d1df08bfd70caf63b14d2ccc56038ed85e23d0) )
ROM_END

ROM_START( m4brktak )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b-t v1-0 p1", 0xc000, 0x004000, CRC(a3457409) SHA1(ceaca37f20a055b18a24ee99e43991df95e9b520) )
	ROM_LOAD( "b-t v1-0 p2", 0x8000, 0x004000, CRC(7465cc6f) SHA1(f984e41c310bc58d7a668ec9f31c238fbf5de9c6) )
ROM_END

ROM_START( m4sunclb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sucxe__0.2", 0x0000, 0x010000, CRC(fd702a6f) SHA1(0f6d553fcb096ca4874bb971425dabfbe18db31d) )
ROM_END

ROM_START( m4sunclba )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sucxed_0.2", 0x0000, 0x010000, CRC(70802bc3) SHA1(69b36f716cb608931f933cb58e47232b18064f9d) )
ROM_END

ROM_START( m4graffd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "grafittirom.bin", 0x0000, 0x010000, CRC(36135d6e) SHA1(e71eedabae36971739f8a6fd56a4a954de29944b) )
ROM_END

using namespace mpu4_traits;

#define GAME_FLAGS (MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL)

GAME(199?, m4rsg,     0,          mod2_cheatchr_pal<mpu4_characteriser_pal::viva_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Ready Steady Go (Barcrest) (MPU4, Mod 2 type) (RSG 1.2)",GAME_FLAGS )
GAME(199?, m4rsga,    m4rsg,      mod2_cheatchr_pal<mpu4_characteriser_pal::viva_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Ready Steady Go (Barcrest) (MPU4, Mod 2 type) (R4G 1.0)",GAME_FLAGS )

GAME(199?, m4stopcl,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::viva_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Stop the Clock (Barcrest) (MPU4) (SC 2.5)",GAME_FLAGS )

GAME(199?, m4crkpot,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::crkpot_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Crackpot 100 Club (Barcrest) (MPU4) (C1P 1.2)",GAME_FLAGS )
GAME(199?, m4crkpota, m4crkpot,   mod2_cheatchr_pal<mpu4_characteriser_pal::crkpot_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Crackpot 100 Club (Barcrest) (MPU4) (CP 3.8)",GAME_FLAGS )
GAME(199?, m4crkpotb, m4crkpot,   mod2_cheatchr_pal<mpu4_characteriser_pal::crkpot_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Crackpot 100 Club (Barcrest) (MPU4) (CP 3.1)",GAME_FLAGS )

GAME(199?, m4hiroll,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::crkpot_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","High Roller (Barcrest) (Dutch) (MPU4) (HR 3.0)",GAME_FLAGS )

GAME(199?, m4multcl,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::m400_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Multiplay Club (Barcrest) (MPU4, MP 2.8)",GAME_FLAGS )

GAME(199?, m4reelpk,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::squids_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Reel Poker (Barcrest) (MPU4) (R2P 3.0)",GAME_FLAGS )

GAME(199?, m4grbbnk,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::intcep_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Grab The Bank (Barcrest) (MPU4) (G4B 2.0)",GAME_FLAGS )
GAME(199?, m4grbbnka, m4grbbnk,   mod2_cheatchr_pal<mpu4_characteriser_pal::intcep_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Grab The Bank (Barcrest) (MPU4) (G4B 2.1)",GAME_FLAGS )
// doesn't run valid code, bad (missing a ROM)
GAME(199?, m4grbbnkb, m4grbbnk,   mod2_cheatchr(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Grab The Bank (Barcrest) (MPU4) (G4B 1.0)",GAME_FLAGS )

GAME(199?, m4sgrab,   0,          mod2_cheatchr_pal<mpu4_characteriser_pal::intcep_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Smash 'n' Grab (Barcrest) (MPU4) (SAG 1.0, set 1)",GAME_FLAGS )
GAME(199?, m4sgraba,  m4sgrab,    mod2_cheatchr_pal<mpu4_characteriser_pal::intcep_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Smash 'n' Grab (Barcrest) (MPU4) (SAG 1.0, set 2)",GAME_FLAGS )
GAME(199?, m4sgrabb,  m4sgrab,    mod2_cheatchr_pal<mpu4_characteriser_pal::intcep_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Smash 'n' Grab (Barcrest) (MPU4) (SAG 3.4)",GAME_FLAGS )

GAME(199?, m4potlck,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::graff_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Pot Luck 100 Club (Barcrest) (MPU4) (P1L 2.2)",GAME_FLAGS )
GAME(199?, m4potlcka, m4potlck,   mod2_cheatchr_pal<mpu4_characteriser_pal::graff_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Pot Luck 100 Club (Barcrest) (MPU4) (PL 2.7)",GAME_FLAGS )

GAME(199?, m4wayin,   0,          mod2_cheatchr_pal<mpu4_characteriser_pal::wayin_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Super Way In (Barcrest) (MPU4) (WS 1.0)",GAME_FLAGS )
// non-standard protection, hack?
GAME(199?, m4wayina,  m4wayin,    mod2_bootleg_fixedret<0x40>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "bootleg","Way In Mk 2 (Barcrest) (bootleg) (MPU4)",GAME_FLAGS )

GAME(199?, m4stakeu,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::m400_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Stake Up Club (Barcrest) (MPU4) (SU 4.4)",GAME_FLAGS )
GAME(199?, m4stakeua, m4stakeu,   mod2_cheatchr_pal<mpu4_characteriser_pal::m400_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Stake Up Club (Barcrest) (MPU4) (SU 4.8)",GAME_FLAGS )

GAME(199?, m4clbclm,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::rhm_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Club Climber (Barcrest) (MPU4, C1C 3.3)",GAME_FLAGS )
GAME(199?, m4clbclma, m4clbclm,   mod2_cheatchr_pal<mpu4_characteriser_pal::rhm_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Club Climber (Barcrest) (MPU4, CC 4.5)",GAME_FLAGS )

GAMEL(199?, m4actpak, 0,          mod2_cheatchr_pal<mpu4_characteriser_pal::clbveg_characteriser_prot>(R4, RT1, HT), mpu4,            mpu4mod2_machines_state, init_m4,      ROT0,   "Barcrest","Action Pack (Barcrest) (MPU4) (AP 0.4)",GAME_FLAGS, layout_m4actpak )
GAMEL(199?, m4actpaka,m4actpak,   mod2_cheatchr_pal<mpu4_characteriser_pal::clbveg_characteriser_prot>(R4, RT1, HT), mpu4,            mpu4mod2_machines_state, init_m4,      ROT0,   "Barcrest","Action Pack (Barcrest) (MPU4) (AP 0.5)",GAME_FLAGS, layout_m4actpak )

GAMEL(199?, m4alladv, 0,          mod2_cheatchr_pal<mpu4_characteriser_pal::buc_characteriser_prot>(R4, RT1, HT), mpu4,            mpu4mod2_machines_state, init_m4,      ROT0,   "Barcrest","All Cash Advance (Barcrest) (MPU4) (C2B 6.0)",GAME_FLAGS, layout_m4alladv )

GAME(199?, m4clbdbl,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R5R, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Club Double (Barcrest) (MPU4) (CD 1.6)",GAME_FLAGS )

GAME(199?, m4hittp2,  m4hittop,   mod2_cheatchr_pal<mpu4_characteriser_pal::hittop_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Hit The Top (Barcrest) (MPU4, Mod 2 type, H4T 2.0, set 1)",GAME_FLAGS )
GAME(199?, m4hittp2a, m4hittop,   mod2_cheatchr_pal<mpu4_characteriser_pal::hittop_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Hit The Top (Barcrest) (MPU4, Mod 2 type, H4T 2.0, set 2)",GAME_FLAGS )

GAME(199?, m4toptena, m4topten,   mod2_cheatchr_pal<mpu4_characteriser_pal::take2_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Top Tenner (Barcrest) (MPU4, Mod 2 type, TP 2.7)",GAME_FLAGS )

GAME(199?, m4toplot,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::toplot_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Top The Lot (Barcrest) (MPU4, T4L 1.0)",GAME_FLAGS )

GAME(199?, m4topgr,   0,          mod2_cheatchr_pal<mpu4_characteriser_pal::topgear_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Top Gear (Barcrest) (MPU4) (TG4 1.1)",GAME_FLAGS )

GAME(199?, m4bj,      0,          mod2_cheatchr_pal<mpu4_characteriser_pal::viz_characteriser_prot>(R5R, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Black Jack (Barcrest) (Dutch) (MPU4) (BJ 1.6)",GAME_FLAGS )

GAME(199?, m4flash,   0,          mod2_cheatchr_pal<mpu4_characteriser_pal::pzmoney_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Flash Cash (Barcrest) (MPU4, FC 1.0)",GAME_FLAGS )

GAME(199?, m4swpnot,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::actionbank_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Swap-A-Note (Barcrest) (MPU4) (SN 3.3)",GAME_FLAGS )
GAME(199?, m4swpnota, m4swpnot,   mod2_cheatchr_pal<mpu4_characteriser_pal::actionbank_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Swap-A-Note (Barcrest) (MPU4) (SN 3.2)",GAME_FLAGS )
GAME(199?, m4swpnotb, m4swpnot,   mod2_cheatchr_pal<mpu4_characteriser_pal::actionbank_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Swap-A-Note (Barcrest) (MPU4) (SN 3.5)",GAME_FLAGS ) // was in a set named Centrepoint, but clearly fits here

GAME(199?, m4actnot,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Action Note (Barcrest) (MPU4) (AN 1.2)",GAME_FLAGS )

GAME(199?, m4cardcs,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Card Cash (Barcrest) (MPU4) (CCS 1.9)",GAME_FLAGS )

GAME(199?, m4bnknot,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Bank A Note (Barcrest) (MPU4) (BN 1.0)",GAME_FLAGS )

GAME(199?, m4moneym,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Money Maker (Barcrest) (MPU4) (MMK 1.6)",GAME_FLAGS )

// this and Cash Counter might be swapped around
GAME(199?, m4cashcn,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::tentendia_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Cash Connect (Barcrest) (MPU4) (CCO 3.2)",GAME_FLAGS )

GAME(199?, m4cashco,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::vivlv_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Cash Counter (Barcrest) (MPU4) (C3 2.4)",GAME_FLAGS )
GAME(199?, m4cashcoa, m4cashco,   mod2_cheatchr_pal<mpu4_characteriser_pal::vivlv_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Cash Counter (Barcrest) (MPU4) (C3 1.8)",GAME_FLAGS )
GAME(199?, m4cashcob, m4cashco,   mod2_cheatchr_pal<mpu4_characteriser_pal::vivlv_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Cash Counter (Barcrest) (MPU4) (CO 0.5)",GAME_FLAGS )
GAME(199?, m4cashcoc, m4cashco,   mod2_cheatchr_pal<mpu4_characteriser_pal::vivlv_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Cash Counter (Barcrest) (MPU4) (C3 3.1)",GAME_FLAGS )
GAME(199?, m4cashcod, m4cashco,   mod2_cheatchr_pal<mpu4_characteriser_pal::vivlv_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Cash Connect (Barcrest) (MPU4) (C3 2.0)",GAME_FLAGS )

// unusual input mapping
GAME(199?, m4cashmx,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::cashmx_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Cash Matrix (Barcrest) (MPU4) (CM 1.7, set 1)",GAME_FLAGS ) // hangs during play
GAME(199?, m4cashmxa, m4cashmx,   mod2_cheatchr_pal<mpu4_characteriser_pal::cashmx_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Cash Matrix (Barcrest) (MPU4) (CM 1.7, set 2)",GAME_FLAGS ) // hangs during play

GAME(199?, m4cashzn,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::m578_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Cash Zone (Barcrest) (MPU4) (CAZ 1.2)",GAME_FLAGS )
GAME(199?, m4cashzna, m4cashzn,   mod2_cheatchr_pal<mpu4_characteriser_pal::m578_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Cash Zone (Barcrest) (MPU4) (CAZ 1.5)",GAME_FLAGS )
// non-standard protection
GAME(199?, m4czne,    m4cashzn,   mod2_bootleg_fixedret<0x9a>(R4, RT1),       mpu4,    mpu4mod2_machines_state, init_m4, ROT0,   "bootleg?","Cash Zone (bootleg) (MPU4) (CAZ 1.5)",GAME_FLAGS )


GAME(199?, m4dblup,   0,          mod2_cheatchr_pal<mpu4_characteriser_pal::addr_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Double Up (Barcrest) (MPU4) (DU 1.5)",GAME_FLAGS ) // token alarm

GAME(199?, m4eighth,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::alf_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Eighth Wonder (Barcrest) (MPU4) (WON 2.2)",GAME_FLAGS )
//
GAME(199?, m4eightha, m4eighth,   mod2_cheatchr_pal<mpu4_characteriser_pal::eighth_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "BWB","Eighth Wonder (Barcrest) (MPU4) (BEW 0.3, set 1)",GAME_FLAGS )
GAME(199?, m4eighthb, m4eighth,   mod2_cheatchr_pal<mpu4_characteriser_pal::eighth_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "BWB","Eighth Wonder (Barcrest) (MPU4) (BEW 0.3, set 2)",GAME_FLAGS )
GAME(199?, m4eighthc, m4eighth,   mod2_cheatchr_pal<mpu4_characteriser_pal::eighth_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "BWB","Eighth Wonder (Barcrest) (MPU4) (BEW 0.3, set 3)",GAME_FLAGS )
GAME(199?, m4eighthd, m4eighth,   mod2_cheatchr_pal<mpu4_characteriser_pal::eighth_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "BWB","Eighth Wonder (Barcrest) (MPU4) (BEW 0.3, set 4, bad?)",GAME_FLAGS )
GAME(199?, m4eighthe, m4eighth,   mod2_cheatchr_pal<mpu4_characteriser_pal::eighth_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "BWB","Eighth Wonder (Barcrest) (MPU4) (BEW 0.3, set 5)",GAME_FLAGS )
GAME(199?, m4eighthf, m4eighth,   mod2_cheatchr_pal<mpu4_characteriser_pal::eighth_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "BWB","Eighth Wonder (Barcrest) (MPU4) (BEW 0.3, set 6)",GAME_FLAGS )
GAME(199?, m4eighthg, m4eighth,   mod2_cheatchr_pal<mpu4_characteriser_pal::eighth_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "BWB","Eighth Wonder (Barcrest) (MPU4) (BEW 0.3, set 7)",GAME_FLAGS )

GAME(199?, m4frtprs,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::m4lv_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Fruit Preserve (Barcrest) (MPU4) (F4P 1.1, set 1)",GAME_FLAGS )
GAME(199?, m4frtprsa, m4frtprs,   mod2_cheatchr_pal<mpu4_characteriser_pal::m4lv_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Fruit Preserve (Barcrest) (MPU4) (F4P 1.1, set 2)",GAME_FLAGS )

GAME(199?, m4gldstr,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::magicdragon_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Gold Strike (Barcrest) (MPU4) (G4S 2.0)",GAME_FLAGS )

GAME(199?, m4grands,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::grandclub_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Grandstand Club (Barcrest) (MPU4) (G2D 4.0)",GAME_FLAGS )
GAME(199?, m4grandsa, m4grands,   mod2_cheatchr_pal<mpu4_characteriser_pal::grandclub_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Grandstand Club (Barcrest) (MPU4) (GD 1.1)",GAME_FLAGS )

GAME(199?, m4intcep,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::intcep_characteriser_prot>(R4, RT1), mpu4_70pc,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Interceptor (Barcrest) (MPU4) (INT 3.0)",GAME_FLAGS ) // set % key
GAME(199?, m4intcepa, m4intcep,   mod2_cheatchr_pal<mpu4_characteriser_pal::intcep_characteriser_prot>(R4, RT1), mpu4_70pc,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Interceptor (Barcrest) (MPU4) (INT 3.0X)",GAME_FLAGS ) // set % key
// non-standard protection? hack?
GAME(199?, m4intcepb, m4intcep,   mod2_bootleg_fixedret<0x9a>(R4, RT1), mpu4_70pc,            mpu4mod2_machines_state, init_m4,     ROT0,   "hack?","Interceptor (Barcrest) (MPU4) (INT 1.1)",GAME_FLAGS )

GAME(199?, m4megbks,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::age_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Mega Bucks (Barcrest) (MPU4) (BUC 4.1X)",GAME_FLAGS )
GAME(199?, m4megbksa, m4megbks,   mod2_cheatchr_pal<mpu4_characteriser_pal::age_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Mega Bucks (Barcrest) (MPU4) (BUC 4.1CX)",GAME_FLAGS )
GAME(199?, m4megbksb, m4megbks,   mod2_cheatchr_pal<mpu4_characteriser_pal::age_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Mega Bucks (Barcrest) (MPU4) (BUC 4.1XD)",GAME_FLAGS )
GAME(199?, m4megbksc, m4megbks,   mod2_cheatchr_pal<mpu4_characteriser_pal::age_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Mega Bucks (Barcrest) (MPU4) (BUC 3.1)",GAME_FLAGS )

GAME(199?, m4mirage,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::alf_characteriser_prot>(R4, RT1), mpu4_70pc,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Mirage (Barcrest) (MPU4) (RAG 4.1)",GAME_FLAGS )

GAME(199?, m4nifty,   0,          mod2_cheatchr_pal<mpu4_characteriser_pal::nifty_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Nifty Fifty (Barcrest) (MPU4) (NF 2.0)",GAME_FLAGS )
GAME(199?, m4niftya,  m4nifty,    mod2_cheatchr_pal<mpu4_characteriser_pal::nifty_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Nifty Fifty (Barcrest) (MPU4) (NF 2.1, set 1)",GAME_FLAGS )
GAME(199?, m4niftyb,  m4nifty,    mod2_cheatchr_pal<mpu4_characteriser_pal::nifty_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Nifty Fifty (Barcrest) (MPU4) (NF 2.1, set 2)",GAME_FLAGS )

GAME(199?, m4nudqst,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::fruitfall_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Nudge Quest (Barcrest) (MPU4) (NQ 2.0)",GAME_FLAGS )

GAME(199?, m4r2r,     0,          mod2_cheatchr_pal<mpu4_characteriser_pal::rhm_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Reel 2 Reel (Barcrest) (MPU4) (RR  3.0)",GAME_FLAGS )

GAME(199?, m4runawy,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::m441_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Runaway Trail (Barcrest) (MPU4) (R4T 1.1)",GAME_FLAGS )
GAME(199?, m4runawyb, m4runawy,   mod2_cheatchr_pal<mpu4_characteriser_pal::m441_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Runaway Trail (Barcrest) (MPU4) (R4T 1.3)",GAME_FLAGS )

GAME(199?, m4silshd,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::m450_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Silver Shadow (Barcrest) (MPU4)",GAME_FLAGS )
// these don't boot (bad dumps)
GAME(199?, m4silshda, m4silshd,   mod2_cheatchr(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Silver Shadow (Barcrest) (MPU4) (SH 2.0, set 1)",GAME_FLAGS )
GAME(199?, m4silshdb, m4silshd,   mod2_cheatchr(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Silver Shadow (Barcrest) (MPU4) (SH 2.0, set 2)",GAME_FLAGS )

GAME(199?, m4solsil,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::luckystrike_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Solid Silver Club (Barcrest) (MPU4) (SOS 2.2)",GAME_FLAGS )
GAME(199?, m4solsila, m4solsil,   mod2_cheatchr_pal<mpu4_characteriser_pal::luckystrike_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Solid Silver Club (Barcrest) (MPU4) (SOS 2.1)",GAME_FLAGS )

GAME(199?, m4starbr,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::starsbars_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Stars And Bars (Barcrest) (Dutch) (MPU4) (DSB 2.8)",GAME_FLAGS )

GAME(199?, m4sunset,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::eighth_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "Barcrest","Sunset Boulevard (Barcrest) (MPU4) (BSB 0.4)",GAME_FLAGS )
GAME(199?, m4sb5,     m4sunset,   mod2_cheatchr_pal<mpu4_characteriser_pal::eighth_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Sunset Boulevard (Barcrest) (MPU4) (BSB 0.3)",GAME_FLAGS )
//
GAME(199?, m4sunsetd, m4sunset,   mod2_cheatchr_pal<mpu4_characteriser_pal::sunsetb_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "Barcrest","Sunset Boulevard (Barcrest) (MPU4) (SBU 2.0)",GAME_FLAGS )
//
GAME(199?, m4sunsete, m4sunset,   mod2_cheatchr_pal<mpu4_characteriser_pal::sunsetbalt_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "Barcrest","Sunset Boulevard (Barcrest) (MPU4) (BS__ 1.1)",GAME_FLAGS )
GAME(199?, m4sunsetf, m4sunset,   mod2_cheatchr_pal<mpu4_characteriser_pal::sunsetbalt_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "Barcrest","Sunset Boulevard (Barcrest) (MPU4) (BS__ 1.0, set 1)",GAME_FLAGS )
GAME(199?, m4sunsetg, m4sunset,   mod2_cheatchr_pal<mpu4_characteriser_pal::sunsetbalt_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "Barcrest","Sunset Boulevard (Barcrest) (MPU4) (BS__ 1.0, set 2)",GAME_FLAGS )
GAME(199?, m4sunseth, m4sunset,   mod2_cheatchr_pal<mpu4_characteriser_pal::sunsetbalt_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "Barcrest","Sunset Boulevard (Barcrest) (MPU4) (BS__ 1.0, set 3, bad)",GAME_FLAGS )
GAME(199?, m4sunseti, m4sunset,   mod2_cheatchr_pal<mpu4_characteriser_pal::sunsetbalt_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "Barcrest","Sunset Boulevard (Barcrest) (MPU4) (BS__ 1.0, set 4)",GAME_FLAGS )
GAME(199?, m4sunsetj, m4sunset,   mod2_cheatchr_pal<mpu4_characteriser_pal::sunsetbalt_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "Barcrest","Sunset Boulevard (Barcrest) (MPU4) (BS__ 1.0, set 5)",GAME_FLAGS )
//
GAME(199?, m4sunsetk, m4sunset,   mod2_cheatchr_pal<mpu4_characteriser_pal::toptake_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "Barcrest","Sunset Boulevard (Barcrest) (MPU4) (SB__ 1.1)",GAME_FLAGS )
GAME(199?, m4sunsetl, m4sunset,   mod2_cheatchr_pal<mpu4_characteriser_pal::toptake_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "Barcrest","Sunset Boulevard (Barcrest) (MPU4) (SB__ 1.0, set 1)",GAME_FLAGS )
GAME(199?, m4sunsetm, m4sunset,   mod2_cheatchr_pal<mpu4_characteriser_pal::toptake_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "Barcrest","Sunset Boulevard (Barcrest) (MPU4) (SB__ 1.0, set 2)",GAME_FLAGS )
GAME(199?, m4sunsetn, m4sunset,   mod2_cheatchr_pal<mpu4_characteriser_pal::toptake_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "Barcrest","Sunset Boulevard (Barcrest) (MPU4) (SB__ 1.0, set 3)",GAME_FLAGS )
GAME(199?, m4sunseto, m4sunset,   mod2_cheatchr_pal<mpu4_characteriser_pal::toptake_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "Barcrest","Sunset Boulevard (Barcrest) (MPU4) (SB__ 1.0, set 4)",GAME_FLAGS )
GAME(199?, m4sunsetp, m4sunset,   mod2_cheatchr_pal<mpu4_characteriser_pal::toptake_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "Barcrest","Sunset Boulevard (Barcrest) (MPU4) (SB__ 1.0, set 5)",GAME_FLAGS )
GAME(199?, m4sunsetq, m4sunset,   mod2_cheatchr_pal<mpu4_characteriser_pal::toptake_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "Barcrest","Sunset Boulevard (Barcrest) (MPU4) (SB__ 1.0, set 6)",GAME_FLAGS )
GAME(199?, m4sunsetr, m4sunset,   mod2_cheatchr_pal<mpu4_characteriser_pal::toptake_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "Barcrest","Sunset Boulevard (Barcrest) (MPU4) (SB__ 1.0, set 7)",GAME_FLAGS )
GAME(199?, m4sunsets, m4sunset,   mod2_cheatchr_pal<mpu4_characteriser_pal::toptake_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "Barcrest","Sunset Boulevard (Barcrest) (MPU4) (SB__ 1.0, set 8)",GAME_FLAGS )
GAME(199?, m4sunsett, m4sunset,   mod2_cheatchr_pal<mpu4_characteriser_pal::toptake_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "Barcrest","Sunset Boulevard (Barcrest) (MPU4) (SB__ 1.0, set 9)",GAME_FLAGS )
// non-standard protection
GAME(199?, m4sunsetc, m4sunset,   mod2_bootleg_fixedret<0xd0>(R4, RT1),    mpu4,    mpu4mod2_machines_state, init_m4, ROT0,   "bootleg","Sunset Boulevard (Barcrest) (bootleg) (MPU4) (OSB 0.2)",GAME_FLAGS )


GAME(199?, m4supslt,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::rr6_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Supa Slot (Barcrest) (MPU4) (S4S 1.0)",GAME_FLAGS )

GAME(199?, m4suptub,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::alf_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Super Tubes (Barcrest) (MPU4) (S4T 1.0, set 1)",GAME_FLAGS )
GAME(199?, m4suptuba, m4suptub,   mod2_cheatchr_pal<mpu4_characteriser_pal::alf_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Super Tubes (Barcrest) (MPU4) (S4T 1.0, set 2)",GAME_FLAGS )

GAME(199?, m4suptwo,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::actionbank_characteriser_prot>(R4, RT1), mpu4_70pc,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Super Two (Barcrest) (MPU4) (SUT 1.2)",GAME_FLAGS ) // set % key

GAME(199?, m4tiktak,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::tictak_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Tic Tak Cash (Barcrest) (MPU4) (TC 1.1)",GAME_FLAGS )

// these have the same version number but use different protection PALs, is one modified?
GAME(199?, m4topact,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::topaction_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Top Action (Barcrest) (Dutch) (MPU4) (TA 2.2, set 1)",GAME_FLAGS )
//
GAME(199?, m4topacta, m4topact,   mod2_cheatchr_pal<mpu4_characteriser_pal::m441_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Top Action (Barcrest) (Dutch) (MPU4) (TA 2.2, set 2)",GAME_FLAGS )

GAME(199?, m4topst,   0,          mod2_cheatchr_pal<mpu4_characteriser_pal::du91_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Top Stop (Barcrest) (MPU4) (TSP 0.5)",GAME_FLAGS )

GAME(199?, m4toptak,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::toptake_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Top Take (Barcrest) (MPU4) (TTK 1.1)",GAME_FLAGS )

GAME(199?, m4tribnk,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::tribank_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Triple Bank (Barcrest) (Dutch) (MPU4) (DTB 1.2)",GAME_FLAGS )

// unprotected
GAME(199?, m4tupen,   0,          mod2(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "bootleg","Tuppenny Cracker (unprotected bootleg) (MPU4)",GAME_FLAGS ) // bootleg of Barcrest game

GAME(199?, m421,      0,          mod2_cheatchr_pal<mpu4_characteriser_pal::tentendia_characteriser_prot>(R5R, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Twenty One (Barcrest) (Dutch) (MPU4) (DTO 2.0)",GAME_FLAGS )

GAMEL(199?, m4alpha,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::m435_characteriser_prot>(R4, RT1, HT), mpu4,            mpu4mod2_machines_state, init_m4,       ROT0,   "Barcrest","Alphabet (Barcrest) (MPU4) (A4B 1.0)",GAME_FLAGS, layout_m4alpha )

GAME(199?, m4bjack,   0,          mod2_cheatchr_pal<mpu4_characteriser_pal::phr_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Black Jack (Barcrest) (MPU4) (B2J 2.2)",GAME_FLAGS )
GAME(199?, m4bjacka,  m4bjack,    mod2_cheatchr_pal<mpu4_characteriser_pal::phr_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Black Jack (Barcrest) (MPU4) (BLA 2.0)",GAME_FLAGS )

GAME(199?, m4bjsm,    0,          mod2_cheatchr_pal_rtc<mpu4_characteriser_pal::sunsetb_characteriser_prot>(R5R, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Blackjack Super Multi (Barcrest) (German) (MPU4) (SM H1.6)",GAME_FLAGS )
GAME(199?, m4bjsma,   m4bjsm,     mod2_cheatchr_pal_rtc<mpu4_characteriser_pal::sunsetb_characteriser_prot>(R5R, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Blackjack Super Multi (Barcrest) (MPU4) (SM 1.6)",GAME_FLAGS ) // is this a different game?

GAME(198?, m4supsl,   0,          mod2_cheatchr_pal<mpu4_characteriser_pal::nifty_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest",   "Supa Silva (Barcrest) (MPU4) (SS2V 1.0)",GAME_FLAGS )

GAME(199?, m4flshlt,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::graff_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "BWB","Flashlite (BWB) (MPU4) (FLT 1.0, set 1)",GAME_FLAGS )
GAME(199?, m4flshltb, m4flshlt,   mod2_cheatchr_pal<mpu4_characteriser_pal::graff_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "BWB","Flashlite (BWB) (MPU4) (FLT 1.0, set 2)",GAME_FLAGS )
GAME(199?, m4flshltc, m4flshlt,   mod2_cheatchr_pal<mpu4_characteriser_pal::graff_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "BWB","Flashlite (BWB) (MPU4) (FLT 1.0, set 3)",GAME_FLAGS )
GAME(199?, m4flshltd, m4flshlt,   mod2_cheatchr_pal<mpu4_characteriser_pal::graff_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "BWB","Flashlite (BWB) (MPU4) (FLT 1.0, set 4)",GAME_FLAGS )
GAME(199?, m4flshlte, m4flshlt,   mod2_cheatchr_pal<mpu4_characteriser_pal::graff_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "BWB","Flashlite (BWB) (MPU4) (FLT 1.0, set 5)",GAME_FLAGS )
GAME(199?, m4flshltf, m4flshlt,   mod2_cheatchr_pal<mpu4_characteriser_pal::graff_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "BWB","Flashlite (BWB) (MPU4) (FLT 1.0, set 6)",GAME_FLAGS )
GAME(199?, m4flshltg, m4flshlt,   mod2_cheatchr_pal<mpu4_characteriser_pal::graff_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "BWB","Flashlite (BWB) (MPU4) (FLT 1.0, set 7)",GAME_FLAGS )
//
GAME(199?, m4flshlta, m4flshlt,   mod2_cheatchr_pal<mpu4_characteriser_pal::berseralt_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "BWB","Flashlite (BWB) (MPU4) (BFL 0.5)",GAME_FLAGS )

GAME(199?, m4blflsh,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::blueflash_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "BWB","Blue Flash (BWB) (MPU4) (TBF 0.3)",GAME_FLAGS )
GAME(199?, m4blflsha, m4blflsh,   mod2_cheatchr_pal<mpu4_characteriser_pal::blueflash_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "BWB","Blue Flash (BWB) (MPU4) (BFL 0.3, set 1)",GAME_FLAGS )
GAME(199?, m4blflshb, m4blflsh,   mod2_cheatchr_pal<mpu4_characteriser_pal::blueflash_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "BWB","Blue Flash (BWB) (MPU4) (BFL 0.3, set 2)",GAME_FLAGS )
GAME(199?, m4blflshc, m4blflsh,   mod2_cheatchr_pal<mpu4_characteriser_pal::blueflash_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "BWB","Blue Flash (BWB) (MPU4) (BFL 0.3, set 3)",GAME_FLAGS )
GAME(199?, m4blflshd, m4blflsh,   mod2_cheatchr_pal<mpu4_characteriser_pal::blueflash_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "BWB","Blue Flash (BWB) (MPU4) (BFL 0.3, set 4)",GAME_FLAGS )
GAME(199?, m4blflshe, m4blflsh,   mod2_cheatchr_pal<mpu4_characteriser_pal::blueflash_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "BWB","Blue Flash (BWB) (MPU4) (BFL 0.3, set 5)",GAME_FLAGS )

GAME(199?, m4ttak,    0,          mod2_cheatchr_pal<mpu4_characteriser_pal::clbveg_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Tic Tac Take (Barcrest) (MPU4) (TIC 2.0)",GAME_FLAGS )

GAME(199?, m4actbnka, m4actbnk,   mod2_cheatchr_pal<mpu4_characteriser_pal::age_characteriser_prot>(R4, RT1), mpu4jackpot8tkn, mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Action Bank (Barcrest) (Mod 2 type, AC3.0) (MPU4)",GAME_FLAGS ) // set jackpot key to 8GBP TOKEN & stake key
GAME(199?, m4actbnkb, m4actbnk,   mod2_cheatchr_pal<mpu4_characteriser_pal::age_characteriser_prot>(R4, RT1), mpu4jackpot8tkn, mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Action Bank (Barcrest) (Mod 2 type, ACT2.0) (MPU4)",GAME_FLAGS )

GAMEL(199?, m4actclb, 0,          mod2_cheatchr_pal<mpu4_characteriser_pal::kqee2_characteriser_prot>(R4, RT1, HT), mpu420p,            mpu4mod2_machines_state, init_m4,      ROT0,   "Barcrest","Action Club (Barcrest) (MPU4) (ABV 1.9)",GAME_FLAGS, layout_m4actclb ) // set stake to boot
//
GAMEL(199?, m4actclba,m4actclb,   mod2_cheatchr_pal<mpu4_characteriser_pal::actclba_characteriser_prot>(R4, RT1, HT), mpu420p,            mpu4mod2_machines_state, init_m4,      ROT0,   "Barcrest","Action Club (Barcrest) (MPU4) (A2C 1.1)",GAME_FLAGS, layout_m4actclb ) //  ^^

GAME(199?, m4bluemn,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::m4lv_characteriser_prot>(R4, RT1), mpu4_70pc,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Blue Moon (Barcrest) (MPU4) (BLU 2.3)",GAME_FLAGS )
GAME(199?, m4bluemna, m4bluemn,   mod2_cheatchr_pal<mpu4_characteriser_pal::m4lv_characteriser_prot>(R4, RT1), mpu4_70pc,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Blue Moon (Barcrest) (MPU4) (BLU 2.1)",GAME_FLAGS )
// not using standard protection, hack?
GAME(199?, m4bluemnb, m4bluemn,   mod2_bootleg_fixedret<0x51>(R4, RT1), mpu4_70pc,            mpu4mod2_machines_state, init_m4,     ROT0,   "hack?","Blue Moon (Barcrest) (MPU4) (BLU 1.1)",GAME_FLAGS )

GAME(199?, m4take2,   0,          mod2_cheatchr_pal<mpu4_characteriser_pal::take2_characteriser_prot>(R4, RT1), mpu4,   mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Take Two (Barcrest) (MPU4) (TTO 1.2)",GAME_FLAGS )
// not using standard protection, hack?
GAME(199?, m4take2a,  m4take2,    mod2_bootleg_fixedret<0x11>(R4, RT1),                                          mpu4,   mpu4mod2_machines_state, init_m4,     ROT0,   "bootleg","Take Two (Barcrest) (MPU4) (TTO 1.1) (set 1)",GAME_FLAGS )
GAME(199?, m4t266,    m4take2,    mod2_bootleg_fixedret<0x11>(R4, RT1),                                          mpu4,   mpu4mod2_machines_state, init_m4,     ROT0,   "bootleg","Take Two (Barcrest) (MPU4) (TTO 1.1) (set 2)",GAME_FLAGS )

GAME(199?, m4pont,    0,          mod2_cheatchr_pal<mpu4_characteriser_pal::pontoon_characteriser_prot>(R4, RT1), mpu420p,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Pontoon Club (Barcrest) (MPU4) (PON 3.0)",GAME_FLAGS ) // set stake to boot
GAME(199?, m4ponta,   m4pont,     mod2_cheatchr_pal<mpu4_characteriser_pal::pontoon_characteriser_prot>(R4, RT1), mpu420p,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Pontoon Club (Barcrest) (MPU4) (PON 4.0)",GAME_FLAGS )//  ^^

GAME(199?, m4loadmn,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::andybt_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Loads A Money (Barcrest) (MPU4) (LA 1.0)",GAME_FLAGS )
GAME(199?, m4loadmna, m4loadmn,   mod2_cheatchr_pal<mpu4_characteriser_pal::andybt_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Loads A Money (Barcrest) (MPU4) (LA 1.1, set 1)",GAME_FLAGS )
GAME(199?, m4loadmnb, m4loadmn,   mod2_cheatchr_pal<mpu4_characteriser_pal::andybt_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Loads A Money (Barcrest) (MPU4) (LA 1.1, set 2)",GAME_FLAGS )

GAME(199?, m4celclb,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::celclb_characteriser_prot>(R4, RT1), mpu420p,            mpu4mod2_machines_state, init_m4,      ROT0,   "Barcrest","Celebration Club (Barcrest) (MPU4) (CEL 1.5)",GAME_FLAGS ) // set stake

GAME(199?, m4centpt, 0,   mod2_cheatchr_pal<mpu4_characteriser_pal::actionbank_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Centrepoint (Barcrest) (MPU4) (DU 1.3)",GAME_FLAGS )

GAME(199?, m4clbcls,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::m533_characteriser_prot>(R4, RT1), mpu420p,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Club Classic (Barcrest) (MPU4) (CI  1.1)",GAME_FLAGS ) // set stake

GAME(199?, m4c999,   0,           mod2_cheatchr_pal<mpu4_characteriser_pal::tictak_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "Barcrest","Cloud 999 (Barcrest) (MPU4) (CLN 4.0 V)",GAME_FLAGS )
// these are bootlegs with non-standard protection
GAME(199?, m4c999a,   m4c999,     mod2_bootleg_fixedret<0x51>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "bootleg","Cloud 999 (Barcrest) (bootleg) (MPU4) (CLN 3.6)",GAME_FLAGS )
GAME(199?, m4c999b,   m4c999,     mod2_bootleg_fixedret<0x51>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "bootleg","Cloud 999 (Barcrest) (bootleg) (MPU4) (CLN 3.0)",GAME_FLAGS )
GAME(199?, m4c999c,   m4c999,     mod2_bootleg_fixedret<0x80>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "bootleg","Cloud 999 (Barcrest) (bootleg) (MPU4) (OC9 0.3, set 1)",GAME_FLAGS )
GAME(199?, m4c999d,   m4c999,     mod2_bootleg_fixedret<0x9a>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4, ROT0,   "bootleg","Cloud 999 (Barcrest) (bootleg) (MPU4) (OC9 0.3, set 2)",GAME_FLAGS )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAME(year, setname, parent, mod2_cheatchr_pal<mpu4_characteriser_pal::hittop_characteriser_prot>(R4, RT1), mpu4, mpu4mod2_machines_state, init_m4, ROT0, company, title, GAME_FLAGS )

// "(C)1993  B.W.B." and "SC9 5.0"
GAME_CUSTOM( 199?, m4c999g,   m4c999,   "c9o20__1.1",   0x0000, 0x010000, CRC(e05fa532) SHA1(63d070416a4e6979302901bb33e20c994cb3723e), "BWB","Cloud 999 (Barcrest) (MPU4) (SC9 5.0)" )
GAME_CUSTOM( 199?, m4c999h,   m4c999,   "c9o20d_1.1",   0x0000, 0x010000, CRC(047b2d83) SHA1(b83f8fe6477226ef3e75f406020ea4f8b3d55c32), "BWB","Cloud 999 (Barcrest) (MPU4) (SC9 5.0 D)" )
// no copyright string and "SC9 1.0" (hack or early Bwb set?) (still expects regular CHR protection)
GAME_CUSTOM( 199?, m4c999i,   m4c999,   "c9o02__1.1",   0x0000, 0x010000, CRC(109f7040) SHA1(3fe9da13d9746e1cdaf6dcd539e4af624d2cec71), "hack?","Cloud 999 (Barcrest) (MPU4) (SC9 1.0, hack?, set 1)" )
GAME_CUSTOM( 199?, m4c999j,   m4c999,   "c9o05__1.1",   0x0000, 0x010000, CRC(2c821aa8) SHA1(33fba7dea0f66e7b0251971864d5a2923f96f8cd), "hack?","Cloud 999 (Barcrest) (MPU4) (SC9 1.0, hack?, set 2)" )
GAME_CUSTOM( 199?, m4c999e,   m4c999,   "c9o10__1.1",   0x0000, 0x010000, CRC(c5063185) SHA1(ca98038ccd85ebc370cacce8583ddbc1f759558d), "hack?","Cloud 999 (Barcrest) (MPU4) (SC9 1.0, hack?, set 3)" )
GAME_CUSTOM( 199?, m4c999f,   m4c999,   "c9o10d_1.1",   0x0000, 0x010000, CRC(6b20b16d) SHA1(15079fc5f14f545c291d357a795e6b41ca1d5a47), "hack?","Cloud 999 (Barcrest) (MPU4) (SC9 1.0, hack?, set 4)" ) // doesn't have 'D' set but is a dataport set

ROM_START( m4c9c )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cncs.p1", 0x0000, 0x010000, CRC(10f15e2a) SHA1(c17ab13764d74302246984245485cb7692913b44) )
ROM_END

// requires stake set to boot (just hangs otherwise)
GAME(199?, m4c9c,     0,          mod2_cheatchr_pal<mpu4_characteriser_pal::tentendia_characteriser_prot>(R4, RT1),    mpu420p,    mpu4mod2_machines_state, init_m4,  ROT0,   "Barcrest","Cloud Nine Club (Barcrest) (MPU4) (CNC 2.1)",GAME_FLAGS )

// works
GAME(199?, m4bjac,    0,          mod2_cheatchr_pal<mpu4_characteriser_pal::bjac_characteriser_prot>(R5R, RT1), mpu420p,            mpu4mod2_machines_state, init_m4, ROT0,   "Barcrest","Blackjack Club (Barcrest) (MPU4) (C2J 1.8)",GAME_FLAGS )
GAME(199?, m4bjaca,   m4bjac,     mod2_cheatchr_pal<mpu4_characteriser_pal::bjac_characteriser_prot>(R5R, RT1), mpu420p,            mpu4mod2_machines_state, init_m4, ROT0,   "Barcrest","Blackjack Club (Barcrest) (MPU4) (C2J 2.1)",GAME_FLAGS )

// works
GAME(199?, m4clbcnt,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::andybt_characteriser_prot>(R5R, RT1), mpu420p,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Club Connect (Barcrest) (MPU4) (CON 1.2)",GAME_FLAGS )
GAME(199?, m4clbcnta, m4clbcnt,   mod2_cheatchr_pal<mpu4_characteriser_pal::andybt_characteriser_prot>(R5R, RT1), mpu420p,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Club Connect (Barcrest) (MPU4) (CON 1.0)",GAME_FLAGS )
GAME(199?, m4clbcntb, m4clbcnt,   mod2_cheatchr_pal<mpu4_characteriser_pal::andybt_characteriser_prot>(R5R, RT1), mpu420p,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Club Connect (Barcrest) (MPU4) (CON 1.1)",GAME_FLAGS )
GAME(199?, m4clbcntc, m4clbcnt,   mod2_cheatchr_pal<mpu4_characteriser_pal::andybt_characteriser_prot>(R5R, RT1), mpu420p,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Club Connect (Barcrest) (MPU4) (CON 1.5, set 1)",GAME_FLAGS )
GAME(199?, m4clbcntd, m4clbcnt,   mod2_cheatchr_pal<mpu4_characteriser_pal::andybt_characteriser_prot>(R5R, RT1), mpu420p,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Club Connect (Barcrest) (MPU4) (CON 1.5, set 2)",GAME_FLAGS )

// works
GAME(199?, m4frtfl,   0,          mod2_cheatchr_pal<mpu4_characteriser_pal::fruitfall_characteriser_prot>(R5R, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Fruit Full Club (Barcrest) (MPU4) (FFC 0.3)",GAME_FLAGS )
GAME(199?, m4frtfla,  m4frtfl,    mod2_cheatchr_pal<mpu4_characteriser_pal::fruitfall_characteriser_prot>(R5R, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Fruit Full Club (Barcrest) (MPU4) (FFC 1.0)",GAME_FLAGS )
GAME(199?, m4frtflc,  m4frtfl,    mod2_cheatchr_pal<mpu4_characteriser_pal::fruitfall_characteriser_prot>(R5R, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Fruit Full Club (Barcrest) (MPU4) (FFC 1.2)",GAME_FLAGS )

// works
GAME(199?, m4frtlnk,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::fruitfall_characteriser_prot>(R5R, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Fruit Link Club (Barcrest) (MPU4) (FLC 1.8, set 1)",GAME_FLAGS )
GAME(199?, m4frtlnka, m4frtlnk,   mod2_cheatchr_pal<mpu4_characteriser_pal::fruitfall_characteriser_prot>(R5R, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Fruit Link Club (Barcrest) (MPU4) (FLC 1.6, set 2)",GAME_FLAGS )

// works
GAME(199?, m4milclb,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::milclb_characteriser_prot>(R5R, RT1), mpu420p,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Millionaire's Club (Barcrest) (MPU4) (MI2 1.0, set 1)",GAME_FLAGS )
GAME(199?, m4milclba, m4milclb,   mod2_cheatchr_pal<mpu4_characteriser_pal::milclb_characteriser_prot>(R5R, RT1), mpu420p,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Millionaire's Club (Barcrest) (MPU4) (MI2 1.0, set 2)",GAME_FLAGS )
GAME(199?, m4milclbb, m4milclb,   mod2_cheatchr_pal<mpu4_characteriser_pal::milclb_characteriser_prot>(R5R, RT1), mpu420p,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Millionaire's Club (Barcrest) (MPU4) (MI2 1.0, set 3)",GAME_FLAGS )
GAME(199?, m4milclbc, m4milclb,   mod2_cheatchr_pal<mpu4_characteriser_pal::milclb_characteriser_prot>(R5R, RT1), mpu420p,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Millionaire's Club (Barcrest) (MPU4) (MIL 5.0, set 1)",GAME_FLAGS )
GAME(199?, m4milclbd, m4milclb,   mod2_cheatchr_pal<mpu4_characteriser_pal::milclb_characteriser_prot>(R5R, RT1), mpu420p,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Millionaire's Club (Barcrest) (MPU4) (MIL 5.0, set 2)",GAME_FLAGS )

// works
GAME(199?, m4jpjmp,   0,          mod2_cheatchr_pal<mpu4_characteriser_pal::vivlv_characteriser_prot>(R5R, RT1), mpu420p,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Jackpot Jump (Barcrest) (MPU4) (VJC 2.0)",GAME_FLAGS )
GAME(199?, m4jpjmpa,  m4jpjmp,    mod2_cheatchr_pal<mpu4_characteriser_pal::vivlv_characteriser_prot>(R5R, RT1), mpu420p,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Jackpot Jump (Barcrest) (MPU4) (VJC 1.3)",GAME_FLAGS )

GAME(199?, m4ambass,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::nifty_characteriser_prot>(R4, RT1), mpu4_dutch,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Ambassador (Barcrest) (Dutch) (MPU4) (DAM 3.7)",GAME_FLAGS )

GAME(199?, m4graffd,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::turboplay_characteriser_prot>(R7, RT1), mpu4_dutch, mpu4mod2_machines_state, init_m4,  ROT0,   "Barcrest","Grafitti (Barcrest) (Dutch) (MPU4) (DGR 1.3)",GAME_FLAGS )

GAME(199?, m4atlan,   0,          mod2_cheatchr_pal<mpu4_characteriser_pal::m533_characteriser_prot>(R4, RT1), mpu4_dutch,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Atlantis (Barcrest) (Dutch) (MPU4) (DAT 1.4)",GAME_FLAGS ) // was also an identical set marked 'Triple Dice'

GAME(199?, m4bjc,     0,          mod2_cheatchr_pal<mpu4_characteriser_pal::vivlv_characteriser_prot>(R7, RT1), mpu4_dutch,            mpu4mod2_machines_state, init_m4, ROT0,   "Barcrest","Black Jack Club (Barcrest) (Dutch) (MPU4) (DBC 1.1)",GAME_FLAGS )

GAME(199?, m4exprs,   0,          mod2_cheatchr_pal<mpu4_characteriser_pal::cosmiccasino_characteriser_prot>(R6A, RT1), mpu4_dutch,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Express (Barcrest) (Dutch) (MPU4) (DXP 2.0)",GAME_FLAGS )

GAME(199?, m4brdway,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::phr_characteriser_prot>(R4, RT1), mpu4_dutch,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Broadway (Barcrest) (Dutch) (MPU4) (DBR 1.1)",GAME_FLAGS )

GAME(199?, m4bigbn,   0,          mod2_cheatchr_pal<mpu4_characteriser_pal::m4lv_characteriser_prot>(R6A, RT1), mpu4_dutch,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Big Ben (Barcrest) (Dutch) (MPU4) (DBB 1.2)",GAME_FLAGS )

GAME(199?, m4cheryo,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::cheryo_characteriser_prot>(R6A, RT1), mpu4_dutch,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Cherryo (Barcrest) (Dutch) (MPU4) (DCH 1.4)",GAME_FLAGS )

GAME(199?, m4giant,   0,          mod2_cheatchr_pal<mpu4_characteriser_pal::giant_characteriser_prot>(R6A, RT1), mpu4_dutch,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Giant (Barcrest) (Dutch) (MPU4) (DGI 2.1)",GAME_FLAGS )

GAME(199?, m4holdon,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::m441_characteriser_prot>(R6A, RT1), mpu4_dutch,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Hold On (Barcrest) (Dutch) (MPU4) (DHO 2.5)",GAME_FLAGS )

GAME(199?, m4libty,   0,          mod2_cheatchr_pal<mpu4_characteriser_pal::crkpot_characteriser_prot>(R6A, RT1), mpu4_dutch,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Liberty (Barcrest) (Dutch) (MPU4) (DLI 1.0)",GAME_FLAGS )

GAME(199?, m4meglnk,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::andybt_characteriser_prot>(R4, RT1), mpu4_dutch,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Megalink (Barcrest) (Dutch) (MPU4) (DML 2.0)",GAME_FLAGS )

GAME(199?, m4multwy,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::kingqn_characteriser_prot>(R4, RT1), mpu4_dutch,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Multiway (Barcrest) (Dutch) (MPU4) (DMU 1.7)",GAME_FLAGS )

GAME(199?, m4num1,    0,          mod2_cheatchr_pal<mpu4_characteriser_pal::pzmoney_characteriser_prot>(R4, RT1), mpu4_dutch,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Number One (Barcrest) (Dutch) (MPU4) (DNO 1.7)",GAME_FLAGS )

GAME(199?, m4nudup,   0,          mod2_cheatchr_pal<mpu4_characteriser_pal::clbveg_characteriser_prot>(R4, RT1), mpu4_dutch,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Nudge Up (Barcrest) (Dutch) (MPU4) (DNU 2.5)",GAME_FLAGS )

GAME(199?, m4omega,   0,          mod2_cheatchr_pal<mpu4_characteriser_pal::intcep_characteriser_prot>(R6A, RT1), mpu4_dutch,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Omega (Barcrest) (Dutch) (MPU4) (DOM 2.3)",GAME_FLAGS )

GAME(199?, m4randr,   0,          mod2_cheatchr_pal<mpu4_characteriser_pal::randroul_characteriser_prot>(R4, RT1), mpu4_dutch,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Random Roulette (Barcrest) (Dutch) (MPU4) (DRR 2.2)",GAME_FLAGS )

GAME(199?, m4samu,    0,          mod2_cheatchr_pal<mpu4_characteriser_pal::viva_characteriser_prot>(R6A, RT1), mpu4_dutch,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Samurai (Barcrest) (Dutch) (MPU4) (DSM 1.0)",GAME_FLAGS )

GAME(199?, m4stards,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::rr6_characteriser_prot>(R4, RT1), mpu4_dutch,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Stardust (Barcrest) (Dutch) (MPU4) (DSD 1.3)",GAME_FLAGS )

GAME(199?, m4tbreel,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::grandclub_characteriser_prot>(R4, RT1), mpu4_dutch,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Turbo Reel (Barcrest) (Dutch) (MPU4) (DTR 3.1)",GAME_FLAGS )

GAME(199?, m4tbrldx,  m4tbreel,   mod2_cheatchr_pal<mpu4_characteriser_pal::rhm_characteriser_prot>(R4, RT1), mpu4_dutch,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Turbo Reel Deluxe (Barcrest) (Dutch) (MPU4) (DTU 3.0)",GAME_FLAGS )

GAME(199?, m4taj,     0,          mod2_cheatchr_pal<mpu4_characteriser_pal::tajmahal_characteriser_prot>(R4, RT1), mpu4_dutch,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Taj Mahal (Barcrest) (Dutch) (MPU4) (DTM 1.0)",GAME_FLAGS )

GAME(199?, m4tricol,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::tricolor_characteriser_prot>(R6A, RT1), mpu4_dutch,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Tricolor (Barcrest) (Dutch) (MPU4) (DTC 2.5)",GAME_FLAGS )

GAME(199?, m4twilgt,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::pontoon_characteriser_prot>(R6A, RT1), mpu4_dutch,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Twilight (Barcrest) (Dutch) (MPU4) (DTL 2.2)",GAME_FLAGS )

GAME(199?, m4wildms,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>(R6A, RT1), mpu4_dutch,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Wild Mystery (Barcrest) (Dutch) (MPU4) (DWM 1.8)",GAME_FLAGS )

GAME(199?, m4suptrn,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::actionbank_characteriser_prot>(R7, RT1), mpu4_dutch,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Supatron (Barcrest) (MPU4) (DSU 2.1)",GAME_FLAGS )

GAME(199?, m421club,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::blackwhite_characteriser_prot>(R7, RT1), mpu4_dutch,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","21 Club (Barcrest) (Dutch) (MPU4) (DTW 2.7)",GAME_FLAGS )

GAME(199?, m4class,   0,          mod2_cheatchr_pal<mpu4_characteriser_pal::take2_characteriser_prot>(R7, RT1), mpu4_dutch,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","First Class (Barcrest) (Dutch) (MPU4) (DFC 2.0, set 1)",GAME_FLAGS )
// INITIALIZE COMMS, before any prot sequence
GAME(199?, m4classa,  m4class,    mod2_cheatchr_pal<mpu4_characteriser_pal::take2_characteriser_prot>(R7, RT1), mpu4_dutch,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","First Class (Barcrest) (Dutch) (MPU4) (DFC 2.0, set 2)",GAME_FLAGS )

GAME(199?, m4thehit,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::toptake_characteriser_prot>(R7, RT1), mpu4_dutch,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","The Hit (Barcrest) (MPU4) (DTH 1.7)",GAME_FLAGS )

GAME(199?, m4gldgat,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::m450_characteriser_prot>(R7, RT1),   mpu4_dutch,    mpu4mod2_machines_state, init_m4, ROT0,   "Barcrest","Golden Gate (Barcrest) (Dutch) (MPU4) (DGG 2.2)",GAME_FLAGS )

GAME(199?, m4toma,    0,          mod2_cheatchr_pal<mpu4_characteriser_pal::m400_characteriser_prot>(R7, RT1),   mpu4_dutch,    mpu4mod2_machines_state, init_m4, ROT0,   "Barcrest","Tomahawk (Barcrest) (Dutch) (MPU4) (DTK 2.3)",GAME_FLAGS )

GAME(199?, m4topdk,   0,          mod2_cheatchr_pal<mpu4_characteriser_pal::intcep_characteriser_prot>(R4, RT1),       mpu4_dutch,    mpu4mod2_machines_state, init_m4, ROT0,   "Barcrest","Top Deck (Barcrest) (Dutch) (MPU4) (DT 2.6)",GAME_FLAGS )

// boots but no coins?
GAME(199?, m4bigchd,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::clbveg_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Big Chief (Barcrest) (Dutch) (MPU4) (BCH 1.5)",GAME_FLAGS ) // why code BCH on a dutch?

// boots with percent key
GAME(199?, m4dbl9,    0,          mod2_cheatchr_pal<mpu4_characteriser_pal::du91_characteriser_prot>(R4, RT1), mpu4_70pc,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Double 9's (Barcrest) (MPU4) (DU9 1.0)",GAME_FLAGS )
// non-standard chr use, hack?
GAME(199?, m4dbl9a,   m4dbl9,     mod2_bootleg_fixedret<0x51>(R4, RT1), mpu4_70pc,            mpu4mod2_machines_state, init_m4,     ROT0,   "bootleg","Double 9's (Barcrest) (bootleg) (MPU4) (DU9 0.2)",GAME_FLAGS )

GAME(199?, m4nick,    0,          mod2_cheatchr_pal<mpu4_characteriser_pal::viz_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Nickelodeon (Barcrest) (MPU4) (NIL 4.1, set 1)",GAME_FLAGS )
GAME(199?, m4nicka,   m4nick,     mod2_cheatchr_pal<mpu4_characteriser_pal::viz_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Nickelodeon (Barcrest) (MPU4) (NIL 4.1, set 2)",GAME_FLAGS )
GAME(199?, m4nickb,   m4nick,     mod2_cheatchr_pal<mpu4_characteriser_pal::viz_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Nickelodeon (Barcrest) (MPU4) (NIL 4.1, set 3)",GAME_FLAGS )
GAME(199?, m4nickc,   m4nick,     mod2_cheatchr_pal<mpu4_characteriser_pal::viz_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Nickelodeon (Barcrest) (MPU4) (NIL 4.1, set 4)",GAME_FLAGS )
GAME(199?, m4nickd,   m4nick,     mod2_cheatchr_pal<mpu4_characteriser_pal::viz_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Nickelodeon (Barcrest) (MPU4) (NIL 4.1, set 5)",GAME_FLAGS )
// non-standard protection, hack?
GAME(199?, m4nicke,   m4nick,     mod2_bootleg_fixedret<0x1b>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "bootleg","Nickelodeon (Barcrest) (bootleg) (MPU4) (NIL 2.5)",GAME_FLAGS )

GAME(199?, m4joljok,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Jolly Joker (Barcrest) (MPU4) (JOJ 1.6)",GAME_FLAGS )
// non-standard protection
GAME(199?, m4unkjok,  m4joljok,   mod2_bootleg_fixedret<0x9a>(R4, RT1),       mpu4,    mpu4mod2_machines_state, init_m4, ROT0,   "bootleg","Jolly Joker (Barcrest) (MPU4) (bootleg) (JJ1 0.1, set 1)",GAME_FLAGS )
GAME(199?, m4unkjoka, m4joljok,   mod2_bootleg_fixedret<0x9a>(R4, RT1),       mpu4,    mpu4mod2_machines_state, init_m4, ROT0,   "bootleg","Jolly Joker (Barcrest) (MPU4) (bootleg) (JJ1 0.1. set 2)",GAME_FLAGS )

GAME(199?, m4joltav,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::m574_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Jolly Taverner (Barcrest) (MPU4) (TAV 1.3)",GAME_FLAGS )
//
GAME(199?, m4joltava, m4joltav,   mod2_cheatchr_pal<mpu4_characteriser_pal::viva_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Jolly Taverner (Barcrest) (MPU4) (JT__ 2.0, set 1)",GAME_FLAGS )
GAME(199?, m4joltavb, m4joltav,   mod2_cheatchr_pal<mpu4_characteriser_pal::viva_characteriser_prot>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Barcrest","Jolly Taverner (Barcrest) (MPU4) (JT__ 2.0, set 2)",GAME_FLAGS )
//
GAME(199?, m4unkjokb, m4joltav,   mod2_bootleg_fixedret<0x11>(R4, RT1),       mpu4,    mpu4mod2_machines_state, init_m4, ROT0,   "bootleg","Jolly Taverner (Barcrest) (bootleg) (MPU4) (TAV 1.1, set 1)",GAME_FLAGS )
GAME(199?, m4unkjokc, m4joltav,   mod2_bootleg_fixedret<0x11>(R4, RT1),       mpu4,    mpu4mod2_machines_state, init_m4, ROT0,   "bootleg","Jolly Taverner (Barcrest) (bootleg) (MPU4) (TAV 1.1, set 2)",GAME_FLAGS )

// this has a 'tri98' protection sequence check in ROM, but the code appears to have been hacked to expect a different response.
GAME(199?, m4btclok,  0,          mod2_bootleg_fixedret<0x45>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "bootleg","Beat The Clock (Barcrest) (bootleg) (MPU4)",GAME_FLAGS )

// protection has been hacked
GAME(199?, m4brktak,  0,          mod2_bootleg_fixedret<0x45>(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "bootleg","Break & Take (Barcrest) (bootleg) (MPU4)",GAME_FLAGS )

// runs if you set a stake, missing an extender for the credits display?
GAME(199?, m4sunclb,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::intcep_characteriser_prot>(R4, RT1),       mpu420p,    mpu4mod2_machines_state, init_m4, ROT0,   "BWB","Sun Club (BWB) (MPU4) (SUC 0.2, set 1)",GAME_FLAGS )
GAME(199?, m4sunclba, m4sunclb,   mod2_cheatchr_pal<mpu4_characteriser_pal::intcep_characteriser_prot>(R4, RT1),       mpu420p,    mpu4mod2_machines_state, init_m4, ROT0,   "BWB","Sun Club (BWB) (MPU4) (SUC 0.2, set 2)",GAME_FLAGS )


// these were found in with mod4oki sets, but don't attempt to play samples, only use the AY

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAME(year, setname, parent, mod2_cheatchr_pal<mpu4_characteriser_pal::tentendia_characteriser_prot>(R4, RT1), mpu4, mpu4mod2_machines_state, init_m4, ROT0, company, title, GAME_FLAGS )

// "(C)1991 BARCREST" and "NN3 0.1"
GAME_CUSTOM( 199?, m4nnww2,       0,       "nn3xs.p1",     0x0000, 0x010000, CRC(13d02d21) SHA1(8e4dac8e60538884d3f3a92fc1bb9f41276be4c8), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4, Mod2 type) (NN3 0.1 X)" )
GAME_CUSTOM( 199?, m4nnww2__a,    m4nnww2, "nn3xad.p1",    0x0000, 0x010000, CRC(8ccfceb8) SHA1(762ab26826d3d2a4dd7999a71724389344e9dafb), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4, Mod2 type) (NN3 0.1 XAD)" )
GAME_CUSTOM( 199?, m4nnww2__b,    m4nnww2, "nn3xb.p1",     0x0000, 0x010000, CRC(9b0dd473) SHA1(9975dafea8c7d6ccfc9f826adb1a0d3d0ed9740a), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4, Mod2 type) (NN3 0.1 BX)" )
GAME_CUSTOM( 199?, m4nnww2__c,    m4nnww2, "nn3xbd.p1",    0x0000, 0x010000, CRC(21bf4a89) SHA1(200c9ccc4bc2a93fcd0f68bb00ad4391bdeecda1), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4, Mod2 type) (NN3 0.1 BXD)" )
GAME_CUSTOM( 199?, m4nnww2__d,    m4nnww2, "nn3xd.p1",     0x0000, 0x010000, CRC(11e22c45) SHA1(6da31eea7b25612d99cc79f6f9579622f105c862), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4, Mod2 type) (NN3 0.1 XD)" )
GAME_CUSTOM( 199?, m4nnww2__e,    m4nnww2, "nn3xdk.p1",    0x0000, 0x010000, CRC(0f4642c6) SHA1(53a0b8bc102c2b1c0db71887470b70852b09a4e9), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4, Mod2 type) (NN3 0.1 KXD)" )
GAME_CUSTOM( 199?, m4nnww2__f,    m4nnww2, "nn3xdy.p1",    0x0000, 0x010000, CRC(ba3c1cf0) SHA1(ab94227018c3f9173e6a648749d455afd1ed36ce), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4, Mod2 type) (NN3 0.1 YXD)" )
GAME_CUSTOM( 199?, m4nnww2__g,    m4nnww2, "nn3xk.p1",     0x0000, 0x010000, CRC(ec3a9831) SHA1(0b3ba86faf39cf3a1e42cb1c31fd2c50c24d65dc), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4, Mod2 type) (NN3 0.1 KX)" )
GAME_CUSTOM( 199?, m4nnww2__h,    m4nnww2, "nn3xr.p1",     0x0000, 0x010000, CRC(6416481c) SHA1(b06ed4964d9cbf403905504ac68abdab53131476), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4, Mod2 type) (NN3 0.1 RX)" )
GAME_CUSTOM( 199?, m4nnww2__i,    m4nnww2, "nn3xrd.p1",    0x0000, 0x010000, CRC(0fd3f9b9) SHA1(99115b217cfc54b52469ffc77e7a7592907c53ea), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4, Mod2 type) (NN3 0.1 RD)" ) // X not set here
GAME_CUSTOM( 199?, m4nnww2__j,    m4nnww2, "nn3xy.p1",     0x0000, 0x010000, CRC(8a5d0f4b) SHA1(ef727e7ee8bb20d1b201927186a1a4f83e1e7497), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4, Mod2 type) (NN3 0.1 YX)" )

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAME(year, setname, parent, mod2_cheatchr_pal<mpu4_characteriser_pal::cashmx_characteriser_prot>(R4, RT1), mpu4, mpu4mod2_machines_state, init_m4, ROT0, company, title, GAME_FLAGS )

// "(C)1991 BARCREST" and "NNU 5.2"
GAME_CUSTOM( 199?, m4nnww2__k,    m4nnww2, "nnus.p1",      0x0000, 0x010000, CRC(3e3a829e) SHA1(5aa3a56e007bad4dacdc3c993c87569e4250eecd), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4, Mod2 type) (NNU 5.2)" )
GAME_CUSTOM( 199?, m4nnww2__l,    m4nnww2, "nnux.p1",      0x0000, 0x010000, CRC(38806ebf) SHA1(a897a33e3260de1b284b01a65d1da7cbe05d51f8), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4, Mod2 type) (NNU 5.2 X)" )
GAME_CUSTOM( 199?, m4nnww2__m,    m4nnww2, "nnuxb.p1",     0x0000, 0x010000, CRC(c4dba8df) SHA1(0f8516cc9b2f0be9d1c936667974cd8116018dad), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4, Mod2 type) (NNU 5.2 BX)" )
GAME_CUSTOM( 199?, m4nnww2__n,    m4nnww2, "nnuxc.p1",     0x0000, 0x010000, CRC(797e0c4d) SHA1(211b0a804643731275d0075461f8d94985fde1db), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4, Mod2 type) (NNU 5.2 CX)" )
// "(C)1991 BARCREST" and "NNU 4.0"
GAME_CUSTOM( 199?, m4nnww2__o,    m4nnww2, "nnu40x.bin",   0x0000, 0x010000, CRC(63e3d7df) SHA1(1a5a00185ec5150f5b05765f06297d7884540aaf), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4, Mod2 type) (NNU 4.0 X)" )


#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( 0x10000, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAME( year, setname, parent, mod2_bootleg_fixedret<0x06>(R4, RT1), mpu4, mpu4mod2_machines_state, init_m4, ROT0, company, title, GAME_FLAGS )

// no copyright string and "NNU 5.0"
GAME_CUSTOM( 199?, m4nnww2__hx3,   m4nnww2, "classic adders + ladders_alt",     0x0000, 0x010000, CRC(ac948903) SHA1(e07023efd7722a661a2bbf93c0a168af70ad6c20), "hack","Nudge Nudge Wink Wink (Barcrest) (MPU4, Mod2 type) (NNU 5.0, hack, set 1)")
GAME_CUSTOM( 199?, m4nnww2__hx4,   m4nnww2, "classic adders + ladders_alt2",    0x0000, 0x010000, CRC(843ed53d) SHA1(b1dff249df37800744e3fc9c32be20a62bd130a1), "hack","Nudge Nudge Wink Wink (Barcrest) (MPU4, Mod2 type) (NNU 5.0, hack, set 2)")

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( 0x10000, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAME( year, setname, parent, mod2(R4, RT1), mpu4, mpu4mod2_machines_state, init_m4, ROT0, company, title, GAME_FLAGS)

// fails to boot even without touching protection address due to checksum error.    These were in 'adders + ladders' sets but are clearly not
GAME_CUSTOM( 199?, m4nnww2__hx5,   m4nnww2, "nik56c",                           0x0000, 0x010000, CRC(05fa11d1) SHA1(01d3d0c504489f1513a0c3aa26e910c9604f5366) BAD_DUMP, "hack","Nudge Nudge Wink Wink (Barcrest) (MPU4, Mod2 type) (NNU 5.0, hack, set 3)")

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( 0x10000, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAME( year, setname, parent, mod2_bootleg_fixedret<0x0e>(R4, RT1), mpu4, mpu4mod2_machines_state, init_m4, ROT0, company, title, GAME_FLAGS)

// different protection
// no copyright string and "NNU 3.4"
GAME_CUSTOM( 199?, m4nnww2__hx1,  m4nnww2, "nnww2010",     0x0000, 0x010000, CRC(67b1c7b5) SHA1(495e25bc2051ab78e473cd0c36e0c1825c06db14), "hack","Nudge Nudge Wink Wink (Barcrest) (MPU4, Mod2 type) (NNU 3.4, hack, set 1)" )
GAME_CUSTOM( 199?, m4nnww2__hx2,  m4nnww2, "wink2010",     0x0000, 0x010000, CRC(056a2ffa) SHA1(9da96d70ff850b6672ae7009253e179fa7159db4), "hack","Nudge Nudge Wink Wink (Barcrest) (MPU4, Mod2 type) (NNU 3.4, hack, set 2)" )


ROM_START( m4holywd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hollywood 5p.bin", 0x0000, 0x010000, CRC(fb4ebb6e) SHA1(1ccfa81c173011ce70640097c85b532fd44f5a6e) )
ROM_END

// non-standard protection, makes some invalid writes
GAME(199?, m4holywd,  0,          mod2_bootleg_fixedret<0xd0>(R4, RT1),       mpu4,    mpu4mod2_machines_state, init_m4, ROT0,   "bootleg?","Hollywood (bootleg) (MPU4) (HOL 1.0)",GAME_FLAGS )

/*********************************************************************************************************

   High Rise

*********************************************************************************************************/

ROM_START( m4hirise )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hiix.p1", 0x0000, 0x010000, CRC(c68c816c) SHA1(2ec89d83f3b658700433fc165358290ce58eba64) )
ROM_END

ROM_START( m4hirisea )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hirs.p1", 0x0000, 0x010000, CRC(a38f771e) SHA1(c1502200671389a1fe6dcb9c043d22583d5991dc) )
ROM_END

ROM_START( m4hiriseb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hirs20dd", 0x0000, 0x010000, CRC(89941670) SHA1(28859adfa79dce53c348c63b46f6f5a068f2b2de) )
ROM_END

ROM_START( m4hirisec )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hirx.p1", 0x0000, 0x010000, CRC(4280a16b) SHA1(c9179ec17404a6f084679ad5f04e53a50f00af98) )
ROM_END

ROM_START( m4hirised )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hirxc.p1", 0x0000, 0x010000, CRC(1ad1d942) SHA1(91d02212606e22b280be9640433e013bc50e5ea8) )
ROM_END

ROM_START( m4hirisee )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hrise206", 0x0000, 0x010000, CRC(58b4bbdd) SHA1(0b76d27147fbadba97328eb9d2dc81cff9d576e0) )
ROM_END

GAME(199?, m4hirise,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::hirise_characteriser_prot>(R5, RT2),   mpu4,    mpu4mod2_machines_state, init_m4, ROT0,   "Barcrest","High Rise (Barcrest) (MPU4) (HII 0.3)",GAME_FLAGS )
GAME(199?, m4hirisea, m4hirise,   mod2_cheatchr_pal<mpu4_characteriser_pal::hirise_characteriser_prot>(R5, RT2),   mpu4,    mpu4mod2_machines_state, init_m4, ROT0,   "Barcrest","High Rise (Barcrest) (MPU4) (HIR 3.1, set 1)",GAME_FLAGS )
GAME(199?, m4hirisec, m4hirise,   mod2_cheatchr_pal<mpu4_characteriser_pal::hirise_characteriser_prot>(R5, RT2),   mpu4,    mpu4mod2_machines_state, init_m4, ROT0,   "Barcrest","High Rise (Barcrest) (MPU4) (HIR 3.1, set 2)",GAME_FLAGS )
GAME(199?, m4hirised, m4hirise,   mod2_cheatchr_pal<mpu4_characteriser_pal::hirise_characteriser_prot>(R5, RT2),   mpu4,    mpu4mod2_machines_state, init_m4, ROT0,   "Barcrest","High Rise (Barcrest) (MPU4) (HIR 3.1, set 3)",GAME_FLAGS )
GAME(199?, m4hirisee, m4hirise,   mod2_cheatchr_pal<mpu4_characteriser_pal::hirise_characteriser_prot>(R5, RT2),   mpu4,    mpu4mod2_machines_state, init_m4, ROT0,   "Barcrest","High Rise (Barcrest) (MPU4) (HIR 3.0)",GAME_FLAGS )
// non-standard protection
GAME(199?, m4hiriseb, m4hirise,   mod2_bootleg_fixedret<0x88>(R5, RT2),   mpu4,    mpu4mod2_machines_state, init_m4, ROT0,   "bootleg?","High Rise (Barcrest) (MPU4) (HIR 1.5, bootleg?)",GAME_FLAGS )


/*********************************************************************************************************

    unknown 'RED'

*********************************************************************************************************/

#undef GAME_CUSTOM
#define GAME_CUSTOM(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAME(year, setname, parent, mod2_bootleg_fixedret<0x13>(R4, RT1), mpu4, mpu4mod2_machines_state, init_m4, ROT0, company, title, GAME_FLAGS )


// no copyright string in header, although 1988 BARCREST string exists elsewhere
// RED 0.4, different protection?, hack, might be a different game, has touchscreen related strings?
GAME_CUSTOM( 199?, m4redunk,    0,   "redx_20_.8",   0x0000, 0x010000, CRC(b5e8dec5) SHA1(74777ed7f78ef7cc615beadf097380569832a75a), "bootleg","unknown Barcrest MPU4 'RED 0.4' (MPU4) (bootleg)" )

/*********************************************************************************************************

    Super Play
     - set was originally called "Black Bull" and as 'Czech' region
     - dipswitches probably all different (has coinage dip?)

*********************************************************************************************************/

ROM_START( m4blkbul )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cbb08.epr", 0x0000, 0x010000, CRC(09376df6) SHA1(ba3b101accb6bbfbf75b9d22621dbda4efcb7769) )
ROM_END

INPUT_PORTS_START( m4blkbul )
	PORT_INCLUDE( mpu4 )

	PORT_MODIFY("DIL1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL1:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL1:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL1:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x08, "Coin Value?" ) PORT_DIPLOCATION("DIL1:04") // needed to boot
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL1:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL1:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL1:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL1:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )
INPUT_PORTS_END

GAME(199?, m4blkbul,  0,          mod2_cheatchr_pal<mpu4_characteriser_pal::alf_characteriser_prot>(R4, RT2),   m4blkbul,    mpu4mod2_machines_state, init_m4, ROT0,   "Barcrest","Super Play (Czech) (Barcrest) (MPU4) (XSP 0.8)",GAME_FLAGS )


/*********************************************************************************************************

    Top Run
     - currently runs with door open, probably not by Barcrest?

*********************************************************************************************************/

ROM_START( m4toprn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "toprun_v1_1.bin", 0xc000, 0x004000, CRC(9b924324) SHA1(7b155467f30cc22f7cda301ae770fb2a889c9c66) )
	ROM_LOAD( "toprun_v1_2.bin", 0x8000, 0x004000, CRC(940fafa9) SHA1(2a8b669c51c8df50710bd8b552ab30a5d1a136ab) )
ROM_END

GAME(199?, m4toprn,   0,          mod2(R4, RT1),       mpu4,    mpu4mod2_machines_state, init_m4, ROT0,   "<unknown>","Top Run (Dutch) (MPU4)",GAME_FLAGS ) // unique behavior  (START UP IN countdown)

/*****************************************************************************************************************************************************************************
*
* Four More
* - does not appear to be a Barcrest codebase, and game seems to be unprotected
* - requires very specific AY handling to produce sounds, see note in mpu4.cpp
*
*****************************************************************************************************************************************************************************/

ROM_START( m4fourmr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "frmr5p26.bin", 0x8000, 0x008000, CRC(f0c5bd8a) SHA1(39026459008ed5b5bd3a10841799227fef70e5b5) )
ROM_END

// runs, unprotected, 17 May 1990 BWBNFM26 in ROM
GAME(1990, m4fourmr,  0,          mod2(R4, RT1),       mpu4,    mpu4mod2_machines_state, init_m4, ROT0,   "BWB","Four More (BWB) (MPU4)",GAME_FLAGS ) // no sound with either system?

/*****************************************************************************************************************************************************************************
*
* Line Up
* Speculator Club
* - does not appear to be a Barcrest codebase, and game seems to be unprotected
* - requires very specific AY handling to produce sounds, see note in mpu4.cpp
* - reels shake continuously, does it need a different reel type?
*
*****************************************************************************************************************************************************************************/

ROM_START( m4lineup )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lineup5p1.bin", 0xc000, 0x004000, CRC(9ba9edbd) SHA1(385e01816b5631b6896e85343ae96b3c36f9647a) )
	ROM_LOAD( "lineup5p2.bin", 0x8000, 0x004000, CRC(e9e4dfb0) SHA1(46a0efa84770036366c7a6a33ef1d42c7b2b782b) )
	ROM_LOAD( "lineup5p3.bin", 0x6000, 0x002000, CRC(86623376) SHA1(e29442bfcd401361287852b87673368322e946b5) )
ROM_END

ROM_START( m4lineupa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lu2_10p1.bin", 0xc000, 0x004000, CRC(2fb89062) SHA1(55e86de8fd0d36cca9aab8ad5aae7b4f5a62b940) )
	ROM_LOAD( "lu2_10p2.bin", 0x8000, 0x004000, CRC(9d820af2) SHA1(63d27df91f80e47eb8c9685fcd2c3eff902a2ef8) )
	ROM_LOAD( "lu2_10p3.bin", 0x6000, 0x002000, CRC(8c8a210c) SHA1(2599d979f1a62e9ef6acc70d0ad5c9b4a65d712a) )
ROM_END

ROM_START( m4specu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "speculator.bin", 0x8000, 0x008000, CRC(4035d20c) SHA1(4a534294c5c7332eacd09ca44f351d6a6850cc29) )
ROM_END

GAME(199?, m4lineup,  0,          mod2(R4, RT1),       mpu4,    mpu4mod2_machines_state, init_m4, ROT0,   "BWB","Line Up (BWB) (MPU4) (set 1)",GAME_FLAGS )
GAME(199?, m4lineupa, m4lineup,   mod2(R4, RT1),       mpu4,    mpu4mod2_machines_state, init_m4, ROT0,   "BWB","Line Up (BWB) (MPU4) (set 2)",GAME_FLAGS )

// very similar game to above
GAME(199?, m4specu,   0,          mod2(R4, RT1),       mpu4,    mpu4mod2_machines_state, init_m4, ROT0,   "BWB","Speculator Club (BWB) (MPU4)",GAME_FLAGS )

/*********************************************************************************************************

    Sets below have different protection, are they bootlegs?

    Some sources credit a game called Blast-a-Bank to Carfield, is this it?

    Apart from the protection they do appear to be build off a Barcrest codebase

*********************************************************************************************************/

// different protection, call/response check with 6 possible values
GAME(199?, m4blstbk,  0,          mod2_chr_blastbnk(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "bootleg?","Blast A Bank (MPU4) (BB 1.0)",GAME_FLAGS )

// different protection, call/response check with 6 possible values
GAME(199?, m4copcsh,  0,          mod2_chr_copcash(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "bootleg","Coppa Cash (MPU4) (FC 2.0)",GAME_FLAGS )

/*********************************************************************************************************

    Test programs for base (mod2) hardware

*********************************************************************************************************/

ROM_START( m4tst )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "ut4.p1",  0xC000, 0x4000,  CRC(086dc325) SHA1(923caeb61347ac9d3e6bcec45998ddf04b2c8ffd))
ROM_END

ROM_START( m4tst2 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "ut2.p1",  0xe000, 0x2000,  CRC(f7fb6575) SHA1(f7961cbd0801b9561d8cd2d23081043d733e1902))
ROM_END

ROM_START( m4clr )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "meter-zero.p1",  0x8000, 0x8000,  CRC(e74297e5) SHA1(49a2cc85eda14199975ec37a794b685c839d3ab9))
ROM_END

ROM_START( m4rltst )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rtv.p1", 0x08000, 0x08000, CRC(7b78f3f2) SHA1(07ef8e6a08fd70ee48e4463672a1230ecc669532) )
ROM_END

GAME( 198?, m4tst,    0,          mod2_no_bacta(R4, RT1, OVER),       mpu4,    mpu4mod2_machines_state, init_m4,  ROT0,   "Barcrest","MPU4 Unit Test (Program 4)",MACHINE_MECHANICAL )
GAME( 198?, m4tst2,   0,          mod2_no_bacta(R4, RT1, OVER),       mpu4,    mpu4mod2_machines_state, init_m4,  ROT0,   "Barcrest","MPU4 Unit Test (Program 2)",MACHINE_MECHANICAL )
GAME( 198?, m4clr,    0,          mod2_no_bacta(R4, RT1, OVER),       mpu4,    mpu4mod2_machines_state, init_m4,  ROT0,   "Barcrest","MPU4 Meter Clear ROM",MACHINE_MECHANICAL )
GAME( 198?, m4rltst,  0,          mod2_no_bacta(R4, RT1, OVER),       mpu4,    mpu4mod2_machines_state, init_m4,  ROT0,   "Barcrest","MPU4 Reel Test (3.0)",MACHINE_MECHANICAL )


/*********************************************************************************************************

    Sets below are NOT based on Barcrest code, and don't have Characteriser protection etc.

*********************************************************************************************************/

/* Pcp */

// runs and plays, but no lamps or other display..
GAME(199?, m4cshino,  0,          mod2(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Pcp","Cashino Deluxe (Pcp) (MPU4)",GAME_FLAGS )

// runs and plays, but no lamps or other display..
GAME(199?, m4jjc,     0,          mod2(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Pcp","Jumping Jack Cash (Pcp) (MPU4) (set 1)",GAME_FLAGS )
GAME(199?, m4jjca,    m4jjc,      mod2(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Pcp","Jumping Jack Cash (Pcp) (MPU4) (set 2)",GAME_FLAGS )

// runs and plays, but no lamps or other display..
GAME(199?, m4spton,   0,          mod2(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Pcp","Spot On (Pcp) (MPU4)",GAME_FLAGS )

GAME(199?, m4exlin,   0,          mod2(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Pcp","Extra Lines (Pcp) (MPU4) (set 1)",GAME_FLAGS )
GAME(199?, m4exlina,  m4exlin,    mod2(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Pcp","Extra Lines (Pcp) (MPU4) (set 2)",GAME_FLAGS )

GAME(199?, m4supjst,  0,          mod2(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Pcp","Super Jester (Pcp) (MPU4) (set 1)",GAME_FLAGS )
GAME(199?, m4supjsta, m4supjst,   mod2(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Pcp","Super Jester (Pcp) (MPU4) (set 2)",GAME_FLAGS )
GAME(199?, m4supjstb, m4supjst,   mod2(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Pcp","Super Jester (Pcp) (MPU4) (set 3)",GAME_FLAGS )
GAME(199?, m4supjstc, m4supjst,   mod2(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Pcp","Super Jester (Pcp) (MPU4) (set 4)",GAME_FLAGS )
GAME(199?, m4supjstd, m4supjst,   mod2(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Pcp","Super Jester (Pcp) (MPU4) (set 5)",GAME_FLAGS )
GAME(199?, m4supjste, m4supjst,   mod2(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Pcp","Super Jester (Pcp) (MPU4) (set 6)",GAME_FLAGS )

GAME(199?, m4frcrak,  0,          mod2(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Pcp","Fruit Cracker (Pcp) (MPU4)",GAME_FLAGS )

GAME(199?, m4clbrpl,  0,          mod2(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Pcp","Club Replay (PCP) (MPU4) (01)",GAME_FLAGS )

/* Misc */

GAME(199?, m4spnwin,  0,          mod2(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Cotswold Microsystems","Spin A Win (Cotswold Microsystems) (MPU4) (set 1)",GAME_FLAGS ) // works?
GAME(199?, m4spnwina, m4spnwin,   mod2(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "Cotswold Microsystems","Spin A Win (Cotswold Microsystems) (MPU4) (set 2)",GAME_FLAGS )

// was marked as JPM, but that doesn't seem likely
GAME(199?, m4pick,    0,          mod2(R4, RT1), mpu4,            mpu4mod2_machines_state, init_m4,     ROT0,   "<unknown>","Pick A Fruit (Dutch) (MPU4)",GAME_FLAGS )

// No reels
GAMEL(1989?, m4conn4, 0,          mod2(OVER), connect4,        mpu4mod2_machines_state, init_m4,      ROT0, "Dolbeck Systems","Connect 4",MACHINE_IMPERFECT_GRAPHICS|MACHINE_REQUIRES_ARTWORK,layout_connect4 )

// TUNE ALARM (may or may not be sound-related); probably needs RS-232 link
GAME(198?, m4funh,    0,          mod4psg(R4, RT1), mpu4,         mpu4mod2_machines_state, init_m4, 0,      "<unknown>",      "Fun House (unknown) (MPU4)", GAME_FLAGS ) // was in the SC1 Fun House set
