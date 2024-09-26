// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek, hap
/***************************************************************************

Penguin Adventure bootleg (tagged 'Screen', 1988)
Original release was on MSX, by Konami in 1986. There is no official arcade release of this game.

Driver by Mariusz Wojcieszek

This seems to be the MSX version hacked to run on cheap Korean(?) bootleg hardware.
Bosses are at wrong stages when compared to the original, probably to make the game more
difficult early on. This is also the cause of some gfx glitches when reaching a boss.

Basic components include.....
Z80 @ 3.579533MHz [10.7386/3]
TMS9128 @ 10.7386MHz
AY-3-8910 @ 1.789766MHz [10.7386/6]
8255
4416 RAM x2
4164 RAM x8
10.7386 XTAL
10 position DIPSW
NOTE! switches 1, 3 & 5 must be ON or the game will not boot.

== MSX2 hardware version:

It's on the same PCB as sfkick, but with a small daughterboard for the sound chip, and no epoxy block.
Positions originally for YM2203 and extra Z80 are not populated.

TODO:
- pengadvb: add dipswitch
- pengadvb: A timer apparently expires when beating stage 4 (signalled by a long beeping sound).
  Player needs to insert another credit and press start button (?) in order to continue.
  Is this timer supposed to be shown on screen or there are additional 7-LEDs not handled?
- pengadvb2: V9938 video chip with 64KB VRAM instead of TMS9128, game works ok though
- pengadvb2: The CBK1029 PCB is also emulated by sfkick.cpp. Merge drivers? The board is differently populated.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/tms9928a.h"
#include "sound/ay8910.h"
#include "machine/i8255.h"
#include "machine/bankdev.h"
#include "screen.h"
#include "speaker.h"


namespace {

class pengadvb_state : public driver_device
{
public:
	pengadvb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_page(*this, "page%u", 0U)
		, m_bank(*this, "bank%u", 0U)
	{ }

	void pengadvb(machine_config &config);

	void init_pengadvb();
	void init_pengadvb2();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	uint8_t mem_r(offs_t offset);
	void mem_w(offs_t offset, uint8_t data);
	void megarom_bank_w(offs_t offset, uint8_t data);

	void psg_port_b_w(uint8_t data);
	uint8_t ppi_port_a_r();
	void ppi_port_a_w(uint8_t data);
	uint8_t ppi_port_b_r();
	void ppi_port_c_w(uint8_t data);

	void pengadvb_decrypt(const char* region);
	void bank_mem(address_map &map) ATTR_COLD;
	void io_mem(address_map &map) ATTR_COLD;
	void program_mem(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device_array<address_map_bank_device, 4> m_page;
	required_memory_bank_array<4> m_bank;

	uint8_t m_primary_slot_reg = 0;
	uint8_t m_kb_matrix_row = 0;
};


/***************************************************************************

  Z80 Memory map

***************************************************************************/

uint8_t pengadvb_state::mem_r(offs_t offset)
{
	return m_page[offset >> 14 & 3]->read8(offset);
}

void pengadvb_state::mem_w(offs_t offset, uint8_t data)
{
	m_page[offset >> 14 & 3]->write8(offset, data);
}

void pengadvb_state::megarom_bank_w(offs_t offset, uint8_t data)
{
	m_bank[offset >> 13 & 3]->set_entry(data & 0xf);
}

void pengadvb_state::program_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(pengadvb_state::mem_r), FUNC(pengadvb_state::mem_w)); // 4 pages of 16KB
}

void pengadvb_state::bank_mem(address_map &map)
{
	// slot 0, MSX BIOS
	map(0x00000, 0x07fff).rom().region("maincpu", 0);

	// slot 1, MegaROM
	map(0x14000, 0x15fff).bankr("bank0");
	map(0x16000, 0x17fff).bankr("bank1");
	map(0x18000, 0x19fff).bankr("bank2");
	map(0x1a000, 0x1bfff).bankr("bank3");
	map(0x14000, 0x1bfff).w(FUNC(pengadvb_state::megarom_bank_w));

	// slot 3, 16KB RAM
	map(0x3c000, 0x3ffff).ram();
}

void pengadvb_state::io_mem(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x98, 0x99).rw("tms9128", FUNC(tms9128_device::read), FUNC(tms9128_device::write));
	map(0xa0, 0xa1).w("aysnd", FUNC(ay8910_device::address_data_w));
	map(0xa2, 0xa2).r("aysnd", FUNC(ay8910_device::data_r));
	map(0xa8, 0xab).rw("ppi8255", FUNC(i8255_device::read), FUNC(i8255_device::write));
}


/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( pengadvb )
	PORT_START("IN0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	// bit 1 is also tested, unknown purpose.
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(0xee, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( pengadvb2 ) // reads are scrambled
	PORT_START("IN0")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_START1 ) // also used for button 2 (pistol)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0xa4, 0xa4, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x84, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xa4, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x24, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_5C ) )
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:8")
INPUT_PORTS_END


/***************************************************************************

  IC Interfaces

***************************************************************************/

// AY8910
void pengadvb_state::psg_port_b_w(uint8_t data)
{
	// leftover from msx ver?
}

/**************************************************************************/

// I8255
uint8_t pengadvb_state::ppi_port_a_r()
{
	return m_primary_slot_reg;
}

void pengadvb_state::ppi_port_a_w(uint8_t data)
{
	if (data != m_primary_slot_reg)
	{
		for (int i = 0; i < 4; i++)
			m_page[i]->set_bank(data >> (i * 2) & 3);

		m_primary_slot_reg = data;
	}
}

uint8_t pengadvb_state::ppi_port_b_r()
{
	switch (m_kb_matrix_row)
	{
		case 0x00:
			return ioport("IN1")->read();
		case 0x01:
			return ioport("IN2")->read();
		case 0x04:
			return ioport("DSW1")->read();
		default:
			break;
	}

	return 0xff;
}

void pengadvb_state::ppi_port_c_w(uint8_t data)
{
	m_kb_matrix_row = data & 0x0f;
}


/***************************************************************************

  Machine config(s)

***************************************************************************/

void pengadvb_state::pengadvb(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(10'738'635)/3);
	m_maincpu->set_addrmap(AS_PROGRAM, &pengadvb_state::program_mem);
	m_maincpu->set_addrmap(AS_IO, &pengadvb_state::io_mem);

	ADDRESS_MAP_BANK(config, "page0").set_map(&pengadvb_state::bank_mem).set_options(ENDIANNESS_LITTLE, 8, 18, 0x10000);
	ADDRESS_MAP_BANK(config, "page1").set_map(&pengadvb_state::bank_mem).set_options(ENDIANNESS_LITTLE, 8, 18, 0x10000);
	ADDRESS_MAP_BANK(config, "page2").set_map(&pengadvb_state::bank_mem).set_options(ENDIANNESS_LITTLE, 8, 18, 0x10000);
	ADDRESS_MAP_BANK(config, "page3").set_map(&pengadvb_state::bank_mem).set_options(ENDIANNESS_LITTLE, 8, 18, 0x10000);

	i8255_device &ppi(I8255(config, "ppi8255"));
	ppi.in_pa_callback().set(FUNC(pengadvb_state::ppi_port_a_r));
	ppi.out_pa_callback().set(FUNC(pengadvb_state::ppi_port_a_w));
	ppi.in_pb_callback().set(FUNC(pengadvb_state::ppi_port_b_r));
	ppi.out_pc_callback().set(FUNC(pengadvb_state::ppi_port_c_w));

	/* video hardware */
	tms9128_device &vdp(TMS9128(config, "tms9128", XTAL(10'738'635)));
	vdp.set_screen("screen");
	vdp.set_vram_size(0x4000);
	vdp.int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ay8910_device &aysnd(AY8910(config, "aysnd", XTAL(10'738'635)/6));
	aysnd.port_a_read_callback().set_ioport("IN0");
	aysnd.port_b_write_callback().set(FUNC(pengadvb_state::psg_port_b_w));
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.50);
}


/***************************************************************************

  Machine start/init

***************************************************************************/

void pengadvb_state::machine_start()
{
	save_item(NAME(m_primary_slot_reg));
	save_item(NAME(m_kb_matrix_row));
}

void pengadvb_state::machine_reset()
{
	m_primary_slot_reg = 0;
	m_kb_matrix_row = 0;

	for (int i = 0; i < 4; i++)
	{
		m_page[i]->set_bank(0);
		m_bank[i]->set_entry(i);
	}
}

void pengadvb_state::pengadvb_decrypt(const char* region)
{
	uint8_t *mem = memregion(region)->base();
	uint32_t memsize = memregion(region)->bytes();

	// data lines swap
	for (int i = 0; i < memsize; i++)
		mem[i] = bitswap<8>(mem[i],7,6,5,3,4,2,1,0);

	// address line swap
	std::vector<uint8_t> buf(memsize);
	memcpy(&buf[0], mem, memsize);
	for (int i = 0; i < memsize; i++)
		mem[i] = buf[bitswap<24>(i,23,22,21,20,19,18,17,16,15,14,13,5,11,10,9,8,7,6,12,4,3,2,1,0)];
}

void pengadvb_state::init_pengadvb()
{
	pengadvb_decrypt("maincpu");
	pengadvb_decrypt("game");

	init_pengadvb2();
}

void pengadvb_state::init_pengadvb2()
{
	// init banks
	for (int i = 0; i < 4; i++)
		m_bank[i]->configure_entries(0, 0x10, memregion("game")->base(), 0x2000);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( pengadvb )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "rom.u5", 0x00000, 0x8000, CRC(d21950d2) SHA1(0b1815677f17a680ba63c3839bea2d451813eec8) )

	ROM_REGION( 0x20000, "game", 0 )
	ROM_LOAD( "rom.u7",  0x00000, 0x8000, CRC(d4b4a4a4) SHA1(59f9299182fd8aedc7a4e9b0ddd685f2a71c033f) )
	ROM_LOAD( "rom.u8",  0x08000, 0x8000, CRC(eada2232) SHA1(f4182f0921b621acd8be6077eb9639b31a97e907) )
	ROM_LOAD( "rom.u9",  0x10000, 0x8000, CRC(6478c561) SHA1(6f9a794a5bb51e96552f6d1e9fa6515659d25933) )
	ROM_LOAD( "rom.u10", 0x18000, 0x8000, CRC(5c48360f) SHA1(0866e20969f57b7b7c59df8f7ca203f18c7c9870) )
ROM_END

ROM_START( pengadvb2 ) // CBK1029 PCB
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "7l",  0x00000, 0x8000, CRC(9645ab69) SHA1(7a15b501d2c357b9fda83c811b0d728df318ceb2) )

	ROM_REGION( 0x20000, "game", 0 )
	ROM_LOAD( "7a",  0x00000, 0x8000, CRC(8434344c) SHA1(c3002df12fb5395506a16abcefbcb4f5cbe3eb6a) )
	ROM_LOAD( "7c",  0x08000, 0x8000, CRC(0274f6eb) SHA1(185b3819357abf65988971e9deece3b5c67dd1d0) )
	ROM_LOAD( "7d",  0x10000, 0x8000, CRC(8cb1f223) SHA1(ff5db3c373e6d919b4e8e06c3e4607b150f31964) )
	ROM_LOAD( "7e",  0x18000, 0x8000, CRC(60764899) SHA1(a75e59c2ecf8cebdb99708cb18390157ad7b6993) )
ROM_END

} // anonymous namespace


GAME( 1988, pengadvb,  0,        pengadvb, pengadvb,  pengadvb_state, init_pengadvb,  ROT0, "bootleg (Screen)", "Penguin Adventure (bootleg of MSX version, encrypted)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, pengadvb2, pengadvb, pengadvb, pengadvb2, pengadvb_state, init_pengadvb2, ROT0, "bootleg (Comet)", "Penguin Adventure (bootleg of MSX version, not encrypted)", MACHINE_SUPPORTS_SAVE )
