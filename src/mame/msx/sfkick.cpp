// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/*
  Super Free Kick / Spinkick by HEC (Haesung Enterprise Co.)

    driver by Tomasz Slanina

  Hacked MSX2 home computer hardware. Romset contains
  modified ( (c) strings removed and patched boot sequence)
  MSX2 bios. Yamaha VDP v9938 is hidden in huge epoxy block.
  There's also an additional Z80 to drive sound.


     1      2       3        4        5         6         7
+----------------------------------------------------------------+
|                                                                |
|  C1182  Y3014B       YM2203C           Z80A             a7     | A
|                                                                |
| 1UP     GL324                                                  | B
|                                                         c7     |
| 2UP                                     c5                     | C
|                                                                |
+-+       DSW1              74139        6116             d7     | D
  |                              21.47727 MHz                    |
+-+       74241         CN1              74244                   | E
|                                                                |
|         DSW2                           74373                   | G
|                                                                |
|         74241    4464     4464     7404     74139       h7     | H
| J                           CN2                                |
| A       74157    4464     4464     7400     74670              | J
| M                                                       j7     |
| M       40106    74169    7404     7432     74670              | K
| A                                                              |
|         74241    74169    74138    7432     7402        l7     | L
|                                                                |
+-+       74241    74153    74139    74138    74138      6264    | M
  |                                                              |
+-+       74241     82C55                 Z80A           6264    | N
|                                                                |
+----------------------------------------------------------------+

Board # CBK1029

CN1: 40 PIN Connector (Epoxy Block )
CN2: 8  PIN Connector (Epoxy Block)
1UP: 4 PIN Connector (Analog Controls)
2UP: 4 PIN Connector (Analog Controls)

Z8400A (x2)
UM82C55A-PC
YM2203C

Documentation as per manual:

            Main Jamma Connector
    Solder Side    |        Parts Side
------------------------------------------------------------------
           GND | A | 1 | GND
           GND | B | 2 | GND
            +5 | C | 3 | +5
            +5 | D | 4 | +5
               | E | 5 |
           +12 | F | 6 | +12
----- KEY -----| H | 7 |----- KEY -----
               | J | 8 |
               | K | 9 |
   Speaker (-) | L | 10| Speaker (+)
               | M | 11|
   Video Green | N | 12| Video Red
    Video Sync | P | 13| Video Blue
 Player 1 Left | R | 14| Player 2 Right
Player 1 Right | S | 15| Player 2 Left
 Coin Switch 2 | T | 16| Coin Switch 1
Player 2 Start | U | 17| Player 1 Start
               | V | 18|
               | W | 19|
               | X | 20|
               | Y | 21|
Player 2 Shoot | Z | 22| Player 1 Shoot
               | a | 23|
               | b | 24|
               | c | 25|
               | d | 26|
           GND | e | 27| GND
           GND | f | 28| GND

         ____
        /    \
       | Dial |
        \____/
       /|   |\
     /  |   | \
 Blue Red Black Yellow
  /     |   |    \
Left  +5v  GND   Right


DIPSW-1
------------------------------------------------------------------
    DipSwitch Title   | Function | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
------------------------------------------------------------------
    Cabinet Style     | Upright  |off|                           |
                      | Cocktail |on |                           |
------------------------------------------------------------------
    Stage Select      |   Off    |   |off|                       |
                      |   On     |   |on |                       |
------------------------------------------------------------------
    Freeze Screen     |   Off    |       |off|                   |
                      |   On     |       |on |                   |
------------------------------------------------------------------
   Test / Game Mode   |   Game   |           |off|               |
                      |   Test   |           |on |               |
------------------------------------------------------------------
    Allow Continue    |   Off    |               |off|           |
                      |   On     |               |on |           |
------------------------------------------------------------------
                      | 1cn/1cr  |                   |off|off|off|
                      | 1cn/2cr  |                   |on |off|off|
                      | 1cn/3cr  |                   |off|on |off|
        Coinage       | 1cn/5cr  |                   |on |on |off|
                      | 2cn/1cr  |                   |off|off|on |
                      | 2cn/3cr  |                   |on |off|on |
                      | 3cn/1cr  |                   |off|on |on |
                      | 3cn/2cr  |                   |on |on |on |
------------------------------------------------------------------

DIPSW-2
------------------------------------------------------------------
    DipSwitch Title   | Function | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
------------------------------------------------------------------
      No Comment      |   N/A    |off|                           |
------------------------------------------------------------------
     Demo Sounds      |   Yes    |   |off|                       |
                      |   No     |   |on |                       |
------------------------------------------------------------------
                      |    1     |       |off|off|               |
    Players Count     |    2     |       |on |off|               |
                      |    3     |       |off|on |               |
                      |    5     |       |on |on |               |
-----------------------------------------------------------------
                      |   None   |               |off|off|       |
         Bonus        |Every 20K |               |on |off|       |
                      |20K & 50K |               |off|on |       |
                      |Every 50K |               |on |on |       |
------------------------------------------------------------------
                      |   Easy   |                       |off|off|
      Difficulty      |  Normal  |                       |on |off|
                      |   Hard   |                       |off|on |
                      |  V.Hard  |                       |on |on |
------------------------------------------------------------------

*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/bankdev.h"
#include "machine/gen_latch.h"
#include "machine/i8255.h"
#include "sound/ymopn.h"
#include "video/v9938.h"

#include "screen.h"
#include "speaker.h"


namespace {

class sfkick_state : public driver_device
{
public:
	sfkick_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_page(*this, "page%u", 0U),
		m_bank(*this, "bank%u", 0U),
		m_in0(*this, "IN0"),
		m_in1(*this, "IN1"),
		m_dial(*this, "DIAL"),
		m_dsw1(*this, "DSW1"),
		m_dsw2(*this, "DSW2")
	{ }

	void sfkick(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	uint8_t mem_r(offs_t offset);
	void mem_w(offs_t offset, uint8_t data);
	void bank_w(offs_t offset, uint8_t data);
	uint8_t ppi_port_b_r();
	void ppi_port_a_w(uint8_t data);
	void ppi_port_c_w(uint8_t data);
	void irqhandler(int state);
	void sfkick_io_map(address_map &map) ATTR_COLD;
	void sfkick_map(address_map &map) ATTR_COLD;
	void sfkick_sound_io_map(address_map &map) ATTR_COLD;
	void sfkick_sound_map(address_map &map) ATTR_COLD;
	void bank_mem(address_map &map) ATTR_COLD;

	uint8_t m_primary_slot_reg = 0;
	uint8_t m_input_mux = 0;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device_array<address_map_bank_device, 4> m_page;
	required_memory_bank_array<8> m_bank;
	required_ioport m_in0;
	required_ioport m_in1;
	required_ioport m_dial;
	required_ioport m_dsw1;
	required_ioport m_dsw2;
};


#define MASTER_CLOCK    XTAL(21'477'272)

uint8_t sfkick_state::mem_r(offs_t offset)
{
	return m_page[offset >> 14 & 3]->read8(offset);
}

void sfkick_state::mem_w(offs_t offset, uint8_t data)
{
	m_page[offset >> 14 & 3]->write8(offset, data);
}

void sfkick_state::bank_w(offs_t offset, uint8_t data)
{
	m_bank[offset >> 13 & 7]->set_entry(data & 0xf);
}

uint8_t sfkick_state::ppi_port_b_r()
{
	switch(m_input_mux & 0x0f)
	{
		case 0: return m_in0->read();
		case 1: return m_in1->read();
		case 2: return bitswap<8>(m_dial->read(), 4, 5, 6, 7, 3, 2, 1, 0);
		case 3: return m_dsw1->read();
		case 4: return m_dsw2->read();
	}
	return 0xff;
}

void sfkick_state::ppi_port_a_w(uint8_t data)
{
	if (data != m_primary_slot_reg)
	{
		for (int i = 0; i < 4; i++)
			m_page[i]->set_bank(data >> (i * 2) & 3);

		m_primary_slot_reg = data;
	}
}

void sfkick_state::sfkick_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(sfkick_state::mem_r), FUNC(sfkick_state::mem_w)); // 4 pages of 16KB
}

void sfkick_state::bank_mem(address_map &map)
{
	// slot 0, MSX BIOS
	map(0x00000, 0x07fff).rom().region("bios", 0);
	map(0x08000, 0x0bfff).rom().region("cartridge", 0x4000);

	// slot 1, extrom
	map(0x10000, 0x13fff).rom().region("extrom", 0x4000);

	// slot 2, banked
	map(0x20000, 0x21fff).bankr(m_bank[0]);
	map(0x22000, 0x23fff).bankr(m_bank[1]);
	map(0x24000, 0x25fff).bankr(m_bank[2]);
	map(0x26000, 0x27fff).bankr(m_bank[3]);
	map(0x28000, 0x29fff).bankr(m_bank[4]);
	map(0x2a000, 0x2bfff).bankr(m_bank[5]);
	map(0x2c000, 0x2dfff).bankr(m_bank[6]);
	map(0x2e000, 0x2ffff).bankr(m_bank[7]);
	map(0x20000, 0x2ffff).w(FUNC(sfkick_state::bank_w));

	// slot 3, 16KB RAM
	map(0x3c000, 0x3ffff).ram();
}

void sfkick_state::sfkick_io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0xa0, 0xa7).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x98, 0x9b).rw("v9938", FUNC(v9938_device::read), FUNC(v9938_device::write));
	map(0xa8, 0xab).rw("ppi8255", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xb4, 0xb5).ram(); // loopback ? req by sfkicka (MSX BIOS leftover)
}

void sfkick_state::sfkick_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xc7ff).ram();
}

void sfkick_state::sfkick_sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x04, 0x05).rw("ym1", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
}

void sfkick_state::ppi_port_c_w(uint8_t data)
{
	m_input_mux = data;
}

static INPUT_PORTS_START( sfkick )
	PORT_START("IN0")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNUSED ) // unused ?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("IN1")
	PORT_BIT( 0x71, IP_ACTIVE_LOW, IPT_UNUSED ) // unused ?
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DIAL")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(20) PORT_REVERSE

	PORT_START("DSW1") // bitswapped at read! 76543210 -> 45673210
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x10, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x01, 0x01,  "Stage Select" ) PORT_DIPLOCATION("SW1:2") // How does this work??
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20,  "Freeze" )   PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02,  "Test Mode" )    PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x8c, 0x8c, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:6,8,7")
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x84, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x8c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x88, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )

	PORT_START("DSW2") // bitswapped at read! 76543210 -> 45673210
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:1" ) // Manual states "No Comment"
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x41, 0x01, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x41, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x82, 0x02, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x80, "Every 20,000" )
	PORT_DIPSETTING(    0x02, "20,000 & 50,000" )
	PORT_DIPSETTING(    0x00, "Every 50,000" )
	PORT_DIPSETTING(    0x82, DEF_STR( None ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END

void sfkick_state::machine_start()
{
	// init banks
	for (int i = 0; i < 8; i++)
		m_bank[i]->configure_entries(0, 0x10, memregion("banked")->base(), 0x2000);

	save_item(NAME(m_primary_slot_reg));
	save_item(NAME(m_input_mux));
}

void sfkick_state::machine_reset()
{
	m_primary_slot_reg = 0;
	m_input_mux = 0;

	for (int i = 0; i < 4; i++)
		m_page[i]->set_bank(0);
	for (int i = 0; i < 8; i++)
		m_bank[i]->set_entry(i);
}

void sfkick_state::irqhandler(int state)
{
	m_soundcpu->set_input_line_and_vector(0, state ? ASSERT_LINE : CLEAR_LINE, 0xff); // Z80
}

void sfkick_state::sfkick(machine_config &config)
{
	Z80(config, m_maincpu, MASTER_CLOCK/6);
	m_maincpu->set_addrmap(AS_PROGRAM, &sfkick_state::sfkick_map);
	m_maincpu->set_addrmap(AS_IO, &sfkick_state::sfkick_io_map);

	config.set_maximum_quantum(attotime::from_hz(60000));

	Z80(config, m_soundcpu, MASTER_CLOCK/6);
	m_soundcpu->set_addrmap(AS_PROGRAM, &sfkick_state::sfkick_sound_map);
	m_soundcpu->set_addrmap(AS_IO, &sfkick_state::sfkick_sound_io_map);

	ADDRESS_MAP_BANK(config, m_page[0]).set_map(&sfkick_state::bank_mem).set_options(ENDIANNESS_LITTLE, 8, 18, 0x10000);
	ADDRESS_MAP_BANK(config, m_page[1]).set_map(&sfkick_state::bank_mem).set_options(ENDIANNESS_LITTLE, 8, 18, 0x10000);
	ADDRESS_MAP_BANK(config, m_page[2]).set_map(&sfkick_state::bank_mem).set_options(ENDIANNESS_LITTLE, 8, 18, 0x10000);
	ADDRESS_MAP_BANK(config, m_page[3]).set_map(&sfkick_state::bank_mem).set_options(ENDIANNESS_LITTLE, 8, 18, 0x10000);

	v9938_device &v9938(V9938(config, "v9938", MASTER_CLOCK));
	v9938.set_screen_ntsc("screen");
	v9938.set_vram_size(0x80000);
	v9938.int_cb().set_inputline(m_maincpu, 0);
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	i8255_device &ppi(I8255A(config, "ppi8255"));
	ppi.out_pa_callback().set(FUNC(sfkick_state::ppi_port_a_w));
	ppi.in_pb_callback().set(FUNC(sfkick_state::ppi_port_b_r));
	ppi.out_pc_callback().set(FUNC(sfkick_state::ppi_port_c_w));

	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	ym2203_device &ym1(YM2203(config, "ym1", MASTER_CLOCK/6));
	ym1.irq_handler().set(FUNC(sfkick_state::irqhandler));
	ym1.add_route(0, "mono", 0.25);
	ym1.add_route(1, "mono", 0.25);
	ym1.add_route(2, "mono", 0.25);
	ym1.add_route(3, "mono", 0.50);
}


ROM_START( sfkick )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION(0x20000,  "banked", ROMREGION_ERASEFF)
	ROM_LOAD( "sfkick2.a7", 0x00000, 0x8000, CRC(1dcaec5e) SHA1(7e063d46fb6606df2d772866cc55f207035b98c4) )
	ROM_LOAD( "sfkick3.c7", 0x08000, 0x8000, CRC(639d3cf2) SHA1(950fd28058d32e4532eb6e99454dcaef092a955e) )
	ROM_LOAD( "sfkick4.d7", 0x10000, 0x8000, CRC(ee1b344e) SHA1(d33fbad017cc4838192e9c540621537edb7e8dc4) )
	// 0x18000-0x1ffff = empty

	ROM_REGION(0x8000,  "extrom", 0)
	ROM_LOAD( "sfkick5.h7", 0x00000, 0x8000, CRC(8e8bd9cf) SHA1(f493de40147fdd67a48d4c90b01170fbd6ea074e) )

	ROM_REGION(0x8000,  "cartridge", 0)
	ROM_LOAD( "sfkick6.j7", 0x0000, 0x8000, CRC(7a91ac4b) SHA1(afc5e2c2fe0cd108235ac6ae2775cc9a0b1c9f76) )

	ROM_REGION(0x8000,  "bios", 0)
	ROM_LOAD( "sfkick7.l7", 0x00000, 0x8000, CRC(8cd94c63) SHA1(e6dba66c8716593b8ab88f79f7205211938d1598) )

	ROM_REGION(0x10000,  "soundcpu", 0)
	ROM_LOAD( "sfkick1.c5", 0x00000, 0x8000, CRC(2f5e3b7a) SHA1(d2ff566b415ab10c0681fa1eb221a56e3c137ecf) )
ROM_END


ROM_START( sfkicka )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION(0x20000,  "banked", ROMREGION_ERASEFF)
	ROM_LOAD( "c145.bin", 0x00000, 0x8000, CRC(1dcaec5e) SHA1(7e063d46fb6606df2d772866cc55f207035b98c4) )
	ROM_LOAD( "c146.bin", 0x08000, 0x8000, CRC(57afc4c6) SHA1(ee28b3f74e3175c22f542855b09f1673d048b1fa) )
	ROM_LOAD( "c147.bin", 0x10000, 0x8000, CRC(ee1b344e) SHA1(d33fbad017cc4838192e9c540621537edb7e8dc4) )
	// 0x18000-0x1ffff = empty

	ROM_REGION(0x8000,  "extrom", 0)
	ROM_LOAD( "c149.bin", 0x00000, 0x8000, CRC(2edbf61f) SHA1(23dcff43faf222a4b69001312ce4b1c920e2f4c2) )

	ROM_REGION(0x8000,  "cartridge", 0)
	ROM_LOAD( "c150.bin", 0x0000, 0x8000, CRC(20412918) SHA1(b0fefa957b20373ffb84d9ff97a2e84a9a3af56c) )

	ROM_REGION(0x8000,  "bios", 0)
	ROM_LOAD( "c151.bin", 0x00000, 0x8000, CRC(8cd94c63) SHA1(e6dba66c8716593b8ab88f79f7205211938d1598) )

	ROM_REGION(0x10000,  "soundcpu", 0)
	ROM_LOAD( "c130.bin", 0x00000, 0x8000, CRC(2f5e3b7a) SHA1(d2ff566b415ab10c0681fa1eb221a56e3c137ecf) )
ROM_END


ROM_START( spinkick )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION(0x20000,  "banked", ROMREGION_ERASEFF)
	ROM_LOAD( "spinkick.r2", 0x00000, 0x8000, CRC(1dcaec5e) SHA1(7e063d46fb6606df2d772866cc55f207035b98c4) )
	ROM_LOAD( "spinkick.r3", 0x08000, 0x8000, CRC(e86a194a) SHA1(19a02375ec463e795770403c3e948d754919458b) )
	ROM_LOAD( "spinkick.r4", 0x10000, 0x8000, CRC(ee1b344e) SHA1(d33fbad017cc4838192e9c540621537edb7e8dc4) )
	// 0x18000-0x1ffff = empty

	ROM_REGION(0x8000,  "extrom", 0)
	ROM_LOAD( "spinkick.r5", 0x00000, 0x8000, CRC(8e8bd9cf) SHA1(f493de40147fdd67a48d4c90b01170fbd6ea074e) )

	ROM_REGION(0x8000,  "cartridge", 0)
	ROM_LOAD( "spinkick.r6", 0x0000, 0x8000, CRC(7a91ac4b) SHA1(afc5e2c2fe0cd108235ac6ae2775cc9a0b1c9f76) )

	ROM_REGION(0x8000,  "bios", 0)
	ROM_LOAD( "spinkick.r7", 0x00000, 0x8000, CRC(8cd94c63) SHA1(e6dba66c8716593b8ab88f79f7205211938d1598) )

	ROM_REGION(0x10000,  "soundcpu", 0)
	ROM_LOAD( "spinkick.r1", 0x00000, 0x8000, CRC(2f5e3b7a) SHA1(d2ff566b415ab10c0681fa1eb221a56e3c137ecf) )
ROM_END

} // anonymous namespace


GAME( 1988, sfkick,   0,      sfkick, sfkick, sfkick_state, empty_init, ROT90, "Haesung/HJ Corp", "Super Free Kick (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 198?, sfkicka,  sfkick, sfkick, sfkick, sfkick_state, empty_init, ROT90, "Haesung",         "Super Free Kick (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, spinkick, sfkick, sfkick, sfkick, sfkick_state, empty_init, ROT90, "Haesung/Seojin",  "Hec's Spinkick",          MACHINE_SUPPORTS_SAVE )
