// license:BSD-3-Clause
// copyright-holders:

/*
Skeleton driver for NSM/Löwen ST25 platform of gambling machines
Infos can be found at https://wiki.goldserie.de/index.php?title=Spiel_und_System_Modul_25

  NSM STE25.1 216575A/1012
  ___________________________________________________________________________________
 |                 __________                             XTAL       SERIAL         |
 | .              |TL7705ACP|           ______________  16.000 MHz                  |
 | .               __________          | NEC V25     |                ___________   |
 | .              |74HCT574 |          | D70322L-8   |                |74HC123N |   |
 | .               __________          |             |                ____________  |
 | .              |74HCT245N|          |             |                |V62C518256|  |
 | .                                   |_____________|                              |
 | .           ___________               __________   __________   _____________    |
 | .          |74HC32N   |              |74HC138N |  |74HC04N  |  | Spiel und   |   |
 | .           ___________   __________  __________   __________  | System      |   |
 |     RST    |74HC08N   |  |74HC368B1| |74AS138N |  |74HCT21N |  | Modul       |   |
 | .           ___________               __________   __________  | ROM Module  |   |
 | .          |74HC00N   |              |74HC4050N|  |74HC04N  |  | [SCC2592AC] |   |
 | .           ___________                                        |             |   |
 | .          |74HC32N   |                                XTAL    | [M27C4001 ] |   |
 | .                                                    3.686MHz  |_____________|   |
 |             +SERVICE+                                           __________       |
 |                                                                | OKI     |       |
 |                                                                | M6376   |       |
 |                                 TDA2005                        |__________       |
 | .                 VOL         __HEATSINK__                                       |
 | .                                                                                |
 |__________________________________________________________________________________|

 Rom Module
  ______________
 | [CONNECTOR]  |
 |              |
 |  TMS27C020   | // Program ROM IC2
 |              |
 |M48T18-150PC1 | // Timekeeper RAM IC3
 |              |
 |  TMS27C020   | // Sound ROM IC1
 |              |
 | [CONNECTOR]  |
 _______________

*/

#include "emu.h"

#include "cpu/nec/v25.h"

#include "machine/mc68681.h"
#include "machine/timekpr.h"
#include "sound/okim6376.h"

#include "speaker.h"


namespace {

class st25_state : public driver_device
{
public:
	st25_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void st25(machine_config &config) ATTR_COLD;

private:
	required_device<v25_device> m_maincpu;

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;
};


void st25_state::program_map(address_map &map)
{
	//map(0x00000, 0x3ffff).ram();
	//map(0x40000, 0x7ffff).rom().region("maincpu", 0);
	map(0xfc000, 0xfffff).rom().region("maskrom", 0);
}

void st25_state::io_map(address_map &map)
{
	// map(0x8000, 0x8000).w();
}

void st25_state::data_map(address_map &map)
{
	map(0x100, 0x1ff).ram();
}


static INPUT_PORTS_START(st25)
	PORT_START("IN0")
INPUT_PORTS_END


void st25_state::st25(machine_config &config)
{
	// Basic machine hardware

	V25(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &st25_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &st25_state::io_map);
	m_maincpu->set_addrmap(AS_DATA, &st25_state::data_map);
	m_maincpu->pt_in_cb().set([this] () { logerror("%s: pt in\n", machine().describe_context()); return uint8_t(0); });
	m_maincpu->p0_in_cb().set([this] () { logerror("%s: p0 in\n", machine().describe_context()); return uint8_t(0); });
	m_maincpu->p1_in_cb().set([this] () { logerror("%s: p1 in\n", machine().describe_context()); return uint8_t(0); });
	m_maincpu->p2_in_cb().set([this] () { logerror("%s: p2 in\n", machine().describe_context()); return uint8_t(0); });
	m_maincpu->p0_out_cb().set([this] (uint8_t data) { logerror("%s: p0 out %02X\n", machine().describe_context(), data); });
	m_maincpu->p1_out_cb().set([this] (uint8_t data) { logerror("%s: p1 out %02X\n", machine().describe_context(), data); });
	m_maincpu->p2_out_cb().set([this] (uint8_t data) { logerror("%s: p2 out %02X\n", machine().describe_context(), data); });


	M48T02(config, "m48t18", 0); // ST M48T18-150PC1

	SCN2681(config, "uart", 3.6864_MHz_XTAL); // Philips SCC2692AC1N28

	// Sound hardware
	SPEAKER(config, "mono").front_center();
	OKIM6376(config, "oki", 4_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.5); // Divider not verified
}

ROM_START(alphar)
	ROM_REGION(0x4000, "maskrom", 0)
	ROM_LOAD("d70322.icc2", 0x0000, 0x4000, CRC(a3be4fee) SHA1(3e19009d90f71ab21d927cdd31dc60dda652e045))

	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("w27e40.ic2", 0x00000, 0x80000, CRC(3cba9ebe) SHA1(f49a00e0d6f6e34e7fa24bc4339e51c6834bba67))

	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD("w27e40.ic1",   0x00000, 0x80000, CRC(f893b557) SHA1(194135c0cbcb270ebeb297c2f2e26e6101b44daf))
ROM_END

ROM_START(amarillo)
	ROM_REGION(0x4000, "maskrom", 0)
	ROM_LOAD("d70322.icc2", 0x0000, 0x4000, CRC(a3be4fee) SHA1(3e19009d90f71ab21d927cdd31dc60dda652e045))

	ROM_REGION(0x80000, "soundrom", 0) //????????
	ROM_LOAD("27c4001_snd", 0x00000, 0x80000, CRC(2ccf9464) SHA1(02b16fe7465ad28ce96f38390fafbceafca2d23c))

	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD("27c040.ic1", 0x00000, 0x80000, CRC(2114485c) SHA1(ee0bb436367e87bacfe703d0a8ee98c5362e0014))

	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("27c040.ic2", 0x00000, 0x80000, CRC(b5058562) SHA1(c96ca309ca8214dcaeeef41ac29e8c325c08a9d9))
ROM_END

ROM_START(arenau)
	ROM_REGION(0x4000, "maskrom", 0)
	ROM_LOAD("d70322.icc2", 0x0000, 0x4000, CRC(a3be4fee) SHA1(3e19009d90f71ab21d927cdd31dc60dda652e045))

	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD("27c4001.ic1", 0x00000, 0x80000, CRC(93cdf476) SHA1(5b80e76bd04056ff53c7e11cfc5364cea30e4aed))

	ROM_REGION(0x40000, "maincpu", 0)
	ROM_LOAD("27c2001.ic2", 0x00000, 0x40000, CRC(2348e6c3) SHA1(7708a2ffc3b5154bd1793fb7332e26125cdc9696))
ROM_END

ROM_START(avanti)
	ROM_REGION(0x4000, "maskrom", 0)
	ROM_LOAD("d70322.icc2", 0x0000, 0x4000, CRC(a3be4fee) SHA1(3e19009d90f71ab21d927cdd31dc60dda652e045))

	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD("avantie_w27e040.ic1", 0x00000, 0x80000, CRC(47defe53) SHA1(65c246e9051fa1b0f9a855331d55282c6b4ccbc0))

	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("avantie_w27e040.ic2", 0x00000, 0x80000, CRC(14278f3a) SHA1(82a8a5e35e0eee8f4dbcb0e7b6491528c6444fad))
ROM_END

ROM_START(ballermann)
	ROM_REGION(0x4000, "maskrom", 0)
	ROM_LOAD("d70322.icc2", 0x0000, 0x4000, CRC(a3be4fee) SHA1(3e19009d90f71ab21d927cdd31dc60dda652e045))

	ROM_REGION(0x40000, "maincpu", 0)
	ROM_LOAD("27c2001.ic2", 0x00000, 0x40000, CRC(a20915f1) SHA1(cd7e1339bc635a8e16381858b93fe28f04fa725d))

	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD("27c4001.ic1", 0x00000, 0x80000, CRC(1dd6fee1) SHA1(bdd0e478069f822d2b940aa449ec61e9f07d4b3b))
ROM_END

ROM_START(bigactione)
	ROM_REGION(0x4000, "maskrom", 0)
	ROM_LOAD("d70322.icc2", 0x0000, 0x4000, CRC(a3be4fee) SHA1(3e19009d90f71ab21d927cdd31dc60dda652e045))

	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("27c020a.ic2", 0x00000, 0x80000, CRC(177e3fee) SHA1(a4ca38dfdf79eb3524381ea3b6fa7700ad24a966))

	ROM_REGION(0x2000, "nvram", 0)
	ROM_LOAD("m48t18.ic3", 0x00000, 0x2000, CRC(5b1e8172) SHA1(3ee9dfcba8fea095b6e003ba20bbc57fbeb5359e))

	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD("27c040.ic1",   0x00000, 0x80000, CRC(8c708e53) SHA1(ef91a5a21ba69ad2870f7201bb4d90b4bc94c4ec))
ROM_END

ROM_START(bigkick)
	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD("w27e040_big_kick_st25.ic1", 0x00000, 0x80000, CRC(fa752fed) SHA1(5da6f37ebe0095fc74a1c54df86bb3ca492e92f4))

	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("w27e040_big_kick_st25.ic2", 0x00000, 0x80000, CRC(7277e039) SHA1(67e17c675aa68b3e828027c12ea7f51a6ead9549))

	ROM_REGION(0x04000, "maskrom", 0)
	ROM_LOAD("d70322.icc2", 0x00000, 0x04000, CRC(a3be4fee) SHA1(3e19009d90f71ab21d927cdd31dc60dda652e045))
ROM_END

ROM_START(blizzard)
	ROM_REGION(0x4000, "maskrom", 0)
	ROM_LOAD("d70322.icc2", 0x0000, 0x4000, CRC(a3be4fee) SHA1(3e19009d90f71ab21d927cdd31dc60dda652e045))

	ROM_REGION(0x40000, "maincpu", 0)
	ROM_LOAD("tms27c020dip32_blizzard_pgm_119438_170296.ic2", 0x00000, 0x40000, CRC(7ff91608) SHA1(988335313141ca63d06abab6fd2542b167c5c04a))

	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD("tms27c020dip32_blizzard_snd_118999_090595.ic1", 0x00000, 0x80000, CRC(4a609ee5) SHA1(a65aa3cb57f36c56d19cbc1116541dc4a320c6bc))
ROM_END

ROM_START(boostersp)
	ROM_REGION(0x04000, "maskrom", 0)
	ROM_LOAD("d70322.icc2", 0x00000, 0x04000, CRC(a3be4fee) SHA1(3e19009d90f71ab21d927cdd31dc60dda652e045))

	ROM_REGION(0x02000, "nvram", 0)
	ROM_LOAD("m48t18_100pc1_zlk_ok_2033_2.ic3", 0x00000, 0x02000, CRC(e8135250) SHA1(00e4f427911c29ffd478eef57c6db43ecda40eb2))

	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD("w27e040_booster_speed_st25.ic1", 0x00000, 0x80000, CRC(11c8eead) SHA1(2bac833eb0d894fa54c01311b7dbf35b16e1f984))

	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("w27e040_booster_speed_st25.ic2", 0x00000, 0x80000, CRC(9d86d9b9) SHA1(32c6845210807549bf7808b8815a0ac98f2b203a))
ROM_END

ROM_START(citytower)
	ROM_REGION(0x40000, "nvram", 0)
	ROM_LOAD("m27c2001dip32_city_tower_0705.ic3", 0x00000, 0x40000, CRC(e4fbff9c) SHA1(f2081d43051e05ddc54879cf4080746eb8fa43f8))

	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD("panther_city_tower_musik_st1.ic1", 0x00000, 0x80000, CRC(0dcf91a7) SHA1(2e29b55c83bfd649dd92c087eeaa887676755e9f))

	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("panther_city_tower.ic2", 0x00000, 0x80000, CRC(454f200b) SHA1(087ca6b34fc7b5d14fc9ab3f32dc46254acb54a9))

	ROM_REGION(0x04000, "maskrom", 0)
	ROM_LOAD("d70322.icc2", 0x00000, 0x04000, CRC(a3be4fee) SHA1(3e19009d90f71ab21d927cdd31dc60dda652e045))
ROM_END

ROM_START(colossos)
	ROM_REGION(0x04000, "maskrom", 0)
	ROM_LOAD("d70322.icc2", 0x00000, 0x04000, CRC(a3be4fee) SHA1(3e19009d90f71ab21d927cdd31dc60dda652e045))

	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD("w27c40.ic1", 0x00000, 0x80000, CRC(11c8eead) SHA1(2bac833eb0d894fa54c01311b7dbf35b16e1f984))

	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("w27c40.ic2", 0x00000, 0x80000, CRC(724d0d1e) SHA1(f8f1d78e101757afddbbe47b14c0c17ee77e800e))
ROM_END

ROM_START(galaktica)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("galaktica_124460.ic2_w27e040_12.ic2", 0x00000, 0x80000, CRC(a99c6250) SHA1(a9129eeec99c630b0a3e6355deedb86a1ae5062c))

	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD("galaktica_124461.ic1_w27e040_12.ic1", 0x00000, 0x80000, CRC(0e8acf71) SHA1(184472d62e094a724cd21954459e872f8d1b30c8))

	ROM_REGION(0x04000, "maskrom", 0)
	ROM_LOAD("d70322.icc2", 0x00000, 0x04000, CRC(a3be4fee) SHA1(3e19009d90f71ab21d927cdd31dc60dda652e045))
ROM_END

ROM_START(jamaica)
	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD("panther_jamaica_musik_st1.ic1", 0x00000, 0x80000, CRC(7dee50d4) SHA1(1d96ee159b07db84c4f0373f5df4df648ecef786))

	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("panther_jamaica.ic2", 0x00000, 0x80000, CRC(a7119368) SHA1(e50174bd7bb2ba00ab9fda3995007d60f0811242))

	ROM_REGION(0x04000, "maskrom", 0)
	ROM_LOAD("d70322.icc2", 0x00000, 0x04000, CRC(a3be4fee) SHA1(3e19009d90f71ab21d927cdd31dc60dda652e045))
ROM_END

ROM_START(macaor)
	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD("123623.ic1", 0x00000, 0x80000, CRC(dccc242f) SHA1(9c0df10dc0028286a02dada673fa56bd6f137f67))

	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("123742.ic2", 0x00000, 0x80000, CRC(3eeb68c3) SHA1(09f606988608dc89b1714347145b5b01352aa144))

	ROM_REGION(0x04000, "maskrom", 0)
	ROM_LOAD("d70322.icc2", 0x00000, 0x04000, CRC(a3be4fee) SHA1(3e19009d90f71ab21d927cdd31dc60dda652e045))
ROM_END


ROM_START(majesto)
	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD("majesto_st1_m27c4001_121308_17.09.98_euro.ic1", 0x00000, 0x80000, CRC(59033ced) SHA1(39b0821dc2b347677f2803ba1a2c570231f89102))

	ROM_REGION(0x04000, "maskrom", 0)
	ROM_LOAD("d70322.icc2", 0x00000, 0x04000, CRC(a3be4fee) SHA1(3e19009d90f71ab21d927cdd31dc60dda652e045))

	ROM_REGION(0x02000, "nvram", 0)
	ROM_LOAD("majesto_2033_m48t18.ic3", 0x00000, 0x02000, CRC(3e7d6bf6) SHA1(0863371517a4acb6744fa2ed98b01a675c008b9b))

	ROM_REGION(0x40000, "maincpu", 0)
	ROM_LOAD("majesto_m27c2001_st2_122559_28.09.02.ic2", 0x00000, 0x40000, CRC(e08a308c) SHA1(7e015508949e32fd86334ae0e95baf11ca5e26b2))
ROM_END

ROM_START(matrixx)
	ROM_REGION(0x4000, "maskrom", 0)
	ROM_LOAD("d70322.icc2", 0x0000, 0x4000, CRC(a3be4fee) SHA1(3e19009d90f71ab21d927cdd31dc60dda652e045))

	ROM_REGION(0x02000, "nvram", 0)
	ROM_LOAD("tk_m48t18_100pc1.ic3", 0x00000, 0x02000, CRC(675c3ac6) SHA1(11c97bf008b588ec415eeba830fdfabf8d0ce580))

	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD("winbondw27e040_12.ic1", 0x00000, 0x80000, CRC(11c8eead) SHA1(2bac833eb0d894fa54c01311b7dbf35b16e1f984))

	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("winbondw27e040_12.ic2", 0x00000, 0x80000, CRC(6def28eb) SHA1(baab03b436277185aee806aff5f9d804a5bc4664))

	ROM_REGION(0x80000, "soundrom", 0)
	ROM_LOAD("winbondw27e040_12_sound_10_08_02", 0x00000, 0x80000, CRC(2ccf9464) SHA1(02b16fe7465ad28ce96f38390fafbceafca2d23c))
ROM_END

ROM_START(multiclassic)
	ROM_REGION(0x02000, "nvram", 0)
	ROM_LOAD("m48t18_100pc1_l.ic3", 0x00000, 0x02000, CRC(5814553f) SHA1(7bcfe719f6305a19c0db3b95ab13c81a773b2a46))

	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD("w27e040_tsop32_123774_02.07.2004.ic1", 0x00000, 0x80000, CRC(80d754c3) SHA1(b30d18a89d1e9d3dc54687d6a355a3a3d0957c5c))

	ROM_REGION(0x80000, "bp", 0)
	ROM_LOAD("w27e040_tsop32_123775_02.07.2004_back_panel.icf5", 0x00000, 0x80000, CRC(633d0f1e) SHA1(1ba518c5fb7367bf2a43b079a8f8b8db5c6fc5af))

	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("w27e040_tsop32_123986_15.09.2004.ic2", 0x00000, 0x80000, CRC(123e9290) SHA1(c252640ae166fba6b8b76474f3ad2162c0521e87))

	ROM_REGION(0x20000, "coin", 0)
	ROM_LOAD("w29ee011__191_043_v5.05_10.05.04__emp_nsm_euro", 0x00000, 0x20000, CRC(079de5e5) SHA1(7f634769a5d9f56d6dae7a66e21b155b21787c36))

	ROM_REGION(0x04000, "maskrom", 0)
	ROM_LOAD("d70322.icc2", 0x00000, 0x04000, CRC(a3be4fee) SHA1(3e19009d90f71ab21d927cdd31dc60dda652e045))
ROM_END

ROM_START(purpurr)
	ROM_REGION(0x4000, "maskrom", 0)
	ROM_LOAD("d70322.icc2", 0x0000, 0x4000, CRC(a3be4fee) SHA1(3e19009d90f71ab21d927cdd31dc60dda652e045))

	ROM_REGION(0x40000, "maincpu", 0)
	ROM_LOAD("pur_pur_royal_2804_w27e020.ic2", 0x00000, 0x40000, CRC(f7058b6a) SHA1(ad307de9dc979e6c21237b893bb186fc75533b60))

	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD("pur_pur_royal_musik_m27c4001.ic1", 0x00000, 0x80000, CRC(5d7f46d1) SHA1(5fe866c2fc13c8b8a1bd0e14f81a0f7fc3fd0c82))
ROM_END

ROM_START(robin)
	ROM_REGION(0x04000, "maskrom", 0)
	ROM_LOAD("d70322.icc2", 0x00000, 0x04000, CRC(a3be4fee) SHA1(3e19009d90f71ab21d927cdd31dc60dda652e045))

	ROM_REGION(0x40000, "maincpu", 0)
	ROM_LOAD("m27c2001_panther_robin_1705.ic2", 0x00000, 0x40000, CRC(1e465f8d) SHA1(63cd069c867c54f24af893ec2d6ad36016b7e179))

	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD("m27c4001_panther_robin_musik.ic1", 0x00000, 0x80000, CRC(0dcf91a7) SHA1(2e29b55c83bfd649dd92c087eeaa887676755e9f))
ROM_END

ROM_START(smaragd)
	ROM_REGION(0x04000, "maskrom", 0)
	ROM_LOAD("d70322.icc2", 0x00000, 0x04000, CRC(a3be4fee) SHA1(3e19009d90f71ab21d927cdd31dc60dda652e045))

	ROM_REGION(0x08000, "oki", 0)
	ROM_LOAD("115191_c5.ic1", 0x00000, 0x08000, CRC(346b91cb) SHA1(6bb7beaeef890ca7e0e322b5400e4257101c0c88))

	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD("115192_c6.ic2", 0x00000, 0x08000, CRC(6bc0009e) SHA1(a4a10ddd93517cb27be06901232f925f7cbcee52))
ROM_END

ROM_START(stakeoffe)
	ROM_REGION(0x4000, "maskrom", 0)
	ROM_LOAD("d70322.icc2", 0x0000, 0x4000, CRC(a3be4fee) SHA1(3e19009d90f71ab21d927cdd31dc60dda652e045))

	ROM_REGION(0x40000, "maincpu", 0)
	ROM_LOAD("27c020a.ic2", 0x00000, 0x40000, CRC(b1553dc1) SHA1(d04d1e0d7cf553588d6abf2f5c95e0d8a761f8b6))

	ROM_REGION(0x02000, "nvram", 0)
	ROM_LOAD("super_take_off_e_m48t18_100pc1.ic3", 0x00000, 0x02000, CRC(0091ed01) SHA1(3de7a61b145b50255b8374a537c31de505ede370))

	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD("27c040.ic1",   0x00000, 0x80000, CRC(d9592e5e) SHA1(5de917a1c584a39a85e6f356d25924a65eaddf89))
ROM_END

ROM_START(superpasch)
	ROM_REGION(0x4000, "maskrom", 0)
	ROM_LOAD("d70322.icc2", 0x0000, 0x4000, CRC(a3be4fee) SHA1(3e19009d90f71ab21d927cdd31dc60dda652e045))

	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("124253.ic2", 0x00000, 0x80000, CRC(fe23b37a) SHA1(9d461b01d05c6e71e3d32800a429ad3f733d7274))

	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD("124254.ic1",   0x00000, 0x80000, CRC(f893b557) SHA1(194135c0cbcb270ebeb297c2f2e26e6101b44daf))
ROM_END

ROM_START(tango)
	ROM_REGION(0x04000, "maskrom", 0)
	ROM_LOAD("d70322.icc2", 0x00000, 0x04000, CRC(a3be4fee) SHA1(3e19009d90f71ab21d927cdd31dc60dda652e045))

	ROM_REGION(0x40000, "maincpu", 0)
	ROM_LOAD("am27c020_nsm_tango.ic2", 0x00000, 0x40000, CRC(7c0fec14) SHA1(9c2c463c9b39dd1167203c67fb6632d9379a37fe))

	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD("tms27c040_nsm_tango.ic1", 0x00000, 0x80000, CRC(fee679c1) SHA1(c702d41805b05ccc50a2291eb30950a4be9a6e75))
ROM_END

ROM_START(tobago)
	ROM_REGION(0x04000, "maskrom", 0)
	ROM_LOAD("d70322.icc2", 0x00000, 0x04000, CRC(a3be4fee) SHA1(3e19009d90f71ab21d927cdd31dc60dda652e045))

	ROM_REGION(0x40000, "maincpu", 0)
	ROM_LOAD("27c2001.ic2", 0x00000, 0x40000, CRC(dc7e529b) SHA1(3bf7b3e0a27c808061c47515513fa6e76d26cd63))

	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD("27c4001.ic1", 0x00000, 0x80000, CRC(d1d6a78a) SHA1(bef512186af630f938d429c3db4d88ae9523272a))
ROM_END

ROM_START(xeno)
	ROM_REGION(0x04000, "maskrom", 0)
	ROM_LOAD("d70322.icc2", 0x00000, 0x04000, CRC(a3be4fee) SHA1(3e19009d90f71ab21d927cdd31dc60dda652e045))

	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD("w27e040_xeno_125227.ic1", 0x00000, 0x80000, CRC(a37314b8) SHA1(b3bb3bacb6a12057b10f3f5e9a039924abf1d43d))

	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("w27e040_xeno_125227.ic2", 0x00000, 0x80000, CRC(349e38d1) SHA1(0e97ad119cf6864aee826da4c1d560094ff6f22d))

	ROM_REGION(0x80000, "soundrom", 0)
	ROM_LOAD("w27e040_xeno_125228.ic5", 0x00000, 0x80000, CRC(f3475039) SHA1(62e7a132d88976249ec2c047bf47d39a60636ec9))
ROM_END


} // anonymous namespace


//   YEAR  NAME      PARENT   MACHINE INPUT    CLASS   INIT        ROT   COMPANY         FULLNAME                                                  FLAGS
GAME(1994, smaragd,      0,     st25, st25, st25_state, empty_init, ROT0, "Panther",  "Smaragd",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(1995, blizzard,     0,     st25, st25, st25_state, empty_init, ROT0, u8"Löwen",  "Blizzard",          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(1995, tango,        0,     st25, st25, st25_state, empty_init, ROT0, u8"Löwen",  "Tango",             MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(1996, tobago,       0,     st25, st25, st25_state, empty_init, ROT0, "Bergmann", "Tobago",            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(1997, ballermann,   0,     st25, st25, st25_state, empty_init, ROT0, "Panther",  "Ballermann 6",      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(1998, arenau,       0,     st25, st25, st25_state, empty_init, ROT0, u8"Löwen",  "Unimint Arena",     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(1998, citytower,    0,     st25, st25, st25_state, empty_init, ROT0, "Panther",  "City Tower",        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(1998, jamaica,      0,     st25, st25, st25_state, empty_init, ROT0, u8"Löwen",  "Jamaica",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(1998, majesto,      0,     st25, st25, st25_state, empty_init, ROT0, u8"Löwen",  "Majesto",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(1998, purpurr,      0,     st25, st25, st25_state, empty_init, ROT0, "Panther",  "Pur Pur Royal",     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(1999, robin,        0,     st25, st25, st25_state, empty_init, ROT0, "Panther",  "Robin",             MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(2000, bigactione,   0,     st25, st25, st25_state, empty_init, ROT0, "Panther",  "Big Action 3000 E", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(2001, stakeoffe,    0,     st25, st25, st25_state, empty_init, ROT0, "Panther",  "Super Take Off E",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(2002, boostersp,    0,     st25, st25, st25_state, empty_init, ROT0, u8"Löwen",  "Booster Speed",     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(2003, colossos,     0,     st25, st25, st25_state, empty_init, ROT0, u8"Löwen",  "Colossos",          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(2003, matrixx,      0,     st25, st25, st25_state, empty_init, ROT0, u8"Löwen",  "Matrixx",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(2004, avanti,       0,     st25, st25, st25_state, empty_init, ROT0, u8"Löwen",  "Avanti",            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(2004, macaor,       0,     st25, st25, st25_state, empty_init, ROT0, u8"Löwen",  "Rotamint Macao",    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(2004, multiclassic, 0,     st25, st25, st25_state, empty_init, ROT0, u8"Löwen",  "Multiclassic",      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(2005, alphar,       0,     st25, st25, st25_state, empty_init, ROT0, u8"Löwen",  "Rotamint Alpha",    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(2005, superpasch,   0,     st25, st25, st25_state, empty_init, ROT0, u8"Löwen",  "Super Pasch",       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(2006, bigkick,      0,     st25, st25, st25_state, empty_init, ROT0, u8"Löwen",  "Big Kick",          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(2006, galaktica,    0,     st25, st25, st25_state, empty_init, ROT0, u8"Löwen",  "Galaktica",         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(2007, amarillo,     0,     st25, st25, st25_state, empty_init, ROT0, u8"Löwen",  "Amarillo",          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(2007, xeno,         0,     st25, st25, st25_state, empty_init, ROT0, u8"Löwen",  "Xeno",              MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)

