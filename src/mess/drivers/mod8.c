/***************************************************************************

        Microsystems International Limited MOD-8

        M.I.L. was formed in 1968 from a joint venture between the Canadian
        Government and Northern Telecom. It produced a variety of computer
        chips, eproms, etc, plus parts for the telephone company. It folded
        in 1975.
        (Info from http://www.cse.yorku.ca/museum/v_tour/artifacts/artifacts.htm)


        14/06/2011 Modernised & above notes added.
        02/12/2009 Working driver [Miodrag Milanovic]
        18/11/2009 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/i8008/i8008.h"
#include "machine/teleprinter.h"

class mod8_state : public driver_device
{
public:
	mod8_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_teleprinter(*this, TELEPRINTER_TAG)
	{ }

	required_device<teleprinter_device> m_teleprinter;
	DECLARE_WRITE8_MEMBER(out_w);
	DECLARE_WRITE8_MEMBER(tty_w);
	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_READ8_MEMBER(tty_r);
	UINT16 m_tty_data;
	UINT8 m_tty_key_data;
	int m_tty_cnt;
};

WRITE8_MEMBER( mod8_state::out_w )
{
	m_tty_data >>= 1;
	m_tty_data |= BIT(data, 0) ? 0x8000 : 0;
	m_tty_cnt++;

	if (m_tty_cnt == 10)
	{
		m_teleprinter->write(space, 0, (m_tty_data >> 7) & 0x7f);
		m_tty_cnt = 0;
	}
}

WRITE8_MEMBER( mod8_state::tty_w )
{
	m_tty_data = 0;
	m_tty_cnt = 0;
}

READ8_MEMBER( mod8_state::tty_r )
{
	UINT8 d = m_tty_key_data & 1;
	m_tty_key_data >>= 1;
	return d;
}

static ADDRESS_MAP_START(mod8_mem, AS_PROGRAM, 8, mod8_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000,0x6ff) AM_ROM
	AM_RANGE(0x700,0xfff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(mod8_io, AS_IO, 8, mod8_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00,0x00) AM_READ(tty_r)
	AM_RANGE(0x0a,0x0a) AM_WRITE(out_w)
	AM_RANGE(0x0b,0x0b) AM_WRITE(tty_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( mod8 )
INPUT_PORTS_END

static IRQ_CALLBACK ( mod8_irq_callback )
{
	return 0xC0; // LAA - NOP equivalent
}

static MACHINE_RESET(mod8)
{
	device_set_irq_callback(machine.device("maincpu"), mod8_irq_callback);
}

WRITE8_MEMBER( mod8_state::kbd_put )
{
	m_tty_key_data = data ^ 0xff;
	cputag_set_input_line(machine(), "maincpu", 0, HOLD_LINE);
}

static GENERIC_TELEPRINTER_INTERFACE( teleprinter_intf )
{
	DEVCB_DRIVER_MEMBER(mod8_state, kbd_put)
};

static MACHINE_CONFIG_START( mod8, mod8_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8008, 800000)
	MCFG_CPU_PROGRAM_MAP(mod8_mem)
	MCFG_CPU_IO_MAP(mod8_io)

	MCFG_MACHINE_RESET(mod8)

	/* video hardware */
	MCFG_GENERIC_TELEPRINTER_ADD(TELEPRINTER_TAG, teleprinter_intf)
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( mod8 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "mon8.001", 0x0000, 0x0100, CRC(b82ac6b8) SHA1(fbea5a6dd4c779ca1671d84089f857a3f548ffcb))
	ROM_LOAD( "mon8.002", 0x0100, 0x0100, CRC(8b82bc3c) SHA1(66222511527b27e56a5a1f9656d424d407eac7d3))
	ROM_LOAD( "mon8.003", 0x0200, 0x0100, CRC(679ae913) SHA1(22423efcb9051c9812fcbac9a27af70415d0dd81))
	ROM_LOAD( "mon8.004", 0x0300, 0x0100, CRC(2a4e580f) SHA1(8b0cb9660fde3cacd299faaa31724e4f3262d77f))
	ROM_LOAD( "mon8.005", 0x0400, 0x0100, CRC(e281bb1a) SHA1(cc7c2746e075512dbf5eed88ae3aea009558dbd0))
	ROM_LOAD( "mon8.006", 0x0500, 0x0100, CRC(b7e2f585) SHA1(5408adabc3df6e6ea8dcfb2327b2883b435ab85e))
	ROM_LOAD( "mon8.007", 0x0600, 0x0100, CRC(49a5c626) SHA1(66c1865db9151818d3b20ec3c68dd793cb98a221))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT   COMPANY                      FULLNAME       FLAGS */
COMP( 1974, mod8,   0,       0,      mod8,      mod8, driver_device,    0, "Microsystems International Ltd", "MOD-8", GAME_NO_SOUND_HW)

