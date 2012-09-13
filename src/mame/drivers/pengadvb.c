/*

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
10 position DIPSW (where are they read??)
NOTE! switches 1, 3 & 5 must be ON or the game will not boot.
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/tms9928a.h"
#include "sound/ay8910.h"
#include "machine/i8255.h"


class pengadvb_state : public driver_device
{
public:
	pengadvb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_main_mem;
	UINT8 m_mem_map;
	UINT8 m_mem_banks[4];
	DECLARE_WRITE8_MEMBER(mem_w);
	DECLARE_READ8_MEMBER(pengadvb_psg_port_a_r);
	DECLARE_READ8_MEMBER(pengadvb_ppi_port_a_r);
	DECLARE_WRITE8_MEMBER(pengadvb_ppi_port_a_w);
	DECLARE_READ8_MEMBER(pengadvb_ppi_port_b_r);
	DECLARE_WRITE_LINE_MEMBER(vdp_interrupt);
	DECLARE_DRIVER_INIT(pengadvb);
	virtual void machine_start();
	virtual void machine_reset();
};



static void mem_map_banks(running_machine &machine)
{
	pengadvb_state *state = machine.driver_data<pengadvb_state>();
	/*  memorymap: (rest is assumed unmapped)
        slot 0
            0000-7fff   BIOS ROM
        slot 1
            4000-bfff   game ROM
        slot 3
            c000-ffff   RAM
    */

	// page 0 (0000-3fff)
	switch(state->m_mem_map & 3)
	{
		case 0:
			// BIOS
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_bank(0x0000, 0x3fff, "bank1" );
			state->membank("bank1")->set_base(state->memregion("maincpu")->base());
			break;

		default:
			machine.device("maincpu")->memory().space(AS_PROGRAM)->unmap_read(0x0000, 0x3fff);
			break;
	}

	// page 1 (4000-7fff)
	switch(state->m_mem_map >> 2 & 3)
	{
		case 0:
			// BIOS
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_bank(0x4000, 0x5fff, "bank21" );
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_bank(0x6000, 0x7fff, "bank22" );
			state->membank("bank21")->set_base(machine.root_device().memregion("maincpu")->base() + 0x4000);
			state->membank("bank22")->set_base(machine.root_device().memregion("maincpu")->base() + 0x4000 + 0x2000);
			break;

		case 1:
			// game
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_bank(0x4000, 0x5fff, "bank21" );
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_bank(0x6000, 0x7fff, "bank22" );
			state->membank("bank21")->set_base(machine.root_device().memregion("game")->base() + state->m_mem_banks[0]*0x2000);
			state->membank("bank22")->set_base(machine.root_device().memregion("game")->base() + state->m_mem_banks[1]*0x2000);
			break;

		default:
			machine.device("maincpu")->memory().space(AS_PROGRAM)->unmap_read(0x4000, 0x7fff);
			break;
	}

	// page 2 (8000-bfff)
	switch(state->m_mem_map >> 4 & 3)
	{
		case 1:
			// game
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_bank(0x8000, 0x9fff, "bank31" );
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_bank(0xa000, 0xbfff, "bank32" );
			state->membank("bank31")->set_base(machine.root_device().memregion("game")->base() + state->m_mem_banks[2]*0x2000);
			state->membank("bank32")->set_base(machine.root_device().memregion("game")->base() + state->m_mem_banks[3]*0x2000);
			break;

		default:
			machine.device("maincpu")->memory().space(AS_PROGRAM)->unmap_read(0x8000, 0xbfff);
			break;
	}

	// page 3 (c000-ffff)
	switch(state->m_mem_map >> 6 & 3)
	{
		case 3:
			// RAM
			machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_bank(0xc000, 0xffff, "bank4" );
			state->membank("bank4")->set_base(state->m_main_mem);
			break;

		default:
			machine.device("maincpu")->memory().space(AS_PROGRAM)->unmap_read(0xc000, 0xffff);
			break;
	}
}

WRITE8_MEMBER(pengadvb_state::mem_w)
{
	if (offset >= 0xc000)
	{
		// write to RAM
		if ((m_mem_map >> 6 & 3) == 3)
			m_main_mem[offset - 0xc000] = data;
	}
	else if (offset >= 0x4000 && (m_mem_map >> (offset >> 13 & 6) & 3) == 1 && (m_mem_banks[(offset - 0x4000) >> 13] != (data & 0xf)))
	{
		// ROM bankswitch
		m_mem_banks[(offset - 0x4000) >> 13] = data & 0xf;
		mem_map_banks(machine());
	}
}


static ADDRESS_MAP_START( program_mem, AS_PROGRAM, 8, pengadvb_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROMBANK("bank1")
	AM_RANGE(0x4000, 0x5fff) AM_ROMBANK("bank21")
	AM_RANGE(0x6000, 0x7fff) AM_ROMBANK("bank22")
	AM_RANGE(0x8000, 0x9fff) AM_ROMBANK("bank31")
	AM_RANGE(0xa000, 0xbfff) AM_ROMBANK("bank32")
	AM_RANGE(0xc000, 0xffff) AM_ROMBANK("bank4")
	AM_RANGE(0x0000, 0xffff) AM_WRITE(mem_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_mem, AS_IO, 8, pengadvb_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x98, 0x98) AM_DEVREADWRITE( "tms9928a", tms9928a_device, vram_read, vram_write )
	AM_RANGE(0x99, 0x99) AM_DEVREADWRITE( "tms9928a", tms9928a_device, register_read, register_write )
	AM_RANGE(0xa0, 0xa1) AM_DEVWRITE_LEGACY("aysnd", ay8910_address_data_w)
	AM_RANGE(0xa2, 0xa2) AM_DEVREAD_LEGACY("aysnd", ay8910_r)
	AM_RANGE(0xa8, 0xab) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)
ADDRESS_MAP_END

static INPUT_PORTS_START( pengadvb )
	PORT_START("IN0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2)
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_COIN1) PORT_IMPULSE(1)
	PORT_BIT(0xfe, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


READ8_MEMBER(pengadvb_state::pengadvb_psg_port_a_r)
{
	return machine().root_device().ioport("IN0")->read();
}

static const ay8910_interface pengadvb_ay8910_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_DRIVER_MEMBER(pengadvb_state,pengadvb_psg_port_a_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

READ8_MEMBER(pengadvb_state::pengadvb_ppi_port_a_r)
{
	return m_mem_map;
}

WRITE8_MEMBER(pengadvb_state::pengadvb_ppi_port_a_w)
{
	if (data != m_mem_map)
	{
		m_mem_map = data;
		mem_map_banks(machine());
	}
}

READ8_MEMBER(pengadvb_state::pengadvb_ppi_port_b_r)
{
	i8255_device *ppi = machine().device<i8255_device>("ppi8255");
	if ((ppi->read(space, 2) & 0x0f) == 0)
		return machine().root_device().ioport("IN1")->read();

	return 0xff;
}

static I8255A_INTERFACE(pengadvb_ppi8255_interface)
{
	DEVCB_DRIVER_MEMBER(pengadvb_state,pengadvb_ppi_port_a_r),
	DEVCB_DRIVER_MEMBER(pengadvb_state,pengadvb_ppi_port_a_w),
	DEVCB_DRIVER_MEMBER(pengadvb_state,pengadvb_ppi_port_b_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

WRITE_LINE_MEMBER(pengadvb_state::vdp_interrupt)
{
	machine().device("maincpu")->execute().set_input_line(0, (state ? ASSERT_LINE : CLEAR_LINE));
}

static TMS9928A_INTERFACE(pengadvb_tms9928a_interface)
{
	"screen",
	0x4000,
	DEVCB_DRIVER_LINE_MEMBER(pengadvb_state,vdp_interrupt)
};

static void pengadvb_postload(running_machine &machine)
{
	mem_map_banks(machine);
}

void pengadvb_state::machine_start()
{

	state_save_register_global_pointer(machine(), m_main_mem, 0x4000);
	state_save_register_global(machine(), m_mem_map);
	state_save_register_global_array(machine(), m_mem_banks);
	machine().save().register_postload(save_prepost_delegate(FUNC(pengadvb_postload), &machine()));
}

void pengadvb_state::machine_reset()
{

	m_mem_map = 0;
	m_mem_banks[0] = m_mem_banks[1] = m_mem_banks[2] = m_mem_banks[3] = 0;
	mem_map_banks(machine());
}


static MACHINE_CONFIG_START( pengadvb, pengadvb_state )

	MCFG_CPU_ADD("maincpu", Z80, XTAL_10_738635MHz/3)		  /* 3.579545 Mhz */
	MCFG_CPU_PROGRAM_MAP(program_mem)
	MCFG_CPU_IO_MAP(io_mem)


    MCFG_I8255_ADD( "ppi8255", pengadvb_ppi8255_interface)

	/* video hardware */
	MCFG_TMS9928A_ADD( "tms9928a", TMS9928A, pengadvb_tms9928a_interface )
	MCFG_TMS9928A_SCREEN_ADD_NTSC( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE( "tms9928a", tms9928a_device, screen_update )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8910, (float)XTAL_10_738635MHz/6)
	MCFG_SOUND_CONFIG(pengadvb_ay8910_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

static void pengadvb_decrypt(running_machine &machine, const char* region)
{
	UINT8 *mem = machine.root_device().memregion(region)->base();
	int memsize = machine.root_device().memregion(region)->bytes();
	UINT8 *buf;
	int i;

	// data lines swap
	for ( i = 0; i < memsize; i++ )
	{
		mem[i] = BITSWAP8(mem[i],7,6,5,3,4,2,1,0);
	}

	// address line swap
	buf = auto_alloc_array(machine, UINT8, memsize);
	memcpy(buf, mem, memsize);
	for ( i = 0; i < memsize; i++ )
	{
		mem[i] = buf[BITSWAP24(i,23,22,21,20,19,18,17,16,15,14,13,5,11,10,9,8,7,6,12,4,3,2,1,0)];
	}
	auto_free(machine, buf);
}

DRIVER_INIT_MEMBER(pengadvb_state,pengadvb)
{
	pengadvb_decrypt(machine(), "maincpu");
	pengadvb_decrypt(machine(), "game");

	m_main_mem = auto_alloc_array(machine(), UINT8, 0x4000);
}

ROM_START( pengadvb )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "rom.u5", 0x00000, 0x8000, CRC(d21950d2) SHA1(0b1815677f17a680ba63c3839bea2d451813eec8) )

	ROM_REGION( 0x20000, "game", 0 )
	ROM_LOAD( "rom.u7",  0x00000, 0x8000, CRC(d4b4a4a4) SHA1(59f9299182fd8aedc7a4e9b0ddd685f2a71c033f) )
	ROM_LOAD( "rom.u8",  0x08000, 0x8000, CRC(eada2232) SHA1(f4182f0921b621acd8be6077eb9639b31a97e907) )
	ROM_LOAD( "rom.u9",  0x10000, 0x8000, CRC(6478c561) SHA1(6f9a794a5bb51e96552f6d1e9fa6515659d25933) )
	ROM_LOAD( "rom.u10", 0x18000, 0x8000, CRC(5c48360f) SHA1(0866e20969f57b7b7c59df8f7ca203f18c7c9870) )

ROM_END

GAME( 1988, pengadvb, 0, pengadvb, pengadvb, pengadvb_state, pengadvb, ROT0, "bootleg (Screen) / Konami", "Penguin Adventure (bootleg of MSX version)", GAME_SUPPORTS_SAVE )
