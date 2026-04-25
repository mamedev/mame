// license:BSD-3-Clause
// copyright-holders:
/***************************************************************************************************

  Skeleton driver for Gaelco 'Scalextric', an electromechanical arcade using real toy slot cars.
  The exact hardware configuration is unknown.
  It unclear if Gaelco distributed this game directly or if they used a partner like Cresmatic or
  Covielsa. It's also unknown if the game was sold under the name of 'Scalextric' or if it was
  just the internal name at Gaelco and they used a different external name (currently, 'Scalextric'
  is a trademark for toy slot cars from Purbeck Capital Partners, but at that time it was from
  EXIN [until 1993], Tyco Toys [from 1993 to 1998] and TecniToys [from 1998 to 2012]).

***************************************************************************************************/

#include "emu.h"

#include "cpu/mcs51/i8051.h"
#include "cpu/pic16c5x/pic16c5x.h"

#include "sound/okim6295.h"

#include "speaker.h"

namespace {

class scalextric_state : public driver_device
{
public:
	scalextric_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
		, m_6295(*this, "musicrom")
	{
	}

	void scalextric(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<pic16c57_device> m_subcpu;
	required_device<okim6295_device> m_6295;
};

static INPUT_PORTS_START(scalextric)
INPUT_PORTS_END

// Unknown hardware configuration. Everything is guessed from the ROMs contents.
void scalextric_state::scalextric(machine_config &config)
{
	I8051(config, m_maincpu, 8'000'000); // Guess

	PIC16C57(config, "subcpu", 8'000'000);  // Guess

	OKIM6295(config, "musicrom", 2'000'000, okim6295_device::PIN7_LOW); // Guess

	SPEAKER(config, "mono").front_center(); // Guess
}

ROM_START(scxsp)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD("ok_27c256.bin", 0x00000, 0x08000, CRC(3845b691) SHA1(2116d2b5cc8e4be3ff0fb1b948bb1b5b26e38757)) // 20-jul-1994

	ROM_REGION(0x02000, "subcpu", 0)
	/* User ID0=0x00F, ID1=0x00F, ID2=0x00F, ID3=0x00F.
           Config Word = 0x0FFD. */
	ROM_LOAD("9-3_esp_33646_pic16c57.u1", 0x00000, 0x02000, CRC(520f6e27) SHA1(3c3f148c211ed0a71d9d896e921a158d736c6154)) // 9-mar

	ROM_REGION(0x40000, "musicrom", 0)
	ROM_LOAD("esp_10-2_27c2001.u6", 0x00000, 0x40000, CRC(5a639b7a) SHA1(61538824e51e9daf232319d68070fcd9fd425e8d)) // 10-feb

	ROM_REGION(0x200, "eeprom", 0)
	ROM_LOAD("93c66.bin", 0x00000, 0x200, NO_DUMP)
ROM_END

ROM_START(scxspa)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD("ok_27c256.bin",        0x00000, 0x08000, CRC(3845b691) SHA1(2116d2b5cc8e4be3ff0fb1b948bb1b5b26e38757)) // 20-jul-1994

	ROM_REGION(0x02000, "subcpu", 0)
	/* User ID0=0x00F, ID1=0x00F, ID2=0x00F, ID3=0x00F.
           Config Word = 0x0FFF. */
	ROM_LOAD("mostra_spain_24-1-04_33924_pic16c57.bin", 0x00000, 0x02000, CRC(e331429c) SHA1(b6e9cd1a4dd92f0b1f04c1e27d5b965949d1e215)) // 24-jan

	ROM_REGION(0x40000, "musicrom", 0)
	ROM_LOAD("esp_10-2_27c2001.u6",  0x00000, 0x40000, CRC(5a639b7a) SHA1(61538824e51e9daf232319d68070fcd9fd425e8d)) // 10-feb

	ROM_REGION(0x200, "eeprom", 0)
	ROM_LOAD("93c66.bin",  0x00000, 0x200, NO_DUMP)
ROM_END

ROM_START(scxspb)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD("ok_27c256.bin",        0x00000, 0x08000, CRC(3845b691) SHA1(2116d2b5cc8e4be3ff0fb1b948bb1b5b26e38757)) // 20-jul-1994

	ROM_REGION(0x02000, "subcpu", 0)
	/* User ID0=0x00F, ID1=0x00F, ID2=0x00F, ID3=0x00F.
           Config Word = 0x0FF5. */
	ROM_LOAD("esp_4-12_pic16c57.u1", 0x00000, 0x02000, CRC(2836d5bb) SHA1(07b20515d1fdb19e44be984ba24955f57b563d9a)) // 04-dec

	ROM_REGION(0x40000, "musicrom", 0)
	ROM_LOAD("esp_10-2_27c2001.u6",  0x00000, 0x40000, CRC(5a639b7a) SHA1(61538824e51e9daf232319d68070fcd9fd425e8d)) // 10-feb

	ROM_REGION(0x200, "eeprom", 0)
	ROM_LOAD("93c66.bin",  0x00000, 0x200, NO_DUMP)
ROM_END

ROM_START(scxspc)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD("ok_27c256.bin",        0x00000, 0x08000, CRC(3845b691) SHA1(2116d2b5cc8e4be3ff0fb1b948bb1b5b26e38757)) // 20-jul-1994

	ROM_REGION(0x02000, "subcpu", 0)
	/* User ID0=0x00F, ID1=0x00F, ID2=0x00F, ID3=0x00F.
           Config Word = 0x0FFD. */
	ROM_LOAD("q_mostra_esca_sp3_334a2_pic16c57.bin", 0x00000, 0x02000, CRC(d6a0d0ec) SHA1(bb64e0e9c432584766014277ecf12752e3c6948f))

	ROM_REGION(0x40000, "musicrom", 0)
	ROM_LOAD("esp_10-2_27c2001.u6",  0x00000, 0x40000, CRC(5a639b7a) SHA1(61538824e51e9daf232319d68070fcd9fd425e8d)) // 10-feb

	ROM_REGION(0x200, "eeprom", 0)
	ROM_LOAD("93c66.bin",  0x00000, 0x200, NO_DUMP)
ROM_END

ROM_START(scxspd)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD("ok_27c256.bin",        0x00000, 0x08000, CRC(3845b691) SHA1(2116d2b5cc8e4be3ff0fb1b948bb1b5b26e38757)) // 20-jul-1994

	ROM_REGION(0x02000, "subcpu", 0)
	/* User ID0=0x00F, ID1=0x00F, ID2=0x00F, ID3=0x00F.
           Config Word = 0x0FFD. */
	ROM_LOAD("mostra_pic16c57.bin", 0x00000, 0x02000, CRC(414f08b6) SHA1(fb471db71adde7fcd2d931f16d1d2dd6bb17dfad))

	ROM_REGION(0x40000, "musicrom", 0)
	ROM_LOAD("esp_10-2_27c2001.u6",  0x00000, 0x40000, CRC(5a639b7a) SHA1(61538824e51e9daf232319d68070fcd9fd425e8d)) // 10-feb

	ROM_REGION(0x200, "eeprom", 0)
	ROM_LOAD("93c66.bin",  0x00000, 0x200, NO_DUMP)
ROM_END

ROM_START(scxam)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD("ok_27c256.bin", 0x00000, 0x08000, CRC(3845b691) SHA1(2116d2b5cc8e4be3ff0fb1b948bb1b5b26e38757)) // 20-jul

	ROM_REGION(0x02000, "subcpu", 0)
	/* User ID0=0x00F, ID1=0x00F, ID2=0x00F, ID3=0x00F.
           Config Word = 0x0FFD. */
	ROM_LOAD("america_mostra_puc16c57.bin", 0x00000, 0x02000, CRC(fbcf2894) SHA1(d2cd7d601716b97624ba7cf1d8e6e7ab9daa60ca))

	ROM_REGION(0x40000, "musicrom", 0)
	ROM_LOAD("por_27c020.u6", 0x00000, 0x40000, NO_DUMP) // Uses the music ROM from another region?

	ROM_REGION(0x200, "eeprom", 0)
	ROM_LOAD("por_93c66.bin", 0x00000, 0x200, NO_DUMP)
ROM_END

ROM_START(scxen)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD("ok_27c256.bin", 0x00000, 0x08000, CRC(3845b691) SHA1(2116d2b5cc8e4be3ff0fb1b948bb1b5b26e38757)) // 20-jul-1994

	ROM_REGION(0x02000, "subcpu", 0)
	/* User ID0=0x00F, ID1=0x00F, ID2=0x00F, ID3=0x00F.
           Config Word = 0x0FFD. */
	ROM_LOAD("9-3_ing_33749_pic16c57.u1", 0x00000, 0x02000, CRC(9f5e1c52) SHA1(2120f070f95039138e1471060e1cd4653b17101a)) // 09-mar

	ROM_REGION(0x40000, "musicrom", 0)
	ROM_LOAD("ing_escal_d79b_27c020.u6", 0x00000, 0x40000, CRC(a61f73d8) SHA1(5b6a7f5f4ca539e686ab238211a76a58b9bab437)) // 8-nov

	ROM_REGION(0x200, "eeprom", 0)
	ROM_LOAD("i_93c66.bin", 0x00000, 0x200, CRC(d45ac0fb) SHA1(8edb51ac03b1f93396f3abc6218f41a25d4125ef))
ROM_END

// V3?
ROM_START(scxena)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD("ok_27c256.bin", 0x00000, 0x08000, CRC(3845b691) SHA1(2116d2b5cc8e4be3ff0fb1b948bb1b5b26e38757)) // 20-jul-1994

	ROM_REGION(0x02000, "subcpu", 0)
	/* User ID0=0x00F, ID1=0x00F, ID2=0x00F, ID3=0x00F.
           Config Word = 0x0FFD. */
	ROM_LOAD("mostra_esca_ing3_ing_pic16c57.bin", 0x00000, 0x02000, CRC(47f8446f) SHA1(4738073da9d8020e11c972c6657c788b1ddd2f08))

	ROM_REGION(0x40000, "musicrom", 0)
	ROM_LOAD("ing_escal_d79b_27c020.u6", 0x00000, 0x40000, CRC(a61f73d8) SHA1(5b6a7f5f4ca539e686ab238211a76a58b9bab437)) // 8-nov

	ROM_REGION(0x200, "eeprom", 0)
	ROM_LOAD("i_93c66.bin", 0x00000, 0x200, CRC(d45ac0fb) SHA1(8edb51ac03b1f93396f3abc6218f41a25d4125ef))
ROM_END

ROM_START(scxfr)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD("ok_27c256.bin", 0x00000, 0x08000, CRC(3845b691) SHA1(2116d2b5cc8e4be3ff0fb1b948bb1b5b26e38757)) // 20-jul-1994

	ROM_REGION(0x02000, "subcpu", 0)
	/* User ID0=0x00F, ID1=0x00F, ID2=0x00F, ID3=0x00F.
           Config Word = 0x0FFD. */
	ROM_LOAD("fra_33749_10-7_pic16c57.u1", 0x00000, 0x02000, CRC(1c5b2e41) SHA1(9efc95995eca1935bc34f208003ea2ba9c527ba5)) // 10-jul

	ROM_REGION(0x40000, "musicrom", 0)
	ROM_LOAD("franca_escal_8-11_27c020.u6", 0x00000, 0x40000, CRC(ac8cd17f) SHA1(f804b3edf81097e3f6ce8d61230d7b954625738b)) // 8-nov

	ROM_REGION(0x200, "eeprom", 0)
	ROM_LOAD("f_93c66.bin", 0x00000, 0x200, CRC(138da63c) SHA1(5ac3de511cea15c88eca9abdabe7edd159e116c3))
ROM_END

// V3?
ROM_START(scxfra)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD("ok_27c256.bin", 0x00000, 0x08000, CRC(3845b691) SHA1(2116d2b5cc8e4be3ff0fb1b948bb1b5b26e38757)) // 20-jul-1994

	ROM_REGION(0x02000, "subcpu", 0)
	/* User ID0=0x00F, ID1=0x00F, ID2=0x00F, ID3=0x00F.
           Config Word = 0x0FFD. */
	ROM_LOAD("mostra_esca_fra3_fra_pic16c57.bin", 0x00000, 0x02000, CRC(c4fd767c) SHA1(fcec74f824d1a63931903783f6b3c4b0e19ab46e))

	ROM_REGION(0x40000, "musicrom", 0)
	ROM_LOAD("franca_escal_8-11_27c020.u6", 0x00000, 0x40000, CRC(ac8cd17f) SHA1(f804b3edf81097e3f6ce8d61230d7b954625738b)) // 8-nov

	ROM_REGION(0x200, "eeprom", 0)
	ROM_LOAD("f_93c66.bin", 0x00000, 0x200, CRC(138da63c) SHA1(5ac3de511cea15c88eca9abdabe7edd159e116c3))
ROM_END

ROM_START(scxgr)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD("ok_27c256.bin", 0x00000, 0x08000, CRC(3845b691) SHA1(2116d2b5cc8e4be3ff0fb1b948bb1b5b26e38757)) // 20-jul

	ROM_REGION(0x02000, "subcpu", 0)
	/* User ID0=0x00F, ID1=0x00F, ID2=0x00F, ID3=0x00F.
           Config Word = 0x0FFD. */
	ROM_LOAD("9-3_ale_3373d_pic16c57.u1", 0x00000, 0x02000, CRC(c16249e8) SHA1(b5f5202bb181120c3f45af134c9392ed72963f61)) // 09-mar

	ROM_REGION(0x40000, "musicrom", 0)
	ROM_LOAD("por_27c020.u6", 0x00000, 0x40000, NO_DUMP) // Uses the music ROM from another region?

	ROM_REGION(0x200, "eeprom", 0)
	ROM_LOAD("por_93c66.bin", 0x00000, 0x200, NO_DUMP)
ROM_END

ROM_START(scxit)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD("ok_27c256.bin", 0x00000, 0x08000, CRC(3845b691) SHA1(2116d2b5cc8e4be3ff0fb1b948bb1b5b26e38757)) // 20-jul

	ROM_REGION(0x02000, "subcpu", 0)
	/* User ID0=0x00F, ID1=0x00F, ID2=0x00F, ID3=0x00F.
           Config Word = 0x0FFD. */
	ROM_LOAD("ita_14-3_33740_pic16c57.u1", 0x00000, 0x02000, CRC(60da3ba2) SHA1(a8576ebb99899cedf9a6114b6279a67be7f05e24)) // 14-mar

	ROM_REGION(0x40000, "musicrom", 0)
	ROM_LOAD("ita_e636_27c020.u6", 0x00000, 0x40000, CRC(ad762b9b) SHA1(6a7fa7dbe7ad8cc7d3bffc768b4184f8dc0bfb8c)) // 10-feb

	ROM_REGION(0x200, "eeprom", 0)
	ROM_LOAD("ita_93c66.bin", 0x00000, 0x200, CRC(be29e90f) SHA1(aa5de3e18f47a002a71cf479f1cfb862088a560c))
ROM_END

// V3?
ROM_START(scxita)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD("ok_27c256.bin", 0x00000, 0x08000, CRC(3845b691) SHA1(2116d2b5cc8e4be3ff0fb1b948bb1b5b26e38757)) // 20-jul

	ROM_REGION(0x02000, "subcpu", 0)
	/* User ID0=0x00F, ID1=0x00F, ID2=0x00F, ID3=0x00F.
           Config Word = 0x0FFD. */
	ROM_LOAD("mostra_esca_ita3_ita_pic16c57.bin", 0x00000, 0x02000, CRC(116ce768) SHA1(1dc926c0a0fe7f3bb970305b0dedaa952ec4837c))

	ROM_REGION(0x40000, "musicrom", 0)
	ROM_LOAD("ita_e636_27c020.u6", 0x00000, 0x40000, CRC(ad762b9b) SHA1(6a7fa7dbe7ad8cc7d3bffc768b4184f8dc0bfb8c)) // 10-feb

	ROM_REGION(0x200, "eeprom", 0)
	ROM_LOAD("ita_93c66.bin", 0x00000, 0x200, CRC(be29e90f) SHA1(aa5de3e18f47a002a71cf479f1cfb862088a560c))
ROM_END

ROM_START(scxpt)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD("ok_27c256.bin", 0x00000, 0x08000, CRC(3845b691) SHA1(2116d2b5cc8e4be3ff0fb1b948bb1b5b26e38757)) // 20-jul

	ROM_REGION(0x02000, "subcpu", 0)
	/* User ID0=0x00F, ID1=0x00F, ID2=0x00F, ID3=0x00F.
           Config Word = 0x0FFD. */
	ROM_LOAD("9-3_por_3373f_pic16c57.u1", 0x00000, 0x02000, CRC(2e21de9a) SHA1(979188bb71d1e241500bd4266222d0d28154fe5a)) // 9-mar

	ROM_REGION(0x40000, "musicrom", 0)
	ROM_LOAD("por_27c020.u6", 0x00000, 0x40000, NO_DUMP) // Uses the music ROM from another region?

	ROM_REGION(0x200, "eeprom", 0)
	ROM_LOAD("por_93c66.bin", 0x00000, 0x200, NO_DUMP)
ROM_END

ROM_START(scxus)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD("ok_27c256.bin", 0x00000, 0x08000, CRC(3845b691) SHA1(2116d2b5cc8e4be3ff0fb1b948bb1b5b26e38757)) // 20-jul

	ROM_REGION(0x02000, "subcpu", 0)
	/* User ID0=0x00F, ID1=0x00F, ID2=0x00F, ID3=0x00F.
           Config Word = 0x0FFD. */
	ROM_LOAD("otu_913_usa_3373c_pic16c57.u1", 0x00000, 0x02000, CRC(12eacf0f) SHA1(b37cddb8d735707191dce6f6622a8fd80ea2d3e3))

	ROM_REGION(0x40000, "musicrom", 0)
	ROM_LOAD("por_27c020.u6", 0x00000, 0x40000, NO_DUMP) // Uses the music ROM from another region?

	ROM_REGION(0x200, "eeprom", 0)
	ROM_LOAD("por_93c66.bin", 0x00000, 0x200, NO_DUMP)
ROM_END

} // anonymous namespace

GAME(1994, scxsp,  0,     scalextric, scalextric, scalextric_state, empty_init, ROT0, "Gaelco", "Scalextric (Spain, set 1)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(1994, scxspa, scxsp, scalextric, scalextric, scalextric_state, empty_init, ROT0, "Gaelco", "Scalextric (Spain, set 2)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(1994, scxspb, scxsp, scalextric, scalextric, scalextric_state, empty_init, ROT0, "Gaelco", "Scalextric (Spain, set 3)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(1994, scxspc, scxsp, scalextric, scalextric, scalextric_state, empty_init, ROT0, "Gaelco", "Scalextric (Spain, set 4)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(1994, scxspd, scxsp, scalextric, scalextric, scalextric_state, empty_init, ROT0, "Gaelco", "Scalextric (Spain, set 5)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(1994, scxam,  scxsp, scalextric, scalextric, scalextric_state, empty_init, ROT0, "Gaelco", "Scalextric (America)",        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(1994, scxen,  scxsp, scalextric, scalextric, scalextric_state, empty_init, ROT0, "Gaelco", "Scalextric (England, set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(1994, scxena, scxsp, scalextric, scalextric, scalextric_state, empty_init, ROT0, "Gaelco", "Scalextric (England, set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(1994, scxfr,  scxsp, scalextric, scalextric, scalextric_state, empty_init, ROT0, "Gaelco", "Scalextric (France, set 1)",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(1994, scxfra, scxsp, scalextric, scalextric, scalextric_state, empty_init, ROT0, "Gaelco", "Scalextric (France, set 2)",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(1994, scxgr,  scxsp, scalextric, scalextric, scalextric_state, empty_init, ROT0, "Gaelco", "Scalextric (Germany)",        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(1994, scxit,  scxsp, scalextric, scalextric, scalextric_state, empty_init, ROT0, "Gaelco", "Scalextric (Italy, set 1)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(1994, scxita, scxsp, scalextric, scalextric, scalextric_state, empty_init, ROT0, "Gaelco", "Scalextric (Italy, set 2)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(1994, scxpt,  scxsp, scalextric, scalextric, scalextric_state, empty_init, ROT0, "Gaelco", "Scalextric (Portugal)",       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(1994, scxus,  scxsp, scalextric, scalextric, scalextric_state, empty_init, ROT0, "Gaelco", "Scalextric (USA)",            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
