// license:BSD-3-Clause
// copyright-holders:stonedDiscord
/*

Bally Wulff Eurotec
German Fruit Machines / Gambling Machines

The predecessor is the Technik 2000 system.

The CPU board has the CPU, RTC, RAM and ROM

CPU: Motorola MC68EC000
RAM: Samsung K6T4016C3C 256Kx16 bit Low Power CMOS Static RAM
RTC: Epson RTC72421

 _________________________________
|                                 |
| ||  74HC14    XTAL  74HC02 LM311|
| ||  74HC32 74HC08       BATT    |
| || 74HC137  _______    RTC72421 |
| ||         |68EC000|            |
| || 74HC245 |       |      RAM   |
| || 74HC245 |_______|            |
| ||     IC15            IC10     |
|_________________________________|

The right half is usually covered by a plastic shield and locked with a lead seal.
It sits on a main board that has a PLCC84 labeled

ATMEL/TEMIC MBZ
ULC 1240 V2.0
5/98
Bally Wulff
YYWW

which seems to be an Ultimate Logic Conversion from an Actel FPGA

older boards are labelled
002.600.002       002.600.11B2

newer with OKI sound
0B01.0600.1100B4

TODO:
the ULC/FPGA contains some kind of protection and handles most IO
DS1985 serial number chip

*/


#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "machine/msm6242.h"
#include "machine/nvram.h"
#include "sound/okim6376.h"

#include "speaker.h"

namespace {

class ballyw_state : public driver_device
{
public:
	ballyw_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_rtc(*this, "rtc")
	{ }

	void b2(machine_config &config) ATTR_COLD;
	void b4(machine_config &config) ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<rtc72421_device> m_rtc;
};

void ballyw_state::mem_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x17ffff).ram().share("nvram");
}

static INPUT_PORTS_START( ballyw )
	PORT_START("IN0")
INPUT_PORTS_END


void ballyw_state::b2(machine_config &config)
{
	M68000(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &ballyw_state::mem_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // battery backed

	RTC72421(config, "rtc", XTAL(32'768)); // internal oscillator

	SPEAKER(config, "mono").front_center();
}

void ballyw_state::b4(machine_config &config)
{
	b2(config);

	OKIM6376(config, "snd", 4_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 1.0);
}

ROM_START( action4u )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("action4you_27c4001.ic10", 0x00000, 0x80000, CRC(eec1918a) SHA1(717222b618a1d7870fc2ade013148d25b35949ed))
	ROM_LOAD16_BYTE("action4you_27c4001.ic15", 0x00001, 0x80000, CRC(bf5fb08d) SHA1(6485a45a69a0d6e20d672da164179fd85f19cce2))
ROM_END

ROM_START( actnts7 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("action_total_s7.0_ic10_27c4001.ic10", 0x00000, 0x80000, CRC(58546c66) SHA1(d459507ffeec582b182bf7bbe584e5232e1bf018))
	ROM_LOAD16_BYTE("action_total_s7.0_ic15_27c4001.ic15", 0x00001, 0x80000, CRC(aef92f1f) SHA1(34cdf09460a5737132ecd2907e234a5e95837d0e))
ROM_END

ROM_START( arcon )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("arcon_s2_ic10_27c4001.ic10", 0x00000, 0x80000, CRC(c7643bea) SHA1(cb079dadbb270f36dc33f0c633696b9a9c5a4595))
	ROM_LOAD16_BYTE("arcon_s2_ic15_27c4001.ic15", 0x00001, 0x80000, CRC(15746d9b) SHA1(34868120dcd504d8703406173585ba492f505929))
ROM_END

ROM_START( atlntis6 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("atlantis_s6_27c4001.ic10", 0x00000, 0x80000, CRC(ee63713e) SHA1(0d109a01e7764c70c6a70df63a2a65187a49ff1d))
	ROM_LOAD16_BYTE("atlantis_s6_27c4001.ic15", 0x00001, 0x80000, CRC(3f56d229) SHA1(103c5f7284f28ba4ff87366ba8a90de3be1805a1))
ROM_END

ROM_START( b18plus )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("27c4001.ic10", 0x00000, 0x80000, CRC(1cce461c) SHA1(699007425b4914579059229e9522f232384f1f0f))
	ROM_LOAD16_BYTE("27c4001.ic15", 0x00001, 0x80000, CRC(e908dc14) SHA1(530085c138662ad3657c4249b1d4e08488d9a932))
ROM_END

ROM_START( b493 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("m27c4001.ic10", 0x00000, 0x80000, CRC(56a4e306) SHA1(3737eadda26481af26907561e6a6bf641a1a2bbf))
	ROM_LOAD16_BYTE("m27c4001.ic15", 0x00001, 0x80000, CRC(a65dab78) SHA1(aa2658d11602ee7ff5465da3136f8184dba7ffb5))

	ROM_REGION( 0x100000, "snd", 0 )
	ROM_LOAD("sound_493_m27c801.ic13", 0x00000, 0x100000, CRC(3a403f3f) SHA1(47beae5efe486cb7f74ff786e33e8cd4543bf61d))
ROM_END

ROM_START( bbust )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("blockbuster_27c4001.ic10", 0x00000, 0x80000, CRC(932d1dc6) SHA1(c4edf979e3b54cffc7fc9be94986b784f4cef56f))
	ROM_LOAD16_BYTE("blockbuster_27c4001.ic15", 0x00001, 0x80000, CRC(73def7ac) SHA1(329504586ab70d8cf4aeeaa7c70b5ba2a3725b2f))
ROM_END

ROM_START( belamis3 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("bel_ami_s3_even_27c4001.ic10", 0x00000, 0x80000, CRC(ea2b4fde) SHA1(a797f6da4d9e6b88a88eb13a163d7df09b83c015))
	ROM_LOAD16_BYTE("bel_ami_s3_odd_27c4001.ic15", 0x00001, 0x80000, CRC(1951de6e) SHA1(7d280a42bd5922cbe5a4c75fd34e9e9d0f2bc96e))
ROM_END

ROM_START( bheroe )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("27c4001.ic10", 0x00001, 0x80000, CRC(3d705c19) SHA1(e0a26c75d374c055b82cc0f67792b334fc2478f5))
	ROM_LOAD16_BYTE("27c4001.ic15", 0x00000, 0x80000, CRC(16866ed8) SHA1(d278d6c3fc0838317b161e13ed063e6efda5a8c2))
ROM_END

ROM_START( bjagas17 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("babajaga_1032_s17.0_even_bw1032.ic10", 0x00000, 0x80000, CRC(c12198e6) SHA1(425b9d837e571c42e6f952e1019bee0e4ef9fe99))
	ROM_LOAD16_BYTE("babajaga_1032_s17.0_odd_bw1032.ic15", 0x00001, 0x80000, CRC(11112a7f) SHA1(846be0091ae8d29281a4e96038e19483ef7ec55e))
ROM_END

ROM_START( boss )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("164-s4.0_even_boss_27c4001.ic10", 0x00000, 0x80000, CRC(051c1bcc) SHA1(e6175dccf8f3f83c95fee528b0b27c931b29fd4d))
	ROM_LOAD16_BYTE("164-s4.0_odd_boss_27c4001.ic15", 0x00001, 0x80000, CRC(cfc0f736) SHA1(12900f8d62f4692ee309ed109e2c02a31288338f))
ROM_END

ROM_START( bullys11 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("bully_s11_27c4001.ic10", 0x00000, 0x80000, CRC(dc3a9da8) SHA1(28d938008e82a708a40785ec96ecefaf44def944))
	ROM_LOAD16_BYTE("bully_s11_27c4001.ic15", 0x00001, 0x80000, CRC(818fb3df) SHA1(269243196fa0e7cf06780a50dd599844981701e5))
ROM_END

ROM_START( caipis7 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("caipi_s7.0_27c4001.ic10", 0x00000, 0x80000, CRC(35a00e1f) SHA1(0604da1c31daf3580fd28e15e753a384da0bbaf3))
	ROM_LOAD16_BYTE("caipi_s7.0_27c4001.ic15", 0x00001, 0x80000, CRC(e4e36a35) SHA1(8d9a58be2c378408851a78623b3e8f13d79408f4))
ROM_END

ROM_START( daxxs4 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("daxx_s4_even_27c4001.ic10", 0x00000, 0x80000, CRC(dd8f803c) SHA1(a134a3cfb89c4211444957bf2bb8a6e0bda22d61))
	ROM_LOAD16_BYTE("daxx_s4_odd_27c4001.ic15", 0x00001, 0x80000, CRC(b0ade2f5) SHA1(cab6304db257c7728551c58c846dc39cb3ef308e))
ROM_END

ROM_START( daxxs8 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("daxx_s8_27c4001.ic10", 0x00000, 0x80000, CRC(9424a55f) SHA1(809aa426ce0ffd2c302e0e705b0509af8ede9f37))
	ROM_LOAD16_BYTE("daxx_s8_27c4001.ic15", 0x00001, 0x80000, CRC(6627566c) SHA1(775ff0bc3286333a45d3cf7e15b1f1e4ce79b840))
ROM_END

ROM_START( duo )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("27c4001.ic10", 0x00000, 0x80000, CRC(9bb2c2ea) SHA1(a464c5d0352c610d7774c84995dfca0308a58791))
	ROM_LOAD16_BYTE("27c4001.ic15", 0x00001, 0x80000, CRC(dda0a229) SHA1(4fd3102e6054ee6c477635107360b1452eafbcc6))
ROM_END

ROM_START( elevens3 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("27c4001_s3.ic10", 0x00000, 0x80000, CRC(d307d2eb) SHA1(de411cc31934a5a14a14432bce8618116aaa8ea8))
	ROM_LOAD16_BYTE("27c4001_s3.ic15", 0x00001, 0x80000, CRC(730e88ce) SHA1(9a818c6b19797ff15c4f05471a3773f0b5c6687c))
ROM_END

ROM_START( endsp9 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("27c4001_s9.ic10", 0x00000, 0x80000, CRC(f50e3e6b) SHA1(cbb4acf3190c50941a85e380dbbf1fdf4e650175))
	ROM_LOAD16_BYTE("27c4001_s9.ic15", 0x00001, 0x80000, CRC(e590ae7f) SHA1(80f0026aaa892168f10e32ddf60fae3ee1e2eec9))
ROM_END

ROM_START( fgames11 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("freegames_s11_27c4001.ic10", 0x00000, 0x80000, CRC(75ffd578) SHA1(45e3d6a7ef6ffe34c5b803017e9542afa9b5c2e8))
	ROM_LOAD16_BYTE("freegames_s11_27c4001.ic15", 0x00001, 0x80000, CRC(db1b094a) SHA1(efde8ee329cf4387b4244dcf5c27b285e0ef31bb))
ROM_END

ROM_START( figame3 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("firegame_s3_27c4001.ic10", 0x00000, 0x80000, CRC(e33e8c78) SHA1(4880b3e2873ecc9e9e56db0fdc2cdc880f9eda5a))
	ROM_LOAD16_BYTE("firegame_s3_27c4001.ic15", 0x00001, 0x80000, CRC(279ec5ef) SHA1(b26a26f5e42d098bd7ddd6746559de37c571d0f5))
ROM_END

ROM_START( foxxs5 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("foxx_s5_27c4001.ic10", 0x00000, 0x80000, CRC(3c9e41d0) SHA1(ef5a1b02f5415b768c3a16660a5a64838675d848))
	ROM_LOAD16_BYTE("foxx_s5_27c4001.ic15", 0x00001, 0x80000, CRC(238a4f29) SHA1(399596c9bb999f46ad85c32464386e3fca2a4097))
ROM_END

ROM_START( glriasl4 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("123-s4.0_even_gloria_sl.ic10", 0x00001, 0x80000, CRC(1be58a11) SHA1(1a1b1f51c9c3f4bd16832a689ee90a5f8faac453))
	ROM_LOAD16_BYTE("123-s4.0_odd_gloria_sl.ic15",  0x00000, 0x80000, CRC(06d7ee41) SHA1(8bf4b1ce16d9f021381b7dae5069ba8383b150cd))

	ROM_REGION( 0x100000, "snd", 0 )
	ROM_LOAD("123-sound_1.4_gloria_sl.ic13", 0x00000, 0x100000, CRC(71cfbd7e) SHA1(efe2e92cfb7de4b6145aa3462fda010282b31090))

	ROM_REGION( 0x800, "ident", 0 )
	ROM_LOAD("127401373.p15", 0x000, 0x800, CRC(215275b6) SHA1(297dd49d42122e2f5f131d610d6a00b42beee677))
ROM_END

ROM_START( glriasl5 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("gloriasl_s5_27c4001.ic10", 0x00001, 0x80000, CRC(a8371c7c) SHA1(da226a605ef438e6cf18273b3caed2713d4baf5c))
	ROM_LOAD16_BYTE("gloriasl_s5_27c4001.ic15", 0x00000, 0x80000, CRC(167e4772) SHA1(5aba5a12a12e098bce2bdd3b68461ce16960c112))
ROM_END

ROM_START( graffity )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("27c4001.ic10", 0x00001, 0x80000, CRC(62b92b1b) SHA1(5785f8020ec5ad6ea91b05bb1e717aedafaf4e5d))
	ROM_LOAD16_BYTE("27c4001.ic15", 0x00000, 0x80000, CRC(ae627d4d) SHA1(3a9a67cca7b71073d0415b63942645ed82b0c67a))
ROM_END

ROM_START( hardrock )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("hardrock_27c4001.ic10", 0x00001, 0x80000, CRC(d45fc17f) SHA1(339239e16c604dc0c63fee009ec5b880852d807b))
	ROM_LOAD16_BYTE("hardrock_27c4001.ic15", 0x00000, 0x80000, CRC(bbeb79b7) SHA1(1432e97ed92bb83c73c6885fc194604a21a392a5))
ROM_END

ROM_START( hslots5 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("hotslot_s5_27c4001.ic10", 0x00001, 0x80000, CRC(f4ae4610) SHA1(fa568abf4378231ed9fcaa4599ef737f1484cccf))
	ROM_LOAD16_BYTE("hotslot_s5_27c4001.ic15", 0x00000, 0x80000, CRC(7abb577b) SHA1(2e4af58e5890f2db7c93bba1f40412cfff1d2519))
ROM_END

ROM_START( hxntanz )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("27c4001.ic10", 0x00001, 0x80000, CRC(cd3d8bf1) SHA1(0c1505a0580e44cfd82f97a8e1a04b70344ffcb0))
	ROM_LOAD16_BYTE("27c4001.ic15", 0x00000, 0x80000, CRC(bd7050ed) SHA1(d787bb5670ba4f712868cae893d5ad10e3f59ae0))
ROM_END

ROM_START( jjacksl )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("m27c4001_jumpin_jack_even.ic10", 0x00001, 0x80000, CRC(587cf35b) SHA1(414ac09fb2b768d3caf67a1e08d0b251f9520abb))
	ROM_LOAD16_BYTE("m27c4001_jumpin_jack_odd.ic15", 0x00000, 0x80000, CRC(c510bd3e) SHA1(76e2c88c906feee56b060d4d1fcc936fdf1ba9dd))

	ROM_REGION( 0x100000, "snd", 0 )
	ROM_LOAD("m27c801_soundprogramm.ic13", 0x00000, 0x100000, CRC(726e94d8) SHA1(c02bceaa9f2316913da1f18ad0af0a101f7d77d3))
ROM_END

ROM_START( kingcs2 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("king_classicsl_s2_even_27c4001.ic10", 0x00001, 0x80000, CRC(45239437) SHA1(f1f7e6071f1ae3a504376c8ac2acdac40a7d33c4))
	ROM_LOAD16_BYTE("king_classicsl_s2_ood_27c4001.ic15", 0x00000, 0x80000, CRC(0ffff1e4) SHA1(01e4f953d1cb7a7bc68e09a0299fdd07f078d7f4))
ROM_END

ROM_START( kingcs3 )
	ROM_REGION( 0x100000, "maincpu",  0 )
	ROM_LOAD16_BYTE("king_classicsl_s3_27c4001.ic10", 0x00001, 0x80000, CRC(20889a44) SHA1(57c19c6d2ad9a4ec8599c10844e7f2093dc8d0c8))
	ROM_LOAD16_BYTE("king_classicsl_s3_27c4001.ic15", 0x00000, 0x80000, CRC(bc26dba5) SHA1(51051e9a0652d0bddea9ca82c0254810f8083134))
ROM_END

ROM_START( limbos3 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("limbo_s3_27c4001.ic10", 0x00001, 0x80000, CRC(d2ecfdc4) SHA1(85b685082a0afaf85d186d4c75d0a36ce14b7232))
	ROM_LOAD16_BYTE("limbo_s3_27c4001.ic15", 0x00000, 0x80000, CRC(2b05541a) SHA1(f0dee5cc36a05d0876a1c9885b340b7d27c1a26b))
ROM_END

ROM_START( luxxpro )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("m27c4001_odd.ic15", 0x00000, 0x80000, CRC(70772887) SHA1(a2806bd4c81d7784c8aba354652ee2359b31831a))
	ROM_LOAD16_BYTE("m27c4001_even.ic10", 0x00001, 0x80000, CRC(d2056fef) SHA1(d050a92dd071418fb414d3d633cc29dc56ca5b74))
ROM_END

ROM_START( m200s5 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("ic_10_133_s5_0_even.ic10", 0x00001, 0x80000, CRC(2e0f99bd) SHA1(866c7b629e60ab4d76afb005ec1974d5597b80f8))
	ROM_LOAD16_BYTE("ic_15_133_s5_0_odd.ic15", 0x00000, 0x80000, CRC(84f91001) SHA1(811fbb87a00038cce74a8c4e2e26c50d401550c9))

	ROM_REGION( 0x100000, "snd", 0 )
	ROM_LOAD("sound_27c801", 0x00000, 0x100000, CRC(dee2333a) SHA1(157a2ccf20368c7040106ef73e4588d389ca3afc))
ROM_END

ROM_START( machmehr )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("27c4001_s1.3.ic10", 0x00001, 0x80000, CRC(a25d69cf) SHA1(1cb9e1980eb575ddbdd6792b3c510bc2f2aec6d7))
	ROM_LOAD16_BYTE("27c4001_s1.3.ic15", 0x00000, 0x80000, CRC(b36fa7f1) SHA1(7eed1fe4ec103fa7b14f84b2d90dc931dd562dc7))
ROM_END

ROM_START( neons12 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("s12_even.ic10", 0x00001, 0x80000, CRC(b612ef43) SHA1(ae7a02c9652ce99ce6befccf9e34b35b0539929f))
	ROM_LOAD16_BYTE("s12_odd.ic15", 0x00000, 0x80000, CRC(7819fb1d) SHA1(b24c02537f02e8a4bf3b42444f9c8608599168f1))
ROM_END

ROM_START( neons13 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("neon_s13_27c4001.ic10", 0x00001, 0x80000, CRC(6d168910) SHA1(baa0f20fcf96c17f0cbe7a195c16b6259d22a3b7))
	ROM_LOAD16_BYTE("neon_s13_27c4001.ic15", 0x00000, 0x80000, CRC(af331cd1) SHA1(3ed41ea6f1d0b3c8881e25ddf5c8400007ac6894))
ROM_END

ROM_START( oxigens7 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("oxigen_s7_27c4001.ic10", 0x00001, 0x80000, CRC(aaf4f806) SHA1(e452fd301de1da11c0981a8bf846df27ce4b57b4))
	ROM_LOAD16_BYTE("oxigen_s7_27c4001.ic15", 0x00000, 0x80000, CRC(f5f0a5ed) SHA1(18c8613c3f40159f707ed65130bda5c32c87effc))
ROM_END

ROM_START( phntoms7 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("phantom_s7_27c4001.ic10", 0x00001, 0x80000, CRC(f7985b1c) SHA1(7f6ca09ac4bf7f2d1f1fd1ccc5f59b2d379cfd8c))
	ROM_LOAD16_BYTE("phantom_s7_27c4001.ic15", 0x00000, 0x80000, CRC(12956a62) SHA1(22675e73c1ab12c36546324d1db68a53c1fe53da))
ROM_END

ROM_START( pratngs4 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("piratengold_s4_27c4001.ic10", 0x00001, 0x80000, CRC(b1270a84) SHA1(9a19522f91ddd8316b06d5ad42ea02bcd5bf1b4d))
	ROM_LOAD16_BYTE("piratengold_s4_27c4001.ic15", 0x00000, 0x80000, CRC(aa6eca61) SHA1(f6cf3e9d35f211310077a774245134ec655deda6))
ROM_END

ROM_START( pratngs5 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("piratengold_s5_27c4001.ic10", 0x00001, 0x80000, CRC(806ef56c) SHA1(705b9e8d4e3759ca244c178fb11192015aa71471))
	ROM_LOAD16_BYTE("piratengold_s5_27c4001.ic15", 0x00000, 0x80000, CRC(83b347d5) SHA1(b8e93f794c91df771491d82f0ff8afc0434f54b0))
ROM_END

ROM_START( ptimes11 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("27c4001_s11.ic10", 0x00001, 0x80000, CRC(03260a89) SHA1(4e22db1ce95eba067f4f8e166fe9c7e2bf398b18))
	ROM_LOAD16_BYTE("27c4001_s11.ic15", 0x00000, 0x80000, CRC(315b5dfd) SHA1(4eea3795100a8475ef2da67e245847d77754986b))
ROM_END

ROM_START( qriscs4 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("quickrisc_s4_27c4001.ic10", 0x00001, 0x80000, CRC(788ae262) SHA1(a326c8c5847b7e00cedc615df955d55a5b901d28))
	ROM_LOAD16_BYTE("quickrisc_s4_27c4001.ic15", 0x00000, 0x80000, CRC(f6b7a256) SHA1(cb28e809234bf520be2f8cf4f78e53841ed9882c))
ROM_END

ROM_START( rexe )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("rex_27c4001.ic10", 0x00001, 0x80000, CRC(6c4f0770) SHA1(367784c724c12dd5a673cf8f44b9b44b0c5e2871))
	ROM_LOAD16_BYTE("rex_27c4001.ic15", 0x00000, 0x80000, CRC(4afca1e3) SHA1(97a7dc44702589cfd03978f318399b7e97af6831))
ROM_END

ROM_START( skys6 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("101-s6.0_even_sky.ic10", 0x00001, 0x80000, CRC(8bf3fd6d) SHA1(c95a3651e025e9d2c99c708e643fe3a2982a39ad))
	ROM_LOAD16_BYTE("101-s6.0_odd_sky.ic15",  0x00000, 0x80000, CRC(0ce4f9d7) SHA1(d4cea08466cf86de7c27ffdfead456f796e4a0af))
ROM_END

ROM_START( snfairx2 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("sunfairxl_s2_27c4001.ic10", 0x00001, 0x80000, CRC(18e594d5) SHA1(db23102668906759c87f597aea4871b43500a79e))
	ROM_LOAD16_BYTE("sunfairxl_s2_27c4001.ic15", 0x00000, 0x80000, CRC(e7f56087) SHA1(b9e73aa524aee671fa6426c93c49a309c7ab8a21))
ROM_END

ROM_START( spdruns3 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("speedrun_s3_27c4001.ic10", 0x00001, 0x80000, CRC(07099a7a) SHA1(2b6d451e0ef0513d2438b93fce4984c2e35bc5bf))
	ROM_LOAD16_BYTE("speedrun_s3_27c4001.ic15", 0x00000, 0x80000, CRC(e4bea1f1) SHA1(649fa9368c3cd22cfb9ea713daeb554c8764c550))
ROM_END

ROM_START( spdruns4 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("speedrun_s4_27c4001.ic10", 0x00001, 0x80000, CRC(7622784f) SHA1(dc1953c6f6ea2f4782ddd99903960f4c892b6f25))
	ROM_LOAD16_BYTE("speedrun_s4_27c4001.ic15", 0x00000, 0x80000, CRC(29a992f0) SHA1(165da87cafde7fc7ee3670c19119d868d118f5a6))
ROM_END

ROM_START( texmex )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("ic_10_183_s2_0_even.ic10", 0x00001, 0x80000, CRC(cc5c342a) SHA1(4ccd85772aebfea38a8e87a5f5cba62d5b47e2a3))
	ROM_LOAD16_BYTE("ic_15_183_s2_0_odd.ic15", 0x00000, 0x80000, CRC(c1f1cffc) SHA1(2c5c7c3bea8ecd6ec39afd5ca0c502491e019662))
ROM_END

ROM_START( tgnias11 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("totalgenial_s11_27c4001.ic10", 0x00001, 0x80000, CRC(986e1321) SHA1(58e677f22a443e3945dec77df6a4f1b725dcf89f))
	ROM_LOAD16_BYTE("totalgenial_s11_27c4001.ic15", 0x00000, 0x80000, CRC(852b9f86) SHA1(9839aeeaf7186c2edb84fe00a7808cad9d653325))
ROM_END

ROM_START( trpcals3 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("tropical_s3_27c4001.ic10", 0x00001, 0x80000, CRC(167086ad) SHA1(11e9127e4bbc02372e7381538d29d8988c0b12d2))
	ROM_LOAD16_BYTE("tropical_s3_27c4001.ic15", 0x00000, 0x80000, CRC(830b9627) SHA1(b84307191f814046a7ed3264d46c8488dad17166))
ROM_END

ROM_START( tscool )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("thatscool_27c4001.ic10", 0x00001, 0x80000, CRC(2aa5e560) SHA1(908222b5cff52bcad3a0123930aed78d0191b7c1))
	ROM_LOAD16_BYTE("thatscool_27c4001.ic15", 0x00000, 0x80000, CRC(254b3410) SHA1(386bacfa6ab9c63a75e3303b129a64c4e61e086b))
ROM_END

ROM_START( ttanpro )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("titanpro_27c4001.ic10", 0x00001, 0x80000, CRC(e4f860ff) SHA1(1909e65b58eb87cbe86d2bfa7273d9d5e86e2f21))
	ROM_LOAD16_BYTE("titanpro_27c4001.ic15", 0x00000, 0x80000, CRC(e3d8e02f) SHA1(84b0327fd8fbcf957d4716e24b0db875f9b2e204))
ROM_END

ROM_START( victoria )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("victoria_27c4001.ic10", 0x00001, 0x80000, CRC(0b9d551a) SHA1(4d91ebc04b3b9f75bfd3c071351be7347f3c2c62))
	ROM_LOAD16_BYTE("victoria_27c4001.ic15", 0x00000, 0x80000, CRC(637a6af9) SHA1(0d20c89eac895c85268aa0c7d76760df9788745c))
ROM_END

ROM_START( winplay )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("m27c4001_winplay.ic10", 0x00001, 0x80000, CRC(12e2f506) SHA1(5674937c13ef5ed20ec415e0d6ba12ea8ef5aa7b))
	ROM_LOAD16_BYTE("m27c4001_winplay.ic15", 0x00000, 0x80000, CRC(0f42b3b9) SHA1(95e03e0d32bbbd5d8afcf4a36e65c07d406ba725))
ROM_END

ROM_START( xcross )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("x_cross_27c4001.ic10", 0x00001, 0x80000, CRC(60a98551) SHA1(f127139986a8d5dbadc6caeeb04e5747bb005a49))
	ROM_LOAD16_BYTE("x_cross_27c4001.ic15", 0x00000, 0x80000, CRC(906134d8) SHA1(720df7aecbe8243a76eeb20ced1060bb12624c1e))
ROM_END

} // anonymous namespace

GAME(2001, limbos3,  0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Limbo (S3)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2002, skys6,    0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Sky (S6)",               MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2002, machmehr, 0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Mach Mehr",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2002, qriscs4,  0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Quick Risc (S4)",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2002, trpcals3, 0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Tropical (S3)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2003, action4u, 0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Action 4U SL",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2003, m200s5,   0, b4, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "M 200 (S5)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2003, jjacksl,  0, b4, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Jumpin Jack SL",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2003, winplay,  0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Winplay SL",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2003, belamis3, 0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Bel Ami (S3)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2003, figame3,  0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Fire Game (S3)",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2003, glriasl4, 0, b4, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Gloria SL (S4)",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2003, glriasl5, 0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Gloria SL (S5)",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2003, kingcs2,  0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "King Classic SL (S2)",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2003, kingcs3,  0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "King Classic SL (S3)",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2004, b493,     0, b4, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "493",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2004, actnts7,  0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Action Total (S7)",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2004, arcon,    0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Arcon",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2004, bheroe,   0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "HERO",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2004, rexe,     0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Rex",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2004, snfairx2, 0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Sunfair XL (S2)",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2004, tscool,   0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "That's Cool",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2005, boss,     0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Boss",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2005, bullys11, 0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Bully (S11)",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2005, elevens3, 0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Eleven (S3)",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2005, foxxs5,   0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Foxx (S5)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2005, luxxpro,  0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Luxx Pro",               MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2005, spdruns3, 0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Speed Run (S3)",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2005, spdruns4, 0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Speed Run (S4)",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2005, texmex,   0, b4, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Tex Mex",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2005, ttanpro,  0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Titan Pro",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2005, victoria, 0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Victoria",               MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2006, atlntis6, 0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Atlantis (S6)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2006, bbust,    0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Block Buster",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2006, caipis7,  0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Caipi (S7)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2006, daxxs4,   0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Daxx (S4)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2006, daxxs8,   0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Daxx (S8)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2006, duo,      0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Duo",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2006, endsp9,   0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Endspiel (S9)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2006, graffity, 0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Graffity",               MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2006, neons12,  0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Neon (S12)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2006, neons13,  0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Neon (S13)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2006, ptimes11, 0, b4, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Play Time (S11)",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2006, tgnias11, 0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Total Genial (S11)",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2007, bjagas17, 0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Baba Jaga (S17)",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2007, hardrock, 0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Hard Rock",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2007, hxntanz,  0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Hexen Tanz",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2007, hslots5,  0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Hot Slot (S5)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2007, oxigens7, 0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Oxigen (S7)",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2007, phntoms7, 0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Phantom (S7)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2007, pratngs4, 0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Piratengold (S4)",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2007, pratngs5, 0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Piratengold (S5)",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2007, xcross,   0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "X-Cross",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2008, b18plus,  0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "18+",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(2008, fgames11, 0, b2, ballyw, ballyw_state, empty_init, ROT0, "Bally Wulff", "Freegames (S11)",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
