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

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/tms9928a.h"
#include "sound/ay8910.h"
#include "machine/i8255.h"
#include "machine/bankdev.h"


class pengadvb_state : public driver_device
{
public:
	pengadvb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	required_device<cpu_device> m_maincpu;

	address_map_bank_device *m_page[4];
	memory_bank *m_bank[4];
	UINT8 m_primary_slot_reg;
	UINT8 m_kb_matrix_row;

	DECLARE_READ8_MEMBER(mem_r);
	DECLARE_WRITE8_MEMBER(mem_w);
	DECLARE_WRITE8_MEMBER(megarom_bank_w);

	DECLARE_WRITE8_MEMBER(pengadvb_psg_port_b_w);
	DECLARE_READ8_MEMBER(pengadvb_ppi_port_a_r);
	DECLARE_WRITE8_MEMBER(pengadvb_ppi_port_a_w);
	DECLARE_READ8_MEMBER(pengadvb_ppi_port_b_r);
	DECLARE_WRITE8_MEMBER(pengadvb_ppi_port_c_w);

	DECLARE_DRIVER_INIT(pengadvb);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void pengadvb_decrypt(const char* region);
};


/***************************************************************************

  Z80 Memory map

***************************************************************************/

READ8_MEMBER(pengadvb_state::mem_r)
{
	return m_page[offset >> 14 & 3]->read8(space, offset);
}

WRITE8_MEMBER(pengadvb_state::mem_w)
{
	m_page[offset >> 14 & 3]->write8(space, offset, data);
}

WRITE8_MEMBER(pengadvb_state::megarom_bank_w)
{
	m_bank[offset >> 13 & 3]->set_entry(data & 0xf);
}

static ADDRESS_MAP_START( program_mem, AS_PROGRAM, 8, pengadvb_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(mem_r, mem_w) // 4 pages of 16KB
ADDRESS_MAP_END

static ADDRESS_MAP_START( bank_mem, AS_PROGRAM, 8, pengadvb_state )
	// slot 0, MSX BIOS
	AM_RANGE(0x00000, 0x07fff) AM_ROM AM_REGION("maincpu", 0)

	// slot 1, MegaROM
	AM_RANGE(0x14000, 0x15fff) AM_ROMBANK("bank0")
	AM_RANGE(0x16000, 0x17fff) AM_ROMBANK("bank1")
	AM_RANGE(0x18000, 0x19fff) AM_ROMBANK("bank2")
	AM_RANGE(0x1a000, 0x1bfff) AM_ROMBANK("bank3")
	AM_RANGE(0x14000, 0x1bfff) AM_WRITE(megarom_bank_w)

	// slot 3, 16KB RAM
	AM_RANGE(0x3c000, 0x3ffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_mem, AS_IO, 8, pengadvb_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x98, 0x98) AM_DEVREADWRITE("tms9128", tms9128_device, vram_read, vram_write)
	AM_RANGE(0x99, 0x99) AM_DEVREADWRITE("tms9128", tms9128_device, register_read, register_write)
	AM_RANGE(0xa0, 0xa1) AM_DEVWRITE("aysnd", ay8910_device, address_data_w)
	AM_RANGE(0xa2, 0xa2) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE(0xa8, 0xab) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)
ADDRESS_MAP_END


/***************************************************************************

  Inputs

***************************************************************************/

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


/***************************************************************************

  IC Interfaces

***************************************************************************/

// AY8910
WRITE8_MEMBER(pengadvb_state::pengadvb_psg_port_b_w)
{
	// leftover from msx ver?
}

/**************************************************************************/

// I8255
READ8_MEMBER(pengadvb_state::pengadvb_ppi_port_a_r)
{
	return m_primary_slot_reg;
}

WRITE8_MEMBER(pengadvb_state::pengadvb_ppi_port_a_w)
{
	if (data != m_primary_slot_reg)
	{
		for (int i = 0; i < 4; i++)
			m_page[i]->set_bank(data >> (i * 2) & 3);

		m_primary_slot_reg = data;
	}
}

READ8_MEMBER(pengadvb_state::pengadvb_ppi_port_b_r)
{
	// TODO: dipswitch
	switch (m_kb_matrix_row)
	{
		case 0x0:
			return ioport("IN1")->read();

		default:
			break;
	}

	return 0xff;
}

WRITE8_MEMBER(pengadvb_state::pengadvb_ppi_port_c_w)
{
	m_kb_matrix_row = data & 0x0f;
}

/***************************************************************************

  Machine config(s)

***************************************************************************/

static MACHINE_CONFIG_START( pengadvb, pengadvb_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_10_738635MHz/3)
	MCFG_CPU_PROGRAM_MAP(program_mem)
	MCFG_CPU_IO_MAP(io_mem)

	// -_-;
	MCFG_DEVICE_ADD("page0", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(bank_mem)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(18)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x10000)

	MCFG_DEVICE_ADD("page1", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(bank_mem)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(18)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x10000)

	MCFG_DEVICE_ADD("page2", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(bank_mem)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(18)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x10000)

	MCFG_DEVICE_ADD("page3", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(bank_mem)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(18)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x10000)

	MCFG_DEVICE_ADD("ppi8255", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(pengadvb_state, pengadvb_ppi_port_a_r))
	MCFG_I8255_OUT_PORTA_CB(WRITE8(pengadvb_state, pengadvb_ppi_port_a_w))
	MCFG_I8255_IN_PORTB_CB(READ8(pengadvb_state, pengadvb_ppi_port_b_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(pengadvb_state, pengadvb_ppi_port_c_w))

	/* video hardware */
	MCFG_DEVICE_ADD("tms9128", TMS9128, XTAL_10_738635MHz/2)
	MCFG_TMS9928A_VRAM_SIZE(0x4000)
	MCFG_TMS9928A_OUT_INT_LINE_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_TMS9928A_SCREEN_ADD_NTSC( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE("tms9128", tms9128_device, screen_update)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8910, XTAL_10_738635MHz/6)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("IN0"))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(pengadvb_state, pengadvb_psg_port_b_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


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
	UINT8 *mem = memregion(region)->base();
	int memsize = memregion(region)->bytes();

	// data lines swap
	for (int i = 0; i < memsize; i++)
	{
		mem[i] = BITSWAP8(mem[i],7,6,5,3,4,2,1,0);
	}

	// address line swap
	dynamic_buffer buf(memsize);
	memcpy(&buf[0], mem, memsize);
	for (int i = 0; i < memsize; i++)
	{
		mem[i] = buf[BITSWAP24(i,23,22,21,20,19,18,17,16,15,14,13,5,11,10,9,8,7,6,12,4,3,2,1,0)];
	}
}

DRIVER_INIT_MEMBER(pengadvb_state,pengadvb)
{
	pengadvb_decrypt("maincpu");
	pengadvb_decrypt("game");

	// init banks
	static const char * const pagenames[] = { "page0", "page1", "page2", "page3" };
	static const char * const banknames[] = { "bank0", "bank1", "bank2", "bank3" };
	for (int i = 0; i < 4; i++)
	{
		m_page[i] = machine().device<address_map_bank_device>(pagenames[i]);

		m_bank[i] = membank(banknames[i]);
		m_bank[i]->configure_entries(0, 0x10, memregion("game")->base(), 0x2000);
	}
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


GAME( 1988, pengadvb, 0, pengadvb, pengadvb, pengadvb_state, pengadvb, ROT0, "bootleg (Screen) / Konami", "Penguin Adventure (bootleg of MSX version)", MACHINE_SUPPORTS_SAVE )
