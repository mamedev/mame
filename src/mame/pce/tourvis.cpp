// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek
/****************************************************************************

    TourVisión
    Driver by Mariusz Wojcieszek and Stephh

    Bootleg PC-Engine based arcade board from the Spanish company TourVisión.
    Two known hardware revisions, one with a sub-board with the PC-Engine chipset
    and the other as an integrated PCB.

    Todo: complete jamma interface emulation.

    By now, ten known BIOS versions.
    U4-52 (dumped from a board with-subboard PCB).
    U4-55 (dumped from an integrated PCB).
    U4-60 (dumped from a board with-subboard PCB).

    Known games list can be found in hash/pce_tourvision.xml.

* Denotes Not Dumped

 _______________________________________________________________________________________________________________________________________________
|                                                                                                                                               |
|                                           ____________               ____________               ____________               ____________       |
|                                          |T74LS125AB1|  ____        |T74LS125AB1|  ____        |T74LS125AB1|  ____        |T74LS125AB1|  ____ |
|                                          -------------  |  |        -------------  |  |        -------------  |  |        -------------  |  | |
|                                           ____________  |  |         ____________  |  |         ____________  |  |         ____________  |  | |
|                                          | SN74F245N |  |  |        | SN74F245N |  |  |        | SN74F245N |  |  |        | SN74F245N |  |  | |
|                                          -------------  |  |        -------------  |  |        -------------  |  |        -------------  |  | |
|                                           ____________  |  |         ____________  |  |         ____________  |  |         ____________  |  | |
|       ___________   ____________         | SN74F245N |  |  |        | SN74F245N |  |  |        | SN74F245N |  |  |        | SN74F245N |  |  | |
|      |4116R-001  | |X74LS32B1  |         -------------  |  |        -------------  |  |        -------------  |  |        -------------  |  | |
|      ------------- -------------                        |JP|                       |JP|                       |JP|                       |JP| |
|       ___________                                       | 4|                       | 3|                       | 2|                       | 1| |
|__     ___________                         ____________  |  |         ____________  |  |         ____________  |  |         ____________  |  | |
   |    ____________  ____________         | 74LS244N  |  |  |        | 74LS244N  |  |  |        | 74LS244N  |  |  |        | 74LS244N  |  |  | |
 __|   |SN74LS257SN| |4116R-001  |         -------------  |  |        -------------  |  |        -------------  |  |        -------------  |  | |
|__    ------------- -------------          ____________  |  |         ____________  |  |         ____________  |  |         ____________  |  | |
|__     ____________  ____________         | SN74F245N |  |  |        | SN74F245N |  |  |        | SN74F245N |  |  |        | SN74F245N |  |  | |
|__    | 74LS157N  | | 74LS157N  |         -------------  |  |        -------------  |  |        -------------  |  |        -------------  |  | |
|__    ------------- -------------          ____________  |  |         ____________  |  |         ____________  |  |         ____________  |  | |
|__     ____________  ____________         | 74LS244N  |  ----        | 74LS244N  |  ----        | 74LS244N  |  ----        | 74LS244N  |  ---- |
|__    | SN74LS08N | | SN74LS08N |         -------------              -------------              -------------              -------------       |
|__    ------------- -------------                                                                                                              |
|__     ____________  ____________                                                                                            ____________      |
|__    | SN74LS08N | | SN74LS08N |                                                                                            |HSRM2264LM10     |
|__    ------------- -------------                                                                                            |__________|      |
|__     ____________  ____________                                                                                                              |
|__    | 74LS138N  | | 74LS138N  |                                                                                                              |
|__    ------------- -------------                                                                                                              |
|__     ____________  ____________  ____________   ____________   ____________  ____________  ____________                                      |
|__    |  74LS244N | |  74LS244N |  |T74LS32B1 |   |MC14017BCP|  | T74LS14B1 | | GD74LS393 | |T74LS125AB1|                                      |
|__    ------------- -------------  ------------   ------------  ------------- ------------  -------------                                      |
|__      .........     .........    ____________                                                                                                |
|__      .........     .........    | 74LS138N |                     _________                                                                  |
|__     ___________   ___________   ------------                     |       |                                      ______                      |
|__    |4116R-001  | |4116R-001  |  _________________                |       |                                     | HU  |        HSRM20256LM12 |
|__    ------------- ------------- |                |                |  BT1  |                                     |C6270|          ___         |
|__      __________  ____________  | NEC D4465C     |                |       |                                     |     |         |  |         |
|__     | TC4011BP|  |SN74LS373N | |________________|                |_______|                                     |_____|         |  |         |
|__     -----------  -------------  _________________                                                                              |__|         |
|__    ..... ___________     ____  | TOURVISION BIOS|                                                                                           |
|__         | 74LS138N |     |XT1| |                |                                                     ______     ______          ___        |
   |        ------------     |___| |________________|               ____                                 | HU  |    | HU  |         |  |        |
 __|    IC_  _____________                   ________              |    |                                |C6280A    |C6260A         |  |        |
|      |36| |  74LS244N  |  _________________________              | PT |                                |     |    |     |         |__|        |
|      ---- -------------  |                        |              |____|                                |_____|    |_____|                     |
|  ________  _____________ | NEC D8085AHC           |    ___________   ___________   _______                                       HSRM20256LM12|
| | DIP 2 | |  74LS244N  | |________________________|   | T74LS14B1|  |MC14001BCP|  |LM393N|                          ________                  |
| --------- --------------  _________________________   ------------  ------------  --------                         |D74HCU04C                 |
|  ________  _____________ |                        |    ___________                                                 ---------                  |
|  ________ |  74LS244N  | | NEC D8155HC            |   |  7407N   |                                             ____                           |
| | DIP 1 | -------------- |________________________|   ------------                                             |XT2|                          |
| ---------          ____                _____________         _____________                                     |___|                          |
|                    JP107              | JP 106     |        | JP 105     |                                                                    |
|                    ----               --------------        --------------                                                                    |
|_______________________________________________________________________________________________________________________________________________|

IC36  = ST NE 555N 99201
XT1   = 6144 KSS1H
JP107 = 2-pin connector
JP106 = 14-pin connector to 2-digit 7 segments display
JP105 = 16-pin connector (unknown functionality)
PT    = Push-type switch
BT1   = 3.6 V battery
XT2   = 21.32825 MHz UNI 90-H
JP1-4 = Carts slots

Games are dumped directly from the cartridge edge connector using the following adapter:

 ----------------------------------------------------------------------------
 Cartridge pinout
 ----------------------------------------------------------------------------

                       +----------+
                (N.C.) |50      49| +5V
                   +5V |48      47| +5V
                   A18 |46      45| +5V
                   A14 |44      43| A17
                    A8 |42      41| A13
                   A11 |40      39| A9
                   A10 |38      37| OE#
                    D7 |36      35| CE#
(front of           D5 |34      33| D6               (rear of
 cartridge)         D3 |32      31| D4                cartridge)
                    D2 |30      29| GND
                    D0 |28      27| D1
                    A1 |26      25| A0
                    A3 |24      23| A2
                    A5 |22      21| A4
                    A7 |20      19| A6
                   A15 |18      17| A12
                   A19 |16      15| A16
                   GND |14      13| (N.C.)
                   GND |12      11| GND)
                 (KEY) |10------09| (KEY)
                   ID7 |08      07| ID6
                   ID5 |06      05| ID4
                   ID3 |04      03| ID2
                   ID1 |02      01| ID0
                       +----------+

 ----------------------------------------------------------------------------
 27C080 pinout
 ----------------------------------------------------------------------------
                        +----v----+
                    A19 | 1     32| +5V
                    A16 | 2     31| A18
                    A15 | 3     30| A17
                    A12 | 4     29| A14
                     A7 | 5     28| A13
                     A6 | 6     27| A8
                     A5 | 7     26| A9
                     A4 | 8     25| A11
                     A3 | 9     24| OE#
                     A2 |10     23| A10
                     A1 |11     22| CE#
                     A0 |12     21| D7
                     D0 |13     20| D6
                     D1 |14     19| D5
                     D2 |15     18| D4
                    GND |16     17| D3
                        +---------+


Stephh notes for 8085 code:
0xe01d : game slot number (range 0-3) - sometimes inc'ed/dec'ed/zeroed, but also filled based on games slot status (code at 0x01e8) :
0x8009 and 0x800a : main timer (BCD, LSB first)

0xe054 to 0xe057 : display timer (main, LSdigit first, stored in 4 lower bits)
0xe058 to 0xe05b : display timer (game slot 1, LSdigit first, stored in 4 upper bits)
0xe05c to 0xe05f : display timer (game slot 2, LSdigit first, stored in 4 lower bits)
0xe060 to 0xe063 : display timer (game slot 3, LSdigit first, stored in 4 upper bits)
0xe064 to 0xe067 : display timer (game slot 4, LSdigit first, stored in 4 lower bits)

display timer (main) "filled" with code at 0x054e
display timer (game slot n) "filled" with code at 0x04e3

coin insertion routine at 0x0273
coin 1 triggers code at 0x02d7
coin 2 triggers code at 0x028f

in each coin insertion routine, you need to insert n coins (based on DSW settings) then you are awarded u units of time (also based on DSW settings)
I can't tell ATM if units are seconds (even if values in tables seem very related to them)

****************************************************************************
Notes from system11:
Game ID is configured on carts using pins 1 -> 8, these form a single byte integer, known IDs are tagged above.  IDs can be shared between games.  Game handling is defined based on data tables in the BIOS, using offset + ID*data block size.  Data block starts in 6.0 BIOS as follows:

1184   1284   1384   1484   1584   3584   5d84   5684

For full information see:
http://blog.system11.org/?p=1943

****************************************************************************/

#include "emu.h"
#include "pcecommn.h"

#include "cpu/h6280/h6280.h"
#include "cpu/i8085/i8085.h"
#include "video/huc6260.h"
#include "video/huc6270.h"
#include "machine/i8155.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

namespace
{

class tourvision_state : public pce_common_state
{
public:
	tourvision_state(const machine_config &mconfig, device_type type, const char *tag)
		: pce_common_state(mconfig, type, tag)
		, m_subcpu(*this, "subcpu")
		, m_cart(*this, "cartslot")
	{ }

	void tourvision(machine_config &config);

private:
	void tourvision_8085_d000_w(uint8_t data);
	void tourvision_i8155_a_w(uint8_t data);
	void tourvision_i8155_b_w(uint8_t data);
	void tourvision_i8155_c_w(uint8_t data);
	void tourvision_timer_out(int state);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	void pce_io(address_map &map) ATTR_COLD;
	void pce_mem(address_map &map) ATTR_COLD;
	void tourvision_8085_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_subcpu;
	required_device<generic_slot_device> m_cart;
	uint32_t  m_rom_size = 0;
};

DEVICE_IMAGE_LOAD_MEMBER(tourvision_state::cart_load)
{
	m_rom_size = m_cart->common_get_size("rom");
	m_cart->rom_alloc(m_rom_size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), m_rom_size, "rom");

	uint8_t *rgn = memregion("maincpu")->base();
	uint8_t *base = m_cart->get_rom_base();

	if (m_rom_size == 0x0c0000)
	{
		memcpy(rgn+0x000000, base+0x000000, 0x0c0000 );
		memcpy(rgn+0x0c0000, base+0x080000, 0x040000 );
	}
	else if (m_rom_size == 0x060000)
	{
		memcpy(rgn+0x000000, base+0x000000, 0x040000 );
		memcpy(rgn+0x040000, base+0x000000, 0x040000 );
		memcpy(rgn+0x080000, base+0x040000, 0x020000 );
		memcpy(rgn+0x0a0000, base+0x040000, 0x020000 );
		memcpy(rgn+0x0c0000, base+0x040000, 0x020000 );
		memcpy(rgn+0x0e0000, base+0x040000, 0x020000 );
	}
	else
	{
		for (int i=0; i<0x100000; i+=m_rom_size)
			memcpy(rgn+i, base+0x000000, m_rom_size);
	}

#if 0
	{
		FILE *fp;
		fp=fopen("tourvision.bin", "w+b");
		if (fp)
		{
			fwrite(rgn, 0x100000, 1, fp);
			fclose(fp);
		}
	}
#endif

	return std::make_pair(std::error_condition(), std::string());
}

// Note from system11: This system actually supports 2 players

INPUT_PORTS_START( tourvision )
	PCE_STANDARD_INPUT_PORT_P1

	PORT_START( "DSW1" )
	PORT_DIPNAME( 0x07, 0x07, "Coins needed 1" )
	PORT_DIPSETTING(    0x07, "1" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPNAME( 0x78, 0x78, "Time units 1" )
	PORT_DIPSETTING(    0x78, "900" )
	PORT_DIPSETTING(    0x70, "720" )
	PORT_DIPSETTING(    0x68, "600" )
	PORT_DIPSETTING(    0x60, "540" )
	PORT_DIPSETTING(    0x58, "480" )
	PORT_DIPSETTING(    0x50, "420" )
	PORT_DIPSETTING(    0x48, "360" )
	PORT_DIPSETTING(    0x40, "300" )
	PORT_DIPSETTING(    0x38, "270" )
	PORT_DIPSETTING(    0x30, "240" )
	PORT_DIPSETTING(    0x28, "210" )
	PORT_DIPSETTING(    0x20, "180" )
	PORT_DIPSETTING(    0x18, "150" )
	PORT_DIPSETTING(    0x10, "120" )
	PORT_DIPSETTING(    0x08, "90" )
	PORT_DIPSETTING(    0x00, "60" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))

	PORT_START( "DSW2" )
	PORT_DIPNAME( 0x03, 0x03, "Coins needed 2" )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x78, 0x78, "Time units 2" )
	PORT_DIPSETTING(    0x78, "1500" )
	PORT_DIPSETTING(    0x70, "1200" )
	PORT_DIPSETTING(    0x68, "1080" )
	PORT_DIPSETTING(    0x60, "960" )
	PORT_DIPSETTING(    0x58, "900" )
	PORT_DIPSETTING(    0x50, "840" )
	PORT_DIPSETTING(    0x48, "780" )
	PORT_DIPSETTING(    0x40, "720" )
	PORT_DIPSETTING(    0x38, "660" )
	PORT_DIPSETTING(    0x30, "600" )
	PORT_DIPSETTING(    0x28, "540" )
	PORT_DIPSETTING(    0x20, "480" )
	PORT_DIPSETTING(    0x18, "420" )
	PORT_DIPSETTING(    0x10, "360" )
	PORT_DIPSETTING(    0x08, "300" )
	PORT_DIPSETTING(    0x00, "240" )
	PORT_DIPUNKNOWN( 0x80, 0x00 )
// SW2 bit 7 might be "free play" HIGH and when "Coins needed 2" is set to "1" (multiple comparisons with 0x83) in BIOS0 and BIOS1.
// In BIOS2, "Coins needed 2" can be set to anything (multiple comparisons with 0x80) instead.
// Of course, it can also be sort of "Test mode" or "Debug mode" ...

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_CUSTOM ) // games slot status in bits 3 to 7
INPUT_PORTS_END

void tourvision_state::pce_mem(address_map &map)
{
	map(0x000000, 0x0FFFFF).rom();
	map(0x1F0000, 0x1F1FFF).ram().mirror(0x6000);
	map(0x1FE000, 0x1FE3FF).rw("huc6270", FUNC(huc6270_device::read), FUNC(huc6270_device::write));
	map(0x1FE400, 0x1FE7FF).rw(m_huc6260, FUNC(huc6260_device::read), FUNC(huc6260_device::write));
}

void tourvision_state::pce_io(address_map &map)
{
	map(0x00, 0x03).rw("huc6270", FUNC(huc6270_device::read), FUNC(huc6270_device::write));
}

void tourvision_state::tourvision_8085_d000_w(uint8_t data)
{
	//logerror( "D000 (8085) write %02x\n", data );
}

void tourvision_state::tourvision_8085_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x80ff).rw("i8155", FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
	map(0x8100, 0x8107).rw("i8155", FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
	map(0x9000, 0x9000).portr("DSW1");
	map(0xa000, 0xa000).portr("DSW2");
	map(0xb000, 0xb000).nopr(); // unknown (must NOT be == 0x03 ? code at 0x1154)
	map(0xc000, 0xc000).portr("SYSTEM");
	map(0xd000, 0xd000).w(FUNC(tourvision_state::tourvision_8085_d000_w));
	map(0xe000, 0xe1ff).ram();
	map(0xf000, 0xf000).nopr(); // protection or internal counter ? there is sometimes some data in BIOS0 which is replaced by 0xff in BIOS1
}

void tourvision_state::tourvision_i8155_a_w(uint8_t data)
{
	//logerror("i8155 Port A: %02X\n", data);
}

void tourvision_state::tourvision_i8155_b_w(uint8_t data)
{
	// Selects game slot in bits 0 - 1
	//logerror("i8155 Port B: %02X\n", data);
}

void tourvision_state::tourvision_i8155_c_w(uint8_t data)
{
	//logerror("i8155 Port C: %02X\n", data);
}

void tourvision_state::tourvision_timer_out(int state)
{
	m_subcpu->set_input_line(I8085_RST55_LINE, state ? CLEAR_LINE : ASSERT_LINE );
	//logerror("Timer out %d\n", state);
}


void tourvision_state::tourvision(machine_config &config)
{
	// Basic machine hardware
	H6280(config, m_maincpu, PCE_MAIN_CLOCK/3);
	m_maincpu->set_addrmap(AS_PROGRAM, &tourvision_state::pce_mem);
	m_maincpu->set_addrmap(AS_IO, &tourvision_state::pce_io);
	m_maincpu->port_in_cb().set(FUNC(tourvision_state::pce_joystick_r));
	m_maincpu->port_out_cb().set(FUNC(tourvision_state::pce_joystick_w));
	m_maincpu->add_route(0, "lspeaker", 1.00);
	m_maincpu->add_route(1, "rspeaker", 1.00);

	config.set_maximum_quantum(attotime::from_hz(60));

	I8085A(config, m_subcpu, 18000000/3 /*?*/);
	m_subcpu->set_addrmap(AS_PROGRAM, &tourvision_state::tourvision_8085_map);

	// Video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(PCE_MAIN_CLOCK, huc6260_device::WPF, 64, 64 + 1024 + 64, huc6260_device::LPF, 18, 18 + 242);
	screen.set_screen_update(FUNC(pce_common_state::screen_update));
	screen.set_palette(m_huc6260);

	HUC6260(config, m_huc6260, PCE_MAIN_CLOCK);
	m_huc6260->next_pixel_data().set("huc6270", FUNC(huc6270_device::next_pixel));
	m_huc6260->time_til_next_event().set("huc6270", FUNC(huc6270_device::time_until_next_event));
	m_huc6260->vsync_changed().set("huc6270", FUNC(huc6270_device::vsync_changed));
	m_huc6260->hsync_changed().set("huc6270", FUNC(huc6270_device::hsync_changed));

	huc6270_device &huc6270(HUC6270(config, "huc6270", 0));
	huc6270.set_vram_size(0x10000);
	huc6270.irq().set_inputline(m_maincpu, 0);

	i8155_device &i8155(I8155(config, "i8155", 1000000 /*?*/));
	i8155.out_pa_callback().set(FUNC(tourvision_state::tourvision_i8155_a_w));
	i8155.out_pb_callback().set(FUNC(tourvision_state::tourvision_i8155_b_w));
	i8155.out_pc_callback().set(FUNC(tourvision_state::tourvision_i8155_c_w));
	i8155.out_to_callback().set(FUNC(tourvision_state::tourvision_timer_out));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	generic_cartslot_device &cartslot(GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "tourvision_cart", "bin"));
	cartslot.set_device_load(FUNC(tourvision_state::cart_load));
	cartslot.set_must_be_loaded(true);

	SOFTWARE_LIST(config, "tv_list").set_original("pce_tourvision");
}

#define TOURVISION_BIOS \
	ROM_REGION( 0x8000, "subcpu", 0 ) \
	ROM_SYSTEM_BIOS( 0, "60", "V4-60" ) \
	ROMX_LOAD( "v4-60.ic29", 0x0000, 0x8000, CRC(1fd27e22) SHA1(b103d365eac3fa447c2e9addddf6974b4403ed41), ROM_BIOS(0) ) \
	ROM_SYSTEM_BIOS( 1, "55", "V4-55" ) \
	ROMX_LOAD( "v4-55.ic29", 0x0000, 0x8000, CRC(87cf66c1) SHA1(d6b42137be7a07a0e299c2d922328a6a9a2b7b8f), ROM_BIOS(1) ) \
	ROM_SYSTEM_BIOS( 2, "54", "V4-54" ) \
	ROMX_LOAD( "v4-54.ic29", 0x0000, 0x8000, CRC(125e7a6e) SHA1(6f03c49acaad4feb9c2187f84fc1cb5451d0eb2b), ROM_BIOS(2) ) \
	ROM_SYSTEM_BIOS( 3, "53", "V4-53" ) \
	ROMX_LOAD( "v4-53.ic29", 0x0000, 0x8000, CRC(bccb53c9) SHA1(a27113d70cf348c7eafa39fc7a76f55f63723ad7), ROM_BIOS(3) ) \
	ROM_SYSTEM_BIOS( 4, "52", "V4-52" ) \
	ROMX_LOAD( "v4-52.ic29", 0x0000, 0x8000, CRC(ffd7b0fe) SHA1(d1804865c91e925a01b05cf441e8458a3db23f50), ROM_BIOS(4) ) \
	ROM_SYSTEM_BIOS( 5, "51", "V4-51" ) \
	ROMX_LOAD( "v4-51.ic29", 0x0000, 0x8000, CRC(2de0b946) SHA1(c13051453aff3a2329f18ebade75c69aa3b1c28d), ROM_BIOS(5) ) \
	ROM_SYSTEM_BIOS( 6, "43", "V4-43" ) \
	ROMX_LOAD( "v4-43.ic29", 0x0000, 0x8000, CRC(88da23f3) SHA1(9d24faa116129783e55c7f79a4a08902a236d5a6), ROM_BIOS(6) ) \
	ROM_SYSTEM_BIOS( 7, "42", "V4-42" ) \
	ROMX_LOAD( "v4-42.ic29", 0x0000, 0x8000, CRC(0372606d) SHA1(8c6a63865b0b4951ea032725cf022d353115b93b), ROM_BIOS(7) ) \
	ROM_SYSTEM_BIOS( 8, "40", "V4-40" ) \
	ROMX_LOAD( "v4-40.ic29", 0x0000, 0x8000, CRC(ba6290cc) SHA1(92b0e9f55791e892ec209de4fadd80faef370622), ROM_BIOS(8) ) \
	ROM_SYSTEM_BIOS( 9, "20", "VT-20" ) \
	ROMX_LOAD( "vt_2.0.bin", 0x0000, 0x8000, CRC(36012f88) SHA1(5bd42fb51aa48ff65e704ea06a9181bb87ed2137), ROM_BIOS(9) ) \
	ROM_SYSTEM_BIOS(10, "11", "VT-11" ) \
	ROMX_LOAD( "vt_1.1.bin", 0x0000, 0x8000, CRC(27abbc36) SHA1(881ea7802b9e241473bc8ced0472e0f1851c9886), ROM_BIOS(10) )

ROM_START(tourvis)
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASE00 )

	TOURVISION_BIOS // BIOS rom type is 27C256
ROM_END

} // anonymous namespace

GAME( 19??, tourvis, 0, tourvision, tourvision, tourvision_state, init_pce_common, ROT0, u8"bootleg (TourVisión)", u8"TourVisión (PC Engine bootleg)", MACHINE_IS_BIOS_ROOT | MACHINE_NOT_WORKING )
