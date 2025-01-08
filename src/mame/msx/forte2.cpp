// license:BSD-3-Clause
// copyright-holders:hap, Mariusz Wojcieszek
/*******************************************************************************

Brazilian bootleg board from 1989. Forte II Games, Industria Brasileira.

MAME driver by Mariusz Wojcieszek & hap, based on
information from Alexandre Souza (a.k.a. "Tabajara").

Hardware is based on MSX1, excluding i8255 PPI:
- 64KB RAM, largely unused
- 64KB EPROM (2764-15, contains hacked BIOS and game ROM)
- Z80 @ 3.58MHz
- GI AY-3-8910
- TI TMS9928A
- (no dipswitches)

Pesadelo (means 'nightmare' in Portuguese), 1989 bootleg of Knightmare (Majou
Densetsu in Japan) (C) 1986 Konami, originally released exclusively on MSX.
This arcade conversion has been made a bit harder, eg. bonus power-ups deplete
three times quicker, and the game starts at a later, more difficult level.
A precise translation of the Brazilian Portuguese text displayed upon
inserting a coin is:

    NIGHTMARE DIFFICULTY-LEVEL 2 DOES NOT ACCUMULATE
    CREDITS , ONLY INSERT A NEW
    COIN AFTER THE END OF THE GAME
    IN ORDER TO START THE GAME PRESS
    THE FIRE BUTTON.

               GOOD LUCK!

If the coin detector is activated for a few seconds, an error message
FICHA PRESA (meaning STUCK COIN) shows up blinking and beeping.

According to Alexandre, there are more games for this board, but not
found/dumped yet.

*******************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "video/tms9928a.h"
#include "sound/ay8910.h"

#include "screen.h"
#include "speaker.h"


namespace {

class forte2_state : public driver_device
{
public:
	forte2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_inputs(*this, "IN0")
	{ }

	void init_pesadelo();
	void pesadelo(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_ioport m_inputs;

	u8 m_input_mask = 0xff;

	u8 input_r();
	void input_mask_w(u8 data);

	void io_mem(address_map &map) ATTR_COLD;
	void program_mem(address_map &map) ATTR_COLD;
};



/*******************************************************************************
    Initialization
*******************************************************************************/

void forte2_state::init_pesadelo()
{
	u8 *mem = memregion("maincpu")->base();
	int memsize = memregion("maincpu")->bytes();

	// data swap
	for (int i = 0; i < memsize; i++)
	{
		mem[i] = bitswap<8>(mem[i],3,5,6,7,0,4,2,1);
	}

	// address line swap
	std::vector<u8> buf(&mem[0], &mem[memsize]);

	for (int i = 0; i < memsize; i++)
	{
		mem[bitswap<16>(i,11,9,8,13,14,15,12,7,6,5,4,3,2,1,0,10)] = buf[i];
	}
}

void forte2_state::machine_start()
{
	save_item(NAME(m_input_mask));
}



/*******************************************************************************
    I/O
*******************************************************************************/

u8 forte2_state::input_r()
{
	return m_inputs->read() | (m_input_mask & 0x7f);
}

void forte2_state::input_mask_w(u8 data)
{
	// PSG reg 15, writes 0 at coin insert, 0xff at boot and game over
	m_input_mask = data;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void forte2_state::program_mem(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xffff).ram();
}

void forte2_state::io_mem(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x98, 0x99).rw("tms9928a", FUNC(tms9928a_device::read), FUNC(tms9928a_device::write));
	map(0xa0, 0xa1).w("aysnd", FUNC(ay8910_device::address_data_w));
	map(0xa2, 0xa2).r("aysnd", FUNC(ay8910_device::data_r));
	map(0xa8, 0xab).noprw(); // no 8255
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( pesadelo )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void forte2_state::pesadelo(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 3.579545_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &forte2_state::program_mem);
	m_maincpu->set_addrmap(AS_IO, &forte2_state::io_mem);

	// video hardware
	tms9928a_device &vdp(TMS9928A(config, "tms9928a", 10.738635_MHz_XTAL));
	vdp.set_screen("screen");
	vdp.set_vram_size(0x4000);
	vdp.int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", 3.579545_MHz_XTAL/2));
	aysnd.port_a_read_callback().set(FUNC(forte2_state::input_r));
	aysnd.port_b_write_callback().set(FUNC(forte2_state::input_mask_w));
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.50);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( pesadelo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr2764.15", 0x00000, 0x10000, CRC(1ae2f724) SHA1(12880dd7ad82acf04861843fb9d4f0f926d18f6b) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT  MACHINE   INPUT     CLASS         INIT           SCREEN  COMPANY                     FULLNAME                                   FLAGS
GAME( 1989, pesadelo, 0,      pesadelo, pesadelo, forte2_state, init_pesadelo, ROT0,   "bootleg (Forte II Games)", "Pesadelo (bootleg of Konami Knightmare)", MACHINE_SUPPORTS_SAVE )
