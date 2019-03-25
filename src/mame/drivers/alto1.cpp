// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

2017-11-05 Skeleton

Xerox Alto I

Similar in architecture to the Alto II and the Lilith, with 74181 ALUs and unique CPU, this was the first computer with a GUI.


************************************************************************************************************************************/

#include "emu.h"

class alto1_state : public driver_device
{
public:
	alto1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		//      , m_maincpu(*this, "maincpu")
	{ }

	void alto1(machine_config &config);
private:
	//  required_device<cpu_device> m_maincpu;
};


static INPUT_PORTS_START( alto1 )
INPUT_PORTS_END

MACHINE_CONFIG_START(alto1_state::alto1)
MACHINE_CONFIG_END

ROM_START( alto1 )
	ROM_REGION( 0x4000, "roms", 0 )
	ROM_LOAD( "00_23.bin",    0x0000, 0x0100, CRC(56cc822f) SHA1(b337ff7abf5da92008c72cf9585faadf552ede32) )
	ROM_LOAD( "01_23.bin",    0x0100, 0x0100, CRC(37ea8320) SHA1(85a60869f49b73cbf80392566253aa86a3b5767c) )
	ROM_LOAD( "02_23.bin",    0x0200, 0x0100, CRC(c8e5966d) SHA1(d519d9b3e08f5b0032a1b3487a4ee389764895b5) )
	ROM_LOAD( "03_23.bin",    0x0300, 0x0100, CRC(1d894299) SHA1(a7cd3071caa22d5efbd9b3a2368e1d131b66481b) )
	ROM_LOAD( "04_23.bin",    0x0400, 0x0100, CRC(e0870437) SHA1(075129edaa4992bd61b530cdc0b8ce8b363e5144) )
	ROM_LOAD( "05_23.bin",    0x0500, 0x0100, CRC(734f2bfc) SHA1(31a05a01ba635f2c58ae88acf54efc86f78fc621) )
	ROM_LOAD( "06_23.bin",    0x0600, 0x0100, CRC(2ee8f9ee) SHA1(5a5c0d464a268954ad458b905c0170ca95a2c2ac) )
	ROM_LOAD( "07_23.bin",    0x0700, 0x0100, CRC(c4f64e7a) SHA1(837333b8b2d9c8ce24571914f5e38c2b3491e7ce) )
	ROM_LOAD( "10_23.bin",    0x0800, 0x0100, CRC(ddc71e20) SHA1(813d503627e4d65b44af2f6038db30a81ff2d99f) )
	ROM_LOAD( "11_23.bin",    0x0900, 0x0100, CRC(cc5cca76) SHA1(6beac0676d11411325bb61d0be2e842a3b4d8dd0) )
	ROM_LOAD( "12_23.bin",    0x0a00, 0x0100, CRC(8d4fa6c0) SHA1(9c1d5f661c34678e65e09a2b1961818a98098c47) )
	ROM_LOAD( "13_23.bin",    0x0b00, 0x0100, CRC(f73d5e3b) SHA1(e4b2a33e6b8a54133d298e3d8e4856f85c84764c) )
	ROM_LOAD( "14_23.bin",    0x0c00, 0x0100, CRC(8eee7d4a) SHA1(c7917f096b8e56a5393c1901ecb0d5248e5592e1) )
	ROM_LOAD( "15_23.bin",    0x0d00, 0x0100, CRC(b648751c) SHA1(bb192751521e2d9948501cc3a68d92a15cf5eae8) )
	ROM_LOAD( "16_23.bin",    0x0e00, 0x0100, CRC(011a309e) SHA1(2914519695fc65a09f797203881c42d2e5abc782) )
	ROM_LOAD( "17_23.bin",    0x0f00, 0x0100, CRC(6ccb4acb) SHA1(c307ee5bb5c72a37bab389eb7668f9367811ec2d) )
	ROM_LOAD( "20_23.bin",    0x1000, 0x0100, CRC(00846711) SHA1(330a4cfd1d15b31cf361db72fc44172887af7db5) )
	ROM_LOAD( "21_23.bin",    0x1100, 0x0100, CRC(f08f993b) SHA1(803a872c192a081423f2a65364afd8d0531dfa64) )
	ROM_LOAD( "22_23.bin",    0x1200, 0x0100, CRC(0a15cca5) SHA1(c1ee6f77dfe830cb470a6929ab74d1dce52f4129) )
	ROM_LOAD( "23_23.bin",    0x1300, 0x0100, CRC(197adfe9) SHA1(18b37eb95d7300399adb01f003cbea611a4b48ad) )
	ROM_LOAD( "24_23.bin",    0x1400, 0x0100, CRC(4fe970f0) SHA1(d9791fc60a11d00989cf3b167530593461edb659) )
	ROM_LOAD( "25_23.bin",    0x1500, 0x0100, CRC(b2638af5) SHA1(7b789731af5b390ff51de113149f1af47ed1f4a2) )
	ROM_LOAD( "26_23.bin",    0x1600, 0x0100, CRC(3920d097) SHA1(3a2717368f1a0afb021789165c5c032dce2369dc) )
	ROM_LOAD( "27_23.bin",    0x1700, 0x0100, CRC(cb425e98) SHA1(69573461633a702c284df7a2e931588c289c8529) )
	ROM_LOAD( "30_23.bin",    0x1800, 0x0100, CRC(95a4763a) SHA1(1a28b04d51b9125f92f40f914bf43957383a1edc) )
	ROM_LOAD( "31_23.bin",    0x1900, 0x0100, CRC(0cb672a1) SHA1(20a1cf8295fbe7884fbb1dd26c61f122a43e030f) )
	ROM_LOAD( "32_23.bin",    0x1a00, 0x0100, CRC(46f51eb0) SHA1(5b4a0f94a9c133da0f734ae44b09e0f8bab86058) )
	ROM_LOAD( "33_23.bin",    0x1b00, 0x0100, CRC(491db151) SHA1(1c1a816609abcafe00c80731148140ff68ecac58) )
	ROM_LOAD( "34_23.bin",    0x1c00, 0x0100, CRC(ce6f062f) SHA1(fe8751aa3884c8d2cefc28fb116336912264b962) )
	ROM_LOAD( "35_23.bin",    0x1d00, 0x0100, CRC(d7f5eb01) SHA1(300f42e251eaa2a4ad37e17376c46bd272018d92) )
	ROM_LOAD( "36_23.bin",    0x1e00, 0x0100, CRC(d4b335f5) SHA1(ed4ed14ab8e8f50cc07ef8d394490ce168bd47be) )
	ROM_LOAD( "37_23.bin",    0x1f00, 0x0100, CRC(320eaae9) SHA1(44bd5bd1e4de054c6feb52dbc0dc4e848bab085e) )
	ROM_LOAD( "c0_23.bin",    0x2000, 0x0100, CRC(ef5634be) SHA1(912dc906069dfbed1c1a846bbe50ef1ed6dfe081) )
	ROM_LOAD( "c1_23.bin",    0x2100, 0x0100, CRC(b698f779) SHA1(ef33c9aefbf3e650bea33d42179a9c9743dfbe4d) )
	ROM_LOAD( "c2_23.bin",    0x2200, 0x0100, CRC(4dfc72e8) SHA1(56ac873d13cc1f351537145a586609a34f79d6af) )
	ROM_LOAD( "c3_23.bin",    0x2300, 0x0100, CRC(cd0e855b) SHA1(75d1bad4c5bded95703be358834a969aa2a56942) )
	ROM_LOAD( "ctr.bin",      0x2400, 0x0020, CRC(fc51b1d1) SHA1(e36c2a12a5da377394264899b5ae504e2ffda46e) )
	ROM_LOAD( "dp14.bin",     0x2500, 0x0100, CRC(8f54203b) SHA1(158d6c8b54d5cc0de7a902001a9ee207ed3358a0) )
	ROM_LOAD( "pram.bin",     0x2600, 0x0100, CRC(8087140e) SHA1(e17d9756150d41d6ff614afa86808a9c77516749) )
ROM_END

COMP( 1973, alto1, 0, 0, alto1, alto1, alto1_state, empty_init, "Xerox", "Alto I", MACHINE_IS_SKELETON )
